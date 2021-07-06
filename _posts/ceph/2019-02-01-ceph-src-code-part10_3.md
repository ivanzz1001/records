---
layout: post
title: PG各字段含义介绍
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

PG的Peering过程是十分复杂的，相关的代码实现也相当冗长。这里我们从侧面出发，介绍一下PG源代码实现中的一些重要字段，弄清其含义及用途之后也有利于我们理解PG的Peering操作：


<!-- more -->
{% highlight string %}
class PG{
public:
  eversion_t  last_update_ondisk;                          // last_update that has committed; ONLY DEFINED WHEN is_active()
  eversion_t  last_complete_ondisk;                        // last_complete that has committed.
  eversion_t  last_update_applied;	
  
// primary state
public:
  pg_shard_t primary;
  pg_shard_t pg_whoami;
  pg_shard_t up_primary;
  vector<int> up, acting, want_acting;
  set<pg_shard_t> actingbackfill, actingset, upset;
  map<pg_shard_t,eversion_t> peer_last_complete_ondisk;
  eversion_t  min_last_complete_ondisk;                      // up: min over last_complete_ondisk, peer_last_complete_ondisk
  eversion_t  pg_trim_to;  

  set<int> blocked_by;                                      //< osds we are blocked by (for pg stats)
  
  
  
// [primary only] content recovery state
protected:
  struct PriorSet {
    const bool ec_pool;
    set<pg_shard_t> probe;                                  // current+prior OSDs we need to probe.
    set<int> down;                                          // down osds that would normally be in @a probe and might be interesting.
    map<int, epoch_t> blocked_by;                           // current lost_at values for any OSDs in cur set for which (re)marking them lost would affect cur set

    bool pg_down;                                          // some down osds are included in @a cur; the DOWN pg state bit should be set.
    boost::scoped_ptr<IsPGRecoverablePredicate> pcontdec;
    PriorSet(bool ec_pool,
	     IsPGRecoverablePredicate *c,
	     const OSDMap &osdmap,
	     const map<epoch_t, pg_interval_t> &past_intervals,
	     const vector<int> &up,
	     const vector<int> &acting,
	     const pg_info_t &info,
	     const PG *debug_pg=NULL);

    bool affected_by_map(const OSDMapRef osdmap, const PG *debug_pg=0) const;
  };

};
{% endhighlight %}

下面我们分别介绍这些字段。


## 1. PG重要字段分析

### 1.1 PG.primary

在如下几种情况下，会计算PG的primary，并调用函数PG::init_primary_up_acting()来初始化primary、up set、acting set信息:

* OSD启动时，加载已存在的PG

* 创建新的PG

* PG分裂产生新的PG

* PG进行Peering操作

下面我们对这些情况进行介绍：


###### 1.1.1 OSD启动加载已存在PG
在OSD启动时，会调用如下函数来加载PG：
{% highlight string %}
int OSD::init()
{
	...

	osdmap = get_map(superblock.current_epoch);

	...

	load_pgs();
	...
}
void OSD::load_pgs()
{
	...

	for (vector<coll_t>::iterator it = ls.begin();it != ls.end();++it) {
	
		...
	
		// generate state for PG's current mapping
		int primary, up_primary;
		vector<int> acting, up;
		
		pg->get_osdmap()->pg_to_up_acting_osds(
			pgid.pgid, &up, &up_primary, &acting, &primary);
	
		pg->init_primary_up_acting(
			up,
			acting,
			up_primary,
			primary);
			
		int role = OSDMap::calc_pg_role(whoami, pg->acting);
		pg->set_role(role);
	
	
		...
	}

	...
}


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
上面我们看到会调用OSDMap::pg_to_up_acting_osds()来计算当前PG的up osds以及acting osds。我们```必须```使用acting set来进行数据的映射；但有些用户也会发现up set也是很有用的，通过对比acting set与up set就可以知道pg_temp是什么。

>注：OSD启动时，并不是直接去monitor拿最新的osdmap，而是从superblock中读取上一次所保存的osdmap。

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


>注： 在PG::init_primary_up_acting()函数中，将acting primary设置给了PG::primary。


