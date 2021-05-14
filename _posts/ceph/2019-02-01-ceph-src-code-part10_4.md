---
layout: post
title: PG info介绍
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


PGInfo存在于PG的整个生命周期中，其在对象数据的写入、数据恢复、PG Peering过程中均发挥重要的作用。本章试图研究pg info在整个PG生命周期中的变化过程，从而对PG及PGInfo有一个更深入的理解。
{% highlight string %}
class PG : DoutPrefixProvider {
public:
	// pg state
	pg_info_t        info;
};

class ReplicatedPG : public PG, public PGBackend::Listener {
public:
	const pg_info_t &get_info() const {
		return info;
	}
};

class PGBackend {
public:
	Listener *parent;
	Listener *get_parent() const { return parent; }

	PGBackend(Listener *l, ObjectStore *store, coll_t coll, ObjectStore::CollectionHandle &ch) :
		store(store),
		coll(coll),
		ch(ch),
		parent(l) {}

	const pg_info_t &get_info() { return get_parent()->get_info(); }
};
{% endhighlight %}

从上面的代码可知，ReplicatedPG以及PGBackend中使用到的PGInfo均为PG::info。
<!-- more -->

## 1. pginfo相关数据结构

### 1.1 pg_info_t数据结构

pg_info_t数据结构定义在osd/osd_types.h头文件中，如下：
{% highlight string %}
/**
 * pg_info_t - summary of PG statistics.
 *
 * some notes: 
 *  - last_complete implies we have all objects that existed as of that
 *    stamp, OR a newer object, OR have already applied a later delete.
 *  - if last_complete >= log.bottom, then we know pg contents thru log.head.
 *    otherwise, we have no idea what the pg is supposed to contain.
 */
struct pg_info_t {
	spg_t pgid;
	eversion_t last_update;      ///< last object version applied to store.
	eversion_t last_complete;    ///< last version pg was complete through.
	epoch_t last_epoch_started;  ///< last epoch at which this pg started on this osd
	
	version_t last_user_version; ///< last user object version applied to store
	
	eversion_t log_tail;         ///< oldest log entry.
	
	hobject_t last_backfill;     ///< objects >= this and < last_complete may be missing
	bool last_backfill_bitwise;  ///< true if last_backfill reflects a bitwise (vs nibblewise) sort
	
	interval_set<snapid_t> purged_snaps;
	
	pg_stat_t stats;
	
	pg_history_t history;
	pg_hit_set_history_t hit_set;
};
{% endhighlight %}

下面我们分别介绍一下各字段：

###### 1.1.1 pgid

pgid用于保存当前PG的pgid信息。

###### 1.1.2 last_update
{% highlight string %}
//src/include/types.h
// NOTE: these must match ceph_fs.h typedefs
typedef uint64_t ceph_tid_t; // transaction id
typedef uint64_t version_t;
typedef __u32 epoch_t;       // map epoch  (32bits -> 13 epochs/second for 10 years)


//osd/osd_types.h
class eversion_t {
public:
	version_t version;
	epoch_t epoch;
	__u32 __pad;
};
{% endhighlight %}
last_update表示PG内最近一次更新的对象版本，还没有在所有OSD上更新完成。在last_update与last_complete之间的操作表示该操作已经在部分OSD上完成，但是还没有全部完成。

下面我们来看一下pginfo.last_update在ceph整个运行过程中的更新操作：

1） PG数据写入阶段增加log entry
{% highlight string %}
eversion_t get_next_version() const {
	eversion_t at_version(get_osdmap()->get_epoch(),pg_log.get_head().version+1);
	assert(at_version > info.last_update);
	assert(at_version > pg_log.get_head());
	return at_version;
}

