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

也许你会疑惑只确定一个值能起什么作用，在Paxos协议里确定并只确定一个值是确定多值的基础，如何确定多值将在第二部分Multi Paxos中介绍，这部分我们聚焦在“Paxos如何确定并只确定一个值”这一问题上。


和2PC类似，Paxos先把节点分成两类，发起提议（proposal)的一方为proposer，参与决议的一方为acceptor。假如只有一个proposer发起提议，并且节点不宕机、消息不丢包，那么acceptor做到以下这点就可以确定一个值：

{% highlight string %}
P1
. 一个acceptor接受他收到的第一项提议
{% endhighlight %}

当然上面要求的前提条件有些严苛，节点不能宕机、消息不能丢包，还只能由一个proposer发起提议。我们尝试放宽条件，假设多个proposer可以同时发起提议，怎样才能做到确定并只确定一个值呢？

首先proposer和acceptor需要满足以下两个条件：

* 1. properser发起的每项提议分别用一个ID标识，提议的组成因此变为（ID，value）
* 2. acceptor可以接受(accept)不止一项提议，当多数(quorum)acceptor接受一项提议时该提议被确定（chosen）

*(注：注意以上“接受”和“确定”的区别)*

我们约定后面发起的提议的ID比前面提议的ID大，并假设可以有多项提议被接受，为做到确定并只确定一个值acceptor要做到以下这点：
{% highlight string %}
P2
. 如果一项值为v的提议被确定，那么后续只确定值为v的提议

 ##（注： 乍看这个条件不太好理解，谨记目标是确定并只确定一个值）
{% endhighlight %}

<br />

由于一项提议被确定(chosen)前必须先被多数派acceptor接受(accepted)，为实现P2，实际上acceptor需要做到：
{% highlight string %}
P2a
. 如果一项值为v的提议被确定，那么acceptor后续只接受值为v的提议
{% endhighlight %}
满足P2a则P2成立(P2a=>P2)





	




<br />
<br />
<br />