###### 1.1.2 PG进行Peering操作
在PG进行peering操作过程中，也会导致重新计算primary、acting set以及up set:
{% highlight string %}
bool OSD::advance_pg(
	epoch_t osd_epoch, PG *pg,
	ThreadPool::TPHandle &handle,
	PG::RecoveryCtx *rctx,
	set<boost::intrusive_ptr<PG> > *new_pgs)
{
	...

	for (;next_epoch <= osd_epoch && next_epoch <= max;++next_epoch) {
		...

		vector<int> newup, newacting;
		int up_primary, acting_primary;
		
		nextmap->pg_to_up_acting_osds(
			pg->info.pgid.pgid,
			&newup, &up_primary,
			&newacting, &acting_primary);

		pg->handle_advance_map(
			nextmap, lastmap, newup, up_primary,
			newacting, acting_primary, rctx);
	}
}

void PG::handle_advance_map(
	OSDMapRef osdmap, OSDMapRef lastmap,
	vector<int>& newup, int up_primary,
	vector<int>& newacting, int acting_primary,
	RecoveryCtx *rctx)
{
	...

	AdvMap evt(
		osdmap, lastmap, newup, up_primary,
		newacting, acting_primary);

  	recovery_state.handle_event(evt, rctx);

	...
}

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
}
{% endhighlight %}
从上面我们看到当有新的OSDMap产生时，在OSD::advance_pg()函数中又计算出了新的upset与acting set。


那具体是在什么地方会导致up set与acting set不一致呢？我们来看peering过程中的GetLog事件：
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
	if (!pg->choose_acting(auth_log_shard, false,
		&context< Peering >().history_les_bound)) {
		
		if (!pg->want_acting.empty()) 
		{
			post_event(NeedActingChange());
		} 
		else {
			post_event(IsIncomplete());
		}
		return;
	}

	// am i the best?
	if (auth_log_shard == pg->pg_whoami) {
		post_event(GotLog());
		return;
	}

	const pg_info_t& best = pg->peer_info[auth_log_shard];

	// am i broken?
	if (pg->info.last_update < best.log_tail) {
		dout(10) << " not contiguous with osd." << auth_log_shard << ", down" << dendl;
		post_event(IsIncomplete());
		return;
	}

	// how much log to request?
	eversion_t request_log_from = pg->info.last_update;
	assert(!pg->actingbackfill.empty());
	for (set<pg_shard_t>::iterator p = pg->actingbackfill.begin();p != pg->actingbackfill.end();++p) {
		if (*p == pg->pg_whoami) continue;
		
		pg_info_t& ri = pg->peer_info[*p];
		if (ri.last_update >= best.log_tail && ri.last_update < request_log_from)
			request_log_from = ri.last_update;
	}

	// how much?
	dout(10) << " requesting log from osd." << auth_log_shard << dendl;
	context<RecoveryMachine>().send_query(
		auth_log_shard,
		pg_query_t(
			pg_query_t::LOG,
			auth_log_shard.shard, pg->pg_whoami.shard,
			request_log_from, pg->info.history,
			pg->get_osdmap()->get_epoch()));

	assert(pg->blocked_by.empty());
	pg->blocked_by.insert(auth_log_shard.osd);
	pg->publish_stats_to_osd();
}
{% endhighlight %}
在上面会调用choose_acting()函数，此时就会导致up set与acting set不一致。下面我们来看这个函数：
{% highlight string %}
/**
 * choose acting
 *
 * calculate the desired acting, and request a change with the monitor
 * if it differs from the current acting.
 *
 * if restrict_to_up_acting=true, we filter out anything that's not in
 * up/acting.  in order to lift this restriction, we need to
 *  1) check whether it's worth switching the acting set any time we get
 *     a new pg info (not just here, when recovery finishes)
 *  2) check whether anything in want_acting went down on each new map
 *     (and, if so, calculate a new want_acting)
 *  3) remove the assertion in PG::RecoveryState::Active::react(const AdvMap)
 * TODO!
 */
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound)

