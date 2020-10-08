---
layout: post
title: ceph的peering过程分析
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


在介绍了statechart状态机和PG的创建过程后，正式开始Peering过程介绍。Peering的过程使一个PG内的OSD达成一个一致状态。当主从副本达成一个一致的状态后，PG处于active状态，Peering过程的状态就结束了。但此时该PG的三个OSD的数据副本上的数据并非完全一致。

PG在如下两种情况下触发Peering过程：

* 当系统初始化时，OSD重新启动导致PG重新加载，或者PG新创建时，PG会发起一次Peering的过程；

* 当有OSD失效，OSD的增加或者删除等导致PG的acting set发生了变化，该PG就会重新发起一次Peering过程；


<!-- more -->


## 1. 基本概念

### 1.1 acting set和up set
acting set是一个PG对应副本所在的OSD列表，该列表是有序的，列表中第一个OSD为主OSD。在通常情况下，up set和acting set列表完全相同。要理解他们的不同之处，需要理解下面介绍的```“临时PG”```概念。

### 1.2 临时PG
假设一个PG的acting set为[0,1,2]列表。此时如果osd0出现故障，导致CRUSH算法重新分配该PG的acting set为[3,1,2]。此时osd3为该PG的主OSD，但是osd3为新加入的OSD，并不能负担该PG上的读操作。所以PG向Monitor申请一个临时的PG，osd1为临时的主OSD，这时up set变为[1,3,2]，acting set依然为[3,1,2]，导致acting set和up set不同。当osd3完成Backfill过程之后，临时PG被取消，该PG的up set修复为acting set，此时acting set和up set都为[3,1,2]列表。

### 1.3 权威日志
权威日志（在代码里一般简写为olog)是一个PG的完整顺序且连续操作的日志记录。该日志将作为数据修复的依据。

### 1.4 up_thru
引入up_thru的概念是为了解决特殊情况： 当两个以上的OSD处于down状态，但是Monitor在两次epoch中检测到了这种状态，从而导致Monitor认为它们是先后宕掉。后宕的OSD有可能产生数据的更新，导致需要等待该OSD的修复，否则有可能产生数据丢失。

```例10-1``` up_thru处理过程

下图为初始情况：

![ceph-chapter10-4](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_4.jpg)

过程如下所示：

1） 在epoch1时，一个PG中有A、B两个OSD（两个副本）都处于up状态。

2） 在epoch2时，Monitor检测到了A处于down状态，B仍然处于up状态。由于Monitor检测可能滞后，实际可能有两种情况：

* 情况1： 此时B其实也已经和A同时宕了，只是Monitor没有检测到。此时PG不可能完成Peering过程，PG没有新数据写入；

* 情况2： 此时B确实处于up状态，由于B上保持了完整的数据，PG可以完成Peering过程并处于active的状态，可以接受新的数据写操作；

上述两种不同的情况，Monitor无法区分。

3） 在epoch3时，Monitor检测到B也宕了。

4） 在epoch4时，A恢复了up的状态后，该PG发起Peering过程，该PG是否允许完成Peering过程处于active状态，可以接受读写操作?

* 如果在epoch2时，属于```情况1```： PG并没有数据更新，B上不会新写入数据，A上的数据保存完整，此时PG可以完成Peering过程从而处于active状态，接受写操作；

* 如果在epoch2时，属于```情况2```: PG上有新数据更新到了osd B，此时osd A缺失一些数据，该PG不能完成Peering过程。


----------
为了使Monitor能够区分上述两种情况，引入了up_thru的概念，up_thru记录了每个OSD完成Peering后的epoch值。其初始值设置为0。

在上述```情况2```，PG如何可以恢复为active状态，在Peering过程，须向Monitor发送消息，Monitor用数组up_thru[osd]来记录该OSD完成Peering后的epoch值。

>注： OSD是通过OSD::send_alive()来向monitor报告up_thru信息的

当引入up_thru后，上述例子的处理过程如下：

**情况1:**

![ceph-chapter10-5](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_5.jpg)
情况1的处理流程如下：

1） 在epoch1时，up_thru[B]为0，也就是说B在epoch为0时参与完成peering

2） 在epoch2时，Monitor检查到OSD A处于down状态，OSD B仍处于up状态（实际B已经处于down状态），PG没有完成Peering过程，不会向Monitor上报更新up_thru的值。

3） epoch3时，A和B两个OSD都宕了；

4） epoch4时，A恢复up状态，PG开始Peering过程，发现up_thru[B]为0，说明在epoch为2时没有更新操作，该PG可以完成Peering过程，PG处于active状态。

**情况2:**

![ceph-chapter10-6](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_6.jpg)
情况2的处理流程如下：

1） 在epoch1时，up_thru[B]为0，也就是说B在epoch为0时参与完成Peering过程；

2） 在epoch2时，Monitor检查到OSD A处于down状态，OSD B还处于up状态，该PG完成peering过程，向Monitor上报B的up_thru变为当前epoch的值为2，此时PG可接受写操作请求；

3）之后可能集群其他OSD宕了（图中没画出）导致epoch变为3，但此时up_thru[B]仍为2；

4） 在epoch4时，A和B都宕了，B的up_thru为2；

5） 在epoch5时，A处于up状态，开始Peering，发现up_thru[B]为2，说明在epoch为2时完成了Peering，有可能有更新操作，该PG需要等待B恢复。否则可能丢失B上更新的数据；

## 2. PG日志
PG日志(pg log)为一个PG内所有更新操作的记录（下文所指的日志，如不特别指出，都是指PG日志）。每个PG对应一个PG日志，它持久化保存在每个PG对应pgmeta_oid对象的omap属性中。

它有如下特点：

* 记录一个PG内所有对象的更新操作元数据信息，并不记录操作的数据；

* 是一个完整的日志记录，版本号是顺序的且连续


###### 2.1 pg_log_t
结构体pg_log_t在内存中保存了该PG的所有操作日志，以及相关的控制结构：
{% highlight string %}
struct pg_log_t{
	eversion_t head;            //日志的头，记录最新的日志记录
	eversion_t tail;            //日志的尾，记录最旧的日志记录

	//用于EC，指示本地可回滚的版本，可回滚的版本都大于版本can_rollback_to
	eversion_t can_rollback_to;

	//在EC的实现中，本地保留了不同版本的数据。本数据段指示本PG里可以删除掉的对象版本
	eversion_t rollback_info_trimmed_to;

	//所有日志的列表
	list<pg_log_entry_t> log;

	...
}
{% endhighlight %}
需要注意的是， PG日志的记录是以整个PG为单位，包括该PG内所有对象的修改记录。

###### 2.2 pg_log_entry_t
结构体pg_log_entry_t记录了PG日志的单条记录，其数据结构如下：
{% highlight string %}
struct pg_log_entry_t{
	__s32 op;                       //操作的类型
	hobject_t soid;                 //操作的对象
	eversion_t version;             //本次操作的版本
	eversion_t prior_version;       //前一个操作的版本
	eversion_t reverting_to;        //本次操作回退的版本（仅用于回滚操作）

	ObjectModDesc mod_desc;         //用于保存本地回滚的一些信息，用于EC模式下的回滚操作
	
	bufferlist snaps;               //克隆操作用于记录当前对象的snap列表
	osd_reqid_t reqid;              //请求唯一标识(called+tid)
	vector<pair<osd_reqid_t, version_t> > extra_reqids;


	version_t user_version;         //用户的版本
	utime_t mtime;                  //这是用户本地时间
	...
};
{% endhighlight %}


###### 2.3 IndexedLog

类IndexedLog继承了类pg_log_t，在其基础上添加了根据一个对象来检索日志的功能，以及其他相关的功能。

###### 2.4 日志的写入

函数PG::add_log_entry()添加pg_log_entry_t条目到PG日志中。同时更新了info.last_complete和info.last_update字段。

PGLog::write_log()函数将日志写到对应的pgmeta_oid对象的kv存储中。这里并没有直接写入磁盘，而是先把日志的修改添加到ObjectStore::Transaction类型的事务中，与数据操作组成一个事务整体提交磁盘。这样可以保证数据操作、日志更新及其pg info信息的更新都在一个事务中，都以原子方式提交到磁盘上。


###### 2.5 日志的trim操作

函数trim()用来删除不需要的旧日志。当日志条目数大于min_log_entries时，需要进行trim操作：
{% highlight string %}
void PGLog::trim(
  LogEntryHandler *handler,
  eversion_t trim_to,
  pg_info_t &info);
{% endhighlight %}

###### 2.6 合并权威日志
函数merge_log()用于把本地日志和权威日志合并：
{% highlight string %}
void PGLog::merge_log(ObjectStore::Transaction& t,
                      pg_info_t &oinfo, pg_log_t &olog, pg_shard_t fromosd,
                      pg_info_t &info, LogEntryHandler *rollbacker,
                      bool &dirty_info, bool &dirty_big_info);
{% endhighlight %}

