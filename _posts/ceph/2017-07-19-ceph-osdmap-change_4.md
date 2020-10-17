---
layout: post
title: OSD从down到out过程中osdmap的变化(3)
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

如下我们接着上一篇分析PG 11.4进入Recovering状态后的恢复过程。

<!-- more --> 

## 1. 进入Recovering状态
在上一篇文章中，我们最后讲述到了PG 11.4进入到了Recovering状态。如下是此一阶段的一个日志片段：
{% highlight string %}
94073:2020-09-11 14:13:14.601894 7fba3d124700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] enter Started/Primary/Active/Recovering
94074:2020-09-11 14:13:14.601916 7fba3d124700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] publish_stats_to_osd 2231:1343
94075:2020-09-11 14:13:14.601948 7fba3d124700 10 osd.3 2231 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]
94078:2020-09-11 14:13:14.601970 7fba37919700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] lock
94079:2020-09-11 14:13:14.601984 7fba37919700 10 osd.3 2231 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]
94080:2020-09-11 14:13:14.601992 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_replicas(1)
94081:2020-09-11 14:13:14.601999 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  peer osd.2 missing 1 objects.
94082:2020-09-11 14:13:14.602005 7fba37919700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  peer osd.2 missing {11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head=201'1}
94083:2020-09-11 14:13:14.602016 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_replicas: recover_object_replicas(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head)
94084:2020-09-11 14:13:14.602023 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] prep_object_replica_pushes: on 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94085:2020-09-11 14:13:14.602031 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] get_object_context: obc NOT found in cache: 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94086:2020-09-11 14:13:14.602088 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] populate_obc_watchers 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94087:2020-09-11 14:13:14.602104 7fba37919700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] ReplicatedPG::check_blacklisted_obc_watchers for obc 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94088:2020-09-11 14:13:14.602112 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] get_object_context: creating obc from disk: 0x7fba66a80c00
94089:2020-09-11 14:13:14.602118 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] get_object_context: 0x7fba66a80c00 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head rwstate(none n=0 w=0) oi: 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head(201'1 client.34245.0:25 dirty|data_digest|omap_digest s 1172 uv 1 dd 21971aec od ffffffff alloc_hint [0 0]) ssc: 0x7fba6f624460 snapset: 0=[]:[]+head
94090:2020-09-11 14:13:14.602137 7fba37919700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recovery got recovery read lock on 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94091:2020-09-11 14:13:14.602147 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] start_recovery_op 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94092:2020-09-11 14:13:14.602158 7fba37919700 10 osd.3 2231 start_recovery_op pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head (1/3 rops)
94093:2020-09-11 14:13:14.602179 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_object: 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94094:2020-09-11 14:13:14.602193 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] prep_push_to_replica: 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head v201'1 size 1172 to osd.2
94095:2020-09-11 14:13:14.602201 7fba37919700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] push_to_replica snapset is 0=[]:[]+head
94096:2020-09-11 14:13:14.602208 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] calc_head_subsets 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head clone_overlap {}
94097:2020-09-11 14:13:14.602215 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] calc_head_subsets 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head  data_subset [0~1172]  clone_subsets {}
94098:2020-09-11 14:13:14.602227 7fba37919700  7 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] send_push_op 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head v 201'1 size 1172 recovery_info: ObjectRecoveryInfo(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head@201'1, size: 1172, copy_subset: [0~1172], clone_subset: {})
94099:2020-09-11 14:13:14.602395 7fba37919700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] send_pushes: sending push PushOp(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head, version: 201'1, data_included: [0~1172], data_size: 1172, omap_header_size: 0, omap_entries_size: 0, attrset_size: 2, recovery_info: ObjectRecoveryInfo(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head@201'1, size: 1172, copy_subset: [0~1172], clone_subset: {}), after_progress: ObjectRecoveryProgress(!first, data_recovered_to:1172, data_complete:true, omap_recovered_to:, omap_complete:true), before_progress: ObjectRecoveryProgress(first, data_recovered_to:0, data_complete:false, omap_recovered_to:, omap_complete:false)) to osd.2
94100:2020-09-11 14:13:14.602442 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  started 1
94101:2020-09-11 14:13:14.602448 7fba37919700 10 osd.3 2231 do_recovery started 1/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]
94130:2020-09-11 14:13:14.663609 7fba45134700 20 osd.3 2231 _dispatch 0x7fba6b4d3400 pg_trim(11.4 to 201'1 e2231) v1
94131:2020-09-11 14:13:14.663622 7fba45134700  7 osd.3 2231 handle_pg_trim pg_trim(11.4 to 201'1 e2231) v1 from osd.2
94133:2020-09-11 14:13:14.663669 7fba45134700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] lock
94134:2020-09-11 14:13:14.663695 7fba45134700 10 osd.3 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] replica osd.2 lcod 201'1
94142:2020-09-11 14:13:14.664128 7fba306fb700 10 osd.3 2231 handle_replica_op MOSDPGPushReply(11.4 2231 [PushReplyOp(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head)]) v2 epoch 2231
94144:2020-09-11 14:13:14.664146 7fba306fb700 15 osd.3 2231 enqueue_op 0x7fba6ba44700 prio 3 cost 8389608 latency 0.000100 MOSDPGPushReply(11.4 2231 [PushReplyOp(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head)]) v2
94147:2020-09-11 14:13:14.664228 7fba3811a700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] lock
94150:2020-09-11 14:13:14.664304 7fba3811a700 10 osd.3 2231 dequeue_op 0x7fba6ba44700 prio 3 cost 8389608 latency 0.000257 MOSDPGPushReply(11.4 2231 [PushReplyOp(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head)]) v2 pg pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]
94151:2020-09-11 14:13:14.664341 7fba3811a700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] handle_message: 0x7fba6ba44700
94153:2020-09-11 14:13:14.664373 7fba3811a700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] publish_stats_to_osd 2231:1344
94155:2020-09-11 14:13:14.664396 7fba3811a700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] publish_stats_to_osd 2231:1345
94156:2020-09-11 14:13:14.664408 7fba3811a700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] pushed 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head to all replicas
94157:2020-09-11 14:13:14.664423 7fba3811a700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  requeue_ops 
94158:2020-09-11 14:13:14.664435 7fba3811a700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] finish_recovery_op 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94160:2020-09-11 14:13:14.664451 7fba3811a700 10 osd.3 2231 finish_recovery_op pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head dequeue=0 (1/3 rops)
94161:2020-09-11 14:13:14.664488 7fba3811a700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] finish_degraded_object 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94166:2020-09-11 14:13:14.664540 7fba37919700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] lock
94167:2020-09-11 14:13:14.664562 7fba37919700 10 osd.3 2231 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]
94168:2020-09-11 14:13:14.664573 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_replicas(1)
94170:2020-09-11 14:13:14.664586 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  peer osd.2 missing 0 objects.
94171:2020-09-11 14:13:14.664598 7fba37919700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  peer osd.2 missing {}
94173:2020-09-11 14:13:14.664621 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_primary recovering 0 in pg
94174:2020-09-11 14:13:14.664633 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_primary missing(0)
94175:2020-09-11 14:13:14.664664 7fba37919700 25 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_primary {}
94176:2020-09-11 14:13:14.664676 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  started 0
94178:2020-09-11 14:13:14.664687 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] start_recovery_ops needs_recovery: {}
94179:2020-09-11 14:13:14.664699 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] start_recovery_ops missing_loc: {}
94180:2020-09-11 14:13:14.664710 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] needs_recovery is recovered
94182:2020-09-11 14:13:14.664721 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] needs_backfill does not need backfill
94183:2020-09-11 14:13:14.664733 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] recovery done, no backfill
94184:2020-09-11 14:13:14.664754 7fba37919700 10 osd.3 2231 do_recovery started 0/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded]
94186:2020-09-11 14:13:14.664987 7fba3d925700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] lock
94187:2020-09-11 14:13:14.665019 7fba3d925700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] handle_peering_event: epoch_sent: 2231 epoch_requested: 2231 AllReplicasRecovered
94188:2020-09-11 14:13:14.665069 7fba3d925700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] exit Started/Primary/Active/Recovering 0.063174 1 0.000091
94189:2020-09-11 14:13:14.665093 7fba3d925700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] enter Started/Primary/Active/Recovered
94190:2020-09-11 14:13:14.665116 7fba3d925700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] needs_recovery is recovered
94192:2020-09-11 14:13:14.665139 7fba3d925700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active] publish_stats_to_osd 2231:1346
94193:2020-09-11 14:13:14.665158 7fba3d925700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active] exit Started/Primary/Active/Recovered 0.000065 0 0.000000
94194:2020-09-11 14:13:14.665187 7fba3d925700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active] enter Started/Primary/Active/Clean
94195:2020-09-11 14:13:14.665199 7fba3d925700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active] finish_recovery
94197:2020-09-11 14:13:14.665210 7fba3d925700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active] clear_recovery_state
94199:2020-09-11 14:13:14.665229 7fba3d925700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] trim_past_intervals: trimming interval(2223-2225 up [3](3) acting [3](3) maybe_went_rw)
94201:2020-09-11 14:13:14.665246 7fba3d925700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2226-2226/1 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] trim_past_intervals: trimming interval(2226-2226 up [3,2](3) acting [3](3))
94202:2020-09-11 14:13:14.665261 7fba3d925700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] share_pg_info
94206:2020-09-11 14:13:14.665311 7fba3d925700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] publish_stats_to_osd 2231:1347
94229:2020-09-11 14:13:14.666750 7fba49ec6700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] lock
94231:2020-09-11 14:13:14.666768 7fba49ec6700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] _finish_recovery
94232:2020-09-11 14:13:14.666780 7fba49ec6700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] purge_strays 
94234:2020-09-11 14:13:14.666796 7fba49ec6700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] publish_stats_to_osd 2231: no change since 2020-09-11 14:13:14.665310
94471:2020-09-11 14:13:15.100566 7fba49ec6700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] lock
94696:2020-09-11 14:13:15.102433 7fba49ec6700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] lock
94697:2020-09-11 14:13:15.102438 7fba49ec6700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] null
95427:2020-09-11 14:13:15.107961 7fba3d124700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] lock
95430:2020-09-11 14:13:15.107975 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] handle_advance_map [3,2]/[3,2] -- 3/3
95434:2020-09-11 14:13:15.108002 7fba3d124700 10 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] state<Started/Primary/Active>: Active advmap
95436:2020-09-11 14:13:15.108010 7fba3d124700 10 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] state<Started>: Started advmap
95438:2020-09-11 14:13:15.108016 7fba3d124700 10 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] check_recovery_sources no source osds () went down
95439:2020-09-11 14:13:15.108023 7fba3d124700 10 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] handle_activate_map 
95440:2020-09-11 14:13:15.108032 7fba3d124700 10 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] state<Started/Primary/Active>: Active: handling ActMap
95442:2020-09-11 14:13:15.108040 7fba3d124700  7 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] state<Started/Primary>: handle ActMap primary
95444:2020-09-11 14:13:15.108047 7fba3d124700 15 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] publish_stats_to_osd 2231: no change since 2020-09-11 14:13:14.665310
95445:2020-09-11 14:13:15.108055 7fba3d124700 10 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] take_waiters
95446:2020-09-11 14:13:15.108062 7fba3d124700 20 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] handle_activate_map: Not dirtying info: last_persisted is 2231 while current is 2232
95447:2020-09-11 14:13:15.108069 7fba3d124700 10 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] handle_peering_event: epoch_sent: 2232 epoch_requested: 2232 NullEvt
97188:2020-09-11 14:13:15.784921 7fba4f557700 25 osd.3 2232  sending 11.4 2231:1347
97408:2020-09-11 14:13:16.204365 7fba46937700 25 osd.3 2232  ack on 11.4 2231:1347
97507:2020-09-11 14:13:16.231365 7fba49ec6700 30 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] lock
97833:2020-09-11 14:13:16.263230 7fba49ec6700 30 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] lock
97836:2020-09-11 14:13:16.263246 7fba49ec6700 10 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] null
98381:2020-09-11 14:13:16.268353 7fba3d124700 30 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] lock
98383:2020-09-11 14:13:16.268362 7fba3d124700 10 osd.3 pg_epoch: 2232 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] handle_advance_map [3,2]/[3,2] -- 3/3
98385:2020-09-11 14:13:16.268371 7fba3d124700 10 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] state<Started/Primary/Active>: Active advmap
98386:2020-09-11 14:13:16.268375 7fba3d124700 10 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] state<Started>: Started advmap
98387:2020-09-11 14:13:16.268380 7fba3d124700 10 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] check_recovery_sources no source osds () went down
98388:2020-09-11 14:13:16.268385 7fba3d124700 10 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] handle_activate_map 
98389:2020-09-11 14:13:16.268389 7fba3d124700 10 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] state<Started/Primary/Active>: Active: handling ActMap
98390:2020-09-11 14:13:16.268393 7fba3d124700  7 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] state<Started/Primary>: handle ActMap primary
98391:2020-09-11 14:13:16.268398 7fba3d124700 15 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] publish_stats_to_osd 2231: no change since 2020-09-11 14:13:14.665310
98392:2020-09-11 14:13:16.268403 7fba3d124700 10 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] take_waiters
98393:2020-09-11 14:13:16.268407 7fba3d124700 20 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] handle_activate_map: Not dirtying info: last_persisted is 2231 while current is 2233
98394:2020-09-11 14:13:16.268412 7fba3d124700 10 osd.3 pg_epoch: 2233 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2231/0 2226/2227/2223) [3,2] r=0 lpr=2227 crt=201'1 lcod 0'0 mlcod 0'0 active+clean] handle_peering_event: epoch_sent: 2233 epoch_requested: 2233 NullEvt
132571:2020-09-11 14:17:52.776670 7fba4fd58700 20 osd.3 2233 11.4 heartbeat_peers 2,3
{% endhighlight %}