{
	...

	if (!pool.info.ec_pool())
		calc_replicated_acting(
			auth_log_shard,
			get_osdmap()->get_pg_size(info.pgid.pgid),
			acting,
			primary,
			up,
			up_primary,
			all_info,
			compat_mode,
			restrict_to_up_acting,
			&want,
			&want_backfill,
			&want_acting_backfill,
			&want_primary,
			ss);
	else
		calc_ec_acting(
			auth_log_shard,
			get_osdmap()->get_pg_size(info.pgid.pgid),
			acting,
			primary,
			up,
			up_primary,
			all_info,
			compat_mode,
			restrict_to_up_acting,
			&want,
			&want_backfill,
			&want_acting_backfill,
			&want_primary,
			ss);


	....

	if (want != acting) {
		dout(10) << "choose_acting want " << want << " != acting " << acting
		<< ", requesting pg_temp change" << dendl;

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
{% endhighlight %}
上面我们看到会调用calc_replicated_acting()来计算我们所期望的acting set，即want_acting。如果want_acting不等于up的话，那么采用want来申请temp pg。申请temp pg会导致产生新的osdmap，因此会触发新的peering操作，从而在OSD::advance_pg()中调用pg_to_up_acting_osds()完成新的acting set的映射。


因此，这里我们可以总结为：up set是直接根据crushmap算出来的，与权威日志等没有任何关系；而acting set是我们真正读写数据时所依赖的一个osd序列，该序列的第一个osd作为primary来负责数据读写，因此必须拥有权威日志。

### 1.2 pg_whoami
包含PG的shard信息，以及所在的OSD信息：
{% highlight string %}
PG::PG(OSDService *o, OSDMapRef curmap,
       const PGPool &_pool, spg_t p):
pg_whoami(osd->whoami, p.shard)
{

	...
}
{% endhighlight %}
对于ReplicatedPG来说，其shard值为shard_id_t::NO_SHARD；对于EC类型的PG来说，由于对数据的读写与PG副本的顺序相关，因此需要shard来指定。

这里我们先来看一下PG的创建：
{% highlight string %}
/*
 * holding osd_lock
 */
void OSD::handle_pg_create(OpRequestRef op)
{
	...

	for (map<pg_t,pg_create_t>::iterator p = m->mkpg.begin();p != m->mkpg.end();++p, ++ci) {

		...

		spg_t pgid;
    	bool mapped = osdmap->get_primary_shard(on, &pgid);

		...
	}

	...
}

bool get_primary_shard(const pg_t& pgid, spg_t *out) const {
	map<int64_t, pg_pool_t>::const_iterator i = get_pools().find(pgid.pool());
	if (i == get_pools().end()) {
		return false;
	}
	
	if (!i->second.ec_pool()) {
		*out = spg_t(pgid);
		return true;
	}
	
	int primary;
	vector<int> acting;
	pg_to_acting_osds(pgid, &acting, &primary);
	
	for (uint8_t i = 0; i < acting.size(); ++i) {
		if (acting[i] == primary) {
			*out = spg_t(pgid, shard_id_t(i));
			return true;
		}
	}
	
	return false;
}
{% endhighlight %}
从上面我们看到对于ReplicatedPG，其直接调用spg_t(pgid)构造函数来创建spg_t对象。

### 1.3 up_primary

通常是up set中的第一个osd为up_primary。

### 1.4 up/acting
这里的up是指PG的up set； acting是指PG的acting set。通常情况下，这两个集合是相同的，只有当系统出现异常时，当前的acting set并不能满足正常的读写请求，那么就会在peering时申请产生pg_temp，此时生成的新的acting set就会与up不一样。

关于up set与acting set，我们在前面已经讲述，因此这里不再赘述。


### 1.5 want_acting
want_acting用于保存我们所期望的的acting set集合，只在pg_temp过程中，该字段有效。下面我们来简单看代码：
{% highlight string %}
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound)
{
	...
	map<pg_shard_t, pg_info_t>::const_iterator auth_log_shard =
		find_best_info(all_info, restrict_to_up_acting, history_les_bound);
	
	if (auth_log_shard == all_info.end()) {
		if (up != acting) {
			dout(10) << "choose_acting no suitable info found (incomplete backfills?),"
				<< " reverting to up" << dendl;

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
		dout(10) << "choose_acting want " << want << " != acting " << acting
			<< ", requesting pg_temp change" << dendl;

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
	
{% endhighlight %}

### 1.6 actingbackfill
actingbackfill在整个代码中只有一个地方进行赋值，我们来看一下：
{% highlight string %}
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound)
{
	...

	set<pg_shard_t> want_backfill, want_acting_backfill;
	vector<int> want;
	pg_shard_t want_primary;
	stringstream ss;

	if (!pool.info.ec_pool())
	calc_replicated_acting(
		auth_log_shard,
		get_osdmap()->get_pg_size(info.pgid.pgid),
		acting,
		primary,
		up,
		up_primary,
		all_info,
		compat_mode,
		restrict_to_up_acting,
		&want,
		&want_backfill,
		&want_acting_backfill,
		&want_primary,
		ss);
	else
	calc_ec_acting(
		auth_log_shard,
		get_osdmap()->get_pg_size(info.pgid.pgid),
		acting,
		primary,
		up,
		up_primary,
		all_info,
		compat_mode,
		restrict_to_up_acting,
		&want,
		&want_backfill,
		&want_acting_backfill,
		&want_primary,
		ss);


	...

	actingbackfill = want_acting_backfill;
	dout(10) << "actingbackfill is " << actingbackfill << dendl;

	...
}

/**
 * calculate the desired acting set.
 *
 * Choose an appropriate acting set.  Prefer up[0], unless it is
 * incomplete, or another osd has a longer tail that allows us to
 * bring other up nodes up to date.
 */
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
	...
}
{% endhighlight %}
对acting_backfill的计算分为如下几个步骤：

1） **acting_backfill的第一个元素是拥有权威日志的OSD**
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
	...

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
		ss << "up[0] needs backfill, osd." << auth_log_shard_id
			<< " selected as primary instead" << std::endl;
		primary = auth_log_shard;
	}
	
	ss << "calc_acting primary is osd." << primary->first
		<< " with " << primary->second << std::endl;

	*want_primary = primary->first;
	want->push_back(primary->first.osd);
	acting_backfill->insert(primary->first);
}
{% endhighlight %}
从上面我们可以看到，如果当前up_primary拥有全量数据，那么就优先选择up_primary；否则选择拥有权威日志的osd作为acting_backfill的第一个元素。

