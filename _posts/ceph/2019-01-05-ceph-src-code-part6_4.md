---
layout: post
title: ceph解读之PGLog(转)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


ceph的PGLog是由PG来维护，记录了该PG的所有操作。其作用类似于数据库里的undo log。PGLog通常只保存近千条的操作记录（默认是3000条），但是当PG处于降级状态时，就会保存更多的日志（默认时10000条），这样就可以在故障的PG重新上线后用来恢复PG的数据。本文主要从PGLog的格式、存储方式、如何参与恢复来解析PGLLog。





<!-- more -->

## 1. PGLog的格式

ceph使用版本控制的方式来标记一个PG内的每一次更新，每个版本包括一个(epoch, version)来组成。其中epoch是osdmap的版本，每当有OSD状态变化如增加、删除等时，epoch就递增；version是PG内每次更新操作的版本号，递增的，由PG内的Primary OSD进行分配的。

PGLog在代码实现中有3个主要的数据结构来维护：pg_info_t、pg_log_t、pg_log_entry_t。三者的关系示意图如下。从结构上可以得知，PGLog里只有对象更新操作相关的内容，没有具体的数据以及偏移大小等，所以后续以PGLog来进行恢复时都是按照整个对象来进行恢复的（默认对象大小是4MB)。

![ceph-chapter64-1](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter64_1.png)


其中：

* last_complete： 在该指针之前的版本都已经在所有OSD上完成更新（只表示内存更新完成）；

* last_update: PG内最近一次更新的对象版本，还没有在所有OSD上完成更新，在last_update和last_complete之间的操作表示该操作已在部分OSD上完成但是还没有全部完成；

* log_tail: 指向pg log最老的那条记录

* head: 最新的pg log记录

* tail: 指向最老的pg log记录的前一个；

* log: 存放实际的pglog记录的list;

## 2. PGLog的存储方式

了解了PGLog的格式之后，我们就来分析一下PGLog的存储方式。在ceph的实现里，对于写IO的处理，都是先封装成一个transaction，然后将这个transaction写到journal里，在journal写完成后，触发回调流程，经过多个线程及回调的处理后再进行写数据到buffer cache的操作，从而完成整个写journal和写本地缓存的流程（具体的流程在《OSD读写处理流程》一文中有详细描述）。


总体来说，PGLog也是封装到transaction中，在写journal的时候一起写到日志盘上，最后在写本地缓存的时候遍历transaction里的内容，将PGLog相关的东西写到leveldb里，从而完成该OSD上PGLog的更新操作。

### 2.1 PGLog更新到journal

###### 2.1.1 写I/O序列化到transaction

在《OSD读写流程》里描述了主OSD上的读写处理流程，这里就不做说明。在ReplicatedPG::do_osd_ops()函数里根据类型```CEPH_OSD_OP_WRITE```就会进行封装写IO到transaction的操作（即将要写的数据encode到ObjectStore::Transaction::tbl里，这是个bufferlist，encode时都先将op编码进去，这样后续在处理时就可以根据op来操作。注意这里的encode其实就是序列化操作）。

这个transaction经过的过程如下：
{% highlight string %}
int ReplicatedPG::prepare_transaction(OpContext *ctx)
{
	...

	int result = do_osd_ops(ctx, ctx->ops);
}
int ReplicatedPG::do_osd_ops(OpContext *ctx, vector<OSDOp>& ops)
{
	...
	PGBackend::PGTransaction* t = ctx->op_t.get();

	...

	switch (op.op) {
		...
		
		case CEPH_OSD_OP_WRITE:
			...
			if (pool.info.require_rollback()) {
				t->append(soid, op.extent.offset, op.extent.length, osd_op.indata, op.flags);
			} else {
				t->write(soid, op.extent.offset, op.extent.length, osd_op.indata, op.flags);
			}
	}
}
{% endhighlight %}
ReplicatedPG::OpContext::op_t  –>  PGBackend::PGTransaction::write（即t->write） –>  RPGTransaction::write  –> ObjectStore::Transaction::write（encode到ObjectStore::Transaction::tbl). 
>  
后面调用ReplicatedBackend::submit_transaction()时传入的```PGTransaction *_t```就是上面这个，通过转换成```RPGTransaction *t```，然后这个函数里用到的```ObjectStore::Transaction *op_t```就是对应到RPGTransaction里的```ObjectStore::Transaction *t```。



###### 2.1.2 PGLog序列化到transaction

* 在ReplicatedPG::prepare_transaction()里调用Replicated::finish_ctx()，然后在finish_ctx()函数里就会调用ctx->log.push_back()就会构造pg_log_entry_t插入到vector log里；

* 在ReplicatedBackend::submit_transaction()里调用parent->log_operation()将PGLog序列化到transaction里。在PG::append_log()里将PGLog相关信息序列化到transaction里。


* 主要序列化到transaction里的内容包括：pg_into_t、pg_log_entry_t，这两种数据结构都是以map的形式encode到transaction的bufferlist里。其中不同的map的value对应的就是pg_info和pg_log_entry的bufferlist。而map的key就是epoch+version构成的字符串“epoch.version”。另外需要注意的是那些map是附带上op和oid作为对象的omap(Object的属性会利用文件的xattr属性存取，因为有些文件系统对xattr的长度有限制，因此超出长度的metadata会被存储在DBObjectMap里。而Object的omap则直接利用DBObjectMap实现）来序列化到transaction里。


###### 2.1.3 Transaction里的内容






<br />
<br />

**[参看]**

1. [ceph解读之PGLog](http://sysnote.github.io/2015/12/18/ceph-pglog/)


<br />
<br />
<br />

