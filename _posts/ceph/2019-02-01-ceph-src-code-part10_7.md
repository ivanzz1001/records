---
layout: post
title: ceph peering机制再研究(3)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


ceph的Peering过程是一个十分复杂的流程，其主要的目的是使一个PG内的OSD达成一个一致的状态。当主从副本达成一个一致的状态后，PG处于active状态，Peering过程的状态就结束了。但此时PG的三个OSD副本上的数据并非完全一致。

PG在如下两种情况下触发Peering过程：

* 当系统初始化时，OSD重新启动导致PG重新加载，或者PG新创建时，PG会发起一次Peering的过程

* 当有OSD失效，OSD的增加或者删除等导致PG的acting set发生了改变，该PG会重新发起一次Peering过程。


<!-- more -->


## 1. 基本概念
在具体的讲解Peering之前，我们先对其中的一些概念做个介绍。

### 1.1 acting set和up set
acting set是一个PG对应副本所在的OSD列表，该列表是有序的，列表中第一个OSD为主OSD。在通常情况下，up set与acting set列表是完全相同的，只有当存在```pg temp```时两者会不同。

下面我们先来看acting set与up set的计算过程：
{% highlight string %}
/**
* map a pg to its acting set as well as its up set. You must use
* the acting set for data mapping purposes, but some users will
* also find the up set useful for things like deciding what to
* set as pg_temp.
* Each of these pointers must be non-NULL.
*/
void pg_to_up_acting_osds(pg_t pg, vector<int> *up, int *up_primary,
                            vector<int> *acting, int *acting_primary) const {
							
    _pg_to_up_acting_osds(pg, up, up_primary, acting, acting_primary);
	
}
{% endhighlight %}
从上面的注释中我们了解到：acting set主要用于数据映射的目的；而up set只是根据crush map计算出来的，有些用户也会发现其具有重要的作用，通过对比acting set与up set我们就知道pg_temp是什么。

下面我们来看_pg_to_up_acting_osds()函数的实现：
{% highlight string %}
void OSDMap::_pg_to_up_acting_osds(const pg_t& pg, vector<int> *up, int *up_primary,
                                   vector<int> *acting, int *acting_primary) const
{
	const pg_pool_t *pool = get_pg_pool(pg.pool());
	
	if (!pool) {
		if (up)
			up->clear();
			
		if (up_primary)
			*up_primary = -1;
			
		if (acting)
			acting->clear();
			
		if (acting_primary)
			*acting_primary = -1;
		
		return;
	}
	
	vector<int> raw;
	vector<int> _up;
	vector<int> _acting;
	int _up_primary;
	int _acting_primary;
	ps_t pps;
	
	_pg_to_osds(*pool, pg, &raw, &_up_primary, &pps);
	_raw_to_up_osds(*pool, raw, &_up, &_up_primary);
	_apply_primary_affinity(pps, *pool, &_up, &_up_primary);
	_get_temp_osds(*pool, pg, &_acting, &_acting_primary);
	
	if (_acting.empty()) {
		_acting = _up;
		
		if (_acting_primary == -1) {
			_acting_primary = _up_primary;
		}
	}
	
	if (up)
		up->swap(_up);
		
	if (up_primary)
		*up_primary = _up_primary;
		
	if (acting)
		acting->swap(_acting);
		
	if (acting_primary)
		*acting_primary = _acting_primary;
}
{% endhighlight %}
从上面我们可以看到，对于PG的映射有多个步骤：

1） **_pg_to_osds()**

{% highlight string %}
int OSDMap::_pg_to_osds(const pg_pool_t& pool, pg_t pg,
			vector<int> *osds, int *primary,ps_t *ppps) const
{
	// map to osds[]
	ps_t pps = pool.raw_pg_to_pps(pg);  // placement ps
	unsigned size = pool.get_size();

	// what crush rule?
	int ruleno = crush->find_rule(pool.get_crush_ruleset(), pool.get_type(), size);
	if (ruleno >= 0)
		crush->do_rule(ruleno, pps, *osds, size, osd_weight);

	_remove_nonexistent_osds(pool, *osds);

	*primary = -1;
	for (unsigned i = 0; i < osds->size(); ++i) {
		if ((*osds)[i] != CRUSH_ITEM_NONE) {
			*primary = (*osds)[i];
			break;
		}
	}
	
	if (ppps)
		*ppps = pps;

	return osds->size();
}

bool exists(int osd) const
{
    //assert(osd >= 0);
    return osd >= 0 && osd < max_osd && (osd_state[osd] & CEPH_OSD_EXISTS);
}

void OSDMap::_remove_nonexistent_osds(const pg_pool_t& pool,
				      vector<int>& osds) const
{
	if (pool.can_shift_osds()) {
		unsigned removed = 0;
		
		for (unsigned i = 0; i < osds.size(); i++) {
			if (!exists(osds[i])) {
				removed++;
				continue;
			}
			
			if (removed) {
				osds[i - removed] = osds[i];
			}
		}
		
		if (removed)
			osds.resize(osds.size() - removed);
	} else {
		for (vector<int>::iterator p = osds.begin(); p != osds.end(); ++p) {
			if (!exists(*p))
				*p = CRUSH_ITEM_NONE;
		}
	}
}
{% endhighlight %}

_pg_to_osds()函数的实现比较简单，其从crushmap中获取PG所映射到的OSD。这里注意，从crushmap获取到到的映射与OSD的运行状态无关(与权重有关，当osd out出去之后，其权重变为了0，在进行crush->do_rule()时就不会再映射到该OSD上了)。之后调用_remove_nonexistent_osds()函数移除在该OSDMap下不存在的OSD。

_pg_to_osds()函数返回了最原始的PG到OSD的映射，并且第一个即为primary。

2) **_raw_to_up_osds()**
{% highlight string %}
void OSDMap::_raw_to_up_osds(const pg_pool_t& pool, const vector<int>& raw,
                             vector<int> *up, int *primary) const
{
	if (pool.can_shift_osds()) {
		// shift left
		up->clear();
		
		for (unsigned i=0; i<raw.size(); i++) {
			if (!exists(raw[i]) || is_down(raw[i]))
				continue;
			up->push_back(raw[i]);
		}
		
		*primary = (up->empty() ? -1 : up->front());
		
	} else {
		// set down/dne devices to NONE
		*primary = -1;
		up->resize(raw.size());
		
		for (int i = raw.size() - 1; i >= 0; --i) {
			if (!exists(raw[i]) || is_down(raw[i])) {
				(*up)[i] = CRUSH_ITEM_NONE;
			} else {
				*primary = (*up)[i] = raw[i];
			}
		}
	}
}
{% endhighlight %}

通过_pg_to_osds()函数，我们获取到了原始的PG到OSD的映射关系。而_raw_to_up_osds()函数主要是移除了在该OSDMap下处于down状态的OSD，即获取到了原始的up osds，并且将第一个设置为up primary。


3) **_apply_primary_affinity()**

此函数主要是处理primary亲和性的设置，这里不做介绍(默认不做设置)。

4) **_get_temp_osds()**
{% highlight string %}
void OSDMap::_get_temp_osds(const pg_pool_t& pool, pg_t pg,
				vector<int> *temp_pg, int *temp_primary) const
{
	pg = pool.raw_pg_to_pg(pg);
	map<pg_t,vector<int32_t> >::const_iterator p = pg_temp->find(pg);
	temp_pg->clear();
	
	if (p != pg_temp->end()) {
		for (unsigned i=0; i<p->second.size(); i++) {
			if (!exists(p->second[i]) || is_down(p->second[i])) {
				if (pool.can_shift_osds()) {
					continue;
				} else {
					temp_pg->push_back(CRUSH_ITEM_NONE);
				}
			} else {
				temp_pg->push_back(p->second[i]);
			}
		}
	}
	
	map<pg_t,int32_t>::const_iterator pp = primary_temp->find(pg);
	*temp_primary = -1;
	
	if (pp != primary_temp->end()) {
		*temp_primary = pp->second;
		
	} else if (!temp_pg->empty()) { // apply pg_temp's primary
		for (unsigned i = 0; i < temp_pg->size(); ++i) {
			if ((*temp_pg)[i] != CRUSH_ITEM_NONE) {
				*temp_primary = (*temp_pg)[i];
				break;
			}
		}
	}
}
{% endhighlight %}

_get_temp_osds()用于获取指定OSDMap下，指定PG所对应的temp_pg所映射到的OSD。

5）**完成PG的up set、acting set的映射**

