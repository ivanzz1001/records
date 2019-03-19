---
layout: post
title: mysql字符集与校对集
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
这里在创建列时只指定了字符集，并未指定校对集。因此会使用utf8字符集，采用utf8默认的校对集```utf8_general_ci```。要查看一个字符集所对应的默认校对集可以通过执行```SHOW CHARACTER SET```命令或者查询```INFORMATION_SCHEMA CHARACTER_SETS```表。

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

对于```SELECT 'string'```这样一个简单的子句，string具有该连接(connection)所默认的字符集与校对集，连接默认的字符集与校对集可以通过系统变量```character_set_connection```和```collation_connection```来获取。

一个字符串常量也有一个可选的```字符集说明```（introducer)与```COLLATE```子句，这样可以表明该字符串常量所采用的字符集与校对集：
<pre>
[_charset_name]'string' [COLLATE collation_name]
</pre>
这里```_charset_name```就是上面提到的```introducer```。它告诉MySQL分析器“接下来的字符串使用的是charset_name字符集”。这里需要注意的是，```introducer```只是一个指示作用，并不会像```CONVERT()```函数那样将该字符串转换为```introducer```字符集。例如：
<pre>
SELECT 'abc';
SELECT _latin1'abc';
SELECT _binary'abc';
SELECT _utf8'abc' COLLATE utf8_danish_ci;
</pre>
字符集```introducer```和```COLLATE```子句的实现是依据标准SQL文档的。

MySQL会按照如下的方式来决定一个```字符串常量```的字符集与校对集：

* 假如同时指定了```_charset_name```与```COLLATE collation_name```，则字符集```charset_name```和校对集```collation_name```将会被使用。```collation_name```必须是该```charset_name```所支持的校对集。

* 假如```_charset_name```被指定，但是```COLLATE```未被指定的话，则字符集```charset_name```与其默认的校对集将会被使用。要查看一个字符集所对应的默认校对集可以通过执行```SHOW CHARACTER SET```命令或者查询```INFORMATION_SCHEMA CHARACTER_SETS```表。


* 假如只指定了```COLLATE collation_name```，但是并未指定```charset_name```，那么由```character_set_connection```系统变量所指定的默认字符集与```collation_name```校对集将会被使用。这里注意```collation_name```必须要被该默认字符集所支持。

* 否则（```_charset_name```与```COLLATE collation_name```均未被指定），由系统变量```character_set_connection```与```collation_connection```所指定的默认字符集与校对集将会被使用。

例如：

* 针对一个非二进制字符串，指定了```latin1```字符集和```latin1_german1_ci```:
<pre>
SELECT _latin1'Müller' COLLATE latin1_german1_ci;
</pre>


* 针对一个非二进制字符串，指定了```utf8```字符集和默认的校对集(utf8默认的校对集是```utf8_general_ci```)：
<pre>
SELECT _utf8'Müller';
</pre>

* 二进制字符串采用```binary```字符集和其默认的校对集(binary):
<pre>
SELECT _binary'Müller';
</pre>

* 非二进制字符串采用该连接默认的字符集和```utf8_general_ci```校对集（注意：假如连接默认的字符集不是utf8的话，将会失败):
<pre>
SELECT 'Müller' COLLATE utf8_general_ci;
</pre>

* 使用连接默认的字符集与校对集：
<pre>
SELECT 'Müller';
</pre>

```introducer```用于指示接下来的字符串常量所采用的字符集，但是并不会改变SQL解析器对该字符串常量转义的处理，转义总是由解析器根据```character_set_connection```指定的字符集来进行处理。

下面的例子显示了使用```character_set_connection```字符集来处理转义。该例子使用```SET NAMES```来改变```character_set_connection```字符集，然后通过```HEX()```函数来显示结果（这样就可以看到确切的返回结果）：