其处理过程如下：

1） 本地日志和权威日志没有重叠的部分：在这种情况下就无法依据日志来修复，只能通过Backfill过程来完成修复。所以先确保权威日志和本地日志有重叠的部分：
{% highlight string %}
// The logs must overlap.
assert(log.head >= olog.tail && olog.head >= log.tail);
{% endhighlight %}

2) 本地日志和权威日志有重叠部分的处理：

* 如果olog.tail小于log.tail，也就是权威日志的尾部比本地日志长。在这种情况下，只要把日志多出的部分添加到本地日志即可，它不影响missing对象集合。

* 本地日志的头部比权威日志的头部长，说明有多出来的divergent日志，调用函数rewind_divergent_log()去处理

* 本地日志的头部比权威日志的头部短，说明有缺失的日志，其处理过程为：把缺失的日志添加到本地日志中，记录missing的对象，并删除多出来的日志记录


----------
下面举例说明merge_log的不同处理情况。

```例10-2``` 函数merge_log()应用举例

**情况1：** 权威日志的尾部版本比本地日志的尾部小，如下所示
![ceph-chapter10-7](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_7.jpg)

本地log的log_tail为obj10(1,6)，权威日志olog的log_tail为obj3(1,4)。

日志合并的处理方式如下所示：

![ceph-chapter10-8](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_8.jpg)

把日志记录obj3(1,4)、obj4(1,5)添加到本地日志中，修改info.log_tail和log.tail指针即可。

**情况2：** 本地日志的头部版本比权威日志长，如下所示：

![ceph-chapter10-9](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_9.jpg)
权威日志的log_head为obj13(1,8)，而本地日志的log_head为obj10(1,11)。本地日志的log_head版本大于权威日志的log_head版本，调用函数rewind_divergent_log()来处理本地有分歧的日志。

在本例的具体处理过程为：把对象obj10、obj11、obj13加入missing列表中用于修复。最后删除多余的日志，如下所示：

![ceph-chapter10-10](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_10.jpg)

本例比较简单，函数rewind_divergent_log()会处理比较复杂的一些情况，后面会介绍到。

**情况3:** 本地日志的头部版本比权威日志的头部短，如下所示

![ceph-chapter10-11](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_11.jpg)

权威日志的log_head为obj10(1,11)，而本地日志的log_head为obj13(1,8)，即本地日志的log_head版本小于权威日志的log_head版本。

其处理方式如下：把本地日志缺失的日志添加到本地，并计算本地缺失的对象。最后把缺失的对象加到missing object列表只能够用于后续的修复，处理结果如下所示：

![ceph-chapter10-12](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_12.jpg)


###### 2.7 处理副本日志
函数proc_replica_log()用于处理其他副本节点发过来的和权威日志有分歧(divergent)的日志。其关键在于计算missing的对象列表，也就是需要修复的对象，如下所示：
{% highlight string %}
void PGLog::proc_replica_log(
  ObjectStore::Transaction& t,
  pg_info_t &oinfo, const pg_log_t &olog, pg_missing_t& omissing,
  pg_shard_t from) const；
{% endhighlight %}
其参数都为远程节点的信息：

- oinfo: 远程节点的pg_info_t

- olog: 远程节点的日志

- omissing: 远程节点的missing信息，为输出信息

- from: 来自远程节点

函数proc_replica_log()的具体处理过程如下：

1） 如果日志不重叠，就无法通过日志来修复，需要进行Backfill过程，直接返回；

2） 如果日志的head相同，说明没有分歧日志(divergent log)，直接返回；

3） 下面处理的都是这种情况： 日志有重叠并且日志的head不相同，需要处理分歧的日志：

![ceph-chapter10-13](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_13.jpg)

  * 计算第一条分歧日志边界first_non_divergent，从本地日志```由后往前```查找小于等于```olog.head```的日志记录

  * 版本lu为分歧日志的边界。如果first_non_divergent没有找到，或者小于log.tail，那么lu就设置为log_tail，否则就设置为first_non_divergent日志记录的版本。

  * 把所有的分歧日志都添加到divergent队列里；

  * 构建一个IndexedLog对象folog，把所有没有分歧的日志添加到folog里
{% highlight string %}
IndexedLog folog;
folog.log.insert(folog.log.begin(), olog.log.begin(), pp);
{% endhighlight %}

  * 调用函数_merge_divergent_entries()处理分歧日志；

  * 更新oinfo的last_update为lu版本

  * 如果有对象missing，就设置last_complete为小于first_missing的版本；

函数_merge_divergent_entries()处理所有的分歧日志，首先把所有分歧日志的对象按照对象分类，然后分别调用函数_merge_object_divergent_entries()对每个分歧日志的对象进行处理。


----------


函数_merge_object_divergent_entries()用于处理单个对象的divergent日志，其处理过程如下：

1） 首先进行比较，如果处理的对象hoid大于info.last_backfill，说明该对象本来就不存在，没有必要修复

>注意：这种情况一般发生在如下情景
>
> 该PG在上一次Peering操作成功后，PG还没有处于clean状态，正在Backfill过程中，就再一次触发了Peering的过程。info.last_backfill为上次最后一个修复的对象。
>
> 在本PG完成Peering后就开始修复，先完成Recovery操作，然后会继续完成上次的Backfill操作，所以没有必要在这里检查来修复。

2） 通过该对象的日志记录来检查版本是否一致。首先确保是同一个对象，本次日志记录的版本prior_version等于上一条日志记录的version值；

3） 版本first_divergent_update为该对象的日志记录中第一个产生分歧的版本；版本last_divergent_update为最后一个产生分歧的版本；版本prior_version为第一个分歧产生的前一个版本，也就是应该存在的对象版本。布尔变量object_not_in_store用来标记该对象不缺失，且第一条分歧日志操作是删除操作。处理分歧日志的5种情况如下所示：


**情况1：**在没有分歧的日志里查找该对象，但是已存在的对象的版本大于第一个分歧对象的版本。这种情况的出现，是由于在merge_log()中产生权威日志时的日志更新，相应的处理已经做了，这里不做任何处理；

**情况2：**如果prior_version为eversion_t()，为对象的create操作或者clone操作，那么这个对象就不需要恢复。如果已经在missing记录中，就删除该missing记录。

**情况3：**如果该对象已经处于missing列表中，如下进行处理：

* 如果日志记录显示当前已经拥有的该对象版本have等于prior_version，说明对象不缺失，不需要修复，删除missing中的记录；

* 否则，修改需要修复的版本need为prior_version；如果prior_version小于等于info.log_tail时，这是不合理的，设置new_divergent_prior用于后续处理；

**情况4：** 如果该对象的所有版本都可以回滚，直接通过本地回滚操作就可以修复，不需要加入missing列表来修复；

**情况5：** 如果不是所有的对象版本都可以回滚，删除相关的版本，把prior_version加入missing记录中用于修复


## 3. Peering的状态转换图

由上一章“ceph peering机制”的分析可知，主OSD上PG对应的状态机RecoveryMachine目前已经处于Started/Primary/Peering状态，从OSD上的PG对应的RecoveryMachine处于Started/Stray状态。本节总体介绍Peering过程的状态转换。

如下图```10-14```所示为Peering状态转换图，其过程如下：

![ceph-chapter10-14](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_14.jpg)

1） 当进入Primary/Peering状态后，就进入默认子状态GetInfo中；

2） 状态GetInfo接收事件```GotInfo```后，转移到GetLog状态中；

3） 如果状态GetLog接收到IsIncomplete事件后，跳转到InComplete状态；

4） 状态GetLog接收到事件```GotLog```后，就转入GetMissing状态

5） 状态GetMissing接收到事件Activate，转入状态Active

由上述Peering的状态转换过程可知，Peering过程基本分为如下三个步骤：

**步骤1** GetInfo: PG的主OSD通过发送消息获取所有从OSD的pg_info信息；

**步骤2** GetLog: 根据各个副本获取的pg_info信息的比较，选择一个拥有权威日志的OSD(auth_log_shard)。如果主OSD不是拥有权威日志的OSD，就从该OSD上拉取权威日志。主OSD完成拉取权威日志后也就拥有了权威日志。

**步骤3** GetMissing: 主OSD拉取其他从OSD的PG日志（或者部分获取，或者全部获取FULL_LOG)。通过与本地权威日志的对比，来计算该OSD上缺失的object信息，作为后续Recovery操作过程的依据。

最后通过Active操作激活主OSD，并发送notify通知消息，激活相应的从OSD。

下面我们会介绍这三个主要步骤。