如下是Recovering的构造函数：
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
在上面的构造函数中首先将自己的状态设置为```Recovering```，之后调用OSD::queue_for_recovery():
{% highlight string %}
bool OSDService::queue_for_recovery(PG *pg)
{
  bool b = recovery_wq.queue(pg);
  if (b)
    dout(10) << "queue_for_recovery queued " << *pg << dendl;
  else
    dout(10) << "queue_for_recovery already queued " << *pg << dendl;
  return b;
}
{% endhighlight %}
上面只是简单的将PG加入到了recovery_wq队列中。

### 1.1 OSD Recovery线程
OSD的Recovery线程会读取recovery_wq中的任务，然后进行处理：
{% highlight string %}
void RecoveryWQ::_process(PG *pg, ThreadPool::TPHandle &handle) override {
	osd->do_recovery(pg, handle);
	pg->put("RecoveryWQ");
}
void OSD::do_recovery(PG *pg, ThreadPool::TPHandle &handle)
{

}
{% endhighlight %}

###### 1.1.1 函数OSD::do_recovery()
{% highlight string %}
void OSD::do_recovery(PG *pg, ThreadPool::TPHandle &handle)
{
	if (max <= 0) {
		dout(10) << "do_recovery raced and failed to start anything; requeuing " << *pg << dendl;
		recovery_wq.queue(pg);
		return;
	}else{
		dout(10) << "do_recovery starting " << max << " " << *pg << dendl;
		#ifdef DEBUG_RECOVERY_OIDS
			dout(20) << "  active was " << recovery_oids[pg->info.pgid] << dendl;
		#endif
		
		int started = 0;
		bool more = pg->start_recovery_ops(max, handle, &started);
		dout(10) << "do_recovery started " << started << "/" << max << " on " << *pg << dendl;

		...

		PG::RecoveryCtx rctx = create_context();
		rctx.handle = &handle;
		
		/*
		* if we couldn't start any recovery ops and things are still
		* unfound, see if we can discover more missing object locations.
		* It may be that our initial locations were bad and we errored
		* out while trying to pull.
		*/
		if (!more && pg->have_unfound()) {
			pg->discover_all_missing(*rctx.query_map);
			if (rctx.query_map->empty()) {
				dout(10) << "do_recovery  no luck, giving up on this pg for now" << dendl;
				recovery_wq.lock();
				recovery_wq._dequeue(pg);
				recovery_wq.unlock();
			}
		}
		
		pg->write_if_dirty(*rctx.transaction);
		OSDMapRef curmap = pg->get_osdmap();
		pg->unlock();
		dispatch_context(rctx, pg, curmap);
	}
}
{% endhighlight %}
在上面的do_recovery()函数中会调用PG::start_recovery_ops()来开启Recovery动作，之后创建一个RecoveryCtx将相应的恢复动作进行记录。

