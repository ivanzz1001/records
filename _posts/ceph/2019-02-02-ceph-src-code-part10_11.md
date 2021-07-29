---
layout: post
title: ceph peering机制再研究(7)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


我们在上一章讲述到在GetInfo状态下抛出GotInfo事件，会直接跳转到GetLog阶段。本章我们就从GetLog开始，继续讲述Ceph的Peering过程。


<!-- more -->

## 1. PG::choose_acting()的实现
在具体讲解GetLog阶段之前，我们先讲述PG::choose_acting()函数的实现。这是一个比较重要但却比较复杂的函数，其主要实现如下功能：

* 选出具有权威日志的OSD

* 计算PG::actingbackfill和PG::backfill_targets两个OSD列表

>注：actingbackfill保存了当前PG的acting列表，包括需要进行backfill操作的OSD列表；backfill_targets列表保存了需要进行backfill的OSD列表

下面我们来看PG::choose_acting()的大体实现：
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
	//1) 调用PG::find_best_info()选取权威日志

    //1.1) 如若选取权威日志失败，返回false，之后PG进入InComplete状态

    //1.2) 选取权威日志成功，继续执行后面操作


	//2) 根据当前的权威日志，调用PG::calc_replicated_acting()计算出acting set（包括want/want_backfill/want_acting_backfill/want_primary)
	//（注：up set只是根据osdmap计算出来的，而acting set是真正负责数据读写的，在异常情况发生时，这两个会不相同）

	
	//3) 根据上面计算出的want，分不同情况进行处理
	
	// 3.1) 如若want中OSD数量小于pool.min_size，将PG::want_acting置空，返回false，进入InComplete状态

	// 3.2) 调用get_pgbackend()->get_is_recoverable_predicate()判断当前PG是否可以恢复，如果不能进行恢复，将PG::want_acting置空，
	//      返回false，进入InComplete状态


	// 如下均为可恢复状态
	// 3.3) 判断want与acting是否相等，如果不相等，则需要申请pg_temp
	// 3.3) 计算backfill_targets
}
{% endhighlight %}
这里我们对PG::want_acting进行一个简单的说明：其主要用于临时保存我们所期望获取到的acting set。


下面我们按照上面的三个步骤讲述其中过程。

### 1.1 查找权威日志
{% highlight string %}
/**
 * find_best_info
 *
 * Returns an iterator to the best info in infos sorted by:
 *  1) Prefer newer last_update
 *  2) Prefer longer tail if it brings another info into contiguity
 *  3) Prefer current primary
 */
