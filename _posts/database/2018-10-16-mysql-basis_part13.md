---
layout: post
title: MySQL数据库备份与恢复(三）
tags:
- database
categories: database
description: MySQL数据库备份与恢复
---


本章首先讲述一下数据库的导入与导出，然后再通过相应的示例讲述一下MySQL数据库的备份。当前SQL版本为：
{% highlight string %}
mysql> status;
--------------
mysql  Ver 14.14 Distrib 5.7.22, for Linux (x86_64) using  EditLine wrapper
{% endhighlight %}


<!-- more -->

## 1. 数据库的导入与导出

### 1.1 导出数据

**1) 使用SELECT ... INTO OUTFILE语句导出数据**

以下示例中我们将数据表```course```导出到```/tmp/course.txt```文件中：
{% highlight string %}
mysql> SELECT * FROM course INTO OUTFILE '/tmp/course.txt';
ERROR 1290 (HY000): The MySQL server is running with the --secure-file-priv option so it cannot execute this statement
mysql> show variables like '%secure%';
+--------------------------+-----------------------+
| Variable_name            | Value                 |
+--------------------------+-----------------------+
| require_secure_transport | OFF                   |
| secure_auth              | ON                    |
| secure_file_priv         | /var/lib/mysql-files/ |
+--------------------------+-----------------------+
3 rows in set (0.01 sec)

mysql> SELECT * FROM course INTO OUTFILE '/var/lib/mysql-files/course.txt';
Query OK, 7 rows affected (0.01 sec)


# cat /var/lib/mysql-files/course.txt 
1       MySQL从入门到精通       1001
2       爱情与婚姻      1002
3       Java从入门到放弃        1003
4       商务礼仪        1004
5       表演的艺术      1005
6       民法    1006
7       民法    1001
{% endhighlight %}
也可以通过命令选项设置数据输出的指定格式，以下示例为导出`CSV```格式：
{% highlight string %}
mysql> SELECT * FROM course INTO OUTFILE '/var/lib/mysql-files/course.txt' FIELDS TERMINATED BY ',' 
 ENCLOSED BY '"' LINES TERMINATED BY "\r\n";
Query OK, 7 rows affected (0.00 sec)

# cat /var/lib/mysql-files/course.txt 
"1","MySQL从入门到精通","1001"
"2","爱情与婚姻","1002"
"3","Java从入门到放弃","1003"
"4","商务礼仪","1004"
"5","表演的艺术","1005"
"6","民法","1006"
"7","民法","1001"
{% endhighlight %}

下面的例子中，生成一个文件，各值用逗号隔开。这种格式可以被许多程序使用：
{% highlight string %}
mysql> SELECT id-1, stuid, coursename FROM course INTO OUTFILE '/var/lib/mysql-files/course.txt' 
    ->  FIELDS TERMINATED BY ',' ENCLOSED BY '"' LINES TERMINATED BY "\n";

# cat /var/lib/mysql-files/course.txt 
"0","1001","MySQL从入门到精通"
"1","1002","爱情与婚姻"
"2","1003","Java从入门到放弃"
"3","1004","商务礼仪"
"4","1005","表演的艺术"
"5","1006","民法"
"6","1001","民法"
{% endhighlight %}


```SELECT ... INTO OUTFILE```语句有如下属性：

* ```LOAD DATA INFILE```是```SELECT ... INTO OUTFILE```的逆操作，为了将一个数据库的数据写入一个文件，使用```SELECT ... INTO OUTFILE```，为了将文件读回数据库，使用```LOAD DATA INFILE```。

* ```SELECT ... INTO OUTFILE```可以把被选择的行写入到一个文件中。该文件被创建到服务器主机上，因此你必须要有目录的写权限；

* 输出不能是一个已经存在的文件，防止文件数据被篡改；

* 你需要有一个登录服务器的账号来检索文件，否则```SELECT ... INTO OUTFILE```不会起任何作用；

* 在UNIX中，该文件被创建后是可读的，权限由MySQL服务器所拥有。这就意味着，你虽然可以读取该文件，但无法删除该文件（除非拥有超级权限）

<br />

**2）导出表作为原始数据**

```mysqldump```是mysql用于转存数据库的实用程序。它主要产生一个SQL脚本，其中包含从头重新创建数据库所必需的命令```CREATE TABLE```、```INSERT```等。使用```mysqldump```导出数据时可以使用```--tab```选项来指定导出到哪个目录（注意该目录必须是```可写```的)。例如：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --no-create-info --tab=/var/lib/mysql-files/ test course
mysqldump: [Warning] Using a password on the command line interface can be insecure.
# ls /var/lib/mysql-files/
course.sql  course.txt

# cat /var/lib/mysql-files/course.sql
# cat /var/lib/mysql-files/course.txt
1       MySQL从入门到精通       1001
2       爱情与婚姻      1002
3       Java从入门到放弃        1003
4       商务礼仪        1004
5       表演的艺术      1005
6       民法    1006
7       民法    1001
{% endhighlight %}


**3) 导出SQL格式数据**
{% highlight string %}
# mysqldump -uroot -ptestAa@123 test course > /var/lib/mysql-files/course.txt
mysqldump: [Warning] Using a password on the command line interface can be insecure.

# cat /var/lib/mysql-files/course.txt 
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
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
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

--
-- Dumping data for table `course`
--

