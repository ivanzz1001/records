---
layout: post
title: os/unix/ngx_process_cycle.h源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本文主要介绍ngx_process_cycle.h头文件，其主要是定义了nginx主进程与工作进程交互的相关命令、主进程循环函数的声明等。
<!-- more -->


<br />
<br />

## 1. 相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PROCESS_CYCLE_H_INCLUDED_
#define _NGX_PROCESS_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_CMD_OPEN_CHANNEL   1
#define NGX_CMD_CLOSE_CHANNEL  2
#define NGX_CMD_QUIT           3
#define NGX_CMD_TERMINATE      4
#define NGX_CMD_REOPEN         5


#define NGX_PROCESS_SINGLE     0
#define NGX_PROCESS_MASTER     1
#define NGX_PROCESS_SIGNALLER  2
#define NGX_PROCESS_WORKER     3
#define NGX_PROCESS_HELPER     4
{% endhighlight %}

### 1.1 nginx进程间管道通信
nginx中各个进程之间是通过管道的方式来进行通信的，如下图所示：

![ngx-processes-comm](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_process_comm.jpg)

上面我们用三种不同颜色的线连接了各个进程，虽然都是一个channel，但是传递这些通道的方式可能有些不同：

* **红色**： nginx master在创建子进程时通过socketpair()创建了一对匿名管道，通过fork()自动将相应的文件描述符传递给子进程的

* **绿色**: 绿色之间的通道是在创建子进程时通过ngx_pass_open_channel()传递的。例如在创建worker-3时，nginx master分别通过ch1[0]、ch2[0]通道将ch3[0]文件描述符传递给worker-1和worker-2，这样worker-1与worker-2就能通过相应的文件描述符和worker-3进行通信了。

* **紫色**: 紫色之间的通道其实也是通过fork()函数自动传递的。例如在创建worker-2时，因为master与worker-1之间的ch1[0]文件描述符是已经存在的，因此新创建的worker-2进程自动的复制了该文件描述符，从而可以自动的获得worker-2到worker-1之间的通道ch1_p[0]（此文件描述符复制于ch1[0])。


我们了解了上述的通信流程，接下来我们介绍一下各相关命令：

* **NGX_CMD_OPEN_CHANNEL**: 打开一个通道，在创建子进程时向其他子进程传递文件描述符。如上面的绿线指示的通道，详见ngx_pass_open_channel()。

* **NGX_CMD_CLOSE_CHANNEL**: 当一个子进程退出时，就需要向其他子进程发送消息关闭对应的通道。例如worker-2进程退出，则master进程需要向worker-1和worker-3进程发送消息，告知关闭ch2_p[0]。请参看：ngx_reap_children()及ngx_channel_handler()

* **NGX_CMD_QUIT**： 用于通知子进程以优雅的方式退出。

* **NGX_CMD_TERMINATE**: 用于通知子进程快速退出

* **NGX_CMD_REOPEN**: 用于通知子进程进行相应的日志回滚。

关于这些命令，我们后续在讲到对应的函数时还会再进一步讲解。

### 1.2 nginx进程类型
<pre>
#define NGX_PROCESS_SINGLE     0
#define NGX_PROCESS_MASTER     1
#define NGX_PROCESS_SIGNALLER  2
#define NGX_PROCESS_WORKER     3
#define NGX_PROCESS_HELPER     4
</pre>
默认情况下nginx是以master-worker方式工作的。上面这些宏分别表示当前的进程类型：

* **NGX_PROCESS_SINGLE**: 只有在nginx.conf配置文件中设置为```master_process off```，且ngx_process值为0时，才有可能会进入单进程工作模式。

* **NGX_PROCESS_MASTER**: 用于指示当前nginx工作方式为master-worker，并且当前进程为master进程

* **NGX_PROCESS_SIGNALLER**: 我们可以通过运行```nginx -s reload```命令的方式来向正在运行的nginx进程发送信号。这种情况下，其实我们只是想发送一个信号，而并不是真正想要运行一个nginx服务器程序。因此这里我们用```NGX_PROCESS_SIGNALLER```来表明当前运行的nginx只是一个发送信号的进程。一般如下情况都只是起一个signaller进程：
<pre>
# nginx -s stop      //快速停止服务

# nginx -s quit      //优雅的停止服务

# nginx -s reopen    //进行日志回滚

