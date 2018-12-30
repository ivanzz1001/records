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

* posted: 用于指示该事件是否投递到了一个队列

* closed: 用于指示本事件所关联的socket句柄或文件句柄是否被关闭

* channel: 指示本事件用于nginx中master与worker之间的通信，以反应子进程是否已经退出

* resolver: 指示本事件用于nginx中的域名解析，判断在nginx的worker进程退出时，是否仍然还有连接处于resolver状态

* cancelable： 本标志用于定时器事件，用于指示当worker进程退出时，本事件应该被忽略。优雅的关闭worker子进程(Graceful worker shutdown)时，如果有```不可取消```(none-cancelable)的定时器事件正处于```scheduled```，那么关闭会被延迟

* accept_context_updated: 当前我们不支持```NGX_WIN32```宏定义，暂不使用

* kq_vnode: 当前我们不支持```NGX_HAVE_KQUEUE```宏定义，在kqueue中```EVFILT_VNODE过滤器```用于检测对文件系统上一个文件的某种改动

* kq_errno: 当前我们不支持```NGX_HAVE_KQUEUE```宏定义。kqueue报告的处于pending状态的错误码

* available： 我们当前并不支持```NGX_HAVE_KQUEUE```以及```NGX_HAVE_IOCP```宏定义。```available:1```通常用于控制accept()是否仍可以继续接受新的连接；
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

* notify: 主要是实现某种类似与```pipe```这样的通知机制。当前主要用在```nginx线程池```的实现方面，当任务处理完成，异步通知主线程。

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

* NGX_USE_LEVEL_EVENT： 表示事件驱动机制的event filter支持水平触发。select, poll, /dev/poll, kqueue, epoll均支持这一模式

* NGX_USE_ONESHOT_EVENT: 表示事件驱动机制的event filter支持oneshot，即表示某个事件触发一次之后，对应的event filter就会被自动的移除，可以确保事件只触发一次。kqueue, epoll支持本选项。

* NGX_USE_CLEAR_EVENT: 表示事件驱动机制的event filter支持边沿触发。epoll、kqueue支持本选项

* NGX_USE_KQUEUE_EVENT: 表示对应的事件驱动机制的event filter支持kqueue特性： eof flag、errno、available data等。一般kqueue支持本选项

* NGX_USE_LOWAT_EVENT: 表示对应的事件驱动机制的event filter支持```低水位标志```(low water mark)。一般kqueue支持本选项

* NGX_USE_GREEDY_EVENT: 表示对应的事件驱动机制的event filter需要重复执行IO操作，直到EAGAIN。一般epoll支持本选项。
<pre>
通常情况下，我们读取数据时，如果实际读取的数据字节数小于我们的请求数，那么此时我们可以直接将event.ready设置为0；
但是对于支持本选项的事件机制，即使遇到此情形，通常还是要继续进行读取，直到返回的EAGAIN为止，才将event.ready设置为0.
</pre>

* NGX_USE_EPOLL_EVENT： 本event filter用于epoll

* NGX_USE_RTSIG_EVENT: 当前已经过时，不再使用

* NGX_USE_AIO_EVENT: 当前已经过时，不再使用

* NGX_USE_IOCP_EVENT: 本event filter用于IOCP

* NGX_USE_FD_EVENT: 表示本event filter没有透明数据，并需要一个文件描述符表。本标志通常用于poll、dev/poll。下面我们以poll为例来说明一下为何要使用此选项：
{% highlight string %}
struct pollfd pfds[1];

ret = poll(fds,1,timeout);
{% endhighlight %}
上面代码，当有读写事件到来时，我们只能够通过```pfds```获取到文件的句柄信息，并没有```透明数据```。但是一般我们可能还需要获取到更为详细的信息(透明数据)，此时我们可能还需要绑定另外一个结构，通过fd就可以简单快捷的定位到。

* NGX_USE_EVENTPORT_EVENT: 本标志表示在notification之后，文件描述符上的所有event filter都会被移除。用于eventport

* NGX_USE_VNODE_EVENT： 本event filter表示支持vnode通知。用于kqueue

