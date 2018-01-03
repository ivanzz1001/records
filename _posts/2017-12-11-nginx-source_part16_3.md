---
layout: post
title: os/unix/ngx_process_cycle.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

这里我们接着上文继续讲述一下ngx_process_cycle.c源文件中的剩余的部分。


<!-- more -->

## 1. 函数ngx_worker_process_cycle()
{% highlight string %}
static void
ngx_worker_process_cycle(ngx_cycle_t *cycle, void *data)
{
    ngx_int_t worker = (intptr_t) data;

    ngx_process = NGX_PROCESS_WORKER;
    ngx_worker = worker;

    ngx_worker_process_init(cycle, worker);

    ngx_setproctitle("worker process");

    for ( ;; ) {

        if (ngx_exiting) {
            ngx_event_cancel_timers();

            if (ngx_event_timer_rbtree.root == ngx_event_timer_rbtree.sentinel)
            {
                ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "exiting");

                ngx_worker_process_exit(cycle);
            }
        }

        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, cycle->log, 0, "worker cycle");

        ngx_process_events_and_timers(cycle);

        if (ngx_terminate) {
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "exiting");

            ngx_worker_process_exit(cycle);
        }

        if (ngx_quit) {
            ngx_quit = 0;
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0,
                          "gracefully shutting down");
            ngx_setproctitle("worker process is shutting down");

            if (!ngx_exiting) {
                ngx_exiting = 1;
                ngx_close_listening_sockets(cycle);
                ngx_close_idle_connections(cycle);
            }
        }

        if (ngx_reopen) {
            ngx_reopen = 0;
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reopening logs");
            ngx_reopen_files(cycle, -1);
        }
    }
}
{% endhighlight %}
这里首先调用ngx_worker_process_init()函数初始化对应的worker进程，然后调用ngx_setproctitle()设置进程的标题。接着进入主循环：
<pre>
for(;;)
{
    //1. 处理exiting条件。之所以放在这里处理，是因为ngx_quit是进行优雅的退出，因此必须放在ngx_process_events_and_timers()
    //   前以使程序在没有任何事件和定时器时可以退出

    //2. 调用ngx_process_events_and_timers()处理网络事件和定时器事件，没有事件产生时程序阻塞在这里

    //3. 收到SIGTERM信号，则ngx_terminate为1，worker子进程马上退出

    //4. 收到SIGQUIT信号，则ngx_quit为1，worker子进程进行优雅退出。关闭监听socket及处于空闲状态的连接

    //5. 收到日志回滚的信号，则ngx_reopen为1，进行相应的日志回滚操作。 
}
</pre>

## 2. 函数ngx_worker_process_init()
{% highlight string %}
static void
ngx_worker_process_init(ngx_cycle_t *cycle, ngx_int_t worker)
{
    sigset_t          set;
    ngx_int_t         n;
    ngx_uint_t        i;
    ngx_cpuset_t     *cpu_affinity;
    struct rlimit     rlmt;
    ngx_core_conf_t  *ccf;
    ngx_listening_t  *ls;

    if (ngx_set_environment(cycle, NULL) == NULL) {
        /* fatal */
        exit(2);
    }

    ccf = (ngx_core_conf_t *) ngx_get_conf(cycle->conf_ctx, ngx_core_module);

    if (worker >= 0 && ccf->priority != 0) {
        if (setpriority(PRIO_PROCESS, 0, ccf->priority) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "setpriority(%d) failed", ccf->priority);
        }
    }

    if (ccf->rlimit_nofile != NGX_CONF_UNSET) {
        rlmt.rlim_cur = (rlim_t) ccf->rlimit_nofile;
        rlmt.rlim_max = (rlim_t) ccf->rlimit_nofile;

        if (setrlimit(RLIMIT_NOFILE, &rlmt) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "setrlimit(RLIMIT_NOFILE, %i) failed",
                          ccf->rlimit_nofile);
        }
    }

    if (ccf->rlimit_core != NGX_CONF_UNSET) {
        rlmt.rlim_cur = (rlim_t) ccf->rlimit_core;
        rlmt.rlim_max = (rlim_t) ccf->rlimit_core;

        if (setrlimit(RLIMIT_CORE, &rlmt) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "setrlimit(RLIMIT_CORE, %O) failed",
                          ccf->rlimit_core);
        }
    }

    if (geteuid() == 0) {
        if (setgid(ccf->group) == -1) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                          "setgid(%d) failed", ccf->group);
            /* fatal */
            exit(2);
        }

        if (initgroups(ccf->username, ccf->group) == -1) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                          "initgroups(%s, %d) failed",
                          ccf->username, ccf->group);
        }

        if (setuid(ccf->user) == -1) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                          "setuid(%d) failed", ccf->user);
            /* fatal */
            exit(2);
        }
    }

    if (worker >= 0) {
        cpu_affinity = ngx_get_cpu_affinity(worker);

        if (cpu_affinity) {
            ngx_setaffinity(cpu_affinity, cycle->log);
        }
    }

