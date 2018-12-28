---
layout: post
title: event/ngx_event_pipe.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

nginx_event_pipe用于实现upstream对上游服务器包体数据的读取，然后在处理之后，将结果返回给请求端(downstream)。




<!-- more -->


## 1. 相关函数指针类型的定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_PIPE_H_INCLUDED_
#define _NGX_EVENT_PIPE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>



typedef struct ngx_event_pipe_s  ngx_event_pipe_t;


// 处理接收自上游服务器的包体的回调函数原型
typedef ngx_int_t (*ngx_event_pipe_input_filter_pt)(ngx_event_pipe_t *p,
                                                    ngx_buf_t *buf);

//向下游发送响应的回调函数原型
typedef ngx_int_t (*ngx_event_pipe_output_filter_pt)(void *data,
                                                    ngx_chain_t *chain);
{% endhighlight %}


## 2. ngx_event_pipe_t数据结构
{% highlight string %}
struct ngx_event_pipe_s {
    ngx_connection_t  *upstream;
    ngx_connection_t  *downstream;

    ngx_chain_t       *free_raw_bufs;
    ngx_chain_t       *in;
    ngx_chain_t      **last_in;

    ngx_chain_t       *writing;

    ngx_chain_t       *out;
    ngx_chain_t       *free;
    ngx_chain_t       *busy;

    /*
     * the input filter i.e. that moves HTTP/1.1 chunks
     * from the raw bufs to an incoming chain
     */

    ngx_event_pipe_input_filter_pt    input_filter;
    void                             *input_ctx;

    ngx_event_pipe_output_filter_pt   output_filter;
    void                             *output_ctx;

#if (NGX_THREADS)
    ngx_int_t                       (*thread_handler)(ngx_thread_task_t *task,
                                                      ngx_file_t *file);
    void                             *thread_ctx;
    ngx_thread_task_t                *thread_task;
#endif

    unsigned           read:1;
    unsigned           cacheable:1;
    unsigned           single_buf:1;
    unsigned           free_bufs:1;
    unsigned           upstream_done:1;
    unsigned           upstream_error:1;
    unsigned           upstream_eof:1;
    unsigned           upstream_blocked:1;
    unsigned           downstream_done:1;
    unsigned           downstream_error:1;
    unsigned           cyclic_temp_file:1;
    unsigned           aio:1;

    ngx_int_t          allocated;
    ngx_bufs_t         bufs;
    ngx_buf_tag_t      tag;

    ssize_t            busy_size;

    off_t              read_length;
    off_t              length;

    off_t              max_temp_file_size;
    ssize_t            temp_file_write_size;

    ngx_msec_t         read_timeout;
    ngx_msec_t         send_timeout;
    ssize_t            send_lowat;

    ngx_pool_t        *pool;
    ngx_log_t         *log;

    ngx_chain_t       *preread_bufs;
    size_t             preread_size;
    ngx_buf_t         *buf_to_file;

    size_t             limit_rate;
    time_t             start_sec;

    ngx_temp_file_t   *temp_file;

    /* STUB */ int     num;
};
{% endhighlight %}
本数据结构是用于实现读取upstream数据包并向downstream返回响应的关键性数据结构。下面简要介绍一下各字段的含义：

* upstream: Nginx与上游服务器之间的连接

* downstream: Nginx与下游客户端之间的连接

* free_raw_bufs: 空闲的用于存放原始数据的缓冲链。（注意： free_raw_bufs中第一个节点可能残留有部分上次未处理的遗留数据）

* in: 用于存放经过```input_filter```处理的上游服务器的响应

* last_in: 指向```in```中的最后一个缓冲区

* writing： 用于将数据写入临时文件的链

* out: 保存着将要发给客户端的缓冲区链表。在写入临时文件成功时，会把in中的缓冲区添加到out中

* free: 空闲缓冲链

* busy: 正在进行处理的缓冲链

* input_filter: 处理接收到的、来自上游服务器的数据

* input_ctx: 用于input_filter的参数，一般是ngx_http_request_t的地址

* output_filter: 向下游服务器发送响应的函数

* output_ctx: output_filter的参数，一般指向ngx_http_request_t结构 

* thread_handler: 如果采用多线程来将数据写入到临时文件，那么此handler保存相应的回调方法。当前```NGX_THREADS```我们暂未定义

* thread_ctx: 用于保存线程上下文

* thread_task: 用于保存线程任务

