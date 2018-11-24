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

Nginx里内存的使用大都十分有特色： 申请了永久保存，抑或伴随这请求的结束而全部释放，还有写满了缓冲再从头接着写，这么做的原因也主要取决于Web Server的特殊的场景，内存的分配和请求相关，一条请求处理完毕，即可释放其相关的内存池，降低了开发中对内存资源管理的复杂度，也减少了内存碎片的存在。

<!-- more -->


## 1. 相关宏的定义
{% highlight string %}
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)

#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)
{% endhighlight %}

这里定义了一些宏：

* NGX_MAX_ALLOC_FROM_POOL: 用于定义内存池中数据块的最大值。一般在x86机器上为(4096-1)

* NGX_DEFAULT_POOL_SIZE: 一个pool默认的总空间大小（注意这里包括```ngx_pool_t```结构体本身的大小）

* NGX_POOL_ALIGNMENT: 内存池本身分配在以16字节对齐的空间上

* NGX_MIN_POOL_SIZE： 内存池的最小空间大小。这里```ngx_pool_t```的大小为80，而```ngx_pool_large_t```的大小为16,因此算出内存池的最小空间大小为112字节。

## 2. ngx_pool_cleanup_t数据结构 
{% highlight string %}
typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;
    void                 *data;
    ngx_pool_cleanup_t   *next;
};
{% endhighlight %}
本数据结构主要用于指示如何释放内存池资源：

* handler: 释放内存池资源的回调函数

* data: 指向要清除的数据

* next: 指向下一个ngx_pool_cleanup_t结构。

## 3. ngx_pool_large_t数据结构
{% highlight string %}
typedef struct ngx_pool_large_s  ngx_pool_large_t;

struct ngx_pool_large_s {
    ngx_pool_large_t     *next;
    void                 *alloc;
};
{% endhighlight %}

大内存块结构。当待分配空间已经超过了池子自身大小，nginx也没有别的好办法，只好按照你需要分配的大小，实际去调用malloc()函数去分配。例如池子大小是1K，而待分配大小是1M。实际上池子里只存储了ngx_pool_large_t结构，这个结构的alloc指针，指向被分配的内存，并把这个指针返回给系统使用。

* next: 指向下一块大内存

* alloc: 指向分配的大块内存

## 4. ngx_pool_t数据结构
{% highlight string %}
typedef struct ngx_pool_s        ngx_pool_t;

typedef struct {
    u_char               *last;
    u_char               *end;
    ngx_pool_t           *next;
    ngx_uint_t            failed;
} ngx_pool_data_t;


struct ngx_pool_s {
    ngx_pool_data_t       d;
    size_t                max;
    ngx_pool_t           *current;
    ngx_chain_t          *chain;
    ngx_pool_large_t     *large;
    ngx_pool_cleanup_t   *cleanup;
    ngx_log_t            *log;
};
{% endhighlight %}

下面我们简要介绍一下这两个数据结构：

1) **ngx_pool_data_t结构体**

```ngx_pool_data_t```用于表示内存池中的一个数据块，供用户分配之用。下面介绍一下各个字段的含义：

* last: 当前内存分配结束位置，即下一段可分配内存的起始位置；

* end: 本内存块的结束位置；

* next: 内存池里面有很多块内存，这些内存块就是通过该指针连成链表的

* failed: 统计该内存池不能满足分配请求的次数，即分配失败的次数；

2） **ngx_pool_t结构体**

```ngx_pool_t```用于维护整个内存池的头部信息。下面介绍一下各个字段的含义：

* d: 内存池的数据块

* max: 数据块大小，可分配小块内存的最大值

* current: 当前内存池，以后的内存分配从该指针指向的内存池中分配；

* chain: 指向一个ngx_chain_t结构

* large: 指向大块内存分配，nginx中，大块内存分配直接采用标准系统接口malloc

* cleanup: 析构函数

* log: 内存分配相关的日志记录

下面画出```ngx_pool_t```结构的示意图：

![ngx-pool-t](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_pool_t.jpg)

注： 其实上面图画的有些小问题，对于```pool->d.next```指向的```ngx_pool_t```结构,在实际使用过程中其实指向的是一个```ngx_pool_data_t```结构，即只是ngx_pool_t结构的一小部分。这样做的原因主要是为了节省空间。



## 5. ngx_pool_cleanup_file_t结构
{% highlight string %}
typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;
{% endhighlight %}

此结构主要用于针对```Nginx静态文件缓存```(open file cache)的pool在释放时传递相应的上下文。

* fd: 所打开文件的句柄

* name: 文件的名称

* log: 所关联的日志对象

## 6. 相关函数声明
{% highlight string %}

//下面两个函数用于分配指定大小的空间，在os/unix/ngx_alloc.c中定义
void *ngx_alloc(size_t size, ngx_log_t *log);
void *ngx_calloc(size_t size, ngx_log_t *log);


// 创建内存块大小为size的内存池
ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);

//销毁一个内存池
void ngx_destroy_pool(ngx_pool_t *pool);

//重置一个内存池
void ngx_reset_pool(ngx_pool_t *pool);

//在内存池中分配指定大小的内存块(所分配的内存会进行对齐）
void *ngx_palloc(ngx_pool_t *pool, size_t size);

//在内存池中分配指定大小的内存块（所分配的内存不会进行对齐）
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);

//在内存池中分配指定大小的内存块，并初始化为0
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);

//向pool中增加一个大内存块
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);

//释放pool中的大内存块
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);

//向pool注册一个cleanup回调（size是回调上下文的大小）
ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);

//执行pool中的cleanup_file回调
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);

//cleanup file时的相关操作(一般用于nginx静态文件缓存）
void ngx_pool_cleanup_file(void *data);

//删除文件（一般用于删除那些与pool绑定的临时文件)
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
{% endhighlight %}



<br />
<br />

**[参看]**

1. [nginx源码学习(二) 内存池结构 ngx_pool_t](https://blog.csdn.net/daniel_ustc/article/details/11645293)

2. [nginx源码分析—内存池结构ngx_pool_t及内存管理](https://www.cnblogs.com/405845829qq/p/4379093.html)

3. [Nginx开发从入门到精通 Nginx开发从入门到精通](http://www.treelib.com/book-detail-id-17-aid-853.html)

4. [nginx 学习四 内存池 ngx_pool_t 和内存管理操作](https://blog.csdn.net/xiaoliangsky/article/details/39523875)

5. [nginx源码解析(二)-内存池与内存管理ngx_pool_t](https://blog.csdn.net/mao19931004/article/details/54377101)

<br />
<br />
<br />