如下是相应日志片段：
{% highlight string %}
94079:2020-09-11 14:13:14.601984 7fba37919700 10 osd.3 2231 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]
94080:2020-09-11 14:13:14.601992 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_replicas(1)
94081:2020-09-11 14:13:14.601999 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  peer osd.2 missing 1 objects.
94082:2020-09-11 14:13:14.602005 7fba37919700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  peer osd.2 missing {11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head=201'1}
94083:2020-09-11 14:13:14.602016 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_replicas: recover_object_replicas(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head)
94084:2020-09-11 14:13:14.602023 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] prep_object_replica_pushes: on 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94085:2020-09-11 14:13:14.602031 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] get_object_context: obc NOT found in cache: 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94086:2020-09-11 14:13:14.602088 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] populate_obc_watchers 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94087:2020-09-11 14:13:14.602104 7fba37919700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] ReplicatedPG::check_blacklisted_obc_watchers for obc 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94088:2020-09-11 14:13:14.602112 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] get_object_context: creating obc from disk: 0x7fba66a80c00
94089:2020-09-11 14:13:14.602118 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] get_object_context: 0x7fba66a80c00 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head rwstate(none n=0 w=0) oi: 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head(201'1 client.34245.0:25 dirty|data_digest|omap_digest s 1172 uv 1 dd 21971aec od ffffffff alloc_hint [0 0]) ssc: 0x7fba6f624460 snapset: 0=[]:[]+head
94090:2020-09-11 14:13:14.602137 7fba37919700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recovery got recovery read lock on 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94091:2020-09-11 14:13:14.602147 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] start_recovery_op 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94092:2020-09-11 14:13:14.602158 7fba37919700 10 osd.3 2231 start_recovery_op pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head (1/3 rops)
94093:2020-09-11 14:13:14.602179 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] recover_object: 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head
94094:2020-09-11 14:13:14.602193 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] prep_push_to_replica: 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head v201'1 size 1172 to osd.2
94095:2020-09-11 14:13:14.602201 7fba37919700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] push_to_replica snapset is 0=[]:[]+head
94096:2020-09-11 14:13:14.602208 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] calc_head_subsets 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head clone_overlap {}
94097:2020-09-11 14:13:14.602215 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] calc_head_subsets 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head  data_subset [0~1172]  clone_subsets {}
94098:2020-09-11 14:13:14.602227 7fba37919700  7 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] send_push_op 11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head v 201'1 size 1172 recovery_info: ObjectRecoveryInfo(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head@201'1, size: 1172, copy_subset: [0~1172], clone_subset: {})
94099:2020-09-11 14:13:14.602395 7fba37919700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded] send_pushes: sending push PushOp(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head, version: 201'1, data_included: [0~1172], data_size: 1172, omap_header_size: 0, omap_entries_size: 0, attrset_size: 2, recovery_info: ObjectRecoveryInfo(11:3b9ee248:::zone_info.28570f88-31e7-4166-b6d7-d22903cead75:head@201'1, size: 1172, copy_subset: [0~1172], clone_subset: {}), after_progress: ObjectRecoveryProgress(!first, data_recovered_to:1172, data_complete:true, omap_recovered_to:, omap_complete:true), before_progress: ObjectRecoveryProgress(first, data_recovered_to:0, data_complete:false, omap_recovered_to:, omap_complete:false)) to osd.2
94100:2020-09-11 14:13:14.602442 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]  started 1
94101:2020-09-11 14:13:14.602448 7fba37919700 10 osd.3 2231 do_recovery started 1/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 rops=1 crt=201'1 lcod 0'0 mlcod 0'0 active+recovering+degraded]
{% endhighlight %}

