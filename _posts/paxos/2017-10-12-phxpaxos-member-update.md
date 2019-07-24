---
layout: post
title: phxpaxos理论介绍(4)： 动态成员变更
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->

## 1. 多数派的本质
在讲解成员变更之前，我们先回顾一下前文介绍的Paxos理论第一篇文章[ Paxos理论介绍(1): 朴素Paxos算法理论推导与证明](https://ivanzz1001.github.io/records/post/paxos/2017/10/09/phxpaxos-theory)，（仔细回顾数学定义和投票约束章节）文中提到```Bqrm```为一轮成功投票所需要的投票者集合，而Paxos算法理论第二条约束要求任意两个```Bqrm```的交集不为空，于是乎我们可以理解为```Bqrm```就是一个多数派的意思，因为在一个固定的投票者集合里面，取多数派作为```Bqrm```，肯定是满足条件的。

而所有的理论介绍，都是基于投票者集合是固定的。一旦投票者集合出现变化，```Bqrm```的定义将不再是多数派，```Bqrm```的取值将变得异常困难，而无法定义Bqrm，Paxos算法的约束就无法达成一致性。也就是说，固定的成员时Paxos算法的根基。

## 2. 人肉配置进行成员变更？
我们再进行第二篇文章[Paxos理论介绍(2): Multi-Paxos与Leader](https://ivanzz1001.github.io/records/post/paxos/2017/10/10/phxpaxos-multi)的回顾，通过文章我们知道Paxos是以独立的实例的方式推进，从而产生一个一致性的有序的系列，而每个实例都是单独运作的Paxos算法。再根据上文，我们得出一个要求，在相同的实例上，我们要求各个成员所认为的成员集合必须是一致的，也就是在一次完整的Paxos算法里面，成员其实还是固定的。

每个成员如何得知这个成员集合是什么？通常我们是通过配置文件。在通过配置的变更能否满足以上的要求呢？我们知道Multi-Paxos在推进的过程中是允许少数派落后的，而在同一个实例里面，获知Value被chosen也是有先后的，那么配置的变更可能出现在以上任何的先后夹缝内，下图演示一个更换节点C为D的样例：

![paxos-member-a](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_member_a.jpg)

>注： 绿色代表已经获知chosen value的实例

可以观察到4这个实例，已经出现了成员混乱，(A,C)，(B,D)都可以被认为是Bqrm，但明显这两个Bqrm没有交集，已经违反Paxos协议。

事实上我们追求的是找到一个切入时机，使得Paxos的运作程序都在这相同的时刻完成配置的原子切换，但明显在分布式环境里面能做原子切换的只有一致性算法，所以配置更新不靠谱。
<pre>
题外话，如果真的要使用人肉配置更新，在工程上是有一些办法，通过一些工具加人肉的细微观察来无限逼近这个正确性，但终究只能逼近。
在理论层面我们会放大任何现实中可能不会出现的细微错误，比如时间的不同步，网络包在交换机无限停留，操作系统调度导致的代码段卡壳
等等，这些都会导致这些人肉方法不能上升到理论层面。况且，我们接下来要介绍的动态成员变更算法也是非常的简单。所以这些细致的问题
就不展开来聊了。
</pre>

## 3. Paxos动态成员变更算法
这个算法在```Paxos Made Simple```的最后一段被一句话带过，可能作者认为这个是水到渠成的事情，根本不值一提。

Multi-Paxos决议出的有序系列，一般被用来作为状态机的状态转移输入，一致的状态转移得出一致的状态，这是Paxos的基本应用。那么非常水到渠成的事情就是，成员（投票者集合）本身也是一个状态，我们通过Paxos来决议出成员变更的操作系列，那么各台机器就能获得一致的成员状态。如下图：

![paxos-member-b](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_member_b.jpg)

在4这个实例，我们通过Paxos算法来决议一个成员变更操作，所有的节点在实例4之后都能获取到成员从A,B,C变成了A,B,D，在理论上达到了原子变更的要求。

### 3.1 延缓变更生效









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


