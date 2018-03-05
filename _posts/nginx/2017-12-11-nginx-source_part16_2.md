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

//12: cache manager进程事件处理函数(这里为处理定时器超时事件）
static void ngx_cache_manager_process_handler(ngx_event_t *ev);

//13: cache loader进程事件处理函数（这里为处理定时器超时事件）
static void ngx_cache_loader_process_handler(ngx_event_t *ev);

{% endhighlight %}


## 2. 相关变量定义
{% highlight string %}
ngx_uint_t    ngx_process;
ngx_uint_t    ngx_worker;
ngx_pid_t     ngx_pid;

sig_atomic_t  ngx_reap;
sig_atomic_t  ngx_sigio;
sig_atomic_t  ngx_sigalrm;
sig_atomic_t  ngx_terminate;
sig_atomic_t  ngx_quit;
sig_atomic_t  ngx_debug_quit;
ngx_uint_t    ngx_exiting;
sig_atomic_t  ngx_reconfigure;
sig_atomic_t  ngx_reopen;

sig_atomic_t  ngx_change_binary;
ngx_pid_t     ngx_new_binary;
ngx_uint_t    ngx_inherited;
ngx_uint_t    ngx_daemonized;

sig_atomic_t  ngx_noaccept;
ngx_uint_t    ngx_noaccepting;
ngx_uint_t    ngx_restart;


static u_char  master_process[] = "master process";


static ngx_cache_manager_ctx_t  ngx_cache_manager_ctx = {
    ngx_cache_manager_process_handler, "cache manager process", 0
};

static ngx_cache_manager_ctx_t  ngx_cache_loader_ctx = {
    ngx_cache_loader_process_handler, "cache loader process", 60000
};


static ngx_cycle_t      ngx_exit_cycle;
static ngx_log_t        ngx_exit_log;
static ngx_open_file_t  ngx_exit_log_file;
{% endhighlight %}

前面```ngx_process```到```ngx_noaccept```间的这些变量我们都在头文件中讲述过，这里不再赘述。这里只讲述一下剩余的几个变量：

* **ngx_noaccepting**: 本变量是与```ngx_noaccept```变量搭配一起使用的。因为在master优雅的停止子进程的时候，需要一定的时间，这时会用到此变量表示```正在停止接收```这一状态.

* **ngx_restart**: 本变量是与```ngx_noaccept```变量搭配一起使用的。因为在热升级的时候，新创建的进程的父进程可能已经不是本master进程了，则此时要重启原来的子进程。

* **master_process**: master进程的名称

* **ngx_cache_manager_ctx**: 缓存管理器上下文

* **ngx_cache_loader_ctx**: 缓存加载器上下文

* **ngx_exit_cycle**: 此静态变量主要是为了保存相应信息，使得在进程退出时，信号处理器仍可以用该静态变量

* **ngx_exit_log**: 同上

* **ngx_exit_log_file**: 同上

