---
layout: post
title: ceph peering机制再研究(1)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本节我们从基础出发，来研究ceph peering这一复杂的过程，期望对其工作原理有更深入的理解。


<!-- more -->

## 1. OSD中网络消息的处理

在前面的ceph网络通信章节中，我们介绍了SimpleMessenger网络通信框架。这里OSD实现了Dispatcher接口：
{% highlight string %}
class OSD : public Dispatcher,
	    public md_config_obs_t {
};
{% endhighlight %}
下面我们来看其对Dispatcher接口各函数的具体实现。

1) **ms_can_fast_dispatch()的实现**
{% highlight string %}
bool ms_can_fast_dispatch(Message *m) const {
	switch (m->get_type()) {
		case CEPH_MSG_OSD_OP:
		case MSG_OSD_SUBOP:
		case MSG_OSD_REPOP:
		case MSG_OSD_SUBOPREPLY:
		case MSG_OSD_REPOPREPLY:
		case MSG_OSD_PG_PUSH:
		case MSG_OSD_PG_PULL:
		case MSG_OSD_PG_PUSH_REPLY:
		case MSG_OSD_PG_SCAN:
		case MSG_OSD_PG_BACKFILL:
		case MSG_OSD_EC_WRITE:
		case MSG_OSD_EC_WRITE_REPLY:
		case MSG_OSD_EC_READ:
		case MSG_OSD_EC_READ_REPLY:
		case MSG_OSD_REP_SCRUB:
		case MSG_OSD_PG_UPDATE_LOG_MISSING:
		case MSG_OSD_PG_UPDATE_LOG_MISSING_REPLY:
			return true;
		default:
			return false;
	}
}
{% endhighlight %}
从上面我们看到，对于case中所列举的消息，是可以进行fast_dispatch()进行处理的。

2) **ms_can_fast_dispatch_any()的实现**
{% highlight string %}
bool ms_can_fast_dispatch_any() const { return true; }
{% endhighlight %}
上面说明，会将OSD这个Dispatcher加入到Messenger的fast_dispatchers列表中的。

3) **ms_fast_dispatch()的实现**
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

{% endhighlight %}
从上面可以看出，其会调用dispatch_session_waiting()来进行处理：
{% highlight string %}
void OSD::dispatch_session_waiting(Session *session, OSDMapRef osdmap)
{
	assert(session->session_dispatch_lock.is_locked());
	assert(session->osdmap == osdmap);
	for (list<OpRequestRef>::iterator i = session->waiting_on_map.begin();
	    i != session->waiting_on_map.end() && dispatch_op_fast(*i, osdmap);
	    session->waiting_on_map.erase(i++));
	
	if (session->waiting_on_map.empty()) {
		clear_session_waiting_on_map(session);
	} else {
		register_session_waiting_on_map(session);
	}
	session->maybe_reset_osdmap();
}

bool OSD::dispatch_op_fast(OpRequestRef& op, OSDMapRef& osdmap)
{
	...
}
{% endhighlight %}
在dispatch_op_fast()函数中就会对ms_can_fast_dispatch()所指定的消息进行处理。

4) **ms_fast_preprocess()的实现**
{% highlight string %}
void OSD::ms_fast_preprocess(Message *m)
{
	if (m->get_connection()->get_peer_type() == CEPH_ENTITY_TYPE_OSD) {
		if (m->get_type() == CEPH_MSG_OSD_MAP) {
			MOSDMap *mm = static_cast<MOSDMap*>(m);
			Session *s = static_cast<Session*>(m->get_connection()->get_priv());
			if (s) {
				s->received_map_lock.lock();
				s->received_map_epoch = mm->get_last();
				s->received_map_lock.unlock();
				s->put();
			}
		}
	}
}
{% endhighlight %}
从上面的代码我们看到，对于来自于其他OSD发送过来的CEPH_MSG_OSD_MAP消息，则将session.received_map_epoch设置为收到的OSDMap中的最新版本。


