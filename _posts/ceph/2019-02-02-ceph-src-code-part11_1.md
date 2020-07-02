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

1） 通过调用函数get_parent()来获取PG对象的指针。pgbackend的parent就是相应的PG对象。通过PG获取missing、peer_missing、missing_loc等信息；

2） 从soid对象对应的missing_loc的map中获取该soid对象所在的OSD集合。把该集合保存在shuffle这个向量中。调用random_shuffle操作对OSD列表随机排序，然后选择向量中首个OSD作为缺失对象来拉取源OSD的值。从这一步可知，当修复主OSD上的对象，而多个从OSD上有该对象时，随机选择其中一个源OSD来拉取。

3） 当选择了一个源shard之后，查看该shard对应的peer_missing来确保该OSD上不缺失该对象，即确实拥有该版本的对象。

4） 确定拉取对象的数据范围：

&emsp; a) 如果是head对象，直接拷贝对象的全部，在copy_subset()加入区间(0,-1)，表示全部拷贝，最后设置size为-1：
{% highlight string %}
recovery_info.copy_subset.insert(0, (uint64_t)-1);
recovery_info.size = ((uint64_t)-1);
{% endhighlight %}

&emsp; b) 如果该对象是snap对象，确保head对象或者snapdir对象二者必须存在一个。如果headctx不为空，就可以获取SnapSetContext对象，它保存了snapshot相关的信息。调用函数calc_clone_subsets()来计算需要拷贝的数据范围。

5） 设置PullOp的相关字段，并添加到RPGHandle中

函数calc_clone_subsets()用于修复快照对象。在介绍它之前，这里需要介绍SnapSet的数据结构和clone对象的overlap概念。

在SnapSet结构中，字段clone_overlap保存了clone对象和上一次clone对象的重叠部分：
{% highlight string %}
struct SnapSet {
  snapid_t seq;
  bool head_exists;
  vector<snapid_t> snaps;                                 // 序号降序排列
  vector<snapid_t> clones;                                // 序号升序排列

  //写操作导致的和最新的克隆对象重叠的部分
  map<snapid_t, interval_set<uint64_t> > clone_overlap;  

  map<snapid_t, uint64_t> clone_size;
};
{% endhighlight %}

下面通过一个示例来说明```clone_overlap```数据结构的概念。

```例11-2``` clone_overlap数据结构如图11-2所示:

![ceph-chapter11-4](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter11_4.jpg)

snap3从snap2对象clone出来，并修改了区间3和4，其在对象中范围的offset和length为(4,8)和(8,12)。那么在SnapSet的clone_overlap中就记录：
{% highlight string %}
clone_overlap[3] = {(4,8), (8,12)}
{% endhighlight %}

函数calc_clone_subset()用于修复快照对象时，计算应该拷贝的数据区间。在修复快照对象时，并不是完全拷贝快照对象，这里用于优化的关键在于：快照对象之间是有数据重叠，数据重叠的部分可以通过已存在的本地快照对象的数据拷贝来修复；对于不能通过本地快照对象拷贝修复的部分，才需要从其他副本上拉取对应的数据。

函数calc_clone_subsets()具体实现如下：

1) 首先获取该快照对象的size，把(0,size)加入到data_subset中：
{% highlight string %}
data_subset.insert(0, size);
{% endhighlight %}


2） 向前查找(oldest snap)和当前快照相交的区间，直到找到一个不缺失的快照对象，添加到clone_subset中。这里找的不重叠区间，是从不缺失快照对象到当前修复的快照对象之间从没修改过的区间，所以修复时，直接从已存在的快照对象拷贝所需区间数据即可。

3） 同理，向后查找（newest snap)和当前快照对象相重叠的对象，直到找到一个不缺失的对象，添加到clone_subset中。

4） 去除掉所有重叠的区间，就是需要拉取的数据区间；
{% highlight string %}
data_subset.subtract(cloning);
{% endhighlight %}

对于上述算法，下面举例来说明：

```例11-3``` 快照对象修复示例如图11-3所示：

![ceph-chapter11-5](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter11_5.jpg)

要修复的对象为snap4，不同长度代表各个clone对象的size是不同的，其中```深红色```的区间代表clone后修改的区间。snap2、snap3和snap5都是已经存在的非缺失对象。

算法处理流程如下：

