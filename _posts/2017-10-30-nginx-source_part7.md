---
layout: post
title: os/unix/ngx_alloc.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本文我们主要分析一下nginx内存分配的相关实现源代码。


<!-- more -->


## 1. os/unix/ngx_alloc.h头文件

头文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ALLOC_H_INCLUDED_
#define _NGX_ALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


void *ngx_alloc(size_t size, ngx_log_t *log);
void *ngx_calloc(size_t size, ngx_log_t *log);

#define ngx_free          free


/*
 * Linux has memalign() or posix_memalign()
 * Solaris has memalign()
 * FreeBSD 7.0 has posix_memalign(), besides, early version's malloc()
 * aligns allocations bigger than page size at the page boundary
 */

#if (NGX_HAVE_POSIX_MEMALIGN || NGX_HAVE_MEMALIGN)

void *ngx_memalign(size_t alignment, size_t size, ngx_log_t *log);

#else

#define ngx_memalign(alignment, size, log)  ngx_alloc(size, log)

#endif


extern ngx_uint_t  ngx_pagesize;
extern ngx_uint_t  ngx_pagesize_shift;
extern ngx_uint_t  ngx_cacheline_size;


#endif /* _NGX_ALLOC_H_INCLUDED_ */
{% endhighlight %}

头文件中主要定义了分配内存空间的几个函数原型。在ngx_auto_config.h头文件中，我们有如下两个宏定义：
<pre>
#ifndef NGX_HAVE_POSIX_MEMALIGN
#define NGX_HAVE_POSIX_MEMALIGN  1
#endif


#ifndef NGX_HAVE_MEMALIGN
#define NGX_HAVE_MEMALIGN  1
#endif
</pre>

对于Linux操作系统其一般支持memalign()或posix_memalign();对于solaris一般支持memalign; 对于freebsd7.0一般支持posix_memalign。

因此，这里对于有对齐要求的内存分配，其函数原型如下：
{% highlight string %}
void *ngx_memalign(size_t alignment, size_t size, ngx_log_t *log);
{% endhighlight %}

如下三个变量均定义在os/unix/ngx_alloc.c源文件中：
<pre>
extern ngx_uint_t  ngx_pagesize;
extern ngx_uint_t  ngx_pagesize_shift;
extern ngx_uint_t  ngx_cacheline_size;
</pre>



## 2. os/unix/ngx_alloc.c源文件

源文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_uint_t  ngx_pagesize;
ngx_uint_t  ngx_pagesize_shift;
ngx_uint_t  ngx_cacheline_size;


void *
ngx_alloc(size_t size, ngx_log_t *log)
{
    void  *p;

    p = malloc(size);
    if (p == NULL) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
                      "malloc(%uz) failed", size);
    }

    ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, log, 0, "malloc: %p:%uz", p, size);

    return p;
}


void *
ngx_calloc(size_t size, ngx_log_t *log)
{
    void  *p;

    p = ngx_alloc(size, log);

    if (p) {
        ngx_memzero(p, size);
    }

    return p;
}


#if (NGX_HAVE_POSIX_MEMALIGN)

void *
ngx_memalign(size_t alignment, size_t size, ngx_log_t *log)
{
    void  *p;
    int    err;

    err = posix_memalign(&p, alignment, size);

    if (err) {
        ngx_log_error(NGX_LOG_EMERG, log, err,
                      "posix_memalign(%uz, %uz) failed", alignment, size);
        p = NULL;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_ALLOC, log, 0,
                   "posix_memalign: %p:%uz @%uz", p, size, alignment);

    return p;
}

#elif (NGX_HAVE_MEMALIGN)

void *
ngx_memalign(size_t alignment, size_t size, ngx_log_t *log)
{
    void  *p;

    p = memalign(alignment, size);
    if (p == NULL) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
                      "memalign(%uz, %uz) failed", alignment, size);
    }

    ngx_log_debug3(NGX_LOG_DEBUG_ALLOC, log, 0,
                   "memalign: %p:%uz @%uz", p, size, alignment);

    return p;
}

#endif
{% endhighlight %}


### 2.1 ngx_alloc()函数

```ngx_alloc()```函数较为简单，直接调用malloc()函数分配指定大小的内存空间。


### 2.2 ngx_calloc()函数

```ngx_calloc()```函数首先调用ngx_alloc()函数分配中指定大小的内存空间，然后将这块分配的内存空间都清0。这里ngx_memzero()函数是在src/core/ngx_string.h头文件中定义：
{% highlight string %}
#define ngx_memzero(buf, n)       (void) memset(buf, 0, n)
{% endhighlight %}

### 2.3 ngx_memalign()函数实现-版本1

在宏定义```NGX_HAVE_POSIX_MEMALIGN```为真时，其实现如下：
{% highlight string %}
void *
ngx_memalign(size_t alignment, size_t size, ngx_log_t *log)
{
    void  *p;
    int    err;

    err = posix_memalign(&p, alignment, size);

    if (err) {
        ngx_log_error(NGX_LOG_EMERG, log, err,
                      "posix_memalign(%uz, %uz) failed", alignment, size);
        p = NULL;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_ALLOC, log, 0,
                   "posix_memalign: %p:%uz @%uz", p, size, alignment);

    return p;
}
{% endhighlight %}

```posix_memalign()```函数原型如下：
<pre>
int posix_memalign(void **memptr, size_t alignment, size_t size);
</pre>

其会分配size字节大小的内存，并将该块内存的首地址存放于memptr中。所分配的内存首地址必须为alignment的整数倍，并且必须是2^N，还必须是sizeof(void *)的整数倍。假若size为0的话，memptr会返回为NULL。该函数存在于<stdio.h>头文件中。

**注意**：posix_memalign()会检查alignment参数是否满足对其要求，即必须为2^N，并且还必须是sizeof(void *)的整数倍。



### 2.4 ngx_memalign()函数实现-版本2
在宏定义```NGX_HAVE_MEMALIGN```为真时，其实现如下：
{% highlight string %}
void *
ngx_memalign(size_t alignment, size_t size, ngx_log_t *log)
{
    void  *p;

    p = memalign(alignment, size);
    if (p == NULL) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
                      "memalign(%uz, %uz) failed", alignment, size);
    }

    ngx_log_debug3(NGX_LOG_DEBUG_ALLOC, log, 0,
                   "memalign: %p:%uz @%uz", p, size, alignment);

    return p;
}
{% endhighlight %}
```memalign()```函数原型如下：
<pre>
void *memalign(size_t alignment, size_t size);
</pre>
memalign()函数是一个过时函数，其会分配size大小的内存并返回。所分配的内存首地址必须为alignment的整数倍，并且必须为2^N幂。该函数存在于<malloc.h>头文件中。

**注意**： memalign()可能并不会检查alignment的值是否满足对齐要求，即应为2^N.






<br />
<br />
<br />

