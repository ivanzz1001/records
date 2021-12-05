---
layout: post
title: Linux Page Cache Basics
tags:
- LinuxOps
categories: linuxOps
description: Linux Page Cache Basics
---


Under Linux, the Page Cache accelerates many accesses to files on non volatile storage. This happens because, when it first reads from or writes to data media like hard drives, Linux also stores data in unused areas of memory, which acts as a cache. If this data is read again later, it can be quickly read from this cache in memory. This article will supply valuable background information about this page cache.



<!-- more -->

## 1. Page Cache Or Buffer Cache

目前，Buffer Cache通常被当作Page Cache来使用。在Linux Kernel 2.2版本之前，Page Cache和Buffer Cache通常是分开的，Linux同时含有这两种Cache。而从Linux Kernel 2.4版本开始，这两种Cache合并为一。因此，在当前其实就只含有一种cache，那就是Page Cache。

## 2. Functional Approach

### 2.1 Memory Usage
在Linux中，我们可以通过```free -m```来查看当前有多少主内存(main memory)被用作page cache:
{% highlight string %}
#  free -m
             total       used       free     shared    buffers     cached
Mem:         15976      15195        781          0        167       9153
-/+ buffers/cache:       5874      10102
Swap:         2000          0       1999
{% endhighlight %}

>注：-m选项所显式的内存单位是MB


### 2.2 Writting 
假如执行数据写操作，则数据首先会被写入到Page Cache中，并将所写入的页作为dirty pages中的一页进行管理。Dirty意味着数据存放于Page Cache中，需要首先将数据写入到底层的存储系统。这些dirty pages中的内容会被周期性(或通过sync()、fsync()等系统调用)写入到底层的存储设备。系统的存储设备可能是RAID控制器，也可能是硬盘。

下面的例子演示了：创建一个10MB的文件，然后被写入到Page Cache中。在此过程中，dirty pages数量会增长，直到通过```sync```命令将数据刷入到SSD硬盘。
{% highlight string %}
# dd if=/dev/zero of=testfile.txt bs=1M count=10
10+0 records in
10+0 records out
10485760 bytes (10 MB) copied, 0,0121043 s, 866 MB/s

# cat /proc/meminfo | grep Dirty
Dirty:             10260 kB

# sync
# cat /proc/meminfo | grep Dirty
Dirty:                 0 kB
{% endhighlight %}

1) **Up to Version 2.6.31 of the Kernel: pdflush**

在Linux 2.6.31版本(包括2.6.31)之前，通过```pdflush```线程来确保dirty pages中的数据被周期性的写到storage device。

