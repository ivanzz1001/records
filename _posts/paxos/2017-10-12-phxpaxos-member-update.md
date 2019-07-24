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
我们再进行第二篇文章[Paxos理论介绍(2): Multi-Paxos与Leader](https://ivanzz1001.github.io/records/post/paxos/2017/10/10/phxpaxos-multi)



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


