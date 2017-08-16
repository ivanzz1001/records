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

<br />
<br />
<br />


