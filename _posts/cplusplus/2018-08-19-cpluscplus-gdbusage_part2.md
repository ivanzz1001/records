---
layout: post
title: GDB调试多线程及多进程
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
<pre>
1) 'info threads' 命令显示的'thread ID'可能包含inferior标识符，也可能不包括。例如： '2.1'或者'1'

2) 指定线程数范围，格式为 'inf.thr1-thr2' 或者 'thr1-thr2'。例如： '1.2-4'或'2-4'

3) 一个 'inferior'中的所有线程，可以通过'*'通配符来指定。格式为 'inf.*'或者 '*'。前者指定某个inferior中的所有线程；
  后者指定当前inferior中的所有线程
</pre>

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

* ```thread apply [thread-id-list | all [-ascending]] command```: 本命令允许你在一个或多个线程上应用指定的```command```。如果要在所有线程上按降序的方式应用某个```command```，那么使用 'thread apply all command'; 如果要在所有线程上按升序的方式应用某个```command```，那么使用'thread apply all -ascending command';

* **thread name [name]**: 本命令用于为当前线程指定一个名称。假如并未指定参数的话，那么任何已存在的由用户指定的名称都将被移除。命名后线程的名称会出现在```info threads```的显示信息中。

* **thread find [regexp]**: 用于查询名称或```systag```匹配查询表达式的线程。例如：
<pre>
(gdb) thread find 26688
Thread 4 has target id ’Thread 0x41e02940 (LWP 26688)’
(gdb) info thread 4
Id Target Id Frame
4 Thread 0x41e02940 (LWP 26688) 0x00000031ca6cd372 in select ()
</pre>

* **set libthread-db-search-path [path]**: 假如本变量被设置，那么GDB将会使用所设置的路径(路径目录之间以':'分割)来查找```libthread_db```。假如执行此命令时，并不指定path，那么将会被重置为默认值（在GNU/Linux及Solaris系统下默认值为```$sdir:$pdir```，即系统路径和当前进程所加载线程库的路径)。而在内部，默认值来自于```LIBTHREAD_DB_SEARCH_PATH```宏定义。

在GNU/Linux以及Solaris操作系统上，GDB使用该辅助```libthread_db```库来获取inferior中线程的信息。GDB会使用'libthread-db-search-path'来搜索```libthread_db```。假如'set auto-load libthread-db'被启用的话，GDB首先会搜索该inferior所加载的线程调试库。

<pre>
在使用libthread-db-search-path搜索libthread_db时，有两个特定的路径： $sydir、$pdir

1) $sdir: 搜索共享库的默认的系统路径。本路径是唯一不需要通过'set auto-load libthread_db'命令来启用的

2） $pdir: 指示inferior process加载libpthread库的位置
</pre>

假如在上述目录中找到了```libpthread_db```库，那么GDB就会尝试用当前inferior process来初始化。假如初始化失败的话（一般在libpthread_db与libpthread版本不匹配的情况），GDB就会卸载该libpthread_db，然后尝试继续从下一个路径搜索libpthread_db。假如最后都没有找到适合的版本，GDB会打印相应的警告信息，接着线程调试将会被禁止。

注意： 本命令只在一些特定的平台上可用。


* **show libpthread-db-search-path**: 用于显示当前```libpthread_db```的搜索路径

* **set debug libpthread-db / show debug libpthread-db**: 用于启用或关闭```libpthread-db```相关的事件信息的打印。1为启用， 0为关闭。


* **set scheduler-locking mode**: 用于设置```锁定线程的模式```(scheduler locking mode)。其适用于程序正常执行、record mode以及重放模式。
<pre>
1) mode为off时，则不锁定任何线程，即所有线程在任何时间都可以被执行； 

2) mode为on时，则锁定其他线程，只有当前线程执行； 

3) mode为step时,则当在进行单步调试(single-stepping)时只有当前线程会运行，其他的线程将不会获得运行的机会，这样就可以使得调试的焦点
   只集中于当前线程。但是假如执行的时'continue'、'until'、'finish'这样的非单步调试命令的话，则其他的线程也会运行。

一般来说，除非一个线程在其运行时间片内遇到断点(breakpoint)，否则GDB一般并不会从当前调试线程切换到该线程。
</pre>


* **show scheduler-locking**: 用于显示当前的锁定模式


### 1.3 多线程调试示例

* **示例源代码**

如下是我们所采用的调试示例源代码```test.c```:
{% highlight string %}
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int a = 0;
int b = 0;

