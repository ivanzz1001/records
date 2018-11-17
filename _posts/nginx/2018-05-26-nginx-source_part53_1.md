---
layout: post
title: core/ngx_queue.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章讲述一下nginx中队列的实现。


<!-- more -->


## 1. ngx_queue_t数据结构
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef _NGX_QUEUE_H_INCLUDED_
#define _NGX_QUEUE_H_INCLUDED_


typedef struct ngx_queue_s  ngx_queue_t;

struct ngx_queue_s {
    ngx_queue_t  *prev;
    ngx_queue_t  *next;
};
{% endhighlight %}

这里nginx中队列的实现与我们通常的做法有些不同。通常情况下，我们队列的每一个节点包含```指针元素```与```数据元素```两个部分，但是在这里我们看到却根本没有数据元素。这里nginx是反其道而行之， 将queue作为数据节点的一部分来实现队列。下面给出```nginx_queue_t```使用时的一个整体样例：

![ngx-queue-t](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_queue.jpg)

上面通过```queue```的地址，减去对应的偏移地址，就可以求得元素的首地址。


## 2. nginx队列的基本操作
{% highlight string %}
#define ngx_queue_init(q)                                                     \
    (q)->prev = q;                                                            \
    (q)->next = q


#define ngx_queue_empty(h)                                                    \
    (h == (h)->prev)


#define ngx_queue_insert_head(h, x)                                           \
    (x)->next = (h)->next;                                                    \
    (x)->next->prev = x;                                                      \
    (x)->prev = h;                                                            \
    (h)->next = x


#define ngx_queue_insert_after   ngx_queue_insert_head


#define ngx_queue_insert_tail(h, x)                                           \
    (x)->prev = (h)->prev;                                                    \
    (x)->prev->next = x;                                                      \
    (x)->next = h;                                                            \
    (h)->prev = x


#define ngx_queue_head(h)                                                     \
    (h)->next


#define ngx_queue_last(h)                                                     \
    (h)->prev


#define ngx_queue_sentinel(h)                                                 \
    (h)


#define ngx_queue_next(q)                                                     \
    (q)->next


#define ngx_queue_prev(q)                                                     \
    (q)->prev


#if (NGX_DEBUG)

#define ngx_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next;                                              \
    (x)->prev = NULL;                                                         \
    (x)->next = NULL

#else

#define ngx_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next

#endif


#define ngx_queue_split(h, q, n)                                              \
    (n)->prev = (h)->prev;                                                    \
    (n)->prev->next = n;                                                      \
    (n)->next = q;                                                            \
    (h)->prev = (q)->prev;                                                    \
    (h)->prev->next = h;                                                      \
    (q)->prev = n;


#define ngx_queue_add(h, n)                                                   \
    (h)->prev->next = (n)->next;                                              \
    (n)->next->prev = (h)->prev;                                              \
    (h)->prev = (n)->prev;                                                    \
    (h)->prev->next = h;


#define ngx_queue_data(q, type, link)                                         \
    (type *) ((u_char *) q - offsetof(type, link))
{% endhighlight %}
上面操作较为简单，我们只介绍一下如下几个:

* ```ngx_queue_split```: 本宏定义用于将一个队列从q节点位置分割成两个队列， ```h```与```n```分别为这两个新队列的头。注意，这里```q```与```h```不能指向相同的节点。


![ngx-queue-split](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_queue_split.jpg)


* ```ngx_queue_add```: 本宏定义用户队列```h```的尾部，插入队列```n```。


* ```ngx_queue_data```: 用于求数据部分的首地址


## 3. 相关函数声明
{% highlight string %}
//1) 用于找出一个队列的中间元素。这里可以分成两种情况， 如果队列的总元素个数为奇数， 则直接返回最中间的那个元素；
//  如果队列总的元素个数为偶数， 则返回该队列第二部分的第一个元素。例如总共有4个元素， 则1、2为第一部分， 3、4为
//  第二部分
ngx_queue_t *ngx_queue_middle(ngx_queue_t *queue);


//) 对队列中的元素进行排序
void ngx_queue_sort(ngx_queue_t *queue,
    ngx_int_t (*cmp)(const ngx_queue_t *, const ngx_queue_t *));


#endif /* _NGX_QUEUE_H_INCLUDED_ */
{% endhighlight %}






<br />
<br />

**[参看]**





<br />
<br />
<br />

