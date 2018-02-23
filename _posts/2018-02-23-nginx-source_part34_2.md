---
layout: post
title: core/ngx_conf_file.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们讲述nginx配置文件相关的一些内容。


<!-- more -->

## 1. 相关静态函数声明

{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>

#define NGX_CONF_BUFFER  4096

// 对指令的解析校验等方面的处理
static ngx_int_t ngx_conf_handler(ngx_conf_t *cf, ngx_int_t last);

// 读取配置文件并进行词法分析
static ngx_int_t ngx_conf_read_token(ngx_conf_t *cf);

// 主要用于nginx配置模块在退出时刷新相应的打开文件
static void ngx_conf_flush_files(ngx_cycle_t *cycle);
{% endhighlight %}

<br />
<br />
**[参看]**

1. [初识nginx——配置解析篇](https://www.cnblogs.com/magicsoar/p/5817734.html)

2. [Nginx 配置项参数解析](http://blog.csdn.net/zhangxiao93/article/details/52979993)

3. [Nginx开发从入门到精通](http://tengine.taobao.org/book/)

4. [Nginx-------配置文件解析ngx_conf_handler](http://blog.csdn.net/jackywgw/article/details/48786429)
<br />
<br />
<br />

