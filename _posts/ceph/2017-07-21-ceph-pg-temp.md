---
layout: post
title: ceph之pg_temp分析
tags:
- ceph
categories: ceph
description: ceph之pg_temp分析
---

本文从源代码出发，分析产生pg_temp的场景，及其要解决的问题、


<!-- more -->

## 1. pg_temp的产生

### 1.1 pg_temp请求的发送
在OSD::process_peering_events()函数的最后会调用OSDService::send_pg_temp()来发送MSG_OSD_PGTEMP请求到OSDMonitor，如下：
{% highlight string %}
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...

	service.send_pg_temp();
}
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
{% endhighlight %}

由上述代码可知，send_pg_temp()函数会对pg_temp_wanted中的PG打包成一个MOSDPGTemp请求，发送到OSDMonitor:
{% highlight string %}
void OSDService::_sent_pg_temp()
{
	for (map<pg_t,vector<int> >::iterator p = pg_temp_wanted.begin();p != pg_temp_wanted.end();++p)
		pg_temp_pending[p->first] = p->second;
	pg_temp_wanted.clear();
}
{% endhighlight %}
_sent_pg_temp()函数将已经发送过MOSDPGTemp请求的PG加入到pg_temp_pending中，同时清空pg_temp_wanted。


此外，通过代码上下文我们知道是通过调用queue_want_pg_temp()来向pg_temp_wanted中来添加数据的：
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


### 1.2 pg_temp产生的场景
查询OSDService::queue_want_pg_temp()的调用，我们发现主要有如下两个地方调用：

* PG::choose_acting()

* PG::start_peering_interval()

###### 1.2.1 函数choose_acting()
{% highlight string %}
PG::RecoveryState::GetLog::GetLog(my_context ctx)
  : my_base(ctx),
    NamedState(
      context< RecoveryMachine >().pg->cct, "Started/Primary/Peering/GetLog"),
    msg(0)
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	
	// adjust acting?
	if (!pg->choose_acting(auth_log_shard, false,&context< Peering >().history_les_bound)) {
		if (!pg->want_acting.empty()) {
			post_event(NeedActingChange());
		} else {
			post_event(IsIncomplete());
		}

		return;
	}

	....
}

bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound)
{
	...

	map<pg_shard_t, pg_info_t>::const_iterator auth_log_shard =
		find_best_info(all_info, restrict_to_up_acting, history_les_bound);
	
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
}
{% endhighlight %}
由上面代码，产生pg_temp的场景有：

* 获取PG权威日志失败，且当前的up set与acting set不一致；

* 成功获取PG权威日志，但通过PG::calc_replicated_acting()所计算出来的want与acting不一致。此时，若want等于up，则向OSDMonitor请求删除pg_temp（即不需要pg_temp)，否则向OSDMonitor请求创建pg_temp；

> 注： 当前PG最新的up set、acting set已经在PG::init_primary_up_acting()函数中计算得到，最原始是在OSD::advance_pg()计算得到然后通过AdvMap事件传递过来


对于第一种情况，通常是由于backfill不完整导致的。此时如果up set与acting set不一致，则进入WaitActingChange状态，否则进入InComplete状态。

