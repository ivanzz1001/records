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

1) 如果收到本信号的当前进程为以后台方式工作的master进程，则master进程中将ngx_noaccept置为1，然后向worker进程发送shutdown信号，停止接收外部连接,优雅的停止worker进程(但是master进程本身并不退出，这与shutdown是不同的)，参看os/unix/ngx_process_cycle.c:
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

本标志fcntl及open的O_ASYNC标志等效，用于使能signal-driven I/O: 当此文件描述符变得可读或可写时就会产生相应的信号。本特征只针对终端、伪终端、socket、pipe和FIFO有效。（pipe及FIFO从Linux 2.6版本开始才起作用）参看：1. [SIGIO用法](http://blog.csdn.net/leamonl/article/details/4726480) 2. [ioctl的使用](https://baike.baidu.com/item/ioctl/6392403)


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

    unsigned            respawn:1;                   //表明该进程退出后要不要再进行重启
    unsigned            just_spawn:1;                //表明该进程是否为刚刚建立的进程，以此作为与原来旧进程的区隔
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
下面我们再列出各种类型子进程相应flag的设置：

* **NGX_PROCESS_NORESPAWN:**
<pre>
ngx_processes[s].respawn = 0;
ngx_processes[s].just_spawn = 0;
ngx_processes[s].detached = 0;
</pre>

* **NGX_PROCESS_JUST_SPAWN:**
<pre>
ngx_processes[s].respawn = 0;
ngx_processes[s].just_spawn = 1;
ngx_processes[s].detached = 0;
</pre>

* **NGX_PROCESS_RESPAWN:**
<pre>
ngx_processes[s].respawn = 1;
ngx_processes[s].just_spawn = 0;
ngx_processes[s].detached = 0;
</pre>

* **NGX_PROCESS_JUST_RESPAWN:**
<pre>
ngx_processes[s].respawn = 1;
ngx_processes[s].just_spawn = 1;
ngx_processes[s].detached = 0;
</pre>

* **NGX_PROCESS_DETACHED:**
<pre>
ngx_processes[s].respawn = 0;
ngx_processes[s].just_spawn = 0;
ngx_processes[s].detached = 1;
</pre>

## 4. 函数ngx_execute()与ngx_execute_proc()
{% highlight string %}
ngx_pid_t
ngx_execute(ngx_cycle_t *cycle, ngx_exec_ctx_t *ctx)
{
    return ngx_spawn_process(cycle, ngx_execute_proc, ctx, ctx->name,
                             NGX_PROCESS_DETACHED);
}


static void
ngx_execute_proc(ngx_cycle_t *cycle, void *data)
{
    ngx_exec_ctx_t  *ctx = data;

    if (execve(ctx->path, ctx->argv, ctx->envp) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "execve() failed while executing %s \"%s\"",
                      ctx->name, ctx->path);
    }

    exit(1);
}
{% endhighlight %}

这里ngx_execute()主要是用于产生一个子进程，然后调用exec函数族执行一个可执行文件，以替换当前子进程。主要用于nginx的热升级。

exec函数族主要有如下函数：
{% highlight string %}
#include <unistd.h>

extern char **environ;

int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg,
          ..., char * const envp[]);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execvpe(const char *file, char *const argv[],
           char *const envp[]);
{% endhighlight %}

参看：[linux系统编程之进程（五）：exec系列函数](https://www.cnblogs.com/mickole/p/3187409.html)


## 5. 函数ngx_init_signals()
{% highlight string %}
ngx_int_t
ngx_init_signals(ngx_log_t *log)
{
    ngx_signal_t      *sig;
    struct sigaction   sa;

    for (sig = signals; sig->signo != 0; sig++) {
        ngx_memzero(&sa, sizeof(struct sigaction));
        sa.sa_handler = sig->handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(sig->signo, &sa, NULL) == -1) {
#if (NGX_VALGRIND)
            ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                          "sigaction(%s) failed, ignored", sig->signame);
#else
            ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
                          "sigaction(%s) failed", sig->signame);
            return NGX_ERROR;
#endif
        }
    }

    return NGX_OK;
}
{% endhighlight %}
这里主要是为signals数组中的每一个信号通过sigaction()函数装载对应的处理函数。sigaction()函数原型如下：
{% highlight string %}
#include <signal.h>

int sigaction(int signum, const struct sigaction *act,
             struct sigaction *oldact);