map<pg_shard_t, pg_info_t>::const_iterator PG::find_best_info(
  const map<pg_shard_t, pg_info_t> &infos,
  bool restrict_to_up_acting,
  bool *history_les_bound) const
{
	assert(history_les_bound);
	/* See doc/dev/osd_internals/last_epoch_started.rst before attempting
	* to make changes to this process.  Also, make sure to update it
	* when you find bugs! */
	eversion_t min_last_update_acceptable = eversion_t::max();
	epoch_t max_last_epoch_started_found = 0;

	for (map<pg_shard_t, pg_info_t>::const_iterator i = infos.begin();i != infos.end();++i) {
		if (!cct->_conf->osd_find_best_info_ignore_history_les &&max_last_epoch_started_found < i->second.history.last_epoch_started) {
			*history_les_bound = true;
			max_last_epoch_started_found = i->second.history.last_epoch_started;
		}
		if (!i->second.is_incomplete() && max_last_epoch_started_found < i->second.last_epoch_started) {
			max_last_epoch_started_found = i->second.last_epoch_started;
		}
	}

	for (map<pg_shard_t, pg_info_t>::const_iterator i = infos.begin();i != infos.end();++i) {
		if (max_last_epoch_started_found <= i->second.last_epoch_started) {
			if (min_last_update_acceptable > i->second.last_update)
				min_last_update_acceptable = i->second.last_update;
		}
	}

	if (min_last_update_acceptable == eversion_t::max())
		return infos.end();
	
	map<pg_shard_t, pg_info_t>::const_iterator best = infos.end();

	// find osd with newest last_update (oldest for ec_pool).
	// if there are multiples, prefer
	//  - a longer tail, if it brings another peer into log contiguity
	//  - the current primary
	for (map<pg_shard_t, pg_info_t>::const_iterator p = infos.begin();p != infos.end();++p) {
		if (restrict_to_up_acting && !is_up(p->first) &&!is_acting(p->first))
			continue;

		// Only consider peers with last_update >= min_last_update_acceptable
		if (p->second.last_update < min_last_update_acceptable)
			continue;
	
		// disqualify anyone with a too old last_epoch_started
		if (p->second.last_epoch_started < max_last_epoch_started_found)
			continue;
	
		// Disquality anyone who is incomplete (not fully backfilled)
		if (p->second.is_incomplete())
			continue;

		if (best == infos.end()) {
			best = p;
			continue;
		}

		// Prefer newer last_update
		if (pool.info.require_rollback()) {
			if (p->second.last_update > best->second.last_update)
				continue;
			if (p->second.last_update < best->second.last_update) {
				best = p;
				continue;
			}
		} else {
			if (p->second.last_update < best->second.last_update)
				continue;
			if (p->second.last_update > best->second.last_update) {
				best = p;
				continue;
			}
		}
	
		// Prefer longer tail
		if (p->second.log_tail > best->second.log_tail) {
			continue;
		} else if (p->second.log_tail < best->second.log_tail) {
			best = p;
			continue;
		}
	
		// prefer current primary (usually the caller), all things being equal
		if (p->first == pg_whoami) {
			dout(10) << "calc_acting prefer osd." << p->first<< " because it is current primary" << dendl;
			best = p;
			continue;
		}
	}

	return best;
}
{% endhighlight %}
函数find_best_info()用于选取一个拥有权威日志的OSD。根据```last_epoch_clean```到目前为止，各个past interval期间参与该PG的所有目前还处于up状态的OSD上的pg_info_t信息，来选择一个拥有权威日志的OSD，选择的有限顺序如下：

1） 具有最新的last_update的OSD；

2） 如果条件1相同，选择日志更长的OSD；

3） 如果1，2条件都相同，选择当前的主OSD；

下面我们来看代码的具体实现过程：

1) 首先在所有OSD中计算max_last_epoch_started，然后在拥有最大的last_epoch_started的OSD中计算min_last_update_acceptable的值；

>注：min_last_update_acceptable表示我们可以接受的最小的last_update值

2） 如果min_last_update_acceptable为eversion_t::max()，返回infos.end()，选取失败；

3) 根据以下条件选择一个OSD：

  a) 首先过滤掉last_update小于min_last_update_acceptable，或者last_epoch_started小于max_last_epoch_started_found，或者处于incomplete的OSD；

  b) 如果PG是EC类型，选择最小的last_update；如果PG类型是副本，选择最大的last_update的OSD；

  c) 如果上述条件都相同，选择log tail最小的，也就是日志最长的OSD；

  d) 如果上述条件都相同，选择当前的主OSD； 

综上的选择过程可知，拥有权威日志的OSD特征如下：必须是非incomplete的OSD；必须有最大的last_epoch_started；last_update有可能是最大，但至少是min_last_update_acceptable；有可能是最长的OSD；有可能是主OSD。


