---
layout: post
title: Paxos算法深入分析
tags:
- paxos
categories: paxos
description: paxos算法深入分析
---

在分布式系统设计领域，Paxos可谓是最重要的一致性算法。Google的大牛们称：

<!-- more -->
{% highlight string %}
All working protocols for asynchronous consensus we have so far encountered have Paxos at their core.
{% endhighlight %}
可见此算法的地位。

Paxos算法实现的是分布式系统多个节点之上数据的一致性，这个算法有如下特性：

* 基于消息传递，允许消息传输的丢失、重复、乱序，但是不允许消息被篡改
* 在节点数少于半数失效的情况下仍然能正常的工作，节点失效可以在任何时候发生而不影响算法正常执行

<br />

下面是Basic Paxos算法，注意这个算法只具有在多个冲突请求中选出一个的功能，并不具备序列化多个请求依次执行的功能。

Paxos算法包含Proposer、Acceptor、Learner等角色。实现的时候采用一组固定数目Server,每个Server同时担任上述三个角色，多个Client将自己的请求值```Value_i```随机发给一个Server处理，然后这一组Server经过协商后得出统一的一个值Chosen_Value，这个值必须被每个Server学习到。同时回复给所有发起请求的Client。

具体算法流程如下，为避免歧义，关键字眼Proposer,Proposal,Accept,Value,Choose等保留英文原文。


**(1) 阶段1a --- Prepare(预定Proposal序号）**

每个Proposer拿到某个Client的请求Value_i后，在此阶段还不能发起Proposal，只能发送一个Proposal序号```N```，将序号发送给所有Acceptor（即所有Server包括自己）。整个系统中所有Proposal的序号不能重复，而且每个Proposer自己用到的序号必须是递增的。通常的做法是：假设K台Server协同运行Paxos算法，那么Server_i(i=0...K-1)用的Proposal序号初始值为i，以后每次要产生新序号时递增K，这样保证了所有Server的Proposal序号不重复。

<br />

**(2) 阶段1b --- Respond with Promise**
 
每个Acceptor收到Proposal序号后，先检查之前是否Respond序号更高的Proposal。若没有，那么就给出Response,这个Response带有自己已经Accept的序号最高的Proposal(若还没Accept任何Proposal，则回复null)，同时，Promise自己不再Accept低于接收序号的Proposal。否则，拒绝Respond。

<br />

**(3) 阶段2a --- 发起Proposal，请求Accept**

Proposal如果得到了来自超过半数的Acceptor的Response，那么就有资格向Acceptor发起Proposal<N,value>。其中，N是阶段1a中发送的序号，value是收到Response中序号最大的Proposal的Value，若收到的Response全部是null，那么Value自定义，可以直接选一个Client请求的Value_i。


**4) 阶段2b --- Accept Proposal**

检查收到的Proposal的序号是否违反```阶段1b```的Promise，若不违反，则Accept接收到的Proposal。


<br />

所有Accepter所accept的Proposal要不断通知所有Learner，或者Learner主动去查询。一旦Learner确认Proposal已经被超过半数的Accepter所接受，那么表示这个Proposal的Value被Chosen，Learner就可以学习这个Proposal的Value，同时在自己Server上就可以不再受理Proposor的请求。




<br />
<br />
参看：

1. [Paxos算法深入分析](http://blog.csdn.net/anderscloud/article/details/7175209)

2. [Paxos算法](https://zh.wikipedia.org/zh-cn/Paxos%E7%AE%97%E6%B3%95)



<br />
<br />
<br />


