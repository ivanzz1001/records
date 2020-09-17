---
layout: post
title: OSD从down到out过程中osdmap的变化(2)
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

下面我们就针对这4种场景分别来分析。
<!-- more --> 

## 1. PG 11.4分析
这里我们过滤osd3_watch.txt日志文件，找出所有与PG 11.4相关的日志（如下日志经过了适当的修改）。

### 1.1 收到e2223版本osdmap
{% highlight string %}
2378:2020-09-11 14:05:19.113264 7fba46937700 20 osd.3 2222 _dispatch 0x7fba6ce1a1c0 osd_map(2223..2223 src has 1514..2223) v3
2379:2020-09-11 14:05:19.113452 7fba46937700  3 osd.3 2222 handle_osd_map epochs [2223,2223], i have 2222, src has [1514,2223]
2380:2020-09-11 14:05:19.113460 7fba46937700 10 osd.3 2222 handle_osd_map  got inc map for epoch 2223
2381:2020-09-11 14:05:19.113776 7fba46937700 20 osd.3 2222 got_full_map 2223, nothing requested
2382:2020-09-11 14:05:19.113857 7fba46937700 10 osd.3 2222 write_superblock sb(5341b139-15dc-4c68-925a-179797d894d3 osd.3 e00c8fe5-d49e-42d1-9bfb-4965b9ab75b3 e2223 [1514,2223] lci=[2123,2223])
2383:2020-09-11 14:05:19.113938 7fba46937700 10 osd.3 2222 do_waiters -- start
2384:2020-09-11 14:05:19.113946 7fba46937700 10 osd.3 2222 do_waiters -- finish 
2385:2020-09-11 14:05:19.127142 7fba49ec6700 10 osd.3 2222 _committed_osd_maps 2223..2223
2386:2020-09-11 14:05:19.127165 7fba49ec6700 10 osd.3 2222  advance to epoch 2223 (<= last 2223 <= newest_map 2223)
2387:2020-09-11 14:05:19.127175 7fba49ec6700 30 osd.3 2222 get_map 2223 -cached
2388:2020-09-11 14:05:19.127331 7fba49ec6700  7 osd.3 2223 consume_map version 2223
2422:2020-09-11 14:05:19.127924 7fba49ec6700 30 osd.3 pg_epoch: 2222 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] lock
2588:2020-09-11 14:05:19.130667 7fba49ec6700 30 osd.3 pg_epoch: 2222 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] lock
2589:2020-09-11 14:05:19.130679 7fba49ec6700 10 osd.3 pg_epoch: 2222 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] null
3126:2020-09-11 14:05:19.133146 7fba49ec6700 20 osd.3 2223 11.4 heartbeat_peers 0,2,3
{% endhighlight %}
1) **osd3接收到CEPH_MSG_OSD_MAP消息**
{% highlight string %}
void OSD::_dispatch(Message *m)
{
	dout(20) << "_dispatch " << m << " " << *m << dendl;

	switch(m->get_type())
	{
		case CEPH_MSG_OSD_MAP:
    	handle_osd_map(static_cast<MOSDMap*>(m));
    	break;
	}

	...
}
{% endhighlight %}

2) **handle_osd_map()解析osdmap消息，并写入superblock**
{% highlight string %}
void OSD::handle_osd_map(MOSDMap *m){
	...
	epoch_t first = m->get_first();
	epoch_t last = m->get_last();
	dout(3) << "handle_osd_map epochs [" << first << "," << last << "], i have "
		<< superblock.newest_map
		<< ", src has [" << m->oldest_map << "," << m->newest_map << "]"
		<< dendl;


	...

	// store new maps: queue for disk and put in the osdmap cache
	epoch_t start = MAX(superblock.newest_map + 1, first);
	for (epoch_t e = start; e <= last; e++) {
		map<epoch_t,bufferlist>::iterator p;
		p = m->maps.find(e);
		if (p != m->maps.end()) {
			dout(10) << "handle_osd_map  got full map for epoch " << e << dendl;
			OSDMap *o = new OSDMap;
			bufferlist& bl = p->second;
	
			o->decode(bl);
	
			ghobject_t fulloid = get_osdmap_pobject_name(e);
			t.write(coll_t::meta(), fulloid, 0, bl.length(), bl);
			pin_map_bl(e, bl);
			pinned_maps.push_back(add_map(o));
	
			got_full_map(e);
			continue;
		}

		...
	}

	...
	// superblock and commit
	write_superblock(t);
	store->queue_transaction(
		service.meta_osr.get(),
		std::move(t),
		new C_OnMapApply(&service, pinned_maps, last),
		new C_OnMapCommit(this, start, last, m), 0);
}
{% endhighlight %}
在handle_osd_map()函数中解析收到的MOSDMap消息，然后写入superblock中，提交成功后回调C_OnMapCommit()，并触发调用_committed_osd_maps().

3) **_committed_osd_maps()**
{% highlight string %}
void OSD::_committed_osd_maps(epoch_t first, epoch_t last, MOSDMap *m){
	...

	// advance through the new maps
	for (epoch_t cur = first; cur <= last; cur++) {
		dout(10) << " advance to epoch " << cur
			<< " (<= last " << last
			<< " <= newest_map " << superblock.newest_map
			<< ")" << dendl;

		OSDMapRef newmap = get_map(cur);
	}

	// yay!
	consume_map();

	...
}
{% endhighlight %}
本函数用于消费被保存的osdmap，其具体实现为consume_map():
{% highlight string %}
void OSD::consume_map()
{
	assert(osd_lock.is_locked());
	dout(7) << "consume_map version " << osdmap->get_epoch() << dendl;

	// scan pg's
	{
		RWLock::RLocker l(pg_map_lock);
		for (ceph::unordered_map<spg_t,PG*>::iterator it = pg_map.begin();it != pg_map.end();++it) {
			PG *pg = it->second;
			pg->lock();
	
			....
		}
	}

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
}
{% endhighlight %}
程序运行到这里，就构造了一个NullEvt到OSD的消息队列，从而触发相应的Peering流程。

