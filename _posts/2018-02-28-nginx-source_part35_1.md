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

    size_t              pool_size;            //要为新的tcp连接建立内存池的话，内存池的大小为pool_size


    /* should be here because of the AcceptEx() preread 
     * 主要是为了支持Windows套接字AcceptEx()函数接受一个新的连接
     */
    size_t              post_accept_buffer_size;


    /* should be here because of the deferred accept 
     *
     * TCP连接建立后，在post_accept_timeout之后仍然没有收到用户数据，则内核直接丢弃连接
     */
    ngx_msec_t          post_accept_timeout;

    ngx_listening_t    *previous;            //指向前一个ngx_listening_t结构
    ngx_connection_t   *connection;          //当前监听句柄对应的connection结构

    ngx_uint_t          worker;              //指定当前listen句柄是属于第几个worker进程

    unsigned            open:1;              //为1表示监听句柄有效，为0表示正常关闭

    /* 用已有的ngx_cycle_t来初始化ngx_cycle_t结构体时，不关闭监听端口，对于运行中升级有用 */
    unsigned            remain:1;

    /* 1:跳过设置当前ngx_listening_t结构体中的套接字    0：正常初始化 
    unsigned            ignore:1;

    unsigned            bound:1;       /* already bound 置1表示已绑定*/
    unsigned            inherited:1;   /* inherited from previous process 当前监听句柄从上一个进程来的 */
    unsigned            nonblocking_accept:1;     //暂时没有用到
    unsigned            listen:1;                 //当前结构体对应的socket已经被监听
    unsigned            nonblocking:1;            //是否阻塞，也没用。因为nginx是异步非阻塞的
    unsigned            shared:1;                 /* shared between threads or processes ，没人用- -，毕竟worker间没联系*/
    unsigned            addr_ntop:1;              //为1的话，表示连接建立后，所建立连接的IP地址的字符串表示形式需要自行设置
    unsigned            wildcard:1;               //表示当前监听句柄是否支持通配：主要是通配UDP等接受数据

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:1;
#endif
#if (NGX_HAVE_REUSEPORT)                          //在ngx_auto_config.h中我们支持此选项
    unsigned            reuseport:1;              //是否支持地址复用
    unsigned            add_reuseport:1;          //是否将一个端口不复用的socket转变成端口复用的socket
#endif
    unsigned            keepalive:2;              //是否为此监听socket设置keepalive选项

#if (NGX_HAVE_DEFERRED_ACCEPT)                    //在ngx_auto_config.h中我们支持此选项  
    unsigned            deferred_accept:1;        //当前socket延迟接受的状态
    unsigned            add_deferred:1;           //当前socket是否需要被设置为延迟接受
#ifdef SO_ACCEPTFILTER
    char               *accept_filter;            //代表accept filter参数
#endif
#endif
#if (NGX_HAVE_SETFIB)                             //当前并不支持此选项
    int                 setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)                       //在ngx_auto_config.h中我们支持此选项
    int                 fastopen;                 //-1: 表示不支持快速打开 0：暂时未开启快速打开； 1：：表示开启了块打开
#endif

};
{% endhighlight %}

下面我们对涉及到的一些知识点做一个简要的说明：

**1) TCP keepalive**

关于TCP keepalive有如下几个选项：

* ```TCP_KEEPIDLE```: 设置连接上如果没有数据发送的话，多久后发送keepalive探测分组，单位是秒

* ```TCP_KEEPINTVL```: 前后两侧探测之间的时间间隔，单位是秒

* ```TCP_KEEPCNT```: 关闭一个非活跃连接之前的最大重试次数

<br />

**2) SO_REUSEADDR与SO_REUSEPORT选项**

```SO_REUSEADDR```提供如下4个功能：

* ```SO_REUSEADDR```允许启动一个监听服务器并绑定众所周知端口，即使以前建立的将此端口用作他们的本地端口的连接仍存在。则通常是重启监听服务器时出现，若不设置此选项，则bind时将出错。

* ```SO_REUSEADDR```允许在同一端口上启动同一服务器的多个实例，只要每个实例绑定一个不同的本地IP地址即可。对于TCP，我们根本不能启动绑定相同IP地址和端口的多个服务器。