### 1.2 计算PG的acting列表
{% highlight string %}
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
	ss << "calc_acting newest update on osd." << auth_log_shard->first<< " with " << auth_log_shard->second
		<< (restrict_to_up_acting ? " restrict_to_up_acting" : "") << std::endl;
	pg_shard_t auth_log_shard_id = auth_log_shard->first;
	
	// select primary
	map<pg_shard_t,pg_info_t>::const_iterator primary;
	if (up.size() &&!all_info.find(up_primary)->second.is_incomplete() &&
	  all_info.find(up_primary)->second.last_update >=auth_log_shard->second.log_tail) {
		ss << "up_primary: " << up_primary << ") selected as primary" << std::endl;
		primary = all_info.find(up_primary); // prefer up[0], all thing being equal
	} else {
		assert(!auth_log_shard->second.is_incomplete());
		ss << "up[0] needs backfill, osd." << auth_log_shard_id<< " selected as primary instead" << std::endl;
		primary = auth_log_shard;
	}

	ss << "calc_acting primary is osd." << primary->first<< " with " << primary->second << std::endl;
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
		  cur_info.last_update < MIN(primary->second.log_tail,auth_log_shard->second.log_tail)) {
			/* We include auth_log_shard->second.log_tail because in GetLog,
			* we will request logs back to the min last_update over our
			* acting_backfill set, which will result in our log being extended
			* as far backwards as necessary to pick up any peers which can
			* be log recovered by auth_log_shard's log */
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
		if (cur_info.is_incomplete() ||cur_info.last_update < primary->second.log_tail) {
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

		vector<int>::const_iterator acting_it = find(acting.begin(), acting.end(), i->first.osd);
		if (acting_it != acting.end())
			continue;
	
		if (i->second.is_incomplete() ||i->second.last_update < primary->second.log_tail) {
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
本函数计算PG相关的下列OSD列表：

* want: PG所期望的acting列表

* backfill: 需要进行backfill的OSD；

* acting_backfill: 与want的OSD列表相同，只不过这里是pg_shard_t类型

* want_primary: 所期望的acting set的primary OSD


具体处理过程如下：

1） 首先选择want列表中的primary OSD

   a) 如果up_primary处于非incomplete状态，并且其last_update大于等于权威日志的log_tail，说明up_primary的日志和权威日志有重叠，可以通过日志记录恢复，优先选择up_primary为主OSD；

   b) 否则选择auth_log_shard，也就是拥有权威日志的OSD为主OSD；

   c) 把want_primary加入到want以及acting_backfill列表中；

2) 函数的输入参数size为要选择的副本数，依次从up、acting、all_info里选择size各副本OSD：

   a) 如果该OSD上的PG处于incomplete状态，或者cur_info.last_update小于MIN(primary->second.log_tail,auth_log_shard->second.log_tail)，则该PG副本无法通过日志修复，只能通过Backfill操作来修复。把该OSD加入backfill和acting_backfill集合中；

   b) 否则就可以根据PG日志来修复，只加入acting_backfill和want集合中，不用加入到backfill列表中；

### 1.3 choose_acting()实现详解
我们在上面介绍了：

* PG::find_best_info()查找权威日志

* PG::calc_replicated_acting()计算期望的acting集合

下面我们来详细介绍以下PG::choose_acting()的实现：

