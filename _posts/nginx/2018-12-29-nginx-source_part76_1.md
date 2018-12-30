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


## 4. 相关变量声明
{% highlight string %}
static int                  ep = -1;
static struct epoll_event  *event_list;
static ngx_uint_t           nevents;

#if (NGX_HAVE_EVENTFD)
static int                  notify_fd = -1;
static ngx_event_t          notify_event;
static ngx_connection_t     notify_conn;
#endif

#if (NGX_HAVE_FILE_AIO)

int                         ngx_eventfd = -1;
aio_context_t               ngx_aio_ctx = 0;

static ngx_event_t          ngx_eventfd_event;
static ngx_connection_t     ngx_eventfd_conn;

#endif

static ngx_str_t      epoll_name = ngx_string("epoll");

static ngx_command_t  ngx_epoll_commands[] = {

    { ngx_string("epoll_events"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_epoll_conf_t, events),
      NULL },

    { ngx_string("worker_aio_requests"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_epoll_conf_t, aio_requests),
      NULL },

      ngx_null_command
};


ngx_event_module_t  ngx_epoll_module_ctx = {
    &epoll_name,
    ngx_epoll_create_conf,               /* create configuration */
    ngx_epoll_init_conf,                 /* init configuration */

    {
        ngx_epoll_add_event,             /* add an event */
        ngx_epoll_del_event,             /* delete an event */
        ngx_epoll_add_event,             /* enable an event */
        ngx_epoll_del_event,             /* disable an event */
        ngx_epoll_add_connection,        /* add an connection */
        ngx_epoll_del_connection,        /* delete an connection */
#if (NGX_HAVE_EVENTFD)
        ngx_epoll_notify,                /* trigger a notify */
#else
        NULL,                            /* trigger a notify */
#endif
        ngx_epoll_process_events,        /* process the events */
        ngx_epoll_init,                  /* init the events */
        ngx_epoll_done,                  /* done the events */
    }
};

ngx_module_t  ngx_epoll_module = {
    NGX_MODULE_V1,
    &ngx_epoll_module_ctx,               /* module context */
    ngx_epoll_commands,                  /* module directives */
    NGX_EVENT_MODULE,                    /* module type */
    NULL,                                /* init master */
    NULL,                                /* init module */
    NULL,                                /* init process */
    NULL,                                /* init thread */
    NULL,                                /* exit thread */
    NULL,                                /* exit process */
    NULL,                                /* exit master */
    NGX_MODULE_V1_PADDING
};
{% endhighlight %}
下面我们简要介绍一下这些变量：

* ep: 存放epoll_create创建的句柄对象

* event_list: 存放epoll_wait()获取到的事件

* nevents: event_list长度

* notify_fd: 用于发送或接收通知的fd

* notify_event： 通知事件

* notify_conn： notify所关联的ngx_connection_t对象

* ngx_eventfd： aio相关的fd，epoll监控此文件句柄。当前不支持NGX_HAVE_FILE_AIO

* ngx_aio_ctx： aio所关联的上下文对象。当前不支持NGX_HAVE_FILE_AIO

* ngx_eventfd_event: aio所关联的事件。当前不支持NGX_HAVE_FILE_AIO

* ngx_eventfd_conn： aio所关联的ngx_connection_t对象。当前不支持NGX_HAVE_FILE_AIO

* epoll_name: 只是epoll模块的名称

* ngx_epoll_commands： epoll模块所支持的指令

* ngx_epoll_module_ctx: epoll模块所关联的上下文对象

* ngx_epoll_module： epoll模块对象


## 6. aio操作相关
{% highlight string %}
#if (NGX_HAVE_FILE_AIO)

/*
 * We call io_setup(), io_destroy() io_submit(), and io_getevents() directly
 * as syscalls instead of libaio usage, because the library header file
 * supports eventfd() since 0.3.107 version only.
 */

static int
io_setup(u_int nr_reqs, aio_context_t *ctx)
{
    return syscall(SYS_io_setup, nr_reqs, ctx);
}


