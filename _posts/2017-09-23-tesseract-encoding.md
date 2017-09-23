---
layout: post
title: 浅析字符编码
tags:
- ocr
categories: ocr
description: 浅析字符编码
---

写过网站或其他程序的人会经常遇到各种各样的因字符编码产生的问题，这里我们重点讲述一下常用的Unicode、UTF-8、UTF-16、UTF-32/GBK、GB2312、ISO-8859-1等编码。


<!-- more -->
<br />
<br />

参看:

1. [浅析unicode和UTF-8、UTF-16、UTF-32的区别](http://blog.csdn.net/co_yiqiu/article/details/54954392)

2. [GBK,UTF-8,和ISO8859-1之间的编码与解码](http://blog.csdn.net/xiongchao2011/article/details/7276834)

3. [【字符编码】彻底理解字符编码](http://www.cnblogs.com/leesf456/p/5317574.html)


## 1. 编码字符集和字符集编码

首先我们从字面意思来理解：```编码字符集```指的是字符集，例如我们可以将汉字作为一个字符集、将英文字母作为一个字符集； ```字符集编码```指的是编码，是对前者按某种规则进行的编码，例如我们可以将汉字集合用GBK进行编码,用utf-8进行编码。下面以unicode来进行说明：

[Unicode](https://baike.baidu.com/item/Unicode/750500?fr=aladdin)是计算机可选领域里的一项业界标准，包括字符集、编码方案等。注意这里包括两层含义： 

* Unicode定义了字符集（即编码字符集）
* Unicode为自己定义的字符集指定了编码方案（字符集编码）

Unicode是为了解决传统的字符编码方案的局限而产生的，它为每种语言中的每个字符设定了统一并且唯一的二进制编码，以满足跨语言、跨平台进行文本转换、处理的要求。Unicode从1990年开始研发，1994年正式公布。

 









<br />
<br />
<br />