LOCK TABLES `course` WRITE;
/*!40000 ALTER TABLE `course` DISABLE KEYS */;
INSERT INTO `course` VALUES (1,'MySQL从入门到精通',1001),(2,'爱情与婚姻',1002),(3,'Java从入门到放弃',1003),
(4,'商务礼仪',1004),(5,'表演的艺术',1005),(6,'民法',1006),(7,'民法',1001);
/*!40000 ALTER TABLE `course` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2018-10-24 19:57:59
{% endhighlight %}

如果你需要备份所有的数据库，可以使用如下的命令：
{% highlight string %}
mysqldump -u root -ptestAa@123 --all-databases > database_dump.txt
{% endhighlight %}


**4) 将数据表及数据库拷贝至其他主机**

如果你需要将数据拷贝至其他的MySQL服务器上，你可以在```mysqldump```命令中指定数据库名及数据库表（如果完整备份数据库，则不用指定表名)。

在源主机上执行以下命令，将数据备份至dump.txt文件中：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 database_name table_name > /var/lib/mysql-files/dump.txt
{% endhighlight %}

此时，如果需要将备份的数据库导入到MysQL服务器中，可以使用以下命令，使用以下命令以前你需要确认数据库已经创建：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 database_name < dump.txt
{% endhighlight %}


### 1.2 MySQL导入数据
**1) mysql命令导入**

语法格式为：
{% highlight string %}
# mysql -u用户名 -p密码 < dumpfile.txt
{% endhighlight %}
例如：
{% highlight string %}
# mysql -uroot -ptestAa@123 < dump.txt
{% endhighlight %}

以上命令将备份的整个数据库```dump.txt```导入。

**2) source命令导入**

source命令导入数据库需要先登录到数据库终端：
{% highlight string %}
mysql> create database aaa;
mysql> use aaa;
mysql> set names utf8;             //设置编码
mysql> source /tmp/abc.sql
{% endhighlight %}

**3) 使用LOAD DATA导入数据**

MySQL中提供了```LOAD DATA INFILE```语句来插入数据。以下实例中将从当前目录中读取文件```dump.txt```,将该文件中的数据插入到当前数据库的```mytbl```表中。
{% highlight string %}
mysql> LOAD DATA LOCAL INFILE 'dump.txt' INTO TABLE mytbl;
{% endhighlight %}
如果指定```LOCAL```关键字，则表明从客户主机上按路径读取文件；如果没有指定，则文件在服务器上按路径读取文件。可以明确的指出在```LOAD DATA```语句中列的```分隔符```和```行尾标记```(默认是```tab```与```\n```)。例如：
{% highlight string %}
mysql> LOAD DATA LOCAL INFILE 'dump.txt' INTO TABLE mytbl
  -> FIELDS TERMINATED BY ':'
  -> LINES TERMINATED BY '\r\n';
{% endhighlight %}

```LOAD DATA```默认情况下是按照数据文件中列的顺序插入数据的，如果数据文件中的列与插入表中的列不一致，则需要指定列的顺序。如在数据文件中列的顺序为```a,b,c```，但在插入表的列顺序为```b,c,a```，则数据导入语法如下：
{% highlight string %}
mysql> LOAD DATA LOCAL INFILE 'dump.txt' 
    -> INTO TABLE mytbl (b, c, a);
{% endhighlight %}


**4) mysqlimport导入数据**

mysqlimport客户端提供了```LOAD DATA INFILE```这样的SQL语句的命令行接口。```mysqlimport```的大多数选项直接对应于```LOAD DATA INFILE```子句。从文件```dump.txt```中将数据导入到```mytbl```数据表中，可以使用如下命令(这里注意文件名必须与表名相同）：
{% highlight string %}
# mysqlimport -u root -ptestAa@123 --local database_name mytbl.txt
{% endhighlight %}

mysqlimport命令可以指定选项来设置相应的格式：
{% highlight string %}
# mysqlimport -u root -ptestAa@123 --local --fields-terminated-by=":" \
   --lines-terminated-by="\r\n"  database_name mytbl.txt
{% endhighlight %}
mysqlimport语句中使用```--columns```选项来设置列的顺序：
{% highlight string %}
# mysqlimport -u root -ptestAa@123 --local --columns=b,c,a \
    database_name mytbl.txt
{% endhighlight %}

下面列出```mysqlimport```常用的一些选项：

* ```--delete```或```-d```: 新数据导入数据表中之前删除数据表中额所有信息；

* ```--force```或```-f```: 不管是否遇到错误，mysqlimport将强制继续插入数据

* ```--ignore```或```-i```: mysqlimport跳过或忽略那些有相同唯一关键字的行，导入文件中的数据将被忽略

* ```--lock-tables```或```-l```: 数据被插入之前锁住表，这样就防止了你在更新数据库时由于用户的查询与更新产生干扰；

* ```--replace```或```-r```: 这个选项与```--ignore```选项相反。此选项将替代表中有相同唯一关键字的记录；

* ```--fields-enclosed-by=char```: 指定文本文件中数据的记录以什么括起的，很多情况下数据以双引号括起。默认情况下数据是没有被字符括起的。

* ```--fields-terminated-by=char```: 指定各个数据的值之间的分隔符。默认的分隔符是```tab```键值

* ```--line-terminated-by=str```: 此选项指定文本文件中行与行之间的分割字符串。默认情况下,mysqlimport以```\n```作为行分隔符。


## 2. MySQL表锁以及FLUSH TABLES操作

**1) 创建测试表**

如下我们创建测试表```t1```和```t2```:
{% highlight string %}
use test;

DROP TABLE if EXISTS `t1`;

CREATE TABLE `t1` (
  `i` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`i`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO t1(i) VALUES(100);


DROP TABLE if EXISTS `t2`;
CREATE TABLE `t2` (
  `i` int(255) NOT NULL,
  PRIMARY KEY (`i`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO t2(i) VALUES(1000);
{% endhighlight %}
执行以下语句进行导入：
<pre>
# mysql -uroot -ptestAa@123 < ./test.sql 
mysql: [Warning] Using a password on the command line interface can be insecure.
</pre>

**2） 测试读锁**

**Session 1**:
{% highlight string %}
mysql> lock table t1 read;
Query OK, 0 rows affected (0.00 sec)

mysql> select * from t1 limit 1;
+-----+
| i   |
+-----+
| 100 |
+-----+
1 row in set (0.00 sec)


mysql> INSERT INTO t1(i) VALUES(101);
ERROR 1099 (HY000): Table 't1' was locked with a READ lock and can't be updated

mysql> UPDATE t1 SET i=101 where i=100;
ERROR 1099 (HY000): Table 't1' was locked with a READ lock and can't be updated
{% endhighlight %}
由上面可见，对于加了读锁的表，在执行加读锁的```session```中可进行读操作，但不能进行插入与更新，当然也不能进行删除。下面再对```t2```进行操作：
{% highlight string %}
mysql> select * from t2 limit 1;
ERROR 1100 (HY000): Table 't2' was not locked with LOCK TABLES
mysql> INSERT INTO t2(i) VALUES(1001);
ERROR 1100 (HY000): Table 't2' was not locked with LOCK TABLES
mysql> UPDATE t2 SET i=1001 where i=1000;
ERROR 1100 (HY000): Table 't2' was not locked with LOCK TABLES
{% endhighlight %}
从上面可知，对于没有加锁的表，不能在执行加锁的session中对表进行访问（包括增删查改）。


<br />

**Session 2**:
{% highlight string %}
mysql> SELECT * FROM t1 LIMIT 1;
+-----+
| i   |
+-----+
| 100 |
+-----+
1 row in set (0.00 sec)

mysql> SELECT * FROM t2 LIMIT 1;
+------+
| i    |
+------+
| 1000 |
+------+
1 row in set (0.00 sec)

mysql> INSERT INTO t2(i) VALUES(1001);
Query OK, 1 row affected (0.00 sec)

mysql> INSERT INTO t1(i) VALUES(101);
//卡死于此
{% endhighlight %}
由上面可见，加了读锁的表，在不同session中不可以进行插入操作（更新和删除同理）。但是可以对任一的表进行读取。

<br />
经实验验：

* 如果我们执行```exit```退出```session1```，那么上面卡死在```session2```中插入就会马上被执行。

* 我们在```session1```中执行```unlock table```，那么上面卡死在```session2```中的插入也会马上执行



**3） 测试写锁**

下面我们再来测试一下写锁（记得先在```session1```解锁刚刚被加锁的表）.

**Session 1**:
{% highlight string %}
mysql> lock table t1 write;
Query OK, 0 rows affected (0.01 sec)

mysql> INSERT INTO t1(i) VALUES(102);
Query OK, 1 row affected (0.01 sec)

mysql> SELECT * FROM t1 LIMIT 3;
+-----+
| i   |
+-----+
| 100 |
| 101 |
| 102 |
+-----+
3 rows in set (0.00 sec)

mysql> SELECT * FROM t2 LIMIT 3;
ERROR 1100 (HY000): Table 't2' was not locked with LOCK TABLES

mysql> UPDATE t1 SET i=103 WHERE i=102;
Query OK, 1 row affected (0.01 sec)
Rows matched: 1  Changed: 1  Warnings: 0

mysql> DELETE FROM t1 WHERE i=103;
Query OK, 1 row affected (0.01 sec)
{% endhighlight %}


**Session 2**:
{% highlight string %}
mysql> select * from t2 limit 3;
+------+
| i    |
+------+
| 1000 |
| 1001 |
+------+
2 rows in set (0.01 sec)

mysql> select * from t1 limit 3;
//卡死于此
{% endhighlight %}
从上面可知，若```session```中对表加了写锁，则同一```session```中对该表可以进行增删查改操作。但其他```session```中对该表的读取和修改都会被阻塞，直至表锁被释放。

**4） 在未执行完的query上加锁**

接下来，我们了解下加锁前表上有尚未执行完成的```query```时会怎样？

**Session 1**(记得先解锁刚刚被加锁的表):
{% highlight string %}
mysql> select i,sleep(60) from t1 limit 1;
+-----+-----------+
| i   | sleep(60) |
+-----+-----------+
| 100 |         0 |
+-----+-----------+
1 row in set (1 min 7.08 sec)
{% endhighlight %}

**Session 2**:
{% highlight string %}
mysql> lock table t1 read;
Query OK, 0 rows affected (0.00 sec)

mysql> unlock table;
Query OK, 0 rows affected (0.00 sec)

mysql> lock table t1 write;
Query OK, 0 rows affected (52.75 sec)
{% endhighlight %}

可见，表上有尚未完成的查询操作时可以加读锁，但加写锁会阻塞。

<br />

**Session 1**:
{% highlight string %}
mysql> begin;
Query OK, 0 rows affected (0.00 sec)

mysql> INSERT INTO t1(i) VALUES(105);
Query OK, 1 row affected (0.00 sec)

mysql> commit;							//等待session2执行获取读锁再提交
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}

**Session 2**:
{% highlight string %}
mysql> lock table t1 read;
Query OK, 0 rows affected (23.39 sec)
{% endhighlight %}

上面我们看到，表上有尚未提交的事务，获取读锁会阻塞。


**5) FLUSH TABLES与锁**

接下来，来了解下```FLUSH TABLES```时表上有尚未执行完成的查询会怎样？

**Session 1**:
{% highlight string %}
mysql> select i,sleep(60) from t1 limit 1;
{% endhighlight %}

**Session 2**:
{% highlight string %}
mysql> flush tables t1;
{% endhighlight %}
可见，由于将要被```flush```的表上有查询尚未完成，因此```flush tables```操作被阻塞，直至所有表上的操作完成，```flush tables```操作才得以完成。

**Session 3**:
{% highlight string %}
mysql> select * from t1 limit 1;
{% endhighlight %}
由于```flush tables```被阻塞，导致后续其他```session```中对该表的查询也会被阻塞。

**Session 4**:
{% highlight string %}
mysql> use information_schema;
Database changed
mysql> select * from processlist where db='test';
+----+------+-----------+------+---------+------+-------------------------+------------------------------------+
| ID | USER | HOST      | DB   | COMMAND | TIME | STATE                   | INFO                               |
+----+------+-----------+------+---------+------+-------------------------+------------------------------------+
| 22 | root | localhost | test | Query   |   17 | Waiting for table flush | select * from t1 limit 1           |
| 14 | root | localhost | test | Query   |   21 | Waiting for table flush | flush tables t1                    |
| 19 | root | localhost | test | Query   |   24 | User sleep              | select i,sleep(60) from t1 limit 1 |
+----+------+-----------+------+---------+------+-------------------------+------------------------------------+
3 rows in set (0.01 sec)
{% endhighlight %}
在另外一个session中，通过执行上面的查询(或```SHOW PROCESSLIST```)我们可以看到线程的状态。直至ID为```19```的线程执行完了SQL查询之后，```flush tables```动作才得以完成。继而后续的```select```操作才顺利完成。

<br />

从上面```Session 1/2/3/4```可见，执行```flush tables```操作或者隐含包含```flush tables```的操作时要小心谨慎。在上面所有步骤都执行完成之后，我们看到：
{% highlight string %}
mysql> select * from processlist where db='test';
+----+------+-----------+------+---------+------+-------+------+
| ID | USER | HOST      | DB   | COMMAND | TIME | STATE | INFO |
+----+------+-----------+------+---------+------+-------+------+
| 22 | root | localhost | test | Sleep   |  113 |       | NULL |
| 14 | root | localhost | test | Sleep   |  117 |       | NULL |
| 19 | root | localhost | test | Sleep   |  120 |       | NULL |
+----+------+-----------+------+---------+------+-------+------+
{% endhighlight %}


## 3. MySQL binlog基本配置与格式设定
我们在前面已经较为详细的介绍了MySQL binlog的三种格式：```statement-based log```、```row-based log```以及```mix-based log```。这里我们只讲述一下```binlog```的基本配置与格式设定。

### 3.1 基本配置
mysql binlog日志格式可以在mysql的配置文件```my.cnf```中通过相应的属性来进行设置：
<pre>
[mysqld]
# binlog的日志格式
binlog_format=MIXED

# 指定binlog日志名（一般存放于/var/lib/mysql目录下)
log_bin=master-logbin

# binlog过期清理时间
expire_logs_days=7

# binlog每一个日志文件大小
max_binlog_size=100M
</pre>

### 3.2 mysql binlog日志分析
当前我们MySQL的```my.cnf```配置如下：
{% highlight string %}
[mysqld]
log-bin=master-logbin
server-id=1921681001
{% endhighlight %}

下面我们来简单看一下MySQL的binlog文件：
{% highlight string %}
# cp /var/lib/mysql/master-logbin* ./

# mysqlbinlog ./master-logbin.000002 | more
/*!50530 SET @@SESSION.PSEUDO_SLAVE_MODE=1*/;
/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,COMPLETION_TYPE=0*/;
DELIMITER /*!*/;
# at 4
#181011 17:16:40 server id 1921681001  end_log_pos 123 CRC32 0x8fe8a193         Start: binlog v 4, server v 5.7.22-log created 181011 17:16:40 at startup
ROLLBACK/*!*/;
BINLOG '
+BS/Ww9phopydwAAAHsAAAAAAAQANS43LjIyLWxvZwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAD4FL9bEzgNAAgAEgAEBAQEEgAAXwAEGggAAAAICAgCAAAACgoKKioAEjQA
AZOh6I8=
'/*!*/;
# at 123
#181011 17:16:40 server id 1921681001  end_log_pos 154 CRC32 0x975f89ab         Previous-GTIDs
# [empty]
# at 154
#181011 18:58:39 server id 1921681001  end_log_pos 219 CRC32 0x93411134         Anonymous_GTID  last_committed=0        sequence_number=1       rbr_only=no
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at 219
#181011 18:58:39 server id 1921681001  end_log_pos 399 CRC32 0x79ae24eb         Query   thread_id=2     exec_time=0     error_code=0
SET TIMESTAMP=1539255519/*!*/;
SET @@session.pseudo_thread_id=2/*!*/;
SET @@session.foreign_key_checks=1, @@session.sql_auto_is_null=0, @@session.unique_checks=1, @@session.autocommit=1/*!*/;
SET @@session.sql_mode=1436549152/*!*/;
SET @@session.auto_increment_increment=1, @@session.auto_increment_offset=1/*!*/;
/*!\C utf8 *//*!*/;
SET @@session.character_set_client=33,@@session.collation_connection=33,@@session.collation_server=8/*!*/;
SET @@session.lc_time_names=0/*!*/;
SET @@session.collation_database=DEFAULT/*!*/;
CREATE USER 'repl'@'%' IDENTIFIED WITH 'mysql_native_password' AS '*3FFD6E04483514E561849FD8D866C05A69EFA570'
/*!*/;
# at 399
#181011 18:58:51 server id 1921681001  end_log_pos 464 CRC32 0x8ef7b206         Anonymous_GTID  last_committed=1        sequence_number=2       rbr_only=no
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at 464
#181011 18:58:51 server id 1921681001  end_log_pos 595 CRC32 0xfc5ae713         Query   thread_id=2     exec_time=0     error_code=0
SET TIMESTAMP=1539255531/*!*/;
GRANT REPLICATION SLAVE ON *.* TO 'repl'@'%'
/*!*/;
# at 595
#181011 18:59:09 server id 1921681001  end_log_pos 660 CRC32 0x2c95c9cb         Anonymous_GTID  last_committed=2        sequence_number=3       rbr_only=no
SET @@SESSION.GTID_NEXT= 'ANONYMOUS'/*!*/;
# at 660
#181011 18:59:09 server id 1921681001  end_log_pos 747 CRC32 0x7046e9a5         Query   thread_id=2     exec_time=0     error_code=0
SET TIMESTAMP=1539255549/*!*/;
SET @@session.time_zone='SYSTEM'/*!*/;
FLUSH PRIVILEGES
{% endhighlight %}

