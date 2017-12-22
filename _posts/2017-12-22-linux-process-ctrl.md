---
layout: post
title: Linux进程控制
tags:
- LinuxOps
categories: linux
description: Linux进程控制
---

本章我们介绍一下Linux下进程的暂停、恢复、后台进程、前台进程等相关的操作。



<!-- more -->


## 1. 进程控制
有如下程序test.c:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
     while(1)
     {
         sleep(20);
     }
     return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test
</pre>
开启另外一个控制台查看进程相应状态：
<pre>
[root@localhost ~]# ps -aux | grep test | grep -v grep
root      35691  0.0  0.0   4156   344 pts/2    S+   21:57   0:00 ./test
</pre>
可以看到test进程的进程ID为35691，状态为```S+```。这里我们介绍一下```ps -aux```显示进程状态的各个字符的含义：
{% highlight string %}
D    uninterruptible sleep (usually IO)
R    running or runnable (on run queue)
S    interruptible sleep (waiting for an event to complete)
T    stopped by job control signal
t    stopped by debugger during the tracing
W    paging (not valid since the 2.6.xx kernel)
X    dead (should never be seen)
Z    defunct ("zombie") process, terminated but not reaped by its parent

//For BSD formats and when the stat keyword is used, additional characters may be displayed:

<    high-priority (not nice to other users)
N    low-priority (nice to other users)
L    has pages locked into memory (for real-time and custom IO)
s    is a session leader
l    is multi-threaded (using CLONE_THREAD, like NPTL pthreads do)
+    is in the foreground process group
{% endhighlight %}




<br />
<br />

**[参看]:**

1. [LINUX 暂停、继续进程](https://www.cnblogs.com/TerrySunShine/p/5842220.html)


<br />
<br />
<br />