对于第二种情况，我们先来看一下PG::calc_replicated_acting()函数：
{% highlight string %}
void PG::calc_replicated_acting(
  map<pg_shard_t, pg_info_t>::const_iterator auth_log_shard,
  unsigned size,
  const vector<int> &acting,
  pg_shard_t acting_primary,
  const vector<int> &up,
  pg_shard_t up_primary,
  const map<pg_shard_t, pg_info_t> &all_info,
  bool compat_mode,
  bool restrict_to_up_acting,
  vector<int> *want,
  set<pg_shard_t> *backfill,
  set<pg_shard_t> *acting_backfill,
  pg_shard_t *want_primary,
  ostream &ss)
{
	ss << "calc_acting newest update on osd." << auth_log_shard->first
		<< " with " << auth_log_shard->second
		<< (restrict_to_up_acting ? " restrict_to_up_acting" : "") << std::endl;
	pg_shard_t auth_log_shard_id = auth_log_shard->first;
	
	// select primary
	map<pg_shard_t,pg_info_t>::const_iterator primary;
	if (up.size() &&
	  !all_info.find(up_primary)->second.is_incomplete() &&
	  all_info.find(up_primary)->second.last_update >=
	  auth_log_shard->second.log_tail) {
		ss << "up_primary: " << up_primary << ") selected as primary" << std::endl;
		primary = all_info.find(up_primary); // prefer up[0], all thing being equal
	} else {
		assert(!auth_log_shard->second.is_incomplete());
		ss << "up[0] needs backfill, osd." << auth_log_shard_id << " selected as primary instead" << std::endl;
		primary = auth_log_shard;
	}

	ss << "calc_acting primary is osd." << primary->first
		<< " with " << primary->second << std::endl;
	*want_primary = primary->first;
	want->push_back(primary->first.osd);
	acting_backfill->insert(primary->first);
	unsigned usable = 1;

	// select replicas that have log contiguity with primary.
	// prefer up, then acting, then any peer_info osds 
	for (vector<int>::const_iterator i = up.begin();i != up.end();++i) {
		pg_shard_t up_cand = pg_shard_t(*i, shard_id_t::NO_SHARD);
		if (up_cand == primary->first)
			continue;

		const pg_info_t &cur_info = all_info.find(up_cand)->second;
		if (cur_info.is_incomplete() ||
		  cur_info.last_update < MIN(
		  primary->second.log_tail,
		  auth_log_shard->second.log_tail)) {

			/* We include auth_log_shard->second.log_tail because in GetLog,
			* we will request logs back to the min last_update over our
			* acting_backfill set, which will result in our log being extended
			* as far backwards as necessary to pick up any peers which can
			* be log recovered by auth_log_shard's log */
			* 
			ss << " shard " << up_cand << " (up) backfill " << cur_info << std::endl;
			if (compat_mode) {
				if (backfill->empty()) {
					backfill->insert(up_cand);
					want->push_back(*i);
					acting_backfill->insert(up_cand);
				}
			} else {
				backfill->insert(up_cand);
				acting_backfill->insert(up_cand);
			}
		} else {
			want->push_back(*i);
			acting_backfill->insert(up_cand);
			usable++;
			ss << " osd." << *i << " (up) accepted " << cur_info << std::endl;
		}
	}

	// This no longer has backfill OSDs, but they are covered above.
	for (vector<int>::const_iterator i = acting.begin();i != acting.end();++i) {
		pg_shard_t acting_cand(*i, shard_id_t::NO_SHARD);
		if (usable >= size)
			break;
	
		// skip up osds we already considered above
		if (acting_cand == primary->first)
			continue;
		vector<int>::const_iterator up_it = find(up.begin(), up.end(), acting_cand.osd);
		if (up_it != up.end())
			continue;
	
		const pg_info_t &cur_info = all_info.find(acting_cand)->second;
		if (cur_info.is_incomplete() ||
		  cur_info.last_update < primary->second.log_tail) {
			ss << " shard " << acting_cand << " (stray) REJECTED "<< cur_info << std::endl;
		} else {
			want->push_back(*i);
			acting_backfill->insert(acting_cand);
			ss << " shard " << acting_cand << " (stray) accepted "<< cur_info << std::endl;
			usable++;
		}
	}

	if (restrict_to_up_acting) {
		return;
	}
	for (map<pg_shard_t,pg_info_t>::const_iterator i = all_info.begin();i != all_info.end();++i) {
		if (usable >= size)
			break;
	
		// skip up osds we already considered above
		if (i->first == primary->first)
			continue;
		vector<int>::const_iterator up_it = find(up.begin(), up.end(), i->first.osd);
		if (up_it != up.end())
			continue;
		vector<int>::const_iterator acting_it = find(
			acting.begin(), acting.end(), i->first.osd);
		if (acting_it != acting.end())
			continue;
	
		if (i->second.is_incomplete() ||
		  i->second.last_update < primary->second.log_tail) {
			ss << " shard " << i->first << " (stray) REJECTED "<< i->second << std::endl;
		} else {
			want->push_back(i->first.osd);
			acting_backfill->insert(i->first);
			ss << " shard " << i->first << " (stray) accepted "<< i->second << std::endl;
			usable++;
		}
	}
}
{% endhighlight %}

want的计算方式如下：

1） 计算want_primary