## 2. 函数ngx_master_process_cycle()
{% highlight string %}
void
ngx_master_process_cycle(ngx_cycle_t *cycle)
{
    char              *title;
    u_char            *p;
    size_t             size;
    ngx_int_t          i;
    ngx_uint_t         n, sigio;
    sigset_t           set;
    struct itimerval   itv;
    ngx_uint_t         live;
    ngx_msec_t         delay;
    ngx_listening_t   *ls;
    ngx_core_conf_t   *ccf;

    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGIO);
    sigaddset(&set, SIGINT);
    sigaddset(&set, ngx_signal_value(NGX_RECONFIGURE_SIGNAL));
    sigaddset(&set, ngx_signal_value(NGX_REOPEN_SIGNAL));
    sigaddset(&set, ngx_signal_value(NGX_NOACCEPT_SIGNAL));
    sigaddset(&set, ngx_signal_value(NGX_TERMINATE_SIGNAL));
    sigaddset(&set, ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
    sigaddset(&set, ngx_signal_value(NGX_CHANGEBIN_SIGNAL));

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "sigprocmask() failed");
    }

    sigemptyset(&set);


    size = sizeof(master_process);

    for (i = 0; i < ngx_argc; i++) {
        size += ngx_strlen(ngx_argv[i]) + 1;
    }

    title = ngx_pnalloc(cycle->pool, size);
    if (title == NULL) {
        /* fatal */
        exit(2);
    }

    p = ngx_cpymem(title, master_process, sizeof(master_process) - 1);
    for (i = 0; i < ngx_argc; i++) {
        *p++ = ' ';
        p = ngx_cpystrn(p, (u_char *) ngx_argv[i], size);
    }

    ngx_setproctitle(title);


    ccf = (ngx_core_conf_t *) ngx_get_conf(cycle->conf_ctx, ngx_core_module);

    ngx_start_worker_processes(cycle, ccf->worker_processes,
                               NGX_PROCESS_RESPAWN);
    ngx_start_cache_manager_processes(cycle, 0);

    ngx_new_binary = 0;
    delay = 0;
    sigio = 0;
    live = 1;

    for ( ;; ) {
        if (delay) {
            if (ngx_sigalrm) {
                sigio = 0;
                delay *= 2;
                ngx_sigalrm = 0;
            }

            ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                           "termination cycle: %M", delay);

            itv.it_interval.tv_sec = 0;
            itv.it_interval.tv_usec = 0;
            itv.it_value.tv_sec = delay / 1000;
            itv.it_value.tv_usec = (delay % 1000 ) * 1000;

            if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                              "setitimer() failed");
            }
        }

        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, cycle->log, 0, "sigsuspend");

        sigsuspend(&set);

        ngx_time_update();

        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                       "wake up, sigio %i", sigio);

        if (ngx_reap) {
            ngx_reap = 0;
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, cycle->log, 0, "reap children");

            live = ngx_reap_children(cycle);
        }

        if (!live && (ngx_terminate || ngx_quit)) {
            ngx_master_process_exit(cycle);
        }

        if (ngx_terminate) {
            if (delay == 0) {
                delay = 50;
            }

            if (sigio) {
                sigio--;
                continue;
            }

            sigio = ccf->worker_processes + 2 /* cache processes */;

            if (delay > 1000) {
                ngx_signal_worker_processes(cycle, SIGKILL);
            } else {
                ngx_signal_worker_processes(cycle,
                                       ngx_signal_value(NGX_TERMINATE_SIGNAL));
            }

            continue;
        }

        if (ngx_quit) {
            ngx_signal_worker_processes(cycle,
                                        ngx_signal_value(NGX_SHUTDOWN_SIGNAL));

            ls = cycle->listening.elts;
            for (n = 0; n < cycle->listening.nelts; n++) {
                if (ngx_close_socket(ls[n].fd) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                                  ngx_close_socket_n " %V failed",
                                  &ls[n].addr_text);
                }
            }
            cycle->listening.nelts = 0;

            continue;
        }

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
            ngx_start_cache_manager_processes(cycle, 1);

            /* allow new processes to start */
            ngx_msleep(100);

            live = 1;
            ngx_signal_worker_processes(cycle,
                                        ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
        }

        if (ngx_restart) {
            ngx_restart = 0;
            ngx_start_worker_processes(cycle, ccf->worker_processes,
                                       NGX_PROCESS_RESPAWN);
            ngx_start_cache_manager_processes(cycle, 0);
            live = 1;
        }

        if (ngx_reopen) {
            ngx_reopen = 0;
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reopening logs");
            ngx_reopen_files(cycle, ccf->user);
            ngx_signal_worker_processes(cycle,
                                        ngx_signal_value(NGX_REOPEN_SIGNAL));
        }

        if (ngx_change_binary) {
            ngx_change_binary = 0;
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "changing binary");
            ngx_new_binary = ngx_exec_new_binary(cycle, ngx_argv);
        }

        if (ngx_noaccept) {
            ngx_noaccept = 0;
            ngx_noaccepting = 1;
            ngx_signal_worker_processes(cycle,
                                        ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
        }
    }
}
{% endhighlight %}

这里会分成```master进程初始化```以及```master进程主循环```两个部分来进行讲解。

### 2.1 master进程初始化

**1) master进程信号屏蔽**

关于```sigprocmask()```函数请先参看: [ngx_process_cycle源码分析-附录](https://ivanzz1001.github.io/records/post/nginx/2017/12/11/nginx-source_part16_appendix)
这里主要是屏蔽一下10个信号：

* SIGCHLD

* SIGALRM

* SIGIO

* SIGINT

* NGX_RECONFIGURE_SIGNAL

* NGX_REOPEN_SIGNAL

* NGX_NOACCEPT_SIGNAL

* NGX_TERMINATE_SIGNAL

* NGX_SHUTDOWN_SIGNAL

* NGX_CHANGEBIN_SIGNAL

可以看到，与ngx_process.c中signals数组中的信号一致，只是少了```SIGSYS```与```SIGPIPE```。这是因为这两个信号的处理函数都是```SIG_IGN```, sigsuspend()并不会受到所忽略的信号的影响。

**2) 设置master进程的标题**
{% highlight string %}
size = sizeof(master_process);

for (i = 0; i < ngx_argc; i++) {
    size += ngx_strlen(ngx_argv[i]) + 1;
}

