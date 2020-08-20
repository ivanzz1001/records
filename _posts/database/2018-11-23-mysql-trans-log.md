---
layout: post
title: MySQL事务日志
tags:
- database
categories: database
description:  MySQL事务日志
---

innodb事务日志包括redo log和undo log。redo log是重做日志，提供前滚操作；undo log是回滚日志，提供回滚操作。

undo log不是redo log的逆向过程，其实它们都算是用来恢复的日志：

* redo log通常是物理日志，记录的是数据页的物理修改，而不是某一行或几行修改成怎样怎样，它用来恢复提交后的物理数据页（恢复数据页，且只能恢复到最后一次提交的位置）。

* undo log用来回滚行记录到某个版本。undo log一般是逻辑日志，根据每行记录进行记录。


<!-- more -->

## 1. redo log

### 1.1 redo log和二进制日志的区别
redo log不是二进制日志。虽然二进制日志中也记录了innodb表的很多操作，也能实现重做的功能，但是它们之间有很大的区别。

1） 二进制日志是在存储引擎的上层产生的，不管是什么存储引擎，对数据库进行了修改都会产生二进制日志。而redo log是innodb层产生的，只记录该存储引擎中表的修改，并且二进制日志先于redo log被记录，具体见后文group commit小结。

2) 二进制日志记录操作的方法是逻辑性的语句。即便它是基于行格式的记录方式，其本质也还是逻辑的SQL设置，如该行记录的每列的值是多少。而redo log是在物理格式上的日志，它记录的是数据库中每个页的修改。

3） 二进制日志只在每次事务提交的时候一次性写入缓存中的日志文件(对于非事务表的操作，则是每次执行语句成功后就直接写入）。而redo log在数据准备修改前写入缓存中的redo log中，然后才对缓存中的数据执行修改操作；而且保证在发出事务提交指令时，先向缓存中的redo log写入日志，写入完成后才执行提交动作。


4） 因为二进制日志只在提交的时候一次性写入，所以二进制日志中的记录方式和提交顺序有关，且一次提交对应一次记录。而redo log中是记录的物理页的修改，redo log文件中同一个事务可能多次修改，最后一个提交的事务记录会覆盖所有未提交的事务记录。例如事务```T1```，可能在redo log中记录了```T1-1```、```T1-2```、```T1-3```、```T1*```共4个操作，其中```T1*```表示最后提交时的日志记录，所以对应的数据页最终状态是```T1*```对应的操作结果。而且redo log是并发写入的，不同事务之间的不同版本的记录会穿插写入到redo log文件中，例如可能redo log的记录方式如下：
<pre>
T1-1, T1-2, T2-1, T2-2, T2*, T1-3, T1* 
</pre>

5) 事务日志记录的是物理页的情况，它具有幂等性，因此记录日志的方式极其简练。幂等性的意思是多次操作前后状态是一样的，例如新插入一行后又删除该行，前后状态没有变化。而二进制日志记录的是所有影响数据的操作，记录的内容较多。例如插入一行记录一次，删除该行又记录一次。

### 1.2 redo log的基本概念
当数据库对数据做修改的时候，需要把数据页从磁盘读到buffer pool中，然后在buffer pool中进行修改，那么这个时候buffer pool中的数据页就与磁盘上的数据页内容不一致，称buffer pool的数据页为dirty page脏数据。如果这个时候发生非正常的DB服务重启，那么这些数据还在内存，并没有同步到磁盘文件中（注意，同步到磁盘文件是个随机IO），也就是会发生数据丢失。但如果这个时候，能够再有一个文件，当buffer pool中的data page变更结束后，把相应修改记录保存到这个文件（注意，记录日志是顺序IO），那么当DB服务发生crash的情况，恢复DB的时候，也可以根据这个文件的记录内容，重新应用到磁盘文件，数据保持一致。

这个文件就是redo log，用于记录数据修改后的记录，顺序记录。它可以带来如下好处：

* 当buffer pool中的dirty page还没有刷新到磁盘的时候，发生crash，启动服务后，可通过redo log找到需要重新刷新到磁盘文件的记录；

* buffer pool中的数据直接flush到disk file，是一个随机IO，效率较差，而把buffer pool中的数据记录到redo log，是一个顺序IO，可以提高事务提交的速度。

假设修改```tba表```中id=2的行数据，把Name='B'修改为Name='B2'，那么redo日志就会用来存放Name='B2'的记录。如果这个修改在flush到磁盘文件时出现异常，可以使用redo log实现重做操作，保证事务的持久性。


