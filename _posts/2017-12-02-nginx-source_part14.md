---
layout: post
title: os/unix/ngx_linux_sendfile_chain.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要分析一下ngx_linux_sendfile_chain.c源文件。

<!-- more -->


<br />
<br />


## 1. 相关函数声明
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


static ssize_t ngx_linux_sendfile(ngx_connection_t *c, ngx_buf_t *file,
    size_t size);

#if (NGX_THREADS)
#include <ngx_thread_pool.h>

#if !(NGX_HAVE_SENDFILE64)
#error sendfile64() is required!
#endif

static ngx_int_t ngx_linux_sendfile_thread(ngx_connection_t *c, ngx_buf_t *file,
    size_t size, size_t *sent);
static void ngx_linux_sendfile_thread_handler(void *data, ngx_log_t *log);
#endif
{% endhighlight %}










<br />
<br />
<br />

