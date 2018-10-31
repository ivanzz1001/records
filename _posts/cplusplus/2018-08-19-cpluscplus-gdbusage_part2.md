---
layout: post
title: GDB调试调试多线程及多进程(part 1)
tags:
- cplusplus
categories: cplusplus
description: GDB的使用
---

本文档主要参看<<Debugging with GDB>> ```Tenth Edition, for gdb version 8.0.1```，本节我们主要讲述一下使用GDB来调试多线程及多进程程序。

<!-- more -->



## 1. 调试多线程

### 1.1 概念介绍

在有一些操作系统上，比如GNU/Linux与Solaris，一个进程可以有多个执行线程。线程的精确语义因操作系统不同而有一些区别，但一般来说一个程序的线程类似与多进程，除了多线程是共享同一个地址空间之外。另一方面，每一个线程都有其自己的寄存器(registers)和执行栈(execution stack)，并可能拥有其自己的私有内存。

GDB提供了如下的一些```facilities```来用于支持多线程的调试：

* 新线程的自动通知

* **thread thread_id**: 用于在线程之间切换的命令

* **info threads**: 用于查询当前存在的线程信息

* **thread apply [thread-id-list] [all] args**: 对一系列的线程应用某一个命令

* thread-specific breakpoints

* **set print thread-events**: 控制是否打印线程启动、退出消息

* **set libthread-db-search-path path**: 假如默认的选择不兼容当前程序的话，让用户选择使用那个```thread-db```

上面的```线程调试facility```使得你可以在程序运行期间观察到所有的线程，但是无论在什么时候只要被gdb接管控制权，只会有一个线程处于focus状态。该线程被称为```current thread```。GDB调试命令都是以当前线程(current thread)的视角来显示程序信息。

当GDB在程序中检测到有一个新的线程，其都会打印该线程在目标系统的标识信息，格式为```[New systag]```, 这里```systag```是一个线程标识，其具体的形式可能依系统不同而有些差异。例如在GNU/Linux操作系统上，当GDB检测到有一个新的线程时，你可能会看到：
<pre>
[New Thread 0x41e02940 (LWP 25582)]
</pre>
相反，在一些其他的系统上，```systag```可能只是一个很简单的标识，例如```process 368```。


用于调试目的，GDB会用其自己的线程号与每一个“线程inferior”相关联。在同一个```inferior```下，所有线程之间的标识号都是唯一的；但是不同```inferior```下，线程之间的标识号则可能不唯一。你可以通过```inferior-num.thread-num```语法来引用某一个```inferior```中的指定线程（这被称为```qualified Thread ID```)。例如，线程```2.3```引用```inferior 2```中线程number为2的线程。假如你省略```inferior number```的话，则GDB默认引用的是当前```inferior```中的线程。

在你创建第二个```inferior```之前，GDB并不会在```thread IDs```部分显示```inferior number```。

有一些命令接受以空格分割的```thread ID```列表作为参数，一个列表元素可以是：

* ```info threads```命令显示的```thread ID```可能包含```inferior```标识符，也可能不包括。例如： ```2.1```或者```1```

* 指定线程数范围，格式为```inf.thr1-thr2```或者```thr1-thr2```。例如： ```1.2-4```或```2-4```

* 一个```inferior```中的所有线程，可以通过```*```通配符来指定。格式为```inf.*```或者```*```。前者指定某个```inferior```中的所有线程； 后者指定当前```inferior```中的所有线程

例如，假如当前的```inferior```是1，```inferior 7```有一个线程，其ID为```7.1```，则线程列表```1 2-3 4.5 6.7-9 7.*```表示```inferior 1```中的线程1至线程3，```inferior 4```中的线程5，```inferior 6```中的线程7至线程9， 以及```inferior 7```中的所有线程。

从GDB的视角来看，一个进程至少有一个线程。换句话说，GDB会为程序的主线程指定一个```thread number```，即使在该程序并不是多线程的情况下。参看如下：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
        printf("hello,world!\n");

        return 0x0;
}
{% endhighlight %}
编译调试：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) list
1       #include <stdio.h>
2       #include <stdlib.h>
3
4       int main(int argc, char *argv[])
5       {
6               printf("hello,world!\n");
7
8               return 0x0;
9       }
(gdb) b 6
Breakpoint 1 at 0x40053c: file test.c, line 6.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:6
6               printf("hello,world!\n");
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) info inferiors
  Num  Description       Executable        
* 1    process 26158     /root/workspace/./test 
(gdb) info threads
  Id   Target Id         Frame 
* 1    process 26158 "test" main (argc=1, argv=0x7fffffffe638) at test.c:6
(gdb) 
{% endhighlight %}

假如GDB检测到程序是多线程的，假如某个线程在断点处暂停时，其就会打印出该线程的ID及线程的名称：
<pre>
Thread 2 "client" hit Breakpoint 1, send_message () at client.c:68
</pre>
相似的，当程序收到一个信号之后，其会打印如下的信息：
<pre>
Thread 1 "main" received signal SIGINT, Interrupt.
</pre>


### 1.2 GDB线程相关命令

* **info threads [thread-id-list]**: 用于显示一个或多个线程的信息。假如并未指定参数的话，则显示所有线程的信息。你可以指定想要显示的线程列表。GDB会按如下方式显示每一个线程：
{% highlight string %}
1. 由GDB指定的每一个线程的thread number

2. 由GDB指定的全局thread number(假如指定了'-gid'选项的话）

3. 目标系统的线程标识符(systag)

4. 线程名称。线程的名称可以由用户指定，在某一些情况下也可以由程序自身指定

5. 该线程的当前stack frame信息

注意： '*'指示的线程表示为当前线程
{% endhighlight %}
例如：
<pre>
(gdb) info threads
Id Target Id Frame
* 1 process 35 thread 13 main (argc=1, argv=0x7ffffff8)
2 process 35 thread 23 0x34e5 in sigpause ()
3 process 35 thread 27 0x34e5 in sigpause ()
at threadtest.c:68
</pre>
假如当前你正在调试多个```inferiors```，则GDB会使用限定的```inferior-num.thread-num```这样的格式来显示```thread IDs```。否则的话，则只会显示```thread-num```。

假如指定了```-gid```选项，那么在执行```info threads```命令时就会显示每一个线程的```global thread ID```:
<pre>
(gdb) info threads
Id GId Target Id Frame
1.1 1 process 35 thread 13 main (argc=1, argv=0x7ffffff8)
1.2 3 process 35 thread 23 0x34e5 in sigpause ()
1.3 4 process 35 thread 27 0x34e5 in sigpause ()
* 2.1 2 process 65 thread 1 main (argc=1, argv=0x7ffffff8)
</pre>


* **thread thread-id**: 使```thread-id```所指定的线程为当前线程。该命令的参数```thread-id```是GDB所指定的```thread ID```，即上面```info threads```命令显示的第一列。通过此命令切换之后，GDB会打印你所选中的线程的系统标识和当前的栈帧信息：
<pre>
(gdb) thread 2
[Switching to thread 2 (Thread 0xb7fdab70 (LWP 12747))]
#0 some_function (ignore=0x0) at example.c:8
8 printf ("hello\n");
</pre>
类似于在创建线程时打印出的```[New ...]```这样的消息，```Switching to```后面的消息打印也依赖于你所使用的系统

* 



<br />
<br />

**[参看]**


1. [GDB Inferior Tutorial](http://moss.cs.iit.edu/cs351/gdb-inferiors.html)

2. [gdb调试多进程与多线程](https://blog.csdn.net/snow_5288/article/details/72982594)


<br />
<br />
<br />