这里我们主要是需要注意一下```server id```以及```end_log_pos```这两个字段。

## 4. MySQL 数据备份实战
MySQL数据的备份类型根据自身的特性主要分为以下几组：

* 完全备份： 是备份整个数据集(即整个数据库)

* 部分备份: 备份部分数据集（例如，只备份一个表）。而部分备份又可以分为```增量备份```和```差异备份```。
<pre>
增量备份： 备份自上一次备份以来（增量或完全）变化的数据。 优点是节约空间，但是还原较为麻烦

差异备份： 备份自上一次完全备份以来变化的数据。优点是还原比增量备份简单，缺点是浪费空间。
</pre>

![db-backup](https://ivanzz1001.github.io/records/assets/img/db/db_increment_backup.jpg)


### 4.1 MySQL备份数据的方式
在MySQL中我们备份数据一般有几种方式：

* 热备份： 指的是当数据库进行备份时，数据库的读写操作均不受影响；

* 温备份： 指的是当数据库进行备份时，数据库的读操作可以执行，但不能执行写操作；

* 冷备份; 指的是当数据库进行备份时，数据库不能进行读、写操作，即数据库要下线；

MySQL中进行不同方式的备份还需要考虑存储引擎是否支持：

![db-backup](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_backup.jpg)

我们在考虑数据备份时，除了数据库的运行状态之外，还需要考虑对于MySQL数据库中数据的备份方式：

* 物理备份： 一般就是通过```tar```、```cp```等命令直接打包复制数据库的数据文件以达到备份的效果；

* 逻辑备份： 通过特定工具从数据库中导出数据并另存备份（逻辑备份会丢失数据精度）。

### 4.2 备份需要考虑的问题
定制备份策略前，我们还需要考虑一些问题：

**我们要备份什么？**

一般情况下，我们需要备份的数据分为以下几种：

* 数据

* 二进制日志、InnoDB事务日志

* 代码（存储过程、存储函数、触发器、事件调度器）

* 服务器配置文件

<br />

**备份工具**

这里我们例举出常用的几种备份工具：
<pre>
mysqldump: 逻辑备份工具、适用于所有的存储引擎，支持温备份、完全备份、部分备份， 对于InnoDB存储存储引擎支持热备份；

cp、tar等归档复制工具： 物理备份工具，适用于所有的存储引擎，冷备份、完全备份、部分备份

lvm2 snapshot: 近乎热备，借助文件系统管理工具进行备份

mysqlhotcopy: 名不副实的一个工具，几乎冷备，仅支持MyISAM存储引擎

xtrabackup: 一款非常强大的InnoDB/XtraDB热备工具，支持完全备份、增量备份，由percona提供
</pre>

### 4.3 实战演练

这里我们介绍两种：

* 物理全量备份 + 增量备份

* 逻辑全量备份 + 增量备份

#### 4.3.1 物理全量备份 + 增量备份

我们当前数据库状态：
{% highlight string %}
mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| app                |
| mysql              |
| performance_schema |
| sys                |
| test               |
+--------------------+
6 rows in set (0.01 sec)

mysql> use test;
Database changed
mysql> show tables;
+----------------+
| Tables_in_test |
+----------------+
| course         |
| runoob_tbl     |
| student        |
+----------------+
3 rows in set (0.00 sec)
{% endhighlight %}
并且当前我们已经开启了binlog日志：
{% highlight string %}
mysql> SHOW VARIABLES LIKE 'sql_log_bin';
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| sql_log_bin   | ON    |
+---------------+-------+
1 row in set (0.00 sec)
{% endhighlight %}

**1) 向数据库施加读锁**

