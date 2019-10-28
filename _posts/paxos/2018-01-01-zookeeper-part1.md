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

Zookeeper是一个开放源代码的分布式协调服务，由知名互联网公司雅虎创建，是Google Chubby的开源实现。Zookeeper的设计目标是将那些复杂且容易出错的分布式一致性服务封装起来，构成一个高效可靠的原语集，并以一系列简单易用的接口提供给用户使用。

### 1.1 Zookeeper是什么
Zookeeper是一个典型的分布式数据一致性的解决方案，分布式应用程序可以基于它实现诸如数据发布/订阅、负载均衡、命名服务、分布式协调/通知、集群管理、Master选举、分布式锁和分布式队列等功能。Zookeeper可以保证如下分布式一致性特性。

* 顺序一致性： 从同一个客户端发起的事务请求，最终将会严格的按照其发起顺序被应用到Zookeeper中去

* 原子性： 所有事务请求的处理结果在整个集群中所有机器上的应用情况是一致的，也就是说，要么整个集群所有机器都成功应用了某一个事务，要么都没用应用，一定不会出现集群中部分机器应用了该事务，而另外一部分没有应用的情况。

* 单一视图(single system image): 无论客户端连接的是哪个Zookeeper服务器，其看到的服务端数据模型都是一致的

* 可靠性： 一旦服务端成功地应用了一个事务，并完成对客户端的响应，那么该事务所引起的服务端状态变更将会一直保存下来，除非有另一个事务又对其进行了变更。

* 实时性： 通常人们看到实时性的第一反应是，一旦一个事务被成功应用，那么客户端能够立即从服务端上读取到这个事务变更后的最新数据状态。这里需要注意的是，Zookeeper仅仅保证在一定的时间段内，客户端最终一定能够从服务端上读取到最新的数据状态。

### 1.2 Zookeeper的设计目标
Zookeeper致力于提供一个高性能、高可用，且具有严格的顺序访问控制能力（主要是写操作的严格顺序性）的分布式协调服务。高性能使得Zookeeper能够应用于那些对系统吞吐



<br />
<br />
**参看：**

1. [Zookeeper和 Google Chubby对比分析](https://www.cnblogs.com/grefr/p/6088115.html)

2. [The Chubby lock service for loosely coupled distributed systems](https://github.com/lwhile/The-Chubby-lock-service-for-loosely-coupled-distributed-systems-zh_cn)

3. [zookeeper官网](https://zookeeper.apache.org/)

<br />
<br />
<br />


