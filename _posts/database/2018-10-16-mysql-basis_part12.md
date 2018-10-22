---
layout: post
title: MySQL数据库备份与恢复(二）
tags:
- database
categories: database
description: MySQL数据库备份与恢复
---


本章我们主要讲述一下使用```mysqldump```来做备份，以及```使用binlog来做基于时间点的增量恢复```.之后还会简单介绍一下```MyISAM```表的维护会恢复。


<!-- more -->

## 1. 使用mysqldump来做备份
本章会介绍如何使用```mysqldump```来产生dump文件，并且如何来重新加载dump文件。dump文件主要有如下方面的用途：

* 作为备份，用于在数据丢失时来恢复数据

* 作为建立复制从机(replication slave)的源数据

* 作为测试用的源数据：1） 作为源数据库的一份拷贝，这样可以使得你在使用的时候并不会破坏源数据； 2） 测试一些MySQL潜在的更新兼容性问题

根据是```mysqldump```在运行时是否添加了```--tab```选项，```mysqldump```会产生两种不同类型的输出：

* 在没有添加```--tab```选项时，mysqldump会SQL语句写到标准输出当中。输出包含了相应对象(databases、tables、stored routines等）的创建语句(CREATE statements)以及表的插入语句(INSERT statements)。标准输出可以被保存到一个文件当中，并在后续通过```mysql```来重新恢复dump出来的对象。我们可以通过其他的一些选项来修正所导出的SQL语句的格式，并且控制导出哪些对象数据。

* 在添加了```--tab```选项后，对每一个dump出的表，mysqldump会产生两个输出文件。其中一个文件作为```tab-delimited text```(即表的数据文件，表的每一行对应与输出文件的一行），存放于输出目录中，名称为```tbl_name.txt```。另外，还会导出一个创建表的语句(CREATE TABLE statements)的文件，存放于输出目录中，名称为```tbl_name.sql```。

### 1.1 使用mysqldump以SQL格式导出数据
本节会描述如何使用```mysqldump```来创建SQL格式的dump文件。关于如何加载这种类型的dump文件，请参看下一节：```重新加载SQL格式的备份```。

默认情况下，mysqldump会以SQL格式将相应的信息写到标准输出中。你也可以保存到一个文件中：
{% highlight string %}
# mysqldump [arguments] > file_name
{% endhighlight %}
如果要dump出所有的数据库，可以加上```--all-databases```选项：
{% highlight string %}
# mysqldump --all-databases > dump.sql
{% endhighlight %}

如果要dump出指定的数据库，则直接在```--databases```选项后面指定相应的数据库：
{% highlight string %}
# mysqldump --databases db1 db2 db3 > dump.sql
{% endhighlight %}

```--databases```选项会导致该选项后的所有```names```都被当做数据库名。而在没有该选项的情况下，```mysqldump```会将第一个作为数据库名，后续的名称作为是表名。

在添加了```--all-databases```或者```--databases```选项之后，mysqldump对每一个数据库会将```CREATE DATABASE```以及```USE```这样的SQL语句写入到输出中。这确保了当在dump文件重新加载时，假如对应的数据库不存在时就会重新创建该数据库，并且将相应的数据重新加载回原来的数据库。假如你想要在dump文件重新创建数据库之前强制删除原有的数据库，那么你可以使用```--add-drop-database```选项。在这种情况下，mysqldump会在创建数据库语句```CREATE DATABASE```之前加上删除数据库的语句```DROP DATABASE```。

如果我们只需要导出一个数据库，那么可以通过如下的方式在命令行指定：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --databases test > dump.sql
{% endhighlight %}
另外对于单个数据库的情况下，我们也可以忽略```--databases```选项：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 test > dump.sql
{% endhighlight %}

上面两条导出语句的区别是， 在未指定```--databases```选项时，dump文件并不会包含```CREATE DATABASE```和```USE```这样的SQL语句，这就意味着：

* 当你重新加载dump文件时，你必须指定一个默认的数据库名，使得mysql服务器知道将数据加载到哪个数据库；

* 在加载时，你可以指定一个与源数据库名不同的一个名称，这意味着你可以将数据加载到一个不同的数据库；

* 假如在重新加载时，数据库不存在，那么必须首先创建好对应的数据库；

* 因为输出的dump文件并不包含```CREATE DATABASE```语句，因此```--add-drop-database```选项并不起作用。假如使用该选项的话，也并不会产生```DROP DATABASE```语句；

<br />
如果只想从一个数据库中dump出特定的表，可以通过类似于如下的命令在数据库名后面指定表名：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 test t1 t3 t7 > dump.sql
{% endhighlight %}

### 1.2 重新加载SQL格式备份
要加载通过```mysqldump```导出的包含SQL语句的dump文件，可以使用mysql客户端来完成。假如dump文件是通过指定了```--all-databases```以及```--database```选项的mysqldump来导出的，那么该dump文件包含了```CREATE DATABASE```以及```USE```这样的SQL语句，因此我们在导入时并不需要指定一个默认的数据库：
{% highlight string %}
# mysql -uroot -ptestAa@123 < dump.sql
{% endhighlight %}

另外，如果我们在MySQL中的话，可以通过```source```命令来执行：
{% highlight string %}
mysql> source dump.sql
{% endhighlight %}

<br />
假如导出的文件只是```单个```的数据库，并不包含```CREATE DATABASE```以及```USE```这样的SQL语句，那么如果有必要的话我们可以先通过如下的命令创建数据库：
{% highlight string %}
# mysqladmin -uroot -ptestAa@123 create db1
{% endhighlight %}

然后在导入dump数据时指定对应的数据库名：
{% highlight string %}
# mysql db1 < dump.sql
{% endhighlight %}

另外，如果在mysql中的话，可以先创建数据库，然后选择该数据库作为默认的数据库，接着再加载dump文件;
{% highlight string %}
mysql> CREATE DATABASE IF NOT EXISTS db1;
mysql> USE db1;
mysql> source dump.sql
{% endhighlight %}


### 1.3 使用mysqldump导出Delimited-Text格式数据
本节我们会介绍如何使用```mysqldump```来创建```delimited-text```格式的dump文件。关于如何加载该格式的dump文件，请参看下一节```加载Delimited-Text格式备份```.

假如在调用```mysqldump```时指定了```--tab=dir_name```选项，那么其将会使用```dir_name```作为输出目录，并分别对数据库中的每一个表dump出两个文件。并且用```表名```来作为输出文件的```basename```。例如对于名称为```t1```的表，则导出时产生的两个文件分别为```t1.sql```和```t1.txt```。其中```.sql```文件包含了创建表的语句(```CREATE TABLE``` statements)，```.txt```文件包含了表的数据，文件中的每一行对应于表中数据的每一行：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --tab=/var/lib/mysql-files/ test
mysqldump: [Warning] Using a password on the command line interface can be insecure.

# ls /var/lib/mysql-files/ -al
total 24
drwxr-x---.  2 mysql mysql  124 Oct 22 14:12 .
drwxr-xr-x. 58 root  root  4096 Oct 22 11:02 ..
-rw-r--r--.  1 root  root  1534 Oct 22 14:12 course.sql
-rw-rw-rw-.  1 mysql mysql  155 Oct 22 14:12 course.txt
-rw-r--r--.  1 root  root  1547 Oct 22 14:12 runoob_tbl.sql
-rw-rw-rw-.  1 mysql mysql    0 Oct 22 14:12 runoob_tbl.txt
-rw-r--r--.  1 root  root  1360 Oct 22 14:12 student.sql
-rw-rw-rw-.  1 mysql mysql  101 Oct 22 14:12 student.txt


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

-- Dump completed on 2018-10-22 14:12:25


# cat /var/lib/mysql-files/course.txt
1       MySQL从入门到精通       1001
2       爱情与婚姻      1002
3       Java从入门到放弃        1003
4       商务礼仪        1004
5       表演的艺术      1005
6       民法    1006
7       民法    1001
{% endhighlight %}

包含表数据的```.txt```文件是由```MySQL Server```来进行写入的，因此其文件```所有者```是运行```MySQL Server```的系统账户，请参看上面```course.txt```文件的所有者为```mysql```。MySQL Server使用```SELECT ... INFO OUTFILE```来将对应的数据写入文件，因此必须要有相应的操作权限（如果当前导出的文件在指定目录已经存在，那么将会报错）。

另外，MySQL Server还会将创建表的语句发送给```mysqldump```，这些信息会由mysqldump写入到```.sql```文件中。因此这些文件的所有者是执行```mysqldump```时所采用的账户。

在使用```--tab```选项时最好是只dump本地的```MySQL Server```。假如你用于dump一个远程server的话，则```--tab```目录必须要在远程和本地均存在。其中```.txt```文件会写入到```MySQL Server```宿主机一侧的对应目录(即远程目录），而```.sql```文件会写入到调用```mysqldump```宿主机一侧。

对于```mysqldump --tab```，MySQL Server默认会将表数据写入到```.txt```文件中，表中的每一行数据对应于导出文件中的每一行数据，一行中列之间采用```tab```键值来分割，并且每一列的值并未在双引号中。为了使得在导出时可以使用不同的格式，mysqldump支持如下的一些选项：

* ```--fields-terminated-by=str```： 用于指定列之间的分隔符，默认是```tab```键值

* ```--fields-enclosed-by=char```: The character within which to enclose column values (default: no character).

* ```--fields-optionally-enclosed-by=char```: The character within which to enclose non-numeric column values (default: no character).

* ```--fields-escaped-by=char```: 用于指定使用哪个字符来转义特定的字符（默认并不进行转义）

* ```--lines-terminated-by=str```: 用于指定换行符(默认是： ```\n```)

依赖于你对这些选项所指定的值，有必要在命令行上为命令解释器```quota```或```escape```一些值。另外，一般通过```十六进制```格式来指定这些选项的值。假设我们需要使用mysqldump来将导出的数据的每一列包含在```双引号```中，那么就可以使用```--fields-enclosed-by```选项。但是该字符对于某一些命令行解释器来说有特定的含义，因此我们可能必须特别处理。例如，在Unix上，你可以通过如下方式指定为每一列的值加上```双引号```：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --tab=/var/lib/mysql-files/ --fields-enclosed-by='"' test
{% endhighlight %}
对于在任何平台上，都可以通过```hex```格式来进行指定：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --tab=/var/lib/mysql-files/ --fields-enclosed-by=0x22 test
{% endhighlight %}

通常情况下，可以同时使用多个选项来格式化导出的数据。例如，以```逗号```来分割列，以```\r\n```（carriage-return/newline)来分割行。因此我们可以使用如下：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --tab=/tmp --fields-terminated-by=, --fields-enclosed-by='"' --lines-terminated-by=0x0d0a db1
{% endhighlight %}

在你导出数据时指定了相应的选项来格式化导出数据，那么在你后续进行导入时也需要指定同样的选项，这样以确保对dump文件内容的正确解释。

### 1.4 加载Delimited-Text格式备份

对于使用```mysqldump --tab```生成的备份，每个表都对应于两个文件： ```.sql```文件包含了创建表的SQL语句```CREATE TABLE```， ```.txt```文件包含了表的数据文件。要加载一个表，首先我们进入对应的存放这些导出文件的目录，然后使用mysql来处理```.sql```文件以创建一个空表，接着再使用```mysqlimport```来将数据导入到表中。
{% highlight string %}
# mysql -uroot -ptestAa@123 db1 < t1.sql
# mysqlimport -uroot -ptestAa@123 db1 t1.txt
{% endhighlight %}

另外一种使用```mysqlimport```来加载数据文件的方法就是在MySQL中使用```LOAD DATA INFILE```语句：
{% highlight string %}
mysql> USE db1;
mysql> LOAD DATA INFILE 't1.txt' INTO TABLE t1;
{% endhighlight %}

假如你在使用mysqldump导出数据时指定了相应的格式化选项，那么在使用```mysqlimport```或者```LOAD DATA INFILE```导入数据时也必须使用同样的选项，以确保可以对导出的表数据进行正确的解释：
{% highlight string %}
# mysqlimport -uroot -ptestAa@123 --fields-terminated-by=, --fields-enclosed-by='"' --lines-terminated-by=0x0d0a db1 t1.txt
{% endhighlight %}
或者：
{% highlight string %}
mysql> USE db1;
mysql> LOAD DATA INFILE 't1.txt' INTO TABLE t1
FIELDS TERMINATED BY ',' FIELDS ENCLOSED BY '"'
LINES TERMINATED BY '\r\n';
{% endhighlight %}


### 1.5 mysqldump使用提示
本节将会介绍一下使用mysqlump来解决一些特定的问题：

* 如何拷贝一个数据库

* 如何将一个数据库从一个服务器拷贝到另外一个服务器

* 如何dump出```stored programs```(存储过程、函数、触发器、事件）

* 如何分别dump出```definitions```和```data```

**1) 拷贝数据库**
{% highlight string %}
# mysqldump -uroot -ptestAa@123 db1 > dump.sql
# mysqladmin -uroot -ptestAa@123 create db2
# mysql -uroot -ptestAa@123 db2 < dump.sql
{% endhighlight %}

在这里我们执行```mysqldump```时不能指定```--databases```选项，因为这会导致```USED db1```这样的语句也存在于dump文件中，这样就会覆盖我们在命令行指定要导入```db2```中。

**2） 将数据库从一个服务器拷贝到另一个服务器**

在```Server1```上执行：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --databases db1 > dump.sql
{% endhighlight %}

然后在```Server2```上执行：
{% highlight string %}
# mysql -uroot -ptestAa@123 < dump.sql
{% endhighlight %}

这里在使用```mysqldump```时指定```--databases```选项，使得在导出的dump文件中会包含```CREATE DATABASE```及```USE```这样的SQL语句。我们后续在重新加载数据时，假如没有对应的数据库则会自动的进行创建，并将数据加载到该数据库。

<br />
另外，你也可以在执行```mysqldump```时忽略```--databases```选项。如果这样的话，在后续```Server2```上导入时有必要的话你必须先创建好数据库，然后再将该dump文件导入到该指定的数据库。

在```Server1```上执行：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 db1 > dump.sql
{% endhighlight %}
然后在```Server2```上执行：
{% highlight string %}
# mysqladmin -uroot -ptestAa@123 create db1
# mysqldump -uroot -ptestAa@123 mysql db1 < dump.sql
{% endhighlight %}

在这种情况下，你可以指定导入到一个不同的数据库。因此在执行```mysqldump```时忽略```--databases```选项可以使得你将数据库从一个数据库导出，然后再导入到另外一个不同的数据库。

**3) Dumping Stored Programs**

可以有多个选项控制```mysqldump```如何处理```stored programs```(储存过程与函数、触发器、事件）：

* ```--events```: Dump Event Scheduler event

* ```--routines```: Dump出存储过程和函数

* ```--triggers```: 为表dump出触发器

默认情况下```--triggers```是启用的，因此当表被dump出时，通常都包含该表对应的触发器。其他的选项在默认情况下是被禁用的，如果要dump出所对应的对象时，必须显示的指定这些选项。如果要显示的禁止这些选项，可以使用它们的skip形式: ```--skip-events```、```--skip-routines```或者```--skip-triggers```。

**4) 分别Dump出表的定义及数据**

mysqldump的```--no-data```选项用于指示不要dump出表的数据，这样就使得在导出的dump文件只包含创建表的语句。相反，选项```--no-create-info```用于告诉```mysqldump```抑制输出创建表的语句(```CREATE``` statements)，这样就会使得只导出表数据。

例如，下面我们分别导出```test```数据库的```表定义```与```表数据```:
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --no-data test > dump-defs.sql
# mysqldump -uroot -ptestAa@123 --no-create-info test > dump-data.sql
{% endhighlight %}

我们可以在导出```definition```时指定```--routines```或者```--events```选项，这样就会同时导出相应的```存储过程及函数```定义、```event```定义:
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --no-data --routines --events test > dump-defs.sql
{% endhighlight %}

**5) 使用mysqldump来测试MySQL upgrade兼容性**


当我们需要对```MySQL```版本进行更新的时候，需要慎重的考虑单独的安装一个比当前MySQL更新的版本。然后你可以从当前生产环境的Server中dump出数据库及对象定义，然后将dump文件导入到一个更新版本的MySQL中，以检测是否工作正常（注： 这也适合MySQL降级）。

在MySQL生产环境中执行：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --all-databases --no-data --routines --events > dump-defs.sql
{% endhighlight %}
在更新的MySQL服务器中执行：
{% highlight string %}
# mysql -uroot -ptestAa@123 < dump-defs.sql
{% endhighlight %}
因为上面我们并未导出相应的表数据，因此在执行时可以很快的处理完成。这使得你可以在不加载数据的情况下较快的找出潜在的不兼容性。我们需要仔细的检查在导入dump文件时的相关警告及错误信息。

在你已经检查相关```definition```处理正常之后，我们可以dump出数据库的表数据，然后再加载到更新的MySQL Server中。

生产环境中导出数据：
{% highlight string %}
# mysqldump -uroot -ptestAa@123 --all-databases --no-create-info > dump-data.sql
{% endhighlight %}
在更新服务器上执行：
{% highlight string %}
# mysql -uroot -ptestAa@123 < dump-data.sql
{% endhighlight %}

之后我们就可以检查表的相关数据是否正常，并做一些查询测试。


## 2. 使用binlog来做基于时间点的增量恢复
```Point-in-time```恢复涉及到将数据恢复到某一个指定的时间点。一般情况下是在我们做完全量恢复之后，然后再做本类型的恢复（全量备份有多种方法，请参看上一章```数据库备份方法```）。基于时间点的恢复使得我们在做完全量恢复之后，可以再对后续的更改进行恢复。
<pre>
注意： 
</pre>















<br />
<br />
**[参看]**:

1. [学会用各种姿势备份MySQL数据库](http://www.cnblogs.com/liangshaoye/p/5464794.html)

2. [Mysql Binlog三种格式介绍及分析](https://www.cnblogs.com/itcomputer/articles/5005602.html)




<br />
<br />
<br />

