---
layout: post
title: phxpaxos理论介绍(3)： Master选举
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->

## 1. Master
开门见山，我们先明确一下Master的定义。Master是一个角色，这个角色的特点是，在我们选定的一些节点集合内，任一时刻，仅有一个节点成为Master或者没有任何节点成为Master。这是一个非常严格的单点定义。

Master的应用非常广泛。比如在分布式存储里面，我们希望读取一个最新的值，那么常见的做法是我们先选举出一个Master，读写都经由Master来完成，那么在Master上读取到的就肯定是最新的。另外还比如一些仲裁模块，往往也希望由Master来协助。

## 2. Master选举与Paxos的关系
如何选举Master？由于Master具有严格的单点定义，那么必须有一个强一致性的算法才能完成选举，当然我们这里采用了Paxos。但Master选举算法自身也是一个通用性的算法，它可以与任何强一致性算法搭配来完成，而无需要求一定是Paxos。所以这里我们希望设计一个与Paxos完全解耦的工程实现，也就是Master选举只用到Paxos工程实现的API，而无需侵入Paxos算法内部。

## 3. Paxos的工程应用
这个涉及到Paxos工程上API设计以及状态机，这里先不展开讲，来看一张图相信大家就懂了，图片来自论文"Paxos Made Live"。

![paxos-made-live](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_made_live.jpg)

Paxos的应用简明来讲就是由算法确定一个操作系列，通过编写这些操作系列的callback(也就是状态机的状态转移函数），使得节点进行相同顺序的callback，从而保证各个节点的状态一致。

## 4. Master选举租约算法
![paxos-be-master](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_be_master.jpg)

BeMaster是一个操作，这个操作很简单，就是提议```自己```成为master，图片里面```A节点```希望自己成为Master。任何节点都可以发起这个操作尝试将自己提升为Master，除了已经得知别人已被选为Master。当得知别人选为Master后，必须等待```timeout```长度的时间，才能发起BeMaster操作。而如果是获知自己成为Master，那么从BeMaster开始的timeout时间内可认为自己是Master，如图示，T2-T3的时间窗内，视作Master的```任期```。

1) **如何将上面所述的租约算法与Paxos结合起来？**

* BeMaster可以认为是一个Submit操作，其Value携带的就是自己的节点信息

* callback做两件事情，第一：发现Value的节点非自己，则等待timeout时间再发起BeMaster。第二：发现Value的节点是自己，那么将自己提升为Master，并在T(BeMaster)+timeout后过期。

2） **算法正确性如何保证？**

* 一致性由Paxos保证，也就是只要Value被Paxos选举出来，那么其包含的肯定是同一个节点的信息，不会出现选举冲突；

* Master的单点性通过租约算法保证。由于恒定T(BeMaster) < T(Know other as master)，那么Master的过期时间肯定要比非Master节点认为Master过期的时间早，从而保证Master任期内，肯定不会出现其他节点尝试来抢占Master
<pre>
这里给大家提一个问题，图示里面，为何Master任期的起始时间是从BeMaster算起，而不能是从BeMaster success算起？ 相信如果理解了
Paxos算法的读者，应该可以很轻松回答这个问题。
</pre>

3) **Master续任**

只需要在Master任期内成功完成一次BeMaster操作，即可延长Master任期，在正常情况下这样不断迭代下去，一般会使得Master非常的稳定。

![paxos-master-a](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_master_a.jpg)

上图可以看到在多次的BeMaster选举里面，我们需要给每一个任期赋予一个version，这是为什么？下面通过一个例子来解释这个问题。

![paxos-master-b](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_master_b.jpg)

这个图示情况是NodeA不断在续任，但NodeC可能与NodeA无法通信或者其他原因，在获知NodeA第二次续任成功后就再也收不到任何消息了，于是当NodeC认为A的Master任期过期后，即可尝试发起BeMaster操作。这就违背了算法的保证了，出现了NodeA在任期内，但NodeC发起BeMaster操作的情况。

这里问题的本质是，NodeC还未获得最新的Master情况，所以发起了一次错误的BeMaster。version的加入是参考了乐观锁来解决这个问题。发起BeMaster的时候携带上一次version，如果这个version已经不是最新，那么这一次BeMaster自然会失效，从而解决问题。理解乐观锁的读者应该可以很快脑补出version的作用，这里就不详细展开了。

<br />
<br />
**参看：**

1. [Paxos从理论到实践](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)

2. [phxpaxos](https://github.com/Tencent/phxpaxos/blob/master/README.zh_CN.md)

3. [Paxos算法](https://zh.wikipedia.org/zh-cn/Paxos%E7%AE%97%E6%B3%95)

4. [腾讯开源的 Paxos库 PhxPaxos 代码解读-](https://www.cnblogs.com/lijingshanxi/p/10250878.html)

<br />
<br />
<br />


