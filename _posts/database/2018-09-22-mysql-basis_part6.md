---
layout: post
title: mysql相关SQL语法
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
CREATE DATABASE IF NOT EXISTS test DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

use test;

CREATE TABLE IF NOT EXISTS `runoob_tbl`(
   `runoob_id` INT UNSIGNED AUTO_INCREMENT,
   `runoob_title` VARCHAR(100) COLLATE utf8_unicode_ci NOT NULL,
   `runoob_author` VARCHAR(40) NOT NULL,
   `submission_date` DATE,
   PRIMARY KEY ( `runoob_id` )
)ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `runoob_tbl2`(
  `seqid` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '唯一ID值',
  `site` varchar(10) NOT NULL COMMENT 'site info',
  `bucket` varchar(64) NOT NULL COMMENT 'bucket info',
  `md5` varchar(64) NOT NULL COMMENT 'file md5 info',
  `mmhash` bigint(20) unsigned NOT NULL COMMENT 'site and bucket mmurhash',
  `createTs` bigint(20) NOT NULL COMMENT 'create timestamp',
  `modifyTs` bigint(20) NOT NULL COMMENT 'modify timestamp',
  `reserved` int(10) DEFAULT 0 COMMENT 'keep reserved',
  PRIMARY KEY (`seqid`),
  UNIQUE KEY `unique_record` (`md5`, `mmhash`) COMMENT '唯一索引',
)ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- 单行注释

/*
   多行注释
 */
INSERT INTO runoob_tbl2 (site, bucket, md5, mmhash, createTs, modifyTs reserved) 
VALUES('cn', 'test', 'aaaa', 0, unix_timestamp(now()), unix_timestamp(now()), 0);
{% endhighlight %}

**1） 索引和外键**

在创建表的时候可以指定索引和外键，在这里我们介绍一下相关的内容：

* CONSTRAINT symbol: 用于指定约束名称。假如在创建表时添加了```CONSTRAINT symbol```子句并且指定了```symbol```的话，则symbol在整个数据库中必须是唯一的。如果有重复的symbol,则会产生相应的错误。而如果并未指定symbol，那么MySQL或默认帮我们产生一个。

* PRIMARY KEY: 是一个unique索引，并且所有的列必须被定义为```NOT NULL```。假如我们并没有显示的指定```NOT NULL```，那么MySQL会隐式的帮我们指定。一个表只能有一个```PRIMARY KEY```。

对于存储索引为```InnoDB```的表，建议尽量保持```PRIMARY KEY```足够的短以减少存储占用的空间，这是因为```InnoDB```表的二级索引是存储主键列的。

在创建表的时候，请将```PRIMARY KEY```放在最开始，然后是```UNIQUE```索引，再接着是```noneunique```索引。这可以帮助MySQL优化器选择优先使用哪个索引，并且能够更快速的检测到重复的unique值。

一个```PRIMARY KEY```可以是一个多列(multi-column)索引，对于一个多列索引的话，你并不能够在创建列的时候直接在后面指定```PRIMARY KEY```，你可以在一个单独的```PRIMARY KEY(key_par,...)```子句中指定。

* KEY/INDEX: 在这里```KEY```通常等价于```INDEX```。主要为了兼容其他的数据库系统。

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
```DROP TABLE```用于移除一个或多个表，对于每一个表你必须具有```DROP```权限。

### 2.3 修改表
修改表的基本语法如下：
{% highlight string %}
ALTER [ONLINE|OFFLINE] [IGNORE] TABLE tbl_name
[alter_specification [, alter_specification] ...]
[partition_options]

alter_specification:
table_options
| ADD [COLUMN] col_name column_definition
    [FIRST | AFTER col_name]
| ADD [COLUMN] (col_name column_definition,...)
| ADD {INDEX|KEY} [index_name]
    [index_type] (key_part,...) [index_option] ...
| ADD [CONSTRAINT [symbol]] PRIMARY KEY
    [index_type] (key_part,...) [index_option] ...
| ADD [CONSTRAINT [symbol]]
    UNIQUE [INDEX|KEY] [index_name]
    [index_type] (key_part,...) [index_option] ...
| ADD FULLTEXT [INDEX|KEY] [index_name]
    (key_part,...) [index_option] ...
| ADD SPATIAL [INDEX|KEY] [index_name]
    (key_part,...) [index_option] ...
| ADD [CONSTRAINT [symbol]]
    FOREIGN KEY [index_name] (col_name,...)
    reference_definition
| ALGORITHM [=] {DEFAULT|INPLACE|COPY}
| ALTER [COLUMN] col_name {SET DEFAULT literal | DROP DEFAULT}
| CHANGE [COLUMN] old_col_name new_col_name column_definition
    [FIRST|AFTER col_name]
