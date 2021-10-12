---
layout: post
title: 为什么磁盘慢会导致Linux负载飙升(转)
tags:
- LinuxOps
categories: linuxOps
description: 为什么磁盘慢会导致Linux负载飙升
---


本文记录一下Linux系统中，CPU利用率和负载率相关方面的知识，在这里做一个记录。

<!-- more -->


## 1. CPU利用率和负载率的区别
<pre>
# top
top - 10:46:00 up 157 days, 23:25,  3 users,  load average: 0.27, 0.14, 0.14
Tasks: 177 total,   1 running, 176 sleeping,   0 stopped,   0 zombie
%Cpu(s):  1.0 us,  0.4 sy,  0.0 ni, 98.3 id,  0.3 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem : 16015124 total,   530104 free,  2155656 used, 13329364 buff/cache
KiB Swap:  8126460 total,  8099440 free,    27020 used. 12262088 avail Mem 

  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND                                                                     
    1 root      20   0  190924   3672   2172 S   0.0  0.0  34:04.67 systemd                                                                     
    2 root      20   0       0      0      0 S   0.0  0.0   0:04.66 kthreadd                                                                    
    3 root      20   0       0      0      0 S   0.0  0.0   0:43.62 ksoftirqd/0                                                                 
    5 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/0:0H                                                                
    7 root      rt   0       0      0      0 S   0.0  0.0   0:06.95 migration/0                                                                 
    8 root      20   0       0      0      0 S   0.0  0.0   0:00.00 rcu_bh                                                                      
    9 root      20   0       0      0      0 S   0.0  0.0  98:10.43 rcu_sched                                                                   
   10 root      rt   0       0      0      0 S   0.0  0.0   0:39.89 watchdog/0                                                                  
   11 root      rt   0       0      0      0 S   0.0  0.0   0:39.90 watchdog/1                                                                  
   12 root      rt   0       0      0      0 S   0.0  0.0   0:06.94 migration/1                                                                 
   13 root      20   0       0      0      0 S   0.0  0.0   0:42.34 ksoftirqd/1                                                                 
   15 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/1:0H                                                                
   16 root      rt   0       0      0      0 S   0.0  0.0   0:38.03 watchdog/2                                                                  
   17 root      rt   0       0      0      0 S   0.0  0.0   0:06.51 migration/2                                                                 
   18 root      20   0       0      0      0 S   0.0  0.0   0:41.72 ksoftirqd/2                                                                 
   20 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/2:0H                                                                
   21 root      rt   0       0      0      0 S   0.0  0.0   0:36.80 watchdog/3                                                                  
   22 root      rt   0       0      0      0 S   0.0  0.0   0:06.70 migration/3                                                                 
   23 root      20   0       0      0      0 S   0.0  0.0   0:41.07 ksoftirqd/3                                                                 
   25 root       0 -20       0      0      0 S   0.0  0.0   0:00.00 kworker/3:0H                                                                
   26 root      rt   0       0      0      0 S   0.0  0.0   0:35.77 watchdog/4 
</pre>

这里要区别```CPU负载```和```CPU利用率```，它们是不同的两个概念，但它们的信息可以在同一个top命令中进行显示。CPU利用率显示的是程序在运行期间实时占用的CPU百分比，这是对一个时间段内CPU使用状况的统计，通过这个指标可以看出在某一个时间段内CPU被占用的情况， 如果被占用时间很高，那么就需要考虑CPU是否已经处于超负荷运作。而CPU负载显示的是在一段时间内CPU正在处理以及等待CPU处理的进程数之和的统计信息，也就是CPU使用队列的长度的统计信息。


CPU利用率高并不意味着负载就一定大，可能这个任务是一个CPU密集型的。一样CPU低利用率的情况下是否会有高Load Average的情况产生呢？理解```占有时间```和```使用时间```就可以知道，当CPU分配时间片以后，是否使用完全取决于使用者，因此完全可能出现低利用率高Load Average的情况。另外IO设备也可能导致CPU负载高。

