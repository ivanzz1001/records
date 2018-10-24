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
注意： 这里的很多例子都是用'mysql'客户端来处理'mysqlbinlog'产生bin log输出。假如你的binlog包含了'\0'(NULL)字符的话，则该
输出并不能被mysql进行解析，除非指定了'--binary-mode'选项。
</pre>

```Point-in-time```恢复是基于如下规则的：

* 基于时间点的恢复所需要的源信息是```全量备份数据```及全量备份所做的增量备份binlog文件。因此MySQL在启动时必须要指定```--bin-log```选项以启用binlog功能。

为了从binlog恢复数据，首先必须要知道当前binlog文件的名称及存放位置。默认情况下，MySQL Server会在数据目录创建binlog文件，但是也可以通过使用```--bin-log```选项来指定一个不同的位置。要查看所有的binlog文件，可以使用如下的语句：
{% highlight string %}
mysql> SHOW BINARY LOGS;
{% endhighlight %}

要获得当前binlog文件的名称，执行如下的语句：
{% highlight string %}
mysql> SHOW MASTER STATUS;
{% endhighlight %}

* mysqlbinlog工具会将二进制格式的binlog文件中的事件转换成文本形式，这些文本可以被执行及查看。```mysqlbinlog```有一些选项，可以基于事件发生的```时间```或者事件在binlog中的位置来选择binlog中的某一些sections.

* 执行binlog中的事件可以导致数据的修改被重做一次。这就使得可以恢复数据在某一个时间段内的修改。要执行binlog中的事务，我们可以使用mysql来处理```mysqlbinlog```的输出
{% highlight string %}
# mysqlbinlog binlog_files | mysql -u root -ptestAa@123
{% endhighlight %}

* 当你需要查看事件发生的时间或者在binlog中的偏移，并以此来决定执行binlog中的部分事件时，能够方便的查看binlog是很重要的。要查看binlog中的事件，我们可以通过如下的命令来完成
{% highlight string %}
# mysqlbinlog binlog_files | more
{% endhighlight %}

另外，如果要将输出保存到文件中，然后在一个文本编辑器中查看到额话：
{% highlight string %}
# mysqlbinlog binlog_files > tmpfile
# ... edit tmpfile ...
{% endhighlight %}


* 将mysqlbinlog处理后的输出保存到一个文件中通常是很有用的，我们可以预先对对一些事件进行处理，例如删除里面的```DROP DATABASE```语句。也可以删除任何你不想执行的语句。在修改完成之后，执行如下语句
{% highlight string %}
# mysql -uroot -ptestAa@123 < tmpfile
{% endhighlight %}


<br />
假如你有超过一个binlog文件要在MySQL Server上执行的话，安全的方法是只使用一个连接来处理所有的这些binlog。下面展示的一个例子就不是很安全：
{% highlight string %}
# mysqlbinlog binlog.000001 | mysql -u root -p # DANGER!!
# mysqlbinlog binlog.000002 | mysql -u root -p # DANGER!!
{% endhighlight %}

以上面的方法通过使用连接到MySQL Server的不同连接来处理binlog可能会产生一些问题： 假如第一个log文件包含了```CREATE TEMPORARY TABLE```语句，而第一个binlog包含了使用该临时表的语句。当第一个binlog由mysql处理完成之后，MySQL Server会删除掉该临时表； 而当MySQL处理第二个binlog时尝试使用该临时表，则MySQL 服务器会报```unknown table```这样的错误。

要避免这样的问题，使用一个连接来执行所有你想处理的binlog文件。例如：
{% highlight string %}
# mysqlbinlog binlog.000001 binlog.000002 | mysql -u root -ptestAa@123
{% endhighlight %}
另外一种处理方法就是将所有的日志写入到一个单独的文件，然后再处理该文件：
{% highlight string %}
# mysqlbinlog binlog.000001 > /tmp/statements.sql
# mysqlbinlog binlog.000002 >> /tmp/statements.sql
# mysql -u root -ptestAa@123 -e "source /tmp/statements.sql"
{% endhighlight %}

当使用```mysqlbinlog```导出一个包含有GTIDs的binlog文件时，请添加```--skip-gtids```选项。例如：
{% highlight string %}
# mysqlbinlog --skip-gtids binlog.000001 > /tmp/dump.sql
# mysqlbinlog --skip-gtids binlog.000002 >> /tmp/dump.sql
# mysql -u root -ptestAa@123 -e "source /tmp/dump.sql"
{% endhighlight %}


