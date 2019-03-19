---
layout: post
title: mysql常用函数及操作符
tags:
- database
categories: database
description: mysql数据库基础
---


本章我们主要介绍一下MySQL中的一些常用函数和操作符。


<!-- more -->


## 1. 表达式求值中的类型转换
当一个```操作符```被作用于一个不同类型的```操作数```时，会进行一个自动的转换，以使该操作数能与操作符向兼容。其中有一些转换会隐式的发生。例如，下面的例子会自动地将字符串转变成数字，或数字转变成字符串：
{% highlight string %}
mysql> SELECT 1+'1';
-> 2
mysql> SELECT CONCAT(2,' test');
-> '2 test'
{% endhighlight %}
此外，也可以通过```CAST()```函数显示的将一个数字转换成字符串，```CONCAT()```函数可以隐式地转换成字符串类型：
{% highlight string %}
mysql> SELECT 38.8, CAST(38.8 AS CHAR);
-> 38.8, '38.8'
mysql> SELECT 38.8, CONCAT(38.8);
-> 38.8, '38.8'
{% endhighlight %}

在本章的后边将会介绍关于数字向字符串的隐式转换的一些字符集信息，和一些应用于```CREATE TABLE ... SELECT```子句的修改规则。

下面的一些规则描述了在进行```比较操作```时是如何进行类型转换的：

* 假如一个或两个参数均为NULL， 则比较结果为```NULL```。注意除了```NULL-saf```e的```<=>```相等比较操作符，对于```NULL<=>NULL```,返回结果为true,并不需要进行转换。

* 在一个比较操作符中，假如两个参数都是string类型，则它们作为字符串来进行比较。

* 在一个比较操作符中，假如两个参数都是整数类型，则作为整数类型来比较

* 假如一个16进制值不是和数字进行比较的话，那么将会把其看成是二进制字符串(binary strings)

* 假如一个参数是```TIMESTAMP```或```DATETIME```列，而另一个参数是常量，则在进行比较操作之前会将该常量转换成timestamp类型。这样做对```ODBC```更加友好。但是当作为```IN()```函数的参数时，将不会自动的进行转换。为了安全，在进行比较操作时总是用完整的datetime、date、time所对应的字符串来比较较好。例如：当使用```BETWEEN()```函数来比较```date```或```time```类型的值时，最好将两个参数都转换成所期望的类型。

* 假如其中一个参数是```decimal```值，比较操作取决于另一个操作。假如另一个参数是是```decimal```或```整数```值，那么两者将作为```decimal```来进行比较；如果另一个参数是浮点类型，那么两者将作为浮点类型来进行比较。

* 在所有其他情况下，都作为浮点类型值来进行比较


下面的例子展示了在比较过程中，将字符串转变成数字：
{% highlight string %}
mysql> SELECT 1 > '6x';
-> 0
mysql> SELECT 7 > '6x';
-> 1
mysql> SELECT 0 > 'x6';
-> 0
mysql> SELECT 0 = 'x6';
-> 1
{% endhighlight %}

对于将一个```字符串```类型的列和一个数字进行比较，MySQL在该列上查找值时并不能使用索引。假如```str_col```是一个索引字符串列，在执行如下查找语句时，也不能使用索引：
{% highlight string %}
SELECT * FROM tbl_name WHERE str_col=1;
{% endhighlight %}
这是因为有很多不同的字符串都可能在进行转换之后值为1，例如```1```、``` 1```或```1a```等.

另外对于两个浮点数的比较，由于浮点数本身并不精确，这可能会导致出现不一致的结果：
{% highlight string %}
mysql> SELECT '18015376320243458' = 18015376320243458;
-> 1
mysql> SELECT '18015376320243459' = 18015376320243459;
-> 0
{% endhighlight %}
其中一种方法来避免这种错误就是通过```CAST()```显示的进行类型转换：
{% highlight string %}
mysql> SELECT CAST('18015376320243459' AS UNSIGNED) = 18015376320243459;
-> 1
{% endhighlight %}