title = ngx_pnalloc(cycle->pool, size);
if (title == NULL) {
    /* fatal */
    exit(2);
}

p = ngx_cpymem(title, master_process, sizeof(master_process) - 1);
for (i = 0; i < ngx_argc; i++) {
    *p++ = ' ';
    p = ngx_cpystrn(p, (u_char *) ngx_argv[i], size);
}

ngx_setproctitle(title);
{% endhighlight %}
这里首先分配一定的空间用于存放title,然后将```nginx process```字符串与nginx启动参数拼接，最后调用ngx_setproctitle()函数设置进程title。最后大概设置成如下样式：
<pre>
nginx: master process /usr/local/nginx/nginx
</pre>


**3) 启动worker进程**
{% highlight string %}
ngx_start_worker_processes(cycle, ccf->worker_processes,
                               NGX_PROCESS_RESPAWN);
{% endhighlight %}
关于ngx_start_worker_processes()函数我们会在下面进行讲解。这里只注意到一个参数```NGX_PROCESS_RESPAWN```，表示此进程退出后需要重启。

**4) 启动cache manager进程**
{% highlight string %}
ngx_start_cache_manager_processes(cycle, 0);
{% endhighlight %}
关于ngx_start_cache_manager_processes()函数我们会在下面进行讲解。这里注意到传入的值为0。

### 2.2 master主循环

在讲解主循环之前，请先参看附录[ngx_process_cycle源码分析-附录](https://ivanzz1001.github.io/records/post/nginx/2017/12/11/nginx-source_part16_appendix)中关于setitimer()的讲解。

下面我们分几个小的部分来进行讲解：

**1) nginx进程terminate结束**
{% highlight string %}
for ( ;; ) {
        if (delay) {
            if (ngx_sigalrm) {
                sigio = 0;
                delay *= 2;
                ngx_sigalrm = 0;
            }

            ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                           "termination cycle: %M", delay);

            itv.it_interval.tv_sec = 0;
            itv.it_interval.tv_usec = 0;
            itv.it_value.tv_sec = delay / 1000;
            itv.it_value.tv_usec = (delay % 1000 ) * 1000;

            if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                              "setitimer() failed");
            }
        }

        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, cycle->log, 0, "sigsuspend");

        sigsuspend(&set);

        ngx_time_update();

        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                       "wake up, sigio %i", sigio);

        if (ngx_reap) {
            ngx_reap = 0;
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, cycle->log, 0, "reap children");

            live = ngx_reap_children(cycle);
        }

        if (!live && (ngx_terminate || ngx_quit)) {
            ngx_master_process_exit(cycle);
        }

        if (ngx_terminate) {
            if (delay == 0) {
                delay = 50;
            }

            if (sigio) {
                sigio--;
                continue;
            }

            sigio = ccf->worker_processes + 2 /* cache processes */;

            if (delay > 1000) {
                ngx_signal_worker_processes(cycle, SIGKILL);
            } else {
                ngx_signal_worker_processes(cycle,
                                       ngx_signal_value(NGX_TERMINATE_SIGNAL));
            }

            continue;
        }
}
{% endhighlight %}
这里首先涉及到一个延时退出的问题：首先在第一次收到SIGINT信号时，将delay设置为50，sigio设置为worker进程数+cache进程数，然后通过channel的方式告知子进程退出。如果长时间没有退出，则定时器时间就会超时，产生SIGALRM信号，这时会再发送相应的信号告诉子进程退出。如果在1秒钟的时间内子进程都没有退出，则发送SIGKILL信号暴力退出。

这里有两个变量```delay```与```sigio```控制nginx的terminate退出。前一个主要控制超时，后一个主要控制信号。如果在terminate过程中收到指定数量的信号，则继续发送NGX_TERMINATE_SIGNAL信号指示进程退出,同时重置定时器；否则等待delay控制的定时器超时，然后再继续发送退出信号指示进程退出。

在sigsuspend()上面设置超时定时器的部分关键代码段是不会被打断的，因此可以确保定时器设置成功，进入超时倒计时阶段。并且用sigio变量作为定时器超时过程中的一个额外的辅助变量，相互协作共同控制nginx进程的退出。

<br />


**2) 回收退出的子进程**
{% highlight string %}
sigsuspend(&set);

ngx_time_update();

ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
               "wake up, sigio %i", sigio);

if (ngx_reap) {
    ngx_reap = 0;
    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, cycle->log, 0, "reap children");

    live = ngx_reap_children(cycle);
}

