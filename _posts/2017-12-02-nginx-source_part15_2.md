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

* **NGX_RECONFIGURE_SIGNAL:** 这里用作nginx重新读取配置文件，对应的实际信号为SIGHUP。

* **NGX_REOPEN_SIGNAL:** 这里用作nginx的日志文件回滚，对应的实际信号为：
<pre>
#if (NGX_LINUXTHREADS)
#define NGX_REOPEN_SIGNAL        INFO
#else
#define NGX_REOPEN_SIGNAL        USR1
#endif
</pre>

* **NGX_NOACCEPT_SIGNAL:** 对应的实际信号为SIGWINCH，作用请看如下：

1) 如果收到本信号的当前进程为以后台方式工作的master进程，则master进程中将ngx_noaccept置为1，然后向worker进程发送shutdown信号，停止接收外部连接优雅的停止worker进程，参看os/unix/ngx_process_cycle.c:
{% highlight string %}
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
{% endhighlight %}

2) 如果当前nginx是以单进程方式工作，且正以后台话方式运行，收到此信号其实不会受到任何影响

3) 如果收到本信号的worker进程(NGX_PROCESS_WORKER)或者辅助进程（NGX_PROCESS_HELPER），且当前nginx是以daemonized方式工作，则会优雅的停止该worker进程或辅助进程，同时退出时执行ngx_debug_point()，暗示非正常退出。参看os/unix/ngx_process.c:
{% highlight string %}
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
{% endhighlight string %}


* **NGX_TERMINATE_SIGNAL:** 对应的实际信号为TERM。

1) nginx master收到此信号时，会向所有子进程发送TERM信号，通知子进程退出（子进程快速退出）。并在所有子进程退出后，master进程也退出。

2) 单进程方式工作的nginx也会快速退出

3) 工作进程或者辅助进程收到此信号时，会自行快速退出。


* **NGX_SHUTDOWN_SIGNAL:** 对应的实际信号为SIGQUIT。

1） nginx master收到此信号时，会向所有子进程发送QUIT信号，通知子进程在处理完成目前的任务之后优雅的退出。并在所有子进程退出后，master进程也退出

2）单进程方式工作的nginx收到此信号执行相应的退出动作。

3) 工作进程或者辅助进程收到此信号，会自行优雅的退出。


* **NGX_CHANGEBIN_SIGNAL:** 这里用作Nginx平滑升级，对应的实际信号为：
<pre>
#if (NGX_LINUXTHREADS)
#define NGX_CHANGEBIN_SIGNAL     XCPU
#else
#define NGX_CHANGEBIN_SIGNAL     USR2
#endif
</pre>

* **SIGALRM:** 主要是用于nginx master收到TERM信号退出时的一个计时操作

* **SIGINT:** 同上面NGX_TERMINATE_SIGNAL

