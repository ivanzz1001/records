---
layout: post
title: mysql数据库基础（三）
tags:
- database
categories: database
description: mysql数据库基础
---


本章包含两个方面的内容：

* MySQL服务器模式

* MySQL字符集与校对集


<!-- more -->

## 1. MySQL服务器模式
不同的MySQL客户端可以通过不同的模式操作MySQL Server。DBA可以设置一个全局模式，而每个应用程序可以根据需要为相应的会话设置不同的模式。

MySQL操作模式会影响到SQL的语法和相应的SQL语句的校验。

### 1.1 设置SQL模式
默认情况下SQL的模式是```NO_ENGINE_SUBSTITUTION```。如果要在MySQL Server启动的时候就设置好相应的SQL模式的话，可以使用```--sql-mode=<modes>```命令行选项来进行设置，也可以在MySQL配置文件中通过```sql-mode=<modes>```来进行配置。```<modes>```是由一系列由空格分隔的不同的模式组成。如果要清除SQL模式，则只需要在启动时传递```--sql-mode=""```或者在配置文件中配置```sql-mode=""```。

如果需要在运行时更改SQL模式，那么可以设置```全局```与```会话``` sql_mode系统变量：
<pre>
SET GLOBAL sql_mode = 'modes';
SET SESSION sql_mode = 'modes';
</pre>
对于设置全局变量，需要有```SUPER```权限，并且会影响到之后连接的所有客户端。对于设置```会话```变量，则只会影响到当前客户端。每个客户端都可以改变该会话的sql_mode。

可以通过如下命令来获取当前```全局```及```会话```sql_mode:
<pre>
SELECT @@GLOBAL.sql_mode;
SELECT @@SESSION.sql_mode;
</pre>

### 1.2 最重要的SQL模式
SQL有很多模式，下面我们介绍几种常用的重要的SQL模式：

* ANSI: 该模式会改变相应的语法和操作行为，以使最接近标准的SQL。它是一种特殊的```组合模式```(combination modes)。

* STRICT_TRANS_TABLES: 假如一个值并不能插入到一个“事务表”中，那么中断该语句的执行。

* TRANDITIONAL： 使MySQL接近于传统的SQL数据库系统。简单的描述即为“在插入错误的值到一列时直接返回错误，而不是警告”。




## 2. MySQL字符集与校验集
MySQL内置一系列不同的字符集，你可以选择不同的字符集来存储数据，并根据不同的校对集(collations)来进行数据比较。你可以在MySQL Server层级、database层级、table层级、column层级来设置字符集。

本节主要包含以下内容：

* 字符集与校对集是什么？

* 多级系统默认字符集设置

* 指定字符集与校对集的语法

* 受影响的函数及操作

* Unicode支持

* 可用的字符集及校对集变量

* 为错误消息指定语言

* 为月/日选择本地名称

字符集设置问题不仅会影响数据存储，也会影响客户端程序与MySQL Server之间的通信。假如MySQL Server端设置的字符集并不是系统默认的字符集，这时如果一个客户端应用程序要与Server端进行通信，则客户端必须指定相应的字符编码。例如，要使用utf-8 Unicode字符集，则客户端连接服务器之后需要执行如下语句：
<pre>
SET NAMES 'utf8';
</pre>

更多关于为应用程序设置字符集，及client/server之间字符集的相关问题，我们后序都会进行介绍。

### 2.1 字符集和校对集概述
一个```字符集```就是一系列符号和编码。一个```校对集```就是一个字符集中一系列用于字符比较的规则。下面举个例子：假设我们有一个字母表:A、B、a、b， 我们给每一个字母一个编号： A = 0, B = 1, a = 2, b = 3。这里```A```即为一个符号，数字```0```即为```A```的编码。这4个字母及相应的编码组成一个字符集。

假设我们想要比较两个字符串的值： A 和 B。最简单的方法是查看```A```和```B```的编码(0与1)。这里因为```0<1```，因此我们可以说```A<B```。这里我们即是通过了一个```校对集```(比较字符编码)完成了字符的比较。我们称这个最简单的校对集(基于字符编码的比较）为```binary collation```。

但是假如我们认为大小写字母相等的话，应该如何处理呢？ 这时我们需要两条规则： 1） 将小写字母```a```看成等于```A```， 将小写字母```b```看成等于```B```; 2) 比较相应编码。我们称之为```大小写不敏感```校对。它比```binary collation```稍微复杂一点。

