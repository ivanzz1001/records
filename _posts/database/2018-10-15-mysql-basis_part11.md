---
layout: post
title: MySQL数据库备份与恢复
tags:
- database
categories: database
description: MySQL数据库备份与恢复
---


在MySQL运行过程中做好数据库备份是很重要的，因为在假如遇到系统崩溃、硬件损坏、用户误删数据的情况下，你就可以通过数据库备份来进行数据恢复。此外，在需要对数据库进行版本更新重新安装MySQL时，做好数据备份也是很有必要的； 在建立MySQL主从复制时也是需要进行数据库备份。

MySQL提供了很多的备份策略，你可以根据自身需要选择最合适的方法来进行备份。本章主要包括如下方面的内容：

* 备份类型： ```逻辑备份```与```物理备份```、```全量备份```与```增量备份```等

* 创建备份的方法

* 数据库恢复方法，包括恢复到某一个时间点

* 备份计划、压缩和加密

* 表的维护， 使能恢复损坏的表 


<!-- more -->

## 1. 备份与恢复类型

### 1.1 物理备份与逻辑备份

物理备份是指```拷贝存储数据库数据的目录与文件```。这种类型的备份适合于大型的、重要的数据库，并在遇到问题时需要能够快速的进行恢复。

逻辑备份只保存代表逻辑数据库结构（```CREATE DATABASE```, ```CREATE TABLE```语句）与内容（```INSERT```语句或```delimited-text```文件）的信息。这种类型的备份适合于数据量较小的情况，或者需要在不同架构的机器上重建数据这一情况。


**(1) 物理备份**

物理备份方法有如下一些特征：

* 备份需要拷贝```数据库目录```和```文件```。典型情况是需要拷贝整个或部分的MySQL数据目录

* 物理拷贝方法通常比逻辑拷贝更加快速，因为逻辑拷贝只涉及到文件拷贝，并不涉及相应的数据转换

* 输出也比逻辑备份更紧凑

* 因为对于繁忙的、重要的数据库来说，备份速度与紧凑性(compactness)是很重要的，因此一般MySQL企业版备份产品都是采用物理备份。

* 备份和恢复粒度的范围从```数据目录级别```到```文件级别```。取决于存储引擎，物理备份有可能不能提供```表级别```(table-level)粒度。例如，对于InnoDB表来说，可能是一个单独的文件，也可能与其他InnoDB表共享同一个存储文件；对于MyISAM表来说则对应于一系列的文件。

* 另外，对于数据库来说，备份可以包含任何相关联的文件，例如日志和配置文件

* 对于```MEMORY```表中的数据，并不能通过物理备份的方法来进行备份，这是因为```MEMORY```表数据根本不会存放在硬盘上。（MySQL企业版备份产品有相应的特性，可以将```MEMORY```表中的数据也进行备份）

* 物理备份只能够在那些具有相同或相似硬件特性的机器上移植

* 在MySQL服务器也可以进行物理备份。假如服务器正在运行，则需要进行适当的锁操作，使得在备份期间MySQL Server并不会改变数据库的内容。MySQL企业版备份产品能够在备份时对相应的表进行自动加锁。

* 物理备份的工具包括： MySQL Enterprise Backup提供的针对InnoDB表及其他类型表的```mysqlbackup```工具、文件系统级别的命令（比如cp、scp、tar、rsync),或者针对MyISAM表的```mysqlhotcopy```工具。


对于物理备份的```恢复```来说，主要有如下特征：

* MySQL Enterprise Backup直接提供了相应备份的恢复功能

* ndb_restore工具用于恢复NDB表

* 文件系统级别拷贝的文件，或者通过```mysqlhotcopy```工具拷贝的文件，都可以通过文件系统相应的命令拷贝回原来的地方。

<br />

**(2) 逻辑备份**

逻辑备份方法主要有如下特性：

* 是通过查询MySQL Server来获得数据库结构和内容相关的信息

