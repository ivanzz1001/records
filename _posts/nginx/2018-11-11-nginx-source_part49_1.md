---
layout: post
title: core/ngx_palloc.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本章我们主要介绍一下ngx_palloc.h，其实现了Nginx中最基本的内存池操作。nginx中大多数的内存分配都是在pool中完成的，在pool中分配的内存在pool被销毁时会被自动的释放。这就使得可以获得很高的内存分配性能，并使得内存控制更加简单。

一个pool在内部分配对象空间时都是在一个连续的内存块(block)来进行的。一旦一个block满了之后，就会分配一个新的block并添加到pool的block list中。当所请求分配的空间过大，而不能在一个单独的块中进行分配时，则该请求会被转换到使用操作系统的allocator，然后将其返回的内存指针存放到pool中，以方便后续通过pool来释放该内存块。
<!-- more -->


## 1. 





<br />
<br />

**[参看]**

1. [nginx源码学习(二) 内存池结构 ngx_pool_t](https://blog.csdn.net/daniel_ustc/article/details/11645293)

2. [nginx源码分析—内存池结构ngx_pool_t及内存管理](https://www.cnblogs.com/405845829qq/p/4379093.html)

3. [ Nginx开发从入门到精通 Nginx开发从入门到精通](http://www.treelib.com/book-detail-id-17-aid-853.html)

<br />
<br />
<br />

