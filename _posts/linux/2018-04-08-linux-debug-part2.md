---
layout: post
title: gdb如何确定内存释放(转)
tags:
- LinuxOps
categories: linux
description: linux调试
---

文章转自[gdb如何确定内存已经释放](https://blog.csdn.net/weixin_36356040/article/details/112432311)，在此做一个记录，防止原文丢失，并便于后续查阅。

<!-- more -->

## 1. 问题发现
为了更好地实现对项目地管理，我们将组内一个项目迁移到```MDP框架```(基于Spring Boot)，随后我们就发现系统会频繁报出Swap区域使用量过高的异常。

笔者被叫去排查原因，发现配置了4G堆内内存，但实际使用的物理内存竟然高达7G，确实不正常。

JVM参数配置情况如下：
<pre>
-XX:MetaspaceSize=256M -XX:MaxMetaspaceSize=256M -XX:+AlwaysPreTouch -XX:ReservedCodeCacheSize=128m -XX:InitialCodeCacheSize=128m, -Xss512k -Xmx4g -Xms4g,-XX:+UseG1GC -XX:G1HeapRegionSize=4M
</pre>

实际使用的物理内存如下图所示(top命令显式的内存情况)：

![linux-debug](https://ivanzz1001.github.io/records/assets/img/linux/linux-debug-figure1.jfif)





<br />
<br />
**[参看]:**



1. [gdb如何确定内存已经释放](https://blog.csdn.net/weixin_36356040/article/details/112432311)


<br />
<br />
<br />