如果up set非空，并且up_primary当前处于complete状态，且其pg log日志与权威日志有重叠，那么就将up_primary赋值给want_primary，否则就将拥有权威日志的osd赋值给want_primary.

2) 遍历up set列表，如果对应的副本是complete状态，且与want_primary或auth_log_shard有日志重叠，则将该副本osd加入到want；

3）遍历acting列表，如果对应的副本是complete状态，且与want_primary有日志重叠，则将该副本osd加入到want;

4) 遍历该PG获取到的所有pg info，如果对应的副本是complete状态，且与want_primary有日志重叠，则将该副本加入到want

综上，这里求出的want列表中的元素基本是要能够通过日志来进行恢复的，而acting列表中可能还包括一些要backfill的OSD。



----------
针对第二种情况，我们总结一下： 如果want与acting不相等，且want不等于up，那么会产生pg_temp。



###### 1.2.2 函数start_peering_interval()
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

/* Called before initializing peering during advance_map */
void PG::start_peering_interval(
  const OSDMapRef lastmap,
  const vector<int>& newup, int new_up_primary,
  const vector<int>& newacting, int new_acting_primary,
  ObjectStore::Transaction *t)
{
	...

	init_primary_up_acting(
		newup,
		newacting,
		new_up_primary,
		new_acting_primary);

	...

	if (acting.empty() && !up.empty() && up_primary == pg_whoami) {
		dout(10) << " acting empty, but i am up[0], clearing pg_temp" << dendl;
		osd->queue_want_pg_temp(info.pgid.pgid, acting);
	}
}
{% endhighlight %}
上面代码是将pg_temp清除。

