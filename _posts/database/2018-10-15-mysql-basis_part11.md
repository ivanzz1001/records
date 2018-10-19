---
layout: post
title: MySQL数据库备份与恢复
tags:
- database
categories: database
description: MySQL数据库备份与恢复
---


在MySQL运行过程中做好数据库备份是很重要的，因为在假如遇到系统崩溃、硬件损坏、用户误删数据的情况下，你就可以通过数据库备份来进行数据恢复。此外，在需要对数据库进行版本更新重新安装MySQL时，做好数据备份也是很有必要的； 在建立MySQL主从复制时也是需要进行数据库备份。

MySQL提供了很多的本分策略，你可以根据自身需要选择最合适的方法来进行备份。本章主要包括如下方面的内容：

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

* 对于恢复逻辑备份来说，SQL格式的dump文件可以直接有```mysql``` client进行处理。如果要加载```delimited-text```文件，使用```LOAD DATA INFILE```语句或者```mysqlimport```客户端即可。


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













<br />
<br />
**[参看]**:

1. [学会用各种姿势备份MySQL数据库](http://www.cnblogs.com/liangshaoye/p/5464794.html)

2. [Mysql Binlog三种格式介绍及分析](https://www.cnblogs.com/itcomputer/articles/5005602.html)




<br />
<br />
<br />