struct sigaction {
   void     (*sa_handler)(int);
   void     (*sa_sigaction)(int, siginfo_t *, void *);
   sigset_t   sa_mask;      //指定在信号处理过程中，哪些信号应当被阻塞。缺省情况下当前信号本身被阻塞
   int        sa_flags;
   void     (*sa_restorer)(void);
};
{% endhighlight %}
可以用sigaction()函数装载除```SIGKILL```和```SIGSTOP```之外的信号的处理函数。一般通过fork()函数产生的子进程会继承父进程的信号处理。而在执行execve()函数期间，先前被处理的信号会恢复默认；原来被忽略的信号维持不变。


参看： [linux下的struct sigaction](http://blog.csdn.net/a511244213/article/details/45146987)

下面给出一个打印```sa_flags```所有值的程序test.c:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
     printf("SA_NOCLDSTOP:%u\n",SA_NOCLDSTOP);
     printf("SA_NOCLDWAIT:%u\n",SA_NOCLDWAIT);
     printf("SA_NODEFER:%u\n",SA_NODEFER);
     printf("SA_ONSTACK:%u\n",SA_ONSTACK);
     printf("SA_RESETHAND:%u\n",SA_RESETHAND);
     printf("SA_RESTART:%u\n",SA_RESTART);
     printf("SA_SIGINFO:%u\n",SA_SIGINFO);

     return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test
SA_NOCLDSTOP:1
SA_NOCLDWAIT:2
SA_NODEFER:1073741824
SA_ONSTACK:134217728
SA_RESETHAND:2147483648
SA_RESTART:268435456
SA_SIGINFO:4
</pre>


## 6. 函数ngx_signal_handler()
{% highlight string %}
void
ngx_signal_handler(int signo)
{
    char            *action;
    ngx_int_t        ignore;
    ngx_err_t        err;
    ngx_signal_t    *sig;

    ignore = 0;

    err = ngx_errno;

    for (sig = signals; sig->signo != 0; sig++) {
        if (sig->signo == signo) {
            break;
        }
    }

    ngx_time_sigsafe_update();

    action = "";

    switch (ngx_process) {

    case NGX_PROCESS_MASTER:
    case NGX_PROCESS_SINGLE:
        switch (signo) {

        case ngx_signal_value(NGX_SHUTDOWN_SIGNAL):
            ngx_quit = 1;
            action = ", shutting down";
            break;

        case ngx_signal_value(NGX_TERMINATE_SIGNAL):
        case SIGINT:
            ngx_terminate = 1;
            action = ", exiting";
            break;

        case ngx_signal_value(NGX_NOACCEPT_SIGNAL):
            if (ngx_daemonized) {
                ngx_noaccept = 1;
                action = ", stop accepting connections";
            }
            break;

        case ngx_signal_value(NGX_RECONFIGURE_SIGNAL):
            ngx_reconfigure = 1;
            action = ", reconfiguring";
            break;

        case ngx_signal_value(NGX_REOPEN_SIGNAL):
            ngx_reopen = 1;
            action = ", reopening logs";
            break;

        case ngx_signal_value(NGX_CHANGEBIN_SIGNAL):
            if (getppid() > 1 || ngx_new_binary > 0) {

                /*
                 * Ignore the signal in the new binary if its parent is
                 * not the init process, i.e. the old binary's process
                 * is still running.  Or ignore the signal in the old binary's
                 * process if the new binary's process is already running.
                 */

                action = ", ignoring";
                ignore = 1;
                break;
            }

            ngx_change_binary = 1;
            action = ", changing binary";
            break;

        case SIGALRM:
            ngx_sigalrm = 1;
            break;

        case SIGIO:
            ngx_sigio = 1;
            break;

        case SIGCHLD:
            ngx_reap = 1;
            break;
        }

        break;

    case NGX_PROCESS_WORKER:
    case NGX_PROCESS_HELPER:
        switch (signo) {

        case ngx_signal_value(NGX_NOACCEPT_SIGNAL):
            if (!ngx_daemonized) {
                break;
            }
            ngx_debug_quit = 1;
        case ngx_signal_value(NGX_SHUTDOWN_SIGNAL):
            ngx_quit = 1;
            action = ", shutting down";
            break;

        case ngx_signal_value(NGX_TERMINATE_SIGNAL):
        case SIGINT:
            ngx_terminate = 1;
            action = ", exiting";
            break;

        case ngx_signal_value(NGX_REOPEN_SIGNAL):
            ngx_reopen = 1;
            action = ", reopening logs";
            break;

        case ngx_signal_value(NGX_RECONFIGURE_SIGNAL):
        case ngx_signal_value(NGX_CHANGEBIN_SIGNAL):
        case SIGIO:
            action = ", ignoring";
            break;
        }

        break;
    }

    ngx_log_error(NGX_LOG_NOTICE, ngx_cycle->log, 0,
                  "signal %d (%s) received%s", signo, sig->signame, action);

    if (ignore) {
        ngx_log_error(NGX_LOG_CRIT, ngx_cycle->log, 0,
                      "the changing binary signal is ignored: "
                      "you should shutdown or terminate "
                      "before either old or new binary's process");
    }

    if (signo == SIGCHLD) {
        ngx_process_get_status();
    }

    ngx_set_errno(err);
}
{% endhighlight %}

这里我们先介绍一下整体的信号处理流程。关于具体的某一个信号的处理，我们后面会进行介绍。关于```NGX_CHANGEBIN_SIGNAL```热替换信号，我们这里做一个简单说明： 从shell控制台运行的nginx，其master进程的父进程ID(ppid)为```1```。
{% highlight string %}
void
ngx_signal_handler(int signo)
{
    //1: 保留当前ngx_errno

    //2: 从signals全局数组中找到该信号

    //3: 更新nginx中缓存的一些时间

    //4: 对信号按进程类型、信号类型来处理
    switch(ngx_process)
    {
       case NGX_PROCESS_MASTER:
       case NGX_PROCESS_SINGLE:
           switch(signo)
           {
           }
          break;
       case NGX_PROCESS_WORKER:
       case NGX_PROCESS_HELPER:
           switch(signo)
           {
           }
         break;
    }
  
    //5: 对于SIGCHLD信号，调用ngx_process_get_status(void)获得子进程退出状态

    //6: 恢复ngx_errno
}
{% endhighlight %}



## 7. 函数ngx_process_get_status()
{% highlight string %}
static void
ngx_process_get_status(void)
{
    int              status;
    char            *process;
    ngx_pid_t        pid;
    ngx_err_t        err;
    ngx_int_t        i;
    ngx_uint_t       one;

    one = 0;

    for ( ;; ) {
        pid = waitpid(-1, &status, WNOHANG);

        if (pid == 0) {
            return;
        }

        if (pid == -1) {
            err = ngx_errno;

            if (err == NGX_EINTR) {
                continue;
            }

            if (err == NGX_ECHILD && one) {
                return;
            }

            /*
             * Solaris always calls the signal handler for each exited process
             * despite waitpid() may be already called for this process.
             *
             * When several processes exit at the same time FreeBSD may
             * erroneously call the signal handler for exited process
             * despite waitpid() may be already called for this process.
             */

            if (err == NGX_ECHILD) {
                ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, err,
                              "waitpid() failed");
                return;
            }

            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, err,
                          "waitpid() failed");
            return;
        }


        one = 1;
        process = "unknown process";

        for (i = 0; i < ngx_last_process; i++) {
            if (ngx_processes[i].pid == pid) {
                ngx_processes[i].status = status;
                ngx_processes[i].exited = 1;
                process = ngx_processes[i].name;
                break;
            }
        }

        if (WTERMSIG(status)) {
#ifdef WCOREDUMP
            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                          "%s %P exited on signal %d%s",
                          process, pid, WTERMSIG(status),
                          WCOREDUMP(status) ? " (core dumped)" : "");
#else
            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                          "%s %P exited on signal %d",
                          process, pid, WTERMSIG(status));