### 1.2 进入peering流程
{% highlight string %}
3478:2020-09-11 14:05:19.134829 7fba3d124700 30 osd.3 pg_epoch: 2222 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] lock
3482:2020-09-11 14:05:19.134849 7fba3d124700 10 osd.3 pg_epoch: 2222 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] handle_advance_map [3]/[3] -- 3/3
3485:2020-09-11 14:05:19.134866 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] state<Started>: Started advmap
3487:2020-09-11 14:05:19.134879 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] new interval newup [3] newacting [3]
3489:2020-09-11 14:05:19.134892 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] state<Started>: should_restart_peering, transitioning to Reset
3491:2020-09-11 14:05:19.134905 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] exit Started/ReplicaActive/RepNotRecovering 6737.966817 4 0.000070
3493:2020-09-11 14:05:19.134920 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] exit Started/ReplicaActive 6737.966836 0 0.000000
3495:2020-09-11 14:05:19.134936 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] exit Started 6739.036070 0 0.000000
3497:2020-09-11 14:05:19.134951 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] enter Reset
3498:2020-09-11 14:05:19.134961 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] set_last_peering_reset 2223
3499:2020-09-11 14:05:19.134971 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2223 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] Clearing blocked outgoing recovery messages
3504:2020-09-11 14:05:19.134982 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2223 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] Not blocking outgoing recovery messages
3506:2020-09-11 14:05:19.134992 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2223 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] state<Reset>: Reset advmap
3508:2020-09-11 14:05:19.135007 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2223 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] _calc_past_interval_range: already have past intervals back to 2222
3509:2020-09-11 14:05:19.135020 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2223 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] new interval newup [3] newacting [3]
3511:2020-09-11 14:05:19.135030 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2223 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] state<Reset>: should restart peering, calling start_peering_interval again
3512:2020-09-11 14:05:19.135040 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2223 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] set_last_peering_reset 2223
3515:2020-09-11 14:05:19.135065 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [3] r=0 lpr=2223 pi=2215-2222/3 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] start_peering_interval: check_new_interval output: generate_past_intervals interval(2221-2222 up [0,3](0) acting [0,3](0)): not rw, up_thru 2221 up_from 2220 last_epoch_clean 2222
3519:2020-09-11 14:05:19.135076 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [3] r=0 lpr=2223 pi=2215-2222/3 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active]  noting past interval(2221-2222 up [0,3](0) acting [0,3](0) maybe_went_rw)
3521:2020-09-11 14:05:19.135094 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active]  up [0,3] -> [3], acting [0,3] -> [3], acting_primary 0 -> 3, up_primary 0 -> 3, role 1 -> 0, features acting 576460752032874495 upacting 576460752032874495
3522:2020-09-11 14:05:19.135109 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] clear_primary_state
3524:2020-09-11 14:05:19.135133 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] agent_stop
3525:2020-09-11 14:05:19.135142 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_change
3527:2020-09-11 14:05:19.135151 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
3528:2020-09-11 14:05:19.135162 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] publish_stats_to_osd 2223:1329
3530:2020-09-11 14:05:19.135172 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
3531:2020-09-11 14:05:19.135184 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_copy_ops
3533:2020-09-11 14:05:19.135194 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_flush_ops
3534:2020-09-11 14:05:19.135205 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_proxy_ops
3536:2020-09-11 14:05:19.135215 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
3537:2020-09-11 14:05:19.135225 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
3538:2020-09-11 14:05:19.135233 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
3539:2020-09-11 14:05:19.135242 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_change_cleanup
3540:2020-09-11 14:05:19.135251 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_change
3542:2020-09-11 14:05:19.135263 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit NotTrimming
3543:2020-09-11 14:05:19.135272 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter NotTrimming
3544:2020-09-11 14:05:19.135281 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] clear_stats
3545:2020-09-11 14:05:19.135290 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
3546:2020-09-11 14:05:19.135302 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_role_change
3547:2020-09-11 14:05:19.135312 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
3548:2020-09-11 14:05:19.135322 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_recovery
3551:2020-09-11 14:05:19.135331 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] clear_recovery_state
3552:2020-09-11 14:05:19.135342 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] check_recovery_sources no source osds () went down
3554:2020-09-11 14:05:19.135354 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] handle_activate_map 
3556:2020-09-11 14:05:19.135365 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] update_heartbeat_peers 0,2,3 -> 3
3559:2020-09-11 14:05:19.135379 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] take_waiters
3561:2020-09-11 14:05:19.135390 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit Reset 0.000439 1 0.000486
3563:2020-09-11 14:05:19.135404 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started
3565:2020-09-11 14:05:19.135414 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Start
3566:2020-09-11 14:05:19.135425 7fba3d124700  1 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] state<Start>: transitioning to Primary
3569:2020-09-11 14:05:19.135439 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit Start 0.000024 0 0.000000
3571:2020-09-11 14:05:19.135454 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started/Primary
3574:2020-09-11 14:05:19.135465 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started/Primary/Peering
3575:2020-09-11 14:05:19.135478 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetInfo
3578:2020-09-11 14:05:19.135490 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] _calc_past_interval_range: already have past intervals back to 2222
3579:2020-09-11 14:05:19.135505 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2221-2222 up [0,3](0) acting [0,3](0) maybe_went_rw)
3581:2020-09-11 14:05:19.135519 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior  prior osd.0 is down
3583:2020-09-11 14:05:19.135531 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2220-2220 up [0,3](0) acting [3,2](3))
3585:2020-09-11 14:05:19.135545 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior final: probe 3 down 0 blocked_by {}
3587:2020-09-11 14:05:19.135556 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] up_thru 2221 < same_since 2223, must notify monitor
3589:2020-09-11 14:05:19.135569 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2223:1330
3591:2020-09-11 14:05:19.135583 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetInfo 0.000104 0 0.000000
3593:2020-09-11 14:05:19.135595 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetLog
3596:2020-09-11 14:05:19.135609 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting osd.3 11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223)
3599:2020-09-11 14:05:19.135632 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting newest update on osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223)
3601:calc_acting primary is osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223)
3603:2020-09-11 14:05:19.135659 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] actingbackfill is 3
3605:2020-09-11 14:05:19.135668 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] choose_acting want [3] (== acting) backfill_targets 
3607:2020-09-11 14:05:19.135684 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetLog>: leaving GetLog
3609:2020-09-11 14:05:19.135698 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetLog 0.000102 0 0.000000
3611:2020-09-11 14:05:19.135712 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2223: no change since 2020-09-11 14:05:19.135569
3613:2020-09-11 14:05:19.135732 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetMissing
3614:2020-09-11 14:05:19.135743 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetMissing>:  still need up_thru update before going active
3616:2020-09-11 14:05:19.135755 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetMissing 0.000023 0 0.000000
3618:2020-09-11 14:05:19.135768 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2223: no change since 2020-09-11 14:05:19.135569
3620:2020-09-11 14:05:19.135782 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/WaitUpThru
3622:2020-09-11 14:05:19.135793 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_activate_map: Not dirtying info: last_persisted is 2222 while current is 2223
3626:2020-09-11 14:05:19.135811 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2223 epoch_requested: 2223 NullEvt
6672:2020-09-11 14:05:19.153839 7fba496c5700 30 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
6674:2020-09-11 14:05:19.153848 7fba496c5700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] flushed
6719:2020-09-11 14:05:19.153979 7fba3d124700 30 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
6720:2020-09-11 14:05:19.153986 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2223 epoch_requested: 2223 FlushedEvt
6722:2020-09-11 14:05:19.153991 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  requeue_ops 
6877:2020-09-11 14:05:19.529657 7fba4fd58700 20 osd.3 2223 11.4 heartbeat_peers 3
7047:2020-09-11 14:05:19.862229 7fba49ec6700 30 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
7209:2020-09-11 14:05:19.863342 7fba49ec6700 30 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
7210:2020-09-11 14:05:19.863350 7fba49ec6700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] null
7951:2020-09-11 14:05:19.867042 7fba3d925700 30 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
7953:2020-09-11 14:05:19.867061 7fba3d925700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_advance_map [3]/[3] -- 3/3
7955:2020-09-11 14:05:19.867079 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering>: Peering advmap
7956:2020-09-11 14:05:19.867091 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] adjust_need_up_thru now 2223, need_up_thru now false
7957:2020-09-11 14:05:19.867102 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started>: Started advmap
7958:2020-09-11 14:05:19.867134 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] check_recovery_sources no source osds () went down
7959:2020-09-11 14:05:19.867154 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_activate_map 
7960:2020-09-11 14:05:19.867166 7fba3d925700  7 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary>: handle ActMap primary
7961:2020-09-11 14:05:19.867178 7fba3d925700 15 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2223: no change since 2020-09-11 14:05:19.135569
7962:2020-09-11 14:05:19.867196 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] take_waiters
7963:2020-09-11 14:05:19.867209 7fba3d925700  5 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/WaitUpThru 0.731426 4 0.000514
7964:2020-09-11 14:05:19.867224 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering>: Leaving Peering
7965:2020-09-11 14:05:19.867235 7fba3d925700  5 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering 0.731769 0 0.000000
7966:2020-09-11 14:05:19.867250 7fba3d925700  5 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started/Primary/Active
7967:2020-09-11 14:05:19.867262 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] state<Started/Primary/Active>: In Active, about to call activate
7968:2020-09-11 14:05:19.867277 7fba3d925700 20 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate - purged_snaps [] cached_removed_snaps []
7969:2020-09-11 14:05:19.867288 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate - snap_trimq []
7970:2020-09-11 14:05:19.867298 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate - no missing, moving last_complete 201'1 -> 201'1
7971:2020-09-11 14:05:19.867310 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] needs_recovery is recovered
7972:2020-09-11 14:05:19.867322 7fba3d925700 15 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] publish_stats_to_osd 2224:1331
7973:2020-09-11 14:05:19.867334 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] state<Started/Primary/Active>: Activate Finished
7974:2020-09-11 14:05:19.867346 7fba3d925700  5 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] enter Started/Primary/Active/Activating
7975:2020-09-11 14:05:19.867358 7fba3d925700 20 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] handle_activate_map: Not dirtying info: last_persisted is 2223 while current is 2224
7976:2020-09-11 14:05:19.867368 7fba3d925700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] handle_peering_event: epoch_sent: 2224 epoch_requested: 2224 NullEvt
9530:2020-09-11 14:05:19.902099 7fba49ec6700 30 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] lock
9532:2020-09-11 14:05:19.902107 7fba49ec6700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] _activate_committed 2224 peer_activated now 3 last_epoch_started 2222 same_interval_since 2223
9533:2020-09-11 14:05:19.902120 7fba49ec6700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] all_activated_and_committed
9535:2020-09-11 14:05:19.902127 7fba49ec6700 30 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] lock
9536:2020-09-11 14:05:19.902133 7fba49ec6700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] flushed
9709:2020-09-11 14:05:19.902680 7fba3d124700 30 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] lock
9713:2020-09-11 14:05:19.902690 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 activating+undersized+degraded] handle_peering_event: epoch_sent: 2224 epoch_requested: 2224 AllReplicasActivated
9715:2020-09-11 14:05:19.902700 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] share_pg_info
9718:2020-09-11 14:05:19.902709 7fba3d124700 15 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] publish_stats_to_osd 2224:1332
9721:2020-09-11 14:05:19.902719 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] check_local
9724:2020-09-11 14:05:19.902727 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] needs_recovery is recovered
9727:2020-09-11 14:05:19.902736 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] needs_backfill does not need backfill
9730:2020-09-11 14:05:19.902745 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] activate all replicas clean, no recovery
9734:2020-09-11 14:05:19.902757 7fba3d124700 15 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] publish_stats_to_osd 2224: no change since 2020-09-11 14:05:19.902708
9736:2020-09-11 14:05:19.902765 7fba3d124700 20 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] hit_set_clear
9738:2020-09-11 14:05:19.902772 7fba3d124700 20 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] agent_stop
9747:2020-09-11 14:05:19.902799 7fba3d124700 30 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
9750:2020-09-11 14:05:19.902808 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] handle_peering_event: epoch_sent: 2224 epoch_requested: 2224 FlushedEvt
9753:2020-09-11 14:05:19.902817 7fba3d124700 15 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded]  requeue_ops 
10071:2020-09-11 14:05:19.904183 7fba3d124700 30 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
10072:2020-09-11 14:05:19.904190 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] handle_peering_event: epoch_sent: 2224 epoch_requested: 2224 AllReplicasRecovered
10073:2020-09-11 14:05:19.904198 7fba3d124700  5 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] exit Started/Primary/Active/Activating 0.036851 4 0.000283
10076:2020-09-11 14:05:19.904207 7fba3d124700  5 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] enter Started/Primary/Active/Recovered
10078:2020-09-11 14:05:19.904216 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] needs_recovery is recovered
10080:2020-09-11 14:05:19.904225 7fba3d124700  5 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] exit Started/Primary/Active/Recovered 0.000018 0 0.000000
10082:2020-09-11 14:05:19.904234 7fba3d124700  5 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] enter Started/Primary/Active/Clean
10084:2020-09-11 14:05:19.904244 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] finish_recovery
10086:2020-09-11 14:05:19.904251 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] clear_recovery_state
10088:2020-09-11 14:05:19.904260 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] trim_past_intervals: trimming interval(2215-2219 up [3,2](3) acting [3,2](3) maybe_went_rw)
10091:2020-09-11 14:05:19.904269 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2220-2222/2 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] trim_past_intervals: trimming interval(2220-2220 up [0,3](0) acting [3,2](3))
10093:2020-09-11 14:05:19.904279 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2221-2222/1 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] trim_past_intervals: trimming interval(2221-2222 up [0,3](0) acting [0,3](0) maybe_went_rw)
10095:2020-09-11 14:05:19.904288 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] share_pg_info
10096:2020-09-11 14:05:19.904298 7fba3d124700 15 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] publish_stats_to_osd 2224:1333
10449:2020-09-11 14:05:20.014523 7fba49ec6700 30 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
10450:2020-09-11 14:05:20.014538 7fba49ec6700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] _finish_recovery
10451:2020-09-11 14:05:20.014548 7fba49ec6700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] purge_strays 
10452:2020-09-11 14:05:20.014559 7fba49ec6700 15 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] publish_stats_to_osd 2224: no change since 2020-09-11 14:05:19.904297
10587:2020-09-11 14:05:20.681012 7fba4f557700 25 osd.3 2224  sending 11.4 2224:1333
10674:2020-09-11 14:05:20.893262 7fba46937700 25 osd.3 2224  ack on 11.4 2224:1333
10825:2020-09-11 14:05:21.673551 7fba49ec6700 30 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
11084:2020-09-11 14:05:21.675592 7fba49ec6700 30 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
11088:2020-09-11 14:05:21.675608 7fba49ec6700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] null
11643:2020-09-11 14:05:21.678631 7fba3d124700 30 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
11651:2020-09-11 14:05:21.678651 7fba3d124700 10 osd.3 pg_epoch: 2224 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] handle_advance_map [3]/[3] -- 3/3
11656:2020-09-11 14:05:21.678682 7fba3d124700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] state<Started/Primary/Active>: Active advmap
11660:2020-09-11 14:05:21.678697 7fba3d124700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] state<Started>: Started advmap
11663:2020-09-11 14:05:21.678711 7fba3d124700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] check_recovery_sources no source osds () went down
11668:2020-09-11 14:05:21.678729 7fba3d124700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] handle_activate_map 
11672:2020-09-11 14:05:21.678743 7fba3d124700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] state<Started/Primary/Active>: Active: handling ActMap
11679:2020-09-11 14:05:21.678768 7fba3d124700 10 osd.3 2225 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded]
11683:2020-09-11 14:05:21.678778 7fba3d124700  7 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] state<Started/Primary>: handle ActMap primary
11689:2020-09-11 14:05:21.678793 7fba3d124700 15 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] publish_stats_to_osd 2224: no change since 2020-09-11 14:05:19.904297
11692:2020-09-11 14:05:21.678810 7fba3d124700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] take_waiters
11694:2020-09-11 14:05:21.678822 7fba3d124700 20 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] handle_activate_map: Not dirtying info: last_persisted is 2224 while current is 2225
11697:2020-09-11 14:05:21.678833 7fba3d124700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] handle_peering_event: epoch_sent: 2225 epoch_requested: 2225 NullEvt
11703:2020-09-11 14:05:21.678856 7fba37919700 30 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
11706:2020-09-11 14:05:21.678865 7fba37919700 10 osd.3 2225 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded]
11709:2020-09-11 14:05:21.678871 7fba37919700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] recovery raced and were queued twice, ignoring!
11710:2020-09-11 14:05:21.678878 7fba37919700 10 osd.3 2225 do_recovery started 0/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded]
{% endhighlight %}
在上面将PG加入到OSD的peering_wq之后，OSD对应的线程池就会调用process_peering_events()函数来对相应的PG进行处理：
{% highlight string %}
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...

	for (list<PG*>::const_iterator i = pgs.begin();i != pgs.end();++i) {

		...
		
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
	}

	....
}
{% endhighlight %}