static int
io_destroy(aio_context_t ctx)
{
    return syscall(SYS_io_destroy, ctx);
}


static int
io_getevents(aio_context_t ctx, long min_nr, long nr, struct io_event *events,
    struct timespec *tmo)
{
    return syscall(SYS_io_getevents, ctx, min_nr, nr, events, tmo);
}


static void
ngx_epoll_aio_init(ngx_cycle_t *cycle, ngx_epoll_conf_t *epcf)
{
    int                 n;
    struct epoll_event  ee;

#if (NGX_HAVE_SYS_EVENTFD_H)
    ngx_eventfd = eventfd(0, 0);
#else
    ngx_eventfd = syscall(SYS_eventfd, 0);
#endif

    if (ngx_eventfd == -1) {
        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                      "eventfd() failed");
        ngx_file_aio = 0;
        return;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                   "eventfd: %d", ngx_eventfd);

    n = 1;

    if (ioctl(ngx_eventfd, FIONBIO, &n) == -1) {
        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                      "ioctl(eventfd, FIONBIO) failed");
        goto failed;
    }

    if (io_setup(epcf->aio_requests, &ngx_aio_ctx) == -1) {
        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                      "io_setup() failed");
        goto failed;
    }

    ngx_eventfd_event.data = &ngx_eventfd_conn;
    ngx_eventfd_event.handler = ngx_epoll_eventfd_handler;
    ngx_eventfd_event.log = cycle->log;
    ngx_eventfd_event.active = 1;
    ngx_eventfd_conn.fd = ngx_eventfd;
    ngx_eventfd_conn.read = &ngx_eventfd_event;
    ngx_eventfd_conn.log = cycle->log;

    ee.events = EPOLLIN|EPOLLET;
    ee.data.ptr = &ngx_eventfd_conn;

    if (epoll_ctl(ep, EPOLL_CTL_ADD, ngx_eventfd, &ee) != -1) {
        return;
    }

    ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                  "epoll_ctl(EPOLL_CTL_ADD, eventfd) failed");

    if (io_destroy(ngx_aio_ctx) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "io_destroy() failed");
    }

failed:

    if (close(ngx_eventfd) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "eventfd close() failed");
    }

    ngx_eventfd = -1;
    ngx_aio_ctx = 0;
    ngx_file_aio = 0;
}

#endif
{% endhighlight %}
我们当前不支持aio，这里不做介绍。

## 7. 函数ngx_epoll_init()
{% highlight string %}
static ngx_int_t
ngx_epoll_init(ngx_cycle_t *cycle, ngx_msec_t timer)
{
    ngx_epoll_conf_t  *epcf;

    epcf = ngx_event_get_conf(cycle->conf_ctx, ngx_epoll_module);

    if (ep == -1) {
        ep = epoll_create(cycle->connection_n / 2);

        if (ep == -1) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                          "epoll_create() failed");
            return NGX_ERROR;
        }

#if (NGX_HAVE_EVENTFD)
        if (ngx_epoll_notify_init(cycle->log) != NGX_OK) {
            ngx_epoll_module_ctx.actions.notify = NULL;
        }
#endif

#if (NGX_HAVE_FILE_AIO)

        ngx_epoll_aio_init(cycle, epcf);

#endif
    }

    if (nevents < epcf->events) {
        if (event_list) {
            ngx_free(event_list);
        }

        event_list = ngx_alloc(sizeof(struct epoll_event) * epcf->events,
                               cycle->log);
        if (event_list == NULL) {
            return NGX_ERROR;
        }
    }

    nevents = epcf->events;

    ngx_io = ngx_os_io;

    ngx_event_actions = ngx_epoll_module_ctx.actions;

#if (NGX_HAVE_CLEAR_EVENT)
    ngx_event_flags = NGX_USE_CLEAR_EVENT
#else
    ngx_event_flags = NGX_USE_LEVEL_EVENT
#endif
                      |NGX_USE_GREEDY_EVENT
                      |NGX_USE_EPOLL_EVENT;

    return NGX_OK;
}

{% endhighlight %}