我们再回到_pg_to_up_acting_osds()函数的最后，如果当前没有temp_pg，那么该PG的acting set即为up set。


针对up set与acting set我们可以总结为：up set是直接根据crushmap算出来的，与权威日志等没有任何关系；而acting set是我们真正读写数据时所依赖的一个osd序列，该序列的第一个osd作为primary来负责数据读写，因此必须拥有权威日志。


### 1.2 临时PG
假设初始状态一个PG的```up set```与```acting set```均为[0,1,2]列表。此时如果OSD0出现故障，重新发起Peering操作，根据CRUSH算法得到up set为[3,1,2], acting set也为[3,1,2]，但是在OSD::choose_acting()时发现osd3为新添加的OSD，并不能负担该PG上的读操作，不能成为该PG的主OSD。所以PG向Monitor申请一个临时的PG，osd1为临时的主OSD，这时```up set```为[3,1,2]，```acting set```变为[1,2,3]，这就出现了```up set```与```acting set```不同的情况。当osd3完成Backfill过程之后，临时PG被取消，该PG的acting set被修改为[3,2,1]，此时up set与acting set就又相等了。

通过阅读ceph相关源码，发现如下两种情况下会产生pg_temp:

* OSD主动下线，Monitor检测到下线消息之后，会为PG Primary为该OSD的所欲PG申请pg_temp;

* PG在peering过程中发现want与acting不一致时，主动申请pg_temp

下面我们分别介绍这两种情况。

###### 1.2.1 peering过程主动申请pg_temp
 
我们搜索OSD::queue_pg_temp()函数，发现有如下两个地方调用：
{% highlight string %}
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound)
{
	...
	if (auth_log_shard == all_info.end()) {
		if (up != acting) {
			dout(10) << "choose_acting no suitable info found (incomplete backfills?),"<< " reverting to up" << dendl;
			want_acting = up;

			vector<int> empty;
			osd->queue_want_pg_temp(info.pgid.pgid, empty);
		} else {
			dout(10) << "choose_acting failed" << dendl;
			assert(want_acting.empty());
		}
		return false;
	}

	...
	if (want != acting) {
		dout(10) << "choose_acting want " << want << " != acting " << acting<< ", requesting pg_temp change" << dendl;
		want_acting = want;
	
		if (want_acting == up) {
			// There can't be any pending backfill if
			// want is the same as crush map up OSDs.

			assert(compat_mode || want_backfill.empty());
			vector<int> empty;
			osd->queue_want_pg_temp(info.pgid.pgid, empty);
		} else
			osd->queue_want_pg_temp(info.pgid.pgid, want);
		return false;
	}

	...
}


void PG::start_peering_interval(
  const OSDMapRef lastmap,
  const vector<int>& newup, int new_up_primary,
  const vector<int>& newacting, int new_acting_primary,
  ObjectStore::Transaction *t)
{
	...

	if (acting.empty() && !up.empty() && up_primary == pg_whoami) {
		dout(10) << " acting empty, but i am up[0], clearing pg_temp" << dendl;
		osd->queue_want_pg_temp(info.pgid.pgid, acting);
	}
}
{% endhighlight %}
上面的代码中，会调用OSD::queue_want_pg_temp()将对应PG所要请求的acting set放入到队列中。

>注：如果所要请求的acting set为空，则表示清除对应的pg temp。


下面我们来看OSD::queue_want_pg_temp()的实现，以及对应的请求是如何发送出去的：
{% highlight string %}
void OSDService::queue_want_pg_temp(pg_t pgid, vector<int>& want)
{
	Mutex::Locker l(pg_temp_lock);
	map<pg_t,vector<int> >::iterator p = pg_temp_pending.find(pgid);
 
	if (p == pg_temp_pending.end() || p->second != want) {
		pg_temp_wanted[pgid] = want;
	}
}
{% endhighlight %}
其中OSDService::pg_temp_pending用于存放当前```已经```向Monitor发出过pg_temp请求的映射表；而OSDService::pg_temp_wanted表示```将要```发起pg_temp请求的映射表。下面我们来看具体的发送函数：
{% highlight string %}
void OSDService::send_pg_temp()
{
	Mutex::Locker l(pg_temp_lock);
	if (pg_temp_wanted.empty())
		return;

	dout(10) << "send_pg_temp " << pg_temp_wanted << dendl;

	MOSDPGTemp *m = new MOSDPGTemp(osdmap->get_epoch());
	m->pg_temp = pg_temp_wanted;
	monc->send_mon_message(m);
	_sent_pg_temp();
}

void OSDService::_sent_pg_temp()
{
	for (map<pg_t,vector<int> >::iterator p = pg_temp_wanted.begin();p != pg_temp_wanted.end();++p)
		pg_temp_pending[p->first] = p->second;

	pg_temp_wanted.clear();
}

void OSD::ms_handle_connect(Connection *con)
{
	...
	service.requeue_pg_temp();
    service.send_pg_temp();

	...
}


void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...
	service.send_pg_temp();
}
{% endhighlight %}

在OSDService::send_pg_temp()函数中，将pg_temp_wanted中的所有映射表都发送到OSDMonitor。查找该函数，发现会在两个地方被调用：

* OSD::ms_handle_connect(): 当OSD重新连接上monitor时，会requeue以前的pg_temp,然后再重新发送。这时因为，对于以前的pg_temp我们并不确定其是否已经发送出去，因此这里会重新进行发送。

* OSD::process_peering_events(): 发送Peering过程中所要求的pg_temp

###### 1.2.2 OSDMonitor检测到OSD主动下线时自动产生pg_temp
当OSD主动下线时会通过如下向Monitor发送MOSDMarkMeDown消息：
{% highlight string %}
int OSD::shutdown()
{
	if (!service.prepare_to_stop())
		return 0; // already shutting down

	....
}

bool OSDService::prepare_to_stop()
{
	Mutex::Locker l(is_stopping_lock);
	if (get_state() != NOT_STOPPING)
		return false;
	
	OSDMapRef osdmap = get_osdmap();
	if (osdmap && osdmap->is_up(whoami)) {
		dout(0) << __func__ << " telling mon we are shutting down" << dendl;
		set_state(PREPARING_TO_STOP);
		monc->send_mon_message(new MOSDMarkMeDown(monc->get_fsid(),
			osdmap->get_inst(whoami),
			osdmap->get_epoch(),
			true  // request ack
			));
		utime_t now = ceph_clock_now(cct);
		utime_t timeout;
		timeout.set_from_double(now + cct->_conf->osd_mon_shutdown_timeout);
		while ((ceph_clock_now(cct) < timeout) &&
		  (get_state() != STOPPING)) {
			is_stopping_cond.WaitUntil(is_stopping_lock, timeout);
		}
	}
	dout(0) << __func__ << " starting shutdown" << dendl;
	set_state(STOPPING);
	return true;
}
{% endhighlight %}

当Monitor收到该消息后，会做响应的处理（Monitor类继承自Dispatcher，因此可以分发消息），下面我们来看这一过程：

{% highlight string %}
void Monitor::_ms_dispatch(Message *m){
	...

	if ((is_synchronizing() || (s->global_id == 0 && !exited_quorum.is_zero())) &&
	   !src_is_mon && m->get_type() != CEPH_MSG_PING) {
		waitlist_or_zap_client(op);
	} else {
		dispatch_op(op);
	}

	...
}


void Monitor::dispatch_op(MonOpRequestRef op){
	...

	switch (op->get_req()->get_type()) {
	
		// OSDs
		case CEPH_MSG_MON_GET_OSDMAP:
		case MSG_OSD_MARK_ME_DOWN:
		case MSG_OSD_FAILURE:
		case MSG_OSD_BOOT:
		case MSG_OSD_ALIVE:
		case MSG_OSD_PGTEMP:
		case MSG_REMOVE_SNAPS:
			paxos_service[PAXOS_OSDMAP]->dispatch(op);
			break;

		...
	}

	...
}

{% endhighlight %}
此时其会调用OSDMonitor的dispatch()函数来分发消息：
{% highlight string %}
bool PaxosService::dispatch(MonOpRequestRef op){
	....

	// update
	if (prepare_update(op)) {
		double delay = 0.0;
		if (should_propose(delay)) {
			if (delay == 0.0) {
				propose_pending();
			} else {
				// delay a bit
				if (!proposal_timer) {
					/**
					* Callback class used to propose the pending value once the proposal_timer
					* fires up.
					*/
					proposal_timer = new C_MonContext(mon, [this](int r) {
						proposal_timer = 0;
						if (r >= 0)
							propose_pending();
						else if (r == -ECANCELED || r == -EAGAIN)
							return;
						else
							assert(0 == "bad return value for proposal_timer");
					});
					dout(10) << " setting proposal_timer " << proposal_timer << " with delay of " << delay << dendl;
					mon->timer.add_event_after(delay, proposal_timer);
				} else { 
					dout(10) << " proposal_timer already set" << dendl;
				}
			}

		} else {
			dout(10) << " not proposing" << dendl;
		}
	}   

	return true;  
}
{% endhighlight %}

