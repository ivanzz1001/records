---
layout: post
title: core/ngx_slab.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们介绍一下Nginx中基于slab的内存分配机制。


这里，我们先简单介绍一下Linux下的slab。```slab```是Linux操作系统的一种内存分配机制。其工作是针对一些经常分配并释放的对象，如进程描述符等，这些对象的大小一般比较小，如果直接采用伙伴系统来进行分配和释放，不仅会造成大量的内存碎片，而且处理速度也太慢。而slab分配器是基于对象进行管理的，相同类型的对象归为一类（如进程描述符就是一类），每当要申请这样一个对象，slab分配器就从一个slab列表中分配一个这样大小的单元出去，而当要释放时，将其重新保存到该列表中，而不是直接返回给伙伴系统，从而避免这些内存碎片。slab分配器并不丢失已分配的对象，而是释放并把它们保存在内存中。当以后又要请求新的对象时，就可以从内存直接获取而不用重复初始化。

<!-- more -->


## 1. ngx_slab_page_s数据结构
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SLAB_H_INCLUDED_
#define _NGX_SLAB_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_slab_page_s  ngx_slab_page_t;

struct ngx_slab_page_s {
    uintptr_t         slab;
    ngx_slab_page_t  *next;
    uintptr_t         prev;
};
{% endhighlight %}
本数据结构对应于Nginx slab内存管理的```页```概念，```页```在slab管理设计中是很核心的概念。**ngx_slab_page_t**中各字段根据```不同内存页类型```有不同的含义，下面我们就分别介绍一下：

1) **小块内存，小于ngx_slab_exact_size**

* slab: 表示该页面上存放的等长内存块大小，当然是用位偏移的方式存放的；

* next: 指向双链表的下一个元素，如果不在双链表中，则为0

* prev: 低2位为11，以NGX_SLAB_SMALL表示当前页面存放的是小块内存

2) **中等内存， 等于ngx_slab_exact_size**

* slab: 作为bitmap表示页面上的内存块是否已被使用

* next: 指向双链表的下一个元素，如果不在双链表中，则为0

* prev: 低2位为10，以NGX_SLAB_EXACT表示当前页面存放的是中等大小的内存

3) **大块内存，大于ngx_slab_exact_size而小于ngx_slab_max_size**

* slab: 高NGX_SLAB_MAP_MASK位表示bitmap，而低NGX_SLAB_SHIFT_MASK位表示存放的内存块大小；

* next: 指向双链表的下一个元素，如果不在双链表中，则为0

* prev: 低2位为01， 以NGX_SLAB_BIG表示当前页面存放的是大块内存

4) **超大内存，大于等于ngx_slab_max_size**

* slab: 超大内存会使用一页或者多页，这些页都在一起使用。对于这批页面中的第1页，slab的前3位会被设为NGX_SLAB_PAGE_START，其余位表示紧随其后相邻的同批页面数；反之，slab会被设置为NGX_SLAB_PAGE_BUSY

* next: 指向双链表的下一个元素，如果不在双链表中，则为0

* prev: 低2位为00，以NGX_SLAB_PAGE表示当前页面是以整页来使用。

## 2. ngx_slab_pool_t数据结构
{% highlight string %}
typedef struct {
    ngx_shmtx_sh_t    lock;

    size_t            min_size;
    size_t            min_shift;

    ngx_slab_page_t  *pages;
    ngx_slab_page_t  *last;
    ngx_slab_page_t   free;

    u_char           *start;
    u_char           *end;

    ngx_shmtx_t       mutex;

    u_char           *log_ctx;
    u_char            zero;

    unsigned          log_nomem:1;

    void             *data;
    void             *addr;
} ngx_slab_pool_t;
{% endhighlight %}
本数据结构作为nginx slab内存管理的池结构，用于记录及管理整个slab的内存分配情况。下面我们简要介绍一下各字段的含义：

* lock:


## 3. 相关函数声明
{% highlight string %}
void ngx_slab_init(ngx_slab_pool_t *pool);
void *ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_calloc(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_calloc_locked(ngx_slab_pool_t *pool, size_t size);
void ngx_slab_free(ngx_slab_pool_t *pool, void *p);
void ngx_slab_free_locked(ngx_slab_pool_t *pool, void *p);
{% endhighlight %}



<br />
<br />

**[参看]**

1. [slab](https://baike.baidu.com/item/slab/5803993?fr=aladdin)

2. [Linux内存管理中的slab分配器](https://www.cnblogs.com/pengdonglin137/p/3878552.html)

3. [nginx中slab实现](https://www.cnblogs.com/fll369/archive/2012/11/26/2789704.html)

4. [共享内存管理之slab机制](https://blog.csdn.net/hnudlz/article/details/50972596)

5. [nginx slab内存管理](http://www.cnblogs.com/doop-ymc/p/3412572.html)

6. [Nginx开发从入门到精通](http://tengine.taobao.org/book/index.html#)
<br />
<br />
<br />

