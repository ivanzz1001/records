---
layout: post
title: mysql数据类型
tags:
- database
categories: database
description: mysql数据库基础
---



本文讲述一下mysql数据库的一些基础知识及相关操作。当前的数据库版本为：```5.7.22 MySQL Community Server (GPL)```


<!-- more -->

## 1. 数据库的创建与删除

* 创建数据库

登录MySQL数据库以后，我们可以通过```CREATE```命令创建数据库，语法如下：
{% highlight string %}
CREATE DATABASE <dbname> 
{% endhighlight %}

如下我们创建一个名称为```test```的数据库：
{% highlight string %}
mysql> create database test;
Query OK, 1 row affected (0.04 sec)

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
6 rows in set (0.00 sec)
{% endhighlight %}

* 删除数据库

登录MySQL数据库后，我们可以通过```DROP```命令删除数据库，语法如下：
{% highlight string %}
DROP DATABASE <dbname>
{% endhighlight %}
如下我们删除```test```数据库：
{% highlight string %}
mysql> drop database test;
Query OK, 0 rows affected (0.08 sec)
{% endhighlight %}

* 选择数据库

使用```USE```命令选择数据库：
{% highlight string %}
USE <dbname>
{% endhighlight %}


## 2. MySQL数据类型概述
MySQL数据类型整体上可以分为如下几大类：

* 数值类型

* 日期和时间类型

* 字符串类型

* 空间(Spatial)类型

这里我们只介绍前面3种，最后一种暂不做介绍。在下面```MySQL数据类型```章节中，对于数据类型的描述通常通常遵循如下规则：

* ```M```用于指定数值类型的最大显示宽度。对于浮点类型和定点类型，```M```用于指定可以存放的数字个数(精度）； 对于字符串类型，```M```用于指定最大长度。这里```M```允许的最大值依赖于数据类型。

* ```D```适用于定点数和浮点数类型，用于指定小数部分的宽度。最大的可能值为30，但是不要超过```M^2```

* ```fsp```作用于```TIME```、```DATETIME```和```TIMESTAMP```数据类型，用于指定小于```1秒```时的精度，即秒后面的小数部分的宽度。假如指定了```fsp```的话，其范围必须是[0,6]，其中0表示没有小数部分。假如省略的话，则默认精度为0

* 中括号```[]```用于指定该选项可选



### 2.1 数值类型

对于整数类型，```M```用于指定```整数部分```的显示宽度。显示宽度最大可以为255。```显示宽度```与该```数值类型```表示范围无关； 对于```浮点类型```和```定点类型```，```M```用于指定可以存放的数字个数。

<pre>
MySQL中数据类型的显示宽度:

显示宽度只是指明MySQL最大可能显示的数字个数，数值的位数小于指定宽度时会有空格填充，取决于你的设置。如果插入了大于显示
宽度的值，只要该值不超过该类型的取值范围，数值依然可以插入显示出来。

例如，我们在创建表时，可以在INT后面加入数值。请注意这个数值不代表数据的长度。例如id字段的数据类型为INT(4)，注意到后面
的数字4，这表示该数据类型指定的显示宽度，指定能够显示的数值中数字的个数，实际存储的长度还是INT的取值范围

创建表时，可以在INT后面加入数值。请注意这个数值不代表数据的长度。例如id字段的数据类型为INT(４)，注意到后面的数字4，这表示的是该数据类型指定的显示宽度，指定能够显示的数值中数字的个数，实际存储的长度还是上表中INT的取值范围-2147483648~2147483648。
</pre>

如果对于某一个数值列指定了```ZEROFILL```，则MySQL会自动的将该列添加```UNSIGNED```属性。数值类型允许添加```SIGNED```和```UNSIGNED```属性修饰，默认情况下是```SIGNED```类型。

<pre>
SERIAL类型等价于BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE;
</pre>

参看如下例子：
{% highlight string %}
mysql> CREATE TABLE test(id int PRIMARY KEY, val TINYINT(10) ZEROFILL);
Query OK, 0 rows affected (0.18 sec)

mysql> desc test;
+-------+-------------------------------+------+-----+---------+-------+
| Field | Type                          | Null | Key | Default | Extra |
+-------+-------------------------------+------+-----+---------+-------+
| id    | int(11)                       | NO   | PRI | NULL    |       |
| val   | tinyint(10) unsigned zerofill | YES  |     | NULL    |       |
+-------+-------------------------------+------+-----+---------+-------+
2 rows in set (0.02 sec)

mysql> INSERT INTO test(id,val) VALUES(1,12),(2,7),(4,101);
Query OK, 3 rows affected (0.06 sec)
Records: 3  Duplicates: 0  Warnings: 0

mysql> select * from test;
+----+------------+
| id | val        |
+----+------------+
|  1 | 0000000012 |
|  2 | 0000000007 |
|  4 | 0000000101 |
+----+------------+
3 rows in set (0.00 sec)
{% endhighlight %}
上面我们在创建表时列指定了zerofill，如果存入```12```，那么查询出来的结果就是```0000000012```，左边用```0```来填充。如果我们没有指定zerofill，默认用空格来填充。

下面我们就对各数值类型做一个详细的介绍：


**1） BIT[(M)]类型**

比特类型，其中```M```用于指定每一个值的bit位数，范围是[1,64]。缺省状态下默认值为1。

**2) TINYINT[(M)] [UNSIGNED] [ZEROFILL]**