2） **从upset中选择**
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
	...
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

	...
}
{% endhighlight %}
从上面我们可以看到，对于upset中的osd，不管是否与权威日志有重叠，都会被加入到acting_backfill中。

3) **从acting set中选择**
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
	...
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
			ss << " shard " << acting_cand << " (stray) REJECTED "
			<< cur_info << std::endl;
		} 
		else {
			want->push_back(*i);
			acting_backfill->insert(acting_cand);
			ss << " shard " << acting_cand << " (stray) accepted "
				<< cur_info << std::endl;
			usable++;
		}
	}

	...
}

bool is_incomplete() const { return !last_backfill.is_max(); }
{% endhighlight %}
从上面我们可以看到，对于acting set中的元素，其需要同时满足如下两个条件才会被加入到acting_backfill中：

* last_backfill已经完成

* 与权威日志有重叠

4) **从all_info中选择**
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

			ss << " shard " << i->first << " (stray) REJECTED "
				<< i->second << std::endl;
		} else {
			want->push_back(i->first.osd);
			acting_backfill->insert(i->first);
			ss << " shard " << i->first << " (stray) accepted "
				<< i->second << std::endl;
			usable++;
		}
	}
}
{% endhighlight %}
从上面我们可以看到，对于all_info中的元素，其需要同时满足如下两个条件才会被加入到acting_backfill中：

* last_backfill已经完成

* 与权威日志有重叠


----------

这里```all_info```是怎么来的？我们来简单看一下获取peer_info信息的流程：
{% highlight string %}
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound)
{
	map<pg_shard_t, pg_info_t> all_info(peer_info.begin(), peer_info.end());

	...
}

void PG::proc_master_log(
  ObjectStore::Transaction& t, pg_info_t &oinfo,
  pg_log_t &olog, pg_missing_t& omissing, pg_shard_t from)
{
	...
	peer_info[from] = oinfo;
}

void PG::proc_replica_log(
  ObjectStore::Transaction& t,
  pg_info_t &oinfo, pg_log_t &olog, pg_missing_t& omissing,
  pg_shard_t from)
{
	...
	peer_info[from] = oinfo;
	...
}

bool PG::proc_replica_info(
  pg_shard_t from, const pg_info_t &oinfo, epoch_t send_epoch)
{
	...
	peer_info[from] = oinfo;
}
{% endhighlight %}
从上面可以看出，peer_info主要来源于上面三处。而这些peer_info又是由于在peering过程中调用GetInfo来进行触发的，现在我们来看：
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

void PG::RecoveryState::GetInfo::get_infos()
{
	PG *pg = context< RecoveryMachine >().pg;
	unique_ptr<PriorSet> &prior_set = context< Peering >().prior_set;
	
	pg->blocked_by.clear();
	for (set<pg_shard_t>::const_iterator it = prior_set->probe.begin();it != prior_set->probe.end();++it) {
		...
	}	
}
{% endhighlight %}
从上面我们看到peer_info是来源于prior_set，这就引出PG::build_prior()函数，我们会在相关章节进行详细介绍。