由此来看，仅仅从CPU的使用率来判断CPU是否处于一种超负荷的工作状态还是不够的，必须结合Load Average来全局的看CPU的使用情况。网上有个例子来说明两者的区别如下：某公用电话亭，有一个人在打电话，四个人在等待，每人限定使用电话一分钟，若有人一分钟之内没有打完电话，只能挂掉电话去排队，等待下一轮。```电话```在这里就相当于```CPU```，而正在或等待打电话的人就相当于任务数。在电话亭使用过程中，肯定会有人打完电话走掉，有人没有打完电话而选择重新排队，更会有新增的人在这儿排队，这个人数的变化就相当于任务数的增减。为了统计平均负载情况，我们```5秒钟```统计一次人数，并在第1、5、15分钟的时候对统计情况取平均值，从而形成第1、5、15分钟的平均负载。有的人拿起电话就打，一直打完1分钟，而有的人可能前三十秒在找电话号码，或者在犹豫要不要打，后三十秒才真正在打电话。如果把电话看作CPU，人数看作任务，我们就说前一个人（任务）的CPU利用率高，后一个人（任务）的CPU利用率低。当然， CPU并不会在前三十秒工作，后三十秒歇着，CPU是一直在工作。只是说，有的程序涉及到大量的计算，所以CPU利用率就高，而有的程序牵涉到计算的部分很少，CPU利用率自然就低。但无论CPU的利用率是高是低，跟后面有多少任务在排队没有必然关系。


CPU数量和CPU核心数（即内核数）都会影响到CPU负载，因为任务最终是要分配到CPU核心去处理的。两块CPU要比一块CPU好，双核要比单核好。因此，我们需要记住，除去CPU性能上的差异，CPU负载是基于内核数来计算的，即“有多少内核，即有多少负载”，如单核最好不要超过100%，也就是负载为1.00，如此类推。

Linux里有一个```/proc```目录，存放的是当前运行系统的虚拟映射，其中有一个文件为cpuinfo，这个文件里存放着CPU的信息。```/proc/cpuinfo```文件按逻辑CPU而非真实CPU分段落显示信息，每个逻辑CPU的信息占用一个段落，第一个逻辑CPU标识从0开始。

<pre>
# cat /proc/cpuinfo 
processor       : 0
vendor_id       : GenuineIntel
cpu family      : 6
model           : 60
model name      : Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz
stepping        : 3
microcode       : 0x20
cpu MHz         : 3500.000
cache size      : 8192 KB
physical id     : 0
siblings        : 8
core id         : 0
cpu cores       : 4
apicid          : 0
initial apicid  : 0
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm ida arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid xsaveopt
bogomips        : 6984.00
clflush size    : 64
cache_alignment : 64
address sizes   : 39 bits physical, 48 bits virtual
power management:

processor       : 1
vendor_id       : GenuineIntel
cpu family      : 6
model           : 60
model name      : Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz
stepping        : 3
microcode       : 0x20
cpu MHz         : 3500.000
cache size      : 8192 KB
physical id     : 0
siblings        : 8
core id         : 1
cpu cores       : 4
apicid          : 2
initial apicid  : 2
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm ida arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid xsaveopt
bogomips        : 6984.00
clflush size    : 64
cache_alignment : 64
address sizes   : 39 bits physical, 48 bits virtual
power management:

processor       : 2
vendor_id       : GenuineIntel
cpu family      : 6
model           : 60
model name      : Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz
stepping        : 3
microcode       : 0x20
cpu MHz         : 3528.027
cache size      : 8192 KB
physical id     : 0
siblings        : 8
core id         : 2
cpu cores       : 4
apicid          : 4
initial apicid  : 4
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm ida arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid xsaveopt
bogomips        : 6984.00
clflush size    : 64
cache_alignment : 64
address sizes   : 39 bits physical, 48 bits virtual
power management:

processor       : 3
vendor_id       : GenuineIntel
cpu family      : 6
model           : 60
model name      : Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz
stepping        : 3
microcode       : 0x20
cpu MHz         : 3502.187
cache size      : 8192 KB
physical id     : 0
siblings        : 8
core id         : 3
cpu cores       : 4
apicid          : 6
initial apicid  : 6
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm ida arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid xsaveopt
bogomips        : 6984.00
clflush size    : 64
cache_alignment : 64
address sizes   : 39 bits physical, 48 bits virtual
power management:

processor       : 4
vendor_id       : GenuineIntel
cpu family      : 6
model           : 60
model name      : Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz
stepping        : 3
microcode       : 0x20
cpu MHz         : 3533.496
cache size      : 8192 KB
physical id     : 0
siblings        : 8
core id         : 0
cpu cores       : 4
apicid          : 1
initial apicid  : 1
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm ida arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid xsaveopt
bogomips        : 6984.00
clflush size    : 64
cache_alignment : 64
address sizes   : 39 bits physical, 48 bits virtual
power management:

processor       : 5
vendor_id       : GenuineIntel
cpu family      : 6
model           : 60
model name      : Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz
stepping        : 3
microcode       : 0x20
cpu MHz         : 3899.902
cache size      : 8192 KB
physical id     : 0
siblings        : 8
core id         : 1
cpu cores       : 4
apicid          : 3
initial apicid  : 3
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm ida arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid xsaveopt
bogomips        : 6984.00
clflush size    : 64
cache_alignment : 64
address sizes   : 39 bits physical, 48 bits virtual
power management:

processor       : 6
vendor_id       : GenuineIntel
cpu family      : 6
model           : 60
model name      : Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz
stepping        : 3
microcode       : 0x20
cpu MHz         : 3500.410
cache size      : 8192 KB
physical id     : 0
siblings        : 8
core id         : 2
cpu cores       : 4
apicid          : 5
initial apicid  : 5
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm ida arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid xsaveopt
bogomips        : 6984.00
clflush size    : 64
cache_alignment : 64
address sizes   : 39 bits physical, 48 bits virtual
power management:

processor       : 7
vendor_id       : GenuineIntel
cpu family      : 6
model           : 60
model name      : Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz
stepping        : 3
microcode       : 0x20
cpu MHz         : 3502.324
cache size      : 8192 KB
physical id     : 0
siblings        : 8
core id         : 3
cpu cores       : 4
apicid          : 7
initial apicid  : 7
fpu             : yes
fpu_exception   : yes
cpuid level     : 13
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm ida arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid xsaveopt
bogomips        : 6984.00
clflush size    : 64
cache_alignment : 64
address sizes   : 39 bits physical, 48 bits virtual
power management:
</pre>

要理解该文件中的CPU信息，有几个相关的概念要知道，如：```processor```表示逻辑CPU的标识、```model name```表示真实CPU的型号信息、```physical id```表示真实CPU和标识、```cpu cores```表示真实CPU的内核数等等。

逻辑CPU的描述：现在的服务器一般都使用了"超线程"（Hyper-Threading，简称HT）技术来提高CPU的性能。超线程技术是在一颗CPU同时执行多个程序而共同分享一颗CPU内的资源，理论上要像两颗CPU一样在同一时间执行两个线程。虽然采用超线程技术能同时执行两个线程，但它并不象两个真正的CPU那样，每各CPU都具有独立的资源。当两个线程都同时需要某一个资源时，其中一个要暂时停止，并让出资源，直到这些资源闲置后才能继续。因此超线程的性能并不等于两颗CPU的性能。具有超线程技术的CPU还有一些其它方面的限制。


## 2. CPU负载率的计算方式
Load average的概念源自Unix系统，虽然各家的公式不尽相同，但都是用于衡量```正在使用CPU的进程数量```和```正在等待CPU的进程数量```，一句话就是runnable processes的数量。
所以Load average可以作为CPU瓶颈的参考指标，如果大于CPU的数量，说明CPU可能不够用了。

但是，在Linux上有点差异！

Linux上的Load average除了包括```正在使用CPU的进程数量```和```正在等待CPU的进程数量```之外，还包括*uninterruptible sleep*的进程数量。
Linux设计者的逻辑是，*uninterruptible sleep*应该都是很短暂的，很快就会恢复运行，所以被等同于```runnable```。然而，*uninterruptible sleep*
即使再短暂也是sleep，何况现实世界中*uninterruptible sleep*未必很短暂，大量的、或长时间的*uninterruptible sleep*通常意味着IO设备遇到了瓶颈。
众所周知，sleep状态的进程是不需要CPU的，即使所有的CPU都空闲，正在sleep的进程也是运行不了的，所以sleep进程的数量绝对不适合用作衡量CPU负载的指标。
Linux把*uninterruptible sleep*进程算进Load average的做法直接颠覆了Load average的本来意义。所以在Linux系统上，Load average这个指标基本失去了作用，
因为你不知道它代表什么意思，当看到Load average很高的时候，你不知道```runnable```进程太多还是```uninterruptible sleep```进程太多，也就无法判断是
CPU不够用还是IO设备有瓶颈。

从另一个方面来说，也就可以解释为什么磁盘慢时（大量磁盘使用时），CPU负载会飙高了。基本上我碰到CPU负载高的情况就两种情形：CPU本身处理太多任务，再加上
软中断和上下文切换太频繁导致负载高；再就是磁盘太慢导致了不可中断睡眠太多，导致CPU负载高。





<br />
<br />

**[参看]:**

1. [为什么磁盘慢会导致Linux负载飙升？](https://www.sohu.com/a/198294497_151779)

2. [怎么理解Linux软中断？](https://www.cnblogs.com/mysky007/p/12307056.html)


<br />
<br />
<br />