而在实际生活中，大部分字符集都有很多字符： 可能包含整个字母表， 对于一些东方语言（例如中文）则可能包含更多的字符、特殊符号及标点符号。并且大多数的校对集也拥有很多规则， 而不仅仅是区分大小写，也包括是否区分```重音```等。

幸运的是，MySQL可以帮我们处理这些情况：

* 使用不同的字符集来存储字符串

* 使用不同的校对集来比较字符串

* 在同一个server、database、甚至table中可以混合使用多种不同的字符集和校对集

* 可以在任何层级(server、database、table、column）指定字符集和校对集

为了有效的使用这些特性，你必须知道有哪些```字符集```和```校对集```可用，并且知道如何更改默认的字符集与校对集，还需要了解它们是如何影响相关字符串操作和函数的。

### 2.2 MySQL中的字符集和校对集
MySQL Server支持很多字符集，我们可以通过使用```INFORMATION_SCHEMA CHARACTER_SETS```表或者```SHOW CHARACTER SET```命令来查看支持哪些字符集：
{% highlight string %}
mysql> use information_schema;
Database changed
mysql> select * from CHARACTER_SETS limit 10;
+--------------------+----------------------+-----------------------------+--------+
| CHARACTER_SET_NAME | DEFAULT_COLLATE_NAME | DESCRIPTION                 | MAXLEN |
+--------------------+----------------------+-----------------------------+--------+
| big5               | big5_chinese_ci      | Big5 Traditional Chinese    |      2 |
| dec8               | dec8_swedish_ci      | DEC West European           |      1 |
| cp850              | cp850_general_ci     | DOS West European           |      1 |
| hp8                | hp8_english_ci       | HP West European            |      1 |
| koi8r              | koi8r_general_ci     | KOI8-R Relcom Russian       |      1 |
| latin1             | latin1_swedish_ci    | cp1252 West European        |      1 |
| latin2             | latin2_general_ci    | ISO 8859-2 Central European |      1 |
| swe7               | swe7_swedish_ci      | 7bit Swedish                |      1 |
| ascii              | ascii_general_ci     | US ASCII                    |      1 |
| ujis               | ujis_japanese_ci     | EUC-JP Japanese             |      3 |
+--------------------+----------------------+-----------------------------+--------+
10 rows in set (0.00 sec)

mysql> SHOW CHARACTER SET;
+----------+---------------------------------+---------------------+--------+
| Charset  | Description                     | Default collation   | Maxlen |
+----------+---------------------------------+---------------------+--------+
| big5     | Big5 Traditional Chinese        | big5_chinese_ci     |      2 |
| dec8     | DEC West European               | dec8_swedish_ci     |      1 |
| cp850    | DOS West European               | cp850_general_ci    |      1 |
| hp8      | HP West European                | hp8_english_ci      |      1 |
| koi8r    | KOI8-R Relcom Russian           | koi8r_general_ci    |      1 |
| latin1   | cp1252 West European            | latin1_swedish_ci   |      1 |
| latin2   | ISO 8859-2 Central European     | latin2_general_ci   |      1 |
| swe7     | 7bit Swedish                    | swe7_swedish_ci     |      1 |
| ascii    | US ASCII                        | ascii_general_ci    |      1 |
| ujis     | EUC-JP Japanese                 | ujis_japanese_ci    |      3 |
| sjis     | Shift-JIS Japanese              | sjis_japanese_ci    |      2 |
| hebrew   | ISO 8859-8 Hebrew               | hebrew_general_ci   |      1 |
| tis620   | TIS620 Thai                     | tis620_thai_ci      |      1 |
| euckr    | EUC-KR Korean                   | euckr_korean_ci     |      2 |
| koi8u    | KOI8-U Ukrainian                | koi8u_general_ci    |      1 |
| gb2312   | GB2312 Simplified Chinese       | gb2312_chinese_ci   |      2 |
| greek    | ISO 8859-7 Greek                | greek_general_ci    |      1 |
| cp1250   | Windows Central European        | cp1250_general_ci   |      1 |
| gbk      | GBK Simplified Chinese          | gbk_chinese_ci      |      2 |
| latin5   | ISO 8859-9 Turkish              | latin5_turkish_ci   |      1 |
| armscii8 | ARMSCII-8 Armenian              | armscii8_general_ci |      1 |
| utf8     | UTF-8 Unicode                   | utf8_general_ci     |      3 |
| ucs2     | UCS-2 Unicode                   | ucs2_general_ci     |      2 |
| cp866    | DOS Russian                     | cp866_general_ci    |      1 |
| keybcs2  | DOS Kamenicky Czech-Slovak      | keybcs2_general_ci  |      1 |
| macce    | Mac Central European            | macce_general_ci    |      1 |
| macroman | Mac West European               | macroman_general_ci |      1 |
| cp852    | DOS Central European            | cp852_general_ci    |      1 |
| latin7   | ISO 8859-13 Baltic              | latin7_general_ci   |      1 |
| utf8mb4  | UTF-8 Unicode                   | utf8mb4_general_ci  |      4 |
| cp1251   | Windows Cyrillic                | cp1251_general_ci   |      1 |
| utf16    | UTF-16 Unicode                  | utf16_general_ci    |      4 |
| utf16le  | UTF-16LE Unicode                | utf16le_general_ci  |      4 |
| cp1256   | Windows Arabic                  | cp1256_general_ci   |      1 |
| cp1257   | Windows Baltic                  | cp1257_general_ci   |      1 |
| utf32    | UTF-32 Unicode                  | utf32_general_ci    |      4 |
| binary   | Binary pseudo charset           | binary              |      1 |
| geostd8  | GEOSTD8 Georgian                | geostd8_general_ci  |      1 |
| cp932    | SJIS for Windows Japanese       | cp932_japanese_ci   |      2 |
| eucjpms  | UJIS for Windows Japanese       | eucjpms_japanese_ci |      3 |
| gb18030  | China National Standard GB18030 | gb18030_chinese_ci  |      4 |
+----------+---------------------------------+---------------------+--------+
41 rows in set (0.00 sec)
{% endhighlight %}

默认情况下，```SHOW CHARACTER SET```会显示所有可用的字符集。它也支持一个可选的```LIKE```或```WHERE```选项来过滤某种类型的匹配。例如：
{% highlight string %}
mysql> SHOW CHARACTER SET LIKE 'latin%';
+---------+-----------------------------+-------------------+--------+
| Charset | Description                 | Default collation | Maxlen |
+---------+-----------------------------+-------------------+--------+
| latin1  | cp1252 West European        | latin1_swedish_ci |      1 |
| latin2  | ISO 8859-2 Central European | latin2_general_ci |      1 |
| latin5  | ISO 8859-9 Turkish          | latin5_turkish_ci |      1 |
| latin7  | ISO 8859-13 Baltic          | latin7_general_ci |      1 |
+---------+-----------------------------+-------------------+--------+
4 rows in set (0.00 sec)
{% endhighlight %}

一个给定的字符集通常有至少一种校对集，而大多数的字符集都有多种不同的校对集。如果要找出一种字符集有哪些校对集的话，可以使用```INFORMATION_SCHEMA COLLATIONS```表或者通过```SHOW COLLATION```命令。

默认情况下，```SHOW COLLATION```会显示所有可用的校对集，其也可以通过一个可选的```LIKE```或```WHERE```选项来匹配显示哪些校对集。例如，我们要查询默认的字符集```latin1```的校对集，则可以通过如下命令：
{% highlight string %}
mysql> SHOW COLLATION WHERE Charset = 'latin1';
+-------------------+---------+----+---------+----------+---------+
| Collation         | Charset | Id | Default | Compiled | Sortlen |
+-------------------+---------+----+---------+----------+---------+
| latin1_german1_ci | latin1  |  5 |         | Yes      |       1 |
| latin1_swedish_ci | latin1  |  8 | Yes     | Yes      |       1 |
| latin1_danish_ci  | latin1  | 15 |         | Yes      |       1 |
| latin1_german2_ci | latin1  | 31 |         | Yes      |       2 |
| latin1_bin        | latin1  | 47 |         | Yes      |       1 |
| latin1_general_ci | latin1  | 48 |         | Yes      |       1 |
| latin1_general_cs | latin1  | 49 |         | Yes      |       1 |
| latin1_spanish_ci | latin1  | 94 |         | Yes      |       1 |
+-------------------+---------+----+---------+----------+---------+
8 rows in set (0.00 sec)
{% endhighlight %}

关于上表```latin1```校对集的含义如下：

![mysql-collation-latin1](https://ivanzz1001.github.io/records/assets/img/db/mysql_collation_latin1.jpg)

一般来说，```校对集```具有如下的特性：

* 两个不同的字符集，其校对集一般不同；

* 每一个字符集都有一个默认的校对集。例如，```latin1```与```utf8```的默认校对集分别是```latin1_swedish_ci```与``` utf8_general_ci```。可以通过```INFORMATION_SCHEMA CHARACTER_SETS```表或者```SHOW CHARACTER SET```来查询默认的校对集。

* 校对集的名字一般以对应的字符集名字开头，后边跟着一个或多个后缀

当一个字符集拥有多个```校对集```时，也许我们并不清楚使用哪一个校对集更符合我们应用程序的要求。为了避免选择一个不适当的校对集，这时我们通常会选择使用一些有代表性的数据来进行测试验证。

**1) 字符集Repertoire(曲目)**

一个```字符集```的repertoire就是该字符集所包含的一系列字符。字符串表达式拥有一个```repertoire```属性，其可能含有两种值：

* ASCII： 该表达式只能包含Unicode范围为[U+0000, U+007F]之间的字符

* UNICODE： 表达式可以包含Unicode范围为[U+0000, U+10FFFF]之间的字符。这包含了```基本的多语言层```字符(Basic Multilingual Plane, BMP)范围[U+0000, U+FFFF]，也包括BMP范围外的超级字符[U+10000, U+10FFFF]。

```ASCII```范围是```UNICODE```范围的一个子集，因此一个只含```ASCII曲目```的字符串可以安全的转换为```UNICODE曲目```的字符串，或者是另一种是```ASCII```超级的字符集。（注： 除```swe7```之外，所有的字符集都是```ASCII```的超级)。通过使用```repertoire```，使得在表达式中可以进行自动的字符集转换，这样就可以避免MySQL中很多类似```illegal mix of collations```这样的错误。