### 1.7 actingset
actingset其实就是pg.acting，只是其用std::set<pg_shard_t>方式来进行存储，在init_primary_up_acting()函数中进行设置：
{% highlight string %}
void init_primary_up_acting(
	const vector<int> &newup,
	const vector<int> &newacting,
	int new_up_primary,
	int new_acting_primary) {

	actingset.clear();
	acting = newacting;
	for (uint8_t i = 0; i < acting.size(); ++i) {
		if (acting[i] != CRUSH_ITEM_NONE)
			actingset.insert(
				pg_shard_t(
					acting[i],
					pool.info.ec_pool() ? shard_id_t(i) : shard_id_t::NO_SHARD));
	}

	...
}
{% endhighlight %}

### 1.8 upset
upset其实就是pg.up，只是其用std::set<pg_shard_t>方式来进行存储，在init_primary_acting()函数中进行设置：
{% highlight string %}
void init_primary_up_acting(
	const vector<int> &newup,
	const vector<int> &newacting,
	int new_up_primary,
	int new_acting_primary) {

	...
	for (uint8_t i = 0; i < up.size(); ++i) {
		if (up[i] != CRUSH_ITEM_NONE)
			upset.insert(
				pg_shard_t(
					up[i],
					pool.info.ec_pool() ? shard_id_t(i) : shard_id_t::NO_SHARD));
	}

	...
}
{% endhighlight %}

### 1.9 last_update_ondisk

我们在代码中搜寻```last_update_ondisk```，看到对其的修改主要有如下两个地方：
{% highlight string %}
void PG::activate(ObjectStore::Transaction& t,
		  epoch_t activation_epoch,
		  list<Context*>& tfin,
		  map<int, map<spg_t,pg_query_t> >& query_map,
		  map<int,
		      vector<
			pair<pg_notify_t,
			     pg_interval_map_t> > > *activator_map,
                  RecoveryCtx *ctx)
{
	...
	
	if (is_primary()) {
		last_update_ondisk = info.last_update;
		min_last_complete_ondisk = eversion_t(0,0);  // we don't know (yet)!
	}

	...
}

void ReplicatedPG::repop_all_committed(RepGather *repop)
{
	dout(10) << __func__ << ": repop tid " << repop->rep_tid << " all committed "<< dendl;
	repop->all_committed = true;
	
	if (!repop->rep_aborted) {
		if (repop->v != eversion_t()) {
			last_update_ondisk = repop->v;
			last_complete_ondisk = repop->pg_local_last_complete;
		}
		eval_repop(repop);
	}
}
{% endhighlight %}
下面我们来分析：

1） **PG::activate()**

PG::activate()会在PG peering完成时被调用，这里将last_update_ondisk设置为了info.last_update。

>注： peering完成时，对于PG primary，其info.last_update是否就等于info.last_complete?

2) **PG::repop_all_committed()**


PG::repop_all_committed()函数会在对象的三个副本的日志都写完成时被调用。通过该函数我们看到，repop->v赋值给了last_update_ondisk。现在我们来看这中间的过程：
{% highlight string %}
void ReplicatedPG::execute_ctx(OpContext *ctx)
{
	...
	ctx->at_version = get_next_version();

	// issue replica writes
	ceph_tid_t rep_tid = osd->get_tid();
	
	RepGather *repop = new_repop(ctx, obc, rep_tid);
	
	issue_repop(repop, ctx);
	eval_repop(repop);
	repop->put();
}

void ReplicatedPG::issue_repop(RepGather *repop, OpContext *ctx)
{
	const hobject_t& soid = ctx->obs->oi.soid;
	dout(7) << "issue_repop rep_tid " << repop->rep_tid<< " o " << soid<< dendl;
	
	repop->v = ctx->at_version;
	
	...
}
{% endhighlight %}
从上面我们可以看到，其实就是将ctx->at_version赋值给了repop->v。

也就是说，当PG::repop_all_committed()被回调，说明该OpContext对应的事务日志已经提交到硬盘，此时会直接将last_update_ondisk设置为ctx->at_version。

>疑问：此处如何保证按顺序提交的事务，其响应也是按顺序返回的呢？比如事务A、事务B按顺序提交，但是事务A响应丢失，从而导致ReplicatedPG::repop_all_committed()中last_update_ondisk是一个更新的ctx->at_version。
>
>如能保证PG每个节点所成功提交的事务都是连续的，似乎也没有问题。代码中是如何保证？？？

