---
layout: post
title: linux 操作系统资源限制
tags:
- LinuxOps
categories: linux
description: Linux操作系统资源限制
---

本文主要讲述一下Linux操作系统上的一些资源限制，做一个记录。我们当前的操作系统环境为：
<pre>
# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.3.1611 (Core) 
Release:        7.3.1611
Codename:       Core

# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>
如果要查看```proc```的相关说明，可以通过执行：
<pre>
# man proc
# man 5 proc
</pre>

<!-- more -->


## 1. Linux Thread资源限制
目前线程资源限制由以下几个系统参数共同决定：

* 参数```/proc/sys/kernel/threads-max```，有直接关系，每个进程中最多创建的的线程数目
* 参数```/proc/sys/kernel/pid_max```，有直接关系，系统中最多分配的pid数量
* 参数```/proc/sys/vm/max_map_count```，数量越大，能够创建的线程数目越多，目前具体关系未明

通过echo设置：
{% highlight string %}
# echo 2061219 > /proc/sys/kernel/threads-max
# echo 4194303 > /proc/sys/kernel/pid_max
# echo 4194303 > /proc/sys/vm/max_map_count
{% endhighlight %}


### 1.1 Linux Thread资源限制参数详解

一个进程的线程资源限制是由```若干```个系统参数共同决定的，下面我们分别介绍一下：

1） **threads_max**

关于```threads-max```的说明，我们先来看看官方手册上的说明：
<pre>
/proc/sys/kernel/threads-max (since Linux 2.3.11)
	      This file	specifies the  system-wide  limit  on  the  number  of
	      threads (tasks) that can be created on the system.
</pre>
本参数是一个```系统级别```(system-wide)的参数，用于指示整个系统可生成的最大线程数。我们可以通过如下方式来查看当前值：
<pre>
# cat /proc/sys/kernel/threads-max 
2061206
</pre>
我们有两种方法来修改这个参数：

* 临时修改
{% highlight string %}
# echo 2061206 > /proc/sys/kernel/threads-max
# cat /proc/sys/kernel/threads-max
2061206
{% endhighlight %}

* 永久修改
<pre>
# sysctl -w kernel.threads-max=2061206
</pre>


2) **pid_max**

关于```pid_max```的说明，我们先来看看官方手册上的说明：
<pre>
/proc/sys/kernel/pid_max	(since Linux 2.5.34)
	      This  file  specifies the	value at which PIDs wrap around	(i.e.,
	      the value	in this	file is	one greater  than  the	maximum	 PID).
	      The  default  value  for	this  file, 32768, results in the same
	      range of PIDs as on earlier kernels.  On 32-bit platforms, 32768
	      is  the  maximum	value for pid_max.  On 64-bit systems, pid_max
	      can be set to any	value up to 2^22 (PID_MAX_LIMIT, approximately
	      4	million).
</pre>
此参数用于指定系统能够分配的PID的个数（即系统能够创建的最大```进程```个数)。我们可以通过如下方法来查看当前值：
<pre>
# cat /proc/sys/kernel/pid_max
4194303
</pre>

有两种方法来修改这个参数：

* 临时修改
{% highlight string %}
# echo 4194303 > /proc/sys/kernel/pid_max
# cat /proc/sys/kernel/pid_max
4194303
{% endhighlight %}

* 永久修改
<pre>
# sysctl -w kernel.pid_max=4194303
</pre>

3) **max_map_count**

关于```max_map_count```的描述，这里我们并没有直接在官方找到，但在网上找到如下一段：
<pre>
“This file contains the maximum number of memory map areas a process may have. Memory map areas are used as a side-effect of calling malloc, directly by mmap and mprotect, and also when loading shared libraries.

While most applications need less than a thousand maps, certain programs, particularly malloc debuggers, may consume lots of them, e.g., up to one or two maps per allocation.

The default value is 65536.”
</pre>