### 2.1 使用Event时间来做Point-in-Time恢复
为了指明恢复的开始时间与结束时间，可以为```mysqlbinlog```指定```--start-datetime```和```--stop-datetime```选项（格式为DATETIME)。例如，假设在2005年4月20日10:00 a.m.执行了一条SQL语句用于将一个表删除，为了恢复表和数据，可以先恢复前一天晚上的全量备份，然后再执行如下的命令做增量恢复：
{% highlight string %}
# mysqlbinlog --stop-datetime="2005-04-20 9:59:59" \
/var/log/mysql/bin.123456 | mysql -u root -ptestAa@123
{% endhighlight %}

该命令会将数据恢复到```--stop-datetime```所指定的时刻。假如在几小时之后你并未检测到相应的错误，那么你可能会想要继续恢复后续被改变的数据。基于此，我们可以再次运行```mysqlbinlog```,并指定从哪个时间点开始恢复：
{% highlight string %}
# mysqlbinlog --start-datetime="2005-04-20 10:01:00" \
/var/log/mysql/bin.123456 | mysql -u root -ptestAa@123
{% endhighlight %}

在上面的语句中，从10:01 a.m.之后所记录的SQL语句都会被重复执行。这样我们通过一次全量恢复，两次增量恢复就将数据恢复到```[-, 2005-04-20 9:59:59] ∪ [2005-04-20 10:01:00,-]```。

使用这种方法可以做基于时间点的恢复，因此你首先必须要确定某一个事件发生的确切时间点。如果只想查看binlog的内容，而并不需要执行，可以使用如下的命令：
{% highlight string %}
# mysqlbinlog /var/log/mysql/bin.123456 > /tmp/mysql_restore.sql
{% endhighlight %}
然后我们可以通过检查```mysql_restore.sql```来找出相应的信息。

<pre>
说明： 假如在某一个时间点同时有多个事件发生，则mysqlbinlog并不能通过时间点来将某个事件排除。
</pre>


### 2.2 使用事件偏移来做基于时间点的恢复
我们可以通过指定```--start-position```与```--stop-position```来做基于log位置的恢复。这与```基于Event时间的恢复```类似，只是这里指定的是```日志的偏移位置```而不是```日期```。使用偏移位置使得你可以更精确的指定恢复日志的哪一部分，特别是当有很多的事务发生在同一时刻这样一种情况。为了查找到当前binlog中我们不想执行的某个事务的偏移位置，我们可以在该事务发生的时间点附近多次执行如下命令，从而确切的查找到该事务的偏移：
{% highlight string %}
# mysqlbinlog --start-datetime="2005-04-20 9:55:00" \
--stop-datetime="2005-04-20 10:05:00" \
/var/log/mysql/bin.123456 > /tmp/mysql_restore.sql
{% endhighlight %}
该命令会在```/tmp```目录创建一个小的文本文件，其包含了在```执行删除```附近的SQL语句。打开该文件，找出我们并不想执行的那条语句（删除语句），然后再找出该语句在binlog中的偏移，之后在我们恢复时就可以将该值指定为```--stop-position```选项的值。通常位置偏移是在```log_pos```标签后的一个值。在做完前面的全量恢复之后，我们就可以再做基于```偏移位置```的增量恢复。例如：
{% highlight string %}
# mysqlbinlog --stop-position=368312 /var/log/mysql/bin.123456 \
| mysql -u root -ptestAa@123

# mysqlbinlog --start-position=368315 /var/log/mysql/bin.123456 \
| mysql -u root -ptestAa@123
{% endhighlight %}
上面第一条命令会恢复所有的事务，直到给定```stop position```。第二个命令会恢复从```start position```开始直到binlog结尾的所有事务。因为```mysqlbinlog```的输出会在每条SQL语句之前包含```SET TIMESTAMP```语句，因此恢复的数据及相关的MySQL日志将会反映每一个事务的原始执行时刻。



## 3. MyISAM表的维护及崩溃恢复
本节主要会介绍如何使用```myisamchk```工具来检查或修复```MyISAM```表（```.MYD```用于存储MyISAM表的数据，```.MYI```用于存储MyISAM表数据的相关索引）。你可以使用```myisamck```来进行检查、修复或优化数据库表。下面的一些章节将会描述如何进行这些操作，以及如何建立数据库表的维护计划。

尽管使用```myisamck```来修复数据库表是很安全的，但在实际进行修复或任何维护操作之前最好还是做好备份，因为这可能会导致对表做很大的改变。

