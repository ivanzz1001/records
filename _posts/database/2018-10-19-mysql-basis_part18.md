---
layout: post
title: MySQL数据的导入与导出
tags:
- database
categories: database
description: MySQL数据的导入与导出
---

本章介绍一下MySQL数据的导入与导出、数据库表的复制。


<!-- more -->

## 1. MySQL数据的导出
当前我们有如下数据库：
{% highlight string %}
mysql> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| sys                |
| test               |
+--------------------+
5 rows in set (0.00 sec)

mysql> use test;
Database changed
mysql> show tables;
+----------------+
| Tables_in_test |
+----------------+
| course         |
| student        |
+----------------+
2 rows in set (0.00 sec)
{% endhighlight %}


1) **使用 SELECT ... INTO OUTFILE语句导出数据**

以下实例中，我们将数据表```student```数据导出到```/tmp/student.txt```文件中：
{% highlight string %}
mysql> SELECT * FROM student INTO OUTFILE '/tmp/student.txt';
ERROR 1290 (HY000): The MySQL server is running with the --secure-file-priv option so it cannot execute this statement
{% endhighlight %}
我们在执行上面的SQL语句时出现错误，这是因为MySQL为了安全性考虑，限定了文件只能导出到指定的目录。我们通过如下命令查看能够导出的目录：
{% highlight string %}
mysql> SHOW VARIABLES LIKE "secure_file_priv";
+------------------+-----------------------+
| Variable_name    | Value                 |
+------------------+-----------------------+
| secure_file_priv | /var/lib/mysql-files/ |
+------------------+-----------------------+
1 row in set (0.14 sec)
{% endhighlight %}
```secure_file_priv```字段的值可取如下三种类型：

* 值为null: 表示禁止数据导出

* 值为某个目录（例如```/tmp/```)： 可以将数据导出到该目录

* 值为空字符串： 可以导出数据，并且没有目录限制

