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

> 在此处需要注意一点， 一般





<br />
<br />
**[参看]**:

1. [mysql的undo log和redo log](https://www.cnblogs.com/wyy123/p/7880077.html)

2. [MySQL日志系统：redo log、binlog、undo log 区别与作用](https://blog.csdn.net/u010002184/article/details/88526708)

3. [详细分析MySQL事务日志(redo log和undo log)](https://www.cnblogs.com/f-ck-need-u/archive/2018/05/08/9010872.html)


<br />
<br />
<br />

