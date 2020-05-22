---
layout: post
title: ceph的数据读写(2)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


本章介绍ceph的服务端OSD（书中简称OSD模块或者OSD）的实现。其对应的源代码在src/osd目录下。OSD模块是Ceph服务进程的核心实现，它实现了服务端的核心功能。本章先介绍OSD模块静态类图相关数据结构，再着重介绍服务端数据的写入和读取流程。

<!-- more -->


## 1. 读写操作的序列图
写操作的序列图如下图6-2所示：

![ceph-chapter6-2](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter6_2.jpg)

写操作分为三个阶段：

* **阶段1**：从函数ms_fast_dispatch()到函数op_wq.queue函数为止，其处理过程都在网络模块的回调函数中处理，主要检查当前OSD的状态，以及epoch是否一致；

* **阶段2**： 这个阶段在工作队列op_wq中的线程池里处理，在类ReplicatedPG里，其完成对PG的状态、对象的状态的检查，并把请求封装成事务。

* **阶段3**： 本阶段也是在工作队列op_wq中的线程池里处理，主要功能都在类ReplicatedBackend中实现。核心工作就是把封装好的事务通过网络分发到从副本上，最后调用本地FileStore的函数完成本地对象的数据写入。

通过上面，其实我们可以这样理解OSD、ReplicatedPG以及ReplicatedBackend三者之间的关系： OSD是PG操作的主入口，而ReplicatedPG对应于一个特定的PG，其调用ReplicatedBackend来完成Object读写消息的发送及事件的处理。

>注： OSD进程的主入口为src/ceph_osd.cc

## 2. 读写流程代码分析
在介绍了上述的数据结构和基本的流程之后，下面将从服务端接收到消息开始，分三个阶段具体分析读写的过程。

### 2.1 阶段1： 接收请求
读写请求都是从OSD::ms_fast_dispatch()开始，它是接收读写消息message的入口。下面从这里开始读写操作的分析。本阶段所有的函数是被网络模块的接收线程调用，所以理论上应该尽可能的简单，处理完成后交给后面的OSD模块的op_wq工作队列来处理。

###### 1. ms_fast_dispatch
{% highlight string %}
void OSD::ms_fast_dispatch(Message *m)
{
	if (service.is_stopping()) {
		m->put();
		return;
	}

	OpRequestRef op = op_tracker.create_request<OpRequest, Message*>(m);
	{
		#ifdef WITH_LTTNG
			osd_reqid_t reqid = op->get_reqid();
		#endif
		tracepoint(osd, ms_fast_dispatch, reqid.name._type,
		reqid.name._num, reqid.tid, reqid.inc);
	}

	OSDMapRef nextmap = service.get_nextmap_reserved();
	Session *session = static_cast<Session*>(m->get_connection()->get_priv());
	if (session) {
		{
			Mutex::Locker l(session->session_dispatch_lock);
			update_waiting_for_pg(session, nextmap);
			session->waiting_on_map.push_back(op);
			dispatch_session_waiting(session, nextmap);
		}
		session->put();
	}

	service.release_map(nextmap);
}
{% endhighlight %};
函数ms_fast_dispatch()为OSD注册了网络模块的回调函数，其被网络的接收线程调用，具体实现过程如下：

1） 首先检查service，如果已经停止了，就直接返回；

2） 调用函数op_tracker.create_request()把Message消息转换成OpRequest类型，数据结构OpRequest包装了Message，并添加了一些其他信息。

3） 获取nextmap（也就是最新的osdmap)和session，类Session保存了一个Connection的相关信息。

4） 调用函数update_waiting_for_pg来更新session里保存的OSDMap信息；

5） 把请求加入waiting_on_map的列表里；

6） 调用函数dispatch_session_waiting()处理，它循环调用函数dispatch_op_fast处理请求；

