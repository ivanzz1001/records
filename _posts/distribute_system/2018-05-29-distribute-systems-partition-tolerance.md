---
layout: post
title: CAP理论中分区容错性的理解
tags:
- 分布式系统
categories: distribute-systems
description: 分布式之CAP原理
---


本文介绍一下CAP理论中分区容错性相关内容（转）。


<!-- more -->

## 1. CAP理论中分区容错性的理解
在CAP理论中, 对partition tolerance分区容错性的解释一般指的是分布式网络中部分网络不可用时, 系统依然正常对外提供服务, 而传统的系统设计中往往将这个放在最后一位. 这篇文章对这个此进行了分析和重新定义, 并说明了在不同规模分布式系统中的重要性.


The ```'CAP'``` theorem is a hot topic in the design of distributed data storage systems. However, it’s often widely misused. In this post I hope to highlight why the common ‘consistency, availability and partition tolerance: pick two’ formulation is inadequate for distributed systems. In fact, the lesson of the theorem is that the choice is almost always between sequential consistency and high availability.

It’s very common to invoke the 'CAP theorem' when designing, or talking about designing, distributed data storage systems. The theorem, as commonly stated, gives system designers a choice between three competing guarantees:

* Consistency – roughly meaning that all clients of a data store get responses to requests that 'make sense'. For example, if Client A writes 1 then 2 to location X, Client B cannot read 2 followed by 1.

* Availability – all operations on a data store eventually return successfully. We say that a data store is 'available' for, e.g. write operations.

* Partition tolerance – if the network stops delivering messages between two sets of servers, will the system continue to work correctly?


This is often summarised as a single sentence: “consistency, availability, partition tolerance. Pick two.”. Short, snappy and useful.

At least, that’s the conventional wisdom. Many modern distributed data stores, including those often caught under the ‘NoSQL’ net, pride themselves on offering availability and partition tolerance over strong consistency; the reasoning being that short periods of application misbehavior are less problematic than short periods of unavailability. Indeed, Dr. Michael Stonebraker posted an article on the ACM’s blog bemoaning the preponderance of systems that are choosing the ‘AP’ data point, and that consistency and availability are the two to choose. However for the vast majority of systems, I contend that the choice is almost always between consistency and availability, and unavoidably so.

Dr. Stonebraker’s central thesis is that, since partitions are rare, we might simply sacrifice ‘partition-tolerance’ in favour of sequential consistency and availability – a model that is well suited to traditional transactional data processing and the maintainance of the good old ACID invariants of most relational databases. I want to illustrate why this is a misinterpretation of the CAP theorem.


We first need to get exactly what is meant by ‘partition tolerance’ straight. Dr. Stonebraker asserts that a system is partition tolerant if processing can continue in both partitions in the case of a network failure.

“If there is a network failure that splits the processing nodes into two groups that cannot talk to each other, then the goal would be to allow processing to continue in both subgroups.”

This is actually a very strong partition tolerance requirement. Digging into the history of the CAP theorem reveals some divergence from this definition.

