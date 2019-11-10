---
layout: post
title: Zookeeper架构、ZAB协议、选举
tags:
- paxos
categories: paxos
description: Zookeeper之ZAB协议
---


本节我们我们首先会介绍Zookeeper的整体架构，之后再重点介绍一下相关的选举算法。


<!-- more -->

## 1. Zookeeper是什么
Zookeeper是一个分布式协调服务，可用于服务发现、分布式锁、分布式领导选举、配置管理等。

这一切的基础，都是Zookeeper提供了一个类似于Linux文件系统的树形结构（可认为是轻量级的内存文件系统，但只适合存少量信息，完全不适合存储大量文件或者大文件），同时提供了对每个节点的监控与通知机制。

既然是一个文件系统，就不得不提Zookeeper是如何保证数据一致性的。本文将介绍Zookeeper如何保证数据一致性，如何进行Leader选举，以及数据监控、通知机制的语义保证。

## 2. Zookeeper架构
### 2.1 角色
Zookeeper集群是一个基于主从复制的高可用集群，每个服务器承担如下三种角色中的一种：

* Leader： 一个Zookeeper集群同一时间只会有一个实际工作的Leader，它会发起并维护与各Follower及Observer间的心跳。所有的写操作必须要通过Leader完成，再由Leader将写操作广播给其他服务器。

* Follower：一个Zookeeper集群可以同时存在多个Follower，它会响应Leader的心跳。Follower可直接处理并返回客户端的读请求，同时会将写请求转发给Leader处理，并负责在Leader处理写请求时对请求进行投票。

* Observer： 角色与Follower类似，但无投票权



![zoo-arch](https://ivanzz1001.github.io/records/assets/img/paxos/zookeeper_arch.png)



<br />
<br />
**参看：**

1. [Zookeeper架构、ZAB协议、选举](https://www.cnblogs.com/fanguangdexiaoyuer/p/10311228.html)

2. [ZAB协议](https://www.cnblogs.com/liuyi6/p/10726338.html)

3. [Zookeeper 原理 之ZAB，选举](https://blog.csdn.net/weixin_40792878/article/details/87475881)

<br />
<br />
<br />


