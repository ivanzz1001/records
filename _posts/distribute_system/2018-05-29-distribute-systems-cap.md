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

如果不要求P（分区容忍性），即认为分区不会发生，则C（强一致性）和A（可用性）是可以保证的。但其实分区不是你想不想的问题，而是始终会存在，CA系统基本上是单机系统，比如单机数据库。2PC是实现强一致性的具体手段。

![distri-cap-1](https://ivanzz1001.github.io/records/assets/img/distribute/distri-cap-1.jpg)

图片来自于： [PODC-keynote.pdf](http://www.cs.berkeley.edu/~brewer/cs262b-2004/PODC-keynote.pdf)


2) **CP without A**

如果不要求A（可用），相当于每个请求都需要在Server之间强一致，而P（分区）会导致同步时间无限延长，如此CP也是可以保证的。很多传统的数据库分布式事务都属于这种模式。

![distri-cap-2](https://ivanzz1001.github.io/records/assets/img/distribute/distri-cap-2.jpg)

图片来自于： [PODC-keynote.pdf](http://www.cs.berkeley.edu/~brewer/cs262b-2004/PODC-keynote.pdf)

3） **AP without C**

要高可用并允许分区，则需放弃一致性。一旦分区发生，节点之间可能会失去联系，为了高可用，每个节点只能用本地数据提供服务，而这样会导致全局数据的不一致性。现在众多的NoSQL都属于此类。

![distri-cap-3](https://ivanzz1001.github.io/records/assets/img/distribute/distri-cap-3.jpg)

图片来自于： [PODC-keynote.pdf](http://www.cs.berkeley.edu/~brewer/cs262b-2004/PODC-keynote.pdf)


## 2. CAP理论的证明
该理论由Brewer提出，2年后就是2002年，Lynch与其他人证明了Brewer猜想，从而把CAP上升为一个定理。但是，它只是证明了CAP三者不可能同时满足，并没有证明任意二者都可满足的问题。所以，该证明被认为是一个收窄的结果。


Lynch的证明相对比较简单： 采用反证法，如果三者可同时满足，则因为允许P的存在，一定存在Server之间的丢包，如此则不能保证C，证明简洁而严谨。

在该证明中，对CAP的定义进行了更明确的声明：

**C:** 一致性被称为原子对象，任何的读写都应该看起来是```原子```的，或串行的。写后面的读一定能读到前面写的内容，所有的读写请求都好像被全局排序一样。

**A:** 对任何非失败节点都应该在有限时间内给出请求的回应。（请求的可终止性）

**P:** 允许节点之间丢失任意多的消息，当网络分区发生时，节点之间的消息可能会完全丢失。

对于CAP进一步的案例解释：

2010年的这篇文章*brewers-cap-theorem-on-distributed-systems/*，用了3个例子来阐述CAP，分别是：
<pre>
example 1: 单点的mysql;

example 2: 两个mysql， 但不同的mysql存储不同的数据子集，相当于sharding；

example 3: 两个mysql，对A的一个insert操作，需要在B上执行成功才认为操作完成（类似于复制集）
</pre>

作者认为在```example 1```和```example 2```上都能保证强一致性，但不能保证可用性；在```example 3```这个例子中，由于分区(Partition)的存在，就需要在一致性和可用性之间权衡。对于复制而言，在很多场景下不追求强一致性。比如用户支付之后，交易记录落地了，但可能消费记录的消息同步存在延迟，比如消息阻塞了。在金融业务中，采取类似两地三中心架构，往往可能采取本地数据和异地机房数据同时写成功再返回的方式。这样付出了性能的损耗，响应时间变长。但发生机房故障后，能确保数据时完全可读写的，保障了一致性。

## 3. CAP理论澄清
【CAP理论十二年回顾： “规则”变了】 一文首发于Computer杂志，后由```InfoQ```和```IEEE```联合呈现，非常精彩[3]，文章表达了几个观点：

1) **“三选二”是一个伪命题**




<br />
<br />

**[参看]:**

1. [分布式系统之CAP原理](https://www.cnblogs.com/heapStark/p/8351852.html)

2. [PODC-keynote.pdf](https://people.eecs.berkeley.edu/~brewer/cs262b-2004/PODC-keynote.pdf)

3. [CAP理论十二年回顾： “规则”变了](http://www.infoq.com/cn/articles/cap-twelve-years-later-how-the-rules-have-changed/)

<br />
<br />
<br />