小整数类型。其中```SIGNED```的范围为[-128,127]，```UNSIGNED```的范围为[0,255]。

**3） BOOL,BOOLEAN类型**

则两种类型等价于```TINYINT(1)```，0值被认为false，非0值被认为是true。

**4) SMALLINT[(M)] [UNSIGNED] [ZEROFILL]**

小整数类型。其中```SIGNED```的取值范围是[-32768,32767]，```UNSIGNED```的范围是[0,65535]。

**5) MEDIUMINT[(M)] [UNSIGNED] [ZEROFILL]**

中等大小整数类型。其中```SIGNED```的取值范围是[-8388608,8388607]，```UNSIGNED```的取值范围是[0,16777215]。

**6) INT [(M)] [UNSIGNED] [ZEROFILL]**

普通大小整数。其中```SIGNED```的取值范围是[-2147483648,2147483647]，```UNSIGNED```的取值范围是[0,4294967295]

**7) INTEGER[(M)] [UNSIGNED] [ZEROFILL]**

含义同```INT```。

**8) BIGINT [(M)] [UNSIGNED] [ZEROFILL]**

大整数类型，其中```SIGNED```的取值范围是[-9223372036854775808,9223372036854775807]， ```UNSIGNED```的取值范围是[0,18446744073709551615]。

```SERIAL```等价于```BIGINT UNSIGNED NOT NULL AUTO_INCREMENT UNIQUE```。

**9) DECIMAL[(M[,D])] [UNSIGNED] [ZEROFILL]**

一个折叠的定点数。其中```M```用于指定数字的总位数，```D```用于指定小数部分的位数。注意： 在计算```M```时，并不包括正负号(+/-)和小数点的长度。假如```D```是0的话，则表示没有小数部分。针对```DECIMAL```类型,```M```最大可取值为65，```D```最大可取值为30。假如```D```省略的话，默认值为0； 假如```M```省略的话，默认值为10.

<pre>
DEC[(M[,D])] [UNSIGNED] [ZEROFILL]

NUMERIC[(M[,D])] [UNSIGNED] [ZEROFILL]

FIXED[(M[,D])] [UNSIGNED] [ZEROFILL]

上面3个与DECIMAL等价，其中FIXED主要是为了兼容其他数据库。
</pre>

**10） FLOAT [(M, D)] [UNSIGNED] [ZEROFILL]**