| [DEFAULT] CHARACTER SET [=] charset_name [COLLATE [=] collation_name]
| CONVERT TO CHARACTER SET charset_name [COLLATE collation_name]
| {DISABLE|ENABLE} KEYS
| {DISCARD|IMPORT} TABLESPACE
| DROP [COLUMN] col_name
| DROP {INDEX|KEY} index_name
| DROP PRIMARY KEY
| DROP FOREIGN KEY fk_symbol
| FORCE
| LOCK [=] {DEFAULT|NONE|SHARED|EXCLUSIVE}
| MODIFY [COLUMN] col_name column_definition
    [FIRST | AFTER col_name]
| ORDER BY col_name [, col_name] ...
| RENAME [TO|AS] new_tbl_name
| ADD PARTITION (partition_definition)
| DROP PARTITION partition_names
| TRUNCATE PARTITION {partition_names | ALL}
| COALESCE PARTITION number
| REORGANIZE PARTITION partition_names INTO (partition_definitions)
| EXCHANGE PARTITION partition_name WITH TABLE tbl_name
| ANALYZE PARTITION {partition_names | ALL}
| CHECK PARTITION {partition_names | ALL}
| OPTIMIZE PARTITION {partition_names | ALL}
| REBUILD PARTITION {partition_names | ALL}
| REPAIR PARTITION {partition_names | ALL}
| REMOVE PARTITIONING
{% endhighlight %}
```ALTER TABLE```用于改变表的结构。例如，你可以添加或删除一列，创建或销毁索引，更改已存在列的类型，重命名列或表。

下面给出一些示例，假如我们通过如下语句创建了表```t1```:
{% highlight string %}
CREATE TABLE `t1` (
	`a` INTERGER,
	`b` CHAR(10)
);
{% endhighlight %}

* 将表从```t1```命名为```t2```
{% highlight string %}
ALTER TABLE `t1` RENAME `t2```;
{% endhighlight %}

* 将列```a```从```INTEGER```类型转换为```TINYINT NOT NULL```类型，并且将列```b```从```CHAR(10```类型转换为```CHAR(20)```类型，且将列```b```的名称由b更改为```c```
{% highlight string %}
ALTER TABLE `t2` MODIFY a TINYINT NOT NULL, CHANGE `b` `c` CHAR(20);
{% endhighlight %}

* 添加一个新的```TIMESTAMP```类型的列```d```
{% highlight string %}
ALTER TABLE t2 ADD d TIMESTAMP;
{% endhighlight %}

* 在列```d```上增加一个索引，在列```a```上增一个```UNIQUE```索引
{% highlight string %}
ALTER TABLE t2 ADD INDEX (d), ADD UNIQUE (a);
{% endhighlight %}

* 移除列```c```
{% highlight string %}
ALTER TABLE t2 DROP COLUMN c;
{% endhighlight %}

