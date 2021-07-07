---
layout: post
title: ceph之up_thru分析
tags:
- ceph
categories: ceph
description: ceph之up_thru分析
---

本文结合源代码来分析一下ceph中up_thru的作用及其要解决的问题。大家都知道，OSDMap的作用之一便是维护Ceph集群OSD的状态信息，所以基于此想先提出一个疑问： Ceph集群中有1个osd down了，那么osdmap会发生什么变化？ osdmap会更新几次？ 带着这个问题，本文深入探讨up_thru。



<!-- more -->


## 1. up_thru简介
### 1.1 引入up_thru目的
up_thru的引入目的是为了解决如下这类极端场景：

比如集群有两个osd(osd.1, osd.2)共同承载着一批PG来服务业务io。如果osd.1在某个时刻down了，并且随后osd.2也down了，再随后osd.1又up了，那么此时osd.1是否能提供服务？

如果osd.1 down掉期间，osd.2有数据更新，那么显然osd.1再次up后是不能服务的；但是如果osd.2没有数据更新，那么osd.1再次up后是可以提供服务的。


### 1.2 up_thru到底是啥？

要想知道up_thru到底是啥，可以先通过其相关数据结构感受一下，如下：
{% highlight string %}
/*
 * we track up to two intervals during which the osd was alive and
 * healthy.  the most recent is [up_from,up_thru), where up_thru is
 * the last epoch the osd is known to have _started_.  i.e., a lower
 * bound on the actual osd death.  down_at (if it is > up_from) is an
 * upper bound on the actual osd death.
 *
 * the second is the last_clean interval [first,last].  in that case,
 * the last interval is the last epoch known to have been either
 * _finished_, or during which the osd cleanly shut down.  when
 * possible, we push this forward to the epoch the osd was eventually
 * marked down.
 *
 * the lost_at is used to allow build_prior to proceed without waiting
 * for an osd to recover.  In certain cases, progress may be blocked 
 * because an osd is down that may contain updates (i.e., a pg may have
 * gone rw during an interval).  If the osd can't be brought online, we
 * can force things to proceed knowing that we _might_ be losing some
 * acked writes.  If the osd comes back to life later, that's fine to,
 * but those writes will still be lost (the divergent objects will be
 * thrown out).
 */
struct osd_info_t {
	epoch_t last_clean_begin;  // last interval that ended with a clean osd shutdown
	epoch_t last_clean_end;
	epoch_t up_from;          // epoch osd marked up
	epoch_t up_thru;          // lower bound on actual osd death (if > up_from)
	epoch_t down_at;         // upper bound on actual osd death (if > up_from)
	epoch_t lost_at;         // last epoch we decided data was "lost"
};

class OSDMap{
	...

private:
	vector<osd_info_t> osd_info;
	ceph::shared_ptr< map<pg_t,vector<int32_t> > > pg_temp;  // temp pg mapping (e.g. while we rebuild)
	ceph::shared_ptr< map<pg_t,int32_t > > primary_temp;  // temp primary mapping (e.g. while we rebuild)

};
{% endhighlight %}

* last_clean_begin和last_clean_end: 以一个clean的osd关闭而结束的最终区间

* up_from: osd被标记为up时的epoch

* up_thru: 实际osd终止时的下界(如果它大于up_from)

* down_at: 实际osd终止时的上界(如果它大于up_from)

* lost_at: 决定数据被丢失时的epoch。通常是通过```ceph osd lost <id>```命令主动将某一个OSD标记为lost状态，这样在进行数据恢复时就不会等待该OSD上的数据了。

osd_info_t结构用来表述随着时间推移，各个状态的epoch。


在OSD处于alive + healthy期间，我们通常会跟踪两个interval。其中最近(the most recent)一个interval就是[up_from, up_thru)，这里```up_thru```就是OSD被确信进入```_started_```状态时的前一个epoch值，换句话说就是OSD实际进入```death```状态的下边界值。而```down_at```(假设其大于```up_from```的话)就是OSD实际进入```death```状态的上边界值。

