---
layout: post
title: os/unix/ngx_process.c源码分析(附录)
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

假若一个子进程的状态发生了改变，则如上调用会马上返回。否则，这些系统调用会一直阻塞直到一个子进程的状态发生了改变，又或者被信号中断（这里假设中断的系统调用并不会自动重启： 通过设置sigaction()的SA_RESTART标志)。在本章中，一个子进程的状态发生了改变，但是并未被如上wait函数族所wait的话，则称该子进程处于```waitable状态```.


### 1.1 wait()函数
wait()系统调用会挂起调用进程的执行，直到其子进程终止。wait(&status)调用等价于：
<pre>
waitpid(-1,&status,0);
</pre>

### 1.2 waitpid()函数
{% highlight string %}
pid_t waitpid(pid_t pid, int *status, int options);
{% endhighlight %}
waitpid()系统调用会挂起调用进程的执行，直到```pid```参数指定的子进程状态发生改变。默认情况下，waitpid()只会等待终止的子进程，但是该行为可以通过```options```参数所修改。

**1) 参数pid**

参数pid的取值可以为：
{% highlight string %}
 < -1    等待任何一个子进程，该子进程的进程组ID等于pid的绝对值

 -1      等待任何一个子进程

 0       等待任何一个子进程，该子进程的进程组ID等于调用进程的进程组ID

 > 0     等待进程ID为pid的子进程
{% endhighlight %}

**2) 参数options**

参数options可以为如下常量的按位或.

* **WNOHANG**: 假若并没有子进程退出的话，则马上返回

* **WUNTRACED**: 假若一个子进程已经暂停的话(此子进程不应该被ptrace()系统调用所traced)，则函数返回。如果被traced的子进车已经停止的话，则不管本选项有没有提供其都会返回相应的状态。

* **WCONTINUED**: 假若一个暂停的子进程通过SIGCONT信号恢复运行的话，则函数也会返回。（此选项从Linux 2.6.10版本开始支持）

**3) 参数status**

假若status参数不为NULL的话，wait()及waitpid()会将相应的状态信息存储到status所指向的空间中。这个整数值可以通过如下的一些宏来进行解释：

* **WIFEXITED(status)**： 假如子进程是正常终止的话，则返回true。（正常终止是指：调用exit()、_exit()或者main()函数return返回）

* **WEXITSTATUS(status)**: 返回子进程的退出状态。status的低8位存储着相应的退出状态（通过exit()、_exit()、main()返回的退出状态)。 此宏定义只应在```WIFEXITED(status)```返回true时使用。

* **


<br />
<br />
**[参考]:**

1. [UNIX再学习 -- exit 和 wait 系列函数](http://blog.csdn.net/qq_29350001/article/details/70255915)


<br />
<br />
<br />