if (!live && (ngx_terminate || ngx_quit)) {
    ngx_master_process_exit(cycle);
}
{% endhighlight %}
这里我们看到sigsuspend()接收到信号，调用对应的信号处理函数返回后首先会调用ngx_time_update()函数更新相应的时间。然后检测收到的信号，如果该信号是进程退出信号，则调用ngx_reap_children()回收相应的资源；如果该信号是terminate或quit信号，则调用ngx_master_process_exit()函数执行退出主循环。

<br />

**3) 执行ngx_quit退出操作**
{% highlight string %}
if (ngx_quit) {
    ngx_signal_worker_processes(cycle,
                                ngx_signal_value(NGX_SHUTDOWN_SIGNAL));

    ls = cycle->listening.elts;
    for (n = 0; n < cycle->listening.nelts; n++) {
        if (ngx_close_socket(ls[n].fd) == -1) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                          ngx_close_socket_n " %V failed",
                          &ls[n].addr_text);
        }
    }
    cycle->listening.nelts = 0;

    continue;
}
{% endhighlight %}
nginx_quit是优雅退出，因此这里首先向对应的子进程(worker进程、缓存管理器进程）发送shutdown信号，然后依次关闭对应的监听socket。

这里注意到，当执行terminate和quit退出时，末尾执行continue语句，并不会再对如ngx_reconfigure、ngx_restart等做出反应。
<br />

**4) 执行ngx_reconfigure**
{% highlight string %}
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
    ngx_start_cache_manager_processes(cycle, 1);

    /* allow new processes to start */
    ngx_msleep(100);

    live = 1;
    ngx_signal_worker_processes(cycle,
                                ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
}
{% endhighlight %}

ngx_reconfigure会重新加载配置文件。如果是平滑升级时，执行重新加载配置文件的操作，则ngx_new_binary条件为真，此时会新创建worker进程及cache manager进程，关于这一点请参看[ngx_process_cycle源码分析-附录](https://ivanzz1001.github.io/records/post/nginx/2017/12/11/nginx-source_part16_appendix)中相关章节。

如果是普通的**重新加载配置文件**操作，则首先读取配置文件，然后创建新的worker进程、cache manager进程，最后向原来旧的子进程发送shutdown信号优雅的退出。

<br />

**5) 执行ngx_restart**
{% highlight string %}
if (ngx_restart) {
    ngx_restart = 0;
    ngx_start_worker_processes(cycle, ccf->worker_processes,
                               NGX_PROCESS_RESPAWN);
    ngx_start_cache_manager_processes(cycle, 0);
    live = 1;
}
{% endhighlight %}
ngx_restart是配合```ngx_noaccept```使用的，ngx_noaccept会优雅的停止worker、cache manager进程，而如果此时又正好碰到平滑升级失败，则通过ngx_restart重新创建worker进程及cache manager进程。

<br />

**6) 执行ngx_reopen**
{% highlight string %}
if (ngx_reopen) {
    ngx_reopen = 0;
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reopening logs");
    ngx_reopen_files(cycle, ccf->user);
    ngx_signal_worker_processes(cycle,
                                ngx_signal_value(NGX_REOPEN_SIGNAL));
}
{% endhighlight %}
ngx_reopen()重新回滚日志。首先调用ngx_reopen_files()重新打开文件，然后向对应的子进程发送reopen信号。

<br />


**7) 执行ngx_change_binary**
{% highlight string %}
if (ngx_change_binary) {
    ngx_change_binary = 0;
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "changing binary");
    ngx_new_binary = ngx_exec_new_binary(cycle, ngx_argv);
}
{% endhighlight %}
这里执行平滑升级。首先创建出一个子进程，然后再执行exec函数族用新的nginx二进制文件替换当前子进程。

<br />

**8) 执行ngx_noaccept**
{% highlight string %}
if (ngx_noaccept) {
    ngx_noaccept = 0;
    ngx_noaccepting = 1;
    ngx_signal_worker_processes(cycle,
                                ngx_signal_value(NGX_SHUTDOWN_SIGNAL));
}
{% endhighlight %}
发送shutdown信号通知子进程优雅的退出。但是master进程不会退出，这一点是与ngx_quit相区别的，也不会向ngx_quit那样关闭监听socket。