## 4. pg_info数据结构
数据结构pg_info_t保存了PG在OSD上的一些描述信息。该数据结构在Peering的过程中，以及后续的数据修复中都发挥了重要的作用，理解该数据结构的各个关节字段的含义可以更好地理解相关的过程。pg_info_t数据结构如下(src/osd/osd_types.h)：
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
	spg_t pgid;                             //PG的id
	eversion_t last_update;                 //< last object version applied to store
	eversion_t last_complete;               //last version pg was complete through.
	epoch_t last_epoch_started;             //last epoch at which this pg started on this osd
	
	version_t last_user_version;            //最后更新的user object的版本号，用于分层存储
	
	eversion_t log_tail;                    //日志的尾部版本
	
	//上一次backfill操作的对象指针。如果该OSD的Backfill操作没有完成，那么[last_bakfill, last_complete)之间的对象可能
	//处于missing状态
	hobject_t last_backfill;             
	bool last_backfill_bitwise;             //true if last_backfill reflects a bitwise (vs nibblewise) sort
	
	interval_set<snapid_t> purged_snaps;    //PG要删除的snap集合
	
	pg_stat_t stats;                        //PG的统计信息
	
	pg_history_t history;                   //PG的历史信息
	pg_hit_set_history_t hit_set;           //这是Cache Tier用的hit_set
};
{% endhighlight %}

>注：另外还请参看[ceph中PGLog处理流程](https://ivanzz1001.github.io/records/post/ceph/2019/01/05/ceph-src-code-part6_3)



###### 4.1 last_update介绍

表示该PG所存储的最后一个object的版本号。一个object的版本号包括两个部分，我们来看一下eversion_t结构的定义：
{% highlight string %}
class eversion_t {
public:
	version_t version;
	epoch_t epoch;
};
{% endhighlight %} 
关于一个对象version的设置，是在ReplicatedPG::execute_ctx()通过如下代码来设置的：
{% highlight string %}
void ReplicatedPG::execute_ctx(OpContext *ctx)
{
	...

	// version
	ctx->at_version = get_next_version();
	ctx->mtime = m->get_mtime();

	...
}

eversion_t get_next_version() const {
	eversion_t at_version(get_osdmap()->get_epoch(),
		pg_log.get_head().version+1);

	assert(at_version > info.last_update);
	assert(at_version > pg_log.get_head());

	return at_version;
}
{% endhighlight %}
可以看到一个对象的eversion包括：

* epoch： 该对象在进行写入操作时的osdmap版本号。注： 由于对象的写入是由PG的主OSD发起的，主OSD将写入操作封装成transaction，然后发送给自己及其他副本OSD，因此这里的epoch是指打包事务操作时该PG对应主OSD的osdmap对应的epoch。

* version: pg_log内部所指定的一个版本号，该版本号是一个单调递增的整数值，PG每存储一个object，对应的内部version值就增加1。

###### 4.2 last_complete介绍

表示该PG确定已经完成的最后一个对象的版本号，在该指针之前的版本都已经在所有的OSD上完成更新（只表示内存更新完成）。 这意味着在last_complete时刻：

* all objects that existed as of that stamp

* OR a newer object

* OR have already applied a later delete

通常情况下(pg处于clean状态）last_complete等于last_update，其更新也会通过如下跟随last_update一起递进：
{% highlight string %}
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
但是假如PG出现异常，则会出现last_complete落后于last_update的情况，此时就需要通过recovering等操作来使last_complete逐渐追赶上last_update。


结构pg_history_t保存了PG的一些历史信息：
{% highlight string %}
struct pg_history_t {
	epoch_t epoch_created;            //PG创建时候的epoch值
	epoch_t last_epoch_started;       //PG启动时候的epoch值
	epoch_t last_epoch_clean;         //PG处于clean状态时的epoch值
	epoch_t last_epoch_split;         //该PG上一次分裂时候的epoch值
	epoch_t last_epoch_marked_full;   
	...
};
{% endhighlight %}

###### 4.3 last_epoch_started介绍
last_epoch_started字段有两个地方出现，一个是pg_info_t结构里的last_epoch_started，代表最后一次Peering成功后的epoch值，是本地PG完成Peering后就设置的。另一个是pg_history_t结构体里的last_epoch_started，是PG里所有的OSD都完成Peering后设置的epoch值。如下图所示：

![last-epoch-started](https://ivanzz1001.github.io/records/assets/img/ceph/sca/last_epoch_started.jpg)


此外，关于last_epoch_started，在doc/dev/osd_internals/last_epoch_started.rst中也有介绍，我们来看一下：

**info.last_epoch_started**: 记录了PG在```interval i```完成Peering时的epoch值，在i(包括i)之前的interval提交的所有写操作均会反映到local info/log中，而对于i之后的interval所提交的写操作将不会在当前local info/log中得到体现。由于提交的写操作不可能出现分歧(注：peering已经完成)，因此即使用一个更旧的info.last_epoch_started来获取权威log/info信息，也不可能会影响到当前的info.last_epoch_started。


**info.history.last_epoch_started**: 记录了最近一个interval中，PG作为一个整体完成peering操作后设置的epoch值。由于提交的所有写操作都是由acting set中的OSD提交的，任何无歧义的write操作都会确保acting set中每一个OSD都记录了history.last_epoch_started。

如下我们介绍一下last_epoch_started的更新操作：

1） info.last_epoch_started: PG在本OSD上完成Peering操作，就会马上更新其对应的last_epoch_started的值。参看PG::activate()函数

2） info.history.last_epoch_started： 对于本字段的更新操作，情况较为复杂。

对于PG副本OSD上的info.history.last_epoch_started字段的更新会在Peering完成后马上进行，参看PG::activate()；另外还会在Primary OSD向副本OSD发送pg_query_t::INFO时得到更新：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Stray::react(const MQuery& query){
	if (query.query.type == pg_query_t::INFO) {
		...

		pg->update_history_from_master(query.query.history);
		...
	}
}

void PG::update_history_from_master(pg_history_t new_history)
{
	unreg_next_scrub();
	if (info.history.merge(new_history))
		dirty_info = true;
	reg_next_scrub();
}
{% endhighlight %}
在上面的merge函数中，其会将info.history.last_epoch_started更新为一个较大的值。如此确保了:
{% highlight string %}
replica.info.history.last_epoch_started >= primary.info.history.last_epoch_started
{% endhighlight %}

对于PG主OSD上的info.history.last_epoch_started的更新，可以分为如下两种情况（假设有PG 1.0[0,1,2])：

* 主动触发更新： 此种情况为在主OSD完成peering之前，所有的副本OSD就已经完成了Peering，此时会主动触发更新history.last_epoch_started
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

	// find out when we commit
	t.register_on_complete(
		new C_PG_ActivateCommitted(
			this,
			get_osdmap()->get_epoch(),
			activation_epoch));

	...
}

void PG::_activate_committed(epoch_t epoch, epoch_t activation_epoch)
{
	if (pg_has_reset_since(epoch)) {
		...
	} else if (is_primary()) {
		...
		if (peer_activated.size() == actingbackfill.size())
			all_activated_and_committed();
	}
	...
}

void PG::all_activated_and_committed()
{
	...
	queue_peering_event(
		CephPeeringEvtRef(
			std::make_shared<CephPeeringEvt>(
			get_osdmap()->get_epoch(),
			get_osdmap()->get_epoch(),
			AllReplicasActivated())));
}

boost::statechart::result PG::RecoveryState::Active::react(const AllReplicasActivated &evt){
	...

	// info.last_epoch_started is set during activate()
	pg->info.history.last_epoch_started = pg->info.last_epoch_started;

	...
}
{% endhighlight %}

* 被动触发更新： 此种情况为主OSD完成peering时，还有副本OSD没有完成Peering(没有收到副本OSD完成Peering的通知消息)，此时主OSD先进入Active阶段，然后再后续收到副本OSD发送来的通知消息时，触发更新history.last_epoch_started的值
{% highlight string %}
boost::statechart::result PG::RecoveryState::Active::react(const MInfoRec& infoevt){
	...

	if (pg->is_actingbackfill(infoevt.from)) {
		dout(10) << " peer osd." << infoevt.from << " activated and committed" << dendl;
		pg->peer_activated.insert(infoevt.from);
		pg->blocked_by.erase(infoevt.from.shard);
		pg->publish_stats_to_osd();

		if (pg->peer_activated.size() == pg->actingbackfill.size()) {
			pg->all_activated_and_committed();
		}
	}

	...
}
void PG::all_activated_and_committed()
{
	...
	queue_peering_event(
		CephPeeringEvtRef(
			std::make_shared<CephPeeringEvt>(
			get_osdmap()->get_epoch(),
			get_osdmap()->get_epoch(),
			AllReplicasActivated())));
}

boost::statechart::result PG::RecoveryState::Active::react(const AllReplicasActivated &evt){
	...

	// info.last_epoch_started is set during activate()
	pg->info.history.last_epoch_started = pg->info.last_epoch_started;

	...
}
{% endhighlight %}

* 获取到权威日志时触发更新
{% highlight string %}
void PG::proc_master_log(
  ObjectStore::Transaction& t, pg_info_t &oinfo,
  pg_log_t &olog, pg_missing_t& omissing, pg_shard_t from)
{

	...

	// See doc/dev/osd_internals/last_epoch_started
	if (oinfo.last_epoch_started > info.last_epoch_started) {
		info.last_epoch_started = oinfo.last_epoch_started;
		dirty_info = true;
	}
	if (info.history.merge(oinfo.history))
		dirty_info = true;

	assert(cct->_conf->osd_find_best_info_ignore_history_les ||
	info.last_epoch_started >= info.history.last_epoch_started);

	...
}
{% endhighlight %}


###### 4.4 last_user_version介绍
通常是由librados所指定的一个对象的版本编号。在进行对象的修改操作时，rados通常会事先查询是否有该对象，如果有的话，则每一次修改对应的对象版本号就会加1。参看：
{% highlight string %}
void ReplicatedPG::do_op(OpRequestRef& op){
	...

	ObjectContextRef obc;


	int r = find_object_context(
		oid, &obc, can_create,
		m->has_flag(CEPH_OSD_FLAG_MAP_SNAP_CLONE),
		&missing_oid);
	...
}
int ReplicatedPG::find_object_context(const hobject_t& oid,
				      ObjectContextRef *pobc,
				      bool can_create,
				      bool map_snapid_to_clone,
				      hobject_t *pmissing)
{
	
}
{% endhighlight %}

###### 4.5 last_complete和last_backfill的区别
在这里特别指出last_update和last_complete、last_backfill之间的区别。下面通过一个例子来讲解，同时也可以大概了解PG数据恢复的流程。在数据恢复过程中先进行Recovery过程，再进行Backfill过程（可以参考第11章的详细介绍）。

**情况1：** 在PG处于clean状态时，last_complete就等于last_update的值，并且等于PG日志中的head版本。它们都同步更新，此时没有区别。last_backfill设置为MAX值。例如，下面的PG日志里有三条日志记录。此时last_update和last_complete以及pg_log.head都指向版本(1,2)。由于没有缺失的对象，不需要恢复，last_backfill设置为MAX值。示例如下图所示：

![ceph-chapter10-15](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_15.jpg)

**情况2：** 当该osd1发生异常之后，过一段时间后又重新恢复，当完成了Peering状态后的情况。此时该PG可以继续接受更新操作。例如：下面的灰色字体的日志记录为该osd1崩溃期间缺失的日志，obj7为新的写入的操作日志记录。last_update指向最新的更新版本(1,7)，last_complete仍然指向版本(1,2)。即last_update指的是最新的版本，last_complete指的是上次的更新版本。过程如下：

![ceph-chapter10-16](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_16.jpg)

```last_complte为Recovery修复进程完成的指针```。当该PG开始进行Recovery工作时，last_complete指针随着Recovery过程推进，它指向完成修复的版本。例如：当Recovery完成后last_complete指针指向最后一个修复的对象的版本(1,6)，如下图所示：

![ceph-chapter10-17](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_17.jpg)

```last_backfill为Backfill修复进程的指针```。在Ceph Peering的过程中，该PG有osd2无法根据PG日志记录来恢复，就需要进行Backfill过程。last_backfill初始化为MIN对象，用来记录Backfill的修复进程中已修复的对象。例如：进行Backfill操作时，扫描本地对象(按照对象的hash值排序）。last_backfill随修复的过程不断推进。如果对象小于等于last_backfill，就是已经修复完成的对象。如果对象大于last_backfill且对象的版本小于last_complete，就是处于缺失还没有修复的对象。过程如下所示：

![ceph-chapter10-18](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_18.jpg)

当恢复完成之后，last_backfill设置为MAX值，表明恢复完成，设置last_complete等于last_update的值。

## 5. GetInfo状态

GetInfo过程获取该PG在其他OSD上的结构图pg_info_t信息（也成pg_info信息）。这里的其他OSD包括当前PG的活跃OSD，以及past_interval期间该PG所有处于up状态的OSD。

由上面的第3节介绍可知，当PG进入Primary/Peering状态后，就进入默认的子状态GetInfo里。其主要流程在构造函数里完成：
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
{% endhighlight %}
在构造函数GetInfo里，完成了核心的功能，实现过程如下：

1) 调用函数generate_past_intervals()计算past_intervals的值
{% highlight string %}
pg->generate_past_intervals();
{% endhighlight %}

2) 调用函数build_prior()构造获取pg_info_t信息的OSD列表
{% highlight string %}
pg->build_prior(prior_set);
{% endhighlight %}

