---
layout: post
title: event/ngx_event_connect.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


在前面相关章节，我们知道nginx定义了```ngx_connection_t```数据结构来表示连接，这种连接通常表示由客户端主动发起、Nginx服务器被动接收的TCP连接（当然UDP连接也会用该结构来表示），称为被动连接。还有一类连接，在某些请求的处理过程中，Nginx会试图主动向其他上游服务器建立连接，并以此连接与上游服务器进行通信，Nginx定义**ngx_peer_connection_t**结构来表示，这类可以称为```主动连接```。本质上来说，```主动连接```是以**ngx_connection_t**结构体为基础实现的。


<!-- more -->


## 1. 
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_CONNECT_H_INCLUDED_
#define _NGX_EVENT_CONNECT_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define NGX_PEER_KEEPALIVE           1
#define NGX_PEER_NEXT                2
#define NGX_PEER_FAILED              4


typedef struct ngx_peer_connection_s  ngx_peer_connection_t;

typedef ngx_int_t (*ngx_event_get_peer_pt)(ngx_peer_connection_t *pc,
    void *data);
typedef void (*ngx_event_free_peer_pt)(ngx_peer_connection_t *pc, void *data,
    ngx_uint_t state);
#if (NGX_SSL)

typedef ngx_int_t (*ngx_event_set_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
typedef void (*ngx_event_save_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
#endif
{% endhighlight %}

```ngx_peer_connection_t```在使用过程中会有不同状态，这里定义相应的状态掩码：

* NGX_PEER_KEEPALIVE: keepalive的状态掩码

* NGX_PEER_NEXT: next状态掩码

* NGX_PEER_FAILED: failed状态掩码

下面我们简要介绍一下上述的一些函数指针：

* ngx_event_get_peer_pt: 通过该方法从连接池中获取一个新连接。一般会考虑采用round-robin、IP哈希或其他其他负载均衡的方式

* ngx_event_free_peer_pt： 通过该方法将使用完毕的连接释放回连接池。

* ngx_event_set_peer_session_pt： 通常用于SSL的连接，用于设置会话信息。当前我们支持**NGX_SSL**宏定义

* ngx_event_save_peer_session_pt: 通常用于SSL的连接，用于保存会话信息。当前我们支持**NGX_SSL**宏定义

## 2. ngx_peer_connection_s数据结构
{% highlight string %}
struct ngx_peer_connection_s {
    ngx_connection_t                *connection;

    struct sockaddr                 *sockaddr;
    socklen_t                        socklen;
    ngx_str_t                       *name;

    ngx_uint_t                       tries;
    ngx_msec_t                       start_time;

    ngx_event_get_peer_pt            get;
    ngx_event_free_peer_pt           free;
    void                            *data;

#if (NGX_SSL)
    ngx_event_set_peer_session_pt    set_session;
    ngx_event_save_peer_session_pt   save_session;
#endif

    ngx_addr_t                      *local;

    int                              type;
    int                              rcvbuf;

    ngx_log_t                       *log;

    unsigned                         cached:1;

                                     /* ngx_connection_log_error_e */
    unsigned                         log_error:2;
};
{% endhighlight %}


<br />
<br />

**[参看]**

1. [Nginx学习笔记(十九)：Nginx连接](https://blog.csdn.net/fzy0201/article/details/23782397)




<br />
<br />
<br />

