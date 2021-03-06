---
layout: post
title: ECMP网络的介绍(转)
tags:
- network
categories: network
description: ECMP网络的介绍
---

目前数据中心网络广泛应用的Fabric架构中会应用大量的ECMP(Equal-Cost MultiPath Routing, 简称ECMP)，其优点主要体现在可以提高网络冗余性和可靠性，同时也提高了网络资源利用率；大量的ECMP链路在特定场景下运行过程中会引发其他问题。例如，当某条ECMP链路断开后，ECMP组内所有链路流量都会被重新HASH，在有状态的服务器区域(如LVS)中将导致雪崩现象，又或者会出现多级ECMP的HASH极化导致链路拥塞等。

<!-- more -->


## 1. ECMP(Equal-Cost MultiPath routing)
ECMP是一个逐跳的基于流的负载均衡策略，当路由器发现同一目的地址出现多个最优路径时，会更新路由表，为此目的地址添加多条规则，对应于多个下一跳。可同时利用这些路径转发数据，增加带宽。ECMP算法被多种路由协议支持，例如： OSPF、ISIS、EIGRP、BGP等。在数据中心架构VL2中也提到使用ECMP作为负载均衡算法。

对于未开启ECMP的网络来说，无法充分利用路径资源。例如下图1所示：

![ECMP-fig-1](https://ivanzz1001.github.io/records/assets/img/network/ECMP-fig-1.png)


假设从```S0```到Server的路径为*S0-S1-S2-S4*（即图中橘色路径），那么即便存在另一条等价路径（即图中蓝色路径），路由器仍然会每次选择第一条橘色路径转发数据。除非此条路径发生拥塞，才会重新选择路径。

当开启ECMP功能时，便可同时利用两条路径，进行基于流的负载均衡。例如```主机A```到Server的数据流选择橘色路径，```主机B```到Server的数据流选择蓝色路径。

ECMP的路径选择策略有多种方法：

* 哈希： 例如根据源IP地址的哈希为流选择路径

* 轮询： 各个流在多条路径之间轮询传输

* 基于路径权重： 根据路径的权重分配流，权重大的路径分配的流数量更多

## 2. ECMP面临的问题
然而，ECMP是一种较为简单的负载均衡策略，其在实际使用中面临的问题也不容忽视。

1） **可能增加链路的拥塞**

ECMP并没有拥塞感知的机制，只是将流分散到不同的路径上转发。对于已经产生拥塞的路径来说，很可能加剧路径的拥塞。而使用哈希的方法，产生哈希碰撞也会增加链路的拥塞可能。

2） **非对称网络使用效果不好**

例如下图2中：

![ECMP-fig-2](https://ivanzz1001.github.io/records/assets/img/network/ECMP-fig-2.png)

A和```h3```之间的通信，ECMP只是均匀的将流通过B、D两条路径分别转发，但实际上，在B处可以承担更多的流量。因为B后面还有两条路径可以到达h3。

3） **基于流的负载均衡效果不好**

ECMP对于流大小相差不多的情况效果更好，而对于流大小差异较大，例如大象流和老鼠流并存的情况下，效果不好。如上图2，```主机h1```到A的流量为15，```主机h2```到A的流量为5。那么无论为h1的流量选择哪条路径都会发生拥塞。但若将```h1```的流拆分成两部分传输，可以避免拥塞的情况。

以上，为使用ECMP算法进行负载均衡的分析，在数据中心这种突发性流量多，大象流与老鼠流并存的环境中，需要慎重考虑选择的负载均衡策略，ECMP简单易部署但也存在较多问题需要注意。









<br />
<br />

**参考**:

1. [深入看ECMP(详解其实现机制)](https://blog.csdn.net/aiaiai010101/article/details/84673687)

2. [数据中心网络等价多路径(ECMP)技术应用研究](http://datacenter.it168.com/a2018/0907/5025/000005025371.shtml)



<br />
<br />
<br />

