---
layout: post
title: 函数调用栈分析
tags:
- LinuxOps
categories: linux
description: linux
---


本章我们主要讲述一下Linux操作系统环境下C函数的调用栈，然后再讲述一下GCC中的```-fomit-frame-pointer```编译选项。


<!-- more -->


## 1. 使用格式





<br />
<br />

**[参看]:**

1. [函数调用栈的获取原理分析](http://blog.csdn.net/study_live/article/details/43274271)

2. [gcc编译过程查看汇编代码](https://www.2cto.com/kf/201503/386420.html)

3. [GCC “-fomit-frame-pointer”编译选项的含义](http://www.linuxidc.com/Linux/2013-03/81246.htm)

4. [C性能调优---GCC编译选项-fomit-frame-pointer](http://www.cnblogs.com/islandscape/p/3444122.html)

5. [深入理解C语言的函数调用过程](http://blog.chinaunix.net/uid-23069658-id-3981406.html)

6. [函数调用栈 剖析＋图解](http://blog.csdn.net/wangyezi19930928/article/details/16921927)

7. [x86 Assembly Guide](http://www.cs.virginia.edu/~evans/cs216/guides/x86.html)


<br />
<br />
<br />


