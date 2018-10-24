---
layout: post
title: MySQL数据库备份与恢复(三）
tags:
- database
categories: database
description: MySQL数据库备份与恢复
---


本章首先讲述一下数据库的导入与导出，然后再通过相应的示例讲述一下MySQL数据库的备份。


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
CREATE TABLE `t1` (
  `i` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`i`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

 CREATE TABLE `t2` (
  `i` int(15) NOT NULL,
  PRIMARY KEY (`i`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
{% endhighlight %}


**2） 测试读锁**





<br />
<br />
**[参看]**:

1. [学会用各种姿势备份MySQL数据库](http://www.cnblogs.com/liangshaoye/p/5464794.html)

2. [Mysql Binlog三种格式介绍及分析](https://www.cnblogs.com/itcomputer/articles/5005602.html)

3. [MySQL 表锁以及FLUSH TABLES操作](https://blog.csdn.net/zyz511919766/article/details/49336101/)




<br />
<br />
<br />