1） 向前查找和snap4重叠的区间，直到遇到非缺失对象snap2为止。从snap4到snap2一直重叠的区间为1,5,8三个区间。因此，修复对象snap4时，修复1,5,8区间的数据，可以直接从已存在的本地非缺失对象snap2拷贝即可。

2） 同理，向后查找和snap4重叠的区间，直到遇到非缺失对象snap5为止。snap5和snap4重叠的区间为1,2,3,4,7,8六个区间。因此，修复对象4时，直接从本地对象snap4中拷贝区间1,2,3,4,7,8即可。

3） 去除上述本地就可修复的区间，对象snap4只有区间6需要从其他OSD上拷贝数据来修复。

###### 3.3.2 push操作
函数start_pushes()获取actingbackfill的OSD列表，通过peer_missing查找缺失该对象的OSD，调用prep_push_to_replica()打包PushOp请求。

函数prep_push_to_replica()函数实现过程如下：
{% highlight string %}
void ReplicatedBackend::prep_push_to_replica(
  ObjectContextRef obc, const hobject_t& soid, pg_shard_t peer,
  PushOp *pop, bool cache_dont_need);
{% endhighlight %}

1) 如果需要push的对象是snap对象：检查如果head对象缺失，调用prep_push()推送head对象;如果是headdir对象缺失，则调用prep_push()推送headdir对象；

2） 如果是snap对象，调用函数calc_clone_subsets()来计算需要推送的快照对象的数据区间；

3） 如果是head对象，调用函数calc_head_subsets()来计算需要推送的head对象的区间，其原理和计算快照对象类似，这里就不详细说明了。最后调用prep_push()封装PushInfo信息，在函数build_push_op()里读取要push的实际数据。

###### 3.3.3 处理修复操作
函数run_recover_op()调用send_pushed()函数和send_pulls()函数把请求发送给相关的OSD，这个流程比较简单。

当主OSD把对象推送给缺失该对象的从OSD后，从OSD需要调用函数handle_push()来实现数据写入工作，从而完成该对象的修复。同样，当主OSD给从OSD发起拉取对象的请求来修复自己缺失的对象时，需要调用函数handle_pulls()来处理该请求的应对。

在函数ReplicatedBackend::handle_push()里处理handle_push的请求，主要调用submit_push_data()函数来写入数据。

handle_pull()函数收到一个PullOp操作，返回PushOp操作，处理流程如下：
{% highlight string %}
void ReplicatedBackend::handle_pull(pg_shard_t peer, PullOp &op, PushOp *reply)
{
  const hobject_t &soid = op.soid;
  struct stat st;
  int r = store->stat(ch, ghobject_t(soid), &st);
  if (r != 0) {
    get_parent()->clog_error() << get_info().pgid << " "
			       << peer << " tried to pull " << soid
			       << " but got " << cpp_strerror(-r) << "\n";
    prep_push_op_blank(soid, reply);
  } else {
    ObjectRecoveryInfo &recovery_info = op.recovery_info;
    ObjectRecoveryProgress &progress = op.recovery_progress;
    if (progress.first && recovery_info.size == ((uint64_t)-1)) {
      // Adjust size and copy_subset
      recovery_info.size = st.st_size;
      recovery_info.copy_subset.clear();
      if (st.st_size)
        recovery_info.copy_subset.insert(0, st.st_size);
      assert(recovery_info.clone_subset.empty());
    }

    r = build_push_op(recovery_info, progress, 0, reply);
    if (r < 0)
      prep_push_op_blank(soid, reply);
  }
}
{% endhighlight %}

1) 首先调用store->stat()函数，验证该对象是否存在，如果不存在，则调用函数prep_push_op_blank()，直接返回空值；

2） 如果该对象存在，获取ObjectRecoveryInfo和ObjectRecoveryProgress结构。如果progress.first为true并且recovery_info.size为-1，说明是全拷贝修复：将recovery_info.size设置为实际对象的size，清空recovery_info.copy_subset，并把(0,size)区间添加到recovery_info.copy_subset.insert(0, st.st_size)的拷贝区间。

3） 调用函数build_push_op()，构建PullOp结构。如果出错，调用prep_push_op_blank()，直接返回空值。


----------

函数build_push_op()完成构建push的请求。具体处理如下：
{% highlight string %}
int ReplicatedBackend::build_push_op(const ObjectRecoveryInfo &recovery_info,
				     const ObjectRecoveryProgress &progress,
				     ObjectRecoveryProgress *out_progress,
				     PushOp *out_op,
				     object_stat_sum_t *stat,
                                     bool cache_dont_need);
{% endhighlight %}

