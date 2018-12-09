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
本数据结构作为nginx slab内存管理的池结构，用于记录及管理整个slab的内存分配情况。每一个slab内存池对应着一块共享内存，这是一段线性的连续的地址空间，这里不止是有将要分配给使用者的应用内存，还包括slab管理结构，事实上从这块内存的首地址开始就是管理结构体ngx_slab_pool_t。下面我们简要介绍一下各字段的含义：

* lock: 为下面的互斥锁```mutex```服务，使用```原子锁```来实现的Nginx互斥锁需要用到本变量；

* min_size: 设定的最小内存块长度；

* min_shift: min_size对应的位偏移，因为slab的算法大量采用位操作，在后面我们可以看出先计算出min_shift很有好处。

* pages: 每一页对应一个ngx_slab_page_t页描述结构体，所有的ngx_slab_page_t存放在连续的内存中构成数组，而pages就是数组首地址。


* last: 

* free: 所有的空闲页组成一个链表挂在free成员上。

* start: 所有的实际页面全部连续地放在一起，第一页的首地址就是start

* end: 指向这段共享内存的尾部

* mutex: nginx对互斥锁结构的封装

* log_ctx: slab操作失败时会记录日志，为区别是哪个slab共享内存出错，可以在slab中分配一段内存存放描述的字符串，然后再用log_ctx指向这个字符串；

* zero: 实际上就是'\0'，当log_ctx没有赋值时，将直接指向zero，表示空字符串防止出错；

* data: 由各个使用slab的模块自由使用，slab管理内存时不会用到它

* addr: 指向所属的ngx_shm_zone_t里的ngx_shm_t成员的addr成员，一般用于指示一段共享内存块的起始位置

<pre>
注： slab中每一个页大小为ngx_pagesize，这个大小用于实际的内存分配，但另外还需要一个ngx_slab_page_t结构
来管理该页。该结构所占用的空间是24个字节(我们当前是32bit的ubuntu系统），即管理一页需要额外付出24字节的空间。
</pre>
下面给出```ngx_slab_pool_t```数据结构的一个整体视图：

![ngx-slab-pool](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_slab_pool.jpg)




## 3. 相关函数声明
{% highlight string %}
//用于初始化slab管理池
void ngx_slab_init(ngx_slab_pool_t *pool);

//用于从slab管理池中分配出一块指定大小的内存
void *ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);

//与ngx_slab_alloc()类似，只是默认认为在调用此函数前我们已经加了锁，不会出现互斥性问题
void *ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size);

//从slab管理池中分配出一块指定大小的内存，并初始化为0
void *ngx_slab_calloc(ngx_slab_pool_t *pool, size_t size);

//与ngx_slab_alloc_locked()相似
void *ngx_slab_calloc_locked(ngx_slab_pool_t *pool, size_t size);

//用于将一块内存归还回slab管理池
void ngx_slab_free(ngx_slab_pool_t *pool, void *p);

//与ngx_slab_free()类似，只是默认认为在调用此函数前我们已经加了锁，不会出现互斥性问题
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