5) **ms_dispatch()的实现**
{% highlight string %}
bool OSD::ms_dispatch(Message *m)
{
	if (m->get_type() == MSG_OSD_MARK_ME_DOWN) {
		service.got_stop_ack();
		m->put();
		return true;
	}
	
	// lock!
	
	osd_lock.Lock();
	if (is_stopping()) {
		osd_lock.Unlock();
		m->put();
		return true;
	}
	
	while (dispatch_running) {
		dout(10) << "ms_dispatch waiting for other dispatch thread to complete" << dendl;
		dispatch_cond.Wait(osd_lock);
	}
	dispatch_running = true;
	
	do_waiters();
	_dispatch(m);
	do_waiters();
	
	dispatch_running = false;
	dispatch_cond.Signal();
	
	osd_lock.Unlock();
	
	return true;
}
{% endhighlight %}
ms_dispatch()函数实现对普通消息的分发处理。现在我们来看一下_dispatch()函数：
{% highlight string %}
void OSD::_dispatch(Message *m)
{
	assert(osd_lock.is_locked());
	dout(20) << "_dispatch " << m << " " << *m << dendl;
	
	logger->set(l_osd_buf, buffer::get_total_alloc());
	logger->set(l_osd_history_alloc_bytes, SHIFT_ROUND_UP(buffer::get_history_alloc_bytes(), 20));
	logger->set(l_osd_history_alloc_num, buffer::get_history_alloc_num());
	logger->set(l_osd_cached_crc, buffer::get_cached_crc());
	logger->set(l_osd_cached_crc_adjusted, buffer::get_cached_crc_adjusted());
	
	switch (m->get_type()) {
	
		// -- don't need lock --
		case CEPH_MSG_PING:
			dout(10) << "ping from " << m->get_source() << dendl;
			m->put();
			break;
		
		// -- don't need OSDMap --
		
		// map and replication
		case CEPH_MSG_OSD_MAP:
			handle_osd_map(static_cast<MOSDMap*>(m));
			break;
		
		// osd
		case MSG_PGSTATSACK:
			handle_pg_stats_ack(static_cast<MPGStatsAck*>(m));
			break;
		
		case MSG_MON_COMMAND:
			handle_command(static_cast<MMonCommand*>(m));
			break;
		case MSG_COMMAND:
			handle_command(static_cast<MCommand*>(m));
			break;
		
		case MSG_OSD_SCRUB:
			handle_scrub(static_cast<MOSDScrub*>(m));
			break;
		
		// -- need OSDMap --
		
		default:
		{
			OpRequestRef op = op_tracker.create_request<OpRequest, Message*>(m);
			// no map?  starting up?
			if (!osdmap) {
				dout(7) << "no OSDMap, not booted" << dendl;
				logger->inc(l_osd_waiting_for_map);
				waiting_for_osdmap.push_back(op);
				op->mark_delayed("no osdmap");
				break;
			}
		
			// need OSDMap
			dispatch_op(op);
		}
	}
	
	logger->set(l_osd_buf, buffer::get_total_alloc());
	logger->set(l_osd_history_alloc_bytes, SHIFT_ROUND_UP(buffer::get_history_alloc_bytes(), 20));
	logger->set(l_osd_history_alloc_num, buffer::get_history_alloc_num());

}

void OSD::dispatch_op(OpRequestRef op)
{
	switch (op->get_req()->get_type()) {
	
		case MSG_OSD_PG_CREATE:
			handle_pg_create(op);
			break;
		case MSG_OSD_PG_NOTIFY:
			handle_pg_notify(op);
			break;
		case MSG_OSD_PG_QUERY:
			handle_pg_query(op);
			break;
		case MSG_OSD_PG_LOG:
			handle_pg_log(op);
			break;
		case MSG_OSD_PG_REMOVE:
			handle_pg_remove(op);
			break;
		case MSG_OSD_PG_INFO:
			handle_pg_info(op);
			break;
		case MSG_OSD_PG_TRIM:
			handle_pg_trim(op);
			break;
		case MSG_OSD_PG_MISSING:
			assert(0 =="received MOSDPGMissing; this message is supposed to be unused!?!");
			break;
		
		case MSG_OSD_BACKFILL_RESERVE:
			handle_pg_backfill_reserve(op);
			break;
		case MSG_OSD_RECOVERY_RESERVE:
			handle_pg_recovery_reserve(op);
			break;
	}
}
{% endhighlight %}
上面我们看到了最重要的对CEPH_MSG_OSD_MAP消息的处理。