3) 调用函数get_infos()给参与的OSD发送获取请求
{% highlight string %}
get_infos();
{% endhighlight %}
由上述可知，GetInfo()过程基本分成三个步骤：计算past_interval的过程；通过调用函数build_prior()来计算要获取pg_info信息的OSD列表；最后调用函数get_infos()给相关的OSD发送消息来获取pg_info信息，并处理接收到的Ack应答。

###### 5.1 计算past_interval
past_interval是epoch的一个序列。在该序列内一个PG的acting set和up set不会变化。current_interval是一个特殊的past_interval，它是当前最新的一个没有变化的序列。示例如下：

说明如下：

1） Ceph系统当前的epoch值为20，PG1.0的acting set和up set都为列表[0,1,2]；

2） osd3失效导致了osd map变化，epoch变为21；

3） osd5失效导致了osd map变化，epoch变为22；

4） osd6失效导致了osd map变化，epoch变为23；

上述3次epoch变化都不会改变PG1.0的acting set和up set。

![ceph-chapter10-19](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_19.jpg)

5) osd2失效导致了osd map变化，epoch变为24；此时导致PG1.0的acting set和up set变为[0,1,8]，若此时Peering过程成功完成，则last_epoch_started为24。

6） osd12失效导致了osd map变化，epoch变为25，此时如果PG1.0完成了Recovery操作，处于clean状态，last_epoch_clean就为25；

7） osd13失效导致了osd map变化，epoch变为26。

epoch序列[20,21,22,23]就为PG1.0的一个past_interval，epoch序列[24,25,26]就为PG1.0的current_interval。

数据结构```pg_interval_t```用于保存past_interval的信息（src/osd/osd_types.h)：
{% highlight string %}
struct pg_interval_t {
	vector<int32_t> up, acting;          //在本interval阶段PG处于up和acting状态的OSD
	epoch_t first, last;                 //起始和结束epoch
	bool maybe_went_rw;                  //在这个阶段是否有数据读写操作
	int32_t primary;                     //主OSD
	int32_t up_primary;                  //up状态的主OSD
};
{% endhighlight %}
上例中，past_interval对象的p值为：
{% highlight string %}
p = {
	up = [0,1,2],
	acting = [0,1,2],
	first = 20,
	last = 23,
	maybe_went_rw = true,
	primary = 0,
	up_primary = 0,
}
{% endhighlight %}
函数generate_past_intervals()用于计算past_intervals的值，计算的结果保存在PG中past_intervals的map结构里，map的key值为first epoch的值：
{% highlight string %}
map<epoch_t, pg_interval_t> past_intervals;
{% endhighlight %}

具体计算过程如下：

1） 调用函数_calc_past_interval_range()推测需要计算的past_interval的起始epoch值（start)和结束epoch值(end)。如果返回false，说明不需要计算past_interval，所有的past_interval已经计算好了。

2） 从start到end开始计算past_interval。过程为调用函数check_new_interval()比较两次epoch对应的osd map的变化。如果检查是一个新值，就创建一个新的past_interval对象。


----------

{% highlight string %}
bool PG::_calc_past_interval_range(epoch_t *start, epoch_t *end, epoch_t oldest_map);
{% endhighlight %}
函数_calc_past_interval_range()用于计算past_interval的范围。参数oldest_map为OSD的superblock里保存的最老osd map，输出为start和end，分别为需要计算的past_interval的start和end值。具体实现过程如下：

**计算end值如下所示：**

1） 变量end为当前osd map的epoch值，而如果info.history.same_interval_since不为空，就设置为该值。该值表示和当前的osd map的epoch值在同一个interval中。
{% highlight string %}
if (info.history.same_interval_since) {
	*end = info.history.same_interval_since;
} else {
	//当前PG可能是新引入的，计算整个range期间interval
	*end = osdmap_ref->get_epoch();
}
{% endhighlight %}

2） 查看past_intervals里已经计算的past_interval的第一个epoch，如果已经比info.history.last_epoch_clean小，就不用计算了，直接返回false。否则设置end为其first值
{% highlight string %}
*end = past_intervals.begin()->first;
{% endhighlight %}


**计算start值如下所示：**

1） start设置为info.history.last_epoch_clean，从最后一次last_epoch_clean算起；

2） 当PG为新建时，从info.history.epoch_create开始计算

3) oldest_map值为保存的最早osd map的值，如果start小于这个值，相关的osd map信息缺失，所以无法计算。

所以将start设置为三者的最大值：
{% highlight string %}
*start = MAX(MAX(info.history.epoch_created,
		   info.history.last_epoch_clean),
	       oldest_map);
{% endhighlight %}

