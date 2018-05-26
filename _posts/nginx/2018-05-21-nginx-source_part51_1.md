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











<br />
<br />

**[参看]**





<br />
<br />
<br />