影响索引的```myisamck```操作可能会导致```MyISAM FULLTEXT```索引被重建，而使用```full-text```参数重建后的索引可能与MySQL Server当前的值不兼容。为了避免这个问题，请参看```myisamck常用选项```。

MyISAM表的维护也可以通过直接使用SQL语句来完成，这与使用```myisamchk```是相似的：

* 要检查```MyISAM```表，使用CHECK TABLE;

* 要修复```MyISAM```表，使用REPAIR TABLE;

* 要优化```MyISAM```表，使用OPTIMIZE TABLE;

* 要分析```MyISAM```表，使用ANALYZE TABLE;

这些语句可以被直接的使用，也可以通过```mysqlcheck```客户端程序来使用。```myisamck```可以通过自动的执行这些语句来完成工作。在使用```myisamck```时，你必须确该表当前并未在使用，这样就不会出现在表修复期间再夹杂其他的干扰操作。

### 3.1 使用myisamck来进行崩溃恢复

假如你使用```禁止外部锁```模式（这是默认情况）运行mysqld，那么当mysqld正在使用同一个Table时你并不能依赖使用```myisamck```来检查该表。假如你能够确保在运行```myisamchk```时并没有人会通过mysqld来访问该表时，你只需要在对表进行检查之前执行```mysqladmin flush-tables```。假如你并不能保证无人访问，那么在你对表进行检查时，必须首先关闭掉```mysqld```。假如你使用```myisamchk```来检查当前```mysqld```正在更新的表，那么你可能会收到```表被损坏```这样的警告信息，而实际上很可能没有被损坏。

假如MySQL Server在运行时启用了```外部锁```，则你可以使用```myisamck```在任何时间对表进行检查。在这种情况下，假如MySQL服务器尝试修改一个当前正被```myisamchk```使用的表，则服务器将会等待```myisamck```处理完成之后才继续。

假如你使用```myisamchk```来修复或者优化表，则你必须时刻确保```mysqld```服务并未使用该表。假如你并未停止```mysqld```，则你必须至少要执行```mysqladmin flush-tables```, 然后才能执行```myisamchk```。假如```myisamchk```以及```mysqld```同时访问该表时，可能会损坏该表。

当在进行崩溃恢复时，很重要的一点就是理解数据库中每一个```MyISAM```表```tbl_name```都对应着数据库目录下的三个文件：

|      FILE     |        Purpose                                                  | 
|:-------------:|:----------------------------------------------------------------|
| tbl_name.frm  |Definition (format) File                                         |
| tbl_name.MYD  |Data file                                                        |
| tbl_name.MYI  |Index file                                                       |

如上三种文件的每一种都可能会遇到很多不同类型的损坏，但是通常问题是发生在```data file```以及```index file```。

```myisamchk```在修复时会通过一行一行的创建```.MYD```数据文件的拷贝来进行的。在修复完成之后，其就会删除原来老的```.MYD```文件，并将新创建的文件命名回原来老文件的名称。假如在使用```myisamck```时指定了```--quick```选项，则并不会创建一个临时的```.MYD```文件，而是直接假定原来的```.MYD```文件是完好的，然后只会重新再产生一个新的索引文件。这通常是安全的，因为```myisam```会自动的检测```.MYD```文件是否被损坏，假如被损坏的话则直接退出。你也可以在执行```myisamchk```时指定两次```--quick```选项。在这种情况下，```myisamchk```在遇到一些错误时（例如```duplicate-key```错误)就不会直接退出，而是通过尝试修改```.MYD```数据文件来解决这些问题。通常情况下，指定两次```--quick```选项只会在当你只剩下少量空闲硬盘空间的情况下才会使用。但是在运行```myisamck```之前最好还是需要做好备份。


### 3.2 如何检查MyISAM表的错误
要检查一个```MyISAM```表，可以使用如下的命令：

* ```myisamchk tbl_name```: 这通常可以找出```99.99%```的错误。但是其并不能找出只涉及到数据文件损坏的错误(极低概率发生）。假如你想要检查一个数据库表，通常情况你可以在运行```myisamchk```不指定任何选项，或者只指定一个```-s```选项。

* ```myisamchk -m tbl_name```:  这通常可以找出```99.999%```的错误。首先会检查所有的索引入口以找出相应的错误，然后再通过索引入口读取所有的行数据。```myisamchk```会计算所有行的```key```值的checksum，然后与索引中的key的checksum进行比较。

* ```myisamchk -e tbl_name```: 会执行一个完全的数据检查（```-e```表示```extended check```)。```myisamchk```会检查索引中的每一个key，然后看其是否确实指向正确的行数据。这在检查一些大表且具有多个索引的情况下，可能会耗费比较长一段时间。通常,```myisamchk```在检测到第一个错误的时候就会停止退出。假如你想要获得更多的信息，可以加上```-v```(verbose)选项。这会导致```myisamchk```继续进行检测，知道检测到多达20个错误为止。


