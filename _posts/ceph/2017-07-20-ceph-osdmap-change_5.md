---
layout: post
title: OSD从down到out过程中osdmap的变化(4)
tags:
- ceph
categories: ceph
description: ceph pg
---


接着上一章，我们这里结合```osd3_watch.txt```日志文件，以及pgmap_active_clean.txt、pgmap_in_down.txt、pgmap_out_down.txt，从中选出```4```个具有代表性的PG，来分析一下osd0从```in+up```到```in+down```再到```out+down```这一整个过程中PG所执行的动作。

选取的4个PG如下：
<pre>
    in + up                                                        in + down                                                       out + down
---------------------------------------------------------------------------------------------------------------------------------------------------------------------
pg_stat up      up_primary	acting	acting_primary      pg_stat  up     up_primary	acting	acting_primary       pg_stat    up      up_primary	acting	acting_primary

11.4    [0,3]   0           [0,3]	0                   11.4	 [3]    3	        [3]	    3                    11.4	    [3,2]	3	        [3,2]	3

22.2c   [0,3]   0	        [0,3]	0                   22.2c	 [3]    3	        [3]	    3                    22.2c	    [5,7]	5	        [5,7]	5

22.2a   [3,0]   3	        [3,0]	3                   22.2a	 [3]    3	        [3]	    3                    22.2a	    [3,6]	3	        [3,6]	3

22.16   [3,7]   3	        [3,7]	3                   22.16	 [3,7]	3       	[3,7]	3                    22.16	    [3,7]	3	        [3,7]	3
</pre>

上面4个PG代表4种典型的场景：

* **PG 11.4** : PG的主osd关闭，在osd out之后，PG的其中一个副本进行remap

* **PG 22.2c**: PG的主OSD关闭，在osd out之后，PG的两个副本进行remap

* **PG 22.2a**: PG的副本OSD关闭，在osd out之后，PG的其中一个副本进行remap

* **PG 22.16**: 关闭的OSD并不是PG的任何副本

使用如下命令从osd3_watch.txt中分别导出该PG相关的日志信息：
{% highlight string %}
# grep -rnw "11.4" ./osd3_watch.txt > ./osd3_watch_pg_11.4.txt
# grep -rnw "22.2c" ./osd3_watch.txt > ./osd3_watch_pg_22.2c.txt
# grep -rnw "22.2a" ./osd3_watch.txt > ./osd3_watch_pg_22.2a.txt
# grep -rnw "22.16" ./osd3_watch.txt > ./osd3_watch_pg_22.16.txt
# ls -al
总用量 32900
drwxr-xr-x  2 root root     4096 9月  12 19:59 .
dr-xr-x---. 8 root root     4096 9月  11 11:32 ..
-rw-r--r--  1 root root   170772 9月  12 19:58 osd3_watch_pg_11.4.txt
-rw-r--r--  1 root root    39861 9月  12 19:59 osd3_watch_pg_22.16.txt
-rw-r--r--  1 root root   127958 9月  12 19:59 osd3_watch_pg_22.2a.txt
-rw-r--r--  1 root root    96501 9月  12 19:59 osd3_watch_pg_22.2c.txt
-rw-r--r--  1 root root 33228328 9月  11 14:19 osd3_watch.txt
{% endhighlight %}

这里主要分析一下PG 22.16在这一阶段的变化过程。

<!-- more --> 

## 1. PG 22.16分析
这里我们过滤osd3_watch.txt日志文件，找出所有与PG 22.16相关的日志（如下日志经过了适当的修改）。