# nginx -s reload    //告诉原nginx进程重新加载配置文件
</pre>

* **NGX_PROCESS_WORKER**： 表明当前进程是worker进程

* **NGX_PROCESS_HELPER**: 表明当前进程为辅助进程。例如缓存管理器是作为一个辅助进程。


## 2. 缓存管理器上下文
{% highlight string %}
typedef struct {
    ngx_event_handler_pt       handler;
    char                      *name;
    ngx_msec_t                 delay;
} ngx_cache_manager_ctx_t;
{% endhighlight %}
主要是用于保存缓存管理器进程的一个上下文数据：

* **handler**: 该缓存管理器所绑定的事件处理器

* **name**: 缓存管理器进程的名称

* **delay**: 给缓存管理器绑定的一个定时器延迟


## 3. 相关函数声明
{% highlight string %}
void ngx_master_process_cycle(ngx_cycle_t *cycle);     
void ngx_single_process_cycle(ngx_cycle_t *cycle);
{% endhighlight %}

第一个函数为nginx工作在master-worker方式下，master进程的主循环； 第二个函数为nginx工作在单进程方式下的主循环。

## 4. 相关变量的声明
{% highlight string %}
extern ngx_uint_t      ngx_process;
extern ngx_uint_t      ngx_worker;
extern ngx_pid_t       ngx_pid;
extern ngx_pid_t       ngx_new_binary;
extern ngx_uint_t      ngx_inherited;
extern ngx_uint_t      ngx_daemonized;
extern ngx_uint_t      ngx_exiting;

extern sig_atomic_t    ngx_reap;
extern sig_atomic_t    ngx_sigio;
extern sig_atomic_t    ngx_sigalrm;
extern sig_atomic_t    ngx_quit;
extern sig_atomic_t    ngx_debug_quit;
extern sig_atomic_t    ngx_terminate;
extern sig_atomic_t    ngx_noaccept;
extern sig_atomic_t    ngx_reconfigure;
extern sig_atomic_t    ngx_reopen;
extern sig_atomic_t    ngx_change_binary;
{% endhighlight %} 

这些变量都定义在os/unix/ngx_process_cycle.c文件中，下面分别简单介绍一下各变量：

* **ngx_process**: 用于指示当前进程是什么类型的进程，如在master-worker工作模式下，在master进程中其值为```NGX_PROCESS_MASTER```, 在worker进程中其值为```NGX_PROCESS_WORKER```。

* **ngx_worker**： 用于指示当前worker进程是第几个worker子进程

* **ngx_pid**: 用于保存当前进程的进程ID

* **ngx_new_binary**: 用于指示当前进程是否是热替换的进程（主要用于热升级）

* **ngx_inherited**: 此变量主要是在热升级的情况下，避免重新创建新的socket。因为socket文件描述符创建时只要没有设置close_on_exec，则执行exec替换后对应的文件描述符仍可以使用。

* **ngx_daemonized**: 用于指示当前nginx是否是以daemonized方式运行。nginx默认是以守护进程的方式来运行的；通过热升级的方式运行的nginx一般也是以守护进程的方式来运行。
<pre>
在热升级方式下，旧的nginx进程一般是先fork()出一个新的子进程，在子进程中执行新的nginx二进制文件。在升级完成之后，
旧的nginx就会优雅的退出。因此通过热升级的方式运行的nginx一般也是以守护进程的方式来运行的。
</pre>

* **ngx_exiting**: 用于指示当前子进程是否处于**正在退出**状态。

* **ngx_reap**: 用于指示回收对应的子进程

