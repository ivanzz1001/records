---
layout: post
title: mysql数据库基础（三）
tags:
- database
categories: database
description: mysql数据库基础
---

本节主要讲述```MySQL字符集与校对集```。

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


<!-- more -->


## 1. 字符集和校对集概述
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

## 2 MySQL中的字符集和校对集
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

### 2.1 字符集Repertoire(曲目)

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

* 带有两个或两个以上string参数的Function会继承```最宽```参数的```repertoire```作为返回结果的repertoire(```UNICODE```比```ASCII```宽)。考虑如下```CONCAT()```调用：
<pre>
CONCAT(_ucs2 X'0041', _ucs2 X'0042')
CONCAT(_ucs2 X'0041', _ucs2 X'00C2')
</pre>
上面第一个调用，返回结果的```repertoire```是```ASCII```，这是因为两个参数都在ascii字符集范围内。而对于第二个调用，返回结果的```repertoire```是```UNICODE```，这是因为第二个参数并不在ascii字符集范围内。

* 函数返回结果的```repertoire```仅仅只受某一个参数字符集和校对集的影响
<pre>
IF(column1 < column2, 'smaller', 'greater')
</pre>
返回结果的```repertoire```是```ASCII```，这是因为两个字符串参数都是```ASCII repertoire```。函数的第一个参数并不会影响返回结果的```repertoire```。

### 2.2 UTF8 For Metadata
这里我们将```utf8 for metadata```翻译为```采用utf8字符集的元数据```。在MySQL中，任何描述一个数据库的数据都被称为```元数据```，这包括：列名、数据库名、用户名、版本名称、和大部分通过```SHOW```命令返回的字符串结果。此外，元数据还包括数据库```INFORMATION_SCHEMA```中所有的table的内容，这是因为这些表本身包含了数据库对象信息。

典型的```metadata```必须满足如下两个需求：

* 所有的元数据必须有相同的字符集。否则，针对```INFORMATION_SCHEMA```数据库中的表不管是做```SHOW```命令操作，还是执行```SELECT```查询操作都可能会运行不正常，这是因为在这些表中即使针对同一列(column)，在不同的行都可能会有不同的字符集。

* 元数据必须包含所有语言中的所有字符。否则，用户很可能不能用他们的本土语言来命名```表```和```列```。

为了满足这两个需求，MySQL采用```UNICODE```字符集（即UTF-8)来存储元数据。这样即使采用本土语言的话也不会造成任何问题。这里你应该记住，```metadata都采用utf8字符集```。

由于元数据上面的这些需求，使得```USER()```、```CURRENT_USER()```、```SESSION_USER()```、```SYSTEM_USER()```、```DATABASE()```和```VERSION()```这些函数的返回结果均采用utf8字符集编码。

MySQL server设置```character_set_system```系统变量的值为元数据的字符集：
{% highlight string %}
mysql> SHOW VARIABLES LIKE 'character_set_system';
+----------------------+-------+
| Variable_name | Value |
+----------------------+-------+
| character_set_system | utf8 |
+----------------------+-------+
{% endhighlight %}
虽然说元数据是采用```UNICODE```来存储的，但是这并不意味着执行查询操作时返回的```column headers```信息以及```DESCRIBE```函数的返回结果都必须与```character_set_system```所设置的字符集一致。当你使用```SELECT column1 FROM t```时，```column1```这个头本身也会从MySQL Server返回给客户端，而其字符集编码是受```character_set_results```系统变量决定的，该变量的默认值为utf8。假如你想要MySQL Server将元数据的结果以不同的字符集返回，那么可以使用```SET NAMES```语句来强制MySQL Server端进行字符集转换。```SET NAMES```命令会设置```character_set_results```系统变量和一些其他的相关系统变量。另一方面，客户端应用程序在从server端收到响应结果后，也可以进行字符集转换。值得指出的是在客户端进行转换通常会更加高效，但是并不能够保证所有的客户端都支持这样的转换。