static void * pthread_run1(void *arg)
{
        int runflag = 1;

        while(runflag)
        {
                a++;
                sleep(1);
        }

        pthread_exit((void *)a);

        return NULL;
} 

static void * pthread_run2(void *arg)
{
        int runflag = 1;

        while(runflag)
        {
                b++;
                sleep(1);
        }

        pthread_exit((void *)b);

        return NULL;
}

int main(int argc,char *argv[])
{
        pthread_t tid1, tid2;
        int retval_1, retval_2;

        pthread_create(&tid1, NULL, pthread_run1, NULL);
        pthread_create(&tid2, NULL, pthread_run2, NULL);

        pthread_join(tid1,(void *)&retval_1);
        pthread_join(tid2,(void *)&retval_2);

        printf("retval_1: %d\n", retval_1);
        printf("retval_2: %d\n", retval_2);

        return 0x0;
}
{% endhighlight %}



* **编译运行**
<pre>
# gcc -c -g test.c gcc -c -g test.c -Wno-int-to-pointer-cast
# gcc -o test test.o -lpthread

# ps -aL | grep test
 40900  40900 pts/0    00:00:00 test
 40900  40901 pts/0    00:00:00 test
 40900  40902 pts/0    00:00:00 test
</pre>

然后我们再通过如下命令查看主线程和两个子线程之间的关系：
<pre>
# pstree -p 40900
test(40900)─┬─{test}(40901)
            └─{test}(40902)

</pre>

再接着通过```pstack```来查看线程栈结构：
<pre>
# pstack 40900
Thread 3 (Thread 0x7fd44f426700 (LWP 40901)):
#0  0x00007fd44f4e566d in nanosleep () from /lib64/libc.so.6
#1  0x00007fd44f4e5504 in sleep () from /lib64/libc.so.6
#2  0x0000000000400757 in pthread_run1 (arg=0x0) at test.c:14
#3  0x00007fd44f7efdc5 in start_thread () from /lib64/libpthread.so.0
#4  0x00007fd44f51e73d in clone () from /lib64/libc.so.6
Thread 2 (Thread 0x7fd44ec25700 (LWP 40902)):
#0  0x00007fd44f4e566d in nanosleep () from /lib64/libc.so.6
#1  0x00007fd44f4e5504 in sleep () from /lib64/libc.so.6
#2  0x0000000000400794 in pthread_run2 (arg=0x0) at test.c:30
#3  0x00007fd44f7efdc5 in start_thread () from /lib64/libpthread.so.0
#4  0x00007fd44f51e73d in clone () from /lib64/libc.so.6
Thread 1 (Thread 0x7fd44fc0c740 (LWP 40900)):
#0  0x00007fd44f7f0ef7 in pthread_join () from /lib64/libpthread.so.0
#1  0x00000000004007ff in main (argc=1, argv=0x7ffdbfa69a38) at test.c:46
</pre>


* **GDB调试多线程程序**

1） 启动gdb调试，并在上述代码```a++```处加上断点
<pre>
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b test.c:14 
Breakpoint 1 at 0x400749: file test.c, line 13.
</pre>



2） 运行并查看inferiors及threads信息 
<pre>
(gdb) r
Starting program: /root/workspace/./test 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".
[New Thread 0x7ffff77ff700 (LWP 41362)]
[Switching to Thread 0x7ffff77ff700 (LWP 41362)]

Breakpoint 1, pthread_run1 (arg=0x0) at test.c:13
13                      a++;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) info inferiors
  Num  Description       Executable        
* 1    process 41788     /root/workspace/./test 
(gdb) info threads
  Id   Target Id         Frame 
  3    Thread 0x7ffff6ffe700 (LWP 41793) "test" 0x00007ffff7835480 in sigprocmask () from /lib64/libc.so.6
* 2    Thread 0x7ffff77ff700 (LWP 41792) "test" pthread_run1 (arg=0x0) at test.c:14
  1    Thread 0x7ffff7fe3740 (LWP 41788) "test" 0x00007ffff7bc9ef7 in pthread_join () from /lib64/libpthread.so.0
</pre>
从上面我们看到当前停在我们设置的断点处。

接着我们执行如下：
<pre>
(gdb) s
15                      sleep(1);
(gdb) s
12              while(runflag)
(gdb) s