### 1.1 接收到e2223版本的osdmap
如下是接收到e2223版本osdmap时的日志片段：
{% highlight string %}
3993:2020-09-11 14:05:19.137161 7fba3d124700 10 osd.3 pg_epoch: 2222 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] handle_advance_map [3,7]/[3,7] -- 3/3
3996:2020-09-11 14:05:19.137168 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] state<Started/Primary/Active>: Active advmap
3998:2020-09-11 14:05:19.137173 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] state<Started>: Started advmap
3999:2020-09-11 14:05:19.137177 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] check_recovery_sources no source osds () went down
4003:2020-09-11 14:05:19.137182 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] handle_activate_map 
4004:2020-09-11 14:05:19.137187 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] state<Started/Primary/Active>: Active: handling ActMap
4006:2020-09-11 14:05:19.137192 7fba3d124700  7 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] state<Started/Primary>: handle ActMap primary
4008:2020-09-11 14:05:19.137197 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] publish_stats_to_osd 2223:188
4009:2020-09-11 14:05:19.137202 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] take_waiters
4011:2020-09-11 14:05:19.137207 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] handle_activate_map: Not dirtying info: last_persisted is 2200 while current is 2223
4013:2020-09-11 14:05:19.137212 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[22.16( empty local-les=2133 n=0 ec=155 les/c/f 2133/2133/0 2123/2132/2132) [3,7] r=0 lpr=2132 crt=0'0 mlcod 0'0 active+clean] handle_peering_event: epoch_sent: 2223 epoch_requested: 2223 NullEvt
{% endhighlight %}

处理流程如下：

1） **接收新的osdmap**
{% highlight string %}
void OSD::handle_osd_map(MOSDMap *m)
{
	...

	// superblock and commit
	write_superblock(t);
	store->queue_transaction(
		service.meta_osr.get(),
		std::move(t),
		new C_OnMapApply(&service, pinned_maps, last),
		new C_OnMapCommit(this, start, last, m), 0);
	service.publish_superblock(superblock);
}
{% endhighlight %}

在osdmap被写入到object store后，会回调C_OnMapCommit()函数，然后调用到OSD::_committed_osd_maps()，如下：
{% highlight string %}
void OSD::_committed_osd_maps(epoch_t first, epoch_t last, MOSDMap *m)
{
	...
	consume_map();

	...
}
{% endhighlight %}
在OSD::consume_map()函数中产生NullEvt事件，触发Peering过程：
{% highlight string %}
void OSD::consume_map()
{
	...
	// scan pg's
	{
		RWLock::RLocker l(pg_map_lock);
		for (ceph::unordered_map<spg_t,PG*>::iterator it = pg_map.begin();it != pg_map.end();++it) {
			PG *pg = it->second;
			pg->lock();
			pg->queue_null(osdmap->get_epoch(), osdmap->get_epoch());
			pg->unlock();
		}

    	logger->set(l_osd_pg, pg_map.size());
	}
	...
}
{% endhighlight %}

2) **处理Peering事件**
{% highlight string %}
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...
	PG::RecoveryCtx rctx = create_context();
	rctx.handle = &handle;

	for (list<PG*>::const_iterator i = pgs.begin();i != pgs.end();++i) {
		set<boost::intrusive_ptr<PG> > split_pgs;
		PG *pg = *i;
		pg->lock_suspend_timeout(handle);
		curmap = service.get_osdmap();
		if (pg->deleting) {
			pg->unlock();
			continue;
		}

		if (!advance_pg(curmap->get_epoch(), pg, handle, &rctx, &split_pgs)) {
			// we need to requeue the PG explicitly since we didn't actually
			// handle an event
			peering_wq.queue(pg);
		} else {
			assert(!pg->peering_queue.empty());
			PG::CephPeeringEvtRef evt = pg->peering_queue.front();
			pg->peering_queue.pop_front();
			pg->handle_peering_event(evt, &rctx);
		}

		need_up_thru = pg->need_up_thru || need_up_thru;
		same_interval_since = MAX(pg->info.history.same_interval_since,
			same_interval_since);
		pg->write_if_dirty(*rctx.transaction);
		if (!split_pgs.empty()) {
			rctx.on_applied->add(new C_CompleteSplits(this, split_pgs));
			split_pgs.clear();
		}
		dispatch_context_transaction(rctx, pg, &handle);
		pg->unlock();
		handle.reset_tp_timeout();
	}
	if (need_up_thru)
		queue_want_up_thru(same_interval_since);

	dispatch_context(rctx, 0, curmap, &handle);
	
	service.send_pg_temp();
}
{% endhighlight %}
首先调用advance_pg()完成PG对osdmap的追赶：
{% highlight string %}
bool OSD::advance_pg(
  epoch_t osd_epoch, PG *pg,
  ThreadPool::TPHandle &handle,
  PG::RecoveryCtx *rctx,
  set<boost::intrusive_ptr<PG> > *new_pgs)
{
	epoch_t next_epoch = pg->get_osdmap()->get_epoch() + 1;
	OSDMapRef lastmap = pg->get_osdmap();

	...
	for (;next_epoch <= osd_epoch && next_epoch <= max;++next_epoch) {

		..

		
		vector<int> newup, newacting;
		int up_primary, acting_primary;

		nextmap->pg_to_up_acting_osds(
			pg->info.pgid.pgid,
			&newup, &up_primary,
			&newacting, &acting_primary);

		pg->handle_advance_map(
			nextmap, lastmap, newup, up_primary,
			newacting, acting_primary, rctx);
	
		...

	}

	pg->handle_activate_map(rctx);

	...
}
{% endhighlight %}