* ```myisamchk -e -i tbl_name```: 类似于前一个命令，只是这里的```-i```选项会告诉```myisamchk```以打印一些额外的统计信息。

<br />
在大多数情况下，一个简单的```myisamchk```命令（不添加任何启动选项）就足以检查一个表了。

### 3.3 如何修复MyISAM表
在本节会讨论如何使用```myisamchk```来修复```MyISAM```表（扩展名为```.MYI```和```.MYD```)。当然你也可以使用```CHECK TABLE```和```REPAIR TABLE```这样的SQL语句来检测和修复```MyISAM```表。

```MyISAM```表受到损坏的征兆包括查询时异常崩溃，以及如下一些易观察到的错误：

* ```tbl_name.frm``` is locked against change

* 并不能找到```tbl_name.MYI```

* Unexpected end of file

* Record file is crashed

* Got error ```nnn``` from table handler

要想获得更多关于错误的详细信息，可以在命令行执行```perror nnn```,这里```nnn```表示的是错误编码。下面的例子显示了如何使用```perror```来找出一些经常用的错误号所指示的问题：
{% highlight string %}
# perror 126 127 132 134 135 136 141 144 145
OS error code 126:  Required key not available
MySQL error code 126: Index file is crashed
OS error code 127:  Key has expired
MySQL error code 127: Record file is crashed
OS error code 132:  Operation not possible due to RF-kill
MySQL error code 132: Old database file
MySQL error code 134: Record was already deleted (or record file crashed)
MySQL error code 135: No more room in record file
MySQL error code 136: No more room in index file
MySQL error code 141: Duplicate unique key or constraint on write or update
MySQL error code 144: Table is crashed and last repair failed
MySQL error code 145: Table was marked as crashed and should be repaired
{% endhighlight %}
```Error 135```(record文件并没有更多的空间）以及```Error 136```(index文件并没有更多的空间）一般并不能够通过简单的修复就可以好的。在这种情况下， 你必须使用```ALTER TABLE```来增加```MAX_ROWS```以及```AVG_ROW_LENGTH```这两个表选项的值：
{% highlight string %}
ALTER TABLE tbl_name MAX_ROWS=xxx AVG_ROW_LENGTH=yyy;
{% endhighlight %}
对于其他的错误，你必须对表进行修复。```myisamchk```通常可以检测并修复这些错误。

```myisamchk```的修复过程涉及到4个步骤。首先在开始进行修复之前，进入到数据库数据的存放目录，并检查相应表文件的权限信息。在Unix上，请确保启动```mysqld```的用户具有读取这些文件的权限（并且你也需要具有访问这些文件的权限）。假如你需要对这些文件进行修改的话，还需要具有该文件的写权限。

假如你想要通过命令行的方式来修复一个```MyISAM```表，则你必须首先停止```mysqld```服务器。需要注意的是，当你在远程执行```mysqladmin shutdown```命令时，```mysqld```在```mysqladmin```返回之后可能还会运行一小段时间，直到所有的SQL语句处理完成且所有的索引的修改已经flush到了硬盘。

如下是修复的一个步骤：

**1）检查MyISAM表**

首先运行```myisamchk *.MYI```或者```myisamchk -e *.MYI```，假如想抑制一些非必要的信息，则可以使用```-s```选项。假如当前```mysqld```已经停止，你可以使用```--update-state```选项以告诉```myisamchk```将表标志为```checked```。

你只需要对那些使用```myisamchk```检查时检测出错误的表进行修复。对于这些表，请执行下面的```步骤2```

假如你在使用```myisamchk```进行检查时，遇到了一些```unexpected errors```（比如```out of memory```错误)，或者```myisamchk```崩溃，那么请执行下面的```步骤3```。

**2) 简单安全修复(Easy safe repair)**