###### 1.2.1 OSD::advance_pg()函数
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
在advance_pg()函数中，PG对每一个epoch的osdmap进行处理： 首先调用pg_to_up_acting_osds()来计算```PG11.4```所对应的up set和acting set；然后调用handle_advance_map()来触发peering； 之后会调用handle_activate_map()来触发```peering完成后```的一些其他工作。

###### 1.2.2 PG::handle_advance_map()函数
{% highlight string %}
void PG::handle_advance_map(
  OSDMapRef osdmap, OSDMapRef lastmap,
  vector<int>& newup, int up_primary,
  vector<int>& newacting, int acting_primary,
  RecoveryCtx *rctx)
{
	dout(10) << "handle_advance_map "
		<< newup << "/" << newacting
		<< " -- " << up_primary << "/" << acting_primary
		<< dendl;

	...
	
	AdvMap evt(
		osdmap, lastmap, newup, up_primary,
		newacting, acting_primary);
	recovery_state.handle_event(evt, rctx);
	...
}
{% endhighlight %}
这里```PG11.4```对应的up set为[3], acting set也为[3]，up_primary为3，acting_primary也为3。之后生成一个```AdvMap```事件，投递到```recovery_state```中，最终出发新的Peering过程。


### 1.3 peering状态机运转

###### 1.3.1 ReplicaActive状态对AdvMap事件的处理
由于PG 11.4最开始的的up set为[0,3]， acting set为[0,3]，因此在active + clean状态时，该PG在OSD3上是一个replica，其所属的状态也应该是```ReplicaActive```状态。下面我们来看一下，当处于ReplicaActive状态时，下时收到AdvMap事件的处理：在ReplicaActive状态下没有对AdvMap事件的直接的处理函数，因此会调用其父状态```Started```状态的处理函数:
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
在对应的react()函数中调用should_restart_peering()来检查是否需要重新启动peering操作。下面我们来看一下该函数的实现：
{% highlight string %}
bool PG::should_restart_peering(
  int newupprimary,
  int newactingprimary,
  const vector<int>& newup,
  const vector<int>& newacting,
  OSDMapRef lastmap,
  OSDMapRef osdmap)
{
  if (pg_interval_t::is_new_interval(
	primary.osd,
	newactingprimary,
	acting,
	newacting,
	up_primary.osd,
	newupprimary,
	up,
	newup,
	osdmap,
	lastmap,
	info.pgid.pgid)) {
    dout(20) << "new interval newup " << newup
	     << " newacting " << newacting << dendl;
    return true;
  } else {
    return false;
  }
}
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
	...
}
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
这里old_acting_primary为0， new_acting_primary为3； old_acting为[0,3]，new_acting为[3]； old_up_primary为0，new_up_primary为3； old_up为[0,3]，new_up为[3]。参看代码，满足如何任何一个条件即可判断为new_interval:

