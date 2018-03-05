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

**(1)** 首先要做的是调用umask将文件模式创建屏蔽字设置为一个已知值（通常是0）。由继承得来的文件模式创建屏蔽字可能会被设置为拒绝某些权限。如果守护进程要创建文件，那么它可能要设置特定的权限。例如，守护进程要创建组可读、组可写的文件，继承的文件模式创建屏蔽字可能会屏蔽上述两种权限中的一种，而使其无法发挥作用。另一方面，如果守护进程调用的库函数创建了文件，那么将文件模式创建屏蔽字设置为一个限制性更强的值（如007)可能会更明智，因为库函数可能不允许调用者通过一个显示的函数参数来设置权限。


**(2)** 调用fork，然后使父进程exit。这样做实现了下面几点。第一，如果该守护进程是作为一条简单的shell命令启动的，那么父进程终止会让shell认为这条命令已经执行完毕。 第二，虽然子进程继承了父进程的进程组ID，但获得了一个新的进程ID，这就保证了子进程不是一个进程组的组长进程。这是下面要进行的setsid调用的先决条件。


**(3)** 调用setsid创建一个新会话。使调用进程：

* 成为新会话的首进程

* 成为一个新进程组的组长进程

* 没有控制终端

<pre>
在基于System V的系统中，有些人建议在此时再调用一次fork，终止父进程，继续使用子进程中的守护进程。
这就保证了该守护进程不是会话首进程，于是按照System V规则可以防止它取得控制终端。为了避免取得控制
终端的另一种方法是，无论何时打开一个终端设备，都一定要指定O_NOCTTY。
</pre>

**(4)** 将当前工作目录更改为根目录。从父进程处继承过来的当前工作目录可能在一个挂载的文件系统中。因为守护进程通常在系统再引导之前是一直存在的，所以如果守护进程的当前工作目录在一个挂载文件系统中，那么该文件系统就不能被卸载。

或者，某些守护进程还可能会把当前工作目录更改到某个指定位置，并在此位置进行它们的全部工作。例如，行式打印机假脱机守护进程可能将其工作目录更改到它们的spool目录上。

**(5)** 关闭不再需要的文件描述符。这使守护进程不再持有从其父进程继承来的任何文件描述符（父进程可能是shell进程，或是某个其他进程）。可以使用open_max函数或getrlimit函数来判定最高文件描述符值，并关闭直到该值的所有描述符。

**(6)** 某些守护进程打开/dev/null使其具有文件描述符0，1和2，这样任何一个试图读标准输入、写标准输出或标准错误的库例程都不会产生任何效果。因为守护进程并不与终端设备相关联，所以其输出无处显示，也无处从交互式用户那里接收输入。即使守护进程是从交互式会话启动的，但是守护进程是在后台运行的，所以登录会话的终止并不影响守护进程。如果其他用户在同一终端设备上登录，我们不希望在该终端上见到守护进程的输出，用户也不期望他们在终端上的输入被守护进程读取。

<br />

如下给出一个例子(test.c)：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>
#include <unistd.h>

void daemonize(const char *cmd)
{
    int i,fd0,fd1,fd2,fd3;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    /*
     * Clear file creation mask 
    */
    umask(0);

    /*
     * Get maximum number of file descriptors
    */
    if(getrlimit(RLIMIT_NOFILE,&rl) < 0)
    {
       printf("can't get file limit\n");
       exit(-1);
    }

    /*
     * Become a session leader to lose controlling TTY
    */
    if((pid = fork()) < 0)
    {
       printf("%s can't fork\n",cmd);
       exit(-2);
    }
    else if(pid != 0)           //parent
       exit(0);
    
    setsid();

    /*
     * Ensure future opens won't allocate controlling TTYs
    */
   sa.sa_handler = SIG_IGN;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   if(sigaction(SIGHUP, &sa,NULL) < 0)
   {
      printf("%s: can't ignore SIGHUP\n",cmd);
      exit(-3);
   }
   if((pid = fork()) < 0)
   {
      printf("%s: can't fork\n",cmd);
      exit(-4);
   }
   else if(pid != 0)         //parent
      exit(0);
   
   
   /*
    * Change the current working directory to the root so
    * we won't prevent file systems from being unmounted.
   */
   if(chdir("/") < 0)
   {
      printf("%s: can't change directory to /\n",cmd);
      exit(-5);
   }


   /*
    * Close all open file descriptions.
   */ 
   if(rl.rlim_max == RLIM_INFINITY)
       rl.rlim_max = 1024;
   for(i = 0; i < rl.rlim_max;i++)
       close(i);

   /*
    * Attach file descriptors 0, 1 and 2 to /dev/null
   */
   fd0 = open("/dev/null",O_RDWR);
   fd1 = dup(0);
   fd2 = dup(0);
  
   /*
    * Initialize the log file
   */
   openlog(cmd, LOG_CONS, LOG_DAEMON);
   if(fd0 != 0 || fd1 != 1 || fd2 != 2)
   {
      syslog(LOG_ERR,"unexpected file descriptors %d %d %d\n",
        fd0, fd1, fd2);
      exit(-6);
   }
}


int main(int argc,char *argv[])
{
    daemonize(argv[0]);
    while(1)
    {
      syslog(LOG_INFO, "%s running daemonized\n",argv[0]);
      usleep(1000*1000);
    }
    return 0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test
[root@localhost test-src]# pgrep -lf test
101892 test

[root@localhost test-src]# echo $?
0

[root@localhost test-src]# tail -f /var/log/messages
Nov 14 18:42:22 localhost ./test: ./test running daemonized
Nov 14 18:42:23 localhost ./test: ./test running daemonized
Nov 14 18:42:24 localhost ./test: ./test running daemonized
Nov 14 18:42:25 localhost ./test: ./test running daemonized
Nov 14 18:42:26 localhost ./test: ./test running daemonized
Nov 14 18:42:27 localhost ./test: ./test running daemonized
Nov 14 18:42:28 localhost ./test: ./test running daemonized
Nov 14 18:42:29 localhost ./test: ./test running daemonized
Nov 14 18:42:30 localhost ./test: ./test running daemonized
Nov 14 18:42:31 localhost ./test: ./test running daemonized
Nov 14 18:42:32 localhost ./test: ./test running daemonized

[root@localhost test-src]# pgrep -lf test | awk '{print $1}' | xargs kill -9
</pre>





<br />
<br />







<br />
<br />
<br />