1) 如果progress.first为true，就需要获取对象的元数据信息。通过store->omap_get_header()获取omap的header信息，通过store->getattrs()获取对象的扩展属性信息，并验证oi.version是否为recovery_info.version；否则返回-EINVAL值。如果成功，new_progress.first设置为false。

2） 上一步只是获取了omap的header信息，并没有获取omap信息。这一步首先判断progress.omap_complete是否完成（初始化设置为false)，如果没有完成，就迭代获取omap的(key,value)信息，并检查一次获取信息的大小不能超过cct->_conf->osd_recovery_max_chunk设置的值（默认为8MB）。特别需要注意的是，当该配置参数的值小于一个对象的size时，一个对象的修复需要多次数据的push操作。为了保证数据的完整一致性，先把数据拷贝到PG的temp存储空间。当拷贝完成之后，再移动到该PG的实际空间中。

3） 开始拷贝数据：检查recovery_info.copy_subset，也就是拷贝的区间；

4） 调用函数store->fiemap()来确定有效数据的区间out_op->data_included的值，通过store->read()读取相应的数据到data里。

5） 设置PullOp的相关字段，并返回。

## 4. Backfill过程

当PG完成了Recovery过程之后，如果backfill_targets不为空，表明有需要Backfill过程的OSD，就需要启动Backfill的任务，来完成PG的全部修复。下面介绍Backfill过程相关的数据结构和具体处理过程。


### 4.1 相关数据结构
数据结构BackfillInterval用来记录每个peer上的Backfill过程（src/osd/pg.h)。
{% highlight string %}
struct BackfillInterval {
	//一个peer的backfill_interval信息
	eversion_t version;                         //扫描时的最新对象版本

	map<hobject_t,eversion_t,hobject_t::Comparator> objects;
	bool sort_bitwise;
	hobject_t begin;
	hobject_t end;
};
{% endhighlight %}

其字段说明如下：

* version: 记录扫描对象列表时，当前PG对象更新的最新版本，一般为last_update，由于此时PG处于active状态，可能正在进行写操作。其用来检查从上次扫描到现在是否有对象写操作。如果有，完成写操作的对象在已扫描的对象列表中，进行Backfill操作时，该对象就需要更新为最新版本。

* objects: 扫描到准备进行Backfill操作的对象列表；

* begin: 当前处理的对象；

* end: 本次扫描对象的结束，用于作为下次扫描对象的开始：


### 4.2 Backfill的具体实现
函数recovery_backfill()作为Backfill过程的核心函数，控制整个Backfill修复进程。其工作流程如下。

1） 初始设置

在函数on_activate()里设置了PG的属性值new_backfill为true，设置了last_backfill_started为earliest_backfill()的值。该函数计算需要backfill的OSD中，peer_info信息里保存的last_backfill的最小值。

peer_backfill_info的map中保存各个需要backfill的OSD所对应backfillInterval对象信息。首先初始化begin和end都为peer_info.last_backfill，由PG的Peering过程可知，在函数activate()里，如果需要Backfill的OSD，设置该OSD的peer_info的last_backfill为hobject_t()，也就是MIN对象。

backfills_inf_flight保存了正在进行Backfill操作的对象，pending_backfill_updates保存了需要删除的对象。

2） 设置backfill_info.begin为last_backfill_started，调用函数update_range()来更新需要进行Backfill操作的对象列表；

3） 根据各个peer_info的last_backfill对应的backfillInterval信息进行trim操作。根据last_backfill_started来更新backfill_info里相关字段；

4） 如果backfill_info.begin小于等于earliest_peer_backfill()，说明需要继续扫描更多的对象，backfill_info重新设置，这里特别注意的是，backfill_info的version字段也重新设置为(0,0)，这会导致在随后调用的update_scan()函数再调用scan_range()函数来扫描对象；

5） 进行比较，如果pbi.begin小于backfill_info.begin，需要向各个OSD发送MOSDPGScan::OP_SCAN_GET_DIGEST消息来获取该OSD目前拥有的对象列表；

6） 当获取所有OSD的对象列表后，就对比当前主OSD的对象列表来进行修复。

7） check对象指针，就是当前OSD中最小的需要进行Backfill操作的对象：

