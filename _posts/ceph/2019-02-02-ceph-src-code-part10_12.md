---
layout: post
title: ceph peering机制再研究(8)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

在上一章我们讲述到PG Primary获取到权威日志之后，会进入到GetMissing阶段。本文介绍GetMissing阶段的处理，包括日志操作以及Active操作。


<!-- more -->

## 1. Peering之GetMissing状态处理
GetMissing的处理过程为：首先，拉取各个从OSD上的有效日志；其次，用主OSD上的权威日志与各个从OSD的日志进行对比，从而计算出各个从OSD上不一致的对象并保存在对应的pg_missing_t结构中，作为后续数据修复的依据。

主OSD的不一致的对象信息，已经在调用函数PG::proc_master_log()合并权威日志的过程中计算出来，所以这里只计算从OSD上不一致的对象。


### 1.1 GetMissing状态
在GetMissing的构造函数里，通过对比主OSD上的权威pg_info信息，来获取从OSD上的日志信息。
{% highlight string %}
struct GetMissing : boost::statechart::state< GetMissing, Peering >, NamedState {
	set<pg_shard_t> peer_missing_requested;

	explicit GetMissing(my_context ctx);
	void exit();

typedef boost::mpl::list <
	boost::statechart::custom_reaction< QueryState >,
	boost::statechart::custom_reaction< MLogRec >,
	boost::statechart::transition< NeedUpThru, WaitUpThru >
> reactions;
	boost::statechart::result react(const QueryState& q);
	boost::statechart::result react(const MLogRec& logevt);
};

PG::RecoveryState::GetMissing::GetMissing(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Peering/GetMissing")
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	assert(!pg->actingbackfill.empty());
	for (set<pg_shard_t>::iterator i = pg->actingbackfill.begin();i != pg->actingbackfill.end();++i) {
		if (*i == pg->get_primary()) continue;
		const pg_info_t& pi = pg->peer_info[*i];
	
		if (pi.is_empty())
			continue;                                // no pg data, nothing divergent
	
		if (pi.last_update < pg->pg_log.get_tail()) {
			dout(10) << " osd." << *i << " is not contiguous, will restart backfill" << dendl;
			pg->peer_missing[*i];
			continue;
		}
		if (pi.last_backfill == hobject_t()) {
			dout(10) << " osd." << *i << " will fully backfill; can infer empty missing set" << dendl;
			pg->peer_missing[*i];
			continue;
		}
	
		if (pi.last_update == pi.last_complete &&  // peer has no missing
		  pi.last_update == pg->info.last_update) {  // peer is up to date
			// replica has no missing and identical log as us.  no need to
			// pull anything.
			// FIXME: we can do better here.  if last_update==last_complete we
			//        can infer the rest!
			dout(10) << " osd." << *i << " has no missing, identical log" << dendl;
			pg->peer_missing[*i];
			continue;
		}
	
		// We pull the log from the peer's last_epoch_started to ensure we
		// get enough log to detect divergent updates.
		eversion_t since(pi.last_epoch_started, 0);
		assert(pi.last_update >= pg->info.log_tail);  // or else choose_acting() did a bad thing
 
		if (pi.log_tail <= since) {
			dout(10) << " requesting log+missing since " << since << " from osd." << *i << dendl;
			context< RecoveryMachine >().send_query(
			  *i,
			  pg_query_t(
				pg_query_t::LOG,
				i->shard, pg->pg_whoami.shard,
				since, pg->info.history,
				pg->get_osdmap()->get_epoch()));
		} else {
			dout(10) << " requesting fulllog+missing from osd." << *i
			  << " (want since " << since << " < log.tail " << pi.log_tail << ")"<< dendl;
			context< RecoveryMachine >().send_query(
			  *i, 
			  pg_query_t(
				pg_query_t::FULLLOG,
				i->shard, pg->pg_whoami.shard,
				pg->info.history, pg->get_osdmap()->get_epoch()));
		}
		peer_missing_requested.insert(*i);
		pg->blocked_by.insert(i->osd);
	}
	
	if (peer_missing_requested.empty()) {
		if (pg->need_up_thru) {
			dout(10) << " still need up_thru update before going active" << dendl;
			post_event(NeedUpThru());
			return;
		}
	
		// all good!
		post_event(Activate(pg->get_osdmap()->get_epoch()));
	} else {
		pg->publish_stats_to_osd();
	}
}
{% endhighlight %}
在GetMissing构造函数中，遍历pg->actingbackfill的OSD列表，针对每一个pg_shard_t做如下处理：

1） 不需要获取PG日志的情况

  a) 如果当前副本为PG Primary，因为其已经有了完整的权威日志，因此可以直接跳过；

  b) 如果pi.is_empty()为空，没有任何信息，需要Backfill过程来修复，不需要获取日志；

  c) 




<br />
<br />

**[参看]**

1. [ceph博客](http://aspirer.wang/)

2. [ceph官网](https://docs.ceph.com/en/latest/dev/internals/)

3. [PEERING](https://docs.ceph.com/en/latest/dev/peering/)

4. [分布式系统](https://blog.csdn.net/chdhust)
<br />
<br />
<br />