下面我们就来看看具体的产生pg_temp的过程：

1）**处理MSG_OSD_MARK_ME_DOWN消息**
{% highlight string %}
bool OSDMonitor::prepare_update(MonOpRequestRef op)
{
	...

	switch (m->get_type()) {
		// damp updates
		case MSG_OSD_MARK_ME_DOWN:
			return prepare_mark_me_down(op);
		....
	}
	
	return false;
}

bool OSDMonitor::prepare_mark_me_down(MonOpRequestRef op)
{
	op->mark_osdmon_event(__func__);
	MOSDMarkMeDown *m = static_cast<MOSDMarkMeDown*>(op->get_req());
	int target_osd = m->get_target().name.num();
	
	assert(osdmap.is_up(target_osd));
	assert(osdmap.get_addr(target_osd) == m->get_target().addr);
	
	mon->clog->info() << "osd." << target_osd << " marked itself down\n";
	pending_inc.new_state[target_osd] = CEPH_OSD_UP;
	if (m->request_ack)
		wait_for_finished_proposal(op, new C_AckMarkedDown(this, op));
	return true;
}
{% endhighlight %}
如上，在函数中构造了一个proposal: C_AckMarkedDown，然后插入到waiting_for_finished_proposal列表中，准备对提议进行表决。因为Paxos需要对每一个提议进行持久化，因此这里会调用：
{% highlight string %}
void OSDMonitor::encode_pending(MonitorDBStore::TransactionRef t){
	...

	if (g_conf->mon_osd_prime_pg_temp)
		maybe_prime_pg_temp();

	...
}
{% endhighlight %}

跟踪到这里，我们发现maybe_prime_pg_temp()可能会为该主动关闭的OSD预先产生好pg_temp。下面我们来看看该函数的实现。

2）**OSDMonitor::maybe_prime_pg_temp()**
{% highlight string %}
void OSDMonitor::maybe_prime_pg_temp(){

	...
	utime_t stop = ceph_clock_now(NULL);
	stop += g_conf->mon_osd_prime_pg_temp_max_time;

	...

	if(all){
		...
	}else{
		dout(10) << __func__ << " " << osds.size() << " interesting osds" << dendl;

		for (set<int>::iterator p = osds.begin(); p != osds.end(); ++p) {
			n -= prime_pg_temp(next, pg_map, *p);
			if (--n <= 0) {
				n = chunk;
				if (ceph_clock_now(NULL) > stop) {
					dout(10) << __func__ << " consumed more than "<< g_conf->mon_osd_prime_pg_temp_max_time
						<< " seconds, stopping"<< dendl;
					break;
				}
			}
		}
	}
}
{% endhighlight %}

上面函数中调用prime_pg_temp()来为指定的OSD构建pg_temp:
{% highlight string %}
int OSDMonitor::prime_pg_temp(OSDMap& next, PGMap *pg_map, int osd)
{
	dout(10) << __func__ << " osd." << osd << dendl;
	int num = 0;

	ceph::unordered_map<int, set<pg_t> >::iterator po = pg_map->pg_by_osd.find(osd);
	if (po != pg_map->pg_by_osd.end()) {
		for (set<pg_t>::iterator p = po->second.begin();p != po->second.end();++p, ++num) {
			ceph::unordered_map<pg_t, pg_stat_t>::iterator pp = pg_map->pg_stat.find(*p);
			if (pp == pg_map->pg_stat.end())
				continue;
			prime_pg_temp(next, pp);
		}
	}
	return num;
}

void OSDMonitor::prime_pg_temp(OSDMap& next,
			       ceph::unordered_map<pg_t, pg_stat_t>::iterator pp)
{
	// do not prime creating pgs
	if (pp->second.state & PG_STATE_CREATING)
		return;

	// do not touch a mapping if a change is pending
	if (pending_inc.new_pg_temp.count(pp->first))
		return;

	vector<int> up, acting;
	int up_primary, acting_primary;
	next.pg_to_up_acting_osds(pp->first, &up, &up_primary, &acting, &acting_primary);

	if (acting == pp->second.acting)
		return;  // no change since last pg update, skip

	vector<int> cur_up, cur_acting;
	osdmap.pg_to_up_acting_osds(pp->first, &cur_up, &up_primary,&cur_acting, &acting_primary);

	if (cur_acting == acting)
		return;  // no change this epoch; must be stale pg_stat

	if (cur_acting.empty())
		return;  // if previously empty now we can be no worse off

	const pg_pool_t *pool = next.get_pg_pool(pp->first.pool());
	if (pool && cur_acting.size() < pool->min_size)
		return;  // can be no worse off than before
	
	dout(20) << __func__ << " " << pp->first << " " << cur_up << "/" << cur_acting<< " -> " << up << "/" << acting
		<< ", priming " << cur_acting<< dendl;

	pending_inc.new_pg_temp[pp->first] = cur_acting;
}
{% endhighlight %}
上面首先会遍历该OSD上的每一个PG，然后根据相应的条件产生pg_temp。


### 1.3 pg_history_t数据结构
{% highlight string %}
/**
 * pg_history_t - information about recent pg peering/mapping history
 *
 * This is aggressively shared between OSDs to bound the amount of past
 * history they need to worry about.
 */
struct pg_history_t {
	epoch_t epoch_created;       // epoch in which PG was created
	epoch_t last_epoch_started;  // lower bound on last epoch started (anywhere, not necessarily locally)
	epoch_t last_epoch_clean;    // lower bound on last epoch the PG was completely clean.
	epoch_t last_epoch_split;    // as parent
	epoch_t last_epoch_marked_full;  // pool or cluster
	
	/**
	* In the event of a map discontinuity, same_*_since may reflect the first
	* map the osd has seen in the new map sequence rather than the actual start
	* of the interval.  This is ok since a discontinuity at epoch e means there
	* must have been a clean interval between e and now and that we cannot be
	* in the active set during the interval containing e.
	*/
	epoch_t same_up_since;       // same acting set since
	epoch_t same_interval_since;   // same acting AND up set since
	epoch_t same_primary_since;  // same primary at least back through this epoch.
	
	eversion_t last_scrub;
	eversion_t last_deep_scrub;
	utime_t last_scrub_stamp;
	utime_t last_deep_scrub_stamp;
	utime_t last_clean_scrub_stamp;
};
{% endhighlight %}
pg_history_t数据结构在PG运行过程中具有重要的作用，其一般用来记录已经发生的且具有一定权威性的内容，可以在OSD之间来进行共享。

* epoch_created: PG创建时的epoch值，由PGMonitor生成

* last_epoch_started:PG上一次激活(activate)时的epoch值

* last_epoch_clean：PG上一次进入clean状态时的epoch值

* same_up_since：记录当前up set出现的最早时刻

* same_interval_since： 表示的是某一个interval的第一个osdmap版本号

### 1.4 past_interval介绍
所谓```past_interval```就是osdmap版本号epoch的一个序列，在该序列内一个PG的acting set与up set不会发生变化。

{% highlight string %}
/**
 * pg_interval_t - information about a past interval
 */
struct pg_interval_t {
	vector<int32_t> up, acting;
	epoch_t first, last;
	bool maybe_went_rw;
	int32_t primary;
	int32_t up_primary;
	
	pg_interval_t()
		: first(0), last(0),maybe_went_rw(false),primary(-1),up_primary(-1)
	{}
};

class PG : DoutPrefixProvider {
public:
	 map<epoch_t,pg_interval_t> past_intervals;
};
{% endhighlight %}

每一个PG都会维护一个past_intervals的映射表，其中key为该```past_interval```的第一个epoch值。下面简要介绍一下pg_interval_t各字段的值：

* pg_interval_t::up => PG在该interval期间的up set;

* pg_interval_t::acting => PG在该interval期间的acting set

* pg_interval_t::first => 该interval的第一个epoch值

* pg_interval_t::last => 该interval的最后一个epoch值

* pg_interval_t::maybe_went_rw => 表示在该interval期间可能执行了写操作

* pg_interval_t::primary => PG在该该interval期间的acting primary