另一个则是```last_clean``` interval[first, last]，在这种情况下，last interval就是OSD进入```_finished```状态时的前一个epoch值，或者OSD执行```cleanly``` shutdown的这一段区间。


>注： 关于up_thru的计算，我们会在下文讲述到



```lost_at```的作用在于不必等待OSD恢复，即可允许调用build_prior()来进行处理。在很多情况下，在osd处于down状态时，progress可能会被阻塞（这些操作中可能就包含一些更新操作）。假设OSD并不能回到online状态，我们可以强制继续进行处理，尽管这可能会丢失一些已经```应答```的write操作。稍后假设OSD重新回到线上后，则这些写操作仍然会被丢弃（the divergent objects will be thrown out）。

通过上面的数据结构我们便可以知道，up_thru是作为osd的信息保存在osdmap里的，其类型便是osdmap的版本号(epoch_t)。

既然其是保存在osdmap里面，那么我们可以通过把osdmap dump出来看看，如下：
<pre>
# ceph osd dump
epoch 2222
fsid 5341b139-15dc-4c68-925a-179797d894d3
created 2018-05-11 16:11:51.479785
modified 2020-09-11 12:13:01.076048
flags sortbitwise,require_jewel_osds
pool 11 '.rgw.root' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 170 flags hashpspool stripe_width 0
pool 12 'default.rgw.control' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 172 flags hashpspool stripe_width 0
pool 13 'default.rgw.data.root' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 16 pgp_num 16 last_change 174 flags hashpspool stripe_width 0
pool 14 'default.rgw.gc' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 16 pgp_num 16 last_change 1142 flags hashpspool stripe_width 0
pool 15 'default.rgw.log' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 178 flags hashpspool stripe_width 0
pool 16 'default.rgw.intent-log' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 180 flags hashpspool stripe_width 0
pool 17 'default.rgw.usage' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 182 flags hashpspool stripe_width 0
pool 18 'default.rgw.users.keys' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 184 flags hashpspool stripe_width 0
pool 19 'default.rgw.users.email' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 186 flags hashpspool stripe_width 0
pool 20 'default.rgw.users.swift' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 188 flags hashpspool stripe_width 0
pool 21 'default.rgw.users.uid' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 190 flags hashpspool stripe_width 0
pool 22 'default.rgw.buckets.index' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 256 pgp_num 256 last_change 192 flags hashpspool stripe_width 0
pool 23 'default.rgw.buckets.data' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 256 pgp_num 256 last_change 194 flags hashpspool stripe_width 0
pool 24 'default.rgw.meta' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 197 flags hashpspool stripe_width 0
pool 25 'default.rgw.buckets.non-ec' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 200 flags hashpspool stripe_width 0
max_osd 10
osd.0 up   in  weight 1 up_from 2220 up_thru 2221 down_at 2212 last_clean_interval [2206,2211) 10.17.155.113:6800/29412 10.17.155.113:6801/29412 10.17.155.113:6802/29412 10.17.155.113:6803/29412 exists,up 67990973-0316-4f57-b721-ce61d886572c
osd.1 up   in  weight 1 up_from 2119 up_thru 2220 down_at 2114 last_clean_interval [2045,2113) 10.17.155.113:6808/5393 10.17.155.113:6809/5393 10.17.155.113:6810/5393 10.17.155.113:6811/5393 exists,up e6c55932-820c-474d-aff7-0ed4081f2a33
osd.2 up   in  weight 1 up_from 2118 up_thru 2220 down_at 2093 last_clean_interval [2045,2092) 10.17.155.113:6804/5391 10.17.155.113:6805/5391 10.17.155.113:6806/5391 10.17.155.113:6807/5391 exists,up dfe500d8-5778-4379-9c48-1f1390fa8f0a
osd.3 up   in  weight 1 up_from 2123 up_thru 2221 down_at 2095 last_clean_interval [2041,2093) 10.17.155.114:6800/894 10.17.155.114:6801/894 10.17.155.114:6802/894 10.17.155.114:6803/894 exists,up e00c8fe5-d49e-42d1-9bfb-4965b9ab75b3
osd.4 up   in  weight 1 up_from 2125 up_thru 2221 down_at 2121 last_clean_interval [2042,2120) 10.17.155.114:6804/6327 10.17.155.114:6805/6327 10.17.155.114:6806/6327 10.17.155.114:6807/6327 exists,up dede8fcc-0b34-4296-83e2-a48966b22c36
osd.5 up   in  weight 1 up_from 2127 up_thru 2221 down_at 2121 last_clean_interval [2042,2120) 10.17.155.114:6808/6391 10.17.155.114:6809/6391 10.17.155.114:6810/6391 10.17.155.114:6811/6391 exists,up c2cde97e-c27a-4560-a46c-68695be79ff1
osd.6 up   in  weight 1 up_from 2040 up_thru 2221 down_at 2039 last_clean_interval [2020,2038) 10.17.155.115:6800/820 10.17.155.115:6801/820 10.17.155.115:6802/820 10.17.155.115:6803/820 exists,up d12f28d8-8fff-4a77-b344-d5d0b3e6949c
osd.7 up   in  weight 1 up_from 2072 up_thru 2221 down_at 2051 last_clean_interval [2015,2038) 10.17.155.115:6804/26346 10.17.155.115:6805/26346 10.17.155.115:6806/26346 10.17.155.115:6807/26346 exists,up 26035aa5-6759-4856-8bdb-1507f5b052e6
osd.8 up   in  weight 1 up_from 2087 up_thru 2221 down_at 2085 last_clean_interval [2074,2086) 10.17.155.115:6808/26484 10.17.155.115:6812/1026484 10.17.155.115:6813/1026484 10.17.155.115:6814/1026484 exists,up bca13e21-d64c-433f-87e4-4d5ea309f28a
</pre>

