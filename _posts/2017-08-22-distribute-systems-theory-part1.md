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






<br />
<br />
<br />