6) **ms_handle_connect()的实现**
{% highlight string %}
void OSD::ms_handle_connect(Connection *con)
{
	if (con->get_peer_type() == CEPH_ENTITY_TYPE_MON) {
		Mutex::Locker l(osd_lock);
		if (is_stopping())
			return;

		dout(10) << "ms_handle_connect on mon" << dendl;
	
		if (is_preboot()) {
			start_boot();
		} else if (is_booting()) {
			_send_boot();       // resend boot message
		} else {
			map_lock.get_read();
			Mutex::Locker l2(mon_report_lock);
	
			utime_t now = ceph_clock_now(NULL);
			last_mon_report = now;
	
			// resend everything, it's a new session
			send_alive();
			service.requeue_pg_temp();
			service.send_pg_temp();
			requeue_failures();
			send_failures();
			send_pg_stats(now);
	
			map_lock.put_read();
		}
	
		// full map requests may happen while active or pre-boot
		if (requested_full_first) {
			rerequest_full_maps();
		}
	}
}
{% endhighlight %}
OSD只会主动向Monitor以及其他OSD发起连接。这里处理向其他Monitor发起连接的请求回调。

7) **ms_handle_fast_connect()的实现**
{% highlight string %}
void OSD::ms_handle_fast_connect(Connection *con)
{
	if (con->get_peer_type() != CEPH_ENTITY_TYPE_MON) {
		Session *s = static_cast<Session*>(con->get_priv());
		if (!s) {
			s = new Session(cct);
			con->set_priv(s->get());
			s->con = con;
			dout(10) << " new session (outgoing) " << s << " con=" << s->con
				<< " addr=" << s->con->get_peer_addr() << dendl;
			// we don't connect to clients
			assert(con->get_peer_type() == CEPH_ENTITY_TYPE_OSD);
			s->entity_name.set_type(CEPH_ENTITY_TYPE_OSD);
		}
		s->put();
	}
}
{% endhighlight %}
OSD只会主动向Monitor以及其他OSD发起连接。这里处理向其他OSD主动发起连接的回调请求。


8) **ms_handle_accept()的实现**

在OSD中没有对ms_handle_accept()函数进行重新实现。

9）**ms_handle_fast_accept()的实现**
{% highlight string %}
bool ms_can_fast_dispatch_any() const { return true; }

void OSD::ms_handle_fast_accept(Connection *con)
{
	if (con->get_peer_type() != CEPH_ENTITY_TYPE_MON) {
		Session *s = static_cast<Session*>(con->get_priv());
		if (!s) {
			s = new Session(cct);
			con->set_priv(s->get());
			s->con = con;
			dout(10) << "new session (incoming)" << s << " con=" << con
			  << " addr=" << con->get_peer_addr()
			  << " must have raced with connect" << dendl;
			assert(con->get_peer_type() == CEPH_ENTITY_TYPE_OSD);
			s->entity_name.set_type(CEPH_ENTITY_TYPE_OSD);
		}
		s->put();
	}
}
{% endhighlight %}
由于ms_can_fast_dispatch_any()永远返回true，因此OSD会接受Monitor，RadosGw以及其他OSD的连接。通过上面的代码，我们看到当RadosGW和其他OSD向OSD发起新的连接时，会构造生成一个新的Session，将其作为connection的private值保存起来。

10) **ms_handle_reset()的实现**
{% highlight string %}
bool OSD::ms_handle_reset(Connection *con)
{
	OSD::Session *session = (OSD::Session *)con->get_priv();
	dout(1) << "ms_handle_reset con " << con << " session " << session << dendl;
	if (!session)
		return false;
	session->wstate.reset(con);
	session->con.reset(NULL);  // break con <-> session ref cycle
	session_handle_reset(session);
	session->put();
	return true;
}
{% endhighlight %}
当连接被reset时，回调此函数清空该连接对应的session信息

11）**ms_handle_remote_reset()的实现**
{% highlight string %}
void ms_handle_remote_reset(Connection *con) {}
{% endhighlight %}

上面对ms_handle_remote_reset()的实现为空。

12) **ms_get_authorizer()的实现**
{% highlight string %}
bool OSD::ms_get_authorizer(int dest_type, AuthAuthorizer **authorizer, bool force_new)
{
	dout(10) << "OSD::ms_get_authorizer type=" << ceph_entity_type_name(dest_type) << dendl;
	
	if (dest_type == CEPH_ENTITY_TYPE_MON)
		return true;
	
	if (force_new) {
		/* the MonClient checks keys every tick(), so we should just wait for that cycle to get through */
		if (monc->wait_auth_rotating(10) < 0)
			return false;
	}
	
	*authorizer = monc->auth->build_authorizer(dest_type);
	return *authorizer != NULL;
}
{% endhighlight %}
上面可以看到其调用build_authorizer()来构建AuthAuthorizer。

