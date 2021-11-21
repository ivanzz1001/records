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

### 1.1 查看物理CPU信息
我们可以通过```lscpu```命令来获取实际的物理CPU信息：
<pre>
# lscpu
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                56
On-line CPU(s) list:   0-55
Thread(s) per core:    2
Core(s) per socket:    14
Socket(s):             2
NUMA node(s):          2
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 79
Model name:            Intel(R) Xeon(R) CPU E5-2680 v4 @ 2.40GHz
Stepping:              1
CPU MHz:               1250.531
BogoMIPS:              4794.42
Virtualization:        VT-x
L1d cache:             32K
L1i cache:             32K
L2 cache:              256K
L3 cache:              35840K
NUMA node0 CPU(s):     0-13,28-41
NUMA node1 CPU(s):     14-27,42-55
</pre>
下面我们解释一下各字段的含义：

* Architecture: 用于指示CPU架构。当前我们使用的CPU架构为x86_64

* CPU op-mode(s): 表明此CPU可以32 bit模式运行，也可以64 bit模式运行

* Byte Order: CPU字节序为小端字节序

* CPU(s): 指明当前逻辑CPU的个数 
<pre>
逻辑CPU个数 = 物理CPU个数 * 每颗物理cpu的核数 * 超线程数
</pre>

* On-line CPUs(s) list: 用于指明当前Linux实例所使用的CPU列表

* Thread(s) per core: 每个CPU核支持的线程数。大于1，我们一般称为支持```超线程```

* Core(s) per socket: 每颗物理CPU的核数

* Socket(s): 物理CPU的个数

* NUMA node(s): Non-Uniform Memory Access节点的个数

* Vendor ID: 生产商

* CPU family： CPU家族编号

* Model: CPU型号

* Model name: CPU型号名称，这里为```Intel至强处理器```(Xeon: Extreme Edition)

* Virtualization: CPU支持的虚拟化技术

**总结：**
{% highlight string %}
socket就是主板上的CPU插槽; Core就是socket里独立的一组程序执行的硬件单元，比如寄存器，计算单元等; 
Thread：就是超线程hyperthread的概念，逻辑的执行单元，独立的执行上下文，但是共享core内的寄存器和计算单元。
{% endhighlight %}

### 1.2 查看逻辑CPU信息

cpu信息记录在/proc/cpuinfo中，但信息比较多：
<pre>
# more /proc/cpuinfo
processor       : 0
vendor_id       : GenuineIntel
cpu family      : 6
model           : 79
model name      : Intel(R) Xeon(R) CPU E5-2680 v4 @ 2.40GHz
stepping        : 1
microcode       : 0xb000021
cpu MHz         : 2401.406
cache size      : 35840 KB
physical id     : 0
siblings        : 28
core id         : 0
cpu cores       : 14
apicid          : 0
initial apicid  : 0
fpu             : yes
fpu_exception   : yes
cpuid level     : 20
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm cons
tant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid dc
a sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch ida arat epb pln pts dtherm intel_pt tpr_shadow vnmi flexpriorit
y ept vpid fsgsbase tsc_adjust bmi1 hle avx2 smep bmi2 erms invpcid rtm cqm rdseed adx smap xsaveopt cqm_llc cqm_occup_llc cqm_mbm_total cqm_mbm_local
bogomips        : 4788.76
clflush size    : 64
cache_alignment : 64
address sizes   : 46 bits physical, 48 bits virtual
power management:

processor       : 1
vendor_id       : GenuineIntel
cpu family      : 6
model           : 79
model name      : Intel(R) Xeon(R) CPU E5-2680 v4 @ 2.40GHz
stepping        : 1
microcode       : 0xb000021
cpu MHz         : 2902.031
cache size      : 35840 KB
physical id     : 0
siblings        : 28
core id         : 1
cpu cores       : 14
apicid          : 2
initial apicid  : 2
fpu             : yes
fpu_exception   : yes
...
processor       : 55
vendor_id       : GenuineIntel
cpu family      : 6
model           : 79
model name      : Intel(R) Xeon(R) CPU E5-2680 v4 @ 2.40GHz
stepping        : 1
microcode       : 0xb000021
cpu MHz         : 1388.343
cache size      : 35840 KB
physical id     : 1
siblings        : 28
core id         : 14
cpu cores       : 14
apicid          : 61
initial apicid  : 61
fpu             : yes
fpu_exception   : yes
cpuid level     : 20
wp              : yes
flags           : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 fma cx16 xtpr pdcm pcid dca sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch ida arat epb pln pts dtherm intel_pt tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 hle avx2 smep bmi2 erms invpcid rtm cqm rdseed adx smap xsaveopt cqm_llc cqm_occup_llc cqm_mbm_total cqm_mbm_local
bogomips        : 4794.42
clflush size    : 64
cache_alignment : 64
address sizes   : 46 bits physical, 48 bits virtual
power management:
</pre>
下面我们简要介绍如何查看cpu的几个比较关键的数据。

