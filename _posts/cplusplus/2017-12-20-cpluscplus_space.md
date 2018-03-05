---
layout: post
title: 0xC2 0xA0这样的空格 NO-BREAK SPACE
tags:
- cplusplus
categories: cplusplus
description: 0xC2 0xA0这样的空格 NO-BREAK SPACE
---

本章主要记录一下C/C++基础方面的一些内容，以备后续查验。


<!-- more -->


前台的字符串传递到后台进行处理，发现了一个较诡异的问题：字符串中的一个空格(ASCII: 0x20)被UTF-8编码之后变成了一个诡异的字符(0xC2 0xA0)! 但在后台其表象还是空格。

在UTF-8编码里面存在一个特殊的字符，其编码是“0xC2 0xA0”，转换成字符的时候表现为一个半角空格，跟一般的半角空格（ASCII 0x20）不同的是它的宽度不会被压缩，所以排版中常能用到它。但是GB2312、Unicode之类编码中并没有这样的字符，所以转换后前台会显示为“?”号，只是显示为问号而不是真正的问号，所以无法被替换！

可以用如下的命令来去掉这样的空格：
{% highlight string %}
# tr -d "\302\240" < bad > good  
{% endhighlight %}

可以用如下的命令来替换这样的空格：
{% highlight string %}
# cat bad| tr "\302\240" " " 
{% endhighlight %}



<br />
<br />

**[参看]:**

1. [c2a0 这样的空格 NO-BREAK SPACE](http://blog.csdn.net/u012063703/article/details/50561160)

2. [UTF-8编码的空格（194 160）问题](http://www.cnblogs.com/mingmingruyuedlut/archive/2012/07/04/2575180.html)


<br />
<br />
<br />





