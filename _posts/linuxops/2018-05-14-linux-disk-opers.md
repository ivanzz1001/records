---
layout: post
title: linux中硬盘相关操作
tags:
- LinuxOps
categories: linuxOps
description: linux中硬盘相关操作
---


本节我们介绍一下Linux中硬盘的相关操作： 查看硬盘信息、分区、硬盘检测等


<!-- more -->




## 1. sgdisk用法

**1) 硬盘格式化**

下面删除硬盘/dev/sda上面的所有分区信息。
<pre>
#  sgdisk -Zog /dev/sda
</pre>

**2) 修复硬盘**
<pre>
# xfs_repair /dev/sda1
</pre>

## 2. du与df命令

1） **du命令**

本命令用于统计目录（或文件）所占用的磁盘空间大小。所支持的命令行参数主要有：

* ```-a``` 统计目录中的所有文件大小，而不仅仅显示目录所占用的磁盘空间

* ```-b``` 统计目录中的文件大小，以KB为单位

* ```-h``` 以便于人类阅读的方式统计目录中的文件大小

参看如下示例：
{% highlight string %}
# # du -a -h ./
4.0K    ./workspace/test.cpp
8.0K    ./workspace/test
16K     ./workspace
28K     ./.bash_history
4.0K    ./.bashrc
4.0K    ./.profile
0       ./.cache/motd.legal-displayed
4.0K    ./.cache
12K     ./.viminfo
72K     ./
{% endhighlight %}

2) **df命令**

本命令用于显示磁盘分区上可使用的磁盘空间。这里记住两个参数就可以：

* ```-a``` 查看系统当前挂载的所有文件系统所占用的磁盘空间(单位默认是KB)
<pre>
# df -a
Filesystem     1K-blocks    Used Available Use% Mounted on
sysfs                  0       0         0    - /sys
proc                   0       0         0    - /proc
udev             1010636       0   1010636   0% /dev
devpts                 0       0         0    - /dev/pts
tmpfs             206084    6564    199520   4% /run
/dev/sda1       39089600 4434104  32646820  12% /
securityfs             0       0         0    - /sys/kernel/security
tmpfs            1030404     260   1030144   1% /dev/shm
tmpfs               5120       0      5120   0% /run/lock
tmpfs            1030404       0   1030404   0% /sys/fs/cgroup
cgroup                 0       0         0    - /sys/fs/cgroup/systemd
pstore                 0       0         0    - /sys/fs/pstore
cgroup                 0       0         0    - /sys/fs/cgroup/memory
cgroup                 0       0         0    - /sys/fs/cgroup/cpu,cpuacct
cgroup                 0       0         0    - /sys/fs/cgroup/perf_event
cgroup                 0       0         0    - /sys/fs/cgroup/net_cls,net_prio
cgroup                 0       0         0    - /sys/fs/cgroup/hugetlb
cgroup                 0       0         0    - /sys/fs/cgroup/freezer
cgroup                 0       0         0    - /sys/fs/cgroup/cpuset
cgroup                 0       0         0    - /sys/fs/cgroup/pids
cgroup                 0       0         0    - /sys/fs/cgroup/blkio
cgroup                 0       0         0    - /sys/fs/cgroup/devices
systemd-1              -       -         -    - /proc/sys/fs/binfmt_misc
mqueue                 0       0         0    - /dev/mqueue
hugetlbfs              0       0         0    - /dev/hugepages
debugfs                0       0         0    - /sys/kernel/debug
fusectl                0       0         0    - /sys/fs/fuse/connections
tmpfs             206084      56    206028   1% /run/user/1000
gvfsd-fuse             0       0         0    - /run/user/1000/gvfs
/dev/sr1         1534080 1534080         0 100% /media/ivan1001/Ubuntu 16.04.2 LTS i386
/dev/sr0           49148   49148         0 100% /media/ivan1001/CDROM
tmpfs             206084       0    206084   0% /run/user/0
binfmt_misc            0       0         0    - /proc/sys/fs/binfmt_misc
</pre>

* ```-h``` 以方便人类阅读的方式展示各文件系统所占用的磁盘空间
<pre>
# df -h
Filesystem      Size  Used Avail Use% Mounted on
udev            987M     0  987M   0% /dev
tmpfs           202M  6.5M  195M   4% /run
/dev/sda1        38G  4.3G   32G  12% /
tmpfs          1007M  260K 1006M   1% /dev/shm
tmpfs           5.0M     0  5.0M   0% /run/lock
tmpfs          1007M     0 1007M   0% /sys/fs/cgroup
tmpfs           202M   56K  202M   1% /run/user/1000
/dev/sr1        1.5G  1.5G     0 100% /media/ivan1001/Ubuntu 16.04.2 LTS i386
/dev/sr0         48M   48M     0 100% /media/ivan1001/CDROM
tmpfs           202M     0  202M   0% /run/user/0
</pre>



<br />
<br />

**[参看]**

1. [SGDISK 的使用](http://www.rodsbooks.com/gdisk/sgdisk.html)

2. [sgdisk基本用法](https://blog.csdn.net/ygtlovezf/article/details/76269800)

3. [sgdisk常用操作](http://hustcat.github.io/sgdisk-basic/)

2. [GNU Parted的使用](https://wiki.archlinux.org/index.php/GNU_Parted)



<br />
<br />
<br />