现在我们将```student```表中的数据导出到*/var/lib/mysql-files/*目录：
{% highlight string %}
mysql> SELECT * FROM student INTO OUTFILE '/var/lib/mysql-files/student.txt';
Query OK, 8 rows affected (0.03 sec)

# cat /var/lib/mysql-files/student.txt 
1001    ivan1001
1002    郭晋安
1003    杨怡
1004    林文龙
1005    周丽淇
1006    天堂哥
1007    欢喜哥
10000   test10000
{% endhighlight %}

你也可以通过命令选项来设置数据输出的指定格式，以下实例为导出```CSV```格式：
{% highlight string %}
mysql> SELECT * FROM student INTO OUTFILE '/var/lib/mysql-files/student-2.txt'  
    -> FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"'
    -> LINES TERMINATED BY '\n';
Query OK, 8 rows affected (0.01 sec)


# cat /var/lib/mysql-files/student-2.txt 
1001,"ivan1001"
1002,"郭晋安"
1003,"杨怡"
1004,"林文龙"
1005,"周丽淇"
1006,"天堂哥"
1007,"欢喜哥"
10000,"test10000"
{% endhighlight %}

在下面的例子中，我们生成一个文件，同时在每项数据后面加上时间戳信息。这种格式可以被许多程序所使用：
{% highlight string %}
mysql> SELECT stuid, stuname, UNIX_TIMESTAMP(now()) INTO OUTFILE '/var/lib/mysql-files/student-3.txt'
    -> FIELDS TERMINATED BY ';' OPTIONALLY ENCLOSED BY '"'
    -> LINES TERMINATED BY '\n'
    -> FROM student;
Query OK, 8 rows affected (0.03 sec)

# cat /var/lib/mysql-files/student-3.txt 
1001;"ivan1001";1561967440
1002;"郭晋安";1561967440
1003;"杨怡";1561967440
1004;"林文龙";1561967440
1005;"周丽淇";1561967440
1006;"天堂哥";1561967440
1007;"欢喜哥";1561967440
10000;"test10000";1561967440
{% endhighlight %}

*SELECT ... INTO OUTFILE*语句有以下**属性**：

* *LOAD DATA INFILE* 是 *SELECT ... INTO OUTFILE*的逆操作。为了将一个数据库表的数据写入文件，使用*SELECT ... INTO OUTFILE*; 为了将文件数据读回数据库，使用*LOAD DATA INFILE*

* *SELECT ... INTO OUTFILE*语句用于将```select```选中的语句写入到一个文件中。该文件将会被创建到服务器主机上，因此你必须拥有对该目录的写权限，才能使用此语法

* 输出不能是一个已存在的文件，防止文件数据被篡改（这主要是```安全```方面的考虑）

* 你需要一个登录服务器的账号来检索文件，否则我们取不到导出的文件（即*SELECT ... INTO OUTFILE*只能将数据导出到MySQL宿主机）

* 在Unix中，该文件被创建后是可读的，权限由MySQL服务器所拥有。这意味着，虽然你可以读取该文件，但是可能无法将其删除

2) **导出表作为原始数据**

```mysqldump```是MySQL用于转存数据库的实用程序。它主要产生一个SQL脚本，其中包含从头重新创建数据库所必需的命令```CREATE TABLE```、```INSERT```等。使用```mysqldump```导出数据需要使用```--tab```选项来指定文件导出的目录，该目录必须是可写的：

{% highlight string %}
# mysqldump -uroot -ptestAa@123 --tab=/var/lib/mysql-files/ test student course
mysqldump: [Warning] Using a password on the command line interface can be insecure.
# ls /var/lib/mysql-files/
course.sql  course.txt  student.sql  student.txt


# cat /var/lib/mysql-files/student.sql
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
-- Table structure for table `student`
--

DROP TABLE IF EXISTS `student`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `student` (
  `stuid` int(11) NOT NULL,
  `stuname` varchar(128) NOT NULL,
  PRIMARY KEY (`stuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-07-01 16:21:27

# cat /var/lib/mysql-files/student.txt
1001    ivan1001
1002    郭晋安
1003    杨怡
1004    林文龙
1005    周丽淇
1006    天堂哥
1007    欢喜哥
10000   test10000
{% endhighlight %}
注意上面导出的目录也必须是```secure_file_priv```所指定的木。另外我们看到对于一个表，会导出两个文件：一个是表的定义文件(以```.sql```结尾），另一个是表数据文件(以```.txt```结尾)。例如我们导出的```student```，就对应于```student.sql```和```student.txt```两个文件。

3） **导出SQL格式数据**

我们可以通过使用```mysqldump```命令导出SQL格式的数据到指定的文件，如下所示：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 test student > dump.txt
mysqldump: [Warning] Using a password on the command line interface can be insecure.
{% endhighlight %}
以上命令创建的文件内容如下：
{% highlight string %}
# cat dump.txt 
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
-- Table structure for table `student`
--

DROP TABLE IF EXISTS `student`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `student` (
  `stuid` int(11) NOT NULL,
  `stuname` varchar(128) NOT NULL,
  PRIMARY KEY (`stuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `student`
--

LOCK TABLES `student` WRITE;
/*!40000 ALTER TABLE `student` DISABLE KEYS */;
INSERT INTO `student` VALUES (1001,'ivan1001'),(1002,'郭晋安'),(1003,'杨怡'),(1004,'林文龙'),(1005,'周丽淇'),(1006,'天堂哥'),(1007,'欢喜哥'),(10000,'test10000');
/*!40000 ALTER TABLE `student` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-07-01 16:27:36
{% endhighlight %}
如果你需要导出整个数据库的数据，可以使用以下命令：
{% highlight string %}
# mysqldump -u root -ptestAa@123 test > database_dump.txt
{% endhighlight %}

如果需要备份所有数据库，可以使用如下命令：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --all-databases > database_dump.txt
{% endhighlight %}
上面```--all-databases```选项在```MySQL 3.23.12```及以后版本加入。该方法可以用于实现数据库的备份策略。

4) **将数据表及数据库拷贝至其他主机**

如果你需要将数据拷贝至其他的MySQL服务器上，你可以在```mysqldump```命令中指定数据库名及数据表。在源主机上执行以下命令，将数据备份至```dump.txt```文件中：
{% highlight string %}
# mysqldump -uroot -p database_name table_name > dump.txt
{% endhighlight %}
如果需要完整备份数据库，则无需使用特定的表名称。

如果你需要将备份的数据库导入到MySQL服务器中，可以使用以下命令，使用以下命令需要确认数据库已经创建：
{% highlight string %}
# mysql -uroot -p database_name < dump.txt
{% endhighlight %}

你也可以使用以下命令将导出的数据直接导入到远程的服务器上，但请确保两台服务器是相通的，是可以互相访问的：
{% highlight string %}
# mysqldump -uroot -p database_name | mysql -h other-host.com database_name
{% endhighlight %}
以上命令中使用了管道来将导出的数据导入到指定的远程主机上。

## 2. MySQL导入数据

1) **mysql命令导入**

使用mysql命令导入的语法格式如下：
{% highlight string %}
# mysql -u<usr_name> -p<usr_pwd> < data.sql
{% endhighlight %}
如下是一个具体的实例：
{% highlight string %}
# mysql -uroot -ptestAa@123 < selected_db.sql
{% endhighlight %}


<br />
<br />
**[参看]**:


1. [MySQL教程](https://www.runoob.com/mysql/mysql-database-export.html)



<br />
<br />
<br />

