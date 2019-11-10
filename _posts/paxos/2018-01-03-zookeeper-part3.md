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

### 2.2 原子广播
为了保证写操作的一致性与可用性，Zookeeper专门设计了一种名为原子广播(ZAB)的支持崩溃恢复的一致性协议。基于该协议，Zookeeper实现了一种主从模式的系统架构来保持集群中各个副本之间的数据一致性。

根据ZAB协议，所有的写操作都必须通过Leader完成，Leader写入本地日志后再复制到所有的Follower节点。

一旦Leader节点无法工作，ZAB协议能够自动从Follower节点重新选出一个合适的替代者，即新的Leader，该过程即为```领导者```选举。在Leader选举过程中，是ZAB协议中最为重要和复杂的过程。

### 2.3 写操作
###### 2.3.1 写Leader
通过Leader进行写操作流程如下图所示

![zoo-write](https://ivanzz1001.github.io/records/assets/img/paxos/zoo_writeleader.png)

由上图可见，通过Leader进行写操作，主要分为5步：

1） 客户端向Leader发起写请求；

2） Leader将写请求以Proposal的形式发给所有Follower并等待ACK；

3） Follower收到Leader的Proposal后返回ACK；

4） Leader得到过半数的ACK(Leader对自己默认有一个ACK）后向所有的Follower和Observer发送Commit

5） Leader将处理结果返回给客户端

这里要注意：

* Leader不需要得到Observer的ACK，即Observer无权投票；

* Leader不需要得到所有Follower的ACK，只要收到过半的ACK即可，同时Leader本身对自己有一个ACK。上图中有4个Follower，只需要其中两个返回ACK即可，因为(2+1)/(4+1) > 1/2;

* Observer虽然无权投票，但仍须同步Leader的数据，从而在处理读请求时可以返回尽可能新的数据

###### 2.3.2 写Follower/Observer
通过Follower/Observer进行写操作流程如下图所示：

![zoo-write](https://ivanzz1001.github.io/records/assets/img/paxos/zoo_writefollower.png)

从上图可见：

* Follower/Observer均可以接受写请求，但不能直接处理，而需要将写请求转发给Leader处理；

* 除了多了一步请求转发，其他流程与直接写Leader无任何区别；

### 2.4 读操作
Leader/Follower/Observer都可直接处理读请求，从本地内存中读取数据并返回给客户端即可。

![zoo-read](https://ivanzz1001.github.io/records/assets/img/paxos/zoo_read.png)

由于处理读请求不需要服务器之间的交互，Follower/Observer越多，整体可处理的读请求量越大，也即读性能越好。

## 3. FastLeaderElection原理
### 3.1 术语介绍

1) **myid**

每个zookeeper服务器，都需要在数据文件夹下创建一个名为```myid```的文件，该文件包含整个Zookeeper集群唯一的ID（整数）。例如，某Zookeeper集群包含三台服务器，hostname分别为zoo1、zoo2和zoo3，其中myid分别为1、2和3，则在配置文件中其ID与hostname必须一一对应，如下所示。在该配置文件中，server.后面的数据即为myid:
<pre>
server.1=zoo1:2888:3888
server.2=zoo2:2888:3888
server.3=zoo3:2888:3888
</pre>

2) **zxid**

类似于RDBMS中的事务ID，用于标识一次更新操作的Proposal ID。为了保证顺序性，该zkid必须单调递增。因此，Zookeeper使用一个64位的数来表示，高32位是Leader的epoch，从1开始，每次选出新的Leader，epoch加1。低32位为该epoch内的序号(counter)，每次epoch变化，都将低32位的序号重置。这样保证了zkid的全局递增性。

### 3.2 支持的Leader选举算法
可通过electionAlg配置项设置Zookeeper用于Leader选举的算法。到3.4.10版本为止，可选项有：

* 0---基于UDP的LeaderElection

* 1---基于UDP的FastLeaderElection

* 2---基于UDP和认证的FastLeaderElection

* 3---基于TCP的FastLeaderElection

在3.4.10版本中，默认值是3，也即基于TCP的FastLeaderElection。另外三种算法已经被弃用，并且有计划在之后的版本中将它们彻底删除而不再支持。

### 3.3 FastLeaderElection
FastLeaderElection选举算法是标准的Fast Paxos算法实现，可解决LeaderElection选举算法收敛速度慢的问题。

###### 3.3.1 服务器状态
* LOOKING: 不确定Leader状态。该状态下的服务器认为当前集群中没有Leader，会发起Leader选举；

* FOLLOWING: 跟随者状态。表明当前服务器角色是Follower，并且它知道Leader是谁

* LEADING： 领导者状态。表明当前服务器角色是Leader，它会维护与Follower间的心跳

* OBSERVING: 观察者状态。表明当前服务器角色是Observer，与Follower唯一的不同在于不参与选举，也不参与集群写操作时的投票；

###### 3.3.2 选票数据结构

每个服务器在进行Leader选举时，都会发送如下关键信息：

* logicClock: 每个服务器都会维护一个自增的整数，名为logicClock，它表示这是该服务器发起的第多少轮投票；

* state: 当前服务器的状态

* self_id: 当前服务器的myid

* self_zxid: 当前服务器上所保存的数据的最大zxid

* vote_id: 被推举的服务器的myid

* vote_zxid: 被推举的服务器上所保存的数据的最大zxid

###### 3.3.3 投票流程
1） **自增选举轮次**

Zookeeper规定所有有效的投票都必须在同一轮次中。每个服务器在开始新一轮投票时，会先对自己维护的logicClock进行自增操作。

2） **初始化选票**

每个服务器在广播自己的选票前，会将自己的投票箱清空。该投票箱记录了所收到的选票。例：服务器2投票给服务器3，服务器3投票给服务器1




<br />
<br />
**参看：**

1. [Zookeeper架构、ZAB协议、选举](https://www.cnblogs.com/fanguangdexiaoyuer/p/10311228.html)

2. [ZAB协议](https://www.cnblogs.com/liuyi6/p/10726338.html)

3. [Zookeeper 原理 之ZAB，选举](https://blog.csdn.net/weixin_40792878/article/details/87475881)

<br />
<br />
<br />


