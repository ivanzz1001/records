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
我们知道一个PG有三个副本（通常情况下），有所谓的Primary/Replica之分，因此其中的一些字段按功能来说可划分为：

* 作用于所有PG副本

* 仅作用于Primary副本

下面我们分别介绍这些字段。


## 1. PG重要字段分析









<br />
<br />

**[参看]**



<br />
<br />
<br />

