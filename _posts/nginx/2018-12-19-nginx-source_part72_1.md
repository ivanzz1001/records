---
layout: post
title: event/ngx_event_timer.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章讲述一下Nginx中通过红黑树来实现定时器的管理。


<!-- more -->


## 1. 相关函数声明
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_TIMER_H_INCLUDED_
#define _NGX_EVENT_TIMER_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define NGX_TIMER_INFINITE  (ngx_msec_t) -1

#define NGX_TIMER_LAZY_DELAY  300


//初始化定时器管理红黑树
ngx_int_t ngx_event_timer_init(ngx_log_t *log);

//在红黑树中查找当前最近超时时间（即红黑树的最左节点）
ngx_msec_t ngx_event_find_timer(void);

//处理已经超时的定时器
void ngx_event_expire_timers(void);

//取消定时器红黑树中的所有定时器，按超时时间从小到大取消，并调用该定时器所关联的handler()回调函数
void ngx_event_cancel_timers(void);
{% endhighlight %}





<br />
<br />

**[参看]**






<br />
<br />
<br />

