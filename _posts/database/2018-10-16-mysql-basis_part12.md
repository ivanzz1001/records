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






<br />
<br />
**[参看]**:

1. [学会用各种姿势备份MySQL数据库](http://www.cnblogs.com/liangshaoye/p/5464794.html)

2. [Mysql Binlog三种格式介绍及分析](https://www.cnblogs.com/itcomputer/articles/5005602.html)




<br />
<br />
<br />