* read: 表示已经读取到了来自上游的响应

* cacheable: 表示是否启用文件缓存

* single_buf: 表示接收上游响应时，一次只能接收一个ngx_buf_t缓冲区。通常```IOCP事件驱动机制```下，此值为1

* free_bufs： 一旦不再接收上游包体，将尽可能地释放缓冲区。用此标志位来指示

* upstream_done: 表示Nginx与上游交互已经结束，即上游包体数据读取完毕

* upstream_error： 表示读取上游数据出错的标记

* upstream_eof: 内核网络缓冲区读取完毕，通常是上游服务器关闭了连接

* upstream_blocked: 表示暂时阻塞读取上游响应的流程。此时会先调用ngx_event_pipe_write_to_downstream()函数发送缓冲区中的数据给下游，从而腾出缓冲区空间，再调用ngx_event_pipe_read_upstream()函数读取上游数据。

* downstream_done： 表示downstream处理完毕，与下游的交互已经结束

* downstream_error: downstream处理出错

* cyclic_temp_file: 复用临时文件。它是由ngx_http_upstream_conf_t中的同名成员赋值的

* aio: aio异步标记

* allocated: 配置的buffer已分配使用的个数。参看如下```bufs```字段

* bufs: 记录了接收上游响应的内存缓冲区大小，bufs.size表示每个内存缓冲区大小，bufs.num表示最多可以有num个缓冲区

* tag: 用于设置、比较缓冲区链表中的```ngx_buf_t```结构体的tag标志位

* busy_size: busy缓冲区中等待发送响应长度的最大值，当达到busy_size时，必须等待busy缓冲区发送了足够的数据，才能继续发送out和in中的内容

* read_length： 已经接收到来自上游响应包体的长度

* length： 动态变化的剩余上游包体长度

* max_temp_file_size: 表示临时文件的最大长度

* temp_file_write_size: 表示一次写入文件时数据的最大长度

* read_timeout: 读取上游响应的超时时间

* send_timeout: 向下游发送响应的超时时间

* send_lowat: 向下游发送响应时，TCP连接中设置的send_lowat```水位```

* pool: 所关联的内存池结构

* log: 所关联的日志对象

* preread_bufs: 通常用于存储在接收上游服务器响应头部阶段，已经读取到响应包体， 此时存储这个预先读取到的响应包体。

* preread_size: 表示在接收上游服务器响应头部阶段，已经读取到响应包体长度

* buf_to_file: 将数据写入临时文件所用的缓冲

* limit_rate: upstream读取限速

* start_sec: 用于记录```event_pipe```开始处理的时间

* temp_file： 存放上游响应的临时文件

* num:  STUB信息, 已使用的ngx_buf_t缓冲区数目


## 3. 相关函数声明
{% highlight string %}
//用于实现upstream对上游服务器包体数据的读取，然后在处理之后，将结果返回给请求端(downstream)
ngx_int_t ngx_event_pipe(ngx_event_pipe_t *p, ngx_int_t do_write);

//用于拷贝input filter中buf缓存
ngx_int_t ngx_event_pipe_copy_input_filter(ngx_event_pipe_t *p, ngx_buf_t *buf);

//将对应的buffer 'b'归还回p->free_raw_bufs
ngx_int_t ngx_event_pipe_add_free_buf(ngx_event_pipe_t *p, ngx_buf_t *b);
{% endhighlight %}





<br />
<br />

**[参看]**


1. [《深入理解Nginx》笔记之ngx_event_pipe_s结构体](https://blog.csdn.net/yzt33/article/details/47835311)

2. [ngx_event_pipe_write_to_downstream分析](https://blog.csdn.net/kai_ding/article/details/21316493)

3. [ngx_event_pipe_read_upstream分析](https://blog.csdn.net/kai_ding/article/details/20404875)

4. [nginx upstream模块详解（基础结构篇）](https://blog.csdn.net/huzilinitachi/article/details/79543153)

5. [nginx upstream模块详解（处理流程篇二 upstream与event_pipe交互）](https://blog.csdn.net/huzilinitachi/article/details/79565365)

6. [nginx的upstream模块数据转发过程及流量控制分析](https://blog.csdn.net/zhaomangzheng/article/details/24390783)

7. [Nginx upstream原理分析-带buffering给客户端返回数据](http://chenzhenianqing.com/articles/689.html)




<br />
<br />
<br />

