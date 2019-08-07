---
layout: post
title: 微信自研生产级paxos类库PhxPaxos实现原理介绍
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->


## 1. 前言
>本文是一篇无需任何分布式以及paxos算法基础的可以看懂的。

标题主要有三个关键字： 生产级、paxos、 实现，覆盖了本文的重点。什么叫生产级，直译就是能用于生产线的，而非实验产品。生产级别拥有超高的稳定性，不错的性能，能真正服务于用户的。paxos就不说了，而实现是本文最大的重点，本文将避开paxos算法理论与证明，直入实现细节，告诉大家一个生产级别的paxos库背后的样子。为何要写这篇文章？ paxos算法理论与证明不是更重要么？我几年前曾经也读过Paxos论文，虽然大致理解了算法的过程，但是在脑海中却无一个场景去构建这个算法，而后也就慢慢印象淡化以至于最近重读Paxos论文的时候，感觉像是第一次读论文的样子。




<br />
<br />
**参看：**

1. [微信自研生产级paxos类库PhxPaxos实现原理介绍](https://mp.weixin.qq.com/s?__biz=MzI4NDMyNTU2Mw==&mid=2247483695&idx=1&sn=91ea422913fc62579e020e941d1d059e&scene=21#wechat_redirect)

<br />
<br />
<br />