## 2. 函数ngx_single_process_cycle()
{% highlight string %}
void
ngx_single_process_cycle(ngx_cycle_t *cycle)
{
    ngx_uint_t  i;

    if (ngx_set_environment(cycle, NULL) == NULL) {
        /* fatal */
        exit(2);
    }

    for (i = 0; cycle->modules[i]; i++) {
        if (cycle->modules[i]->init_process) {
            if (cycle->modules[i]->init_process(cycle) == NGX_ERROR) {
                /* fatal */
                exit(2);
            }
        }
    }

    for ( ;; ) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, cycle->log, 0, "worker cycle");

        ngx_process_events_and_timers(cycle);

        if (ngx_terminate || ngx_quit) {

            for (i = 0; cycle->modules[i]; i++) {
                if (cycle->modules[i]->exit_process) {
                    cycle->modules[i]->exit_process(cycle);
                }
            }

            ngx_master_process_exit(cycle);
        }

        if (ngx_reconfigure) {
            ngx_reconfigure = 0;
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reconfiguring");

            cycle = ngx_init_cycle(cycle);
            if (cycle == NULL) {
                cycle = (ngx_cycle_t *) ngx_cycle;
                continue;
            }

            ngx_cycle = cycle;
        }

        if (ngx_reopen) {
            ngx_reopen = 0;
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reopening logs");
            ngx_reopen_files(cycle, (ngx_uid_t) -1);
        }
    }
}
{% endhighlight %}
nginx只有在配置文件中master_process设置为off时，才会以单进程运行。这里首先设置相应的环境变量，并初始化对应的模块，然后进入主循环。

单进程nginx与master-worker方式运行的nginx，在主循环的处理上还是有很大的不同。在master-worker方式运行的主循环中，master只需要用sigsuspend()接收相应的信号，然后处理。而单进程nginx,它是通过在主循环中调用:
<pre>
ngx_process_events_and_timers(cycle);
</pre>
不断的接收和处理相关的事件来维持运行的。当ngx_terminate、ngx_quit、ngx_reopen等信号发生时，信号就会中断相应的事件处理模型(select、poll、epoll等）的系统调用，从而执行相应的信号处理函数，并在主循环中执行相应信号引起的其他操作：

**1) 执行ngx_terminate/ngx_quit**
{% highlight string %}
if (ngx_terminate || ngx_quit) {

    for (i = 0; cycle->modules[i]; i++) {
        if (cycle->modules[i]->exit_process) {
            cycle->modules[i]->exit_process(cycle);
        }
    }

    ngx_master_process_exit(cycle);
}
{% endhighlight %}
这里退出相应的模块，然后调用ngx_master_process_exit()退出主循环。

<br />

**2) 执行ngx_reconfigure**
{% highlight string %}
if (ngx_reconfigure) {
    ngx_reconfigure = 0;
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reconfiguring");

    cycle = ngx_init_cycle(cycle);
    if (cycle == NULL) {
        cycle = (ngx_cycle_t *) ngx_cycle;
        continue;
    }

    ngx_cycle = cycle;
}
{% endhighlight %}
此种情况下，没有相应的平滑升级等方面需要考虑，直接调用ngx_init_cycle()重新初始化配置即可。

<br />

**3) 执行ngx_reopen**
{% highlight string %}
if (ngx_reopen) {
    ngx_reopen = 0;
    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reopening logs");
    ngx_reopen_files(cycle, (ngx_uid_t) -1);
}
{% endhighlight %}
这里直接重新打开文件实现日志回滚。


## 3. 函数ngx_start_worker_processes()
{% highlight string %}
static void
ngx_start_worker_processes(ngx_cycle_t *cycle, ngx_int_t n, ngx_int_t type)
{
    ngx_int_t      i;
    ngx_channel_t  ch;

    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "start worker processes");

    ngx_memzero(&ch, sizeof(ngx_channel_t));

    ch.command = NGX_CMD_OPEN_CHANNEL;

    for (i = 0; i < n; i++) {

        ngx_spawn_process(cycle, ngx_worker_process_cycle,
                          (void *) (intptr_t) i, "worker process", type);

        ch.pid = ngx_processes[ngx_process_slot].pid;
        ch.slot = ngx_process_slot;
        ch.fd = ngx_processes[ngx_process_slot].channel[0];

        ngx_pass_open_channel(cycle, &ch);
    }
}
{% endhighlight %}

这里循环调用ngx_spawn_process()产生n个worker子进程。并向ngx_processes表中的其他子进程(worker子进程以及cache manager子进程）传送文件描述符，用于进程之间的通信。这里有两点需要注意的地方：

**1) 参数i的作用**
{% highlight string %}
ngx_spawn_process(cycle, ngx_worker_process_cycle,
                          (void *) (intptr_t) i, "worker process", type);
{% endhighlight %}
这里传递参数i，主要是为了标识当前所产生的进程是属于第几个worker子进程，然后将该子进程与CPU进行绑定，即设置该进程的CPU亲和性。

**2) 子进程间的通信**

子进程之间的通信都是通过往channel[0]中写数据，然后从channel[1]中读取数据。


