---
layout: post
title: Zookeeper的安装及使用
tags:
- paxos
categories: paxos
description: Zookeeper的安装及使用
---

Apache ZooKeeper是由Apache Hadoop的子项目发展而来，于2010年11月正式成为了Apache的顶级项目。Zookeeper为分布式应用提供了高效且可靠的分布式协调服务、提供了诸如统一命名服务、配置管理和分布式锁等分布式的基础服务。在解决分布式数据一致性方面，Zookeeper并没有直接采用paxos算法，而是采用了一种被称为```ZAB```(Zookeeper Atomic Broadcast)的一致性协议。

本文我们将首先对Zookeeper进行一个整体上的介绍，包括Zookeeper的设计目标、由来以及它的基本概念，然后将会重点介绍ZAB这一Zookeeper中非常重要的一致性协议。

<!-- more -->


## 1. zookeeper简介




<br />
<br />
**参看：**

1. [Zookeeper和 Google Chubby对比分析](https://www.cnblogs.com/grefr/p/6088115.html)

2. [The Chubby lock service for loosely coupled distributed systems](https://github.com/lwhile/The-Chubby-lock-service-for-loosely-coupled-distributed-systems-zh_cn)

3. [zookeeper官网](https://zookeeper.apache.org/)

<br />
<br />
<br />


