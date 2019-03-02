---
layout: post
title: Linux Swap交换分区介绍总结(转)
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





<br />
<br />

**[参看]:**

1. [Linux Swap交换分区介绍总结](https://blog.csdn.net/x_r_su/article/details/52957559)



<br />
<br />
<br />


