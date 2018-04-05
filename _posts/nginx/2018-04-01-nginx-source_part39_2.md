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




<br />
<br />

**[参考]**

1. [nginx文件结构](https://blog.csdn.net/apelife/article/details/53043275)

2. [Nginx中目录树的遍历](https://blog.csdn.net/weiyuefei/article/details/38313663)

<br />
<br />
<br />

