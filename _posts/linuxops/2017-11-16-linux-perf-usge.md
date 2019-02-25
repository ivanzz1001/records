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


















<br />
<br />

**[参看]:**

1. []()



<br />
<br />
<br />