#endif

        } else {
            ngx_log_error(NGX_LOG_NOTICE, ngx_cycle->log, 0,
                          "%s %P exited with code %d",
                          process, pid, WEXITSTATUS(status));
        }

        if (WEXITSTATUS(status) == 2 && ngx_processes[i].respawn) {
            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                          "%s %P exited with fatal code %d "
                          "and cannot be respawned",
                          process, pid, WEXITSTATUS(status));
            ngx_processes[i].respawn = 0;
        }

        ngx_unlock_mutexes(pid);
    }
}
{% endhighlight %}

在介绍本函数之前，请先参看[Linux wait函数族介绍](https://ivanzz1001.github.io/records/post/nginx/2017/12/02/nginx-source_part15_appendix)。

### 7.1 waitpid()返回-1情况的处理
这里我们主要讲述一下waitpid()函数对返回-1这种情况的处理。我们先来看如下程序test.c：
{% highlight string %}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

static void sighandler(int sig)
{
   
}
int main(int argc,char *argv[])
{
   pid_t pid;

   signal(SIGCHLD,SIG_IGN);

   pid = fork();

   if(pid == -1)
     return -1;
   else if(pid == 0)
   {
       printf("child 1\n");
       exit(0);
   }
   printf("parent_1\n");
   sleep(1);               //wait child1 exit

   signal(SIGCHLD,sighandler);
   pid = waitpid(-1,NULL,0);
   printf("pid: %d\n",pid);
   if(pid == -1)
     printf("errno: %s\n",strerror(errno));
   

   pid = fork();
   if(pid == -1)
       return -1;
   else if(pid == 0)
   {
      printf("child 2\n");
      exit(0);
   }
   
  printf("parent_2\n");
  sleep(1);               //wait child2 exit
  pid = waitpid(-1,NULL,0);
  printf("pid: %d\n",pid);
  
  if(pid == -1)
     printf("errno: %s\n",strerror(errno));
 

  pid = waitpid(-1,NULL,0);
  printf("pid: %d\n",pid);
  if(pid == -1)
    printf("errno: %s\n",strerror(errno));

  return 0x0;  
}
{% endhighlight %}

编译运行：
<pre>
root@ubuntu:/home/ivan1001/Share/test-src# gcc -o test test.c
root@ubuntu:/home/ivan1001/Share/test-src# ./test
parent_1
child 1
pid: -1
errno: No child processes
parent_2
child 2
pid: 4972
pid: -1
errno: No child processes
</pre>
从上面我们可以看到，如果我们将对于某一个子进程的```SIGCHLD```信号设置处理函数为```SIG_IGN```，则调用waitpid()函数会返回-1，并提示errno为```ECHILD```。如果并没有任何可等待的子进程退出，函数waitpid()也会返回-1。

接下来我们来看nginx中对pid为-1的情况下的处理：
{% highlight string %}
if (pid == -1) {
    err = ngx_errno;

    if (err == NGX_EINTR) {
        continue;
    }

    if (err == NGX_ECHILD && one) {
        return;
    }

    /*
     * Solaris always calls the signal handler for each exited process
     * despite waitpid() may be already called for this process.
     *
     * When several processes exit at the same time FreeBSD may
     * erroneously call the signal handler for exited process
     * despite waitpid() may be already called for this process.
     */

    if (err == NGX_ECHILD) {
        ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, err,
                      "waitpid() failed");
        return;
    }

    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, err,
                  "waitpid() failed");
    return;
}
{% endhighlight %}