* 逻辑备份比物理备份慢，这是因为必须要通过访问数据库来获得相应的信息，然后再将这些信息转换成相应的逻辑格式。假设输出是需要写到client一侧，那么MySQL Server还需要将这些信息返回给备份程序；

* 输出比物理备份更大，特别是当被保存为文本文件的时候

* 备份和恢复的粒度可以是server级别（all databases)、数据库级别（all tables in a particular database)、或者表级别。与存储引擎无关

* 备份并不包含日志和配置文件，也不包含其他一些与数据库相关的文件

* 备份是以逻辑结构来存储的，因此不依赖于特定的机器，具有高可移植性

* 逻辑备份需要在MySQL Server运行的情况下进行。MySQL Server不能处于offline状态

* 逻辑备份的工具包括```mysqldump```程序以及```SELECT ... INTO OUTFILE```语句。这对MySQL的任何存储引擎来说都适用，甚至是```MEMORY```表

* 对于恢复逻辑备份来说，SQL格式的dump文件可以直接由```mysql``` client进行处理。如果要加载```delimited-text```文件，使用```LOAD DATA INFILE```语句或者```mysqlimport```客户端即可。


### 1.2 Online备份与Offline备份

Online备份是在MySQL Server运行时进行的，因此可以从MySQL Server获得相应的数据库信息。Offline备份是在MySQL Server停止的情况下进行的备份。Online备份与Offline备份的区别也可以被描述为```热备份```（hot backup)与```冷备份```(cold backup); 另外还有一种所谓的```温备份```(warm backup)，就是指虽然保持MySQL Server处于运行状态，但是在访问外部数据库文件的时候首先会加上锁以防止数据被修改。

```Online```备份有如下一些特征：

* ```Online```备份对其他访问MySQL数据库的客户端具有较小的侵入性。在备份期间，其他客户端也仍然可以连接上MySQL Server，并且取决于所执行的操作，客户端一般还是可以访问数据库数据（通常情况下，仍然可以读）

* ```Online```备份时必须增加适当的锁，以防止备份期间数据被修改，从而破坏备份数据的完整性。```MySQL Enterprise Backup```产品通常都有自动的加锁功能。


```Offline```备份有如下一些特征：

* ```Offline```备份会影响到其他客户端的连接，因为在备份期间MySQL Server是处于不可用状态。正是因为这样的原因，这种备份通常是在slave上进行的，因为在slave处于offline状态下一般不会影响系统的可用性。

* 备份流程较为简单，因为并没有客户端访问数据库。

<br />
同样```Online```与```Offline```恢复也类似。然而，通常情况下```Online恢复```会比```Online备份```对客户端产生更大的影响，这是因为恢复一般需要更强的锁。在备份期间，客户端也许还可以进行读操作；而在恢复期间，一般还需要禁止客户端读取MySQL Server的数据。

### 1.3 本地备份与远程备份
本地备份通常在运行MySQL Server的主机上进行，而远程备份通常是在不同的主机。对于有一些类型的备份，可以在远程主机上进行开启，即使	可能相应的备份数据仍需要保存在MySQL Server主机本地。