* 将一列插入到指定的位置
{% highlight string %}
CREATE TABLE `t_part_info` (
  `appid` varchar(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `bucket` varchar(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `object` varchar(512) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `uploadid` varchar(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `seqid` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `create_time` bigint(20) NOT NULL,
  `modify_time` bigint(20) NOT NULL,
  PRIMARY KEY (`seqid`),
  KEY `key_uploadid` (`uploadid`)
) ENGINE=InnoDB AUTO_INCREMENT=1391219 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

ALTER TABLE `t_part_info` MODIFY `seqid` bigint(20) unsigned PRIMARY KEY AUTO_INCREMENT COMMENT 'change column order' first;
ALTER TABLE `t_part_info` ADD `failure_cnt` tinyint UNSIGNED NOT NULL DEFAULT 0 COMMENT 'add failure count column';

ALTER TABLE `t_part_info` MODIFY `seqid` bigint(20) unsigned AUTO_INCREMENT COMMENT 'change column order' first, ADD `failure_cnt` tinyint UNSIGNED NOT NULL DEFAULT 0 COMMENT 'add failure count column';

ALTER TABLE `t_part_info` ADD `author` varchar(64) NOT NULL DEFAULT 'un-named' COMMENT '分片上传者' AFTER `seqid`;
{% endhighlight %}

* 修改列的名称
{% highlight string %}
CREATE TABLE `t_part_info` (
  `appid` varchar(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `bucket` varchar(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `object` varchar(512) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `uploadid` varchar(64) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `seqid` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `create_time` bigint(20) NOT NULL,
  `modify_time` bigint(20) NOT NULL,
  PRIMARY KEY (`seqid`),
  KEY `key_uploadid` (`uploadid`)
) ENGINE=InnoDB AUTO_INCREMENT=1391219 DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

ALTER TABLE `t_part_info` CHANGE `create_time` `create_ts` bigint(20) NOT NULL COMMENT '创建时间';
{% endhighlight %}

## 3. 视图操作
视图操作主要包括：

* 创建视图

* 修改视图

* 删除视图

### 3.1 创建视图
创建视图语法如下：
{% highlight string %}
CREATE
	[OR REPLACE]
	[ALGORITHM = {UNDEFINED | MERGE | TEMPTABLE}]
	[DEFINER = { user | CURRENT_USER }]
	[SQL SECURITY { DEFINER | INVOKER }]
	VIEW view_name [(column_list)]
	AS select_statement
	[WITH [CASCADED | LOCAL] CHECK OPTION]
{% endhighlight %}
```CREATE VIEW```语句用于创建一个新的视图，或者替换一个老的视图（假如指定了```OR REPLACE```)。假如视图不存在，```CREATE OR REPLACE VIEW```等价于```CREATE VIEW```；假如视图已经存在，则```CREATE OR REPLACE```会替换该视图。

```select_statement```是一个```SELECT```子句，用于体统视图的定义。说明： select_statement可以从一个基础表中查询，也可以从另一个视图中查询。

说明： MySQL数据库视图在创建的时候就已经固定下来了(即创建时视图就处于```frozen```状态)，并不会受到后续底层表定义的影响。例如，假如在一个表上通过```SELECT *```定义了一个视图，假如后面再在表中增加了一列，那么其并不会成为该视图的一部分；而如果后续删除了表的一列，那么在查看视图的时候将会产生错误。


下面给出一个示例：
{% highlight string %}
mysql> CREATE TABLE t (qty INT, price INT);
mysql> INSERT INTO t VALUES(3, 50);
mysql> CREATE VIEW v AS SELECT qty, price, qty*price AS value FROM t;
mysql> SELECT * FROM v;
+------+-------+-------+
| qty | price | value |
+------+-------+-------+
| 3 | 50 | 150 |
+------+-------+-------+
{% endhighlight %}


### 3.2 修改视图

修改视图的基本语法如下：
{% highlight string %}
ALTER
[ALGORITHM = {UNDEFINED | MERGE | TEMPTABLE}]
[DEFINER = { user | CURRENT_USER }]
[SQL SECURITY { DEFINER | INVOKER }]
VIEW view_name [(column_list)]
AS select_statement
[WITH [CASCADED | LOCAL] CHECK OPTION]
{% endhighlight %}
上述语句用于修改一个视图（视图必须已经存在）。执行该语句时，必须要有```CREATE VIEW```以及```DROP VIEW```的权限。

### 3.3 删除视图

删除视图语法如下：
{% highlight string %}
DROP VIEW [IF EXISTS]
	view_name [, view_name] ...
[RESTRICT | CASCADE]
{% endhighlight %}
用于删除一个或多个视图。

## 4. 索引操作
索引操作主要包括：

* 创建索引

* 删除索引

### 4.1 创建索引
创建索引语法如下：
{% highlight string %}
CREATE [ONLINE | OFFLINE] [UNIQUE | FULLTEXT | SPATIAL] INDEX index_name
	[index_type]
	ON tbl_name (key_part,...)
	[index_option]
	[algorithm_option | lock_option] ...

key_part:
	col_name [(length)] [ASC | DESC]

index_option:
	KEY_BLOCK_SIZE [=] value
	| index_type
	| WITH PARSER parser_name
	| COMMENT 'string'

index_type:
	USING {BTREE | HASH}

algorithm_option:
	ALGORITHM [=] {DEFAULT | INPLACE | COPY}

lock_option:
	LOCK [=] {DEFAULT | NONE | SHARED | EXCLUSIVE}
{% endhighlight %}
通常你会在使用```CREATE TABLE```语句创建表时同时创建出该表上的所有索引。这里```CREATE INDEX```允许向一个已存在的表中添加索引。可以通过```SHOW INDEX FROM tbl_name```来查看一个表上的索引。

实际上，```CREATE INDEX```会被映射成一个```ALTER TABLE```语句来创建索引。注意，```CREATE INDEX```并不能被用于创建一个```PRIMARY KEY```。

**1） UNIQUE INDEX**

可以在一个表上创建```UNIQUE```索引，```FULLTEXT```索引，```SPATIAL```索引。这里我们主要介绍一下```唯一索引```。一个```UNIQUE```索引会创建约束：要求索引上的值都是唯一的。假如你向表中插入一个新的行，若造成```UNIQUE```索引列的值重复，则会产生相应的错误。假如你在一列上创建一个```前缀UNIQUE```索引，那你需要保证在前缀长度范围内数据是唯一的。注： 对于```UNIQUE```索引，如果该列本身允许```NULL```，则允许有重复的NULL。

假如在一个表的一个```单独```的列上（要求该列的数据类型为```整数类型```)创建```PRIMARY KEY```或```UNIQUE NOT NULL```索引，那么你可以在```SELECT```语句中使用```_rowid```来引用索引列：

* 假如```PRIMARY KEY```索引对应的列是一个```单独的列```，并且列的数据类型是```整数类型```，那么你可以使用```_rowid```来引用该列；假如有一个```PRIMARY KEY```索引，并且该索引列并不是一个单独的整数类型列，那么```_rowid```并不能被使用。

* 否则，可以使用```_rowid```来引用第一个类型为整数的单独```UNIQUE NOT NULL```索引列。假如第一个```UNIQUE NOT NULL```索引列并不是```单独的整数类型列```，那么```_rowid```将不能被使用。

**2) 索引类型(index_type)**

在创建索引时，底层一般都是用```BTREE```或者```HASH```来作为数据结构。一般来说，对于```InnoDB```以及```MyISAM```底层是采用```BTREE```来做索引的；对于```MEMORY```以及```NDB```存储引擎，底层可以采用```BTREE```来做索引，也可以采用```HASH```来做索引。

### 4.2 删除索引
删除索引语法如下：
{% highlight string %}
DROP INDEX [ONLINE|OFFLINE] index_name ON tbl_name
	[algorithm_option | lock_option] ...

algorithm_option:
ALGORITHM [=] {DEFAULT|INPLACE|COPY}

lock_option:
	LOCK [=] {DEFAULT|NONE|SHARED|EXCLUSIVE}
{% endhighlight %}
上面表示从表```tbl_name```上删除名称为```index_name```的索引。注意，对于```PRIMARY KEY```这样的索引，其```index_name```为```PRIMARY```。

## 5. 数据操作
表的数据操作这里我们主要介绍：

* 表数据的插入

* 表数据的删除

* 表数据的更新

* 表数据的查询

另外，其实也包括调用```存储过程```以及一些其他的操作。关于MySQL存储过程，我们后面的章节再进行讲解。

### 5.1 数据插入

插入的基本语法如下：
{% highlight string %}
INSERT [LOW_PRIORITY | DELAYED | HIGH_PRIORITY] [IGNORE]
	[INTO] tbl_name
	[PARTITION (partition_name [, partition_name] ...)]
	[(col_name [, col_name] ...)]
	{VALUES | VALUE} (value_list) [, (value_list)] ...
	[ON DUPLICATE KEY UPDATE assignment_list]

INSERT [LOW_PRIORITY | DELAYED | HIGH_PRIORITY] [IGNORE]
	[INTO] tbl_name
	[PARTITION (partition_name [, partition_name] ...)]
	SET assignment_list
	[ON DUPLICATE KEY UPDATE assignment_list]

INSERT [LOW_PRIORITY | HIGH_PRIORITY] [IGNORE]
	[INTO] tbl_name
	[PARTITION (partition_name [, partition_name] ...)]
	[(col_name [, col_name] ...)]
	SELECT ...
	[ON DUPLICATE KEY UPDATE assignment_list]

value:
	{expr | DEFAULT}

value_list:
	value [, value] ...

assignment:
	col_name = value

assignment_list:
	assignment [, assignment] ...
{% endhighlight %}

```INSERT```用于插入一条新的数据到一个已存在的表中。```INSERT ... VALUES```和```INSERT ... SET```形式的的插入语句显示的指定要插入的值，而```INSERT ... SELECT```形式的插入语句插入从另外一个表中查询出来的结果。而```INSERT```后跟```ON DUPLICATE KEY UPDATE```子句的话，如果在插入时导致一个```UNIQUE```索引或```PRIMARY KEY```重复的话，则该重复的值会被更新。

在进行表插入时需要具有该表的```INSERT```权限。而假如```ON DUPLICATE KEY UPDATE```子句被使用的话，那么还要求具有```UPDATE```权限。


### 5.2 数据删除
删除的基本语法如下：
{% highlight string %}
DELETE [LOW_PRIORITY] [QUICK] [IGNORE] FROM tbl_name
	[PARTITION (partition_name [, partition_name] ...)]
	[WHERE where_condition]
	[ORDER BY ...]
	[LIMIT row_count]
{% endhighlight %}

```DELETE```删除语句用于从表```tbl_name```删除数据，并且返回删除的行数。删除的可选条件```WHERE```用于指定删除哪些满足条件的行，假如并未指定```WHERE```条件的话，则所有的数据均会被删除。假如```LIMIT```子句被指定的话，则会最多删除指定的行数。

假如```ORDER BY```子句被指定的话，则会按照指定的顺序来进行删除。这在搭配```LIMIT```一起使用时很有效。例如，下面的语句首先查找到匹配```WHERE```条件的记录，然后再按```timestamp_column```列对这些行进行排序，最后再删除第一个元素（oldest):
<pre>
DELETE FROM somelog WHERE user = 'jcole'
ORDER BY timestamp_column LIMIT 1;
</pre>


### 5.3 数据更新
```UPDATE```是一个数据库操作语句(DML)，用于修改一个表中的记录:
{% highlight string %}
UPDATE [LOW_PRIORITY] [IGNORE] table_reference
	SET assignment_list
	[WHERE where_condition]
	[ORDER BY ...]
	[LIMIT row_count]

value:
	{expr | DEFAULT}

assignment:
	col_name = value

assignment_list:
	assignment [, assignment] ...
{% endhighlight %} 
```UPDATE```语句用于更新一个表中已存在的列。假如一个```UPDATE```语句包含了一个```ORDER BY```子句的话，则会依照指定的顺序更新行记录，这在一些特性情形下是很有用的（如果不按顺序，可能会导致错误）。假设有一个表```t```包含了一个```UNIQUE```索引列```id```，下面的更新语句则可能会导致```duplicate-key```错误：
<pre>
UPDATE t SET id = id + 1;
</pre>
例如， 假设该表含有```id```为1和2的两条记录，假如在2被更新为3之前将1更新为2，则会产生错误。为了避免这个问题，增加一个```ORDER BY```语句使得id更大的记录优先被更新：
<pre>
UPDATE t SET id = id + 1 ORDER BY id DESC;
</pre>

### 5.4 数据查询
数据查询语法如下：
{% highlight string %}
SELECT
	[ALL | DISTINCT | DISTINCTROW ]
	  [HIGH_PRIORITY]
	  [STRAIGHT_JOIN]
	  [SQL_SMALL_RESULT] [SQL_BIG_RESULT] [SQL_BUFFER_RESULT]
	  [SQL_CACHE | SQL_NO_CACHE] [SQL_CALC_FOUND_ROWS]
	select_expr [, select_expr ...]
	[FROM table_references
	  [PARTITION partition_list]
	[WHERE where_condition]
	[GROUP BY {col_name | expr | position}
	  [ASC | DESC], ... [WITH ROLLUP]]
	[HAVING where_condition]
	[ORDER BY {col_name | expr | position}
	  [ASC | DESC], ...]
	[LIMIT {[offset,] row_count | row_count OFFSET offset}]
	[PROCEDURE procedure_name(argument_list)]
	[INTO OUTFILE 'file_name'
		[CHARACTER SET charset_name]
		export_options
	  | INTO DUMPFILE 'file_name'
	  | INTO var_name [, var_name]]
	[FOR UPDATE | LOCK IN SHARE MODE]]
{% endhighlight %}
```SELECT```用于从一个或多个表中查询数据。针对```SELECT```最常用的子句有如下：

* 每一个```select_expr```用于指示想要获取的列。必须至少有一列

* ```table_reference```用于指定要从那个（些）表中查询数据。关于```JOIN```的查询语法我们后边会进行介绍

* 假如指定了```WHERE```子句的话，其用于指示查询条件。

此外，```SELECT```也能够被用于```查询行值```来进行计算，而不需要指定任何一个表。例如：
{% highlight string %}
mysql> SELECT 1 + 1;
-> 2
{% endhighlight %}
在这种不需要指定表的情况下，也允许通过指定一个Dummy 表```DUAL```：
{% highlight string %}
mysql> SELECT 1 + 1 FROM DUAL;
-> 2
{% endhighlight %}

一般情况下，```SELECT```子句的顺序必须严格按照上面的语法顺序。例如```HAVING```子句必须在```GROUP BY```子句之后并且在```ORDER BY```子句之前。


**1） select_expr**

```select_expr```用于指明要查询哪些列，其可以是```一列```，或者是一个```表达式```，或者是```*```(表示查询所有）：

* 假若查询列表只是一个单独的```*```，表示用于查询所有表的所有列
{% highlight string %}
SELECT * FROM t1 INNER JOIN t2 ...
{% endhighlight %}

* ```tbl_name.*```用于限定查询某一个表的所有列
{% highlight string %}
SELECT t1.*, t2.* FROM t1 INNER JOIN t2 ...
{% endhighlight %}

* 在其他形式下，使用一个未限定的```*```可能会造成SQL语法解析错误，为了避免这个问题，请使用限定的```tbl_name.*```来引用
{% highlight string %}
SELECT AVG(score), t1.* FROM t1 ...
{% endhighlight %}


* 可以使用```AS alias_name```来为```select_expr```指定一个别名，该别名可以被后续的```GROUP BY```、```ORDER BY```以及```HAVING```子句所使用。例如：
{% highlight string %}
SELECT CONCAT(last_name,', ',first_name) AS full_name FROM mytable ORDER BY full_name;
{% endhighlight %}
其实你也可以使用```tbl_name AS alias_name```来对table进行重命名。例如：
{% highlight string %}
SELECT t1.name, t2.salary FROM employee AS t1, info AS t2
WHERE t1.name = t2.name;
{% endhighlight %}

* 在```ORDER BY```、```GROUP BY```子句中，可以使用列名(column names)、列的别名(column aliases)、或列号(column position)来引用```SELECT```查询出的列。列号(column position)从```1```开始：
{% highlight string %}
SELECT college, region, seed FROM tournament ORDER BY region, seed;

SELECT college, region AS r, seed AS s FROM tournament ORDER BY r, s;

SELECT college, region, seed FROM tournament ORDER BY 2, 3;
{% endhighlight %}
这里如果要逆向排序的话，可以在```ORDER BY```子句的列名后面添加```DESC```关键字。

* ```GROUP BY```子句允许你增加一个```WITH ROLLUP```修饰符，以使在```分组```的基础上有一个更高层的```总结视图```
{% highlight string %}
CREATE TABLE sales
(
year INT,
country VARCHAR(20),
product VARCHAR(32),
profit INT
);

mysql> SELECT year, SUM(profit) AS profit
FROM sales
GROUP BY year WITH ROLLUP;
+------+--------+
| year | profit |
+------+--------+
| 2000 | 4525 |
| 2001 | 3010 |
| NULL | 7535 |
+------+--------+
{% endhighlight %}


**2) SELECT ... INTO 语法**

```SELECT ... INTO```形式使得```SELECT```能够将查询结果存入变量或文件：

* SELECT ... INTO var_list: 用于查询列并将查询值存入变量

* SELECT ... INTO OUTFILE: 将查询到的值写入一个文件。可以指定列和行的结束符以格式话输出到文件。注意这里是将查询出来的数据写入到```MySQL server```机器上的某一个文件中，文件必须要存在并且能够被访问。另外，假如你想要将查询出的结果保存到一个远程客户端，那么你必须在远程客户端使用```MySQL Client```软件连接上SQL Server,然后通过```mysql -e "SELECT ..." > file_name```来将文件保存在客户端宿主机上。关于更详细的数据导入导出相关语法我们后序会进行介绍，这里只给出一个例子：
{% highlight string %}
SELECT a,b,a+b INTO OUTFILE '/tmp/result.txt'
FIELDS TERMINATED BY ',' OPTIONALLY ENCLOSED BY '"'
LINES TERMINATED BY '\n'
FROM test_table;
{% endhighlight %}

* SELECT ... INTO DUMPFILE: 将查询出来的```单独一行```数据写入到文件，不进行任何格式化


## 6. 示例

### 6.1 GROUP BY的使用
```GROUP BY```语句根据一个或多个列对结果进行分组。在分组的列上我们可以使用COUNT、SUM、AVG等函数。

**1)  构建示例表**