浮点数类型，其中```SIGNED```的取值范围是[-3.402823466E+38,-1.175494351E-38]，```UNSIGNED```的取值范围是[1.175494351E-38, 3.402823466E+38]。 这些是基于IEEE标准的理论上的限制，实际的取值范围根据硬件与操作系统的不同会略小。

```M```是总的数字的位数，而```D```是小数部分的位数。假如```M```和```D```都被省略的话，则被存为硬件所允许的最大值。

**11） DOUBLE[(M,D)] [UNSIGNED] [ZEROFILL]**

类似于FLOAT。

**12） DOUBLE PRECISION[(M,D)] [UNSIGNED] [ZEROFILL], REAL[(M,D)] [UNSIGNED][ZEROFILL]**

类似于```DOUBLE```。


<br />

### 2.2 日期和时间类型

对于```MySQL	5.6.4```及以上版本，```TIME```，```DATETIME```和```TIMESTAMP```支持低于秒级的精度，最高可达到微妙级(即6个小数位)。要想定义一个带有小数部分的时间，使用语法： ```type_name(fsp)```，其中```type_name```可以为:```TIME```或```DATETIME```或```TIMESTAMP```，```fsp```用于指定精度。例如：
<pre>
CREATE TABLE t1 (t TIME(3), dt DATETIME(6));
</pre>

上面假如指定了```fsp```的值的话，那么取值范围必须是[0,6]。其中```0```表示没有小数部分（即没有低于1s的时间)，假如省略的话默认值为0.

从```MySQL 5.6.5```版本开始，所有的```TIMESTAMP```和```DATETIME```列都属于原子性操作。

下面介绍一下各```日期和时间```类型:

**1) DATE类型**

日期类型，支持的范围从```1000-01-01```至```9999-12-31```。MySQL以```YYYY-MM-DD```的格式显示```DATE```数据，但是在赋值的时候可以使用字符串或者数字。

**2) DATETIME(fsp)类型**

时间日期类型，支持的范围从```1000-01-01 00:00:00.000000```至```9999-12-31 23:59:59.999999```。MySQL以```YYYY-MM-DD HH:MM:SS[.fraction]```格式来显示```DATETIME```数据，但是在赋值的时候允许使用字符串或者数字。

从MySQL 5.6.5开始，对于```DATETIME```列允许自动初始化和更新为当前时间(通过```DEFAULT```和```ON UPDATE```，我们后面会做介绍）。

**3) TIMESTAMP(fsp)类型**

时间戳类型，范围从```1970-01-01 00:00:01.000000``` UTC至```2038-01-19 03:14:07.999999``` UTC。TIMESTAMP存储的是从```1970-01-01 00:00:00```起的一个秒数。```TIMESTAMP```并不能表示```1970-01-01 00:00:00```，这是因为表示从起始纪元开始的0秒，在MySQL中```0s```被保留以代表```0000-00-00 00:00:00```


**4） TIME(fsp)类型**

时间类型，范围从```-838:59:59.000000```至```838:59:59.000000```。 MySQL以```HH:MM:SS[.fraction]```格式显示```TIME```数据，但是在赋值的时候允许使用字符串或者数字。


**5） YEAR[(2/4)]类型**

```YEAR```类型，显示宽度可以为2或者4，默认是4位。```YEAR(2)```和```YEAR(4)```只是在显示的时候有区别，表示的范围是相同的。对于4位格式，表示的范围1901至2155（0000保留); 对于2位格式，表示的范围从70~69，表示1970年至2069年。MySQL以```YYYY```或```YY```的格式显示```YEAR```类型，但是在赋值的时候允许使用字符串或者数字。

<pre>
在时间和日期这种temporal类型中，不能使用SUM()和AVG()这样的MySQL内置函数。因为使用这些函数时需要转换成数字，这有可能会导致错误。

如果要使用，我们可以自己手动来进行转换，然后再将处理结果转换回“时间日期”类型。例如：

