---
layout: post
title: 系统级性能分析工具perf的介绍与使用（转）
tags:
- LinuxOps
categories: linuxOps
description: 系统级性能分析工具perf的介绍与使用
---


系统级性能优化通常包括两个阶段：性能剖析(performance profiling)和代码优化。性能剖析的目标是寻找性能瓶颈，查找引发性能问题的原因及热点代码； 代码优化的目标是针对具体性能问题而优化代码或编译选项，以改善软件性能。

在性能剖析阶段，需要借助于现有的profiling工具，如```perf```等。在代码优化阶段往往需要借助开发者的经验，编写简洁高效的代码，甚至在汇编级别合理使用各种指令，合理安排各种指令的执行顺序。

perf是一款Linux性能分析工具。Linux性能计数器是一个新的基于内核的子系统，它提供一个性能分析框架，比如硬件（CPU、PMU(Performance Monitor Unit))功能和软件（软件计数器、tracepoint)功能。通过perf，应用程序可以利用PMU、tracepoint和内核中的计数器来进行性能统计。它不但可以分析指定应用程序的性能问题，也可以用来分析内核的性能问题，当然也可以同时分析应用程序和内核，从而全面理解应用程序中的性能瓶颈。


使用perf，既可以分析程序运行期间的硬件事件，比如instruction retired、processor clock cycles等； 也可以分析软件事件，比如page fault和进程切换。perf是一款综合性分析工具，大到系统全局性性能，小到进程线程级别，甚至到函数及汇编级别。

当前我们的操作系统环境为：
<pre>
# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core) 

# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>

<!-- more -->

## 1. 背景知识

### 1.1 tracepoints
tracepoints是散落在内核源代码中的一些hook，它们可以在特定的代码被执行到时触发，这一特性可以被各种trace/debug工具所使用。perf将tracepoint产生的时间记录下来，生成报告，通过分析这些报告，调优人员便可以了解程序运行期间内核的各种细节，对性能症状做出准确的诊断。

这些tracepoint对应的sysfs节点在/sys/kernel/debug/tracing/events目录下。
{% highlight string %}
# ls /sys/kernel/debug/tracing/events
block             enable      filemap       header_page  kmem     module  nfsd     printk        rcu     scsi    sunrpc    udp        writeback
compaction        exceptions  ftrace        iommu        libata   mpx     oom      random        regmap  signal  syscalls  vmscan     xen
context_tracking  fence       gpio          irq          mce      napi    pagemap  ras           rpm     skb     task      vsyscall   xfs
drm               filelock    header_event  irq_vectors  migrate  net     power    raw_syscalls  sched   sock    timer     workqueue  xhci-hcd
{% endhighlight %}


### 1.2 硬件之cache
内存读写是很快的，但是还是无法和处理器指令执行速度相比。为了从内存中读取指令和数据，处理器需要等待，用处理器时间来衡量，这种等待非常漫长。cache是一种SRAM，读写速度非常快，能和处理器相匹配。因此，将常用的数据保存在cache中，处理器便无需等待，从而提高性能。cache的尺寸一般都很小，充分利用cache是软件调优非常重要的部分。

## 2. 主要关注点
基于性能分析，可以进行算法优化（空间复杂度和时间复杂度权衡）、代码优化（提高执行速度，减少内存占用）。评估程序对硬件资源的使用情况，例如各级cache的访问次数、各级cache的丢失次数、流水线停顿周期、前端总线访问次数等。评估程序对操作系统资源的使用情况，系统调用次数、上下文切换次数、任务迁移次数。

事件可以分为如下三种：

* Hardware Event由PMU部件产生，在特定的条件下探测性能事件是否发生以及发生的次数。比如cache命中

* Software Event是内核产生的事件，分布在各个功能模块中，统计与操作系统相关的性能事件。比如进程切换、tick数等

* Tracepoint Event是内核中静态tracepoint所触发的事件，这些tracepoint用来判断程序运行期间内核的行为细节，比如slab分配器的分配次数等。

## 3. perf的使用
当前我们环境可以直接通过如下命令来安装perf：
<pre>
# yum install perf
# perf --help

 usage: perf [--version] [--help] [OPTIONS] COMMAND [ARGS]

 The most commonly used perf commands are:
   annotate        Read perf.data (created by perf record) and display annotated code
   archive         Create archive with object files with build-ids found in perf.data file
   bench           General framework for benchmark suites
   buildid-cache   Manage build-id cache.
   buildid-list    List the buildids in a perf.data file
   c2c             Shared Data C2C/HITM Analyzer.
   config          Get and set variables in a configuration file.
   data            Data file related processing
   diff            Read perf.data files and display the differential profile
   evlist          List the event names in a perf.data file
   ftrace          simple wrapper for kernel's ftrace functionality
   inject          Filter to augment the events stream with additional information
   kallsyms        Searches running kernel for symbols
   kmem            Tool to trace/measure kernel memory properties
   kvm             Tool to trace/measure kvm guest os
   list            List all symbolic event types
   lock            Analyze lock events
   mem             Profile memory accesses
   record          Run a command and record its profile into perf.data
   report          Read perf.data (created by perf record) and display the profile
   sched           Tool to trace/measure scheduler properties (latencies)
   script          Read perf.data (created by perf record) and display trace output
   stat            Run a command and gather performance counter statistics
   test            Runs sanity tests.
   timechart       Tool to visualize total system behavior during a workload
   top             System profiling tool.
   probe           Define new dynamic tracepoints
   trace           strace inspired tool

 See 'perf help COMMAND' for more information on a specific command.
</pre>
下面我们对各选项进行一个简单的说明：

* 














<br />
<br />

**[参看]:**

1. [系统级性能分析工具perf的介绍与使用](https://www.cnblogs.com/arnoldlu/p/6241297.html)

2. [Linux kernel profiling with perf](https://perf.wiki.kernel.org/index.php/Tutorial)

3. [Perf使用教程](http://blog.chinaunix.net/uid-10540984-id-3854969.html)

4. [perf命令](https://www.cnblogs.com/xiaogongzi/p/8473821.html)

5. [<<Linux Perf Master>>电子书](https://cloud.tencent.com/developer/article/1019745)

<br />
<br />
<br />


