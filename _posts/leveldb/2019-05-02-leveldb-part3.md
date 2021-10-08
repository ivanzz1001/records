---
layout: post
title: LevelDB实现原理(1)
tags:
- leveldb
categories: leveldb
description: leveldb源代码分析
---


LevelDB是一个可持久化的KV数据库引擎，由Google传奇工程师Jeff Dean和Sanjay Ghemawat开发并开源。无论从设计还是代码上都可以用精致来形容，非常值得细细品味。本文将从整体架构、数据读写等方面介绍一下LevelDB。


<!-- more -->

## 1. 设计思路
做存储的同学都很清楚，对于普通机械磁盘来说顺序写的性能要比随机写大很多。比如对于15000转的SAS盘，4K写IO，顺序写在200MB/s左右，而随机写性能可能只有1MB/s左右。而LevelDB的设计思想正是利用了磁盘的这个特性。LevelDB的数据是存储在磁盘上的，采用LSM-Tree的结构实现。LSM-Tree将磁盘的随机写转化为顺序写，从而大大提高了写速度。为了做到这一点，LSM-Tree的思路是将索引树结构拆成一大一小两颗树，较小的一个常驻内存，较大的一个持久化到磁盘，它们共同维护一个有序的key空间。写入操作会首先操作内存中的树，随着内存中树的不断变大，会触发与磁盘中树的归并操作，而归并操作本身仅有顺序写。如下图所示：

![lsm-tree](https://ivanzz1001.github.io/records/assets/img/leveldb/leveldb-lsm-tree.png)

上图中，合并后的数据块会存储到磁盘空间，而这种存储方式是追加式的，也就是顺序写入磁盘。随着数据的不断写入，磁盘中的树会不断膨胀，为了避免每次参与归并操作的数据量过大，以及优化读操作的考虑，LevelDB将磁盘中的数据又拆分成多层，每一层的数据达到一定容量后会触发下一层的归并操作，每一层的数据量比上一层成倍增长。这也就是LevelDB的名称来源。

## 2. 主要特性

下面是LevelDB官方对其特性的描述，主要包括如下几点： 

* key和value都是任意长度的字节数组； 

* entry（即一条K-V记录）默认是按照key的字典顺序存储的，当然开发者也可以重载这个排序函数； 

* 提供的基本操作接口：Put()、Delete()、Get()、Batch()； 

* 支持批量操作以原子操作进行； 

* 可以创建数据全景的snapshot(快照)，并允许在快照中查找数据； 

* 可以通过前向（或后向）迭代器遍历数据（迭代器会隐含的创建一个snapshot；

* 自动使用Snappy压缩数据； 

* 可移植性；

## 3. 整体结构

对LevelDB有一个整体的认识之后，我们分析一下它的架构。这里面有一个重要的概念（或者模块）需要理解，分别是内存数据的Memtable，分层数据存储的SST文件，版本控制的Manifest、Current文件，以及写Memtable前的WAL。这里简单介绍各个组件的作用和在整个结构中的位置。在介绍之前，我们先看一下整体架构示意图：

![level-arch](https://ivanzz1001.github.io/records/assets/img/leveldb/leveldb-arch.jpg)


对于写数据，接口会同时写入内存表（MemTable）和日志中。当内存表达到阈值时，内存表冻结，变为Immutable MemTable，并将数据写入SST表中，其中SST表是在磁盘上的文件。下面是涉及到主要模块的简单介绍： 

* Memtable：内存数据结构，跳表实现，新的数据会首先写入这里；

* Log文件：写Memtable前会先写Log文件，Log通过append的方式顺序写入。Log的存在使得机器宕机导致的内存数据丢失得以恢复； 

* Immutable Memtable：达到Memtable设置的容量上限后，Memtable会变为Immutable为之后向SST文件的归并做准备，顾名思义，Immutable Mumtable不再接受用户写入，同时会有新的Memtable生成； 

* SST文件：磁盘数据存储文件。分为Level-0到Level-N多层，每一层包含多个SST文件；单层SST文件总量随层次增加成倍增长；文件内数据有序；其中Level0的SST文件由Immutable直接Dump产生，其他Level的SST文件由其上一层的文件和本层文件归并产生；SST文件在归并过程中顺序写生成，生成后仅可能在之后的归并中被删除，而不会有任何的修改操作。 

* Manifest文件： Manifest文件中记录SST文件在不同Level的分布，单个SST文件的最大最小key，以及其他一些LevelDB需要的元信息。 

* Current文件: 从上面的介绍可以看出，LevelDB启动时的首要任务就是找到当前的Manifest，而Manifest可能有多个。Current文件简单的记录了当前Manifest的文件名，从而让这个过程变得非常简单。

## 3. 写数据流程


## 4. 读数据流程


<br />
<br />

**[参看]**

1. [Leveldb高效存储实现](https://stor.51cto.com/art/201903/593197.htm)

2. [LevelDB深入浅出之整体架构](https://zhuanlan.zhihu.com/p/67833030)

3. [leveldb源码阅读系列](https://zhuanlan.zhihu.com/p/80684560)

4. [LevelDB入门教程十篇](https://zhuanlan.zhihu.com/p/25349591)

5. [LevelDB github官网](https://github.com/google/leveldb)

6. [LevelDB官方文档](https://github.com/google/leveldb/blob/master/doc/index.md)

7. [LevelDb实现原理](https://blog.csdn.net/gdutliuyun827/article/details/70911342)

8. [leveldb相关](https://www.zhihu.com/topic/19819000/hot)

9. [深入理解什么是LSM-Tree](https://blog.csdn.net/u010454030/article/details/90414063)

10. [数据的存储结构浅析LSM-Tree和B-tree](https://zhuanlan.zhihu.com/p/145943958)

11. [LSM-Tree入门](https://segmentfault.com/a/1190000020550921)

12. [LSM-Tree论文](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.44.2782&rep=rep1&type=pdf)


13. [LSM-Tree中文介绍](https://blog.csdn.net/baichoufei90/article/details/84841289)

<br />
<br />
<br />

