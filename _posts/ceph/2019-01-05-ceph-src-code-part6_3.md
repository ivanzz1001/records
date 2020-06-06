---
layout: post
title: ceph中PGLog处理流程
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

ceph的PGLog是由PG来维护，记录了该PG的所有操作，其作用类似于数据库里的undo log。PGLog通常只保存近千条的操作记录(默认是3000条， 由osd_min_pg_log_entries指定)，但是当PG处于降级状态时，就会保存更多的日志（默认是10000条），这样就可以在故障的PG重新上线后用来恢复PG的数据。本文主要从PG的格式、存储方式、如何参与恢复来解析PGLog。

相关配置说明：

* osd_min_pg_log_entries: 正常情况下pg log记录的条数

* osd_max_pg_log_entries: 异常情况下pg log记录的条数，达到该限制会进行trim操作

<!-- more -->


## 1. PGLog的格式
ceph使用版本控制的方式来标记一个PG内的每一次更新，每个版本包括一个(epoch, version)来组成。其中，epoch是osdmap的版本，每当有OSD状态变化（如增加、删除等时），epoch就递增；version是PG内每次更新操作的版本号，是递增的，由PG内的Primary OSD进行分配。

PGLog在代码实现中有3个主要的数据结构来维护（相关代码位于src/osd/osd_types.h中）:


* pg_log_entry_t

结构体pg_log_entry_t记录了PG日志的单条记录，其数据结构如下：
{% highlight string %}
struct pg_log_entry_t {
	ObjectModDesc mod_desc;                 //用于保存本地回滚的一些信息，用于EC模式下的回滚操作

	bufferlist snaps;                       //克隆操作，用于记录当前对象的snap列表
	hobject_t  soid;                        //操作的对象
	osd_reqid_t reqid;                      //请求唯一标识(caller + tid)


	vector<pair<osd_reqid_t, version_t> > extra_reqids;


	eversion_t version；                     //本次操作的版本
	eversion_t prior_version;                //前一个操作的版本
	eversion_t reverting_to;                 //本次操作回退的版本（仅用于回滚操作）

	version_t user_version;                 //用户的版本号
	utime_t     mtime;                      //用户的本地时间
	
	__s32      op;                          //操作的类型
	bool invalid_hash;                      // only when decoding sobject_t based entries
	bool invalid_pool;                      // only when decoding pool-less hobject based entries

	...
};
{% endhighlight %}


* pg_log_t

结构体pg_log_t在内存中保存了该PG的所有操作日志，以及相关的控制结构：
{% highlight string %}
/**
 * pg_log_t - incremental log of recent pg changes.
 *
 *  serves as a recovery queue for recent changes.
 */
struct pg_log_t {
	/*
	 *   head - newest entry (update|delete)
	 *   tail - entry previous to oldest (update|delete) for which we have
	 *          complete negative information.  
	 * i.e. we can infer pg contents for any store whose last_update >= tail.
	*/
	eversion_t head;                           //日志的头，记录最新的日志记录
	eversion_t tail;                           //指向最老的pg log记录的前一个


	eversion_t can_rollback_to;                //用于EC，指示本地可以回滚的版本， 可回滚的版本都大于can_rollback_to的值


	//在EC的实现中，本地保留了不同版本的数据。本数据段指示本PG里可以删除掉的对象版本。rollback_info_trimmed_to的值 <= can_rollback_to
	eversion_t rollback_info_trimmed_to;         

	//所有日志的列表
	list<pg_log_entry_t> log;  // the actual log.
	
	...
};
{% endhighlight %}

需要注意的是，PG日志的记录是以整个PG为单位，包括该PG内的所有对象的修改记录。

* pg_info_t

结构体pg_info_t是对当前PG信息的一个统计：
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
	spg_t pgid;                    //对应的PG ID

	//PG内最近一次更新的对象的版本，还没有在所有OSD上完成更新。在last_update和last_complete之间的操作表示
	//该操作已在部分OSD上完成，但是还没有全部完成。
	eversion_t last_update;        
	eversion_t last_complete;      //该指针之前的版本都已经在所有的OSD上完成更新（只表示内存更新完成）

	epoch_t last_epoch_started;    //本PG在启动时候的epoch值
	
	version_t last_user_version;   //最后更新的user object的版本号
	
	eversion_t log_tail;           //用于记录日志的尾部版本
	
	//上一次backfill操作的对象指针。如果该OSD的Backfill操作没有完成，那么[last_bakfill, last_complete)之间的对象可能
	//处于missing状态
	hobject_t last_backfill;      


	bool last_backfill_bitwise;            //true if last_backfill reflects a bitwise (vs nibblewise) sort
	
	interval_set<snapid_t> purged_snaps;   //PG要删除的snap集合
	
	pg_stat_t stats;                       //PG的统计信息
	
	pg_history_t history;                  //PG的历史信息
	pg_hit_set_history_t hit_set;          //这是Cache Tier用的hit_set
};
{% highlight string %}





<br />
<br />

**[参看]**


1. [PGLog写流程梳理](https://blog.csdn.net/Z_Stand/article/details/100082984)

2. [ceph存储 ceph中pglog处理流程](https://blog.csdn.net/skdkjzz/article/details/51488926)

3. [ceph PGLog处理流程](https://my.oschina.net/linuxhunter/blog/679829?p=1)

4. [ceph基于pglog的一致性协议](https://jingyan.baidu.com/article/fa4125ace14cf028ac7092f4.html)


<br />
<br />
<br />

