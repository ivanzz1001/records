---
layout: post
title: os/unix/ngx_channel.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要介绍一下nginx中ngx_channel，其主要用于master进程与worker进程之间的通信。

<!-- more -->


## 1. os/unix/ngx_channel.h头文件

头文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CHANNEL_H_INCLUDED_
#define _NGX_CHANNEL_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


typedef struct {
    ngx_uint_t  command;
    ngx_pid_t   pid;
    ngx_int_t   slot;
    ngx_fd_t    fd;
} ngx_channel_t;


ngx_int_t ngx_write_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size,
    ngx_log_t *log);
ngx_int_t ngx_read_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size,
    ngx_log_t *log);
ngx_int_t ngx_add_channel_event(ngx_cycle_t *cycle, ngx_fd_t fd,
    ngx_int_t event, ngx_event_handler_pt handler);
void ngx_close_channel(ngx_fd_t *fd, ngx_log_t *log);


#endif /* _NGX_CHANNEL_H_INCLUDED_ */
{% endhighlight %}









<br />
<br />

**[参看]:**

1. [nginx进程间的通信](http://blog.csdn.net/walkerkalr/article/details/38237147)




<br />
<br />
<br />