{% highlight string %}
mysql> FLUSH TABLES WITH READ LOCK;
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %} 

**2） 记录下当前binlog日志**
{% highlight string %}
mysql> SHOW MASTER STATUS;
+----------------------+----------+--------------+------------------+-------------------+
| File                 | Position | Binlog_Do_DB | Binlog_Ignore_DB | Executed_Gtid_Set |
+----------------------+----------+--------------+------------------+-------------------+
| master-logbin.000005 |     4713 |              |                  |                   |
+----------------------+----------+--------------+------------------+-------------------+
1 row in set (0.00 sec)
{% endhighlight %}

**3) 拷贝数据库文件做全量备份**
<pre>
# mkdir full_bakup
# ls /var/lib/mysql/
app/                  client-cert.pem       ib_logfile0           master-logbin.000002  master-logbin.index   performance_schema/   server-key.pem
auto.cnf              client-key.pem        ib_logfile1           master-logbin.000003  mysql/                private_key.pem       sys/
ca-key.pem            ib_buffer_pool        ibtmp1                master-logbin.000004  mysql.sock            public_key.pem        test/
ca.pem                ibdata1               master-logbin.000001  master-logbin.000005  mysql.sock.lock       server-cert.pem       
# cp -ar /var/lib/mysql/* ./full_bakup/
# tar -zcvf full_bakup.tar.gz ./full_bakup
</pre>

