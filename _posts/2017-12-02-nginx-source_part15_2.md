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



static void ngx_execute_proc(ngx_cycle_t *cycle, void *data);    
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


上面ngx_signal_t数据类型是对信号相关信息的一个封装。而对于声明的如下几个静态函数：

<pre>
//1: 主要用于exec执行另外一个程序替换当前进程
static void ngx_execute_proc(ngx_cycle_t *cycle, void *data);   

//2: 信号处理函数
static void ngx_signal_handler(int signo);                       

//3: 获得一个子进程的状态
static void ngx_process_get_status(void);                        //

//4: 用于释放共享内存锁
static void ngx_unlock_mutexes(ngx_pid_t pid);
</pre>

对于所声明的全局变量：

* ngx_argc: 保存nginx启动时传递进来的参数数据的个数

* ngx_argv: 由于某些操作系统后续更改进程名的需要，用ngx_argv来备份nginx启动时传递进来的参数。

* ngx_os_argv: 由于某些操作系统后续更改进程名的需要，这里用ngx_os_argv来记录更改后的进程名（可能具有相应的格式要求）

* ngx_process_slot: 主要用于记录子进程ngx_process_t所表示的环境表存放在数组ngx_processes中的哪一个索引处，其会随着fork()函数自动传递给该子进程。

* ngx_channel: 主要用于记录子进程与父进程进行通信的channel号，其会随着fork()函数自动传递给子进程。

* ngx_last_process: 主要用于记录当前ngx_processes数组的最高下标的下一个位置，其也会随着fork()函数自动传递给子进程。例如，目前我们使用了ngx_processes数组的第0，1，3，5，7个位置，那么ngx_last_process则等于8。

* ngx_processes: 当前所有进程的进程环境表。



## 2. nginx中处理的所有信号
{% highlight string %}
ngx_signal_t  signals[] = {
    { ngx_signal_value(NGX_RECONFIGURE_SIGNAL),
      "SIG" ngx_value(NGX_RECONFIGURE_SIGNAL),
      "reload",
      ngx_signal_handler },

    { ngx_signal_value(NGX_REOPEN_SIGNAL),
      "SIG" ngx_value(NGX_REOPEN_SIGNAL),
      "reopen",
      ngx_signal_handler },

    { ngx_signal_value(NGX_NOACCEPT_SIGNAL),
      "SIG" ngx_value(NGX_NOACCEPT_SIGNAL),
      "",
      ngx_signal_handler },

    { ngx_signal_value(NGX_TERMINATE_SIGNAL),
      "SIG" ngx_value(NGX_TERMINATE_SIGNAL),
      "stop",
      ngx_signal_handler },

    { ngx_signal_value(NGX_SHUTDOWN_SIGNAL),
      "SIG" ngx_value(NGX_SHUTDOWN_SIGNAL),
      "quit",
      ngx_signal_handler },

    { ngx_signal_value(NGX_CHANGEBIN_SIGNAL),
      "SIG" ngx_value(NGX_CHANGEBIN_SIGNAL),
      "",
      ngx_signal_handler },

    { SIGALRM, "SIGALRM", "", ngx_signal_handler },

    { SIGINT, "SIGINT", "", ngx_signal_handler },

    { SIGIO, "SIGIO", "", ngx_signal_handler },

    { SIGCHLD, "SIGCHLD", "", ngx_signal_handler },

    { SIGSYS, "SIGSYS, SIG_IGN", "", SIG_IGN },

    { SIGPIPE, "SIGPIPE, SIG_IGN", "", SIG_IGN },

    { 0, NULL, "", NULL }
};
{% endhighlight %}

这里signals数组定义了nginx中要处理的所有信号：

* NGX_RECONFIGURE_SIGNAL: 这里用作nginx重新读取配置文件，对应的实际信号为SIGHUP。

* NGX_REOPEN_SIGNAL： 这里用作nginx的日志文件回滚，对应的实际信号为：
<pre>
#if (NGX_LINUXTHREADS)
#define NGX_REOPEN_SIGNAL        INFO
#else
#define NGX_REOPEN_SIGNAL        USR1
#endif
</pre>

* NGX_NOACCEPT_SIGNAL: 对应的实际信号为SIGWINCH，作用请看如下：

1) 如果收到本信号的当前进程为以后台方式工作的master进程，则master进程中将ngx_noaccept置为1，然后向worker进程发送shutdown信号，停止接收外部连接优雅的停止worker进程，参看os/unix/ngx_process_cycle.c:
<pre>
void
ngx_master_process_cycle(ngx_cycle_t *cycle)
{
   ....
    if (ngx_noaccept) {
            ngx_noaccept = 0;
            ngx_noaccepting = 1;
            ngx_signal_worker_processes(cycle,
                                        ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
        }
   ...
}

</pre>

2) 如果当前nginx是以单进程方式工作，且正以后台话方式运行，收到此信号其实不会受到任何影响

3) 如果收到本信号的worker进程(NGX_PROCESS_WORKER)或者辅助进程（NGX_PROCESS_HELPER），且当前nginx是以daemonized方式工作，则会优雅的停止该worker进程或辅助进程，同时退出时执行ngx_debug_point()，暗示非正常退出。参看os/unix/ngx_process.c:
<pre>
case NGX_PROCESS_WORKER:
void
ngx_signal_handler(int signo)
{
    ....

    case NGX_PROCESS_WORKER:
    case NGX_PROCESS_HELPER:
        switch (signo) {

        case ngx_signal_value(NGX_NOACCEPT_SIGNAL):
            if (!ngx_daemonized) {
                break;
            }
            ngx_debug_quit = 1;                       //注意此处没有break
        case ngx_signal_value(NGX_SHUTDOWN_SIGNAL):
            ngx_quit = 1;
            action = ", shutting down";
            break;

        ...
       }

    ....
}

</pre>








<br />
<br />
**[参考]:**

1. [nginx 进程的类型](http://blog.csdn.net/benbendy1984/article/details/6008581)

2. [初识nginx——配置解析篇](https://www.cnblogs.com/magicsoar/p/5817734.html)


<br />
<br />
<br />

