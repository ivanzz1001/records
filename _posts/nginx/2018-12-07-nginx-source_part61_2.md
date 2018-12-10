---
layout: post
title: core/ngx_slab.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们介绍一下Nginx中基于slab的内存分配机制。




<!-- more -->

## 1. 相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_SLAB_PAGE_MASK   3
#define NGX_SLAB_PAGE        0
#define NGX_SLAB_BIG         1
#define NGX_SLAB_EXACT       2
#define NGX_SLAB_SMALL       3

#if (NGX_PTR_SIZE == 4)

#define NGX_SLAB_PAGE_FREE   0
#define NGX_SLAB_PAGE_BUSY   0xffffffff
#define NGX_SLAB_PAGE_START  0x80000000

#define NGX_SLAB_SHIFT_MASK  0x0000000f
#define NGX_SLAB_MAP_MASK    0xffff0000
#define NGX_SLAB_MAP_SHIFT   16

#define NGX_SLAB_BUSY        0xffffffff

#else /* (NGX_PTR_SIZE == 8) */

#define NGX_SLAB_PAGE_FREE   0
#define NGX_SLAB_PAGE_BUSY   0xffffffffffffffff
#define NGX_SLAB_PAGE_START  0x8000000000000000

#define NGX_SLAB_SHIFT_MASK  0x000000000000000f
#define NGX_SLAB_MAP_MASK    0xffffffff00000000
#define NGX_SLAB_MAP_SHIFT   32

#define NGX_SLAB_BUSY        0xffffffffffffffff

#endif


#if (NGX_DEBUG_MALLOC)

#define ngx_slab_junk(p, size)     ngx_memset(p, 0xA5, size)

#elif (NGX_HAVE_DEBUG_MALLOC)

#define ngx_slab_junk(p, size)                                                \
    if (ngx_debug_malloc)          ngx_memset(p, 0xA5, size)

#else

#define ngx_slab_junk(p, size)

#endif
{% endhighlight %}

1) **所分配的内存大小尺寸**

这里用```2位```来表示所分配的内存大小尺寸，2位最多可以表示4种尺寸：

* NGX_SLAB_PAGE_MASK: 表示尺寸的掩码

* NGX_SLAB_PAGE: 超大内存，大于等于ngx_slab_max_size

* NGX_SLAB_BIG： 大块内存，大于ngx_slab_exact_size而小于ngx_slab_max_size

* NGX_SLAB_EXACT： 中等内存，等于ngx_slab_exact_size

* NGX_SLAB_SMALL： 小块内存，小于ngx_slab_exact_size

下面是exact size与max size的计算：
<pre>
ngx_slab_max_size = ngx_pagesize / 2;

ngx_slab_exact_size = ngx_pagesize / (8 * sizeof(uintptr_t));
</pre>

2) **掩码设置**

在当前我们使用的32bit Ubuntu环境中，```NGX_PTR_SIZE```的值为4，因此：

* NGX_SLAB_PAGE_FREE: 值为0， 用于指示当前page处于空闲状态

* NGX_SLAB_PAGE_BUSY: 值为0xffffffff，用于指示当前page处于busy状态（针对大于等于ngx_slab_max_size的内存块）

* NGX_SLAB_PAGE_START: 值为0x80000000，用于指示连续多个页面的开始页

* NGX_SLAB_SHIFT_MASK： 值为0x0000000f

* NGX_SLAB_MAP_MASK： 值为0xffff0000

* NGX_SLAB_MAP_SHIFT: 值为16

* NGX_SLAB_BUSY: 值为0xffffffff, 用于表示当前slab处于busy状态（针对小于ngx_slab_max_size的内存块）


3） **调试信息**

我们当前并不支持```NGX_DEBUG_MALLOC```与```NGX_HAVE_DEBUG_MALLOC```，因此执行的是如下宏：
<pre>
#define ngx_slab_junk(p, size)
</pre>


## 2. 相关静态函数声明及变量定义
{% highlight string %}
//用于分配指定数量的页
static ngx_slab_page_t *ngx_slab_alloc_pages(ngx_slab_pool_t *pool,
    ngx_uint_t pages);

//释放指定数量的页
static void ngx_slab_free_pages(ngx_slab_pool_t *pool, ngx_slab_page_t *page,
    ngx_uint_t pages);

//打印slab相关的错误日志
static void ngx_slab_error(ngx_slab_pool_t *pool, ngx_uint_t level,
    char *text);

