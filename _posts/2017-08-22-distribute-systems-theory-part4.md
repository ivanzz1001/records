---
layout: post
title: (转）分布式系统理论基础 -- CAP
tags:
- 分布式系统
categories: distribute-systems
description: 分布式系统理论
---

本文转自：[博客园-bangerlee](http://transcoder.baiducontent.com/tc?srd=1&dict=32&h5ad=1&bdenc=1&lid=12288400752482103203&nsrc=IlPT2AEptyoA_yixCFOxXnANedT62v3IEQGG_ytK1DK6mlrte4viZQRAYD06N8qIH5DwgTCccQoDlnGh_mEl8BZZhOgtfq)

<!-- more -->

## 1. 引言

[《分布式系统理论基础 - 一致性、2PC和3PC》](https://ivanzz1001.github.io/records/post/distribute-systems/2017/08/22/distribute-systems-theory-part1) 一文介绍了一致性、达成一致性需要面临的各种问题以及2PC、3PC模型，Paxos协议在节点宕机恢复、消息无序或丢失、网络分化的场景下能保证决议的一致性，是被讨论最广泛的一致性协议。

Paxos协议同时又以其“艰深晦涩”著称，下面结合 Paxos Made Simple 、The Part-Time Parliament 两篇论文，尝试通过Paxos推演、学习和了解Paxos协议。


## 2. Basic Paxos

何为一致性问题？ 简单而言，一致性问题是在节点宕机、消息无序等场景可能出现的情况下，相互独立的节点之间如何达成决议的问题，作为解决一致性问题的协议，Paxos的核心是节点间如何确定并只确定一个值（value）。





<br />
<br />
<br />


