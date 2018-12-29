---
layout: post
title: event/ngx_event_posted.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章主要实现对nginx posted event消息的处理。


<!-- more -->


## 1. ngx_event_posted.h头文件

### 1.1 相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_POSTED_H_INCLUDED_
#define _NGX_EVENT_POSTED_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define ngx_post_event(ev, q)                                                 \
                                                                              \
    if (!(ev)->posted) {                                                      \
        (ev)->posted = 1;                                                     \
        ngx_queue_insert_tail(q, &(ev)->queue);                               \
                                                                              \
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, (ev)->log, 0, "post event %p", ev);\
                                                                              \
    } else  {                                                                 \
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, (ev)->log, 0,                      \
                       "update posted event %p", ev);                         \
    }


#define ngx_delete_posted_event(ev)                                           \
                                                                              \
    (ev)->posted = 0;                                                         \
    ngx_queue_remove(&(ev)->queue);                                           \
                                                                              \
    ngx_log_debug1(NGX_LOG_DEBUG_CORE, (ev)->log, 0,                          \
                   "delete posted event %p", ev);
{% endhighlight %}

下面我们简要介绍一下这两个宏定义：

* ngx_post_event: 此宏定义用于将事件```ev```插入到队列```q```的队尾。

* ngx_delete_posted_event: 此用定义用于将事件```ev```从队列中移除

### 1.2 相关函数声明
{% highlight string %}

//遍历队列中的事件，然后调用事件所绑定的handler回调函数
void ngx_event_process_posted(ngx_cycle_t *cycle, ngx_queue_t *posted);
{% endhighlight %}



### 1.3 相关全局变量声明
{% highlight string %}

extern ngx_queue_t  ngx_posted_accept_events;
extern ngx_queue_t  ngx_posted_events;


#endif /* _NGX_EVENT_POSTED_H_INCLUDED_ */
{% endhighlight %}

* ngx_posted_accept_events: 对于accept事件的投递队列

* ngx_posted_events: 对于普通事件的投递队列


## 2. ngx_event_posted.c源文件

### 2.1 相关全局变量定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ngx_queue_t  ngx_posted_accept_events;
ngx_queue_t  ngx_posted_events;
{% endhighlight %}

* ngx_posted_accept_events: 对于accept事件的投递队列

* ngx_posted_events: 对于普通事件的投递队列

### 2.2 函数ngx_event_process_posted()
{% highlight string %}
void
ngx_event_process_posted(ngx_cycle_t *cycle, ngx_queue_t *posted)
{
    ngx_queue_t  *q;
    ngx_event_t  *ev;

    while (!ngx_queue_empty(posted)) {

        q = ngx_queue_head(posted);
        ev = ngx_queue_data(q, ngx_event_t, queue);

        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                      "posted event %p", ev);

        ngx_delete_posted_event(ev);

        ev->handler(ev);
    }
}
{% endhighlight %}
此函数较为简单，就是不断的从事件队列中取出事件，然后调用事件所绑定的handler()回调函数进行处理




<br />
<br />

**[参看]**






<br />
<br />
<br />

