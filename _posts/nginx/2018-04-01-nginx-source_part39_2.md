---
layout: post
title: core/ngx_file.c源文件分析(1)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx中对所涉及到的文件操作。


<!-- more -->

## 1. 相关变量定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


static ngx_int_t ngx_test_full_name(ngx_str_t *name);


static ngx_atomic_t   temp_number = 0;
ngx_atomic_t         *ngx_temp_number = &temp_number;
ngx_atomic_int_t      ngx_random_number = 123456;
{% endhighlight %}

* 函数```ngx_test_full_name()```用于判断```name```是否为一个绝对路径。

* 变量```temp_number```作为一个临时的占位符空间

* 变量```ngx_temp_number```刚开始指向```temp_number```,而后在初始化event模块时（ngx_event_module_init()函数)，指向一个共享内存空间，在整个Nginx中作为一个临时值使用

* 变量ngx_random_number作为随机值使用，默认初始化值为123456，而在event模块初始化时，被设置为：
{% highlight string %}
ngx_random_number = (tp->msec << 16) + ngx_pid;
{% endhighlight %}


<br />
<br />

**[参考]**

1. [nginx文件结构](https://blog.csdn.net/apelife/article/details/53043275)

2. [Nginx中目录树的遍历](https://blog.csdn.net/weiyuefei/article/details/38313663)

<br />
<br />
<br />

