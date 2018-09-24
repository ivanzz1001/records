---
layout: post
title: mysql数据库基础（六）
tags:
- database
categories: database
description: mysql数据库基础
---


本章主要介绍一下MySQL中的SQL语法


<!-- more -->

## 1. 数据库操作

数据库操作主要包括：

* 创建数据库

* 删除数据库

* 选择数据库

* 修改数据库

**1）创建数据库**

创建数据库的基本语法如下：
{% highlight string %}
CREATE {DATABASE | SCHEMA} [IF NOT EXISTS] db_name
[create_specification] ...

create_specification:
[DEFAULT] CHARACTER SET [=] charset_name
| [DEFAULT] COLLATE [=] collation_name
{% endhighlight %}
```CREATE DATABASE```用于创建一个指定名字的数据库。要想使用该SQL语句，要求用户具有```CREATE```权限。其中create_specification用于指定所创建数据库的字符集特性。数据库的字符集特性会被存储在数据库目录的```db.opt```文件中。

在MySQL中，一个数据库对应着一个目录，数据库中的每一个表(table)都有相应的文件对应。因为在数据库刚建立的时候并没有表(table)，因此目录下仅有一个文件```db.opt```。

假如你在MySQL的数据目录下创建一个目录，MySQL会认为其是一个数据库。

**2) 删除数据库**

删除数据库的基本语法如下：
{% highlight string %}
DROP {DATABASE | SCHEMA} [IF EXISTS] db_name
{% endhighlight %}
```DROP DATABASE```会删除数据库和数据库中的所有表数据。要删除数据库，你必须要有```DROP```权限。语句执行后的返回结果为所删除的表的数目。

**3) 选择数据库**

选择数据库语法如下：
{% highlight string %}
USE db_name;
{% endhighlight %}

**4) 查看所有数据库**

查看所有数据库语法如下：
{% highlight string %}
SHOW DATABASES;
{% endhighlight %}

**5) 查看当前所使用的数据库**
{% highlight string %}
mysql> use app;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed
mysql> SELECT DATABASE();
+------------+
| DATABASE() |
+------------+
| app        |
+------------+
1 row in set (0.00 sec)
{% endhighlight %}

**6) 修改数据库**
{% highlight string %}
ALTER {DATABASE | SCHEMA} [db_name]
alter_specification ...

ALTER {DATABASE | SCHEMA} db_name
UPGRADE DATA DIRECTORY NAME

alter_specification:
[DEFAULT] CHARACTER SET [=] charset_name
| [DEFAULT] COLLATE [=] collation_name
{% endhighlight %}
这里一般只用于修改字符集与校对集。

## 2. 表操作

表的操作主要包括：

* 创建表

* 删除表

* 修改表

**1) 创建数据库表**

基本语法如下：
{% highlight string %}
CREATE [TEMPORARY] TABLE [IF NOT EXISTS] tbl_name
(create_definition,...)
[table_options]
[partition_options]

CREATE [TEMPORARY] TABLE [IF NOT EXISTS] tbl_name
[(create_definition,...)]
[table_options]
[partition_options]
[IGNORE | REPLACE]
[AS] query_expression

CREATE [TEMPORARY] TABLE [IF NOT EXISTS] tbl_name
{ LIKE old_tbl_name | (LIKE old_tbl_name) }

create_definition:
col_name column_definition
| [CONSTRAINT [symbol]] PRIMARY KEY [index_type] (key_part,...)
[index_option] ...
| {INDEX|KEY} [index_name] [index_type] (key_part,...)
[index_option] ...
| [CONSTRAINT [symbol]] UNIQUE [INDEX|KEY]
[index_name] [index_type] (key_part,...)
[index_option] ...
| {FULLTEXT|SPATIAL} [INDEX|KEY] [index_name] (key_part,...)
[index_option] ...
| [CONSTRAINT [symbol]] FOREIGN KEY
[index_name] (col_name,...) reference_definition
| CHECK (expr)

column_definition:
data_type [NOT NULL | NULL] [DEFAULT default_value]
[AUTO_INCREMENT] [UNIQUE [KEY]] [[PRIMARY] KEY]
[COMMENT 'string']
[COLUMN_FORMAT {FIXED|DYNAMIC|DEFAULT}]
[STORAGE {DISK|MEMORY|DEFAULT}]
[reference_definition]
data_type:
(see Chapter 11, Data Types)
key_part:
col_name [(length)] [ASC | DESC]
index_type:
USING {BTREE | HASH}
index_option:
KEY_BLOCK_SIZE [=] value
| index_type
| WITH PARSER parser_name
| COMMENT 'string'
reference_definition:
REFERENCES tbl_name (key_part,...)
[MATCH FULL | MATCH PARTIAL | MATCH SIMPLE]
[ON DELETE reference_option]
[ON UPDATE reference_option]
{% endhighlight %}

例如，下面创建名为```runoob```的表：
{% highlight string %}
CREATE TABLE IF NOT EXISTS `runoob_tbl`(
   `runoob_id` INT UNSIGNED AUTO_INCREMENT,
   `runoob_title` VARCHAR(100) NOT NULL,
   `runoob_author` VARCHAR(40) NOT NULL,
   `submission_date` DATE,
   PRIMARY KEY ( `runoob_id` )
)ENGINE=InnoDB DEFAULT CHARSET=utf8;
{% endhighlight %}






<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)



<br />
<br />
<br />