* PG的acting primary发生了改变；

* PG的acting set发生了改变；

* PG的up primary发生了改变；

* PG的up set发生了改变；

* PG的min_size发生了改变（通常是调整了rule规则）；

* PG的副本size值发生了改变；

* PG进行了分裂

* PG的sort bitwise发生了改变

>注：关于sort bitwise，请参看[sort bitwise是什么意思](https://ceph.com/planet/sortbitwise%E6%98%AF%E4%BB%80%E4%B9%88%E6%84%8F%E6%80%9D/)，下面是一个对sort bitwise作用的一个大体描述
>
>After upgrading, users should set the ‘sortbitwise’ flag to enable the new internal object sort order: ceph osd set sortbitwise
>
>This flag is important for the new object enumeration API and for new backends like BlueStore

因此，这里上面should_restart_peering()返回值肯定为true，因此将```AdvMap```事件继续投递，进入Reset状态。由于原来PG 11.4处于RepNotRecovering状态，因此这里转入Reset状态，会首先退出子状态，打印出如下语句：
{% highlight string %}
3491:2020-09-11 14:05:19.134905 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] exit Started/ReplicaActive/RepNotRecovering 6737.966817 4 0.000070
3493:2020-09-11 14:05:19.134920 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] exit Started/ReplicaActive 6737.966836 0 0.000000
3495:2020-09-11 14:05:19.134936 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2220/2221/2221) [0,3] r=1 lpr=2221 pi=2215-2220/2 luod=0'0 crt=201'1 lcod 0'0 active] exit Started 6739.036070 0 0.000000
{% endhighlight %}