## 2. 运算符

![mysql-operator](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_operator.jpg)

另外，还有:

* ```-```: 负数操作符

* ```XOR```: 逻辑或


### 2.1 运算符优先级
运算符优先级```从高到低```如下所示，在同一行的运算符具有相同的优先级：
{% highlight string %}
INTERVAL
BINARY, COLLATE
!
- (unary minus), ~ (unary bit inversion)
^
*, /, DIV, %, MOD
-, +
<<, >>
&
|
= (comparison), <=>, >=, >, <=, <, <>, !=, IS, LIKE, REGEXP, IN
BETWEEN, CASE, WHEN, THEN, ELSE
NOT
AND, &&
XOR
OR, ||
= (assignment), :=
{% endhighlight %}

相同优先级的运算符，具有从左到右的结合性（注： 赋值运算符结合性为从右到左）。


### 2.2 比较函数和操作符

![mysql-function-operator](https://ivanzz1001.github.io/records/assets/img/db/db_mysql_funcoper.jpg)

下面我们对其中一些函数或操作符进行简单介绍：

* ```<=>```: NULL-safe比较操作符。该操作符类似于```=```操作符做相等比较，但是假如比较的两个操作数都是NULL的话返回```1```，只有一个操作数是```NULL```的话则返回```0```.
{% highlight string %}
mysql> SELECT 1 <=> 1, NULL <=> NULL, 1 <=> NULL;
-> 1, 1, 0
mysql> SELECT 1 = 1, NULL = NULL, 1 = NULL;
-> 1, NULL, NULL
{% endhighlight %}

* IS boolean_value： 用于测试一个值是否是给定的布尔值，这里```boolean_value```可以为```TRUE```、```FALSE```、或者```UNKNOWN```。
{% highlight string %}
mysql> SELECT 1 IS TRUE, 0 IS FALSE, NULL IS UNKNOWN;
-> 1, 1, 1
{% endhighlight %}

* IS NULL: 用于测试一个值是否为NULL
{% highlight string %}
mysql> SELECT 1 IS NULL, 0 IS NULL, NULL IS NULL;
-> 0, 0, 1
{% endhighlight %}


* expr BETWEEN min AND max: 假如```expr```大于等于min，并且小于等于max，则返回1，否则返回0。
{% highlight string %}
mysql> SELECT 2 BETWEEN 1 AND 3, 2 BETWEEN 3 and 1;
-> 1, 0
mysql> SELECT 1 BETWEEN 2 AND 3;
-> 0
mysql> SELECT 'b' BETWEEN 'a' AND 'c';
-> 1
mysql> SELECT 2 BETWEEN 2 AND '3';
-> 1
mysql> SELECT 2 BETWEEN 2 AND 'x-3';
-> 0
{% endhighlight %}


* COALESCE(value,...): 返回列表中的第一个非空值，假如没有非空值，则返回NULL
{% highlight string %}
mysql> SELECT COALESCE(NULL,1);
-> 1
mysql> SELECT COALESCE(NULL,NULL,NULL);
-> NULL
{% endhighlight %}

* GREATEST(value1,value2,...): 返回参数中的最大值
{% highlight string %}
mysql> SELECT GREATEST(2,0);
-> 2
mysql> SELECT GREATEST(34.0,3.0,5.0,767.0);
-> 767.0
mysql> SELECT GREATEST('B','A','C');
-> 'C'
{% endhighlight %}


* expr IN (value,...): 假如expr等于列表中的任何一个值，则返回1，否则返回0
{% highlight string %}
mysql> SELECT 2 IN (0,3,5,7);
-> 0
mysql> SELECT
{% endhighlight %}

* INTERVAL(N,N1,N2,N3,...): 返回第一个比参数值```N```更小的参数索引。这里需确保N1<N2<...<Nn
{% highlight string %}
mysql> SELECT INTERVAL(23, 1, 15, 17, 30, 44, 200);
-> 3
mysql> SELECT INTERVAL(10, 1, 10, 100, 1000);
-> 2
mysql> SELECT INTERVAL(22, 23, 30, 44, 200);
-> 0
{% endhighlight %}


## 3. 字符串函数
MySQL中的字符串相关函数、操作符有很多，如下只列出部分：

|      NAME        |        DESCRIPTION                                            | 
|:----------------:|:--------------------------------------------------------------|
|   ASCII()        |返回最左边一个字符的数值                                          |
|   BIN()          |返回一个数字的二进制字符串表示                                     |
|  BIT_LENGTH()    |返回参数的bit数长度                                              |
|  CHAR()          |返回传入数字所表示的字符                                          |
|  CHAR_LENGTH()   |返回传入参数的字符数目                                            |
|CHARACTER_LENGTH()|等价于CHAR_LENGTH()                                             |
|  CONCAT()        |拼接两个字符串                                                   |
|  CONCAT_WS()     |采用分隔符拼接两个字符串                                          |
|  ELT()           |返回指定索引处的字符串                                            |
|  EXPORT_SET()    |返回一个字符串： 假如value中的对应bit被设置，则返回所对应的字符串     |
|  FIELD()         |返回第一个参数在后续子序列参数中的索引值                            |
|  FIND_IN_SET()   |返回第一个参数在集合中的索引值                                     |
|  FORMAT()        |格式化一个数字                                                   |
|  FROM_BASE64()   |解码Base64编码的字符串                                           |
|  HEX()           |返回一个数的十六进制表示                                          |
|  INSERT()        |在一个指定的位置插入一个字符串                                     |
|  INSTR()         |返回一个子字符串第一次出现的位置索引                                |
|  LCASE()         |等价于LOWER()                                                   |
|  LEFT()          |返回最左边的N个字符                                              |
|  LENGTH()        |返回一个字符串所占用的字节数                                      |
|  LIKE            |简单模式匹配                                                    |
|  NOT LIKE        |简单模式匹配                                                    |
|  LOAD_FILE()     |加载一个指定的文件                                               |
|  LOCATE()        |返回一个子串第一次出现的位置                                      |
|  LOWER()         |返回字符串的小写表示形式                                          |
|  SPACE()         |返回指定书目的空格字符串                                          |
|  STRCMP()        |比较两个字符串                                                   |
|  TO_BASE64()     |将参数转换成Base64编码                                           |
|  TRIM()          |移除开始于结束的空格                                              |
|  UPPER()         |将一个字符串转变成大写表示                                        |


### 3.1 字符串比较

下面我们介绍一下```LIKE```、```NOT LIKE```这对简单的模式匹配操作符的用法。其语法如下：

<pre>
expr LIKE pat [ESCAPE 'escape_char']
expr NOT LIKE pat [ESCAPE 'escape_char']
</pre>
SQL模式匹配操作符，返回结果为1(TRUE)或者0(FALSE)。假如```expr```或```pat```任何一个为NULL的话，返回结果为NULL。


在SQL标准中，```LIKE```会基于```每一个```字符进行比较，这样其就可能产生于```=```操作符不同的结果：
{% highlight string %}
mysql> SELECT 'ä' LIKE 'ae' COLLATE latin1_german2_ci;
+-----------------------------------------+
| 'ä' LIKE 'ae' COLLATE latin1_german2_ci |
+-----------------------------------------+
| 0 |
+-----------------------------------------+
mysql> SELECT 'ä' = 'ae' COLLATE latin1_german2_ci;
+--------------------------------------+
| 'ä' = 'ae' COLLATE latin1_german2_ci |
+--------------------------------------+
| 1 |
+--------------------------------------+
{% endhighlight %}

特别的是，在进行```CHAR```或```VARCHAR```类型的数据比较时，对于```LIKE```并不会忽略尾部的空格，而对于```=```操作符，则会忽略尾部的空格。
{% highlight string %}
mysql> SELECT 'a' = 'a ', 'a' LIKE 'a ';
+------------+---------------+
| 'a' = 'a ' | 'a' LIKE 'a ' |
+------------+---------------+
| 1 | 0 |
+------------+---------------+
1 row in set (0.00 sec)
{% endhighlight %}

对于```LIKE```，你可以使用如下两种通配符：

* ```%```: 可以匹配0个或任意多个字符

* ```_```: 匹配一个字符
{% highlight string %}
mysql> SELECT 'David!' LIKE 'David_';
-> 1
mysql> SELECT 'David!' LIKE '%D%v%';
-> 1
{% endhighlight %}
另外，如果要针对通配符本身(```%```与```_```)来进行比较，那么需要采用转义字符进行转义。默认的转义字符是```\```。

* ```\%```匹配一个```%```字符

* ```\_```匹配一个```_```字符

{% highlight string %}
mysql> SELECT 'David!' LIKE 'David\_';
-> 0
mysql> SELECT 'David_' LIKE 'David\_';
-> 1
{% endhighlight %}

如果要指定一个不同的转义字符，那么可以使用```ESCAPE```子句：
{% highlight string %}
mysql> SELECT 'David_' LIKE 'David|_' ESCAPE '|';
-> 1
{% endhighlight %}

在一般情况下，字符串的比较都是不区分大小写的。除非进行相应的指定：
{% highlight string %}
mysql> SELECT 'abc' LIKE 'ABC';
-> 1
mysql> SELECT 'abc' LIKE _latin1 'ABC' COLLATE latin1_general_cs;
-> 0
mysql> SELECT 'abc' LIKE _latin1 'ABC' COLLATE latin1_bin;
-> 0
mysql> SELECT 'abc' LIKE BINARY 'ABC';
-> 0
{% endhighlight %}

## 4. Numeric Functions and Operators
下面列出了一些常见的数学函数和操作符：


|      NAME        |        DESCRIPTION                                            | 
|:----------------:|:--------------------------------------------------------------|
|   ABC()          |返回一个数的绝对值                                               |
|	ACOS()         |返回一个圆弧的cos值                                              |
|   CEIL()         |返回第一个不小于指定参数的整数值                                   |
|   CEILLING()     |返回第一个不小于指定参数的整数值                                   |
|   CONV()         |将一个数在不同的进制之间转换                                       |
|   CRC32()        |求一个数的CRC32值                                               |
|   FLOOR()        |返回不大于指定参数的最大整数值                                    |
|   RAND()         |返回一个随机浮点值                                              |

## 5. 日期和时间函数

下面是一些常用的```日期与时间```函数：


|      NAME                  |        DESCRIPTION                                            | 
|:--------------------------:|:--------------------------------------------------------------|
|     ADDDATE()              |向一个Date类型的值中添加time value(intervals)                    |
|     ADDTIME()              |增加时间值                                                      |
|  CONVERT_TZ()              |将一个时区转换成另外一个时区                                      |
|  CURDATE()                 |返回当前Date值                                                  |
|CURRENT_DATE(),CURRENT_DATE |返回当前Date值                                                  |
|CURRENT_TIME(),CURRENT_TIME |返回当前时间值                                                  |
|CURRENT_TIMESTAMP(),CURRENT_TIMESTAMP|返回当前时间戳,等价于NOW()                              |
|CURTIME()                   |返回当前时间                                                    |
|   DATE()                   |返回一个Date或DateTime类型值的Date部分                           |
|   DATEDIFF()               |两个日期进行减法操作                                             |
|   DATE_FORMAT()            |格式化一个日期                                                  |
|   NOW()                    |返回当前日期和时间                                              |
|   SEC_TO_TIME()            |将秒数格式化为```HH:MM:SS```                                   |











<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)



<br />
<br />
<br />

