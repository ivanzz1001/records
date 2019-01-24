---
layout: post
title: 字符编码
tags:
- computer-basis
categories: computer-basis
description: 字符编码
---



本文我们简单讲述一下```unicode```编码（Unicode-32与UTF-8)


<!-- more -->


## 1. Unicode

![cb-unicode](https://ivanzz1001.github.io/records/assets/img/computer_basis/cb_unicode.jpg)

## 2. UTF-8

**UTF-8**是对Unicode字符的一个变长编码。utf-8是由**Ken Thompson**和**Rob Pike**发明，同时这两位也是```Golang```的创立者，当前已经称为Unicode的标准(Unicode standard)。utf-8使用**1~4**个字节来编码每一个字符，其中**ASCII**字符使用```1个```字节，对于其他大部分常用的字符使用```2个```或```3个```字节，极少部分采用```4个```字节来编码。第一个字节高比特位为```0```，用于表示7位的**ASCII码**；第一个字节的高位为```10```表示字符占用2个字节：
<pre>
0xxxxxx                                 runes 0−127       (ASCII)

110xxxxx 10xxxxxx                       128−2047          (values <128 unused)

1110xxxx 10xxxxxx 10xxxxxx              2048−65535        (values <2048 unused)

11110xxx 10xxxxxx 10xxxxxx 10xxxxxx     65536−0x10ffff    (other values unused)
</pre>
例如，对于```国```，其Unicode-32值为```\u56fd```，那么编码为utf-8后，其对应的字节为：

![cb-utf8-guo](https://ivanzz1001.github.io/records/assets/img/computer_basis/cb_utf8_guo.jpg)




<br />
<br />

**参看:**

1. [查看字符编码(UTF-8)](http://www.mytju.com/classcode/tools/encode_utf8.asp)



<br />
<br />
<br />