## 2. up_thru的来龙去脉
up_thru的整个生成以及是如何起作用的？整个流程是非常长的，为了让文章变得短小精悍一点，与up_thru不是强相关的流程就不加入代码分析了，只是一笔带过。

为了形象说明up_thru的来龙去脉，我们就沿着上文那两个osd的例子展开说。

### 2.1 up_thru的更新

###### osd.1挂了后发生了什么？

osd.1挂了之后，整个集群反应如下：

* osd上报mon

osd.1挂了后，或者osd.1主动上报，或是其他osd向mon上报osd.1挂了，此时mon已经感知到osd.1挂了。

* mon更新osdmap

monitor将osdmap中osd.1标记为down状态，并将新的osdmap发送给osd.2

* osd.2收到新的osdmap(第一次）

osd.2收到新的osdmap后，相关PG开始peering，若PG发现需要向mon申请更新up_thru信息，那么PG状态变为WaitUpThru。
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
		dout(10) << "up_thru " << get_osdmap()->get_up_thru(osd->whoami)<< " < same_since " << info.history.same_interval_since<< ", must notify monitor" << dendl;
		need_up_thru = true;
	} else {
		dout(10) << "up_thru " << get_osdmap()->get_up_thru(osd->whoami)<< " >= same_since " << info.history.same_interval_since<< ", all is well" << dendl;
		need_up_thru = false;
	}

	set_probe_targets(prior_set->probe);
}

