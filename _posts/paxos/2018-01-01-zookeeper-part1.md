---
layout: post
title: Zookeeper的安装及使用
tags:
- paxos
categories: paxos
description: Zookeeper的安装及使用
---


本文首先会对zookeeper做一个简单介绍，之后再给出相应的示例展示zookeeper的用法。

<!-- more -->


## 1. zookeeper简介
zookeeper是一个中心化的服务，主要用于：

* 维护配置信息

* 提供名称服务

* 分布式同步

* 组服务(group service)

上面所有的这些服务，通常都是作为整个分布式系统中的一个模块被使用。假如我们每一次都需要自己来实现这样一些服务的话，则可能会变得十分繁琐且容易出错，因此我们将其独立出来形成一个标准化的东西就很有必要。



<br />
<br />
**参看：**

1. [Zookeeper和 Google Chubby对比分析](https://www.cnblogs.com/grefr/p/6088115.html)

2. [The Chubby lock service for loosely coupled distributed systems](https://github.com/lwhile/The-Chubby-lock-service-for-loosely-coupled-distributed-systems-zh_cn)

3. [zookeeper官网](https://zookeeper.apache.org/)

<br />
<br />
<br />


