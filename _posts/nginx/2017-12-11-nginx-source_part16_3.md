---
layout: post
title: os/unix/ngx_process_cycle.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

这里我们接着上文继续讲述一下ngx_process_cycle.c源文件中的剩余的部分:

* worker进程的相关处理

* cache manager进程的相关处理

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


## 3. 函数ngx_worker_process_exit()
{% highlight string %}
static void
ngx_worker_process_exit(ngx_cycle_t *cycle)
{
    ngx_uint_t         i;
    ngx_connection_t  *c;

    for (i = 0; cycle->modules[i]; i++) {
        if (cycle->modules[i]->exit_process) {
            cycle->modules[i]->exit_process(cycle);
        }
    }

    if (ngx_exiting) {
        c = cycle->connections;
        for (i = 0; i < cycle->connection_n; i++) {
            if (c[i].fd != -1
                && c[i].read
                && !c[i].read->accept
                && !c[i].read->channel
                && !c[i].read->resolver)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, 0,
                              "*%uA open socket #%d left in connection %ui",
                              c[i].number, c[i].fd, i);
                ngx_debug_quit = 1;
            }
        }

        if (ngx_debug_quit) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, 0, "aborting");
            ngx_debug_point();
        }
    }

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

    ngx_log_error(NGX_LOG_NOTICE, ngx_cycle->log, 0, "exit");

    exit(0);
}
{% endhighlight %} 
在worker子进程退出时，执行步骤如下：

* 关闭cycle中所有的模块

* 处理优雅退出情况(ngx_exiting)
<pre>
对于优雅退出的情况，如果监听socket上仍有多个打开的socket句柄，则调用ngx_debug_point()来处理本worker子进程；
当收到NGX_NOACCEPT_SIGNAL信号时，则直接提示以ngx_debug_point()来处理本worker子进程。
</pre>

* 销毁pool
<pre>
注意：在销毁pool之前会先把ngx_cycle->log相关的数据保存到一个静态的数据结构ngx_exit_cycle中，这是因为在ngx_cycle->pool
销毁之后，有可能仍然会调用到日志打印相关的操作。
</pre>


## 4. 函数ngx_channel_handler()
{% highlight string %}
static void
ngx_channel_handler(ngx_event_t *ev)
{
    ngx_int_t          n;
    ngx_channel_t      ch;
    ngx_connection_t  *c;

    if (ev->timedout) {
        ev->timedout = 0;
        return;
    }

    c = ev->data;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ev->log, 0, "channel handler");

    for ( ;; ) {

        n = ngx_read_channel(c->fd, &ch, sizeof(ngx_channel_t), ev->log);

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, ev->log, 0, "channel: %i", n);

        if (n == NGX_ERROR) {

            if (ngx_event_flags & NGX_USE_EPOLL_EVENT) {
                ngx_del_conn(c, 0);
            }

            ngx_close_connection(c);
            return;
        }

        if (ngx_event_flags & NGX_USE_EVENTPORT_EVENT) {
            if (ngx_add_event(ev, NGX_READ_EVENT, 0) == NGX_ERROR) {
                return;
            }
        }

        if (n == NGX_AGAIN) {
            return;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, ev->log, 0,
                       "channel command: %ui", ch.command);

        switch (ch.command) {

        case NGX_CMD_QUIT:
            ngx_quit = 1;
            break;

        case NGX_CMD_TERMINATE:
            ngx_terminate = 1;
            break;

        case NGX_CMD_REOPEN:
            ngx_reopen = 1;
            break;

        case NGX_CMD_OPEN_CHANNEL:

            ngx_log_debug3(NGX_LOG_DEBUG_CORE, ev->log, 0,
                           "get channel s:%i pid:%P fd:%d",
                           ch.slot, ch.pid, ch.fd);

            ngx_processes[ch.slot].pid = ch.pid;
            ngx_processes[ch.slot].channel[0] = ch.fd;
            break;

        case NGX_CMD_CLOSE_CHANNEL:

            ngx_log_debug4(NGX_LOG_DEBUG_CORE, ev->log, 0,
                           "close channel s:%i pid:%P our:%P fd:%d",
                           ch.slot, ch.pid, ngx_processes[ch.slot].pid,
                           ngx_processes[ch.slot].channel[0]);

            if (close(ngx_processes[ch.slot].channel[0]) == -1) {
                ngx_log_error(NGX_LOG_ALERT, ev->log, ngx_errno,
                              "close() channel failed");
            }

            ngx_processes[ch.slot].channel[0] = -1;
            break;
        }
    }
}
{% endhighlight %}
这里读取worker的channel[1]，如果读取失败，则关闭相应的连接（因为此种情况下，我们也没有其他办法再进行恢复）。接着处理channel发过来的相应的命令。
<pre>
注意：