将下面的语句写入到文件```employee_tbl.sql```：
{% highlight string %}
SET NAMES utf8;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
--  Table structure for `employee_tbl`
-- ----------------------------
DROP TABLE IF EXISTS `employee_tbl`;
CREATE TABLE `employee_tbl` (
  `id` int(11) NOT NULL,
  `name` char(10) NOT NULL DEFAULT '',
  `date` datetime NOT NULL,
  `singin` tinyint(4) NOT NULL DEFAULT '0' COMMENT '登录次数',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
--  Records of `employee_tbl`
-- ----------------------------
BEGIN;
INSERT INTO `employee_tbl` VALUES ('1', '小明', '2016-04-22 15:25:33', '1');
INSERT INTO `employee_tbl` VALUES ('2', '小王', '2016-04-20 15:25:47', '3');
INSERT INTO `employee_tbl` VALUES ('3', '小丽', '2016-04-19 15:26:02', '2');
INSERT INTO `employee_tbl` VALUES ('4', '小王', '2016-04-07 15:26:14', '4');
INSERT INTO `employee_tbl` VALUES ('5', '小明', '2016-04-11 15:26:40', '4');
INSERT INTO `employee_tbl` VALUES ('6', '小明', '2016-04-04 15:26:54', '2');
COMMIT;

SET FOREIGN_KEY_CHECKS = 1;
{% endhighlight %}
然后登录数据库，导入```employ_tbl.sql```,从而创建测试表：
{% highlight string %}
# mysql -uroot -ptestAa@123
mysql> use test;
mysql> source /home/ivan1001/test-src/employee_tbl.sql
Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.02 sec)

