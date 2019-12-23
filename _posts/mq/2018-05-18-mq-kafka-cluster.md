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




<br />
<br />

**[参考]**


1. [kafka工作原理介绍](https://blog.csdn.net/qq_29186199/article/details/80827085)

2. [kafka官网](https://kafka.apache.org/)

3. [kafka体系架构](https://segmentfault.com/a/1190000021175583?utm_source=tag-newest)

<br />
<br />
<br />

