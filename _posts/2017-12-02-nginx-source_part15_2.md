---
layout: post
title: os/unix/ngx_process.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要分析一下ngx_process.c源文件代码，其主要完成进程的产生、信号的初始化等操作。


<!-- more -->


<br />
<br />


## 1. 相关函数及变量的声明
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_channel.h>


typedef struct {
    int     signo;                   //信号编号
    char   *signame;                 //信号名称（官方名称）
    char   *name;                    //信号名称（nginx中起的名称）
    void  (*handler)(int signo);     //该信号的处理函数
} ngx_signal_t;



static void ngx_execute_proc(ngx_cycle_t *cycle, void *data);    //主要用于exec替换当前进程
static void ngx_signal_handler(int signo);
static void ngx_process_get_status(void);
static void ngx_unlock_mutexes(ngx_pid_t pid);


int              ngx_argc;
char           **ngx_argv;
char           **ngx_os_argv;

ngx_int_t        ngx_process_slot;
ngx_socket_t     ngx_channel;
ngx_int_t        ngx_last_process;
ngx_process_t    ngx_processes[NGX_MAX_PROCESSES];
{% endhighlight %}


上面ngx_signal_t数据类型是对信号相关信息的一个封装。







<br />
<br />
<br />