Query OK, 0 rows affected (0.20 sec)

Query OK, 0 rows affected (0.07 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)

Query OK, 1 row affected (0.00 sec)

Query OK, 0 rows affected (0.01 sec)

Query OK, 0 rows affected (0.00 sec)

mysql> select * from employee_tbl;
+----+--------+---------------------+--------+
| id | name   | date                | singin |
+----+--------+---------------------+--------+
|  1 | 小明   | 2016-04-22 15:25:33 |      1 |
|  2 | 小王   | 2016-04-20 15:25:47 |      3 |
|  3 | 小丽   | 2016-04-19 15:26:02 |      2 |
|  4 | 小王   | 2016-04-07 15:26:14 |      4 |
|  5 | 小明   | 2016-04-11 15:26:40 |      4 |
|  6 | 小明   | 2016-04-04 15:26:54 |      2 |
+----+--------+---------------------+--------+
6 rows in set (0.03 sec)
{% endhighlight %}

**2) 使用GROUP BY语句分组查询**
{% highlight string %}
mysql> SELECT NAME, COUNT(*) FROM employee_tbl GROUP BY name;
+--------+----------+
| NAME   | COUNT(*) |
+--------+----------+
| 小丽   |        1 |
| 小明   |        3 |
| 小王   |        2 |
+--------+----------+
3 rows in set (0.12 sec)
{% endhighlight %}

