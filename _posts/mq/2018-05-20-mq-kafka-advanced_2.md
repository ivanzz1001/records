---
layout: post
title: kafka进阶_2
tags:
- mq
categories: mq
description: kafka进阶
---

本章我们主要会介绍如下方面的内容：

* kafka常用操作

* kafka日志数据

* kafka分区数的选择

* kafka controller是什么？


<!-- more -->

## 1. kafka常用操作

## 2. kafka日志数据

## 3. 如何确定kafka分区数

### 3.1 kafka分区数是不是越多越好？

###### 分区多的优点
kafka使用分区将topic的消息打散到多个分区分布保存在不同的broker上，实现了producer和consumer消息处理的高吞吐量。kafka的producer和consumer都可以多线程地并行操作，而每个线程处理的是一个分区的数据。因此分区实际上是调优kafka并行度的最小单元。对于producer而言，它实际上是用多个线程并发地向不同分区所在的broker发起socket连接同时给这些分区发送消息； 而consumer，同一个消费组内所有consumer线程都被指定topic的某一个分区进行进行消费。

所以说，如果一个topic分区越多，理论上整个集群所能达到的吞吐量就越大。


###### 分区不是越多越好

分区是否越多越好呢？显然也不是，每个分区都有自己的开销：

1） **客户端/服务器端内存开销**

kafka 0.8.2之后，在客户端producer有个参数batch.size，默认是16KB。它会为每个分区缓存消息，一旦满了就打包将消息批量发送出去。看上去这是个能够提升性能的设计。不过很显然，因为这个参数是分区级别的，如果分区数越多，这部分缓存所需的内存占用也会更多。假设你有10000个分区，按照默认设置，这部分缓存需要占用约157MB的内存。而consumer端呢？我们抛开获取数据所需的内存不说，只说线程的开销。如果还是假设10000个分区，同时consumer线程数要匹配分区数（注： 大部分情况下，这是最佳的消费吞吐量配置）的话，那么consumer client就需要创建10000个线程，也需要大约10000个socket去获取分区数据。这里面线程切换的开销本身就已经不容小觑了。

服务端的开销也不小，如果阅读kafka源码的话可以发现，服务端的很多组件都在内存中维护了分区级别的缓存，比如controller，FetcherManager等，因此分区数越多，这种缓存的成本就越大。

2) **文件句柄的开销**

每个分区在底层文件系统都有属于自己的一个目录。该目录下通常会有两个文件：base_offset.log和base_offset.index。kafka的controller和ReplicaManager会为每个broker都保存这两个句柄。很明显，如果分区数越多，所需要保持打开状态的文件句柄数也就越多，最终可能会突破你的```ulimit -n```的限制。

3) **降低高可用性**

kafka通过副本(replica)机制来保证高可用。具体做法就是为每个分区保存若干个副本(replica_factor指定副本数）。每个副本保存在不同的broker上，其中的一个副本充当leader，负责处理producer和consumer的读写请求； 其他副本充当follower角色，由kafka controller负责保证与leader的同步。如果leader所在的broker挂掉了，controller会检测到然后在zookeeper的帮助下重新选出新的leader———这中间会有短暂的不可用时间窗口，虽然大部分情况下可能只是几毫秒级别。但如果你有10000个分区，10个broker，也就是说每个broker上有1000个分区。此时这个broker挂掉了，那么zookeeper和controller需要立即对这1000个分区进行Leader选举。比起很少的分区Leader选举而言，这必然要花更长的时间，并且通常不是线性累加的。如果这个broker同时还是controller，情况就更糟了。

###### 如何确定分区数量
可以遵循一定的步骤来尝试确定分区数： 创建一个只有一个分区的topic，然后测试这个topic的producer吞吐量和consumer吞吐量。假设它们的值分别是```Tp```和```Tc```，单位可以是MB/s。然后假设总的目标吞吐量是```Tt```，那么分区数是: p = Tt/max(Tp, Tc)。

说明：Tp表示producer的吞吐量。测试producer通常是很容易的，因为它的逻辑非常简单，就是直接发送消息到kafka就好了。Tc表示consumer的吞吐量，测试```Tc```通常与应用的关系更大，因为```Tc```的值取决于你拿到消息之后执行什么操作，因此```Tc```的测试也要更麻烦些。

### 3.2 消息被发送到哪个分区？

一条消息要被发送到哪个partition，通常是按如下方式来进行处理的：

1） **按照key值分配**

默认情况下，kafka根据传递消息的key来进行分区的分配，即*hash(key)%numPartitions*:
{% highlight string %}
def partition(key: Any, numPartitions: Int): Int = {
    Utils.abs(key.hashCode) % numPartitions
}
{% endhighlight %}
这保证了相同key的消息一定会被路由到相同的分区。



2) **key为null时，从缓存中取分区ID或者随机取一个**