13）**ms_verify_authorizer()的实现**
{% highlight string %}
bool OSD::ms_verify_authorizer(Connection *con, int peer_type,
			       int protocol, bufferlist& authorizer_data, bufferlist& authorizer_reply,
			       bool& isvalid, CryptoKey& session_key)
{
	...
}
{% endhighlight %}
实现对incomming连接的校验。


## 2. OSD模块消息

下面介绍一下OSD模块所使用到的一些消息：

* CEPH_MSG_OSD_OP：客户端进行读写请求时会构造此消息，primary OSD会收到

* MSG_OSD_REPOP：在进行数据修改操作时，Replicated OSD会收到此类消息，由Primary OSD发送

* MSG_OSD_REPOPREPLY: 针对MSG_OSD_REPOP的响应

* MSG_OSD_SUBOP：Primary与Replicas之间针对objects的一些内部操作，主要用于object recovery的时候。

* MSG_OSD_SUBOPREPLY：针对MSG_OSD_SUBOP的响应

* CEPH_OSD_OP_WRITE： 写部分对象

* CEPH_OSD_OP_WRITEFULL: 写一个完整对象


### 2.1 ms_fast分发的消息类型

* CEPH_MSG_OSD_OP

* MSG_OSD_SUBOP

* MSG_OSD_REPOP

* MSG_OSD_SUBOPREPLY

* MSG_OSD_REPOPREPLY

* MSG_OSD_PG_PUSH

* MSG_OSD_PG_PULL

* MSG_OSD_PG_PUSH_REPLY

* MSG_OSD_PG_SCAN

* MSG_OSD_PG_BACKFILL

* MSG_OSD_EC_WRITE

* MSG_OSD_EC_WRITE_REPLY

* MSG_OSD_EC_READ

* MSG_OSD_EC_READ_REPLY

* MSG_OSD_REP_SCRUB

* MSG_OSD_PG_UPDATE_LOG_MISSING

* MSG_OSD_PG_UPDATE_LOG_MISSING_REPLY


### 2.2 普通分发的消息类型

* MSG_OSD_MARK_ME_DOWN

* CEPH_MSG_PING

* CEPH_MSG_OSD_MAP

* MSG_PGSTATSACK

* MSG_MON_COMMAND

* MSG_COMMAND

* MSG_OSD_SCRUB

* MSG_OSD_PG_CREATE

* MSG_OSD_PG_NOTIFY
 
* MSG_OSD_PG_QUERY

* MSG_OSD_PG_LOG
 
* MSG_OSD_PG_REMOVE

* MSG_OSD_PG_INFO

* MSG_OSD_PG_TRIM

* MSG_OSD_PG_MISSING:

* MSG_OSD_BACKFILL_RESERVE

* MSG_OSD_RECOVERY_RESERVE
   

## 3. ms_fast_dispatch()代码分析
通过上文分析，我们知道对于有以下消息，会调用ms_fast_dispatch()来进行分发，下面我们来看一下该函数的实现：
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
{% endhighlight %}

首先我们这里调用service.get_nextmap_reserved()来获得nextmap，我们来看其实现：
{% highlight string %}
class OSDService{
public:
	OSDMapRef osdmap;

	/*
	* osdmap - current published map
	* next_osdmap - pre_published map that is about to be published.
	*
	* We use the next_osdmap to send messages and initiate connections,
	* but only if the target is the same instance as the one in the map
	* epoch the current user is working from (i.e., the result is
	* equivalent to what is in next_osdmap).
	*
	* This allows the helpers to start ignoring osds that are about to
	* go down, and let OSD::handle_osd_map()/note_down_osd() mark them
	* down, without worrying about reopening connections from threads
	* working from old maps.
	*/
	OSDMapRef next_osdmap;

	/// gets ref to next_osdmap and registers the epoch as reserved
	OSDMapRef get_nextmap_reserved() {
		Mutex::Locker l(pre_publish_lock);
		if (!next_osdmap)
			return OSDMapRef();
	
		epoch_t e = next_osdmap->get_epoch();
		map<epoch_t, unsigned>::iterator i =map_reservations.insert(make_pair(e, 0)).first;
		i->second++;
	
		return next_osdmap;
	}

}；
{% endhighlight %}

