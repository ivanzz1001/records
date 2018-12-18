---
layout: post
title: event/ngx_event.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本章我们讲述一下nginx中event的实现。Nginx中的event对象```ngx_event_t```提供了一种机制，能够通知程序发生了某个事件。这里的event主要包括两大种类：

* IO事件

* 定时器事件

<!-- more -->


## 1. ngx_event_ovlp_t数据结构
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_H_INCLUDED_
#define _NGX_EVENT_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_INVALID_INDEX  0xd0d0d0d0


#if (NGX_HAVE_IOCP)

typedef struct {
    WSAOVERLAPPED    ovlp;
    ngx_event_t     *event;
    int              error;
} ngx_event_ovlp_t;

#endif
{% endhighlight %}
当前我们不支持```NGX_HAVE_IOCP```宏定义，因此这里不做介绍。

## 2. ngx_event_s数据结构
{% highlight string %}
struct ngx_event_s {
    void            *data;

    unsigned         write:1;

    unsigned         accept:1;

    /* used to detect the stale events in kqueue and epoll */
    unsigned         instance:1;

    /*
     * the event was passed or would be passed to a kernel;
     * in aio mode - operation was posted.
     */
    unsigned         active:1;

    unsigned         disabled:1;

    /* the ready event; in aio mode 0 means that no operation can be posted */
    unsigned         ready:1;

    unsigned         oneshot:1;

    /* aio operation is complete */
    unsigned         complete:1;

    unsigned         eof:1;
    unsigned         error:1;

    unsigned         timedout:1;
    unsigned         timer_set:1;

    unsigned         delayed:1;

    unsigned         deferred_accept:1;

    /* the pending eof reported by kqueue, epoll or in aio chain operation */
    unsigned         pending_eof:1;

    unsigned         posted:1;

    unsigned         closed:1;

    /* to test on worker exit */
    unsigned         channel:1;
    unsigned         resolver:1;

    unsigned         cancelable:1;

#if (NGX_WIN32)
    /* setsockopt(SO_UPDATE_ACCEPT_CONTEXT) was successful */
    unsigned         accept_context_updated:1;
#endif

#if (NGX_HAVE_KQUEUE)
    unsigned         kq_vnode:1;

    /* the pending errno reported by kqueue */
    int              kq_errno;
#endif

    /*
     * kqueue only:
     *   accept:     number of sockets that wait to be accepted
     *   read:       bytes to read when event is ready
     *               or lowat when event is set with NGX_LOWAT_EVENT flag
     *   write:      available space in buffer when event is ready
     *               or lowat when event is set with NGX_LOWAT_EVENT flag
     *
     * iocp: TODO
     *
     * otherwise:
     *   accept:     1 if accept many, 0 otherwise
     */

#if (NGX_HAVE_KQUEUE) || (NGX_HAVE_IOCP)
    int              available;
#else
    unsigned         available:1;
#endif

    ngx_event_handler_pt  handler;


#if (NGX_HAVE_IOCP)
    ngx_event_ovlp_t ovlp;
#endif

    ngx_uint_t       index;

    ngx_log_t       *log;

    ngx_rbtree_node_t   timer;

    /* the posted queue */
    ngx_queue_t      queue;

#if 0

    /* the threads support */

    /*
     * the event thread context, we store it here
     * if $(CC) does not understand __thread declaration
     * and pthread_getspecific() is too costly
     */

    void            *thr_ctx;

#if (NGX_EVENT_T_PADDING)

    /* event should not cross cache line in SMP */

    uint32_t         padding[NGX_EVENT_T_PADDING];
#endif
#endif
};
{% endhighlight %}
Nginx中的event对象```ngx_event_t```提供了一种机制，能够通知程序发生了某个事件。下面我们详细介绍一下各字段的含义：

* data: 指向用于event handlers的任何事件上下文，通常是指向与该event关联的connection对象。

* write: 用于指示```写事件```发生的标志。缺省状态下表示的是一个```读事件```

* accept: 用于标识该事件是一个accept事件，还是一个posted事件。这两种不同的事件会放到不同的队列来进行处理

* instance: 用于侦测kqueue和epoll中的```陈旧事件```(stale event)

* active: 该标志(flag)用于指示本event被登记(registered)为接收IO通知，通常是来自于epoll、kqueue、poll这样的通知机制

* disabled: 该标志用于指明是否禁止本事件（读事件/写事件)

* ready: 该标记用于指明本event收到了一个IO通知

* oneshot: 一般用于指明当前event是否是属于```一次性事件```

* complete： 当进行异步处理本事件时，标志是否处理完成

* eof: 用于标记在进行```读数据```(read data)的时候，读到了EOF

