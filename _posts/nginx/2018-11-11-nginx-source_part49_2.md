---
layout: post
title: core/ngx_palloc.c头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本章我们分析一下nginx内存池的实现。

<!-- more -->


## 1. 相关静态函数声明
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>

//在内存池中分配指定大小的内存
static ngx_inline void *ngx_palloc_small(ngx_pool_t *pool, size_t size,
    ngx_uint_t align);

//在内存池中分配一个内存块
static void *ngx_palloc_block(ngx_pool_t *pool, size_t size);

//在内存池中分配一个大块内存
static void *ngx_palloc_large(ngx_pool_t *pool, size_t size);

{% endhighlight %}

## 2. 函数ngx_create_pool()
{% highlight string %}
ngx_pool_t *
ngx_create_pool(size_t size, ngx_log_t *log)
{
    ngx_pool_t  *p;

    p = ngx_memalign(NGX_POOL_ALIGNMENT, size, log);
    if (p == NULL) {
        return NULL;
    }

    p->d.last = (u_char *) p + sizeof(ngx_pool_t);
    p->d.end = (u_char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_t);
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->chain = NULL;
    p->large = NULL;
    p->cleanup = NULL;
    p->log = log;

    return p;
}
{% endhighlight %}
本函数用于创建一个指定大小的内存池。去除掉```ngx_pool_t```结构本身占用的空间外，实际可用于分配的空间大小为：
<pre>
size = size - sizeof(ngx_pool_t);
</pre>
此外，这里还用```p->max```限定了如果后续要从该内存池中分配空间的话:
<pre>
1. 若请求分配的空间小于等于p->max，那么会从pool->d中进行分配

2. 否则，从pool->large内存块中进行分配
</pre>

## 3. 函数ngx_destroy_pool()
{% highlight string %}
void
ngx_destroy_pool(ngx_pool_t *pool)
{
    ngx_pool_t          *p, *n;
    ngx_pool_large_t    *l;
    ngx_pool_cleanup_t  *c;

    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                           "run cleanup: %p", c);
            c->handler(c->data);
        }
    }

#if (NGX_DEBUG)

    /*
     * we could allocate the pool->log from this pool
     * so we cannot use this log while free()ing the pool
     */

    for (l = pool->large; l; l = l->next) {
        ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0, "free: %p", l->alloc);
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                       "free: %p, unused: %uz", p, p->d.end - p->d.last);

        if (n == NULL) {
            break;
        }
    }

#endif

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ngx_free(l->alloc);
        }
    }

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_free(p);

        if (n == NULL) {
            break;
        }
    }
}
{% endhighlight %}

此函数用于销毁一个pool。下面我们来简要分析一下：

* 调用该pool所关联的cleanup回调

* 释放pool->large内存块链表

* 释放pool中小内存块链表(pool->d)


## 4. 函数ngx_reset_pool()
{% highlight string %}
void
ngx_reset_pool(ngx_pool_t *pool)
{
    ngx_pool_t        *p;
    ngx_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ngx_free(l->alloc);
        }
    }

    for (p = pool; p; p = p->d.next) {
        p->d.last = (u_char *) p + sizeof(ngx_pool_t);
        p->d.failed = 0;
    }

    pool->current = pool;
    pool->chain = NULL;
    pool->large = NULL;
}
{% endhighlight %}
重置pool。主要完成：

* 释放pool->large内存块链表

* 将pool->d分配出去的小内存都进行复位


## 5. 函数ngx_palloc()
{% highlight string %}
void *
ngx_palloc(ngx_pool_t *pool, size_t size)
{
#if !(NGX_DEBUG_PALLOC)
    if (size <= pool->max) {
        return ngx_palloc_small(pool, size, 1);
    }
#endif

    return ngx_palloc_large(pool, size);
}
{% endhighlight %}
当前我们并未定义```NGX_DEBUG_PALLOC```宏，因此这里会执行该宏块中的代码。这里分配指定大小的内存时，如果分配的内存比较小(size <= pool->max)，则从```pool->d```这种小块内存池分配； 否则从```pool->large```这种大内存块进行分配。