#if (NGX_HAVE_PR_SET_DUMPABLE)

    /* allow coredump after setuid() in Linux 2.4.x */

    if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "prctl(PR_SET_DUMPABLE) failed");
    }

#endif

    if (ccf->working_directory.len) {
        if (chdir((char *) ccf->working_directory.data) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "chdir(\"%s\") failed", ccf->working_directory.data);
            /* fatal */
            exit(2);
        }
    }

    sigemptyset(&set);

    if (sigprocmask(SIG_SETMASK, &set, NULL) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "sigprocmask() failed");
    }

    srandom((ngx_pid << 16) ^ ngx_time());

    /*
     * disable deleting previous events for the listening sockets because
     * in the worker processes there are no events at all at this point
     */
    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {
        ls[i].previous = NULL;
    }

    for (i = 0; cycle->modules[i]; i++) {
        if (cycle->modules[i]->init_process) {
            if (cycle->modules[i]->init_process(cycle) == NGX_ERROR) {
                /* fatal */
                exit(2);
            }
        }
    }

    for (n = 0; n < ngx_last_process; n++) {

        if (ngx_processes[n].pid == -1) {
            continue;
        }

        if (n == ngx_process_slot) {
            continue;
        }

        if (ngx_processes[n].channel[1] == -1) {
            continue;
        }

        if (close(ngx_processes[n].channel[1]) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          "close() channel failed");
        }
    }

    if (close(ngx_processes[ngx_process_slot].channel[0]) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "close() channel failed");
    }

#if 0
    ngx_last_process = 0;
#endif

    if (ngx_add_channel_event(cycle, ngx_channel, NGX_READ_EVENT,
                              ngx_channel_handler)
        == NGX_ERROR)
    {
        /* fatal */
        exit(2);
    }
}
{% endhighlight %}

这里主要完成worker子进程的初始化：

**1) 设置环境变量**
<pre>
在这里设置子进程的环境变量时，会检查配置文件，如果该配置文件中将某一个环境变量设置为空（env_variable=)，则保持该环境变量为空；
否则从系统环境变量中查找来进行赋值。
</pre>

<br />

**2) 设置worker进程优先级**

这里我们首先介绍一下setpriority()函数：
{% highlight string %}
#include <sys/time.h>
#include <sys/resource.h>

int getpriority(int which, int who);
int setpriority(int which, int who, int prio);
{% endhighlight %}
上述两个函数分别用于获取和设置相应进程的调度优先级。参数which可以取值为**PRIO_PROCESS**、**PRIO_GRP**、**PRIO_USER**；参数who用于指定对应的id值，结合参数which共同确定对应的进程(进程ID、进程组ID、有效用户ID),若who取值为0则表示当前调用进程；参数prio用于指定对应的优先级，优先级范围为[-20,19]，并且prio值越小则越优先调度。
<pre>
注： 优先级范围在不同的系统上可能会不同。
</pre>

另外，对于getpriority()函数，如果which和who共同确定多个进程的话，则会返回这些进程中最优先的进程（即prio最低的进程)。

<br />

**3) 设置进程可打开的最大文件描述符数**
{% highlight string %}
if (ccf->rlimit_nofile != NGX_CONF_UNSET) {
    rlmt.rlim_cur = (rlim_t) ccf->rlimit_nofile;
    rlmt.rlim_max = (rlim_t) ccf->rlimit_nofile;

    if (setrlimit(RLIMIT_NOFILE, &rlmt) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "setrlimit(RLIMIT_NOFILE, %i) failed",
                      ccf->rlimit_nofile);
    }
}
{% endhighlight %}

**4) 设置进程产生的coredump文件的最大大小**
{% highlight string %}
if (ccf->rlimit_core != NGX_CONF_UNSET) {
    rlmt.rlim_cur = (rlim_t) ccf->rlimit_core;
    rlmt.rlim_max = (rlim_t) ccf->rlimit_core;

    if (setrlimit(RLIMIT_CORE, &rlmt) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "setrlimit(RLIMIT_CORE, %O) failed",
                      ccf->rlimit_core);
    }
}
{% endhighlight %}
如果值为0，则并不会产生coredump文件。

