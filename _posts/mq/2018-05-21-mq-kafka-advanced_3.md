---
layout: post
title: kafka数据迁移(转)
tags:
- mq
categories: mq
description: kafka进阶
---


本文重点介绍kafka的两类常见数据迁移方式：

* broker内部不同数据盘之间的分区数据迁移

* 不同broker之间的分区数据迁移


<!-- more -->


## 1. broker内部不同数据盘之间的分区数据迁移

### 1.1 背景介绍
最近，腾讯云的一个重要客户发现kafka broker内部的topic分区数据存储分布不均匀，导致部分磁盘100%耗尽，而部分磁盘只有40%的消耗量。

分析原因，发现存在部分topic的分区数据过于集中在某些磁盘导致，比如，以下截图显示的*/data5*数据盘：

![kafka-unblance](https://ivanzz1001.github.io/records/assets/img/mq/kafka_unbalance.jpg)

根据分布式系统的特点，很容易想到采取数据迁移的办法，对broker内部不同数据盘的分区数据进行迁移。在进行线上集群数据迁移之前，为了保证集群的数据完整和安全，必须先在测试集群进行测试。

### 1.2 测试broker内部不同数据盘进行分区数据迁移
1) **建立测试topic并验证生产和消费正常**

我们搭建的测试集群，kafka有3个broker，hostname分别为:

* tbds-172-16-16-11

* tbds-172-16-16-12

* tbds-172-16-16-16

每个broker配置了两块数据盘，缓存数据分别存储在*/data/kafka-logs/*和*/data1/kafka-logs/*。

首先建立测试topic:
<pre>
# ./kafka-topics.sh --create --zookeeper tbds-172-16-16-11:2181 --replication-factor 2 --partitions 3 --topic test_topic
</pre>

然后向topic生产发送500条数据，发送的时候也同时消费数据。然后查看topic的分区数据情况：
<pre>
# ./kafka-consumer-groups.sh --zookeeper tbds-172-16-16-11:2181 --describe --group groupid1
GROUP      TOPIC       PARTITION   CURRENT-OFFSET   LOG-END-OFFSET   LAG   OWNER
groupid1   test_topic  0           172              172              0     kafka-python-1.3.1_tbds-172-16-16-3/172.16.16.3
groupid1   test_topic  1           156              156              0     kafka-python-1.3.1_tbds-172-16-16-3/172.16.16.3
groupid1   test_topic  2           172              172              0     kafka-python-1.3.1_tbds-172-16-16-3/172.16.16.3
</pre>






<br />
<br />

**[参考]**


1. [kafka数据迁移实践](https://blog.csdn.net/mnasd/article/details/82772714)


<br />
<br />
<br />

