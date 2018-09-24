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

### 2.1 创建数据库表

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

**1） 索引和外键**

在创建表的时候可以指定索引和外键，在这里我们介绍一下相关的内容：

* CONSTRAINT symbol: 用于指定约束名称。假如在创建表时添加了```CONSTRAINT symbol```子句并且指定了```symbol```的话，则symbol在整个数据库中必须是唯一的。如果有重复的symbol,则会产生相应的错误。而如果并未指定symbol，那么MySQL或默认帮我们产生一个。

* PRIMARY KEY: 是一个unique索引，并且所有的列必须被定义为```NOT NULL```。假如我们并没有显示的指定```NOT NULL```，那么MySQL会隐式的帮我们指定。一个表只能有一个```PRIMARY KEY```。

对于存储索引为```InnoDB```的表，建议尽量保持```PRIMARY KEY```足够的短以减少存储占用的空间，这是因为```InnoDB```表的二级索引是存储主键列的。

在创建表的时候，请将```PRIMARY KEY```放在最开始，然后是```UNIQUE```索引，再接着是```noneunique```索引。这可以帮助MySQL优化器选择优先使用哪个索引，并且能够更快速的检测到重复的unique值。

一个```PRIMARY KEY```可以是一个多列(multi-column)索引，对于一个多列索引的话，你并不能够在创建列的时候直接在后面指定```PRIMARY KEY```，你可以在一个单独的```PRIMARY KEY(key_par,...)```子句中指定。

* KEY | INDEX: 在这里```KEY```通常等价于```INDEX```。主要为了兼容其他的数据库系统。

* UNIQUE: 唯一索引会创建一个约束要求索引中的所有值都是唯一的。对于所有存储引擎来说，一个```UNIQUE```索引是允许存在多个```NULL```值的（假设该列允许的话）。

* FULLTEXT: 全文索引是一种特殊类型的索引，主要用于全文搜索。只有```InnoDB```和```MyISAM```这两个存储引擎支持全文索引。它们只能够针对```CHAR```、```VARCHAR```和```TEXT```列来创建全文索引。

* SPATIAL: 对于spatial类型数据，你可以创建```SPATIAL```索引。并且只有```MyISAM```类型表的```非空```列才支持```SPATIAL```索引。

* ```FOREIGN KEY```: MySQL支持外键，以允许你跨表参考(reference)相关的数据。并且由于```foreign key```的约束，可以使得数据传播上的一致性。

**2） 创建表时生成的文件**

在数据库的对应目录中，每一个表的格式文件是以```.frm```结尾的，而其他的文件根据存储引擎的不同会略有些差异。

对于```InnoDB```存储引擎来说，存储文件是由```innodb_file_per_table```配置选项所控制的。在创建InnoDB表时，如果此选项被打开，表数据以及所关联的索引都会存储在一个```.ibd```文件中。而如果该选项被关闭的话，所有的```InnoDB```表数据和索引数据都会被存放在系统表空间(system tablespace)中。

对于```MyISAM```表，存储引擎会创建数据文件和索引文件。对于每一个```MyISAM```表tbl_name，都会在硬盘上存在如下三个文件：

* tbl_name.frm: 表格式文件

* tbl_name.MYD: 数据文件

* tbl_name.MYI: 索引文件

### 2.2 删除数据表

删除表的语法如下：
{% highlight string %}
DROP [TEMPORARY] TABLE [IF EXISTS]
tbl_name [, tbl_name] ...
[RESTRICT | CASCADE]
{% endhighlight %}



<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)



<br />
<br />
<br />

