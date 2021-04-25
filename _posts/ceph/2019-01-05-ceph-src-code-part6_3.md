---
layout: post
title: ceph的数据读写(3)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本文会从更高一点的层次来介绍ceph数据的读写流程，并使用流程图的方式展示，便于阅读与理解。

<!-- more -->

## 1. OSD数据读写相关数据结构

![ceph-chapter63-1](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter63_1.jpg)

## 2. 主OSD处理流程
### 2.1 主OSD读写流程
本文基于Jewel版本对OSD读写流程进行分析，如下图所示：


![ceph-chapter63-2](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter63_2.jpg)

具体的处理流程从图中就可以体现出来，这里就不作过多的描述，需要提的几点：

1）读请求由Primary OSD来进行处理；

2）对于写请求，Primary OSD先发消息到副本OSD，然后记录PGLog(在log_operation里只是构造transaction，真正写到磁盘是和journal一起写的），然后再生成本地事务进行本地写的处理；

3）写请求涉及到两步操作，一个是写journal，一个是写本地缓存(page cache)。对于每一个副本都有这两步操作，每个副本都是先写journal，然后再写本地缓存。如果是3副本，就涉及到6次写操作；

4）Primary OSD创建了2个回调函数来处理写journal和写到缓存(分别是C_OSD_RepopCommit和C_OSD_RepopApplied），主副本的写和从副本的写没有先后顺序，有可能主的journal先写完，也有可能从的journal先写完，ceph不管这个顺序，只要保证3副本都写完了之后才返回客户端响应（degrade情况下例外），3个副本的journal写完成(all_commit)，会返回客户端“写操作完成”，而3个副本都写本地缓存完成后（all_applied)，才返回客户端“数据可读”；





<br />
<br />

**[参看]**

1. [ceph OSD读写流程](http://sysnote.github.io/2015/11/25/ceph-osd-rw1/)




<br />
<br />
<br />