* **ngx_sigio**: 用于指示接收到SIGIO信号(目前只是对变量设置值，并未做其他用）

* **ngx_sigalrm**: 主要是用于nginx master收到TERM信号退出时的一个计时操作。通过setitimer()函数在定时到了之后会产生SIGALRM信号。

* **ngx_quit**: 用于指示优雅的关闭nginx

* **ngx_debug_quit**: 一般用作调试的时候，给某一个worker进程或辅助进程发送一个```NGX_NOACCEPT_SIGNAL```信号，然后会设置ngx_debug_quit值为1，接着再设置ngx_quit值为1。这样对应的子进程就会退出，并在退出时执行ngx_debug_point()暗示非正常退出。
{% highlight string %}
void
ngx_signal_handler(int signo)
{
    ...
    case ngx_signal_value(NGX_NOACCEPT_SIGNAL):
        if (!ngx_daemonized) {
            break;
        }
        ngx_debug_quit = 1;                 //注意此处并无break
    case ngx_signal_value(NGX_SHUTDOWN_SIGNAL):
        ngx_quit = 1;
        action = ", shutting down";
        break;
    ...
}
{% endhighlight %}

* **ngx_terminate**: 用于指示快速的终止nginx服务

* **ngx_noaccept**: 用于指示优雅的退出worker进程，对于master及其他辅助进程并不会产生影响。这与shutdown是不同的。

* **ngx_reconfigure**: 用于指示nginx重新读取配置文件

* **ngx_reopen**: 用于指示nginx日志文件回滚

* **ngx_change_binary**: 用于指示nginx进行热升级。

### 4.1 sig_atomic_t数据类型
当把变量声明为该类型时，则会保证该变量在使用或赋值时，无论是在32位还是64位的机器上都能保证操作是原子的。它会根据机器的类型自适应。```sig_atomic_t```这个数据类型是定义在signal.h头文件中的，下面类说说这个类型：

在处理信号(signal)的时候，有时对于一些变量的访问希望不会被中断，无论是硬件中断还是软件中断，这就要求访问或改变这些变量需要在计算机的一条指令内完成。通常情况下，int类型的变量一般是原子访问的，也可以认为sig_atomic_t是int类型的数据。因为对这些变量要求一条指令完成，所以sig_atomic_t不可能是结构体，只会是数字类型。在Linux是这样定义：
<pre>
typedef int __sig_atomic_t;
</pre>
另外GNU C的文档也说比int短的类型通常也具有原子性的。例如short类型。同时指针（地址）也一定是原子性的。该类型在所有GNU C库支持的系统和支持posix的系统中都有定义。

**注：** 关于int型是不是原子类型，可以参看如下文档
<pre>
http://www.gnu.org/s/hello/manual/libc/Atomic-Types.html
</pre>



### 4.2 ngx_inherited示例
关于```ngx_inherited```我们再给出如下例子：

程序1(test.c):
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc,char *argv[])
{

   int fd = -1;
   int sz;
   char buf[128];

   fd = open("./helloworld.txt",O_RDONLY);

   sz = read(fd,buf,5);
   if(sz < 0)
   {
      printf("read failure\n");
      exit(-1);
   }

  buf[sz] = 0;
  printf("test buf: %s\n",buf);

  sprintf(buf,"%d",fd);

  execl("./read","read",buf,NULL);

  return 0x0;

}
{% endhighlight %}

程序2(read.c):
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc,char *argv[])
{
    int fd = -1;
    char buf[128];
    int sz;

    if(argc < 2)
    {
       printf("argc must equal to 2\n");

       return -1;
    }

    fd = atoi(argv[1]);
    if(fd < 0)
    {
       printf("error fd:%d\n",fd);
       return -2;
    }

    sz = read(fd,buf,16);
    if(sz < 0)
    {
       printf("read error\n");
       return -3;
    }

    buf[sz] = 0;
    printf("read buf:%s\n",buf);

    return 0x0;
}
{% endhighlight %}

helloworld.txt:
<pre>
[root@localhost test-src]# cat helloworld.txt 
hello,world
</pre>
程序编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# gcc -o read read.c
[root@localhost test-src]# ./test
test buf: hello
read buf:,world

</pre>
通过上面我们可以看到文件描述符在执行exec后仍然有效（前提是并未加close_on_exec)。


<br />
<br />
**[参看]:**

1. [控制 Nginx 的基本功能的指令](http://www.linuxidc.com/Linux/2012-04/57908.htm)

2. [nginx运行期间修改配置文件的处理](http://blog.csdn.net/opens_tym/article/details/17270411)

3. [nginx学习十四 ngx_master_process_cycle(master进程)](http://blog.csdn.net/xiaoliangsky/article/details/40866855)

4. [ngx_add_inherited_sockets 继承的sockets](http://blog.csdn.net/huangyimo/article/details/50170657)

5. [nginx继承socket 和 热代码替换](http://blog.csdn.net/jiaoyongqing134/article/details/52127732)

6. [volatile 和 sig_atomic_t](https://www.cnblogs.com/GoodGoodWorkDayDayUp/archive/2011/05/14/2046082.html)
<br />
<br />
<br />