* mysqldump可以连接到本地MySQL Server，也可以连接到远程MySQL Server。对于SQL输出(```CREATE```与```INSERT```语句），可以dump到远程也可以dump到本地； 而对于delimited-text输出（一般需要指定```--tab```选项），则只能保存在MySQL Server一侧。

* mysqlhotcopy只能进行本地备份： 它会连接上MySQL Server并进行加锁，以防止数据被修改，然后再拷贝本地的表文件。

* SELECT ... INFO OUTFILE可以在本地也可以在远程客户端上启动，但是输出文件是创建在MySQL Server一侧。

* 物理备份方法通常是在MySQL Server本地来进行的，因为一般需要将MySQL Server设置为Offline状态。

### 1.4 snapshot备份
有一些文件系统实现具有```snapshot```功能。这提供了在某一个时间点逻辑拷贝文件系统的方法，而不需要物理的拷贝整个文件系统。MySQL Server本身并不提供创建文件系统快照的方法。但是可以通过一些第三方的方法如```Veritas```、```LVM```或者```ZFS```来实现。

### 1.5 全量备份与增量备份

全量备份是指备份某个时间点的MySQL Server所管理的全量数据。而增量备份只备份在某一个时间段内发生改变的数据。MySQL有很多种不同的方式来进行全量备份。而对于增量备份来说，通常是通过启用MySQL Server的binlog来完成的，因为binlog会记录相应的数据改变。

### 1.6 全量恢复与基于某一个时间点的增量恢复
```全量恢复```会使用```全量备份```的数据来进行，恢复完成之后就会回到备份创建时的状态。假如全量恢复之后仍然与当前的状态有偏差，则可以通过增量备份来继续进行恢复，这样就可以使得MySQL Server处于一个更```up-to-date```的状态。

```增量恢复```只是恢复某一个时间段内改变的数据，其也被称作```point-in-time```恢复。```point-in-time```恢复通常是在全量恢复之后，使用binlog来做一个更新时间段的恢复操作。

### 1.7 表维护
在表受到损坏的情况下，数据的完整性有可能得不到保证。对于InnoDB表来说，通常不会遇到这样的问题； 而对于```MyISAM```表来说，当问题发生时就需要采用相关的方法来进行修复。

### 1.8 备份计划、压缩与加密
对于自动备份程序来说，制定一个备份计划是很值得的。而在备份完成之后，压缩备份输出可以降低硬盘空间的使用。对于备份数据的加密则可以提供更好的安全性，以防止未授权的用户访问备份的数据。MySQL本身并不提供这样的功能，但```MySQL Enterprise Backup```工具提供了压缩```InnoDB```备份的功能，然后可以通过文件系统提供的工具对备份输出再次进行压缩与加密。

## 2. 数据库备份方法
本节会介绍一下用于制作备份的一些常用方法。这里对于使用```MySQL Enterprise Backup```来进行热备份，我们不做介绍。

### 2.1 使用mysqldump或者mysqlhotcopy来备份
```mysqldump```或者```mysqlhotcopy```脚本都可以用于制作备份。通常```mysqldump```更通用一些，因为其可以用于备份所有类型的表。而```mysqlhotcopy```只能备份特定存储引擎的表。

对于```InnoDB```类型的表，可以在不加锁的情况下进行online备份，只需要在执行```mysqldump```时加上```--single-transaction```选项。

我们在后面会介绍```mysqldump```的使用方法。

### 2.2 通过拷贝表文件来备份
对于那些表数据都用该表对应的私有文件来进行存储的存储引擎来说，可以通过拷贝这些文件来进行备份。例如，对于```MyISAM```类型的表来说，我们可以通过拷贝文件(```*.frm```、```*.MYD```或者```*.MYI```文件）来进行备份。要获得一个一致性的备份，需要停止MySQL Server服务器，或者对拷贝的表进行加锁并flush该表：
{% highlight string %}
FLUSH TABLES tbl_list WITH READ LOCK;
{% endhighlight %}

这里你只需要增加一个读锁；这使得在你备份数据库文件的时候其他客户端仍能够查询该表。这里```FLUSH```用于确保所有处于活跃状态的索引页都会在你开始备份之前写入到硬盘。

你也可以在MySQL Server在不做任何更新的时候，通过拷贝所有的表文件来创建一个二进制拷贝。```mysqlhotcopy```脚本就是采用这种方法。（注意： 表文件拷贝方法并不适应于包含```InnoDB```表的数据库。```mysqlhotcopy```并不能用于```InnoDB```表的拷贝，因为```InnoDB```并不需要将表的数据存放到数据库目录。同样，即使是MySQL Server当前并未更新数据，```InnoDB```也还会有一些已经修改的数据存放与内存，而并没有flush到硬盘上）。
<pre>
注意： 在冷备份时，假如数据表使用的是InnoDB存储引擎，则还需要备份ib_logfile0、ib_logfile1。此文件用于存放InnoDB引擎的事务
日志信息。
</pre>

### 2.3 制作Delimited-Text文件备份

你可以使用如下：
<pre>
SELECT * INTO OUTFILE 'file_name' FROM tbl_name;
</pre>
来创建一个包含表数据的文本文件。该文件被创建在MySQL Server宿主机一侧，并不在client主机上。在执行该语句时，要求输出文件并不存在，因为如果存在的话执行覆盖操作存在一定的风险。该方法适应于任何类型的表，只是复制表数据，并不会复制表结构。

另一种创建文本数据文件（也包括创建表```CREATE TABLE```的语句）的方法就是通过使用```mysqldump```工具，并指定```--tab```选项。例如：
{% highlight string %}
# mysqldump  -uroot -ptestAa@123  --tab="/root/" --fields-terminated-by=”#” test
mysqldump: [Warning] Using a password on the command line interface can be insecure.
mysqldump: Got error: 1290: The MySQL server is running with the --secure-file-priv option so it cannot execute this statement when executing 'SELECT INTO OUTFILE'
{% endhighlight %}
可以看到上面由于MySQL启动时指定了```--secure-file-priv```选项，该选项用于指定一个目录，对于```LOAD DATA```, ```SELECT ... OUTFILE```, 以及```LOAD_FILE()```只能导入或导出到该目录：

* 当```secure-file-priv```的值为NULL时，表示限制mysqld不允许导入导出；

* 当```secure-file-priv```的值为/tmp/，表示限制mysqld导入导出只能发生在/tmp/目录下；

* 当```secure-file-priv```没有具体值时，表示不对mysqld的导入导出做限制

这里我们首先查看```secure-file-priv```的设置：
{% highlight string %}
mysql> show global variables like '%secure%';
+--------------------------+-----------------------+
| Variable_name            | Value                 |
+--------------------------+-----------------------+
| require_secure_transport | OFF                   |
| secure_auth              | ON                    |
| secure_file_priv         | /var/lib/mysql-files/ |
+--------------------------+-----------------------+
3 rows in set (0.00 sec)
{% endhighlight %}
因此我们将文件导出到```/var/lib/mysql-files/```目录下：
{% highlight string %}
# mysqldump  -uroot -ptestAa@123  --tab="/var/lib/mysql-files/" --fields-terminated-by=”#” test
mysqldump: [Warning] Using a password on the command line interface can be insecure.
# ls /var/lib/mysql-files/
course.sql  course.txt  runoob_tbl.sql  runoob_tbl.txt  student.sql  student.txt

# cat /var/lib/mysql-files/course.sql
-- MySQL dump 10.13  Distrib 5.7.22, for Linux (x86_64)
--
-- Host: localhost    Database: test
-- ------------------------------------------------------
-- Server version       5.7.22-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `course`
--

DROP TABLE IF EXISTS `course`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `course` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `coursename` varchar(128) NOT NULL,
  `stuid` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `foreign_stuid` (`stuid`),
  CONSTRAINT `course_ibfk_1` FOREIGN KEY (`stuid`) REFERENCES `student` (`stuid`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-10-19 18:53:03


# cat /var/lib/mysql-files/course.txt 
1”#”MySQL从入门到精通”#”1001
2”#”爱情与婚姻”#”1002
3”#”Java从入门到放弃”#”1003
4”#”商务礼仪”#”1004
5”#”表演的艺术”#”1005
6”#”民法”#”1006
7”#”民法”#”1001
{% endhighlight %}
因为上面指定的字段分隔符```--fields-terminated-by```为```#```，因此这里我们看到是以```#```来做分割。

要加载```delimited-text```文件的话，可以使用```LOAD DATA INFILE```或者```mysqlimport```。

### 2.4 通过binlog来制作增量备份文件
MySQL支持增量备份： 你必须在系统启动时指定```--log-bin```选项以启用binlog功能。binlog可以提供从你创建备份时刻起MySQL数据库所做的更改信息。在你需要进行增量备份的时候，你应该使用```FLUSH LOGS```以开启一个新的binlog日志。在这一步完成之后，你需要拷贝从上一次全量备份时到当前为止的所有binlog(除最后一个FLUSH LOGS产生的binlog外)到备份目录中。这些binlog就是增量备份；关于增量备份的恢复，我们后面会继续进行讲解。当下一次你再做全量备份时，你还是需要使用```FLUSH LOGS```来重新生成新的binlog。


### 2.5 使用Replication Slaves来备份
假如在通过master进行备份时会产生性能问题，其中一种策略就是建立复制从机(replication slave)，并使用复制从机来进行备份。当你使用复制从机来进行备份时，你需要备份slave数据库的```master info```目录以及```relay log info```目录。在你需要恢复slave数据的时候，你需要所有这些文件信息。假如slave正在复制```LOAD DATA INFILE```语句时，则你还需备份存在于目录中的任何```SQL_LOAD-*```文件。slave需要通过这些文件来恢复任何中断的```LOAD DATA INFILE```操作。这些文件存放的目录是通过```--slave-load-tmpdir```选项来指定的。假如MySQL Server在启动时并未指定该选项，在采用MySQL系统变量的```tmpdir```。

### 2.6 恢复损坏的表

假如你必须恢复已经损坏的```MyISAM```表，首先尝试使用```REPAIR TABLE```或者```myisamchk -r```。通常在99.9%的情况下都能修复好。

### 2.7 使用文件系统snapshot来创建备份
假如你使用```Veritas```文件系统的话，你可以通过如下方式来创建备份：

1） 从客户端执行```FLUSH TABLES WITH READ LOCK;```

2) 从另一个shell执行```mount vxfs snapshot```

3) 从第一个客户端执行```UNLOCK TABLES;```

4) 从snapshot拷贝文件