&emsp; a) 检查check对象，如果小于backfill_info.begin，就需要在各个需要Backfill操作的OSD上删除该对象，加入到to_remove队列中；

&emsp; b) 如果check对象大于或者等于backfill_info.begin，检查拥有check对象的OSD，如果版本不一致，加入need_ver_targ中。如果版本相同，就加入keep_ver_targs中。

&emsp; c) 那些begin对象不是check对象的OSD，如果pinfo.last_backfill小于backfill_info.begin，那么，该对象缺失，加入missing_targs列表中；

&emsp; d) 如果pinfo.last_backfill大于backfill_info.begin，说明该OSD修复的进度已经超越当前的主OSD指示的修复进度，加入skip_targs中；

8） 对于keep_ver_targs列表中的OSD，不做任何操作。对于need_ver_targs和missing_targs中的OSD，该对象需要加入到to_push中去修复。

9） 调用函数send_remove_op()给OSD发送删除的消息来删除to_remove中的对象；

10） 调用函数prep_backfill_object_push()把操作打包成PushOp，调用函数pgbackend->run_recovery_op()把请求发送出去。其流程和Recovery流程类似。

11） 最后用new_last_backfill更新各个OSD的pg_info的last_backfill值。如果pinfo.last_backfill为MAX，说明backfill操作完成，给该OSD发送MOSDPGBackfill::OP_BACKFILL_FINISH消息；否则发送MOSDPGBackfill::OP_BACKFILL_PROGRESS来更新各个OSD上的pg_info的last_backfill字段。

下面举例说明。

```例11-4``` 如下图11-4所示，该PG分布在5个OSD上（也就是5个副本，这里为了方便列出各种处理情况），每一行上的对象列表都是相应OSD当前对应backfillInterval的扫描对象列表。osd5为主OSD，是权威的对象列表，其他OSD都对照主OSD上的对象列表来修复。

![ceph-chapter11-6](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter11_6.jpg)

下面举例来说明步骤7中的不同的修复方法：

1） 当前check对象指针为主OSD上保存的peer_backfill_info中begin的最小值，图中check对象应该为obj4对象；

2） 比较check对象和主osd5上的backfill_info.begin对象，由于check小于obj5，所以obj4为多余的对象，所有拥有该check对象的OSD都必须删除该对象。故osd0和osd2上的obj4对象被删除，同时对应的begin指针前移。

![ceph-chapter11-7](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter11_7.jpg)


3） 当前各个OSD的状态如图11-5所示：此时check对象为obj5，比较check和backfill_info.begin的值：

&emsp; a) 对于当前begin未check对象的osd0、osd1、osd4:

    * 对于osd0和osd4，check对象he backfill_info.begin对象都是obj5，且版本号都为(1,4)，加入到keep_ver.targs列表中，不需要修复；

    * 对于osd1，版本号不一致，加入need_ver_targs列表中，需要修复

&emsp; b) 对于当前begin不是check对象的osd2和osd3:

    * 对于osd2，其last_backfill小于backfill_info.begin，显然对象obj5缺失，加入missing_targs修复；

    * 对于osd3，其last_backfill大于backfill_info.begin，也就是说其已经修复到obj6了，obj5应该恢复了，加入skip_targs跳过；

4) 步骤3处理完成，设置last_backfill_started为当前的backfill_info.begin的值。backfill_info.begin指针前移，所有begin等于check对象的begin指针前移，重复以上步骤继续修复。

函数update_range()调用函数scan_range()更新BackfillInterval修复的对象列表，同时检查上次扫描对象列表中，如果有对象发生写操作，就更新该对象修复的版本。

具体实现步骤如下：

1） bi->version记录了扫描要修复的对象列表时PG最新更新的版本号，一般设置为last_update_applied或者info.last_update的值。初始化时，bi->version默认设置为(0,0)，所以小于info.log_tail，就更新bi->version的设置，调用函数scan_range()扫描对象。

2） 检查如果bi->version的值等于info.last_update，说明从上次扫描对象开始到当前时间，PG没有写操作，直接返回。

3） 如果bi->version的值小于info.last_update，说明PG有写操作，需要检查从bi->version到log_head这段日志中的对象：如果该对象有更新操作，修复时就修复最新的版本；如果该对象已经删除，就不需要修复，在修复队列中删除。


----------
下面举例说明update_range()的处理过程：




<br />
<br />

**[参看]**



<br />
<br />
<br />