1) **函数ReplicatedPG::start_recovery_ops()**

{% highlight string %}
bool ReplicatedPG::start_recovery_ops(
  int max, ThreadPool::TPHandle &handle,
  int *ops_started)
{
	...


	const pg_missing_t &missing = pg_log.get_missing();
	
	int num_missing = missing.num_missing();
	int num_unfound = get_num_unfound();
	
	if (num_missing == 0) {
		info.last_complete = info.last_update;
	}
	
	if (num_missing == num_unfound) {
		// All of the missing objects we have are unfound.
		// Recover the replicas.
		started = recover_replicas(max, handle);
	}
	if (!started) {
		// We still have missing objects that we should grab from replicas.
		started += recover_primary(max, handle);
	}
	if (!started && num_unfound != get_num_unfound()) {
		// second chance to recovery replicas
		started = recover_replicas(max, handle);
	}


	bool deferred_backfill = false;
	if (recovering.empty() && state_test(PG_STATE_BACKFILL) && !backfill_targets.empty() && started < max && missing.num_missing() == 0 && waiting_on_backfill.empty()) {
		if (get_osdmap()->test_flag(CEPH_OSDMAP_NOBACKFILL)) {
			dout(10) << "deferring backfill due to NOBACKFILL" << dendl;
			deferred_backfill = true;
		} else if (get_osdmap()->test_flag(CEPH_OSDMAP_NOREBALANCE) &&!is_degraded())  {
			dout(10) << "deferring backfill due to NOREBALANCE" << dendl;
			deferred_backfill = true;
		} else if (!backfill_reserved) {
			dout(10) << "deferring backfill due to !backfill_reserved" << dendl;
			if (!backfill_reserving) {
				dout(10) << "queueing RequestBackfill" << dendl;
				backfill_reserving = true;
				queue_peering_event(
					CephPeeringEvtRef(
						std::make_shared<CephPeeringEvt>(
						  get_osdmap()->get_epoch(),
						  get_osdmap()->get_epoch(),
						 RequestBackfill())));
			}
			deferred_backfill = true;
		} else {
			started += recover_backfill(max - started, handle, &work_in_progress);
		}
	}

	...
}
{% endhighlight %}

