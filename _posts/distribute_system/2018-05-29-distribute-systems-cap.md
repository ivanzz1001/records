---
layout: post
title: 分布式之CAP原理
tags:
- 分布式系统
categories: distribute-systems
description: 分布式之CAP原理
---

本文重点介绍一下分布式系统设计中的CAP原理。

<!-- more -->


## 1. CAP是什么？

CAP理论，被戏称为```[帽子理论]```。CAP理论由Eric Brewer在ACM研讨会上提出，而后CAP被奉为分布式领域的重要理论[1]。


分布式系统的CAP理论，首先把分布式系统中的三个特性进行了如下归纳：

* 一致性(C): 在分布式系统中的所有数据备份，在同一时刻是否有同样的值。（等同于所有节点访问同一份最新的数据副本）

* 可用性(A): 在集群中一部分节点故障后，集群整体是否还能响应客户端的读写请求。（对数据更新具备高可用）

* 分区容忍性(P): 以实际效果而言，分区相当于对通信的时限要求。系统如果不能在时限内达成数据一致性，就意味着发生了分区的情况，必须就当前操作在C和A之间做出选择。（分区状态可理解为部分机器不连通了，比如机器挂了，繁忙失去响应，单机房故障等）
  
Partition字面意思是网络分区，即因网络因素将系统分割为多个单独的部分，有人可能会说，网络分区的情况发生概率非常小啊，是不是不用考虑P，保证CA就好。要理解P，我们看回CAP证明中P的定义：
<pre>
In order to model partition tolerance, the network will be allowed to losearbitrarily(任意丢失) many messages sent
 from one node to another.
</pre>
 
网络分区的情况符合该定义；网络丢包的情况也符合以上定义；另外节点宕机，其他节点发往宕机节点的包也将丢失，这种情况同样符合定义。现实情况下我们面对的是一个不可靠的网络、有一定概率宕机的设备，这两个因素都会导致Partition，因而分布式系统实现中P是一个必须项，而不是可选项。

```高可用、数据一致性```是很多系统设计的目标，但是分区又是不可避免的事情。我们来看一看分别拥有CA、CP和AP的情况：

1) **CA without P**

如果不要求P（即不允许分区），则C（强一致性）和A（可用性）是可以保证的。但其实分区不是你想不想的问题，而是始终会存在，CA系统基本上是单机系统，比如单机数据库。2PC是实现强一致性的具体手段。

![distri-cap-1](https://ivanzz1001.github.io/records/assets/img/distribute/distri-cap-1.jpg)



<br />
<br />

**[参看]:**

1. [分布式系统之CAP原理](https://www.cnblogs.com/heapStark/p/8351852.html)

2. [PODC-keynote.pdf](https://people.eecs.berkeley.edu/~brewer/cs262b-2004/PODC-keynote.pdf)

<br />
<br />
<br />


