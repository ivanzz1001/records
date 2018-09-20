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


<br />
<br />
**[参看]**:


1. [MySQL教程](http://www.runoob.com/mysql/mysql-administration.html)



<br />
<br />
<br />