7） 如果session->waiting_on_map不为空，说明该session里还有等待osdmap的请求，把该session加入到session_waiting_for_map队列里(该队列表示正在等待最新OSDMap的session，当成功获取到最新的OSDMap，那么OSD的相关线程会将该session里的消息马上发送出去)。

###### 2. dispatch_op_fast
{% highlight string %}
bool OSD::dispatch_op_fast(OpRequestRef& op, OSDMapRef& osdmap);
{% endhighlight %}

该函数检查OSD目前的epoch是否最新：

1） 检查变量is_stopping，如果为true，说明当前OSD正处于停止过程中，返回true，这样就会直接删除掉该请求。

2） 调用函数op_required_epoch(op)，从OpRequest中获取msg带的epoch，进行比较，如果该值大于OSD最新的epoch，则调用函数osdmap_subscribe()订阅新的OSDMap更新，返回false值，表明不应该把此请求从session->waiting_on_map里移除。

3） 否则，根据消息类型，调用相应的消息处理函数。本章只关注处理函数handle_op相关的流程。

###### 3. handle_op
{% highlight string %}
void OSD::handle_op(OpRequestRef& op, OSDMapRef& osdmap);
{% endhighlight %}
该函数处理OSD相关的操作，其处理流程如下：

1) 首先调用op_is_discardable，检查该op是否可以丢弃；

2） 构建share_map结构体，获取client_session，从client_session获取last_sent_epoch，调用函数service.should_share_map()来设置share_map.should_send标志，该函数用于检查是否需要通知对方更新epoch值。这里和dispatch_op_fast的处理区别是：上次是更新自己，这里是通知对方更新。需要注意的是，client和OSD的epoch不一致，并不影响读写，只要epoch的变化不影响本次读写PG的OSD list变化。

3） 从消息里获取_pgid，再从_pgid里获取pool
{% highlight string %}
// calc actual pgid
pg_t _pgid = m->get_pg();
int64_t pool = _pgid.pool();
{% endhighlight %}

4) 调用函数osdmap->raw_pg_to_pg，最终调用pg_pool_t::raw_pg_to_pg()函数，对PG做了调整
{% highlight string %}
//参看： src/osd/OSDMap.h
pg_t OSDMap::raw_pg_to_pg(pg_t pg) const {
	map<int64_t,pg_pool_t>::const_iterator p = pools.find(pg.pool());
	assert(p != pools.end());
	return p->second.raw_pg_to_pg(pg);
}

/*
 * map a raw pg (with full precision ps) into an actual pg, for storage
 * 参看： src/osd/Osd_types.cc
 */
pg_t pg_pool_t::raw_pg_to_pg(pg_t pg) const
{
  pg.set_ps(ceph_stable_mod(pg.ps(), pg_num, pg_num_mask));
  return pg;
}
{% endhighlight %}
至于为什么要调用raw_pg_to_pg()，上面代码的注释也比较详细。

5) 调用osdmap->get_primary_shard()函数，获取该PG的主OSD

6） 调用函数get_pg_or_queue_for_pg()，通过pgid获取PG类的指针。如果获取成功，就调用函数enqueue_op处理请求；

7） 如果PG类的指针没有获取成功，做一些错误检查：

&emsp; a) send_map为空，client需要重试；

&emsp; b) 客户端的osdmap里没有当前的pool;

&emsp; c) 当前OSD的osdmap没有该pool，或者当前OSD不是该PG的主OSD


总结，这个函数主要检查了消息的源端epoch是否需要share，最主要的是获取读写请求相关的PG类后，下面就进入PG类的处理。


