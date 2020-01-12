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




## 4. kafka controller





<br />
<br />

**[参考]**



1. [kafka官网](https://kafka.apache.org/)

2. [Kafka的分区数和消费者个数](https://www.jianshu.com/p/dbbca800f607)


<br />
<br />
<br />