**4) 修改数据，做增量备份**

首先在上面执行```FLUSH TABLES WITH READ LOCK```的session中执行如下命令以解锁数据库：
{% highlight string %}
mysql> unlock table;
Query OK, 0 rows affected (0.00 sec)
{% endhighlight %}

然后新创建一个```数据库```及```表```，并向表中插入一些数据：
{% highlight string %}
mysql> CREATE TABLE person(
    ->  id INT(11) PRIMARY KEY AUTO_INCREMENT,
    ->  name char(64) NOT NULL,
    ->  age int
    -> )ENGINE=InnoDB DEFAULT CHARSET=utf8;
Query OK, 0 rows affected (0.02 sec)

mysql> INSERT INTO person(name,age) VALUES("ivan1001",20);
Query OK, 1 row affected (0.01 sec)

mysql> INSERT INTO person(name, age) VALUES("scarllet",18);
Query OK, 1 row affected (0.00 sec)

mysql> select * from person;
+----+----------+------+
| id | name     | age  |
+----+----------+------+
|  1 | ivan1001 |   20 |
|  2 | scarllet |   18 |
+----+----------+------+
2 rows in set (0.01 sec)
{% endhighlight %}

接着再获取增量备份日志：
{% highlight string %}
mysql> FLUSH TABLES WITH READ LOCK;
Query OK, 0 rows affected (0.00 sec)