###### 4. queue_op
{% highlight string %}
void PG::queue_op(OpRequestRef& op)
{
	Mutex::Locker l(map_lock);

	if (!waiting_for_map.empty()) {
		// preserve ordering
		waiting_for_map.push_back(op);
		op->mark_delayed("waiting_for_map not empty");
		return;
	}

	if (op_must_wait_for_map(get_osdmap_with_maplock()->get_epoch(), op)) {
		waiting_for_map.push_back(op);
		op->mark_delayed("op must wait for map");
		return;
	}

	op->mark_queued_for_pg();
	osd->op_wq.queue(make_pair(PGRef(this), op));
	{
		// after queue() to include any locking costs
		#ifdef WITH_LTTNG
			osd_reqid_t reqid = op->get_reqid();
		#endif
		tracepoint(pg, queue_op, reqid.name._type,
		reqid.name._num, reqid.tid, reqid.inc, op->rmw_flags);
	}
}
{% endhighlight %}
该函数实现如下：

1） 加map_lock锁，该锁保护waiting_for_map列表，判断waiting_for_map列表不为空，就把当前Op加入该列表，直接返回。waiting_for_map列表不为空，说明有操作在等待osdmap的更新，说明当前osdmap不信任，不能继续当前的处理。

2） 函数op_must_wait_for_map()判断当前的epoch是否大于Op的epoch，如果是，则必须加入waiting_for_map等待，等待更新PG当前的epoch值。

> 提示： 这里的osdmap的epoch的判断，是一个PG层的epoch的判断。和前面的判断不在一个层次，这里是需要等待的。

3） 最终，把请求加入OSD的op_wq处理队列里

总结，这个函数做在PG类里，做PG层面的相关检查，如果ok，就加入OSD的op_wq工作队列里继续处理。

### 2.2 阶段2： OSD的op_wq处理
op_wq是一个ShardedWQ类型的工作队列。以下操作都是在op_wq对应的线程池里调用做相应的处理。这里着重分析读写流程。

###### 1. dequeue_op
{% highlight string %}
/*
 * NOTE: dequeue called in worker thread, with pg lock
 */
void OSD::dequeue_op(
  PGRef pg, OpRequestRef op,
  ThreadPool::TPHandle &handle);
{% endhighlight %}
1) 检查如果op->send_map_update为true，也就是如果需要更新osdmap，就调用函数service.share_map()更新源端的osdmap信息。在函数OSD::handle_op()里，只在op->send_map_update里设置了是否需要share_map标记，这里才真正去发消息实现share操作。

2） 检查如果pg正在删除，就把本请求丢弃，直接返回；

3） 调用函数pg->do_request(op, handle)处理请求。

总之，本函数主要实现了使请求源端更新osdmap的操作，接下来在PG里调用do_request()来处理。

###### 2. do_request()
本函数进入ReplicatedPG类来处理：
{% highlight string %}
void ReplicatedPG::do_request(OpRequestRef& op, ThreadPool::TPHandle &handle);
{% endhighlight %}
处理过程如下：

1） 调用函数can_discard_request()检查op是否可以直接丢弃掉；

2） 检查变量flushes_in_progress，如果还有flush操作，把op加入waiting_for_peered队列里，直接返回；

3） 如果PG还没有peered,调用函数can_handle_while_inactive()检查pgbackend能否处理该请求，如果可以，就调用pgbackend->handle_message()处理；否则加入waiting_for_peered队列，等待PG完成peering后再处理；

>注意： PG处于inactive状态，pgbackend只能处理MSG_OSD_PG_PULL类型的消息。这种情况可能是： 本OSD可能已经不在该PG的acting osd列表中，但是可能在上一阶段该PG的OSD列表中，所以PG可能含有有效的对象数据，这些对象数据可以被该PG当前的主OSD拉取以修复当前PG的数据。

4） 此时PG处于peered并且flushes_in_progress未0的状态下，检查pgbackend能否处理该请求。pgbackend可以处理数据恢复过程中的PULL和PUSH请求，以及主副本发的从副本的更新相关SUBOP类型的请求。

5） 如果是CEPH_MSG_OSD_OP，检查该PG的状态，如果处于非active状态或者处于replay状态，则把请求添加到waiting_for_active等待队列；

6） 检查如果该pool是cache pool，而该操作没有带CEPH_FEATURE_OSD_CACHEPOOL标志，返回EOPNOTSUPP错误码；