a) 这里首先获取num_missing以及num_unfound的值。如果num_missing为0，直接将info.last_complete设置为info.last_update，表明当前已经处于同步状态； 如果num_missing等于num_unfound，则调用ReplicatedPG::recover_replicas()恢复副本；如果recover_replicas()返回值为0，说明主OSD本身就缺失相应对象，这时需要调用recovery_primary()先恢复主OSD。

b) 在recovering恢复完成之后，才进行Backfill操作

2） **函数ReplicatedPG::recover_replicas()**
{% highlight string %}
int ReplicatedPG::recover_replicas(int max, ThreadPool::TPHandle &handle)
{
	...

	for (set<pg_shard_t>::iterator i = actingbackfill.begin(); i != actingbackfill.end();++i) {

		if (*i == get_primary()) continue;
		pg_shard_t peer = *i;
		map<pg_shard_t, pg_missing_t>::const_iterator pm = peer_missing.find(peer);

		for (map<version_t, hobject_t>::const_iterator p = m.rmissing.begin();p != m.rmissing.end() && started < max;++p) {
			
			...
			
			dout(10) << __func__ << ": recover_object_replicas(" << soid << ")" << dendl;
			map<hobject_t,pg_missing_t::item, hobject_t::ComparatorWithDefault>::const_iterator r = m.missing.find(soid);
			started += prep_object_replica_pushes(soid, r->second.need,

		}
	}

	pgbackend->run_recovery_op(h, get_recovery_op_priority());
	return started;
}
{% endhighlight %}
recovery_replicas()读取副本OSD的missing列表，然后将主OSD上的相应对象通过prep_object_replicas_pushes()推送给从OSD。发送的推送消息类型为：PushOp,响应的消息类型为PushReplyOp.

3） **函数ReplicatedPG::recover_primary()**
{% highlight string %}
int ReplicatedPG::recover_primary(int max, ThreadPool::TPHandle &handle)
{
	while (p != missing.rmissing.end()) {
		...

		if (latest) {
			switch (latest->op) {
				case pg_log_entry_t::CLONE:

					...


				case pg_log_entry_t::LOST_REVERT:
					...

				
			}
		}

		...

		if (!recovering.count(soid)) {
			if (recovering.count(head)) {
				++skipped;
			} else {
				int r = recover_missing(
					soid, need, get_recovery_op_priority(), h);
				switch (r) {
				case PULL_YES:
					++started;
					break;
				case PULL_OTHER:
					++started;
				case PULL_NONE:
					++skipped;
					break;
				default:
					assert(0);
				}
				if (started >= max)
					break;
			}
		}

		...
	}

	pgbackend->run_recovery_op(h, get_recovery_op_priority());
	return started;
}
{% endhighlight %}
ReplicatedPG::recover_primary()用于恢复主OSD上的数据。

### 1.2 进入Active/Recovered状态
在所有副本数据都恢复完成，并且没有需要进行Backfill操作的OSD时，则产生AllReplicasRecovered事件，进入Started/Primary/Active/Recovered状态：
{% highlight string %}
bool ReplicatedPG::start_recovery_ops(
  int max, ThreadPool::TPHandle &handle,
  int *ops_started)
{
	...
	if (state_test(PG_STATE_RECOVERING)) {
		state_clear(PG_STATE_RECOVERING);
		if (needs_backfill()) {
			dout(10) << "recovery done, queuing backfill" << dendl;
			queue_peering_event(
				CephPeeringEvtRef(
				  std::make_shared<CephPeeringEvt>(
					get_osdmap()->get_epoch(),
					get_osdmap()->get_epoch(),
					RequestBackfill())));
		} else {
			dout(10) << "recovery done, no backfill" << dendl;
			queue_peering_event(
				CephPeeringEvtRef(
				  std::make_shared<CephPeeringEvt>(
					get_osdmap()->get_epoch(),
					get_osdmap()->get_epoch(),
					AllReplicasRecovered())));
		}
	} else { // backfilling
		state_clear(PG_STATE_BACKFILL);
		dout(10) << "recovery done, backfill done" << dendl;
		queue_peering_event(
			CephPeeringEvtRef(
			  std::make_shared<CephPeeringEvt>(
				get_osdmap()->get_epoch(),
				get_osdmap()->get_epoch(),
				Backfilled())));
	}
	return false;
}

boost::statechart::result
PG::RecoveryState::Recovering::react(const AllReplicasRecovered &evt)
{
	PG *pg = context< RecoveryMachine >().pg;
	pg->state_clear(PG_STATE_RECOVERING);
	release_reservations();
	return transit<Recovered>();
}
{% endhighlight %}
Recovering状态接收到AllReplicasRecovered事件后，清除Recovering状态，直接进入Recovered状态。

如下是相应构造函数：
{% highlight string %}
PG::RecoveryState::Recovered::Recovered(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Active/Recovered")
{
  pg_shard_t auth_log_shard;

  context< RecoveryMachine >().log_enter(state_name);

  PG *pg = context< RecoveryMachine >().pg;
  pg->osd->local_reserver.cancel_reservation(pg->info.pgid);

  assert(!pg->needs_recovery());

  // if we finished backfill, all acting are active; recheck if
  // DEGRADED | UNDERSIZED is appropriate.
  assert(!pg->actingbackfill.empty());
  if (pg->get_osdmap()->get_pg_size(pg->info.pgid.pgid) <=
      pg->actingbackfill.size()) {
    pg->state_clear(PG_STATE_DEGRADED);
    pg->publish_stats_to_osd();
  }

  // adjust acting set?  (e.g. because backfill completed...)
  bool history_les_bound = false;
  if (pg->acting != pg->up && !pg->choose_acting(auth_log_shard,
						 true, &history_les_bound))
    assert(pg->want_acting.size());

  if (context< Active >().all_replicas_activated)
    post_event(GoClean());
}
{% endhighlight %}

之后如果all_replicas_activated，则直接进入Clean状态。

### 1.3 进入Active/Clean状态
{% highlight string %}
PG::RecoveryState::Clean::Clean(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Active/Clean")
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	
	if (pg->info.last_complete != pg->info.last_update) {
		assert(0);
	}
	pg->finish_recovery(*context< RecoveryMachine >().get_on_safe_context_list());
	pg->mark_clean();
	
	pg->share_pg_info();
	pg->publish_stats_to_osd();

}
{% endhighlight %}
进入Active/Clean状态后，pg->info.last_complete一定等于pg->info.last_update。同时修改PG状态为```Clean```。


<br />
<br />
<br />

1. [ceph存储 PG的状态机和peering过程](https://blog.csdn.net/skdkjzz/article/details/51579903)

2. [Ceph OSDMap 机制浅析](https://www.jianshu.com/p/8ecd6028f5ff?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)