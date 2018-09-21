---
layout: post
title: mysql数据库基础（五）
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


<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)



<br />
<br />
<br />