###### 1.3.2 Reset状态对AdvMap事件的处理
在上面ReplicaActive接收到AdvMap事件，从而引发进入到Reset状态后：
{% highlight string %}
PG::RecoveryState::Reset::Reset(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Reset")
{
  context< RecoveryMachine >().log_enter(state_name);
  PG *pg = context< RecoveryMachine >().pg;

  pg->flushes_in_progress = 0;
  pg->set_last_peering_reset();
}
{% endhighlight %}
在Reset状态的构造函数中，会调用set_last_peering_reset()清空上一次的peering信息。之后再调用Reset::react()来处理post_event()投递进来的AdvMap事件：
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
    dout(10) << "should restart peering, calling start_peering_interval again"
	     << dendl;
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

1） **generate_past_intervals()函数**

在Reset::react(const AdvMap&)函数中，首先调用generate_past_interval():
{% highlight string %}
void PG::generate_past_intervals(){
	....

	if (!_calc_past_interval_range(&cur_epoch, &end_epoch,
		osd->get_superblock().oldest_map)) {
		if (info.history.same_interval_since == 0) {
			info.history.same_interval_since = end_epoch;
			dirty_info = true;
		}
		return;
	}

	...
}
bool PG::_calc_past_interval_range(epoch_t *start, epoch_t *end, epoch_t oldest_map)
{
	...

	// Do we already have the intervals we want?
	map<epoch_t,pg_interval_t>::const_iterator pif = past_intervals.begin();
	if (pif != past_intervals.end()) {
		if (pif->first <= info.history.last_epoch_clean) {
			dout(10) << __func__ << ": already have past intervals back to "
				<< info.history.last_epoch_clean << dendl;
			return false;
		}
		*end = past_intervals.begin()->first;
	}
	
	...
}
{% endhighlight %}
函数_calc_past_interval_range()用于计算一个past_interval的范围。所谓一个past_interval，是指一个连续的epoch段[epoch_start, epoch_end]，在该epoch段内：

* PG的acting primary保持不变；

* PG的acting set保持不变；

* PG的up primary保持不变；

* PG的up set保持不变；

* PG的min_size保持不变；

* PG的副本size值保持不变；

* PG没有进行分裂；

* PG的sort bitwise保持不变；

函数_calc_past_interval_range()如果返回true，表示成功计算到一个past_interval；如果返回false，则表示该interval已经计算过，不必再计算，或者不是一个有效的past_interval。

查询_calc_past_interval_range()，发现只有在如下几个地方会被调用到：

* Reset::react(AdvMap)函数中generate_past_intervals()

* GetInfo::GetInfo(my_context)构造函数中调用generate_past_intervals()

* OSD::build_past_intervals_parallel()函数中（注： 本函数由OSD::load_pgs()在启动时所调用)

从上面三种情况得知，past_interval只会在OSD启动时、或者触发Peering操作时被调用。这里我们结合```osd3_watch_pg_11.4.txt```日志，来看一下这里的执行过程：