下面举例说明计算past_interval的过程。

**例10-3** past_interval计算示例

![ceph-chapter10-20](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_20.jpg)

如上表所示： 一个PG有4个interval。past_interval 1的开始epoch为4，结束epoch为8；past_interval 2的epoch区间为[9,11]；past_interval 3的epoch区间为[12,13]；current_interval的epoch区间为[14,16]。最新的epoch为16，info.history.same_interval_since为14，意指是从epoch 14开始，之后的epoch值和当前的epoch值在同一个interval内。info.history.last_epoch_clean为8，就是说在epoch值为8时，该PG处于clean状态。

计算start和end的方法如下：

1） start的值设置为info.history.last_epoch_clean值，其值为8

2） end值从14开始计算，检查当前已经计算好的past_intervals的值。past_interval的计算是从后往前计算。如果第一个past interval的first小于等于8，也就是past_interval 1已经计算过了，那么后面的past_interval 2和past_interval 3都已经计算过，就直接退出。否则就继续查找没有计算过的past_interval的值。

###### 5.2 构建OSD列表
函数build_prior()根据past_intervals来计算probe_targets列表，也就是要去获取pg_info的OSD列表。
{% highlight string %}
void PG::build_prior(std::unique_ptr<PriorSet> &prior_set);
{% endhighlight %}
具体实现为： 首先重新构造一个PriorSet对象，在PriorSet的构造函数中完成下列操作：

1） 把当前PG的acting set和up set中的OSD加入到probe列表中；

2） 检查每个past_intervals阶段：

&emsp; a) 如果interval.last小于info.history.last_epoch_started，这种情况下past_interval就没有意义，直接跳过；

&emsp; b) 如果该interval的act为空，就跳过；

&emsp; c) 如果该interval没有rw操作，就跳过；

&emsp; d) 对于当前interval的每一个处于acting状态的OSD进行检查：

  * 如果该OSD当前处于up状态，就加入到up_now列表中。同时加入到probe列表中，用于获取权威日志以及后续数据恢复；

  * 如果该OSD当前不是up状态，但是在该past_interval期间还处于up状态，就加入up_now列表中；

  * 否则就加入down列表，该列表保存所有宕了的OSD

  * 如果当前interval确实有宕的OSD，就调用函数pcontdec()，也就是PG的IsPGRecoverablePredicate函数。该函数判断该PG在该interval期间是否可恢复。如果无法恢复，直接设置pg_down为true值。

>注意： 这里特别强调的是，要确保每个interval期间都可以进行修复。函数IsPGRecoverablePredicate实际上是一个类的运算符重载。对于不同类型的PG有不同的实现。对于ReplicatedPG对应的实现类为RPCRecPred，其至少保证有一个处于up状态的OSD；对应ErasureCode(n+m)类型的PG，至少有n个处于up状态的OSD。

3） 如果prior.pg_down设置为true，就直接设置PG为PG_STATE_DOWN状态；

4） 检查是否需要need_up_thru设置；

5）用prior_set->probe设置probe_targets列表


###### 5.3 获取pg_info信息
在上述过程中计算出了PG在past_interval期间以及当前处于up状态的OSD列表，下面就发送请求给OSD来获取pg_info信息：
{% highlight string %}
void PG::RecoveryState::GetInfo::get_infos();
{% endhighlight %}

函数get_infos()向prior_set的probe集合中的每一个OSD发送pg_query_t::INFO消息，来获取PG在该OSD上的pg_info信息。发送消息的过程调用RecoveryMachine类的send_query()函数来进行：
{% highlight string %}
context< RecoveryMachine >().send_query(
	peer, pg_query_t(pg_query_t::INFO,
			 it->shard, pg->pg_whoami.shard,
			 pg->info.history,
			 pg->get_osdmap()->get_epoch()));

peer_info_requested.insert(peer);
pg->blocked_by.insert(peer.osd);
{% endhighlight %}
数据结构pg_notify_t定义了获取pg_info的ACK信息：
{% highlight string %}
struct pg_notify_t {
	epoch_t query_epoch;         //查询时请求消息的epoch
	epoch_t epoch_sent;          //发送时响应消息的epoch
	pg_info_t info;              //pg_info的信息
	shard_id_t to;               //目标OSD
	shard_id_t from;             //源OSD
};

boost::statechart::result PG::RecoveryState::Stray::react(const MQuery& query)
{
	PG *pg = context< RecoveryMachine >().pg;
	if (query.query.type == pg_query_t::INFO) {

		pair<pg_shard_t, pg_info_t> notify_info;
		pg->update_history_from_master(query.query.history);
		pg->fulfill_info(query.from, query.query, notify_info);

		context< RecoveryMachine >().send_notify(
			notify_info.first,
			pg_notify_t(
				notify_info.first.shard, pg->pg_whoami.shard,
				query.query_epoch,
				pg->get_osdmap()->get_epoch(),
				notify_info.second),
			pg->past_intervals);

	} else {
		pg->fulfill_log(query.from, query.query, query.query_epoch);
	}

	return discard_event();
}
{% endhighlight %}

----------

在主OSD接收到pg_info的ACK消息后封装成MNotifyRec事件发送给该PG对应的状态机。在下列的事件处理函数中来处理MNotifyRec事件：
{% highlight string %}
void OSD::dispatch_op(OpRequestRef op)
{
	switch (op->get_req()->get_type()) {
	
	case MSG_OSD_PG_CREATE:
		handle_pg_create(op);
		break;
	case MSG_OSD_PG_NOTIFY:
		handle_pg_notify(op);
		break;
	case MSG_OSD_PG_QUERY:
		handle_pg_query(op);
		break;
	case MSG_OSD_PG_LOG:
		handle_pg_log(op);
		break;
	case MSG_OSD_PG_REMOVE:
		handle_pg_remove(op);
		break;
	case MSG_OSD_PG_INFO:
		handle_pg_info(op);
		break;
	case MSG_OSD_PG_TRIM:
		handle_pg_trim(op);
		break;
	case MSG_OSD_PG_MISSING:
		assert(0 ==
	   		"received MOSDPGMissing; this message is supposed to be unused!?!");
		break;
	
	case MSG_OSD_BACKFILL_RESERVE:
		handle_pg_backfill_reserve(op);
		break;
	case MSG_OSD_RECOVERY_RESERVE:
		handle_pg_recovery_reserve(op);
		break;
	}
}

void OSD::handle_pg_notify(OpRequestRef op){
	...
}

void PG::handle_peering_event(CephPeeringEvtRef evt, RecoveryCtx *rctx){
	....
}

boost::statechart::result PG::RecoveryState::GetInfo::react(const MNotifyRec& infoevt){
	...
}
{% endhighlight %}

react()的具体处理过程如下：

1） 首先从peer_info_requested里删除该peer，同时从blocked_by队列里删除；

2） 调用函数PG::proc_replica_info()来处理副本的pg_info消息：

&emsp; a) 首先检查该OSD的pg_info信息，如果已经存在，并且last_update参数相同，则说明已经处理过，返回false值；否则保存该pg_info的值

&emsp; b) 调用函数has_been_up_since()检查该OSD在send_epoch时已经处于up状态；

&emsp; c) 确保自己是主OSD，把该OSD的pg_info信息保存到peer_info数组，并加入might_have_unfound里。该数组里的OSD用于后续的数据恢复；

&emsp; d) 调用函数unreg_next_scrub()使该PG不在scrub操作的队列中；

&emsp; e) 调用info.history.merge()函数处理```从OSD```发过来的pg_info信息。处理方法是：更新为最新的字段，设置dirty_info为true值；

&emsp; f) 调用函数reg_next_scrub()注册PG下一次的scrub的时间；

&emsp; g) 如果该OSD既不在up数组中也不在acting数组中，那就加入stray_set列表中。当PG处于clean状态时，就会调用purge_strays()函数删除stray状态的PG及其上的对象数据；

&emsp; h) 如果是一个新的OSD，就调用函数update_heartbeat_peers()更新需要heartbeat的OSD列表；

3） 在变量old_start里保存了调用PG::proc_replica_info()前主OSD的pg->info.history.last_epoch_started，如果该epoch值小于合并后的值，说明该值被更新了，从OSD上的epoch值比较新，需要进行如下操作：

&emsp; a) 调用pg->build_prior()重新构建prior_set对象；

&emsp; b) 从peer_info_requested队列中去掉上次构建的prior_set中存在的OSD，这里最新构建上次不存在的OSD列表；

&emsp; c) 调用get_infos()函数重新发送查询peer_info请求；


4） 调用pg->apply_peer_features()更新相关的features值；

5） 当peer_info_requested队列为空，并且prior_set不处于pg_down的状态时，说明收到所有OSD的peer_info并处理完成；

6） 最后检查past_interval阶段至少有一个OSD处于up状态且非incomplete状态；否则该PG无法恢复，标记状态为PG_STATE_DOWN并直接返回；