从上面的注释中我们看到，service.osdmap是指当前已经发布的最新的OSDMap，而service.next_osdmap是将要发布的OSDMap。我们会使用next_osdmap来发送消息和初始化连接，但前提是目标target与当前用户工作在相同的OSDMap epoch。

下面我们从代码中来看一下service.osdmap与service.next_osdmap的区别：

1) **OSDService::publish_map()**

查找publish_map()函数，发现只有两个地方调用：

* OSD::init()时调用publish_map()
{% highlight string %}
int OSD::init()
{
	...
	service.publish_map(osdmap);

	...

	consume_map();

	...
}
{% endhighlight %}

在OSD初始化时，首先加载superblock中所指定的OSDMap版本作为当前的初始化osdmap，然后再调用consume_map()来消费该osdmap，这可能触发启动时PG的第一次peering操作。

* OSD::consume_map()

除了上面介绍的在OSD init阶段会触发调用consume_map()，还会在保存完一个新的OSDMap之后触发调用consume_map()，下面我们来看：
{% highlight string %}
void OSD::consume_map()
{
	...

	service.pre_publish_map(osdmap);
	service.await_reserved_maps();
	service.publish_map(osdmap);

	...
}
{% endhighlight %}
从上面代码我们看出，要等到reserved_maps都消费完成之后，osdmap才会真正发布。如下图所示：

![ceph-chapter10](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_51.jpg)

2） **OSDService::pre_publish_map()**

查找pre_publish_map()函数发现也只有两个地方调用：

* _committed_osd_maps()中调用pre_publish_map()
{% highlight string %}
void OSD::_committed_osd_maps(epoch_t first, epoch_t last, MOSDMap *m)
{
	...

	// advance through the new maps
	for (epoch_t cur = first; cur <= last; cur++) {

		dout(10) << " advance to epoch " << cur<< " (<= last " << last<< " <= newest_map " << superblock.newest_map<< ")" << dendl;
	
		OSDMapRef newmap = get_map(cur);
		assert(newmap);  // we just cached it above!
		
		// start blacklisting messages sent to peers that go down.
		service.pre_publish_map(newmap);
	
		...
	}

	...
}
{% endhighlight %}
当接收到MOSDMap消息，如果有符合条件的新OSDMaps，则会将其打包到一个Transaction中，之后再将该Transaction持久化到硬盘上。当持久化成功，会回调_committed_osd_map()函数。如上代码所示，当前OSD会遍历MOSDMap消息中的所有新OSDMap，然后调用service.pre_publish_map()将去标记为预发布状态。

* OSD::consume_map()中调用pre_publish_map

在_committed_osd_map()函数中还会调用OSD::consume_map():
{% highlight string %}
void OSD::consume_map()
{
	...

	service.pre_publish_map(osdmap);
	service.await_reserved_maps();
	service.publish_map(osdmap);

	...
}
{% endhighlight %}
在consume_map()中会触发发布新接收到的OSDMap，之后再触发相应PG的peering操作。


### 3.1 几个相关变量
在进一步分析ms_fast_dispatch()之前，我们先来分析一下如下几个重要的变量：
{% highlight string %}
struct Session : public RefCountedObject {
	list<OpRequestRef> waiting_on_map;

	map<spg_t, list<OpRequestRef> > waiting_for_pg;
};
class OSD : public Dispatcher,public md_config_obs_t {
public:
	set<Session*> session_waiting_for_map;
  	map<spg_t, set<Session*> > session_waiting_for_pg;
};

{% endhighlight %}
如下图所示：

![ceph-chapter10](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_52.jpg)

* Session::waiting_on_map: 等待在OSDMap上的请求。通常来说，在peering过程中当发现要创建PG，或者Peering过程中发现该PG有分裂出子PG，则可能会把相关的一些请求放入到session对应的waiting_on_map中；
{% highlight string %}
//处理peering event
void OSD::handle_pg_peering_evt(
  spg_t pgid,
  const pg_history_t& orig_history,
  pg_interval_map_t& pi,
  epoch_t epoch,
  PG::CephPeeringEvtRef evt)
{
	//
	wake_pg_waiters(pgid);
}


//PG分裂
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...
	if (!split_pgs.empty()) {
		rctx.on_applied->add(new C_CompleteSplits(this, split_pgs));
		split_pgs.clear();
	}
}
{% endhighlight %}