首先尝试```myisamchk -r -q tbl_name```(这里```-r -q```表示```quick recover mode```)。该命令会尝试修复索引文件而并不会创建一个新的数据文件。假如```数据文件```包含了所有其该有的数据，并且删除链接指向了数据文件中的正确位置，则通过本命令一般可以完成对表的修复。假如通过此命令可以修复成功，那么接着可以继续修复下一个数据表； 否则，使用如下的步骤：

2.1) 在继续进行下面的步骤之前，对数据进行备份

2.2） 使用```myisamchk -r tbl_name```(这里```-r```表示```recover mode```)。该命令会从```数据文件中```移除那些不正确的行以及那些```已删除```的行，并重新创建索引

2.3) 假如前面的步骤失败的话，使用```myisamchk --safe-recover tbl_name```。安全恢复模式会使用一种古老的恢复方法来处理一些```常用恢复方法```不处理的情况，但是一般较为耗时。

<pre>
假如你想要使修复过程更加快速，那么在运行myisamchk时可以设置'sort_buffer_size'与'key_buffer_size'变量的值为当前可用内存的25%左右
</pre>

假如在修复时仍遇到一些```unexpected error```（例如```out of memory```错误），或者```myisamchk```崩溃，那么请执行下面的```步骤3```

**3) Difficult repair**

一般来说，只有在索引文件的前```16KB```块都被损坏，或者包含了一些错误的信息，又或者索引文件丢失的情况下才会执行到此步骤。在这种情况下，就有必要创建一个新的索引文件。请执行如下步骤：

3.1) 将数据移文件至一个安全的位置；

3.2) 使用表描述文件来创建一个新的空(empty)数据文件和索引文件
<pre>
# mysql db_name
</pre>

{% highlight string %}
mysql> SET autocommit=1;
mysql> TRUNCATE TABLE tbl_name;
mysql> quit
{% endhighlight %}

3.3) 拷贝原来老的数据文件到新创建的数据文件那里。然后再回到```步骤2）```，执行```myisamchk -r -q tbl_name```，然后一般就可以修复完成。

<br />
当然，你也可以使用```REPAIR TABLE tbl_name USE_FRM``` SQL语句来自动的完成整个修复过程。在进行表修复的过程中，不要执行任何其他可能产生干扰的操作。

**4) Very difficult repair**

一般只有在```.frm```描述文件也遭到破坏的情况下，你才会执行到本步骤。但是这种情况一般不会发生，因为表的描述文件在表创建之后是不会进行修改的：

4.1） 重新从备份中加载描述文件，然后返回到```步骤3）```，你也可以重新加载索引未见，然后返回奥```步骤2)```。在后一种情况下，你应该使用```myisamchk -r```来进行恢复。

4.2) 假如当前你并没有备份，但是知道该表是如何创建的，那么你可以在另外一个数据库中重新创建该表。然后从重新创建的表的存储目录拷贝```.frm```文件以及```.MYI```文件到当前崩溃的数据库目录中。这样就使得你重新获得了```表定义文件```以及```索引文件```（但是仍采用原来的```.MYD```文件)。然后再回到**步骤2)**尝试重新构造索引。



### 3.4 MyISAM表的优化
为了紧缩分散的行数据以及降低由于删除或更新行数据导致的空间浪费，可以以```recovery mode```来运行```myisamchk```:
{% highlight string %}
# myisamchk -r tbl_name
{% endhighlight %}
当然，你也可以使用```OPTIMIZE TABLE```这样的SQL语句来对表进行优化。```OPTIMIZE TABLE```会进行表的修复和key的分析，也会对索引树进行排序这样使得查看更快。在这里我们进行优化的时候，同样不要进行其他的操作以免干扰。

```myisamchk```有一系列的其他选项，你可以用于提高相关的操作性能：

* ```--analyze```或者```-a```: 进行key的分布分析。通过启用```join```优化器来提交```join```操作的性能

* ```--sort-index```或者```-S```: 对索引块进行排序，这可以使得使用索引来进行```记录定位```或者扫描表时更加快速。

* ```--sort-records=index_num```或者```-R index_num```: 根据一个给定的索引对数据行进行排序。这可以使得数据更加紧凑，并且在使用该索引做基于```范围```的```SELECT```或```ORDER BY```操作时会更加的快速。


<br />
<br />
**[参看]**:

1. [学会用各种姿势备份MySQL数据库](http://www.cnblogs.com/liangshaoye/p/5464794.html)

2. [Mysql Binlog三种格式介绍及分析](https://www.cnblogs.com/itcomputer/articles/5005602.html)




<br />
<br />
<br />