这里注意，一般内存分配时返回的内存起始地址都是对齐过了的。

## 6. ngx_pnalloc()
{% highlight string %}
void *
ngx_pnalloc(ngx_pool_t *pool, size_t size)
{
#if !(NGX_DEBUG_PALLOC)
    if (size <= pool->max) {
        return ngx_palloc_small(pool, size, 0);
    }
#endif

    return ngx_palloc_large(pool, size);
}
{% endhighlight %}
此函数与```ngx_pnalloc()```的区别在于，分配的内存可能没有对齐。

## 7. 函数ngx_palloc_small()
{% highlight string %}
static ngx_inline void *
ngx_palloc_small(ngx_pool_t *pool, size_t size, ngx_uint_t align)
{
    u_char      *m;
    ngx_pool_t  *p;

    p = pool->current;

    do {
        m = p->d.last;

        if (align) {
            m = ngx_align_ptr(m, NGX_ALIGNMENT);
        }

        if ((size_t) (p->d.end - m) >= size) {
            p->d.last = m + size;

            return m;
        }

        p = p->d.next;

    } while (p);

    return ngx_palloc_block(pool, size);
}
{% endhighlight %}
从pool的小内存块中分配指定大小的内存。首先遍历```pool->d```的小内存块链，看是否能分配出指定大小的内存，如果能够分配则直接返回分配的空间首地址； 否则新分配一个小内存块，在其上分配出指定大小的内存返回（并将该新分配的内存块放入池中）

## 8. 函数ngx_palloc_block()
{% highlight string %}
static void *
ngx_palloc_block(ngx_pool_t *pool, size_t size)
{
    u_char      *m;
    size_t       psize;
    ngx_pool_t  *p, *new;

    psize = (size_t) (pool->d.end - (u_char *) pool);

    m = ngx_memalign(NGX_POOL_ALIGNMENT, psize, pool->log);
    if (m == NULL) {
        return NULL;
    }

    new = (ngx_pool_t *) m;

    new->d.end = m + psize;
    new->d.next = NULL;
    new->d.failed = 0;

    m += sizeof(ngx_pool_data_t);
    m = ngx_align_ptr(m, NGX_ALIGNMENT);
    new->d.last = m + size;

    for (p = pool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            pool->current = p->d.next;
        }
    }

    p->d.next = new;

    return m;
}
{% endhighlight %}
分配一个新的block，并在此block上分配指定大小的空间，然后在将此block放入到内存池中。注意pool中一个small block的总大小固定为：
{% highlight string %}
size = p->d.end - (char *)pool; 
{% endhighlight %}
而实际用于分配的空间是小于```size```的。

另外，这里注意实际分配空间的起点是：
{% highlight string %}
m += sizeof(ngx_pool_data_t);
m = ngx_align_ptr(m, NGX_ALIGNMENT);
{% endhighlight %}
再次印证了```pool->d.next```其实指向的真正类型为```ngx_pool_data_t```。

