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

* ```ngx_log_memory_writer()```: 此函数在```NGX_DEBUG```条件下使用,主要用于在调试环境下将buf数据写到内存中。

* ```ngx_log_memory_cleanup()```: 此函数在```NGX_DEBUG```条件下使用，主要用于清除内存buf

* ```ngx_log_memory_buf_t```： 此数据结构在```NGX_DEBUG```条件下使用，用于在内存中保存日志的buf。这是一个循环buffer内存，下面我们详细介绍一下该数据结构中各字段的含义：
{% highlight string %}
#if (NGX_DEBUG)
typedef struct {
    u_char        *start;      //该内存Buff的开始位置
    u_char        *end;        //该内存buff的结束位置
    u_char        *pos;        //实际写日志的其实位置，这是因为在start后会插入一些相应的内存日志的提示信息
    ngx_atomic_t   written;    //当前总共写了多少日志数据
} ngx_log_memory_buf_t;

#endif
{% endhighlight %}



<br />
<br />

**[参考]**

1. [git的使用](https://www.yiibai.com/git/git-quick-start.html)

2. [bit book](https://git-scm.com/book/zh/v2)
<br />
<br />
<br />