3) **对AdvMap事件的处理**
{% highlight string %}
boost::statechart::result PG::RecoveryState::Started::react(const AdvMap& advmap)
{
	dout(10) << "Started advmap" << dendl;
	PG *pg = context< RecoveryMachine >().pg;
	pg->check_full_transition(advmap.lastmap, advmap.osdmap);
	if (pg->should_restart_peering(
	  advmap.up_primary,
	  advmap.acting_primary,
	  advmap.newup,
	  advmap.newacting,
	  advmap.lastmap,
	  advmap.osdmap)) {
		dout(10) << "should_restart_peering, transitioning to Reset" << dendl;
		post_event(advmap);
		return transit< Reset >();
	}
	pg->remove_down_peer_info(advmap.osdmap);
	return discard_event();
}
{% endhighlight %}
这里对于PG 22.16而言，明显是不需要重新Peering的，因此这里直接丢弃AdvMap事件。

4) **对ActMap事件的处理**

因为PG 22.16当前处于active+clean状态，因此调用如下函数处理：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Active::react(const ActMap&)
{
	PG *pg = context< RecoveryMachine >().pg;
	dout(10) << "Active: handling ActMap" << dendl;
	assert(pg->is_primary());
	
	if (pg->have_unfound()) {
		// object may have become unfound
		pg->discover_all_missing(*context< RecoveryMachine >().get_query_map());
	}
	
	if (pg->cct->_conf->osd_check_for_log_corruption)
	pg->check_log_for_corruption(pg->osd->store);
	
	int unfound = pg->missing_loc.num_unfound();
	if (unfound > 0 &&
	pg->all_unfound_are_queried_or_lost(pg->get_osdmap())) {
		if (pg->cct->_conf->osd_auto_mark_unfound_lost) {
		pg->osd->clog->error() << pg->info.pgid.pgid << " has " << unfound<< " objects unfound and apparently lost, would automatically marking lost but NOT IMPLEMENTED\n";
	} else
		pg->osd->clog->error() << pg->info.pgid.pgid << " has " << unfound << " objects unfound and apparently lost\n";
	}
	
	if (!pg->snap_trimq.empty() &&
	pg->is_clean()) {
		dout(10) << "Active: queuing snap trim" << dendl;
		pg->queue_snap_trim();
	}
	
	if (!pg->is_clean() && !pg->get_osdmap()->test_flag(CEPH_OSDMAP_NOBACKFILL) &&
	  (!pg->get_osdmap()->test_flag(CEPH_OSDMAP_NOREBALANCE) || pg->is_degraded())) {
		pg->osd->queue_for_recovery(pg);
	}
	return forward_event();
}
{% endhighlight %}
这里直接将ActMap事件往上抛，然后调用如下函数：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Primary::react(const ActMap&)
{
	dout(7) << "handle ActMap primary" << dendl;
	PG *pg = context< RecoveryMachine >().pg;
	pg->publish_stats_to_osd();
	pg->take_waiters();
	return discard_event();
}
{% endhighlight %}
这里调用PG::publish_stats_to_osd()之后，直接将ActMap事件丢弃。


### 1.2 e2224/e2225版本osdmap的处理
处理方式与上面e2223类似，这里不再赘述。

<br />
<br />
<br />

1. [ceph存储 PG的状态机和peering过程](https://blog.csdn.net/skdkjzz/article/details/51579903)

2. [Ceph OSDMap 机制浅析](https://www.jianshu.com/p/8ecd6028f5ff?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)