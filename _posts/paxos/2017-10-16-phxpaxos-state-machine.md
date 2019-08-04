---
layout: post
title: phxpaxos源码分析： 状态机
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->

## 1. 基本概念

* **Paxos Log**

通过Paxos算法选中的一组有序的提案值。例如，PhxSQL中使用PhxPaxos确定的有序值为binlog。

* **状态机**

业务自定义的如何使用Paxos log的数据消费逻辑。状态机的一个特点是：只要初始状态一致，输入一致，那么引出的最终状态也是一致的。在PhxSQL中，这个状态机就是binlog的replay机制，即在其他节点上执行和主节点一致的binlog操作，保证各个节点数据的一致性。

## 2. 代码设计
状态机相关类的类图如下：

![paxos-state-machine](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_state_machine.jpg)

下面我们简要介绍一下各class的功能：

* **SMFac**: 状态机管理类



<br />
<br />
**参看：**

1. [PhxPaxos源码分析之关于PhxPaxos](https://www.jianshu.com/p/9f1a874a39e5)

2. [PhxPaxos源码解析（1）之概述篇](https://blog.csdn.net/weixin_41713182/article/details/88147487)

3. [PhxPaxos源码分析之状态机](https://www.jianshu.com/p/89377cc9b405)

4. [一致性协议](https://www.jianshu.com/p/0b475b430abe?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)

<br />
<br />
<br />


