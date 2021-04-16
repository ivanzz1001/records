---
layout: post
title: MySQL之redo log(转)
tags:
- database
categories: database
description:  MySQL复制原理详解
---


本文讲述一下MySQL的redo log，以大体了解其基本工作原理。




<!-- more -->

## 1. redo log是什么？
MySQL数据库作为现在互联网公司内最流行的关系型数据库，相信大家都在工作中使用过。InnoDB是MySQL里最为常用的一种存储引擎，主要面向在线事务处理(OLTP)的应用。今天就让我们来探究一下InnoDB是如何一步一步实现事务的，这次我们先讲事务实现的redo log。

首先我们先明确一下InnoDB的修改数据的基本流程，当我们想要修改DB上某一行数据的时候，InnoDB是把数据从磁盘读取到内存的缓冲池上进行修改。这个时候数据在内存中被修改，与磁盘中相比就存在了差异，我们称这种有差异的数据为脏页。InnoDB对脏页的处理不是每次生成脏页就将脏页刷新回磁盘，这样会产生海量的IO操作，严重影响InnoDB的处理性能。对于此，InnoDB有一套完善的处理策略，与我们这次主题关系不大，表过不提。既然脏页与磁盘中的数据存在差异，那么如果在这期间DB出现故障就会造成数据的丢失。为了解决这个问题，redo log就应运而生了。




## 2. redo log工作原理
在讲redo log工作原理之前，先来学习一下MySQL的一些基础：

### 2.1 日志类型

![db-translog4-1](https://ivanzz1001.github.io/records/assets/img/db/db_translog4_1.png)

redo log在数据库重启恢复的时候被使用，因为其属于物理日志的特性，恢复速度远快于逻辑日志。而我们经常使用的binlog就属于典型的逻辑日志。

### 2.2 checkpoing
坦白来讲checkpoint本身是比较复杂的，checkpoint所做的事就是把脏页给刷新回磁盘。所以，当DB重启恢复时，只需要恢复checkpoint之后的数据。这样就能大大缩短恢复时间。当然checkpoint还有其他的作用。

### 2.3 LSN(Log Sequence Number)
LSN实际上就是InnoDB使用的一个版本标记的计数，它是一个单调递增的值。数据页和redo log都有各自的LSN。我们可以根据数据页中的LSN值和redo log中的LSN值判断需要恢复的redo log的位置和大小。

### 2.4 工作原理
好的，现在我们来看看redo log的工作原理。说白了，redo log就是存储了数据修改后的值。当我们提交一个事务时，InnoDB会先去把要修改的数据写入日志，然后再去修改缓冲池里面的真正数据页。

我们着重看看redo log是怎么一步步写入磁盘的。redo log本身也是由两部分所构成，即重做日志缓冲(redo log buffer)和重做日志文件(redo log file)。这样的设计同样也是为了调和内存与磁盘的速度差异。InnoDB写入磁盘的策略可以通过innodb_flush_log_at_trx_commit这个参数来控制。

![db-translog4-2](https://ivanzz1001.github.io/records/assets/img/db/db_translog4_2.png)

* 当该值为1时，当然是最安全的，但是数据库性能会受到一定影响

* 当该值为0时，性能较好但是会丢失掉master thread还没刷新进磁盘部分的数据。

这里我想简单介绍一下master thread，这是InnoDB一个再后台运行的主线程，从名字就能看出这个线程相当的重要。它做的主要工作包括但不限于：刷新日志缓冲，合并插入缓冲，刷新脏页等。master thread大致分为每秒运行一次的操作和每10秒运行一次的操作。master thread中刷新数据，属于checkpoint的一种。所以如果在master thread在刷新日志的间隙，DB出现故障那么将丢失掉这部分数据。 


* 当该值为2时，当DB发生故障能恢复数据。但如果操作系统也出现宕机，那么就会丢失掉文件系统没有及时写入磁盘的数据。

这里说明一下，innodb_flush_log_at_trx_commit设为非0的值时，并不是说不会在master thread中刷新日志了。master thread刷新日志是在不断进行的，所以redo log写入磁盘是在持续的写入。


### 2.5 宕机恢复
DB宕机后重启，InnoDB会首先去查看数据页中的LSN的数值。这个值代表数据页被刷新回磁盘的LSN的大小。然后再去查看redo log的LSN的大小。如果数据页中的LSN值大说明数据页领先于redo log刷新回磁盘，不需要进行恢复。反之需要从redo log中恢复数据。

###### redo log的结构
其实这一部分内容日常工作中很少涉及到，稍微了解一下就够了。

1） **log block**

redo log的存储都是以块(block)为单位进行存储的，每个块的大小为512字节。同磁盘扇区大小一致，可以保证块的写入是原子操作。块又三部分所构成，分别是：

* 日志块头(log block header)

* 日志块尾(log block tailer)

* 日志本身

其中日志头占用12字节，日志尾占用8字节。故每个块实际存储日志的大小为492字节。

2） **log group**

一个日志文件由多个块所构成，多个日志文件形成一个重做日志文件组(redo log group)。不过，log group是一个逻辑上的概念，真实的磁盘上不会这样存储。




<br />
<br />
**[参看]**:

1. [MySQL之Redo Log](https://zhuanlan.zhihu.com/p/86555990)

2. [MySQL事务日志undo log和redo log分析](https://blog.csdn.net/lzw2016/article/details/89420391)

3. [说说MySQL中的Redo log Undo log都在干啥](https://www.cnblogs.com/xinysu/p/6555082.html)

<br />
<br />
<br />

