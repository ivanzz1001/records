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

* NGX_SLAB_PAGE_BUSY: 值为0xffffffff，用于指示当前page处于busy状态

* NGX_SLAB_PAGE_START: 值为0x80000000，用于指示连续多个页面的开始页

* NGX_SLAB_SHIFT_MASK： 值为

<br />
<br />

**[参看]**

1. [slab](https://baike.baidu.com/item/slab/5803993?fr=aladdin)

2. [Linux内存管理中的slab分配器](https://www.cnblogs.com/pengdonglin137/p/3878552.html)

3. [共享内存管理之slab机制](https://blog.csdn.net/hnudlz/article/details/50972596)

<br />
<br />
<br />