1) 在ngx_read_channel()读取出现错误时，如果当前用的网络事件模型是epoll模型，还应该将相应的句柄移除epoll监听队列

2) 对于NGX_USE_EVENTPORT_EVENT模型，需要再次重新添加相应的事件
</pre>

## 5. 函数ngx_cache_manager_process_cycle()
{% highlight string %}
static void
ngx_cache_manager_process_cycle(ngx_cycle_t *cycle, void *data)
{
    ngx_cache_manager_ctx_t *ctx = data;

    void         *ident[4];
    ngx_event_t   ev;

    /*
     * Set correct process type since closing listening Unix domain socket
     * in a master process also removes the Unix domain socket file.
     */
    ngx_process = NGX_PROCESS_HELPER;

    ngx_close_listening_sockets(cycle);

    /* Set a moderate number of connections for a helper process. */
    cycle->connection_n = 512;

    ngx_worker_process_init(cycle, -1);

    ngx_memzero(&ev, sizeof(ngx_event_t));
    ev.handler = ctx->handler;
    ev.data = ident;
    ev.log = cycle->log;
    ident[3] = (void *) -1;

    ngx_use_accept_mutex = 0;

    ngx_setproctitle(ctx->name);

    ngx_add_timer(&ev, ctx->delay);

    for ( ;; ) {

        if (ngx_terminate || ngx_quit) {
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "exiting");
            exit(0);
        }

        if (ngx_reopen) {
            ngx_reopen = 0;
            ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reopening logs");
            ngx_reopen_files(cycle, -1);
        }

        ngx_process_events_and_timers(cycle);
    }
}
{% endhighlight %}
本函数是cache manager和cache loader的主循环函数。

**1) 初始化**

* 标识ngx_process为```NGX_PROCESS_HELPER```辅助进程

* 关闭监听socket

* 初始化本进程
<pre>
注意: ngx_worker_process_init(cycle, -1); 参数为-1，代表并不需要进行进行优先级的设置以及CPU亲和性的设置.
</pre>

* 添加定时器事件
<pre>
ngx_add_timer(&ev, ctx->delay);

1) 对于cache manager进程，ctx->delay为0，表示定时器没有延迟，马上执行。

2) 对于cache loader进程，ctx->delay为60000ms
</pre>

* ngx_use_accept_mutex设置为0，表示当前并不需要抢占accept锁，这是因为cache manager及cache loader进程均不会使用到tcp 80端口对应的socket。

<br />

**2) 主循环**
{% highlight string %}
for ( ;; ) {

    if (ngx_terminate || ngx_quit) {
        ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "exiting");
        exit(0);
    }

    if (ngx_reopen) {
        ngx_reopen = 0;
        ngx_log_error(NGX_LOG_NOTICE, cycle->log, 0, "reopening logs");
        ngx_reopen_files(cycle, -1);
    }

    ngx_process_events_and_timers(cycle);
}
{% endhighlight %}

这里主循环主要是处理网络事件及定时器事件。


## 6. 函数ngx_cache_manager_process_handler()
{% highlight string %}
static void
ngx_cache_manager_process_handler(ngx_event_t *ev)
{
    time_t        next, n;
    ngx_uint_t    i;
    ngx_path_t  **path;

    next = 60 * 60;

    path = ngx_cycle->paths.elts;
    for (i = 0; i < ngx_cycle->paths.nelts; i++) {

        if (path[i]->manager) {
            n = path[i]->manager(path[i]->data);

            next = (n <= next) ? n : next;

            ngx_time_update();
        }
    }

    if (next == 0) {
        next = 1;
    }

    ngx_add_timer(ev, next * 1000);
}
{% endhighlight %}
这是cache manager管理缓存的回调函数，会根据需要管理的缓存数量决定定时器的超时间隔。关于nginx缓存，我们后续还会有更详细的介绍。

## 7. 函数ngx_cache_loader_process_handler()
{% highlight string %}
static void
ngx_cache_loader_process_handler(ngx_event_t *ev)
{
    ngx_uint_t     i;
    ngx_path_t   **path;
    ngx_cycle_t   *cycle;

    cycle = (ngx_cycle_t *) ngx_cycle;

    path = cycle->paths.elts;
    for (i = 0; i < cycle->paths.nelts; i++) {

        if (ngx_terminate || ngx_quit) {
            break;
        }

        if (path[i]->loader) {
            path[i]->loader(path[i]->data);
            ngx_time_update();
        }
    }

    exit(0);
}
{% endhighlight %}
这是cache loader缓存加载的回调函数。在加载完成之后就会调用exit(0)退出进程。



<br />
<br />

**[参看]:**

1. [“惊群”，看看nginx是怎么解决它的](http://blog.csdn.net/russell_tao/article/details/7204260)




<br />
<br />
<br />