此函数会在epoll模块初始化时被调用。下面我们简要分析一下函数的实现：
{% highlight string %}
static ngx_int_t
ngx_epoll_init(ngx_cycle_t *cycle, ngx_msec_t timer)
{
	//1) 获取相应的配置，通常情况下我们不会在配置文件中配置对应选项，那么这里采用默认值
	//events为512，而aio_requests为32
	epcf = ngx_event_get_conf(cycle->conf_ctx, ngx_epoll_module);

	//2) 初始化epoll
	if(ep == -1){
		//2.1) 调用epoll_create创建ep句柄,当前epoll_create()参数其实是无作用的，但是要设为大于0
		ep = epoll_create(cycle->connection_n / 2);

		//2.2) 初始化notify机制
		ngx_epoll_notify_init()

		//2.3) aio初始化，当前我们并不支持
		#if (NGX_HAVE_FILE_AIO)

       		ngx_epoll_aio_init(cycle, epcf);

		#endif
	}

	//3) 为event_list分配空间，用于保存通过epoll_wait获取到的事件

	//4) 保存相应的变量值
	nevents = epcf->events;
	
	ngx_io = ngx_os_io;		//此处保存epoll事件驱动机制下的TCP、UDP发送与接收方法
	
	ngx_event_actions = ngx_epoll_module_ctx.actions;

	//这里我们支持NGX_HAVE_CLEAR_EVENT宏定义，采用边沿触发
	//NGX_USE_GREEDY_EVENT:表示对应的事件驱动机制的event filter需要重复执行IO操作，直到EAGAIN
#if (NGX_HAVE_CLEAR_EVENT)
	ngx_event_flags = NGX_USE_CLEAR_EVENT			
#else
	ngx_event_flags = NGX_USE_LEVEL_EVENT
#endif
                      |NGX_USE_GREEDY_EVENT		
                      |NGX_USE_EPOLL_EVENT;

	return NGX_OK;

}
{% endhighlight %}

## 8. 函数ngx_epoll_notify_init()
{% highlight string %}
#if (NGX_HAVE_EVENTFD)

static ngx_int_t
ngx_epoll_notify_init(ngx_log_t *log)
{
    struct epoll_event  ee;

#if (NGX_HAVE_SYS_EVENTFD_H)
    notify_fd = eventfd(0, 0);
#else
    notify_fd = syscall(SYS_eventfd, 0);
#endif

    if (notify_fd == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "eventfd() failed");
        return NGX_ERROR;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, log, 0,
                   "notify eventfd: %d", notify_fd);

    notify_event.handler = ngx_epoll_notify_handler;
    notify_event.log = log;
    notify_event.active = 1;

    notify_conn.fd = notify_fd;
    notify_conn.read = &notify_event;
    notify_conn.log = log;

    ee.events = EPOLLIN|EPOLLET;
    ee.data.ptr = &notify_conn;

    if (epoll_ctl(ep, EPOLL_CTL_ADD, notify_fd, &ee) == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
                      "epoll_ctl(EPOLL_CTL_ADD, eventfd) failed");

        if (close(notify_fd) == -1) {
            ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                            "eventfd close() failed");
        }

        return NGX_ERROR;
    }

    return NGX_OK;
}
#endif
{% endhighlight %}
本函数用于初始化事件通知机制，用来实现多进程或多线程的之间的等待/通知通知机制(与pipe功能类似)。请参看[eventfd 的分析与具体例子](https://blog.csdn.net/tanswer_/article/details/79008322)。本函数较为简单，这里不做介绍。

## 9. 函数ngx_epoll_notify_handler()
{% highlight string %}
#if (NGX_HAVE_EVENTFD)
static void
ngx_epoll_notify_handler(ngx_event_t *ev)
{
    ssize_t               n;
    uint64_t              count;
    ngx_err_t             err;
    ngx_event_handler_pt  handler;

    if (++ev->index == NGX_MAX_UINT32_VALUE) {
        ev->index = 0;

        n = read(notify_fd, &count, sizeof(uint64_t));

        err = ngx_errno;

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                       "read() eventfd %d: %z count:%uL", notify_fd, n, count);

        if ((size_t) n != sizeof(uint64_t)) {
            ngx_log_error(NGX_LOG_ALERT, ev->log, err,
                          "read() eventfd %d failed", notify_fd);
        }
    }

    handler = ev->data;
    handler(ev);
}