**例1**
{% highlight string %}
mysql> SET NAMES latin1;
mysql> SELECT HEX('à\n'), HEX(_sjis'à\n');
+------------+-----------------+
| HEX('à\n') | HEX(_sjis'à\n') |
+------------+-----------------+
| E00A | E00A |
+------------+-----------------+
{% endhighlight %}
这里,```à```(十六进制值是E0)后面跟着一个```'\n'```，该转义字符表示一个换行符。该转义序列通过```character_set_connection```所设置的```latin1```字符集被解释成为一个换行符(十六进制值为0A)。这即使在第二个字符串```_sjis'à\n'```也是正确的。也就是说,```_sjis```这个introducer并不会影响解析器对转义字符的解析。

**例2**
{% highlight string %}
mysql> SET NAMES sjis;
mysql> SELECT HEX('à\n'), HEX(_latin1'à\n');
+------------+-------------------+
| HEX('à\n') | HEX(_latin1'à\n') |
+------------+-------------------+
| E05C6E | E05C6E |
+------------+-------------------+
{% endhighlight %}

这里```character_set_connection```是```sjis```，在该字符集下序列```à\```(十六进制是E0和5C)是一个有效的宽字节字符。因此，开始的两个字节会被解释成为一个单独的```sjis```字符，这里```\```并不会被解释为转义字符。接下来的```n```(十六进制为6E)也不再是转义序列的一部分。因此这里可以看到```_latin1```这个introducer并不会影响解析器对转义字符的处理。

### 3.7 The National Character Set
标准的SQL定义了```NCHAR```和```NATIONAL CHAR```以指示类型为```CHAR```的列使用一些预定义好的字符集。在MySQL中使用```utf8```作为预定好的字符集。因此如下的例子是等价的：
<pre>
CHAR(10) CHARACTER SET utf8
NATIONAL CHARACTER(10)
NCHAR(10)
</pre>
下面的例子也是等价的：
<pre>
VARCHAR(10) CHARACTER SET utf8
NATIONAL VARCHAR(10)
NVARCHAR(10)
NCHAR VARCHAR(10)
NATIONAL CHARACTER VARYING
</pre>

你也可以使用```N'literal'```(或```n'literal'```)来创建一个national字符集。例如：
<pre>
SELECT N'some text';
SELECT n'some text';
SELECT _utf8'some text';
</pre>
上面三个是等价的。

### 3.8 Character Set Introducers
一个string literal、hexadecimal literal、bit-value literal都可以添加一个字符集说明符和```COLLATE```子句。语法如下：
<pre>
[_charset_name] literal [COLLATE collation_name]
</pre>
例如：
{% highlight string %}
SELECT 'abc';
SELECT _latin1'abc';
SELECT _binary'abc';
SELECT _utf8'abc' COLLATE utf8_danish_ci;
SELECT _latin1 X'4D7953514C';
SELECT _utf8 0x4D7953514C COLLATE utf8_danish_ci;
SELECT _latin1 b'1000001';
SELECT _utf8 0b1000001 COLLATE utf8_danish_ci;
{% endhighlight %}
而MySQL对决定这些字面量的字符集与校对集的方法与```字符串常量```的类似，这里不再赘述。

### 3.9 指定字符集与校对集示例
**1) 例1**
{% highlight string %}
CREATE TABLE t1
(
c1 CHAR(10) CHARACTER SET latin1 COLLATE latin1_german1_ci
) DEFAULT CHARACTER SET latin2 COLLATE latin2_bin;
{% endhighlight %}
这里我们给列```c1```指定了latin1字符集和```latin1_german1_ci```校对集。这里在定义中显示的直接指定。需要注意的是，我们这里将一个字符集为```latin1```的列存放在字符集为```latin2```的表中，这样并不会产生任何问题。

**2) 例2**
{% highlight string %}
CREATE TABLE t1
(
c1 CHAR(10) CHARACTER SET latin1
) DEFAULT CHARACTER SET latin1 COLLATE latin1_danish_ci;
{% endhighlight %}
这里我们对列```c1```使用```latin1```字符集和默认的校对集。这里的默认校对集是指```latin1```对应的默认校对集```latin1_swedish_ci```.

**3) 例3**
{% highlight string %}
CREATE TABLE t1
(
c1 CHAR(10)
) DEFAULT CHARACTER SET latin1 COLLATE latin1_danish_ci;
{% endhighlight %}
我们这里定义了一列```c1```，其拥有默认的字符集与校对集。在这种情况下，MySQL会检查表级别的字符集与校对集，因此这里列```c1```的的字符集是```latin1```、校对集是```latin1_danish_ci```。

**4） 例4**
{% highlight string %}
CREATE DATABASE d1
DEFAULT CHARACTER SET latin2 COLLATE latin2_czech_ci;
USE d1;
CREATE TABLE t1
(
c1 CHAR(10)
);
{% endhighlight %}
这里我们创建列```c1```时并未指定任何的字符集与校对集。我们也并未在表级别指定字符集与校对集，在这种情况下，MySQL会检查数据库级别的字符集与校对集，从而决定表级别的字符集与校对集，然后再决定列级别。因此，这里列```c1```的字符集是```latin2```、校对集是```latin2_czech_ci```。

## 4. 连接(Connection)的字符集与校对集
一个```connection```就是指一个客户端应用程序连接到MySQL服务器，并与其所交互的服务器开启的一个会话。客户端通过该会话连接发送SQL语句，例如查询语句。然后服务器通过该连接返回响应结果或者错误消息。本节主要包括如下几个方面的内容：

* 连接字符集与校对集系统变量(connection character set and collation system variables)

* 用于配置连接字符集的SQL语句（SQL Statements for Connection Character Set Configuration)

* 客户端应用程序连接字符集配置（Client Program Connection Character Set Configuration)

* 用于错误处理的连接字符集配置（Connection Character Set Configuration Error Handling)

### 4.1 Connection字符集与校对集系统变量
有多个MySQL客户端与服务端交互相关的字符集/校对集系统变量。其中的一些我们前面提到过：

* ```character_set_server```和```collation_server```系统变量用于指示服务器端的字符集与校对集；

* ```character_set_database```和```collation_database```系统变量用于指示```默认数据库```的字符集与校对集；

另外，字符集与校对集系统变量也涉及到client/server之间的消息处理。每一个客户端都有一个针对该连接的会话字符集与校对集系统变量(session-specific character set and collation)。这些会话系统变量会在连接的时候被初始化，但是可以在会话过程中被更改。

有多个关于客户端连接(client connection)方面的字符集与校对集方面的问题，可以通过系统变量来进行回答：

* 当SQL语句离开客户端之后，它是采用什么样的字符集？ 

MySQL Server会使用```character_set_client```系统变量来决定客户端发送过来的SQL语句的字符集。

* 在服务器接收到客户端发送过来的SQL语句后，MySQL Server采用何种字符集来解析它们？

为了确定要用何种字符集，MySQL Server会根据```character_set_connection```和```collation_connection```系统变量：

1） MySQL Server会将Client发送的SQL语句从```character_set_client```转换成```character_set_connection```字符集。这里有一个特例需要注意，针对带有```introducer```的字符串字面量，比如```_utf8mb4```或者```_latin2```，此时MySQL Server采用```introducer```所指定的字符集与校对集来解析该字符串字面量。

2） ```collation_connection```对于比较字符串字面量是很重要的。对于列(column)类型是字符串的值，采用的是该列本身的校对集来比较，而不是用```collation_connection```指定的校对集。


* 在MySQL将查询结果返回给客户端之前，应当对查询结果采用何种字符集？

```character_set_results```系统变量用于决定MySQL Server采用何种字符集将查询结果返回给客户端。返回值包括：result data(比如column values)、results metadata(比如column names)、错误消息(error messages)。

<br />
客户端可以对这些系统变量值进行微调，或者是直接采用默认值。假如你不想使用默认值，你必须为每一个连接到服务器连接进行字符设置。

### 4.2 配置连接字符集的SQL语句
有两个SQL语句可用于设置连接相关的字符集系统变量：

* SET NAMES 'charset_name' [COLLATE 'collation_name']

这里```SET NAMES```用于指示将采用何种字符集向MySQL服务器发送SQL语句。因此，```SET NAMES 'cp1251'```这样一条SQL语句告诉MySQL服务器，后序从本客户端发送出去的消息都采用```cp1251```字符集。它也用于指定服务器应该以何种字符集来返回结果信息（例如，当你执行```SELECT```语句返回查询结果的时候，其用于指定采用何种字符集）

一条```SET NAMES 'charset_name'```语句等价于如下三条语句：
<pre>
SET character_set_client = charset_name;
SET character_set_results = charset_name;
SET character_set_connection = charset_name;
</pre>
这里设置```character_set_connection```为charset_name，也隐含着将```collation_connection```设置为charset_name所默认的校对集。我们一般并不需要显示的设定collation。我们我们要显示的为```collation_connection```设定校对集，可以通过如下：
<pre>
SET NAMES 'charset_name' COLLATE 'collation_name'
</pre>

* SET CHARACTER SET 'charset_name'

```SET CHARACTER SET```类似于```SET NAMES```，但是将```character_set_connection```和```collation_connection```的值设置为```character_set_database```和```collation_database```（即用于设置默认数据库的字符集与校对集）。

一条```SET CHARACTER SET charset_name```语句等价于如下三条语句：
<pre>
SET character_set_client = charset_name;
SET character_set_results = charset_name;
SET collation_connection = @@collation_database;
</pre>
设置```collation_connection```也隐含着设置```character_set_connection```。（因为一个collation默认对应着一个字符集）。这里我们并没有必要显示的设置```character_set_connection```。

<br />
如果要查看当前我们为连接所设置的字符集与校对集，使用如下命令：
{% highlight string %}
SHOW SESSION VARIABLES LIKE 'character\_set\_%';
SHOW SESSION VARIABLES LIKE 'collation\_%';
{% endhighlight %}

另外，假如你并不想要MySQL Server在返回结果或者错误消息时进行字符集相关的转换，那么可以将```character_set_results```设置为```NULL```或者```binary```:
<pre>
SET character_set_results = NULL;
SET character_set_results = binary;
</pre>

### 4.3 配置客户端程序的连接字符集
客户端应用程序如mysql, mysqladmin, mysqlcheck, mysqlimport, 和 mysqlshow按如下的步骤来决定默认字符集：

* 假如没有任何其他信息，默认情况下每个客户端在编译时设置的默认字符集为```latin1```;

* 上述每一个客户端应用都会根据操作系统的设置自动检测使用哪种字符集，例如在Unix系统下检测```LANG```或```LC_ALL```这样的环境变量。

* 上述每一个客户端应用程序都支持一个```--default-character-set```选项，该选项使用户能够显示的指定采用何种字符集。



对于```mysql```这一客户端工具，你可以在配置文件中进行如下配置：
<pre>
[mysql]
default-character-set=koi8r
</pre>
也可以直接在连接上之后，通过执行```SET NAMES```语句进行修改。

## 5. 配置应用程序字符集与校对集
对于应用程序来说，如果采用MySQL默认的字符集与校对集（latin1, latin1_swedish_ci)，那么一般不需要做任何的配置。假如应用程序需要用不同的字符集与校对集来存储数据，那么你可以通过如下几种方式来进行字符集的配置：

* 为每个数据库指定字符设置。
<pre>
CREATE DATABASE mydb
CHARACTER SET utf8
COLLATE utf8_general_ci;
</pre>

* 在MySQL Server启动的时候，为server指定字符集配置；
<pre>
[mysqld]
character-set-server=utf8
collation-server=utf8_general_ci
</pre>

* 在编译MySQL源代码的时候指定默认的字符集配置；
<pre>
cmake . -DDEFAULT_CHARSET=utf8 \
-DDEFAULT_COLLATION=utf8_general_ci
</pre>



<br />
<br />
**[参看]**:

1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)

2. [MySQL 里设置或修改系统变量的几种方法](https://www.cnblogs.com/devcjq/articles/6409470.html)

3. [MySQL 5.6 Reference Manual SET/SHOW的使用(p1842)]()

<br />
<br />
<br />