7) 根据消息的类型，调用相应的处理函数来处理；

本函数开始进入ReplicatedPG层面来处理，主要检查当前PG的状态是否正常，是否可以处理请求。


###### 3. do_op()
函数do_op()比较复杂，处理读写请求的状态检查和一些上下文的准备工作。其中大量的关于快照的处理，本章在遇到快照处理时只简单介绍一下。
{% highlight string %}
void ReplicatedPG::do_op(OpRequestRef& op);
{% endhighlight %}

具体处理过程如下：

1） 调用函数m->finish_decode()，把消息带的数据从bufferlist中解析出相关的字段；

2） 调用osd->osd->init_op_flags()初始化op->rmw_flags，函数init_op_flags()根据flag来设置rmw_flags标志；

3） 如果是读操作：

&emsp; a) 如果消息里带有CEPH_OSD_FLAG_BALANCE_READS（平衡读）或者CEPH_OSD_FLAG_LOCALIZE_READS(本地读）标志，表明主副本都允许读。检查本OSD必须是该PG的primary或者replica之一；

&emsp; b) 如果没有上述标志，读操作只能读取主副本，本OSD必须是该PG的主OSD

4） 调用函数op_has_sufficient_caps()检查是否有相关的操作权限

5） 如果里面含有includes_pg_op操作，调用pg_op_must_wait()检查该操作是否需要等待。如果需要等待，加入waiting_for_all_missing队列；如果不需要等待，调用do_pg_op()处理PG相关的操作。这里的PG操作，都是CEPH_OSD_OP_PGLS等类似的PG相关的操作，需要确保该PG上没有需要修复的对象，否则ls列出的对象就不准确。

6） 检查对象的名字是否超长
{% highlight string %}
// object name too long?
if (m->get_oid().name.size() > g_conf->osd_max_object_name_len) {
	dout(4) << "do_op name is longer than "
		<< g_conf->osd_max_object_name_len
		<< " bytes" << dendl;

	osd->reply_op_error(op, -ENAMETOOLONG);
	return;
}
if (m->get_object_locator().key.size() > g_conf->osd_max_object_name_len) {
	dout(4) << "do_op locator is longer than "
		<< g_conf->osd_max_object_name_len
		<< " bytes" << dendl;
	osd->reply_op_error(op, -ENAMETOOLONG);
	return;
}
if (m->get_object_locator().nspace.size() > g_conf->osd_max_object_namespace_len) {
	dout(4) << "do_op namespace is longer than "
		<< g_conf->osd_max_object_namespace_len
		<< " bytes" << dendl;
	osd->reply_op_error(op, -ENAMETOOLONG);
	return;
}

if (int r = osd->store->validate_hobject_key(head)) {
	dout(4) << "do_op object " << head << " invalid for backing store: "
		<< r << dendl;
	osd->reply_op_error(op, r);
	return;
}
{% endhighlight %}

7) 检查操作的客户端是否在黑名单(blacklist)中

8) 检查磁盘空间是否满
{% highlight string %}
op->may_write();
{% endhighlight %}

9) 检查如果是写操作，并且是snap，返回EINVAL，快照不允许写操作。如果写操作带的数据大于osd_max_write_size(如果设置了），直接返回OSD_WRITETOOBIG错误。
{% highlight string %}
// invalid?
if (m->get_snapid() != CEPH_NOSNAP) {
	osd->reply_op_error(op, -EINVAL);
	return;
}

// too big?
if (cct->_conf->osd_max_write_size && m->get_data_len() > cct->_conf->osd_max_write_size << 20) {
	// journal can't hold commit!
	derr << "do_op msg data len " << m->get_data_len()
	<< " > osd_max_write_size " << (cct->_conf->osd_max_write_size << 20)
	<< " on " << *m << dendl;
	osd->reply_op_error(op, -OSD_WRITETOOBIG);
	return;
}
{% endhighlight %}







<br />
<br />

**[参看]**





<br />
<br />
<br />

