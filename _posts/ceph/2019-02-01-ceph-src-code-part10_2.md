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

2） 如果日志的head相同，说明没有分歧日志



## 3. Peering的状态转换图


## 4. pg_info数据结构

## 5. GetInfo状态

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

