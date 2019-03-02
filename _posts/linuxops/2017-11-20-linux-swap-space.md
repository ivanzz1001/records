---
layout: post
title: Linux swap交换分区介绍总结(转)
tags:
- LinuxOps
categories: linuxOps
description: Linux Swap交换分区介绍总结
---


本章我们介绍一下Linux交换分区。

<!-- more -->


## 1. 交换分区的概念

什么是Linux swap space呢？ 我们先来看看下面两段关于Linux swap space的英文介绍资料：
{% highlight string %}
Linux divides its physical RAM (random access memory) into chucks of memory called pages. 
Swapping is the process whereby a page of memory is copied to the preconfigured space on 
the hard disk, called swap space, to free up that page of memory. The combined sizes of 
the physical memory and the swap space is the amount of virtual memory available.

Swap space in Linux is used when the amount of physical memory (RAM) is full. If the 
system needs more memory resources and the RAM is full, inactive pages in memory are 
moved to the swap space. While swap space can help machines with a small amount of RAM,
it should not be considered a replacement for more RAM. Swap space is located on hard 
drives, which have a slower access time than physical memory.Swap space can be a 
dedicated swap partition (recommended), a swap file, or a combination of swap partitions
and swap files. 
{% endhighlight %}

Linux内核为了提高读写效率和速度，会将文件在内存中进行缓存，这部分内存就是Cache Memory(缓存内容）。即使你的程序运行结束后，Cache Memory也不会自动释放。这就会导致你在Linux系统中程序频繁读写文件后，你会发现可用物理内存变少。当系统的物理内存不够用的时候，就需要将物理内存中的一部分空间释放出来，以供当前运行的程序使用。那些被释放的空间可能来自一些很长时间没什么操作的程序，这些被释放的空间被临时保存到```Swap空间```中，等到那些程序要运行时，再从Swap分区中恢复保存的数据到内存中。这样，系统总是在物理内存不够时，才进行swap交换。

关于swap分区，其实我们有很多疑问，如果能弄清楚这些疑问，那么你对swap的了解掌握就差不多了。如何查看Swap分区大小？ swap分区大小应该如何设置？ 系统在什么时候会使用swap分区？ 是否可以调整？ 如何调整swap分区的大小？ swap分区有什么优劣和要注意的地方？ swap分区是否必要？ 那么我们一个一个来看看这些疑问吧！


## 2. 查看swap分区大小
查看swap分区的大小以及使用情况，一般使用free命令即可。如下所示，swap的大小为2044M，目前没有使用swap分区：
<pre>
# free -m
              total        used        free      shared  buff/cache   available
Mem:           2012         418         509           9        1084        1339
Swap:          2044           0        2044
</pre>

另外，我们还可以使用swapon命令查看当前swap相关信息： 例如swap空间是swap partition, swap size使用情况等详细信息：
<pre>
# swapon -s
Filename                                Type            Size    Used    Priority
/dev/sda5                               partition       2094076 0       -1
# cat /proc/swaps 
Filename                                Type            Size    Used    Priority
/dev/sda5                               partition       2094076 0       -1
</pre>

## 3. 分区大小设置
系统的swap分区大小设置多大才是最优呢？ 关于这个这个问题，应该说只有一个统一的参考标准，具体还应该根据系统实际情况和内存的负荷综合考虑，像ORACLE的官方文档就推荐如下设置，这个是根据物理内存来做参考的：

![swap-cfg](https://ivanzz1001.github.io/records/assets/img/linuxops/linuxops_swap_cfg.png)

另外，在其他博客中也看到下面一个推荐设置，当然我不清楚怎么得到这个标准的。是否合理也无从考证。可以作为一个参考：
<pre>
4G以内的物理内存，SWAP 设置为内存的2倍。

4-8G的物理内存，SWAP 等于内存大小。

8-64G 的物理内存，SWAP 设置为8G。

64-256G物理内存，SWAP 设置为16G
</pre>
上下两个标准确实也很让人无所适从。我就有一次在一台ORACLE数据库服务器(64GB的RAM），按照官方推荐设置了一个很大的swap分区，但是我发现其实这个swap几乎很少用到，其实是浪费了磁盘空间。所以如果根据系统实际情况和内存的负荷综合考虑，其实应该按照第二个参考标准设置为8G即可。当然这个只是个人的一些认知。


## 4. 释放swap分区空间
我们使用如下命令查看当前交换空间使用情况：
<pre>
# free -m
              total        used        free      shared  buff/cache   available
Mem:           2012         418         509           9        1084        1339
Swap:          2044          44        2000

# swapon -s
Filename                                Type            Size    Used    Priority
/dev/sda5                               partition       2094076 44       -1
</pre>

然后使用```swapoff```命令关闭交换分区：
<pre>
# swapoff /dev/sda5
</pre>
再接着使用```swapon```命令启用交换分区，此时查看交换分区的使用情况，你会发现used为0了：
<pre>
# swapon /dev/sda5
# swapon -s
Filename                                Type            Size    Used    Priority
/dev/sda5                               partition       2094076 0       -1
</pre>



<br />
<br />

**[参看]:**

1. [Linux Swap交换分区介绍总结](https://blog.csdn.net/x_r_su/article/details/52957559)



<br />
<br />
<br />