Breakpoint 1, pthread_run1 (arg=0x0) at test.c:14
14                      a++;
(gdb) s
15                      sleep(1);
(gdb) s
12              while(runflag)
(gdb) p b
$1 = 4
</pre>
上面我们看到当我们在单步调试```pthread_run1```的时候，```pthread_run2```也在执行。但是当我们暂停在断点处时，```pthread_run2```是不在执行的。

如果我们想在调试一个线程时，其他线程暂停执行，那么可以使用```set scheduler-locking on```来锁定。例如：
<pre>
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b test.c:14
Breakpoint 1 at 0x400742: file test.c, line 14.
(gdb) r
Starting program: /root/workspace/./test 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".
[New Thread 0x7ffff77ff700 (LWP 41951)]
[Switching to Thread 0x7ffff77ff700 (LWP 41951)]

Breakpoint 1, pthread_run1 (arg=0x0) at test.c:14
14                      a++;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) set scheduler-locking on
(gdb) p b
$1 = 0
(gdb) s
15                      sleep(1);
(gdb) s
s
12              while(runflag)
(gdb) s

Breakpoint 1, pthread_run1 (arg=0x0) at test.c:14
14                      a++;
(gdb) s
15                      sleep(1);
(gdb) s
12              while(runflag)
(gdb) s

Breakpoint 1, pthread_run1 (arg=0x0) at test.c:14
14                      a++;
(gdb) s
15                      sleep(1);
(gdb) s
12              while(runflag)
(gdb) s

Breakpoint 1, pthread_run1 (arg=0x0) at test.c:14
14                      a++;
(gdb) s
15                      sleep(1);
(gdb) s
12              while(runflag)
(gdb) p b
$2 = 0
(gdb)
</pre>


## 2. 调试多进程

### 2.1 基本概念
在大多数系统上，GDB对于通过```fork()```函数创建的子进程的调试都没有专门的支持。当一个程序fork()之后，GDB会继续调试父进程，而子进程仍会畅通无阻的运行。假如你在代码的某个部分设置了断点，然后子进程执行到该位置时，则子进程会受到一个```SIGTRAP```信号并导致子进程退出（除非子进程catch了该信号）。

然而，假如你想要调试子进程的话，也有一种相对简单的取巧方法。就是在执行完fork之后，在进入子进程代码时调用```sleep()```方法。这里可以根据某个环境变量是否设置或者某个文件是否存在来决定是否进入```sleep()```，这样就可以使得我们在非调试状态下避免休眠。当子进程进入sleep状态时，我们就可以通过```ps```命令查看到子进程的进程ID。接着可以通过使用GDB并attach到该子进程，然后就可以像调试普通程序一样进行调试了。

在有一些系统上，GDB对使用```fork()```或```vfork()```函数创建的子进程的调试提供了支持。在GNU/Linux平台上，从内核```2.5.46```版本开始该特性就被支持。

默认情况下，当一个程序forks之后，GDB会继续调试父进程，而对子进程没有任何的影响。

假如你想要跟随子进程而不是父进程，那么可以使用```set follow-fork-mode```命令：

* **set follow-fork-mode mode**: 设置GDB调试器如何对```fork```或者```vfork```进行响应。参数```mode```的取值可以为
<pre>
parent: 表示跟随父进程。这是默认情况

child: 表示跟随子进程
</pre>

* **show follow-fork-mode**: 显示当前的跟随模式

在Linux上，假如```parent```进程与```child```进程都想要调试的话，那么可以使用```set detach-on-fork```命令。

* **set detach-on-fork mode**: 用于告诉GDB在fork()之后是否分离其中的一个进程，或者同时保持对他们的控制。mode可取值为
<pre>
on: 子进程或者父进程将会被分离（取决于follow-fork-mode),使得该进程可以独立的运行。这是默认值

off: 子进程和父进程都会在GDB的控制之下。其中一个进程（取决于follow-fork-mode)可以像平常那样进行调试，而另一个进程处于挂起状态
</pre>

* **show detach-on-fork**: 用于显示```detach-on-fork```模式的值

<br />
假如你选择设置```detach-on-fork```的值为off，那么GDB将会将会保持对所有fork进程的控制（也包括内部fork)。你可以通过使用```info inferiors```命令来查看当前处于GDB控制之下的进程，并使用```inferior```命令来进行切换。

如果要退出对其中一个fork进程的调试，你可以通过使用```detach inferiors```命令来使得该进程独立的运行，或者通过```kill inferiors```命令来将该进程杀死。