如果你没有指定key，那么kafka是如何确定这条消息去往哪个分区的呢？
{% highlight string %}
if(key == null) {  
    val id = sendPartitionPerTopicCache.get(topic)	// 先看看Kafka有没有缓存的现成的分区Id
    id match {
      case Some(partitionId) =>  // 如果有的话直接使用这个分区Id就好了
        partitionId	

      case None => // 如果没有的话，
        val availablePartitions = topicPartitionList.filter(_.leaderBrokerIdOpt.isDefined)  //找出所有可用分区的leader所在的broker
        if (availablePartitions.isEmpty)
          throw new LeaderNotAvailableException("No leader for any partition in topic " + topic)
        val index = Utils.abs(Random.nextInt) % availablePartitions.size	// 从中随机挑一个
        val partitionId = availablePartitions(index).partitionId
        sendPartitionPerTopicCache.put(topic, partitionId)	// 更新缓存以备下一次直接使用
        partitionId
    }
}
{% endhighlight %}

不指定key时，kafka几乎就是随机找一个分区发送无key的消息，然后把这个分区号加入到缓存中以备后面直接使用。当然了，kafka本身也会清空该缓存（默认每10分钟或者每次请求topic元数据时）。

### 3.3 consumer个数与分区数

topic下的一个分区只能被同一个consumer group下的一个consumer线程来消费，但反之并不成立，即一个consumer线程可以消费多个分区的数据，比如kafka提供的ConsoleConsumer，默认就只是一个线程来消费所有分区的数据。

>即分区数决定了同组消费者个数的上限