SELECT SEC_TO_TIME(SUM(TIME_TO_SEC(time_col))) FROM tbl_name;
SELECT FROM_DAYS(SUM(TO_DAYS(date_col))) FROM tbl_name;
</pre>

<br />

### 2.3 字符串类型

针对```CHAR```、```VARCHAR```和```TEXT```类型， MySQL会将```长度```解释成为字符的个数。

另外对于许多```string```类型数据，可以指定字符集(character set)和校对集(collation)，这适用于```CHAR```、```VARCHAR```、```TEXT```、```ENUM```和```SET```数据类型：

* ```CHARACTER SET```属性用于指定字符集，而```COLLATE```属性用于指定校对集：
<pre>
CREATE TABLE t
(
c1 VARCHAR(20) CHARACTER SET utf8,
c2 TEXT CHARACTER SET latin1 COLLATE latin1_general_cs
);
</pre>
上述创建表的语句，首先创建了一个名称为```c1```的列，所采用的字符集为```utf-8```，针对该字符集采用其默认的校对集； 接着创建了一个名称为```c2```的列， 所采用的字符集为```latin1```，针对该字符集采用```latin1_general_cs```校对集。

注意上面```CHARACTER SET```也可以写成```CHARSET```。

* 如果在创建列时指定```CHARACTER SET binary```属性，那么MySQL会将该类型转换成适当的类型：```CHAR```类型被转换成```BINARY```,```VARCHAR```类型被转换成```VARBINARY```，```TEXT```类型被转换成```BLOB```类型。而对于```ENUM```及```SET```类型则不会进行转换。假设我们有如下定义：
<pre>
CREATE TABLE t
(
c1 VARCHAR(10) CHARACTER SET binary,
c2 TEXT CHARACTER SET binary,
c3 ENUM('a','b','c') CHARACTER SET binary
);
</pre>
经过MySQL自动转换后，则变成：
{% highlight string %}
CREATE TABLE t
(
c1 VARBINARY(10),
c2 BLOB,
c3 ENUM('a','b','c') CHARACTER SET binary
);
{% endhighlight %}

* ```BINARY```属性等价于采用该表默认的字符集与该字符集对应的二进制校对集。在这种情况下，比较与排序都是基于该字符编码值来进行的。

* ```ASCII```属性是```CHARACTER SET latin1```简写

* ```UNICODE```属性是```CHARACTER SET ucs2```的简写

字符列的比较和排序都是基于该列的校对集。对于```CHAR```、```VARCHAR```、```TEXT```、```ENUM```和```SET```数据类型，你可以对该列指定```BINARY```属性或者binary校对集(_bin)，这样就会使得在比较和排序的时候使用对应的字符编码数值来作为基准，而不是使用字典顺序。

* [NATIONAL] CHAR[(M)] [CHARACTER SET charset_name] [COLLATE collation_name]： 一个固定长度的字符串，如果长度不够则右填充空格（注意：我们在进行SQL操作的时候，MySQL会自动的将后面填充的空格去除）。```M```表示字符串长度，取值范围是[0,255]，假如省略的话则默认长度为1。

实际上，```CHAR```是```CHARACTER```的简写。MySQL在通常情况下会使用一些预定义好```NATIONAL CHAR```(或```NCHAR```)来作为该列的字符集编码，默认情况下采用```UTF-8```编码。

另外，```CHAR BYTE```数据类型等价于```BINARY```数据类型。

MySQL允许创建类型为```CHAR(0)```的列，这通常在兼容一下老的应用程序的时候很有用： 你需要有这么一列，但是并不会用到它的值。```CHAR(0) NULL```只允许两个值: ```NULL```或```''```(空字符串)

* [NATIONAL] VARCHAR(M) [CHARACTER SET charset_name] [COLLATEcollation_name]: 一个可变长度的字符串，```M```代表该列的最大字符数。```M```的取值范围是[0,65535]，```VARCHAR```最大的有效长度取决于一行所允许的最长字节数(65535)以及对应的字符编码。例如：```UTF-8```编码的字符每一个最长可以达到3字节，这样在utf8编码下```VARCHAR```最长为21844.