7） 最后完成处理，调用函数post_event(GotInfo())抛出GetInfo事件进入状态机的下一个状态

在GetInfo状态里直接定义了当前状态接收到```GotInfo```事件后，直接跳转到下一个状态```GetLog```里：
{% highlight string %}
struct GetInfo : boost::statechart::state< GetInfo, Peering >, NamedState {
	set<pg_shard_t> peer_info_requested;

	explicit GetInfo(my_context ctx);
	void exit();
	void get_infos();

	typedef boost::mpl::list <
		boost::statechart::custom_reaction< QueryState >,
		boost::statechart::transition< GotInfo, GetLog >,
		boost::statechart::custom_reaction< MNotifyRec >
	> reactions;
	boost::statechart::result react(const QueryState& q);
	boost::statechart::result react(const MNotifyRec& infoevt);
};
{% endhighlight %}


## 6. GetLog状态
当PG的主OSD获取到所有从OSD（以及past interval期间的所有参与该PG且目前仍处于active状态的OSD）的pg_info信息后，就跳转到GetLog状态。
{% highlight string %}
PG::RecoveryState::GetLog::GetLog(my_context ctx);
{% endhighlight %}
然后在GetLog的构造函数里做相应的处理，其具体处理过程分析如下：

1） 调用函数pg->choose_acting(auth_log_shard)选出具有权威日志的OSD，并计算出acting_backfill和backfill_targets两个OSD列表。输出保存在auth_log_shard里；

2） 如果选择失败并且want_acting不为空，就抛出NeedActingChange事件，状态机转移到Primary/WaitActingChange状态，等待申请临时PG返回结果。如果want_acting为空，就抛出IsIncomplete事件，PG的状态机转移到Primary/Peering/Incomplete状态。表明失败，PG就处于InComplete状态。

3）如果auth_log_shard等于pg->pg_whoami的值，也就是选出的拥有权威日志的OSD为当前主OSD，直接抛出事件GotLog()完成GetLog过程。

4） 如果pg->info.last_update小于权威OSD的log_tail，也就是本OSD的日志和权威日志不重叠，那么本OSD无法恢复，抛出IsIncomplete事件。经过函数choose_acting()的选择后，主OSD必须是可恢复的。如果主OSD不可恢复，必须申请一个临时PG，选择拥有权威日志的OSD为临时主OSD；

5） 如果自己不是权威日志的OSD，则需要去拥有权威日志的OSD上去拉取权威日志，并与本地合并。

###### 6.1 choose_acting
函数choose_acting用来计算PG的acting_backfill和backfill_targets两个OSD列表。acting_backfill保存了当前PG的acting列表，包括需要进行Backfill操作的OSD列表；backfill_targets列表保存了需要进行Backfill的OSD列表。
{% highlight string %}
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound);
{% endhighlight %}

其处理过程如下：

1) 首先调用函数find_best_info来选举出一个拥有权威日志的OSD，保存在变量auth_log_shard里；

2） 如果没有选举出拥有权威日志的OSD，则进入如下流程：

&emsp; a) 如果up不等于acting，申请临时PG，返回false值；

&emsp; b) 否则确保want_acting列表为空，返回false值

3） 计算是否是compat_mode模式，检查是，如果所有的OSD都支持纠删码，就设置compat_mode值为true；

4） 根据PG的不同类型，调用不同的函数。对应ReplicatedPG调用函数calc_replicated_acting来计算PG的需要列表；
{% highlight string %}
//want_backfill为该PG需要进行Backfill的pg_shard
//want_acting_backfill: 包括进行acting和Backfill的pg_shard
set<pg_shard_t> want_backfill, want_acting_backfill;

//主OSD
vector<int> want;

//在compat_mode模式下，和want_acting_backfill相同
pg_shard_t want_primary;
{% endhighlight %}


5） 下面就是对PG做的一些检查：

&emsp; a) 计算num_want_acting数量，检查如果小于min_size，进行如下操作

  * 如果对于EC，清空want_acting，返回false

  * 对于ReplicatedPG，如果该PG不允许小于min_size的恢复，清空want_acting，返回false值；

&emsp; b) 调用IsPGRecoverablePredicate来判断PG现有的OSD列表是否可以恢复，如果不能恢复，清空want_acting，返回false值

6） 检查如果want不等于acting，设置want_acting为want:

&emsp; a) 如果want_acting等于up，申请empty为pg_temp的OSD列表；

&emsp; b) 否则申请want为pg_temp的OSD列表；

7） 最后设置PG的actingbackfill为want_acting_backfill，设置backfill_targets为want_backfill，并检查backfill_targets里的pg_shard应该不在stray_set里面；

8） 最终返回true值；


----------

下面举例说明需要申请pg_temp的场景：

1） 当前```PG1.0```，其acting列表和up列表都为[0,1,2]，PG处于clean状态；

2） 此时，osd0崩溃，导致该PG经过CRUSH算法重新获得acting和up列表都为[3,1,2]

3) 选择出拥有权威日志的OSD为1，经过calc_replicated_acting()算法，want列表为[1,3,2]，acting_backfill为[1,3,2]，want_backfill为[3]。特别注意want列表第一个为主OSD，如果up_primary无法恢复，就选择权威日志的OSD为主OSD。

4） want[1,3,2]不等于acting[3,1,2]，并且不等于up[3,1,2]，需要向monitor申请pg_temp为want

5） 申请成功pg_temp以后，acting为[3,1,2]，up为[1,3,2]，osd1作为临时的主OSD，处理读写请求。当PG恢复处于clean状态时，pg_temp取消，acting和up都恢复为[3,1,2]。



###### 6.2 find_best_info
函数find_best_info()用于选取一个拥有权威日志的OSD。根据last_epoch_clean到目前为止，各个past_interval期间参与该PG的所有目前还处于up状态的OSD上的pg_info_t信息，来选取一个拥有权威日志的OSD，选择的优先顺序如下：

1） 具有最新last_update的OSD；

2） 如果条件1相同，选择日志更长的OSD；

3） 如果1,2条件相同，选择当前的主OSD；
{% highlight string %}
map<pg_shard_t, pg_info_t>::const_iterator PG::find_best_info(
  const map<pg_shard_t, pg_info_t> &infos,
  bool restrict_to_up_acting,
  bool *history_les_bound) const{
}
{% endhighlight %}
代码实现具体的过程如下：

1） 首先在所有OSD中计算max_last_epoch_started，然后在拥有最大的last_epoch_started的OSD中计算min_last_update_acceptable的值；

2） 如果min_last_update_acceptable为eversion_t::max()，返回infos.end()，选取失败；

3） 根据以下条件选择一个OSD：

&emsp; a) 首先过滤掉last_update小于min_last_update_acceptable，或者last_epoch_started小于max_last_epoch_started_found，或者处于incomplete的OSD。

&emsp; b) 如果PG类型是EC，选择最小的last_update；如果PG类型是副本，选择最大的last_update的OSD；

&emsp; c) 如果上述条件都相同，选择log tail最小的，也就是日志最长的OSD；

&emsp; d) 如果上述条件都相同，选择当前的主OSD；


综上的选择过程可知，拥有权威日志的OSD特征如下：必须是非incomplete的OSD；必须有最大的last_epoch_started；last_update有可能是最大，但至少是min_last_update_acceptable，有可能是日志最长的OSD，有可能是主OSD。



###### 6.3 calc_replicated_acting

本函数计算本PG相关的OSD列表：
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
}
{% endhighlight %}

* want_primary: 主OSD，如果它不是up_primary，就需要申请pg_temp;

* backfill: 需要进行Backfill操作的OSD；

* acting_backfill: 所有进行acting和Backfill的OSD的集合；

* want和acting_backfill的OSD相同，前者类型是pg_shard_t，后者是int类型

具体处理过程如下：

1） 首先选择want_primary列表中的OSD：

&emsp; a) 如果up_primary处于非incomplete状态，并且last_update大于等于权威日志的log_tail，说明up_primary的日志和权威日志有重叠，可通过日志记录恢复，优先选择up_primary为主OSD；

&emsp; b) 否则选择auth_log_shard，也就是拥有权威日志的OSD为主OSD；

&emsp; c) 把主OSD加入到want和acting_backfill列表中；

2） 函数的输入参数size为要选择的副本数，依次从up、acting、all_info里选择size个副本OSD：

&emsp; a) 如果该OSD上的PG处于incomplete的状态，或者cur_info.last_update小于主OSD和auth_log_shard的最小值，则该PG副本无法通过日志修复，只能通过Backfill操作来修复。把该OSD分别加入backfill和acting_backfill集合中。

&emsp; b) 否则就可以根据PG日志来恢复，只加入acting_backfill集合和want列表中，不用加入到Backfill列表中。

