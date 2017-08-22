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
《赌圣》，1990





<br />
<br />
<br />


