---
layout: post
title: core/ngx_cycle.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们讲述一下nginx运行的一个总控型数据结构及相关操作函数。

<!-- more -->


## 1. 相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2
{% endhighlight %}

下面我们对上面几个宏定义进行简单的说明：

* **NGX_CYCLE_POOL_SIZE**: 这里定义nginx cycle所关联的pool大小，默认值```NGX_DEFAULT_POOL_SIZE```，即16KB（该变量定义在core/palloc.h头文件中)

* **NGX_DEBUG_POINTS_STOP**: 定义程序在执行到一些关键错误点时，产生```SIGSTOP```信号。

* **NGX_DEBUG_POINTS_ABORT**: 定义程序在执行到一些关键错误点时，执行abort()函数。

<pre>
可以通过在nginx.conf配置文件使用debug_points指令，指定在一些关键错误点处的行为。
</pre>



<br />
<br />
<br />