###### 6.4 收到缺失的权威日志
如果主OSD不是拥有权威日志的OSD，就需要去拥有权威日志的OSD上拉取权威日志：
{% highlight string %}
boost::statechart::result PG::RecoveryState::GetLog::react(const MLogRec& logevt)
{
  assert(!msg);
  if (logevt.from != auth_log_shard) {
    dout(10) << "GetLog: discarding log from "
	     << "non-auth_log_shard osd." << logevt.from << dendl;
    return discard_event();
  }
  dout(10) << "GetLog: received master log from osd"
	   << logevt.from << dendl;
  msg = logevt.msg;
  post_event(GotLog());
  return discard_event();
}
{% endhighlight %}
当收到权威日志后，封装成MLogRec类型的事件。本函数就用于处理该事件。它首先确认是从auth_log_shard端发送的消息，然后抛出GotLog()事件：
{% highlight string %}
boost::statechart::result PG::RecoveryState::GetLog::react(const GotLog&)
{
  dout(10) << "leaving GetLog" << dendl;
  PG *pg = context< RecoveryMachine >().pg;
  if (msg) {
    dout(10) << "processing master log" << dendl;
    pg->proc_master_log(*context<RecoveryMachine>().get_cur_transaction(),
			msg->info, msg->log, msg->missing, 
			auth_log_shard);
  }
  pg->start_flush(
    context< RecoveryMachine >().get_cur_transaction(),
    context< RecoveryMachine >().get_on_applied_context_list(),
    context< RecoveryMachine >().get_on_safe_context_list());
  return transit< GetMissing >();
}
{% endhighlight %}

本函数捕获GotLog事件，处理过程如下：

1) 如果msg不为空，就调用函数proc_master_log()合并自己缺失的权威日志，并更新自己pg_info相关的信息。从此，作为主OSD，也是拥有权威日志的OSD。

2） 调用函数pg->start_flush()添加一个空操作；

3) 状态转移到GetMissing状态

经过GetLog阶段的处理后，该PG的主OSD已经获取了权威日志，以及pg_info的权威信息。

## 7. GetMissing状态
GetMissing的处理过程为：首先，拉取各个从OSD上的有效日志。其次，用主OSD上的权威日志与各个从OSD的日志进行对比，从而计算出各从OSD上不一致的对象并保存在对应的pg_missing_t结构中，作为后续数据修复的依据。

主OSD的不一致的对象信息，已经在调用proc_master_log()合并权威日志的过程中计算出来，所以这里只计算从OSD上的不一致对象。

###### 7.1 拉取从副本上的日志
在GetMissing的构造函数里，通过对比主OSD上的权威pg_info信息，来获取从OSD上的日志信息：
{% highlight string %}
PG::RecoveryState::GetMissing::GetMissing(my_context ctx);
{% endhighlight %}

其具体处理过程为遍历pg->actingbackfill的OSD列表，然后做如下的处理：

1）不需要获取PG日志的情况：

&emsp; a) pi.is_empty()为空，没有任何信息，需要Backfill过程来修复，不需要获取日志；

&emsp; b) pi.last_update小于pg->pg_log.get_tail()，该OSD的pg_info记录中，last_update小于权威日志的尾部，该OSD的日志和权威日志不重叠，该OSD操作已经远远落后于权威OSD，已经无法根据日志来修复，需要Backfill过程来修复；

&emsp; c) pi.last_backfill为hobject_t()，说明在past_interval期间，该OSD标记需要Backfill操作，实际并没开始Backfill的工作，需要继续Backfill过程；

&emsp; d) pi.last_update等于pi.last_complete，说明该PG没有丢失的对象，已经完成Recovery操作阶段，并且pi.last_update等于pg->info.last_update，说明日志和权威日志的最后更新一致，说明该PG数据完整，不需要恢复。

2） 获取日志的情况：当pi.last_update大于pg->info.log_tail，该OSD的日志记录和权威日志记录重叠，可以通过日志来修复。变量since是从last_epoch_started开始的版本值：

&emsp; a) 如果该PG的日志记录pi.log_tail小于等于版本值since，那就发送消息pg_query_t::LOG，从since开始获取日志记录；

&emsp; b) 如果该PG的日志记录pi.log_tail大于版本值since，就发送消息pg_query_t::FULLLOG来获取该OSD的全部日志记录。


3） 最后检查如果peer_missing_requested为空，说明所有获取日志的请求返回并处理完成。如果需要pg->need_up_thru，抛出post_event(NeedUpThru())；否则，直接调用post_event(Activate(pg->get_osdmap()->get_epoch()))进入```Activate```状态。

下面举例说明获取日志的两种情况：

![ceph-chapter10-21](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_21.jpg)

当前last_epoch_started的值为10,since是last_epoch_started后的首个日志版本值。当前需要恢复的有效日志是经过since操作之后的日志，之前的日志已经没有用了。

对应osd0，其日志log_tail大于since，全部拷贝osd0上的日志；对应osd1，其日志log_tail小于since，只拷贝从since开始的日志记录。


###### 7.2 收到从副本上的日志记录处理
当一个PG的主OSD收到从OSD返回的获取日志ACK应答后，就把该消息封装成MLogRec事件。状态GetMissing接收到该事件后，在下列事件函数里处理该事件：
{% highlight string %}
boost::statechart::result PG::RecoveryState::GetMissing::react(const MLogRec& logevt)
{
  PG *pg = context< RecoveryMachine >().pg;

  peer_missing_requested.erase(logevt.from);
  pg->proc_replica_log(*context<RecoveryMachine>().get_cur_transaction(),
		       logevt.msg->info, logevt.msg->log, logevt.msg->missing, logevt.from);
  
  if (peer_missing_requested.empty()) {
    if (pg->need_up_thru) {
      dout(10) << " still need up_thru update before going active" << dendl;
      post_event(NeedUpThru());
    } else {
      dout(10) << "Got last missing, don't need missing "
	       << "posting CheckRepops" << dendl;
      post_event(Activate(pg->get_osdmap()->get_epoch()));
    }
  }
  return discard_event();
}
{% endhighlight %}
具体过程如下：

1） 调用proc_replica_log()处理日志。通过日志的对比，获取该OSD上处于missing状态的对象列表；

2) 如果peer_missing_requested为空，即所有的获取日志请求返回并处理。如果需要pg->need_up_thru，抛出NeedUpThru()事件。否则，直接调用函数post_event(Activate(pg->get_osdmap()->get_epoch()))进入Activate状态。

函数proc_replica_log()处理各个从OSD上发过来的日志。它通过比较该OSD的日志和本地权威日志，来计算该OSD上处于missing状态的对象列表。具体处理过程调用pg_log.proc_replica_log()来处理日志，输出为omissing，也就是该OSD缺失的对象。



## 8. Active操作
由上可知，如果GetMissing处理成功，就跳转到Activate状态。到本阶段为止，可以说peering主要工作已经完成，但还需要后续的处理，激活各个副本，如下所示：
{% highlight string %}
PG::RecoveryState::Active::Active(my_context ctx);
{% endhighlight %}
状态Activate的构造函数里处理过程如下：

1） 在构造函数里初始化了remote_shards_to_reserve_recovery和remote_shards_to_reserve_backfill，需要Recovery操作和Backfill操作的OSD。

2） 调用函数pg->start_flush()来完成相关数据的flush工作。

3） 调用函数pg->activate()完成最后的激活工作


###### 8.1 MissingLoc
类MissingLoc用来记录处于missing状态对象的位置，也就是缺失对象的正确版本分别在哪些OSD上。恢复时就去这些OSD上去拉取正确对象的对象数据：
{% highlight string %}
 class MissingLoc {
	//缺失的对象 ---> item(现在版本，缺失的版本)
	map<hobject_t, pg_missing_t::item, hobject_t::BitwiseComparator> needs_recovery_map;

	//缺失的对象 ---> 所在的OSD集合
	map<hobject_t, set<pg_shard_t>, hobject_t::BitwiseComparator > missing_loc;

	//所有缺失对象所在的OSD集合
	set<pg_shard_t> missing_loc_sources;
	PG *pg;
	set<pg_shard_t> empty_set;
};
{% endhighlight %}

下面介绍一些MissingLoc处理函数，作用是添加相应的missing对象列表。其对应两个函数：add_active_missing()函数和add_source_info()函数。

add_active_missing()函数用于把一个副本中的所有缺失对象添加到MissingLoc的needs_recovery_map结构里：
{% highlight string %}
void add_active_missing(const pg_missing_t &missing);
{% endhighlight %}

add_source_info()函数用于计算每个缺失对象是否在本OSD上：
{% highlight string %}
/// Adds info about a possible recovery source
bool add_source_info(
	pg_shard_t source,               ///< [in] source
	const pg_info_t &oinfo,         ///< [in] info
	const pg_missing_t &omissing,   ///< [in] (optional) missing
	bool sort_bitwise,             ///< [in] local sort bitwise (vs nibblewise)
	ThreadPool::TPHandle* handle   ///< [in] ThreadPool handle
	);                             ///< @return whether a new object location was discovered
{% endhighlight %}
具体实现如下：

