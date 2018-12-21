---
layout: post
title: event/ngx_event.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本章我们讲述一下nginx中event的实现。Nginx中的event对象```ngx_event_t```提供了一种机制，能够通知程序发生了某个事件。这里的event主要包括两大种类：

* IO事件

* 定时器事件

<!-- more -->


## 1. 相关事件模块变量的声明
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define DEFAULT_CONNECTIONS  512


extern ngx_module_t ngx_kqueue_module;
extern ngx_module_t ngx_eventport_module;
extern ngx_module_t ngx_devpoll_module;
extern ngx_module_t ngx_epoll_module;
extern ngx_module_t ngx_select_module;
{% endhighlight %}
通常情况下，我们Linux操作系统支持select、poll、epoll这三种事件驱动机制。这里nginx启动时会根据当前event模块的配置选择恰当的事件驱动。
{% highlight string %}
注： 这里似乎没有ngx_poll_module，但是在ngx_event_core_init_conf()函数中，通过cycle->modules[i]仍能够选择到。
{% endhighlight %}

## 2. 相关静态函数声明
{% highlight string %}
//初始化event模块 上下文 时的一个回调函数
static char *ngx_event_init_conf(ngx_cycle_t *cycle, void *conf);

//初始化event core模块的回调函数
static ngx_int_t ngx_event_module_init(ngx_cycle_t *cycle);

//初始化进程时候针对event core模块的回调函数，其会在ngx_event_module_init()函数之后才会调用
static ngx_int_t ngx_event_process_init(ngx_cycle_t *cycle);

//配置文件解析到events命令时候的回调函数
static char *ngx_events_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

//配置文件解析到worker_connections指令时的回调函数
static char *ngx_event_connections(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);

