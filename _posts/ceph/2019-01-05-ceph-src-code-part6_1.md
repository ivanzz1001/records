---
layout: post
title: ceph的数据读写
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本章介绍ceph的服务端OSD（书中简称OSD模块或者OSD）的实现。其对应的源代码在src/osd目录下。OSD模块是Ceph服务进程的核心实现，它实现了服务端的核心功能。本章先介绍OSD模块静态类图相关数据结构，再着重介绍服务端数据的写入和读取流程。

<!-- more -->


## 1. OSD模块静态类图
OSD模块的静态类图如下图6-1所示：

![ceph-chapter5-5](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter6_1.jpg)

OSD模块的核心类及其之间的静态类图说明如下：

* 类OSD和OSDService是核心类，处理一个osd节点层面的工作。在早期的版本中，OSD和OSDService是一个类。由于OSD的类承载了太多的功能，后面的版本中引入OSDService类，分担一部分原OSD类的功能；

* 类PG处理PG相关的状态维护以及实现PG层面的基本功能。其核心功能是用boost库的statechart状态机来实现的PG状态转换；

* 类ReplicatedPG继承了类PG，在其基础上实现了PG内的数据读写以及数据恢复相关的操作；

* 类PGBackend的主要功能是把数据以事务的形式同步到一个PG其他从OSD节点上

&emsp; - PGBackend的内部类PGTransaction就是同步的事务接口，其两个类型的实现分别对应RPGTransaction和ECTransaction两个子类；

&emsp; - PGBackend两个子类ReplicatedBackend和ECBackend分别对应PG的两种类型的实现


* 类SnapMapper额外保存对象和对象的快照信息，在对象的属性里保存了相关的快照信息。这里保存的快照信息为冗余信息，用于数据校验。

## 2. 







<br />
<br />

**[参看]**





<br />
<br />
<br />