#endif

{% endhighlight %}
当接收到eventfd发送来的通知事件时，会调用此函数。每获取到一次通知，ev->index的值就会加1，当达到最大时就会调用read()函数一次读出。

## 10. 函数ngx_epoll_done()
{% highlight string %}
static void
ngx_epoll_done(ngx_cycle_t *cycle)
{
    if (close(ep) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "epoll close() failed");
    }

    ep = -1;

#if (NGX_HAVE_EVENTFD)

    if (close(notify_fd) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "eventfd close() failed");
    }

    notify_fd = -1;

#endif

#if (NGX_HAVE_FILE_AIO)

    if (ngx_eventfd != -1) {

        if (io_destroy(ngx_aio_ctx) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "io_destroy() failed");
        }

        if (close(ngx_eventfd) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "eventfd close() failed");
        }

        ngx_eventfd = -1;
    }

    ngx_aio_ctx = 0;

#endif

    ngx_free(event_list);

    event_list = NULL;
    nevents = 0;
}
{% endhighlight %}
当nginx epoll事件驱动机制退出时，调用此方法相应的系统资源。

## 11. 函数ngx_epoll_add_event()
{% highlight string %}
static ngx_int_t
ngx_epoll_add_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags)
{
    int                  op;
    uint32_t             events, prev;
    ngx_event_t         *e;
    ngx_connection_t    *c;
    struct epoll_event   ee;

    c = ev->data;

    events = (uint32_t) event;

    if (event == NGX_READ_EVENT) {
        e = c->write;
        prev = EPOLLOUT;
#if (NGX_READ_EVENT != EPOLLIN|EPOLLRDHUP)
        events = EPOLLIN|EPOLLRDHUP;
#endif

    } else {
        e = c->read;
        prev = EPOLLIN|EPOLLRDHUP;
#if (NGX_WRITE_EVENT != EPOLLOUT)
        events = EPOLLOUT;
#endif
    }

    if (e->active) {
        op = EPOLL_CTL_MOD;
        events |= prev;

    } else {
        op = EPOLL_CTL_ADD;
    }

    ee.events = events | (uint32_t) flags;
    ee.data.ptr = (void *) ((uintptr_t) c | ev->instance);

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "epoll add event: fd:%d op:%d ev:%08XD",
                   c->fd, op, ee.events);

    if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
        ngx_log_error(NGX_LOG_ALERT, ev->log, ngx_errno,
                      "epoll_ctl(%d, %d) failed", op, c->fd);
        return NGX_ERROR;
    }

    ev->active = 1;
#if 0
    ev->oneshot = (flags & NGX_ONESHOT_EVENT) ? 1 : 0;
#endif

    return NGX_OK;
}
{% endhighlight %}
本函数用于实现向epoll事件驱动机制中添加相应的事件。下面我们简要分析一下函数的实现：
{% highlight string %}
static ngx_int_t
ngx_epoll_add_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags)
{
	if (event == NGX_READ_EVENT) {
		//1） 若添加的读事件，这里我们要保存 '写事件' 的相关状态(原因，主要是与epoll添加、移除事件的机制有关)
		e = c->write;
		prev = EPOLLOUT;

		#if (NGX_READ_EVENT != EPOLLIN|EPOLLRDHUP)
			events = EPOLLIN|EPOLLRDHUP;
		#endif

	}else{

		//2) 若添加的写事件，这里我们要保存 '读事件' 的相关状态(原因，主要是与epoll添加、移除事件的机制有关)
		e = c->read;
		prev = EPOLLIN|EPOLLRDHUP;
		#if (NGX_WRITE_EVENT != EPOLLOUT)
			events = EPOLLOUT;
		#endif
	
	}

	//3) 判定到底是采用add还是modify方法来添加事件
	if (e->active) {
		op = EPOLL_CTL_MOD;
		events |= prev;
	
	} else {
		op = EPOLL_CTL_ADD;
	}

	//4) 对应fd所绑定的事件信息。这里注意ev->instance用于侦测epoll中的陈旧事件(stale event)：
	//因为当connection被关闭是ngx_connection_t相应的变量值都会清0
	ee.events = events | (uint32_t) flags;
	ee.data.ptr = (void *) ((uintptr_t) c | ev->instance);	

	//5) 添加到epoll事件驱动机制中
	if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
	}
}
{% endhighlight %}

