---
layout: post
title: core/ngx_cycle.c源文件分析(1)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本文我们讲述一下Nginx cycle运行上下文相关的实现。


<!-- more -->


## 1. 相关静态函数声明
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


//销毁nginx cycle相关的内存池
static void ngx_destroy_cycle_pools(ngx_conf_t *conf);

//初始化共享内存
static ngx_int_t ngx_init_zone_pool(ngx_cycle_t *cycle,
    ngx_shm_zone_t *shm_zone);


//nginx.lock是nginx为了保证只有一个服务运行的锁文件，这里该锁文件是否存在以及是否有权限创建
static ngx_int_t ngx_test_lockfile(u_char *file, ngx_log_t *log);

//清除老的nginx cycle上下文
static void ngx_clean_old_cycles(ngx_event_t *ev);
{% endhighlight %}


## 2. 相关变量定义
{% highlight string %}
volatile ngx_cycle_t  *ngx_cycle;
ngx_array_t            ngx_old_cycles;

static ngx_pool_t     *ngx_temp_pool;
static ngx_event_t     ngx_cleaner_event;

ngx_uint_t             ngx_test_config;
ngx_uint_t             ngx_dump_config;
ngx_uint_t             ngx_quiet_mode;


/* STUB NAME */
static ngx_connection_t  dumb;
/* STUB */
{% endhighlight %}
下面对各个变量做一个简单的说明：

* **ngx_cycle**: 这里```ngx_cycle```作为一个全局变量指向nginx当前运行的上下文环境。因为在运行过程中，上下文可能会经常发生变动，因此这里用```volatile```修饰。

* **ngx_old_cycles**: 保存所有原来到的nginx上下文对象

* **ngx_temp_pool**: nginx的一个临时内存池。这是因为在nginx升级过程中，有一些老的```ngx_cycle_t *```信息需要保存，这需要空间，因此这里开辟一个临时的内存池来存储这些信息。

* **ngx_cleaner_event**: 这里对于```ngx_old_cycles```的清理会采用事件机制来完成，因此这里定义一个cleaner event。

* **ngx_test_config**： 是否是对nginx配置文件进行测试

* **ngx_dump_config**： 是否要dump出nginx配置文件

* **ngx_quiet_mode**: 在测试nginx配置文件时，抑制非错误信息的输出

* **dumb**: 这里之所以用一个桩```dumb```，是因为这里的```ngx_event_t```设计主要是针对nginx网络事件的，每个网络事件都关联着一个```ngx_connection_t```。这里虽然```dumb```是用于定时器事件，但是还是会把该定时器事件看成是一个网络事件来处理，因此要设立一个```STUB```来表明这只是一个桩，并不是一个网络事件。





<br />
<br />
<br />

