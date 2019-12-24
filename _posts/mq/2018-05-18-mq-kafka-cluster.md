---
layout: post
title: kafka体系架构及集群环境的搭建
tags:
- mq
categories: mq
description: kafka体系架构及集群环境的搭建
---

本文主要介绍一下如下两个方面的内容：

* kafka体系架构

* kafka集群环境的搭建


<!-- more -->


## 1. kafka体系架构
kafka是一个分布式的、基于发布订阅的消息系统，主要解决应用解耦、异步消息、流量削峰等问题。

### 1.1 发布订阅模型
消息生产者将消息发布到topic中，同时有多个消息消费者订阅该消息，消费者消费数据之后，并不会清除消息。属于一对多的模式，如下图所示：

![kafka-pubsub-model](https://ivanzz1001.github.io/records/assets/img/mq/kafka_pubsub_model.png)


### 1.2 系统架构
如下是kafka的一个整体架构：

![kafka-arch](https://ivanzz1001.github.io/records/assets/img/mq/kafka_arch.jpg)

上图中标识了一个kafka体系架构，包括若干producer、broker、consumer和一个zookeeper集群。如下再贴出两张带有topic和partition的架构图：

![kafka-work-1](https://ivanzz1001.github.io/records/assets/img/mq/kafka_work_1.jpg)

![kafka-work-2](https://ivanzz1001.github.io/records/assets/img/mq/kafka_work_2.jpg)


下面介绍一下各个角色：

1） **Producer**

消息生产者，将消息push到kafka集群中的broker。

2） **Consumer**

消息消费者，从kafka集群中pull消息，消费消息。

3) **Consumer Group**

消费组，由一到多个consumer组成，每个consumer都属于一个consumer group。消费组在逻辑上是一个订阅者。消费组内每个消费者负责消费不同分区的数据，一个分区只能由一个组内消费者消费；消费组之间互不影响。即每条消息只能被consumer group中的一个consumer，但是可以被多个consumer group消费。这样就实现了单播和多播。


4） **Broker**

一台kafka服务器就是一个Broker，一个集群由多个Broker组成，每个Broker可以容纳多个topic。

5） **Topic**

消息的类别或主题，逻辑上可以理解为队列。Producer只关注消息到哪个Topic，Consumer只关注订阅了哪个Topic。

6） **Partition**

负载均衡与扩展性考虑，一个Topic可以分为多个Partition，物理存储在kafka集群中的多个Broker上。从可靠性上考虑，每个partition都会有备份Replica。

7） **Replica**

Partition的副本，为了保证集群中某个节点发生故障时，该节点上的Partition数据不会丢失，且kafka仍能继续工作，所以kafka提供了副本机制。一个Topic的每个Partition都有若干个副本： 一个Leader和若干个Follower。

8） **Leader**

Replica的主角色，Producer与Consumer只跟Leader进行交互。

9) **Follower**

Replica的从角色，实时从Leader中同步数据，保持和Leader数据的同步。Leader发生故障时，某个Follower会变成新的Leader。

10) **Controller**

kafka集群中的其中一台服务器，用来进行Leader election以及各种Failover(故障转移）

11) **Zookeeper**

kafka通过zookeeper存储集群的meta信息。




### 1.3 Topic和Partition

一个topic可以认为是一类信息，逻辑上的队列，每条消息都要指定topic。为了使得kafka的吞吐量可以线性提高，物理上将topic分成一个或多个partition。每个partition在存储层面是append log文件，消息push进来后，会被追加到log文件的尾部。每条消息在文件中的位置称为offset(偏移量)，offset是一个long型数字，唯一的标识一条信息。因为每条信息都追加到partition的尾部，所以属于磁盘的顺序写，效率很高。如图：

![kafka-partition](https://ivanzz1001.github.io/records/assets/img/mq/kafka_partition.png)

### 1.4 网络模型

kafka的网络模型基于reactor模型。以下来自:[消息中间件—简谈Kafka中的NIO网络通信模型](https://www.jianshu.com/p/a6b9e5342878)

![kafka-reactor](https://ivanzz1001.github.io/records/assets/img/mq/kafka_reactor.jpg)

![kafka-reactor-detail](https://ivanzz1001.github.io/records/assets/img/mq/kafka_reactor_detail.jpg)


1) **Acceptor**

1个接收线程，负责监听新的连接请求，同时注册```OP_ACCEPT```事件，将新的连接按照*round robin*方式交给对应的Processor线程处理。

2） **Processor**

N个处理线程，其中每个Processor都有自己的selector，它会向Acceptor分配的SocketChannel注册相应的```OP_READ```事件，N的大小由*num.networker.threads*决定；

3） **KafkaRequestHandler**

M个处理线程，包含在线程池KafkaRequestHandlerPool内部，从RequestChannel的全局请求队列requestQueue中获取请求数据并交给KafkaApis处理，M的大小由*num.io.threads*决定。


4） **RequestChannel**

其为Kafka服务端的请求通道，该数据结构中包含了一个全局的请求队列requestQueue和多个与Processor处理器相对应的响应队列responseQueue，提供给Processor与请求处理线程KafkaRequestHandler、Processor与KafkaApis交换数据的地方。

5） **NetworkClient**

其底层是对Java NIO进行相应的封装，位于kafka的网络接口层。Kafka消息生产者对象KafkaProducer的send()方法主要调用NetworkClient完成消息发送。

6） **SocketServer**

其是一个NIO的服务，它同时启动一个Acceptor接收线程和多个Processor处理线程。提供了一种典型的reactor多线程模式，将接收客户端请求和处理请求相分离。

7） **KafkaServer**

代表了一个Kafka Broker实例。其startup方法为实例启动的入口。

8） **KafkaApis**

Kafka的业务逻辑处理API，负责处理不同类型的请求。比如，“发送消息”、“获取消息偏移量offset”和“处理心跳请求”等。




<br />
<br />

**[参考]**


1. [kafka工作原理介绍](https://blog.csdn.net/qq_29186199/article/details/80827085)

2. [kafka官网](https://kafka.apache.org/)

3. [kafka体系架构](https://segmentfault.com/a/1190000021175583?utm_source=tag-newest)

4. [Consumer的offset保存在哪里](https://blog.csdn.net/chaiyu2002/article/details/86545658)

<br />
<br />
<br />

