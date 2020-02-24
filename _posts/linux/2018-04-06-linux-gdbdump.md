---
layout: post
title: Linux中使用gdb dump内存
tags:
- LinuxOps
categories: linux
description: Linux中使用GDB dump内存
---



本文我们先讲解Linux进程空间布局，以及/proc/pid/maps各字段具体含义，之后再介绍Linux中如何使用gdb来dump内存。



<!-- more -->

## 1. Linux进程空间布局

在32位系统中，Linux进程的虚拟地址空间布局如下：

![linux-memory](https://ivanzz1001.github.io/records/assets/img/linux/linux-memory-layout.png)

进程虚拟地址空间为0x0~0xFFFFFFFF，一共4GB大小。其中低位的3GB为用户空间，高位的1GB为内核空间。下面介绍一下空间的各个部分：

1） **保留区(reserved)**

它并不是一个单一的内存区域，而是对地址空间中受到操作系统保护而禁止用户进程访问的地址区域的总称。大多数操作系统中，极小的地址通常都是不允许访问的，如```NULL```。C语言将无效指针赋值为0x0也是出于这种考虑，因为0地址上正常情况下不会存放有效的可访问数据。

>注： 这里0x08048000是大概约定俗成的偏移，其值为128MB多一点

2) **代码和只读数据区(ro)**

这一部分，只能读不能写。代码、常量字符串、#define定义的常量存放在这。

3) **数据段**

保存全局变量、静态变量。可执行文件中的数据被映射至该区，包括```.data```和```.bss```。

4） **堆**

代码和数据区往上是运行时堆。从下往上增长。与代码/数据段在程序加载时就确定了大小不同，堆可以在运行时动态扩展或收缩。调用如malloc/free、new/delete这样的库函数时，操作的内存区域就在堆中。堆的范围通常较大，如在32位Linux系统中，堆的理论值可以达到2.9GB。堆顶的位置可以通过函数```brk```和```sbrk```进行动态调整。

5) **文件映射区**

动态库、共享内存等映射物理空间的内存，一般是mmap()所分配的虚拟地址空间。该区域用于映射可执行文件用到的动态链接库。在Linux2.4版本中，若可执行文件依赖共享库，则系统会为这些动态库在从0x40000000开始的地址分配相应的空间，并在程序装载时将其载入到该空间。在Linux2.6内核中，共享库的起始地址上移至更靠近栈区的位置。

6） **栈**

栈用于维护函数调用的上下文，编译器用栈来实现函数调用。跟堆一样，用户栈在程序运行期间可以动态扩展和收缩。与堆相比，栈通常较小，典型值为8MB。从上往下增长。

7) **内核空间**

内核总是驻留在内存中，是操作系统的一部分。内核空间就是为内核保留的，不允许应用程序读写这个区域的内容或直接调用内核代码定义的函数。

## 2. /proc/pid/maps详解
我们可以通过*man 5 proc*来从官方手册上查看*/proc/pid/maps*各字段的含义。

我们可以通过*/proc/pid/maps*文件来查看```pid```进程当前所映射的内存区域以及相应区域的访问权限。该文件的格式类似于如下：
<pre>
address           perms offset  dev   inode       pathname
00400000-00452000 r-xp 00000000 08:02 173521      /usr/bin/dbus-daemon
00651000-00652000 r--p 00051000 08:02 173521      /usr/bin/dbus-daemon
00652000-00655000 rw-p 00052000 08:02 173521      /usr/bin/dbus-daemon
00e03000-00e24000 rw-p 00000000 00:00 0           [heap]
00e24000-011f7000 rw-p 00000000 00:00 0           [heap]
...
35b1800000-35b1820000 r-xp 00000000 08:02 135522  /usr/lib64/ld-2.15.so
35b1a1f000-35b1a20000 r--p 0001f000 08:02 135522  /usr/lib64/ld-2.15.so
35b1a20000-35b1a21000 rw-p 00020000 08:02 135522  /usr/lib64/ld-2.15.so
35b1a21000-35b1a22000 rw-p 00000000 00:00 0
35b1c00000-35b1dac000 r-xp 00000000 08:02 135870  /usr/lib64/libc-2.15.so
35b1dac000-35b1fac000 ---p 001ac000 08:02 135870  /usr/lib64/libc-2.15.so
35b1fac000-35b1fb0000 r--p 001ac000 08:02 135870  /usr/lib64/libc-2.15.so
35b1fb0000-35b1fb2000 rw-p 001b0000 08:02 135870  /usr/lib64/libc-2.15.so
...
f2c6ff8c000-7f2c7078c000 rw-p 00000000 00:00 0    [stack:986]
...
7fffb2c0d000-7fffb2c2e000 rw-p 00000000 00:00 0   [stack]
7fffb2d48000-7fffb2d49000 r-xp 00000000 00:00 0   [vdso]
</pre> 
下面我们介绍一下各列的含义：