//配置文件解析到use指令时的回调函数
static char *ngx_event_use(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

//配置文件解析到debug_connection指令时的回调函数
static char *ngx_event_debug_connection(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);

//创建event core模块上下文的回调函数
static void *ngx_event_core_create_conf(ngx_cycle_t *cycle);

//初始化event core模块上下文的回调函数
static char *ngx_event_core_init_conf(ngx_cycle_t *cycle, void *conf);
{% endhighlight %}


## 3. 相关变量定义
{% highlight string %}
static ngx_uint_t     ngx_timer_resolution;
sig_atomic_t          ngx_event_timer_alarm;

static ngx_uint_t     ngx_event_max_module;

ngx_uint_t            ngx_event_flags;
ngx_event_actions_t   ngx_event_actions;


static ngx_atomic_t   connection_counter = 1;
ngx_atomic_t         *ngx_connection_counter = &connection_counter;


ngx_atomic_t         *ngx_accept_mutex_ptr;
ngx_shmtx_t           ngx_accept_mutex;
ngx_uint_t            ngx_use_accept_mutex;
ngx_uint_t            ngx_accept_events;
ngx_uint_t            ngx_accept_mutex_held;
ngx_msec_t            ngx_accept_mutex_delay;
ngx_int_t             ngx_accept_disabled;


#if (NGX_STAT_STUB)

ngx_atomic_t   ngx_stat_accepted0;
ngx_atomic_t  *ngx_stat_accepted = &ngx_stat_accepted0;
ngx_atomic_t   ngx_stat_handled0;
ngx_atomic_t  *ngx_stat_handled = &ngx_stat_handled0;
ngx_atomic_t   ngx_stat_requests0;
ngx_atomic_t  *ngx_stat_requests = &ngx_stat_requests0;
ngx_atomic_t   ngx_stat_active0;
ngx_atomic_t  *ngx_stat_active = &ngx_stat_active0;
ngx_atomic_t   ngx_stat_reading0;
ngx_atomic_t  *ngx_stat_reading = &ngx_stat_reading0;
ngx_atomic_t   ngx_stat_writing0;
ngx_atomic_t  *ngx_stat_writing = &ngx_stat_writing0;
ngx_atomic_t   ngx_stat_waiting0;
ngx_atomic_t  *ngx_stat_waiting = &ngx_stat_waiting0;

#endif
{% endhighlight %}

下面我们简要介绍一下这些字段：

* ngx_timer_resolution: 可以通过nginx配置文件主配置段中的```timer_resolution```指令来降低worker进程的定时器分辨率(timer resolution)，这样就可以降低gettimeofday()系统调用的次数。默认情况下，gettimeofday()会在每一次收到kernel事件时被调用。我们可以通过降低timer resolution的值，这样gettimeofday()函数就会每隔```interval```时间被调用一次。例如：
<pre>
timer_resolution 100ms;
</pre>
内部```interval```的实现依赖于所使用的方法：
<pre>
假如所采用的事件驱动机制是```kqueue```的话，使用EVFILT_TIMER filter来实现
假如所采用的事件驱动机制是```eventport```的话，使用timer_create()来实现
否则，采用setitimer()来实现
</pre>

* ngx_event_timer_alarm: 当收到```SIGALARM```信号时，会将此变量置为1，以指示更新Nginx缓存时间

* ngx_event_max_module: 当前属于NGX_EVENT_MODULE模块的个数

*  ngx_event_flags: 用于保存当前```事件驱动机制```所支持的事件掩码

* ngx_event_actions: 用于保存当前事件驱动机制下的actions函数

* connection_counter/ngx_connection_counter: 用于统计当前的连接数

* ngx_accept_mutex_ptr: 存放互斥量内存地址的指针

* ngx_accept_mutex: accept互斥锁

* ngx_use_accept_mutex: 表示是否需要通过对accept加锁来解决惊群问题。当nginx worker进程数```大于1```时，且配置文件中开启了```accept_mutex```时，这个标志置为1

* ngx_accept_events: 表示在获取accept互斥锁的时候，是否还需要调用ngx_enable_accept_events()来使能accept events。通常采用```eventport```这一事件驱动机制时才需要。

* ngx_accept_mutex_held: 用于指示当前是否拿到了锁

* ngx_accept_mutex_delay: 当```accept_mutex```被启用的话，如果当前有另一个worker进程正在accept新连接(connections)，则当前worker进程最长会等待多长时间尝试重新开始accept新连接。其实就是获取互斥锁的最大延迟时间

* ngx_accept_disabled: 用于控制当前worker进程是否参与获取```ngx_accept_mutex```。一般在当前worker进程连接数过多时，就不会参与竞争，这样可以起到负载均衡的目的。

<br />

我们当前并不支持```NGX_STAT_STUB```，该宏定义需要在启用```HTTP_STUB_STATUS```编译选项时才会定义。请参看[Module ngx_http_stub_status_module](https://nginx.org/en/docs/http/ngx_http_stub_status_module.html)但是这里我们还是简单介绍一下各字段的含义。

* ngx_stat_accepted0/ngx_stat_accepted: 用于统计当前nginx一共accept多少连接

* ngx_stat_handled0/ngx_stat_handled: 用于统计当前所处理的总的连接数。通常情况下，本字段的值与```ngx_stat_accepted```字段的值相同，除非达到了一些资源的限制（例如，worker_connections的限制）

* ngx_stat_requests0/ngx_stat_requests: 用于统计客户端总的请求数

* ngx_stat_active0/ngx_stat_active: 用于统计当前处于active状态的客户端连接数，这也包括```Waiting```状态的连接

* ngx_stat_reading0/ngx_stat_reading: 当前Nginx正在读取```请求头```(request header)的连接数

* ngx_stat_writing0/ngx_stat_writing: 当前nginx正在向客户端```回写应答```(write the response back)的连接数

* ngx_stat_waiting0/ngx_stat_waiting: 当前正处于空闲状态的客户端连接数。

## 4. event module相关变量
{% highlight string %}
static ngx_command_t  ngx_events_commands[] = {

    { ngx_string("events"),
      NGX_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
      ngx_events_block,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_core_module_t  ngx_events_module_ctx = {
    ngx_string("events"),
    NULL,
    ngx_event_init_conf
};


ngx_module_t  ngx_events_module = {
    NGX_MODULE_V1,
    &ngx_events_module_ctx,                /* module context */
    ngx_events_commands,                   /* module directives */
    NGX_CORE_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_str_t  event_core_name = ngx_string("event_core");


static ngx_command_t  ngx_event_core_commands[] = {

    { ngx_string("worker_connections"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_event_connections,
      0,
      0,
      NULL },

    { ngx_string("use"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_event_use,
      0,
      0,
      NULL },

    { ngx_string("multi_accept"),
      NGX_EVENT_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      0,
      offsetof(ngx_event_conf_t, multi_accept),
      NULL },

    { ngx_string("accept_mutex"),
      NGX_EVENT_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      0,
      offsetof(ngx_event_conf_t, accept_mutex),
      NULL },

    { ngx_string("accept_mutex_delay"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      0,
      offsetof(ngx_event_conf_t, accept_mutex_delay),
      NULL },

    { ngx_string("debug_connection"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_event_debug_connection,
      0,
      0,
      NULL },

      ngx_null_command
};


ngx_event_module_t  ngx_event_core_module_ctx = {
    &event_core_name,
    ngx_event_core_create_conf,            /* create configuration */
    ngx_event_core_init_conf,              /* init configuration */

    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};


ngx_module_t  ngx_event_core_module = {
    NGX_MODULE_V1,
    &ngx_event_core_module_ctx,            /* module context */
    ngx_event_core_commands,               /* module directives */
    NGX_EVENT_MODULE,                      /* module type */
    NULL,                                  /* init master */
    ngx_event_module_init,                 /* init module */
    ngx_event_process_init,                /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};
{% endhighlight %}

下面简单介绍一下这些变量：

* ngx_events_commands: nginx的```events```命令

* ngx_events_module_ctx: events模块上下文

* ngx_events_module: nginx events模块

* event_core_name: nginx 事件核心模块名称为```event_core```

* ngx_event_core_commands： nginx事件核心模块命令

* ngx_event_core_module_ctx: nginx事件核心模块上下文

* ngx_event_core_module： nginx事件核心模块

## 5. 函数ngx_process_events_and_timers()
{% highlight string %}
void
ngx_process_events_and_timers(ngx_cycle_t *cycle)
{
    ngx_uint_t  flags;
    ngx_msec_t  timer, delta;

    if (ngx_timer_resolution) {
        timer = NGX_TIMER_INFINITE;
        flags = 0;

    } else {
        timer = ngx_event_find_timer();
        flags = NGX_UPDATE_TIME;

#if (NGX_WIN32)

        /* handle signals from master in case of network inactivity */

        if (timer == NGX_TIMER_INFINITE || timer > 500) {
            timer = 500;
        }

#endif
    }

    if (ngx_use_accept_mutex) {
        if (ngx_accept_disabled > 0) {
            ngx_accept_disabled--;

        } else {
            if (ngx_trylock_accept_mutex(cycle) == NGX_ERROR) {
                return;
            }

            if (ngx_accept_mutex_held) {
                flags |= NGX_POST_EVENTS;

            } else {
                if (timer == NGX_TIMER_INFINITE
                    || timer > ngx_accept_mutex_delay)
                {
                    timer = ngx_accept_mutex_delay;
                }
            }
        }
    }

    delta = ngx_current_msec;

    (void) ngx_process_events(cycle, timer, flags);

    delta = ngx_current_msec - delta;

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                   "timer delta: %M", delta);

    ngx_event_process_posted(cycle, &ngx_posted_accept_events);

    if (ngx_accept_mutex_held) {
        ngx_shmtx_unlock(&ngx_accept_mutex);
    }

    if (delta) {
        ngx_event_expire_timers();
    }

    ngx_event_process_posted(cycle, &ngx_posted_events);
}
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

7. [Nginx源码分析 - Event事件篇 - Event模块和配置的初始化](https://blog.csdn.net/initphp/article/details/52434261)

8. [文章5：Nginx源码分析--事件循环](https://blog.csdn.net/yankai0219/article/details/8453297)

<br />
<br />
<br />