* Session::waiting_for_pg: 等待在指定PG上的请求。通常来说，当获取不到指定的PG时(比如当前并没有获得到最新的OSDMap，从而导致PG找不到)，就会将请求放入到waiting_for_pg中；



* OSD::session_waiting_for_map: 保存等待在OSDMap上的Session;

* OSD::session_waiting_for_pg：保存等待在指定PG上的session。


### 3.1 函数update_waiting_for_pg()
{% highlight string %}
void OSD::update_waiting_for_pg(Session *session, OSDMapRef newmap)
{
	assert(session->session_dispatch_lock.is_locked());
	if (!session->osdmap) {
		session->osdmap = newmap;
		return;
	}
	
	if (newmap->get_epoch() == session->osdmap->get_epoch())
		return;
	
	assert(newmap->get_epoch() > session->osdmap->get_epoch());
	
	map<spg_t, list<OpRequestRef> > from;
	from.swap(session->waiting_for_pg);
	
	for (map<spg_t, list<OpRequestRef> >::iterator i = from.begin();i != from.end();from.erase(i++)) {
		set<spg_t> children;

		if (!newmap->have_pg_pool(i->first.pool())) {
			// drop this wait list on the ground
			i->second.clear();
		} else {
			assert(session->osdmap->have_pg_pool(i->first.pool()));
			if (i->first.is_split(session->osdmap->get_pg_num(i->first.pool()), newmap->get_pg_num(i->first.pool()), &children)) {
				for (set<spg_t>::iterator child = children.begin(); child != children.end(); ++child) {

					unsigned split_bits = child->get_split_bits(newmap->get_pg_num(child->pool()));
					list<OpRequestRef> child_ops;

					OSD::split_list(&i->second, &child_ops, child->ps(), split_bits);

					if (!child_ops.empty()) {
						session->waiting_for_pg[*child].swap(child_ops);
						register_session_waiting_on_pg(session, *child);
					}
				}
			}
		}

		if (i->second.empty()) {
			clear_session_waiting_on_pg(session, i->first);
		} else {
			session->waiting_for_pg[i->first].swap(i->second);
		}
	}
	
	session->osdmap = newmap;
}
{% endhighlight %}
主要处理有PG分裂情况下，更新session的waiting_for_pg。

>注：PG的分裂会造成Monitor更新OSDMap


### 3.2 dispatch_session_waiting()函数
{% highlight string %}
void OSD::dispatch_session_waiting(Session *session, OSDMapRef osdmap)
{
	assert(session->session_dispatch_lock.is_locked());
	assert(session->osdmap == osdmap);
	for (list<OpRequestRef>::iterator i = session->waiting_on_map.begin();
	   i != session->waiting_on_map.end() && dispatch_op_fast(*i, osdmap); session->waiting_on_map.erase(i++));
	
	if (session->waiting_on_map.empty()) {
		clear_session_waiting_on_map(session);
	} else {
		register_session_waiting_on_map(session);
	}
	session->maybe_reset_osdmap();
}
{% endhighlight %}
dispatch_session_waiting()就是将session上waiting_on_map里面的请求，调用dispatch_op_fast()转发出去。

## 4. ms_dispatch()分析
{% highlight string %}
bool OSD::ms_dispatch(Message *m)
{
	if (m->get_type() == MSG_OSD_MARK_ME_DOWN) {
	service.got_stop_ack();
		m->put();
		return true;
	}
	
	// lock!
	
	osd_lock.Lock();
	if (is_stopping()) {
		osd_lock.Unlock();
		m->put();
		return true;
	}
	
	while (dispatch_running) {
		dout(10) << "ms_dispatch waiting for other dispatch thread to complete" << dendl;
		dispatch_cond.Wait(osd_lock);
	}
	dispatch_running = true;
	
	do_waiters();
	_dispatch(m);
	do_waiters();
	
	dispatch_running = false;
	dispatch_cond.Signal();
	
	osd_lock.Unlock();
	
	return true;
}
{% endhighlight %}
此函数用于分发非fast消息。我们来看，这里有一个```osd_lock```，这是一把十分大的锁。我们知道一个DispatchQueue会有dispatch_thread以及local_delivery_thread这两个线程来进行分发，这就存在竞争关系。OSD作为一个Dispatcher，使用osd_lock来保证同一时刻，只能有一个线程调用到此函数。

###### 4.1 do_waiters()函数

