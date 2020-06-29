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

```SELECT ... INTO OUTFILE```语句有以下属性：

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

### 2.1 mysql命令导入

使用mysql命令导入的语法格式如下：
{% highlight string %}
# mysql -u<usr_name> -p<usr_pwd> < data.sql
{% endhighlight %}
如下是一个具体的实例：
{% highlight string %}
# mysql -uroot -ptestAa@123 test < student.sql
# mysql -uroot -ptestAa@123 < selected_db.sql
{% endhighlight %}

以上命令将备份的整个数据库```selected_db.sql```导入。

### 2.2 source命令导入

source命令导入数据库需要先登录到数据库终端：
{% highlight string %}
mysql> create database abc;      # 创建数据库
mysql> use abc;                  # 使用已创建的数据库 
mysql> set names utf8;           # 设置编码
mysql> source /home/abc/abc.sql  # 导入备份数据库
{% endhighlight %}



### 2.3 使用LOAD DATA导入数据
MySQL中提供了```LOAD DATA INFILE```语句来导入数据。以下实例中将从当前目录中读取文件```dump.txt```，该文件中的数据插入到当前数据库的```mytbl```表中：
{% highlight string %}
mysql> LOAD DATA LOCAL INFILE 'dump.txt' INTO TABLE mytbl;
{% endhighlight %}

如果指定```LOCAL```关键字，则表明从客户主机上按路径读取文件。如果没有指定，则文件在服务器上按路径读取文件。


另外，你可以明确地在```LOAD DATA```语句中指出列值的分隔符和行尾标记，但是默认是定位符和换行符。```SELECT INTO```与```LOAD DATA```这两个命令的```FIELDS```和```LINES```子句的语法是一样的，并且两个子句都是可选的。但是如果两个同时被指定，```FIELDS```子句必须出现在```LINES```子句之前。

如果用户指定一个```FIELDS```子句，它的子句(**TERMINATED BY**、**[OPTIONALLY] ENCLOSED BY**和**ESCAPED BY**)也是可选的。不过，用户必须至少指定它们中的一个：
{% highlight string %}
mysql> LOAD DATA LOCAL INFILE 'dump.txt' INTO TABLE mytbl
  -> FIELDS TERMINATED BY ':'
  -> LINES TERMINATED BY '\r\n';
{% endhighlight %}

```LOAD DATA```默认情况下是按照数据文件中列的顺序插入数据的，如果数据文件中的列与插入表中的列不一致，则需要指定列的顺序。
{% highlight string %}
LOAD DATA LOCAL INFILE 'D:\\dump.txt' INTO TABLE mytbl FIELDS TERMINATED BY ' '  LINES TERMINATED BY '\n'(uid, bucket, object, size, status, update_ts);
{% endhighlight %}

### 2.4 使用mysqlimport导入数据
```mysqlimport```客户端提供了```LOAD DATA INFILE```语句的一个命令行接口。```mysqlimport```的大多数选项直接对应```LOAD DATA INFILE```子句。从文件```dump.txt```中将数据导入到```mytbl```数据表中，可以使用以下命令：
{% highlight string %}
$ mysqlimport -u root -p --local database_name dump.txt
password *****
{% endhighlight %}

```mysqlimport```命令可以指定选项来设置指定格式，命令语句格式如下：
{% highlight string %}
$ mysqlimport -u root -p --local --fields-terminated-by=":" \
   --lines-terminated-by="\r\n"  database_name dump.txt
password *****
{% endhighlight %}
```mysqlimport```语句中使用```--columns```选项来设置列的顺序：
{% highlight string %}
$ mysqlimport -u root -p --local --columns=b,c,a \
    database_name dump.txt
password *****
{% endhighlight %}

1) **mysqlimport的常用选项介绍**
<pre>
    选项                                       功能