假如你使用GDB来调试子进程，并且是在执行完```vfork```再调用```exec```，那么GDB会调试该新的target直到遇到第一个breakpoint。另外，假如你在orginal program的main函数中设置了断点，那么在子进程的main函数中也会保持有该断点。

在有一些系统上，当子进程是通过```vfork()```函数产生的，那么在完成```exec```调用之前，你将不能对父进程或子进程进行调试。

假如在执行完```exec```调用之后，你通过运行```run```命令，那么该新的```target```将会重启。如果要重启父进程的话，使用```file```命令并将参数设置为```parent executable name```。默认情况下，当一个exec执行完成之后，GDB会丢弃前一个可执行镜像的符号表。你可以通过```set follow-exec-mode```命令来改变这一行为：

* **set follow-exec-mode mode**: 当程序调用```exec```之后，GDB相应的行为。```exec```调用会替换一个进程的镜像。mode取值可以为：
 
1） new: GDB会创建一个新的inferior，并将该进程重新绑定到新的inferior。在执行```exec```之前的所运行的程序可以通过重启原先的inferior(original inferior)来进行 重启。例如：
{% highlight string %}
(gdb) info inferiors
(gdb) info inferior
Id Description Executable
* 1 <null> prog1
(gdb) run
process 12020 is executing new program: prog2
Program exited normally.
(gdb) info inferiors
Id Description Executable
1 <null> prog1
* 2 <null> prog2
{% endhighlight %}

2) same: GDB会将exec之后的新镜像加载到同一个```inferior```中,以替换原来的镜像。在执行exec之后如果要重启该inferior，那么可以通过运行```run```命令。这是默认模式。例如：
{% highlight string %}
(gdb) info inferiors
Id Description Executable
* 1 <null> prog1
(gdb) run
process 12020 is executing new program: prog2
Program exited normally.
(gdb) info inferiors
Id Description Executable
* 1 <null> prog2
{% endhighlight %}


### 2.2 调试示例

* **调试子进程**

1） 示例源码
{% highlight string %}
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc,char *argv[])
{
        pid_t pid;

        pid = fork();

        if(pid < 0)
        {
                return -1;
        }
        else if(pid > 0)
        {
                return 1;
        }
        printf("hello,world!\n");

        return 0x0;
}
{% endhighlight %}

2) 调试步骤

首先执行下面的命令进行编译：
<pre>
# gcc -g -c test.c
# gcc -o test test.o
</pre>

在调试多进程程序时，GDB默认会追踪处理父进程。例如：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40058c: file test.c, line 10.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:10
10              pid = fork();
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
Detaching after fork from child process 52320.
hello,world!
12              if(pid < 0)
(gdb) n
16              else if(pid > 0)
(gdb) n
18                      return 1;
(gdb) n
23      }
(gdb) n
0x00007ffff7a3db35 in __libc_start_main () from /lib64/libc.so.6
{% endhighlight %}
上面我们看到，子进程很快就打印出了```hello,world!```,说明GDB并没有控制住子进程。而在父进程中，我们通过单步执行到第18行的return，然后父进程返回退出。

如果要调试子进程，要使用如下的命令: ```set follow-fork-mode child```。例如：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40058c: file test.c, line 10.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:10
10              pid = fork();
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) set follow-fork-mode child
(gdb) show follow-fork-mode
Debugger response to a program call of fork or vfork is "child".
(gdb) n
[New process 52457]
[Switching to process 52457]
12              if(pid < 0)
(gdb) n
16              else if(pid > 0)
(gdb) n
20              printf("hello,world!\n");
(gdb) n
hello,world!
22              return 0x0;
(gdb) n
23      }
{% endhighlight %}
上面我们看到程序执行到第20行： 子进程打印出```hello,world!```.


* **同时调试父进程和子进程**

1） 示例源码
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    pid_t pid;

    pid = fork();
    if (pid < 0)
    {
        exit(1);
    }
    else if (pid > 0)
    {
        printf("Parent\n");
        exit(0);
    }
    printf("Child\n");
    return 0;
}
{% endhighlight %}


2) 调试步骤

首先通过执行下面的命令执行编译：
<pre>
# gcc -g -c test.c
# gcc -o test test.o
</pre>

从前面我们知道，GDB默认情况下只会追踪父进程的运行，而子进程会独立运行，GDB不会控制。

