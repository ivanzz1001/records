---
layout: post
title: os/unix/ngx_process_cycle.h源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本文主要介绍ngx_process_cycle.h头文件，其主要是定义了nginx主进程与工作进程交互的相关命令、主进程循环函数的声明等。
<!-- more -->


<br />
<br />

## 1. 相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PROCESS_CYCLE_H_INCLUDED_
#define _NGX_PROCESS_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_CMD_OPEN_CHANNEL   1
#define NGX_CMD_CLOSE_CHANNEL  2
#define NGX_CMD_QUIT           3
#define NGX_CMD_TERMINATE      4
#define NGX_CMD_REOPEN         5


#define NGX_PROCESS_SINGLE     0
#define NGX_PROCESS_MASTER     1
#define NGX_PROCESS_SIGNALLER  2
#define NGX_PROCESS_WORKER     3
#define NGX_PROCESS_HELPER     4
{% endhighlight %}

### 1.1 nginx进程间管道通信
nginx中各个进程之间是通过管道的方式来进行通信的，如下图所示：

![ngx-processes-comm](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_process_comm.jpg)

上面我们用三种不同颜色的线连接了各个进程，虽然都是一个channel，但是传递这些通道的方式可能有些不同：

* **红色**： nginx master在创建子进程时通过socketpair()创建了一对匿名管道，通过fork()自动将相应的文件描述符传递给子进程的

* **绿色**: 绿色之间的通道是在创建子进程时通过ngx_pass_open_channel()传递的。例如在创建worker-3时，nginx master分别通过ch1[0]、ch2[0]通道将ch3[0]文件描述符传递给worker-1和worker-2，这样worker-1与worker-2就能通过相应的文件描述符和worker-3进行通信了。

* **紫色**: 紫色之间的通道其实也是通过fork()函数自动传递的。例如在创建worker-2时，因为master与worker-1之间的ch1[0]文件描述符是已经存在的，因此新创建的worker-2进程自动的复制了该文件描述符，从而可以自动的获得worker-2到worker-1之间的通道ch1_p[0]（此文件描述符复制于ch1[0])。


我们了解了上述的通信流程，接下来我们介绍一下各相关命令：

* **NGX_CMD_OPEN_CHANNEL**: 打开一个通道，在创建子进程时向其他子进程传递文件描述符。如上面的绿线指示的通道，详见ngx_pass_open_channel()。

* **NGX_CMD_CLOSE_CHANNEL**: 当一个子进程退出时，就需要向其他子进程发送消息关闭对应的通道。例如worker-2进程退出，则master进程需要向worker-1和worker-3进程发送消息，告知关闭ch2_p[0]。请参看：ngx_reap_children()及ngx_channel_handler()

* **NGX_CMD_QUIT**： 用于通知子进程以优雅的方式退出。

* **NGX_CMD_TERMINATE**: 用于通知子进程快速退出

* **NGX_CMD_REOPEN**: 用于通知子进程进行相应的日志回滚。

关于这些命令，我们后续在讲到对应的函数时还会再进一步讲解。

### 1.2 nginx进程类型
<pre>
#define NGX_PROCESS_SINGLE     0
#define NGX_PROCESS_MASTER     1
#define NGX_PROCESS_SIGNALLER  2
#define NGX_PROCESS_WORKER     3
#define NGX_PROCESS_HELPER     4
</pre>



<br />
<br />
<br />