* pg_interval_t::up_primary =>PG在该interval期间的up primary

#### 1.4.1 pg_interval_t的几个重要函数

1) **is_new_interval(): 版本1**
{% highlight string %}
bool pg_interval_t::is_new_interval(
  int old_acting_primary,
  int new_acting_primary,
  const vector<int> &old_acting,
  const vector<int> &new_acting,
  int old_up_primary,
  int new_up_primary,
  const vector<int> &old_up,
  const vector<int> &new_up,
  int old_size,
  int new_size,
  int old_min_size,
  int new_min_size,
  unsigned old_pg_num,
  unsigned new_pg_num,
  bool old_sort_bitwise,
  bool new_sort_bitwise,
  pg_t pgid) {
	return old_acting_primary != new_acting_primary ||
		new_acting != old_acting ||
		old_up_primary != new_up_primary ||
		new_up != old_up ||
		old_min_size != new_min_size ||
		old_size != new_size ||
		pgid.is_split(old_pg_num, new_pg_num, 0) ||
		old_sort_bitwise != new_sort_bitwise;
}
{% endhighlight %}
从上面我们看出，只要该PG的up set与acting set任何一个发生变化，或者PG发生了分裂，都会产生一个新的interval。

2） **is_new_interval()：版本2**
{% highlight string %}
bool pg_interval_t::is_new_interval(
  int old_acting_primary,
  int new_acting_primary,
  const vector<int> &old_acting,
  const vector<int> &new_acting,
  int old_up_primary,
  int new_up_primary,
  const vector<int> &old_up,
  const vector<int> &new_up,
  OSDMapRef osdmap,
  OSDMapRef lastmap,
  pg_t pgid) {
	return !(lastmap->get_pools().count(pgid.pool())) ||
		is_new_interval(old_acting_primary,
			new_acting_primary,
			old_acting,
			new_acting,
			old_up_primary,
			new_up_primary,
			old_up,
			new_up,
			lastmap->get_pools().find(pgid.pool())->second.size,
			osdmap->get_pools().find(pgid.pool())->second.size,
			lastmap->get_pools().find(pgid.pool())->second.min_size,
			osdmap->get_pools().find(pgid.pool())->second.min_size,
			lastmap->get_pg_num(pgid.pool()),
			osdmap->get_pg_num(pgid.pool()),
			lastmap->test_flag(CEPH_OSDMAP_SORTBITWISE),
			osdmap->test_flag(CEPH_OSDMAP_SORTBITWISE),
			pgid);
}
{% endhighlight %}
从上面可以看出，如果lastmap中不含有当前PG所在的pool，那么为一个新的interval；另外PG的up set与acting set任何一个发生变化，或者PG发生了分裂，都会产生一个新的interval。

3）**check_new_interval()**
{% highlight string %}
bool pg_interval_t::check_new_interval(
  int old_acting_primary,                                ///< [in] primary as of lastmap
  int new_acting_primary,                                ///< [in] primary as of osdmap
  const vector<int> &old_acting,                         ///< [in] acting as of lastmap
  const vector<int> &new_acting,                         ///< [in] acting as of osdmap
  int old_up_primary,                                    ///< [in] up primary of lastmap
  int new_up_primary,                                    ///< [in] up primary of osdmap
  const vector<int> &old_up,                             ///< [in] up as of lastmap
  const vector<int> &new_up,                             ///< [in] up as of osdmap
  epoch_t same_interval_since,                           ///< [in] as of osdmap
  epoch_t last_epoch_clean,                              ///< [in] current
  OSDMapRef osdmap,                                      ///< [in] current map
  OSDMapRef lastmap,                                     ///< [in] last map
  pg_t pgid,                                             ///< [in] pgid for pg
  IsPGRecoverablePredicate *could_have_gone_active,      /// [in] predicate whether the pg can be active
  map<epoch_t, pg_interval_t> *past_intervals,           ///< [out] intervals
  std::ostream *out)                                     ///< [out] debug ostream
{
	// remember past interval
	//  NOTE: a change in the up set primary triggers an interval
	//  change, even though the interval members in the pg_interval_t
	//  do not change.
	if (is_new_interval(
	  old_acting_primary,
	  new_acting_primary,
	  old_acting,
	  new_acting,
	  old_up_primary,
	  new_up_primary,
	  old_up,
	  new_up,
	  osdmap,
	  lastmap,
	  pgid)) {
		pg_interval_t& i = (*past_intervals)[same_interval_since];
		i.first = same_interval_since;
		i.last = osdmap->get_epoch() - 1;
		assert(i.first <= i.last);
		i.acting = old_acting;
		i.up = old_up;
		i.primary = old_acting_primary;
		i.up_primary = old_up_primary;

    	unsigned num_acting = 0;
		for (vector<int>::const_iterator p = i.acting.begin(); p != i.acting.end();++p)
			if (*p != CRUSH_ITEM_NONE)
				++num_acting;
	
		const pg_pool_t& old_pg_pool = lastmap->get_pools().find(pgid.pool())->second;
		set<pg_shard_t> old_acting_shards;
		old_pg_pool.convert_to_pg_shards(old_acting, &old_acting_shards);
	
		if (num_acting && i.primary != -1 && num_acting >= old_pg_pool.min_size && (*could_have_gone_active)(old_acting_shards)) {
			if (out)
			*out << "generate_past_intervals " << i<< ": not rw," << " up_thru " << lastmap->get_up_thru(i.primary)
				<< " up_from " << lastmap->get_up_from(i.primary)<< " last_epoch_clean " << last_epoch_clean << std::endl;

			if (lastmap->get_up_thru(i.primary) >= i.first && lastmap->get_up_from(i.primary) <= i.first) {
				i.maybe_went_rw = true;
				if (out)
					*out << "generate_past_intervals " << i << " : primary up " << lastmap->get_up_from(i.primary)
						<< "-" << lastmap->get_up_thru(i.primary) << " includes interval"<< std::endl;

			} else if (last_epoch_clean >= i.first && last_epoch_clean <= i.last) {
				// If the last_epoch_clean is included in this interval, then
				// the pg must have been rw (for recovery to have completed).
				// This is important because we won't know the _real_
				// first_epoch because we stop at last_epoch_clean, and we
				// don't want the oldest interval to randomly have
				// maybe_went_rw false depending on the relative up_thru vs
				// last_epoch_clean timing.
				i.maybe_went_rw = true;
				if (out)
					*out << "generate_past_intervals " << in<< " : includes last_epoch_clean " << last_epoch_clean
						<< " and presumed to have been rw" << std::endl;
			} else {
				i.maybe_went_rw = false;
				if (out)
					*out << "generate_past_intervals " << i << " : primary up " << lastmap->get_up_from(i.primary)
						<< "-" << lastmap->get_up_thru(i.primary) << " does not include interval" << std::endl;
			}
		} else {
			i.maybe_went_rw = false;
			if (out)
				*out << "generate_past_intervals " << i << " : acting set is too small" << std::endl;
		}

		return true;
	} else {
		return false;
	}
}
{% endhighlight %}
>注： same_interval_since表示的是某一个interval的第一个osdmap版本号

本函数通过对比```lastmap```与```osdmap```，判断当前是否是一个新的interval，如果是则将lastmap放入```past_intervals```中，然后再判断past_interval是否可能执行了写操作。如果past_interval中有acting set，并且数量达到了对应pool的min_size，且通过该old_acting_shards足以进入active状态：

* 如果该PG的primary osd在lastmap时的```up_thru```值大于等于past_interval.first，且该OSD的up_from值小于等于past_interval.first，则可能发生了写入操作，将past_interval.maybe_went_rw设置为true；

* 如果last_epoch_clean值在[past_interval.first, past_interval.last]之间时，则当前的past_interval肯定是由于recovery恢复完成，然后产生acting change事件从而改变acting set而生成的，在此一过程中也是可能进行写入操作，因此将past_interval.maybe_went_rw设置为true；

* 其他情况，不可能会执行写入操作，将past_interval.maybe_went_rw设置为false；

#### 1.4.2 PG::past_intervals的产生

past_intervals的产生主要有两个地方，下面我们来分析：

