---
layout: post
title: 以两军问题为背景来演绎BasicPaxos
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->

## 1. 背景
在计算机通信理论中，有一个著名的两军问题，讲述通信的双方通过ACK来达成共识，永远会有一个在途的ACK需要进行确认，因此无法达成共识。

两军问题和BasicPaxos非常相似：

>1) 通信的各方需要达成共识
>
>2) 通信的各方仅需达成一个共识
>
>3) 假设信道不稳定，有丢包、延迟或者重放，但消息不会被篡改

BasicPaxos最早以希腊会议为背景来讲解，但普通人不理解希腊会议的运作模式，因此看BasicPaxos的论文会比较难理解。两军问题的背景大家更熟悉，因此尝试用这个背景来演绎一下BasicPaxos。

为了配合BasicPaxos的多数派概念，把两军改为3军；同时假设了将军、参谋和通信兵的角色。

###### 假设的3军问题

paxos_three.jpeg



<br />
<br />
**参看：**

1. [Paxos从理论到实践](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw?)

<br />
<br />
<br />


