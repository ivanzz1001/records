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
    int                 status;           //用于保存子进程的退出状态
    ngx_socket_t        channel[2];       //进程的channel，通过socketpair来创建

    ngx_spawn_proc_pt   proc;             //进程的初始化函数，在每次创建完worker进程时调用
    void               *data;             //向进程初始化函数传递的参数
    char               *name;             //进程名称

    unsigned            respawn:1;        //对这些标示，我们下面会做详细介绍
    unsigned            just_spawn:1;     
    unsigned            detached:1;       
    unsigned            exiting:1;        //表明该进程处于正在退出状态
    unsigned            exited:1;         //表明该进程已经退出了
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

* **NGX_PROCESS_NORESPAWN:** 子进程退出时，父进程不会再次创建，该标记用在创建"cache loader process"。
<pre>
请参看os/unix/ngx_process_cycle.c:

static void
ngx_start_cache_manager_processes(ngx_cycle_t *cycle, ngx_uint_t respawn)
</pre>

* **NGX_PROCESS_JUST_SPAWN:** 当```nginx -s reload```时，如果还有未加载的proxy_cache_path，则需要再次创建"cache loader process"加载，并用NGX_PROCESS_JUST_SPAWN给这个进程做记号。防止nginx master向**老的worker进程、老的cache manager进程、老的cache loader进程（如果存在）**发送NGX_CMD_QUIT或SIGQUIT时，误以为我们新创建的"cache loader process"是原来老旧的，而将其错误的杀掉。
{% highlight string %}
请参看 os/unix/ngx_process_cycle.c:

void
ngx_master_process_cycle(ngx_cycle_t *cycle)
{
    ...
      if (ngx_reconfigure) {
            ...

            ngx_start_cache_manager_processes(cycle, 1);            //此处指示将其标记为一个新的cache loader process

            /* allow new processes to start */
            ngx_msleep(100);

            live = 1;
            ngx_signal_worker_processes(cycle,
                                        ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
        }
    ...
}
{% endhighlight %}

* **NGX_PROCESS_RESPAWN:** 子进程异常退出时，master会重新创建它，如当worker或```cache manager process```异常退出时，父进程会重新创建它。

* **NGX_PROCESS_JUST_RESPAWN:** 当```nginx -s reload```时，master会向老的```worker进程```，**老的cache manager process，老的cache loader process(如果存在)**发送ngx_write_channel(NGX_CMD_QUIT)(如果失败则发送SIGQUIT信号）。NGX_PROCESS_JUST_RESPAWN用来标记进程数组中哪些是新创建的子进程，而其他的就是属于老的子进程。
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

* **NGX_PROCESS_DETACHED:** 热代码替换(这里通过exec函数族替换当前进程)。
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

**3) ngx_process_t结构体respawn和just_spawn标志**

接着上面2）进行讲解。respawn用于标记进程挂了要不要重启，启动的worker进程都是设置respawn=1的（不管ngx_start_worker_processes()用NGX_PROCESS_RESPAWN 还是 NGX_PROCESS_JUST_RESPAWN)。


如果worker进程的退出返回值是2，即fatal error的话，则不重启了：
<pre>
请参看os/unix/ngx_process.c:

static void
ngx_process_get_status(void)
{
     ...
     
     if (WEXITSTATUS(status) == 2 && ngx_processes[i].respawn) {
            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                          "%s %P exited with fatal code %d "
                          "and cannot be respawned",
                          process, pid, WEXITSTATUS(status));
            ngx_processes[i].respawn = 0;
        }
     .....
}
</pre>

<br />

**关于just_spawn**则要从ngxin配置开始说起：

nginx master收到SIGHUP信号时，ngx_signal_handler()设置ngx_reconfigure=1，然后在master进程循环里，检测到reconfigure为1时，运行ngx_init_cycle()，然后启动新的worker进程：
<pre>
ngx_start_worker_processes(cycle, ccf->worker_processes,
                                       NGX_PROCESS_JUST_RESPAWN);
