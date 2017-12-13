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


## 1. ngx_process_t数据结构
ngx_process_t代表进程数据结构，主要用于记录Nginx产生的worker进程相关信息。ngx_exec_ctx_t用于记录进程执行时传递的上下文信息
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
    ngx_pid_t           pid;              //代表该进程PID
    int                 status;           //
    ngx_socket_t        channel[2];       //进程的channel，通过socketpair来创建

    ngx_spawn_proc_pt   proc;             //进程的初始化函数，在每次创建完worker进程时调用
    void               *data;             //向进程初始化函数传递的参数
    char               *name;             //进程名称

    unsigned            respawn:1;        //对这些标示，我们下面会做详细介绍
    unsigned            just_spawn:1;     
    unsigned            detached:1;       
    unsigned            exiting:1;        
    unsigned            exited:1;        
} ngx_process_t;


typedef struct {
    char         *path;                 //用于传递可执行文件路径
    char         *name;                 //用于传递要创建的进程的名称
    char *const  *argv;                 //用于传递相关参数
    char *const  *envp;                 //用于传递相关环境变量
} ngx_exec_ctx_t;


#define NGX_MAX_PROCESSES         1024    //定义最多可拥有的进程数目

#define NGX_PROCESS_NORESPAWN     -1
#define NGX_PROCESS_JUST_SPAWN    -2     
#define NGX_PROCESS_RESPAWN       -3     
#define NGX_PROCESS_JUST_RESPAWN  -4     
#define NGX_PROCESS_DETACHED      -5
{% endhighlight %}

**1) ngx_process_t.channel[2]**

nginx master与worker进程之间使用unix套接字进行通信： nginx在创建worker进程前先调用socketpair(int channel[2])，然后将channel[0-1]设置为异步读写方式，并注册event事件，父进程使用channel[0]，子进程使用channel[1]实现双方的通信。

<br />


**2) 创建子进程时用到的标示**
<pre>
#define NGX_PROCESS_NORESPAWN     -1
#define NGX_PROCESS_JUST_SPAWN    -2
#define NGX_PROCESS_RESPAWN       -3
#define NGX_PROCESS_JUST_RESPAWN  -4
#define NGX_PROCESS_DETACHED      -5
</pre>
下面介绍这些标示：

* NGX_PROCESS_NORESPAWN: 子进程退出时，父进程不会再次创建，该标记用在创建"cache loader process"。
<pre>
请参看os/unix/ngx_process_cycle.c:

static void
ngx_start_cache_manager_processes(ngx_cycle_t *cycle, ngx_uint_t respawn)
</pre>

* NGX_PROCESS_JUST_SPAWN: 当```nginx -s reload```时，如果还有未加载的proxy_cache_path，则需要再次创建"cache loader process"加载，并用NGX_PROCESS_JUST_SPAWN给这个进程做记号。防止nginx master向**老的worker进程、老的cache manager进程、老的cache loader进程（如果存在）**发送NGX_CMD_QUIT或SIGQUIT时，误以为我们新创建的"cache loader process"是原来老旧的，而将其错误的杀掉。
{% highlight string %}
请参看 os/unix/ngx_process_cycle.c:

void
ngx_master_process_cycle(ngx_cycle_t *cycle)
{
    ...
      if (ngx_reconfigure) {
            ngx_reconfigure = 0;

            if (ngx_new_binary) {
                ngx_start_worker_processes(cycle, ccf->worker_processes,
                                           NGX_PROCESS_RESPAWN);
                ngx_start_cache_manager_processes(cycle, 0);
                ngx_noaccepting = 0;

                continue;
            }

            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reconfiguring");

            cycle = ngx_init_cycle(cycle);
            if (cycle == NULL) {
                cycle = (ngx_cycle_t *) ngx_cycle;
                continue;
            }

            ngx_cycle = cycle;
            ccf = (ngx_core_conf_t *) ngx_get_conf(cycle->conf_ctx,
                                                   ngx_core_module);
            ngx_start_worker_processes(cycle, ccf->worker_processes,
                                       NGX_PROCESS_JUST_RESPAWN);
            ngx_start_cache_manager_processes(cycle, 1);               //此处指示将其标记为一个新的cache loader process

            /* allow new processes to start */
            ngx_msleep(100);

            live = 1;
            ngx_signal_worker_processes(cycle,
                                        ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
        }
    ...
}
{% endhighlight %}

*  NGX_PROCESS_RESPAWN: 子进程异常退出时，master会重新创建它，如当worker或```cache manager process```异常退出时，父进程会重新创建它。

* NGX_PROCESS_JUST_RESPAWN: 当```nginx -s reload```时，master会向老的```worker进程```，**老的cache manager process，老的cache loader process(如果存在)**发送ngx_write_channel(NGX_CMD_QUIT)(如果失败则发送SIGQUIT信号）。NGX_PROCESS_JUST_RESPAWN用来标记进程数组中哪些是新创建的子进程，而其他的就是属于老的子进程。
{% highlight string %}
请参看 os/unix/ngx_process_cycle.c:

void
ngx_master_process_cycle(ngx_cycle_t *cycle)
{
      ......

      if (ngx_reconfigure) {
            .....

            ngx_start_worker_processes(cycle, ccf->worker_processes,
                                       NGX_PROCESS_JUST_RESPAWN);
            ngx_start_cache_manager_processes(cycle, 1);

            .....

            ngx_signal_worker_processes(cycle,
                                        ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
        }
       .....
}
{% endhighlight %}

* NGX_PROCESS_DETACHED: 热代码替换。
<pre>
请参看os/unix/ngx_process.c：

ngx_pid_t
ngx_execute(ngx_cycle_t *cycle, ngx_exec_ctx_t *ctx)
{
    return ngx_spawn_process(cycle, ngx_execute_proc, ctx, ctx->name,
                             NGX_PROCESS_DETACHED);
}
</pre>





<br />
<br />
**[参看]:**

1. [ngx_master_process_cycle 多进程(一)](http://blog.csdn.net/lengzijian/article/details/7587740)

2. [nginx的进程模型](http://blog.csdn.net/gsnumen/article/details/7979484?reload)

3. [nginx process的respawn和just_spawn 标志](http://kofreestyler.blog.163.com/blog/static/1077907512011215362391/)

<br />
<br />
<br />