1） 查看cpu型号
{% highlight string %}
# cat /proc/cpuinfo | grep name | cut -f2 -d: | uniq -c
{% endhighlight %}
例如：
<pre>
# cat /proc/cpuinfo | grep name | cut -f2 -d: | uniq -c
     56  Intel(R) Xeon(R) CPU E5-2680 v4 @ 2.40GHz
</pre>

2) 查看cpu个数

总核数 = 物理cpu个数 x 每颗物理cpu的核数

总逻辑cpu数 = 物理cpu个数 x 每颗物理cpu的核数 x 超线程数

<pre>
// 查看物理cpu个数
# cat /proc/cpuinfo| grep "physical id"| sort| uniq| wc -l
2


// 查看每个物理cpu中core的个数
# cat /proc/cpuinfo| grep "cpu cores"| uniq
cpu cores       : 14


// 查看逻辑cpu的个数
# cat /proc/cpuinfo| grep "processor"| wc -l
56
</pre>
由上图可知：主机ceph001-node1有2颗物理cpu，10核40线程



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
ngx_ncpu:56
</pre>


## 2. 查看网卡信息
首先我们通过```ifconfig -a```或者```ip addr```命令来查看所有的物理网卡：
{% highlight string %}
# ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet 10.17.253.180/32 brd 10.17.253.180 scope global lo:0
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP qlen 1000
    link/ether 38:90:a5:7f:7c:ac brd ff:ff:ff:ff:ff:ff
    inet 10.17.253.170/24 brd 10.17.253.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::3a90:a5ff:fe7f:7cac/64 scope link 
       valid_lft forever preferred_lft forever
3: eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP qlen 1000
    link/ether 38:90:a5:7f:7c:ad brd ff:ff:ff:ff:ff:ff
    inet 10.17.254.170/24 brd 10.17.254.255 scope global eth1
       valid_lft forever preferred_lft forever
    inet6 fe80::3a90:a5ff:fe7f:7cad/64 scope link 
       valid_lft forever preferred_lft forever
4: eno1: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc mq state DOWN qlen 1000
    link/ether 70:70:8b:1f:0e:04 brd ff:ff:ff:ff:ff:ff
5: eno2: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc mq state DOWN qlen 1000
    link/ether 70:70:8b:1f:0e:05 brd ff:ff:ff:ff:ff:ff
{% endhighlight %}
接着，我们可以使用```ethtool```命令查看某一块网卡的详细信息：
<pre>
# ethtool eth0
Settings for eth0:
        Supported ports: [ FIBRE ]
        Supported link modes:   10000baseT/Full 
        Supported pause frame use: No
        Supports auto-negotiation: No
        Advertised link modes:  10000baseT/Full 
        Advertised pause frame use: No
        Advertised auto-negotiation: No
        Speed: 10000Mb/s
        Duplex: Full
        Port: FIBRE
        PHYAD: 0
        Transceiver: external
        Auto-negotiation: off
        Current message level: 0x00000000 (0)
                               
        Link detected: yes
</pre>

我们可以通过如下命令查看```网络控制器```的具体型号:
<pre>
# lspci | grep Ethernet  
06:00.0 Ethernet controller: Cisco Systems Inc VIC Ethernet NIC (rev a2)
07:00.0 Ethernet controller: Cisco Systems Inc VIC Ethernet NIC (rev a2)
0f:00.0 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
0f:00.1 Ethernet controller: Intel Corporation I350 Gigabit Network Connection (rev 01)
</pre>

然后通过查询```/sys/bus/pci/```查询，可以找出某一个```网卡控制器```具体对应的是哪一个网络接口。例如，我们查询```06:00.0```这一个网络控制器所对应的网络接口：
<pre>
# cat /sys/bus/pci/devices/0000\:06\:00.0/net/eth0/address 
38:90:a5:7f:7c:ac
</pre>
上面我们看到```eth0```接口所使用的是```思科万兆网卡```。