###### 1.4.2.1 PG::generate_past_intervals()
{% highlight string %}
void PG::generate_past_intervals()
{
	epoch_t cur_epoch, end_epoch;
	if (!_calc_past_interval_range(&cur_epoch, &end_epoch,osd->get_superblock().oldest_map)) {
		if (info.history.same_interval_since == 0) {
			info.history.same_interval_since = end_epoch;
			dirty_info = true;
		}

		return;
	}
	
	OSDMapRef last_map, cur_map;
	int primary = -1;
	int up_primary = -1;
	vector<int> acting, up, old_acting, old_up;
	
	cur_map = osd->get_map(cur_epoch);
	cur_map->pg_to_up_acting_osds(
	get_pgid().pgid, &up, &up_primary, &acting, &primary);
	epoch_t same_interval_since = cur_epoch;

	dout(10) << __func__ << " over epochs " << cur_epoch << "-"<< end_epoch << dendl;

	++cur_epoch;
	for (; cur_epoch <= end_epoch; ++cur_epoch) {
		int old_primary = primary;
		int old_up_primary = up_primary;
		last_map.swap(cur_map);
		old_up.swap(up);
		old_acting.swap(acting);
	
		cur_map = osd->get_map(cur_epoch);
		pg_t pgid = get_pgid().pgid;
		if (last_map->get_pools().count(pgid.pool()))
			pgid = pgid.get_ancestor(last_map->get_pg_num(pgid.pool()));

		cur_map->pg_to_up_acting_osds(pgid, &up, &up_primary, &acting, &primary);
	
		boost::scoped_ptr<IsPGRecoverablePredicate> recoverable(get_is_recoverable_predicate());
		std::stringstream debug;
		bool new_interval = pg_interval_t::check_new_interval(
			old_primary,
			primary,
			old_acting,
			acting,
			old_up_primary,
			up_primary,
			old_up,
			up,
			same_interval_since,
			info.history.last_epoch_clean,
			cur_map,
			last_map,
			pgid,
			recoverable.get(),
			&past_intervals,
			&debug);

		if (new_interval) {
			dout(10) << debug.str() << dendl;
			same_interval_since = cur_epoch;
		}
	}
	
	// PG import needs recalculated same_interval_since
	if (info.history.same_interval_since == 0) {
		assert(same_interval_since);
		dout(10) << __func__ << " fix same_interval_since " << same_interval_since << " pg " << *this << dendl;
		dout(10) << __func__ << " past_intervals " << past_intervals << dendl;

		// Fix it
		info.history.same_interval_since = same_interval_since;
	}
	
	// record our work.
	dirty_info = true;
	dirty_big_info = true;
}
{% endhighlight %}
我们来分析一下该函数的实现：

1） **调用_calc_past_interval_range()计算当前PG的past_interval可能的范围**
{% highlight string %}
bool PG::_calc_past_interval_range(epoch_t *start, epoch_t *end, epoch_t oldest_map)
{
	if (info.history.same_interval_since) {
		*end = info.history.same_interval_since;
	} else {
		// PG must be imported, so let's calculate the whole range.
		*end = osdmap_ref->get_epoch();
	}
	
	// Do we already have the intervals we want?
	map<epoch_t,pg_interval_t>::const_iterator pif = past_intervals.begin();
	if (pif != past_intervals.end()) {
		if (pif->first <= info.history.last_epoch_clean) {
			dout(10) << __func__ << ": already have past intervals back to "<< info.history.last_epoch_clean << dendl;
			return false;
		}

		*end = past_intervals.begin()->first;
	}
	
	*start = MAX(MAX(info.history.epoch_created,
		info.history.last_epoch_clean),
		oldest_map);
	if (*start >= *end) {
		dout(10) << __func__ << " start epoch " << *start << " >= end epoch " << *end<< ", nothing to do" << dendl;
		return false;
	}
	
	return true;
}
{% endhighlight %}

* 计算end值

a) info.history.same_interval_since是指当前interval(```current_interval```)的第一个osdmap版本号。因此，如果same_interval_since值不为0，那就将当前要计算的```past_intervals```的end设置为该值。
{% highlight string %}
if (info.history.same_interval_since) {
	*end = info.history.same_interval_since;
} else {
	// PG must be imported, so let's calculate the whole range.
	*end = osdmap_ref->get_epoch();
}
{% endhighlight %}

b) 查看past_intervals里已经计算的past_interval的第一个epoch，如果已经比info.history.last_epoch_clean小，就不用计算了，直接返回false。否则设置end为其first值。
{% highlight string %}
*end = past_intervals.begin()->first;
{% endhighlight %}

* 计算start值

a) start设置为info.history.last_epoch_clean，从最后一次last_epoch_clean算起

b）当PG为新建时，从info.history.epoch_started开始计算

c）oldest_map值为保存的最早osd map值，如果start小于这个值，相关的osdmap信息缺失，所以无法计算

所以将start设置为三者的最大值：
{% highlight string %}
*start = MAX(MAX(info.history.epoch_created,
	info.history.last_epoch_clean),
	oldest_map);
{% endhighlight %}

下面我们给出一个示例来说明计算past_intervals的过程

* past_intervals计算示例

![ceph-chapter10-6](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_20.jpg)

如上表所示：一个PG有4个interval。past_interval 1，开始epoch为4，结束的epoch为8；past_interval 2的epoch区间为(9,11)；past_interval 3的区间为(12,13)；current_interval的区间为(14,16)。最新的epoch为16，info.history.same_interval_since为14，意指是从epoch 14开始，之后的epoch值和当前的epoch值在同一个interval内。info.history.last_epoch_clean为8，就是说在epoch值为8时，该PG处于clean状态。

计算start 和 end的方法如下：

a）start的值设置为info.history.last_epoch_clean值，其值为8

b）end值从14开始计算，检查当前已经计算好的past_intervals的值。past_interval的计算是从后往前计算。如果第一个past_interval的first小于等于8，也就是past_interval 1已经计算过了，那么后面的past_interval 2和past_interval 3都已经计算过，就直接退出。否则就继续查找没有计算过的past_interval值。


###### 1.4.2.2 OSD::build_past_intervals_parallel()
{% highlight string %}
/*
 * build past_intervals efficiently on old, degraded, and buried
 * clusters.  this is important for efficiently catching up osds that
 * are way behind on maps to the current cluster state.
 *
 * this is a parallel version of PG::generate_past_intervals().
 * follow the same logic, but do all pgs at the same time so that we
 * can make a single pass across the osdmap history.
 */
struct pistate {
	epoch_t start, end;
	vector<int> old_acting, old_up;
	epoch_t same_interval_since;
	int primary;
	int up_primary;
};

