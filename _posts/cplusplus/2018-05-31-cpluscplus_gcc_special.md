---
layout: post
title: GNU C相关特性
tags:
- cplusplus
categories: cplusplus
description: Centos7下OpenResty
---

本文主要参考《Using the GNU Compiler Collection》， 介绍一下GNU C的一些特性。

<!-- more -->



## 1. 零长度数组

在```GNU C```中允许零长度数组， 其经常作为一个结构体的最后一个元素，用于指示一个可变长object的头：
{% highlight string %}
struct line {
int length;
char contents[0];
};
struct line *thisline = (struct line *)malloc (sizeof (struct line) + this_length);
thisline->length = this_length;
{% endhighlight %}
在```ISO C90```版本中，你必须给上面```contents```成员指定长度为1， 但这样会导致浪费一个字节的空间或者使```malloc()```函数参数复杂化。

在```ISO C99```版本中，你可以使用一个柔性数组(flexible array member)，但是其在语法和语义上面还是与```零长度数组```有一些不同：

* 






<br />
<br />

**[参考]**



<br />
<br />
<br />