## 4. 函数ngx_start_cache_manager_processes()
{% highlight string %}
static void
ngx_start_cache_manager_processes(ngx_cycle_t *cycle, ngx_uint_t respawn)
{
    ngx_uint_t       i, manager, loader;
    ngx_path_t     **path;
    ngx_channel_t    ch;

    manager = 0;
    loader = 0;

    path = ngx_cycle->paths.elts;
    for (i = 0; i < ngx_cycle->paths.nelts; i++) {

        if (path[i]->manager) {
            manager = 1;
        }

        if (path[i]->loader) {
            loader = 1;
        }
    }

    if (manager == 0) {
        return;
    }

    ngx_spawn_process(cycle, ngx_cache_manager_process_cycle,
                      &ngx_cache_manager_ctx, "cache manager process",
                      respawn ? NGX_PROCESS_JUST_RESPAWN : NGX_PROCESS_RESPAWN);

    ngx_memzero(&ch, sizeof(ngx_channel_t));

    ch.command = NGX_CMD_OPEN_CHANNEL;
    ch.pid = ngx_processes[ngx_process_slot].pid;
    ch.slot = ngx_process_slot;
    ch.fd = ngx_processes[ngx_process_slot].channel[0];

    ngx_pass_open_channel(cycle, &ch);

    if (loader == 0) {
        return;
    }

    ngx_spawn_process(cycle, ngx_cache_manager_process_cycle,
                      &ngx_cache_loader_ctx, "cache loader process",
                      respawn ? NGX_PROCESS_JUST_SPAWN : NGX_PROCESS_NORESPAWN);

    ch.command = NGX_CMD_OPEN_CHANNEL;
    ch.pid = ngx_processes[ngx_process_slot].pid;
    ch.slot = ngx_process_slot;
    ch.fd = ngx_processes[ngx_process_slot].channel[0];

    ngx_pass_open_channel(cycle, &ch);
}
{% endhighlight %}
这里首先检查是否有相应的管理器与加载器，如果有的话则创建对应的子进程。关于cache manager我们后边会进行更详细的探讨。

## 5. 函数ngx_pass_open_channel()
{% highlight string %}
static void
ngx_pass_open_channel(ngx_cycle_t *cycle, ngx_channel_t *ch)
{
    ngx_int_t  i;

    for (i = 0; i < ngx_last_process; i++) {

        if (i == ngx_process_slot
            || ngx_processes[i].pid == -1
            || ngx_processes[i].channel[0] == -1)
        {
            continue;
        }

        ngx_log_debug6(NGX_LOG_DEBUG_CORE, cycle->log, 0,
                      "pass channel s:%i pid:%P fd:%d to s:%i pid:%P fd:%d",
                      ch->slot, ch->pid, ch->fd,
                      i, ngx_processes[i].pid,
                      ngx_processes[i].channel[0]);

        /* TODO: NGX_AGAIN */

        ngx_write_channel(ngx_processes[i].channel[0],
                          ch, sizeof(ngx_channel_t), cycle->log);
    }
}
{% endhighlight %}
这里主要是通过master进程，向其各个子进程通过ngx_write_channel()传递文件描述符。

