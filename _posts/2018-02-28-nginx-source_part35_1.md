---
layout: post
title: core/ngx_connection.h头文件分析 
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们介绍一下nginx中对socket connection的抽象。


<!-- more -->

## 1. ngx_listening_t数据结构
下面我们对```ngx_listening_t```数据结构做一个简单的介绍：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;

struct ngx_listening_s {
    ngx_socket_t        fd;                        //代表当前监听socket句柄

    struct sockaddr    *sockaddr;                  //代表当前监听socket的地址
    socklen_t           socklen;                   //地址大小 size of sockaddr
    size_t              addr_text_max_len;         //字符串表示的ip地址的最大长度限制
    ngx_str_t           addr_text;                 //字符串表示的ip地址

    int                 type;                      //socket类型，当type值为SOCK_STREAM,表示的tcp；

    //TCP实现监听时的backlog队列，它表示允许正在通过三次握手建立tcp连接但还没有任何进程开始处理的连接最大个数
    int                 backlog;                   
    int                 rcvbuf;                    //接收缓存大小
    int                 sndbuf;                    //发送缓存大小
#if (NGX_HAVE_KEEPALIVE_TUNABLE)                   //当前我们支持keepalive特性
    int                 keepidle;                  
    int                 keepintvl;
    int                 keepcnt;
#endif

    /* handler of accepted connection （对新建立连接的处理方法）
     * 
     * 例如：
     * 对于http连接，其handler指定为ngx_http_init_connection
     * 对于mail连接，其handler指定为ngx_mail_init_connection
     * 对于stream连接，其handler指定为ngx_stream_init_connection
     */
    ngx_connection_handler_pt   handler;


    /* array of ngx_http_in_addr_t, for example
     * 主要用于http/mail/stream等模块，保存当前监听端口对应的所有主机名 
     */
    void               *servers;  

    ngx_log_t           log;                  //log是自身携带的一个日志，主要用于各个模块的日志信息格式化
    ngx_log_t          *logp;                 //通过error log命令配置的日志

    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout;

    ngx_listening_t    *previous;
    ngx_connection_t   *connection;

    ngx_uint_t          worker;

    unsigned            open:1;
    unsigned            remain:1;
    unsigned            ignore:1;

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;
    unsigned            wildcard:1;

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:1;
#endif
#if (NGX_HAVE_REUSEPORT)
    unsigned            reuseport:1;
    unsigned            add_reuseport:1;
#endif
    unsigned            keepalive:2;

#if (NGX_HAVE_DEFERRED_ACCEPT)
    unsigned            deferred_accept:1;
    unsigned            add_deferred:1;
#ifdef SO_ACCEPTFILTER
    char               *accept_filter;
#endif
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    int                 fastopen;
#endif

};
{% endhighlight %}

注意：
<pre>
关于TCP keepalive有如下几个选项：

1) TCP_KEEPIDLE: 设置连接上如果没有数据发送的话，多久后发送keepalive探测分组，单位是秒

2) TCP_KEEPINTVL: 前后两侧探测之间的时间间隔，单位是秒

3) TCP_KEEPCNT: 关闭一个非活跃连接之前的最大重试次数
</pre>




<br />
<br />

**[参考]:**

1. [TCP KeepAlive的几个附加选项](https://www.cnblogs.com/cobbliu/p/4655542.html)

2. [nginx源码初读](http://blog.csdn.net/wuchunlai_2012/article/details/50732741)



<br />
<br />
<br />

