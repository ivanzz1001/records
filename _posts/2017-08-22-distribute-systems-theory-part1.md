---
layout: post
title: (转）分布式系统理论基础 -- 一致性、2PC和3PC
tags:
- 分布式系统
categories: distribute-systems
description: 分布式系统理论
---

本文转自：[博客园-bangerlee](http://transcoder.tradaquan.com/tc?srd=1&dict=32&h5ad=1&bdenc=1&lid=12288400752482103203&nsrc=IlPT2AEptyoA_yixCFOxXnANedT62v3IEQGG_ytK1DK6mlrte4viZQRAYD06N8qIH5DwgTCccQoDlnGd_W9i9RVZhOgtfq)

<!-- more -->

## 1. 引言
狭义的分布式系统指由网络连接的计算机系统，每个节点独立地承担计算或存储任务，节点间通过网络系统工作。广义的分布式系统是一个相对的概念。正如[Leslie Lamport](https://en.wikipedia.org/wiki/Leslie_Lamport)所说：
<pre>
What is a distributed systeme. Distribution is in the eye of the beholder.
 
To the user sitting at the keyboard, his IBM personal computer is a nondistributed system. 

To a flea crawling around on the circuit board, or to the engineer who designed it, it's very much a distributed system.
</pre>

一致性是分布式理论中的根本性问题，近半个世纪以来，科学家们围绕着一致性问题提出了很多理论模型，依据这些理论模型，业界也出现了很多工程实践投影。下面我们从一致性问题、特定条件下解决一致性问题的两种方法（2PC，3PC）入门，了解最基础的分布式系统理论。


## 2. 一致性(consensus)
何为一致性问题？简单而言，一致性问题就是相互独立的节点之间如何达成一项决议的问题。分布式系统中，进行数据库事务提交（commit transaction)、Leader选举、序列号生成等都会遇到一致性问题。这个问题在我们的日常生活中也很常见，比如牌友怎么商定在几点在哪打几圈麻将：
![《赌圣》，1990](https://ivanzz1001.github.io/records/assets/img/ceph/distribute/timg.jpg)

假设一个具有N个节点的分布式系统，当其满足以下条件时，我们说这个系统满足一致性：

* 1. 全认同（agreement）：所有N个节点都认同一个结果
* 2. 值合法（validity)： 该结果必须由N个节点中的节点提出
* 3. 可结束（termination）： 决议过程在一定时间内结束，不会无休止的进行下去

可能有人会说，决定什么时候在哪戳麻将，4个人商量一下就OK，这不是很简单吗？

<br />
但就是这样看似简单的事情，分布式系统实现起来并不轻松，因为它面临着这些问题：

* 消息传递异步无序（asychronous): 现实网络不是一个可靠的信道，存在消息延迟、丢失，节点间消息传递做不到同步有序（synchronous)
* 节点宕机（fail-stop): 节点持续宕机，不会恢复
* 节点宕机恢复（fail-recover): 节点宕机一段时间后恢复，在分布式系统中最常见
* 网络分化（network partion): 网络链路出现问题，将N个节点隔离成多个部分
* 拜占庭将军问题（byzantine failure): 节点或宕机或逻辑失败，甚至不按套路出牌抛出干扰决议的信息

<br />

假设现实场景中也存在这样的问题，我们看看结果会怎样：
{% highlight string %}
-----------------------------------------------------------------
我： 老王，今晚7点老地方，搓够48圈不见不散！
....

(第二天凌晨3点）隔壁老王： 没问题！
//
 消息延迟 

我： ....

-----------------------------------------------------------------
我： 小张，今晚7点老地方，搓够48圈不见不散！
小张： No ...

(两小时后...）
小张： No problem!
// 
  宕机节点恢复

我： ...

-----------------------------------------------------------------
我： 李老头，今晚7点老地方，搓够48圈不见不散！
老李： 必须的，大保健走起！
//
  拜占庭将军

（这是要打麻将呢？ 还是要大保健？ 还是一边打麻将一边大保健...)

------------------------------------------------------------------
{% endhighlight %}

还能不能愉快的玩耍....![哭](https://ivanzz1001.github.io/records/assets/img/ceph/distribute/timg.png)

<br />

我们把以上所列的问题称为系统模型（system model)，讨论分布式系统理论和工程实践的时候，必须划定模型。例如，有如下两种模型：
* 1： 异步环境（asynchronous)下，节点宕机（fail-stop)
* 2： 异步环境（asynchronous)下，节点宕机恢复（fail-recover)、网络分化（network partition)

2比1多了节点恢复、网络分化的考量，因而对这两种模型的研究和工程解决方案必定是不同的，在还没有明晰索要解决的问题前谈解决方案都是一本正经的耍流氓。

<br/>

一致性还具备两个属性： 一个是强一致性(safety)，它要求所有节点状态一致、共进退；一个是可用(liveness)，它要求分布式系统24*7无间断对外服务。 FLP定理（FLP impossibility)已经证明在一个收窄的模型中（异步环境并只存在节点宕机），不能同时满足safety和liveness。

<br />

FLP定理是分布式系统中的基础理论，正如物理学中的能量守恒定律彻底否定了永动机的存在，FLP定理否定了同时满足safety和liveness的一致性协议的存在。
![《怦然心动 (Flipped)》，2010](https://ivanzz1001.github.io/records/assets/img/ceph/distribute/timg-1.jpg)




<br />
<br />
<br />