假如```character_set_results```被设置为了```NULL```，则并不会进行转换，MySQL直接将元数据以其原始的字符集进行返回（即```character_set_system所设置的字符集)。

来自MySQL Server端的错误消息也与metadata一样自动的进行字符集转换。

假如在一条SQL语句中，你正使用类似于```USER()```这样的函数(注： 返回结果为utf8字符集)来比较或赋值，这时你并不需要担心，因为MySQL会自动的帮你进行转换：
<pre>
SELECT * FROM t1 WHERE USER() = latin1_column;
</pre>
上面这条语句之所以可以正常的工作，这是因为在比较之前，```latin1_column```的内容会自动转换成utf8字符集。
<pre>
INSERT INTO t1 (latin1_column) SELECT USER();
</pre>
上面这条语句之所以可以正常工作，是因为在进行赋值之前，```USER()```函数的返回结果会自动转换为latin1字符集。

尽管自动字符集转换本身并不属于SQL标准，标准并未说明任何一个所支持的字符集都是```UNICODE```字符集的子集。但是大家知道```what applies to a superset can apply to a subset```，因此我们相信针对unicode的校对集可以用在其他非Unicode类型的字符串上。

## 3. 设置字符集与校对集
在四个不同的层级（server、database、table、column）都有默认的字符集与校对集设置。在如下章节的描述尽管看起来可能很复杂，但实际上这样一个多级默认值可以让用户很明晰的知道返回结果的字符集。

```CHARACTER SET```子句被用于设置一个字符集。此外,```CHARSET```也被用于设置字符集，与```CHARACTER SET```等价。

字符集问题不仅会影响数据存储，也会影响客户端与MySQL服务器之间的通信。假如一个客户端应用程序与一个非默认字符集的```MySQL Server```进行通信，这时你需要知道确切的字符集。例如，如果要使用```utf8``` unicode字符集，客户端需要在连接MySQL服务器之后执行如下的命令：
<pre>
SET NAMES 'utf8';
</pre>
更多关于client/server通信之间的字符集问题，请参看如下的章节。

### 3.1 校对集命名规范

MySQL校对集的名字通常遵循如下规范：

* 校对集的名字以其所对应的字符集名字开头，后面跟着一个或多个后缀以表明该校对集的特性。例如: ```utf8_general_ci```与```latin1_swedish_cli```是针对```utf8```和```latin1```字符集的校对集。这里需要指出的是，```binary```字符集，它只有一个校对集，且名称也叫```binary```，并没有任何后缀。

* 针对特定语言的校对集包含该语言名称。例如： ```utf8-turkish_ci```与```utf8_hungarian_ci```按utf8字符集```turkish```、```Hungarian```校对集进行排序。

* 校对集的后缀用于指明该校对集是否大小写敏感、音调敏感(accent sensitive)或者是直接binary比较。下表列出了后缀表示的一些含义：

![mysql-collation-sense](https://ivanzz1001.github.io/records/assets/img/db/mysql_collation_sensitive.jpg)

对于非二进制(nonbinary)校对集名称来说，如果没有指定```accent sensitiviey```，那么其就由```case sensitivity```决定。假如一个校对集名称并未包含```_ai```或```_as```，则名字中的```_ci```后缀隐含着```_ai```, 名字中的```_cs```隐含着```_as```。例如，```latin1_general_ci```显示的指明了忽略大小写，同时隐含的指明了```accent insensitive```；而```latin1_general_cs```显示的指明了大小写敏感，同时隐含的指明了```accent sensitive```。

对于```binary```字符集的```binary```校对集，比较是基于字节数字的值来进行的。针对非二进制字符集(nobinary character set)的```_bin```校对集，比较是基于该字符编码的数值来进行的（有一些字符可能占用多个字节），

* 针对unicode字符集来说，校对集的名称也可能会含有一个版本号，以表明采用的是哪个版本的UCA(Unicode Collation Algorithm)。基于```UCA```的校对集假如不包含版本的话，则默认采用的是UCA 4.0.0版本。

* ```utf8_unicode_ci```(并没有版本号）是基于UCA 4.0.0算法

### 3.2 Server级别字符集和校对集
MySQL Server有一个server字符集和server校对集。这可以在MySQL Server启动的时候通过命令行或者配置文件来设置，也可以在运行时通过命令来进行设置。

起初，server的字符集与校对集依赖于你启动ysqld时传入的选项，你可以使用```--character-set-server```来设置server字符集。另外，你也可以在后面添加```--collation-server```来设置校对集。假如你并未进行设置的话，则等价于设置为```--character-set-server=latin1```。假如你只设置了字符集（例如:latin1)，并未设置校对集，则等价于设置:
<pre>
--character-set-server=latin1 --collationserver=latin1_swedish_ci
</pre>
这是因为latin1的默认校对集为```latin1_swedish_ci```。因此，如下三个命令的含义是一致的：
<pre>
mysqld
mysqld --character-set-server=latin1
mysqld --character-set-server=latin1 \
--collation-server=latin1_swedish_ci
</pre>

其中一种方法来改变mysqld启动时的默认值就是通过重新编译源代码：
<pre>
cmake . -DDEFAULT_CHARSET=latin1

或
cmake . -DDEFAULT_CHARSET=latin1 -DDEFAULT_COLLATION=latin1_german1_ci
</pre>
```mysqld```与```cmake```都会检查```字符集/校对集```的组合是否有效，假如无效的话，两个程序都会打印出相关的错误信息并退出。

假如在通过```CREATE DATABASE```子句创建数据库的时候未指定数据库的```字符集```与```校对集```，则默认会采用server的```字符集```与```校对集```。

当前运行的MySQL Server的字符集与校对集是由```character_set_server```与```collation_server```这两个系统变量决定的，这些值可以在运行时被更改。


### 3.3 数据库级别字符集与校对集
每一个数据库都有一个数据库级别的字符集与校对集。可以在执行```CREATE DATABASE```或者```ALTER DATABASE```的时候添加相应的选项来指定该数据库的字符集与校对集:
<pre>
CREATE DATABASE db_name
[[DEFAULT] CHARACTER SET charset_name]
[[DEFAULT] COLLATE collation_name]
ALTER DATABASE db_name
[[DEFAULT] CHARACTER SET charset_name]
</pre>
这里假如将关键字```DATABASE```换成```SCHEMA```,则是创建数据库schema。

所有的数据库选项都存放在一个名为```db.opt```的文本文件中，该文件可以在数据库目录中找到。上面的```CHARACTER SET```与```COLLATE```子句可以使得在创建数据库时指定不同的字符集与校对集。例如：
<pre>
CREATE DATABASE db_name CHARACTER SET latin1 COLLATE latin1_swedish_ci;
</pre>

MySQL按如下的步骤来选择的数据库的字符集与校对集：

* 假如```CHARACTER SET charset_name```与```COLLATE collation_name```都被指定，则charset_name字符集、collation_name校对集会被使用；

* 假如只指定了```CHARACTER SET charset_name```，而并未指定```COLLATE```，那么charset_name字符集会被使用，并采用该字符集默认的校对集。要查看一个字符集所对应的默认校对集可以通过执行```SHOW CHARACTER SET```命令或者查询```INFORMATION_SCHEMA CHARACTER_SETS```表。

* 假如只指定了```COLLATE collation_name```，并未指定```CHARACTER SET```，则与collation_name所关联的字符集会被使用，校对集使用collation_name。

* 否则（```CHARACTER SET```与```COLLATE```均为被指定），采用server的字符集与校对集

```默认数据库```(default database)的字符集与校对集由```character_set_database```与```collation_database```这两个系统变量决定。无论什么时候默认数据库改变，MySQL server都会去设置这两个值。假如没有默认数据库的话，则这两个值与系统级别的```character_set_server```与```collation_server```相同。

要查看一个数据库的默认字符集与校对集，可以通过如下的命令：
<pre>
USE db_name;
SELECT @@character_set_database, @@collation_database;
</pre>
另外，如果不想切换数据库，那么可以通过如下的命令来查看：
<pre>
SELECT DEFAULT_CHARACTER_SET_NAME, DEFAULT_COLLATION_NAME
FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = 'db_name';
</pre>

数据库的```字符集```与```校对集```会从以下方面影响MySQL Server的操作：

* 对于```CREATE TABLE```语句，假如在创建表的时候并未指定字符集与校对集，则会继承所对应库的字符集与校对集。如果要覆盖该继承，则需要在创建表的时候显示通过```CHARACTER SET```与```COLLATE```来指定。

* 针对```LOAD DATA```命令，如果并未指定```CHARACTER SET```子句，则MySQL server会使用```character_set_database```系统变量所指定的字符集来解释文件内容。如果要覆盖此字符集，需要提供一个显示的```CHARACTER SET```子句。

* 针对存储方法（存储过程或函数），在该方法创建时并未指定```CHARACTER SET```与```COLLATE```属性的话，那么则会采用数据库的字符集与校对集。

### 3.4 表级别字符集与校对集
每一个表都有对应的字符集与校对集。在```CREATE TABLE```或者```ALTER TABLE```时可以加上字符集与校对集相关的选项：
<pre>
CREATE TABLE tbl_name (column_list)
[[DEFAULT] CHARACTER SET charset_name]
[COLLATE collation_name]]
ALTER TABLE tbl_name
[[DEFAULT] CHARACTER SET charset_name]
[COLLATE collation_name]
</pre>

例如：
{% highlight string %}
CREATE TABLE t1 ( ... )
CHARACTER SET latin1 COLLATE latin1_danish_ci;
{% endhighlight %}
MySQL会按照如下的方式来选择表的字符集与校对集：

* 假如在创建表时同时指定了```CHARACTER SET charset_name```与```COLLATE collation_name```，则所指定的charset_name及collation_name会被使用；

* 假如创建表时只指定了```CHARACTER SET charset_name```，并未指定```COLLATE```，那么charset_name默认的校对集将会被使用。要查看一个字符集所对应的默认校对集可以通过执行```SHOW CHARACTER SET```命令或者查询```INFORMATION_SCHEMA CHARACTER_SETS```表。

* 假如只指定了```COLLATE collation_name```，并未指定```CHARACTER SET```，则与collation_name所关联的字符集会被使用，校对集使用collation_name。

* 否则（```CHARACTER SET```与```COLLATE```均为被指定），采用database的字符集与校对集


假如一个表的列(column)并未指定字符集与校对集的话，那么将会使用该表对应的```字符集```与```校对集```。注意：表的字符集与校对集是MySQL的扩展，标准的SQL是没有表级别的字符集与校对集的。

### 3.5 列级别字符集与校对集
每一个```character```列(即类型为```CHAR```、```VARCHAR```或```TEXT```)都有一个```列级别```的字符集与校对集。可以在定义列时加上字符集与校对集选项：
<pre>
col_name {CHAR | VARCHAR | TEXT} (col_length)
[CHARACTER SET charset_name]
[COLLATE collation_name]
</pre>
这些子句也可以用于```ENUM```或者```SET```列：
<pre>
col_name {ENUM | SET} (val_list)
[CHARACTER SET charset_name]
[COLLATE collation_name]
</pre>
例如：
{% highlight string %}
CREATE TABLE t1
(
col1 VARCHAR(5)
CHARACTER SET latin1
COLLATE latin1_german1_ci
);
ALTER TABLE t1 MODIFY
col1 VARCHAR(5)
CHARACTER SET latin1
COLLATE latin1_swedish_ci;
{% endhighlight %}

MySQL会按照如下的方式来选择表的字符集与校对集：

* 假如在创建列时同时指定了```CHARACTER SET charset_name```与```COLLATE collation_name```，则所指定的charset_name及collation_name会被使用；
<pre>
CREATE TABLE t1
(
col1 CHAR(10) CHARACTER SET utf8 COLLATE utf8_unicode_ci
) CHARACTER SET latin1 COLLATE latin1_bin;
</pre>
上面指定了该列的字符集与校对集，因此会使用```utf8```字符集```utf8_unicode_ci```校对集。


* 假如创建列时只指定了```CHARACTER SET charset_name```，并未指定```COLLATE```，那么charset_name默认的校对集将会被使用。
<pre>
CREATE TABLE t1
(
col1 CHAR(10) CHARACTER SET utf8
) CHARACTER SET latin1 COLLATE latin1_bin;
</pre>
这里在创建列时只指定了字符集，并未指定校对集。因此会使用utf8字符集，采用utf8默认的校对集```utf8_general_ci```。。要查看一个字符集所对应的默认校对集可以通过执行```SHOW CHARACTER SET```命令或者查询```INFORMATION_SCHEMA CHARACTER_SETS```表。

* 假如只指定了```COLLATE collation_name```，并未指定```CHARACTER SET```，则与collation_name所关联的字符集会被使用，校对集使用collation_name。
<pre>
CREATE TABLE t1
(
col1 CHAR(10) COLLATE utf8_polish_ci
) CHARACTER SET latin1 COLLATE latin1_bin;
</pre>
这里在创建列时只指定了校对集，并未指定字符集。校对集是```utf8_polish_ci```，与该校对集关联的字符集是```utf8```。

* 否则（```CHARACTER SET```与```COLLATE```均为被指定），则继承table的字符集与校对集
<pre>
CREATE TABLE t1
(
col1 CHAR(10)
) CHARACTER SET latin1 COLLATE latin1_bin;
</pre>
上面创建列时，并未指定字符集与校对集，因此采用表级别的字符集```latin1```与校对集```latin1_bin```。

```CHARACTER SET```与```COLLATE```是标准SQL所支持的。假如你使用```ALTER TABLE```来将一列从一种字符集转换成另一种字符集，MySQL首先会阐释映射该值，但是假如字符集不兼容的话，则可能产生数据丢失。

### 3.6 字符串常量的字符集与校对集
每一个字符串常量都有一个对应的字符集与校对集。





<br />
<br />
**[参看]**:

1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

2. [MySQL 里设置或修改系统变量的几种方法](https://www.cnblogs.com/devcjq/articles/6409470.html)

3. [MySQL 5.6 Reference Manual SET/SHOW的使用(p1842)]()

<br />
<br />
<br />