![db-mysql-dolog](https://ivanzz1001.github.io/records/assets/img/db/db_dolog_1.jpg)

这里注意下redo log跟binary log的区别，redo log是存储引擎层产生的，而binary log是数据库层产生的。假设一个大事务，对```tba```做10万行的记录插入，在这个过程中，一直不断的往redo log顺序记录，而binary log不会记录，直到这个事务提交，才会一次写入到binary log文件中。


redo log包括两部分： 一是内存中日志缓存(redo log buffer)，该部分日志是易失性的；二是磁盘上的重做日志文件(redo log file)，该部分日志是持久的。

在概念上，innodb通过**```force log at commit```**机制实现事务的持久性，即在事务提交的时候，必须先将该事务的所有事务日志写入到磁盘上的redo log file和undo log file中进行持久化。

为了确保每次日志都能写入到事务日志文件中，在每次将log buffer中的日志写入日志文件的过程中都会调用一次操作系统的fsync()操作。因为MariaDB/MySQL是工作在用户空间的，因此MariaDB/MySQL的log buffer处于用户空间的内存中。要写入到磁盘上的log file中（redo:ib_logfileN文件， undo:share_tablespace或.ibd文件），中间还要经过操作系统内核空间的os buffer，调用fsync()的作用就是将os buffer中的日志刷到磁盘上的log file中。

也就是说，从redo log buffer写日志到磁盘的redo log file中，过程如下：

![db-mysql-dolog](https://ivanzz1001.github.io/records/assets/img/db/db_dolog_2.png)

> 在此处需要注意一点， 一般所说的log file并不是磁盘上的物理日志文件，而是操作系统缓存中的log file，官方手册上的意思也是如此（例如：With a value of 2, the contents of the InnoDB log buffer are written to the log file after each transaction commit and the log file is flushed to disk approximately once per second)。但说实话，这不太好理解。既然都称为file了，应该已经属于物理文件了。所以在本文后续内容中都以os buffer或者file system buffer来表示官方手册中所说的Log File，然后log file则表示磁盘上的物理日志文件，即log file on disk。
>
>另外，子所以要经过一层os buffer，是因为open日志文件的时候，open没有使用O_DIRECT标志位，该标志位意味着绕过操作系统层的os buffer，IO直接写到底层存储设备。不使用该标志位意味着将日志进行缓冲，缓冲到了一定容量，或者显式fsync()才会将缓冲中的数据刷到存储设备。使用该标志位意味着每次都要发起系统调用。比如写abcde，不使用o_direct将只发起一次系统调用，使用o_direct将发起5次系统调用。


MySQL支持用户自定义在commit时如何将log buffer中的日志刷到log file中。这种控制通过变量innodb_flush_log_at_trx_commit的值来决定。该变量有3种值： 0、1、2，默认为1。但注意，这个变量只是控制commit动作是否刷新log buffer到磁盘。


* 当设置为1的时候，事务每次提交都会将log buffer中的日志写入os buffer并调用fsync()刷到log file on disk中。这种方式即使系统崩溃也不会丢失任何数据，但是因为每次提交都写入磁盘，IO的性能较差。

* 当设置为0的时候，事务提交时不会将log buffer中的日志写入到os buffer，而是每秒写入os buffer并调用fsync()写入到log file on disk中。也就是说设置为0时是（大约）每秒刷新写入到磁盘中的，当系统崩溃，会丢失1秒钟的数据。

* 当设置为2的时候，每次提交都仅写入到os buffer，然后是每秒调用fsync()将os buffer中的日志写入到log file on disk。

![db-mysql-dolog](https://ivanzz1001.github.io/records/assets/img/db/db_dolog_3.png)

注意，有一个变量```innodb_flush_log_at_timeout```的值为1秒，该变量表示的是刷日志的频率，很多人误以为是控制 ```innodb_flush_log_at_trx_commit```值为0和2时的1秒频率，实际上并非如此。测试时将频率设置为5和设置为1，当```innodb_flush_log_at_trx_commit```设置为0和2的时候性能基本都是不变的。关于这个频率是控制什么的，在后面的**"刷日志到磁盘的规则"**中会说。


----------
在主从复制结构中，要保证事务的持久性和一致性，需要对日志相关变量设置为如下：

* 如果启用了二进制日志，则设置sync_binlog=1，即每提交一次事务同步写到磁盘中；

* 总是设置innodb_flush_log_at_trx_commit=1，即每提交一次事务都写到磁盘中

上述两项变量的设置保证了：每次提交事务都写入二进制日志和事务日志，并在提交时将它们刷新到磁盘中。

选择刷日志的时间会严重影响数据修改时的性能，特别是刷到磁盘的过程。下例就测试了```innodb_flush_log_at_trx_commit```分别为0、1、2时的差距:

1) **创建测试表**
{% highlight string %}
#创建测试表
drop table if exists test_flush_log;
create table test_flush_log(id int,name char(50))engine=innodb;


#创建插入指定行数的记录到测试表中的存储过程
drop procedure if exists proc;
delimiter $$
create procedure proc(i int)
begin
    declare s int default 1;
    declare c char(50) default repeat('a',50);
    while s<=i do
        start transaction;
        insert into test_flush_log values(null,c);
        commit;
        set s=s+1;
    end while;
end$$
delimiter ;
{% endhighlight %}

2) **innodb_flush_log_at_trx_commit设置为1**

