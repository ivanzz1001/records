---
layout: post
title: 一个具有层级结构的crushmap示例
tags:
- ceph
- crushmap
categories: ceph
description: crushmap示例
---


前面我们较为详细的介绍了crush算法，为了进一步理解并掌握crush算法，这里我们给出一个具有层级结构的crushmap示例。本文参考：[CRUSHMAP : Example of a Hierarchical Cluster Map](http://ceph.com/geen-categorie/crushmap-example-of-a-hierarchical-cluster-map/)

<!-- more -->


## 1. crushmap层级结构示例

通常我们并不太容易知道如何在crushmap中组织数据，特别是当你需要将数据存储到不同类型的硬盘（例如：SATA硬盘、SAS硬盘、SSD硬盘）上时。下面我们来看如何将crushmap想象成为具有层级结构。

**模型1：**

假设我们将数据存储在两个数据中心。
![crushmap-tree-1.png](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-tree-1.png)

<br />

**模型2：**

在我们介绍了cache pools之后，我们可以很容易的想象到添加SSD硬盘到存储集群中。如下示例中我们将SSD添加到新的主机上。添加完成后，我们需要管理两种类型的硬盘。在只需要描述cluster的物理分割时，我们可能会采用如下的层次结构：
![crushmap-tree-2.png](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-tree-2.png)


**模型3：**

在上面```模型2```的层级结构中，我们很快就会意识到这样的配置并不能支持为特定pool区分硬盘类型。

为了区分这些硬盘并且组织crushmap，最简单的方法就是从根节点开始重新复制一份该树形结构。这样我们就会有两个根：“default”（可以被命名为hdd)和“ssd”。
![crushmap-tree-3.1.png](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-tree-3.1.png)

另一个hdd与ssd混合的例子如下（你需要将每一个host分割开）：
![crushmap-tree-3.2.png](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-tree-3.2.png)


**模型4：**

上面```模型3```的问题是我们通过硬盘的不同类型来分割集群。这样的话我们就不能通过一个入口（entry）来选择“dc-1”和“dc-2”中任何一块硬盘。例如，我们就不能定义一个规则来将数据存储到数据中心的任何一块硬盘上。

要解决这个问题，我们可以在root层级上增加一个其他的入口点。例如，按如下方式添加一个新的entry point来允许访问所有类型的硬盘。
![crushmap-tree-4.png](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-tree-4.png)


**模型5：**

假若我们需要在树中维持更多逻辑关系，我们可以添加更多的层级。例如一些逻辑层级代表磁盘类型，而其他一些则代表物理分布。这些层级可以根据我们的需要来添加。例如，这里我们添加了一个“pool”层级，或者也可以被称为“type”层级或其他（图中浅绿部分）：
![crushmap-tree-5.1.png](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-tree-5.1.png)
![crushmap-tree-5.2.png](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-tree-5.2.png)



**模型6：**

在上面```模型5```中，虽然已经可以满足我们的需求了，但是因为层级太多，不便于我们阅读。特别是在当同一台宿主机上同时存在SSD和HDD时就会变得更加难以阅读。同样在这种情况下，```模型5```也并不能在物理数据存放层面进行逻辑区分。要解决这个问题，我们可以在HOST和OSD之间插入一个新的层级：
![crushmap-tree-6.png](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap-tree-6.png)


<br />
<br />
<br />