**3) 使用WITH ROLLUP**

```WITH ROLLUP```可以实现分组统计数据基础上再进行相同的统计(SUM/AVG/COUNT)。例如我们将以上的数据表按名字进行分组，再统计每个人登录的次数：
{% highlight string %}
mysql> SELECT name, SUM(singin) as singin_count FROM  employee_tbl GROUP BY name WITH ROLLUP;
+--------+--------------+
| name   | singin_count |
+--------+--------------+
| 小丽   |            2 |
| 小明   |            7 |
| 小王   |            7 |
| NULL   |           16 |
+--------+--------------+
4 rows in set (0.08 sec)
{% endhighlight %}

其中记录```NULL```表示所有人的登录次数。我们可以使用```coalesce```来设置一个可取代```NULL```的名称， ```coalesce```的语法：
<pre>
select coalesce(a,b,c);
</pre>
参数说明： 如果a==NULL，则选择b； 如果b==NULL，则选择c；如果都为NULL，则返回NULL

以下实例中如果名字为NULL，我们使用```总数```来替代：
{% highlight string %}
mysql> SELECT coalesce(name,'总数'), SUM(singin) as singin_count FROM  employee_tbl GROUP BY name WITH ROLLUP;
+-------------------------+--------------+
| coalesce(name,'总数')   | singin_count |
+-------------------------+--------------+
| 小丽                    |            2 |
| 小明                    |            7 |
| 小王                    |            7 |
| 总数                    |           16 |
+-------------------------+--------------+
4 rows in set (0.03 sec)
{% endhighlight %}