## 12. 函数ngx_epoll_del_event()
{% highlight string %}
static ngx_int_t
ngx_epoll_del_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags)
{
    int                  op;
    uint32_t             prev;
    ngx_event_t         *e;
    ngx_connection_t    *c;
    struct epoll_event   ee;

    /*
     * when the file descriptor is closed, the epoll automatically deletes
     * it from its queue, so we do not need to delete explicitly the event
     * before the closing the file descriptor
     */

    if (flags & NGX_CLOSE_EVENT) {
        ev->active = 0;
        return NGX_OK;
    }

    c = ev->data;

    if (event == NGX_READ_EVENT) {
        e = c->write;
        prev = EPOLLOUT;

    } else {
        e = c->read;
        prev = EPOLLIN|EPOLLRDHUP;
    }

    if (e->active) {
        op = EPOLL_CTL_MOD;
        ee.events = prev | (uint32_t) flags;
        ee.data.ptr = (void *) ((uintptr_t) c | ev->instance);

    } else {
        op = EPOLL_CTL_DEL;
        ee.events = 0;
        ee.data.ptr = NULL;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "epoll del event: fd:%d op:%d ev:%08XD",
                   c->fd, op, ee.events);

    if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
        ngx_log_error(NGX_LOG_ALERT, ev->log, ngx_errno,
                      "epoll_ctl(%d, %d) failed", op, c->fd);
        return NGX_ERROR;
    }

    ev->active = 0;

    return NGX_OK;
}
{% endhighlight %}
本函数用于从epoll事件驱动机制中移除一个事件。下面我们简要分析一下函数的实现：
{% highlight string %}
static ngx_int_t
ngx_epoll_del_event(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags)
{
	//1) 当fd被关闭时，此时该fd所关联的事件会自动的从epoll事件驱动机制中移除，我们可以不用做任何处理
	if (flags & NGX_CLOSE_EVENT) {
		ev->active = 0;
		return NGX_OK;
	}

	if (event == NGX_READ_EVENT) {

		//2） 若移除的是读事件，这里我们要保存 '写事件' 的相关状态(原因，主要是与epoll添加、移除事件的机制有关)
		e = c->write;
		prev = EPOLLOUT;
	}else{
		
		//3) 若移除的是写事件，这里我们要保存 '读事件' 的相关状态(原因，主要是与epoll添加、移除事件的机制有关)
		e = c->read;
        prev = EPOLLIN|EPOLLRDHUP;
	}

	//4) 判定到底是采用modify还是del方法来移除事件

	//5) 移除事件
	if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
	}
}
{% endhighlight %}