mysql> SHOW MASTER STATUS;
+----------------------+----------+--------------+------------------+-------------------+
| File                 | Position | Binlog_Do_DB | Binlog_Ignore_DB | Executed_Gtid_Set |
+----------------------+----------+--------------+------------------+-------------------+
| master-logbin.000005 |     5698 |              |                  |                   |
+----------------------+----------+--------------+------------------+-------------------+
1 row in set (0.00 sec)
{% endhighlight %}

结合上次我们做的全量备份，当前我们只需要再备份```master-logbin.000005```这个文件即可:
<pre>
# mkdir increment_bakup
# cp /var/lib/mysql/master-logbin.000005 increment_bakup/
# tar -zcvf increment_bakup.tar.gz ./increment_bakup
</pre>

**5) 停止数据库，并删除mysql数据目录**

我们通过如下方式来模拟数据丢失：
{% highlight string %}
# mysqladmin -uroot -ptestAa@123 shutdown
# rm -rf /var/lib/mysql
{% endhighlight %}

**6) 重新启动MySQL**
{% highlight string %}
# systemctl start mysqld
# systemctl status mysqld
● mysqld.service - MySQL Server
   Loaded: loaded (/usr/lib/systemd/system/mysqld.service; enabled; vendor preset: disabled)
   Active: active (running) since Fri 2018-10-26 14:12:21 CST; 11s ago
     Docs: man:mysqld(8)
           http://dev.mysql.com/doc/refman/en/using-systemd.html
  Process: 97944 ExecStart=/usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid $MYSQLD_OPTS (code=exited, status=0/SUCCESS)
  Process: 97863 ExecStartPre=/usr/bin/mysqld_pre_systemd (code=exited, status=0/SUCCESS)
 Main PID: 97947 (mysqld)
   Memory: 318.9M
   CGroup: /system.slice/mysqld.service
           └─97947 /usr/sbin/mysqld --daemonize --pid-file=/var/run/mysqld/mysqld.pid