void ReplicatedPG::execute_ctx(OpContext *ctx)
{
	// version
	ctx->at_version = get_next_version();
	ctx->mtime = m->get_mtime();
}
void ReplicatedPG::finish_ctx(OpContext *ctx, int log_op_type, bool maintain_ssc,
			      bool scrub_ok)
{
	...
	 // append to log
	ctx->log.push_back(pg_log_entry_t(log_op_type, soid, ctx->at_version,
		ctx->obs->oi.version,
		ctx->user_at_version, ctx->reqid,
		ctx->mtime));

}
void PG::add_log_entry(const pg_log_entry_t& e)
{
	// raise last_complete only if we were previously up to date
	if (info.last_complete == info.last_update)
		info.last_complete = e.version;
	
	// raise last_update.
	assert(e.version > info.last_update);
	info.last_update = e.version;
	
	// raise user_version, if it increased (it may have not get bumped
	// by all logged updates)
	if (e.user_version > info.last_user_version)
	info.last_user_version = e.user_version;
	
	// log mutation
	pg_log.add(e);
	dout(10) << "add_log_entry " << e << dendl;
}
{% endhighlight %}
从上面可以看到，在PG数据写入阶段，将pg_log_entry_t添加进pg_log时，会将info.last_update更新为ctx->at_version，

2) 进入activate阶段更新本地保存的peer.last_update
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

	// if primary..
	if (is_primary()) {
		for (set<pg_shard_t>::iterator i = actingbackfill.begin();i != actingbackfill.end();++i) {

			if (*i == pg_whoami) continue;
			pg_shard_t peer = *i;
			pg_info_t& pi = peer_info[peer];


			...
			/*
			* cover case where peer sort order was different and
			* last_backfill cannot be interpreted
			*/
			bool force_restart_backfill =!pi.last_backfill.is_max() && pi.last_backfill_bitwise != get_sort_bitwise();

			if (pi.last_update == info.last_update && !force_restart_backfill) {

				//已经追上权威

			}else if (pg_log.get_tail() > pi.last_update || pi.last_backfill == hobject_t() ||
				force_restart_backfill ||(backfill_targets.count(*i) && pi.last_backfill.is_max())){

				/* ^ This last case covers a situation where a replica is not contiguous
				* with the auth_log, but is contiguous with this replica.  Reshuffling
				* the active set to handle this would be tricky, so instead we just go
				* ahead and backfill it anyway.  This is probably preferrable in any
				* case since the replica in question would have to be significantly
				* behind.
				*/
				// backfill(日志不重叠，采用backfill方式来进行恢复)

				pi.last_update = info.last_update;
				pi.last_complete = info.last_update;
				pi.set_last_backfill(hobject_t(), get_sort_bitwise());
				pi.last_epoch_started = info.last_epoch_started;
				pi.history = info.history;
				pi.hit_set = info.hit_set;
				pi.stats.stats.clear();
			}else{
				//catch up(具有日志重叠，直接采用pglog进行恢复)

				m = new MOSDPGLog(i->shard, pg_whoami.shard,get_osdmap()->get_epoch(), info);

				// send new stuff to append to replicas log
				//(拷贝pg_log中last_update之后的日志到m中)
				m->log.copy_after(pg_log.get_log(), pi.last_update);
			}

			// peer now has(此处认为peer完成，因此更新本地pi.last_update)
			pi.last_update = info.last_update;
		}
	}
}
{% endhighlight %}
从上面的代码可以，当PG primary进入activate阶段，表示副本之间已经达成一致，此时对于PG primary来说，可以更新本地保存的peer.last_update为权威的last_update。

3）PG分裂时设置info.last_update
{% highlight string %}
void PG::split_into(pg_t child_pgid, PG *child, unsigned split_bits)
{
	...

	pg_log.split_into(child_pgid, split_bits, &(child->pg_log));
	child->info.last_complete = info.last_complete;

	info.last_update = pg_log.get_head();
	child->info.last_update = child->pg_log.get_head();

	...
}
{% endhighlight %}
在PG分裂时，肯定已经完成了peering操作，此时info.last_update必定等于pg_log.get_head，既然如此为何代码中还要在重新设置一遍呢？这是因为在PG分裂时，pg_log也要进行分裂，原来的head有可能被分裂到了child中了，因此这里需要重新设置当前PG的last_update。如下图所示：


