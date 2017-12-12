---
layout: post
title: os/unix/ngx_process.h源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本文主要介绍一下ngx_process.h头文件，其主要定义了进程产生、信号初始化等进程环境相关的函数原型。

<!-- more -->


<br />
<br />

## 1. os/unix/ngx_process.h头文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PROCESS_H_INCLUDED_
#define _NGX_PROCESS_H_INCLUDED_


#include <ngx_setaffinity.h>
#include <ngx_setproctitle.h>


typedef pid_t       ngx_pid_t;

#define NGX_INVALID_PID  -1

typedef void (*ngx_spawn_proc_pt) (ngx_cycle_t *cycle, void *data);

typedef struct {
    ngx_pid_t           pid;
    int                 status;
    ngx_socket_t        channel[2];

    ngx_spawn_proc_pt   proc;
    void               *data;
    char               *name;

    unsigned            respawn:1;
    unsigned            just_spawn:1;
    unsigned            detached:1;
    unsigned            exiting:1;
    unsigned            exited:1;
} ngx_process_t;


typedef struct {
    char         *path;
    char         *name;
    char *const  *argv;
    char *const  *envp;
} ngx_exec_ctx_t;


#define NGX_MAX_PROCESSES         1024

#define NGX_PROCESS_NORESPAWN     -1
#define NGX_PROCESS_JUST_SPAWN    -2
#define NGX_PROCESS_RESPAWN       -3
#define NGX_PROCESS_JUST_RESPAWN  -4
#define NGX_PROCESS_DETACHED      -5


#define ngx_getpid   getpid

#ifndef ngx_log_pid
#define ngx_log_pid  ngx_pid
#endif


ngx_pid_t ngx_spawn_process(ngx_cycle_t *cycle,
    ngx_spawn_proc_pt proc, void *data, char *name, ngx_int_t respawn);
ngx_pid_t ngx_execute(ngx_cycle_t *cycle, ngx_exec_ctx_t *ctx);
ngx_int_t ngx_init_signals(ngx_log_t *log);
void ngx_debug_point(void);


#if (NGX_HAVE_SCHED_YIELD)
#define ngx_sched_yield()  sched_yield()
#else
#define ngx_sched_yield()  usleep(1)
#endif


extern int            ngx_argc;
extern char         **ngx_argv;
extern char         **ngx_os_argv;

extern ngx_pid_t      ngx_pid;
extern ngx_socket_t   ngx_channel;
extern ngx_int_t      ngx_process_slot;
extern ngx_int_t      ngx_last_process;
extern ngx_process_t  ngx_processes[NGX_MAX_PROCESSES];


#endif /* _NGX_PROCESS_H_INCLUDED_ */
{% endhighlight %}


## 2. ngx_process_t数据结构
ngx_process_t代表进程数据结构，主要用于记录Nginx产生的worker进程相关信息。ngx_exec_ctx_t用于记录进程执行时传递的上下文信息
{% highlight string %}
typedef pid_t       ngx_pid_t;

#define NGX_INVALID_PID  -1

typedef void (*ngx_spawn_proc_pt) (ngx_cycle_t *cycle, void *data);

typedef struct {
    ngx_pid_t           pid;              //代表该进程PID
    int                 status;           //
    ngx_socket_t        channel[2];       //进程的channel，通过socketpair来创建

    ngx_spawn_proc_pt   proc;             //进程的初始化函数，在每次创建完worker进程时调用
    void               *data;             //向进程初始化函数传递的参数
    char               *name;             //进程名称

    unsigned            respawn:1;        //重新创建的进程
    unsigned            just_spawn:1;     //第一次创建的进程
    unsigned            detached:1;       //分离的
    unsigned            exiting:1;        //正在退出的
    unsigned            exited:1;         //已经退出的
} ngx_process_t;


typedef struct {
    char         *path;                 //用于传递可执行文件路径
    char         *name;                 //用于传递要创建的进程的名称
    char *const  *argv;                 //用于传递相关参数
    char *const  *envp;                 //用于传递相关环境变量
} ngx_exec_ctx_t;


#define NGX_MAX_PROCESSES         1024    //定义最多可拥有的进程数目

#define NGX_PROCESS_NORESPAWN     -1
#define NGX_PROCESS_JUST_SPAWN    -2     //第一次创建的进程
#define NGX_PROCESS_RESPAWN       -3     //重新创建的进程
#define NGX_PROCESS_JUST_RESPAWN  -4     //第一次重新创建的进程
#define NGX_PROCESS_DETACHED      -5
{% endhighlight %}







<br />
<br />
**[参看]:**

1. [ngx_master_process_cycle 多进程(一)](http://blog.csdn.net/lengzijian/article/details/7587740)

2. [nginx的进程模型](http://blog.csdn.net/gsnumen/article/details/7979484?reload)

<br />
<br />
<br />

