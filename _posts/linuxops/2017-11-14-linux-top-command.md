---
layout: post
title: Linux top命令的使用
tags:
- LinuxOps
categories: linuxOps
description: Linux top命令的使用
---


本文件简单介绍一下Linux环境下top命令的使用。

<!-- more -->

## 1. top命令

top命令输出：
<pre>
# top
top - 14:39:02 up 1 day,  2:46,  4 users,  load average: 0.00, 0.00, 0.00
Tasks: 243 total,   1 running, 242 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.2 us,  0.2 sy,  0.0 ni, 99.4 id,  0.2 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem:   8093596 total,  7201400 used,   892196 free,     1000 buffers
KiB Swap:  8000508 total,      880 used,  7999628 free.  1732676 cached Mem

  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
 2079 root     20   0 1598784 124000  71432 S   1.3  1.5   8:59.30 compiz
    1 root      20   0   33760   4256   2728 S   0.0  0.1   0:01.33 init
    2 root      20   0       0      0      0 S   0.0  0.0   0:00.02 kthreadd
    3 root      20   0       0      0      0 S   0.0  0.0   0:00.94 ksoftirqd/0
    5 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/0:0H
    7 root      20   0       0      0      0 S   0.0  0.0   0:24.76 rcu_sched
    8 root      20   0       0      0      0 S   0.0  0.0   0:00.00 rcu_bh
    9 root      rt   0       0      0      0 S   0.0  0.0   0:00.00 migration/0
   10 root      rt   0       0      0      0 S   0.0  0.0   0:00.33 watchdog/0
   11 root      rt   0       0      0      0 S   0.0  0.0   0:00.33 watchdog/1
   12 root      rt   0       0      0      0 S   0.0  0.0   0:00.01 migration/1
   13 root      20   0       0      0      0 S   0.0  0.0   0:01.03 ksoftirqd/1
   15 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/1:0H
   16 root      rt   0       0      0      0 S   0.0  0.0   0:00.32 watchdog/2
   17 root      rt   0       0      0      0 S   0.0  0.0   0:00.00 migration/2
   18 root      20   0       0      0      0 S   0.0  0.0   0:00.72 ksoftirqd/2
   20 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/2:0H
   21 root      rt   0       0      0      0 S   0.0  0.0   0:00.33 watchdog/3
   22 root      rt   0       0      0      0 S   0.0  0.0   0:00.01 migration/3
   23 root      20   0       0      0      0 S   0.0  0.0   0:01.28 ksoftirqd/3
   26 root      20   0       0      0      0 S   0.0  0.0   0:00.00 kdevtmpfs  
</pre>

上面top输出包含两个大的部分：

* SUMMARY Display

* FIELDS / Columns

### 1.1 SUMMARY Display

总结性显示部分一般又包含3个区域，每一个区域其实都是可以通过相应的交互命令控制。

**(1) UPTIME and LOAD Averages**
<pre>
top - 14:39:02 up 1 day,  2:46,  4 users,  load average: 0.00, 0.00, 0.00
</pre>

这一部分一般是一个单独的行，包含： 

* program或window名称（这取决于显示模式）

* 当前时间和操作系统启动以来到现在的时长

* 总用户数

* 过去1分钟，5分钟，15分钟的系统平均负载

注： 其实我们可以通过```uptime```命令来查看系统启动时间
<pre>
# uptime
 01:59:31 up  1:08,  2 users,  load average: 0.00, 0.01, 0.01
</pre>

<br />

**(2) TASK and CPU States**
<pre>
Tasks: 243 total,   1 running, 242 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.2 us,  0.2 sy,  0.0 ni, 99.4 id,  0.2 wa,  0.0 hi,  0.0 si,  0.0 st
</pre>
这一部分一般至少包含2行，在SMP(Symmetrical Multi-Processing)环境下，可能会有一些额外的行反映每个CPU的相应状态。

第一行显示 总的任务数或线程数（这依赖于线程模式切换的状态），这可以分成以下几类：running, sleeping, stopped, zombie.

第二行显示 自上一次刷新以来CPU的状态百分比。默认情况下，会显示如下条目：

* **us, user:** 普通用户进程运行在用户空间的时间占总CPU时间的百分比(un-niced user processes)

* **sy, system:** 所有进程运行在内核空间的时间占总CPU时间的百分比

* **ni, nice:** 特权用户进程的运行时间(niced user processes)

* **id, idle:** 内核空闲时间

* **wa, IO-wait:** 等待IO完成所花费的时间（IO时间）

* **hi:** 服务硬件中断所消耗的时间

* **si:** 服务软件中断所消耗的时间

* **st:**  hypervisor 服务另一个虚拟处理器的时候，虚拟 CPU 等待实际 CPU 的时间的百分比

<br />

{% highlight string %}
1. Linux nice命令以更改过的优先序来执行程序