* **SIGGIO:** 暂时只是设置ngx_sigio为1，不会显著产生什么作用。参看[Nginx 源代码笔记 - SIGIO](http://ialloc.org/posts/2014/08/03/ngx-notes-sigio/)

* **SIGCHLD:** 子进程退出时会向父进程返回相应的信号，此时父进程会做相应的清理操作。

* **SIGSYS:** 信号被忽略

* **SIGPIPE:** 信号被忽略


## 3. 函数ngx_spawn_process()
{% highlight string %}
ngx_pid_t
ngx_spawn_process(ngx_cycle_t *cycle, ngx_spawn_proc_pt proc, void *data,
    char *name, ngx_int_t respawn)
{
    u_long     on;
    ngx_pid_t  pid;
    ngx_int_t  s;

    if (respawn >= 0) {
        s = respawn;

    } else {
        for (s = 0; s < ngx_last_process; s++) {
            if (ngx_processes[s].pid == -1) {
                break;
            }
        }

        if (s == NGX_MAX_PROCESSES) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, 0,
                          "no more than %d processes can be spawned",
                          NGX_MAX_PROCESSES);
            return NGX_INVALID_PID;
        }
    }


    if (respawn != NGX_PROCESS_DETACHED) {

        /* Solaris 9 still has no AF_LOCAL */

        if (socketpair(AF_UNIX, SOCK_STREAM, 0, ngx_processes[s].channel) == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "socketpair() failed while spawning \"%s\"", name);
            return NGX_INVALID_PID;
        }

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, cycle->log, 0,
                       "channel %d:%d",
                       ngx_processes[s].channel[0],
                       ngx_processes[s].channel[1]);

        if (ngx_nonblocking(ngx_processes[s].channel[0]) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          ngx_nonblocking_n " failed while spawning \"%s\"",
                          name);
            ngx_close_channel(ngx_processes[s].channel, cycle->log);
            return NGX_INVALID_PID;
        }

        if (ngx_nonblocking(ngx_processes[s].channel[1]) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          ngx_nonblocking_n " failed while spawning \"%s\"",
                          name);
            ngx_close_channel(ngx_processes[s].channel, cycle->log);
            return NGX_INVALID_PID;
        }

        on = 1;
        if (ioctl(ngx_processes[s].channel[0], FIOASYNC, &on) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "ioctl(FIOASYNC) failed while spawning \"%s\"", name);
            ngx_close_channel(ngx_processes[s].channel, cycle->log);
            return NGX_INVALID_PID;
        }

        if (fcntl(ngx_processes[s].channel[0], F_SETOWN, ngx_pid) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "fcntl(F_SETOWN) failed while spawning \"%s\"", name);
            ngx_close_channel(ngx_processes[s].channel, cycle->log);
            return NGX_INVALID_PID;
        }

        if (fcntl(ngx_processes[s].channel[0], F_SETFD, FD_CLOEXEC) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "fcntl(FD_CLOEXEC) failed while spawning \"%s\"",
                           name);
            ngx_close_channel(ngx_processes[s].channel, cycle->log);
            return NGX_INVALID_PID;
        }

        if (fcntl(ngx_processes[s].channel[1], F_SETFD, FD_CLOEXEC) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "fcntl(FD_CLOEXEC) failed while spawning \"%s\"",
                           name);
            ngx_close_channel(ngx_processes[s].channel, cycle->log);
            return NGX_INVALID_PID;
        }

        ngx_channel = ngx_processes[s].channel[1];

    } else {
        ngx_processes[s].channel[0] = -1;
        ngx_processes[s].channel[1] = -1;
    }

    ngx_process_slot = s;


    pid = fork();

    switch (pid) {

    case -1:
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "fork() failed while spawning \"%s\"", name);
        ngx_close_channel(ngx_processes[s].channel, cycle->log);
        return NGX_INVALID_PID;

    case 0:
        ngx_pid = ngx_getpid();
        proc(cycle, data);
        break;

    default:
        break;
    }

    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "start %s %P", name, pid);

    ngx_processes[s].pid = pid;
    ngx_processes[s].exited = 0;

    if (respawn >= 0) {
        return pid;
    }

    ngx_processes[s].proc = proc;
    ngx_processes[s].data = data;
    ngx_processes[s].name = name;
    ngx_processes[s].exiting = 0;

    switch (respawn) {

    case NGX_PROCESS_NORESPAWN:
        ngx_processes[s].respawn = 0;
        ngx_processes[s].just_spawn = 0;
        ngx_processes[s].detached = 0;
        break;

    case NGX_PROCESS_JUST_SPAWN:
        ngx_processes[s].respawn = 0;
        ngx_processes[s].just_spawn = 1;
        ngx_processes[s].detached = 0;
        break;

    case NGX_PROCESS_RESPAWN:
        ngx_processes[s].respawn = 1;
        ngx_processes[s].just_spawn = 0;
        ngx_processes[s].detached = 0;
        break;

    case NGX_PROCESS_JUST_RESPAWN:
        ngx_processes[s].respawn = 1;
        ngx_processes[s].just_spawn = 1;
        ngx_processes[s].detached = 0;
        break;

    case NGX_PROCESS_DETACHED:
        ngx_processes[s].respawn = 0;
        ngx_processes[s].just_spawn = 0;
        ngx_processes[s].detached = 1;
        break;
    }

    if (s == ngx_last_process) {
        ngx_last_process++;
    }

    return pid;
}
{% endhighlight %}
本函数用于产生一个新的子进程，并将产生的子进程信息登记在全局的ngx_processes表中。如下是ngx_processes进程表的一个结构：

