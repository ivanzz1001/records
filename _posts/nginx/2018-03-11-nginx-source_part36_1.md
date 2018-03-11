---
layout: post
title: core/ngx_crc.h源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


这里我们主要讲述一下crc(循环冗余校验），再接着讲述一下ngx_crc()这一nginx内部的实现。


<!-- more -->


## 1. 循环冗余校验(crc)



<br />
<br />

**[参考]:**

1. [CRC校验](https://baike.baidu.com/item/CRC%E6%A0%A1%E9%AA%8C/3439037)

2. [CRC32校验算法C语言版(查表法)](https://www.cnblogs.com/kerndev/p/5537379.html)

3. [CRC32](https://baike.baidu.com/item/CRC32/7460858)

4. [[转载]CRC32加密算法原理](https://www.cnblogs.com/dacainiao/p/5565046.html)

5. [循环冗余检验 (CRC) 算法原理](https://www.cnblogs.com/esestt/archive/2007/08/09/848856.html)
<br />
<br />
<br />