### 6.2 MySQL NULL值处理
我们已经知道，MySQL使用SQL SELECT 命令及WHERE子句来读取数据表中的数据，但是当提供的查询条件字段为```NULL```时，该命令可能就无法正常工作。为了处理这种情况，MySQL提供了三大运算符：

* **IS NULL**: 当列的值为NULL，此运算符返回true

* **IS NOT NULL**: 当列的值不为NULL，此运算符返回true

* **<=>**: 安全的NULL比较操作符(不同于```=```运算符)，当比较两个NULL值时返回true

在MySQL中，NULL通过```=```运算符与任何数比较都返回```NULL```:
{% highlight string %}
mysql> select 0=NULL, 1=NULL, NULL=NULL, 1<>NULL, 0<>NULL, NULL<>NULL;
+--------+--------+-----------+---------+---------+------------+
| 0=NULL | 1=NULL | NULL=NULL | 1<>NULL | 0<>NULL | NULL<>NULL |
+--------+--------+-----------+---------+---------+------------+
|   NULL |   NULL |      NULL |    NULL |    NULL |       NULL |
+--------+--------+-----------+---------+---------+------------+
1 row in set (0.00 sec)
{% endhighlight %}

**1) 示例**

尝试以下实例：
{% highlight string %}
mysql> use test;
Database changed
mysql> CREATE TABLE runoob_test_tbl (
    -> runoob_author varchar(40) NOT NULL,
    -> runoob_count int
    -> )ENGINE InnoDB DEFAULT CHARACTER SET utf8;
