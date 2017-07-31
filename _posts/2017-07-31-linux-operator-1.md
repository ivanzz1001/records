---
layout: post
title: linux运维
tags:
- linux oper
categories: linux运维
description: Linux运维方面知识
---

本文主要介绍Linux运维方面的一些常见的命令的用法：
<!-- more -->


## 查看Linux下cpu信息

cpu信息记录在/proc/cpuinfo中，但信息比较多：
<pre>
[root@ceph001-node1 ~ ]$ more /proc/cpuinfo
processor       : 0
vendor_id       : GenuineIntel
cpu family      : 6
model           : 63
model name      : Intel(R) Xeon(R) CPU E5-2650 v3 @ 2.30GHz
stepping        : 2
microcode       : 0x2d
cpu MHz         : 2300.628
cache size      : 25600 KB
physical id     : 0
siblings        : 20
core id         : 0
cpu cores       : 10
apicid          : 0
initial apicid  : 0
fpu             : yes
fpu_exception   : yes
cpuid level     : 15
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm cons
tant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid dca sse4_1
 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1
 avx2 smep bmi2 erms invpcid cqm xsaveopt cqm_llc cqm_occup_llc
bogomips        : 4589.59
clflush size    : 64
cache_alignment : 64
address sizes   : 46 bits physical, 48 bits virtual
power management:

....
</pre>
下面我们简要介绍如何查看cpu的几个比较关键的数据。

1） 查看cpu型号
{% highlight string %}
cat /proc/cpuinfo | grep name | cut -f2 -d: | uniq -c
{% endhighlight %}
例如：
<pre>
[root@ceph001-node1 ~ ]$ cat /proc/cpuinfo | grep name | cut -f2 -d: | uniq -c
     40  Intel(R) Xeon(R) CPU E5-2650 v3 @ 2.30GHz
</pre>

2) 查看cpu个数

总核数 = 物理cpu个数 x 每颗物理cpu的核数

总逻辑cpu数 = 物理cpu个数 x 每颗物理cpu的核数 x 超线程数

<pre>
# 查看物理cpu个数
[root@ceph001-node1 ~ ]$ cat /proc/cpuinfo| grep "physical id"| sort| uniq| wc -l
2


# 查看每个物理cpu中core的个数
[root@ceph001-node1 ~ ]$ cat /proc/cpuinfo| grep "cpu cores"| uniq
cpu cores       : 10


# 查看逻辑cpu的个数
[root@ceph001-node1 ~ ]$ cat /proc/cpuinfo| grep "processor"| wc -l
40
</pre>
由上图可知：主机ceph001-node1有2颗物理cpu，10核40线程

如果不想自己算，也可以通过lscpu命令：
<pre>
[root@ceph001-node1 ~ ]$ lscpu
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                40
On-line CPU(s) list:   0-39
Thread(s) per core:    2
Core(s) per socket:    10
Socket(s):             2
NUMA node(s):          2
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 63
Model name:            Intel(R) Xeon(R) CPU E5-2650 v3 @ 2.30GHz
Stepping:              2
CPU MHz:               2300.179
BogoMIPS:              4594.92
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              256K
L3 cache:              25600K
NUMA node0 CPU(s):     0-9,20-29
NUMA node1 CPU(s):     10-19,30-39
</pre>