PG::RecoveryState::GetMissing::GetMissing(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Peering/GetMissing")
{
	...
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

osd.2判断是否需要向mon申请更新up_thru消息，若需要，则向mon发送该消息。
{% highlight string %}
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...
	if (need_up_thru)
    	queue_want_up_thru(same_interval_since);
}
void OSD::queue_want_up_thru(epoch_t want)
{
	map_lock.get_read();
	epoch_t cur = osdmap->get_up_thru(whoami);
	Mutex::Locker l(mon_report_lock);
	if (want > up_thru_wanted) {
		dout(10) << "queue_want_up_thru now " << want << " (was " << up_thru_wanted << ")"<< ", currently " << cur<< dendl;
		up_thru_wanted = want;
		send_alive();
	} else {
		dout(10) << "queue_want_up_thru want " << want << " <= queued " << up_thru_wanted<< ", currently " << cur<< dendl;
	}
	map_lock.put_read();
}

void OSD::send_alive()
{
	assert(mon_report_lock.is_locked());
	if (!osdmap->exists(whoami))
		return;
	epoch_t up_thru = osdmap->get_up_thru(whoami);
	dout(10) << "send_alive up_thru currently " << up_thru << " want " << up_thru_wanted << dendl;

	if (up_thru_wanted > up_thru) {
		dout(10) << "send_alive want " << up_thru_wanted << dendl;
		monc->send_mon_message(new MOSDAlive(osdmap->get_epoch(), up_thru_wanted));
	}
}
{% endhighlight %}

* mon更新osdmap

mon收到消息后，更新osdmap里面osd.2的up_thru信息，并将新的osdmap发送给osd.2

* osd.2收到新的osdmap(第二次)

osd.2接收到新的osdmap后，相关PG开始peering，相关PG状态由WaitUpThru变为Active，可以开始服务业务io。

###### 具体的up_thru更新相关流程如下
以下是osd.2收到新的osdmap(第一次）后相关操作：

1） PG判断其对应的主osd是否需要更新up_thru

pg开始peering，以下是peering相关函数：
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
对于info.history.same_interval_since等于0的情况，一般表明该PG才刚刚导入，其值可能会在多个地方被更新，我们这里暂时不介绍。通常情况下，info.history.same_interval_since的值不为0， 其更新一般发生在PG::start_peering_interval()函数中：
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

	// did acting, up, primary|acker change?
	if (!lastmap) {
		dout(10) << " no lastmap" << dendl;
		dirty_info = true;
		dirty_big_info = true;
		info.history.same_interval_since = osdmap->get_epoch();
	} else {
		std::stringstream debug;
		assert(info.history.same_interval_since != 0);
		boost::scoped_ptr<IsPGRecoverablePredicate> recoverable(
		get_is_recoverable_predicate());

		bool new_interval = pg_interval_t::check_new_interval(
			old_acting_primary.osd,
			new_acting_primary,
			oldacting, newacting,
			old_up_primary.osd,
			new_up_primary,
			oldup, newup,
			info.history.same_interval_since,
			info.history.last_epoch_clean,
			osdmap,
			lastmap,
			info.pgid.pgid,
			recoverable.get(),
			&past_intervals,
			&debug);

		dout(10) << __func__ << ": check_new_interval output: "<< debug.str() << dendl;
		if (new_interval) {
			dout(10) << " noting past " << past_intervals.rbegin()->second << dendl;
			dirty_info = true;
			dirty_big_info = true;
			info.history.same_interval_since = osdmap->get_epoch();
		}
	}
	
	if (old_up_primary != up_primary ||	oldup != up) {
		info.history.same_up_since = osdmap->get_epoch();
	}
	// this comparison includes primary rank via pg_shard_t
	if (old_acting_primary != get_primary()) {
		info.history.same_primary_since = osdmap->get_epoch();
	}

	....
}
{% endhighlight %}
由上可见，当产生一个新的interval时，pg->info.history.same_interval_since通常会被赋值为当前的osdmap版本。


2) osd判断其本次做peering的所有PG中是否有需要申请up_thru的。若有，该osd向monitor申请up_thru
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

### 2.2 PG状态的转变
osd.2收到新的osdmap(```第一次```）后，PG进入peering后如果需要mon更新up_thru，其会先进入NeedUpThru状态：
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
等到osd.2收到新的osdmap(```第二次```)后，该osdmap也即为更新了up_thru后的新osdmap，PG当前正处于WaitUpThru状态。当正处于WaitUpThru状态的PG收到osdmap，会把PG->need_up_thru置为false，如下：
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
    if (need_up_thru && up_thru >= info.history.same_interval_since) {
        dout(10) << "adjust_need_up_thru now " << up_thru << ", need_up_thru now false" << dendl;
        need_up_thru = false;
        return true;
    }

    return false;
}
{% endhighlight %}

wait_up_thru状态的PG收到ActMap事件后，由于need_up_thru标记已经被设置为false了，所以开始进入active状态。进入active状态后，也就可以开始io服务了：
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