5) umount该snapshot

对于其他的文件系统，例如```LVM```或者```ZFS```也可以采用类似的方法。


## 3. 复制和备份策略示例
本章主要会介绍一下制作备份的步骤，并在遇到如下情况时如何通过备份来对数据进行恢复：

* 操作系统崩溃

* 电源断电

* 文件系统损坏

* 硬件故障（包括硬件驱动、主板损坏等等）

这里假设数据采用的存储引擎是```InnoDB```，该存储引擎支持```事务```与自动崩溃恢复。这里同时假设在崩溃的时候，MySQL正处于```工作负`载```的情况下，否则并不需要对MySQL进行恢复。

对于操作系统崩溃或电源断电这种情况，我们可以假设MySQL的硬盘数据在系统重启之后是可用的。这时候```InnoDB```数据文件由于系统崩溃或断电处于不一致的状态，但是InnoDB会通过读取日志文件，并且在日志文件中找到处于```pending commited```和```noncommited```状态还没来得及flush到数据文件的事务。之后InnoDB就会自动的回滚这些尚未提交的事务，并且flush那些已经提交的事务到数据文件中。关于恢复进程执行情况，我们可以通过MySQL的错误日志来进行查看。如下就是一个示例日志的摘要：
{% highlight string %}
InnoDB: Database was not shut down normally.
InnoDB: Starting recovery from log files...
InnoDB: Starting log scan based on checkpoint at
InnoDB: log sequence number 0 13674004
InnoDB: Doing recovery: scanned up to log sequence number 0 13739520
InnoDB: Doing recovery: scanned up to log sequence number 0 13805056
InnoDB: Doing recovery: scanned up to log sequence number 0 13870592
InnoDB: Doing recovery: scanned up to log sequence number 0 13936128
...
InnoDB: Doing recovery: scanned up to log sequence number 0 20555264
InnoDB: Doing recovery: scanned up to log sequence number 0 20620800
InnoDB: Doing recovery: scanned up to log sequence number 0 20664692
InnoDB: 1 uncommitted transaction(s) which must be rolled back
InnoDB: Starting rollback of uncommitted transactions
InnoDB: Rolling back trx no 16745
InnoDB: Rolling back of trx no 16745 completed
InnoDB: Rollback of uncommitted transactions completed
InnoDB: Starting an apply batch of log records to the database...
InnoDB: Apply batch completed
InnoDB: Started
mysqld: ready for connections
{% endhighlight %}
对于```文件系统崩溃```或者硬件问题，我们假设在系统重启之后MySQL硬盘数据处于不可用状态。这就意味着并不能成功的进行重启操作，因为当前硬盘数据的其中一些块并不能进行读取了。在这种情况下，可能需要重新格式化硬盘、重新安装操作系统、或者还需要修复其他的底层问题。然后使用备份来恢复MySQL数据，这就意味着我们前期已经制作了备份。要处理这种情况，就意味着我们要设计并实现一个备份策略。

### 3.1 建立备份策略
建立MySQL数据库备份计划是很有用的。通常我们可以采用多种方法来建立一个全量备份(在某一个时间点的数据快照）。例如，```MySQL Enterprise Backup```就可以对整个MySQL实例进行物理备份，并可以在备份```InnoDB```数据文件时对MySQL服务造成中断或过重的负载。可以使用mysqldump来进行逻辑备份。在这里我们使用的是```mysqldump```>

如下假设我们对MySQL所有数据库的所有InnoDB表做一个全量的备份，备份日期为星期天的下午1点：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --all-databases --master-data --single-transaction > backup_sun_1_PM.sql
{% endhighlight %}
上面产生的```.sql```文件包含了一系列的SQL ```INSERT```语句，后续我们可以使用这些语句来对dump出来的表进行恢复。

该备份操作在开始备份之前需要在所有的表上增加一把全局的读锁（可以使用```FLUSH TABLES WITH READ LOCK```来创建读锁）。一旦获取到了读锁，就会读取到binlog的位置，然后执行完上面的```mysqldump```语句后锁就会被释放。假如在前面```FLUSH```语句执行时，有一个长更新语句正在执行，那么备份操作就可以能会停止直到这些更新语句执行完成。在mysqldump完成之后，MySQL Server就又可以进行正常的读和写操作了。

我们前面执行备份时假设了备份的MySQL表存储引擎是```InnoDB```，因此使用```--single-transaction```来进行一致性读(consistent read)，并保证mysqldump所看见的数据都不会被更改（对于其他客户端对InnoDB表的修改,对mysqldump处理进程来说都是不可见的）。假如备份操作也包含其他```非事务性```表，则这个一致性需要保证在备份期间数据不会发生修改。例如对于mysql数据库中的```MyISAM```表来说，必须在备份期间保证不对MySQL账户进行修改。

全量备份是很有必要的，但是可能通常情况下创建全量备份并不是很方便，因为全量备份会产生大量的备份文件，并耗时很长一段时间。因此可能在有一些情况下全量备份并不是一个最优选择。我们可以先创建一个初始的全量备份，后续再创建增量备份。增量备份会比全量备份更小且耗时更短。之后在进行恢复时，就必须通过初始全量备份与后续的增量备份来共同完成。

要制作增量备份的话，我们需要保存数据增量的修改。在MySQL中，这些修改都会被保存在binlog中，因此MySQL中应该在启动的时候通过```--log-bin```选项启用binlog功能。在binlog启动之后，当数据更新时，MySQL就会将相应的更改写入到对应的日志文件中。通常我们可以在MySQL的数据目录找到```--log-bin```选项所指定的日志文件：
<pre>
-rw-rw---- 1 guilhem guilhem 1277324 Nov 10 23:59 gbichot2-bin.000001
-rw-rw---- 1 guilhem guilhem 4 Nov 10 23:59 gbichot2-bin.000002
-rw-rw---- 1 guilhem guilhem 79 Nov 11 11:06 gbichot2-bin.000003
-rw-rw---- 1 guilhem guilhem 508 Nov 11 11:08 gbichot2-bin.000004
-rw-rw---- 1 guilhem guilhem 220047446 Nov 12 16:47 gbichot2-bin.000005
-rw-rw---- 1 guilhem guilhem 998412 Nov 14 10:08 gbichot2-bin.000006
-rw-rw---- 1 guilhem guilhem 361 Nov 14 10:07 gbichot2-bin.index
</pre>
在每一次MySQL重启启动的时候，都会使用接下来的一个序号来创建一个新的binlog文件。当MySQL Server正在运行时，也可以通过手动执行```FLUSH LOGS```或者```mysqladmin flush-logs```命令来关闭当前的binlog文件并重新开启一个新的binlog。同样```mysqldump```也有相应的选项来flush日志文件。数据目录中的```.index```文件包含了所有对应目录中的binlog文件列表。

在进行MySQL恢复时，binlog文件是很重要的，因为这些binlog保存的是MySQL数据库的增量备份数据。假如你在执行全量备份的时候，确保了fush日志的话，则之后的binlog文件包含了从该时刻起后续所有MySQL数据的更改。我们可以通过修改前面的```mysqldump```命令使得在进行全量备份的时候```FLUSH```日志，并且在导出的文件中包含当前新的binlog文件名：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --single-transaction --flush-logs --master-data=2 \
--all-databases > backup_sunday_1_PM.sql
{% endhighlight %}

在执行完上述命令之后，在MySQL数据目录会包含一个新的binlog文件```gbichot2-bin.000007```,这是因为```--flush-logs```选项会导致服务器刷新日志。而```--master-data```选项会导致mysqldump将binlog信息写到输出中，因此我们可以在导出的```.sql```文件中找到类似于如下：
{% highlight string %}
-- Position to start replication or point-in-time recovery from
-- CHANGE MASTER TO MASTER_LOG_FILE='gbichot2-bin.000007',MASTER_LOG_POS=4;
{% endhighlight %}

因为mysqldump命令是用于制作一个全量备份，因此上面两行代表着：

* 该dump文件包含了所有在```gbichot2-bin.000007```之前发生过改变的数据；

* 在备份之后所做的改变都只存在于```gbichot-bin.000007```及之后的binlog文件中

然后在第二天的13:00(Monday 1 p.m.)，我们可以通过刷新日志来再做一个增量备份。例如，执行```mysqladmin flush-logs```命令以创建一个新的日志文件```gbichot2-bin.000008```,则所有在Sunday 1 p.m.到Monday 1 p.m.之间发生改变的数据都会被记录到```gbichot2-bin.000007```这个日志文件中。该增量备份时很重要的，我们可以将其拷贝到一个安全的地方（例如将其拷贝到磁带、DVD、或者另外一台主机上）。在Tuesday 1 p.m.，可以再执行```mysqladmin flush-logs```命令，则在Monday 1 p.m.到Tuesday 1 p.m.这段时间所做的修改保存在```gbichot2-bin.000009```文件中（我们也应该将该备份拷贝到其他一个安全的环境中）。


MySQL binlog会占用大量的硬盘空间。如果要释放这些空间的话，需要适时的不断purge这些日志，其中一种方法就是通过删除那些已经不再需要的日志文件，比如说当我们在进行一个全量备份时就可以把全量备份之前的binlog日志删掉：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --single-transaction --flush-logs --master-data=2 \
--all-databases --delete-master-logs > backup_sunday_1_PM.sql
{% endhighlight %}
这里注意，假如你当前的MySQL Server是一个replication master的话，那么通过```mysqldump --delete-master-logs```删除binlog是很危险的，这是因为slave也许还没有处理完master的binlog日志。


<pre>
说明：

1） --master-data选项

在上面我们用到了mysqldump的--master-data选项，该选项将binlog的偏移位置和文件名追加到输出文件中。
如果值为1，将会输出CHANGE MASTER命令； 如果值为2，输出的CHANGE MASTER命令前添加注释信息。该选项将打开
--lock-all-tables选项，除非--single-transaction也被指定（在这种情况下，将会在开始执行dump时的很短一段
时间内获得一把读锁，请参看下面对--single-transaction的说明）。在所有情况下，在执行dump时都会引发对日志
的相关操作。本选项会自动的关闭--lock-tables

2) --single-transaction选项

在一个事务中，通过dump所有的数据表来创建一个一致性的snapshot。一般只工作于那些支持事务的存储引擎表（例如InnoDB存
储引擎），在使用mysqldump时本选项对于其他的存储引擎并不能保证一致性。当一个--single-transaction dump正在执行
时，为了保证导出一个有效的dump文件（即正确的table内容以及binlog偏移位置），任何其他的客户端都不应该使用下述SQL
语句： ALTER TABLE, DROP TABLE, RENAME TABLE, TRUNCATE TABLE，这是因为产生一致性dump文件时并没有对这些操作
进行隔离。本选项会自动的关闭--lock-tables
</pre>


### 3.2 使用备份来进行恢复
现在假设在```星期三```的早上8点(Wensday 8 a.m.)有一个灾难性的崩溃，我们需要使用备份来进行恢复。为了恢复，首先我们需要使用在```Sunday 1 p.m.```建立的全量备份来完成。该全量备份是一系列的SQL 语句，因此我们可以很容易的进行恢复：
{% highlight string %}
# mysql -uroot -ptestAa@123 < backup_sunday_1_PM.sql
{% endhighlight %}

执行完上面步骤之后数据就可以恢复到```Sunday 1 p.m.```。然后，为了恢复之后数据的更改，我们必须使用增量备份来进行恢复，即使用```gbichot2-bin.000007```及```gbichot2-bin.000008```日志文件来进行恢复。这里我们可以从某个备份位置获取到这些文件，然后通过类似于如下的命令来进行处理：
{% highlight string %}
# mysqlbinlog gbichot2-bin.000007 gbichot2-bin.000008 | mysql -uroot -ptestAa@123
{% endhighlight %}

到此为止，我们已经将数据恢复到了```Tuesday 1 p.m.```这一时刻，但是仍然丢失了从该时刻起到系统崩溃这一段时间的数据。为了避免这一部分数据的丢失，我们必须要有存放于其他安全为止的binlog(例如RAID硬盘或SAN硬盘等)文件（这就是为什么我们启动时会用```--log-bin```选项指定一个不同于MySQL数据目录的其他物理硬盘位置。因为这样做，我们可以即使在MySQL主数据目录丢失的情况下，仍能在一个安全的地方保存日志文件）。假如我们在安全的地方建立了相应的binlog，则我们会有```gbichot2-bin.000009```这样一个文件（或者其他后续的binlog文件），然后我们就可以通过使用```mysqlbinlog```和```mysql```来将数据恢复到崩溃前的最新时刻：
{% highlight string %}
# mysqlbinlog gbichot2-bin.000009 ... | mysql -uroot -ptestAa@123
{% endhighlight %}
更多关于使用```mysqlbinlog```来处理binlog文件的信息，请参看后面一章```使用binlog来做基于时间点的增量恢复```.




### 3.3 备份策略总结
在操作系统崩溃或者电源断电的情况下，```InnoDB```可以通过它自身来进行数据的恢复。但是为了确保数据不被丢失，一般你还是需要：

* 在运行MySQL Server时，总是开启```--log-bin```选项，甚至是```--log-bin=log_name```,这里log_name可能是某一个安全的媒体介质。假如你有这样一个安全的媒体存储介质，该技术也是一种很要的硬盘负载均衡的方法。

* 周期性的建立全量备份。请参看```3.1 建立备份策略```介绍的在线非阻塞备份

* 通过执行```FLUSH LOGS```或者```mysqladmin flush-logs```来刷新日志，周期性的进行增量备份






<br />
<br />
**[参看]**:

1. [学会用各种姿势备份MySQL数据库](http://www.cnblogs.com/liangshaoye/p/5464794.html)

2. [Mysql Binlog三种格式介绍及分析](https://www.cnblogs.com/itcomputer/articles/5005602.html)




<br />
<br />
<br />