innodb_flush_log_at_trx_commit的值为1，即每次提交都刷日志到磁盘。测试此时插入10W条记录的时间:
{% highlight string %}
mysql> call proc(100000);
Query OK, 0 rows affected (15.48 sec)
{% endhighlight %}

结果是15.48秒。

3) **innodb_flush_log_at_trx_commit设置为2**

innodb_flush_log_at_trx_commit设置为2的时候，即每次提交都刷新到os buffer中，但每秒才刷入磁盘中：
{% highlight string %}
mysql> set @@global.innodb_flush_log_at_trx_commit=2;    
mysql> truncate test_flush_log;

mysql> call proc(100000);
Query OK, 0 rows affected (3.41 sec)
{% endhighlight %}
结果插入时间大减，只需3.41秒。

4） **innodb_flush_log_at_trx_commit设置为0**

innodb_flush_log_at_trx_commit设置为0的时候，即每秒才刷到os buffer和磁盘：
{% highlight string %}
mysql> set @@global.innodb_flush_log_at_trx_commit=0;
mysql> truncate test_flush_log;

mysql> call proc(100000);
Query OK, 0 rows affected (2.10 sec)
{% endhighlight %}
结果只有2.1秒。


最后可以发现，其实值为2和0的时候，它们的差距并不太大，但2却比0要安全的多。它们都是每秒从os buffer刷到磁盘，它们之间的时间差体现在log buffer刷到os buffer上。因为将log buffer中的日志刷新到os buffer只是内存数据的转移，并没有太大的开销，所以每次提交和每秒刷入差距并不大。可以测试插入更多的数据来比较，以下是插入100W行数据的情况。从结果可见，值为2和0的时候差距并不大，但值为1的性能却差太多。


尽管设置为0和2可以大幅度提升插入性能，但是在故障的时候可能会丢失1秒钟数据，这1秒钟很可能有大量的数据，从上面的测试结果看，100W条记录也只消耗了20多秒，1秒钟大约有4W-5W条数据，尽管上述插入的数据简单，但却说明了数据丢失的大量性。更好的插入数据的做法是将值设置为1，然后修改存储过程，将每次循环都提交修改为只提交一次，这样既能保证数据的一致性，也能提升性能，修改如下：
{% highlight string %}
drop procedure if exists proc;
delimiter $$
create procedure proc(i int)
begin
    declare s int default 1;
    declare c char(50) default repeat('a',50);
    start transaction;
    while s<=i DO
        insert into test_flush_log values(null,c);
        set s=s+1;
    end while;
    commit;
end$$
delimiter ;
{% endhighlight %}
测试值为1的情况：
{% highlight string %}
mysql> set @@global.innodb_flush_log_at_trx_commit=1;
mysql> truncate test_flush_log;

mysql> call proc(1000000);
Query OK, 0 rows affected (11.26 sec)
{% endhighlight %}

### 1.3 日志块(log block)
innodb存储引擎中，redo log是以块为单位进行存储的，每个块占用512字节，这称为redo log block。所以不管是log buffer中还是os buffer中以及redo log file on disk中，都是这样以512字节的块存储的。

每个redo log block由3部分组成：

* 日志块头

* 日志块尾

* 日志主体

其中日志块头占用12字节，日志块尾占用8字节，所以每个redo log block的日志主体部分只有512 - 12 -8 = 492字节。