void OSD::build_past_intervals_parallel()
{
	map<PG*,pistate> pis;
	
	// calculate junction of map range
	epoch_t end_epoch = superblock.oldest_map;
	epoch_t cur_epoch = superblock.newest_map;

	{
		RWLock::RLocker l(pg_map_lock);
		for (ceph::unordered_map<spg_t, PG*>::iterator i = pg_map.begin();i != pg_map.end();++i) {
			PG *pg = i->second;
	
			epoch_t start, end;
			if (!pg->_calc_past_interval_range(&start, &end, superblock.oldest_map)) {
				if (pg->info.history.same_interval_since == 0)
					pg->info.history.same_interval_since = end;
				continue;
			}
	
			dout(10) << pg->info.pgid << " needs " << start << "-" << end << dendl;
			pistate& p = pis[pg];
			p.start = start;
			p.end = end;
			p.same_interval_since = 0;
	
			if (start < cur_epoch)
				cur_epoch = start;
			if (end > end_epoch)
				end_epoch = end;
		}
	}


	if (pis.empty()) {
		dout(10) << __func__ << " nothing to build" << dendl;
		return;
	}
	
	dout(1) << __func__ << " over " << cur_epoch << "-" << end_epoch << dendl;
	assert(cur_epoch <= end_epoch);
	
	OSDMapRef cur_map, last_map;
	for ( ; cur_epoch <= end_epoch; cur_epoch++) {
		dout(10) << __func__ << " epoch " << cur_epoch << dendl;
		last_map = cur_map;
		cur_map = get_map(cur_epoch);
	
		for (map<PG*,pistate>::iterator i = pis.begin(); i != pis.end(); ++i) {
			PG *pg = i->first;
			pistate& p = i->second;
	
			if (cur_epoch < p.start || cur_epoch > p.end)
				continue;

			vector<int> acting, up;
			int up_primary;
			int primary;
			pg_t pgid = pg->info.pgid.pgid;
			if (p.same_interval_since && last_map->get_pools().count(pgid.pool()))
				pgid = pgid.get_ancestor(last_map->get_pg_num(pgid.pool()));

			cur_map->pg_to_up_acting_osds(
				pgid, &up, &up_primary, &acting, &primary);

			if (p.same_interval_since == 0) {
				dout(10) << __func__ << " epoch " << cur_epoch << " pg " << pg->info.pgid<< " first map, acting " << acting
					<< " up " << up << ", same_interval_since = " << cur_epoch << dendl;
				p.same_interval_since = cur_epoch;
				p.old_up = up;
				p.old_acting = acting;
				p.primary = primary;
				p.up_primary = up_primary;
				continue;
      		}

			assert(last_map);
			
			boost::scoped_ptr<IsPGRecoverablePredicate> recoverable(pg->get_is_recoverable_predicate());
			std::stringstream debug;
			bool new_interval = pg_interval_t::check_new_interval(
				p.primary,
				primary,
				p.old_acting, acting,
				p.up_primary,
				up_primary,
				p.old_up, up,
				p.same_interval_since,
				pg->info.history.last_epoch_clean,
				cur_map, last_map,
				pgid,
				recoverable.get(),
				&pg->past_intervals,
				&debug);
			if (new_interval) {
				dout(10) << __func__ << " epoch " << cur_epoch << " pg " << pg->info.pgid<< " " << debug.str() << dendl;
				p.old_up = up;
				p.old_acting = acting;
				p.primary = primary;
				p.up_primary = up_primary;
				p.same_interval_since = cur_epoch;
			}
		}
	}

	// Now that past_intervals have been recomputed let's fix the same_interval_since
	// if it was cleared by import.
	for (map<PG*,pistate>::iterator i = pis.begin(); i != pis.end(); ++i) {
		PG *pg = i->first;
		pistate& p = i->second;
	
		if (pg->info.history.same_interval_since == 0) {
			assert(p.same_interval_since);
			dout(10) << __func__ << " fix same_interval_since " << p.same_interval_since << " pg " << *pg << dendl;
			dout(10) << __func__ << " past_intervals " << pg->past_intervals << dendl;
			// Fix it
			pg->info.history.same_interval_since = p.same_interval_since;
		}
	}

	// write info only at the end.  this is necessary because we check
	// whether the past_intervals go far enough back or forward in time,
	// but we don't check for holes.  we could avoid it by discarding
	// the previous past_intervals and rebuilding from scratch, or we
	// can just do this and commit all our work at the end.
	ObjectStore::Transaction t;
	int num = 0;
	for (map<PG*,pistate>::iterator i = pis.begin(); i != pis.end(); ++i) {
		PG *pg = i->first;
		pg->lock();
		pg->dirty_big_info = true;
		pg->dirty_info = true;
		pg->write_if_dirty(t);
		pg->unlock();
	
		// don't let the transaction get too big
		if (++num >= cct->_conf->osd_target_transaction_size) {
			store->apply_transaction(service.meta_osr.get(), std::move(t));
			t = ObjectStore::Transaction();
			num = 0;
		}
	}

	if (!t.empty())
		store->apply_transaction(service.meta_osr.get(), std::move(t));
}
{% endhighlight %}

本函数与PG::generate_past_intervals()类似，只是为了快速的计算所有PG(注：pg primary为本OSD的pgs)的past_intervals，并在计算完成后保存。

#### 1.4.3 PG::past_intervals使用场景
搜寻PG::generate_past_intervals()，我们发现主要在如下地方被调用：

1） **Reset状态接收到AdvMap事件**

{% highlight string %}
boost::statechart::result PG::RecoveryState::Reset::react(const AdvMap& advmap)
{
	PG *pg = context< RecoveryMachine >().pg;
	dout(10) << "Reset advmap" << dendl;
	
	// make sure we have past_intervals filled in.  hopefully this will happen
	// _before_ we are active.
	pg->generate_past_intervals();
	
	pg->check_full_transition(advmap.lastmap, advmap.osdmap);
	
	if (pg->should_restart_peering(
	  advmap.up_primary,
	  advmap.acting_primary,
	  advmap.newup,
	  advmap.newacting,
	  advmap.lastmap,
	  advmap.osdmap)) {
		dout(10) << "should restart peering, calling start_peering_interval again"<< dendl;
		pg->start_peering_interval(
		advmap.lastmap,
		advmap.newup, advmap.up_primary,
		advmap.newacting, advmap.acting_primary,
		context< RecoveryMachine >().get_cur_transaction());
	}

	pg->remove_down_peer_info(advmap.osdmap);
	return discard_event();
}
{% endhighlight %}

正常情况下，基本上这是第一次产生past_interval的地方。

2） **GetInfo阶段产生past_interval**
{% highlight string %}
PG::RecoveryState::GetInfo::GetInfo(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Peering/GetInfo")
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	pg->generate_past_intervals();
	unique_ptr<PriorSet> &prior_set = context< Peering >().prior_set;
	
	assert(pg->blocked_by.empty());
	
	if (!prior_set.get())
		pg->build_prior(prior_set);
	
	pg->reset_min_peer_features();
	get_infos();
	if (peer_info_requested.empty() && !prior_set->pg_down) {
		post_event(GotInfo());
	}
}
{% endhighlight %}
暂时未知为何要重新计算past_intervals。

3）**Primary OSD恢复期间构建might_have_unfound集合时**
{% highlight string %}
/* Build the might_have_unfound set.
 *
 * This is used by the primary OSD during recovery.
 *
 * This set tracks the OSDs which might have unfound objects that the primary
 * OSD needs. As we receive pg_missing_t from each OSD in might_have_unfound, we
 * will remove the OSD from the set.
 */
void PG::build_might_have_unfound()
{
	assert(might_have_unfound.empty());
	assert(is_primary());
	
	dout(10) << __func__ << dendl;
	
	// Make sure that we have past intervals.
	generate_past_intervals();

	...
}
{% endhighlight %}


#### 1.4.3 PG::past_intervals的清理

PG::past_intervals在运行过程中不可能一直大规模保留，肯定是需要清理的：
{% highlight string %}
/*
 * Trim past_intervals.
 *
 * This gets rid of all the past_intervals that happened before last_epoch_clean.
 */
void PG::trim_past_intervals()
{
	std::map<epoch_t,pg_interval_t>::iterator pif = past_intervals.begin();
	std::map<epoch_t,pg_interval_t>::iterator end = past_intervals.end();
	while (pif != end) {
		if (pif->second.last >= info.history.last_epoch_clean)
			return;
		dout(10) << __func__ << ": trimming " << pif->second << dendl;
		past_intervals.erase(pif++);
		dirty_big_info = true;
	}
}
{% endhighlight %}
从上面的代码可以看出，最后保留的第一个past_interval的last必须要大于等于info.history.last_epoch_clean。

下面我们来什么时候会触发调用trim_past_intervals()呢？
{% highlight string %}
void PG::mark_clean()
{
	// only mark CLEAN if we have the desired number of replicas AND we
	// are not remapped.
	if (actingset.size() == get_osdmap()->get_pg_size(info.pgid.pgid) &&
		up == acting)
	state_set(PG_STATE_CLEAN);
	
	// NOTE: this is actually a bit premature: we haven't purged the
	// strays yet.
	info.history.last_epoch_clean = get_osdmap()->get_epoch();
	
	trim_past_intervals();
	
	if (is_clean() && !snap_trimq.empty())
		queue_snap_trim();
	
	dirty_info = true;
}
{% endhighlight %}
在进入clean状态时，首先会将当前osdmap的版本号赋值给info.history.last_epoch_clean，之后调用trim_past_intervals()来清除掉一些不必要的past_intervals。






## 2. up_thru
大家都知道，OSDMap的作用之一便是维护Ceph集群OSD的状态信息，所以基于此想先提出一个疑问：Ceph集群中有1个osd down了，那么osdmap会发生什么变化？osdmap会更新几次？带着这个问题，本文深入探讨up_thru。

