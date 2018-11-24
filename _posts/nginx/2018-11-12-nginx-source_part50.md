---
layout: post
title: core/ngx_parse.c(h)文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本章我们介绍一下ngx_parse.c(h)文件，其主要用于解析如下三方面：

* size单位解析（如KB,MB等）

* offset偏移解析(如KB、MB、GB等）

* 时间单位解析

<!-- more -->


## 1. ngx_parsh.h头文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PARSE_H_INCLUDED_
#define _NGX_PARSE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


ssize_t ngx_parse_size(ngx_str_t *line);
off_t ngx_parse_offset(ngx_str_t *line);
ngx_int_t ngx_parse_time(ngx_str_t *line, ngx_uint_t is_sec);


#endif /* _NGX_PARSE_H_INCLUDED_ */

{% endhighlight %}
这里主要是声明了三个函数，分别用于size解析、offset解析、时间解析。


## 2. ngx_parse.c源文件




<br />
<br />

**[参看]**



<br />
<br />
<br />