## 12. 函数
{% highlight string %}
static ngx_int_t
ngx_epoll_add_connection(ngx_connection_t *c)
{
    struct epoll_event  ee;

    ee.events = EPOLLIN|EPOLLOUT|EPOLLET|EPOLLRDHUP;
    ee.data.ptr = (void *) ((uintptr_t) c | c->read->instance);

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "epoll add connection: fd:%d ev:%08XD", c->fd, ee.events);

    if (epoll_ctl(ep, EPOLL_CTL_ADD, c->fd, &ee) == -1) {
        ngx_log_error(NGX_LOG_ALERT, c->log, ngx_errno,
                      "epoll_ctl(EPOLL_CTL_ADD, %d) failed", c->fd);
        return NGX_ERROR;
    }

    c->read->active = 1;
    c->write->active = 1;

    return NGX_OK;
}
{% endhighlight %}
将一个connection(连接）添加到事件驱动机制中，连接上的读、写事件即被加入到了事件驱动机制中


## 13. 函数ngx_epoll_del_connection()
{% highlight string %}
static ngx_int_t
ngx_epoll_del_connection(ngx_connection_t *c, ngx_uint_t flags)
{
    int                 op;
    struct epoll_event  ee;

    /*
     * when the file descriptor is closed the epoll automatically deletes
     * it from its queue so we do not need to delete explicitly the event
     * before the closing the file descriptor
     */

    if (flags & NGX_CLOSE_EVENT) {
        c->read->active = 0;
        c->write->active = 0;
        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "epoll del connection: fd:%d", c->fd);

    op = EPOLL_CTL_DEL;
    ee.events = 0;
    ee.data.ptr = NULL;

    if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
        ngx_log_error(NGX_LOG_ALERT, c->log, ngx_errno,
                      "epoll_ctl(%d, %d) failed", op, c->fd);
        return NGX_ERROR;
    }

    c->read->active = 0;
    c->write->active = 0;

    return NGX_OK;
}

{% endhighlight %}
从事件驱动机制中删除一个连接的读、写事件。这里注意，如果关闭连接的话，则让操作系统自动的将相应的事件移除

## 14. 函数
{% highlight string %}
#if (NGX_HAVE_EVENTFD)

static ngx_int_t
ngx_epoll_notify(ngx_event_handler_pt handler)
{
    static uint64_t inc = 1;

    notify_event.data = handler;

    if ((size_t) write(notify_fd, &inc, sizeof(uint64_t)) != sizeof(uint64_t)) {
        ngx_log_error(NGX_LOG_ALERT, notify_event.log, ngx_errno,
                      "write() to eventfd %d failed", notify_fd);
        return NGX_ERROR;
    }

    return NGX_OK;
}

#endif
{% endhighlight %}
使用eventfd向外发出通知。

## 15. 函数
{% highlight string %}
static ngx_int_t
ngx_epoll_process_events(ngx_cycle_t *cycle, ngx_msec_t timer, ngx_uint_t flags)
{
    int                events;
    uint32_t           revents;
    ngx_int_t          instance, i;
    ngx_uint_t         level;
    ngx_err_t          err;
    ngx_event_t       *rev, *wev;
    ngx_queue_t       *queue;
    ngx_connection_t  *c;

    /* NGX_TIMER_INFINITE == INFTIM */

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                   "epoll timer: %M", timer);

    events = epoll_wait(ep, event_list, (int) nevents, timer);

    err = (events == -1) ? ngx_errno : 0;

    if (flags & NGX_UPDATE_TIME || ngx_event_timer_alarm) {
        ngx_time_update();
    }

    if (err) {
        if (err == NGX_EINTR) {

            if (ngx_event_timer_alarm) {
                ngx_event_timer_alarm = 0;
                return NGX_OK;
            }

            level = NGX_LOG_INFO;

        } else {
            level = NGX_LOG_ALERT;
        }

        ngx_log_error(level, cycle->log, err, "epoll_wait() failed");
        return NGX_ERROR;
    }

    if (events == 0) {
        if (timer != NGX_TIMER_INFINITE) {
            return NGX_OK;
        }

        ngx_log_error(NGX_LOG_ALERT, cycle->log, 0,
                      "epoll_wait() returned no events without timeout");
        return NGX_ERROR;
    }

    for (i = 0; i < events; i++) {
        c = event_list[i].data.ptr;

        instance = (uintptr_t) c & 1;
        c = (ngx_connection_t *) ((uintptr_t) c & (uintptr_t) ~1);

        rev = c->read;

        if (c->fd == -1 || rev->instance != instance) {

            /*
             * the stale event from a file descriptor
             * that was just closed in this iteration
             */

            ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                           "epoll: stale event %p", c);
            continue;
        }

        revents = event_list[i].events;

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                       "epoll: fd:%d ev:%04XD d:%p",
                       c->fd, revents, event_list[i].data.ptr);

        if (revents & (EPOLLERR|EPOLLHUP)) {
            ngx_log_debug2(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                           "epoll_wait() error on fd:%d ev:%04XD",
                           c->fd, revents);
        }

