---
layout: post
title: core/ngx_module.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要讲述一下nginx module的实现。


<!-- more -->


## 1. 相关函数声明及变量定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Maxim Dounin
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_MAX_DYNAMIC_MODULES  128


static ngx_uint_t ngx_module_index(ngx_cycle_t *cycle);
static ngx_uint_t ngx_module_ctx_index(ngx_cycle_t *cycle, ngx_uint_t type,
    ngx_uint_t index);


ngx_uint_t         ngx_max_module;
static ngx_uint_t  ngx_modules_n;
{% endhighlight %}




<br />
<br />

**[参看]**

1. [nginx-module-t数据结构](https://blog.csdn.net/u014082714/article/details/46125135)

<br />
<br />
<br />