2. st 的全称是 Steal Time ，就是 Xen Hypervisor 分配给运行在其它虚拟机上的任务的实际 CPU 时间。
%st(Steal time) 是当 hypervisor 服务另一个虚拟处理器的时候，虚拟 CPU 等待实际 CPU 的时间的百分比。
Steal 值比较高的话，需要向主机供应商申请扩容虚拟机。服务器上的另一个虚拟机拥有更大更多的 CPU 时间片，需要申请升级以与之竞争。
另外，高 steal 值也可能意味着主机供应商在服务器上过量地出售虚拟机。如果升级了虚拟机， steal 值还是不降的话，应该寻找另一家服务供应商。
低 steal 值意味着应用程序在目前的虚拟机上运作良好。因为虚拟机不会经常地为了 CPU 时间与其它虚拟机激烈竞争，虚拟机会更快地响应。
主机供应商没有过量地出售虚拟服务，绝对是一件好事情。
{% endhighlight %}


**(3) MEMORY Usage**
<pre>
KiB Mem:   8093596 total,  7201400 used,   892196 free,     1000 buffers
KiB Swap:  8000508 total,      880 used,  7999628 free.  1732676 cached Mem
</pre>
这一个部分包含两行，单位可以是KiB到EiB,这依赖于使用top时所采用的选项。

默认情况下，第一行反映的```物理内存```的情况，一般包含如下几类： total, free, used, buff/cache

第二行反映的是虚拟内存的情况，一般包含如下几类：total, free, used, avail。

关于```cached Mem```，这里再做一个说明：
<pre>
cached Mem用于指示缓冲的交换区总量。内存中的内容被换出到交换区,而后又被换入到内存,
但使用过的交换区尚未被覆盖。该数值即为这些内容已存在于内存中的交换区的大小，相应的
内存再次被换出时可不必再对交换区写入。
</pre>


<br />

{% highlight string %}
第二行的"avail" 一般是针对可用于启动新应用程序的物理内存的评估。
{% endhighlight %}

### 1.2 3. FIELDS / Columns

<pre>
  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
 2079 midea     20   0 1598784 124000  71432 S   1.3  1.5   8:59.30 compiz
    1 root      20   0   33760   4256   2728 S   0.0  0.1   0:01.33 init
    2 root      20   0       0      0      0 S   0.0  0.0   0:00.02 kthreadd
    3 root      20   0       0      0      0 S   0.0  0.0   0:00.94 ksoftirqd/0
    5 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/0:0H
    7 root      20   0       0      0      0 S   0.0  0.0   0:24.76 rcu_sched
    8 root      20   0       0      0      0 S   0.0  0.0   0:00.00 rcu_bh
    9 root      rt   0       0      0      0 S   0.0  0.0   0:00.00 migration/0
   10 root      rt   0       0      0      0 S   0.0  0.0   0:00.33 watchdog/0
   11 root      rt   0       0      0      0 S   0.0  0.0   0:00.33 watchdog/1
   12 root      rt   0       0      0      0 S   0.0  0.0   0:00.01 migration/1
   13 root      20   0       0      0      0 S   0.0  0.0   0:01.03 ksoftirqd/1
   15 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/1:0H
   16 root      rt   0       0      0      0 S   0.0  0.0   0:00.32 watchdog/2
   17 root      rt   0       0      0      0 S   0.0  0.0   0:00.00 migration/2
   18 root      20   0       0      0      0 S   0.0  0.0   0:00.72 ksoftirqd/2
   20 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/2:0H
   21 root      rt   0       0      0      0 S   0.0  0.0   0:00.33 watchdog/3
   22 root      rt   0       0      0      0 S   0.0  0.0   0:00.01 migration/3
   23 root      20   0       0      0      0 S   0.0  0.0   0:01.28 ksoftirqd/3
   26 root      20   0       0      0      0 S   0.0  0.0   0:00.00 kdevtmpfs  
</pre>

下面简要对如下各字段进行介绍：

* **PID:** 进程ID

* **USER:** The effective user name of the task's owner

* **PR:** 任务调度的优先权(假如显示为rt，表示实时调度）

* **NI:** 该任务的nice值。假如nice值小于0，表明其具有一个更高的优先权；大于0，表明其具有一个更低的优先权；等于0，表明任务调度时优先级别并不会被调整。

* **VIRT:** 该任务所使用的虚拟内存大小

* **RES:** 该任务所使用的物理内存大小（resident）

* **SHR:** 该任务可用的共享内存大小（通常并不是可用物理内存大小）

* **%CPU:** 该任务耗费的CPU时间百分比

* **%MEM:** 该任务所占用的物理内存百分比

* **TIME+:** 该任务自启动以来所耗费的CPU时间

* **COMMAND:** 启动该任务所使用的命令行

注：
<pre>
top命令默认按CPU占用的大小进行排序，我们可以在top界面输入M来按内存排序，
输入P来按CPU占用排序。
</pre>



<br />
<br />

**[参看]:**

1. [linux TOP命令各参数详解](http://www.cnblogs.com/sbaicl/articles/2752068.html)


<br />
<br />
<br />