do_waiters()主要用于处理当前阻塞在waiting_for_osdmap上的请求。比如有些请求需要new osdMap，那么就会先将这些请求放入waiting_for_osdmap上。然后在新的OSDMap准备好后，就会调用take_waiters()将其加入到finished列表中：
{% highlight string %}
void OSD::activate_map(){
	...

	// process waiters
	take_waiters(waiting_for_osdmap);
}

void take_waiters(list<OpRequestRef>& ls) {
	finished_lock.Lock();
	finished.splice(finished.end(), ls);
	finished_lock.Unlock();
}
{% endhighlight %}

有如下两种情况会将请求加入到waiting_for_osdmap上：

* 分发消息时没有OSDMap
{% highlight string %}
void OSD::_dispatch(Message *m)
{
	switch (m->get_type()) {

	default:
		OpRequestRef op = op_tracker.create_request<OpRequest, Message*>(m);
		// no map?  starting up?
		if (!osdmap) {
			dout(7) << "no OSDMap, not booted" << dendl;
			logger->inc(l_osd_waiting_for_map);
			waiting_for_osdmap.push_back(op);
			op->mark_delayed("no osdmap");
			break;
		}
	}
}
{% endhighlight %}

* 需要更新的OSDMap的请求
{% highlight string %}
bool OSD::require_same_or_newer_map(OpRequestRef& op, epoch_t epoch,
				    bool is_fast_dispatch)
{
	Message *m = op->get_req();
	dout(15) << "require_same_or_newer_map " << epoch << " (i am " << osdmap->get_epoch() << ") " << m << dendl;
	
	assert(osd_lock.is_locked());
	
	// do they have a newer map?
	if (epoch > osdmap->get_epoch()) {
		dout(7) << "waiting for newer map epoch " << epoch<< " > my " << osdmap->get_epoch() << " with " << m << dendl;
		wait_for_new_map(op);
		return false;
	}
	
	if (!require_self_aliveness(op->get_req(), epoch)) {
		return false;
	}
	
	// ok, our map is same or newer.. do they still exist?
	if (m->get_connection()->get_messenger() == cluster_messenger && !require_same_peer_instance(op->get_req(), osdmap, is_fast_dispatch)) {
		return false;
	}
	
	return true;
}

void OSD::wait_for_new_map(OpRequestRef op)
{
	// ask?
	if (waiting_for_osdmap.empty()) {
		osdmap_subscribe(osdmap->get_epoch() + 1, false);
	}
	
	logger->inc(l_osd_waiting_for_map);
	waiting_for_osdmap.push_back(op);
	op->mark_delayed("wait for new map");
}
{% endhighlight %}

###### 4.2 _dispatch()分发消息

_dispatch()会分发如下消息，其中有一些不需要依赖OSDMap，另外一些则需要：

1） **无需OSDMap的消息**

* CEPH_MSG_PING

* CEPH_MSG_OSD_MAP

* MSG_MON_COMMAND

* MSG_COMMAND

* MSG_OSD_SCRUB

2) **需要OSDMap的消息**

* MSG_OSD_PG_CREATE

* MSG_OSD_PG_NOTIFY：由PG stray发送到PG primary的通知消息（其中包含pginfo信息)。

>注: Peering过程中，RecoveryCtx::send_notify()发送的就是此消息

* MSG_OSD_PG_QUERY：replica(stray)上用于处理来自于primary的PG查询。

>注：Peering过程中GetInfo就是通过发送此消息来查询的

* MSG_OSD_PG_LOG：PG一个副本向另一个副本发送的PGLog信息。通常发生于Peering过程

* MSG_OSD_PG_REMOVE

* MSG_OSD_PG_INFO：PG的一个副本向另一个副本发送的PGInfo信息。

>注：Peering过程中，发现有missing object时，就通过PG::search_for_missing()来发送此查询消息

* MSG_OSD_PG_TRIM

* MSG_OSD_PG_MISSING

* MSG_OSD_BACKFILL_RESERVE

* MSG_OSD_RECOVERY_RESERVE


## 5. OSD运行状态
下面我们简要介绍一下OSD运行中的几个状态，以便更好的理解peering:

![ceph-chapter10](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_53.jpg)




<br />
<br />

**[参看]**


1. [Ceph OSD](https://gitee.com/wanghongxu/cephknowledge/blob/master/Ceph-OSD.md)

2. [Ceph pg分裂流程及可行性分析](https://blog.csdn.net/weixin_30607659/article/details/96305842)

<br />
<br />
<br />

