---
layout: post
title: os/unix/ngx_process.c(h)源码分析附录
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章主要介绍一下ngx_process.c源文件中所涉及到的一些其他知识点：wait函数族。


<!-- more -->


## 1. wait()函数族
{% highlight string %}
#include <sys/types.h>
#include <sys/wait.h>

pid_t wait(int *status);

pid_t waitpid(pid_t pid, int *status, int options);

int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
{% endhighlight %}
如上三个函数都是用于在调用进程中等待(wait)子进程的状态改变，然后获得状态改变的子进程的相关信息。子进程的状态改变一般有如下几种：

* 子进程终止 (terminate)

* 子进程接受到信号而暂停(stopped)

* 子进程从暂停状态恢复运行(resume)

在一个子进程终止的情形下，调用wait允许操作系统释放该子进程所占用的资源；如果并未执行一个wait操作，则这个已经终止的子进程将会保持在```zombie```状态。

假若一个子进程的状态发生了改变，则如上调用会马上返回。否则，这些系统调用会一直阻塞直到一个子进程的状态发生了改变，又或者被信号中断（这里假设中断的系统调用并不会自动重启： 通过设置sigaction()的SA_RESTART标志)。在本章中，一个子进程的状态发生了改变，但是并未被如上wait函数族所wait的话，则称该子进程处于```waitable终止状态```.







<br />
<br />
**[参考]:**

1. [UNIX再学习 -- exit 和 wait 系列函数](http://blog.csdn.net/qq_29350001/article/details/70255915)


<br />
<br />
<br />