## 6. 函数ngx_signal_worker_processes()
{% highlight string %}
static void
ngx_signal_worker_processes(ngx_cycle_t *cycle, int signo)
{
    ngx_int_t      i;
    ngx_err_t      err;
    ngx_channel_t  ch;

    ngx_memzero(&ch, sizeof(ngx_channel_t));

#if (NGX_BROKEN_SCM_RIGHTS)

    ch.command = 0;

#else

    switch (signo) {

    case ngx_signal_value(NGX_SHUTDOWN_SIGNAL):
        ch.command = NGX_CMD_QUIT;
        break;

    case ngx_signal_value(NGX_TERMINATE_SIGNAL):
        ch.command = NGX_CMD_TERMINATE;
        break;

    case ngx_signal_value(NGX_REOPEN_SIGNAL):
        ch.command = NGX_CMD_REOPEN;
        break;

    default:
        ch.command = 0;
    }

#endif

    ch.fd = -1;


    for (i = 0; i < ngx_last_process; i++) {

        ngx_log_debug7(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                       "child: %i %P e:%d t:%d d:%d r:%d j:%d",
                       i,
                       ngx_processes[i].pid,
                       ngx_processes[i].exiting,
                       ngx_processes[i].exited,
                       ngx_processes[i].detached,
                       ngx_processes[i].respawn,
                       ngx_processes[i].just_spawn);

        if (ngx_processes[i].detached || ngx_processes[i].pid == -1) {
            continue;
        }

        if (ngx_processes[i].just_spawn) {
            ngx_processes[i].just_spawn = 0;
            continue;
        }

        if (ngx_processes[i].exiting
            && signo == ngx_signal_value(NGX_SHUTDOWN_SIGNAL))
        {
            continue;
        }

        if (ch.command) {
            if (ngx_write_channel(ngx_processes[i].channel[0],
                                  &ch, sizeof(ngx_channel_t), cycle->log)
                == NGX_OK)
            {
                if (signo != ngx_signal_value(NGX_REOPEN_SIGNAL)) {
                    ngx_processes[i].exiting = 1;
                }

                continue;
            }
        }

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, cycle->log, 0,
                       "kill (%P, %d)", ngx_processes[i].pid, signo);

        if (kill(ngx_processes[i].pid, signo) == -1) {
            err = ngx_errno;
            ngx_log_error(NGX_LOG_ALERT, cycle->log, err,
                          "kill(%P, %d) failed", ngx_processes[i].pid, signo);

            if (err == NGX_ESRCH) {
                ngx_processes[i].exited = 1;
                ngx_processes[i].exiting = 0;
                ngx_reap = 1;
            }

            continue;
        }

        if (signo != ngx_signal_value(NGX_REOPEN_SIGNAL)) {
            ngx_processes[i].exiting = 1;
        }
    }
}
{% endhighlight %}
本函数是向worker子进程、cache manager子进程发送信号。首先在发送前准备好相应的命令:
<pre>
#if (NGX_BROKEN_SCM_RIGHTS)

#else

#endif
</pre>
这里并未定义```NGX_BROKEN_SCM_RIGHTS```，因此执行的是else分支。接下来循环向各个子进程发送相关命令：首先用ngx_write_channel()向子进程发送信息，如果发送失败则用kill()直接向对应的子进程发送信号。

## 7. 函数ngx_reap_children()
{% highlight string %}
static ngx_uint_t
ngx_reap_children(ngx_cycle_t *cycle)
{
    ngx_int_t         i, n;
    ngx_uint_t        live;
    ngx_channel_t     ch;
    ngx_core_conf_t  *ccf;

    ngx_memzero(&ch, sizeof(ngx_channel_t));

    ch.command = NGX_CMD_CLOSE_CHANNEL;
    ch.fd = -1;

    live = 0;
    for (i = 0; i < ngx_last_process; i++) {

        ngx_log_debug7(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                       "child: %i %P e:%d t:%d d:%d r:%d j:%d",
                       i,
                       ngx_processes[i].pid,
                       ngx_processes[i].exiting,
                       ngx_processes[i].exited,
                       ngx_processes[i].detached,
                       ngx_processes[i].respawn,
                       ngx_processes[i].just_spawn);

        if (ngx_processes[i].pid == -1) {
            continue;
        }

        if (ngx_processes[i].exited) {

            if (!ngx_processes[i].detached) {
                ngx_close_channel(ngx_processes[i].channel, cycle->log);

                ngx_processes[i].channel[0] = -1;
                ngx_processes[i].channel[1] = -1;

                ch.pid = ngx_processes[i].pid;
                ch.slot = i;

                for (n = 0; n < ngx_last_process; n++) {
                    if (ngx_processes[n].exited
                        || ngx_processes[n].pid == -1
                        || ngx_processes[n].channel[0] == -1)
                    {
                        continue;
                    }

                    ngx_log_debug3(NGX_LOG_DEBUG_CORE, cycle->log, 0,
                                   "pass close channel s:%i pid:%P to:%P",
                                   ch.slot, ch.pid, ngx_processes[n].pid);

                    /* TODO: NGX_AGAIN */

                    ngx_write_channel(ngx_processes[n].channel[0],
                                      &ch, sizeof(ngx_channel_t), cycle->log);
                }
            }

            if (ngx_processes[i].respawn
                && !ngx_processes[i].exiting
                && !ngx_terminate
                && !ngx_quit)
            {
                if (ngx_spawn_process(cycle, ngx_processes[i].proc,
                                      ngx_processes[i].data,
                                      ngx_processes[i].name, i)
                    == NGX_INVALID_PID)
                {
                    ngx_log_error(NGX_LOG_ALERT, cycle->log, 0,
                                  "could not respawn %s",
                                  ngx_processes[i].name);
                    continue;
                }


                ch.command = NGX_CMD_OPEN_CHANNEL;
                ch.pid = ngx_processes[ngx_process_slot].pid;
                ch.slot = ngx_process_slot;
                ch.fd = ngx_processes[ngx_process_slot].channel[0];

                ngx_pass_open_channel(cycle, &ch);

                live = 1;

                continue;
            }

            if (ngx_processes[i].pid == ngx_new_binary) {

                ccf = (ngx_core_conf_t *) ngx_get_conf(cycle->conf_ctx,
                                                       ngx_core_module);

                if (ngx_rename_file((char *) ccf->oldpid.data,
                                    (char *) ccf->pid.data)
                    == NGX_FILE_ERROR)
                {
                    ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                                  ngx_rename_file_n " %s back to %s failed "
                                  "after the new binary process \"%s\" exited",
                                  ccf->oldpid.data, ccf->pid.data, ngx_argv[0]);
                }

                ngx_new_binary = 0;
                if (ngx_noaccepting) {
                    ngx_restart = 1;
                    ngx_noaccepting = 0;
                }
            }

            if (i == ngx_last_process - 1) {
                ngx_last_process--;

            } else {
                ngx_processes[i].pid = -1;
            }

        } else if (ngx_processes[i].exiting || !ngx_processes[i].detached) {
            live = 1;
        }
    }

    return live;
}
{% endhighlight %}
本函数回收所有已经退出子进程。如果ngx_processes进程表中仍有未退出的子进程，则返回1；否则返回0。接着执行循环关闭与该子进程相关的channel：
{% highlight string %}
for(ngx_processes)
{
     //1: 关闭退出子进程相关的channel。主要是需要告知其他子进程不应该再用该退出子进程的channel[0]了

    //2: 对于需要respawn的进程，调用ngx_spawn_process()重启进程

    //3: 如果退出的子进程是平滑升级的子进程，此时会将nginx.pid.oldbin更改回nginx.pid，并且如果原来的worker子进程
    //   执行了优雅的退出的话，此时会重启worker子进程。

    //4: 更改ngx_processes表中的退出子进程的pid
}
{% endhighlight %}