* error: 用于标记在进行读(read event)或者写(write event)时，是否发生了错误

* timedout： 用于标记事件定时器(event timer)是否已经超时

* timer_set: 用于标记事件定时器(event timer)被设置，并且没有超时

* delayed: 用于标记由于```速率限制```(rate limiting)的原因导致IO被延迟

* deferred_accept: 是否进行延迟accept，主要是为了提高程序的性能方面考虑

* pending_eof: 本标志用于指示所对应socket上有未处理的EOF，即使在EOF之前可能仍存在一些可用数据。这通常是由epoll的```EPOLLRDHUP```时间产生的或者kqueue的```EV_EOF```标志产生的。

* posted: 用于指示该事件是否要投递到一个队列

* closed: 用于指示本事件所关联的socket句柄或文件句柄是否被关闭

* channel: 指示本事件用于nginx中master与worker之间的通信，以反应子进程是否已经退出

* resolver: 指示本事件用于nginx中的域名解析，判断在nginx的worker进程退出时，是否仍然还有连接处于resolver状态

* cancelable： 本标志用于定时器事件，用于指示当worker进程退出时，本事件应该被忽略。优雅的关闭worker子进程(Graceful worker shutdown)时，如果有```不可取消```(none-cancelable)的定时器事件正处于```scheduled```，那么关闭会被延迟

* accept_context_updated: 当前我们不支持```NGX_WIN32```宏定义，暂不使用

* kq_vnode: 当前我们不支持```NGX_HAVE_KQUEUE```宏定义，在kqueue中```EVFILT_VNODE过滤器```用于检测对文件系统上一个文件的某种改动

* kq_errno: 当前我们不支持```NGX_HAVE_KQUEUE```宏定义。kqueue报告的处于pending状态的错误码

* available： 我们当前并不支持```NGX_HAVE_KQUEUE```以及```NGX_HAVE_IOCP```宏定义。```available:1```用于指示当前event所关联的socket是否可用；
<pre>
#if (NGX_HAVE_KQUEUE) || (NGX_HAVE_IOCP)
    int              available;
#else
    unsigned         available:1;			
#endif
</pre>

* handler: 当事件发生时所执行的回调函数。该回调函数的原型在core/ngx_core.h头文件中定义
<pre>
typedef void (*ngx_event_handler_pt)(ngx_event_t *ev);
</pre>

* ovlp: 当前我们并不支持```NGX_HAVE_IOCP```宏定义

* index: 其通常用于表示本event属于某一个event数组中的哪个索引处

* log: 本event所关联的日志对象

* timer: 红黑树节点，用于插入当前事件(event)到timer树中

* queue: Queue节点，用于将本事件post到一个队列中（不同类型的事件，可能会投递到不同的队列）


## 3. ngx_event_aio_s数据结构
{% highlight string %}
#if (NGX_HAVE_FILE_AIO)

struct ngx_event_aio_s {
    void                      *data;
    ngx_event_handler_pt       handler;
    ngx_file_t                *file;

#if (NGX_HAVE_AIO_SENDFILE)
    ssize_t                  (*preload_handler)(ngx_buf_t *file);
#endif

    ngx_fd_t                   fd;

#if (NGX_HAVE_EVENTFD)
    int64_t                    res;
#endif

#if !(NGX_HAVE_EVENTFD) || (NGX_TEST_BUILD_EPOLL)
    ngx_err_t                  err;
    size_t                     nbytes;
#endif

    ngx_aiocb_t                aiocb;
    ngx_event_t                event;
};

#endif
{% endhighlight %}

当前我们并不支持```NGX_HAVE_FILE_AIO```宏定义，这里不做介绍。

## 4. ngx_event_actions_t数据结构
{% highlight string %}
typedef struct {
    ngx_int_t  (*add)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
    ngx_int_t  (*del)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);

    ngx_int_t  (*enable)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);
    ngx_int_t  (*disable)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);

    ngx_int_t  (*add_conn)(ngx_connection_t *c);
    ngx_int_t  (*del_conn)(ngx_connection_t *c, ngx_uint_t flags);

    ngx_int_t  (*notify)(ngx_event_handler_pt handler);

    ngx_int_t  (*process_events)(ngx_cycle_t *cycle, ngx_msec_t timer,
                                 ngx_uint_t flags);

    ngx_int_t  (*init)(ngx_cycle_t *cycle, ngx_msec_t timer);
    void       (*done)(ngx_cycle_t *cycle);
} ngx_event_actions_t;

extern ngx_event_actions_t   ngx_event_actions;
{% endhighlight %}