遍历needs_recovery_map里的所有对象，对每个对象做如下处理：

1） 如果oinfo.last_update < need(所需的缺失对象的版本），就跳过；

2） 如果该PG正常的last_backfill指针小于MAX值，说明还处于Backfill阶段，但是sort_bitwise不正确，跳过；

3） 如果该对象大于last_backfill，显然该对象不存在，跳过；

4） 如果该对象大于last_complete，说明该对象或者是上次Peering之后缺失的对象，还没有来得及修复；或者是新创建的对象。检查如果在missing记录已存在，就是上次缺失的对象，直接跳过；否则就是新创建的对象，存在该OSD中。

5） 经过上述检查后，确认该对象在本OSD上，在missing_loc添加该对象的location为本OSD。

###### 8.2 Active状态
PG::activate()函数是Peering过程的最后一步：
{% highlight string %}
void PG::activate(ObjectStore::Transaction& t,
		  epoch_t activation_epoch,
		  list<Context*>& tfin,
		  map<int, map<spg_t,pg_query_t> >& query_map,
		  map<int,
		      vector<
			pair<pg_notify_t,
			     pg_interval_map_t> > > *activator_map,
                  RecoveryCtx *ctx);
{% endhighlight %}
该函数完成以下功能：

- 更新一些pg_info的参数信息

- 给replica发消息，激活副本PG

- 计算MissingLoc，也就是缺失对象分布在哪些OSD上，用于后续的恢复

具体处理过程如下：

1） 如果需要客户回答，就把PG添加到replay_queue队列里；

2） 更新info.last_epoch_started变量，info.last_epoch_started指的是本OSD在完成目前Peering进程后的更新，而info.history.last_epoch_started是PG的所有OSD都确认完成Peering的更新。

3） 更新一些相关的字段；

4） 注册C_PG_ActivateCommitted()回调函数，该函数最终完成activate的工作；

5） 初始化snap_trimq快照相关的变量；

6） 设置info.last_complete指针：

  * 如果missing.num_missing()等于0，表明处于clean状态。直接更新info.last_complete等于info.last_update，并调用pg_log.reset_recovery_pointers()调整log的complete_to指针。

  * 否则，如果有需要恢复的对象，就调用函数pg_log.activate_not_complete(info)，设置info.last_complete为缺失的第一个对象的前一个版本。

7） 以下都是主OSD的操作，给每个从OSD发送MOSDPGLog类型的消息，激活该PG的从OSD上的副本。分别对应三种不同处理：

  * 如果pi.last_update等于info.last_update，这种情况下，该OSD本身就是clean的，不需要给该OSD发送其他信息。添加到activator_map只发送pg_info来激活从OSD。其最终的执行在PeeringWQ的线程执行完状态机的事件处理后，在函数OSD::dispatch_context()里调用OSD::do_info()函数实现；

  * 需要Backfill操作的OSD，发送pg_info，以及osd_min_pg_log_entries数量的PG日志；

  * 需要Recovery操作的OSD，发送pg_info，以及从缺失的日志

8） 设置MissingLoc，也就是统计缺失的对象，以及缺失的对象所在的OSD，核心就是调用MissingLoc的add_source_info()函数，见MissingLoc的相关分析。

9） 如果需要恢复，把该PG加入到osd->queue_for_recovery(this)的恢复队列中

10） 如果PG的size小于act set的size，也就是当前的OSD不够，就标记PG的状态为```PG_STATE_DEGRADED```和```PG_STATE_UNDERSIZED```状态，最后标记PG为```PG_STATE_ACTIVATING```状态

###### 8.3 收到从OSD的MOSDPGLog的应答
当收到从OSD发送的MOSDPGLog的ACK消息后，触发MInfoRec事件，下面这个函数处理该事件：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Active::react(const MInfoRec& infoevt);
{% endhighlight %}

处理过程比较简单： 检查该请求的源OSD在本PG的actingbackfill列表中，以及等待列表中删除该OSD。最后检查，当收集到所有从OSD发送的ACK，就调用函数all_activated_and_committed()触发AllReplicasActivated事件。

对应主OSD在事务的回调函数C_PG_ActivateCommitted()里实现，最终调用_activate_committed()加入peer_activated集合里。

###### 8.4 AllReplicasActivated
这个函数处理AllReplicasActivated事件：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Active::react(const AllReplicasActivated &evt);
{% endhighlight %}
当所有的replica处于activated状态时，进行如下处理：

1） 取消PG_STATE_ACTIVATING和PG_STATE_CREATING状态，如果该PG上acting状态的OSD数量大于等于pool的min_size，设置该PG为PG_STATE_ACTIVE的状态，否则设置为PG_STATE_PEERED状态；

2） ReplicatedPG::check_local()检查本地的stray对象是否被删除

3） 如果有读写请求在等待peering操作，则把该请求添加到处理队列pg->requeue_ops(pg->waiting_for_peered)；

4） 调用函数ReplicatedPG::on_activate()，如果需要Recovery操作，触发DoRecovery事件，如果需要Backfill操作，触发RequestBackfill事件；否则触发AllReplicasRecovered事件；

5） 初始化Cache Tier需要的hit_set对象；

6） 初始化Cache Tier需要的agent对象；





## 9. 副本端的状态转移
当创建PG后，根据不同的角色，如果是主OSD，PG对应的状态机就进入Primary状态。如果不是主OSD，就进入Stray状态。

###### 9.1 Stray状态
Stray状态有两种情况：

**情况1：** 只接收到PGINFO的处理
{% highlight string %}
boost::statechart::result PG::RecoveryState::Stray::react(const MInfoRec& infoevt);
{% endhighlight %}

从PG接收到主PG发送的MInfoRec事件，也就是接收到主OSD发送的pg_info信息。其判断如果当前pg->info.last_update大于infoevt.info.last_update，说明当前的日志有divergent的日志，调用函数rewind_divergent_log()清理日志即可。最后抛出Activate(infoevt.info.last_epoch_started)事件，进入ReplicaActive状态。


----------


**情况2：** 接收到MOSDPGLog消息
{% highlight string %}
boost::statechart::result PG::RecoveryState::Stray::react(const MLogRec& logevt);
{% endhighlight %}
当从PG接收到MLogRec事件，就对应着接收到主PG发送的MOSDPGLog消息，其通知PG处于activate状态，具体处理过程如下：

1） 如果msg->info.last_backfill为hobject_t()，需要Backfill操作的OSD；

2） 否则就是需要Recovery操作的OSD，调用merge_log()把主OSD发送过来的日志合并

抛出Activate(logevt.msg->info.last_eopch_started)事件，使副本转移到ReplicaActive状态。


###### 9.2 ReplicaActive状态
ReplicaActive状态如下：
{% highlight string %}
boost::statechart::result PG::RecoveryState::ReplicaActive::react(
  const Activate& actevt);
{% endhighlight %}

当处于ReplicaActive状态，接收到Activate事件，就调用函数pg->activate()。在函数_activate_committed()给主PG发送应答信息，告诉自己处于activate状态，设置PG为activate状态。





## 10. 状态机异常处理

在上面的流程介绍中，只介绍了正常状态机的转换流程。Ceph之所以用状态机来实现PG的状态转换，就是可以实现任何异常情况下的处理。下面介绍当OSD失效时，导致相关的PG重新进行Peering的机制。

当一个OSD失效，Monitor会通过heartbeat检测到，导致osd map发生了变化，Monitor会把最新的osd map推送给OSD，导致OSD上的受影响PG重新进行Peering操作。

具体流程如下：

1） 在函数OSD::handle_osd_map()处理osd map的变化，该函数调用consume_map()，对每一个PG调用pg->queue_null()，把PG加入到peering_wq中。

2） peering_wq的处理函数process_peering_events()调用OSD::advance_pg()函数，在该函数里调用PG::handle_advance_map()给PG的状态机RecoveryMachine发送AdvMap事件：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Started::react(const AdvMap& advmap);
{% endhighlight %}
当处于Started状态，接收到AdvMap事件，调用函数pg->should_restart_peering()检查，如果是new_interval，就跳转到Reset状态，重新开始一次Peering过程。

## 11. 小结

本章介绍了Ceph的Peering过程，其核心过程就是通过各个OSD上保存的PG日志选出一个权威日志的OSD。以该OSD上的日志为基础，对比其他OSD上的日志记录，计算出各个OSD上缺失的对象信息。这样，PG就使各个OSD的数据达成了一致。



<br />
<br />

**[参看]**

1. [Ceph源码解析：PG peering](https://www.cnblogs.com/chenxianpao/p/5565286.html)

2. [PEERING](https://docs.ceph.com/docs/master/dev/peering/)

3. [ceph存储 PG的数据恢复过程](https://blog.csdn.net/skdkjzz/article/details/51579432)

4. [Ceph: manually repair object](https://ceph.com/planet/ceph-manually-repair-object/)

<br />
<br />
<br />

