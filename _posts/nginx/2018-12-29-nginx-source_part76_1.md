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
上面只是用于测试epoll能不能编译过，这里不做介绍。

## 2. ngx_epoll_conf_t数据结构
{% highlight string %}
typedef struct {
    ngx_uint_t  events;
    ngx_uint_t  aio_requests;
} ngx_epoll_conf_t;
{% endhighlight %}
此数据结构用于保存epoll配置文件中相应的配置信息：

* events: 用于指示epoll_wait每次能处理的最大事件数。
<pre>
int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout)
</pre>

* aio_requests: 当aio搭配epoll事件驱动机制一起使用时，单个worker进程最大的未完成异步IO操作数。(在v1.1.4和1.0.7中出现，当前我们并不使用aio)


## 3. 相关静态函数声明
{% highlight string %}
//作为epoll module上下文初始化的回调函数
static ngx_int_t ngx_epoll_init(ngx_cycle_t *cycle, ngx_msec_t timer);


//当前我们支持NGX_HAVE_EVENTFD宏定义。主要是实现某种类似与pipe这样的通知机制。当前主要用在nginx线程池的实现方面，当任务处理完成，异步通知主线程
#if (NGX_HAVE_EVENTFD)
//初始化notify通知机制
static ngx_int_t ngx_epoll_notify_init(ngx_log_t *log);

//收到通知之后的回调函数，所关联的事件为notify_event
static void ngx_epoll_notify_handler(ngx_event_t *ev);
#endif

//事件驱动机制退出时的回调函数
static void ngx_epoll_done(ngx_cycle_t *cycle);

//往epoll事件驱动机制中添加一个事件
static ngx_int_t ngx_epoll_add_event(ngx_event_t *ev, ngx_int_t event,
    ngx_uint_t flags);

//从epoll事件驱动机制中移除一个事件
static ngx_int_t ngx_epoll_del_event(ngx_event_t *ev, ngx_int_t event,
    ngx_uint_t flags);

//将一个connection(连接）添加到事件驱动机制中，连接上的读、写事件即被加入到了事件驱动机制中
static ngx_int_t ngx_epoll_add_connection(ngx_connection_t *c);

//从事件驱动机制中删除一个连接的读、写事件
static ngx_int_t ngx_epoll_del_connection(ngx_connection_t *c,
    ngx_uint_t flags);


#if (NGX_HAVE_EVENTFD)
//通过notify_fd向外发出通知
static ngx_int_t ngx_epoll_notify(ngx_event_handler_pt handler);
#endif

//epoll事件驱动机制处理事件的主流程
static ngx_int_t ngx_epoll_process_events(ngx_cycle_t *cycle, ngx_msec_t timer,
    ngx_uint_t flags);


//当前我们不支持NGX_HAVE_FILE_AIO宏定义， 此函数作为获取异步IO通知的回调函数
#if (NGX_HAVE_FILE_AIO)
static void ngx_epoll_eventfd_handler(ngx_event_t *ev);
#endif

//作为epoll模块初始化时，会通过调用此函数创建相应的配置信息ngx_epoll_conf_t
static void *ngx_epoll_create_conf(ngx_cycle_t *cycle);

//作为epoll模块初始化时，会通过调用此函数来初始化相应的配置信息ngx_epoll_conf_t
static char *ngx_epoll_init_conf(ngx_cycle_t *cycle, void *conf);

{% endhighlight %}




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

10. [linux下syscall函数 间接系统调用](https://www.cnblogs.com/jiangzhaowei/p/4192290.html)

<br />
<br />
<br />