{% highlight string %}
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound)
{
	map<pg_shard_t, pg_info_t> all_info(peer_info.begin(), peer_info.end());
	all_info[pg_whoami] = info;
	
	for (map<pg_shard_t, pg_info_t>::iterator p = all_info.begin();p != all_info.end();++p) {
		dout(10) << "calc_acting osd." << p->first << " " << p->second << dendl;
	}
	
	map<pg_shard_t, pg_info_t>::const_iterator auth_log_shard = find_best_info(all_info, restrict_to_up_acting, history_les_bound);

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
	
	assert(!auth_log_shard->second.is_incomplete());
	auth_log_shard_id = auth_log_shard->first;

	// Determine if compatibility needed
	bool compat_mode = !cct->_conf->osd_debug_override_acting_compat;
	if (compat_mode) {
		bool all_support = true;
		OSDMapRef osdmap = get_osdmap();
	
		for (map<pg_shard_t, pg_info_t>::iterator it = all_info.begin();it != all_info.end();++it) {
			pg_shard_t peer = it->first;
	
			const osd_xinfo_t& xi = osdmap->get_xinfo(peer.osd);
			if (!(xi.features & CEPH_FEATURE_OSD_ERASURE_CODES)) {
				all_support = false;
				break;
			}
		}
		if (all_support)
			compat_mode = false;
	}

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

	dout(10) << ss.str() << dendl;
	
	unsigned num_want_acting = 0;
	for (vector<int>::iterator i = want.begin();i != want.end();++i) {
		if (*i != CRUSH_ITEM_NONE)
			++num_want_acting;
	}

	// We go incomplete if below min_size for ec_pools since backfill
	// does not currently maintain rollbackability
	// Otherwise, we will go "peered", but not "active"
	if (num_want_acting < pool.info.min_size &&
	(pool.info.ec_pool() ||!cct->_conf->osd_allow_recovery_below_min_size)) {
		want_acting.clear();
		dout(10) << "choose_acting failed, below min size" << dendl;
		return false;
	}
	
	/* Check whether we have enough acting shards to later perform recovery */
	boost::scoped_ptr<IsPGRecoverablePredicate> recoverable_predicate(get_pgbackend()->get_is_recoverable_predicate());
	set<pg_shard_t> have;
	for (int i = 0; i < (int)want.size(); ++i) {
		if (want[i] != CRUSH_ITEM_NONE)
			have.insert(pg_shard_t(want[i],pool.info.ec_pool() ? shard_id_t(i) : shard_id_t::NO_SHARD));
	}

	if (!(*recoverable_predicate)(have)) {
		want_acting.clear();
		dout(10) << "choose_acting failed, not recoverable" << dendl;
		return false;
	}

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

	want_acting.clear();
	actingbackfill = want_acting_backfill;
	dout(10) << "actingbackfill is " << actingbackfill << dendl;
	assert(backfill_targets.empty() || backfill_targets == want_backfill);
	if (backfill_targets.empty()) {
		// Caller is GetInfo
		backfill_targets = want_backfill;
		for (set<pg_shard_t>::iterator i = backfill_targets.begin();i != backfill_targets.end();++i) {
			assert(!stray_set.count(*i));
		}
	} else {
		// Will not change if already set because up would have had to change
		assert(backfill_targets == want_backfill);
		// Verify that nothing in backfill is in stray_set

		for (set<pg_shard_t>::iterator i = want_backfill.begin();i != want_backfill.end();++i) {
			assert(stray_set.find(*i) == stray_set.end());
		}
	}

	dout(10) << "choose_acting want " << want << " (== acting) backfill_targets " << want_backfill << dendl;
	return true;
}
{% endhighlight %}
函数choose_acting()用来计算PG的acting_backfill和backfill_targets两个OSD列表。其中acting_backfill保存了当前PG的acting列表，包括需要进行backfill操作的OSD列表；backfill_targets列表保存了需要进行backfill操作的OSD列表。其处理过程如下：

1）首先构建all_info列表，然后调用PG::find_best_info()来选举出一个拥有权威日志的OSD，保存在变量auth_log_shard中；

2）如果没有选举出拥有权威日志的OSD，则进入如下流程：

   a) 如果up不等于acting，将want_acting设置为up，申请取消pg_temp，返回false。（此种情况下会进入NeedActingChange状态）

   b) 否则确保want_acting列表为空，返回false值；（此种情况下会进入InComplete状态）

3) 计算是否是compat_mode模式，检查是，如果所有的OSD都支持纠删码，就设置compat_mode值为false；

4) 根据PG的不同类型，调用不同的函数，对应ReplicatedPG调用函数PG::calc_replicated_acting()来计算PG需要的列表：
{% highlight string %}
set<pg_shard_t> want_backfill, want_acting_backfill;
vector<int> want;
pg_shard_t want_primary;
{% endhighlight %}

* want_backfill: 该PG需要i女性backfill的pg_shard

* want_acting_backfill: 包括进行acting和backfill的pg_shard

* want: 在compat_mode下，和want_acting_backfill相同

* want_primary: 所期望的主OSD


