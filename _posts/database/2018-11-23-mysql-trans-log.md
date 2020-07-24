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

### 1.3 日志快(log block)



<br />
<br />
**[参看]**:

1. [mysql的undo log和redo log](https://www.cnblogs.com/wyy123/p/7880077.html)

2. [MySQL日志系统：redo log、binlog、undo log 区别与作用](https://blog.csdn.net/u010002184/article/details/88526708)

3. [详细分析MySQL事务日志(redo log和undo log)](https://www.cnblogs.com/f-ck-need-u/archive/2018/05/08/9010872.html)


<br />
<br />
<br />

