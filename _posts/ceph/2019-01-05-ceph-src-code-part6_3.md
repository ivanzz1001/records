---
layout: post
title: ceph的数据读写(3)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本文会从更高一点的层次来介绍ceph数据的读写流程，并使用流程图的方式展示，便于阅读与理解。

<!-- more -->

## 1. OSD数据读写相关数据结构

![ceph-chapter63-1](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter63_1.jpg)

## 2. 主OSD处理流程
### 2.1 主OSD读写流程
本文基于Jewel版本对OSD读写流程进行分析，如下图所示：


![ceph-chapter63-2](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter63_2.jpg)

具体的处理流程从图中就可以体现出来，这里就不作过多的描述，需要提的几点：

1）读请求由Primary OSD来进行处理；

2）对于写请求，Primary OSD先发消息到副本OSD，然后记录PGLog(在log_operation里只是构造transaction，真正写到磁盘是和journal一起写的），然后再生成本地事务进行本地写的处理；

3）写请求涉及到两步操作，一个是写journal，一个是写本地缓存(page cache)。对于每一个副本都有这两步操作，每个副本都是先写journal，然后再写本地缓存。如果是3副本，就涉及到6次写操作；

4）Primary OSD创建了2个回调函数来处理写journal和写到缓存(分别是C_OSD_RepopCommit和C_OSD_RepopApplied），主副本的写和从副本的写没有先后顺序，有可能主的journal先写完，也有可能从的journal先写完，ceph不管这个顺序，只要保证3副本都写完了之后才返回客户端响应（degrade情况下例外），3个副本的journal写完成(all_commit)，会返回客户端“写操作完成”，而3个副本都写本地缓存完成后（all_applied)，才返回客户端“数据可读”；


### 2.2 主OSD的写操作事务的处理
![ceph-chapter63-3](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter63_3.jpg)

下面我们对上图进行简要说明：

1）OSD中写操作的处理中涉及到很多回调函数（这也是ceph本身的一个特点），这些回调函数追溯到Context类，这个类是回调函数类的抽象基类，继承它的类只需要在子类中实现它的finish()函数，然后在finish函数中调用字注册的回调函数，就能完成回调；

2） 上面说到的调用Context类的finish()函数就能进行回调，那么具体调用它的地方就在Finisher类的finisher_thread_entry()里，这个是finisher的线程处理函数，并且finisher还有一个finisher_queue，实现生产者消费者模型，生产者往finisher_queue里放东西，并通过条件变量通知finisher的线程处理函数来进行处理，在这个线程处理函数里就能通过调用Context类的complete()函数，然后调用其子类实现的finish()函数，从而完成回调的处理；

3）OSD的写操作先放到writeq里，通知FileJournal的线程处理函数进行journal的处理，然后再放入到journal的finisher_queue里，然后对应的finisher线程处理函数调用之前注册的回调函数(C_JournaledAhead对应的函调函数是FileStore::_journaled_ahead)进行处理，这里就分成两条线路进行下去：

* 放入ondisk_finisher，进而触发ondisk_finisher的线程处理函数进行处理，就会调用到C_OSD_OnOpCommit的回调函数ReplicatedBackend::op_commit()，在这里会检查waiting_for_commit是否为空（如果是3副本，这里面就有3项，每完成一个副本的写journal操作，就会从waiting_for_commit里删除一个），如果为空，才调用C_OSD_RepopCommit的回调函数repop_all_committed()，从而调用ReplicatedPG::eval_repop()发给客户端写操作完成；

* 放入op_wq里，FileStore::op_tp线程池就会从op_wq中取出进行操作，在FileStore::OpWQ::_process()去进行写到本地缓存的操作（FileStore::_write())，并且在完成后FileStore::OpWQ::_process_finish()中处理，类似的也是放到一个finisher_queue里(op_finisher)，然后finisher的线程调用C_OSD_OnOpApplied的回调函数（ReplicatedBackend::op_applied）来处理。如果waiting_for_applied为空（如果是3副本，这里面就有3项，每完成一个副本的写本地操作，就会从waiting_for_applied里删除一个），才调用C_OSD_RepopApplied的回调函数repop_all_applied()，进而调用ReplicatedPG::eval_repop()发给客户端数据可读；


4）在FileStore::queue_transactions()中调用submit_manager来进行提交；在FileStore::_do_op()中调用apply_manager来进行应用，如下图所示：

![ceph-chapter63-5](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter63_5.jpg)

5）数据写入过程中的数据回调函数如下

![ceph-chapter63-6](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter63_6.jpg)


### 2.3 副本OSD的消息处理及主OSD的响应处理

![ceph-chapter63-4](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter63_4.jpg)

下面我们对上图进行简要说明：

1） 副本OSD收到主OSD的写请求时，也是按照消息处理的流程从队列里取出来进行处理，先写PGLog，然后注册两个回调函数，之后进行写journal和写本地缓存的操作，在处理完成后发送响应给主OSD(sub_op_modify_commit()和sub_op_modify_applied())，具体流程可参看上一节；

2） 主OSD收到副本OSD的响应后，从in_progress_ops中找到op，op里保存有注册的回调函数，判断flag如果是CEPH_OSD_FLAG_ONDISK，则从waiting_for_commit中删除，否则从waiting_for_applied中删除；然后根据waiting_for_commit、waiting_for_applied是否为空，来调用对应的回调函数，在完成的情况下发送消息给客户端；




<br />
<br />

**[参看]**

1. [ceph OSD读写流程](http://sysnote.github.io/2015/11/25/ceph-osd-rw1/)




<br />
<br />
<br />