![db-mysql-dolog](https://ivanzz1001.github.io/records/assets/img/db/db_dolog_4.png)

因为redo log记录的是数据页的变化，当一个数据页产生的变化需要使用超过492字节的redo log来记录，那么就会使用多个redo log block来记录该数据页的变化。

日志块头包含4部分：

* log_block_hdr_no: (4字节)该日志块在redo log buffer中的位置ID

* log_block_hdr_data_len： （2字节）该log block中已记录的log大小。写满该log block时为0x200，表示512字节；

* log_block_first_rec_group： （2字节）该log block中第一个log的开始偏移位置；

* log_block_checkpoint_no: (4字节）写入检查点信息的位置；

关于日志块头的第三部分```log_block_first_rec_group```，因为有时候一个数据页产生的日志量超出了一个日志块，这就需要用多个日志块来记录该页的相关日志。例如，某一数据页产生了552字节的日志量，那么需要占用两个日志块，第一个日志块占用492字节，第二个日志块占用60个字节，那么对于第二个日志块来说，它的第一个log的开始位置就是73字节(60+12)。如果该部分的值和```log_block_hdr_data_len```相等，则说明该log block中没有新开始的日志块，即表示该日志块用来延续前一个日志块。

日志尾只有一个部分：log_block_trl_no，该值和块头的log_block_hdr_no相等。

上面说的是一个日志块的内容，在redo log buffer或者redo log file on disk中，由很多log block组成。如下图：

![db-mysql-dolog](https://ivanzz1001.github.io/records/assets/img/db/db_dolog_5.png)

### 1.4 log group和redo log file
log group表示的是redo log group，一个组内由多个大小完全相同的redo log file组成。组内redo log file的数量由变量innodb_log_files_group决定，默认值为2，即两个redo log file。这个组是一个逻辑的概念，并没有真正的文件来表示这是一个组，但是可以通过变量innodb_log_group_home_dir来定义组的目录，redo log file都放在这个目录下，默认是在datadir下。

{% highlight string %}
mysql> show global variables like "innodb_log%";
+-----------------------------+----------+
| Variable_name               | Value    |
+-----------------------------+----------+
| innodb_log_buffer_size      | 8388608  |
| innodb_log_compressed_pages | ON       |
| innodb_log_file_size        | 50331648 |
| innodb_log_files_in_group   | 2        |
| innodb_log_group_home_dir   | ./       |
+-----------------------------+----------+

[root@xuexi data]# ll /mydata/data/ib*
-rw-rw---- 1 mysql mysql 79691776 Mar 30 23:12 /mydata/data/ibdata1
-rw-rw---- 1 mysql mysql 50331648 Mar 30 23:12 /mydata/data/ib_logfile0
-rw-rw---- 1 mysql mysql 50331648 Mar 30 23:12 /mydata/data/ib_logfile1
{% endhighlight %}
可以看到在默认的数据目录下，有两个ib_logfile开头的文件，它们就是log group中的redo log file，而且它们的大小完全一致且等于变量innodb_log_file_size定义的值。第一个文件```ibdata1```是在没有开启innodb_file_per_table时的共享表空间文件，对应于开启innodb_file_per_table是的```.ibd```文件。

在innodb将log buffer中的redo log block刷到这些log file中时，会以追加写入的方式循环轮询写入。即先在第一个log file(即ib_logfile0)的尾部追加写，直到满了之后向第二个log file(即ib_logfile1)写。当第二个log file满了会清空一部分第一个log file继续写入。

由于是将log buffer中的日志刷到log file，所以在log file中记录日志的方式也是log block的方式。

在每个组的第一个redo log file中，前2KB记录了4个特定的部分，从2KB之后才开始记录log block。除了第一个redo log file中会记录，log group中的其他log file不会记录这2KB，但是却会腾出这2KB的空间。如下：

![db-mysql-dolog](https://ivanzz1001.github.io/records/assets/img/db/db_dolog_6.png)

redo log file的大小对innodb的性能影响非常大，设置的太大，恢复的时候就会时间较长；设置的太小，就会导致在写redo log的时候循环切换redo log file。

### 1.5 redo log的格式

因为innodb存储引擎存储数据的单元是页(和SQL Server中的一样），所以redo log也是基于页的格式来记录的。默认情况下，innodb的页的大小是16KB(innodb_page_size变量控制），一个页内可以存放非常多的log block(每个512字节），而log block中记录的又是数据页的变化。

其中log block中492字节的部分是log body，该log body的格式分为4部分：

* redo_log_type: 占用1个字节，表示redo log的日志类型；

* space: 表示表空间的ID，采用压缩的方式后，占用的空间可能小于4字节；

* page_no: 表示页的偏移量，同样是压缩过的；

* redo_log_body: 表示每个重做日志的数据部分，恢复时会调用相应的函数进行解析。例如insert语句和delete语句写入redo log的内容


如下图，分别是insert和delete大致的记录方式：

![db-mysql-dolog](https://ivanzz1001.github.io/records/assets/img/db/db_dolog_7.png)


### 1.6 日志刷盘的规则
log buffer中未刷到磁盘的日志称为脏日志(dirty log)。

在上面说过，默认情况下事务每次提交的时候都会刷事务日志到磁盘中，这是因为变量innodb_flush_log_at_trx_commit的值为1。但是innodb不仅仅只会在有commit动作后才会刷日志到磁盘，这只是innodb存储引擎刷日志的规则之一。

刷日志到磁盘有以下几种规则：

1. 发出commit动作时。已经说明过，commit发出后是否刷日志由变量innodb_flush_log_at_trx_commit控制；

2. 每秒刷一次。这个刷日志的频率由变量innodb_flush_log_at_timeout值决定，默认是1秒。要注意，这个刷日志频率和commit动作无关。

3. 当log buffer中已经使用的内存超过一半时；

4. 当有checkpoint时，checkpoint在一定程度上代表了刷到磁盘时日志所处的LSN位置。

### 1.7 数据页刷盘的规则及checkpoint
内存中(buffer pool)未刷到磁盘的数据称为脏数据(dirty data)。由于数据和日志都以页的形式存在，所以脏页表示脏数据和脏日志。

在上面一节我们介绍了日志是何时刷到磁盘的，不仅仅是日志需要刷盘，脏数据页也一样需要刷盘。

在innodb中，数据刷盘的规则只有一个： checkpoint。但是触发checkpoint的情况却有几种。不管怎样，checkpoint触发后，会将buffer中脏数据页和脏日志页都刷到磁盘。

innodb存储引擎中checkpoint分为两种：

* sharp checkpoint: 在重用redo log文件（例如切换日志文件）的时候，将所有已记录到redo log中对应的脏数据刷到磁盘。

* fuzzy checkpoing: 一次只刷一小部分的日志到磁盘，而非将所有脏日志刷盘。有以下几种情况会触发该检查点：

  - master thread checkpoint: 由master线程控制，每秒或每10秒刷入一定比例的脏页到磁盘；

  - flush_lru_list checkpoing: 从MySQL5.6开始可通过innodb_page_cleaners变量指定专门负责脏页刷盘的page cleaner线程的个数，该线程的目的是为了保证lru列表有可用的空闲页。
  
  - async/sync flush checkpoint: 同步刷盘还是异步刷盘。例如还有非常多的脏页没刷到磁盘（非常多是多少，有比例控制），这时候会选择同步刷到磁盘，但这很少出现；如果脏页不是很多，可以选择异步刷到磁盘；如果脏页很少，可以暂时不刷脏页到磁盘。

  - dirty page too much checkpoint: 脏页太多时强制触发检查点，目的是为了保证缓存有足够的空闲空间。too much的比例由变量innodb_max_dirt_page_pct控制，MySQL5.6默认的值是75，即当脏页占缓冲池的百分之75后，就强制刷一部分脏页到磁盘。

由于刷脏页需要一定的时间来完成，所以记录检查点的位置是在每次刷盘结束之后才在redo log中标记的。

MySQL停止时是否将脏数据和脏日志刷入磁盘，由变量innodb_fast_shutdown={ 0|1|2 }控制，默认值为1，即停止时只做一部分purge，忽略大多数flush操作(但至少会刷日志)，在下次启动的时候再flush剩余的内容，实现fast shutdown。

### 1.8 LSN超详细分析
### 1.9 innodb的恢复行为
在启动innodb的时候，不管上次是正常关闭还是异常关闭，总是会进行恢复操作。

因为redo log记录的是数据页的物理变化，因此恢复的时候速度比逻辑日志（如二进制日志）要快很多。而且，innodb自身也做了一定程度的优化，让恢复速度变得更快。

重启innodb时，checkpoint表示已经完整刷到磁盘上data page上的LSN，因此恢复时仅需要恢复从checkpoint开始的日志部分。例如，当数据库在上一次checkpoint的LSN为10000时宕机，且事务是已经提交过的状态。启动数据库时会检查磁盘中数据页的LSN，如果数据页的LSN小于日志中的LSN，则会从检查点开始恢复。

还有一种情况，在宕机前正处于checkpoint的刷盘过程，且数据页的刷盘进度超过了日志页的刷盘进度。这时候宕机，数据页中记录的LSN就会大于日志页中的LSN，在重启的恢复过程中会检查到这一情况，这时超出日志进度的部分将不会重做，因为这本身就表示已经做过的事情，无需再重做。

另外，事务日志具有幂等性，所以多次操作得到同一结果的行为在日志中只记录一次。而二进制日志不具有幂等性，多次操作会全部记录下来，在恢复的时候会多次执行二进制日志中的记录，速度就慢得多。例如，某记录中id初始值为2，通过update将值设置为了3，后来又设置成了2，在事务日志中记录的将是无变化的页，根本无需恢复；而二进制会记录下两次update操作，恢复时也将执行这两次update操作，速度比事务日志恢复更慢。

### 1.10 和redo log有关的几个变量

* innodb_flush_log_at_trx_commit={0|1|2}： 指定何时将事务日志刷到磁盘，默认为1。

  - 0： 表示每秒将“log buffer”同步到“os buffer”且从“os buffer”刷到磁盘日志文件中；
  
  - 1: 表示每事务提交都将“log buffer”同步到“os buffer”且从“os buffer”刷到磁盘日志文件中;
  
  - 2： 表示每事务提交都将“log buffer”同步到“os buffer”但每秒才从“os buffer”刷到磁盘日志文件中。


* innodb_log_buffer_size： log buffer的大小，默认8M

* innodb_log_file_size： 事务日志的大小，默认5M

* innodb_log_files_group =2： 事务日志组中的事务日志文件个数，默认2个

* innodb_log_group_home_dir =./： 事务日志组路径，当前目录表示数据目录

* innodb_mirrored_log_groups =1： 指定事务日志组的镜像组个数，但镜像功能好像是强制关闭的，所以只有一个log group。在MySQL5.7中该变量已经移除。


## 2. undo log

### 2.1 基本概念

undo log有两个作用：提供回滚和多版本并发控制（MVCC: Multi-Version Concurrency Control)。

在数据修改的时候，不仅记录了redo，还记录了相对应的undo，如果因为某些原因导致事务失败或回滚了，可以借助undo来进行回滚。

undo log和redo log记录物理日志不一样，它是逻辑日志。可以认为当delete一条记录时，undo log中会记录一条对应的insert记录，反之亦然，当update一条记录时，它记录一条对应相反的update记录。

当执行rollback时，就可以从undo log中的逻辑记录读取到相应的内容并进行回滚。有时候应用到MVCC的时候，也是通过undo log来实现的： 当读取的某一行被其他事务锁定时，它可以从undo log中分析出该行记录以前的数据是什么，从而提供该行版本信息，让用户实现非锁定一致性读取。

undo log是采用段(segment)的方式来记录的，每个undo操作在记录的时候占用一个undo log segment。

另外，undo log也会产生redo log，因为undo log也要实现持久性保护。

undo日志用于存放数据修改被修改前的值，假设修改```tba表```中id=2的行数据，把Name='B'修改为Name='B2'，那么undo日志就会用来存放Name='B'的记录，如果这个修改出现异常，可以使用undo日志来实现回滚操作，保证事务的一致性。对数据的变更操作，主要来自INSERT、UPDATE、DELETE，而undo log分为两种类型，一种是INSERT_UNDO(insert操作），记录插入的唯一键值；一种是UPDATE_UNDO(包含UPDATE及DELETE操作），记录修改的唯一键值以及old column记录。

![db-mysql-dolog](https://ivanzz1001.github.io/records/assets/img/db/db_dolog_8.jpg)

### 2.2 undo log的存储方式

innodb存储引擎对undo的管理采用段的方式。rollback segment称为回滚段，每个回滚段中有1024个undo log segment。

在以前老版本，只支持1个rollback segment，这样就只能记录1024个undo log segment。后来MySQL5.5可以支持128个rollback segment，即支持```128 * 1024```个undo log segment，还可以通过变量innodb_undo_logs(5.6版本以前该变量是innodb_rollback_segments)自定义多少个rollback segment，默认值为128。

undo log默认存放在共享表空间中：
<pre>
# ls /var/lib/mysql/ibd*
/var/lib/mysql/ibdata1
</pre>

如果开启了innodb_file_per_table，将放在每个表的```.ibd```文件中。

在MySQL5.6中，undo的存放位置还可以通过变量innodb_undo_directory来自定义存放目录，默认值为```.```，表示datadir。

默认rollback segment全部写在一个文件中，但可以通过设置变量innodb_undo_tablespaces平均分配到多个文件中。该变量默认值为0，即全部写入一个表空间文件。该变量为静态变量，只能在数据库实例停止状态下修改，如写入配置文件或者启动时带对应的参数。但是innodb存储引擎在启动过程中提示，不建议修改为非0的值，如下：
{% highlight string %}
2017-03-31 13:16:00 7f665bfab720 InnoDB: Expected to open 3 undo tablespaces but was able
2017-03-31 13:16:00 7f665bfab720 InnoDB: to find only 0 undo tablespaces.
2017-03-31 13:16:00 7f665bfab720 InnoDB: Set the innodb_undo_tablespaces parameter to the
2017-03-31 13:16:00 7f665bfab720 InnoDB: correct value and retry. Suggested value is 0
{% endhighlight %}

### 2.3 和undo log相关的变量
undo相关的变量在MySQL5.6中已经变得很少。如下：它们的意义在上文中已经解释了
{% highlight string %}
mysql> show global variables like '%undo%';
+--------------------------+------------+
| Variable_name            | Value      |
+--------------------------+------------+
| innodb_max_undo_log_size | 1073741824 |
| innodb_undo_directory    | ./         |
| innodb_undo_log_truncate | OFF        |
| innodb_undo_logs         | 128        |
| innodb_undo_tablespaces  | 3          |
+--------------------------+------------+

mysql> show global variables like '%truncate%';
+--------------------------------------+-------+
| Variable_name                        | Value |
+--------------------------------------+-------+
| innodb_purge_rseg_truncate_frequency | 128   |
| innodb_undo_log_truncate             | OFF   |
+--------------------------------------+-------+
{% endhighlight %}

### 2.4 delete/update操作的内部机制
当事务提交的时候，innodb不会立即删除undo log，因为后续还可能会用到undo log，如隔离级别为repeatable read时，事务读取的都是开启事务时的最新提交行版本，只要该事务不结束，该行版本就不能删除，即undo log不能删除。

但是在事务提交的时候，会将该事务对应的undo log放到删除列表中，未来通过purge来删除。并且提交事务时，还会判断undo log分配的页是否可以重用，如果可以重用，则会分配给后面来的事务，避免为每个独立的事务分配独立的undo log页而浪费存储空间和性能。

通过undo log记录delete和update操作的结果发现（insert操作无需分析，就是插入行而已）：

* delete操作实际上不会直接删除，而是将delete对象打上delete tag，标记为删除，最终的删除操作是purge线程完成的。

* update分为两种情况：update的列是否为主键列

  - 如果不是主键列，在undo log中直接反向记录是如何update的。即update是直接进行的
  
  - 如果是主键列，update分两步执行： 先删除该行，再插入一行目标行

## 3. undo及redo如何记录事务
###### 3.1 undo+redo事务的简化过程
假设有A、B两个数据，值分别为1、2。开始一个事务，事务的操作内容为： 把1修改为3，2修改为4。那么实际的记录如下（简化）：
<pre>
A. 事务开始

B. 记录A=1到undo log

C. 修改A=3

D. 记录A=3到redo log

E. 记录B=2到undo log

F. 修改B=4

G. 记录B=4到redo log

H. 将redo log写入磁盘

I. 事务提交
</pre>

###### 3.2 IO影响
undo+redo的设计主要考虑的是提升IO性能，增大数据库的吞吐量。可以看出，B、D、E、G、H均是新增操作，但是B、D、E、G是缓冲到buffer区，只有G是增加了IO操作，为了保证redo log能够有比较好的IO性能，InnoDB的redo log的设计有以下几个特点：

1) 尽量保持redo log存储在一段连续的空间上。因此在系统第一次启动时，就会将日志文件的空间完全分配。以顺序追加的方式记录redo log，通过顺序IO来改善性能；

2） 批量写入日志。日志并不是直接写入文件，而是先写入redo log buffer。当需要将日志刷新到磁盘时（如事务提交），将许多日志一起写入磁盘。

3） 并发的事务共享redo log的存储空间，它们的redo log按语句的执行顺序，依次交替的记录在一起，以减少日志占用的空间。例如，redo log中的记录内容可能是这样的：
{% highlight string %}
记录1: <trx1, insert …>

记录2: <trx2, update …>

记录3: <trx1, delete …>

记录4: <trx3, update …>

记录5: <trx2, insert …>
{% endhighlight %}

4) 因为3)的原因，当一个事务将redo log写入磁盘时，也会将其他未提交的事务的日志写入磁盘；

5） redo log上只进行顺序追加的操作，当一个事务需要回滚时，它的redo log记录也不会从redo log中删除掉；

###### 3.3 恢复
前面说到```未提交```的事务和```回滚```了的事务也会记录redo log，因此在进行恢复时，这些事务要进行特殊的处理。有两种不同的恢复策略：

A: 进行恢复时，只重做已经提交了的事务；

B: 进行恢复时，重做所有事务包括未提交的事务和回滚了的事务，然后通过undo log回滚那些未提交的事务；

MySQL数据库InnoDB存储引擎使用了B策略，InnoDB存储引擎中的恢复机制有如下几个特点：

a) 在重做redo log时，并不关心事务性。恢复时，没有BEGIN，也没有COMMIT、ROLLBACK的行为。也不关心每个日志是哪个事务的。尽管事务ID等事务相关的内容会记入redo log，这些内容只是被当作要操作的数据的一部分。

b) 使用B策略就必须要将undo log持久化，而且必须要在写redo log之前将对应的undo log写入磁盘。undo和redo log这种关联，使得持久化变得复杂起来。为了降低复杂度，innodb将undo log看做数据，因此记录undo log的操作也会记录到redo log中。这样undo log就可以像数据一样缓存起来，而不用在redo log之前写入磁盘了。包含undo log操作的redo log，看起来是这样的：
{% highlight string %}
记录1: <trx1, Undo log insert <undo_insert …>>

记录2: <trx1, insert …>

记录3: <trx2, Undo log insert <undo_update …>>

记录4: <trx2, update …>

记录5: <trx3, Undo log insert <undo_delete …>>

记录6: <trx3, delete …>
{% endhighlight %}

c) 到这里，还有一个问题没有弄清楚。既然redo没有事务性，那岂不是会重新执行被回滚了的事务？确实是这样，同时innodb也会将事务回滚时的操作也记录到redo log中。回滚操作本质上也是对数据进行修改，因此回滚时对数据的操作也会记录到redo log中。一个回滚了的事务的redo log，看起来是这样的：
{% highlight string %}
记录1: <trx1, Undo log insert <undo_insert …>>

记录2: <trx1, insert A…>

记录3: <trx1, Undo log insert <undo_update …>>

记录4: <trx1, update B…>

记录5: <trx1, Undo log insert <undo_delete …>>

记录6: <trx1, delete C…>

记录7: <trx1, insert C>

记录8: <trx1, update B to old value>

记录9: <trx1, delete A>
{% endhighlight %}
一个被回滚了的事务在恢复时的操作就是先redo再undo，因此不会破坏数据的一致性。

## 4. mysql innodb安装目录下文件介绍
以下这些文件都是在指定的数据库数据目录下：

1) **redo log重做日志**
<pre>
ib_logfile0  ib_logfile1
</pre>

2) **undo log回滚日志**
<pre>
ibdata1  ibdata2(存储在共享表空间中)
</pre>

3) **临时表**

ibtmp1文件用于存放数据库操作期间产生的临时数据，用完即删除。比如连表查询时产生的一些中间表的存储等。

![db-mysql-dolog](https://ivanzz1001.github.io/records/assets/img/db/db_dolog_9.png)




<br />
<br />
**[参看]**:

1. [mysql的undo log和redo log](https://www.cnblogs.com/wyy123/p/7880077.html)

2. [MySQL日志系统：redo log、binlog、undo log 区别与作用](https://blog.csdn.net/u010002184/article/details/88526708)

3. [详细分析MySQL事务日志(redo log和undo log)](https://www.cnblogs.com/f-ck-need-u/archive/2018/05/08/9010872.html)

4. [mysql innodb安装目录下文件介绍： 日志记录redu/undo log及临时表ibtmp1](https://www.cnblogs.com/quzq/p/12833381.html)

<br />
<br />
<br />