Query OK, 0 rows affected (0.54 sec)

mysql> INSERT INTO runoob_test_tbl (runoob_author, runoob_count) VALUES ('RUNOOB', 20);
mysql> INSERT INTO runoob_test_tbl (runoob_author, runoob_count) VALUES ('菜鸟教程', NULL);
mysql> INSERT INTO runoob_test_tbl (runoob_author, runoob_count) VALUES ('Google', NULL);
mysql> INSERT INTO runoob_test_tbl (runoob_author, runoob_count) VALUES ('FK', 20);

mysql> select * from runoob_test_tbl;
+---------------+--------------+
| runoob_author | runoob_count |
+---------------+--------------+
| RUNOOB        |           20 |
| 菜鸟教程      |         NULL |
| Google        |         NULL |
| FK            |           20 |
+---------------+--------------+
4 rows in set (0.00 sec)
{% endhighlight %}

如下我们可以看到，```=```与```!=```运算符是不起作用的：
{% highlight string %}
mysql> select * from runoob_test_tbl where runoob_count=NULL;
Empty set (0.01 sec)

mysql> select * from runoob_test_tbl where runoob_count!=NULL;
Empty set (0.00 sec)
{% endhighlight %}

要查找表中的数据必须使用```IS NULL```或```IS NOT NULL```或```<=>```操作符：
{% highlight string %}
mysql> select * from runoob_test_tbl where runoob_count IS NULL;
+---------------+--------------+
| runoob_author | runoob_count |
+---------------+--------------+
| 菜鸟教程      |         NULL |
| Google        |         NULL |
+---------------+--------------+
2 rows in set (0.01 sec)

mysql> select * from runoob_test_tbl where runoob_count<=>NULL;
+---------------+--------------+
| runoob_author | runoob_count |
+---------------+--------------+
| 菜鸟教程      |         NULL |
| Google        |         NULL |
+---------------+--------------+
2 rows in set (0.00 sec)

mysql> select * from runoob_test_tbl where runoob_count IS NOT NULL;
+---------------+--------------+
| runoob_author | runoob_count |
+---------------+--------------+
| RUNOOB        |           20 |
| FK            |           20 |
+---------------+--------------+
2 rows in set (0.00 sec)
{% endhighlight %}


### 6.3 MySQL复制表
如果我们要完全复制MySQL的数据表，包括表的结构、索引、默认值等。这里我们介绍一下如何完整的复制MySQL数据表，步骤如下：

* 使用```SHOW CREATE TABLE```命令获取创建数据表的语句，该语句包含了原数据表的结构、索引等；

* 获取表的元数据之后，使用该元数据创建新表

* 如果想复制表的内容，可以使用```INSERT INTO ... SELECT```语句来实现

**1） 获取数据表的完整结构**
{% highlight string %}
mysql> SHOW CREATE TABLE runoob_tbl \G
*************************** 1. row ***************************
       Table: runoob_tbl
Create Table: CREATE TABLE `runoob_tbl` (
  `runoob_id` int(11) NOT NULL AUTO_INCREMENT,
  `runoob_title` varchar(100) NOT NULL DEFAULT '',
  `runoob_author` varchar(40) NOT NULL DEFAULT '',
  `submission_date` date DEFAULT NULL,
  PRIMARY KEY (`runoob_id`),
  UNIQUE KEY `AUTHOR_INDEX` (`runoob_author`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1
1 row in set (0.02 sec)
{% endhighlight %}

**2) 创建新表**

采用上面的获取到的表结构```元数据```，我们来创建新表```clone_tbl```:
{% highlight string %}
CREATE TABLE `clone_tbl` (
  `runoob_id` int(11) NOT NULL AUTO_INCREMENT,
  `runoob_title` varchar(100) NOT NULL DEFAULT '',
  `runoob_author` varchar(40) NOT NULL DEFAULT '',
  `submission_date` date DEFAULT NULL,
  PRIMARY KEY (`runoob_id`),
  UNIQUE KEY `AUTHOR_INDEX` (`runoob_author`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
{% endhighlight %}

**3) 克隆旧表中的数据**

执行完上面的步骤之后，我们通过```INSERT INTO ... SELECT```来将旧表中的数据拷贝到新表：
{% highlight string %}
mysql> INSERT INTO clone_tbl (runoob_id, runoob_title, runoob_author, submission_date) 
    -> SELECT runoob_id, runoob_title, runoob_author, submission_date FROM runoob_tbl;
Query OK, 3 rows affected (0.06 sec)
Records: 0  Duplicates: 0  Warnings: 0
{% endhighlight %}






<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)



<br />
<br />
<br />

