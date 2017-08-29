---
layout: post
title: linux 操作系统资源限制
tags:
- LinuxOps
categories: linux
description: Linux操作系统资源限制
---

本文主要讲述一下Linux操作系统上的一些资源限制，做一个记录。
<!-- more -->


## 1. Linux Thread资源限制
目前线程资源限制由以下几个系统参数共同决定：

* 参数```/proc/sys/kernel/threads-max```，有直接关系，每个进程中做多创建的的线程数目
* 参数```/proc/sys/kernel/pid_max```，有直接关系，系统中最多分配的pid数量
* 参数```/proc/sys/vm/max_map_count```，数量越大，能够创建的线程数目越多，目前具体关系未明

通过echo设置：
{% highlight string %}
# echo 204800 > /proc/sys/kernel/threads-max
# echo 204800 > /proc/sys/kernel/pid_max
# echo 204800 > /proc/sys/kernel/pid_max
{% endhighlight %}




<br />
<br />
<br />





