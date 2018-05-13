---
layout: post
title: core/ngx_log.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们主要讲述一下nginx中日志的相关实现。


<!-- more -->


## 1. 相关静态函数声明
{% highlight strig %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


static char *ngx_error_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_log_set_levels(ngx_conf_t *cf, ngx_log_t *log);
static void ngx_log_insert(ngx_log_t *log, ngx_log_t *new_log);


#if (NGX_DEBUG)

static void ngx_log_memory_writer(ngx_log_t *log, ngx_uint_t level,
    u_char *buf, size_t len);
static void ngx_log_memory_cleanup(void *data);


typedef struct {
    u_char        *start;
    u_char        *end;
    u_char        *pos;
    ngx_atomic_t   written;
} ngx_log_memory_buf_t;

#endif
{% endhighlight %}

下面简要介绍一下这些函数：

* ```ngx_error_log()```: 解析error_log指令时相应的处理函数

* ```ngx_log_set_levels()```: 主要是为了设置日志级别

* ```ngx_log_insert()```: 将new_log插入到log链表中，log链表是按```log->log_level```从大到小的顺序排列的。




<br />
<br />

**[参考]**

1. [git的使用](https://www.yiibai.com/git/git-quick-start.html)

2. [bit book](https://git-scm.com/book/zh/v2)
<br />
<br />
<br />