## 9. 函数ngx_palloc_large()
{% highlight string %}
static void *
ngx_palloc_large(ngx_pool_t *pool, size_t size)
{
    void              *p;
    ngx_uint_t         n;
    ngx_pool_large_t  *large;

    p = ngx_alloc(size, pool->log);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    large = ngx_palloc_small(pool, sizeof(ngx_pool_large_t), 1);
    if (large == NULL) {
        ngx_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}
{% endhighlight %}

这里首先分配指定大小的一块```large block```，接着将其放入pool的large-block链中。做的比较有意思的一点是，在插入large-block链时，如果该链的```ngx_pool_large_t```已经没有空闲的alloc可供保存新分配的空间，或者已经遍历了该链的前5个节点都找不到相应的保存位置，那么会新分配一个```ngx_pool_large_t```结构来存储新分配的空间并将该节点链接到large-block链中。

## 10. 函数ngx_pmemalign()
{% highlight string %}
void *
ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment)
{
    void              *p;
    ngx_pool_large_t  *large;

    p = ngx_memalign(alignment, size, pool->log);
    if (p == NULL) {
        return NULL;
    }

    large = ngx_palloc_small(pool, sizeof(ngx_pool_large_t), 1);
    if (large == NULL) {
        ngx_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

{% endhighlight %}
本函数分配一块指定大小，按alignment字节对齐的内存，将其插入到pool->large链表的表头

## 11. 函数ngx_pfree()
{% highlight string %}
ngx_int_t
ngx_pfree(ngx_pool_t *pool, void *p)
{
    ngx_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                           "free: %p", l->alloc);
            ngx_free(l->alloc);
            l->alloc = NULL;

            return NGX_OK;
        }
    }

    return NGX_DECLINED;
}
{% endhighlight %}
这里释放pool->large节点占用的空间，注意并未释放节点本身。

## 12. 函数ngx_pcalloc()
{% highlight string %}
void *
ngx_pcalloc(ngx_pool_t *pool, size_t size)
{
    void *p;

    p = ngx_palloc(pool, size);
    if (p) {
        ngx_memzero(p, size);
    }

    return p;
}

{% endhighlight %}
在pool中分配指定大小的内存，并将其清0.

## 13. 函数ngx_pool_cleanup_add()
{% highlight string %}
ngx_pool_cleanup_t *
ngx_pool_cleanup_add(ngx_pool_t *p, size_t size)
{
    ngx_pool_cleanup_t  *c;

    c = ngx_palloc(p, sizeof(ngx_pool_cleanup_t));
    if (c == NULL) {
        return NULL;
    }

    if (size) {
        c->data = ngx_palloc(p, size);
        if (c->data == NULL) {
            return NULL;
        }

    } else {
        c->data = NULL;
    }

    c->handler = NULL;
    c->next = p->cleanup;

    p->cleanup = c;

    ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, p->log, 0, "add cleanup: %p", c);

    return c;
}

{% endhighlight %}
此函数用于向pool->cleanup链中添加一个新成员。

## 14. 函数ngx_pool_run_cleanup_file()
{% highlight string %}
void
ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd)
{
    ngx_pool_cleanup_t       *c;
    ngx_pool_cleanup_file_t  *cf;

    for (c = p->cleanup; c; c = c->next) {
        if (c->handler == ngx_pool_cleanup_file) {

            cf = c->data;

            if (cf->fd == fd) {
                c->handler(cf);
                c->handler = NULL;
                return;
            }
        }
    }
}
{% endhighlight %}
执行pool->cleanup链中handler为ngx_pool_cleanup_file的回调

## 15. 函数ngx_pool_cleanup_file()
{% highlight string %}
void
ngx_pool_cleanup_file(void *data)
{
    ngx_pool_cleanup_file_t  *c = data;

    ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, c->log, 0, "file cleanup: fd:%d",
                   c->fd);

    if (ngx_close_file(c->fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, c->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", c->name);
    }
}
{% endhighlight %}
一个专门用于清除```nginx静态文件缓存```的cleanup回调。

## 16. 函数ngx_pool_delete_file()
{% highlight string %}
void
ngx_pool_delete_file(void *data)
{
    ngx_pool_cleanup_file_t  *c = data;

    ngx_err_t  err;

    ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, c->log, 0, "file cleanup: fd:%d %s",
                   c->fd, c->name);

    if (ngx_delete_file(c->name) == NGX_FILE_ERROR) {
        err = ngx_errno;

        if (err != NGX_ENOENT) {
            ngx_log_error(NGX_LOG_CRIT, c->log, err,
                          ngx_delete_file_n " \"%s\" failed", c->name);
        }
    }

    if (ngx_close_file(c->fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, c->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", c->name);
    }
}

{% endhighlight %}
一个与pool关联的，用于删除文件的cleanup回调。


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