查看代码：[pdflush](https://elixir.bootlin.com/linux/v2.6.31-rc1/source/mm/pdflush.c)

2） **As of Version 2.6.32: per-backing-device based writeback**

由于pdflush有一些性能上的问题，因此，在Linux Kernel 2.6.32版本中，Jens Axboe开发了一个新的、更高效的writeback机制。

查看相应代码：[page writeback](https://elixir.bootlin.com/linux/v5.16-rc3/source/mm/page-writeback.c)

writeback机制为每一个设备设置若干线程。下面的例子中，展示了某主机为SSD硬盘(/dev/sda)以及普通硬盘(/dev/sdb)所设置的线程：
{% highlight string %}
# ls -l /dev/sda
brw-rw---- 1 root disk 8, 0 2011-09-01 10:36 /dev/sda
# ls -l /dev/sdb
brw-rw---- 1 root disk 8, 16 2011-09-01 10:36 /dev/sdb

# ps -eaf | grep -i flush
root       935     2  0 10:36 ?        00:00:00 [flush-8:0]
root       936     2  0 10:36 ?        00:00:00 [flush-8:16]
{% endhighlight %}

>注：当前可能需要使用如下命令来查看
>
> ps -eaf | grep -i writeback


### 2.3 Reading
File blocks不仅仅在执行write操作时会被写入到Page Cache中，在执行read操作时也会被写入到Page Cache。例如，当你前后两次读取同一个100MB的文件，第二次读取速度明显会更快。这是因为第二次读取的数据直接来自于内存中的Page Cache，而不需要再次从硬盘读。如下的例子展示了当重新播放一个200MB字节的视频时，Page Cache的增长情况：
{% highlight string %}
# free -m
             total       used       free     shared    buffers     cached
Mem:          3884       1812       2071          0         60       1328
-/+ buffers/cache:        424       3459
Swap:         1956          0       1956

# vlc video.avi
[...]

# free -m
             total       used       free     shared    buffers     cached
Mem:          3884       2056       1827          0         60       1566
-/+ buffers/cache:        429       3454
Swap:         1956          0       1956
{% endhighlight %}

假如当前Linux内存不够用，则未被使用的Page Cache区域会被自动释放。

### 2.4 Optimizing the Page Cache

通过操作系统自动的将file blocks存放到Page Cache通常是十分有好处的。但是有些数据，比如log文件或MySQL dump文件，通常在写完之后并不需要马上读取。因此，在这种情况下，file blocks经常是非必要性的占用了Page Cache，从而可能挤压了一些可能更需要使用page cache的数据。

周期性的执行logrotate，并对相应的日志进行压缩就是一个良好的行为。当一个500MB的日志文件被压缩成10MB，则原来的日志文件就会从Page Cache中移除，这样就可以节省490MB的Page Cache空间。

因此，对某些应用来说，将page cache中的file blocks清理是完全合理的。例如，rsync命令其实就已经有这样一个patch可用。

## 3. References

* [The Buffer Cache (Section 15.3)](http://books.google.de/books?id=lZpW6xmXrzoC&pg=PA348&dq=linux+buffer+cache+page+cache&cd=1) page 348, Linux-Kernel Manual: Guidelines for the Design and Implementation of Kernel 2.6, Robert Love, Addison-Wesley, 2005

* [Linux 2.6. – 32 Per-backing-device based writeback](http://kernelnewbies.org/Linux_2_6_32#head-72c3f91947738f1ea52f9ed21a89876730418a61) (kernelnewbies.org)
 
* [Clearing The Linux Buffer Cache](http://www.straylightrun.net/2009/12/03/clearing-the-linux-buffer-cache/) (blog.straylightrun.net, 03.12.2009)
 
* [Improving Linux performance by preserving Buffer Cache State](http://insights.oetiker.ch/linux/fadvise.html) (insights.oetiker.ch)


## 4. Additional Information

* [Page Cache, the Affair Between Memory and Files](http://duartes.org/gustavo/blog/post/page-cache-the-affair-between-memory-and-files) (Blog)

* [Linux buffer cache state](http://panoskrt.wordpress.com/2009/10/27/linux-buffer-cache-state/) (Blog)

* [The Linux Page Cache and pdflush: Theory of Operation and Tuning for Write-Heavy Loads](http://www.westnet.com/~gsmith/content/linux-pdflush.htm) (Last update 8/08/2007)

* [drop_caches](http://www.linuxinsight.com/proc_sys_vm_drop_caches.html) (linuxinsight.com)

* [Examining Linux 2.6 Page-Cache Performance](http://www.linuxinsight.com/ols2005_examining_linux_2_6_page_cache_performance.html) (linuxinsight.com)

* [Page cache](http://en.wikipedia.org/wiki/Page_cache) (en.wikipedia.org)

<br />
<br />

**[参看]:**

1. [Linux操作系统原理-buffer与cache](https://is-cloud.blog.csdn.net/article/details/105896326)

2. [linux buffer 刷到磁盘](https://blog.csdn.net/weixin_29475313/article/details/116713846)

3. [Linux Page Cache Basics](https://www.thomas-krenn.com/en/wiki/Linux_Page_Cache_Basics)

4. [mm/pdflush.c - worker threads for writing back filesystem data](http://reqorts.qa.ubuntu.com/reports/ogasawara/gcov-isos/coverage-example/mm/pdflush.c.gcov.html)

5. [pdflush](https://elixir.bootlin.com/linux/v2.6.31-rc1/source/mm/pdflush.c)

6. [Linux 3.2中回写机制的变革](https://www.cnblogs.com/youngerchina/p/5624457.html)
<br />
<br />
<br />