max_map_count文件包含限制```一个进程```可以拥有的```虚拟内存区域```(VMA)的数量。虚拟内存区域是一个连续的虚拟地址空间区域。在进程的生命周期中，每当程序尝试在内存中映射文件，链接到共享内存段，或者分配堆栈空间的时候，这些区域将被创建。调优这个值将限制进程可拥有VMA的数量。限制一个进程拥有VMA的总数可能导致应用程序出错，因为当进程达到了VMA上限但又只能释放少量的内存给其他的内核进程使用时，操作系统会抛出内存不足的错误。

>说明： 单进程mmap的限制会影响单个进程可创建的线程数

要修改*max_map_count*需要root权限。有两种方法来修改这个参数：

* 临时修改
{% highlight string %}
# echo 4194303 >/proc/sys/vm/max_map_count
# cat /proc/sys/vm/max_map_count
4194303
{% endhighlight %}

* 永久修改
<pre>
# sysctl -w vm.max_map_count=4194303
</pre>

## 2. 用户层面进程数限制
在上面我们介绍了```系统层面```的总限制，普通用户层面所能创建的最大进程数会受系统层面总限制的影响。下面我们介绍一下普通用户层面进程数的相关限制。

###### 查看用户能打开的最大进程数

我们可以使用如下命令来查看用户层面对系统资源的相关限制：
<pre>
# ulimit -a
core file size          (blocks, -c) 0
data seg size           (kbytes, -d) unlimited
scheduling priority             (-e) 0
file size               (blocks, -f) unlimited
pending signals                 (-i) 1030603
max locked memory       (kbytes, -l) 64
max memory size         (kbytes, -m) unlimited
open files                      (-n) 1000000
pipe size            (512 bytes, -p) 8
POSIX message queues     (bytes, -q) 819200
real-time priority              (-r) 0
stack size              (kbytes, -s) 8192
cpu time               (seconds, -t) unlimited
max user processes              (-u) 1030603
virtual memory          (kbytes, -v) unlimited
file locks                      (-x) unlimited
</pre>
上面我们看到*max user processes*一行显示，当前用户(root)最多能创建**1030603**个进程。

###### 这个值是怎么来的

1) **root用户**

对于root用户，```ulimit -u```出现的*max user processes*的值默认是*/proc/sys/kernel/threads-max*值的1/2，即系统线程数的一半：
<pre>
# cat /proc/sys/kernel/threads-max
2061206
# ulimit -u
1030603
</pre>


2) **普通用户**

对于普通用户，```ulimit -u```出现的*max user processes*的值默认是*/etc/security/limits.d/20-nproc.conf*(注： centos6是90-nproc.conf)文件中的对应软限制：
<pre>
# ulimit -u
4096

# cat /etc/security/limits.d/20-nproc.conf 
# Default limit for number of user's processes to prevent
# accidental fork bombs.
# See rhbz #432903 for reasoning.

*          soft    nproc     4096
root       soft    nproc     unlimited
</pre>

###### 如何修改这个值
1) **root用户**

对于root用户，我们可以在*/etc/security/limits.conf*文件里添加如下内容：
{% highlight string %}
# echo "* soft nproc 1030603" >> /etc/security/limits.conf
# echo "* hard nproc 1030603" >> /etc/security/limits.conf
{% endhighlight %}
然后再执行如下命令让立即生效：
<pre>
# ulimit -SHu 1030603                   
</pre>
注意： 修改这里，普通用户*max user processes*值是不生效的，需要修改*/etc/security/limits.d/20-nproc.conf*文件中的值。如果使用```*```号让全局用户生效是受文件*/etc/security/limits.d/20-nproc.conf*中nproc值大小制约的；而如果仅仅是针对某个用户，那么就不受该文件(20-nproc.conf)nproc值大小的影响。



2) **普通用户**

对于普通用户，我们可以修改*/etc/security/limits.d/20-nproc.conf*文件。因为普通用户是受这个文件里值的影响：
<pre>
# echo "* soft nproc 65536" >> /etc/security/limits.d/20-nproc.conf
</pre>
然后再执行如下命令让立即生效：
<pre>
# ulimit -SHu 65535
</pre>

###### 查看某个进程的限制及状态