## 2. up_thru应用
上文描述了当osd.1挂掉后，整个集群的响应，其中包括up_thru的更新。本节描述一下up_thru的应用，也即本文最开始的那个例子：当osd.1再次up后，其是否能提供服务？

### 2.1 场景说明
假设初始时刻，也即osd.1、osd.2都健康，对应PG都是active+clean状态时osdmap的版本号epoch是80，此时osd.1以及osd.2的up_thru必然都```小于```80。

1） **不能提供IO服务的场景**

osd.1 down掉，osdmap版本号epoch变为81；

如果osd.2向mon申请更新up_thru成功，此时osdmap版本号epoch为82(osd.2的up_thru变为81)，由于osd.2收到新的osdmap后，PG的状态就可以变为active，也即可以提供IO服务了，所以如果up_thru更新成功了，可以判断osd.2有新的写入了（当然，可能存在虽然up_thru更新了，但是osd.2进入到active之前就挂了，那也是没有数据更新的）；

osd.2 down掉，osdmap版本号epoch变为83；

osd.1进程up，osdmap版本号epoch变为84；

2） **能提供IO服务的场景**

osd.1 down掉，osdmap版本号epoch变为81；

osd B没有peering成功，没有向monitor上报其up_thru，没有更新操作。

如果osd.2向mon申请更新up_thru失败，比如上报之前就挂了；

osd.2 down掉， osdmap版本号变为82；

osd.1进程up，osdmap版本号变为83；

### 2.2 io是否能服务的代码判断逻辑
通过如下流程判断对应PG是否能提供io服务：

* **生成past_interval**

在peering阶段会调用如下函数生成past_interval（osd启动时也会从底层读到相关信息进而生成past_interval):

{% highlight string %}
bool pg_interval_t::check_new_interval(
  int old_acting_primary,
  int new_acting_primary,
  const vector<int> &old_acting,
  const vector<int> &new_acting,
  int old_up_primary,
  int new_up_primary,
  const vector<int> &old_up,
  const vector<int> &new_up,
  epoch_t same_interval_since,
  epoch_t last_epoch_clean,
  OSDMapRef osdmap,
  OSDMapRef lastmap,
  pg_t pgid,
  IsPGRecoverablePredicate *could_have_gone_active,
  map<epoch_t, pg_interval_t> *past_intervals,
  std::ostream *out)
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

		//此处产生新的interval
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
				*out << "generate_past_intervals " << i<< ": not rw,"<< " up_thru " << lastmap->get_up_thru(i.primary) << " up_from " << lastmap->get_up_from(i.primary)
			       << " last_epoch_clean " << last_epoch_clean<< std::endl;

			/*
			 * 本past_interval的最后一个osdmap中主osd的up_thru版本号大于等于本past_interval的第一个osdmap的版本，
			 * 也即本past_interval阶段，osd更新过up_thru
			 */
			if (lastmap->get_up_thru(i.primary) >= i.first && lastmap->get_up_from(i.primary) <= i.first) {
				i.maybe_went_rw = true;

				if (out)
					*out << "generate_past_intervals " << i << " : primary up " << lastmap->get_up_from(i.primary)<< "-" << lastmap->get_up_thru(i.primary)
					  << " includes interval"<< std::endl;

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
					*out << "generate_past_intervals " << i << " : includes last_epoch_clean " << last_epoch_clean << " and presumed to have been rw" << std::endl;
			} else {
				i.maybe_went_rw = false;
				if (out)
					*out << "generate_past_intervals " << i<< " : primary up " << lastmap->get_up_from(i.primary)<< "-" << lastmap->get_up_thru(i.primary)
					  << " does not include interval"<< std::endl;
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
这里还是以上述两种场景来描述past_interval的更新：

1） 不能提供io服务的场景

如上所述，osdmap版本号epoch分别是80、84，这几个osdmap版本号会对应如下4个past_interval:[-, 80], [81,82], [83,83], [84, +]

并且在[81,82]这个past_interval中，maybe_went_rw会被设置为true（因为满足上面的条件lastmap->get_up_thru(i.primary) >= i.first)，因为在osdmap版本号为82时，osd.2对应的up_thru为81，等于本past_interval的第一个osdmap的版本号81。

