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

2. 这里并未显示的将标准错误attach到/dev/null中，因此在一般情况下标准错误输出仍可以输出到控制台（除非打开的/dev/null句柄fd恰巧等于STDERR_FILENO）。
</pre>


## 2. Unix高级环境编程--守护进程


<br />
<br />







<br />
<br />
<br />