</pre>
然后对worker进程发送shutdown信号，优雅的关闭旧的worker进程：
<pre>
ngx_signal_worker_processes(cycle,
                                        ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
</pre>
该函数是对所有worker进程进行循环发信号的，所以要用一个标记just_spawn来标记刚刚生成的进程：
<pre>
if (ngx_processes[i].just_spawn) {
        ngx_processes[i].just_spawn = 0;
        continue;
    }
</pre>
上面的```NGX_PROCESS_JUST_RESPAWN```会设置ngx_processes[s].just_spawn=1。


## 2. 相关函数声明
{% highlight string %}
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
{% endhighlight %}

ngx_log_pid这里被定义为ngx_pid， 其(ngx_pid)在os/unix/ngx_process_cycle.c文件中定义，并在nginx初始化时将该全局变量设置为主进程的进程ID。


<pre>
//1: 用于产生一个子进程
ngx_pid_t ngx_spawn_process(ngx_cycle_t *cycle,
    ngx_spawn_proc_pt proc, void *data, char *name, ngx_int_t respawn);

//2: 产生一个子进程并用exec函数族替换该子进程
ngx_pid_t ngx_execute(ngx_cycle_t *cycle, ngx_exec_ctx_t *ctx);

//3: 初始化nginx中相关的信号处理
ngx_int_t ngx_init_signals(ngx_log_t *log);

//4: 设置一些程序的调试点（主要是为了方便在调试时定位一些严重的错误）
void ngx_debug_point(void);
</pre>

在ngx_auto_config.h头文件中，我们有如下定义：
<pre>
#ifndef NGX_HAVE_SCHED_YIELD
#define NGX_HAVE_SCHED_YIELD  1
#endif
</pre>


## 3. 相关变量声明
{% highlight string %}
extern int            ngx_argc;
extern char         **ngx_argv;
extern char         **ngx_os_argv;

extern ngx_pid_t      ngx_pid;
extern ngx_socket_t   ngx_channel;
extern ngx_int_t      ngx_process_slot;
extern ngx_int_t      ngx_last_process;
extern ngx_process_t  ngx_processes[NGX_MAX_PROCESSES];
{% endhighlight %}
如上除```ngx_pid```均定义在os/unix/ngx_process.c文件中：
<pre>
int              ngx_argc;           //保存nginx启动时传递进来的参数数据的个数
char           **ngx_argv;           //通过分配额外的空间保存nginx启动时传递进来的参数（由于某些系统改进程名的需要）
char           **ngx_os_argv;        //保存nginx启动时传递进来的参数（由于某些系统改进程名的需要，其所指向的值可能发生改变）

ngx_int_t        ngx_process_slot;   //
ngx_socket_t     ngx_channel;
ngx_int_t        ngx_last_process;
ngx_process_t    ngx_processes[NGX_MAX_PROCESSES];
</pre>

* ngx_process_slot: 主要用于记录子进程ngx_process_t所表示的环境表存放在数组ngx_processes中的哪一个索引处，其会随着fork()函数自动传递给该子进程。

* ngx_channel: 主要用于记录子进程与父进程进行通信的channel号，其会随着fork()函数自动传递给子进程(保存socketpair()产生的channel[1]，以自动的告诉子进程通过channel[1]与父进程中的channel[0]进行通信）。

* ngx_last_process: 主要用于记录当前ngx_processes数组的最高下标的下一个位置，其也会随着fork()函数自动传递给子进程。例如，目前我们使用了ngx_processes数组的第0，1，3，5，7个位置，那么ngx_last_process则等于8。

* ngx_processes: 当前所有进程的进程环境表。


**说明：**由于改进程名的需要，这里延伸出ngx_argv与ngx_os_argv。对于FreeBSD, NetBSD, OpenBSD等操作系统，由于其原生支持setproctitle()，则可能没有这样的问题。参看：os/unix/ngx_setproctitle.h文件



<br />
<br />
**[参看]:**

1. [ngx_master_process_cycle 多进程(一)](http://blog.csdn.net/lengzijian/article/details/7587740)

2. [nginx的进程模型](http://blog.csdn.net/gsnumen/article/details/7979484?reload)

3. [nginx process的respawn和just_spawn 标志](http://kofreestyler.blog.163.com/blog/static/1077907512011215362391/)

<br />
<br />
<br />

