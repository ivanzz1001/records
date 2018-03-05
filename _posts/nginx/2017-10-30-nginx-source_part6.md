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


## 2. ngx_os_init()解析

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
在ngx_auto_config.h头文件中，我们有如下定义：
<pre>
#ifndef NGX_HAVE_SC_NPROCESSORS_ONLN
#define NGX_HAVE_SC_NPROCESSORS_ONLN  1
#endif
</pre>
因此，我们这里逻辑CPU个数为1。然后通过如下：
{% highlight string %}
ngx_cpuinfo();
{% endhighlight %}
获得当前ngx_cacheline_size为64.

<br />

**3） 获得进程文件句柄数**
{% highlight string %}
if (getrlimit(RLIMIT_NOFILE, &rlmt) == -1) {
    ngx_log_error(NGX_LOG_ALERT, log, errno,
                  "getrlimit(RLIMIT_NOFILE) failed");
    return NGX_ERROR;
}

ngx_max_sockets = (ngx_int_t) rlmt.rlim_cur;
{% endhighlight %}
对于我们当前环境，通过如下程序test7.c测试:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>



int main(int argc,char *argv[])
{
    struct rlimit  rlmt;

    if (getrlimit(RLIMIT_NOFILE, &rlmt) == -1)
    {
       return -1;
    }

    printf("max open file: %d %d\n",(int)rlmt.rlim_cur,
       (int)rlmt.rlim_max);

    return 0;
}
{% endhighlight %}
编译运行：
<pre>
root@ubuntu:~/test-src# gcc -o test7 test7.c 
root@ubuntu:~/test-src# ./test7
max open file: 1024 1048576
</pre>

其实还可以用getrlimit()函数获得其他资源的限制信息。如下做一个简单介绍：
<pre>
名称 	            意义
RLIMIT_AS 	     进程总共可用的内存大小的最大值
RLIMIT_CORE 	 core文件的最大尺寸，如果为0说明不能创建core文件
RLIMIT_CPU 	     CPU时间的最大值（单位：秒）
RLIMIT_DATA 	 数据段大小的最大值
RLIMIT_FSIZE 	 创建文件的大小的最大值
RLIMIT_LOCKS 	 进程可建立的文件锁的数量的最大值
RLIMIT_MEMLOCK 	 进程中使用mlock锁定内存的最大尺寸
RLIMIT_NOFILE 	 进程中文件的打开数量的最大值
RLIMIT_NPROC 	 每个real user id的子进程数量的最大值
RLIMIT_RSS 	     最大常驻存储区大小
RLIMIT_SBSIZE 	 socket缓冲的大小的最大值
RLIMIT_STACK 	 栈的最大尺寸
RLIMIT_VMEM 	 =RLIMIT_AS
</pre>

也可以在Linux命令行通过如下命令来查看当前操作系统资源的一些限制：
<pre>
root@ubuntu:~/test-src# ulimit -a
core file size          (blocks, -c) 0
data seg size           (kbytes, -d) unlimited
scheduling priority             (-e) 0
file size               (blocks, -f) unlimited
pending signals                 (-i) 5620
max locked memory       (kbytes, -l) 64
max memory size         (kbytes, -m) unlimited
open files                      (-n) 1024
pipe size            (512 bytes, -p) 8
POSIX message queues     (bytes, -q) 819200
real-time priority              (-r) 0
stack size              (kbytes, -s) 8192
cpu time               (seconds, -t) unlimited
max user processes              (-u) 5620
virtual memory          (kbytes, -v) unlimited
file locks                      (-x) unlimited
</pre>


<br />

**4) 判断是否具有继承的非阻塞属性**
{% highlight string %}
#if (NGX_HAVE_INHERITED_NONBLOCK || NGX_HAVE_ACCEPT4)
    ngx_inherited_nonblocking = 1;
#else
    ngx_inherited_nonblocking = 0;
#endif
{% endhighlight %}

当前我们在ngx_auto_config.h头文件中有如下定义：
<pre>
#ifndef NGX_HAVE_ACCEPT4
#define NGX_HAVE_ACCEPT4  1
#endif
</pre>
因此这里我们可以获得继承的非阻塞属性。```NGX_HAVE_INHERITED_NONBLOCK```在当前环境并未定义。

<br />

**5) 初始化随机函数种子**
{% highlight string %}
srandom(ngx_time());
{% endhighlight %}



## 3. 打印当前系统状态
{% highlight string %}
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
{% endhighlight %}


在ngx_auto_config.h头文件中具有如下宏定义：
<pre>
#ifndef NGX_COMPILER
#define NGX_COMPILER  "gcc 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4) "
#endif
</pre>

这里主要打印出如下信息：

* nginx版本号

* 编译器版本号

* 操作系统版本号

* 当前进程打开文件句柄数




<br />
<br />

**[参看]:**

1. [在Linux下的进程资源的限制（struct rlimit）详解](http://blog.csdn.net/bingqingsuimeng/article/details/12167139)


<br />
<br />
<br />