Oct 26 14:12:16 bogon mysqld_pre_systemd[97863]: Full path required for exclude: net:[4026533235].
Oct 26 14:12:16 bogon mysqld_pre_systemd[97863]: Full path required for exclude: net:[4026533331].
Oct 26 14:12:17 bogon mysqld_pre_systemd[97863]: Full path required for exclude: net:[4026532659].
Oct 26 14:12:17 bogon mysqld_pre_systemd[97863]: Full path required for exclude: net:[4026533043].
Oct 26 14:12:17 bogon mysqld_pre_systemd[97863]: Full path required for exclude: net:[4026532851].
Oct 26 14:12:17 bogon mysqld_pre_systemd[97863]: Full path required for exclude: net:[4026532947].
Oct 26 14:12:17 bogon mysqld_pre_systemd[97863]: Full path required for exclude: net:[4026533139].
Oct 26 14:12:17 bogon mysqld_pre_systemd[97863]: Full path required for exclude: net:[4026533235].
Oct 26 14:12:17 bogon mysqld_pre_systemd[97863]: Full path required for exclude: net:[4026533331].
Oct 26 14:12:21 bogon systemd[1]: Started MySQL Server.
[root@bogon mysql-test]# netstat -nlp | grep mysql
tcp6       0      0 :::3306                 :::*                    LISTEN      97947/mysqld        
unix  2      [ ACC ]     STREAM     LISTENING     7821746  97947/mysqld         /var/lib/mysql/mysql.sock
{% endhighlight %}

```注：``` 重启MySQL时, 如果是编译安装的应该不能启动, 如果rpm安装则会重新初始化数据库

接下来我们登录数据库查看一下：
{% highlight string %}
# mysql -uroot -ptestAa@123
mysql: [Warning] Using a password on the command line interface can be insecure.
ERROR 1045 (28000): Access denied for user 'root'@'localhost' (using password: YES)
{% endhighlight %}
我们看到因为所有数据都已经丢失，我们目前已经无法登录数据库了。本次MySQL数据库启动，是MySQL重新初始化的数据库，可以通过如下方式找到初始登录密码：
<pre>
# grep "password" /var/log/mysqld.log 
2018-10-26T06:12:18.237980Z 1 [Note] A temporary password is generated for root@localhost: N=1MH/_yJkB2
2018-10-26T06:16:39.369163Z 2 [Note] Access denied for user 'root'@'localhost' (using password: YES)
</pre>
因此如下我们先使用该密码以登录查看：
{% highlight string %}
# mysql -uroot -pN=1MH/_yJkB2

//登录后必须首先修改密码才能再进行操作
mysql> show databases;
ERROR 1820 (HY000): You must reset your password using ALTER USER statement before executing this statement.
mysql> ALTER USER 'root'@'localhost' IDENTIFIED BY 'testAa@123';
Query OK, 0 rows affected (0.00 sec)

mysql> flush privileges;
Query OK, 0 rows affected (0.01 sec)


mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| sys                |
+--------------------+
4 rows in set (0.00 sec)
{% endhighlight %}
上面我们看到原来的数据全部丢失了。


**7) 全量数据恢复**

首先使用上述创建的全量备份，来进行全量恢复(其实我们可以不用上面```步骤6```的操作)：
<pre>
# mysqld -uroot -ptestAa@123 shutdown
# tar -zxvf full_bakup.tar.gz
#
# mkdir -p /var/lib/mysql
# chown -R mysql:mysql /var/lib/mysql
# cp -ar full_bakup/* /var/lib/mysql/
</pre>
接着重启mysql数据库：
{% highlight string %}
# systemctl start mysqld
Job for mysqld.service failed because the control process exited with error code. See "systemctl status mysqld.service" and "journalctl -xe" for details.

# # journalctl -xe
-- 
-- Unit mysqld.service has failed.
-- 
-- The result is failed.
Oct 26 14:43:29 bogon systemd[1]: Unit mysqld.service entered failed state.
Oct 26 14:43:29 bogon systemd[1]: mysqld.service failed.
Oct 26 14:43:29 bogon setroubleshoot[110880]: SELinux is preventing /usr/sbin/mysqld from 'read, write' accesses on the file master-logbin.index. For complete SELinux mess
Oct 26 14:43:29 bogon python[110880]: SELinux is preventing /usr/sbin/mysqld from 'read, write' accesses on the file master-logbin.index.
                                      
                                      *****  Plugin catchall (100. confidence) suggests   **************************
                                      
                                      If you believe that mysqld should be allowed read write access on the master-logbin.index file by default.
                                      Then you should report this as a bug.
                                      You can generate a local policy module to allow this access.
                                      Do
                                      allow this access for now by executing:
                                      # ausearch -c 'mysqld' --raw | audit2allow -M my-mysqld
                                      # semodule -i my-mysqld.pp
                                      
Oct 26 14:43:29 bogon systemd[1]: mysqld.service holdoff time over, scheduling restart.
Oct 26 14:43:29 bogon systemd[1]: start request repeated too quickly for mysqld.service
Oct 26 14:43:29 bogon systemd[1]: Failed to start MySQL Server.
-- Subject: Unit mysqld.service has failed
-- Defined-By: systemd
-- Support: http://lists.freedesktop.org/mailman/listinfo/systemd-devel
-- 
-- Unit mysqld.service has failed.
-- 
-- The result is failed.
Oct 26 14:43:29 bogon systemd[1]: Unit mysqld.service entered failed state.
Oct 26 14:43:29 bogon systemd[1]: mysqld.service failed.
Oct 26 14:44:01 bogon systemd[1]: Started Session 3689 of user root.
{% endhighlight %}
上面基本上是由于访问权限引起的，这里我们可以有两种方法来进行处理：

* 关闭SeLinux
<pre>
# setenforce 0
# getenforce 
</pre>
上面指示临时关闭```SeLinux```，如果要永久修改，则可以：
<pre>
# cat /etc/selinux/config 