--------------------------------------------------------------------------------------------
-d 或者 --delete            新数据导入数据表中之前删除数据表中的所有信息
-f 或者 --force             不管是否遇到错误，mysqlimport将强制继续插入数据
-i 或者 --ignore            mysqlimport跳过或者忽略那些有相同唯一关键字的行，导入文件中的数据将被忽略
-l 或者 --lock-tables       数据被插入之前锁住表，这样就防止了你在更新数据时，用户的查询和更新受到影响
-r 或者 --replace           这个选项与-i选项的作用相反；此选项将替换表中有相同唯一关键字的记录

--fields-enclosed-by=char   指定文本文件中数据的记录是以什么括起的，很多情况下数据以双引号括起。默认的情况下数据是没有被字符括起的
--fields-terminated-by=char 指定各个数据的值之间的分隔符，在句号分割的文件中，分隔符是句号。你可以用此选项指定数据之间的分隔符。默认
                            的分割符是跳格符(Tab)
--lines-terminated-by=str   此选项指定文本文件中行与行之间数据的分割字符串或字符。默认的情况下，mysqlimport以newline为行分隔符。
                            您可以选择用一个字符串来替代一个单个的字符：一个新行或者一个回车
</pre>
```mysqlimport```命令常用的选项还有```-v```显示版本(version)，```-p```提示输入密码(password)等。


## 3. MySQL复制表

如果我们需要完全的复制MySQL的数据表，包括表的结构、索引、默认值等。如果仅仅使用```CREATE TABLE```、```SELECT```命令是无法实现的。本节将介绍如何完整的复制MySQL数据表，步骤如下：

* 使用**SHOW CREATE TABLE**命令获取创建数据表(```CREATE TABLE```)的语句，该语句包含了原数据表的结构、索引等；

* 采用上面获取到的原数据表结构创建一个新的复制表

* 如果你想要复制表的内容，你就可以使用```INSERT INTO ... SELECT```语句来实现


### 3.1 示例
尝试以下示例来复制表```runoob_tbl```:

1) **获取数据表的完整结构**
{% highlight string %}
mysql> SHOW CREATE TABLE runoob_tbl \G;
*************************** 1. row ***************************
       Table: runoob_tbl
Create Table: CREATE TABLE `runoob_tbl` (
  `runoob_id` int(11) NOT NULL auto_increment,
  `runoob_title` varchar(100) NOT NULL default '',
  `runoob_author` varchar(40) NOT NULL default '',
  `submission_date` date default NULL,
  PRIMARY KEY  (`runoob_id`),
  UNIQUE KEY `AUTHOR_INDEX` (`runoob_author`)
) ENGINE=InnoDB 
1 row in set (0.00 sec)

ERROR:
No query specified
{% endhighlight %}

2) **修改SQL语句的数据库表名，并执行SQL语句**
{% highlight string %}
mysql> CREATE TABLE `clone_tbl` (
  -> `runoob_id` int(11) NOT NULL auto_increment,
  -> `runoob_title` varchar(100) NOT NULL default '',
  -> `runoob_author` varchar(40) NOT NULL default '',
  -> `submission_date` date default NULL,
  -> PRIMARY KEY  (`runoob_id`),
  -> UNIQUE KEY `AUTHOR_INDEX` (`runoob_author`)
-> ) ENGINE=InnoDB;
Query OK, 0 rows affected (1.80 sec)
{% endhighlight %}

3) **复制表中的内容**

如下我们使用```INSERT INTO ... SELECT```语句来实现数据内容的复制：
{% highlight string %}
mysql> INSERT INTO clone_tbl (runoob_id,
    ->                        runoob_title,
    ->                        runoob_author,
    ->                        submission_date)
    -> SELECT runoob_id,runoob_title,
    ->        runoob_author,submission_date
    -> FROM runoob_tbl;
Query OK, 3 rows affected (0.07 sec)
Records: 3  Duplicates: 0  Warnings: 0
{% endhighlight %}
执行完上述步骤之后，你将完整的复制表，包括表结构以及表数据。





<br />
<br />
**[参看]**:


1. [MySQL教程](https://www.runoob.com/mysql/mysql-database-export.html)



<br />
<br />
<br />

