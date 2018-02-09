---
layout: post
title: core/ngx_buf.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要讲述一下nginx缓冲区结构及其相应的操作。


<!-- more -->


## 1. core/ngx_buf.h头文件

下面我们分几个部分来介绍ngx_buf.h头文件

### 1.1 ngx_buf_s数据结构
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

* pos: 当buf指向的数据在内存中的时候，pos指向的是这段数据的开始位置

* last: 当buf指向的数据在内存中的时候， pos指向的是这段数据的结束位置

* file_pos: 当buf指向的数据是在文件中的时候，file_pos指向的是这段数据的开始位置在文件中的偏移量

* file_last: 当buf指向的数据是在文件中的时候，file_last执行的是这段数据的结束位置在文件中的偏移量

* start: 当buf指向的数据是在内存里的时候，这一整块内存所包含的内容可能被包含在多个buf中（比如某段数据中间插入了其他数据，这一块数据就需要被拆分开），那么这些buf中的start和end都指向这一块内存的开始地址和结束地址。而pos和last指向没buf所实际包含的数据的开始和结尾。

* end： 解释参见start.

* tag: 实际上是一个void *类型的指针，使用者可以关联任意的对象上去，只要对使用者有意义

* file: 当buf所包含的内容在文件时，file字段指向对应的文件对象

* shadow: 当这个buf完整copy了另外一个buf的所有字段的时候，那么这两个buf实际指向的是同一块内存，或是同一个文件的同一部分，此时这两个buf的shadow字段都是指向对方的。那么对于这样的两个buf，在释放的时候，就需要使用者特别小心，具体由哪里释放，要提前考虑好，如果造成资源的多次释放，可能造成程序崩溃。使用shadow主要是为了节约内存，因为当有多个地方要操作这一块内存的时候，就可以新建一个shadow,对shadow的操作（这里并不修改所指向内存块的内容）不会影响到原buf。
<pre>
当前缓冲区的影子缓冲区，该成员很少用到。仅仅在使用缓冲区转发上游服务器的响应时才使用了shadow成员，这是因为nginx太节约内存了。分配一块内存并使用ngx_buf_t表示接收到的上游服务器响应后，在向下游客户端转发时可能会把这块内存存储到文件中，也可能直接向下游发送，此时nginx绝对不会重新复制一份内存用于新的目的，而是再次建立一个ngx_buf_t结构体指向原内存，这样多个ngx_buf_t结构体指向了同一块内存，它们之间的关系就通过shadow成员来引用，这种设计过于复杂，通常不建议使用。
</pre>

* temporary: 为1时表示该buf所包含的内容是在一个用户创建的内存块中，并且可以被在filter处理的过程中进行变更，而不会造成问题

* memory: 为1时表示该buf所包含的内容是在内存中，但是这些内容却不能被进行处理的filter进行变更

* mmap: 为1时表示该buf所包含的内容是在内存中，是通过mmap()使用内存映射从文件映射到内存中的，这些内容却不能被进行处理的filter进行变更。

* recycled: 可以回收的。也就是这个buf是可以被释放的。这个字段通常是配合shadow字段一起使用的，对于使用ngx_create_temp_buf()函数所创建的buf，并且是另外一个buf的shadow，那么使用这个字段来标示这个buf是可以释放的。

* in_file: 为1时表示该buf所包含的内容是在文件中

* flush： 遇到有flush字段被设置为1的buf chain，则该chain的数据即便不是最后结束的数据(last_buf被设置，标示所有要输出的内容都完了），也会进行输出，不会受postpone_output配置的限制，但是会受到发送速率等其他条件的限制。

* sync: 为1时表示可以对该buf进行同步操作，容易引起堵塞

* last_buf: 数据被以多个chain传递给了过滤器，此字段为1表示这是缓冲区链表ngx_chain_t上最后一块待处理的缓冲区

* last_in_chain： 在当前的chain里面，此buf是最后一个。特别要注意的是标志为last_in_chain的buf并不一定是last_buf，但是标志为last_buf的buf则一定是last_in_chain的。这是因为数据会被以多个chain传递给某个filter模块。

* last_shadow: 在创建一个buf的shadow的时候，通常将新创建的一个buf的last_shadow置为1，表示为最后一个影子缓冲区。

* temp_file: 由于受内存使用的限制，有时候一些buf的内容需要被写到磁盘上的临时文件中去，那么这时就设置此标志。



<br />
<br />

**[参看]:**

1. [Nginx源码分析 - 基础数据结构篇 - 缓冲区结构 ngx_buf.c](http://blog.csdn.net/initphp/article/details/50611021)

2. [Nginx基本数据结构之ngx_buf_t](http://blog.csdn.net/wangpengqi/article/details/17247407)

3. [Nginx关键数据结构分析（一） ngx_buf_t](https://segmentfault.com/a/1190000004014230)

4. [Nginx开发从入门到精通](https://www.kancloud.cn/kancloud/master-nginx-develop/51880)

5. [结合源码看nginx-1.4.0之nginx内存管理详解](https://www.cnblogs.com/didiaoxiong/p/nginx_memory.html)
<br />
<br />
<br />