#if 0
        if (revents & ~(EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP)) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, 0,
                          "strange epoll_wait() events fd:%d ev:%04XD",
                          c->fd, revents);
        }
#endif

        if ((revents & (EPOLLERR|EPOLLHUP))
             && (revents & (EPOLLIN|EPOLLOUT)) == 0)
        {
            /*
             * if the error events were returned without EPOLLIN or EPOLLOUT,
             * then add these flags to handle the events at least in one
             * active handler
             */

            revents |= EPOLLIN|EPOLLOUT;
        }

        if ((revents & EPOLLIN) && rev->active) {

#if (NGX_HAVE_EPOLLRDHUP)
            if (revents & EPOLLRDHUP) {
                rev->pending_eof = 1;
            }
#endif

            rev->ready = 1;

            if (flags & NGX_POST_EVENTS) {
                queue = rev->accept ? &ngx_posted_accept_events
                                    : &ngx_posted_events;

                ngx_post_event(rev, queue);

            } else {
                rev->handler(rev);
            }
        }

        wev = c->write;

        if ((revents & EPOLLOUT) && wev->active) {

            if (c->fd == -1 || wev->instance != instance) {

                /*
                 * the stale event from a file descriptor
                 * that was just closed in this iteration
                 */

                ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                               "epoll: stale event %p", c);
                continue;
            }

            wev->ready = 1;
#if (NGX_THREADS)
            wev->complete = 1;
#endif

            if (flags & NGX_POST_EVENTS) {
                ngx_post_event(wev, &ngx_posted_events);

            } else {
                wev->handler(wev);
            }
        }
    }

    return NGX_OK;
}
{% endhighlight %}
此函数作为epoll事件驱动机制处理事件的主函数。下面我们简要分析一下函数的实现：
{% highlight string %}
static ngx_int_t
ngx_epoll_process_events(ngx_cycle_t *cycle, ngx_msec_t timer, ngx_uint_t flags)
{
	//1) 等待事件的产生
	events = epoll_wait(ep, event_list, (int) nevents, timer);

	//2) 保存相应的错误信息
	err = (events == -1) ? ngx_errno : 0;

	//3) 更新Nginx缓存时间
	if (flags & NGX_UPDATE_TIME || ngx_event_timer_alarm) {
        ngx_time_update();
    }

	//3) 错误处理 
	if (err){
		if (err == NGX_EINTR) {

			//SIGALRM信号产生的中断，这里我们上面已经更新了时间，因此这里直接将ngx_event_timer_alarm变量置为0
			if (ngx_event_timer_alarm) {
				ngx_event_timer_alarm = 0;
				return NGX_OK;
			}
		}
	}

	//4) 未返回任何事件
	if (events == 0) {

		//超时导致的返回
		if (timer != NGX_TIMER_INFINITE) {
            return NGX_OK;
        }
	}
	
	//5) 遍历返回的事件列表
	for (i = 0; i < events; i++) {
		
		if (c->fd == -1 || rev->instance != instance) {
			//5.1） 对应的连接已经关闭(连接相关的字段都会清零)，本事件是属于一个陈旧事件
		}

		if (revents & (EPOLLERR|EPOLLHUP)) {
			//5.2) 在对应的fd上产生了错误，这里打印错误提示信息
		}

	
		if ((revents & (EPOLLERR|EPOLLHUP))
			&& (revents & (EPOLLIN|EPOLLOUT)) == 0)
		{
			//5.3) 对应的fd上产生了错误，也没有产生读、写事件，这里为了能将相应的错误告知
			//外层，强制revents |= EPOLLIN|EPOLLOUT;
			
		}

		//5.4) 处理可读事件
		if ((revents & EPOLLIN) && rev->active) {

			//读取到了对端关闭
			#if (NGX_HAVE_EPOLLRDHUP)
	            if (revents & EPOLLRDHUP) {
	                rev->pending_eof = 1;
	            }
			#endif


			//ready置为1，表示有可读事件
			rev->ready = 1;

			// post事件或直接处理
			if (flags & NGX_POST_EVENTS) {
				queue = rev->accept ? &ngx_posted_accept_events
					: &ngx_posted_events;
			
				ngx_post_event(rev, queue);
			
			} else {
				rev->handler(rev);
			}

		}	

		//5.5) 处理可写事件
		wev = c->write;	
		if ((revents & EPOLLOUT) && wev->active) {
			
			if (c->fd == -1 || wev->instance != instance) {
				// 对应的连接已经关闭(连接相关的字段都会清零)，本事件是属于一个陈旧事件
				continue;
			}

			//ready置为1，表示有可写事件
			wev->ready = 1;
			#if (NGX_THREADS)
				wev->complete = 1;
			#endif


			// post事件或直接处理
			if (flags & NGX_POST_EVENTS) {
				ngx_post_event(wev, &ngx_posted_events);
			
			} else {
				wev->handler(wev);
			}

		}

	}
}
{% endhighlight %}


