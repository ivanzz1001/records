---
layout: post
title: ceph peering机制再研究(7)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


我们在上一章讲述到在GetInfo状态下抛出GotInfo事件，会直接跳转到GetLog阶段。本章我们就从GetLog开始，继续讲述Ceph的Peering过程。


<!-- more -->

## 1. PG::choose_acting()的实现
在具体讲解GetLog阶段之前，我们先讲述PG::choose_acting()函数的实现。这是一个比较重要但却比较复杂的函数，其主要实现如下功能：

* 选出具有权威日志的OSD

* 计算PG::actingbackfill和PG::backfill_targets两个OSD列表

>注：actingbackfill保存了当前PG的acting列表，包括需要进行backfill操作的OSD列表；backfill_targets列表保存了需要进行backfill的OSD列表





<br />
<br />

**[参看]**

1. [ceph博客](http://aspirer.wang/)

2. [ceph官网](https://docs.ceph.com/en/latest/dev/internals/)

3. [PEERING](https://docs.ceph.com/en/latest/dev/peering/)

4. [分布式系统](https://blog.csdn.net/chdhust)
<br />
<br />
<br />