![ngx-processes](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_processes.jpg)


**1) 查找新进程的在ngx_processes数组中的存放位置**
{% highlight string %}
ngx_pid_t
ngx_spawn_process(ngx_cycle_t *cycle, ngx_spawn_proc_pt proc, void *data,
    char *name, ngx_int_t respawn)
{
    ...

    if (respawn >= 0) {
        s = respawn;

    } else {
        for (s = 0; s < ngx_last_process; s++) {
            if (ngx_processes[s].pid == -1) {
                break;
            }
        }

        if (s == NGX_MAX_PROCESSES) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, 0,
                          "no more than %d processes can be spawned",
                          NGX_MAX_PROCESSES);
            return NGX_INVALID_PID;
        }
    }
   ....
}
{% endhighlight %}

这里当respawn参数大于等于0时，直接将respawn索引处存放新生成进程的ngx_process_t；否则在进程表中查找到一个合适的位置保存。

**2) 生成父子进程通信的channel**

如果将要生成的子进程为```NGX_PROCESS_DETACHED```类型，因为其最后是要通过exec函数族替换子进程的，所以这里不需要通过channel来通信，直接将channel[0-1]置为-1；如果为其他类型的子进程，则按如下方式产生并设置channel:

* socketpair()产生unix域的流式socket

* 设置channel[0]非阻塞(此fd在父进程中使用）

* 设置channel[1]非阻塞(此fd通过后续的fork自动传递给子进程，在子进程中使用）

* 设置channel[0]为FIOASYNC

本标志fcntl及open的O_ASYNC标志等效，用于使能signal-driven I/O: 当此文件描述符变得可读或可写时就会产生相应的信号。本特征只针对终端、伪终端、socket、pipe和FIFO有效。（pipe及FIFO从Linux 2.6版本开始才起作用）参看：1. [SIGIO](http://blog.csdn.net/leamonl/article/details/4726480) 2. [ioctl](https://baike.baidu.com/item/ioctl/6392403)


* 设置channel[0]的OWNER为当前进程的pid

* 设置channel[0]属性为FD_CLOEXEC，这用于指示在执行exec函数族时关闭对应的文件描述符.

* 设置channel[1]属性为FD_CLOEXEC，这用于指示在执行exec函数族时关闭对应的文件描述符.

参看: [执行时关闭标识位 FD_CLOEXEC 的作用](https://www.cnblogs.com/sunrisezhang/p/4113500.html)

<br />

**3）产生子进程**
{% highlight string %}
pid = fork();

switch (pid) {

case -1:
    ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                  "fork() failed while spawning \"%s\"", name);
    ngx_close_channel(ngx_processes[s].channel, cycle->log);
    return NGX_INVALID_PID;

case 0:
    ngx_pid = ngx_getpid();
    proc(cycle, data);
    break;

default:
    break;
}
{% endhighlight %}
上面执行fork()函数产生子进程，子进程执行相应的回调函数。

**4) 在进程表中登记相应的子进程信息**

这里再次列出ngx_process_t数据结构，方便对照：
{% highlight string %}
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
{% endhighlight %}

这里注意一下当```respawn>=0```,这种情况一般是重启某一个子进程，此时一般设置完pid之后，就可以直接返回，其他的信息直接复用上一次的即可：
{% highlight string %}
ngx_processes[s].pid = pid;
ngx_processes[s].exited = 0;

if (respawn >= 0) {
    return pid;
}
{% endhighlight %}





<br />
<br />
**[参考]:**

1. [nginx 进程的类型](http://blog.csdn.net/benbendy1984/article/details/6008581)

2. [初识nginx——配置解析篇](https://www.cnblogs.com/magicsoar/p/5817734.html)

3. [Nginx源码分析 - 主流程篇 - 解析配置文件](http://blog.csdn.net/initphp/article/details/51911189)

<br />
<br />
<br />

