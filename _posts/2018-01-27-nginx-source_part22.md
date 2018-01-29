---
layout: post
title: os/unix/ngx_socket.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本章我们主要讲述一下nginx中socket的一些相关设置函数。


<!-- more -->

## 1. os/unix/ngx_socket.h头文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SOCKET_H_INCLUDED_
#define _NGX_SOCKET_H_INCLUDED_


#include <ngx_config.h>


#define NGX_WRITE_SHUTDOWN SHUT_WR

typedef int  ngx_socket_t;

#define ngx_socket          socket
#define ngx_socket_n        "socket()"


#if (NGX_HAVE_FIONBIO)

int ngx_nonblocking(ngx_socket_t s);
int ngx_blocking(ngx_socket_t s);

#define ngx_nonblocking_n   "ioctl(FIONBIO)"
#define ngx_blocking_n      "ioctl(!FIONBIO)"

#else

#define ngx_nonblocking(s)  fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK)
#define ngx_nonblocking_n   "fcntl(O_NONBLOCK)"

#define ngx_blocking(s)     fcntl(s, F_SETFL, fcntl(s, F_GETFL) & ~O_NONBLOCK)
#define ngx_blocking_n      "fcntl(!O_NONBLOCK)"

#endif

int ngx_tcp_nopush(ngx_socket_t s);
int ngx_tcp_push(ngx_socket_t s);

#if (NGX_LINUX)

#define ngx_tcp_nopush_n   "setsockopt(TCP_CORK)"
#define ngx_tcp_push_n     "setsockopt(!TCP_CORK)"

#else

#define ngx_tcp_nopush_n   "setsockopt(TCP_NOPUSH)"
#define ngx_tcp_push_n     "setsockopt(!TCP_NOPUSH)"

#endif


#define ngx_shutdown_socket    shutdown
#define ngx_shutdown_socket_n  "shutdown()"

#define ngx_close_socket    close
#define ngx_close_socket_n  "close() socket"


#endif /* _NGX_SOCKET_H_INCLUDED_ */
{% endhighlight %}
下面我们对该头文件进行简单的介绍：


### 1.1 socket相关数据类型定义
{% highlight string %}
#define NGX_WRITE_SHUTDOWN SHUT_WR

typedef int  ngx_socket_t;

#define ngx_socket          socket
#define ngx_socket_n        "socket()"
{% endhighlight %}
这里定义了ngx_socket_t数据类型及ngx_socket函数。

### 1.2 socket阻塞性设置
{% highlight string %}
#if (NGX_HAVE_FIONBIO)

int ngx_nonblocking(ngx_socket_t s);
int ngx_blocking(ngx_socket_t s);

#define ngx_nonblocking_n   "ioctl(FIONBIO)"
#define ngx_blocking_n      "ioctl(!FIONBIO)"

#else

#define ngx_nonblocking(s)  fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK)
#define ngx_nonblocking_n   "fcntl(O_NONBLOCK)"

#define ngx_blocking(s)     fcntl(s, F_SETFL, fcntl(s, F_GETFL) & ~O_NONBLOCK)
#define ngx_blocking_n      "fcntl(!O_NONBLOCK)"

#endif
{% endhighlight %}
在ngx_auto_config.h头文件中，有如下定义：
<pre>
#ifndef NGX_HAVE_FIONBIO
#define NGX_HAVE_FIONBIO  1
#endif
</pre>
因此这里我们采用ioctl方式设置socket的阻塞特性。

### 1.3 socket push特性设置
{% highlight string %}
int ngx_tcp_nopush(ngx_socket_t s);
int ngx_tcp_push(ngx_socket_t s);

#if (NGX_LINUX)

#define ngx_tcp_nopush_n   "setsockopt(TCP_CORK)"
#define ngx_tcp_push_n     "setsockopt(!TCP_CORK)"

#else

#define ngx_tcp_nopush_n   "setsockopt(TCP_NOPUSH)"
#define ngx_tcp_push_n     "setsockopt(!TCP_NOPUSH)"

#endif
{% endhighlight %}
这里我们首先讲述一下```TCP_CORK```:

