---
layout: post
title: linux运维(1)
tags:
- LinuxOps
categories: linuxOps
description: Linux运维方面知识
---

本文主要介绍Linux运维方面的一些常见的命令的用法：
<!-- more -->


## 1. 查看Linux下cpu信息

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

processor       : 38
vendor_id       : GenuineIntel
cpu family      : 6
model           : 63
model name      : Intel(R) Xeon(R) CPU E5-2650 v3 @ 2.30GHz
stepping        : 2
microcode       : 0x2d
cpu MHz         : 2300.000
cache size      : 25600 KB
physical id     : 1
siblings        : 20
core id         : 11
cpu cores       : 10
apicid          : 55
initial apicid  : 55
fpu             : yes
fpu_exception   : yes
cpuid level     : 15
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid dca sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid cqm xsaveopt cqm_llc cqm_occup_llc
bogomips        : 4594.46
clflush size    : 64
cache_alignment : 64
address sizes   : 46 bits physical, 48 bits virtual
power management:

processor       : 39
vendor_id       : GenuineIntel
cpu family      : 6
model           : 63
model name      : Intel(R) Xeon(R) CPU E5-2650 v3 @ 2.30GHz
stepping        : 2
microcode       : 0x2d
cpu MHz         : 2299.910
cache size      : 25600 KB
physical id     : 1
siblings        : 20
core id         : 12
cpu cores       : 10
apicid          : 57
initial apicid  : 57
fpu             : yes
fpu_exception   : yes
cpuid level     : 15
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid dca sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid cqm xsaveopt cqm_llc cqm_occup_llc
bogomips        : 4594.46
clflush size    : 64
cache_alignment : 64
address sizes   : 46 bits physical, 48 bits virtual
power management:
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

{% highlight string %}
# 查看物理cpu个数
[root@ceph001-node1 ~ ]$ cat /proc/cpuinfo| grep "physical id"| sort| uniq| wc -l
2


# 查看每个物理cpu中core的个数
[root@ceph001-node1 ~ ]$ cat /proc/cpuinfo| grep "cpu cores"| uniq
cpu cores       : 10


# 查看逻辑cpu的个数
[root@ceph001-node1 ~ ]$ cat /proc/cpuinfo| grep "processor"| wc -l
40
{% endhighlight %}
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

3) 通过程序查看总逻辑CPU数
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
  int ngx_ncpu; 
   
  ngx_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
  printf("ngx_ncpu:%d\n",ngx_ncpu);

  return 0;
}
{% endhighlight %}
编译运行：
<pre>
[root@server-70 home]# gcc -o test test.c
[root@server-70 home]# ./test
ngx_ncpu:40
</pre>

## 2. Linux下查看内存大小
<pre>
# free
              total        used        free      shared  buff/cache   available
Mem:       10058704      474400     8964360        9264      619944     9187956
Swap:       5112828           0     5112828

# cat /proc/meminfo | grep MemTotal
MemTotal:       10058704 kB
</pre>

## 3. Linux下查看硬盘大小
<pre>
# fdisk -l

Disk /dev/sda: 85.9 GB, 85899345920 bytes, 167772160 sectors
Units = sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disk label type: dos
Disk identifier: 0x000c3eb0

   Device Boot      Start         End      Blocks   Id  System
/dev/sda1   *        2048      616447      307200   83  Linux
/dev/sda2          616448    10842111     5112832   82  Linux swap / Solaris
/dev/sda3        10842112   167772159    78465024   83  Linux	
</pre>

## 4. windows系统如何查看物理cpu核数(补充)

参看：https://jingyan.baidu.com/article/59703552e83cf98fc0074005.html

Windows命令行输入```wmic```，然后再输入```cpu get *```，拖动滚动调找到NumberOfcores和NumberOfLogicalProcessors。例如：
<pre>
Microsoft Windows [版本 6.1.7601]
版权所有 (c) 2009 Microsoft Corporation。保留所有权利。

C:\Users\Administrator>wmic
wmic:root\cli>cpu get *
AddressWidth  Architecture  Availability  Caption                               ConfigManagerErrorCode  ConfigManagerUserConfig  
64            9             3             Intel64 Family 6 Model 60 Stepping 3                                                   


CpuStatus  CreationClassName  CurrentClockSpeed  CurrentVoltage  DataWidth  Description                           
1          Win32_Processor    3501               12              64         Intel64 Family 6 Model 60 Stepping 3  


DeviceID  ErrorCleared  ErrorDescription  ExtClock  Family  InstallDate  L2CacheSize  L2CacheSpeed  L3CacheSize  
CPU0                                      100       179                  1024                       8192         

L3CacheSpeed  LastErrorCode  Level  LoadPercentage  Manufacturer  MaxClockSpeed  Name                                       
0                            6      82              GenuineIntel  3501           Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz  

NumberOfCores  NumberOfLogicalProcessors  OtherFamilyDescription  PNPDeviceID  PowerManagementCapabilities  
4              8                                                                                            

PowerManagementSupported  ProcessorId       ProcessorType  Revision  Role  SocketDesignation  Status  StatusInfo  Stepping  
FALSE                     BFEBFBFF000306C3  3              15363     CPU   SOCKET 0           OK      3                     

SystemCreationClassName  SystemName   UniqueId  UpgradeMethod  Version  VoltageCaps
Win32_ComputerSystem     ZHANGYW6668            36

wmic:root\cli>
</pre>
```注意```: 上面为了方便显示，对格式进行了适当的调整。

此外，windows上内存型号可以通过```memorychip```来进行查看。

<br />
<br />
<br />





