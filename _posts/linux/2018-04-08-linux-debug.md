---
layout: post
title: Linux下top+pstack+gdb的组合拳定位程序进程线程问题
tags:
- LinuxOps
categories: linux
description: linux调试
---

在Linux下如果发现程序运行异常，比如CPU、内存占用很高，可以采用Linux下自带的一些命令，帮助我们找到问题所在。

<!-- more -->


## 1. top+pstack+gdb的组合拳
闲言少说，先直接上操作实例，再做原理讲解。

### 1.1 用top命令找到最占CPU的进程
<pre>
# top
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND          
22688 root      20   0 1842m 136m  13m S 110.0  0.9   1568:44 test-program 
</pre>


### 1.2 使用pstack跟踪进程栈
pstack是一个shell脚本，用于打印正在运行的进程的栈跟踪信息，它实际上是gstack的一个链接，而gstack本身是基于gdb封装的shell脚本。此命令可显示每个进程的栈跟踪。

```pstack```命令必须由相应进程的属主或者root运行，我们可以使用pstack来确定进程挂起的位置。此命令允许使用的唯一选项就是要检查的进程的PID。与```jstack```相比，它能对潜在的**死锁**予以提示，而pstack只提供了线索，需要gdb进一步的确定。

pstack是gdb的一部分，如果系统没有pstack命令，使用yum搜索安装gdb即可。

这个命令在排查进程问题时非常有用，比如我们发现一个服务一直处于work状态（如假死状态，好似死循环），使用这个命令就能轻松定位为题所在。我们可以在一段时间内，多次执行pstack，若发现代码栈总是停在同一个位置，那个位置就需要重点关注，和可能就是出问题的地方。
{% highlight string %}
$ pstack 22688
Thread 44 (Thread 0x7fa97035f700 (LWP 22689)):
#0  0x00007fa96f386a00 in sem_wait () from /lib64/libpthread.so.0
#1  0x0000000000dfef12 in uv_sem_wait ()
#2  0x0000000000d67832 in node::DebugSignalThreadMain(void*) ()
#3  0x00007fa96f380aa1 in start_thread () from /lib64/libpthread.so.0
#4  0x00007fa96f0cdaad in clone () from /lib64/libc.so.6
Thread 43 (Thread 0x7fa96efe4700 (LWP 22690)):
#0  0x00007fa96f386a00 in sem_wait () from /lib64/libpthread.so.0
#1  0x0000000000e08a38 in v8::base::Semaphore::Wait() ()
#2  0x0000000000dddde9 in v8::platform::TaskQueue::GetNext() ()
#3  0x0000000000dddf3c in v8::platform::WorkerThread::Run() ()
#4  0x0000000000e099c0 in v8::base::ThreadEntry(void*) ()
#5  0x00007fa96f380aa1 in start_thread () from /lib64/libpthread.so.0
#6  0x00007fa96f0cdaad in clone () from /lib64/libc.so.6
Thread 42 (Thread 0x7fa96e5e3700 (LWP 22691)):
#0  0x00007fa96f386a00 in sem_wait () from /lib64/libpthread.so.0
#1  0x0000000000e08a38 in v8::base::Semaphore::Wait() ()
#2  0x0000000000dddde9 in v8::platform::TaskQueue::GetNext() ()
#3  0x0000000000dddf3c in v8::platform::WorkerThread::Run() ()
#4  0x0000000000e099c0 in v8::base::ThreadEntry(void*) ()
#5  0x00007fa96f380aa1 in start_thread () from /lib64/libpthread.so.0
#6  0x00007fa96f0cdaad in clone () from /lib64/libc.so.6
Thread 41 (Thread 0x7fa96dbe2700 (LWP 22692)):
#0  0x00007fa96f386a00 in sem_wait () from /lib64/libpthread.so.0
#1  0x0000000000e08a38 in v8::base::Semaphore::Wait() ()
#2  0x0000000000dddde9 in v8::platform::TaskQueue::GetNext() ()
#3  0x0000000000dddf3c in v8::platform::WorkerThread::Run() ()
#4  0x0000000000e099c0 in v8::base::ThreadEntry(void*) ()
#5  0x00007fa96f380aa1 in start_thread () from /lib64/libpthread.so.0
#6  0x00007fa96f0cdaad in clone () from /lib64/libc.so.6
{% endhighlight %}

然后使用top命令查看指定进程最耗CPU的线程：
<pre>
# top -H -p 22688
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND    
22970 root      20   0 1842m 136m  13m R  100.2 0.9 1423:40 test-program
</pre>
从上面我们看到，找到的最耗CPU的线程为```22970```。

>注： 这里的PID是系统给每个线程分配的唯一的线程号，不是进程号，但名称也是PID。这两者的具体区别可见[linux中pid，tid， 以及 真实pid的关系](http://blog.csdn.net/u012398613/article/details/52183708)

接着我们使用线程号PID反查该线程在进程中的内部ID标识：
<pre>
# pstack 22688 | grep 22970
Thread 10 (Thread 0x7fa92f5fe700 (LWP 22970)):
</pre>
从上面我们就找到了线程22970对应的是线程10。


之后，我们使用```vim```查看进程快照，定位到具体的线程，并查看其调用堆栈：
{% highlight string %}
$ pstack 22688 | vim -
Thread 10 (Thread 0x7fa92f5fe700 (LWP 22970)):
#0  0x00007fa96f02a04f in vfprintf () from /lib64/libc.so.6
#1  0x00007fa96f054712 in vsnprintf () from /lib64/libc.so.6
#2  0x00007fa967b3861c in lv_write_log () from /opt/test-program
#3  0x00007fa967b26173 in LvJbuf::pjmedia_jbuf_put_rtp_pkg(pjmedia_rtp_decoded_pkg const*, int*) () from /opt/test-program
#4  0x00007fa96782409f in livesrv::LvAudio::on_rtp_stream(void*, unsigned int, unsigned int) () from /opt/test-program
#5  0x00007fa96781fc87 in livesrv::LvMedia::recv_media(void*, unsigned int, unsigned char, unsigned int) () from /opt/test-program
#6  0x00007fa967818c7f in livesrv::LvChannel::do_recv_media_check_thread2() () from /opt/test-program/node_modules/livesource/Debug/linux/livesource.node
#7  0x00007fa967814699 in recv_media_process2(void*) () from /opt/test-program
#8  0x00007fa96f380aa1 in start_thread () from /lib64/libpthread.so.0
#9  0x00007fa96f0cdaad in clone () from /lib64/libc.so.6
{% endhighlight %}
上面的操作基本定位到了具体线程和大概的函数。如果想查看具体的原因，如现场的函数中变量的数值等，就要使用GDB的实时调试功能。

### 1.3 使用GDB实时调试进程
{% highlight string %}
>gdb attach 22688
:thread 10
:bt
:frame x
:p xxx
{% endhighlight %}





<br />
<br />
**[参看]:**

1. [Linux下top+pstack+gdb的组合拳定位程序进程线程问题并调试](http://www.yihaomen.com/article/linux/715.htm)

2. [gdb如何确定内存 已经释放](https://blog.csdn.net/weixin_36356040/article/details/112432311)

3. [LD_PRELOAD作用](https://blog.csdn.net/chen_jianjian/article/details/80627693)

4. [LD_PRELOAD基础用法](http://work.loidair.com/index.php/2020/07/19/ld_preload%E5%9F%BA%E7%A1%80%E7%94%A8%E6%B3%95/)

<br />
<br />
<br />





