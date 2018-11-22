---
layout: post
title: core/ngx_buf.h源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们讲述一下ngx_buf.h头文件，其主要是定义了nginx buf缓冲区结构及相关的操作声明。


<!-- more -->


## 1 ngx_buf_s数据结构
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_BUF_H_INCLUDED_
#define _NGX_BUF_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef void *            ngx_buf_tag_t;

typedef struct ngx_buf_s  ngx_buf_t;

struct ngx_buf_s {
    u_char          *pos;
    u_char          *last;
    off_t            file_pos;
    off_t            file_last;

    u_char          *start;         /* start of buffer */
    u_char          *end;           /* end of buffer */
    ngx_buf_tag_t    tag;
    ngx_file_t      *file;
    ngx_buf_t       *shadow;


    /* the buf's content could be changed */
    unsigned         temporary:1;

    /*
     * the buf's content is in a memory cache or in a read only memory
     * and must not be changed
     */
    unsigned         memory:1;

    /* the buf's content is mmap()ed and must not be changed */
    unsigned         mmap:1;

    unsigned         recycled:1;
    unsigned         in_file:1;
    unsigned         flush:1;
    unsigned         sync:1;
    unsigned         last_buf:1;
    unsigned         last_in_chain:1;

    unsigned         last_shadow:1;
    unsigned         temp_file:1;

    /* STUB */ int   num;
};
{% endhighlight %}

* **pos**: 当buf指向的数据在内存中的时候，pos指向的是这段数据的开始位置

* **last**: 当buf指向的数据在内存中的时候， pos指向的是这段数据的结束位置

* **file_pos**: 当buf指向的数据是在文件中的时候，file_pos指向的是这段数据的开始位置在文件中的偏移量

* **file_last**: 当buf指向的数据是在文件中的时候，file_last执行的是这段数据的结束位置在文件中的偏移量

* **start**: 当buf指向的数据是在内存里的时候，这一整块内存所包含的内容可能被包含在多个buf中（比如某段数据中间插入了其他数据，这一块数据就需要被拆分开），那么这些buf中的start和end都指向这一块内存的开始地址和结束地址。而pos和last指向没buf所实际包含的数据的开始和结尾。

* **end**： 解释参见start.

* **tag**: 实际上是一个void *类型的指针，使用者可以关联任意的对象上去，只要对使用者有意义

* **file**: 当buf所包含的内容在文件时，file字段指向对应的文件对象

* **shadow**: 当这个buf完整copy了另外一个buf的所有字段的时候，那么这两个buf实际指向的是同一块内存，或是同一个文件的同一部分，此时这两个buf的shadow字段都是指向对方的。那么对于这样的两个buf，在释放的时候，就需要使用者特别小心，具体由哪里释放，要提前考虑好，如果造成资源的多次释放，可能造成程序崩溃。使用shadow主要是为了节约内存，因为当有多个地方要操作这一块内存的时候，就可以新建一个shadow,对shadow的操作（这里并不修改所指向内存块的内容）不会影响到原buf。
{% highlight string %}
当前缓冲区的影子缓冲区，该成员很少用到。仅仅在使用缓冲区转发上游服务器的响应时才使用了shadow成员，这是因为nginx太节约内存了。
分配一块内存并使用ngx_buf_t表示接收到的上游服务器响应后，在向下游客户端转发时可能会把这块内存存储到文件中，也可能直接向下游发送，
此时nginx绝对不会重新复制一份内存用于新的目的，而是再次建立一个ngx_buf_t结构体指向原内存，这样多个ngx_buf_t结构体指向了同一块内
存，它们之间的关系就通过shadow成员来引用，这种设计过于复杂，通常不建议使用。
{% endhighlight %}

* **temporary**: 为1时表示该buf所包含的内容是在一个用户创建的内存块中，并且可以被在filter处理的过程中进行变更，而不会造成问题

* **memory**: 为1时表示该buf所包含的内容是在内存中，但是这些内容却不能被进行处理的filter进行变更

* **mmap**: 为1时表示该buf所包含的内容是在内存中，是通过mmap()使用内存映射从文件映射到内存中的，这些内容却不能被进行处理的filter进行变更。

* **recycled**: 可以回收的。也就是这个buf是可以被释放的。这个字段通常是配合shadow字段一起使用的，对于使用ngx_create_temp_buf()函数所创建的buf，并且是另外一个buf的shadow，那么使用这个字段来标示这个buf是可以释放的。

* **in_file**: 为1时表示该buf所包含的内容是在文件中