## 2. OSDMonitor端的处理
OSDMonitor接收到MSG_OSD_PGTEMP消息后，按如下方式进行处理：
{% highlight string %}
void Monitor::_ms_dispatch(Message *m){
	...

	if ((is_synchronizing() || (s->global_id == 0 && !exited_quorum.is_zero())) &&
	   !src_is_mon && m->get_type() != CEPH_MSG_PING) {
		waitlist_or_zap_client(op);
	} else {
		dispatch_op(op);
	}
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
bool OSDMonitor::preprocess_query(MonOpRequestRef op)
{
	....
	switch (m->get_type()) {
	...

	case MSG_OSD_PGTEMP:
		return preprocess_pgtemp(op);

	...
	}
}
{% endhighlight %}

接下来我们来看对```MSG_OSD_PGTEMP```消息的处理:

* 预处理阶段
{% highlight string %}
bool OSDMonitor::preprocess_pgtemp(MonOpRequestRef op)
{
	MOSDPGTemp *m = static_cast<MOSDPGTemp*>(op->get_req());
	dout(10) << "preprocess_pgtemp " << *m << dendl;
	vector<int> empty;
	int from = m->get_orig_source().num();
	size_t ignore_cnt = 0;
	
	// check caps
	MonSession *session = m->get_session();
	if (!session)
		goto ignore;
	if (!session->is_capable("osd", MON_CAP_X)) {
		dout(0) << "attempt to send MOSDPGTemp from entity with insufficient caps "<< session->caps << dendl;
		goto ignore;
	}
	
	if (!osdmap.is_up(from) ||
	  osdmap.get_inst(from) != m->get_orig_source_inst()) {
		dout(7) << "ignoring pgtemp message from down " << m->get_orig_source_inst() << dendl;
		goto ignore;
	}

	for (map<pg_t,vector<int32_t> >::iterator p = m->pg_temp.begin(); p != m->pg_temp.end(); ++p) {
		dout(20) << " " << p->first<< (osdmap.pg_temp->count(p->first) ? (*osdmap.pg_temp)[p->first] : empty)<< " -> " << p->second << dendl;
	
		// does the pool exist?
		if (!osdmap.have_pg_pool(p->first.pool())) {
			/*
			* 1. If the osdmap does not have the pool, it means the pool has been
			*    removed in-between the osd sending this message and us handling it.
			* 2. If osdmap doesn't have the pool, it is safe to assume the pool does
			*    not exist in the pending either, as the osds would not send a
			*    message about a pool they know nothing about (yet).
			* 3. However, if the pool does exist in the pending, then it must be a
			*    new pool, and not relevant to this message (see 1).
			*/
			dout(10) << __func__ << " ignore " << p->first << " -> " << p->second<< ": pool has been removed" << dendl;
			ignore_cnt++;
			continue;
		}

    	int acting_primary = -1;
		osdmap.pg_to_up_acting_osds(
			p->first, nullptr, nullptr, nullptr, &acting_primary);
		if (acting_primary != from) {
			/* If the source isn't the primary based on the current osdmap, we know
			* that the interval changed and that we can discard this message.
			* Indeed, we must do so to avoid 16127 since we can't otherwise determine
			* which of two pg temp mappings on the same pg is more recent.
			*/
			dout(10) << __func__ << " ignore " << p->first << " -> " << p->second<< ": primary has changed" << dendl;
			ignore_cnt++;
			continue;
		}
		
		// removal?
		if (p->second.empty() && (osdmap.pg_temp->count(p->first) ||
		  osdmap.primary_temp->count(p->first)))
			return false;

		// change?
		//  NOTE: we assume that this will clear pg_primary, so consider
		//        an existing pg_primary field to imply a change
		if (p->second.size() && (osdmap.pg_temp->count(p->first) == 0 ||
		  (*osdmap.pg_temp)[p->first] != p->second ||
		  osdmap.primary_temp->count(p->first)))
			return false;
	}

	// should we ignore all the pgs?
	if (ignore_cnt == m->pg_temp.size())
		goto ignore;
	
	dout(7) << "preprocess_pgtemp e" << m->map_epoch << " no changes from " << m->get_orig_source_inst() << dendl;
	_reply_map(op, m->map_epoch);
	return true;
	
ignore:
	return true;
}
{% endhighlight %}

预处理阶段主要是先进行一些基本的检查：

* 如果pg_temp请求不是来自于OSD，则忽略

* 如果pg_temp请求的OSD当前不是up状态，则忽略

* 遍历MSG_OSD_PGTEMP请求中的每一个PG，对其进行相应的检查

  - 如果PG所对应的pool不存在，则忽略

  - 根据当前osdmap计算出的acting_primary与from不是同一个OSD的话，则忽略该PG的处理
  
  - 如果PG所申请的pg_temp为空，且osdmap当前对应的pg_temp不为空，则说明需要删除当前osdmap的pg_temp，返回false，已进一步处理

* 后续处理阶段
{% highlight string %}
bool OSDMonitor::prepare_pgtemp(MonOpRequestRef op)
{
	op->mark_osdmon_event(__func__);
	MOSDPGTemp *m = static_cast<MOSDPGTemp*>(op->get_req());
	int from = m->get_orig_source().num();
	dout(7) << "prepare_pgtemp e" << m->map_epoch << " from " << m->get_orig_source_inst() << dendl;

	for (map<pg_t,vector<int32_t> >::iterator p = m->pg_temp.begin(); p != m->pg_temp.end(); ++p) {
		uint64_t pool = p->first.pool();
		if (pending_inc.old_pools.count(pool)) {
			dout(10) << __func__ << " ignore " << p->first << " -> " << p->second<< ": pool pending removal" << dendl;
			continue;
		}
		if (!osdmap.have_pg_pool(pool)) {
			dout(10) << __func__ << " ignore " << p->first << " -> " << p->second<< ": pool has been removed" << dendl;
			continue;
		}

		pending_inc.new_pg_temp[p->first] = p->second;
	
		// unconditionally clear pg_primary (until this message can encode
		// a change for that, too.. at which point we need to also fix
		// preprocess_pg_temp)
		if (osdmap.primary_temp->count(p->first) ||
		  pending_inc.new_primary_temp.count(p->first))
			pending_inc.new_primary_temp[p->first] = -1;
	}
	
	// set up_thru too, so the osd doesn't have to ask again
	update_up_thru(from, m->map_epoch);
	
	wait_for_finished_proposal(op, new C_ReplyMap(this, op, m->map_epoch));
	return true;
}
{% endhighlight %}
这里注意到，首先会调用update_up_thru()来建立一个新的up_thru，然后发起一个新的proposal，提交表决，表决通过后将响应返回给OSD。

## 3. 总结
pg_temp产生的主要原因是：根据osdmap产生的acting set当前不能简单的通过pg log来进行恢复，则此时通过申请pg_temp来暂时担负起这一时期的工作。


<br />
<br />

**[参看]:**




<br />
<br />
<br />