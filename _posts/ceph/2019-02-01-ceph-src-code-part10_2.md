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
假设一个PG的acting set为[0,1,2]列表。此时如果osd0出现故障，导致CRUSH算法重新分配该PG的acting set为[3,1,2]。此时osd3为该PG的主OSD，但是osd3为新加入的OSD，并不能负担该PG上的读操作。所以PG向Monitor申请一个临时的PG，osd1为临时的主OSD，这是up set变为[1,3,2]，acting set依然为[3,1,2]，导致acting set和up set不同。当osd3完成Backfill过程之后，临时PG被取消，该PG的up set修复为acting set，此时acting set和up set都为[3,1,2]列表。

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
数据结构pg_info_t保存了PG在OSD上的一些描述信息。该数据结构在Peering的过程中，以及后续的数据修复中都发挥了重要的作用，理解该数据结构的各个关节字段的含义可以更好地理解相关的过程。pg_info_t数据结构如下：
{% highlight string %}
struct pg_info_t {
	spg_t pgid;                             //PG的id
	eversion_t last_update;                 //PG最后一次更新的版本
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
###### 4.1 last_epoch_started介绍
last_epoch_started字段有两个地方出现，一个是pg_info_t结构里的last_epoch_started，代表最后一次Peering成功后的epoch值，是本地PG完成Peering后就设置的。另一个是pg_history_t结构体里的last_epoch_started，是PG里所有的OSD都完成Peering后设置的epoch值。

###### 4.2 last_complete和last_backfill的区别
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




## 6. GetLog状态

## 7. GetMissing状态

## 8. Active操作

## 9. 副本端的状态转移


## 10. 小结



<br />
<br />

**[参看]**

1. [Ceph源码解析：PG peering](https://www.cnblogs.com/chenxianpao/p/5565286.html)


<br />
<br />
<br />

