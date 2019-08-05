---
layout: post
title: phxpaxos源码分析： 归档机制
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->

## 1. 基本概念
上一章我们讲解了PhxPaxos的状态机，各种场景下可以定制不同的状态机，每个状态机独立消费一个paxos log以驱动业务状态变更。

状态机消费完paxos log之后，paxos log是否可以删除呢？答案是不行。有如下几个原因：

* 某些paxos节点可能由于网络等原因落后于其他节点，需要学习现有的paxos log；

* 业务消费完paxos log之后，可能由于重启等原因出现数据丢失，需要通过paxos log做重放(replay)

那什么时候可以删除paxos log呢？答案是不知道。因为某个节点可能永远处于离线状态，这时候必须保留从最初到现在所有的paxos log。但另一方面，如果数据不删除将无限增长，这是无法忍受的。

PhxPaxos因此引入了Checkpoint机制，关于该机制的详细描述请参见[《状态机Checkpoint详解》](https://github.com/Tencent/phxpaxos/wiki/%E7%8A%B6%E6%80%81%E6%9C%BACheckpoint%E8%AF%A6%E8%A7%A3)，这里简要说明如下：

1） 一个Checkpoint代表着一份某一时刻被固化下来的状态机数据，它通过```sm.h```下的*StateMachine::GetCheckpointInstanceID()*函数反馈它的精确时刻；

2） 每次启动replay时，只需要从*GetCheckpointInstanceID()*所指向的paxos log位置开始，而不是从0开始；

3） Node::SetHoldPaxosLogCount()控制需要保留多少在StateMachine::GetCheckpointInstanceID()之前的paxos log

4) 保留一定数量的paxos log的目的在于，如果其他节点数据不对齐，可以通过保留的这部分paxos log完成对齐，而不需要checkpoint数据介入；

5) 如果对齐数据已被删除，这时需要Checkpoint数据传输；

## 2. 代码设计
Checkpoint机制相关类图如下：

![paxos-checkpoint](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_checkpoint.jpg)





<br />
<br />
**参看：**

1. [PhxPaxos源码分析之关于PhxPaxos](https://www.jianshu.com/p/9f1a874a39e5)

2. [PhxPaxos源码解析（1）之概述篇](https://blog.csdn.net/weixin_41713182/article/details/88147487)

3. [PhxPaxos源码分析之状态机](https://www.jianshu.com/p/89377cc9b405)

4. [如何进行成员变更](https://github.com/Tencent/phxpaxos/wiki/%E5%A6%82%E4%BD%95%E8%BF%9B%E8%A1%8C%E6%88%90%E5%91%98%E5%8F%98%E6%9B%B4)

5. [一致性协议](https://www.jianshu.com/p/0b475b430abe?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)

<br />
<br />
<br />


