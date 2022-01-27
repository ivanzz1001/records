---
layout: post
title: pulsar架构
tags:
- mq
categories: mq
description: pulsar架构
---


本文从整体上介绍一下Pulsar的架构，在此做个记录。



<!-- more -->

## 1. Architecture Overview

从更高层级来看，一个Pulsar实例(instance)是由一个或多个Pulsar cluster所组成的。一个instance中的clusters可以相互之间自我复制数据(replicate data)。

在一个Pulsar集群中：

* One or more brokers handles and load balances incoming messages from producers, dispatches messages to consumers, communicates with the Pulsar configuration store to handle various coordination tasks, stores messages in BookKeeper instances (aka bookies), relies on a cluster-specific ZooKeeper cluster for certain tasks, and more.

* A BookKeeper cluster consisting of one or more bookies handles persistent storage of messages.

* A ZooKeeper cluster specific to that cluster handles coordination tasks between Pulsar clusters.

下图是一个Pulsar集群的整体架构：

![pulsar](https://ivanzz1001.github.io/records/assets/img/mq/pulsar-system-architecture.png)

在Pulsar实例的层级，有一个实例级别(instance-wide) Zookeeper集群，被称为configuration store，用于处理涉及多集群之间的任务协调。

## 2. Brokers

Pulsar消息broker是一个是一个无状态的组件，主要负责运行如下两个其他的组件：

* HTTP Server，向producers及consumers暴露任务管理及[topic lookup](https://pulsar.apache.org/docs/en/concepts-clients#client-setup-phase)相关的RESTful接口。生产者连接到broker来发布消息，消费者连接到broker来消费消息。

* 消息分发器(dispatcher)，是一个采用私有二进制协议([binary protocol](https://pulsar.apache.org/docs/en/develop-binary-protocol))所实现的异步TCP服务器，主要用于数据的传输。

基于性能方面的考虑，消息(messages)通常是从[managerd ledger](https://pulsar.apache.org/docs/en/concepts-architecture-overview/#managed-ledgers)所分发出去，除非backlog超出了缓存大小。假如backlog增长到超出cache大小，broker将会从BookKeeper中读取消息条目。

最后，针对全局topic(global topic)，为了支持geo-replication，broker会使用复制器(replicator)将本区域的消息条目重新发布到远程区域。

> For a guide to managing Pulsar brokers, see the brokers [guide](https://pulsar.apache.org/docs/en/admin-api-brokers) guide.

## 3. Clusters

一个Pulsar instance是由一个或多个pulsar集群所组成的。而clusters又是由如下组成：

* 一个或多个pulsar brokers

* Zookeeper所实现的quorum，用于cluster级别的配置和协调

* 一个bookies集群来持久化存储消息(message)

clusters之间可以基于[geo-replication](https://pulsar.apache.org/docs/en/concepts-replication)来自我复制数据。

> For a guide to managing Pulsar clusters, see the [clusters](https://pulsar.apache.org/docs/en/admin-api-clusters) guide.

## 4. Metadata store
Pulsar metadata store维持着```一个```pulsar集群的所有元数据信息，比如topic元数据、schema、broker负载信息等等。Pulsar使用[Apache Zookeeper](https://zookeeper.apache.org/)来做元数据存储、配置集群信息以及协调操作。Pulsar metadata store可以部署到一个单独的Zookeeper集群上，也可以部署到一个已存在的Zookeeper集群上。你可以使用一个Zookeeper集群来同时存放Pulsar metadata store以及[BookKeeper metadata sotre](https://bookkeeper.apache.org/docs/latest/getting-started/concepts/#metadata-storage)。假如想要部署Pulsar brokers连接到已存在的BookKeeper集群，你需要各自单独的为Pulsar metadata store及BookKeeper metadata store分别部署一个Zookeeper集群。

在一个Pulsar instance中：

* configuration store quorum存放tenants、namespaces和其他一些需要全局一致性的配置信息；

* 每一个cluster都有其自己的本地Zookeeper，用于存储特定集群(cluster-specific)的配置和协调信息，比如哪些brokers负责某个topic、报告broker load等。

## 5. Configuration store
configuration store维护着一个pulsar实例(instance)的所有配置信息，比如clusters、tenants、namespaces、partitioned topic相关的配置信息等等。一个Pulsar实例可以是单独一个local cluster，也可以是多个local clusters，还可以是多个跨区域clusters。因此，在一个Pulsar实例中，configuration store可以跨clusters共享配置信息。configuration store可以被部署到一个单独的Zookeeper集群，也可以被部署到一个已存在的Zookeeper集群。

## 6. Persistent storage



<br />
<br />

**[参考]**

1. [pulsar 官网](https://pulsar.apache.org/)

2. [pulsar 中文网](https://pulsar.apache.org/docs/zh-CN/next/concepts-architecture-overview/)

3. [pulsar 集群搭建](https://blog.csdn.net/weixin_33775572/article/details/92127055)


<br />
<br />
<br />

