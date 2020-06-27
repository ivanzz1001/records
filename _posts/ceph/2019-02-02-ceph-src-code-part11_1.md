---
layout: post
title: ceph数据修复
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

当PG完成了Peering过程后，处于Active状态的PG就可以对外提供服务了。如果该PG的各个副本上有不一致的对象，就需要进行修复。Ceph的修复过程有两种：Recovery和Backfill。

Recovery是仅依据PG日志中的缺失记录来修复不一致的对象。Backfill是PG通过重新扫描所有的对象，对比发现缺失的对象，通过整体拷贝来修复。当一个OSD失效时间过长导致无法根据PG日志来修复，或者新加入的OSD导致数据迁移时，就会启动Backfill过程。


从第10章可知，PG完成Peering过程后，就处于activate状态，如果需要Recovery，就产生DoRecovery事件，触发修复操作。如果需要Backfill，机会产生RequestBackfill事件来触发Backfill操作。在PG的修复过程中，如果既有需要Recovery过程的OSD，又有需要Backfill过程的OSD，那么处理过程需要先进行Recovery过程的修复，再完成Backfill过程的修复。

本章介绍Ceph的数据修复的实现过程。首先介绍数据修复的资源预约的知识，然后通过介绍修复的状态转换图，大概了解整个数据修复的过程。最后分别详细介绍Recovery过程和Backfill过程的具体实现。

<!-- more -->


## 1. 资源预约
在数据修复的过程中，为了控制一个OSD上正在修复的PG最大数目，需要资源预约，在主OSD上和从OSD上都需要预约。如果没有预约成功，需要阻塞等待。一个OSD能同时修复的最大PG数在配置选项osd_max_backfills中设置，默认值为1。

类AsyncReserver用来管理资源预约，其模板参数```<T>```为要预约的资源类型。该类实现了异步的资源预约。当成功完成资源预约后，就调用注册的回调函数通知调用方预约成功(src/common/AsyncReserver.h)：
{% highlight string %}
template <typename T>
class AsyncReserver {
  Finisher *f;               //当预约成功，用来执行的回调函数
  unsigned max_allowed;      //定义允许的最大资源数量，在这里指允许修复的PG的数量
  unsigned min_priority;     //最小的优先级
  Mutex lock;

  //优先级到待预约资源链表的映射，pair<T, Context *>定义预约的资源和注册的回调函数
  map<unsigned, list<pair<T, Context*> > > queues;

  //资源在queues链表中的位置指针
  map<T, pair<unsigned, typename list<pair<T, Context*> >::iterator > > queue_pointers;

  //预约成功，正在使用的资源
  set<T> in_progress;
};
{% endhighlight %}

### 1.1 资源预约
函数request_reservation()用于预约资源：
{% highlight string %}
/**
* Requests a reservation
*
* Note, on_reserved may be called following cancel_reservation.  Thus,
* the callback must be safe in that case.  Callback will be called
* with no locks held.  cancel_reservation must be called to release the
* reservation slot.
*/
void request_reservation(
  T item,                   ///< [in] reservation key
  Context *on_reserved,     ///< [in] callback to be called on reservation
  unsigned prio
  ) {
	Mutex::Locker l(lock);
	assert(!queue_pointers.count(item) &&
	!in_progress.count(item));
	queues[prio].push_back(make_pair(item, on_reserved));
	queue_pointers.insert(make_pair(item, make_pair(prio,--(queues[prio]).end())));
	do_queues();
}
{% endhighlight %}

具体处理过程如下：

1） 把要请求的资源根据优先级添加到queue队列中，并在queue_pointers中添加其对应的位置指针：
{% highlight string %}
queues[prio].push_back(make_pair(item, on_reserved));
queue_pointers.insert(make_pair(item, make_pair(prio,--(queues[prio]).end())));
{% endhighlight %}

2） 调用函数do_queues()用来检查queue中的所有资源预约申请：从优先级高的请求开始检查，如果还有配额并且其请求的优先级至少不小于最小优先级，就把资源授权给它。


3） 在queue队列中删除该资源预约请求，并在queue_ponters删除该资源的位置信息。把该资源添加到in_progress队列中，并把请求相应的回调函数添加到Finisher类中，使其执行该回调函数。最后通知预约成功。