本数据结构定义了一些函数指针，用于处理与```事件```相关的一些操作。每一个事件模块(event module)都需要实现如下10个方法。下面简单介绍一下这些函数指针：

* add: 将一个事件添加到 ```事件驱动机制```(如select、poll、epoll、kqueue)中

* del: 将一个事件从事件驱动机制中删除

* enable: 启用一个事件。通常情况下，我们并不会使用到，因为将事件添加到```事件驱动机制```中以后，一般并不需要另行启用

* disable: 禁用一个事件。通常情况下，我们并不会使用到

* add_conn: 将一个```connection```(连接）添加到事件驱动机制中，连接上的读、写事件即被加入到了事件驱动机制中

* del_conn: 从事件驱动机制中删除一个连接的读、写事件

* notify: 主要是实现某种类似与```pipe```这样的通知机制。当前主要用在```nginx线程池````的实现方面，当任务处理完成，异步通知主线程。

* process_events: 处理事件的方法。由于select、poll、epoll等不同的事件驱动机制，对事件的处理也有些不同。

* init: 一般用于初始化事件驱动机制

* done: 事件驱动机制退出时的回调函数

接着声明了一个全局的ngx_event_actions_t类型的对象： ngx_event_actions。


## 5. 事件驱动机制特性掩码
{% highlight string %}
/*
 * The event filter requires to read/write the whole data:
 * select, poll, /dev/poll, kqueue, epoll.
 */
#define NGX_USE_LEVEL_EVENT      0x00000001

/*
 * The event filter is deleted after a notification without an additional
 * syscall: kqueue, epoll.
 */
#define NGX_USE_ONESHOT_EVENT    0x00000002

/*
 * The event filter notifies only the changes and an initial level:
 * kqueue, epoll.
 */
#define NGX_USE_CLEAR_EVENT      0x00000004

/*
 * The event filter has kqueue features: the eof flag, errno,
 * available data, etc.
 */
#define NGX_USE_KQUEUE_EVENT     0x00000008

/*
 * The event filter supports low water mark: kqueue's NOTE_LOWAT.
 * kqueue in FreeBSD 4.1-4.2 has no NOTE_LOWAT so we need a separate flag.
 */
#define NGX_USE_LOWAT_EVENT      0x00000010

/*
 * The event filter requires to do i/o operation until EAGAIN: epoll.
 */
#define NGX_USE_GREEDY_EVENT     0x00000020

/*
 * The event filter is epoll.
 */
#define NGX_USE_EPOLL_EVENT      0x00000040

/*
 * Obsolete.
 */
#define NGX_USE_RTSIG_EVENT      0x00000080

/*
 * Obsolete.
 */
#define NGX_USE_AIO_EVENT        0x00000100

/*
 * Need to add socket or handle only once: i/o completion port.
 */
#define NGX_USE_IOCP_EVENT       0x00000200

/*
 * The event filter has no opaque data and requires file descriptors table:
 * poll, /dev/poll.
 */
#define NGX_USE_FD_EVENT         0x00000400

/*
 * The event module handles periodic or absolute timer event by itself:
 * kqueue in FreeBSD 4.4, NetBSD 2.0, and MacOSX 10.4, Solaris 10's event ports.
 */
#define NGX_USE_TIMER_EVENT      0x00000800

/*
 * All event filters on file descriptor are deleted after a notification:
 * Solaris 10's event ports.
 */
#define NGX_USE_EVENTPORT_EVENT  0x00001000

/*
 * The event filter support vnode notifications: kqueue.
 */
#define NGX_USE_VNODE_EVENT      0x00002000
{% endhighlight %}

上面我们介绍到有select、poll、epoll、kqueue、eventport等不同的事件驱动机制。每一种事件驱动机制所能支持的一些特性可能也有些不同。下面我们分别介绍一些这些标志：

* NGX_USE_LEVEL_EVENT： 表示







<br />
<br />

**[参看]**

1. [nginx events](https://nginx.org/en/docs/dev/development_guide.html#events)

2. [nginx event 模块解析](https://blog.csdn.net/jackywgw/article/details/48676643)

3. [Linux TCP_DEFER_ACCEPT的作用](https://blog.csdn.net/for_tech/article/details/54175571)

4. [freebsd高级I/O,kevent的资料很详细](https://www.cnblogs.com/fengyv/archive/2012/07/30/2614783.html)

5. [Nginx学习笔记(十八)：事件处理框架](https://blog.csdn.net/fzy0201/article/details/23171207)

6. [事件和连接](https://blog.csdn.net/nestler/article/details/37570401)

7. [Nginx学习笔记(十八)：事件处理框架](https://blog.csdn.net/fzy0201/article/details/23171207)

<br />
<br />
<br />