**5) 设置进程的group id和uid**
{% highlight string %}
if (geteuid() == 0) {
    if (setgid(ccf->group) == -1) {
        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                      "setgid(%d) failed", ccf->group);
        /* fatal */
        exit(2);
    }

    if (initgroups(ccf->username, ccf->group) == -1) {
        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                      "initgroups(%s, %d) failed",
                      ccf->username, ccf->group);
    }

    if (setuid(ccf->user) == -1) {
        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                      "setuid(%d) failed", ccf->user);
        /* fatal */
        exit(2);
    }
}
{% endhighlight %}
这里若```geteuid()```返回值为0，则表示当前进程是以root特权身份执行的，此种情况下拥有权限可以设置生成的worker子进程的group id和user id。函数```setgid()```用于设置有效组ID。关于initgroups函数，有如下：
{% highlight string %}
#include <sys/types.h>
#include <grp.h>

int initgroups(const char *user, gid_t group);
{% endhighlight %}
initgroups()函数通过读取组数据库/etc/group来初始化组访问列表，然后使用组成员中拥有上述函数参数指定的user的组，此外参数group也会添加到这个组访问列表中。
<pre>
说明： 参数user必须为NON-NULL
</pre>

<br />

**6) 设置worker子进程cpu亲和性**
{% highlight string %}
if (worker >= 0) {
    cpu_affinity = ngx_get_cpu_affinity(worker);

    if (cpu_affinity) {
        ngx_setaffinity(cpu_affinity, cycle->log);
    }
}
{% endhighlight %}

**7) 设置进程为dumpable**
{% highlight string %}
#if (NGX_HAVE_PR_SET_DUMPABLE)

    /* allow coredump after setuid() in Linux 2.4.x */

    if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "prctl(PR_SET_DUMPABLE) failed");
    }

#endif
{% endhighlight %}
当前我们在ngx_auto_config.h头文件中拥有如下定义：
<pre>
#ifndef NGX_HAVE_PR_SET_DUMPABLE
#define NGX_HAVE_PR_SET_DUMPABLE  1
#endif
</pre>
这里用于支持Linux 2.4.x系统，只有在setuid()完成后，才允许coredump。


**8) 设置worker子进程的工作目录**
{% highlight string %}
if (ccf->working_directory.len) {
    if (chdir((char *) ccf->working_directory.data) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "chdir(\"%s\") failed", ccf->working_directory.data);
        /* fatal */
        exit(2);
    }
}
{% endhighlight %}


**9) 清空worker子进程的信号屏蔽掩码**
{% highlight string %}
sigemptyset(&set);

if (sigprocmask(SIG_SETMASK, &set, NULL) == -1) {
    ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                  "sigprocmask() failed");
}
{% endhighlight %}
我们在worker子进程中与master等待signal不同，worker子进程是需要不断的处理网络事件以及定时器事件。

<br />

**10) 设置当前进程的随机数种子**
{% highlight string %}
srandom((ngx_pid << 16) ^ ngx_time());
{% endhighlight %}

**11) 清除监听socket上以前的事件**
{% highlight string %}
/*
 * disable deleting previous events for the listening sockets because
 * in the worker processes there are no events at all at this point
 */
ls = cycle->listening.elts;
for (i = 0; i < cycle->listening.nelts; i++) {
    ls[i].previous = NULL;
}
{% endhighlight %}

**12) 初始化相应模块**
{% highlight string %}
for (i = 0; cycle->modules[i]; i++) {
    if (cycle->modules[i]->init_process) {
        if (cycle->modules[i]->init_process(cycle) == NGX_ERROR) {
            /* fatal */
            exit(2);
        }
    }
}
{% endhighlight %}

**13) 关闭相应的管道**
{% highlight string %}
for (n = 0; n < ngx_last_process; n++) {

    if (ngx_processes[n].pid == -1) {
        continue;
    }

    if (n == ngx_process_slot) {
        continue;
    }

    if (ngx_processes[n].channel[1] == -1) {
        continue;
    }

    if (close(ngx_processes[n].channel[1]) == -1) {
        ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                      "close() channel failed");
    }
}

if (close(ngx_processes[ngx_process_slot].channel[0]) == -1) {
    ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                  "close() channel failed");
}
{% endhighlight %}
在worker子进程中，其只用其本身的channel[1]来接收数据；用其他子进程的channel[0]来发送数据。

**14) 将worker子进程的channel[1]添加到事件表中**
{% highlight string %}
#if 0
    ngx_last_process = 0;
#endif

    if (ngx_add_channel_event(cycle, ngx_channel, NGX_READ_EVENT,
                              ngx_channel_handler)
        == NGX_ERROR)
    {
        /* fatal */
        exit(2);
    }
{% endhighlight %}
将worker的channel[1]添加到可读事件列表中。这里针对channel的同一个fd，不会同时进行读写操作。


<br />
<br />

**[参看]:**



<br />
<br />
<br />

