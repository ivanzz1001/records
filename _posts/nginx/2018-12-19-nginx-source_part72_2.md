---
layout: post
title: event/ngx_event_timer.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章讲述一下Nginx中通过红黑树来实现定时器的管理。


<!-- more -->


## 1. 相关变量定义
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ngx_rbtree_t              ngx_event_timer_rbtree;
static ngx_rbtree_node_t  ngx_event_timer_sentinel;
{% endhighlight %}

* ngx_event_timer_rbtree: 用于管理定时器的红黑树

* ngx_event_timer_sentinel: 红黑树所用到的一个sentinel节点

## 2. 函数ngx_event_timer_init()
{% highlight string %}
/*
 * the event timer rbtree may contain the duplicate keys, however,
 * it should not be a problem, because we use the rbtree to find
 * a minimum timer value only
 */

ngx_int_t
ngx_event_timer_init(ngx_log_t *log)
{
    ngx_rbtree_init(&ngx_event_timer_rbtree, &ngx_event_timer_sentinel,
                    ngx_rbtree_insert_timer_value);

    return NGX_OK;
}
{% endhighlight %}
此函数用于初始化红黑树结构。

## 3. 函数ngx_event_find_timer()
{% highlight string %}
ngx_msec_t
ngx_event_find_timer(void)
{
    ngx_msec_int_t      timer;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    if (ngx_event_timer_rbtree.root == &ngx_event_timer_sentinel) {
        return NGX_TIMER_INFINITE;
    }

    root = ngx_event_timer_rbtree.root;
    sentinel = ngx_event_timer_rbtree.sentinel;

    node = ngx_rbtree_min(root, sentinel);

    timer = (ngx_msec_int_t) (node->key - ngx_current_msec);

    return (ngx_msec_t) (timer > 0 ? timer : 0);
}
{% endhighlight %}

此函数用于从红黑树中查找当前最近(将要)过期定时器的剩余过期时间。

## 4. 函数ngx_event_expire_timers()
{% highlight string %}
void
ngx_event_expire_timers(void)
{
    ngx_event_t        *ev;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    sentinel = ngx_event_timer_rbtree.sentinel;

    for ( ;; ) {
        root = ngx_event_timer_rbtree.root;

        if (root == sentinel) {
            return;
        }

        node = ngx_rbtree_min(root, sentinel);

        /* node->key > ngx_current_time */

        if ((ngx_msec_int_t) (node->key - ngx_current_msec) > 0) {
            return;
        }

        ev = (ngx_event_t *) ((char *) node - offsetof(ngx_event_t, timer));

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

        ev->timedout = 1;

        ev->handler(ev);
    }
}
{% endhighlight %}
此函数用于处理当前红黑树中所有已经过期的定时器。下面我们简要分析一下函数的执行流程：
{% highlight string %}
void
ngx_event_expire_timers(void)
{
	sentinel = ngx_event_timer_rbtree.sentinel;
	
	for ( ;; ) {
	
		//1) 获取最近将要过期的红黑树节点
		node = ngx_rbtree_min(root, sentinel);

		if ((ngx_msec_int_t) (node->key - ngx_current_msec) > 0){
			//2) 未过期，直接return返回
		}

		//3) 获得过期节点的ngx_event_t结构
		ev = (ngx_event_t *) ((char *) node - offsetof(ngx_event_t, timer));

		//4) 从红黑树中移除该过期事件
		ngx_rbtree_delete(&ngx_event_timer_rbtree, &ev->timer);

		//5) 调用定时器所绑定的handler回调函数
		ev->timer_set = 0;			//timer_set标志为置为0
			
		ev->timedout = 1;			// timeout标志为置为1，表示定时器已经超时
		
		ev->handler(ev);

	}
}
{% endhighlight %}


## 5. 函数ngx_event_cancel_timers()
{% highlight string %}
void
ngx_event_cancel_timers(void)
{
    ngx_event_t        *ev;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    sentinel = ngx_event_timer_rbtree.sentinel;

    for ( ;; ) {
        root = ngx_event_timer_rbtree.root;

        if (root == sentinel) {
            return;
        }

        node = ngx_rbtree_min(root, sentinel);

        ev = (ngx_event_t *) ((char *) node - offsetof(ngx_event_t, timer));

        if (!ev->cancelable) {
            return;
        }

        ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                       "event timer cancel: %d: %M",
                       ngx_event_ident(ev->data), ev->timer.key);

        ngx_rbtree_delete(&ngx_event_timer_rbtree, &ev->timer);

#if (NGX_DEBUG)
        ev->timer.left = NULL;
        ev->timer.right = NULL;
        ev->timer.parent = NULL;
#endif

        ev->timer_set = 0;

        ev->handler(ev);
    }
}
{% endhighlight %}
此函数用于取消红黑树中的所有定时器，直到遇到一个不可取消(ev->cancelable为0）的定时器为止。这里注意到，在取消时，并没有将**ev->timeout**置为1，这一点与上面介绍的**ngx_event_expire_timers()**函数是不同的。





<br />
<br />

**[参看]**






<br />
<br />
<br />