这里假设主进程有3个子进程，如果这三个子进程分别在不同时间退出，则最后一个子进程退出时是有可能会发生:
<pre>
if (err == NGX_ECHILD && one) {
    return;
}
</pre>
如果这三个子进程同时退出的话，就会产生三个```SIGCHLD```。由于在信号处理过程中，缺省情况下对于与本信号相同的信号会进行阻塞，因此这三个```SIGCHLD```的信号处理函数会在上一个调用完成之后依次被调用。而在第一次调用信号处理函数时，waitpid()函数就会将3个子进程的相关数据结构进行回收，那后面两次调用waitpid()函数则有可能会出现：
<pre>
/*
 * Solaris always calls the signal handler for each exited process
 * despite waitpid() may be already called for this process.
 *
 * When several processes exit at the same time FreeBSD may
 * erroneously call the signal handler for exited process
 * despite waitpid() may be already called for this process.
 */

if (err == NGX_ECHILD) {
    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, err,
                  "waitpid() failed");
    return;
}

ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, err,
              "waitpid() failed");
return;
</pre>


### 7.2 WTERMSIG()打印退出信号码
这里通过WTERMSIG()宏定义打印出到底是什么信号导致的子进程退出。

### 7.3 子进程严重错误退出
在子进程出现严重错误的情况下退出，此时即使此子进程```respawn```标志为1，也不再重新创建此子进程：
{% highlight string %}
if (WEXITSTATUS(status) == 2 && ngx_processes[i].respawn) {
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                  "%s %P exited with fatal code %d "
                  "and cannot be respawned",
                  process, pid, WEXITSTATUS(status));
    ngx_processes[i].respawn = 0;
}
{% endhighlight %}

### 7.4 解除已终止子进程锁
因为在nginx中大量使用到了```共享内存```，这里面就会涉及到相应的锁。因此在子进程退出时，会将相应的锁进行释放。关于具体的释放方法，我们后面会进行介绍。
<pre>
ngx_unlock_mutexes(pid);
</pre>