Seth Gilbert and Professor Nancy Lynch provided both a formalisation and a proof of the CAP theorem in their [2002 SIGACT paper](http://lpd.epfl.ch/sgilbert/pubs/BrewersConjecture-SigAct.pdf). We should defer to their definition of partition tolerance – if we are going to invoke CAP as a mathematical truth, we should formalize our foundations, otherwise we are building on very shaky ground. Gilbert and Lynch define partition tolerance as follows:

>"The network will be allowed to lose arbitrarily many messages sent from one node to another"
>
>网络将被允许丢失从一个节点发送到另一个节点的任意多条消息

Note that Gilbert and Lynch’s definition isn’t a property of a distributed application, but a property of the network in which it executes. This is often misunderstood: partition tolerance is not something we have a choice about designing into our systems. If you have a partition in your network, you lose either consistency (because you allow updates to both sides of the partition) or you lose availability (because you detect the error and shutdown the system until the error condition is resolved). Partition tolerance means simply developing a coping strategy by choosing which of the other system properties to drop. This is the real lesson of the CAP theorem – if you have a network that may drop messages, then you cannot have both availability and consistency, you must choose one. We should really be writing Possibility of Network Partitions => not(availability and consistency), but that’s not nearly so snappy.


Dr. Stonebraker’s definition of partition tolerance is actually a measure of availability – if a write may go to either partition, will it eventually be responded to? This is a very meaningful question for systems distributed across many geographic locations, but for the LAN case it is less common to have two partitions available for writes. However, it is encompassed by the requirement for availability that we already gave – if your system is available for writes at all times, then it is certainly available for writes during a network partition.

So what causes partitions? Two things, really. The first is obvious – a network failure, for example due to a faulty switch, can cause the network to partition. The other is less obvious, but fits with the definition from Gilbert and Lynch: machine failures, either hard or soft. In an asynchronous network, i.e. one where processing a message could take unbounded time, it is impossible to distinguish between machine failures and lost messages. Therefore a single machine failure partitions it from the rest of the network. A correlated failure of several machines partitions them all from the network. Not being able to receive a message is the same as the network not delivering it. In the face of sufficiently many machine failures, it is still impossible to maintain availability and consistency, not because two writes may go to separate partitions, but because the failure of an entire ‘quorum’ of servers may render some recent writes unreadable.

This is why defining ```P``` as ‘allowing partitioned groups to remain available’ is misleading – machine failures are partitions, almost tautologously, and by definition cannot be available while they are failed. Yet, Dr. Stonebraker says that he would suggest choosing CA rather than P. This feels rather like we are invited to both have our cake and eat it. Not ‘choosing’ P is analogous to building a network that will never experience multiple correlated failures. This is unreasonable for a distributed system – precisely for all the valid reasons that are laid out in the CACM post about correlated failures, OS bugs and cluster disasters – so what a designer has to do is to decide between maintaining consistency and availability. Dr. Stonebraker tells us to choose consistency, in fact, because availability will unavoidably be impacted by large failure incidents. This is a legitimate design choice, and one that the traditional RDBMS lineage of systems has explored to its fullest, but it implicitly protects us neither from availability problems stemming from smaller failure incidents, nor from the high cost of maintaining sequential consistency.

When the scale of a system increases to many hundreds or thousands of machines, writing in such a way to allow consistency in the face of potential failures can become very expensive (you have to write to one more machine than failures you are prepared to tolerate at once). This kind of nuance is not captured by the CAP theorem: consistency is often much more expensive in terms of throughput or latency to maintain than availability. Systems such as ZooKeeper are explicitly sequentially consistent because there are few enough nodes in a cluster that the cost of writing to quorum is relatively small. The Hadoop Distributed File System (HDFS) also chooses consistency – three failed datanodes can render a file’s blocks unavailable if you are unlucky. Both systems are designed to work in real networks, however, where partitions and failures will occur*, and when they do both systems will become unavailable, having made their choice between consistency and availability. That choice remains the unavoidable reality for distributed data stores.

下面说说我对CAP的理解:

1) A: 可用性。 主要是在高负载下的可用性, 以及低延迟响应. 这个在当前的系统设计中是排在第一位的, 尽量保证服务不会失去响应

2）C： 一致性, 强一致性, 或是时序一致性, 或是滞后的最终一致性. 分别代表了系统需要保障A和P的能力时, 在一致性上的妥协.


3）P：容错性, 在节点间通信失败时保证系统不受影响. 对容错的要求提高会降低对可用性或一致性的期望, 要么停止系统用于错误恢复, 要么继续服务但是降低一致性

<br />
<br />

**[参看]:**

1. [CAP理论中分区容错性的理解](https://blog.csdn.net/weixin_33859504/article/details/85999230)

<br />
<br />
<br />


