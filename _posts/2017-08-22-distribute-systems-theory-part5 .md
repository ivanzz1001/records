---
layout: post
title: (转）分布式系统理论基础 - 选举、多数派和租约
tags:
- 分布式系统
categories: distribute-systems
description: 分布式系统理论
---

本文转自：[博客园-bangerlee](http://transcoder.baiducontent.com/tc?srd=1&dict=32&h5ad=1&bdenc=1&lid=12288400752482103203&nsrc=IlPT2AEptyoA_yixCFOxXnANedT62v3IEQGG_ytK1DK6mlrte4viZQRAYD06N8qIH5DwgTCccQoDlnGg_W1e8RVZhOgtfq)

<!-- more -->

## 1. 引言

选举(election)是分布式系统实践中常见的问题，通过打破节点间的对等关系，选得的leader(或叫Master、Coordinator)有助于实现事务原子性、提升决议效率。 多数派(quorum)的思路帮助我们在网络分化的情况下达成决议一致性， 在leader选举的场景下帮助我们选出唯一leader。租约(lease)在一定期限内给予节点特定权利，也可以用于实现leader选举。

下面我们就来学习分布式系统理论中的选举、多数派和租约。

## 2. 选举(election)
一致性问题(consistency)是独立的节点间如何达成决议的问题，选出大家都认可的leader本质上也是一致性问题，因而如何应对宕机恢复、网络分化等在Leader选举中也需要考量。


Bully算法是最常见的选举算法，其要求每个节点对应一个序号，序号最高的节点为leader。leader宕机后次高序号的节点被重选为leader，过程如下：
![election](https://ivanzz1001.github.io/records/assets/img/distribute/paxos_election.png)







<br />
<br />
<br />


