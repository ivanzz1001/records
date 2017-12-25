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




<br />
<br />
<br />