### 1.2 取消预约
函数cancle_reservation()用于释放拥有的不再使用的资源：
{% highlight string %}
/**
* Cancels reservation
*
* Frees the reservation under key for use.
* Note, after cancel_reservation, the reservation_callback may or
* may not still be called. 
*/
void cancel_reservation(
  T item                   ///< [in] key for reservation to cancel
  ) {
	Mutex::Locker l(lock);
	if (queue_pointers.count(item)) {
	  unsigned prio = queue_pointers[item].first;
	  delete queue_pointers[item].second->second;
	  queues[prio].erase(queue_pointers[item].second);
	  queue_pointers.erase(item);
	} else {
	  in_progress.erase(item);
	}
	do_queues();
}
{% endhighlight %}

具体处理过程如下：

1） 如果该资源还在queue队列中，就删除（这属于异常情况的处理）；否则再in_progress队列中删除该资源

2） 调用do_queues()函数把该资源重新授权给其他等待的请求。


## 2. 数据修复状态转换图
如下图11-1所示的是修复过程状态转换图。当PG进入Active状态后，就进入默认的子状态Activating:

![ceph-chapter11-1](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter11_1.jpg)

数据修复的状态转换过程如下所示：

**情况1：**当进入Activating状态后，如果此时所有的副本都完整，不需要修复，其状态转移过程如下：

1） Activating状态接收到AllReplicasRecovered事件，直接转换到Recovered状态

2） Recovered状态接收到GoClean事件，整个PG转入Clean状态


----------


**情况2：** 当进入Activating状态后，没有Recovery过程，只需要Backfill过程的情况：

1） Activating状态直接接收到RequestBackfill事件，进入WaitLocalBackfillReserved状态；

2） 当WaitLocalBackfillReserved状态接收到LocalBackfillReserved事件后，意味着本地资源预约成功，转入WaitRemoteBackfillReserved；

3） 所有副本资源预约成功后，主PG就会接收到AllBackfillsReserved事件，进入Backfilling状态，开始实际数据Backfill操作过程；

4） Backfilling状态接收到Backfilled事件，标志Backfill过程完成，进入Recovered状态；

5） 异常处理：当在状态WaitRemoteBackfillReserved和Backfilling接收到RemoteReservationRejected事件，表明资源预约失败，进入NotBackfilling状态，再次等待RequestBackfilling事件来重新发起Backfill过程；


----------

**情况3：**当PG既需要Recovery过程，也可能需要Backfill过程时，PG先完成Recovery过程，再完成Backfill过程，特别强调这里的先后顺序。其具体过程如下：

1） Activating状态：在接收到DoRecovery事件后，转移到WaitLocalRecoveryReserved状态；

2） WaitLocalRecoveryReserved状态：在这个状态中完成本地资源的预约。当收到LocalRecoveryReserved事件后，标志着本地资源预约的完成，转移到WaitRemoteRecoveryReserved状态；

3） WaitRemoteRecoveryReserved状态：在这个状态中完成远程资源的预约。当接收到AllRemotesReserved事件，标志着该PG在所有参与数据修复的从OSD上完成资源预约，进入Recoverying状态；

4） Recoverying状态：在这个状态完成实际的数据修复工作。完成后把PG设置为PG_STATE_RECOVERING状态，并把PG添加到recovery_wq工作队列中，开始启动数据修复：
{% highlight string %}
PG::RecoveryState::Recovering::Recovering(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Active/Recovering")
{
  context< RecoveryMachine >().log_enter(state_name);

  PG *pg = context< RecoveryMachine >().pg;
  pg->state_clear(PG_STATE_RECOVERY_WAIT);
  pg->state_set(PG_STATE_RECOVERING);
  pg->publish_stats_to_osd();
  pg->osd->queue_for_recovery(pg);
}
{% endhighlight %}

5) 在Recoverying状态完成Recovery工作后，如果需要Backfill工作，就接收RequestBackfill事件，转入Backfill流程；

6） 如果没有Backfill工作流程，直接接收AllReplicasRecovered事件，转入Recovered状态；

7） Recovered状态：到达本状态，意味着已经完成数据修复工作。当收到事件GoClean后，PG就进入clean状态。

## 3. Recovery过程
数据修复的依据是在Peering过程中产生的如下信息：

- 主副本上的缺失对象的信息保存在pg_log类的pg_missing_t结构中；

- 各从副本上的缺失对象信息保存在OSD对应的peer_missing中的pg_missing_t结构中；

- 缺失对象的位置信息保存在类MissingLoc中

根据以上信息，就可以知道该PG里各个OSD缺失的对象信息，以及该缺失的对象目前在哪些OSD上有完整的信息。基于上面的信息，数据修复过程就相对比较清晰：