2） 能提供io服务的场景

如上所述，osdmap的版本号epoch分别是80--83，这几个osdmap版本号会对应如下4个past_interval: [-, 80], [81], [82], [83,+]。


* **根据past_interval判断io是否能提供服务**

这里以不能服务io的场景为例，如下：
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
	
	// 如果prior.pg_down为true，pg状态被设置为down，此时pg状态为down+peering
	if (prior.pg_down) {
		state_set(PG_STATE_DOWN);
	}
	
	if (get_osdmap()->get_up_thru(osd->whoami) < info.history.same_interval_since) {
		dout(10) << "up_thru " << get_osdmap()->get_up_thru(osd->whoami) << " < same_since " << info.history.same_interval_since << ", must notify monitor" << dendl;
		need_up_thru = true;
	} else {
		dout(10) << "up_thru " << get_osdmap()->get_up_thru(osd->whoami)<< " >= same_since " << info.history.same_interval_since << ", all is well" << dendl;
		need_up_thru = false;
	}
	set_probe_targets(prior_set->probe);
}

PG::PriorSet::PriorSet(bool ec_pool,
		       IsPGRecoverablePredicate *c,
		       const OSDMap &osdmap,
		       const map<epoch_t, pg_interval_t> &past_intervals,
		       const vector<int> &up,
		       const vector<int> &acting,
		       const pg_info_t &info,
		       const PG *debug_pg)
  : ec_pool(ec_pool), pg_down(false), pcontdec(c)
{
	// Include current acting and up nodes... not because they may
	// contain old data (this interval hasn't gone active, obviously),
	// but because we want their pg_info to inform choose_acting(), and
	// so that we know what they do/do not have explicitly before
	// sending them any new info/logs/whatever.
	for (unsigned i=0; i<acting.size(); i++) {
		if (acting[i] != CRUSH_ITEM_NONE)
			probe.insert(pg_shard_t(acting[i], ec_pool ? shard_id_t(i) : shard_id_t::NO_SHARD));
	}

	// It may be possible to exlude the up nodes, but let's keep them in
	// there for now.
	for (unsigned i=0; i<up.size(); i++) {
		if (up[i] != CRUSH_ITEM_NONE)
			probe.insert(pg_shard_t(up[i], ec_pool ? shard_id_t(i) : shard_id_t::NO_SHARD));
	}
	

	//遍历所有的past_intervals{80},{81,82},{83},{84}
	for (map<epoch_t,pg_interval_t>::const_reverse_iterator p = past_intervals.rbegin();p != past_intervals.rend();++p) {

		const pg_interval_t &interval = p->second;              //取得pg_interval_t

		dout(10) << "build_prior " << interval << dendl;
	
		if (interval.last < info.history.last_epoch_started)
			break;  // we don't care
	
		if (interval.acting.empty())
			continue;
	
		//还是上面的例子，当这里运行到{81,82}这个interval时，由于maybe_went_rw为true了，故而无法continue退出，还要继续走
		if (!interval.maybe_went_rw)
			continue;
	
		// look at candidate osds during this interval.  each falls into
		// one of three categories: up, down (but potentially
		// interesting), or lost (down, but we won't wait for it).
		set<pg_shard_t> up_now;
		bool any_down_now = false;  // any candidates down now (that might have useful data)
	
		// consider ACTING osds
		for (unsigned i=0; i<interval.acting.size(); i++) {
			int o = interval.acting[i];
			if (o == CRUSH_ITEM_NONE)
				continue;

			pg_shard_t so(o, ec_pool ? shard_id_t(i) : shard_id_t::NO_SHARD);
	
			const osd_info_t *pinfo = 0;
			if (osdmap.exists(o))
				pinfo = &osdmap.get_info(o);
	
			//如果该OSD处于up状态，就加入到up_now列表中，同时加到probe列表。用于获取权威日志以及后续数据恢复
			if (osdmap.is_up(o)) {
				// include past acting osds if they are up.
				probe.insert(so);
				up_now.insert(so);
			} else if (!pinfo) {
 				//该列表保存了所有down列表

				dout(10) << "build_prior  prior osd." << o << " no longer exists" << dendl;
				down.insert(o);
			} else if (pinfo->lost_at > interval.first) {
				dout(10) << "build_prior  prior osd." << o << " is down, but lost_at " << pinfo->lost_at << dendl;
				up_now.insert(so);
				down.insert(o);
			} else {
				// 该列表保存了所有down列表
				// 当这里运行到{81,82}这个interval时，
				// 由于这个past_interval中的osd.2此时(注： 这里的此时为84) down了，所以会走到这里

				dout(10) << "build_prior  prior osd." << o << " is down" << dendl;
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
	
	dout(10) << "build_prior final: probe " << probe<< " down " << down<< " blocked_by " << blocked_by<< (pg_down ? " pg_down":"")<< dendl;
}
{% endhighlight %}

综上所述，当osd.1再次up后，最终到{84}这个interval时，根据上面一系列函数调用，此时的PG状态最终会变为peering+down，此时便无法响应服务io了。

## 3. 总结
在文章开头我们提出了一个疑问，即Ceph集群中有1个osd down了，那么osdmap会发生什么变化？ osdmap会更新几次？

答案是： 不一定，主要是分以下两种情况：

* 该挂掉的osd与其他osd无共同承载的PG

此时osdmap只会更新1次，变化便是osdmap中osd的状态从up更新为down。因为都不存在相关PG，也就不存在peering，也就没有up_thru的更新了，所以osdmap变化1次。

* 该挂掉的osd与其他osd有共同承载的PG

此时osdmap会至少更新两次，其中第1次是更新osdmap中osd的状态，第2次便是更新相关osd的up_thru。


这里我们通过实例说明osdmap变化情况：

1） 集群初始状态信息如下
{% highlight string %}
# ceph osd tree
ID CLASS WEIGHT  TYPE NAME                       STATUS REWEIGHT PRI-AFF
-1       1.74658 root default
-5       0.87329     host pubt2-ceph1-dg-163-org
 0   ssd 0.87329         osd.0                       up  1.00000 1.00000
-3       0.87329     host pubt2-ceph2-dg-163-org
 1   ssd 0.87329         osd.1                       up  1.00000 1.00000

# ceph osd dump | grep epoch
epoch 416  // osdmap版本号
{% endhighlight %}

2) down掉osd.0后集群信息如下
{% highlight string %}
# ceph osd tree 
ID CLASS WEIGHT  TYPE NAME                       STATUS REWEIGHT PRI-AFF
-1       1.74658 root default
-5       0.87329     host pubt2-ceph1-dg-163-org
 0   ssd 0.87329         osd.0                     down  1.00000 1.00000
-3       0.87329     host pubt2-ceph2-dg-163-org
 1   ssd 0.87329         osd.1                       up  1.00000 1.00000
# ceph osd dump | grep epoch
epoch 418  // osdmap版本号
{% endhighlight %}
如上所示，当down掉osd.0之后，osdmap版本号增加了2个。那我们分别看看这两个osdmap有啥变化，可以通过命令```ceph osd dump epoch```把相应osdmap打印出来：
<pre>
# ceph osd dump 416
# ceph osd dump 417
# ceph osd dump 418
</pre>
可以看到osdmap.416与osdmap.417的差别就是osd.0的状态变化；osdmap.417与osdmap.418的差别就是更新了osd.1的up_thru。


## 4. 补充
其实在OSDMonitor端，还有一种情况可能会触发引起up_thru的变更：
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
由如上代码，对应的OSD请求pg_temp时，触发更新该OSD的up_thru

<br />
<br />

**[参看]:**

1. [分布式存储架构：Ceph之up_thru来龙去脉分析](https://zhuanlan.zhihu.com/p/166527885)

2. [OSDMap](https://gitee.com/wanghongxu/cephknowledge/blob/master/Ceph-OSDMap.md)

<br />
<br />
<br />