5) 下面就是对PG做一些检查：

   a) 计算num_want_acting数量，检查如果小于min_size，进行如下操作：

    * 如果对于EC，清空want_acting，返回false值；(注：此种情况下，PG会进入InComplete状态)

    * 对于ReplicatedPG，如果该PG不允许小于min_size的恢复，清空want_acting，返回false；（注：此种情况下，PG会进入InComplete状态）

   b) 调用IsPGRecoverablePredicate来判断PG现有的OSD列表是否可以恢复，如果不能恢复，情况want_acting，返回false值；（注：此种情况下，PG会进入InComplete状态）

6） 检查如果当前want不等于acting，设置want_acting为want：

   a) 如果want_acting等于up，申请empty为pg_temp的OSD列表，返回false；（注：此种情况下会进入NeedActingChange状态）

   b) 否则申请want为pg_temp的OSD列表；（注：此种情况下PG会进入NeedActingChange状态)

7) 最后设置PG的actingbackfill为want_acting_backfill，设置backfill_targets为want_backfill，并检查backfill_targets里的pg_shard应该不在stray_set里面；

8) 最终返回true值；


----------
下面我们举例说明需要申请pg_temp的场景：

1） 当前PG1.0，其acting列表和up列表都为[0,1,2]，PG处于clean状态；

2） 此时，osd0崩溃，导致该PG经过CRUSH算法重新获得acting和up列表都为[3,1,2]；

3) 选择出拥有权威日志的osd为1，经过PG::calc_replicated_acting()算法，want列表为[1,3,2]，acting_backfill为[1,3,2]，want_backfill为[3]。特别注意want列表第一个为主OSD，如果up_primary无法恢复，就选择权威日志的OSD为主OSD。

4) want[1,3,2]不等于acting[3,1,2]时，并且不等于up[3,1,2]，需要向monitor申请pg_temp为want；

5） 申请成功pg_temp以后，acting为[1,3,2]，up为[3,1,2]，osd1作为临时的主OSD，处理读写请求。当该PG恢复处于clean状态，pg_temp取消，acting和up都恢复为[3,1,2]。


## 2. Peering状态详细处理(续)

我们在前面介绍了Peering之GetInfo状态的处理细节，这里介绍GetLog状态的处理。

### 2.1 GetLog状态
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

当PG的主OSD获取到所有从OSD（以及past_intervals期间所有参与该PG，且目前仍处于up状态的OSD）的pg_info信息后，就跳转到GetLog状态。

现在我们来看GetLog状态构造函数的处理：

1） 调用函数PG::choose_acting(auth_log_shard)选出具有权威日志的OSD，并计算出acting_backfill和backfill_targets两个OSD列表。输出保存在auth_log_shard里。

2) 如果选择失败并且want_acting不为空，就抛出NeedActingChange事件，状态机转移到Primary/WaitActingChange状态，等待申请临时PG返回结果。如果want_acting为空，就抛出IsInComplete事件，PG状态机转移到Primary/Peering/InComplete状态。表明PG失败，PG就处于InComplete状态。

3） 如果auth_log_shard等于pg->pg_whoami，也就是选出的拥有权威日志的OSD为当前主OSD，直接抛出事件GotLog()完成GetLog过程；

4）如果pg->info.last_update小于权威OSD的log_tail，也就是本OSD的日志和权威日志不重叠，那么本OSD无法恢复，抛出IsInComplete事件。经过函数PG::choose_acting()的选择后，主OSD必须是可恢复的。如果主OSD不可恢复，必须申请一个临时PG，选择拥有权威日志的OSD为临时主OSD；

5） 如果自己不是权威日志的OSD，则需要去拥有权威日志的OSD去拉去权威日志，并与本地合并。

<br />

#### 2.1.1 接收缺失的权威日志

在上面我们讲到，如果主OSD不是拥有权威日志的OSD，就需要去拥有权威日志的OSD上拉取权威日志。这里我们先大概给出一张PG Primary发送```pg_query_t```请求到权威日志的OSD拉取权威日志的流程：







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

