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


** 3) 启动worker进程**
{% highlight string %}
{% endhighlight %}






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

<br />
<br />
<br />