## 6. 事件驱动机制中的各种事件
{% highlight string %}
/*
 * The event filter is deleted just before the closing file.
 * Has no meaning for select and poll.
 * kqueue, epoll, eventport:         allows to avoid explicit delete,
 *                                   because filter automatically is deleted
 *                                   on file close,
 *
 * /dev/poll:                        we need to flush POLLREMOVE event
 *                                   before closing file.
 */
#define NGX_CLOSE_EVENT    1

/*
 * disable temporarily event filter, this may avoid locks
 * in kernel malloc()/free(): kqueue.
 */
#define NGX_DISABLE_EVENT  2

/*
 * event must be passed to kernel right now, do not wait until batch processing.
 */
#define NGX_FLUSH_EVENT    4


/* these flags have a meaning only for kqueue */
#define NGX_LOWAT_EVENT    0
#define NGX_VNODE_EVENT    0


#if (NGX_HAVE_EPOLL) && !(NGX_HAVE_EPOLLRDHUP)
#define EPOLLRDHUP         0
#endif


#if (NGX_HAVE_KQUEUE)

#define NGX_READ_EVENT     EVFILT_READ
#define NGX_WRITE_EVENT    EVFILT_WRITE

#undef  NGX_VNODE_EVENT
#define NGX_VNODE_EVENT    EVFILT_VNODE

/*
 * NGX_CLOSE_EVENT, NGX_LOWAT_EVENT, and NGX_FLUSH_EVENT are the module flags
 * and they must not go into a kernel so we need to choose the value
 * that must not interfere with any existent and future kqueue flags.
 * kqueue has such values - EV_FLAG1, EV_EOF, and EV_ERROR:
 * they are reserved and cleared on a kernel entrance.
 */
#undef  NGX_CLOSE_EVENT
#define NGX_CLOSE_EVENT    EV_EOF

#undef  NGX_LOWAT_EVENT
#define NGX_LOWAT_EVENT    EV_FLAG1

#undef  NGX_FLUSH_EVENT
#define NGX_FLUSH_EVENT    EV_ERROR

#define NGX_LEVEL_EVENT    0
#define NGX_ONESHOT_EVENT  EV_ONESHOT
#define NGX_CLEAR_EVENT    EV_CLEAR

#undef  NGX_DISABLE_EVENT
#define NGX_DISABLE_EVENT  EV_DISABLE


#elif (NGX_HAVE_DEVPOLL && !(NGX_TEST_BUILD_DEVPOLL)) \
      || (NGX_HAVE_EVENTPORT && !(NGX_TEST_BUILD_EVENTPORT))

#define NGX_READ_EVENT     POLLIN
#define NGX_WRITE_EVENT    POLLOUT

#define NGX_LEVEL_EVENT    0
#define NGX_ONESHOT_EVENT  1


#elif (NGX_HAVE_EPOLL) && !(NGX_TEST_BUILD_EPOLL)

#define NGX_READ_EVENT     (EPOLLIN|EPOLLRDHUP)
#define NGX_WRITE_EVENT    EPOLLOUT

#define NGX_LEVEL_EVENT    0
#define NGX_CLEAR_EVENT    EPOLLET
#define NGX_ONESHOT_EVENT  0x70000000
#if 0
#define NGX_ONESHOT_EVENT  EPOLLONESHOT
#endif


#elif (NGX_HAVE_POLL)

#define NGX_READ_EVENT     POLLIN
#define NGX_WRITE_EVENT    POLLOUT

#define NGX_LEVEL_EVENT    0
#define NGX_ONESHOT_EVENT  1


#else /* select */

#define NGX_READ_EVENT     0
#define NGX_WRITE_EVENT    1

#define NGX_LEVEL_EVENT    0
#define NGX_ONESHOT_EVENT  1

#endif /* NGX_HAVE_KQUEUE */


#if (NGX_HAVE_IOCP)
#define NGX_IOCP_ACCEPT      0
#define NGX_IOCP_IO          1
#define NGX_IOCP_CONNECT     2
#endif


#ifndef NGX_CLEAR_EVENT
#define NGX_CLEAR_EVENT    0    /* dummy declaration */
#endif
{% endhighlight %}
这里主要是定义了各```事件驱动机制```中的一些事件。通过宏屏蔽不同事件驱动机制的差异。

## 7. 相关函数的宏定义
{% highlight string %}