参看:[ceph之up_thru分析](https://ivanzz1001.github.io/records/post/ceph/2017/07/21/ceph-up_thru)

### 2.1 引入up_thru的目的
up_thru的引入是为了解决如下这类极端场景：

比如集群有两个osd(osd.1, osd.2)功能承载一批PG来服务业务IO。如果osd.1在某个时刻down了，并且随后osd.2也down了，再随后osd.1又up了，那么此时osd.1是否能提供服务？

如果osd.1 down掉期间，osd.2有数据更新，那么显然osd.1再次up后是不能服务的；但是如果osd.2没有数据更新，那么osd.1再次up后是可以提供服务的。


### 2.2 up_thru到底是啥？
要想知道up_thru到底是啥，可以先通过其相关数据结构感受一下，如下：
{% highlight string %}
class OSDMap { // osdmap数据结构
    vector<osd_info_t> osd_info; // osd信息 
}

struct osd_info_t {
    epoch_t up_thru;  // up_thru本尊
}
{% endhighlight %}
通过数据结构我们便可以知道，up_thru是作为osd的信息保存在osdmap里的，其类型便是osdmap的版本号(epoch_t)。

既然其是保存在osdmap里面的，那么我们可以通过把osdmap dump出来看看，如下：
<pre>
usrname@hostname:# ceph osd dump
epoch ***
fsid ***
// 如下便看到了up_thru,277便是osdmap的版本号
osd.1 up   in  weight 1 up_from 330 up_thru 277 down_at 328 ... 
osd.2 up   in  weight 1 up_from 329 up_thru 277 down_at 328 ...
</pre>


### 2.3 up_thru的来龙去脉
up_thru的整个生命周期以及是如何起作用的？整个流程是非常长的，为了让文章变得短小精悍一点，与up_thru不是强相关的流程就不加入代码分析了，只是一笔带过。

为了形象说明up_thru的来龙去脉，我们就沿着上文那两个OSD的例子展开说。

#### 3.3.1 up_thru的更新

###### osd.1挂了后发生了什么？

osd.1挂了之后，整个集群反应如下：

* osd上报mon

osd.1挂了后，或是osd.1主动上报，或是其他osd向mon上报osd.1挂了，此时mon已经感知到osd.1挂了

* mon更新osdmap

该osdmap中标记该down掉的osd状态为down，并将新的osdmap发送给osd.2

* osd.2收到新的osdmap(第一次)

osd.2收到新的osdmap后，相关PG开始peering，若PG发现需要向mon申请更新up_thru信息，那么PG状态变为WaitUpThru;

osd.2判断是否需要向mon申请更新up_thru消息，若需要，则向mon发送该信息。

* mon更新osdmap

mon收到消息后，更新osdmap里面osd.2的up_thru信息，并将新的osdmap发送给osd.2

* osd.2收到新的osdmap(第二次）

osd.2收到新的osdmap后，相关PG开始peering，相关PG状态由WaitUpThru变为active，可以开始服务业务IO。


###### 具体的up_thru更新相关流程如下

以下是osd.2收到新的osdmap(第一次)后相关操作：

1） PG判断其对应的主OSD是否需要更新up_thru

PG开始peering，以下是peering相关函数：
{% highlight string %}
PG::build_prior(std::unique_ptr<PriorSet> &prior_set) 
{

    /* 这里需要引入一个概念past_interval:
    past_interval是osdmap版本号epoch 的一个序列。在该序列内一个PG的 acting set 和 up set不会变化。

    如果osd.1挂了，那么pg的up set肯定发生变化了，也即产生了一个新的past_interval，那么此时会更新info.history.same_interval_since为新的osdmap版本号
    因为same_interval_since表示的是最近一个past_interval的第一个osdmap的版本号

    所以如果osd.1挂了后，此时这个条件肯定满足
    */
    if (get_osdmap()->get_up_thru(osd->whoami) < info.history.same_interval_since) 
    {
        need_up_thru = true;
    } else {
        need_up_thru = false;
    }
}
{% endhighlight %}

2) osd判断其本次做Peering的所有PG中是否有需要申请up_thru的。若有，该OSD向monitor申请up_thru
{% highlight string %}
OSD::process_peering_events(  
    const list<PG*> &pgs, //本次peering的所有PG
    ThreadPool::TPHandle &handle)
{
    bool need_up_thru = false;
    epoch_t same_interval_since = 0;
    for (list<PG*>::const_iterator i = pgs.begin();i != pgs.end();++i) 
    {
        ......
        need_up_thru = pg->need_up_thru || need_up_thru;
        //获得所有PG中最大的info.history.same_interval_since
        same_interval_since = MAX(pg->info.history.same_interval_since,same_interval_since);
    }

    if (need_up_thru) //只要有一个PG需要申请up_thru,则申请
        queue_want_up_thru(same_interval_since); 
}

OSD::queue_want_up_thru(epoch_t want) 
{
    epoch_t cur = osdmap->get_up_thru(whoami); //获得当前osdmap中该osd对应的osdmap版本
    if (want > up_thru_wanted)  //如果所有PG中最大的info.history.same_interval_since大于当前的，说明该osd需要申请up_thru
    {
        up_thru_wanted = want;
        send_alive();
    }
}

OSD::send_alive()
{
    // 向mon发送更新up_thru的消息
    monc->send_mon_message(new MOSDAlive(osdmap->get_epoch(), up_thru_wanted));   
}
{% endhighlight %}

3) monitor接受osd的申请
{% highlight string %}
OSDMonitor::preprocess_alive(MonOpRequestRef op)
{
    //如果最新的osdmap中该osd的up_thru大于等于osd想要申请的。那么monitor不需要决议了，只需要把osd中缺失的所有osdmap发送给它
    if (osdmap.get_up_thru(from) >= m->want) 
    {
        _reply_map(op, m->version);
        return true; 
    }
    return false; //返回false，monitor还需要继续决议
}

// 若monitor还需要继续决议，则继续往下走
OSDMonitor::prepare_alive(MonOpRequestRef op)
{

    update_up_thru(from, m->version);
    //决议完成后调用回调C_ReplyMap。把新的osdmap发给osd.2        
    wait_for_finished_proposal(op, new C_ReplyMap(this, op, m->version));
}

OSDMonitor::update_up_thru(int from, epoch_t up_thru)
{
    pending_inc.new_up_thru[from] = up_thru;
}

// monitor决议完之后，最终会调用下面的函数进行应用，更新OSDMap
OSDMap::apply_incremental(const Incremental &inc)
{
    //更新osdmap中该osd对应的up_thru字段
    for (map<int32_t,epoch_t>::const_iterator i = inc.new_up_thru.begin();i != inc.new_up_thru.end();
    {
        osd_info[i->first].up_thru = i->second;
    }
}
{% endhighlight %}


###### PG状态的转变
osd.2收到新的osdmap(第一次)后，PG进入peering后如果需要mon更新up_thru，其会先进入NeedUpThru状态：
{% highlight string %}
PG::RecoveryState::GetMissing::GetMissing（）
{
    if (pg->need_up_thru)  //如果该PG的need_up_thru标志被置位。这是在上文的build_prior函数中操作的
    {
        post_event(NeedUpThru()); //pg进入WaitUpThru状态
        return;
    }
}
{% endhighlight %}

等到osd.2收到新的osdmap(第二次)后，该osdmap也即是更新了up_thru后的新osdmap，PG当前处于WaitUpThru状态，当处于WaitUpThru状态的PG收到OSDMap，会把pg->need_up_thru置为false，如下：
{% highlight string %}
PG::RecoveryState::Peering::react(const AdvMap& advmap) 
{
    /*
    当本身已经处于peering时，收到Advmap,会先看下是否会对prio有影响，只有有的情况下，才会进入reset
    像本身已经处于wait_up_thru状态的PG则不会进入reset，它会执行下面的adjust_need_up_thru，然后把PG->need_up_thru标记设置为false。
    */
    if (prior_set.get()->affected_by_map(advmap.osdmap, pg)) 
    {
        post_event(advmap);
        return transit< Reset >();
    }
    // 把PG->need_up_thru标记设置为false
    pg->adjust_need_up_thru(advmap.osdmap);
    return forward_event();
}

bool PG::adjust_need_up_thru(const OSDMapRef osdmap)
{
	epoch_t up_thru = get_osdmap()->get_up_thru(osd->whoami);

	if (need_up_thru &&up_thru >= info.history.same_interval_since) {
		dout(10) << "adjust_need_up_thru now " << up_thru << ", need_up_thru now false" << dendl;
		need_up_thru = false;
		return true;
	}
	return false;
}
{% endhighlight %}
wait_up_thru状态的PG收到ActMap事件后，由于need_up_thru标记已经设为false了，所以开始进入active状态，进入active状态后，也就可以开始io服务了：
{% highlight string %}
boost::statechart::result PG::RecoveryState::WaitUpThru::react(const ActMap& am)
{
	PG *pg = context< RecoveryMachine >().pg;
	if (!pg->need_up_thru) {
		post_event(Activate(pg->get_osdmap()->get_epoch()));
	}
	return forward_event();
}
{% endhighlight %}

### 2.4 up_thru应用
上文描述了当osd.1挂掉后，整个集群的响应，其中包括up_thru的更新。本节描述一下up_thru的应用，也即本文开始的那个例子，当osd.1再次up后，其能否提供服务？

###### 场景说明
假设初始时刻，也即```osd.1```、```osd.2```都健康，对应PG都是active + clean时osdmap的版本号epoch是80，此时osd.1以及osd.2的up_thru必然都小于80。

1）**不能提供服务的场景**

osd.1 down掉，osdmap版本号epoch变为81；

如果osd.2向mon申请更新up_thru成功，此时osdmap版本号epoch为82(osd.2的up_thru变为81），由于osd.2收到新的osdmap后，PG的状态就可以变为active，也即可以提供io服务了，所以如果up_thru更新成功了，可以判定osd.2有新的写入了（当然，可能存在虽然up_thru更新了，但是osd.2进入到active之前就挂了，那也是没有数据更新的）；

osd.2 down掉，osdmap版本号变为83；

osd.1进程up，osdmap版本号变为84；

2）**能提供IO服务的场景**


osd.1 down掉，osdmap版本号epoch变为81；

osd.2没有peering成功，没有向monitor上报相应的up_thru，没有更新操作（比如，osd.2向mon申请更新up_thru失败，在上报之前就挂了)

osd.2 down掉，osdmap版本号epoch变为82；

osd.1 进程up后，osdmap版本号变为83；

###### IO能否提供服务的代码逻辑判断

通过如下流程判断对应PG能否提供IO服务：

1） **生成past_interval**

在peering阶段会调用如下函数生成past_interval（osd启动时也会从底层读到相关信息进而生成past_interval):
{% highlight string %}
bool pg_interval_t::check_new_interval( ... )
{
	if (is_new_interval(...)) {
		if (num_acting &&i.primary != -1 &&num_acting >= old_pg_pool.min_size &&(*could_have_gone_active)(old_acting_shards)) {
 			...
		}
	}
}
{% endhighlight %}

