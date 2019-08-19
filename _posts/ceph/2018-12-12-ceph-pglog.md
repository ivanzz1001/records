---
layout: post
title: 基于pglog的Ceph一致性存储问题
tags:
- ceph
categories: ceph
description: pglog
---

分布式存储系统通常采用多副本的方式来保证系统的可靠性，而多副本之间如何保证数据的一致性就是系统的核心。Ceph号称统一存储，其核心RADOS既支持多副本，也支持纠删码。本文主要分析Ceph的多副本一致性协议。

<!-- more -->

## 1. pglog及读写流程

Ceph使用pglog来保证多副本之间的一致性，pglog的示意图如下：pglog主要用来记录做了什么操作，比如修改、删除等，而每一条记录里包含了对象信息，还有版本。

Ceph使用版本控制的方式来标记一个PG内的每一次更新，每个版本包括一个(epoch、version)来组成： 其中epoch时osdmap版本，每当OSD状态变化如增加、删除等时，epoch就递增； version是PG内每次更新操作的版本号，递增的，由PG内的Primary OSD进行分配。

![ceph-pglog](https://ivanzz1001.github.io/records/assets/img/ceph/pg/ceph_pglog.png)

每个副本上都维护了pglog，pglog里最重要的两个指针就是last_complete和last_update，正常情况下，每个副本上这两个指针都指向同一个位置，当出现机器重启、网络中断等故障时，故障副本的这两个指针就会有所区别，以便于记录副本间的差异。

为了便于说明Ceph的一致性协议，先简要描述一下Ceph的读写处理流程：

**写处理流程：**

1) client把写请求发到Primary OSD上，Primary OSD将写请求序列化到一个事务中（在内存里），然后构造一条pglog记录，有序列化到这个事务中，之后再将这个事务以DirectIO的方式异步写入journal，同时Primary OSD把写请求和pglog(pglog_entry是由primary生成的）发送到Replicas上。

2) 在Primary OSD将事务写到journal上后，会通过一些列的线程和回调处理，然后将这个事务里的数据写入filesystem(只写到文件系统的缓存里，会有线程定期刷数据），这个事务里的pglog记录（也包括pginfo的last_complete和last_update)会写到leveldb，还有一些扩展属性相关的也在这个事务里，在遍历这个事务时也会写到leveldb；

3) 在Replicas上，也进行类似于Primary的动作，先写Journal，写成功会给Primary发送一个committd ack，然后将这个事务里的数据写到filesystem， pglog与pginfo写到leveldb里，写完后会给Primary发送另外一个applied ack;

4) Primary在自己完成journal的写入时，以及在收到Replica的committed ack时都会检查是否多个副本都写入journal成功了，如果是则向client端发送ack通知写完成； primary在自己完成事务写到文件系统和leveldb后，以及在收到replica的applied ack时都会检查是否多个副本都写文件系统成功，如果是则向client端发送ack通知数据可读；

**读处理流程：**

对读流程来说，就比较简单，都是由Primary来处理，这里就不多说了。

## 2. 故障恢复

### 2.1 Primary故障恢复

![ceph-primary-restore](https://ivanzz1001.github.io/records/assets/img/ceph/pg/ceph_primay_restore.png)

### 2.2 Replica故障恢复

![ceph-replica-restore](https://ivanzz1001.github.io/records/assets/img/ceph/pg/ceph_replica_restore.png)


<br />
<br />

**[参看]**

1. [基于pglog的Ceph一致性存储问题](http://udn.yyuap.com/thread-103051-1-1.html)

<br />
<br />
<br />

