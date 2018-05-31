---
layout: post
title: C++虚函数实现的基本原理
tags:
- cplusplus
categories: cplusplus
description: Centos7下OpenResty
---

本文讲述一下C++中虚函数的实现的基本原理。本文参考网上很多文章并经过亲自试验， 修正了其中的一些错误。实验操作系统环境为64bit Centos7.3：

<!-- more -->
<pre>
# uname -a
Linux compile 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 
</pre>









<br />
<br />

**[参考]**

1. [C++继承时的对象内存模型](https://www.cnblogs.com/haoyul/p/7287719.html)

2. [虚函数的实现的基本原理](https://www.cnblogs.com/malecrab/p/5572730.html) 注： 本文在内存模型方面存在一定错误


<br />
<br />
<br />





