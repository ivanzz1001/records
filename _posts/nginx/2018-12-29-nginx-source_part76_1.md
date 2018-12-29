---
layout: post
title: event/modules/ngx_epoll_module.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们介绍一下nginx epoll模块的实现。


<!-- more -->


## 1. NGX_TEST_BUILD_EPOLL测试
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#if (NGX_TEST_BUILD_EPOLL)

/* epoll declarations */

#define EPOLLIN        0x001
#define EPOLLPRI       0x002
#define EPOLLOUT       0x004
#define EPOLLRDNORM    0x040
#define EPOLLRDBAND    0x080
#define EPOLLWRNORM    0x100
#define EPOLLWRBAND    0x200
#define EPOLLMSG       0x400
#define EPOLLERR       0x008
#define EPOLLHUP       0x010

#define EPOLLRDHUP     0x2000

#define EPOLLET        0x80000000
#define EPOLLONESHOT   0x40000000

#define EPOLL_CTL_ADD  1
#define EPOLL_CTL_DEL  2
#define EPOLL_CTL_MOD  3

typedef union epoll_data {
    void         *ptr;
    int           fd;
    uint32_t      u32;
    uint64_t      u64;
} epoll_data_t;

struct epoll_event {
    uint32_t      events;
    epoll_data_t  data;
};


int epoll_create(int size);

int epoll_create(int size)
{
    return -1;
}


int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
    return -1;
}


int epoll_wait(int epfd, struct epoll_event *events, int nevents, int timeout);

int epoll_wait(int epfd, struct epoll_event *events, int nevents, int timeout)
{
    return -1;
}

#if (NGX_HAVE_EVENTFD)
#define SYS_eventfd       323
#endif

#if (NGX_HAVE_FILE_AIO)

#define SYS_io_setup      245
#define SYS_io_destroy    246
#define SYS_io_getevents  247

typedef u_int  aio_context_t;

struct io_event {
    uint64_t  data;  /* the data field from the iocb */
    uint64_t  obj;   /* what iocb this event came from */
    int64_t   res;   /* result code for this event */
    int64_t   res2;  /* secondary result */
};


#endif
#endif /* NGX_TEST_BUILD_EPOLL */
{% endhighlight %}
上面只是用于测试使用，这里不做介绍。



<br />
<br />

**[参看]**

1. [nginx events](https://nginx.org/en/docs/dev/development_guide.html#events)

2. [nginx event 模块解析](https://blog.csdn.net/jackywgw/article/details/48676643)

3. [Nginx学习笔记(十八)：事件处理框架](https://blog.csdn.net/fzy0201/article/details/23171207)

4. [事件和连接](https://blog.csdn.net/nestler/article/details/37570401)

5. [eventfd 的分析与具体例子](https://blog.csdn.net/tanswer_/article/details/79008322)

6. [EVENTFD](http://man7.org/linux/man-pages/man2/eventfd.2.html)

7. [事件模块（二）ngx_epoll_module详解](https://blog.csdn.net/ws891033655/article/details/25643465)

8. [Linux下的I/O复用与epoll详解](https://www.cnblogs.com/lojunren/p/3856290.html)

9. [Nginx配置详解](http://www.cnblogs.com/knowledgesea/p/5175711.html)

<br />
<br />
<br />

