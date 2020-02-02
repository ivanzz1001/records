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
发现test_topic生产和消费数据都正常。

2) **将分区数据在磁盘间进行迁移**

现在登录*tbds-172-16-16-12*这台broker节点，将test_topic的分区数据目录*/data1/kafka-logs/test_topic-0/*移动到*/data/kafka-logs/*目录：
<pre>
# mv /data1/kafka-logs/test_topic-0 /data/kafka-logs/
</pre>
查看*/data/kafka-logs/*目录下，分区*test_topic-0*的数据：
<pre>
# pwd
/data/kafka-logs/test_topic-0
# ll
total 28
-rw-r--r-- 1 kafka hadoop 10485760 Jan 18 15:28 000000000000000000000.index
-rw-r--r-- 1 kafka hadoop    21065 Jan 18 15:26 000000000000000000000.log
</pre>

3) **再次对测试topic生产和消费数据**

再次发送500条数据，同时消费数据，然后查看数据状况：
<pre>
# ./kafka-consumer-groups.sh --zookeeper tbds-172-16-16-11:2181 --describe --group groupid1
GROUP      TOPIC       PARTITION   CURRENT-OFFSET   LOG-END-OFFSET   LAG   OWNER
groupid1   test_topic  0           337              337              0     kafka-python-1.3.1_tbds-172-16-16-3/172.16.16.3
groupid1   test_topic  1           304              304              0     kafka-python-1.3.1_tbds-172-16-16-3/172.16.16.3
groupid1   test_topic  2           359              359              0     kafka-python-1.3.1_tbds-172-16-16-3/172.16.16.3
</pre>

再次查看*tbds-172-16-16-12*这个broker节点的/*data/kafka-logs/test_topic-0*分区目录下的数据：
<pre>
# pwd
/data/kafka-logs/test_topic-0
# ll
total 28
-rw-r--r-- 1 kafka hadoop 10485760 Jan 18 15:28 000000000000000000000.index
-rw-r--r-- 1 kafka hadoop    21065 Jan 18 15:26 000000000000000000000.log
</pre>
发现，从*/data1/kafka-logs/*移动到*/data/kafka-logs/*目录下的分区数据目录*test_topic-0*(也就是编号为0的分区）缓存数据并没有增加。

这是因为*test_topic*每个分区有两个replicas，因此，我们找到编号为0的另外一个分区replica数据存储在*tbds-172-16-16-16*这个broker节点。登陆*tbds-172-16-16-16*这个broker节点，打开编号为0的分区缓存数据目录，得到如下信息：
<pre>
# pwd
/data1/kafka-logs/test_topic-0
# ll
total 48
-rw-r--r-- 1 kafka hadoop 10485760 Jan 18 15:37 000000000000000000000.index
-rw-r--r-- 1 kafka hadoop    40801 Jan 18 15:38 000000000000000000000.log
</pre>
上面我们看到log文件的大小发生了改变，即*tbds-172-16-16-16*这台broker节点的分区数据目录*test_topic-0*内缓存数据量是增加的，也就是缓存有再次生产发送的message数据。

由此可见，经过移动之后的*tbds-172-16-16-12*这台broker节点的编号为0的分区数据缓存目录内，并没有新增缓存数据。与之对应的，没有做分区数据移动操作的*tbds-172-16-16-16*这台broker节点的编号为0的分区缓存数据目录内新增再次发送的数据。

是不是意味着不能在broker的磁盘间移动分区数据呢？

4） **调用重启大法：重启kafka**


重启kafka集群，重启完成后，发现*tbds-172-16-16-12*这台broker节点的编号为0的分区缓存数据目录内的数据也增加到正常水平：
<pre>
# pwd
/data/kafka-logs/test_topic-0
# ll
total 44
-rw-r--r-- 1 kafka hadoop 10485760 Jan 18 15:44 000000000000000000000.index
-rw-r--r-- 1 kafka hadoop    40801 Jan 18 15:44 000000000000000000000.log
</pre>
表明重启之后，broker的不同磁盘间迁移数据已经生效。

5) **验证磁盘间迁移分区数据生效**

再次向*test_topic-0*发送500条数据，同时消费数据，然后查看数据情况：
<pre>
# ./kafka-consumer-groups.sh --zookeeper tbds-172-16-16-11:2181 --describe --group groupid1
GROUP      TOPIC       PARTITION   CURRENT-OFFSET   LOG-END-OFFSET   LAG   OWNER
groupid1   test_topic  0           521              521              0     kafka-python-1.3.1_tbds-172-16-16-3/172.16.16.3
groupid1   test_topic  1           468              468              0     kafka-python-1.3.1_tbds-172-16-16-3/172.16.16.3
groupid1   test_topic  2           511              511              0     kafka-python-1.3.1_tbds-172-16-16-3/172.16.16.3
</pre>
查看*tbds-172-16-16-12*这个个broker节点的*test_topic-0*分区数据的缓存目录：
<pre>
# pwd
/data/kafka-logs/test_topic-0
# ll
total 72
-rw-r--r-- 1 kafka hadoop 10485760 Jan 18 15:53 000000000000000000000.index
-rw-r--r-- 1 kafka hadoop    63283 Jan 18 15:53 000000000000000000000.log
</pre>
查看*tbds-172-16-16-16*这个个broker节点的*test_topic-0*分区数据的缓存目录：
<pre>
# pwd
/data1/kafka-logs/test_topic-0
# ll
total 72
-rw-r--r-- 1 kafka hadoop 10485760 Jan 18 15:53 000000000000000000000.index
-rw-r--r-- 1 kafka hadoop    63283 Jan 18 15:53 000000000000000000000.log
</pre>
发现两个replicas完全一样。

### 1.3 结论
kafka broker内部不同数据盘之间可以自由迁移分区数据目录。迁移完成后，重启kafka即可生效。

## 2. 不同broker之间传输分区数据



<br />
<br />

**[参考]**


1. [kafka数据迁移实践](https://blog.csdn.net/mnasd/article/details/82772714)


<br />
<br />
<br />

