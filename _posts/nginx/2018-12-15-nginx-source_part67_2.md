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