![past-interval-brief](https://ivanzz1001.github.io/records/assets/img/ceph/sca/past_interval_brief.jpg)

由此，我们在这里计算past_interval返回false:
<pre>
_calc_past_interval_range: already have past intervals back to 2222
</pre>


2） **start_peering_interval()函数**
{% highlight string %}
void PG::start_peering_interval(
  const OSDMapRef lastmap,
  const vector<int>& newup, int new_up_primary,
  const vector<int>& newacting, int new_acting_primary,
  ObjectStore::Transaction *t)
{
	...

	set_last_peering_reset();

	...

	init_primary_up_acting(
		newup,
		newacting,
		new_up_primary,
		new_acting_primary);

	....
	if(!lastmap){
		
	}else{
		...
		pg_interval_t::check_new_interval(....);

		dout(10) << __func__ << ": check_new_interval output: "
			<< debug.str() << dendl;
		if (new_interval) {
			dout(10) << " noting past " << past_intervals.rbegin()->second << dendl;
			dirty_info = true;
			dirty_big_info = true;
			info.history.same_interval_since = osdmap->get_epoch();
		}
	}

	...

	dout(10) << " up " << oldup << " -> " << up 
		<< ", acting " << oldacting << " -> " << acting 
		<< ", acting_primary " << old_acting_primary << " -> " << new_acting_primary
		<< ", up_primary " << old_up_primary << " -> " << new_up_primary
		<< ", role " << oldrole << " -> " << role
		<< ", features acting " << acting_features
		<< " upacting " << upacting_features
		<< dendl;
	
	// deactivate.
	state_clear(PG_STATE_ACTIVE);
	state_clear(PG_STATE_PEERED);
	state_clear(PG_STATE_DOWN);
	state_clear(PG_STATE_RECOVERY_WAIT);
	state_clear(PG_STATE_RECOVERING);
	
	peer_purged.clear();
	actingbackfill.clear();
	snap_trim_queued = false;
	scrub_queued = false;
	
	// reset primary state?
	if (was_old_primary || is_primary()) {
		osd->remove_want_pg_temp(info.pgid.pgid);
	}
	clear_primary_state();

	// pg->on_*
	on_change(t);

	assert(!deleting);
	
	// should we tell the primary we are here?
	send_notify = !is_primary();
	
	if (role != oldrole || was_old_primary != is_primary()) {
		// did primary change?
		if (was_old_primary != is_primary()) {
			state_clear(PG_STATE_CLEAN);
			clear_publish_stats();
	
			// take replay queue waiters
			list<OpRequestRef> ls;

			for (map<eversion_t,OpRequestRef>::iterator it = replay_queue.begin();it != replay_queue.end();++it)
				ls.push_back(it->second);

			replay_queue.clear();
			requeue_ops(ls);
		}
	
		on_role_change();
	
		// take active waiters
		requeue_ops(waiting_for_peered);
	
	} else {
		....
	}

	cancel_recovery();
	....
}
{% endhighlight %}
在收到osdmap，从而触发peering操作之前，调用start_peering_interval()函数，完成相关状态的重新设置。

* set_last_peering_reset()用于清空上一次的peering数据；

* init_primary_up_acting()用于设置当前新的acting set以及up set

* check_new_interval(): 用于检查是否是一个新的interval。如果是新interval，则计算出该新的past_interval：

>generate_past_intervals interval(2221-2222 up [0,3](0) acting [0,3](0)): not rw, up_thru 2221 up_from 2220 last_epoch_clean 2222

在该interval[2221,2222]下，up set为[0,3]，acting set为[0,3]，并且up_thru为2221。而当前在epoch 2223下，up set变为[3]，acting set为[3]，因此对于该PG来说，OSD3的角色由```1```变为```0```，即由replica变为primary。


* 清除PG的相关状态。关于PG的状态是由一个```unsigned```类型的整数来表示的
{% highlight string %}
/*
 * pg states
 */
#define PG_STATE_CREATING     (1<<0)  				// creating
#define PG_STATE_ACTIVE       (1<<1)  				// i am active.  (primary: replicas too)
#define PG_STATE_CLEAN        (1<<2)  				// peers are complete, clean of stray replicas.
#define PG_STATE_DOWN         (1<<4) 				 // a needed replica is down, PG offline
#define PG_STATE_REPLAY       (1<<5) 				 // crashed, waiting for replay
//#define PG_STATE_STRAY      (1<<6)  				// i must notify the primary i exist.
#define PG_STATE_SPLITTING    (1<<7)  				// i am splitting
#define PG_STATE_SCRUBBING    (1<<8)  				// scrubbing
#define PG_STATE_SCRUBQ       (1<<9)  				// queued for scrub
#define PG_STATE_DEGRADED     (1<<10) 				// pg contains objects with reduced redundancy
#define PG_STATE_INCONSISTENT (1<<11) 				// pg replicas are inconsistent (but shouldn't be)
#define PG_STATE_PEERING      (1<<12)				// pg is (re)peering
#define PG_STATE_REPAIR       (1<<13) 				// pg should repair on next scrub
#define PG_STATE_RECOVERING   (1<<14) 				// pg is recovering/migrating objects
#define PG_STATE_BACKFILL_WAIT     (1<<15) 			// [active] reserving backfill
#define PG_STATE_INCOMPLETE   (1<<16) 				// incomplete content, peering failed.
#define PG_STATE_STALE        (1<<17) 				// our state for this pg is stale, unknown.
#define PG_STATE_REMAPPED     (1<<18) 				// pg is explicitly remapped to different OSDs than CRUSH
#define PG_STATE_DEEP_SCRUB   (1<<19) 				// deep scrub: check CRC32 on files
#define PG_STATE_BACKFILL  (1<<20) 					// [active] backfilling pg content
#define PG_STATE_BACKFILL_TOOFULL (1<<21) 			// backfill can't proceed: too full
#define PG_STATE_RECOVERY_WAIT (1<<22) 				// waiting for recovery reservations
#define PG_STATE_UNDERSIZED    (1<<23) 				// pg acting < pool size
#define PG_STATE_ACTIVATING   (1<<24) 				// pg is peered but not yet active
#define PG_STATE_PEERED        (1<<25) 				// peered, cannot go active, can recover
#define PG_STATE_SNAPTRIM      (1<<26) 				// trimming snaps
#define PG_STATE_SNAPTRIM_WAIT (1<<27) 				// queued to trim snaps
{% endhighlight %}
这里将```active```状态清除了，因此变为```inactive```状态。

* clear_primary_state(): 用于清空peering状态

* PG 11.4所在OSD3从replica变为primary，因此这里还要处理大量因角色变化而产生的操作；

* cancel_recovery(): 取消PG当前正在进行中的recovery动作；


到此为止，由OSD::advance_pg()函数中pg->handle_advance_map()所触发的AdvMap事件就已经处理完成。

###### 1.3.3 进入Started状态

在完成上面的Reset状态的操作之后，接着在OSD::advance_pg()函数中通过pg->handle_activate_map()函数，从而引发进入Started状态。如下是这一过程的一个日志片段：
{% highlight string %}
3554:2020-09-11 14:05:19.135354 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] handle_activate_map 
3556:2020-09-11 14:05:19.135365 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] update_heartbeat_peers 0,2,3 -> 3
3559:2020-09-11 14:05:19.135379 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] take_waiters
3561:2020-09-11 14:05:19.135390 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit Reset 0.000439 1 0.000486
3563:2020-09-11 14:05:19.135404 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started
3565:2020-09-11 14:05:19.135414 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Start
3566:2020-09-11 14:05:19.135425 7fba3d124700  1 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] state<Start>: transitioning to Primary
3569:2020-09-11 14:05:19.135439 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit Start 0.000024 0 0.000000
3571:2020-09-11 14:05:19.135454 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started/Primary
3574:2020-09-11 14:05:19.135465 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started/Primary/Peering
3575:2020-09-11 14:05:19.135478 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetInfo
3578:2020-09-11 14:05:19.135490 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] _calc_past_interval_range: already have past intervals back to 2222
3579:2020-09-11 14:05:19.135505 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2221-2222 up [0,3](0) acting [0,3](0) maybe_went_rw)
3581:2020-09-11 14:05:19.135519 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior  prior osd.0 is down
3583:2020-09-11 14:05:19.135531 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2220-2220 up [0,3](0) acting [3,2](3))
3585:2020-09-11 14:05:19.135545 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior final: probe 3 down 0 blocked_by {}
3587:2020-09-11 14:05:19.135556 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] up_thru 2221 < same_since 2223, must notify monitor
3589:2020-09-11 14:05:19.135569 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2223:1330
3591:2020-09-11 14:05:19.135583 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetInfo 0.000104 0 0.000000
3593:2020-09-11 14:05:19.135595 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetLog
3596:2020-09-11 14:05:19.135609 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting osd.3 11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223)
3599:2020-09-11 14:05:19.135632 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting newest update on osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223)
3601:calc_acting primary is osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223)
3603:2020-09-11 14:05:19.135659 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] actingbackfill is 3
3605:2020-09-11 14:05:19.135668 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] choose_acting want [3] (== acting) backfill_targets 
3607:2020-09-11 14:05:19.135684 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetLog>: leaving GetLog
3609:2020-09-11 14:05:19.135698 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetLog 0.000102 0 0.000000
3611:2020-09-11 14:05:19.135712 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2223: no change since 2020-09-11 14:05:19.135569
3613:2020-09-11 14:05:19.135732 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetMissing
3614:2020-09-11 14:05:19.135743 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetMissing>:  still need up_thru update before going active
3616:2020-09-11 14:05:19.135755 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetMissing 0.000023 0 0.000000
3618:2020-09-11 14:05:19.135768 7fba3d124700 15 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2223: no change since 2020-09-11 14:05:19.135569
3620:2020-09-11 14:05:19.135782 7fba3d124700  5 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/WaitUpThru
3622:2020-09-11 14:05:19.135793 7fba3d124700 20 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_activate_map: Not dirtying info: last_persisted is 2222 while current is 2223
{% endhighlight %}