- 对于主OSD缺失的对象，随机选择一个拥有该对象的OSD，把数据拉取过来；

- 对于replica缺失的对象，从主副本上把缺失的对象数据推送到从副本上来完成数据的修复；

- 对于比较特殊的快照对象，在修复时加入了一些优化的方法；

### 3.1 触发修复
Recovery过程由PG的主OSD来触发并控制整个修复的过程。在修复的过程中，先修复主OSD上缺失（或者不一致）的对象，然后修复从OSD上缺失的对象。由数据修复状态转换过程可知，当PG处于Activate/Recoverying状态后，该PG被加入到OSD的RecoveryWQ工作队列中。在recovery_wq里，其工作队列的线程池的处理函数调用do_recovery()函数来执行实际的数据修复操作：
{% highlight string %}
void OSD::do_recovery(PG *pg, ThreadPool::TPHandle &handle);
{% endhighlight %}

函数do_recovery()由RecoveryWQ工作队列的线程池的线程执行。其输入的参数为要修复的PG，具体处理流程如下：

1） 配置选项osd_recovery_sleep设置了线程做一次修复后的休眠时间。如果设置了该值，每次线程开始先休眠相应的时间长度。该参数默认值为0，不需要休眠。

2） 加入recovery_wq.lock()锁，用来保护recovery_wq队列以及变量recovery_ops_active。计算可修复对象的max值，其值为允许修复的最大对象数osd_recovery_max_active减去正在修复的对象数recovery_ops_active，然后调用函数recovery_wq.unlock()解锁；


3） 如果max小于等于0，即没有修复对象的配额，就把PG重新加入工作队列recovery_wq中并返回；否则如果max大于0，调用pg->lock_suspend_timeout(handle)重新设置线程超时时间。检查PG的状态，如果该PG处于正在被删除的状态，或者既不处于peered状态，也不是主OSD，则直接退出；

4） 调用函数pg->start_recovery_ops()修复，返回值more为还需要修复的对象数目。输出参数started为已经开始修复的对象数。

5） 如果more为0，也就是没有修复的对象了。但是pg->have_unfound()不为0，还有unfound对象（即缺失的对象，目前不知道在哪个OSD上能找到完整的对象），调用函数discover_all_missing()在might_have_unfound队列中的OSD上继续查找该对象，查找的方法就是给相关的OSD发送获取OSD的pg_log的消息。

6） 如果rctx.query_map->empty()为空，也就是没有找到其他OSD去获取pg_log来查找unfound对象，就结束该PG的recover操作，调用函数从recovery_wq._dequeue(pg)删除PG；

7） 函数dispatch_context()做收尾工作，在这里发送query_map的请求，把ctx.transaction的事务提交到本地对象存储中。


由上过程分析可知，do_recovery()函数的核心功能是计算要修复对象的max值，然后调用函数start_recovery_ops()来启动修复。

### 3.2 ReplicatedPG
类ReplicatedPG用于处理Replicate类型PG的相关修复操作。下面分析它用于修复的start_recovery_ops()函数及其相关函数的具体实现。

###### 3.2.1 start_recovery_ops()
函数start_recovery_ops()调用recovery_primary()和recovery_replicas()来修复该PG上对象的主副本和从副本。修复完成后，如果仍需要Backfill过程，则抛出相应事件触发PG状态机，开始Backfill的修复进程。

{% highlight string %}
bool ReplicatedPG::start_recovery_ops(
  int max, ThreadPool::TPHandle &handle,
  int *ops_started);
{% endhighlight %}

该函数具体处理过程如下：

1） 首先检查OSD，确保该OSD是PG的主OSD。如果PG已经处于PG_STATE_RECOVERING或者PG_STATE_BACKFIL的状态则退出；

2） 从pg_log获取missing对象，它保存了主OSD缺失的对象。参数num_missing为主OSD缺失的对象数目；num_unfound为该PG上缺失的对象却没有找到该对象其他正确副本所在的OSD；如果num_missing为0，说明主OSD不缺失对象，直接设置info.last_complete为最新版本info.last_update的值；

3） 如果num_missing等于num_unfound，说明主OSD所缺失对象都为unfound类型的对象，先调用函数recover_replicas()启动修复replica上的对象；

4） 如果started为0，也就是已经启动修复的对象数量为0，调用函数recover_primary()修复主OSD上的对象；

