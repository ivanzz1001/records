---
layout: post
title: mysql数据库基础（二）
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


## 2. MySQL数据类型
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

**5) MEDIUMINT[(M]) [UNSIGNED] [ZEROFILL]**

中等大小整数类型。其中```SIGNED```的取值范围是[-8388608,8388607]，```UNSIGNED```的取值范围是[0,16777215]。





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


<br />
<br />
<br />