3）**总结**

last_update_ondisk记录的是PG所有副本均完成写日志操作的最新对象版本号。


### 1.10 last_complete_ondisk

我们在代码中搜寻```last_update_ondisk```，看到对其的修改主要有如下两个地方：
{% highlight string %}
void ReplicatedPG::repop_all_committed(RepGather *repop)
{
	dout(10) << __func__ << ": repop tid " << repop->rep_tid << " all committed "<< dendl;
	repop->all_committed = true;
	
	if (!repop->rep_aborted) {
		if (repop->v != eversion_t()) {
			last_update_ondisk = repop->v;
			last_complete_ondisk = repop->pg_local_last_complete;
		}
		eval_repop(repop);
	}
}

void update_last_complete_ondisk(
	eversion_t lcod) {

	last_complete_ondisk = lcod;
}
{% endhighlight %}
下面我们来分析：

1） **ReplicatedPG::repop_all_committed()**

对于repop_all_committed()函数，其会在对象日志写入完成时回调。我们看到在该函数中，其将repop->pg_local_last_complete赋值给了last_complete_ondisk:
{% highlight string %}
void ReplicatedPG::execute_ctx(OpContext *ctx)
{
	...
	ctx->at_version = get_next_version();

	// issue replica writes
	ceph_tid_t rep_tid = osd->get_tid();
	
	RepGather *repop = new_repop(ctx, obc, rep_tid);
	
	issue_repop(repop, ctx);
	eval_repop(repop);
	repop->put();
}

ReplicatedPG::RepGather *ReplicatedPG::new_repop(
  OpContext *ctx, ObjectContextRef obc,
  ceph_tid_t rep_tid)
{
	if (ctx->op)
		dout(10) << "new_repop rep_tid " << rep_tid << " on " << *ctx->op->get_req() << dendl;
	else
		dout(10) << "new_repop rep_tid " << rep_tid << " (no op)" << dendl;
	
	RepGather *repop = new RepGather(ctx, rep_tid, info.last_complete);
	
	repop->start = ceph_clock_now(cct);
	
	repop_queue.push_back(&repop->queue_item);
	repop->get();
	
	osd->logger->inc(l_osd_op_wip);
	
	return repop;
}    

RepGather(OpContext *c, ceph_tid_t rt,
  eversion_t lc) :
	hoid(c->obc->obs.oi.soid),
	op(c->op),
	queue_item(this),
	nref(1),
	rep_tid(rt), 
	rep_aborted(false), rep_done(false),
	all_applied(false), all_committed(false),
	pg_local_last_complete(lc),
	lock_manager(std::move(c->lock_manager)),
	on_applied(std::move(c->on_applied)),
	on_committed(std::move(c->on_committed)),
	on_success(std::move(c->on_success)),
	on_finish(std::move(c->on_finish)) {}
{% endhighlight %}
由上面可见，repop_all_committed()函数中记录的last_complete_ondisk可能并不是当前最新写入的事务的version值，而可能是一个更老的info.last_complete(注：猜测主要原因可能是本PG还可能在进行backfilling操作，因此last_complete_ondisk会是一个更老的位置）


2) **update_last_complete_ondisk()**

对于sub_op_modify_commit()函数，其会在PG的非主副本日志写入完成时被调用：
{% highlight string %}
void ReplicatedBackend::sub_op_modify(OpRequestRef op)
{
	...
	RepModifyRef rm(std::make_shared<RepModify>());
	rm->op = op;
	rm->ackerosd = ackerosd;
	rm->last_complete = get_info().last_complete;
	rm->epoch_started = get_osdmap()->get_epoch();
	...
}
void ReplicatedBackend::sub_op_modify_commit(RepModifyRef rm)
{
	...

	get_parent()->update_last_complete_ondisk(rm->last_complete);

	...
}
void update_last_complete_ondisk(eversion_t lcod)
{
	last_complete_ondisk = lcod;
}
{% endhighlight %}
从上面我们可以看到，对于PG的非主副本，当写日志操作完成时触发回调sub_op_modify_commit()，从而更新last_complete_ondisk，其更新的值同样为当前PGInfo的last_complete值。


3） **总结**

last_complete_ondisk用于记录PG所有副本均完成写对象到硬盘的版本号，其是通过pginfo.last_complete来得到的。在后面的章节中，我们会分析PGInfo在整个PG生命周期中的变化。





<br />
<br />

**[参看]**



<br />
<br />
<br />