此外，我们还可以通过如下命令来查看相应的网卡信息：
{% highlight string %}
# hwconfig |grep Network
Network:        eth0 (bonding): 29:31:52:a8:e9:a6
Network:        slave0 (igb): Intel 82580 Gigabit Network Connection, 28:31:52:a8:e1:d6, 1000Mb/s <full-duplex>
Network:        slave1 (igb): Intel 82580 Gigabit Network Connection, 28:31:52:a8:e1:d6, 1000Mb/s <full-duplex>
{% endhighlight %}


## 3. 查看详细的物理硬盘信息
我们可以通过```smartctl```命令来查看某一块硬盘的详细信息：
<pre>
# smartctl -a /dev/sda
smartctl 6.2 2017-02-27 r4394 [x86_64-linux-3.10.0-514.26.2.el7.x86_64] (local build)
Copyright (C) 2002-13, Bruce Allen, Christian Franke, www.smartmontools.org

=== START OF INFORMATION SECTION ===
Vendor:               SEAGATE
Product:              ST8000NM0075
Revision:             E0C2
User Capacity:        8,001,563,222,016 bytes [8.00 TB]
Logical block size:   512 bytes
Physical block size:  4096 bytes
Lowest aligned LBA:   0
Logical block provisioning type unreported, LBPME=0, LBPRZ=0
Rotation Rate:        7200 rpm
Form Factor:          3.5 inches
Logical Unit id:      0x5000c500945aed3f
Serial number:        ZA18K1280000R751ZZND
Device type:          disk
Transport protocol:   SAS
Local Time is:        Wed Mar 27 18:15:58 2019 CST
SMART support is:     Available - device has SMART capability.
SMART support is:     Enabled
Temperature Warning:  Enabled

=== START OF READ SMART DATA SECTION ===
SMART Health Status: OK

Current Drive Temperature:     46 C
Drive Trip Temperature:        60 C

Manufactured in week 33 of year 2017
Specified cycle count over device lifetime:  10000
Accumulated start-stop cycles:  160
Specified load-unload count over device lifetime:  300000
Accumulated load-unload cycles:  683
Elements in grown defect list: 0

Vendor (Seagate) cache information
  Blocks sent to initiator = 153684224
  Blocks received from initiator = 817543504
  Blocks read from cache and sent to initiator = 1936936257
  Number of read and write commands whose size <= segment size = 193285480
  Number of read and write commands whose size > segment size = 241028

Vendor (Seagate/Hitachi) factory information
  number of hours powered up = 12608.93
  number of minutes until next internal SMART test = 56

Error counter log:
           Errors Corrected by           Total   Correction     Gigabytes    Total
               ECC          rereads/    errors   algorithm      processed    uncorrected
           fast | delayed   rewrites  corrected  invocations   [10^9 bytes]  errors
read:   872412800        0         0  872412800          0     125423.013           0
write:         0        0         0         0          0      38284.696           0

Non-medium error count:     1354


[GLTSD (Global Logging Target Save Disable) set. Enable Save with '-S on']
SMART Self-test log
Num  Test              Status                 segment  LifeTime  LBA_first_err [SK ASC ASQ]
     Description                              number   (hours)
# 1  Background short  Completed                   -       3                 - [-   -    -]
# 2  Background short  Completed                   -       3                 - [-   -    -]
# 3  Background short  Completed                   -       2                 - [-   -    -]
Long (extended) Self Test duration: 47220 seconds [787.0 minutes]
</pre>

上面我们看到采用的是```SAS```接口的希捷硬盘。

>注：
>  8,001,563,222,016 bytes = 7.277379356324672698974609375 TB
>  
> SEAGATE中文名“希捷”