1） **address列**

进程中某一个虚拟内存空间的起始地址与结束地址。

2） **perms列**

用于指示该内存区域相应的访问权限。其中
<pre>
r = read
w = write
x = execute
s = shared
p = private (copy on write)
</pre>

3) **offset列**

虚拟内存区域在被映射文件中的偏移量。

4） **dev列**

映像文件的主次设备号(major:minor)

5) **inode列**

映像文件在该设备上的inode号。其中0表示没有inode节点与该内存区域所绑定，通常BSS段内存区域的inode值就为0。

6）**pathname列**

通常是内存区域所映射到的文件。对于ELF文件，你可以很容易的找出相应段的偏移（通过*readelf -l*命令)；另外也有一些其他的pseudo-paths:

* ```[stack]```: 初始进程(即主线程）的栈区

* ```[stack:<tid>]```: 线程<tid>的栈区，其对应于/proc/[pid]/task/[tid]/路径中相关信息。（since Linux 3.4)

* ```[vdso]```: 虚拟动态链接对象

* ```[heap]```: 进程的堆区

假如```pathname```列为空的话，则是通过mmap()函数所创建的匿名映射。

注： 上面我们看到类似于如下的行
<pre>
00400000-00452000 r-xp 00000000 08:02 173521      /usr/bin/dbus-daemon
00651000-00652000 r--p 00051000 08:02 173521      /usr/bin/dbus-daemon
00652000-00655000 rw-p 00052000 08:02 173521      /usr/bin/dbus-daemon
</pre>
*/usr/bin/dbus-daemon*映像文件被分成了3行，通常第一行为可执行文件的代码段、第二行为可执行文件的只读数据段、第三行为可执行文件的可读写数据段。


## 3. 使用GDB来dump内存
在应急响应中，我们往往会有dump出某一块内存来进行分析的必要。今天要讲的是利用gdb命令dump出sshd进程的内存。

按照Linux系统的设计哲学，内核只提供dump内存的机制，用户想要dump什么样的内存，dump多少内存是属于策略问题，由用户来决定。

在真实的使用场景中，主要有两种使用方式：

* 一种是dump某一个进程的地址空间来供用户在进程挂掉之后debug分析，也就是通常所说的coredump

* 另一种就是dump整个系统的内存空间，以便于系统管理员debug分析系统挂掉的原因，也就是通常所说的kdump。由于dump内存的逻辑依然需要系统可以正常工作，管理系统的各种资源，所以kdump整个过程依赖kexec和一个额外的dump内核来保证整个流程正确的执行。

以下演示的是dump出某一个进程(sshd)的内存：
<pre>
# ps -ef | grep sshd 
root       959     1  0 03:56 ?        00:00:00 /usr/sbin/sshd -D
root      1997   959  0 03:58 ?        00:00:00 sshd: root@pts/4
root      3565  2085  0 07:22 pts/4    00:00:00 grep --color=auto sshd

