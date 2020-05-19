---
layout: post
title: ceph的数据读写(2)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本章介绍ceph的服务端OSD（书中简称OSD模块或者OSD）的实现。其对应的源代码在src/osd目录下。OSD模块是Ceph服务进程的核心实现，它实现了服务端的核心功能。本章先介绍OSD模块静态类图相关数据结构，再着重介绍服务端数据的写入和读取流程。

<!-- more -->


## 1. 读写操作的序列图
写操作的序列图如下图6-2所示：

![ceph-chapter6-2](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter6_2.jpg)







<br />
<br />

**[参看]**





<br />
<br />
<br />

