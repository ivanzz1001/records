---
layout: post
title: (转）分布式系统理论基础 -- CAP
tags:
- 分布式系统
categories: distribute-systems
description: 分布式系统理论
---

本文转自：[博客园-bangerlee](http://transcoder.baiducontent.com/tc?srd=1&dict=32&h5ad=1&bdenc=1&lid=12288400752482103203&nsrc=IlPT2AEptyoA_yixCFOxXnANedT62v3IEQGG_ytK1DK6mlrte4viZQRAYD06N8qIH5DwgTCccQoDlnGc0W9e9RpZhOgtfq)

<!-- more -->

## 1. 引言

CAP是分布式系统，特别是分布式存储领域被讨论最多的理论。“什么是CAP定理？”在Quora分布式系统分类下排名FAQ的No.1。 CAP在程序员中也有叫广的普及，它不仅仅是“C、A、P不能同时满足，最多只能3选2”，以下尝试综合各方观点，从发展历史、工程实践等角度讲述CAP理论。希望大家透过本文对CAP理论有更多地了解和认识。


## 2. CAP定理
CAP由Eric Brewer 在2000年PODC会议上提出 ，是Eric Brewer在Inktomi期间研发搜索引擎、分布式Web缓存时得出的关于数据一致性(consistency)、服务可用性（availability)、分区容错性（partion-tolerance)的猜想：
<pre>
It is impossible for a web service to provide the three following guarantees : Consistency, Availability and Partition-tolerance.
</pre>

该猜想在提出两年后被证明成立，成为我们熟知的CAP定理：

* 数据一致性（consistency)： 如果系统对一个写操作返回成功，那么之后的读请求都必须读到这个新数据；如果返回失败，那么所有的读操作都不能读到这个数据，对调用者而言数据具有强一致性（strong consistency)(又叫原子性atomic、线性一致性linearizable consistency)
* 服务可用性（availability)： 所有读写请求在一定时间内得到响应，可终止、不会一直等待
* 分区容错性（partion-tolerance)： 在网络分区的情况下，被分割的节点仍能正常对外服务


在某时刻如果满足AP，分割的节点同时对外服务但不能相互通信，将导致状态不一致，即不能满足C；如果满足CP，网络分区的情况下为达成C，请求只能一直等待，即不满足A；如果要满足CA，在一定时间内要达到节点状态一致，要求不能出现网络分区，则不能满足P。

C、A、P三者最多只能满足其中两个，和FLP定理一样，CAP定理也指示了一个不可达的结果(impossibility result)。


## 3. CAP的工程启示

CAP理论提出7、8年后，NoSql圈将CAP理论当作对抗传统关系型数据库的依据、阐明自己放宽对数据一致性(consistency)要求的正确性，随后引起了大范围关于CAP理论的讨论。

CAP理论看似给我们出了一道3选2的选择题，但在工程实践中存在很多现实限制条件，需要我们做更多地考量与权衡，避免进入CAP认识误区。







<br />
<br />
<br />