如果同时调试父进程和子进程，可以使用```set detach-on-fork off```(默认值是on)命令，这样GDB就能同时调试父子进程，并且在调试一个进程时，另一个进程处于挂起状态。例如：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) show detach-on-fork
Whether gdb will detach the child of a fork is on.
(gdb) set detach-on-fork off
(gdb) start
Temporary breakpoint 1 at 0x4005cc: file test.c, line 8.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:8
8           pid = fork();
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
[New process 53415]
9           if (pid < 0)
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) info inferiors
  Num  Description       Executable        
  2    process 53415     /root/workspace/./test 
* 1    process 52646     /root/workspace/./test 
(gdb) n
13          else if (pid > 0)
(gdb) n
15              printf("Parent\n");
(gdb) n
Parent
16              exit(0);
(gdb) n
[Inferior 1 (process 52646) exited normally]
(gdb) n
The program is not being run.
(gdb) info inferiors
  Num  Description       Executable        
  2    process 53415     /root/workspace/./test 
* 1    <null>            /root/workspace/./test 
(gdb) inferior 2
[Switching to inferior 2 [process 53415] (/root/workspace/./test)]
[Switching to thread 2 (process 53415)] 
#0  0x00007ffff7ada74c in fork () from /lib64/libc.so.6
(gdb) bt
#0  0x00007ffff7ada74c in fork () from /lib64/libc.so.6
#1  0x00000000004005d1 in main (argc=1, argv=0x7fffffffe638) at test.c:8
(gdb) n
Single stepping until exit from function fork,
which has no line number information.
main (argc=1, argv=0x7fffffffe638) at test.c:9
9           if (pid < 0)
(gdb) n
13          else if (pid > 0)
(gdb) n
18          printf("Child\n");
(gdb) n
Child
19          return 0;
(gdb) 
{% endhighlight %}

上面在使用```set detach-on-fork off```命令之后，使用```info inferiors```命令查看进程状态，可以看到父进程处在被GDB调试的状态（前面显示```*```表示正在被调试）。当父进程退出后，用```inferior infno```切换到子进程去调试。

此外，如果想让父子进程同时运行，可以使用```set schedule-multiple on```(默认值为off)命令，仍以上述代码为例：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) set detach-on-fork off
(gdb) set schedule-multiple on
(gdb) start
Temporary breakpoint 1 at 0x4005cc: file test.c, line 8.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:8
8           pid = fork();
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
[New process 54810]
Child
[process 54810 exited]
9           if (pid < 0)
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb)
{% endhighlight %}
可以看到打印出了```Child```，证明子进程也在运行了。


## 3. 设置用于返回的书签
在许多操作系统上，GDB能够将程序的运行状态保存为snapshot，这被称为```checkpoint```，后续我们就可以通过相应的命令返回到该checkpoint。

回退到一个```checkpoint```，会使得所有发生在该checkpoint之后的操作都会被做undo。这包括内存的修改、寄存器的修改、甚至是系统的状态（有一些限制）。事实上，类似于回到保存checkpoint的时间点。

因此，当你在单步调试程序，并且认为快接近有错误的代码点时，你就可以先保存一个checkpoint。然后，你继续进行调试，假如碰巧错过了该关键代码段，这时你就可以回退到该checkpoint并从该位置继续进行调试，而不用完全从头开始来调试整个程序。

要使用```checkpoint/restart```方法来进行调试的话，需要用到如下命令：

* **checkpoint**: 将调试程序的当前执行状态保存为一个snapshot。本命令不携带任何参数，但是其实GDB内部对于每一个checkpoint都会指定一个整数ID，这有些类似于breakpoint ID.

* **info checkpoints**: 列出当前调试session所保存的checkpoints。对于每一个checkpoint，都会有如下信息被列出
<pre>
Checkpoint ID
Process ID
Code Address
Source line, or label
</pre>

* **restart checkpoint-id**: 重新装载```checkpoint-id```位置的程序状态。所有的程序变量、寄存器、栈帧等都会被恢复为在保存该checkpoint时的状态。实际上，GDB类似于将时间拨回到保存该checkpoint的时间点。

注意，对于breakpoints、GDB variables、command history等，在执行恢复到某个checkpoint时并不会受到影响。一般来说，checkpoint只存储调试程序的信息，而并不存储调试器本身的信息。

* **delete checkpoint checkpoint-id**: 删除以前保存的某个checkpoint

<br />
返回到前一个保存的checkpoint时，将会恢复该调试程序的用户状态，也会恢复一部分的操作系统状态，包括文件指针。恢复时，并不会对一个文件中的数据执行```un-write```操作，但是会将文件指针恢复到原来的位置，因此之前所写的数据可以被```overwritten```。对于那些以读模式打开的文件，文件指针将会恢复到原来所读的位置。

