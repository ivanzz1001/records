---
layout: post
title: os/unix/ngx_posix_init.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本文我们主要介绍一下ngx_posix_init.c， 主要完成贴近于操作系统层面的一些变量的预先初始化。


<!-- more -->


## 1. os/unix/ngx_posix_init.c源文件

源文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <nginx.h>


ngx_int_t   ngx_ncpu;
ngx_int_t   ngx_max_sockets;
ngx_uint_t  ngx_inherited_nonblocking;
ngx_uint_t  ngx_tcp_nodelay_and_tcp_nopush;


struct rlimit  rlmt;


ngx_os_io_t ngx_os_io = {
    ngx_unix_recv,
    ngx_readv_chain,
    ngx_udp_unix_recv,
    ngx_unix_send,
    ngx_udp_unix_send,
    ngx_writev_chain,
    0
};


ngx_int_t
ngx_os_init(ngx_log_t *log)
{
    ngx_uint_t  n;

#if (NGX_HAVE_OS_SPECIFIC_INIT)
    if (ngx_os_specific_init(log) != NGX_OK) {
        return NGX_ERROR;
    }
#endif

    if (ngx_init_setproctitle(log) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_pagesize = getpagesize();
    ngx_cacheline_size = NGX_CPU_CACHE_LINE;

    for (n = ngx_pagesize; n >>= 1; ngx_pagesize_shift++) { /* void */ }

#if (NGX_HAVE_SC_NPROCESSORS_ONLN)
    if (ngx_ncpu == 0) {
        ngx_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    }
#endif

    if (ngx_ncpu < 1) {
        ngx_ncpu = 1;
    }

    ngx_cpuinfo();

    if (getrlimit(RLIMIT_NOFILE, &rlmt) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, errno,
                      "getrlimit(RLIMIT_NOFILE) failed");
        return NGX_ERROR;
    }

    ngx_max_sockets = (ngx_int_t) rlmt.rlim_cur;

#if (NGX_HAVE_INHERITED_NONBLOCK || NGX_HAVE_ACCEPT4)
    ngx_inherited_nonblocking = 1;
#else
    ngx_inherited_nonblocking = 0;
#endif

    srandom(ngx_time());

    return NGX_OK;
}


void
ngx_os_status(ngx_log_t *log)
{
    ngx_log_error(NGX_LOG_NOTICE, log, 0, NGINX_VER_BUILD);

#ifdef NGX_COMPILER
    ngx_log_error(NGX_LOG_NOTICE, log, 0, "built by " NGX_COMPILER);
#endif

#if (NGX_HAVE_OS_SPECIFIC_INIT)
    ngx_os_specific_status(log);
#endif

    ngx_log_error(NGX_LOG_NOTICE, log, 0,
                  "getrlimit(RLIMIT_NOFILE): %r:%r",
                  rlmt.rlim_cur, rlmt.rlim_max);
}


#if 0

ngx_int_t
ngx_posix_post_conf_init(ngx_log_t *log)
{
    ngx_fd_t  pp[2];

    if (pipe(pp) == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno, "pipe() failed");
        return NGX_ERROR;
    }

    if (dup2(pp[1], STDERR_FILENO) == -1) {
        ngx_log_error(NGX_LOG_EMERG, log, errno, "dup2(STDERR) failed");
        return NGX_ERROR;
    }

    if (pp[1] > STDERR_FILENO) {
        if (close(pp[1]) == -1) {
            ngx_log_error(NGX_LOG_EMERG, log, errno, "close() failed");
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}

#endif
{% endhighlight %}


**(1) 相关初始化**

在os/unix/ngx_linux_config.h头文件中定义了```NGX_HAVE_OS_SPECIFIC_INIT```,因此会执行：
<pre>
#if (NGX_HAVE_OS_SPECIFIC_INIT)
    if (ngx_os_specific_init(log) != NGX_OK) {
        return NGX_ERROR;
    }
#endif
</pre>

然后初始化proctitle相关环境。再接着执行如下初始化：
{% highlight string %}
ngx_pagesize = getpagesize();
ngx_cacheline_size = NGX_CPU_CACHE_LINE;

for (n = ngx_pagesize; n >>= 1; ngx_pagesize_shift++) { /* void */ }
{% endhighlight %}

对于当前环境，我们写一个简单的程序(test6.c)来进行测试：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc,char *argv[])
{

   int pagesize;

   pagesize = getpagesize();

   printf("pagesize: %d\n",pagesize);
   return 0;
}
{% endhighlight %}
编译运行：
<pre>
root@ubuntu:~/test-src# gcc -o test6 test6.c 
root@ubuntu:~/test-src# ./test6
pagesize: 4096
</pre>

这里我们看到，当前内存的页大小为4096。ngx_cacheline_size被初始化为32（注意，后面获取CPU信息时针对当前CPU会修改为64)。ngx_pagesize_shift为12.

<br />

**(2) 初始化cpu相关信息**









<br />
<br />
<br />