# This file controls the state of SELinux on the system.
# SELINUX= can take one of these three values:
#     enforcing - SELinux security policy is enforced.
#     permissive - SELinux prints warnings instead of enforcing.
#     disabled - No SELinux policy is loaded.
SELINUX=enforcing
# SELINUXTYPE= can take one of three two values:
#     targeted - Targeted processes are protected,
#     minimum - Modification of targeted policy. Only selected processes are protected. 
#     mls - Multi Level Security protection.
SELINUXTYPE=targeted 
</pre>
本方法关闭```SeLinux```虽然可行，但是不建议使用。

* 配置使得允许访问```/var/lib/mysql```目录
{% highlight string %}
# semanage fcontext -a -t mysqld_db_t "/var/lib/mysql(/.*)?"
# chcon -Rv -u system_u -r object_r -t mysqld_db_t /var/lib/mysql
# restorecon -Rv /var/lib/mysql/
# ls -Z /var/lib/mysql
drwxr-x---. mysql mysql system_u:object_r:mysqld_db_t:s0 app
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 auto.cnf
-rw-------. mysql mysql system_u:object_r:mysqld_db_t:s0 ca-key.pem
-rw-r--r--. mysql mysql system_u:object_r:mysqld_db_t:s0 ca.pem
-rw-r--r--. mysql mysql system_u:object_r:mysqld_db_t:s0 client-cert.pem
-rw-------. mysql mysql system_u:object_r:mysqld_db_t:s0 client-key.pem
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 ib_buffer_pool
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 ibdata1
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 ib_logfile0
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 ib_logfile1
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 ibtmp1
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 master-logbin.000001
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 master-logbin.000002
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 master-logbin.000003
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 master-logbin.000004
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 master-logbin.000005
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 master-logbin.000006
-rw-r-----. mysql mysql system_u:object_r:mysqld_db_t:s0 master-logbin.index
drwxr-x---. mysql mysql system_u:object_r:mysqld_db_t:s0 mysql
srwxrwxrwx. mysql mysql system_u:object_r:mysqld_var_run_t:s0 mysql.sock
-rw-------. mysql mysql system_u:object_r:mysqld_db_t:s0 mysql.sock.lock
drwxr-x---. mysql mysql system_u:object_r:mysqld_db_t:s0 performance_schema
-rw-------. mysql mysql system_u:object_r:mysqld_db_t:s0 private_key.pem
-rw-r--r--. mysql mysql system_u:object_r:mysqld_db_t:s0 public_key.pem
-rw-r--r--. mysql mysql system_u:object_r:mysqld_db_t:s0 server-cert.pem
-rw-------. mysql mysql system_u:object_r:mysqld_db_t:s0 server-key.pem
drwxr-x---. mysql mysql system_u:object_r:mysqld_db_t:s0 sys
drwxr-x---. mysql mysql system_u:object_r:mysqld_db_t:s0 test

//重启操作系统
# reboot


# ps -ef | grep mysql
root        891      1  0 16:34 ?        00:00:00 /bin/bash /usr/bin/mysqld_pre_systemd
root       2395    891 19 16:35 ?        00:00:00 /usr/bin/python -Es /usr/sbin/semanage fcontext -a -e /var/lib/mysql /var/lib/mysql-keyring
root       2413   2171  0 16:35 pts/0    00:00:00 grep --color=auto mysql


//登录MySQL
# mysql -uroot -ptestAa@123
mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| app                |
| mysql              |
| performance_schema |
| sys                |
| test               |
+--------------------+
{% endhighlight %}

上面我们看到已经完成了全量恢复。

**8) 增量恢复**

这里我们使用binlog来做增量恢复。上面我们备份了增量日志文件```master-logbin.000005```，并且知道了position。因此我们可以来进行增量恢复：
<pre>
# mysqlbinlog --start-position=4713 --stop-position=5689 ./master-logbin.000005 | mysql -uroot -ptestAa@123
mysql: [Warning] Using a password on the command line interface can be insecure.
</pre>

下面我们登录MySQL以检查是否恢复成功：
{% highlight string %}
# mysql -uroot -ptestAa@123

mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| app                |
| mysql              |
| performance_schema |
| sys                |
| test               |
| test2              |
+--------------------+
7 rows in set (0.00 sec)

mysql> use test2;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed
mysql> show tables;
+-----------------+
| Tables_in_test2 |
+-----------------+
| person          |
+-----------------+
1 row in set (0.00 sec)

mysql> select * from person;
+----+----------+------+
| id | name     | age  |
+----+----------+------+
|  1 | ivan1001 |   20 |
|  2 | scarllet |   18 |
+----+----------+------+
2 rows in set (0.00 sec)
{% endhighlight %}





<br />
<br />
**[参看]**:

1. [学会用各种姿势备份MySQL数据库](http://www.cnblogs.com/liangshaoye/p/5464794.html)

2. [Mysql Binlog三种格式介绍及分析](https://www.cnblogs.com/itcomputer/articles/5005602.html)

3. [MySQL 表锁以及FLUSH TABLES操作](https://blog.csdn.net/zyz511919766/article/details/49336101/)

4. [SELinux and MySQL](https://blogs.oracle.com/jsmyth/selinux-and-mysql)

5. [centOS 权限问题-selinux小结](https://blog.csdn.net/mjlfto/article/details/80547974)


<br />
<br />
<br />

