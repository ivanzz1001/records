---
layout: post
title: ceph客户端之librbd
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


librbd模块实现了RBD(rados block device)接口，其基于Librados实现了对RBD的基本操作。librbd的架构如图5-3所示：

![ceph-chapter5-7](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter5_8.jpg)

在最上层是librbd层，模块cls_rbd是一个Cls扩展模块，实现了RBD的元数据相关的操作。RBD的数据访问直接通过Librados来实现。在最底层是OSDC层完成数据的发送。


<!-- more -->








<br />
<br />

**[参看]**





<br />
<br />
<br />