如下的讨论展示了一些```表达式示例```及与它们对应的repertoires，并且描述了如何使用```repertoire```来改变表达式的评估求值：

* 一个字符串常量的```repertiore```取决于字符串的内容，并且可能与该字符串的```字符集```的repertiore不同：
<pre>
SET NAMES utf8; SELECT 'abc';
SELECT _utf8'def';
SELECT N'MySQL';
</pre>

这里尽管在开始时将字符集设置为了```utf8```，但是实际上字符串所包含的字符均为```ASCII```范围内的字符，因此它们的```repertoire```是```ASCII```而不是```UNICODE```。

* ```ASCII```字符集的列。下表中，```c1```具有```ASCII``` repertoire
<pre>
CREATE TABLE t1 (c1 CHAR(1) CHARACTER SET ascii);
</pre>
下面的列子展示了由于没有一个合适的```repertoire```而导致的错误：
<pre>
CREATE TABLE t1 (
c1 CHAR(1) CHARACTER SET latin1,
c2 CHAR(1) CHARACTER SET ascii);
INSERT INTO t1 VALUES ('a','b');
</pre>
假设没有```repertoire```,将会发生如下的错误：
<pre>
ERROR 1267 (HY000): Illegal mix of collations (latin1_swedish_ci,IMPLICIT)
and (ascii_general_ci,IMPLICIT) for operation 'concat'
</pre>
使用```repertoire```之后，子集可以无损的转换成超级(ascii转换成latin1)，这样就可以正确的返回结果（这是MySQL默认支持的）：
<pre>
+---------------+
| CONCAT(c1,c2) |
+---------------+
| ab            |
+---------------+
</pre>