所谓的CORK就是塞子的意思，形象的理解就是用CORK将连接塞住，使数据先不发送出去，等到拔去塞子后再发送出去。设置该选项后，内核会尽量把小数据包拼成一个大的数据包(一个MTU)再发送出去，当然若一定时间后（一般为200ms，该值尚待确认），内核仍然没有组合成一个MTU时也必须发送现有的数据。

然而，TCP_CORK的实现可能并不像你想象的那么完美，CORK并不会将连接完全塞住。内核其实并不知道应用层到底什么时候会发送第二批数据用于和第一批数据拼接以达到MTU的大小，因此内核会给出一个时间限制，在该时间内没有拼接成一个大包（努力接近MTU）的话，内核就会无条件发送。也就是说若应用层程序发送小数据包的间隔不够短时，TCP_CORK就没有一点作用，反而会失去数据的实时性（每个小包数据都会延时一定时间再发送）。

<pre>
这里的TCP_NOPUSH与TCP_CORK含义是相同的。
</pre>

在ngx_auto_headers.h头文件中，有如下定义：
<pre>
#ifndef NGX_LINUX
#define NGX_LINUX  1
#endif
</pre>
因此这里我们采用```setsockopt(TCP_CORK)```的方式来进行设置。

### 1.4 关闭socket
{% highlight string %}
#define ngx_shutdown_socket    shutdown
#define ngx_shutdown_socket_n  "shutdown()"

#define ngx_close_socket    close
#define ngx_close_socket_n  "close() socket"
{% endhighlight %}
关于socket shutdown()与close()的区别，请参看[tcp半关闭状态](https://ivanzz1001.github.io/records/post/tcpip/2018/01/27/tcpip_part1)一文。



## 2. os/unix/ngx_socket.c源代码
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * ioctl(FIONBIO) sets a non-blocking mode with the single syscall
 * while fcntl(F_SETFL, O_NONBLOCK) needs to learn the current state
 * using fcntl(F_GETFL).
 *
 * ioctl() and fcntl() are syscalls at least in FreeBSD 2.x, Linux 2.2
 * and Solaris 7.
 *
 * ioctl() in Linux 2.4 and 2.6 uses BKL, however, fcntl(F_SETFL) uses it too.
 */


#if (NGX_HAVE_FIONBIO)

int
ngx_nonblocking(ngx_socket_t s)
{
    int  nb;

    nb = 1;

    return ioctl(s, FIONBIO, &nb);
}


int
ngx_blocking(ngx_socket_t s)
{
    int  nb;

    nb = 0;

    return ioctl(s, FIONBIO, &nb);
}

#endif


#if (NGX_FREEBSD)

int
ngx_tcp_nopush(ngx_socket_t s)
{
    int  tcp_nopush;

    tcp_nopush = 1;

    return setsockopt(s, IPPROTO_TCP, TCP_NOPUSH,
                      (const void *) &tcp_nopush, sizeof(int));
}


int
ngx_tcp_push(ngx_socket_t s)
{
    int  tcp_nopush;

    tcp_nopush = 0;

    return setsockopt(s, IPPROTO_TCP, TCP_NOPUSH,
                      (const void *) &tcp_nopush, sizeof(int));
}

#elif (NGX_LINUX)


int
ngx_tcp_nopush(ngx_socket_t s)
{
    int  cork;

    cork = 1;

    return setsockopt(s, IPPROTO_TCP, TCP_CORK,
                      (const void *) &cork, sizeof(int));
}


int
ngx_tcp_push(ngx_socket_t s)
{
    int  cork;

    cork = 0;

    return setsockopt(s, IPPROTO_TCP, TCP_CORK,
                      (const void *) &cork, sizeof(int));
}

#else

int
ngx_tcp_nopush(ngx_socket_t s)
{
    return 0;
}


int
ngx_tcp_push(ngx_socket_t s)
{
    return 0;
}

#endif
{% endhighlight %}
这里程序代码比较简单，不再赘述。

<br />
<br />
**[参看]:**

1. [linux网络编程之shutdown() 与 close()函数详解](https://www.cnblogs.com/dingxiaoqiang/p/7535015.html)

2. [linux下close()与shutdown()](http://www.360doc.com/content/17/0701/08/33093582_667895986.shtml)

<br />
<br />
<br />