#define ngx_process_events   ngx_event_actions.process_events
#define ngx_done_events      ngx_event_actions.done

#define ngx_add_event        ngx_event_actions.add
#define ngx_del_event        ngx_event_actions.del
#define ngx_add_conn         ngx_event_actions.add_conn
#define ngx_del_conn         ngx_event_actions.del_conn

#define ngx_notify           ngx_event_actions.notify

#define ngx_add_timer        ngx_event_add_timer
#define ngx_del_timer        ngx_event_del_timer


extern ngx_os_io_t  ngx_io;

#define ngx_recv             ngx_io.recv
#define ngx_recv_chain       ngx_io.recv_chain
#define ngx_udp_recv         ngx_io.udp_recv
#define ngx_send             ngx_io.send
#define ngx_send_chain       ngx_io.send_chain
#define ngx_udp_send         ngx_io.udp_send
{% endhighlight %}
这里主要是对相关函数指针进行重新定义，方便使用。

## 8. event模块宏
{% highlight string %}
#define NGX_EVENT_MODULE      0x544E5645  /* "EVNT" */
#define NGX_EVENT_CONF        0x02000000
{% endhighlight %}
这里主要是nginx event模块相应的标识

## 9. ngx_event_conf_t数据结构
{% highlight string %}
typedef struct {
    ngx_uint_t    connections;
    ngx_uint_t    use;

    ngx_flag_t    multi_accept;
    ngx_flag_t    accept_mutex;

    ngx_msec_t    accept_mutex_delay;

    u_char       *name;

#if (NGX_DEBUG)
    ngx_array_t   debug_connection;
#endif
} ngx_event_conf_t;
{% endhighlight %}
本数据结构用于保存event模块相关配置。下面简要介绍一下各字段的含义：

* connections: 指定一个worker进程可以打开的最大连接数。注意这不仅包括客户端的连接数，还包括nginx到后端代理服务器之间的连接等