5） 如果started仍然为0，且num_unfound有变化，再次启动recover_replicas()修复副本；

6） 如果started不为0，设置work_in_progress的值为true;

7） 如果recovering队列为空，也就是没有正在进行Recovery操作的对象，状态为PG_STATE_BACKFILL，并且backfill_targets不为空，started小于max，missing.num_missing()为0的情况下：

&emsp; a) 如果标志get_osdmap()->test_flag(CEPH_OSDMAP_NOBACKFILL)设置了，就推迟Backfill过程；

&emsp; b) 如果标志CEPH_OSDMAP_NOREBALANCE设置了，且是degrade的状态，推迟Backfill过程；

&emsp; c) 如果backfill_reserved没有设置，就抛出RequestBackfill事件给状态机，启动Backfill过程；

&emsp; d) 否则，调用函数recover_backfill()开始Backfill过程

8） 最后PG如果处于PG_STATE_RECOVERING状态，并且对象修复成功，就检查：如果需要Backfill过程，就向PG的状态机发送RequestBackfill事件；如果不需要Backfill过程，就抛出AllReplicasRecovered事件；

9） 否则，PG的状态就是PG_STATE_BACKFILL状态，清除该状态，抛出Backfilled事件；

###### 3.2.2 recover_primary()
函数recover_primary()用来修复一个PG的主OSD上缺失的对象：
{% highlight string %}
int ReplicatedPG::recover_primary(int max, ThreadPool::TPHandle &handle);
{% endhighlight %}

其处理过程如下：

1） 调用pgbackend->open_recovery_op()返回一个PG类型相关的PGBackend::RecoveryHandle。对于ReplicatedPG对应的RPGHandle，内部有两个map，保存了Push和Pull操作的封装PushOp和PullOp:
{% highlight string %}
struct RPGHandle : public PGBackend::RecoveryHandle {
	map<pg_shard_t, vector<PushOp> > pushes;
	map<pg_shard_t, vector<PullOp> > pulls;
};
{% endhighlight %}


2） last_requested为上次修复的指针，通过调用low_bound()函数来获取还没有修复的对象。

3） 遍历每一个未被修复的对象：latest为日志记录中保存的该缺失对象的最后一条日志，soid为缺失的对象。如果latest不为空：

&emsp; a) 如果该日志记录是pg_log_entry_t::CLONE类型，这里不做任何的特殊处理，直到成功获取snapshot相关的信息SnapSet后再处理；

&emsp; b) 如果该日志记录类型为pg_log_entry_t::LOST_REVERT类型：该revert操作为数据不一致时，管理员通过命令行强行回退到指定版本，reverting_to记录了回退的版本：

  * 如果item.have等于latest->reverting_to版本，也就是通过日志记录显示当前已经拥有回退的版本，那么就获取对象的ObjectContext，如果检查对象当前的版本obc->obs.io.version等于latest->version，说明该回退操作完成；

  * 如果item.have等于latest->reverting_to，但是对象当前的版本obc->obs.io.version不等于latest->version，说明没有执行回退操作，直接修改对象的版本号为latest->version即可。

  * 否则，需要拉取该reverting_to版本的对象，这里不做特殊的处理，只是检查所有OSD是否拥有该版本的对象，如果有就加入到missing_loc记录该版本的位置信息，由后续修复继续来完成。

&emsp; c) 如果该对象在recovering过程中，表明正在修复，或者其head对象正在修复，跳过，并计数增加skipped；否则调用函数recover_missing()来修复。

4） 调用函数pgbackend->run_recovery_op()，把PullOp或者PushOp封装的消息发送出去；

----------
下面举例说明，当最后的日志记录类型为LOST_REVERT时的修复过程：

```例11-1``` 日志修复过程

PG日志的记录如下： 每个单元代表一条日志记录，分别为对象的名字和版本以及操作，版本的格式为(epoch, version)。灰色的部分代表本OSD上缺失的日志记录，该日志记录是从权威日志记录中拷贝过来的，所以当前该日志记录是连续完整的。

![ceph-chapter11-2](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter11_2.jpg)


**情况1：** 正常情况的修复

缺失的对象列表为[obj1,obj2]。当前修复对象为obj1.由日志记录可知，对象obj1被修改过三次，分别为版本6,7,8。当前拥有的obj1对象的版本have值为4,修复时只修复到最后修改的版本8即可。

**情况2：** 最后一个操作为LOST_REVERT类型的操作

![ceph-chapter11-3](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter11_3.jpg)