```VARCHAR```存储数据的格式为： 长度+数据。其中```长度```部分占用1个或2个字节，假如数据的长度小于等于255，则可用1个字节来存储， 否则需要用两个字节来进行存储。

```VARCHAR```数据类型是```CHARACTER VARYING```的简写。MySQL会用预先定义好的字符集来对VARCHAR列进行编码。

* BINARY(M): ```BINARY```类型类似于```CHAR```类型，但是其用于存储```二进制字节(byte)```。

* VARBINARY(M): 类似于```VARCHAR```类型，但是其用于存储```二进制字节(byte)```，M代表允许的最大字节数。

* TINYBLOB: 最大允许255个字节

* TINYTEXT [CHARACTER SET charset_name] [COLLATE collation_name]： 最长允许255个字符。假如是属于```宽字符```的话，则允许的长度会更少。

* BLOB[(M)]: 最大允许65535个字节，

* TEXT[(M)] [CHARACTER SET charset_name] [COLLATE collation_name]: 最长允许65535个字符。假如是属于```宽字符```的话，则允许的长度会更少。

* MEDIUMBLOB: 最长允许```2^24-1```个字节(16777215)。

* MEDIUMTEXT [CHARACTER SET charset_name] [COLLATE collation_name]： 最长允许```2^24-1```个字符(16777215)。假如是属于```宽字符```的话，则允许的长度会更少。

* LONGBLOB： 最长允许4GB字节。而实际应用中依赖于client/Server通信协议所允许的最大长度

* LONGTEXT [CHARACTER SET charset_name] [COLLATE collation_name]: 最大长度允许4GB字节。对于宽字符，则允许的长度会更少。而实际应用中依赖于client/Server通信协议所允许的最大长度

* ENUM('value1','value2',...) [CHARACTER SET charset_name] [COLLATE collation_name]： 枚举类型，取值只能为```value1```、...、```value2```，```NULL```或者特殊错误值```''```。在内部实现上，枚举是采用数字来表示。理论上，允许的最大枚举个数为65535，而实际上一般小于3000.

* SET('value1','value2',...) [CHARACTER SET charset_name] [COLLATE collation_name]: 集合类型。该列取值只允许是集合中的一个或多个元素

<pre>
说明： 对于CHAR、VARCHAR类型，MySQL都是采用字符个数而非字节个数来统计长度的。例如CHAR(3),对于latin字
母来说占用的最长字节长度是3字节；而对于utf-8类型的中文来说，占用的最长字节长度为9字节。
</pre>


## 3. MySQL 数值类型(Numeric Type)

MySQL支持所有标准的SQL数值类型， 这包括```exact```数值类型: INTEGER, SMALLINT, DECIMAL, 和 NUMERIC。 也包括```approximate```数值类型: FLOAT,
REAL, 和 DOUBLE PRECISION。

### 3.1 严格整数类型
下面列出MySQL中的一些严格整数类型：