接着使用```lsblk```或```fdisk -l```命令来查看分区信息：
<pre>
# lsblk
NAME              MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
sda                 8:0    0   7.3T  0 disk 
├─sda1              8:1    0    10G  0 part 
└─sda2              8:2    0   7.3T  0 part /var/lib/ceph/osd/ceph-7
sdb                 8:16   0   7.3T  0 disk 
├─sdb1              8:17   0    10G  0 part 
└─sdb2              8:18   0   7.3T  0 part /var/lib/ceph/osd/ceph-10
sdc                 8:32   0   7.3T  0 disk 
├─sdc1              8:33   0    10G  0 part 
└─sdc2              8:34   0   7.3T  0 part /var/lib/ceph/osd/ceph-19
sdd                 8:48   0   7.3T  0 disk 
├─sdd1              8:49   0    10G  0 part 
└─sdd2              8:50   0   7.3T  0 part /var/lib/ceph/osd/ceph-28
sde                 8:64   0   7.3T  0 disk 
├─sde1              8:65   0    10G  0 part 
└─sde2              8:66   0   7.3T  0 part /var/lib/ceph/osd/ceph-31
sdf                 8:80   0   7.3T  0 disk 
├─sdf1              8:81   0    10G  0 part 
└─sdf2              8:82   0   7.3T  0 part /var/lib/ceph/osd/ceph-33
sdg                 8:96   0   7.3T  0 disk 
├─sdg1              8:97   0    10G  0 part 
└─sdg2              8:98   0   7.3T  0 part /var/lib/ceph/osd/ceph-41
sdh                 8:112  0   7.3T  0 disk 
sdi                 8:128  0   7.3T  0 disk 
├─sdi1              8:129  0    10G  0 part 
└─sdi2              8:130  0   7.3T  0 part /var/lib/ceph/osd/ceph-47
sdj                 8:144  0   7.3T  0 disk 
├─sdj1              8:145  0    10G  0 part 
└─sdj2              8:146  0   7.3T  0 part /var/lib/ceph/osd/ceph-49
sdk                 8:160  0   7.3T  0 disk 
├─sdk1              8:161  0     1M  0 part 
├─sdk2              8:162  0   600M  0 part /boot
├─sdk3              8:163  0     8G  0 part [SWAP]
└─sdk4              8:164  0   7.3T  0 part 
  ├─test-lv_root 253:0    0    20G  0 lvm  /
  └─test-lv_var  253:1    0   7.3T  0 lvm  /var
sdl                 8:176  0 447.1G  0 disk 
├─sdl1              8:177  0    10G  0 part 
└─sdl2              8:178  0 437.1G  0 part /var/lib/ceph/osd/ceph-90
sdm                 8:192  0 447.1G  0 disk 
├─sdm1              8:193  0    10G  0 part 
</pre>


## 4. Linux下查看内存大小
我们可以通过如下命令查看物理内存条信息：
<pre>
# sudo dmidecode -t memory 
# dmidecode 3.0
Scanning /dev/mem for entry point.
SMBIOS 3.0 present.

Handle 0x0022, DMI type 16, 23 bytes
Physical Memory Array
        Location: System Board Or Motherboard
        Use: System Memory
        Error Correction Type: Multi-bit ECC
        Maximum Capacity: 1536 GB
        Error Information Handle: Not Provided
        Number Of Devices: 24

Handle 0x0024, DMI type 17, 40 bytes
Memory Device
        Array Handle: 0x0022
        Error Information Handle: Not Provided
        Total Width: 72 bits
        Data Width: 64 bits
        Size: 32 GB
        Form Factor: DIMM
        Set: None
        Locator: DIMM_A1
        Bank Locator: NODE 0 CHANNEL 0 DIMM 0
        Type: DDR4
        Type Detail: Registered (Buffered)
....
</pre>

并通过```free```命令查看具体的使用情况：
<pre>
# free -h
              total        used        free      shared  buff/cache   available
Mem:           251G         93G         35G        5.3G        122G        123G
Swap:          8.0G        481M        7.5G
</pre>
Buffers是针对raw disk的块缓存，主要是以raw block的方式缓存文件系统的元数据（比如超级块信息等)，这个值一般比较小（20M左右）；而Cached是针对某些具体的文件进行读缓存，以增加文件的访问效率而使用的，可以说是用于文件系统中文件缓存使用。

这里```buff```为/proc/meminfo中的```Buffers``` , 而```cache```包括/proc/meminfo中的```Cached```和```Slab```。



## 5. windows系统如何查看物理cpu核数(补充)

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
**[参看]:**

1. [NUMA体系结构详解](https://blog.csdn.net/ustc_dylan/article/details/45667227)

2. [32-bit, 64-bit CPU op-mode on Linux](https://unix.stackexchange.com/questions/77718/32-bit-64-bit-cpu-op-mode-on-linux)

3. [NUMA概述](http://www.ipcpu.com/2015/11/numa-cpu-concept/)

<br />
<br />
<br />





