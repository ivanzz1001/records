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


## 2. 相关全局变量声明
<pre>
extern ngx_rbtree_t  ngx_event_timer_rbtree;
</pre>

用于存放Nginx定时器的红黑树结构。

## 3. 函数ngx_event_del_timer()
{% highlight string %}
static ngx_inline void
ngx_event_del_timer(ngx_event_t *ev)
{
    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "event timer del: %d: %M",
                    ngx_event_ident(ev->data), ev->timer.key);

    ngx_rbtree_delete(&ngx_event_timer_rbtree, &ev->timer);

#if (NGX_DEBUG)
    ev->timer.left = NULL;
    ev->timer.right = NULL;
    ev->timer.parent = NULL;
#endif

    ev->timer_set = 0;
}
{% endhighlight %}
直接调用ngx_rbtree_delete()函数从红黑树中移除指定的节点。注意，移除之后，还需要将```timer_set```置为0.

## 4. 函数ngx_event_add_timer()
{% highlight string %}
static ngx_inline void
ngx_event_add_timer(ngx_event_t *ev, ngx_msec_t timer)
{
    ngx_msec_t      key;
    ngx_msec_int_t  diff;

    key = ngx_current_msec + timer;

    if (ev->timer_set) {

        /*
         * Use a previous timer value if difference between it and a new
         * value is less than NGX_TIMER_LAZY_DELAY milliseconds: this allows
         * to minimize the rbtree operations for fast connections.
         */

        diff = (ngx_msec_int_t) (key - ev->timer.key);

        if (ngx_abs(diff) < NGX_TIMER_LAZY_DELAY) {
            ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                           "event timer: %d, old: %M, new: %M",
                            ngx_event_ident(ev->data), ev->timer.key, key);
            return;
        }

        ngx_del_timer(ev);
    }

    ev->timer.key = key;

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "event timer add: %d: %M:%M",
                    ngx_event_ident(ev->data), timer, ev->timer.key);

    ngx_rbtree_insert(&ngx_event_timer_rbtree, &ev->timer);

    ev->timer_set = 1;
}


#endif /* _NGX_
{% endhighlight %}

本函数用于向红黑树中插入一个定时节点。下面简要分析一下函数的实现：
{% highlight string %}
static ngx_inline void
ngx_event_add_timer(ngx_event_t *ev, ngx_msec_t timer)
{
	//1） 计算定时器的绝对超时时间
	key = ngx_current_msec + timer;

	//2) ev->timer_set值为1，表示定时器已经被设置，并且还没有超时
	if (ev->timer_set) {

		//2.1) 该定时器原来设置的超时时间ev->timer.key，当前将要设置的超时时间'key'， 假如两者之间相差不超过
		// NGX_TIMER_LAZY_DELAY毫秒，那么这里仍沿用原来的定时时间。这主要是为了降低红黑树的频繁插入、删除
		//操作，以快速的响应客户端的连接请求
		diff = (ngx_msec_int_t) (key - ev->timer.key);
		if (ngx_abs(diff) < NGX_TIMER_LAZY_DELAY) {
			return;
		}
	
		//2.2) 将原来的定时器从红黑树中移除
		ngx_del_timer(ev);
	}

	//3) 将定时器插入红黑树中
	ngx_rbtree_insert(&ngx_event_timer_rbtree, &ev->timer);

	//4) 设置timer_set标志
	ev->timer_set = 1;
}
{% endhighlight %}



<br />
<br />

**[参看]**






<br />
<br />
<br />