![kafka-pc](https://ivanzz1001.github.io/records/assets/img/mq/kafka_partition_consumer.jpg)

所以，如果你的分区数是N，那么最好线程数也保持为N，这样通常能够达到最大的吞吐量。超过N的配置只是浪费系统资源，因为多出的线程不会被分配到任何分区。

###### Consumer消费partition的分配策略
kafka提供了两种分配策略： range和round robin，由参数*partition.assignment.strategy*指定，默认是range策略。

当以下事件发生时，kafka将会进行一次分区分配：

* 同一个Consumer Group内新增消费者

* 消费者离开当前所属的Consumer Group，包括shutdown 或 crash

* 订阅的主题新增分区

将分区的所有权从一个消费者移动到另一个消费者，称为重新平衡(rebalance)。如何rebalance就涉及到本文提到的分区分配策略。

下文我们将详细介绍kafka内置的两种分区分配策略。本文假设我们有个名为```T1```的topic，其中包含了10个分区，然后我们有两个消费者(C1,C2)来消费这10个分区里面的数据，而且C1的*num.streams=1*， C2的*num.streams=2*。


1） **Range strategy**

range策略是对每个topic而言的，首先对同一个topic里面的分区按照序号进行排序，并对消费者按照字母顺序进行排序。在我们的例子里面，排完序的分区将会使0，1，2，3，4，5，6，7，8，9； 消费者线程排完序将会是C1-0，C2-0，C2-1。然后将*partitions的个数*除以*消费者线程的总数*来决定每个消费者线程消费几个分区。如果除不尽，前面几个消费者线程将会多消费一个分区。在我们的例子里面，我们有10个分区，3个消费者线程，10/3=3，而且除不尽，那么消费者线程C1-0将会多消费一个分区，随意最后分区分配的结果看起来是这样的：

* C1-0将消费0,1,2,3分区

* C2-0将消费4,5,6分区

* C2-1将消费7,8,9分区

假如我们有11个分区，那么最后分区分配的结果看起来是这样的：

* C1-0将消费0,1,2,3分区

* C2-0将消费4,5,6,7分区

* C2-1将消费8,9,10分区

假如我们有两个topic(名称分别为T1和T2），分别有10个分区，那么最后分区分配的结果看起来是这样的：

* C1-0将消费T1这个topic的0,1,2,3分区； C1-0将消费T2这个topic的0,1,2,3分区

* C2-0将消费T1这个topic的4,5,6分区； C2-0将消费T2这个topic的4,5,6分区

* C2-1将消费T1这个topic的7,8,9分区； C2-1将消费T2这个topic的7,8,9分区


可以看出，C1-0这个消费者线程比其他消费者线程多消费了2个分区，这就是range strategy的一个很明显的弊端。

2) **RoundRobin strategy**

使用roundrobin策略有两个前提条件必须满足：

* 同一个consumer group里面的所有消费者的num.streams必须相等；

* 每个消费者订阅的topic必须相同

所以这里假设前面提到的两个消费者的*num.streams=2*。Roundrobin策略的工作原理：将所有topic的分区组成TopicAndPartion列表，然后对TopicAndPartition列表按照hashCode进行排序，参看如下代码：
{% highlight string %}
val allTopicPartitions = ctx.partitionsForTopic.flatMap { case(topic, partitions) =>
  info("Consumer %s rebalancing the following partitions for topic %s: %s"
       .format(ctx.consumerId, topic, partitions))
  partitions.map(partition => {
    TopicAndPartition(topic, partition)
  })
}.toSeq.sortWith((topicPartition1, topicPartition2) => {
  /*
   * Randomize the order by taking the hashcode to reduce the likelihood of all partitions of a given topic ending
   * up on one consumer (if it has a high enough stream count).
   */
  topicPartition1.toString.hashCode < topicPartition2.toString.hashCode
})
{% endhighlight %}
最后按照round-robin风格将分区分别分配给不同的消费者线程。

在这个例子里面，假如按照hashCode排完序的topic-partitions组依次为：T1-5、T1-3、T1-0、T1-8、T1-2、T1-1、T1-4、T1-7、T1-6、T1-9，我们的消费者线程排序为C1-0、C1-1、C2-0、C2-1，最后分区分配的结果为：

* C1-0将消费T1-5、T1-2、T1-6分区；

* C1-1将消费T1-3、T1-1、T1-9分区；

* C2-0将消费T1-0、T1-4分区

* C2-1将消费T1-8、T1-7分区

多个topic的分区分配和单个topic类似。遗憾的是，目前我们还不能自定义分区分配策略，只能通过partition.assignment.strategy参数选择range或round-robin。



## 4. kafka controller

本节描述一下kafka controller的实现原理，并对其源代码的实现进行讲解。

### 4.1 controller运行原理

在kafka集群中，controller是多个broker中的一个（也只有一个controller)。它除了实现了正常的broker功能外，还负责选举分区(partition)的Leader。第一个启动的broker会成为一个controller，它会在zookeeper创建一个临时节点(ephemeral): /controller。其他后启动的broker也尝试去创建这样一个临时节点，但会报错，此时这些broker会在该zookeeper的/controller节点上创建一个监控(watch)，这样当该节点状态发生变化（比如： 被删除）时，这些broker就会得到通知。此时，这些broker就可以在得到通知时，继续创建该节点。保证该集群一直都有一个controller节点。

当controller所在的broker节点宕机或断开和zookeeper的连接，它在zookeeper上创建的临时节点就会被自动删除。其他在该节点上都安装了监控的broker节点都会得到通知，此时，这些broker都会尝试去创建这样一个临时的/controller节点，但它们当中只有一个broker(最先创建的那个)能够创建成功，其他的broker会报错：node already exists，接收到该错误的broker节点会再次在该临时节点上安装一个watch来监控该节点状态的变化。每次当一个broker被选举时，将会赋予一个更大的数字（通过zookeeper的条件递增实现），这样其他节点就知道controller目前的数字。


当一个broker宕机而不在当前kafka集群中时，controller将会得到通知（通过监控zookeeper的路径实现），若有些topic的主分区恰好在该broker上，此时controller将重新选择这些主分区。controller将会检查所有没有leader的分区，并决定新的leader是谁（简单的方法是： 选择该分区的下一个副本分区），并给所有的broker发送请求。

每个


<br />
<br />

**[参考]**



1. [kafka官网](https://kafka.apache.org/)

2. [Kafka的分区数和消费者个数](https://www.jianshu.com/p/dbbca800f607)

3. [kafka源码分析—Controller](https://blog.csdn.net/zg_hover/article/details/81672997)


<br />
<br />
<br />