对于要修复的对象obj1，最后一次操作为LOST_REVERT类型的操作，该操作当前版本version为8，修改前的版本prior_version为7，回退版本reverting_to为4。

在这种情况下，日志显示当前已经有版本4，检查对象obj1的实际版本，也就是object_info里保存的版本号：

1） 如果该值是8，说明最后一次revert操作成功，不需要做任何修复动作；

2） 如果该值是4，说明LOST_REVERT操作就没有执行。当然数据内容已经是版本4了，只需要修改object_info的版本为8即可。

如果回退的版本reverting_to不是版本4，而是版本6，那么最终还是需要把obj1的数据修复到版本6的数据。Ceph在这里的处理，仅仅是检查其他OSD缺失的对象中是否有版本6，如果有，就加入到missing_loc中，记录拥有该版本的OSD位置，待后续继续修复。

###### 3.2.3 recover_missing()
函数recover_missing()处理snap对象的修复。在修复snap对象时，必须首先修复head对象或者snapdir对象，获取SnapSet信息，然后才能修复快照对象自己。
{% highlight string %}
int ReplicatedPG::recover_missing(
  const hobject_t &soid, eversion_t v,
  int priority,
  PGBackend::RecoveryHandle *h);
{% endhighlight %}

具体实现如下：

1） 检查如果对象soid时unfound，直接返回PULL_NONE值。暂时无法修复处于unfound的对象

2） 如果修复的是snap对象：

&emsp; a) 查看如果对应的head对象处于missing，递归调用函数recover_missing()先修复head对象；

&emsp; b） 查看如果snapdir对象处于missing，就递归调用函数recover_missing()先修复snapdir对象；

3) 从head对象或者snapdir对象中获取head_obc信息；

4） 调用函数pgbackend->recover_object()把要修复的操作信息封装到PullOp或者PushOp对象中，并添加到RecoveryHandle结构中。


### 3.3 pgbackend
pgbackend封装了不同类型的Pool的实现。ReplicatedBackend实现了replicate类型的PG相关的底层功能，ECBackend实现了Erasure code类型的PG相关的底层功能。

由上一节```3.2```的分析可知，需要调用pgbackend的recover_object()函数来实现修复对象的信息封装。这里只介绍基于副本的。

函数recover_object()实现了pull操作，调用prepare_pull()函数把请求封装成PullOp结构。如果是push操作，就调用start_pushes()把请求封装成PushOp的操作。

###### 3.3.1 pull操作
prepare_pull()函数把要拉取的object相关的操作信息打包成PullOp类信息，如下所示：
{% highlight string %}
void ReplicatedBackend::prepare_pull(
  eversion_t v,                     //要拉取对象的版本信息
  const hobject_t& soid,            //要拉取的对象
  ObjectContextRef headctx,         //拉取对象的ObjectContext信息
  RPGHandle *h);                    //封装后保存的RecoveryHandle
{% endhighlight %}

难点在于snap对象的修复处理过程。下面先介绍PullOp数据结构。

PullOp数据结构如下(src/osd/osd_types.h)：
{% highlight string %}
struct PullOp {
  hobject_t soid;                                     //需要拉取的对象

  ObjectRecoveryInfo recovery_info;                   //需要修复的信息
  ObjectRecoveryProgress recovery_progress;           //对象修复进度信息
};



struct ObjectRecoveryInfo {
  hobject_t soid;                                     //修复的对象
  eversion_t version;                                 //修复对象的版本
  uint64_t size;                                      //修复对象的大小
  object_info_t oi;                                   //修复对象的object_info信息
  SnapSet ss;                                         //修复对象的快照信息

  //对象需要拷贝的集合，在修复快照对象时，需要从别的OSD拷贝到本地的对象的区段集合
  interval_set<uint64_t> copy_subset;

  //clone对象修复时，需要从本地对象拷贝来修复的区间
  map<hobject_t, interval_set<uint64_t>, hobject_t::BitwiseComparator> clone_subset;
};


struct ObjectRecoveryProgress {
  uint64_t data_recovered_to;                         //数据已经修复的位置指针
  string omap_recovered_to;                           //omap已经修复的位置指针
  bool first;                                         //是否是首次修复操作
  bool data_complete;                                 //数据是否修复完成
  bool omap_complete;                                 //omap是否修复完成
};
{% endhighlight %}

函数prepare_pull()具体处理过程如下：














<br />
<br />

**[参看]**



<br />
<br />
<br />