这里还是以上述两种场景来描述past_interval的更新：

* 不能提供IO服务的场景

如上所述，osdmap版本号epoch分别是80~84，这几个osdmap版本号会对应如下4个past_interval: 
<pre>
{80}、{81，82}、{83}、{84}
</pre>

并且在{81,82}这个past_interval中，maybe_went_rw会被设置为true，因为在osdmap版本号为82时，osd.2对应的up_thru为81，这样就等于本past_interval的第一个osdmap版本号81.


* 能提供IO服务的场景

如上所述，osdmap版本号epoch分别是80~83，这几个osdmap版本号分别对应如下4个past_interval:
<pre>
{80}、{81}、{82}、{83}
</pre>

2) **根据past_interval判断IO能否服务**

这里以不能服务IO的场景为例，如下：
{% highlight string %}
// 当osd.1再次up后，peering阶段调用该函数
PG::build_prior(std::unique_ptr<PriorSet> &prior_set)
{
    prior_set.reset(
      new PriorSet(
        pool.info.ec_pool(),
        get_pgbackend()->get_is_recoverable_predicate(),
        *get_osdmap(),
        past_intervals,
        up,
        acting,
        info,
        this));
        PriorSet &prior(*prior_set.get());

    // 如果prior.pg_down为true，pg状态被设置为down，此时pg状态为down+peering
    if (prior.pg_down)
    {
        state_set(PG_STATE_DOWN);
    }
}

PriorSet::PriorSet(map<epoch_t, pg_interval_t> &past_intervals，...)
{
    //遍历所有的past_intervals{80},{81,82},{83},{84}
    for (map<epoch_t,pg_interval_t>::const_reverse_iterator p = past_intervals.rbegin();p != past_intervals.rend(); ++p)
    {
        const pg_interval_t &interval = p->second; //取得pg_interval_t

        //还是上面的例子，当这里运行到{81,82}这个interval时，由于maybe_went_rw为true了，故而无法continue退出，还要继续走
        if (!interval.maybe_went_rw) 
           continue;

		for (unsigned i = 0; i < interval.acting.size(); i++) 
        {
            int o = interval.acting[i]; //acting列表中的osd
            if (osdmap.is_up(o)) //如果该OSD处于up状态，就加入到up_now列表中，同时加到probe列表。用于获取权威日志以及后续数据恢复
            {
                probe.insert(so);
                up_now.insert(so);
            }
            else if (!pinfo) 
            {
                //该列表保存了所有down列表
                down.insert(o);   
            }
            else
            {
                // 该列表保存了所有down列表
                // 当这里运行到{81,82}这个interval时，
                // 由于这个past_interval中的osd.2此时down了，所以会走到这里
                down.insert(o);
                any_down_now = true; 
            }
		}

		// peering由于在线的osd不足，所以不能够继续进行。母函数会根据这个值从而把PG状态设置为DOWN 
		// 当这里运行到{81,82}这个interval时，由于这个past_interval中的唯一的一个osd.2此时down了，所以pg_down肯定会被置为true)
		if (!(*pcontdec)(up_now) && any_down_now) {
			// fixme: how do we identify a "clean" shutdown anyway?
			dout(10) << "build_prior  possibly went active+rw, insufficient up;"<< " including down osds" << dendl;
			for (vector<int>::const_iterator i = interval.acting.begin();i != interval.acting.end();++i) {
				if (osdmap.exists(*i) &&   // if it doesn't exist, we already consider it lost.
				  osdmap.is_down(*i)) {

					pg_down = true;
		
					// make note of when any down osd in the cur set was lost, so that
					// we can notice changes in prior_set_affected.
					blocked_by[*i] = osdmap.get_info(*i).lost_at;
				}
			}
		}

    }
}
{% endhighlight %}
综上所述，当osd.1再次up后，最终到{84}这个interval时，根据上面一系列函数调用，此时的PG状态最终会变为peering+down，此时便无法响应服务IO了。

### 3.5 写在后面
在本章的开头我们提出了一个疑问，即Ceph集群中有一个osd down了，那么osdmap会发生什么变化？osdmap会更新几次？

答案是：不一定，主要分如下两种情况

* 该挂掉的OSD与其他OSD无共同承载的PG

此时，OSDMap只会更新一次，变化便是osdmap中该OSD的状态从up更新为了down。因为都不存在相关PG，也就不存在PG Peering，也就没有up_thru更新了，所以osdmap是变化1次。

* 该挂掉的OSD与其他OSD有功能承载的PG

此时osdmap会至少更新2次，其中第一次是更新osdmap中osd的状态，第二次便是更新相关osd的up_thru。

这里通过实例来说明osdmap变化情况：

1） 集群初始状态信息如下
<pre>
# ceph osd tree
ID CLASS WEIGHT  TYPE NAME                       STATUS REWEIGHT PRI-AFF
-1       1.74658 root default
-5       0.87329     host pubt2-ceph1-dg-163-org
 0   ssd 0.87329         osd.0                       up  1.00000 1.00000
-3       0.87329     host pubt2-ceph2-dg-163-org
 1   ssd 0.87329         osd.1                       up  1.00000 1.00000
# ceph osd dump | grep epoch
epoch 416                                                // osdmap版本号
</pre>


2) down掉osd.0后集群信息如下
<pre>
# ceph osd tree 
ID CLASS WEIGHT  TYPE NAME                       STATUS REWEIGHT PRI-AFF
-1       1.74658 root default
-5       0.87329     host pubt2-ceph1-dg-163-org
 0   ssd 0.87329         osd.0                     down  1.00000 1.00000
-3       0.87329     host pubt2-ceph2-dg-163-org
 1   ssd 0.87329         osd.1                       up  1.00000 1.00000
# ceph osd dump | grep epoch
epoch 418                                  // osdmap版本号
</pre>

如上所示，当down掉osd.0后，osdmap版本号增加了2个。那我们分别看看这两个osdmap有啥变化，可以通过```ceph osd dump epoch```把相应的osdmap打印出来：
<pre>
# ceph osd dump 416
# ceph osd dump 417
# ceph osd dump 418
</pre>
通过对比，我们可以发现```osdmap.416```与```osdmap.417```的差别就是osd.0的状态变化；```osdmap.417```与```osdmap.418```的差别就是更新了osd.1的up_thru。



<br />
<br />

**[参看]**

1. [ceph osd heartbeat 分析](https://blog.csdn.net/ygtlovezf/article/details/72330822)

2. [boost官网](https://www.boost.org/doc/)

3. [在线编译器](https://blog.csdn.net/weixin_39846364/article/details/112328477)

4. [boost在线编译器](http://cpp.sh/)

5. [ceph官网](https://docs.ceph.com/en/latest/dev/internals/)

6. [PEERING](https://docs.ceph.com/en/latest/dev/peering/)
<br />
<br />
<br />

