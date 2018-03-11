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

Cyclic Redundancy Check循环冗余校验，是基于数据计算一组校验码，用于核对数据传输过程中是否被更改或者传输错误。

### 1.1 算法原理
假设数据传输过程中需要发送15位的二进制信息g=**```101001110100001```**，这串二进制码可表示为代数多项式g(x) = x^14 + x^12 + x^9 + x^8 + x^7 + x^5 + 1，其中g中第k位的值，对应g(x)中x^k的系数。将g(x)乘以x^m，既将g后加m个0，然后除以m阶多项式h(x)，得到的(m-1)阶余项r(x)对应的二进制码r就是CRC编码。

h(x)可以自由选择或者使用国际通行标准，一般按照h(x)的阶数m，将CRC算法称为CRC-m，比如CRC-32、CRC-64等。国际通行标准可以参看[http://en.wikipedia.org/wiki/Cyclic_redundancy_check](http://en.wikipedia.org/wiki/Cyclic_redundancy_check)

g(x)和h(x)的除运算，可以通过g和h做xor（异或）运算。比如将11001与10101做xor运算:

![ngx-crc001](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_crc001.gif)



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