1） **handle_activate_map()函数**
{% highlight string %}
void PG::handle_activate_map(RecoveryCtx *rctx)
{
	dout(10) << "handle_activate_map " << dendl;
	ActMap evt;
	recovery_state.handle_event(evt, rctx);
	if (osdmap_ref->get_epoch() - last_persisted_osdmap_ref->get_epoch() > cct->_conf->osd_pg_epoch_persisted_max_stale) {
		dout(20) << __func__ << ": Dirtying info: last_persisted is "
			<< last_persisted_osdmap_ref->get_epoch()
			<< " while current is " << osdmap_ref->get_epoch() << dendl;

		dirty_info = true;
	} else {
		dout(20) << __func__ << ": Not dirtying info: last_persisted is "
			<< last_persisted_osdmap_ref->get_epoch()
			<< " while current is " << osdmap_ref->get_epoch() << dendl;
	}
	if (osdmap_ref->check_new_blacklist_entries()) check_blacklisted_watchers();
}
{% endhighlight %}
这里handle_activate_map()函数构造了一个ActMap事件，并调用recovery_state.handle_event()来进行处理。

2） **Reset::react(const ActMap &)函数**

到此为止，我们其实还没有退出上一个步骤的```Reset```阶段，因此这个对于ActMap事件的处理，是通过如下函数来进行的：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Reset::react(const ActMap&)
{
	PG *pg = context< RecoveryMachine >().pg;
	if (pg->should_send_notify() && pg->get_primary().osd >= 0) {
		context< RecoveryMachine >().send_notify(
			pg->get_primary(),
			pg_notify_t(
				pg->get_primary().shard, pg->pg_whoami.shard,
				pg->get_osdmap()->get_epoch(),
				pg->get_osdmap()->get_epoch(),
				pg->info),
			pg->past_intervals);
	}
	
	pg->update_heartbeat_peers();
	pg->take_waiters();
	
	return transit< Started >();
}
{% endhighlight %}
通常在osdmap发送变动，从而引发PG的acting set、up set发生变动，对于一个PG的副本OSD通常会发送一个通知消息到PG的Primary OSD。这里由于osd3已经是PG11.4的primary OSD，因此这里不需要发送通知消息。

PG的Primary OSD会与Replica OSDs保持心跳，并且这个心跳是由Primary OSD来主动发出并维护的。这里调用pg->update_heartbeat_peers()来更新心跳信息。

最后，通过transit< Started >()直接进入Started状态