## 8. 函数ngx_unlock_mutexes()
{% highlight string %}
static void
ngx_unlock_mutexes(ngx_pid_t pid)
{
    ngx_uint_t        i;
    ngx_shm_zone_t   *shm_zone;
    ngx_list_part_t  *part;
    ngx_slab_pool_t  *sp;

    /*
     * unlock the accept mutex if the abnormally exited process
     * held it
     */

    if (ngx_accept_mutex_ptr) {
        (void) ngx_shmtx_force_unlock(&ngx_accept_mutex, pid);
    }

    /*
     * unlock shared memory mutexes if held by the abnormally exited
     * process
     */

    part = (ngx_list_part_t *) &ngx_cycle->shared_memory.part;
    shm_zone = part->elts;

    for (i = 0; /* void */ ; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }
            part = part->next;
            shm_zone = part->elts;
            i = 0;
        }

        sp = (ngx_slab_pool_t *) shm_zone[i].shm.addr;

        if (ngx_shmtx_force_unlock(&sp->mutex, pid)) {
            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                          "shared memory zone \"%V\" was locked by %P",
                          &shm_zone[i].shm.name, pid);
        }
    }
}
{% endhighlight %}

这里释放两种锁：

* **ngx_accept_mutex**: nginx各子进程抢占互斥锁，然后accept外部进来的连接。如果当前终止的子进程持有了锁，则需要在退出时释放该锁。

* **共享内存锁**: nginx各进程间还涉及到大量的共享内存。在终止的子进程结束时也应该释放相应的锁。这里各共享内存段是以ngx_list_t链表的形式来管理的。下面我们简要给出该链表结构，在后面的章节我们会对ngx_list_t结构做详细的介绍。

![ngx-shm-list](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_shm_list.jpg)

elt[i]中的每一个数据结构都是如下类型(src/core/ngx_cycle.h)：
{% highlight string %}
struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};
{% endhighlight %}


关于```ngx_shmtx_force_unlock()```函数，我们后面在讲解nginx共享内存时会再做详细的介绍。


## 9. 函数ngx_debug_point()
{% highlight string %}
void
ngx_debug_point(void)
{
    ngx_core_conf_t  *ccf;

    ccf = (ngx_core_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_core_module);

    switch (ccf->debug_points) {

    case NGX_DEBUG_POINTS_STOP:
        raise(SIGSTOP);
        break;

    case NGX_DEBUG_POINTS_ABORT:
        ngx_abort();
    }
}
{% endhighlight %}

设置一些程序的调试点，主要是为了方便在调试时定位一些严重的错误。
{% highlight string %}
#include <signal.h>

int raise(int sig);
{% endhighlight %}
raise()函数用于向调用进程或线程发送一个信号。并且如果raise()发送一个信号导致对应的Handler被调用的话，则raise()函数会等到handler被调用完成才会返回。

这里```raise(SIGSTOP)```会停止当前调用进程或线程的执行。看如下程序test.c:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main(int argc,char *argv[])
{
    printf("before calling raise()\n");

    raise(SIGSTOP);

    printf("after calling raise()\n");

    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.c
# ./test
before calling raise()

[1]+  Stopped                 ./test
# ps -ef | grep test | grep -v grep
ivan1001  6852  6828  0 05:03 pts/18   00:00:00 ./test
# kill -CONT 6852
after calling raise()
[1]+  Done                    ./test
</pre>

可以看到```raise(SIGSTOP)```暂停了当前调用进程的执行。


## 10. 函数ngx_os_signal_process()
{% highlight string %}
ngx_int_t
ngx_os_signal_process(ngx_cycle_t *cycle, char *name, ngx_pid_t pid)
{
    ngx_signal_t  *sig;

    for (sig = signals; sig->signo != 0; sig++) {
        if (ngx_strcmp(name, sig->name) == 0) {
            if (kill(pid, sig->signo) != -1) {
                return 0;
            }

            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "kill(%P, %d) failed", pid, sig->signo);
        }
    }

    return 1;
}
{% endhighlight %}

本函数主要用于向某一指定的进程发送指定名称的信号。发送成功返回0，发送失败返回1。


<br />
<br />
**[参考]:**

1. [nginx 进程的类型](http://blog.csdn.net/benbendy1984/article/details/6008581)

2. [初识nginx——配置解析篇](https://www.cnblogs.com/magicsoar/p/5817734.html)

3. [Nginx源码分析 - 主流程篇 - 解析配置文件](http://blog.csdn.net/initphp/article/details/51911189)

4. [Linux C实践(1)：不可忽略或捕捉的信号—SIGSTOP和SIGKILL](http://blog.csdn.net/madpointer/article/details/13091705)


<br />
<br />
<br />