static ngx_uint_t  ngx_slab_max_size;
static ngx_uint_t  ngx_slab_exact_size;
static ngx_uint_t  ngx_slab_exact_shift;
{% endhighlight %}


## 3. 函数ngx_slab_init()
{% highlight string %}
void
ngx_slab_init(ngx_slab_pool_t *pool)
{
    u_char           *p;
    size_t            size;
    ngx_int_t         m;
    ngx_uint_t        i, n, pages;
    ngx_slab_page_t  *slots;

    /* STUB */
    if (ngx_slab_max_size == 0) {
        ngx_slab_max_size = ngx_pagesize / 2;
        ngx_slab_exact_size = ngx_pagesize / (8 * sizeof(uintptr_t));
        for (n = ngx_slab_exact_size; n >>= 1; ngx_slab_exact_shift++) {
            /* void */
        }
    }
    /**/

    pool->min_size = 1 << pool->min_shift;

    p = (u_char *) pool + sizeof(ngx_slab_pool_t);
    size = pool->end - p;

    ngx_slab_junk(p, size);

    slots = (ngx_slab_page_t *) p;
    n = ngx_pagesize_shift - pool->min_shift;

    for (i = 0; i < n; i++) {
        slots[i].slab = 0;
        slots[i].next = &slots[i];
        slots[i].prev = 0;
    }

    p += n * sizeof(ngx_slab_page_t);

    pages = (ngx_uint_t) (size / (ngx_pagesize + sizeof(ngx_slab_page_t)));

    ngx_memzero(p, pages * sizeof(ngx_slab_page_t));

    pool->pages = (ngx_slab_page_t *) p;

    pool->free.prev = 0;
    pool->free.next = (ngx_slab_page_t *) p;

    pool->pages->slab = pages;
    pool->pages->next = &pool->free;
    pool->pages->prev = (uintptr_t) &pool->free;

    pool->start = (u_char *)
                  ngx_align_ptr((uintptr_t) p + pages * sizeof(ngx_slab_page_t),
                                 ngx_pagesize);

    m = pages - (pool->end - pool->start) / ngx_pagesize;
    if (m > 0) {
        pages -= m;
        pool->pages->slab = pages;
    }

    pool->last = pool->pages + pages;

    pool->log_nomem = 1;
    pool->log_ctx = &pool->zero;
    pool->zero = '\0';
}
{% endhighlight %}

本函数用于初始化整个ngx_slab_pool_t结构，下面我们简要分析一下函数的实现：
{% highlight string %}
void
ngx_slab_init(ngx_slab_pool_t *pool)
{
	//1) 计算ngx_slab_max_size，ngx_slab_exact_size， ngx_slab_exact_shift
	// 从前面我们知道ngx_pagesize=4096, 因此
	ngx_slab_max_size = 2048;
	ngx_slab_exact_size = 128;
	ngx_slab_exact_shift = 7;

	//2) 计算得到min_size大小及n的值
	pool->min_size = 8;			//(pool->min_shift值为3）
	ngx_pagesize_shift = 12;
	n = 9;

	//3) 初始化slots
	for (i = 0; i < n; i++) {
        slots[i].slab = 0;
        slots[i].next = &slots[i];
        slots[i].prev = 0;
    }

	//4) 计算剩余空间最多能分配多少个页（每一页大小为ngx_pagesize，另外每一页还对应一个ngx_slab_page_t管理结构)
	pages = (ngx_uint_t) (size / (ngx_pagesize + sizeof(ngx_slab_page_t)));

	//5) 初始化pool->pages及pool->free链表
	pool->pages->slab = pages;			//slab用于记录空闲页数

	//6) 计算实际空闲页数
	m = pages - (pool->end - pool->start) / ngx_pagesize;
    if (m > 0) {
        pages -= m;
        pool->pages->slab = pages;
    }

    pool->last = pool->pages + pages;

	//7)将pool->zero的值置为'\0'
	//pool->zero = '\0';

}
{% endhighlight %}
下面我们画出初始化之后的示意图：

![ngx-slab-init](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_slab_init.jpg)



<br />
<br />

**[参看]**

1. [slab](https://baike.baidu.com/item/slab/5803993?fr=aladdin)

2. [Linux内存管理中的slab分配器](https://www.cnblogs.com/pengdonglin137/p/3878552.html)

3. [共享内存管理之slab机制](https://blog.csdn.net/hnudlz/article/details/50972596)

<br />
<br />
<br />

