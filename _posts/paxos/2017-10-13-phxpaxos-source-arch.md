---
layout: post
title: phxpaxos源代码整体架构
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->

## 1. phxpaxos整体架构
通过对phxecho这个实例的分析，phxpaxos的整体抽象层次如下所示：

![paxos-arch-abstract](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_arch_abstract.jpg)

一个phxpaxos节点可以有多个Group，Group与Group之间完全隔离，不会进行任何通信。每一个Group可以挂载多个StateMachine。

phxpaxos的整体类图结构如下所示：

![paxos-class-diagram](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_class_diagram.jpg)


<br />
<br />
**参看：**

1. [PhxPaxos源码分析之关于PhxPaxos](https://www.jianshu.com/p/9f1a874a39e5)

2. [PhxPaxos源码解析（1）之概述篇](https://blog.csdn.net/weixin_41713182/article/details/88147487)



<br />
<br />
<br />