* use: 保存当前事件模块采用的是哪一种事件处理方法([connection_processing](https://nginx.org/en/docs/events.html))。通常配置文件中不会进行指定，nginx会自己选择当前所支持的最高效的方法，然后保存到此变量中。

* multi_accept: 假如```multi_accept```被禁用的话，worker进程一次只会accept一个新的连接(new connection)。否则，worker进程一次会accept所有新的连接。

* accept_mutex: 假如```accept_mutex```被启用的话，worker进程之间将会轮流地```accept```新的连接(connection)。否则，在新连接(connections)到来时，所有的worker进程都会被唤醒，但可能只有某些进程能够获取到新连接(connections)，其他的worker进程会继续进入休眠状态。这就是所谓的```惊群现象```。
<pre>
注：
 1. 高版本的Linux中，accept不存在惊群问题，不过epoll_wait等操作还有

 2. 在nginx 1.11.3版本之前，本选项是默认启用的
</pre>

* accept_mutex_delay: 当```accept_mutex```被启用的话，如果当前有另一个worker进程正在accept新连接(connections)，则当前worker进程最长会等待多长时间尝试重新开始accept新连接。其实就是获取互斥锁的最大延迟时间

* name: 用于存放当前event_module的名称(select、poll、epoll、kqueue等）

* debug_connection:  为所指定的客户端连接启用调试日志(debugging log)。而对于其他的连接则会使用```error_log```指令所设置的日志级别。调试的连接可以通过**IPv4**或者**IPv6**来指定，也可以通过主机名来指定。此外，也支持unix域socket。参看如下示例：
<pre>
events {
    debug_connection 127.0.0.1;
    debug_connection localhost;
    debug_connection 192.0.2.0/24;
    debug_connection ::1;
    debug_connection 2001:0db8::/32;
    debug_connection unix:;
    ...
}
</pre>


## 10. ngx_event_module_t数据结构
{% highlight string %}
typedef struct {
    ngx_str_t              *name;

    void                 *(*create_conf)(ngx_cycle_t *cycle);
    char                 *(*init_conf)(ngx_cycle_t *cycle, void *conf);

    ngx_event_actions_t     actions;
} ngx_event_module_t;
{% endhighlight %}
本数据结构作为```事件模块```的上下文环境。下面我们简要介绍一下各字段的含义：

* name: 用于保存所使用的```事件驱动机制```的名称(select、poll、epoll、kqueue等)

* create_conf: 用于创建event模块相关配置(ngx_event_conf_t)的回调函数

* init_conf: 用于初始化event模块相关配置(ngx_event_conf_t)的回调函数

* action: event模块的actions操作函数

## 11. 相关全局变量声明
{% highlight string %}
extern ngx_atomic_t          *ngx_connection_counter;

extern ngx_atomic_t          *ngx_accept_mutex_ptr;
extern ngx_shmtx_t            ngx_accept_mutex;
extern ngx_uint_t             ngx_use_accept_mutex;
extern ngx_uint_t             ngx_accept_events;
extern ngx_uint_t             ngx_accept_mutex_held;
extern ngx_msec_t             ngx_accept_mutex_delay;
extern ngx_int_t              ngx_accept_disabled;


#if (NGX_STAT_STUB)

extern ngx_atomic_t  *ngx_stat_accepted;
extern ngx_atomic_t  *ngx_stat_handled;
extern ngx_atomic_t  *ngx_stat_requests;
extern ngx_atomic_t  *ngx_stat_active;
extern ngx_atomic_t  *ngx_stat_reading;
extern ngx_atomic_t  *ngx_stat_writing;
extern ngx_atomic_t  *ngx_stat_waiting;

#endif


#define NGX_UPDATE_TIME         1
#define NGX_POST_EVENTS         2


extern sig_atomic_t           ngx_event_timer_alarm;
extern ngx_uint_t             ngx_event_flags;
extern ngx_module_t           ngx_events_module;
extern ngx_module_t           ngx_event_core_module;


#define ngx_event_get_conf(conf_ctx, module)                                  \
             (*(ngx_get_conf(conf_ctx, ngx_events_module))) [module.ctx_index];

{% endhighlight %}

下面简要介绍一下各变量的含义：

* ngx_connection_counter: 用于统计当前的连接数

* ngx_accept_mutex_ptr: 存放互斥量内存地址的指针

* ngx_accept_mutex: accept互斥锁

* ngx_use_accept_mutex: 表示是否需要通过对accept加锁来解决惊群问题。当nginx worker进程数```大于1```时，且配置文件中开启了```accept_mutex```时，这个标志置为1

* ngx_accept_events： 表示在获取accept互斥锁的时候，是否还需要调用ngx_enable_accept_events()来使能accept events。通常采用```eventport```这一事件驱动机制时才需要。

* ngx_accept_mutex_held： 用于指示当前是否拿到了锁

* ngx_accept_mutex_delay： 当```accept_mutex```被启用的话，如果当前有另一个worker进程正在accept新连接(connections)，则当前worker进程最长会等待多长时间尝试重新开始accept新连接。其实就是获取互斥锁的最大延迟时间

* ngx_accept_disabled： 用于控制当前worker进程是否参与获取```ngx_accept_mutex```。一般在当前worker进程连接数过多时，就不会参与竞争，这样可以起到负载均衡的目的。

<br />

我们当前并不支持```NGX_STAT_STUB```，该宏定义需要在启用```HTTP_STUB_STATUS```编译选项时才会定义。请参看[Module ngx_http_stub_status_module](https://nginx.org/en/docs/http/ngx_http_stub_status_module.html)但是这里我们还是简单介绍一下各字段的含义。

* ngx_stat_accepted: 用于统计当前nginx一共accept多少连接

* ngx_stat_handled: 用于统计当前所处理的总的连接数。通常情况下，本字段的值与```ngx_stat_accepted```字段的值相同，除非达到了一些资源的限制（例如，worker_connections的限制）

* ngx_stat_requests: 用于统计客户端总的请求数

* ngx_stat_active: 用于统计当前处于active状态的客户端连接数，这也包括```Waiting```状态的连接

* ngx_stat_reading: 当前Nginx正在读取```请求头```(request header)的连接数

* ngx_stat_writing: 当前nginx正在向客户端```回写应答```(write the response back)的连接数

* ngx_stat_waiting: 当前正处于空闲状态的客户端连接数。


<br />

* NGX_UPDATE_TIME: 本宏定义用于指示当前是否需要更新nginx缓存时间

* NGX_POST_EVENTS: 本用定义用于指示是否要将```事件```post，使得其可以在当前事件循环(event loop)遍历的后续某个时间点被处理

<br />

* ngx_event_timer_alarm: 表示收到了```SIGALRM```信号产生的alarm事件，收到信号后一般表示需要更新nginx缓存时间

* ngx_event_flags: 用于保存当前```事件驱动机制```所支持的事件掩码

* ngx_events_module： events模块。作为event模块的入口，解析events{}中的配置项，同时管理这些事件模块存储配置项的结构体。select、poll、epoll都是属于events模块，每一个事件模块产生的配置结构体指针都会被放在ngx_events_module模块创建的指针数组中。

* ngx_event_core_module： events核心模块。负责维护事件模块相关核心配置。
<pre>
注： ngx_events_module与ngx_event_core_module与的关系是一种包含与被包含的关系。参看events模块的配置文件：

events {

accept_mutex on;

multi_accept on;

#use epoll; 

worker_connections 1024; 
}
</pre>

* ngx_event_get_conf： 用于获取event模块相关配置项的值

## 12. 相关函数声明
{% highlight string %}
//用于处理TCP连接事件
void ngx_event_accept(ngx_event_t *ev);


//负责处理udp连接的处理
#if !(NGX_WIN32)
void ngx_event_recvmsg(ngx_event_t *ev);
#endif

//尝试获取accept互斥锁
ngx_int_t ngx_trylock_accept_mutex(ngx_cycle_t *cycle);

//对accept相应的日志进行格式化
u_char *ngx_accept_log_error(ngx_log_t *log, u_char *buf, size_t len);

//事件处理主流程
void ngx_process_events_and_timers(ngx_cycle_t *cycle);

//处理读事件
ngx_int_t ngx_handle_read_event(ngx_event_t *rev, ngx_uint_t flags);

//处理写事件
ngx_int_t ngx_handle_write_event(ngx_event_t *wev, size_t lowat);


//Windows平台专用，这里不会使用到
#if (NGX_WIN32)
void ngx_event_acceptex(ngx_event_t *ev);
ngx_int_t ngx_event_post_acceptex(ngx_listening_t *ls, ngx_uint_t n);
u_char *ngx_acceptex_log_error(ngx_log_t *log, u_char *buf, size_t len);
#endif

//用于设置相应的socket的SO_SNDLOWAT选项
ngx_int_t ngx_send_lowat(ngx_connection_t *c, size_t lowat);


//获取connection的fd字段的值
/* used in ngx_log_debugX() */
#define ngx_event_ident(p)  ((ngx_connection_t *) (p))->fd


#include <ngx_event_timer.h>
#include <ngx_event_posted.h>

#if (NGX_WIN32)
#include <ngx_iocp_module.h>
#endif


#endif /* _NGX_EVENT_H_INCLUDED_ */
{% endhighlight %}


## 13. nginx中事件介绍

### 13.1 IO事件
每一条通过调用函数ngx_get_connection()获取到的连接(connection)都绑定了两个事件：c->read以及c->write，分别用于接收对应socket读、写就绪的通知(notification)。所有这些事件都以```边沿触发模式```(Edge-Triggered mode)工作，这意味着这些事件只会在socket状态发生改变时才会触发通知(notification)。举例来说，假如你在一个socket上只读取部分数据(partial read)，那么nginx并不会重复投递(deliver)读通知(read notification)，直到后续有更多的数据到达socket。注意，即使底层的IO通知机制实际上用的是```水平触发```(Level Triggered，如select、poll等)，nginx会将这些通知转换成```边沿触发```(Edge-Triggered)。

为了使得nginx事件通知在不同平台的所有通知系统(notification systems)上都保持一致，在处理完一个IO socket通知或者调用了socket的任何IO函数之后，我们都需要再函数ngx_handle_read_event(rev, flags)或ngx_handle_write_event(wev,lowat)。通常情况下，这些函数都会在每一个读(read)、写(write)事件的handler末尾处被调用一次。

### 13.2 定时器事件

一个event可以被设置为在其超时的时候发送一个notification。由该event所使用的定时器从上一次某一个时间点开始以```毫秒```为单位开始进行计数。当前的毫秒值可以通过```ngx_current_msec```变量来获取到。

函数ngx_add_timer(ev,timer)可以为一个事件设置超时时间，而ngx_del_timer(ev)可以删除一个以前所设置的超时时间。全局的超时红黑树ngx_event_timer_rbtree存储当前所有的超时集。红黑树中节点的key为```ngx_msec_t```类型，其存储的就是定时器的到期时间。红黑树的结构使得能够快速的进行插入与删除操作，也能够快捷的访问到当前最近的超时时间，nginx会使用该最近超时时间来等待IO的发生。

### 13.3 Posted事件
一个event可以被```posted```，这就意味着在当前事件循环的后续某个时间点，该event的handler会被调用。posting events在简化代码以及避免栈溢出方面都是很好实践(good practise)。被投递(posted)的事件会被存放在```post queue```中。宏定义ngx_post_event(ev,q)会将事件```ev```投递到post队列```q```中；而ngx_delete_posted_event(ev)宏定义会将事件```ev```从其当前所投递的队列中移除。通常情况下，events会被投递到**ngx_posted_events**队列中，这些事件会在事件循环的后期被处理： 在所有的IO及定时器事件被处理完成之后。函数**ngx_event_process_posted()**会被调用以处理一个事件队列，其会不断调用event handlers直到整个队列为空。这就意味着一个**posted event handler**可以在一个事件循环中投递多个事件，然后再被处理。请参看如下示例：
{% highlight string %}
void
ngx_my_connection_read(ngx_connection_t *c)
{
    ngx_event_t  *rev;

    rev = c->read;

    ngx_add_timer(rev, 1000);

    rev->handler = ngx_my_read_handler;

    ngx_my_read(rev);
}


void
ngx_my_read_handler(ngx_event_t *rev)
{
    ssize_t            n;
    ngx_connection_t  *c;
    u_char             buf[256];

    if (rev->timedout) { /* timeout expired */ }

    c = rev->data;

    while (rev->ready) {
        n = c->recv(c, buf, sizeof(buf));

        if (n == NGX_AGAIN) {
            break;
        }

        if (n == NGX_ERROR) { /* error */ }

        /* process buf */
    }

    if (ngx_handle_read_event(rev, 0) != NGX_OK) { /* error */ }
}
{% endhighlight %}

示例中，当connection有可读事件到达时，就会调用ngx_my_read_handler()回调函数来读取数据。

### 13.4 Event loop
除了nginx的master进程之外，所有的worker进程都会进行IO操作，因此都有一个```事件循环(event loop)```。（nginx的master进程把其大部分时间都耗费在sigsuspend()函数调用上，以等待信号signals的到达。）nginx事件循环是在函数ngx_process_events_and_timers()中实现的，其会重复的被调用，直到整个进程退出。

事件循环(event loop)有如下的一些stages(阶段）：

* 通过调用ngx_event_find_timer()来查找当前最近的超时时间。该函数会查找红黑树中的最左边的节点，并返回该节点超时的毫秒数

* 根据nginx的配置的特定的事件通知机制,通过调用一个handler来处理IO事件。该handler会等待至少一个IO事件发生，或者等待超时。当一个读(read)、写(write)事件发生时，```ready```标志会被设置，然后事件的handler会被调用。对于Linux来说，通常情况下会使用函数**ngx_epoll_process_events()**，该函数会调用```epoll_wait()```来等待IO事件

* 通过调用ngx_event_expire_timers()来处理超时定时器事件。会从timer红黑树的最左侧节点开始遍历直到一个未超时的事件被找到。对于每一个已经超时的节点，event的```timeout```标志会被设置，event的timer_set标志会被重置，然后事件的handler会被调用

* 通过调用**ngx_event_process_posted()**来处理被投递(posted)的事件。该函数会重复的执行： 移除队列中的第一个元素，然后调用该元素的handler()回调，直到整个队列为空。

所有的nginx进程（包括master进程与worker进程）都会处理信号(signals)。信号处理器只会是指一些```全局变量```的值，这些变量的值会在调用完ngx_process_events_and_timers()函数之后被检查，以做进一步处理。




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

8. [Nginx的accept_mutex配置](https://blog.csdn.net/adams_wu/article/details/51669203)

9. [Nginx配置晋级之路（四）---events模块](https://blog.csdn.net/qq_26711103/article/details/81117770)

<br />
<br />
<br />