3） **Started::Started()函数**
{% highlight string %}
PG::RecoveryState::Started::Started(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started")
{
  context< RecoveryMachine >().log_enter(state_name);
}
{% endhighlight %}
进入Started状态，首先调用相应的构造函数。

###### 1.3.4 进入Started/Start状态
Started的默认初始状态是```Start```状态。这里我们来看Started/Start状态的构造函数：
{% highlight string %}
PG::RecoveryState::Start::Start(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Start")
{
  context< RecoveryMachine >().log_enter(state_name);

  PG *pg = context< RecoveryMachine >().pg;
  if (pg->is_primary()) {
    dout(1) << "transitioning to Primary" << dendl;
    post_event(MakePrimary());
  } else { //is_stray
    dout(1) << "transitioning to Stray" << dendl; 
    post_event(MakeStray());
  }
}
{% endhighlight %}
进入Start状态后，根据PG所在OSD是否为Primary OSD，从而决定是进入Started/Primary状态，还是Started/Stray状态。这里对于PG 11.4而言，在当前epoch 2223时osd3是Primary OSD，因此进入的是Primary状态。

###### 1.3.5 进入Started/Primary状态
如下是Primary状态的构造函数：
{% highlight string %}
PG::RecoveryState::Primary::Primary(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary")
{
	context< RecoveryMachine >().log_enter(state_name);
	PG *pg = context< RecoveryMachine >().pg;
	assert(pg->want_acting.empty());
	
	// set CREATING bit until we have peered for the first time.
	if (pg->info.history.last_epoch_started == 0) {
		pg->state_set(PG_STATE_CREATING);
		// use the history timestamp, which ultimately comes from the
		// monitor in the create case.
		utime_t t = pg->info.history.last_scrub_stamp;
		pg->info.stats.last_fresh = t;
		pg->info.stats.last_active = t;
		pg->info.stats.last_change = t;
		pg->info.stats.last_peered = t;
		pg->info.stats.last_clean = t;
		pg->info.stats.last_unstale = t;
		pg->info.stats.last_undegraded = t;
		pg->info.stats.last_fullsized = t;
		pg->info.stats.last_scrub_stamp = t;
		pg->info.stats.last_deep_scrub_stamp = t;
		pg->info.stats.last_clean_scrub_stamp = t;
	}
}
{% endhighlight %}

这里代码较为简单，不做介绍。

###### 1.3.6 进入Started/Primary/Peering状态
Primary的默认初始子状态为Peering状态，因此这里直接进入Peering状态：
{% highlight string %}
PG::RecoveryState::Peering::Peering(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Peering"),
    history_les_bound(false)
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	assert(!pg->is_peered());
	assert(!pg->is_peering());
	assert(pg->is_primary());
	pg->state_set(PG_STATE_PEERING);
}
{% endhighlight %}
进入Peering状态后，就将PG的状态通过pg->state_set()设置为了PEERING了。

###### 1.3.7 进入Started/Primary/Peering/GetInfo状态
Peering的默认初始子状态为GetInfo状态，因此这里直接进入GetInfo状态：
{% highlight string %}
/*--------GetInfo---------*/
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
进入GetInfo状态后，首先调用generate_past_intervals()计算past_intervals。这里我们在前面Reset阶段就已经计算出了新的past_interval为[2221,2222]。

1） **build_prior()函数**
{% highlight string %}
void PG::build_prior(std::unique_ptr<PriorSet> &prior_set)
{
	if (1) {
		// sanity check
		for (map<pg_shard_t,pg_info_t>::iterator it = peer_info.begin();it != peer_info.end();++it) {
			assert(info.history.last_epoch_started >= it->second.history.last_epoch_started);
		}
	}

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
	
	if (prior.pg_down) {
		state_set(PG_STATE_DOWN);
	}
	
	if (get_osdmap()->get_up_thru(osd->whoami) < info.history.same_interval_since) {
		dout(10) << "up_thru " << get_osdmap()->get_up_thru(osd->whoami)
			<< " < same_since " << info.history.same_interval_since
			<< ", must notify monitor" << dendl;
			need_up_thru = true;
	} else {
		dout(10) << "up_thru " << get_osdmap()->get_up_thru(osd->whoami)
			<< " >= same_since " << info.history.same_interval_since
			<< ", all is well" << dendl;
		need_up_thru = false;
	}
	set_probe_targets(prior_set->probe);
}
{% endhighlight %}
关于PriorSet，我们这里不做介绍，后面我们会再开辟专门的章节来进行说明(参见up_thru)。这里我们给出针对PG 11.4所构建的PriorSet的结果：
{% highlight string %}
3579:2020-09-11 14:05:19.135505 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2221-2222 up [0,3](0) acting [0,3](0) maybe_went_rw)
3581:2020-09-11 14:05:19.135519 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior  prior osd.0 is down
3583:2020-09-11 14:05:19.135531 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2220-2220 up [0,3](0) acting [3,2](3))
3585:2020-09-11 14:05:19.135545 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior final: probe 3 down 0 blocked_by {}
{% endhighlight %}
当前osdmap的epoch为e2223，对于osd3来说其上一次申请的up_thru为e2221，而当前的same_interval_since为e2223，因此这里need_up_thru需要设置为true。

2） **GetInfo::get_infos()**
{% highlight string %}
void PG::RecoveryState::GetInfo::get_infos()
{
	PG *pg = context< RecoveryMachine >().pg;
	unique_ptr<PriorSet> &prior_set = context< Peering >().prior_set;
	
	pg->blocked_by.clear();
	for (set<pg_shard_t>::const_iterator it = prior_set->probe.begin();it != prior_set->probe.end();++it) {
		pg_shard_t peer = *it;
		if (peer == pg->pg_whoami) {
			continue;
		}
		if (pg->peer_info.count(peer)) {
			dout(10) << " have osd." << peer << " info " << pg->peer_info[peer] << dendl;
			continue;
		}

		if (peer_info_requested.count(peer)) {
			dout(10) << " already requested info from osd." << peer << dendl;
			pg->blocked_by.insert(peer.osd);
		} else if (!pg->get_osdmap()->is_up(peer.osd)) {

			dout(10) << " not querying info from down osd." << peer << dendl;

		} else {
			dout(10) << " querying info from osd." << peer << dendl;
			context< RecoveryMachine >().send_query(
				peer, pg_query_t(pg_query_t::INFO,
				it->shard, pg->pg_whoami.shard,
				pg->info.history,
				pg->get_osdmap()->get_epoch()));

			peer_info_requested.insert(peer);
			pg->blocked_by.insert(peer.osd);
		}
	}
	
	pg->publish_stats_to_osd();
}
{% endhighlight %}
这里由上面构造的PriorSet为：
{% highlight string %}
3585:2020-09-11 14:05:19.135545 7fba3d124700 10 osd.3 pg_epoch: 2223 pg[11.4( v 201'1 (0'0,201'1] local-les=2222 n=1 ec=132 les/c/f 2222/2222/0 2223/2223/2223) [3] r=0 lpr=2223 pi=2215-2222/3 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior final: probe 3 down 0 blocked_by {}
{% endhighlight %}

GetInfo()是由PG的主OSD向从OSD发送pg_query_t::INFO消息以获取从OSD的pg_into_t信息。这里对于PG11.4而言，因为目前其prior_set->probe为[3]，因此这里这里在for循环中第一个continue就已经退出了。但在这里我们还是讲述一下GetInfo究竟是要获取哪些info信息，下面我们先来看一下pg_query_t结构体的定义：
{% highlight string %}
/** 
 * pg_query_t - used to ask a peer for information about a pg.
 *
 * note: if version=0, type=LOG, then we just provide our full log.
 */
struct pg_query_t {
	enum {
		INFO = 0,
		LOG = 1,
		MISSING = 4,
		FULLLOG = 5,
	};
	
	__s32 type;
	eversion_t since;
	pg_history_t history;
	epoch_t epoch_sent;
	shard_id_t to;
	shard_id_t from;
};
{% endhighlight %}

如下则是pg_info_t结构体的定义(src/osd/osd_types.h)：
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
	eversion_t last_update;                     ///< last object version applied to store.
	eversion_t last_complete;                  ///< last version pg was complete through.
	epoch_t last_epoch_started;                ///< last epoch at which this pg started on this osd
	
	version_t last_user_version;               ///< last user object version applied to store
	
	eversion_t log_tail;                       ///< oldest log entry.
	
	hobject_t last_backfill;                   ///< objects >= this and < last_complete may be missing
	bool last_backfill_bitwise;                ///< true if last_backfill reflects a bitwise (vs nibblewise) sort
	
	interval_set<snapid_t> purged_snaps;
	
	pg_stat_t stats;
	
	pg_history_t history;
	pg_hit_set_history_t hit_set;
};
{% endhighlight %}



<br />
<br />
<br />