* ```SO_REUSEADDR```允许单个进程绑定同一端口到多个套接字上，只要每个绑定指定不同的本地IP地址即可。则一般不用于TCP服务器。

* ```SO_REUSEADDR```允许完全重复的绑定： 当一个IP地址和端口绑定到某个套接字上时，还允许此IP地址和端口绑定到另一个套接字上。一般来说，这个特性仅在支持多播的系统上才有，而且只对UDP套接字而言（TCP不支持多播）

```SO_REUSEPORT```选项有如下含义：

* 此选项允许完全重复绑定，但仅在想绑定相同IP地址和端口的套接口都指定了此套接口选项才行。

* 当被绑定的IP地址是一个多播地址时，则```SO_REUSEADDR```与```SO_REUSEPORT```等效。 


## 2. 相关枚举及宏定义
{% highlight string %}
typedef enum {
    NGX_ERROR_ALERT = 0,
    NGX_ERROR_ERR,
    NGX_ERROR_INFO,
    NGX_ERROR_IGNORE_ECONNRESET,
    NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
    NGX_TCP_NODELAY_UNSET = 0,
    NGX_TCP_NODELAY_SET,
    NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
    NGX_TCP_NOPUSH_UNSET = 0,
    NGX_TCP_NOPUSH_SET,
    NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01
#define NGX_HTTP_V2_BUFFERED   0x02
{% endhighlight %}


## 3. ngx_connection_s数据结构
{% highlight string %}
struct ngx_connection_s {
    void               *data;
    ngx_event_t        *read;
    ngx_event_t        *write;

    ngx_socket_t        fd;

    ngx_recv_pt         recv;
    ngx_send_pt         send;
    ngx_recv_chain_pt   recv_chain;
    ngx_send_chain_pt   send_chain;

    ngx_listening_t    *listening;

    off_t               sent;

    ngx_log_t          *log;

    ngx_pool_t         *pool;

    int                 type;

    struct sockaddr    *sockaddr;
    socklen_t           socklen;
    ngx_str_t           addr_text;

    ngx_str_t           proxy_protocol_addr;

#if (NGX_SSL)
    ngx_ssl_connection_t  *ssl;
#endif

    struct sockaddr    *local_sockaddr;
    socklen_t           local_socklen;

    ngx_buf_t          *buffer;

    ngx_queue_t         queue;

    ngx_atomic_uint_t   number;

    ngx_uint_t          requests;

    unsigned            buffered:8;

    unsigned            log_error:3;     /* ngx_connection_log_error_e */

    unsigned            unexpected_eof:1;
    unsigned            timedout:1;
    unsigned            error:1;
    unsigned            destroyed:1;

    unsigned            idle:1;
    unsigned            reusable:1;
    unsigned            close:1;
    unsigned            shared:1;

    unsigned            sendfile:1;
    unsigned            sndlowat:1;
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

    unsigned            need_last_buf:1;

#if (NGX_HAVE_IOCP)
    unsigned            accept_context_updated:1;
#endif

#if (NGX_HAVE_AIO_SENDFILE)
    unsigned            busy_count:2;
#endif

#if (NGX_THREADS)
    ngx_thread_task_t  *sendfile_task;
#endif
};

{% endhighlight %}

<br />
<br />

**[参考]:**

1. [TCP KeepAlive的几个附加选项](https://www.cnblogs.com/cobbliu/p/4655542.html)

2. [nginx源码初读](http://blog.csdn.net/wuchunlai_2012/article/details/50732741)

3. [nginx学习十 ngx_cycle_t 、ngx_connection_t 和ngx_listening_t](http://blog.csdn.net/xiaoliangsky/article/details/39831035)

4. [nginx源码分析—处理继承的sockets](http://blog.csdn.net/livelylittlefish/article/details/7277607)

5. [AcceptEx](https://baike.baidu.com/item/AcceptEx/1092364?fr=aladdin)

6. [TCP套接字端口复用SO_REUSEADDR](https://www.cnblogs.com/kex1n/p/7437290.html)

7. [accept filter](http://blog.chinaunix.net/uid-317451-id-92697.html)

<br />
<br />
<br />