* **flush**： 遇到有flush字段被设置为1的buf chain，则该chain的数据即便不是最后结束的数据(last_buf被设置，标示所有要输出的内容都完了），也会进行输出，不会受postpone_output配置的限制，但是会受到发送速率等其他条件的限制。

* **sync**: 为1时表示可以对该buf进行同步操作，容易引起堵塞

* **last_buf**: 数据被以多个chain传递给了过滤器，此字段为1表示这是缓冲区链表ngx_chain_t上最后一块待处理的缓冲区

* **last_in_chain**： 在当前的chain里面，此buf是最后一个。特别要注意的是标志为last_in_chain的buf并不一定是last_buf，但是标志为last_buf的buf则一定是last_in_chain的。这是因为数据会被以多个chain传递给某个filter模块。

* **last_shadow**: 在创建一个buf的shadow的时候，通常将新创建的一个buf的last_shadow置为1，表示为最后一个影子缓冲区。

* **temp_file**: 由于受内存使用的限制，有时候一些buf的内容需要被写到磁盘上的临时文件中去，那么这时就设置此标志。


## 2. ngx_chain_s数据结构
{% highlight string %}
struct ngx_chain_s {
    ngx_buf_t    *buf;
    ngx_chain_t  *next;
};


typedef struct {
    ngx_int_t    num;
    size_t       size;
} ngx_bufs_t;
{% endhighlight %} 
```ngx_chain_s```数据结构形成一个nginx buf链。```ngx_bufs_t```数据结构一般在创建buf链的时候使用： num表明当前的buf数目； size表明每一个buf的空间大小。

## 3. ngx_output_chain_ctx_s数据结构
{% highlight string %}
typedef struct ngx_output_chain_ctx_s  ngx_output_chain_ctx_t;

typedef ngx_int_t (*ngx_output_chain_filter_pt)(void *ctx, ngx_chain_t *in);

#if (NGX_HAVE_FILE_AIO)
typedef void (*ngx_output_chain_aio_pt)(ngx_output_chain_ctx_t *ctx,
    ngx_file_t *file);
#endif

struct ngx_output_chain_ctx_s {
    ngx_buf_t                   *buf;
    ngx_chain_t                 *in;
    ngx_chain_t                 *free;
    ngx_chain_t                 *busy;

    unsigned                     sendfile:1;
    unsigned                     directio:1;
#if (NGX_HAVE_ALIGNED_DIRECTIO)
    unsigned                     unaligned:1;
#endif
    unsigned                     need_in_memory:1;
    unsigned                     need_in_temp:1;
#if (NGX_HAVE_FILE_AIO || NGX_THREADS)
    unsigned                     aio:1;
#endif

#if (NGX_HAVE_FILE_AIO)
    ngx_output_chain_aio_pt      aio_handler;
#if (NGX_HAVE_AIO_SENDFILE)
    ssize_t                    (*aio_preload)(ngx_buf_t *file);
#endif
#endif

#if (NGX_THREADS)
    ngx_int_t                  (*thread_handler)(ngx_thread_task_t *task,
                                                 ngx_file_t *file);
    ngx_thread_task_t           *thread_task;
#endif

    off_t                        alignment;

    ngx_pool_t                  *pool;
    ngx_int_t                    allocated;
    ngx_bufs_t                   bufs;
    ngx_buf_tag_t                tag;

    ngx_output_chain_filter_pt   output_filter;
    void                        *filter_ctx;
};
{% endhighlight %}

```ngx_output_chain_ctx_s```数据结构主要用在ngx_output_chain()函数中，该函数主要用在http upstream模块及http copy filter模块。函数的主要目的就是发送一个buf chain(缓冲链）数据，而ngx_output_chain_ctx_s就用于保存发送的上下文，因此可以说该结构主要用于管理输出buf。

下面我们来简要介绍一下```ngx_output_chain_ctx_s```结构的各字段：

* buf: 这个域就是我们拷贝数据的地方，我们一般输出的话都是从```in```(参看如下)直接copy相应的size大小的数据到buf中。一般作为发送时临时缓存使用

* in: 这个就是我们保存那些需要发送数据的地方

* free: 这个保存了一些空闲的buf，也就是说如果free存在，我们都会直接从free中取buf到前面的buf域。

* busy： 表示马上就要发送的chain。

* sendfile: 用于指定是否使用sendfile

* directio: 是否使用directio

* unaligned: 表明当前数据有没对齐

* need_in_memory: 是否需要当前的数据处于内存中(使用sendfile的话，内存中没有文件拷贝，而我们有时需要处理文件，此时就需要设置这个标记)

* need_in_temp: 表明数据是否需要在用户创建的一个临时内存中（这就使得可以对这一部分数据进行变更）

<pre>
对于NGX_HAVE_FILE_AIO及NGX_THREADS，当前我们并没有定义
</pre>

* allocated： 每次从pool中重新alloc一个buf，这个值都会相应加1

* tag: 模块标记，主要用于buf回收

* output_filter: 输出回调函数

* filter_ctx: 过滤器回调函数上下文

## 3. ngx_chain_writer_ctx_t数据结构
{% highlight string %}
typedef struct {
    ngx_chain_t                 *out;
    ngx_chain_t                **last;
    ngx_connection_t            *connection;
    ngx_pool_t                  *pool;
    off_t                        limit;
} ngx_chain_writer_ctx_t;
{% endhighlight %}
```ngx_chain_write_ctx_t```主要用于upstream模块中。


## 4. 相关宏定义
如下我们简单解释一下各宏定义用途：
{% highlight string %}
#define NGX_CHAIN_ERROR     (ngx_chain_t *) NGX_ERROR


// 用于判断buf b所指向的数据是否在内存中
#define ngx_buf_in_memory(b)        (b->temporary || b->memory || b->mmap)

//用于判断buf b所指向的数据是否仅仅存放与内存中
#define ngx_buf_in_memory_only(b)   (ngx_buf_in_memory(b) && !b->in_file)

//用于判断buf b是否是一个特殊的buf，只含有特殊的标志，并没有包含真正的数据
#define ngx_buf_special(b)                                                   \
    ((b->flush || b->last_buf || b->sync)                                    \
     && !ngx_buf_in_memory(b) && !b->in_file)

//用于判断buf b是否只包含sync标志，而不包含真正数据的特殊buf
#define ngx_buf_sync_only(b)                                                 \
    (b->sync                                                                 \
     && !ngx_buf_in_memory(b) && !b->in_file && !b->flush && !b->last_buf)


//返回buf所包含数据的大小，不管这个数据是在文件里还是在内存里
#define ngx_buf_size(b)                                                      \
    (ngx_buf_in_memory(b) ? (off_t) (b->last - b->pos):                      \
                            (b->file_last - b->file_pos))
{% endhighlight %}

## 5. 相关函数声明
本部分主要是相关函数的声明：
{% highlight string %}

//1) 用于创建一个临时内存缓存，该内存缓冲的数据有可能被后续filter处理过程中进行修改
ngx_buf_t *ngx_create_temp_buf(ngx_pool_t *pool, size_t size);

//2) 创建一个buf链
ngx_chain_t *ngx_create_chain_of_bufs(ngx_pool_t *pool, ngx_bufs_t *bufs);


//3) 创建一个ngx_buf_t数据结构
#define ngx_alloc_buf(pool)  ngx_palloc(pool, sizeof(ngx_buf_t))
#define ngx_calloc_buf(pool) ngx_pcalloc(pool, sizeof(ngx_buf_t))


//4) 从pool中获取一个ngx_chain_t结构
ngx_chain_t *ngx_alloc_chain_link(ngx_pool_t *pool);
#define ngx_free_chain(pool, cl)                                             \
    cl->next = pool->chain;                                                  \
    pool->chain = cl


//5) 用于发送in 链中的数据
ngx_int_t ngx_output_chain(ngx_output_chain_ctx_t *ctx, ngx_chain_t *in);


//6) 将in中的数据copy到ctx，然后将ctx中的数据发送出去
ngx_int_t ngx_chain_writer(void *ctx, ngx_chain_t *in);


//7) 将in链中的数据拷贝到chain中
ngx_int_t ngx_chain_add_copy(ngx_pool_t *pool, ngx_chain_t **chain,
    ngx_chain_t *in);

//8) 获得一个空闲的chain
ngx_chain_t *ngx_chain_get_free_buf(ngx_pool_t *p, ngx_chain_t **free);


//9） 更新链的相关状态
void ngx_chain_update_chains(ngx_pool_t *p, ngx_chain_t **free,
    ngx_chain_t **busy, ngx_chain_t **out, ngx_buf_tag_t tag);

//10) 合并in中中的相邻数据
off_t ngx_chain_coalesce_file(ngx_chain_t **in, off_t limit);

//11） 根据已经成功发送数据的大小，更新in链
ngx_chain_t *ngx_chain_update_sent(ngx_chain_t *in, off_t sent);
{% endhighlight %}

<br />
<br />

**[参看]:**

1. [Nginx源码分析 - 基础数据结构篇 - 缓冲区结构 ngx_buf.c](http://blog.csdn.net/initphp/article/details/50611021)

2. [Nginx基本数据结构之ngx_buf_t](http://blog.csdn.net/wangpengqi/article/details/17247407)

3. [Nginx关键数据结构分析（一） ngx_buf_t](https://segmentfault.com/a/1190000004014230)

4. [Nginx filter分析](https://blog.csdn.net/fengmo_q/article/details/12494781)

5. [Nginx开发从入门到精通](https://www.kancloud.cn/kancloud/master-nginx-develop/51880)

6. [结合源码看nginx-1.4.0之nginx内存管理详解](https://www.cnblogs.com/didiaoxiong/p/nginx_memory.html)

7. [nginx的内存管理](http://simohayha.iteye.com/blog/545192)

8. [ngx_output_chain 函数分析](https://my.oschina.net/astute/blog/316954)
<br />
<br />
<br />