当然，对于那些已经发送到打印机（或其他外部设备）的字符将不能够```snatched back```，而对于从外部设备（例如串口设备）接收到字符则从内部程序缓冲中移除，但是并不能```push back```回串行设备的pipeline中。相似的，对于文件的数据发生了实质性的更改这一情况，也是不能进行恢复。

然而，即使有上面的这些限制，你还是可以返回到checkpoint处开始进行调试，此时可能还可以调试一条不同的执行路径。

最后，当你回退到checkpoint时，程序会回退到上次保存时的状态，但是进程ID会发生改变。每一个checkpoint都会有一个唯一的进程ID，并且会与原来程序的进程ID不同。假如你所调试的程序在本地保存了进程ID的话，则可能会出现一些潜在的问题。

### 3.1 使用checkpoint的潜在优势
在有一些系统上，比如GNU/Linux，通常情况下由于安全原因每一个新进程的地址空间都是随机的。这就使得几乎不太可能在一个绝对的地址上设置一个breakpoint或者watchpoint，因为在程序下一次重启时，程序中symbol的绝对路径可能发生改变。

然而一个checkpoint，等价于一个进程拷贝。因此假如你在main的开始就创建一个checkpoint，后续返回到该checkpoint而不是重启程序，这就可以避免受到重启程序地址随机这一情况的影响。通过返回checkpoint，可以使得程序的```symbols```仍保持在原来的位置


### 3.2 checkpoint使用示例

**1） 示例程序**
{% highlight string %}
#include <stdlib.h>
#include <stdio.h>

static int func()
{
    static int i = 0;
    ++i;

    if (i == 2) {
        return 1;
    }
    return 0;
}

static int func3()
{
    return func();
}

static int func2()
{
    return func();
}

static int func1()
{
    return func();
}

int main()
{
    int ret = 0;

    ret += func1();
    ret += func2();
    ret += func3();

    return ret;
}
{% endhighlight %}

**2) 调试技巧**

首先采用如下的命令编译程序：
<pre>
# gcc -g -c test.c
# gcc -o test test.o
</pre>

下面我们进行调试，在```ret += func1()```前保存一个checkpoint:
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x400551: file test.c, line 32.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main () at test.c:32
32          int ret = 0;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
34          ret += func1();
(gdb) checkpoint
checkpoint: fork returned pid 68595.
(gdb) info checkpoints
  1 process 68595 at 0x400558, file test.c, line 34
* 0 process 68591 (main process) at 0x400558, file test.c, line 34
{% endhighlight %}

然后使用```next```步进，并每次调用完毕，打印ret的值：
{% highlight string %}
(gdb) n
35          ret += func2();
(gdb) p ret
$1 = 0
(gdb) n
36          ret += func3();
(gdb) p ret
$2 = 1
{% endhighlight %}
结果发现，在调用```func2()```后，ret的值变为了1。可是此时，我们已经错过了调试```fun2()```的机会。如果没有```checkpoint```，就需要再次从头调试了。对于这个问题从头调试很容易，但是对于很难复现的bug可能就会比较困难了。

下面我们使用checkpoint恢复：
{% highlight string %}
(gdb) info checkpoints
  1 process 68595 at 0x400558, file test.c, line 34
* 0 process 68591 (main process) at 0x400572, file test.c, line 36
(gdb) restart 1
Switching to process 68595
#0  main () at test.c:34
34          ret += func1();
{% endhighlight %}


上面我们看到，GDB恢复到了保存checkpoint时的状态了。上面```restart 1```中1为checkpoint的ID号。

从上面我们看出checkpoint的用法很简单，但是很有用。就是在平时的简单的bug修复中，也可以加快我们的调试速度，毕竟减少了不必要的重现bug的时间。





<br />
<br />

**[参看]**


1. [GDB Inferior Tutorial](http://moss.cs.iit.edu/cs351/gdb-inferiors.html)

2. [gdb调试多进程与多线程](https://blog.csdn.net/snow_5288/article/details/72982594)

3. [100个gdb小技巧](https://www.kancloud.cn/wizardforcel/gdb-tips-100/146771)

4. [gdb 调试入门，大牛写的高质量指南](http://blog.jobbole.com/107759/)

5. [GDB技巧：使用checkpoint解决难以复现的Bug](http://blog.chinaunix.net/uid-23629988-id-2943273.html)

<br />
<br />
<br />