## 8. 函数ngx_master_process_exit()
{% highlight string %}
static void
ngx_master_process_exit(ngx_cycle_t *cycle)
{
    ngx_uint_t  i;

    ngx_delete_pidfile(cycle);

    ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "exit");

    for (i = 0; cycle->modules[i]; i++) {
        if (cycle->modules[i]->exit_master) {
            cycle->modules[i]->exit_master(cycle);
        }
    }

    ngx_close_listening_sockets(cycle);

    /*
     * Copy ngx_cycle->log related data to the special static exit cycle,
     * log, and log file structures enough to allow a signal handler to log.
     * The handler may be called when standard ngx_cycle->log allocated from
     * ngx_cycle->pool is already destroyed.
     */


    ngx_exit_log = *ngx_log_get_file_log(ngx_cycle->log);

    ngx_exit_log_file.fd = ngx_exit_log.file->fd;
    ngx_exit_log.file = &ngx_exit_log_file;
    ngx_exit_log.next = NULL;
    ngx_exit_log.writer = NULL;

    ngx_exit_cycle.log = &ngx_exit_log;
    ngx_exit_cycle.files = ngx_cycle->files;
    ngx_exit_cycle.files_n = ngx_cycle->files_n;
    ngx_cycle = &ngx_exit_cycle;

    ngx_destroy_pool(cycle->pool);

    exit(0);
}
{% endhighlight %}
在master退出时，执行步骤如下：

* 删除nginx.pid文件

* 调用各个模块的exit_master()函数

* 关闭监听socket

* 销毁pool

<pre>
注意：在销毁pool之前会先把ngx_cycle->log相关的数据保存到一个静态的数据结构ngx_exit_cycle中，这是因为在ngx_cycle->pool
销毁之后，有可能仍然会调用到日志打印相关的操作。
</pre>


<br />
<br />

**[参看]:**

1. [nginx中cache的设计和实现(一)](http://www.pagefault.info/?p=375&cpage=1)

2. [Nginx缓存机制详解之一缓存管理进程](http://www.it165.net/admin/html/201606/7890.html)

3. [cache loader process进程分析](http://blog.csdn.net/weiyuefei/article/details/35634675)

4. [绑定Nginx到gdb](http://book.51cto.com/art/201305/395377.htm)

5. [Nginx核心进程模型](https://www.cnblogs.com/ljygoodgoodstudydaydayup/p/3888508.html)

6. [event 模块(二) ——事件驱动核心](http://blog.csdn.net/lengzijian/article/details/7601730)

7. [Nginx平滑升级源码分析](http://blog.csdn.net/zdy0_2004/article/details/78230352)

8. [nginx文件结构](http://blog.csdn.net/apelife/article/details/53043275)

<br />
<br />
<br />

