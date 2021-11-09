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

## 1. No Break space(nbsp)
前台的字符串传递到后台进行处理，发现了一个较诡异的问题：字符串中的一个空格(ASCII: 0x20)被UTF-8编码之后变成了一个诡异的字符(0xC2 0xA0)! 但在后台其表象还是空格。

在UTF-8编码里面存在一个特殊的字符，其编码是“0xC2 0xA0”，转换成字符的时候表现为一个半角空格，跟一般的半角空格（ASCII 0x20）不同的是它的宽度不会被压缩，所以排版中常能用到它。但是GB2312、Unicode之类编码中并没有这样的字符，所以转换后前台会显示为“?”号，只是显示为问号而不是真正的问号，所以无法被替换！

可以用如下的命令来```去掉```这样的空格：
{% highlight string %}
# tr -d "\302\240" < bad > good  
{% endhighlight %}

可以用如下的命令来```替换```这样的空格：
{% highlight string %}
# cat bad| tr "\302\240" " " 
{% endhighlight %}

## 2. HTML中的6种空格

HTML提供了5种空格实体（space entity），它们拥有不同的宽度，非断行空格（```&nbsp;```）是常规空格的宽度，可运行于所有主流浏览器。其他几种空格（```&ensp;```、```&emsp;```、```&thinsp;```、```&zwnj;```、```&zwj;```）在不同浏览器中宽度各异。

* ```&nbsp;```: 它叫不换行空格，全称No-Break Space，它是最常见和我们使用最多的空格，大多数的人可能只接触了&nbsp;，它是按下space键产生的空格。在HTML中，如果你用空格键产生此空格，空格是不会累加的（只算1个）。要使用html实体表示才可累加，该空格占据宽度受字体影响明显而强烈。

* ```&ensp;```: 它叫```半角空格```(En Space)。en是字体排印学的计量单位，为em宽度的一半。根据定义，它等同于字体度的一半（如16px字体中就是8px）。名义上是小写字母n的宽度。此空格传承空格家族一贯的特性：透明的，此空格有个相当稳健的特性，就是其 占据的宽度正好是1/2个中文宽度，而且基本上不受字体影响。

* ```&emsp;```：它叫```全角空格```(Em Space)。em是字体排印学的计量单位，相当于当前指定的点数。例如，1 em在16px的字体中就是16px。此空格也传承空格家族一贯的特性：透明的，此空格也有个相当稳健的特性，就是其 占据的宽度正好是1个中文宽度，而且基本上不受字体影响。

* ```&thinsp;```:它叫窄空格，全称是Thin Space。我们不妨称之为```瘦弱空格```，就是该空格长得比较瘦弱，身体单薄，占据的宽度比较小。它是em之六分之一宽。

* ```&zwnj;```: 它叫零宽不连字，全称是Zero Width Non Joiner，简称```ZWNJ```，是一个不打印字符，放在电子文本的两个字符之间，抑制本来会发生的连字，而是以这两个字符原本的字形来绘制。Unicode中的零宽不连字字符映射为```U+200C```，HTML字符值引用为```&zwnj;```

* ```&zwj;```: 它叫零宽连字，全称是Zero Width Joiner，简称```ZWJ```，是一个不打印字符，放在某些需要复杂排版语言（如阿拉伯语、印地语）的两个字符之间，使得这两个本不会发生连字的字符产生了连字效果。零宽连字符的Unicode码位是```U+200D``` ,HTML字符值引用为```&zwj;```。



<br />
<br />

**[参看]:**

1. [c2a0 这样的空格 NO-BREAK SPACE](http://blog.csdn.net/u012063703/article/details/50561160)

2. [UTF-8编码的空格（194 160）问题](http://www.cnblogs.com/mingmingruyuedlut/archive/2012/07/04/2575180.html)

3. [UTF-8 encoding table and Unicode characters](https://www.utf8-chartable.de/unicode-utf8-table.pl?utf8=dec)

4. [HTML中的& nbsp; & ensp; & emsp;等6种空格标记](https://blog.csdn.net/u014781844/article/details/84859693)
<br />
<br />
<br />