![db-mysql](https://ivanzz1001.github.io/records/assets/img/db/mysql_integer_type.jpg)


### 3.2 定点数类型
```DECIMAL```和```NUMERIC```用于存储定点数，当需要一个准确的精度的时候，可以使用这种数据类型。在MySQL中```NUMERIC```数据类型是通过```DECIMAL```来实现的，因此可认为它们等价。

使用时的语法格式为```DECIMAL(a,b)```，其中```a```表示数字的位数，```b```表示小数部分的位数。例如：
<pre>
salary DECIMAL(5,2)
</pre>
在该例子中，5是精度（即有效数字的位数），2是小数部分的位数。在这里```salary```可表示的范围是[-999.99, 999.99]。

在标准的SQL语法中,```DECIMAL(M)```等价于```DECIMAL(M,0)```，类似的```DECIMAL```也等价于```DECIMAL(M,0)```，只不过在M省略的情况下默认值是10.

### 3.3 浮点数类型
浮点数类型包括```单精度```浮点数类型和```双精度```浮点数类型。MySQL使用4个字节来存放```单精度```浮点数，使用8字节来存放双精度浮点数。

### 3.4 BIT类型
MySQL允许使用```BIT```类型来存储二进制位。```BIT(M)```表示可以存储```M```个bit， ```M```的取值范围是[1,64]。我们可以通过```b'value'```的方式来进行赋值。例如： ```b'111'``` 和```b'10000000'```分别代表7和128。

## 4. 时间和日期类型

代表时间和日期的类型主要有： ```DATE```、```TIME```、```DATETIME```、```TIMESTAMP```和```YEAR```。每一种时间类型都有一定的取值范围，其中也包括```0```（其主要用于在对该列指定为一个不合法值时，采用```0```作为默认值)。在使用MySQL```时间和日期类型```时有如下一些通用规则：

* MySQL以统一的格式对```时间或日期```类型进行输出，但是可以接受多种不同格式的输入

* 尽管可以支持多种形式的输入，但是必须确保```日期```部分的顺序必须为```year-month-day```这样的顺序（例如：98-09-04)。

* 用两个数字来表示年份通常会造成歧义，因为其并没有指定属于哪个```世纪```。MySQL在处理两个数字表示的年份时，遵循如下规则：
<pre>
1) 当YEAR的值在[70,99]时，会被MySQL自动的转换为[1970,1999]
2) 当YEAR的值在[00,69]时，会被MySQL自动的转换为[2000,2069]
</pre>

* 当对```Date和time```相应字段进行数值相关运算时，MySQL会自动的将该类型转换成数值类型。

* 默认情况下，MySQL会对超过```时间和日期```范围的值转换为```0```。唯一一个例外是```TIME```类型，如果超出了范围，则会截断至一个有效值。

* MySQL允许在存储```DATE```和```DATETIME```列时，将```day```和```month```设置为0。这在有些情况下很有用，例如当你存储某个人的生日时，不知道具体的month/day，那么就可以将这两个字段设置为0。例如```2009-00-00```。而当你存储了这样的日期时，则在执行如```DATE_SUB()```、```DATE_ADD()```则可能不能获得正确的结果。

* MySQL允许你存储```0000-00-00```这样一个```0```值以作为一个dummy date。这在某一些情况下比一个```NULL```值更为方便，并且会占用更少的数据和索引空间。

* ```Zero```的时间及日期，在通过Connector/ODBC操作的时候会自动转换为NULL，因为ODBC并不能处理这样的值

下表列出了```时间及日期```类型中```zero```值的表示方法，你也可以简单的用```'0'```来表示：


