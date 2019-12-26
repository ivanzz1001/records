---
layout: post
title: kafka集群参数配置
tags:
- mq
categories: mq
description: kafka集群参数配置
---

kafka集群可配置参数有很多，大部分都可以采用默认，很多时候我们可能并不需要关心。在这里我们主要讲述一下那些常用的参数配置。


<!-- more -->


## 1. kafka参数配置
kafka采用key-value（键值）对的方式来进行参数配置。主要分成如下几大类：

* Broker Configs

* Topic Configs

* Producer Configs

* Consumer Configs

* Kafka Connect Configs

* Kafka Streams Configs

* AdminClient Configs

下面我们对每类中常用的一些参数配置做一个简单介绍。

### 1.1 Broker Configs

对于一个broker，如下3个参数是必须要进行配置的：

* broker.id

* log.dirs

* zookeeper.connect

对于topic级别的配置，我们会在下一节进行讲述。

1） **zookeeper.connect**

zookeeper在kafka中充当分布式协调框架的作用，负责协调管理和保存kafka集群的所有Meta数据，比如集群有哪些broker在运行，有哪些topic，以及每个topic有哪些partition，partition中leader replica在哪些机器上。

本配置项用于指定broker所连接的kafka集群，配置格式为*hostname:port*。为实现容错性，通常zookeeper是以集群的方式运行的，因此我们也可以配置多个zookeeper地址，地址与地址之间以```逗号```分割，例如：
<pre>
zookeeper.connect=hostname1:port1,hostname2:port2,hostname3:port3
</pre>

通过上面的方式，默认kafka会将集群的相关信息存放到```/```目录下，而如果我们想更改到一个我们自己设定的目录的话，我们可以采用如下方式：
<pre>
zookeeper.connect=hostname1:port1,hostname2:port2,hostname3:port3/chroot/path
</pre>
这样，我们可以实现的一个功能是： 假如我们有两套不同的kafka集群共用同一个zookeeper集群，这样我们就可以通过不同的根目录来存放相应的信息，而不会产生冲突。

2) **advertised.listeners**

kafka会将本参数所配置的地址发布到zookeeper，以使其他客户端可以使用该发布的地址。假如不配置本参数的话，则默认与*listeners*配置参数相同。（通常我们不需要配置本参数）

3） **listeners**

监听器，即外部连接者通过*protocol://hostname:port*访问开放的kafka服务，这里hostname最好是主机名，而不是IP地址。本参数用于指明kafka的listener列表，列表元素之间以```逗号```分割。如果我们所指定的listener name并不是一个安全协议的话，我们还需要通过*listener.security.protocol.map*来配置所采用的协议。如果我们将hostname配置为```0.0.0.0```，表示绑定所有网卡；如果我们将hostname配置为空，则表示绑定到默认的网卡。例如：
<pre>
listeners=PLAINTEXT://myhost:9092,SSL://:9091 CLIENT://0.0.0.0:9092,REPLICATION://localhost:9093
</pre>


4） **auto.create.topics.enable**

本参数用于控制是否允许在kafka上自动创建topic。通常在我们consume(消费)或者produce(生产）消息时，如果对应的topic不存在，则会自动创建该topic。但这样可能造成topic名混乱，因此我们可以将此配置参数设置为false，以禁止自动创建。

5) **background.threads**

用于指明有多少个后台线程来处理任务。

6） **broker.id**

用于指明本broker的ID。假如未设置的话，则会自动产生一个唯一的broker id。为了避免zookeeper自动产生的broker id与我们设置的broker id相冲突，zookeeper会从*reserved.broker.max.id+1*开始产生。

7） **delete.topic.enable**

用于配置是否允许删除topic。如果是通过admin tool来删除topic的话，通常不受本配置项的影响。

8） **log.dirs**

用于指明kafka的message信息存储在什么位置，可以设置多个路径，比如：/home/kafka1,/home/kafka2,/home/kafka3。最好这些dirs是mount到不同物理磁盘上。好处是：提升读写性能、故障转移，防止磁盘坏了导致broker进程关闭，相当于简约RAID方案

9) **log.retention.bytes**

Kafka会定期为那些超过磁盘空间阈值的topic进行日志段的删除。这个阈值由broker端参数*log.retention.bytes*和topic级别参数*retention.bytes*控制，默认是-1，表示Kafka当前未开启这个留存机制，即不管topic日志量涨到多少，Kafka都不视其为“超过阈值”。如果用户要开启这种留存机制，必须显式设置log.retention.bytes（或retention.bytes）。

一旦用户设置了阈值，那么Kafka就会在定时任务中尝试比较当前日志量```总大小```是否超过阈值至少一个日志段的大小(参看*log.segment.bytes*)。这里所说的总大小是指所有日志段文件的大小，不包括索引文件的大小。如果是则会尝试从最老的日志段文件开始删起。注意这里的“超过阈值至少一个日志段的大小”，这就是说超过阈值的部分必须要大于一个日志段的大小，否则不会进行删除的，原因就是因为删除的标的是日志段文件——即文件只能被当做一个整体进行删除，无法删除部分内容。

举个例子来说明，假设日志段大小是700MB，当前分区共有4个日志段文件，大小分别是700MB，700MB，700MB和1234B——显然1234B那个文件就是active日志段。此时该分区总的日志大小是3*700MB+1234B=2100MB+1234B，如果阈值设置为2000MB，那么超出阈值的部分就是100MB+1234B，小于日志段大小700MB，故Kafka不会执行任何删除操作，即使总大小已经超过了阈值；反之如果阈值设置为1400MB，那么超过阈值的部分就是700MB+1234B > 700MB，此时Kafka会删除最老的那个日志段文件。

注：“上面我们说超过阈值至少一个日志段大小，会对日志进行删除”，其实准确来说是会对日志按“日志留存策略”进行处理。kafka目前支持两种日志留存策略：delete与compact(可以通过log.cleanup.policy来指定)

如果将此配置项设置为-1，表示不对日志进行控制。这是从```空间维度```来对日志进行管理的。

10) 



<br />
<br />

**[参考]**


1. [kafka官网](https://kafka.apache.org/)

2. [kafka configuration](https://kafka.apache.org/documentation/#configuration)

3. [kafka集群参数配置](https://blog.csdn.net/yujianping_123/article/details/96874189)

<br />
<br />
<br />

