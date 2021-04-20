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
Ceph在进行故障恢复的时候会经过Peering的过程。简要来说，peering就是比对各个副本上的pglog，然后根据副本上pglog的差异来构造missing列表，然后在恢复阶段就可以根据missing列表来进行恢复了。peering是按照pg为单位进行的，在进行Peering的过程中，IO请求是会挂起的；当进行完peering阶段进入recovery阶段时，IO可以继续进行。不过当IO请求命中了missing列表的时候，对应的这个待恢复对象会优先进行恢复，当这个对象恢复完成后，再进行IO处理。

因为pglog记录数有限制，当比对各个副本上的pglog时，发现故障的副本已经落后太多，这样就无法根据pglog来恢复了，所以这种情况就只能全量恢复，称为backfill。坏盘、坏机器或者集群扩容时也会触发backfill，这里不做介绍，后续单独一篇文章来进行分析。

基于pglog的一致性协议包含两种恢复过程：一个是primary挂掉后又起来的恢复； 一种是Replica挂掉后又起来的恢复。

### 2.1 Primary故障恢复

![ceph-primary-restore](https://ivanzz1001.github.io/records/assets/img/ceph/pg/ceph_primay_restore.png)

简单起见，图中的数字就表示pglog里不同对象的版本。

1） 正常情况下，都是由Primary处理client端IO请求，这时Primary和Replicas上的last_update和last_complete都会指向pglog最新记录；

2) 当Primary挂掉后，会选出一个Replica作为“临时主”，这个“临时主”负责处理新的读写请求，并且这个时候“临时主”和剩下的Replicas上的last_complete和last_update都更新到该副本上的pglog的最新记录；

3） 当原来的Primary又重启时，会从本地读出pginfo和pglog，当发现last_complete因此将该对象加到missing列表里；

4) Primary发起Peering过程，即“抢回原来的主”，选出权威日志，一般就是“临时主”的pglog，将该权威日志获取过来，与自己的pglog进行merge_log的步骤，构建出missing列表，并且更新自己的last_update为最新的pglog记录（与各个副本一致），这个时候last_complete与last_update之间就会加到missing列表，并且peering完成后会持久化last_complete和last_update；

5) 当有新的写入时，仍然是由Primary负责处理，会更新last_update，副本上会同时更新last_complete，与此同时，Primary会进行恢复，就是从其他副本上拉取对象数据到自己这里进行恢复，每当恢复完一个时，就会更新自己的last_complete(会持久化的），当所有对象都恢复完成后，last_complete就会追上last_update了。

6) 当恢复过程中，Primary又挂了再起来恢复时，先读出本地pglog时就会根据自己的last_complete和last_update构建出missing列表，而在peering的时候比对权威日志和本地的pglog发现权威与自己的last_update都一样，peering的过程中就没有新的对象加到missing列表里。总的来说，missing列表就是由两个地方进行构建的： 一个是osd启动的时候read_log里构建的；另一个是peering的时候比对权威日志构建的。


### 2.2 Replica故障恢复

![ceph-replica-restore](https://ivanzz1001.github.io/records/assets/img/ceph/pg/ceph_replica_restore.png)

与Primary的恢复类似，peering都是由Primary发起的，Replica起来后也会根据pglog的last_complete和last_update构建出replica自己的missing，然后Primary进行peering的时候比对权威日志（即自身）与故障replica的日志，结合replica的missing，构建出peer_missing，然后就遍历peer_missing来恢复对象。然后新的写入时会在各个副本上更新last_complete和last_update，其中故障replica上只更新last_update。恢复过程中，每恢复完一个对象，故障replica会更新last_complete，这样所有对象都恢复完成后，replica的last_complete就会追上last_update。

如果恢复过程中，故障replica又挂掉，然后重启后进行恢复的时候，也是先是读出本地log，对比last_complete与last_update之间的pglog记录里的对象版本与本地读出来的该对象版本，如果本地不是最新的，就会加到missing列表里，然后Primary发起peering的时候发现replica的last_update是最新的，peering过程就没有新的对象加到peering_missing列表里，peer_missing里就是replica自己的missing里的对象。




<br />
<br />

**[参看]**

1. [基于pglog的Ceph一致性存储问题](http://udn.yyuap.com/thread-103051-1-1.html)

2. [ceph RGW类流程图](https://blog.csdn.net/ysr1990813/article/details/84142039)

3. [分布式存储Ceph之PG状态详解](https://blog.didiyun.com/index.php/2018/11/08/didiyun-ceph-pg/)

<br />
<br />
<br />