有时我们改完上述配置之后，程序运行过程中仍然会出现相应的故障，此时我们可以查看*/proc/pid_xxx/limits*进一步了解指定进程当前的限制状况：
<pre>
# ps -ef | grep radosgw | grep -v grep
ceph     2053794       1 21 Jan08 ?        11:16:41 /usr/bin/radosgw -f --cluster ceph --name client.radosgw.ceph001-node1 --setuser ceph --setgroup ceph
# cat /proc/2053794/limits
Limit                     Soft Limit           Hard Limit           Units     
Max cpu time              unlimited            unlimited            seconds   
Max file size             unlimited            unlimited            bytes     
Max data size             unlimited            unlimited            bytes     
Max stack size            8388608              unlimited            bytes     
Max core file size        0                    unlimited            bytes     
Max resident set          unlimited            unlimited            bytes     
Max processes             1048576              1048576              processes 
Max open files            1048576              1048576              files     
Max locked memory         65536                65536                bytes     
Max address space         unlimited            unlimited            bytes     
Max file locks            unlimited            unlimited            locks     
Max pending signals       1030603              1030603              signals   
Max msgqueue size         819200               819200               bytes     
Max nice priority         0                    0                    
Max realtime priority     0                    0                    
Max realtime timeout      unlimited            unlimited            us 
</pre>

然后可以通过查看*/proc/pid_xxx/status*来查看指定进程的当前状态：
<pre>
# ps -ef | grep radosgw | grep -v grep
ceph     2053794       1 21 Jan08 ?        11:16:41 /usr/bin/radosgw -f --cluster ceph --name client.radosgw.ceph001-node1 --setuser ceph --setgroup ceph
# cat /proc/2053794/status
Name:   radosgw
State:  S (sleeping)
Tgid:   2053794
Ngid:   0
Pid:    2053794
PPid:   1
TracerPid:      0
Uid:    167     167     167     167
Gid:    167     167     167     167
FDSize: 65536
Groups:
VmPeak: 205049904 kB
VmSize: 204810168 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:   6864720 kB
VmRSS:   6573440 kB
RssAnon:         6561812 kB
RssFile:           11628 kB
RssShmem:              0 kB
VmData: 204668164 kB
VmStk:       132 kB
VmExe:       224 kB
VmLib:     29900 kB
VmPTE:    283508 kB
VmSwap:   108228 kB
Threads:        129452
SigQ:   0/1030603
SigPnd: 0000000000000000
ShdPnd: 0000000000000000
SigBlk: 0000000000001000
SigIgn: 0000000000001000
SigCgt: 00000001c18066eb
CapInh: 0000000000000000
CapPrm: 0000000000000000
CapEff: 0000000000000000
CapBnd: 0000001ff7ffffff
CapAmb: 0000000000000000
Seccomp:        0
Cpus_allowed:   ffffff,ffffffff
Cpus_allowed_list:      0-55
Mems_allowed:   00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,
00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,
00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000003
Mems_allowed_list:      0-1
voluntary_ctxt_switches:        3881
nonvoluntary_ctxt_switches:     58
</pre>



## 3. sysctl命令介绍
sysctl命令用于运行时配置或显示内核参数，这些参数位于*/proc/sys*目录下。sysctl命令需要procfs的支持。可以使用sysctl命令来读写```sysctl数据```。

sysctl命令的基本语法格式如下：
<pre>
sysctl [options] [variable[=value]] [...]
sysctl -p [file or regexp] [...]
</pre>

下面列举一个常用的选项：

* **-w**: 用于修改某个指定的参数值

* **-a**: 用于显示所有的系统参数

* **-p**: 用于从指定的文件加载系统参数。如果未指定文件名，则默认从*/etc/sysctl.conf*文件进行加载。如果我们直接修改*/etc/sysctl.conf*文件，要让这些参数生效，则可以执行```sysctl -p```即可。




<br />
<br />

**[参看]:**

1. [Linux 一个进程所能创建的最大线程数](https://zhuanlan.zhihu.com/p/31803596)

2. [Linux内核参数调优](https://blog.csdn.net/sa19861211/article/details/90237203)

3. [centos 内核参数说明](https://www.freebsd.org/cgi/man.cgi?query=proc&apropos=0&sektion=5&manpath=CentOS+7.1&arch=default&format=html)

4. [linux kernel document](https://www.kernel.org/doc/)