## 16. 函数ngx_epoll_eventfd_handler()
{% highlight string %}
#if (NGX_HAVE_FILE_AIO)

static void
ngx_epoll_eventfd_handler(ngx_event_t *ev)
{
    int               n, events;
    long              i;
    uint64_t          ready;
    ngx_err_t         err;
    ngx_event_t      *e;
    ngx_event_aio_t  *aio;
    struct io_event   event[64];
    struct timespec   ts;

    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, ev->log, 0, "eventfd handler");

    n = read(ngx_eventfd, &ready, 8);

    err = ngx_errno;

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, ev->log, 0, "eventfd: %d", n);

    if (n != 8) {
        if (n == -1) {
            if (err == NGX_EAGAIN) {
                return;
            }

            ngx_log_error(NGX_LOG_ALERT, ev->log, err, "read(eventfd) failed");
            return;
        }

        ngx_log_error(NGX_LOG_ALERT, ev->log, 0,
                      "read(eventfd) returned only %d bytes", n);
        return;
    }

    ts.tv_sec = 0;
    ts.tv_nsec = 0;

    while (ready) {

        events = io_getevents(ngx_aio_ctx, 1, 64, event, &ts);

        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                       "io_getevents: %d", events);

        if (events > 0) {
            ready -= events;

            for (i = 0; i < events; i++) {

                ngx_log_debug4(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                               "io_event: %XL %XL %L %L",
                                event[i].data, event[i].obj,
                                event[i].res, event[i].res2);

                e = (ngx_event_t *) (uintptr_t) event[i].data;

                e->complete = 1;
                e->active = 0;
                e->ready = 1;

                aio = e->data;
                aio->res = event[i].res;

                ngx_post_event(e, &ngx_posted_events);
            }

            continue;
        }

        if (events == 0) {
            return;
        }

        /* events == -1 */
        ngx_log_error(NGX_LOG_ALERT, ev->log, ngx_errno,
                      "io_getevents() failed");
        return;
    }
}

#endif
{% endhighlight %}
当前我们不支持aio，这里不做介绍。

## 17. 函数ngx_epoll_create_conf()
{% highlight string %}
static void *
ngx_epoll_create_conf(ngx_cycle_t *cycle)
{
    ngx_epoll_conf_t  *epcf;

    epcf = ngx_palloc(cycle->pool, sizeof(ngx_epoll_conf_t));
    if (epcf == NULL) {
        return NULL;
    }

    epcf->events = NGX_CONF_UNSET;
    epcf->aio_requests = NGX_CONF_UNSET;

    return epcf;
}
{% endhighlight %}
本函数用于创建epoll模块所需要的配置信息。

## 18. 函数ngx_epoll_init_conf()
{% highlight string %}
static char *
ngx_epoll_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_epoll_conf_t *epcf = conf;

    ngx_conf_init_uint_value(epcf->events, 512);
    ngx_conf_init_uint_value(epcf->aio_requests, 32);

    return NGX_CONF_OK;
}
{% endhighlight %}
本函数用于默认初始化epoll的配置。


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