![db-date-time](https://ivanzz1001.github.io/records/assets/img/db/mysql_datetime_zero.jpg)


### 4.1 DATE、DATETIME和TIMESTAMP类型
```DATE```、```DATETIME```以及```TIMESTAMP```则三种类型是有关联的。MySQL在存储```TIMESTAMP```类型数据时，会将当前时区的时间转换成UTC时间，然后在获取时又会将UTC时间转换回当前时区时间（对于```DATATIME```则不会做这样的时区转换）。

### 4.2 自动初始化与更新TIMESTAMP和DATETIME
从MySQL5.6.5开始，```TIMESTAMP```和```DATETIME```列可以自动的初始化和更新为当前时间。要实现这样一个功能，需要在创建该列时使用```DEFAULT CURRENT_TIMESTAMP```或(和)```ON UPDATE CURRENT_TIMESTAMP```这样的语句。另外如果要初始化为一个常量值，也可以通过```DEFATLT <constant>```来实现，例如：```DEFAULT 0```或```DEFAULT '2000-01-01 00:00:00'```。

对于```DATETIME```及```TIMESTAMP```列，在创建时我们可以同时指定```default```和```auto-update```值，也可以单独指定其中一个。下面我们简单介绍一下如何指定：

* 同时指定```DEFAULT CURRENT_TIMESTAMP```和```ON UPDATE CURRENT_TIMESTAMP```，这样就会使得当前列以当前时间作为默认值，并且在更新时也会自动设为当前时间
<pre>
CREATE TABLE t1 (
ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
dt DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);
</pre>

* 只设置默认值为当前时间或一个常量值
<pre>
CREATE TABLE t1 (
ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
dt DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE t1 (
ts TIMESTAMP DEFAULT 0,
dt DATETIME DEFAULT 0
);
</pre>

* 设置默认值为一个常量，更新值为当前时间
<pre>
CREATE TABLE t1 (
ts TIMESTAMP DEFAULT 0 ON UPDATE CURRENT_TIMESTAMP,
dt DATETIME DEFAULT 0 ON UPDATE CURRENT_TIMESTAMP
);
</pre>

* 只设置定更新值为当前时间（在这种情况下，对于```TIMESTAMP```类型，默认值则会自动取为0或者NULL; ```DATETIME``类型，默认值会自动取为NULL或0)
<pre>
CREATE TABLE t1 (
ts1 TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, -- default 0
ts2 TIMESTAMP NULL ON UPDATE CURRENT_TIMESTAMP -- default NULL
);

CREATE TABLE t1 (
dt1 DATETIME ON UPDATE CURRENT_TIMESTAMP, -- default NULL
dt2 DATETIME NOT NULL ON UPDATE CURRENT_TIMESTAMP -- default 0
);
</pre>

## 5. String类型
string类型主要包括```CHAR```、```VARCHAR```、```BINARY```、```VARBINARY```、```BLOB```、```TEXT```、```ENUM```和```SET```8种类型。本章会介绍一下这些类型的使用。

### 5.1 CHAR与VARCHAR类型
```CHAR```与```VARCHAR```类型类似，但是在存储于获取数据的时候会有些不同。在最大长度以及如何处理尾部空格方面也有些不同。```CHAR```与```VARCHAR```都是通过在声明时指定一个长度来表明所允许的最大长度。例如: ```CHAR(30)```可以容纳最长30个字符。

对于```CHAR```类型的列来说，其长度是固定的（即你创建表时指定的长度），且范围是[0,255]。当存储```CHAR```类型列时，右侧会用空格填充至所指定的长度。而当获取该列时，尾部的空格会被移除（除非SQL模式中PAD_CHAR_TO_FULL_LENGTH是enable的）

对于```VARCHAR```类型的列，其长度是可变长的。取值范围是[0,65535]。最大的有效长度取决于一行(row)所允许的最大大小（65535)以及相应的字符集编码。```VARCHAR```类型末尾并不会被空格填充至指定的宽度。

在MySQL的```strict```模式下，假如插入的数据长度大于```CHAR```、```VARCHAR```所声明的长度，那么将会插入失败，并返回错误；在其他模式下则可能会对插入的数据进行截断，并返回警告信息。






## 2. 表操作

**1) 导出整个数据库表结构**
{% highlight string %}
# mysqldump -hhostname -uusername -ppassword -d databasename >> databasename.sql
{% endhighlight %}






<br />
<br />
**[参看]**:

1. [MySQL基础](https://www.toutiao.com/a6543580080638001668/)

2. [mySQL](https://baike.baidu.com/item/mySQL/471251?fr=aladdin)

3. [MySQL用户权限(Host,User,Password)管理(mysql.user)](https://blog.csdn.net/typa01_kk/article/details/49126365)

4. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

5. [【MySQL】MySQL数据类型宽度](https://blog.csdn.net/da_guo_li/article/details/79011718)

6. [mysql新建数据库时的collation选择（转）](https://www.cnblogs.com/sonofelice/p/6432986.html)


<br />
<br />
<br />