# cat /proc/959/maps
800da000-801c2000 r-xp 00000000 08:01 2228505    /usr/sbin/sshd
801c3000-801c5000 r--p 000e8000 08:01 2228505    /usr/sbin/sshd
801c5000-801c6000 rw-p 000ea000 08:01 2228505    /usr/sbin/sshd
801c6000-801cb000 rw-p 00000000 00:00 0 
809e9000-80a0a000 rw-p 00000000 00:00 0          [heap]
b6e8c000-b6e97000 r-xp 00000000 08:01 661243     /lib/i386-linux-gnu/libnss_files-2.23.so
b6e97000-b6e98000 r--p 0000a000 08:01 661243     /lib/i386-linux-gnu/libnss_files-2.23.so
b6e98000-b6e99000 rw-p 0000b000 08:01 661243     /lib/i386-linux-gnu/libnss_files-2.23.so
b6e99000-b6e9f000 rw-p 00000000 00:00 0 
b6e9f000-b6eaa000 r-xp 00000000 08:01 661247     /lib/i386-linux-gnu/libnss_nis-2.23.so
b6eaa000-b6eab000 r--p 0000a000 08:01 661247     /lib/i386-linux-gnu/libnss_nis-2.23.so
b6eab000-b6eac000 rw-p 0000b000 08:01 661247     /lib/i386-linux-gnu/libnss_nis-2.23.so
b6eac000-b6eb4000 r-xp 00000000 08:01 661211     /lib/i386-linux-gnu/libnss_compat-2.23.so
b6eb4000-b6eb5000 r--p 00007000 08:01 661211     /lib/i386-linux-gnu/libnss_compat-2.23.so
b6eb5000-b6eb6000 rw-p 00008000 08:01 661211     /lib/i386-linux-gnu/libnss_compat-2.23.so
b6eb6000-b6eb8000 rw-p 00000000 00:00 0 
b6eb8000-b6ecc000 r-xp 00000000 08:01 656913     /lib/i386-linux-gnu/libgpg-error.so.0.17.0
b6ecc000-b6ecd000 r--p 00013000 08:01 656913     /lib/i386-linux-gnu/libgpg-error.so.0.17.0
b6ecd000-b6ece000 rw-p 00014000 08:01 656913     /lib/i386-linux-gnu/libgpg-error.so.0.17.0
b6ece000-b6ecf000 rw-p 00000000 00:00 0 
b6ecf000-b6ee3000 r-xp 00000000 08:01 660822     /lib/i386-linux-gnu/libresolv-2.23.so
b6ee3000-b6ee4000 ---p 00014000 08:01 660822     /lib/i386-linux-gnu/libresolv-2.23.so
b6ee4000-b6ee5000 r--p 00014000 08:01 660822     /lib/i386-linux-gnu/libresolv-2.23.so
b6ee5000-b6ee6000 rw-p 00015000 08:01 660822     /lib/i386-linux-gnu/libresolv-2.23.so
b6ee6000-b6ee8000 rw-p 00000000 00:00 0 
b6ee8000-b6eeb000 r-xp 00000000 08:01 656930     /lib/i386-linux-gnu/libkeyutils.so.1.5
b6eeb000-b6eec000 r--p 00002000 08:01 656930     /lib/i386-linux-gnu/libkeyutils.so.1.5
b6eec000-b6eed000 rw-p 00003000 08:01 656930     /lib/i386-linux-gnu/libkeyutils.so.1.5
b6eed000-b6ef8000 r-xp 00000000 08:01 2229384    /usr/lib/i386-linux-gnu/libkrb5support.so.0.1
b6ef8000-b6ef9000 r--p 0000a000 08:01 2229384    /usr/lib/i386-linux-gnu/libkrb5support.so.0.1
b6ef9000-b6efa000 rw-p 0000b000 08:01 2229384    /usr/lib/i386-linux-gnu/libkrb5support.so.0.1
b6efa000-b6f28000 r-xp 00000000 08:01 2232558    /usr/lib/i386-linux-gnu/libk5crypto.so.3.1
b6f28000-b6f29000 ---p 0002e000 08:01 2232558    /usr/lib/i386-linux-gnu/libk5crypto.so.3.1
b6f29000-b6f2a000 r--p 0002e000 08:01 2232558    /usr/lib/i386-linux-gnu/libk5crypto.so.3.1
b6f2a000-b6f2b000 rw-p 0002f000 08:01 2232558    /usr/lib/i386-linux-gnu/libk5crypto.so.3.1
b6f2b000-b6f44000 r-xp 00000000 08:01 660817     /lib/i386-linux-gnu/libpthread-2.23.so
b6f44000-b6f45000 r--p 00018000 08:01 660817     /lib/i386-linux-gnu/libpthread-2.23.so
b6f45000-b6f46000 rw-p 00019000 08:01 660817     /lib/i386-linux-gnu/libpthread-2.23.so
b6f46000-b6f49000 rw-p 00000000 00:00 0 
b6f49000-b6f65000 r-xp 00000000 08:01 656907     /lib/i386-linux-gnu/libgcc_s.so.1
b6f65000-b6f66000 rw-p 0001b000 08:01 656907     /lib/i386-linux-gnu/libgcc_s.so.1
b6f66000-b7011000 r-xp 00000000 08:01 657014     /lib/i386-linux-gnu/libgcrypt.so.20.0.5
b7011000-b7012000 r--p 000aa000 08:01 657014     /lib/i386-linux-gnu/libgcrypt.so.20.0.5
b7012000-b7015000 rw-p 000ab000 08:01 657014     /lib/i386-linux-gnu/libgcrypt.so.20.0.5
b7015000-b7039000 r-xp 00000000 08:01 656936     /lib/i386-linux-gnu/liblzma.so.5.0.0
b7039000-b703a000 r--p 00023000 08:01 656936     /lib/i386-linux-gnu/liblzma.so.5.0.0
b703a000-b703b000 rw-p 00024000 08:01 656936     /lib/i386-linux-gnu/liblzma.so.5.0.0
b703b000-b7042000 r-xp 00000000 08:01 661248     /lib/i386-linux-gnu/librt-2.23.so
b7042000-b7043000 r--p 00006000 08:01 661248     /lib/i386-linux-gnu/librt-2.23.so
b7043000-b7044000 rw-p 00007000 08:01 661248     /lib/i386-linux-gnu/librt-2.23.so
b7044000-b70b7000 r-xp 00000000 08:01 656996     /lib/i386-linux-gnu/libpcre.so.3.13.2
b70b7000-b70b8000 r--p 00072000 08:01 656996     /lib/i386-linux-gnu/libpcre.so.3.13.2
b70b8000-b70b9000 rw-p 00073000 08:01 656996     /lib/i386-linux-gnu/libpcre.so.3.13.2
b70b9000-b70ba000 rw-p 00000000 00:00 0 
b70ba000-b70bd000 r-xp 00000000 08:01 660820     /lib/i386-linux-gnu/libdl-2.23.so
b70bd000-b70be000 r--p 00002000 08:01 660820     /lib/i386-linux-gnu/libdl-2.23.so
b70be000-b70bf000 rw-p 00003000 08:01 660820     /lib/i386-linux-gnu/libdl-2.23.so
b70bf000-b70d6000 r-xp 00000000 08:01 660774     /lib/i386-linux-gnu/libnsl-2.23.so
b70d6000-b70d7000 r--p 00016000 08:01 660774     /lib/i386-linux-gnu/libnsl-2.23.so
b70d7000-b70d8000 rw-p 00017000 08:01 660774     /lib/i386-linux-gnu/libnsl-2.23.so
b70d8000-b70da000 rw-p 00000000 00:00 0 
b70da000-b728a000 r-xp 00000000 08:01 660818     /lib/i386-linux-gnu/libc-2.23.so
b728a000-b728c000 r--p 001af000 08:01 660818     /lib/i386-linux-gnu/libc-2.23.so
b728c000-b728d000 rw-p 001b1000 08:01 660818     /lib/i386-linux-gnu/libc-2.23.so
b728d000-b7290000 rw-p 00000000 00:00 0 
b7290000-b7293000 r-xp 00000000 08:01 656878     /lib/i386-linux-gnu/libcom_err.so.2.1
b7293000-b7294000 r--p 00002000 08:01 656878     /lib/i386-linux-gnu/libcom_err.so.2.1
b7294000-b7295000 rw-p 00003000 08:01 656878     /lib/i386-linux-gnu/libcom_err.so.2.1
b7295000-b7363000 r-xp 00000000 08:01 2229382    /usr/lib/i386-linux-gnu/libkrb5.so.3.3
b7363000-b7364000 ---p 000ce000 08:01 2229382    /usr/lib/i386-linux-gnu/libkrb5.so.3.3
b7364000-b736a000 r--p 000ce000 08:01 2229382    /usr/lib/i386-linux-gnu/libkrb5.so.3.3
b736a000-b736b000 rw-p 000d4000 08:01 2229382    /usr/lib/i386-linux-gnu/libkrb5.so.3.3
b736b000-b736c000 rw-p 00000000 00:00 0 
b736c000-b73bb000 r-xp 00000000 08:01 2229380    /usr/lib/i386-linux-gnu/libgssapi_krb5.so.2.2
b73bb000-b73bc000 ---p 0004f000 08:01 2229380    /usr/lib/i386-linux-gnu/libgssapi_krb5.so.2.2
b73bc000-b73bd000 r--p 0004f000 08:01 2229380    /usr/lib/i386-linux-gnu/libgssapi_krb5.so.2.2
b73bd000-b73be000 rw-p 00050000 08:01 2229380    /usr/lib/i386-linux-gnu/libgssapi_krb5.so.2.2
b73be000-b73c7000 r-xp 00000000 08:01 661249     /lib/i386-linux-gnu/libcrypt-2.23.so
b73c7000-b73c8000 r--p 00008000 08:01 661249     /lib/i386-linux-gnu/libcrypt-2.23.so
b73c8000-b73c9000 rw-p 00009000 08:01 661249     /lib/i386-linux-gnu/libcrypt-2.23.so
b73c9000-b73f0000 rw-p 00000000 00:00 0 
b73f0000-b7409000 r-xp 00000000 08:01 657058     /lib/i386-linux-gnu/libz.so.1.2.8
b7409000-b740a000 r--p 00018000 08:01 657058     /lib/i386-linux-gnu/libz.so.1.2.8
b740a000-b740b000 rw-p 00019000 08:01 657058     /lib/i386-linux-gnu/libz.so.1.2.8
b740b000-b740d000 r-xp 00000000 08:01 661007     /lib/i386-linux-gnu/libutil-2.23.so
b740d000-b740e000 r--p 00001000 08:01 661007     /lib/i386-linux-gnu/libutil-2.23.so
b740e000-b740f000 rw-p 00002000 08:01 661007     /lib/i386-linux-gnu/libutil-2.23.so
b740f000-b75e2000 r-xp 00000000 08:01 655557     /lib/i386-linux-gnu/libcrypto.so.1.0.0
b75e2000-b75f2000 r--p 001d2000 08:01 655557     /lib/i386-linux-gnu/libcrypto.so.1.0.0
b75f2000-b75f9000 rw-p 001e2000 08:01 655557     /lib/i386-linux-gnu/libcrypto.so.1.0.0
b75f9000-b75fd000 rw-p 00000000 00:00 0 
b75fd000-b7689000 r-xp 00000000 08:01 655605     /lib/i386-linux-gnu/libsystemd.so.0.14.0
b7689000-b768b000 r--p 0008b000 08:01 655605     /lib/i386-linux-gnu/libsystemd.so.0.14.0
b768b000-b768c000 rw-p 0008d000 08:01 655605     /lib/i386-linux-gnu/libsystemd.so.0.14.0
b768c000-b76ae000 r-xp 00000000 08:01 657025     /lib/i386-linux-gnu/libselinux.so.1
b76ae000-b76af000 ---p 00022000 08:01 657025     /lib/i386-linux-gnu/libselinux.so.1
b76af000-b76b0000 r--p 00022000 08:01 657025     /lib/i386-linux-gnu/libselinux.so.1
b76b0000-b76b1000 rw-p 00023000 08:01 657025     /lib/i386-linux-gnu/libselinux.so.1
b76b1000-b76b2000 rw-p 00000000 00:00 0 
b76b2000-b76c0000 r-xp 00000000 08:01 656983     /lib/i386-linux-gnu/libpam.so.0.83.1
b76c0000-b76c1000 r--p 0000d000 08:01 656983     /lib/i386-linux-gnu/libpam.so.0.83.1
b76c1000-b76c2000 rw-p 0000e000 08:01 656983     /lib/i386-linux-gnu/libpam.so.0.83.1
b76c2000-b76de000 r-xp 00000000 08:01 656859     /lib/i386-linux-gnu/libaudit.so.1.0.0
b76de000-b76df000 r--p 0001b000 08:01 656859     /lib/i386-linux-gnu/libaudit.so.1.0.0
b76df000-b76e0000 rw-p 0001c000 08:01 656859     /lib/i386-linux-gnu/libaudit.so.1.0.0
b76e0000-b76ea000 rw-p 00000000 00:00 0 
b76ea000-b76f2000 r-xp 00000000 08:01 657053     /lib/i386-linux-gnu/libwrap.so.0.7.6
b76f2000-b76f3000 r--p 00007000 08:01 657053     /lib/i386-linux-gnu/libwrap.so.0.7.6
b76f3000-b76f4000 rw-p 00008000 08:01 657053     /lib/i386-linux-gnu/libwrap.so.0.7.6
b770a000-b770b000 rw-p 00000000 00:00 0 
b770b000-b770d000 r--p 00000000 00:00 0          [vvar]
b770d000-b770f000 r-xp 00000000 00:00 0          [vdso]
b770f000-b7732000 r-xp 00000000 08:01 660775     /lib/i386-linux-gnu/ld-2.23.so
b7732000-b7733000 r--p 00022000 08:01 660775     /lib/i386-linux-gnu/ld-2.23.so
b7733000-b7734000 rw-p 00023000 08:01 660775     /lib/i386-linux-gnu/ld-2.23.so
bfed9000-bfefa000 rw-p 00000000 00:00 0          [stack]
</pre>

