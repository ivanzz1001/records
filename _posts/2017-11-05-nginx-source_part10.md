---
layout: post
title: os/unix/ngx_daemon.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要介绍一下nginx中ngx_daemon.c将进程后台化，变成守护进程的实现。

<!-- more -->


## 1. os/unix/ngx_daemon.c源文件

源文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_int_t
ngx_daemon(ngx_log_t *log)
{
    int  fd;

    switch (fork()) {
    case -1:
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "fork() failed");
        return NGX_ERROR;

    case 0:
        break;

    default:
        exit(0);
    }

    ngx_pid = ngx_getpid();

    if (setsid() == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "setsid() failed");
        return NGX_ERROR;
    }

    umask(0);

    fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
                      "open(\"/dev/null\") failed");
        return NGX_ERROR;
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "dup2(STDIN) failed");
        return NGX_ERROR;
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "dup2(STDOUT) failed");
        return NGX_ERROR;
    }

#if 0
    if (dup2(fd, STDERR_FILENO) == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "dup2(STDERR) failed");
        return NGX_ERROR;
    }
#endif

    if (fd > STDERR_FILENO) {
        if (close(fd) == -1) {
            ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "close() failed");
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}
{% endhighlight %}

这里ngx_daemon()较为简单，主要有如下步骤：

* 调用fork()产生子进程，同时父进程退出

* 调用setsid()产生一个新的会话

* 调用umask()将文件模式创建屏蔽字设置为一个已知值

* 将标准输入、标准输出attach到/dev/null中

* 关闭打开的/dev/null文件句柄fd
 
<pre>
说明：

1. 此处通过ngx_pid = ngx_getpid();将子进程的进程ID保存到了全局变量ngx_pid中

2. 这里并未显示的将标准错误attach到/dev/null中，因此在一般情况下标准错误输出仍可以输
出到控制台（除非打开的/dev/null句柄fd恰巧等于STDERR_FILENO）。
</pre>


## 2. Unix高级环境编程--守护进程

在编写守护进程程序时需遵循一些基本规则，以防止产生不必要的交互作用。下面先说明这些规则，然后给出一个按照这些规则编写的函数daemonize。

(1) 首先要做的是调用umask将文件模式创建屏蔽字设置为一个已知值（通常是0）。由继承得来的文件模式创建屏蔽字可能会被设置为拒绝某些权限。如果守护进程要创建文件，那么它可能要设置特定的权限。例如，守护进程要创建组可读、组可写的文件，继承的文件模式创建屏蔽字可能会屏蔽上述两种权限中的一种，而使其无法发挥作用。另一方面，如果守护进程调用的库函数创建了文件，那么将文件模式创建屏蔽字设置为一个限制性更强的值（如007)可能会更明智，因为库函数可能不允许调用者通过一个显示的函数参数来设置权限。

<br />

(2) 调用fork，然后使父进程exit。这样做实现了下面几点。第一，如果该守护进程是作为一条简单的shell命令启动的，那么父进程终止会让shell认为这条命令已经执行完毕。 第二，虽然子进程继承了父进程的进程组ID，但获得了一个新的进程ID，这就保证了子进程不是一个进程组的组长进程。这是下面要进行的setsid调用的先决条件。

<br />

(3) 调用setsid创建一个新会话。使调用进程：

* 称为新会话的首进程

* 称为一个新进程组的组长进程

* 没有控制终端

<pre>
在基于System V的系统中，有些人建议在此时再调用一次fork，终止父进程，继续使用子进程中的守护进程。
这就保证了该守护进程不是会话首进程，于是按照System V规则可以防止它取得控制终端。为了避免取得控制
终端的另一种方法是，无论何时打开一个终端设备，都一定要指定O_NOCTTY。
</pre>





<br />
<br />







<br />
<br />
<br />

