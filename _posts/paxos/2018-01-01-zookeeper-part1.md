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
Zookeeper致力于提供一个高性能、高可用，且具有严格的顺序访问控制能力（主要是写操作的严格顺序性）的分布式协调服务。高性能使得Zookeeper能够应用于那些对系统吞吐有明确要求的大型分布式系统中，高可用使得分布式的单点问题得到了很好的解决，而严格的顺序访问控制使得客户端能够基于Zookeeper实现一些复杂的同步原语。下面我们来具体看一下Zookeeper的四个设计目标：

###### 目标1： 简单的数据模型
Zookeeper使得分布式程序能够通过一个共享的、树型结构的名字空间来进行相互协调。这里所说的树型结构的名字空间，是指Zookeeper服务器内存中的一个数目模型，其由一系列被称为```znode```的数据节点组成。总的来说，其数据模型类似于一个文件系统，而ZNode之间的层次关系，就像文件系统的目录结构一样。不过和传统的磁盘文件系统不同的是，Zookeeper将全量数据存储在内存中，以此来实现提高服务器吞吐、减少延迟的目的。

###### 目标2： 可以构建集群
一个Zookeeper集群通常由一组机器组成，一般3~5台机器就可以组成一个可用的Zookeeper集群了。如下图所示：

![zookeeper-service](https://ivanzz1001.github.io/records/assets/img/paxos/zookeeper-service.png)

组成Zookeeper集群的每台机器都会在内存中维护当前的服务器状态，并且每台机器之间都互相保持着通信。值得一提的是，只要集群中存在超过一半的机器能够正常工作，那么整个集群就能够正常对外服务。

Zookeeper的客户端程序会选择和集群中任意一台机器共同来创建一个TCP连接，而一旦客户端和某台Zookeeper服务器之间的连接断开后，客户端会自动连接到集群中的其他机器。

###### 目标3： 顺序访问
对于来自客户端的每个更新请求，Zookeeper都会分配一个全局唯一的递增编号，这个编号反映了所有事务操作的先后顺序（我们称这个编号为zxid，即Zookeeper Transaction ID)，应用程序可以使用Zookeeper的这个特性来实现更高层次的同步原语。

###### 目标4： 高性能
由于Zookeeper将全量数据存储在内存中，并直接服务于客户端的所有非事务请求，因此它尤其适用于以读操作为主的应用场景。作者曾经以3台3.4.3版本的Zookeeper服务器组成集群进行性能压测，100%读请求的场景下压测结果是12~13W的QPS.

### 1.3 Zookeeper基本概念
本节将介绍Zookeeper的几个核心概念。后面很多地方都会涉及到。

1） **集群角色**

通常在分布式系统中，构成一个集群的每一台机器都有自己的角色，最典型的集群模式就是master/slave模式（即主备模式）。在这种模式下，我们把能够处理所有写操作的机器称为master机器，把所有通过异步复制获取最新数据，并提供读服务的机器称为Slave机器。

而在Zookeeper中，这些概念被颠覆了。它没有沿用传统的Master/Slave概念，而是引入了Leader、Follower和Observer三种角色。Zookeeper集群中的所有机器通过一个Leader选举过程来选定一台被称为```Leader```的机器，Leader服务器为客户端提供读和写服务。除Leader外，其他机器包括Follower和Observer。Follower和Observer都能够提供读服务，唯一的区别在于，Observer机器不参与Leader选举过程，也不参与写操作的“过半写成功”策略，因此Observer可以在不影响写性能的情况下提升集群的读性能。

2） **会话(Session)**

Session是指客户端会话，在讲解会话之前，我们首先来了解一下客户端连接。在Zookeeper中，一个客户端连接是指客户端与服务器之间的一个TCP长连接。Zookeeper对外的服务端口默认是```2181```，客户端启动的时候，首先会与服务器建立一个TCP连接，从第一次连接建立开始，客户端会话的生命周期也开始了，通过这个连接，客户端能够通过心跳检测与服务器保持有效的会话，也能够向Zookeeper服务器发送请求并接受响应，同时还能够通过该连接接收来自服务器的Watch事件通知。

Session的sessionTimeout值用来设置一个客户端会话的超时时间。当由于服务器压力太大、网络故障或是客户端主动断开连接等各种原因导致客户端连接断开时，只要在sessionTimeout规定的时间内能够重新连接上集群中任意一台服务器，那么之前创建的会话仍然有效。


3） **数据节点(Znode)**

在谈到分布式的时候，我们通常说的“节点”是指组成集群的每一台机器。然而，在Zookeeper中，“节点”分为两类，第一类同样是指构成集群的机器，我们称之为机器节点； 第二类则是指数据模型中的数据单元，我们称之为数据节点———ZNode。Zookeeper将所有数据存储在内存中，数据模型是一棵树(ZNode Tree)，由斜杠(/)进行分割的路径，就是一个Znode，例如/foo/path1。每个ZNode上都会保存自己的数据内容，同时还会保存一系列属性信息。

在Zookeeper中，ZNode可以分为持久节点和临时节点两类。所谓持久节点是指一旦这个ZNode被创建了，除非主动进行ZNode移除操作，否者这个ZNode将一直保存在Zookeeper上。而临时节点就不一样了，它的生命周期和客户端会话绑定，一旦客户端会话失效，那么这个客户端所创建的所有临时节点都会被移除。另外，Zookeeper还允许用户为每个节点添加一个特殊的属性： SEQUENTIAL。一旦节点被标记上这个属性，那么这个节点被创建的时候，Zookeeper会自动在其节点后面追加一个整形数字，这个整形数字是一个由父节点维护的自增数字。

4） **版本**

在上面我们提到，Zookeeper的每个ZNode上都会存储数据，对应于每个ZNode，Zookeeper都会为其维护一个叫做```Stat```的数据结构，Stat中记录了这个ZNode的三个数据版本，分别是version(当前ZNode的版本）、cversion(当前ZNode子节点的版本）和aversion(当前ZNode的ACL版本）。

5） **Watcher**

Watcher(事件监听器），是Zookeeper中的一个很重要的特性。Zookeeper允许用户在指定节点上注册一些Watcher，并且在一些特定事件触发的时候，Zookeeper服务端会将事件通知到感兴趣的客户端上去，该机制是Zookeeper实现分布式协调服务的重要特性。

6） **ACL**

Zookeeper采用ACL(Access Control Lists)策略来进行权限控制，类似于Unix文件系统的权限控制。Zookeeper定义了如下5种权限：

* CREATE: 创建子节点的权限

* READ： 获取节点数据和子节点列表的权限

* WRITE: 更新节点数据的权限

* DELETE: 删除子节点的权限

* ADMIN: 设置节点ACL的权限

其中尤其需要注意的是，CREATE和DELETE这两种权限都是针对子节点的权限控制。


<br />
<br />
**参看：**

1. [Zookeeper和 Google Chubby对比分析](https://www.cnblogs.com/grefr/p/6088115.html)

2. [The Chubby lock service for loosely coupled distributed systems](https://github.com/lwhile/The-Chubby-lock-service-for-loosely-coupled-distributed-systems-zh_cn)

3. [zookeeper官网](https://zookeeper.apache.org/)

<br />
<br />
<br />