接着启动gdb，将sshd进程attach到gdb上：
{% highlight string %}
# gdb attach 959
GNU gdb (Ubuntu 7.11.1-0ubuntu1~16.5) 7.11.1
Copyright (C) 2016 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
{% endhighlight %}

可以通过如下的命令将指定的内存地址dump到指定的目录下：
<pre>
dump memory /tmp/sshd.dump 0x800da000 0x801c2000  #这里只dump sshd进程的第一块内存
  
dump memory /tmp/sshd.dump 0x800da000 0x801c6000  #dump了指定的内存块

# "dump memory"是命令 
# "/tmp/sshd.dump"是我们想保存dump出的内容的路径。 
# 两个hex是内存地址区间，这跟/proc/959/maps的格式有些不一样。这是以0x开头的16进制表示的
</pre>

例如这里我们dump出第一块内存：
<pre>
(gdb) dump memory /opt/sshd_seg1.dump 0x80010000 0x800f8000
</pre>
之后去/opt目录下，可以看到dump出的*sshd_seg1.dump*文件：
<pre>
# ls /opt/
sshd_seg1.dmp
</pre>
用如下命令查看*sshd_seg1.dmp*内存中至少大于10字符的字符串：
<pre>
# strings -n 10 /opt/sshd_seg1.dump | more
/lib/ld-linux.so.2
libwrap.so.0
_ITM_deregisterTMCloneTable
__gmon_start__
_Jv_RegisterClasses
_ITM_registerTMCloneTable
deny_severity
allow_severity
request_init
hosts_access
libaudit.so.1
audit_open
audit_log_acct_message
libpam.so.0
pam_open_session
pam_close_session
pam_set_item
</pre>



<br />
<br />
**[参看]:**


1. [linux进程内存布局](https://www.cnblogs.com/zuofaqi/p/9846951.html)

2. [linux系统进程的内存布局](https://www.cnblogs.com/coversky/p/7619755.html)

3. [linux系统进程的内存布局](https://www.cnblogs.com/diegodu/p/4552490.html)

4. [linux进程虚拟地址空间](https://www.cnblogs.com/beixiaobei/p/10507462.html)

5. [Linux进程地址空间和进程的内存分布](https://blog.csdn.net/cl_linux/article/details/80328608)

6. [Linux的进程地址空间](https://zhuanlan.zhihu.com/p/66794639)

7. [Linux中使用gdb dump内存](https://blog.csdn.net/qq_36119192/article/details/96474833)

8. [linux tools](https://linuxtools-rst.readthedocs.io/zh_CN/latest/tool/readelf.html)

<br />
<br />
<br />





