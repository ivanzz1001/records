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
//
static char *ngx_event_init_conf(ngx_cycle_t *cycle, void *conf);


static ngx_int_t ngx_event_module_init(ngx_cycle_t *cycle);


static ngx_int_t ngx_event_process_init(ngx_cycle_t *cycle);


static char *ngx_events_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static char *ngx_event_connections(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


static char *ngx_event_use(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static char *ngx_event_debug_connection(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


static void *ngx_event_core_create_conf(ngx_cycle_t *cycle);


static char *ngx_event_core_init_conf(ngx_cycle_t *cycle, void *conf);
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