* 只携带一个字符串参数的Function会继承该参数的```repertoire```。例如```UPPER(_utf8'abc')```具有ascii ```respertoire```,这是因为它的参数具有```ascii``` respertoire。

* 针对一个返回值为字符串的函数（注意：该函数不带字符串参数），则会采用```character_set_connection```（参看p585)作为返回结果的字符集，假如```character_set_connection```是ascii的话，则返回结果```respertoire```是ASCII，否则为```UNICODE```.
<pre>
FORMAT(numeric_column, 4);
</pre>
下面的例子展示了```respertoire```是如何影响MySQL的查询结果的：
<pre>
SET NAMES ascii;
CREATE TABLE t1 (a INT, b VARCHAR(10) CHARACTER SET latin1);
INSERT INTO t1 VALUES (1,'b');
SELECT CONCAT(FORMAT(a, 4), b) FROM t1;
</pre>
假如没有```repertoire```的话，将会发生如下的错误：
<pre>
ERROR 1267 (HY000): Illegal mix of collations (ascii_general_ci,COERCIBLE)
and (latin1_swedish_ci,IMPLICIT) for operation 'concat'
</pre>
而在有```repertoire```的情况下，将能够返回如下的结果：
<pre>
+-------------------------+
| CONCAT(FORMAT(a, 4), b) |
+-------------------------+
| 1.0000b |
+-------------------------+
</pre>




<br />
<br />
**[参看]**:

1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)


<br />
<br />
<br />