![ceph-chapter104-1](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter104_1.jpg)


* last_update:

* last_complete:

* last_epoch_started:

* last_user_version:

* log_tail:

* last_backfill:

* last_backfill_bitwise:

* purged_snaps:

* stats:

* history:

* hit_set:

2) **pg_stat_t数据结构**

pg_stat_t数据结构用于保存当前PG的状态信息，其定义在osd/osd_types.h头文件中：
{% highlight string %}
/** pg_stat
 * aggregate stats for a single PG.
 */
struct pg_stat_t {
	/**************************************************************************
	* WARNING: be sure to update the operator== when adding/removing fields! *
	**************************************************************************/
	eversion_t version;
	version_t reported_seq;  // sequence number
	epoch_t reported_epoch;  // epoch of this report
	__u32 state;
	utime_t last_fresh;   // last reported
	utime_t last_change;  // new state != previous state
	utime_t last_active;  // state & PG_STATE_ACTIVE
	utime_t last_peered;  // state & PG_STATE_ACTIVE || state & PG_STATE_PEERED
	utime_t last_clean;   // state & PG_STATE_CLEAN
	utime_t last_unstale; // (state & PG_STATE_STALE) == 0
	utime_t last_undegraded; // (state & PG_STATE_DEGRADED) == 0
	utime_t last_fullsized; // (state & PG_STATE_UNDERSIZED) == 0
	
	eversion_t log_start;         // (log_start,version]
	eversion_t ondisk_log_start;  // there may be more on disk
	
	epoch_t created;
	epoch_t last_epoch_clean;
	pg_t parent;
	__u32 parent_split_bits;
	
	eversion_t last_scrub;
	eversion_t last_deep_scrub;
	utime_t last_scrub_stamp;
	utime_t last_deep_scrub_stamp;
	utime_t last_clean_scrub_stamp;
	
	object_stat_collection_t stats;
	
	int64_t log_size;
	int64_t ondisk_log_size;    // >= active_log_size
	
	vector<int32_t> up, acting;
	epoch_t mapping_epoch;
	
	vector<int32_t> blocked_by;  ///< osds on which the pg is blocked
	
	utime_t last_became_active;
	utime_t last_became_peered;
	
	/// up, acting primaries
	int32_t up_primary;
	int32_t acting_primary;
	
	bool stats_invalid:1;
	/// true if num_objects_dirty is not accurate (because it was not
	/// maintained starting from pool creation)
	bool dirty_stats_invalid:1;
	bool omap_stats_invalid:1;
	bool hitset_stats_invalid:1;
	bool hitset_bytes_stats_invalid:1;
	bool pin_stats_invalid:1;
};
{% endhighlight %}


3) **pg_history_t数据结构**

pg_history_t用于保存PG最近的peering/mapping历史记录，其定义在osd/osd_types.h头文件中：
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


## 2. PG info信息的初始化

1）**PG构造函数中初始化pginfo**

{% highlight string %}
PG::PG(OSDService *o, OSDMapRef curmap,const PGPool &_pool, spg_t p) 
	: info(p){
}

pg_info_t(spg_t p)
: pgid(p),
  last_epoch_started(0), last_user_version(0),
  last_backfill(hobject_t::get_max()),
  last_backfill_bitwise(false)
{ }
{% endhighlight %}

从上面我们可以看到，在PG的构造函数中设置了pg_info_t.pgid；将last_epoch_started初始化为0；将last_user_version初始化为0；将last_backfill设置为hobject_t::get_max()，表示没有需要backfill的对象；将last_backfill_bitwise设置为false。

<br />
<br />

**[参看]**

1. [ceph中PGLog处理流程](https://ivanzz1001.github.io/records/post/ceph/2019/02/05/ceph-src-code-part14_1)

<br />
<br />
<br />

