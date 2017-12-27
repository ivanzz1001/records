---
layout: post
title: os/unix/ngx_process_cycle.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

ngx_process_cycle.c源文件主要是定义了nginx中各进程的主循环函数。主要是包括：

* ngx_master_process_cycle()主循环

* ngx_single_process_cycle()主循环

* ngx_worker_process_cycle()主循环

* ngx_cache_manager_process_cycle()主循环

（注：cache manager和cache loader都用ngx_cache_manager_process_cycle()函数作为其主循环函数）



<!-- more -->



## 1. 相关函数声明
如下主要是一些静态函数的声明，我们通过注释的方式简要介绍各函数的功能：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_channel.h>


//1: 批量启动工作进程
static void ngx_start_worker_processes(ngx_cycle_t *cycle, ngx_int_t n,
    ngx_int_t type);

//2: 批量启动cache manager进程
static void ngx_start_cache_manager_processes(ngx_cycle_t *cycle,
    ngx_uint_t respawn);

//3: 传递通道，主要是master创建子进程时，向其他的子进程传递文件描述符，以进行子进程之间的通信
static void ngx_pass_open_channel(ngx_cycle_t *cycle, ngx_channel_t *ch);

//4: 向子进程发送信号
static void ngx_signal_worker_processes(ngx_cycle_t *cycle, int signo);

//5: 回收退出的子进程
static ngx_uint_t ngx_reap_children(ngx_cycle_t *cycle);

//6: 退出master进程
static void ngx_master_process_exit(ngx_cycle_t *cycle);

//7: worker进程工作主循环
static void ngx_worker_process_cycle(ngx_cycle_t *cycle, void *data);

//8: worker进程初始化
static void ngx_worker_process_init(ngx_cycle_t *cycle, ngx_int_t worker);

//9: worker进程退出
static void ngx_worker_process_exit(ngx_cycle_t *cycle);

//10: 通道处理器，主要是处理从channel发送过来的命令
static void ngx_channel_handler(ngx_event_t *ev);

//11: cache manager进程主循环
static void ngx_cache_manager_process_cycle(ngx_cycle_t *cycle, void *data);

//12: cache manager进程事件处理函数
static void ngx_cache_manager_process_handler(ngx_event_t *ev);

//13: cache loader进程事件处理函数
static void ngx_cache_loader_process_handler(ngx_event_t *ev);

{% endhighlight %}




<br />
<br />

**[参看]:**

1. [nginx中cache的设计和实现(一)](http://www.pagefault.info/?p=375&cpage=1)

2. [Nginx缓存机制详解之一缓存管理进程](http://www.it165.net/admin/html/201606/7890.html)





<br />
<br />
<br />

