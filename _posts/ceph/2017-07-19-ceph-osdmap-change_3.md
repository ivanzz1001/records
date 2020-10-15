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

如下我们接着上一篇文章，分析在将osd0踢出osdmap之后，PG 11.4所执行的相关动作。

<!-- more --> 

 
## 1. PG 11.4接收到新的osdmap
这里我们首先给出在这一阶段PG 11.4恢复过程的日志：
{% highlight string %}
40998:2020-09-11 14:10:18.963214 7fba49ec6700 30 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
41206:2020-09-11 14:10:18.964245 7fba49ec6700 30 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
41208:2020-09-11 14:10:18.964251 7fba49ec6700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] null
41632:2020-09-11 14:10:18.965606 7fba49ec6700 20 osd.3 2226 11.4 heartbeat_peers 3
42218:2020-09-11 14:10:18.974032 7fba3d925700 30 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] lock
42220:2020-09-11 14:10:18.974041 7fba3d925700 10 osd.3 pg_epoch: 2225 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] handle_advance_map [3,2]/[3] -- 3/3
42222:2020-09-11 14:10:18.974048 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] state<Started/Primary/Active>: Active advmap
42223:2020-09-11 14:10:18.974052 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] state<Started>: Started advmap
42224:2020-09-11 14:10:18.974056 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] new interval newup [3,2] newacting [3]
42225:2020-09-11 14:10:18.974060 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] state<Started>: should_restart_peering, transitioning to Reset
42226:2020-09-11 14:10:18.974064 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] exit Started/Primary/Active/Clean 299.069829 4 0.000234
42227:2020-09-11 14:10:18.974070 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active+undersized+degraded] exit Started/Primary/Active 299.106820 0 0.000000
42228:2020-09-11 14:10:18.974074 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active] agent_stop
42229:2020-09-11 14:10:18.974078 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active] exit Started/Primary 299.838624 0 0.000000
42230:2020-09-11 14:10:18.974082 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 crt=201'1 lcod 0'0 mlcod 0'0 active] clear_primary_state
42231:2020-09-11 14:10:18.974086 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] agent_stop
42232:2020-09-11 14:10:18.974090 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] exit Started 299.838686 0 0.000000
42233:2020-09-11 14:10:18.974095 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] enter Reset
42234:2020-09-11 14:10:18.974099 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2223 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] set_last_peering_reset 2226
42235:2020-09-11 14:10:18.974102 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2226 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] Clearing blocked outgoing recovery messages
42236:2020-09-11 14:10:18.974106 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2226 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] Not blocking outgoing recovery messages
42237:2020-09-11 14:10:18.974110 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2226 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] state<Reset>: Reset advmap
42238:2020-09-11 14:10:18.974114 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2226 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] _calc_past_interval_range start epoch 2224 >= end epoch 2223, nothing to do
42239:2020-09-11 14:10:18.974118 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2226 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] new interval newup [3,2] newacting [3]
42240:2020-09-11 14:10:18.974122 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2226 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] state<Reset>: should restart peering, calling start_peering_interval again
42241:2020-09-11 14:10:18.974125 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2226 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] set_last_peering_reset 2226
42242:2020-09-11 14:10:18.974135 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active+remapped] start_peering_interval: check_new_interval output: generate_past_intervals interval(2223-2225 up [3](3) acting [3](3)): not rw, up_thru 2223 up_from 2123 last_epoch_clean 2224
42245:2020-09-11 14:10:18.974140 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active+remapped]  noting past interval(2223-2225 up [3](3) acting [3](3) maybe_went_rw)
42246:2020-09-11 14:10:18.974148 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active+remapped]  up [3] -> [3,2], acting [3] -> [3], acting_primary 3 -> 3, up_primary 3 -> 3, role 0 -> 0, features acting 576460752032874495 upacting 576460752032874495
42247:2020-09-11 14:10:18.974155 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] clear_primary_state
42248:2020-09-11 14:10:18.974161 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] agent_stop
42249:2020-09-11 14:10:18.974167 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] on_change
42250:2020-09-11 14:10:18.974171 7fba3d925700 15 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped]  requeue_ops 
42251:2020-09-11 14:10:18.974176 7fba3d925700 15 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] publish_stats_to_osd 2226:1334
42252:2020-09-11 14:10:18.974179 7fba3d925700 15 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped]  requeue_ops 
42253:2020-09-11 14:10:18.974183 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] cancel_copy_ops
42254:2020-09-11 14:10:18.974187 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] cancel_flush_ops
42255:2020-09-11 14:10:18.974190 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] cancel_proxy_ops
42256:2020-09-11 14:10:18.974194 7fba3d925700 15 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped]  requeue_ops 
42257:2020-09-11 14:10:18.974197 7fba3d925700 15 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped]  requeue_ops 
42258:2020-09-11 14:10:18.974201 7fba3d925700 15 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped]  requeue_ops 
42259:2020-09-11 14:10:18.974205 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] on_change_cleanup
42260:2020-09-11 14:10:18.974219 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] on_change
42261:2020-09-11 14:10:18.974223 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit NotTrimming
42262:2020-09-11 14:10:18.974227 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] enter NotTrimming
42263:2020-09-11 14:10:18.974231 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] [3] -> [3], replicas changed
42264:2020-09-11 14:10:18.974235 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] cancel_recovery
42265:2020-09-11 14:10:18.974238 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] clear_recovery_state
42266:2020-09-11 14:10:18.974242 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] check_recovery_sources no source osds () went down
42267:2020-09-11 14:10:18.974247 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] handle_activate_map 
42268:2020-09-11 14:10:18.974251 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] update_heartbeat_peers 3 -> 2,3
42270:2020-09-11 14:10:18.974256 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] take_waiters
42271:2020-09-11 14:10:18.974260 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit Reset 0.000165 1 0.000199
42272:2020-09-11 14:10:18.974264 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] enter Started
42273:2020-09-11 14:10:18.974268 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] enter Start
42274:2020-09-11 14:10:18.974272 7fba3d925700  1 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Start>: transitioning to Primary
42275:2020-09-11 14:10:18.974276 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit Start 0.000007 0 0.000000
42276:2020-09-11 14:10:18.974280 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] enter Started/Primary
42277:2020-09-11 14:10:18.974284 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] enter Started/Primary/Peering
42278:2020-09-11 14:10:18.974287 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] enter Started/Primary/Peering/GetInfo
42279:2020-09-11 14:10:18.974292 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] _calc_past_interval_range: already have past intervals back to 2224
42280:2020-09-11 14:10:18.974297 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering]  PriorSet: build_prior interval(2223-2225 up [3](3) acting [3](3) maybe_went_rw)
42281:2020-09-11 14:10:18.974301 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering]  PriorSet: build_prior final: probe 2,3 down  blocked_by {}
42282:2020-09-11 14:10:18.974305 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] up_thru 2223 < same_since 2226, must notify monitor
42283:2020-09-11 14:10:18.974310 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>:  querying info from osd.2
42284:2020-09-11 14:10:18.974318 7fba3d925700 15 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] publish_stats_to_osd 2226:1335
42285:2020-09-11 14:10:18.974322 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] handle_activate_map: Not dirtying info: last_persisted is 2224 while current is 2226
42286:2020-09-11 14:10:18.974326 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] handle_peering_event: epoch_sent: 2226 epoch_requested: 2226 NullEvt
44149:2020-09-11 14:10:18.981601 7fba45134700 20 osd.3 2226 _dispatch 0x7fba6de6d0e0 pg_notify(11.4 epoch 2226) v5
44160:2020-09-11 14:10:18.981660 7fba45134700 30 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] lock
44968:2020-09-11 14:10:18.984778 7fba3d124700 30 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] lock
44969:2020-09-11 14:10:18.984783 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] handle_peering_event: epoch_sent: 2226 epoch_requested: 2226 MNotifyRec from 2 notify: (query_epoch:2226, epoch_sent:2226, info:11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)) features: 0x7ffffffefdfbfff
44970:2020-09-11 14:10:18.984789 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering]  got osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
44971:2020-09-11 14:10:18.984795 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] update_heartbeat_peers 2,3 unchanged
44972:2020-09-11 14:10:18.984799 7fba3d124700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>: Adding osd: 2 peer features: 7ffffffefdfbfff
44973:2020-09-11 14:10:18.984803 7fba3d124700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>: Common peer features: 7ffffffefdfbfff
44974:2020-09-11 14:10:18.984817 7fba3d124700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>: Common acting features: 7ffffffefdfbfff
44975:2020-09-11 14:10:18.984820 7fba3d124700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>: Common upacting features: 7ffffffefdfbfff
44976:2020-09-11 14:10:18.984825 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] exit Started/Primary/Peering/GetInfo 0.010536 2 0.000071
44977:2020-09-11 14:10:18.984830 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] enter Started/Primary/Peering/GetLog
44978:2020-09-11 14:10:18.984834 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] calc_acting osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
44979:2020-09-11 14:10:18.984839 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] calc_acting osd.3 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223)
44980:2020-09-11 14:10:18.984847 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] calc_acting newest update on osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223)
44982:calc_acting primary is osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223)
44983: osd.2 (up) accepted 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
44985:2020-09-11 14:10:18.984851 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] choose_acting want [3,2] != acting [3], requesting pg_temp change
44986:2020-09-11 14:10:18.984857 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] exit Started/Primary/Peering/GetLog 0.000026 0 0.000000
44987:2020-09-11 14:10:18.984861 7fba3d124700 15 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] publish_stats_to_osd 2226:1336
44988:2020-09-11 14:10:18.984865 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering>: Leaving Peering
44989:2020-09-11 14:10:18.984869 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] exit Started/Primary/Peering 0.010585 0 0.000000
44990:2020-09-11 14:10:18.984874 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit Started/Primary 0.010593 0 0.000000
44991:2020-09-11 14:10:18.984878 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] clear_primary_state
44992:2020-09-11 14:10:18.984883 7fba3d124700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] agent_stop
44993:2020-09-11 14:10:18.984887 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] enter Started/Primary
44994:2020-09-11 14:10:18.984891 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] enter Started/Primary/Peering/WaitActingChange
45166:2020-09-11 14:10:18.985528 7fba3d124700 10 osd.3 2226 send_pg_temp {11.4=[],11.6=[],19.1=[],22.2c=[],22.44=[],22.a4=[],22.b5=[],22.ca=[],22.d0=[],22.ec=[],23.d=[],23.13=[],23.30=[],23.6d=[]}
46089:2020-09-11 14:10:19.631436 7fba4fd58700 20 osd.3 2226 11.4 heartbeat_peers 2,3
46393:2020-09-11 14:10:20.010087 7fba49ec6700 30 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] lock
46688:2020-09-11 14:10:20.011975 7fba49ec6700 30 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] lock
46691:2020-09-11 14:10:20.011990 7fba49ec6700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] null
47206:2020-09-11 14:10:20.014765 7fba3d925700 30 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] lock
47209:2020-09-11 14:10:20.014782 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] handle_advance_map [3,2]/[3,2] -- 3/3
47212:2020-09-11 14:10:20.014796 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Started/Primary/Peering/WaitActingChange>: verifying no want_acting [] targets didn't go down
47279:2020-09-11 14:10:20.014814 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Started>: Started advmap
47281:2020-09-11 14:10:20.015156 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] new interval newup [3,2] newacting [3,2]
47283:2020-09-11 14:10:20.015174 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Started>: should_restart_peering, transitioning to Reset
47285:2020-09-11 14:10:20.015184 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit Started/Primary/Peering/WaitActingChange 1.030293 1 0.000106
47288:2020-09-11 14:10:20.015195 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit Started/Primary 1.030307 0 0.000000
47290:2020-09-11 14:10:20.015205 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] clear_primary_state
47292:2020-09-11 14:10:20.015215 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] agent_stop
47294:2020-09-11 14:10:20.015227 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit Started 1.040962 0 0.000000
47295:2020-09-11 14:10:20.015238 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] enter Reset
47296:2020-09-11 14:10:20.015245 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] set_last_peering_reset 2227
47299:2020-09-11 14:10:20.015254 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] Clearing blocked outgoing recovery messages
47301:2020-09-11 14:10:20.015266 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] Not blocking outgoing recovery messages
47303:2020-09-11 14:10:20.015280 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Reset>: Reset advmap
47394:2020-09-11 14:10:20.015296 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] _calc_past_interval_range: already have past intervals back to 2224
47396:2020-09-11 14:10:20.015871 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] new interval newup [3,2] newacting [3,2]
47399:2020-09-11 14:10:20.015889 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Reset>: should restart peering, calling start_peering_interval again
47400:2020-09-11 14:10:20.015898 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] set_last_peering_reset 2227
47403:2020-09-11 14:10:20.015920 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] start_peering_interval: check_new_interval output: generate_past_intervals interval(2226-2226 up [3,2](3) acting [3](3)): not rw, up_thru 2223 up_from 2123 last_epoch_clean 2224
47409:2020-09-11 14:10:20.015946 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  noting past interval(2226-2226 up [3,2](3) acting [3](3))
47411:2020-09-11 14:10:20.015960 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  up [3,2] -> [3,2], acting [3] -> [3,2], acting_primary 3 -> 3, up_primary 3 -> 3, role 0 -> 0, features acting 576460752032874495 upacting 576460752032874495
47413:2020-09-11 14:10:20.015972 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] clear_primary_state
47414:2020-09-11 14:10:20.015983 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] agent_stop
47415:2020-09-11 14:10:20.015991 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_change
47417:2020-09-11 14:10:20.015998 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47419:2020-09-11 14:10:20.016009 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] publish_stats_to_osd 2227:1337
47421:2020-09-11 14:10:20.016019 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47422:2020-09-11 14:10:20.016027 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_copy_ops
47424:2020-09-11 14:10:20.016034 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_flush_ops
47426:2020-09-11 14:10:20.016042 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_proxy_ops
47428:2020-09-11 14:10:20.016051 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47430:2020-09-11 14:10:20.016060 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47432:2020-09-11 14:10:20.016070 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47435:2020-09-11 14:10:20.016080 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_change_cleanup
47437:2020-09-11 14:10:20.016090 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_change
47440:2020-09-11 14:10:20.016101 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit NotTrimming
47442:2020-09-11 14:10:20.016112 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter NotTrimming
47444:2020-09-11 14:10:20.016122 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] [3] -> [3,2], replicas changed
47447:2020-09-11 14:10:20.016131 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_recovery
47449:2020-09-11 14:10:20.016140 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] clear_recovery_state
47451:2020-09-11 14:10:20.016150 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] check_recovery_sources no source osds () went down
47454:2020-09-11 14:10:20.016168 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] handle_activate_map 
47457:2020-09-11 14:10:20.016178 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] update_heartbeat_peers 2,3 unchanged
47458:2020-09-11 14:10:20.016188 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] take_waiters
47461:2020-09-11 14:10:20.016198 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit Reset 0.000960 1 0.001368
47463:2020-09-11 14:10:20.016211 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started
47465:2020-09-11 14:10:20.016221 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Start
47467:2020-09-11 14:10:20.016231 7fba3d925700  1 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] state<Start>: transitioning to Primary
47469:2020-09-11 14:10:20.016242 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit Start 0.000020 0 0.000000
47472:2020-09-11 14:10:20.016253 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started/Primary
47474:2020-09-11 14:10:20.016262 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started/Primary/Peering
47475:2020-09-11 14:10:20.016271 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetInfo
47478:2020-09-11 14:10:20.016281 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] _calc_past_interval_range: already have past intervals back to 2224
47481:2020-09-11 14:10:20.016292 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2226-2226 up [3,2](3) acting [3](3))
47482:2020-09-11 14:10:20.016301 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2223-2225 up [3](3) acting [3](3) maybe_went_rw)
47483:2020-09-11 14:10:20.016310 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior final: probe 2,3 down  blocked_by {}
47484:2020-09-11 14:10:20.016318 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] up_thru 2226 < same_since 2227, must notify monitor
47485:2020-09-11 14:10:20.016327 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>:  querying info from osd.2
47487:2020-09-11 14:10:20.016337 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2227:1338
47489:2020-09-11 14:10:20.016346 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_activate_map: Not dirtying info: last_persisted is 2226 while current is 2227
47492:2020-09-11 14:10:20.016356 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2227 epoch_requested: 2227 NullEvt
50548:2020-09-11 14:10:20.028700 7fba3d124700 20   ? 1956'80 (1956'79) modify   23:1ce16903:::obj-9bjaNA0lG2ZpqCJ:head by client.291879.0:1164 2020-05-30 10:20:25.595078
50723:2020-09-11 14:10:20.029810 7fba3d124700 20 update missing, append 1956'80 (1956'79) modify   23:1ce16903:::obj-9bjaNA0lG2ZpqCJ:head by client.291879.0:1164 2020-05-30 10:20:25.595078
50826:2020-09-11 14:10:20.031026 7fba45134700 20 osd.3 2227 _dispatch 0x7fba6d7401e0 pg_notify(11.4 epoch 2227) v5
50829:2020-09-11 14:10:20.031044 7fba45134700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
50838:2020-09-11 14:10:20.031074 7fba3d124700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
50841:2020-09-11 14:10:20.031095 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2227 epoch_requested: 2227 MNotifyRec from 2 notify: (query_epoch:2227, epoch_sent:2227, info:11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)) features: 0x7ffffffefdfbfff
50846:2020-09-11 14:10:20.031104 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  got osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
50848:2020-09-11 14:10:20.031115 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] update_heartbeat_peers 2,3 unchanged
50850:2020-09-11 14:10:20.031122 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Adding osd: 2 peer features: 7ffffffefdfbfff
50851:2020-09-11 14:10:20.031129 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Common peer features: 7ffffffefdfbfff
50854:2020-09-11 14:10:20.031135 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Common acting features: 7ffffffefdfbfff
50857:2020-09-11 14:10:20.031140 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Common upacting features: 7ffffffefdfbfff
50859:2020-09-11 14:10:20.031147 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetInfo 0.014876 2 0.000170
50862:2020-09-11 14:10:20.031154 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetLog
50863:2020-09-11 14:10:20.031161 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
50865:2020-09-11 14:10:20.031168 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting osd.3 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223)
50873:2020-09-11 14:10:20.031192 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting newest update on osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223)
50875:calc_acting primary is osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223)
50876: osd.2 (up) accepted 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
50878:2020-09-11 14:10:20.031210 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] actingbackfill is 2,3
50880:2020-09-11 14:10:20.031215 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] choose_acting want [3,2] (== acting) backfill_targets 
50881:2020-09-11 14:10:20.031220 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetLog>: leaving GetLog
50882:2020-09-11 14:10:20.031225 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetLog 0.000070 0 0.000000
50883:2020-09-11 14:10:20.031242 7fba3d124700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2227:1339
50885:2020-09-11 14:10:20.031249 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetMissing
50892:2020-09-11 14:10:20.031254 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetMissing>:  still need up_thru update before going active
50894:2020-09-11 14:10:20.031270 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetMissing 0.000021 0 0.000000
50895:2020-09-11 14:10:20.031275 7fba3d124700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2227: no change since 2020-09-11 14:10:20.031240
50896:2020-09-11 14:10:20.031285 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/WaitUpThru
51762:2020-09-11 14:10:20.420033 7fba496c5700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
51764:2020-09-11 14:10:20.420048 7fba496c5700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] flushed
51770:2020-09-11 14:10:20.420077 7fba3d124700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
51772:2020-09-11 14:10:20.420086 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2227 epoch_requested: 2227 FlushedEvt
51773:2020-09-11 14:10:20.420096 7fba3d124700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  requeue_ops 
52620:2020-09-11 14:10:20.631849 7fba4fd58700 20 osd.3 2227 11.4 heartbeat_peers 2,3
53389:2020-09-11 14:10:20.745327 7fba4f557700 25 osd.3 2227  sending 11.4 2227:1339
53689:2020-09-11 14:10:21.017381 7fba49ec6700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
53917:2020-09-11 14:10:21.021443 7fba49ec6700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
53919:2020-09-11 14:10:21.021465 7fba49ec6700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] null
54472:2020-09-11 14:10:21.026500 7fba49ec6700 20 osd.3 2228 11.4 heartbeat_peers 2,3
54757:2020-09-11 14:10:21.028048 7fba3d925700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
54759:2020-09-11 14:10:21.028061 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_advance_map [3,2]/[3,2] -- 3/3
54761:2020-09-11 14:10:21.028073 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering>: Peering advmap
54762:2020-09-11 14:10:21.028081 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] adjust_need_up_thru now 2227, need_up_thru now false
54763:2020-09-11 14:10:21.028087 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started>: Started advmap
54764:2020-09-11 14:10:21.028095 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] check_recovery_sources no source osds () went down
54765:2020-09-11 14:10:21.028104 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_activate_map 
54766:2020-09-11 14:10:21.028112 7fba3d925700  7 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary>: handle ActMap primary
54767:2020-09-11 14:10:21.028120 7fba3d925700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2227: no change since 2020-09-11 14:10:20.031240
54768:2020-09-11 14:10:21.028129 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] take_waiters
54769:2020-09-11 14:10:21.028136 7fba3d925700  5 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/WaitUpThru 0.996851 3 0.000229
54770:2020-09-11 14:10:21.028145 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering>: Leaving Peering
54771:2020-09-11 14:10:21.028152 7fba3d925700  5 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering 1.011890 0 0.000000
54772:2020-09-11 14:10:21.028162 7fba3d925700  5 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started/Primary/Active
54773:2020-09-11 14:10:21.028170 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] state<Started/Primary/Active>: In Active, about to call activate
54774:2020-09-11 14:10:21.028179 7fba3d925700 20 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate - purged_snaps [] cached_removed_snaps []
54775:2020-09-11 14:10:21.028186 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate - snap_trimq []
54776:2020-09-11 14:10:21.028193 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate - no missing, moving last_complete 201'1 -> 201'1
54777:2020-09-11 14:10:21.028200 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate peer osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
54778:2020-09-11 14:10:21.028213 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate peer osd.2 sending log((0'0,201'1], crt=201'1)
54780:2020-09-11 14:10:21.028240 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate peer osd.2 11.4( DNE v 201'1 lc 0'0 (0'0,201'1] local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0) missing missing(1)
54781:2020-09-11 14:10:21.028251 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] needs_recovery osd.2 has 1 missing
54782:2020-09-11 14:10:21.028258 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] add_batch_sources_info: adding sources in batch 1
54783:2020-09-11 14:10:21.028265 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] build_might_have_unfound
54784:2020-09-11 14:10:21.028274 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] _calc_past_interval_range: already have past intervals back to 2224
54785:2020-09-11 14:10:21.028282 7fba3d925700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] build_might_have_unfound: built 2
54786:2020-09-11 14:10:21.028288 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 degraded] activate - starting recovery
54787:2020-09-11 14:10:21.028299 7fba3d925700 10 osd.3 2228 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 degraded]
54789:2020-09-11 14:10:21.028307 7fba3d925700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] publish_stats_to_osd 2228:1340
54790:2020-09-11 14:10:21.028315 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] state<Started/Primary/Active>: Activate Finished
54791:2020-09-11 14:10:21.028322 7fba3d925700  5 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] enter Started/Primary/Active/Activating
54792:2020-09-11 14:10:21.028330 7fba3d925700 20 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] handle_activate_map: Not dirtying info: last_persisted is 2227 while current is 2228
54793:2020-09-11 14:10:21.028337 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] handle_peering_event: epoch_sent: 2228 epoch_requested: 2228 NullEvt
54797:2020-09-11 14:10:21.028438 7fba37919700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] lock
55341:2020-09-11 14:10:21.031547 7fba46937700 25 osd.3 2228  still pending 11.4 2228:1340 > acked 1339,2227
57028:2020-09-11 14:10:21.067607 7fba49ec6700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] lock
57029:2020-09-11 14:10:21.067621 7fba49ec6700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] _activate_committed 2228 peer_activated now 3 last_epoch_started 2224 same_interval_since 2227
57030:2020-09-11 14:10:21.067629 7fba49ec6700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] lock
57031:2020-09-11 14:10:21.067635 7fba49ec6700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] flushed
57048:2020-09-11 14:10:21.067861 7fba3d124700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] lock
57049:2020-09-11 14:10:21.067870 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] handle_peering_event: epoch_sent: 2228 epoch_requested: 2228 FlushedEvt
57050:2020-09-11 14:10:21.067881 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded]  requeue_ops 
60117:2020-09-11 14:10:21.632324 7fba4fd58700 20 osd.3 2228 11.4 heartbeat_peers 2,3
60468:2020-09-11 14:10:21.964346 7fba45134700 20 osd.3 2228 _dispatch 0x7fba69a9dfe0 pg_info(1 pgs e2228:11.4) v4
60469:2020-09-11 14:10:21.964354 7fba45134700  7 osd.3 2228 handle_pg_info pg_info(1 pgs e2228:11.4) v4 from osd.2
60471:2020-09-11 14:10:21.964414 7fba45134700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] lock
60474:2020-09-11 14:10:21.964472 7fba3d124700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] lock
60475:2020-09-11 14:10:21.964509 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] handle_peering_event: epoch_sent: 2228 epoch_requested: 2228 MInfoRec from 2 info: 11.4( v 201'1 lc 0'0 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223)
60476:2020-09-11 14:10:21.964530 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] state<Started/Primary/Active>:  peer osd.2 activated and committed
60477:2020-09-11 14:10:21.964549 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.028307
60478:2020-09-11 14:10:21.964575 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] all_activated_and_committed
60480:2020-09-11 14:10:21.964635 7fba3d124700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] lock
60481:2020-09-11 14:10:21.964670 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] handle_peering_event: epoch_sent: 2228 epoch_requested: 2228 AllReplicasActivated
60482:2020-09-11 14:10:21.964685 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] share_pg_info
60484:2020-09-11 14:10:21.964747 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] publish_stats_to_osd 2228:1341
60485:2020-09-11 14:10:21.964771 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] check_local
60486:2020-09-11 14:10:21.964782 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded]  requeue_ops 
60487:2020-09-11 14:10:21.964794 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] needs_recovery osd.2 has 1 missing
60488:2020-09-11 14:10:21.964805 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] activate not all replicas are up-to-date, queueing recovery
60489:2020-09-11 14:10:21.964828 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.964740
60490:2020-09-11 14:10:21.964867 7fba3d124700 20 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] hit_set_clear
60491:2020-09-11 14:10:21.964881 7fba3d124700 20 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] agent_stop
60493:2020-09-11 14:10:21.965001 7fba3d124700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] lock
60494:2020-09-11 14:10:21.965026 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] handle_peering_event: epoch_sent: 2228 epoch_requested: 2228 DoRecovery
60495:2020-09-11 14:10:21.965041 7fba3d124700  5 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] exit Started/Primary/Active/Activating 0.936718 5 0.000532
60496:2020-09-11 14:10:21.965057 7fba3d124700  5 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] enter Started/Primary/Active/WaitLocalRecoveryReserved
60497:2020-09-11 14:10:21.965075 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] publish_stats_to_osd 2228:1342
60621:2020-09-11 14:10:22.049880 7fba49ec6700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
60891:2020-09-11 14:10:22.051626 7fba49ec6700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
60892:2020-09-11 14:10:22.051634 7fba49ec6700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] null
61652:2020-09-11 14:10:22.055162 7fba3d925700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
61659:2020-09-11 14:10:22.055181 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_advance_map [3,2]/[3,2] -- 3/3
61664:2020-09-11 14:10:22.055197 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active advmap
61667:2020-09-11 14:10:22.055208 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started>: Started advmap
61670:2020-09-11 14:10:22.055219 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] check_recovery_sources no source osds (3) went down
61675:2020-09-11 14:10:22.055233 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map 
61680:2020-09-11 14:10:22.055248 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active: handling ActMap
61686:2020-09-11 14:10:22.055273 7fba3d925700 10 osd.3 2229 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
61690:2020-09-11 14:10:22.055286 7fba3d925700  7 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary>: handle ActMap primary
61695:2020-09-11 14:10:22.055305 7fba3d925700 15 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.965071
61699:2020-09-11 14:10:22.055323 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] take_waiters
61703:2020-09-11 14:10:22.055333 7fba3d925700 20 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map: Not dirtying info: last_persisted is 2228 while current is 2229
61706:2020-09-11 14:10:22.055344 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2229 epoch_requested: 2229 NullEvt
61849:2020-09-11 14:10:22.055821 7fba37919700 30 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
61851:2020-09-11 14:10:22.055840 7fba37919700 10 osd.3 2229 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
61855:2020-09-11 14:10:22.055850 7fba37919700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] recovery raced and were queued twice, ignoring!
61859:2020-09-11 14:10:22.055862 7fba37919700 10 osd.3 2229 do_recovery started 0/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
63796:2020-09-11 14:10:22.633040 7fba4fd58700 20 osd.3 2229 11.4 heartbeat_peers 2,3
64613:2020-09-11 14:10:23.233528 7fba49ec6700 30 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
64889:2020-09-11 14:10:23.236332 7fba49ec6700 30 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
64891:2020-09-11 14:10:23.236346 7fba49ec6700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] null
65325:2020-09-11 14:10:23.239004 7fba3d925700 30 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
65329:2020-09-11 14:10:23.239029 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_advance_map [3,2]/[3,2] -- 3/3
65331:2020-09-11 14:10:23.239052 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active advmap
65334:2020-09-11 14:10:23.239067 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started>: Started advmap
65337:2020-09-11 14:10:23.239085 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] check_recovery_sources no source osds (3) went down
65340:2020-09-11 14:10:23.239104 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map 
65343:2020-09-11 14:10:23.239121 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active: handling ActMap
65346:2020-09-11 14:10:23.239141 7fba3d925700 10 osd.3 2230 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
65348:2020-09-11 14:10:23.239155 7fba3d925700  7 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary>: handle ActMap primary
65353:2020-09-11 14:10:23.239177 7fba3d925700 15 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.965071
65356:2020-09-11 14:10:23.239201 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] take_waiters
65360:2020-09-11 14:10:23.239214 7fba3d925700 20 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map: Not dirtying info: last_persisted is 2228 while current is 2230
65362:2020-09-11 14:10:23.239229 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2230 epoch_requested: 2230 NullEvt
65624:2020-09-11 14:10:23.242068 7fba37919700 30 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
65625:2020-09-11 14:10:23.242099 7fba37919700 10 osd.3 2230 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
65626:2020-09-11 14:10:23.242111 7fba37919700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] recovery raced and were queued twice, ignoring!
65627:2020-09-11 14:10:23.242124 7fba37919700 10 osd.3 2230 do_recovery started 0/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
66155:2020-09-11 14:10:23.245392 7fba49ec6700 20 osd.3 2230 11.4 heartbeat_peers 2,3
68073:2020-09-11 14:10:24.319966 7fba49ec6700 30 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
68718:2020-09-11 14:10:24.323051 7fba49ec6700 30 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
68726:2020-09-11 14:10:24.323069 7fba49ec6700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] null
68741:2020-09-11 14:10:24.323112 7fba3d124700 30 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
68748:2020-09-11 14:10:24.323142 7fba3d124700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_advance_map [3,2]/[3,2] -- 3/3
68752:2020-09-11 14:10:24.323158 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active advmap
68755:2020-09-11 14:10:24.323166 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started>: Started advmap
68758:2020-09-11 14:10:24.323174 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] check_recovery_sources no source osds (3) went down
68760:2020-09-11 14:10:24.323183 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map 
68764:2020-09-11 14:10:24.323189 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active: handling ActMap
68766:2020-09-11 14:10:24.323199 7fba3d124700 10 osd.3 2231 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
68769:2020-09-11 14:10:24.323204 7fba3d124700  7 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary>: handle ActMap primary
68772:2020-09-11 14:10:24.323212 7fba3d124700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.965071
68775:2020-09-11 14:10:24.323220 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] take_waiters
68777:2020-09-11 14:10:24.323226 7fba3d124700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map: Not dirtying info: last_persisted is 2228 while current is 2231
68778:2020-09-11 14:10:24.323232 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2231 epoch_requested: 2231 NullEvt
71105:2020-09-11 14:10:25.746633 7fba4f557700 25 osd.3 2231  sending 11.4 2228:1342
71198:2020-09-11 14:10:25.789587 7fba37919700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
71199:2020-09-11 14:10:25.789593 7fba37919700 10 osd.3 2231 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
71200:2020-09-11 14:10:25.789598 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] recovery raced and were queued twice, ignoring!
71201:2020-09-11 14:10:25.789602 7fba37919700 10 osd.3 2231 do_recovery started 0/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
71309:2020-09-11 14:10:25.797456 7fba35915700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
71315:2020-09-11 14:10:25.797495 7fba3d124700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
71317:2020-09-11 14:10:25.797531 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2228 epoch_requested: 2228 LocalRecoveryReserved
71318:2020-09-11 14:10:25.797542 7fba3d124700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] exit Started/Primary/Active/WaitLocalRecoveryReserved 3.832485 10 0.000382
71320:2020-09-11 14:10:25.797552 7fba3d124700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] enter Started/Primary/Active/WaitRemoteRecoveryReserved
71375:2020-09-11 14:10:26.249869 7fba46937700 25 osd.3 2231  ack on 11.4 2228:1342
71484:2020-09-11 14:10:26.635094 7fba4fd58700 20 osd.3 2231 11.4 heartbeat_peers 2,3
94065:2020-09-11 14:13:14.601725 7fba45134700 20 osd.3 2231 _dispatch 0x7fba6c134b40 MRecoveryReserve GRANT  pgid: 11.4, query_epoch: 2231 v2
94067:2020-09-11 14:13:14.601753 7fba45134700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
94070:2020-09-11 14:13:14.601819 7fba3d124700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
94071:2020-09-11 14:13:14.601855 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2231 epoch_requested: 2231 RemoteRecoveryReserved
94072:2020-09-11 14:13:14.601876 7fba3d124700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] exit Started/Primary/Active/WaitRemoteRecoveryReserved 168.804323 1 0.000031
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

### 1.1 收到新的osdmap
在将osd0踢出osdmap之后，osd3会接收到这一变化，从而触发新的Peering进程。此时osd3接收到```e2226```版本的osdmap:

{% highlight string %}
2020-09-11 14:10:18.954484 7fba46937700 20 osd.3 2225 _dispatch 0x7fba6e721180 osd_map(2226..2226 src has 1514..2226) v3
2020-09-11 14:10:18.954505 7fba46937700  3 osd.3 2225 handle_osd_map epochs [2226,2226], i have 2226, src has [1514,2226]
{% endhighlight %}

然后调用如下函数来进行处理：
{% highlight string %}
void OSD::handle_osd_map(MOSDMap *m)
{
	...

	store->queue_transaction(
		service.meta_osr.get(),
		std::move(t),
		new C_OnMapApply(&service, pinned_maps, last),
		new C_OnMapCommit(this, start, last, m), 0);

	...
}

void OSD::_committed_osd_maps(epoch_t first, epoch_t last, MOSDMap *m)
{
	...
	
	// yay!
	consume_map();

	...
}

void OSD::consume_map()
{
	...

	// scan pg's
	{
		RWLock::RLocker l(pg_map_lock);
		for (ceph::unordered_map<spg_t,PG*>::iterator it = pg_map.begin();it != pg_map.end();++it) {
			PG *pg = it->second;
			pg->lock();
			pg->queue_null(osdmap->get_epoch(), osdmap->get_epoch());
			pg->unlock();
		}
		
		logger->set(l_osd_pg, pg_map.size());
	}
	...
}
{% endhighlight %}

程序运行到这里，就构造了一个NullEvt到OSD的消息队列，从而触发相应的Peering流程。


## 2. 进入Peering流程
进入Peering流程的第一个处理函数为：
{% highlight string %}
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	bool need_up_thru = false;
	epoch_t same_interval_since = 0;
	OSDMapRef curmap = service.get_osdmap();
	PG::RecoveryCtx rctx = create_context();
	rctx.handle = &handle;

	for (list<PG*>::const_iterator i = pgs.begin();i != pgs.end();++i) {
		set<boost::intrusive_ptr<PG> > split_pgs;
		PG *pg = *i;
		pg->lock_suspend_timeout(handle);
		curmap = service.get_osdmap();

		if (pg->deleting) {
			pg->unlock();
			continue;
		}

		if (!advance_pg(curmap->get_epoch(), pg, handle, &rctx, &split_pgs)) {
			// we need to requeue the PG explicitly since we didn't actually
			// handle an event
			peering_wq.queue(pg);
		} else {
			assert(!pg->peering_queue.empty());
			PG::CephPeeringEvtRef evt = pg->peering_queue.front();
			pg->peering_queue.pop_front();
			pg->handle_peering_event(evt, &rctx);
		}

		need_up_thru = pg->need_up_thru || need_up_thru;
		same_interval_since = MAX(pg->info.history.same_interval_since,
		same_interval_since);
		pg->write_if_dirty(*rctx.transaction);

		if (!split_pgs.empty()) {
			rctx.on_applied->add(new C_CompleteSplits(this, split_pgs));
			split_pgs.clear();
		}
		dispatch_context_transaction(rctx, pg, &handle);
		pg->unlock();
		handle.reset_tp_timeout();
	}
	if (need_up_thru)
		queue_want_up_thru(same_interval_since);

	dispatch_context(rctx, 0, curmap, &handle);
	
	service.send_pg_temp();
}
{% endhighlight %}

在上面的OSD::process_peering_events()函数中，遍历该OSD上的每一个PG：调用OSD::advance_pg()来进行osdmap追赶。如果OSD::advance_pg()返回值为true，表明当前已经追赶上最新的osdmap，已经可以处理peering event了；否则，表明当前还没有追赶上，因此会将该pg重新放入peering_wq中。

{% highlight string %}
bool OSD::advance_pg(
  epoch_t osd_epoch, PG *pg,
  ThreadPool::TPHandle &handle,
  PG::RecoveryCtx *rctx,
  set<boost::intrusive_ptr<PG> > *new_pgs)
{
	...

	for (;next_epoch <= osd_epoch && next_epoch <= max;++next_epoch) {
		...

		nextmap->pg_to_up_acting_osds(
			pg->info.pgid.pgid,
			&newup, &up_primary,
			&newacting, &acting_primary);

		pg->handle_advance_map(
			nextmap, lastmap, newup, up_primary,
			newacting, acting_primary, rctx);

		...
	}

	service.pg_update_epoch(pg->info.pgid, lastmap->get_epoch());
	pg->handle_activate_map(rctx);

	...
}
{% endhighlight %}
在这里针对PG 11.4而言，当前osd3的osdmap版本为e2226，而pg 11.4的osdmap版本为e2225，因此会执行上面的for循环。

1） **函数pg_to_up_acting_osds()**
{% highlight string %}
void pg_to_up_acting_osds(pg_t pg, vector<int> *up, int *up_primary,
            vector<int> *acting, int *acting_primary) const {
	_pg_to_up_acting_osds(pg, up, up_primary, acting, acting_primary);
}
void OSDMap::_pg_to_up_acting_osds(const pg_t& pg, vector<int> *up, int *up_primary,
                                   vector<int> *acting, int *acting_primary) const{
}
{% endhighlight %}
函数OSDMap::pg_to_up_acting_osds()根据osdmap来计算指定PG所映射到的osd。这里针对```PG 11.4```而言，计算出的newup为[3,2]， newacting为[3].

2） **函数handle_advance_map()**
{% highlight string %}
void PG::handle_advance_map(
  OSDMapRef osdmap, OSDMapRef lastmap,
  vector<int>& newup, int up_primary,
  vector<int>& newacting, int acting_primary,
  RecoveryCtx *rctx)
{
	dout(10) << "handle_advance_map "
		<< newup << "/" << newacting
		<< " -- " << up_primary << "/" << acting_primary
		<< dendl;

	update_osdmap_ref(osdmap);

	...
	
	AdvMap evt(
		osdmap, lastmap, newup, up_primary,
		newacting, acting_primary);
	recovery_state.handle_event(evt, rctx);
	...
}
{% endhighlight %}
通过上面第一行的打印信息，我们了解到：```PG11.4```对应的up set为[3,2], acting set也为[3]，up_primary为3，acting_primary也为3。

之后会调用PG::update_osdmap_ref()将PG当前的osdmap进行更新；最后产生一个AdvMap事件，交由recovery_state来进行处理，从而触发peering进程。

3) **函数pg_update_epoch（）**
{% highlight string %}
void pg_update_epoch(spg_t pgid, epoch_t epoch) {
	Mutex::Locker l(pg_epoch_lock);
	map<spg_t,epoch_t>::iterator t = pg_epoch.find(pgid);
	assert(t != pg_epoch.end());
	pg_epochs.erase(pg_epochs.find(t->second));
	t->second = epoch;
	pg_epochs.insert(epoch);
}
{% endhighlight %}
函数pg_update_epoch()更新PG的pg_epoch值

4) **函数handle_activate_map()**
{% highlight string %}
void PG::handle_activate_map(RecoveryCtx *rctx)
{
	dout(10) << "handle_activate_map " << dendl;
	ActMap evt;
	recovery_state.handle_event(evt, rctx);
	if (osdmap_ref->get_epoch() - last_persisted_osdmap_ref->get_epoch() > cct->_conf->osd_pg_epoch_persisted_max_stale) {
		dout(20) << __func__ << ": Dirtying info: last_persisted is "<< last_persisted_osdmap_ref->get_epoch()
			<< " while current is " << osdmap_ref->get_epoch() << dendl;
		dirty_info = true;
	} else {
		dout(20) << __func__ << ": Not dirtying info: last_persisted is "<< last_persisted_osdmap_ref->get_epoch()
			<< " while current is " << osdmap_ref->get_epoch() << dendl;
	}

	if (osdmap_ref->check_new_blacklist_entries()) check_blacklisted_watchers();
}
{% endhighlight %}
函数handle_activate_map()用于激活当前阶段的osdmap，通常是用于触发向Replica发送通知消息，以推动Peering的进程。

>注： 通常来说，AdvMap是同步完成的，而ActMap是异步完成的。

### 2.1 Clean状态对AdvMap事件的处理
由于在当前状态，PG 11.4已经处于clean状态，且osd3对于PG 11.4而言是主OSD，因此接收到AdvMap事件时，处理流程如下：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Active::react(const AdvMap& advmap)
{
	...

	return forward_event();
}

boost::statechart::result PG::RecoveryState::Started::react(const AdvMap& advmap)
{
  dout(10) << "Started advmap" << dendl;
  PG *pg = context< RecoveryMachine >().pg;
  pg->check_full_transition(advmap.lastmap, advmap.osdmap);
  if (pg->should_restart_peering(
	advmap.up_primary,
	advmap.acting_primary,
	advmap.newup,
	advmap.newacting,
	advmap.lastmap,
	advmap.osdmap)) {
    dout(10) << "should_restart_peering, transitioning to Reset" << dendl;
    post_event(advmap);
    return transit< Reset >();
  }
  pg->remove_down_peer_info(advmap.osdmap);
  return discard_event();
}
{% endhighlight %}
从以上代码可以看出，其最终会调用pg->should_restart_peering()来检查是否需要触发新的peering操作。通过以前的代码分析，我们知道只要满足如下条件之一即需要重新peering:

* PG的acting primary发生了改变；

* PG的acting set发生了改变；

* PG的up primary发生了改变；

* PG的up set发生了改变；

* PG的min_size发生了改变（通常是调整了rule规则）；

* PG的副本size值发生了改变；

* PG进行了分裂

* PG的sort bitwise发生了改变

这里针对PG 11.4，其up set发生了变化，因此会触发新的peering状态。此时state_machine进入Reset状态。

### 2.2 进入Reset状态
###### 2.2.1 Reset构造函数
{% highlight string %}
PG::RecoveryState::Reset::Reset(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Reset")
{
	context< RecoveryMachine >().log_enter(state_name);
	PG *pg = context< RecoveryMachine >().pg;
	
	pg->flushes_in_progress = 0;
	pg->set_last_peering_reset();
}
void PG::set_last_peering_reset()
{
  dout(20) << "set_last_peering_reset " << get_osdmap()->get_epoch() << dendl;
  if (last_peering_reset != get_osdmap()->get_epoch()) {
    last_peering_reset = get_osdmap()->get_epoch();
    reset_interval_flush();
  }
}
{% endhighlight %}
在Reset构造函数中，调用pg->set_last_peering_reset将last_peering_reset设置为e2226.

###### 2.2.2 处理Adv事件
在进入Reset状态之前，我们通过post_event()向Reset投递了AdvMap事件，这里我们来看对该事件的处理：
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
{% endhighlight %}
上面调用pg->should_restart_peering()再一次检查是否需要重新peering。然后再调用pg->start_peering_interval()来启动peering流程。

1) **函数generate_past_intervals()**
{% highlight string %}
void PG::generate_past_intervals(){
	....

	if (!_calc_past_interval_range(&cur_epoch, &end_epoch,
		osd->get_superblock().oldest_map)) {
		if (info.history.same_interval_since == 0) {
			info.history.same_interval_since = end_epoch;
			dirty_info = true;
		}
		return;
	}

	...
}
bool PG::_calc_past_interval_range(epoch_t *start, epoch_t *end, epoch_t oldest_map)
{
	if (info.history.same_interval_since) {
		*end = info.history.same_interval_since;
	} else {
		// PG must be imported, so let's calculate the whole range.
		*end = osdmap_ref->get_epoch();
	}
	
	// Do we already have the intervals we want?
	map<epoch_t,pg_interval_t>::const_iterator pif = past_intervals.begin();
	if (pif != past_intervals.end()) {
		if (pif->first <= info.history.last_epoch_clean) {
			dout(10) << __func__ << ": already have past intervals back to "<< info.history.last_epoch_clean << dendl;
			return false;
		}
		*end = past_intervals.begin()->first;
	}
	
	*start = MAX(MAX(info.history.epoch_created,
		info.history.last_epoch_clean),
		oldest_map);
	if (*start >= *end) {
		dout(10) << __func__ << " start epoch " << *start << " >= end epoch " << *end<< ", nothing to do" << dendl;
		return false;
	}
	
	return true;
}

{% endhighlight %}
函数_calc_past_interval_range()用于计算一个past_interval的范围。所谓一个past_interval，是指一个连续的epoch段[epoch_start, epoch_end]，在该epoch段内：

* PG的acting primary保持不变；

* PG的acting set保持不变；

* PG的up primary保持不变；

* PG的up set保持不变；

* PG的min_size保持不变；

* PG的副本size值保持不变；

* PG没有进行分裂；

* PG的sort bitwise保持不变；

函数_calc_past_interval_range()如果返回true，表示成功计算到一个past_interval；如果返回false，则表示该interval已经计算过，不必再计算，或者不是一个有效的past_interval。

结合当前的打印日志消息：
{% highlight string %}
42238:2020-09-11 14:10:18.974114 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3] r=0 lpr=2226 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active] _calc_past_interval_range start epoch 2224 >= end epoch 2223, nothing to do
{% endhighlight %}
当前info.history.same_interval_since的值为e2223，而info.history.last_epoch_clean的值为e2224，因此函数_calc_past_interval_range()的返回值为false，表明不是一个有效的past_interval。

2） **函数start_peering_interval()**
{% highlight string %}
void PG::start_peering_interval(
  const OSDMapRef lastmap,
  const vector<int>& newup, int new_up_primary,
  const vector<int>& newacting, int new_acting_primary,
  ObjectStore::Transaction *t)
{
	...

	set_last_peering_reset();

	...

	init_primary_up_acting(
		newup,
		newacting,
		new_up_primary,
		new_acting_primary);

	....
	if(!lastmap){
		
	}else{
		...
		pg_interval_t::check_new_interval(....);

		dout(10) << __func__ << ": check_new_interval output: "
			<< debug.str() << dendl;
		if (new_interval) {
			dout(10) << " noting past " << past_intervals.rbegin()->second << dendl;
			dirty_info = true;
			dirty_big_info = true;
			info.history.same_interval_since = osdmap->get_epoch();
		}
	}

	...

	dout(10) << " up " << oldup << " -> " << up 
		<< ", acting " << oldacting << " -> " << acting 
		<< ", acting_primary " << old_acting_primary << " -> " << new_acting_primary
		<< ", up_primary " << old_up_primary << " -> " << new_up_primary
		<< ", role " << oldrole << " -> " << role
		<< ", features acting " << acting_features
		<< " upacting " << upacting_features
		<< dendl;
	
	// deactivate.
	state_clear(PG_STATE_ACTIVE);
	state_clear(PG_STATE_PEERED);
	state_clear(PG_STATE_DOWN);
	state_clear(PG_STATE_RECOVERY_WAIT);
	state_clear(PG_STATE_RECOVERING);
	
	peer_purged.clear();
	actingbackfill.clear();
	snap_trim_queued = false;
	scrub_queued = false;
	
	// reset primary state?
	if (was_old_primary || is_primary()) {
		osd->remove_want_pg_temp(info.pgid.pgid);
	}
	clear_primary_state();

	// pg->on_*
	on_change(t);

	assert(!deleting);
	
	// should we tell the primary we are here?
	send_notify = !is_primary();
	
	if (role != oldrole || was_old_primary != is_primary()) {
		// did primary change?
		if (was_old_primary != is_primary()) {
			state_clear(PG_STATE_CLEAN);
			clear_publish_stats();
	
			// take replay queue waiters
			list<OpRequestRef> ls;

			for (map<eversion_t,OpRequestRef>::iterator it = replay_queue.begin();it != replay_queue.end();++it)
				ls.push_back(it->second);

			replay_queue.clear();
			requeue_ops(ls);
		}
	
		on_role_change();
	
		// take active waiters
		requeue_ops(waiting_for_peered);
	
	} else {
		....
	}

	cancel_recovery();
	....
}
{% endhighlight %}
在收到新的osdmap，调用advance_map()，然后触发新的Peering之前，会调用PG::start_peering_interval()，完成相关状态的重新设置。

* set_last_peering_reset()用于清空上一次的peering数据；

* init_primary_up_acting()用于设置当前新的acting set以及up set

* 设置info.stats的up set、acting set的值

* 将当前PG的状态设置为REMAPPED
{% highlight string %}
// This will now be remapped during a backfill in cases
// that it would not have been before.
if (up != acting)
	state_set(PG_STATE_REMAPPED);
else
	state_clear(PG_STATE_REMAPPED);
{% endhighlight %}
由于当前PG的up set的值为[3,2]，acting set的值为[3]，因此这里设置PG的状态为```REMAPPED```

* 计算PG 11.4中OSD3的角色
{% highlight string %}
int OSDMap::calc_pg_rank(int osd, const vector<int>& acting, int nrep)
{
	if (!nrep)
		nrep = acting.size();
	for (int i=0; i<nrep; i++) 
		if (acting[i] == osd)
			return i;
	return -1;
}

int OSDMap::calc_pg_role(int osd, const vector<int>& acting, int nrep)
{
	if (!nrep)
		nrep = acting.size();
	return calc_pg_rank(osd, acting, nrep);
}
{% endhighlight %}
这里osd3的角色是primary。

* check_new_interval(): 用于检查是否是一个新的interval。如果是新interval，则计算出该新的past_interval：

> 42242:2020-09-11 14:10:18.974135 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2223/2223/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 luod=0'0 crt=201'1 lcod 0'0 mlcod 0'0 active+remapped] start_peering_interval: check_new_interval output: generate_past_intervals interval(2223-2225 up [3](3) acting [3](3)): not rw, up_thru 2223 up_from 2123 last_epoch_clean 2224

当前的PG osdmap的epoch为e2226，当前的up set为[3,2], acting set为[3]，计算出的一个past interval为[e2223,e2225]，在此一interval期间，up set为[3], acting set也为[3]，up_thru为e2223, up_from为e2123, last_epoch_clean为e2224.

* 清除PG的相关状态。关于PG的状态是由一个```unsigned```类型的整数来表示的
{% highlight string %}
/*
 * pg states
 */
#define PG_STATE_CREATING     (1<<0)  				// creating
#define PG_STATE_ACTIVE       (1<<1)  				// i am active.  (primary: replicas too)
#define PG_STATE_CLEAN        (1<<2)  				// peers are complete, clean of stray replicas.
#define PG_STATE_DOWN         (1<<4) 				 // a needed replica is down, PG offline
#define PG_STATE_REPLAY       (1<<5) 				 // crashed, waiting for replay
//#define PG_STATE_STRAY      (1<<6)  				// i must notify the primary i exist.
#define PG_STATE_SPLITTING    (1<<7)  				// i am splitting
#define PG_STATE_SCRUBBING    (1<<8)  				// scrubbing
#define PG_STATE_SCRUBQ       (1<<9)  				// queued for scrub
#define PG_STATE_DEGRADED     (1<<10) 				// pg contains objects with reduced redundancy
#define PG_STATE_INCONSISTENT (1<<11) 				// pg replicas are inconsistent (but shouldn't be)
#define PG_STATE_PEERING      (1<<12)				// pg is (re)peering
#define PG_STATE_REPAIR       (1<<13) 				// pg should repair on next scrub
#define PG_STATE_RECOVERING   (1<<14) 				// pg is recovering/migrating objects
#define PG_STATE_BACKFILL_WAIT     (1<<15) 			// [active] reserving backfill
#define PG_STATE_INCOMPLETE   (1<<16) 				// incomplete content, peering failed.
#define PG_STATE_STALE        (1<<17) 				// our state for this pg is stale, unknown.
#define PG_STATE_REMAPPED     (1<<18) 				// pg is explicitly remapped to different OSDs than CRUSH
#define PG_STATE_DEEP_SCRUB   (1<<19) 				// deep scrub: check CRC32 on files
#define PG_STATE_BACKFILL  (1<<20) 					// [active] backfilling pg content
#define PG_STATE_BACKFILL_TOOFULL (1<<21) 			// backfill can't proceed: too full
#define PG_STATE_RECOVERY_WAIT (1<<22) 				// waiting for recovery reservations
#define PG_STATE_UNDERSIZED    (1<<23) 				// pg acting < pool size
#define PG_STATE_ACTIVATING   (1<<24) 				// pg is peered but not yet active
#define PG_STATE_PEERED        (1<<25) 				// peered, cannot go active, can recover
#define PG_STATE_SNAPTRIM      (1<<26) 				// trimming snaps
#define PG_STATE_SNAPTRIM_WAIT (1<<27) 				// queued to trim snaps
{% endhighlight %}
这里将```active```状态清除了，因此变为```inactive```状态。

* 情况primary的peering状态
{% highlight string %}
if (was_old_primary || is_primary()) {
	osd->remove_want_pg_temp(info.pgid.pgid);
}
clear_primary_state();
{% endhighlight %}

* PG 11.4所在OSD3的角色未发生变化，仍然为primary，因此有如下输出
{% highlight string %}
42263:2020-09-11 14:10:18.974231 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] [3] -> [3], replicas changed
{% endhighlight %}


* cancel_recovery(): 取消PG当前正在进行中的recovery动作；

到此为止，由OSD::advance_pg()函数中pg->handle_advance_map()所触发的AdvMap事件就已经处理完成。

>AdvMap事件主要作用是： 触发检查是否需要重新Peering，如果需要，并完成Peering的相关初始化操作。


###### 2.2.3 Reset状态下对ActMap事件的处理
这里针对PG 11.4而言，通过AdvMap事件完成了Peering的相关检查操作。然后在OSD::advance_pg()函数中调用PG::handle_activate_map()来激活该OSDMap。这里是对该ActMap事件的处理，参看如下日志片段：
{% highlight string %}
42267:2020-09-11 14:10:18.974247 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] handle_activate_map 
42268:2020-09-11 14:10:18.974251 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] update_heartbeat_peers 3 -> 2,3
42270:2020-09-11 14:10:18.974256 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] take_waiters
42271:2020-09-11 14:10:18.974260 7fba3d925700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit Reset 0.000165 1 0.000199
{% endhighlight %}

如下是相关的处理函数：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Reset::react(const ActMap&)
{
  PG *pg = context< RecoveryMachine >().pg;
  if (pg->should_send_notify() && pg->get_primary().osd >= 0) {
    context< RecoveryMachine >().send_notify(
      pg->get_primary(),
      pg_notify_t(
	pg->get_primary().shard, pg->pg_whoami.shard,
	pg->get_osdmap()->get_epoch(),
	pg->get_osdmap()->get_epoch(),
	pg->info),
      pg->past_intervals);
  }

  pg->update_heartbeat_peers();
  pg->take_waiters();

  return transit< Started >();
}
{% endhighlight %}
通常在osdmap发送变动，从而引发PG的acting set、up set发生变动，对于一个PG的副本OSD通常会发送一个通知消息到PG的Primary OSD。这里由于osd3已经是PG11.4的primary OSD，因此这里不需要发送通知消息。

>注： 关于send_notify变量的设置，是在Initial::react(const Load&)函数中。另外send_notify()最终的发送函数为OSD::do_notifies()



### 2.3 进入Started状态
{% highlight string %}
PG::RecoveryState::Started::Started(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started")
{
	context< RecoveryMachine >().log_enter(state_name);
}
{% endhighlight %}

###### 2.3.1 进入Started/Start状态
```Started```的默认初始子状态为```Start```状态：
{% highlight string %}
PG::RecoveryState::Start::Start(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Start")
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	if (pg->is_primary()) {
		dout(1) << "transitioning to Primary" << dendl;
		post_event(MakePrimary());
	} else { //is_stray
		dout(1) << "transitioning to Stray" << dendl; 
		post_event(MakeStray());
	}
}
{% endhighlight %}
这里osd3对于PG 11.4而言为主OSD，因此产生MakePrimary()事件，进入Started/Primary状态。

###### 2.3.2 进入Started/Primary状态
{% highlight string %}
PG::RecoveryState::Primary::Primary(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary")
{
	context< RecoveryMachine >().log_enter(state_name);
	PG *pg = context< RecoveryMachine >().pg;
	assert(pg->want_acting.empty());
	
	// set CREATING bit until we have peered for the first time.
	if (pg->info.history.last_epoch_started == 0) {
		pg->state_set(PG_STATE_CREATING);
		// use the history timestamp, which ultimately comes from the
		// monitor in the create case.
		utime_t t = pg->info.history.last_scrub_stamp;
		pg->info.stats.last_fresh = t;
		pg->info.stats.last_active = t;
		pg->info.stats.last_change = t;
		pg->info.stats.last_peered = t;
		pg->info.stats.last_clean = t;
		pg->info.stats.last_unstale = t;
		pg->info.stats.last_undegraded = t;
		pg->info.stats.last_fullsized = t;
		pg->info.stats.last_scrub_stamp = t;
		pg->info.stats.last_deep_scrub_stamp = t;
		pg->info.stats.last_clean_scrub_stamp = t;
	}
}
{% endhighlight %}

### 2.4 进入Peering状态
Started/Primary的默认初始子状态为Started/Primary/Peering，我们来看如下Peering的构造函数：
{% highlight string %}
PG::RecoveryState::Peering::Peering(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Peering"),
    history_les_bound(false)
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	assert(!pg->is_peered());
	assert(!pg->is_peering());
	assert(pg->is_primary());
	pg->state_set(PG_STATE_PEERING);
}
{% endhighlight %}
进入Peering状态后，会将PG的状态设置为```Peering```。然后直接进入Peering的默认初始子状态GetInfo.

###### 2.4.1 进入Peering/GetInfo状态
{% highlight string %}
/*--------GetInfo---------*/
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

进入GetInfo状态后，首先调用generate_past_intervals()计算past_intervals。这里我们在前面Reset阶段就已经计算出了新的past_interval为[2223,2225]。从如下日志片段中也能看出：
{% highlight string %}
42279:2020-09-11 14:10:18.974292 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] _calc_past_interval_range: already have past intervals back to 2224
{% endhighlight %}

1) **函数PG::build_prior()**
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
		dout(10) << "up_thru " << get_osdmap()->get_up_thru(osd->whoami)
			<< " < same_since " << info.history.same_interval_since
			<< ", must notify monitor" << dendl;
			need_up_thru = true;
	} else {
		dout(10) << "up_thru " << get_osdmap()->get_up_thru(osd->whoami)
			<< " >= same_since " << info.history.same_interval_since
			<< ", all is well" << dendl;
		need_up_thru = false;
	}
	set_probe_targets(prior_set->probe);
}
{% endhighlight %}
关于PriorSet，我们这里不做介绍，后面我们会再开辟专门的章节来进行说明(参见up_thru)。这里我们给出针对PG 11.4所构建的PriorSet的结果：
{% highlight string %}
42280:2020-09-11 14:10:18.974297 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering]  PriorSet: build_prior interval(2223-2225 up [3](3) acting [3](3) maybe_went_rw)
42281:2020-09-11 14:10:18.974301 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering]  PriorSet: build_prior final: probe 2,3 down  blocked_by {}
42282:2020-09-11 14:10:18.974305 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] up_thru 2223 < same_since 2226, must notify monitor
{% endhighlight %}
当前osdmap的epoch为e2226，对于osd3来说其上一次申请的up_thru为e2223，而当前的same_interval_since为e2226，因此这里need_up_thru需要设置为true。

2) **函数PG::get_infos()**
{% highlight string %}
void PG::RecoveryState::GetInfo::get_infos()
{
	PG *pg = context< RecoveryMachine >().pg;
	unique_ptr<PriorSet> &prior_set = context< Peering >().prior_set;
	
	pg->blocked_by.clear();
	for (set<pg_shard_t>::const_iterator it = prior_set->probe.begin();it != prior_set->probe.end();++it) {
		pg_shard_t peer = *it;
		if (peer == pg->pg_whoami) {
			continue;
		}
		if (pg->peer_info.count(peer)) {
			dout(10) << " have osd." << peer << " info " << pg->peer_info[peer] << dendl;
			continue;
		}

		if (peer_info_requested.count(peer)) {
			dout(10) << " already requested info from osd." << peer << dendl;
			pg->blocked_by.insert(peer.osd);
		} else if (!pg->get_osdmap()->is_up(peer.osd)) {

			dout(10) << " not querying info from down osd." << peer << dendl;

		} else {
			dout(10) << " querying info from osd." << peer << dendl;
			context< RecoveryMachine >().send_query(
				peer, pg_query_t(pg_query_t::INFO,
				it->shard, pg->pg_whoami.shard,
				pg->info.history,
				pg->get_osdmap()->get_epoch()));

			peer_info_requested.insert(peer);
			pg->blocked_by.insert(peer.osd);
		}
	}
	
	pg->publish_stats_to_osd();
}


//位于src/osd/pg.h的RecoveryMachine中
void send_query(pg_shard_t to, const pg_query_t &query) {
	assert(state->rctx);
	assert(state->rctx->query_map);
	(*state->rctx->query_map)[to.osd][spg_t(pg->info.pgid.pgid, to.shard)] = query;
}
{% endhighlight %}
这里由上面构造的PriorSet为：
{% highlight string %}
42280:2020-09-11 14:10:18.974297 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering]  PriorSet: build_prior interval(2223-2225 up [3](3) acting [3](3) maybe_went_rw)
42281:2020-09-11 14:10:18.974301 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering]  PriorSet: build_prior final: probe 2,3 down  blocked_by {}
{% endhighlight %}
GetInfo()是由PG的主OSD向从OSD发送pg_query_t::INFO消息以获取从OSD的pg_into_t信息。这里对于PG11.4而言，因为目前其prior_set->probe为[2，3]。因此这里会向osd2发出pg_query_t::INFO请求。这里我们还是讲述一下GetInfo究竟是要获取哪些info信息，下面我们先来看一下pg_query_t结构体的定义：
{% highlight string %}
/** 
 * pg_query_t - used to ask a peer for information about a pg.
 *
 * note: if version=0, type=LOG, then we just provide our full log.
 */
struct pg_query_t {
	enum {
		INFO = 0,
		LOG = 1,
		MISSING = 4,
		FULLLOG = 5,
	};
	
	__s32 type;
	eversion_t since;
	pg_history_t history;
	epoch_t epoch_sent;
	shard_id_t to;
	shard_id_t from;
};
{% endhighlight %}

如下则是pg_info_t结构体的定义(src/osd/osd_types.h)：
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
	spg_t pgid;
	eversion_t last_update;                     ///< last object version applied to store.
	eversion_t last_complete;                  ///< last version pg was complete through.
	epoch_t last_epoch_started;                ///< last epoch at which this pg started on this osd
	
	version_t last_user_version;               ///< last user object version applied to store
	
	eversion_t log_tail;                       ///< oldest log entry.
	
	hobject_t last_backfill;                   ///< objects >= this and < last_complete may be missing
	bool last_backfill_bitwise;                ///< true if last_backfill reflects a bitwise (vs nibblewise) sort
	
	interval_set<snapid_t> purged_snaps;
	
	pg_stat_t stats;
	
	pg_history_t history;
	pg_hit_set_history_t hit_set;
};
{% endhighlight %}

上面send_query()函数将要查询的pg_query_t::INFO放入RecoveryMachine的state中，然后会由OSD::process_peering_events()中的dispatch_context()来将消息发送出去：
{% highlight string %}
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...
	
	dispatch_context(rctx, 0, curmap, &handle);

	...
}

void OSD::dispatch_context(PG::RecoveryCtx &ctx, PG *pg, OSDMapRef curmap,
                           ThreadPool::TPHandle *handle)
{
	if (service.get_osdmap()->is_up(whoami) && is_active()) {

		do_notifies(*ctx.notify_list, curmap);
		do_queries(*ctx.query_map, curmap);
		do_infos(*ctx.info_map, curmap);

	}
}

/** do_queries
 * send out pending queries for info | summaries
 */
void OSD::do_queries(map<int, map<spg_t,pg_query_t> >& query_map,
		     OSDMapRef curmap)
{
	for (map<int, map<spg_t,pg_query_t> >::iterator pit = query_map.begin();pit != query_map.end();++pit) {
		if (!curmap->is_up(pit->first)) {
			dout(20) << __func__ << " skipping down osd." << pit->first << dendl;
			continue;
		}
		int who = pit->first;
		ConnectionRef con = service.get_con_osd_cluster(who, curmap->get_epoch());

		if (!con) {
			dout(20) << __func__ << " skipping osd." << who
				<< " (NULL con)" << dendl;
			continue;
		}
		service.share_map_peer(who, con.get(), curmap);
		dout(7) << __func__ << " querying osd." << who
			<< " on " << pit->second.size() << " PGs" << dendl;

		MOSDPGQuery *m = new MOSDPGQuery(curmap->get_epoch(), pit->second);
		con->send_message(m);
	}
}
{% endhighlight %}


----------

接着在从OSD接收到MSG_OSD_PG_QUERY消息后：
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
		
		...
	}
}

/** PGQuery
 * from primary to replica | stray
 * NOTE: called with opqueue active.
 */
void OSD::handle_pg_query(OpRequestRef op)
{
	....

	map< int, vector<pair<pg_notify_t, pg_interval_map_t> > > notify_list;

	for (map<spg_t,pg_query_t>::iterator it = m->pg_list.begin();it != m->pg_list.end();++it) {
	
		...

		
		//处理在pg_map中找到了的PG的query请求		
		{
			RWLock::RLocker l(pg_map_lock);
			if (pg_map.count(pgid)) {
				PG *pg = 0;
				pg = _lookup_lock_pg_with_map_lock_held(pgid);
				pg->queue_query(
					it->second.epoch_sent, it->second.epoch_sent,
					pg_shard_t(from, it->second.from), it->second);
				pg->unlock();
				continue;
			}
		}

		//处理在PG_map中未找到的PG的query请求
	}

	do_notifies(notify_list, osdmap);
}

void PG::queue_query(epoch_t msg_epoch,
		     epoch_t query_epoch,
		     pg_shard_t from, const pg_query_t& q)
{
	dout(10) << "handle_query " << q << " from replica " << from << dendl;
	queue_peering_event(
      CephPeeringEvtRef(std::make_shared<CephPeeringEvt>(msg_epoch, query_epoch,
					 MQuery(from, q, query_epoch))));
}

void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )	
{
	...

	PG::CephPeeringEvtRef evt = pg->peering_queue.front();
	pg->peering_queue.pop_front();
	pg->handle_peering_event(evt, &rctx);

	...
}

void PG::handle_peering_event(CephPeeringEvtRef evt, RecoveryCtx *rctx)
{
	dout(10) << "handle_peering_event: " << evt->get_desc() << dendl;
	if (!have_same_or_newer_map(evt->get_epoch_sent())) {
		dout(10) << "deferring event " << evt->get_desc() << dendl;
		peering_waiters.push_back(evt);
		return;
	}

	if (old_peering_evt(evt))
		return;
	recovery_state.handle_event(evt, rctx);
}


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
	}

	...
}

/** do_notifies
 * Send an MOSDPGNotify to a primary, with a list of PGs that I have
 * content for, and they are primary for.
 */

void OSD::do_notifies(
  map<int,vector<pair<pg_notify_t,pg_interval_map_t> > >& notify_list,
  OSDMapRef curmap)
{
	for (map<int,vector<pair<pg_notify_t,pg_interval_map_t> > >::iterator it =notify_list.begin();it != notify_list.end();++it) {
		if (!curmap->is_up(it->first)) {
			dout(20) << __func__ << " skipping down osd." << it->first << dendl;
			continue;
		}

		ConnectionRef con = service.get_con_osd_cluster(it->first, curmap->get_epoch());
		if (!con) {
			dout(20) << __func__ << " skipping osd." << it->first<< " (NULL con)" << dendl;
			continue;
		}
		service.share_map_peer(it->first, con.get(), curmap);
		dout(7) << __func__ << " osd " << it->first << " on " << it->second.size() << " PGs" << dendl;
		MOSDPGNotify *m = new MOSDPGNotify(curmap->get_epoch(),it->second);
		con->send_message(m);
	}
}
{% endhighlight %}
从上面我们可以看到查询pg_query_t::INFO时返回的是一个pg_notify_t类型的包装，其封装了pg_info_t数据结构。

>注： 从上面OSD::handle_pg_query()的注释可看出，handle_pg_query()用于Primary OSD向replica/stray发送查询信息

----------
之后，PG主OSD接收到pg_query_t::INFO的响应信息(MSG_OSD_PG_NOTIFY)：
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
		
		...
	}
}

/** PGNotify
 * from non-primary to primary
 * includes pg_info_t.
 * NOTE: called with opqueue active.
 */
void OSD::handle_pg_notify(OpRequestRef op)
{
	for (vector<pair<pg_notify_t, pg_interval_map_t> >::iterator it = m->get_pg_list().begin();it != m->get_pg_list().end();++it) {
		
		if (it->first.info.pgid.preferred() >= 0) {
			dout(20) << "ignoring localized pg " << it->first.info.pgid << dendl;
			continue;
		}
		
		handle_pg_peering_evt(
			spg_t(it->first.info.pgid.pgid, it->first.to),
			it->first.info.history, it->second,
			it->first.query_epoch,
			PG::CephPeeringEvtRef(
				new PG::CephPeeringEvt(
					it->first.epoch_sent, it->first.query_epoch,
					PG::MNotifyRec(pg_shard_t(from, it->first.from), it->first,
					op->get_req()->get_connection()->get_features())))
		);
	}
}

/*
 * look up a pg.  if we have it, great.  if not, consider creating it IF the pg mapping
 * hasn't changed since the given epoch and we are the primary.
 */
void OSD::handle_pg_peering_evt(
  spg_t pgid,
  const pg_history_t& orig_history,
  pg_interval_map_t& pi,
  epoch_t epoch,
  PG::CephPeeringEvtRef evt)
{
	...
	if (!_have_pg(pgid)) {

		...

	} else {
		// already had it.  did the mapping change?
		PG *pg = _lookup_lock_pg(pgid);
		if (epoch < pg->info.history.same_interval_since) {
			dout(10) << *pg << " get_or_create_pg acting changed in "<< pg->info.history.same_interval_since<< " (msg from " << epoch << ")" << dendl;
			pg->unlock();
			return;
		}

		pg->queue_peering_event(evt);
		pg->unlock();
		return;
	}
}

boost::statechart::result PG::RecoveryState::GetInfo::react(const MNotifyRec& infoevt) 
{
	PG *pg = context< RecoveryMachine >().pg;
	
	set<pg_shard_t>::iterator p = peer_info_requested.find(infoevt.from);
	if (p != peer_info_requested.end()) {
		peer_info_requested.erase(p);
		pg->blocked_by.erase(infoevt.from.osd);
	}
	
	epoch_t old_start = pg->info.history.last_epoch_started;
	if (pg->proc_replica_info(infoevt.from, infoevt.notify.info, infoevt.notify.epoch_sent)) {
		// we got something new ...
		unique_ptr<PriorSet> &prior_set = context< Peering >().prior_set;
		if (old_start < pg->info.history.last_epoch_started) {
			dout(10) << " last_epoch_started moved forward, rebuilding prior" << dendl;
			pg->build_prior(prior_set);
	
			// filter out any osds that got dropped from the probe set from
			// peer_info_requested.  this is less expensive than restarting
			// peering (which would re-probe everyone).
			set<pg_shard_t>::iterator p = peer_info_requested.begin();

			while (p != peer_info_requested.end()) {
				if (prior_set->probe.count(*p) == 0) {
					dout(20) << " dropping osd." << *p << " from info_requested, no longer in probe set" << dendl;
					peer_info_requested.erase(p++);
				} else {
					++p;
				}
			}
			get_infos();
		}

		dout(20) << "Adding osd: " << infoevt.from.osd << " peer features: "<< hex << infoevt.features << dec << dendl;

		pg->apply_peer_features(infoevt.features);

		// are we done getting everything?
		if (peer_info_requested.empty() && !prior_set->pg_down) {
			/*
			* make sure we have at least one !incomplete() osd from the
			* last rw interval.  the incomplete (backfilling) replicas
			* get a copy of the log, but they don't get all the object
			* updates, so they are insufficient to recover changes during
			* that interval.
			*/
			if (pg->info.history.last_epoch_started) {
				for (map<epoch_t,pg_interval_t>::reverse_iterator p = pg->past_intervals.rbegin();p != pg->past_intervals.rend();++p) {
					if (p->first < pg->info.history.last_epoch_started)
						break;
					if (!p->second.maybe_went_rw)
						continue;

					pg_interval_t& interval = p->second;
					dout(10) << " last maybe_went_rw interval was " << interval << dendl;
					OSDMapRef osdmap = pg->get_osdmap();
	
					/*
					* this mirrors the PriorSet calculation: we wait if we
					* don't have an up (AND !incomplete) node AND there are
					* nodes down that might be usable.
					*/
					bool any_up_complete_now = false;
					bool any_down_now = false;
					for (unsigned i=0; i<interval.acting.size(); i++) {
						int o = interval.acting[i];
						if (o == CRUSH_ITEM_NONE)
							continue;

						pg_shard_t so(o, pg->pool.info.ec_pool() ? shard_id_t(i) : shard_id_t::NO_SHARD);
						if (!osdmap->exists(o) || osdmap->get_info(o).lost_at > interval.first)
							continue;  // dne or lost
						if (osdmap->is_up(o)) {
							pg_info_t *pinfo;
							if (so == pg->pg_whoami) {
								pinfo = &pg->info;
							} else {
								assert(pg->peer_info.count(so));
								pinfo = &pg->peer_info[so];
							}

							if (!pinfo->is_incomplete())
								any_up_complete_now = true;
						} else {
								any_down_now = true;
						}
					}

					if (!any_up_complete_now && any_down_now) {
						dout(10) << " no osds up+complete from interval " << interval << dendl;
						pg->state_set(PG_STATE_DOWN);
						pg->publish_stats_to_osd();
						return discard_event();
					}
					break;
				}
			}

			dout(20) << "Common peer features: " << hex << pg->get_min_peer_features() << dec << dendl;
			dout(20) << "Common acting features: " << hex << pg->get_min_acting_features() << dec << dendl;
			dout(20) << "Common upacting features: " << hex << pg->get_min_upacting_features() << dec << dendl;
			post_event(GotInfo());
		}
	}
	return discard_event();
}
{% endhighlight %}
上面获取到PG info之后交由GetInfo::react(const MNotifyRec&)进行处理。到此为止，GetInfo()流程执行完毕。

参看如下日志片段：
{% highlight string %}
42283:2020-09-11 14:10:18.974310 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>:  querying info from osd.2
42284:2020-09-11 14:10:18.974318 7fba3d925700 15 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] publish_stats_to_osd 2226:1335
42285:2020-09-11 14:10:18.974322 7fba3d925700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] handle_activate_map: Not dirtying info: last_persisted is 2224 while current is 2226
42286:2020-09-11 14:10:18.974326 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] handle_peering_event: epoch_sent: 2226 epoch_requested: 2226 NullEvt
44149:2020-09-11 14:10:18.981601 7fba45134700 20 osd.3 2226 _dispatch 0x7fba6de6d0e0 pg_notify(11.4 epoch 2226) v5
44160:2020-09-11 14:10:18.981660 7fba45134700 30 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] lock
44968:2020-09-11 14:10:18.984778 7fba3d124700 30 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] lock
44969:2020-09-11 14:10:18.984783 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] handle_peering_event: epoch_sent: 2226 epoch_requested: 2226 MNotifyRec from 2 notify: (query_epoch:2226, epoch_sent:2226, info:11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)) features: 0x7ffffffefdfbfff
44970:2020-09-11 14:10:18.984789 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering]  got osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
44971:2020-09-11 14:10:18.984795 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] update_heartbeat_peers 2,3 unchanged
44972:2020-09-11 14:10:18.984799 7fba3d124700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>: Adding osd: 2 peer features: 7ffffffefdfbfff
44973:2020-09-11 14:10:18.984803 7fba3d124700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>: Common peer features: 7ffffffefdfbfff
44974:2020-09-11 14:10:18.984817 7fba3d124700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>: Common acting features: 7ffffffefdfbfff
44975:2020-09-11 14:10:18.984820 7fba3d124700 20 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] state<Started/Primary/Peering/GetInfo>: Common upacting features: 7ffffffefdfbfff
44976:2020-09-11 14:10:18.984825 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] exit Started/Primary/Peering/GetInfo 0.010536 2 0.000071
{% endhighlight %}


###### 2.4.2 进入Peering/GetLog状态

我们先给出此一阶段的一个日志片段：
{% highlight string %}
44977:2020-09-11 14:10:18.984830 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] enter Started/Primary/Peering/GetLog
44978:2020-09-11 14:10:18.984834 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] calc_acting osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
44979:2020-09-11 14:10:18.984839 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] calc_acting osd.3 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223)
44980:2020-09-11 14:10:18.984847 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] calc_acting newest update on osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223)
44982:calc_acting primary is osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223)
44983: osd.2 (up) accepted 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
44985:2020-09-11 14:10:18.984851 7fba3d124700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] choose_acting want [3,2] != acting [3], requesting pg_temp change
44986:2020-09-11 14:10:18.984857 7fba3d124700  5 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped+peering] exit Started/Primary/Peering/GetLog 0.000026 0 0.000000
{% endhighlight %}
GetLog构造函数如下：
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
1） 调用函数pg->choose_acting()来选择出拥有权威日志的OSD，并计算出```acting_backfill```和```backfill_targets```两个OSD列表。选择出来的权威OSD通过auth_log_shard参数返回；

2） 如果选择失败，并且want_acting不为空，就抛出NeedActingChange事件，状态机转移到Primary/WaitActingChange状态，等待申请临时PG返回结果；如果want_acting为空，就抛出IsIncomplete事件，PG的状态机转移到Primary/Peering/Incomplete状态，表明失败，PG就处于InComplete状态。

3) 如果auth_log_shard等于pg->pg_whoami，也就是选出的拥有权威日志的OSD为当前主OSD，直接抛出GotLog()完成GetLog过程；

4） 如果pg->info.last_update小于best.log_tail，也就是本OSD的日志和权威日志不重叠，那么本OSD无法恢复，抛出IsInComplete事件。经过函数choose_acting()的选择后，主OSD必须是可恢复的。如果主OSD不可恢复，必须申请临时PG，选择拥有权威日志的OSD为临时主OSD；

5）如果自己不是权威日志的OSD，则需要去拥有权威日志的OSD上去拉取权威日志，并与本地合并。发送pg_query_t::LOG请求的过程与pg_query_t::INFO的过程是一样的。


**2.4.2.1 PG::choose_acting()函数**

函数choose_acting()用来计算PG的acting_backfill和backfill_targets两个OSD列表。acting_backfill保存了当前PG的acting列表，包括需要进行Backfill操作的OSD列表；backfill_targets列表保存了需要进行Backfill的OSD列表。
{% highlight string %}
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound)
{
	...
	map<pg_shard_t, pg_info_t>::const_iterator auth_log_shard =
	   find_best_info(all_info, restrict_to_up_acting, history_les_bound);
	
	if (auth_log_shard == all_info.end()) {
		if (up != acting) {
			dout(10) << "choose_acting no suitable info found (incomplete backfills?)," << " reverting to up" << dendl;
			want_acting = up;
			vector<int> empty;
			osd->queue_want_pg_temp(info.pgid.pgid, empty);
		} else {
			dout(10) << "choose_acting failed" << dendl;
			assert(want_acting.empty());
		}

		return false;
	}

	....

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

}
{% endhighlight %}

1) 首先调用函数PG::find_best_info()来选举出一个拥有权威日志的OSD，保存在变量auth_log_shard里；

2） 如果没有选举出拥有权威日志的OSD，则进入如下流程：

  a) 如果up不等于acting，申请临时PG，返回false值；

  b) 否则确保want_acting列表为空，返回false值；

>注： 在osdmap发生变化时，OSD::advance_pg()中会计算acting以及up，最终调用到OSDMap::_pg_to_up_acting_osds()来进行计算；

3） 计算是否是compat_mode模式，检查是，如果所有的OSD都支持纠删码，就设置compat_mode值为true；

4） 根据PG的不同类型，调用不同的函数。对应ReplicatedPG调用函数calc_replicated_acting()来计算PG需要的列表


在这里针对PG 11.4而言，我们选择出来的权威日志OSD为osd3，PG的up set为[3,2]， acting set为[3]。调用ReplicatedPG::calc_replicated_acting()计算出的want为[3,2]，因此这里want不等于acting，需要申请产生pg_temp。

OSDService::send_pg_temp()请求会在OSD::process_peering_events()的最后发出。通过下面的日志，我们可以看出发出去的pg_temp请求中PG 11.4的pg_temp为[]:
{% highlight string %}
45166:2020-09-11 14:10:18.985528 7fba3d124700 10 osd.3 2226 send_pg_temp {11.4=[],11.6=[],19.1=[],22.2c=[],22.44=[],22.a4=[],22.b5=[],22.ca=[],22.d0=[],22.ec=[],23.d=[],23.13=[],23.30=[],23.6d=[]}
{% endhighlight %}

上面我们可以看到，但```want_acting```等于```up```时，调用PG::queue_want_pg_temp()时传递了一个empty，这个是什么意思呢？ 其实是用于向OsdMonitor发送一个请求，要求删除PG 11.4对应的PG temp，参看如下日志(mon-node7-1.txt)：
{% highlight string %}
2020-09-11 14:10:18.985380 7f8f92d0c700 10 mon.node7-1@0(leader).osd e2226 preprocess_query osd_pgtemp(e2226 {11.4=[],11.6=[],19.1=[],22.2c=[],22.44=[],22.a4=[],22.b5=[],22.ca=[],22.d0=[],22.ec=[],23.d=[],23.13=[],23.30=[],23.6d=[]} v2226) v1 from osd.3 10.17.155.114:6800/894
2020-09-11 14:10:18.985393 7f8f92d0c700 10 mon.node7-1@0(leader).osd e2226 preprocess_pgtemp osd_pgtemp(e2226 {11.4=[],11.6=[],19.1=[],22.2c=[],22.44=[],22.a4=[],22.b5=[],22.ca=[],22.d0=[],22.ec=[],23.d=[],23.13=[],23.30=[],23.6d=[]} v2226) v1
2020-09-11 14:10:18.985403 7f8f92d0c700 20 is_capable service=osd command= exec on cap allow rwx
2020-09-11 14:10:18.985406 7f8f92d0c700 20  allow so far , doing grant allow rwx
2020-09-11 14:10:18.985408 7f8f92d0c700 20  match
2020-09-11 14:10:18.985410 7f8f92d0c700 20 mon.node7-1@0(leader).osd e2226  11.4[0,3] -> []
2020-09-11 14:10:18.985452 7f8f92d0c700  7 mon.node7-1@0(leader).osd e2226 prepare_update osd_pgtemp(e2226 {11.4=[],11.6=[],19.1=[],22.2c=[],22.44=[],22.a4=[],22.b5=[],22.ca=[],22.d0=[],22.ec=[],23.d=[],23.13=[],23.30=[],23.6d=[]} v2226) v1 from osd.3 10.17.155.114:6800/894
2020-09-11 14:10:18.985486 7f8f92d0c700  7 mon.node7-1@0(leader).osd e2226 prepare_pgtemp e2226 from osd.3 10.17.155.114:6800/894
{% endhighlight %}

代码调用流程如下：
{% highlight string %}
bool PaxosService::dispatch(MonOpRequestRef op){
	...

	// preprocess
	if (preprocess_query(op)) 
		return true;  // easy!

	...

	if (prepare_update(op)) {
		...
	}

	...
}

bool OSDMonitor::preprocess_query(MonOpRequestRef op){

	...

	switch (m->get_type()) {
		...
		case MSG_OSD_PGTEMP:
    		return preprocess_pgtemp(op);

		...
	}

	...
}

bool OSDMonitor::prepare_update(MonOpRequestRef op){
	...

	switch (m->get_type()) {
		...
		case MSG_OSD_PGTEMP:
    		return prepare_pgtemp(op);

		...
	}

	...
}
{% endhighlight %}


###### 2.4.3 进入Peering/WaitActingChange状态
在上面Peering/GetLog构造函数中，由于PG::choose_acting()函数返回false，并且pg->want_acting不为空，因此产生NeedActingChange()事件，从而进入WaitActingChange状态。
{% highlight string %}
PG::RecoveryState::WaitActingChange::WaitActingChange(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Peering/WaitActingChange")
{
	context< RecoveryMachine >().log_enter(state_name);
}
{% endhighlight %}

1) **收到AdvMap事件**

在进入WaitActingChange状态之前，由于osd3向OSDMonitor申请pg temp，导致osdmap发生变更，从而触发如下调用(此时osdmap版本为e2227)：
{% highlight string %}
void PG::handle_advance_map(
  OSDMapRef osdmap, OSDMapRef lastmap,
  vector<int>& newup, int up_primary,
  vector<int>& newacting, int acting_primary,
  RecoveryCtx *rctx)
{
	...

	AdvMap evt(
		osdmap, lastmap, newup, up_primary,
		newacting, acting_primary);
	recovery_state.handle_event(evt, rctx);
}

boost::statechart::result PG::RecoveryState::WaitActingChange::react(const AdvMap& advmap)
{
	PG *pg = context< RecoveryMachine >().pg;
	OSDMapRef osdmap = advmap.osdmap;
	
	dout(10) << "verifying no want_acting " << pg->want_acting << " targets didn't go down" << dendl;
	for (vector<int>::iterator p = pg->want_acting.begin(); p != pg->want_acting.end(); ++p) {
		if (!osdmap->is_up(*p)) {
			dout(10) << " want_acting target osd." << *p << " went down, resetting" << dendl;
			post_event(advmap);
			return transit< Reset >();
		}
	}
	return forward_event();
}
boost::statechart::result PG::RecoveryState::Started::react(const AdvMap& advmap)
{
	dout(10) << "Started advmap" << dendl;
	PG *pg = context< RecoveryMachine >().pg;
	pg->check_full_transition(advmap.lastmap, advmap.osdmap);
	if (pg->should_restart_peering(
	  advmap.up_primary,
	  advmap.acting_primary,
	  advmap.newup,
	  advmap.newacting,
	  advmap.lastmap,
	  advmap.osdmap)) {
		dout(10) << "should_restart_peering, transitioning to Reset" << dendl;
		post_event(advmap);
		return transit< Reset >();
	}
	pg->remove_down_peer_info(advmap.osdmap);
	return discard_event();
}
{% endhighlight %}
由于此时up set为[3,2]，acting set也由[3]变为了[3,2]，因此会再一次触发Peering操作。这里重新进入Reset状态。

如下是本阶段的一个日志片段：
{% highlight string %}
47209:2020-09-11 14:10:20.014782 7fba3d925700 10 osd.3 pg_epoch: 2226 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] handle_advance_map [3,2]/[3,2] -- 3/3
47212:2020-09-11 14:10:20.014796 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Started/Primary/Peering/WaitActingChange>: verifying no want_acting [] targets didn't go down
47279:2020-09-11 14:10:20.014814 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Started>: Started advmap
47281:2020-09-11 14:10:20.015156 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] new interval newup [3,2] newacting [3,2]
47283:2020-09-11 14:10:20.015174 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Started>: should_restart_peering, transitioning to Reset
47285:2020-09-11 14:10:20.015184 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit Started/Primary/Peering/WaitActingChange 1.030293 1 0.000106
47288:2020-09-11 14:10:20.015195 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2226 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] exit Started/Primary 1.030307 0 0.000000
{% endhighlight %}


## 3. 重新进入Peering流程
在上面最后收到的osdmap版本为e2227，然后重新进入Reset阶段，触发新的Peering过程。

### 3.1 Reset状态
{% highlight string %}
PG::RecoveryState::Reset::Reset(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Reset")
{
	context< RecoveryMachine >().log_enter(state_name);
	PG *pg = context< RecoveryMachine >().pg;
	
	pg->flushes_in_progress = 0;
	pg->set_last_peering_reset();
}
{% endhighlight %}
调用pg::set_last_peering_reset()设置osdmap的epoch为e2227.

###### 3.1 Reset状态下对AdvMap事件的处理
在进入Reset状态之前，我们向其投递了AdvMap事件，这里我们来看一下对该事件的处理：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Reset::react(const AdvMap& advmap)
{
}
{% endhighlight %}
如下是此一过程的日志片段：
{% highlight string %}

47303:2020-09-11 14:10:20.015280 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Reset>: Reset advmap
47394:2020-09-11 14:10:20.015296 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] _calc_past_interval_range: already have past intervals back to 2224
47396:2020-09-11 14:10:20.015871 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] new interval newup [3,2] newacting [3,2]
47399:2020-09-11 14:10:20.015889 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] state<Reset>: should restart peering, calling start_peering_interval again
47400:2020-09-11 14:10:20.015898 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2]/[3] r=0 lpr=2227 pi=2223-2225/1 crt=201'1 lcod 0'0 mlcod 0'0 remapped] set_last_peering_reset 2227
47403:2020-09-11 14:10:20.015920 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] start_peering_interval: check_new_interval output: generate_past_intervals interval(2226-2226 up [3,2](3) acting [3](3)): not rw, up_thru 2223 up_from 2123 last_epoch_clean 2224
47409:2020-09-11 14:10:20.015946 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2226/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  noting past interval(2226-2226 up [3,2](3) acting [3](3))
47411:2020-09-11 14:10:20.015960 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  up [3,2] -> [3,2], acting [3] -> [3,2], acting_primary 3 -> 3, up_primary 3 -> 3, role 0 -> 0, features acting 576460752032874495 upacting 576460752032874495
47413:2020-09-11 14:10:20.015972 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] clear_primary_state
47414:2020-09-11 14:10:20.015983 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] agent_stop
47415:2020-09-11 14:10:20.015991 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_change
47417:2020-09-11 14:10:20.015998 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47419:2020-09-11 14:10:20.016009 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] publish_stats_to_osd 2227:1337
47421:2020-09-11 14:10:20.016019 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47422:2020-09-11 14:10:20.016027 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_copy_ops
47424:2020-09-11 14:10:20.016034 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_flush_ops
47426:2020-09-11 14:10:20.016042 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_proxy_ops
47428:2020-09-11 14:10:20.016051 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47430:2020-09-11 14:10:20.016060 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47432:2020-09-11 14:10:20.016070 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive]  requeue_ops 
47435:2020-09-11 14:10:20.016080 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_change_cleanup
47437:2020-09-11 14:10:20.016090 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] on_change
47440:2020-09-11 14:10:20.016101 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit NotTrimming
47442:2020-09-11 14:10:20.016112 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter NotTrimming
47444:2020-09-11 14:10:20.016122 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] [3] -> [3,2], replicas changed
47447:2020-09-11 14:10:20.016131 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] cancel_recovery
47449:2020-09-11 14:10:20.016140 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] clear_recovery_state
47451:2020-09-11 14:10:20.016150 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] check_recovery_sources no source osds () went down
{% endhighlight %}
具体的代码分析，我们在前面已经讲解过，这里不再赘述。

### 3.2 ActMap事件的处理
在接收到新的OSDMap，处理完成AdvMap事件之后，接着就会调用PG::handle_activate_map()来激活osdmap:
{% highlight string %}
void PG::handle_activate_map(RecoveryCtx *rctx)
{
}
{% endhighlight %}
如下是相关日志片段，具体代码分析这里将不再赘述：
{% highlight string %}
47454:2020-09-11 14:10:20.016168 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] handle_activate_map 
47457:2020-09-11 14:10:20.016178 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] update_heartbeat_peers 2,3 unchanged
47458:2020-09-11 14:10:20.016188 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] take_waiters
47461:2020-09-11 14:10:20.016198 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] exit Reset 0.000960 1 0.001368
47463:2020-09-11 14:10:20.016211 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started
47465:2020-09-11 14:10:20.016221 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Start
{% endhighlight %}

### 3.3 进入Started状态
```Started```构造函数如下：
{% highlight string %}
/*------Started-------*/
PG::RecoveryState::Started::Started(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started")
{
	context< RecoveryMachine >().log_enter(state_name);
}
{% endhighlight %}

###### 3.3.1 进入Started/Start状态
进入Started状态后，会进去其默认子状态```Start```:
{% highlight string %}
/*-------Start---------*/
PG::RecoveryState::Start::Start(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Start")
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	if (pg->is_primary()) {
		dout(1) << "transitioning to Primary" << dendl;
		post_event(MakePrimary());
	} else { //is_stray
		dout(1) << "transitioning to Stray" << dendl; 
		post_event(MakeStray());
	}
}
{% endhighlight %}
在```Start```状态的构造函数中，根据PG对应OSD的角色不同，选择进入```Primary```状态或者```Stray```状态。

这里osd3对于PG 11.4而言为主OSD，因此产生MakePrimary()事件，进入Started/Primary状态。

###### 3.3.2 进入Started/Primary状态
Primary构造函数如下：
{% highlight string %}
/*---------Primary--------*/
PG::RecoveryState::Primary::Primary(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary")
{
	context< RecoveryMachine >().log_enter(state_name);
	PG *pg = context< RecoveryMachine >().pg;
	assert(pg->want_acting.empty());
	
	// set CREATING bit until we have peered for the first time.
	if (pg->info.history.last_epoch_started == 0) {
		pg->state_set(PG_STATE_CREATING);
		// use the history timestamp, which ultimately comes from the
		// monitor in the create case.
		utime_t t = pg->info.history.last_scrub_stamp;
		pg->info.stats.last_fresh = t;
		pg->info.stats.last_active = t;
		pg->info.stats.last_change = t;
		pg->info.stats.last_peered = t;
		pg->info.stats.last_clean = t;
		pg->info.stats.last_unstale = t;
		pg->info.stats.last_undegraded = t;
		pg->info.stats.last_fullsized = t;
		pg->info.stats.last_scrub_stamp = t;
		pg->info.stats.last_deep_scrub_stamp = t;
		pg->info.stats.last_clean_scrub_stamp = t;
	}
}
{% endhighlight %}
注意： 在上一次退出```Primary```状态时，我们就已经清空了pg->want_acting。

### 3.4 进入Peering状态

Started/Primary的默认初始子状态为Started/Primary/Peering，现在我们来看一下Peering的构造函数：
{% highlight string %}
/*---------Peering--------*/
PG::RecoveryState::Peering::Peering(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Peering"),
    history_les_bound(false)
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	assert(!pg->is_peered());
	assert(!pg->is_peering());
	assert(pg->is_primary());
	pg->state_set(PG_STATE_PEERING);
}
{% endhighlight %}

###### 3.4.1 进入Started/Primary/Peering/GetInfo状态
进入Peering状态后，默认会跳转进入Peering的子状态GetInfo，如下是此过程的一个日志片段：
{% highlight string %}
47475:2020-09-11 14:10:20.016271 7fba3d925700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetInfo
47478:2020-09-11 14:10:20.016281 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] _calc_past_interval_range: already have past intervals back to 2224
47481:2020-09-11 14:10:20.016292 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2226-2226 up [3,2](3) acting [3](3))
47482:2020-09-11 14:10:20.016301 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2223-2225 up [3](3) acting [3](3) maybe_went_rw)
47483:2020-09-11 14:10:20.016310 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior final: probe 2,3 down  blocked_by {}
47484:2020-09-11 14:10:20.016318 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] up_thru 2226 < same_since 2227, must notify monitor
47485:2020-09-11 14:10:20.016327 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>:  querying info from osd.2
47487:2020-09-11 14:10:20.016337 7fba3d925700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2227:1338
47489:2020-09-11 14:10:20.016346 7fba3d925700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_activate_map: Not dirtying info: last_persisted is 2226 while current is 2227
47492:2020-09-11 14:10:20.016356 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2227 epoch_requested: 2227 NullEvt
50548:2020-09-11 14:10:20.028700 7fba3d124700 20   ? 1956'80 (1956'79) modify   23:1ce16903:::obj-9bjaNA0lG2ZpqCJ:head by client.291879.0:1164 2020-05-30 10:20:25.595078
50723:2020-09-11 14:10:20.029810 7fba3d124700 20 update missing, append 1956'80 (1956'79) modify   23:1ce16903:::obj-9bjaNA0lG2ZpqCJ:head by client.291879.0:1164 2020-05-30 10:20:25.595078
50826:2020-09-11 14:10:20.031026 7fba45134700 20 osd.3 2227 _dispatch 0x7fba6d7401e0 pg_notify(11.4 epoch 2227) v5
50829:2020-09-11 14:10:20.031044 7fba45134700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
50838:2020-09-11 14:10:20.031074 7fba3d124700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
50841:2020-09-11 14:10:20.031095 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2227 epoch_requested: 2227 MNotifyRec from 2 notify: (query_epoch:2227, epoch_sent:2227, info:11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)) features: 0x7ffffffefdfbfff
50846:2020-09-11 14:10:20.031104 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  got osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
50848:2020-09-11 14:10:20.031115 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] update_heartbeat_peers 2,3 unchanged
50850:2020-09-11 14:10:20.031122 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Adding osd: 2 peer features: 7ffffffefdfbfff
50851:2020-09-11 14:10:20.031129 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Common peer features: 7ffffffefdfbfff
50854:2020-09-11 14:10:20.031135 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Common acting features: 7ffffffefdfbfff
50857:2020-09-11 14:10:20.031140 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Common upacting features: 7ffffffefdfbfff
50859:2020-09-11 14:10:20.031147 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetInfo 0.014876 2 0.000170
{% endhighlight %}
```GetInfo```状态的构造函数如下：
{% highlight string %}
/*--------GetInfo---------*/
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

1) **函数PG::generate_past_intervals()**
{% highlight string %}
void PG::generate_past_intervals()
{
	epoch_t cur_epoch, end_epoch;
	if (!_calc_past_interval_range(&cur_epoch, &end_epoch,osd->get_superblock().oldest_map)) {
		if (info.history.same_interval_since == 0) {
			info.history.same_interval_since = end_epoch;
			dirty_info = true;
		}
		return;
	}

	...
	
}
bool PG::_calc_past_interval_range(epoch_t *start, epoch_t *end, epoch_t oldest_map)
{
	if (info.history.same_interval_since) {
		*end = info.history.same_interval_since;
	} else {
		// PG must be imported, so let's calculate the whole range.
		*end = osdmap_ref->get_epoch();
	}
	
	// Do we already have the intervals we want?
	map<epoch_t,pg_interval_t>::const_iterator pif = past_intervals.begin();
	if (pif != past_intervals.end()) {
		if (pif->first <= info.history.last_epoch_clean) {
			dout(10) << __func__ << ": already have past intervals back to "<< info.history.last_epoch_clean << dendl;
			return false;
		}
		*end = past_intervals.begin()->first;
	}
	
	*start = MAX(MAX(info.history.epoch_created,
	info.history.last_epoch_clean),
	oldest_map);
	if (*start >= *end) {
		dout(10) << __func__ << " start epoch " << *start << " >= end epoch " << *end<< ", nothing to do" << dendl;
		return false;
	}
	
	return true;
}
{% endhighlight %}
当前的osdmap版本为e2227，在e2226时PG 11.4还未进入Active状态即又开始触发Peering过程，因此在上面PG::_calc_past_interval_range()函数中info.history.last_epoch_clean的值仍为e2224。参看如下日志片段：
{% highlight string %}
47478:2020-09-11 14:10:20.016281 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] _calc_past_interval_range: already have past intervals back to 2224
{% endhighlight %}

2) **函数PG::build_prior()**

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
{% endhighlight %}
```PriorSet```用于记录PG前一阶段的状态信息，主要用于辅助当前阶段的恢复工作。在当前阶段osdmap版本为e2227，up set为[3,2]， acting set为[3,2]，info.history.last_epoch_clean为e2224。在构建PriorSet时，参看如下日志片段：
{% highlight string %}
47481:2020-09-11 14:10:20.016292 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2226-2226 up [3,2](3) acting [3](3))
47482:2020-09-11 14:10:20.016301 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior interval(2223-2225 up [3](3) acting [3](3) maybe_went_rw)
47483:2020-09-11 14:10:20.016310 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior final: probe 2,3 down  blocked_by {}
47484:2020-09-11 14:10:20.016318 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] up_thru 2226 < same_since 2227, must notify monitor
{% endhighlight %}
从上面可以看到，在[e2226,e2226]阶段对应的up set为[3,2]， acting set为[3]； 在[e2223, e2225]阶段对应的up set为[3]， acting set为[3]，并且可能执行了写操作。在当前e2227阶段，probe set为[2,3]， down set为[]，并且获取到OSD3的up_thru为e2226，需要通知monitor。

通知monitor会在OSD::process_peering_events()函数的最后来进行：
{% highlight string %}
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...
	if (need_up_thru)
		queue_want_up_thru(same_interval_since);

	...
}
{% endhighlight %}
关于```PriorSet```更为详细的介绍，我们会在后续up_thru相关文章中进一步说明。

3) **PG::get_infos()**
{% highlight string %}
void PG::RecoveryState::GetInfo::get_infos()
{
	PG *pg = context< RecoveryMachine >().pg;
	unique_ptr<PriorSet> &prior_set = context< Peering >().prior_set;
	
	pg->blocked_by.clear();
	for (set<pg_shard_t>::const_iterator it = prior_set->probe.begin();it != prior_set->probe.end();++it) {
		pg_shard_t peer = *it;
		if (peer == pg->pg_whoami) {
			continue;
		}

		if (pg->peer_info.count(peer)) {
			dout(10) << " have osd." << peer << " info " << pg->peer_info[peer] << dendl;
			continue;
		}

		if (peer_info_requested.count(peer)) {
			dout(10) << " already requested info from osd." << peer << dendl;
			pg->blocked_by.insert(peer.osd);
		} else if (!pg->get_osdmap()->is_up(peer.osd)) {
			dout(10) << " not querying info from down osd." << peer << dendl;
		} else {
			dout(10) << " querying info from osd." << peer << dendl;
			context< RecoveryMachine >().send_query(
			  peer, pg_query_t(pg_query_t::INFO,
				it->shard, pg->pg_whoami.shard,
				pg->info.history,
				pg->get_osdmap()->get_epoch()));
			peer_info_requested.insert(peer);
			pg->blocked_by.insert(peer.osd);
		}
	}
	
	pg->publish_stats_to_osd();
}
{% endhighlight %}
在PG::get_infos()函数中会遍历prior_set，然后向对应的从OSD发送pg_query_t::INFO消息，以获取从OSD的pg_info_t信息。这里对于PG 11.4而言，因为在前面PG::build_prior()所构建的probe为[2,3]，因此这里会向osd2发送pg_query_t::INFO信息。
{% highlight string %}
47483:2020-09-11 14:10:20.016310 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  PriorSet: build_prior final: probe 2,3 down  blocked_by {}
{% endhighlight %}

3) **收到MNotifyRec信息**

在上面步骤2） 中向OSD2发送了pg_query_t::INFO消息，这里收到了对该请求的响应，然后调用GetInfo::react(const MNotifyRec &)来进行处理：
{% highlight string %}
boost::statechart::result PG::RecoveryState::GetInfo::react(const MNotifyRec& infoevt) 
{
}
{% endhighlight %}
下面给出此一过程相应的日志片段：
{% highlight string %}
50841:2020-09-11 14:10:20.031095 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2227 epoch_requested: 2227 MNotifyRec from 2 notify: (query_epoch:2227, epoch_sent:2227, info:11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)) features: 0x7ffffffefdfbfff
50846:2020-09-11 14:10:20.031104 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  got osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
50848:2020-09-11 14:10:20.031115 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] update_heartbeat_peers 2,3 unchanged
50850:2020-09-11 14:10:20.031122 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Adding osd: 2 peer features: 7ffffffefdfbfff
50851:2020-09-11 14:10:20.031129 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Common peer features: 7ffffffefdfbfff
50854:2020-09-11 14:10:20.031135 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Common acting features: 7ffffffefdfbfff
50857:2020-09-11 14:10:20.031140 7fba3d124700 20 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetInfo>: Common upacting features: 7ffffffefdfbfff
50859:2020-09-11 14:10:20.031147 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetInfo 0.014876 2 0.000170
{% endhighlight %}
在上面react(const MNotifyRec&)函数中，首先调用PG::proc_replica_info()来处理从OSD返回过来的pg_info信息。如果PG::proc_replica_info()返回true，表明成功返回的pg_info信息有效，然后执行如下流程：

3.1) 检查pg->info.history.last_epoch_started，如果主OSD的```old_start```小于返回的history.last_epoch_started，表明当前主OSD过于老旧，需要重新build_prior()，然后重新get_infos();

>注： 当前pg->info.history.last_epoch_started的值为e2224??

3.2) 判断我们已经收到了pg->peer_info_requested中所有请求的响应信息。在这些响应中，我们要求在最后一个rw interval中，至少要有一个OSD处于complete状态，这样才能准确获取到该PG在最后所做的修改，后续才能进行恢复
{% highlight string %}
boost::statechart::result PG::RecoveryState::GetInfo::react(const MNotifyRec& infoevt) 
{
	...

	// are we done getting everything?
	if (peer_info_requested.empty() && !prior_set->pg_down){
		/*
		* make sure we have at least one !incomplete() osd from the
		* last rw interval.  the incomplete (backfilling) replicas
		* get a copy of the log, but they don't get all the object
		* updates, so they are insufficient to recover changes during
		* that interval.
		*/
		if (pg->info.history.last_epoch_started) {
			for (map<epoch_t,pg_interval_t>::reverse_iterator p = pg->past_intervals.rbegin();p != pg->past_intervals.rend();++p) {
				if (p->first < pg->info.history.last_epoch_started)
					break;
				if (!p->second.maybe_went_rw)
					continue;

				pg_interval_t& interval = p->second;
				dout(10) << " last maybe_went_rw interval was " << interval << dendl;
				OSDMapRef osdmap = pg->get_osdmap();
	
				/*
				* this mirrors the PriorSet calculation: we wait if we
				* don't have an up (AND !incomplete) node AND there are
				* nodes down that might be usable.
				*/
				bool any_up_complete_now = false;
				bool any_down_now = false;
				for (unsigned i=0; i<interval.acting.size(); i++) {
					int o = interval.acting[i];

					if (o == CRUSH_ITEM_NONE)
						continue;
					pg_shard_t so(o, pg->pool.info.ec_pool() ? shard_id_t(i) : shard_id_t::NO_SHARD);
					if (!osdmap->exists(o) || osdmap->get_info(o).lost_at > interval.first)
						continue;  // dne or lost
					if (osdmap->is_up(o)) {
						pg_info_t *pinfo;
						if (so == pg->pg_whoami) {
							pinfo = &pg->info;
						} else {
							assert(pg->peer_info.count(so));
							pinfo = &pg->peer_info[so];
						}
						if (!pinfo->is_incomplete())
							any_up_complete_now = true;

					} else {
						any_down_now = true;
					}

				}  //end for

				if (!any_up_complete_now && any_down_now) {
					dout(10) << " no osds up+complete from interval " << interval << dendl;

					pg->state_set(PG_STATE_DOWN);
					pg->publish_stats_to_osd();
					return discard_event();

				}  //end if

				break;

			} //end for

		}  //end if
		dout(20) << "Common peer features: " << hex << pg->get_min_peer_features() << dec << dendl;
		dout(20) << "Common acting features: " << hex << pg->get_min_acting_features() << dec << dendl;
		dout(20) << "Common upacting features: " << hex << pg->get_min_upacting_features() << dec << dendl;
		post_event(GotInfo());

	}  //end if

	...
}
{% endhighlight %}
这里针对PG 11.4而言，最后一个past_interval为[e2226,e2226]，倒数第二个past_interval为[e2223, e2225]。

4） **函数PG::proc_replica_info()**
{% highlight string %}
bool PG::proc_replica_info(
  pg_shard_t from, const pg_info_t &oinfo, epoch_t send_epoch)
{
	map<pg_shard_t, pg_info_t>::iterator p = peer_info.find(from);
	if (p != peer_info.end() && p->second.last_update == oinfo.last_update) {
		dout(10) << " got dup osd." << from << " info " << oinfo << ", identical to ours" << dendl;
		return false;
	}
	
	if (!get_osdmap()->has_been_up_since(from.osd, send_epoch)) {
		dout(10) << " got info " << oinfo << " from down osd." << from<< " discarding" << dendl;
		return false;
	}
	
	dout(10) << " got osd." << from << " " << oinfo << dendl;
	assert(is_primary());
	peer_info[from] = oinfo;
	might_have_unfound.insert(from);
	
	unreg_next_scrub();
	if (info.history.merge(oinfo.history))
		dirty_info = true;

	reg_next_scrub();
	
	// stray?
	if (!is_up(from) && !is_acting(from)) {
		dout(10) << " osd." << from << " has stray content: " << oinfo << dendl;
		stray_set.insert(from);
		if (is_clean()) {
			purge_strays();
		}
	}
	
	// was this a new info?  if so, update peers!
	if (p == peer_info.end())
		update_heartbeat_peers();
	
	return true;
}
{% endhighlight %}
PG::proc_replica_info()首先判断收到的pg_info信息是否有效，如下两种情况都被判定为无效，直接返回false

* 收到了重复的pg_info

* pg_info源osd当前是否处于down状态

接着调用pg_history_t::merge()来合并history，这是一个十分重要的函数：
{% highlight string %}
bool merge(const pg_history_t &other) {
	// Here, we only update the fields which cannot be calculated from the OSDmap.
	bool modified = false;
	if (epoch_created < other.epoch_created) {
		epoch_created = other.epoch_created;
		modified = true;
	}

	if (last_epoch_started < other.last_epoch_started) {
		last_epoch_started = other.last_epoch_started;
		modified = true;
	}

	if (last_epoch_clean < other.last_epoch_clean) {
		last_epoch_clean = other.last_epoch_clean;
		modified = true;
	}
	if (last_epoch_split < other.last_epoch_split) {
		last_epoch_split = other.last_epoch_split; 
		modified = true;
	}
	if (last_epoch_marked_full < other.last_epoch_marked_full) {
		last_epoch_marked_full = other.last_epoch_marked_full;
		modified = true;
	}
	if (other.last_scrub > last_scrub) {
		last_scrub = other.last_scrub;
		modified = true;
	}
	if (other.last_scrub_stamp > last_scrub_stamp) {
		last_scrub_stamp = other.last_scrub_stamp;
		modified = true;
	}
	if (other.last_deep_scrub > last_deep_scrub) {
		last_deep_scrub = other.last_deep_scrub;
		modified = true;
	}
	if (other.last_deep_scrub_stamp > last_deep_scrub_stamp) {
		last_deep_scrub_stamp = other.last_deep_scrub_stamp;
		modified = true;
	}
	if (other.last_clean_scrub_stamp > last_clean_scrub_stamp) {
		last_clean_scrub_stamp = other.last_clean_scrub_stamp;
		modified = true;
	}
	return modified;
}
{% endhighlight %}
从上面可以看到，如果相应的history信息发生了改变，则merge()成功，返回true，否则返回false。pg_history_t::merge()函数主要用于合并如下字段：

* history.epoch_created

* history.last_epoch_started

* history.last_epoch_clean

* history.last_epoch_split

* history.last_epoch_marked_full

* history.last_scrub

* history.last_scrub_stamp

* history.last_deep_scrub

* history.last_deep_scrub_stamp

* history.last_clean_scrub_stamp

结合如下打印日志：
{% highlight string %}
50841:2020-09-11 14:10:20.031095 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2227 epoch_requested: 2227 MNotifyRec from 2 notify: (query_epoch:2227, epoch_sent:2227, info:11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)) features: 0x7ffffffefdfbfff
50846:2020-09-11 14:10:20.031104 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  got osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
{% endhighlight %}
如下是打印pg_info的函数实现(src/osd/osd_types.h)：
{% highlight string %}
inline ostream& operator<<(ostream& out, const pg_info_t& pgi) 
{
	out << pgi.pgid << "(";
	if (pgi.dne())
		out << " DNE";
	if (pgi.is_empty())
		out << " empty";
	else {
		out << " v " << pgi.last_update;
		if (pgi.last_complete != pgi.last_update)
			out << " lc " << pgi.last_complete;
		out << " (" << pgi.log_tail << "," << pgi.last_update << "]";
	}
	if (pgi.is_incomplete())
		out << " lb " << pgi.last_backfill << (pgi.last_backfill_bitwise ? " (bitwise)" : " (NIBBLEWISE)");

	//out << " c " << pgi.epoch_created;
	out << " local-les=" << pgi.last_epoch_started;
	out << " n=" << pgi.stats.stats.sum.num_objects;
	out << " " << pgi.history<< ")";

	return out;
}

inline ostream& operator<<(ostream& out, const pg_history_t& h) {
	return out << "ec=" << h.epoch_created
		<< " les/c/f " << h.last_epoch_started << "/" << h.last_epoch_clean
		<< "/" << h.last_epoch_marked_full
		<< " " << h.same_up_since << "/" << h.same_interval_since << "/" << h.same_primary_since;
}
{% endhighlight %}
从这里我们可以知道，对于PG 11.4，从osd2获取到的pg_info信息为：

* info.last_epoch_started为e0

* info.stats.stats.sum.num_objects为0

* info.history.epoch_created为0

* info.history.last_epoch_started为e0

* info.history.last_epoch_clean为e0

* info.history.last_epoch_marked_full为e0

* info.history.same_up_since为e0

* info.history.same_interval_since为e0

* info.history.same_primary_since为e0

###### 3.4.2 进入Started/Primary/Peering/GetLog状态

在上面成功获取到Info信息之后，接着就会进入GetLog状态：
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
{% endhighlight %}
执行步骤如下：


1） 调用函数pg->choose_acting()来选择出拥有权威日志的OSD，并计算出```acting_backfill```和```backfill_targets```两个OSD列表。选择出来的权威OSD通过auth_log_shard参数返回；

2） 如果选择失败，并且want_acting不为空，就抛出NeedActingChange事件，状态机转移到Primary/WaitActingChange状态，等待申请临时PG返回结果；如果want_acting为空，就抛出IsIncomplete事件，PG的状态机转移到Primary/Peering/Incomplete状态，表明失败，PG就处于InComplete状态。

3) 如果auth_log_shard等于pg->pg_whoami，也就是选出的拥有权威日志的OSD为当前主OSD，直接抛出GotLog()完成GetLog过程；

4） 如果pg->info.last_update小于best.log_tail，也就是本OSD的日志和权威日志不重叠，那么本OSD无法恢复，抛出IsInComplete事件。经过函数choose_acting()的选择后，主OSD必须是可恢复的。如果主OSD不可恢复，必须申请临时PG，选择拥有权威日志的OSD为临时主OSD；

5）如果自己不是权威日志的OSD，则需要去拥有权威日志的OSD上去拉取权威日志，并与本地合并。发送pg_query_t::LOG请求的过程与pg_query_t::INFO的过程是一样的。


如下是此一阶段的日志片段：
{% highlight string %}
50862:2020-09-11 14:10:20.031154 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetLog
50863:2020-09-11 14:10:20.031161 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
50865:2020-09-11 14:10:20.031168 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting osd.3 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223)
50873:2020-09-11 14:10:20.031192 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting newest update on osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223)
50875:calc_acting primary is osd.3 with 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223)
50876: osd.2 (up) accepted 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
50878:2020-09-11 14:10:20.031210 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] actingbackfill is 2,3
50880:2020-09-11 14:10:20.031215 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] choose_acting want [3,2] (== acting) backfill_targets 
50881:2020-09-11 14:10:20.031220 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetLog>: leaving GetLog
50882:2020-09-11 14:10:20.031225 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetLog 0.000070 0 0.000000
50883:2020-09-11 14:10:20.031242 7fba3d124700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2227:1339
{% endhighlight %}

**3.4.1.1 函数PG::choose_acting()**
{% highlight string %}
bool PG::choose_acting(pg_shard_t &auth_log_shard_id,
		       bool restrict_to_up_acting,
		       bool *history_les_bound);
{% endhighlight %}
函数choose_acting()用来计算PG的acting_backfill和backfill_targets两个OSD列表。acting_backfill保存了当前PG的acting列表，包括需要进行Backfill操作的OSD列表；backfill_targets列表保存了需要进行Backfill的OSD列表。

当前acting set为[3,2]，获取到的权威日志信息如下：
{% highlight string %}
50863:2020-09-11 14:10:20.031161 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
50865:2020-09-11 14:10:20.031168 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] calc_acting osd.3 11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223)
{% endhighlight %}
因此这里权威肯定是存在于osd3上。


接着计算出来的actingbackfill为[2,3]，want为[3,2]，acting为[3,2]。


----------
在GetLog()构造函数中选择出来的拥有权威日志的OSD为osd3，就是当前的主OSD，因此直接抛出GotLog()事件，从而进入GetMissing状态。

###### 3.4.3 进入Started/Primary/Peering/GetMissing状态
{% highlight string %}
/*------GetMissing--------*/
PG::RecoveryState::GetMissing::GetMissing(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Peering/GetMissing")
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	assert(!pg->actingbackfill.empty());
	for (set<pg_shard_t>::iterator i = pg->actingbackfill.begin();i != pg->actingbackfill.end();++i) {
		if (*i == pg->get_primary()) continue;
		const pg_info_t& pi = pg->peer_info[*i];
	
		if (pi.is_empty())
			continue;                                // no pg data, nothing divergent
	
		if (pi.last_update < pg->pg_log.get_tail()) {
			dout(10) << " osd." << *i << " is not contiguous, will restart backfill" << dendl;
			pg->peer_missing[*i];
			continue;
		}

		if (pi.last_backfill == hobject_t()) {
			dout(10) << " osd." << *i << " will fully backfill; can infer empty missing set" << dendl;
			pg->peer_missing[*i];
			continue;
		}

		if (pi.last_update == pi.last_complete &&  // peer has no missing
		  pi.last_update == pg->info.last_update) {  // peer is up to date
			// replica has no missing and identical log as us.  no need to
			// pull anything.
			// FIXME: we can do better here.  if last_update==last_complete we
			//        can infer the rest!
			dout(10) << " osd." << *i << " has no missing, identical log" << dendl;
			pg->peer_missing[*i];
			continue;
		}

		// We pull the log from the peer's last_epoch_started to ensure we
		// get enough log to detect divergent updates.
		eversion_t since(pi.last_epoch_started, 0);
		assert(pi.last_update >= pg->info.log_tail);  // or else choose_acting() did a bad thing
		if (pi.log_tail <= since) {
			dout(10) << " requesting log+missing since " << since << " from osd." << *i << dendl;
			context< RecoveryMachine >().send_query(
			  *i,
			  pg_query_t(
				pg_query_t::LOG,
				i->shard, pg->pg_whoami.shard,
				since, pg->info.history,
				pg->get_osdmap()->get_epoch()));
		} else {
			dout(10) << " requesting fulllog+missing from osd." << *i<< " (want since " << since << " < log.tail " << pi.log_tail << ")"<< dendl;
			context< RecoveryMachine >().send_query(
			  *i, pg_query_t(
				pg_query_t::FULLLOG,
				i->shard, pg->pg_whoami.shard,
				pg->info.history, pg->get_osdmap()->get_epoch()));
		}
		peer_missing_requested.insert(*i);
		pg->blocked_by.insert(i->osd);
	}
	
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
进入GetMissing状态后，会遍历actingbackfill列表，当前针对PG 11.4而言其actingbackfill为[2,3]。由于PG 11.4中osd2返回来的pg_info.last_update为0，因此这里直接退出循环遍历。

之后由于在GetInfo()构造函数中，调用build_prior()计算得到PG 11.4需要进行up_thru操作，因此这里产生NeedUpThru事件，从而进入WaitUpThru状态。

如下是此一阶段的日志片段：
{% highlight string %}
50885:2020-09-11 14:10:20.031249 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/GetMissing
50892:2020-09-11 14:10:20.031254 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering/GetMissing>:  still need up_thru update before going active
50894:2020-09-11 14:10:20.031270 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/GetMissing 0.000021 0 0.000000
50895:2020-09-11 14:10:20.031275 7fba3d124700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2227: no change since 2020-09-11 14:10:20.031240
{% endhighlight %}



###### 3.4.4 进入Started/Primary/Peering/WaitUpThru状态
{% highlight string %}
/*------WaitUpThru--------*/
PG::RecoveryState::WaitUpThru::WaitUpThru(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Peering/WaitUpThru")
{
	context< RecoveryMachine >().log_enter(state_name);
}
{% endhighlight %}
如下是此一阶段的日志片段：
{% highlight string %}
50896:2020-09-11 14:10:20.031285 7fba3d124700  5 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] enter Started/Primary/Peering/WaitUpThru
51762:2020-09-11 14:10:20.420033 7fba496c5700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
51764:2020-09-11 14:10:20.420048 7fba496c5700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] flushed
51770:2020-09-11 14:10:20.420077 7fba3d124700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
51772:2020-09-11 14:10:20.420086 7fba3d124700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_peering_event: epoch_sent: 2227 epoch_requested: 2227 FlushedEvt
51773:2020-09-11 14:10:20.420096 7fba3d124700 15 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering]  requeue_ops 
52620:2020-09-11 14:10:20.631849 7fba4fd58700 20 osd.3 2227 11.4 heartbeat_peers 2,3
53389:2020-09-11 14:10:20.745327 7fba4f557700 25 osd.3 2227  sending 11.4 2227:1339
53689:2020-09-11 14:10:21.017381 7fba49ec6700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
53917:2020-09-11 14:10:21.021443 7fba49ec6700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
53919:2020-09-11 14:10:21.021465 7fba49ec6700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] null
54472:2020-09-11 14:10:21.026500 7fba49ec6700 20 osd.3 2228 11.4 heartbeat_peers 2,3
54757:2020-09-11 14:10:21.028048 7fba3d925700 30 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] lock
54759:2020-09-11 14:10:21.028061 7fba3d925700 10 osd.3 pg_epoch: 2227 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_advance_map [3,2]/[3,2] -- 3/3
54761:2020-09-11 14:10:21.028073 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary/Peering>: Peering advmap
54762:2020-09-11 14:10:21.028081 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] adjust_need_up_thru now 2227, need_up_thru now false
54763:2020-09-11 14:10:21.028087 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started>: Started advmap
54764:2020-09-11 14:10:21.028095 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] check_recovery_sources no source osds () went down
54765:2020-09-11 14:10:21.028104 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] handle_activate_map 
54766:2020-09-11 14:10:21.028112 7fba3d925700  7 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] state<Started/Primary>: handle ActMap primary
54767:2020-09-11 14:10:21.028120 7fba3d925700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] publish_stats_to_osd 2227: no change since 2020-09-11 14:10:20.031240
54768:2020-09-11 14:10:21.028129 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] take_waiters
54769:2020-09-11 14:10:21.028136 7fba3d925700  5 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 peering] exit Started/Primary/Peering/WaitUpThru 0.996851 3 0.000229
{% endhighlight %}

1) **AdvMap事件的处理**

由于上面need_up_thru，因此会向OSDMonitor发起up_thru请求，导致osdmap发生改变，因此调用OSD::process_peering_events()、OSD::advance_pg()、 PG::handle_advance_map()来处理这一变化，然后产生AdvMap事件。到此，我们获取到的最新的osdmap版本为e2228.


由于当前所处的状态为WaitUpThru，其本身并不能直接处理AdvMap事件，因此会调用其父状态的相关函数来进行处理，我们来看相应的代码：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Peering::react(const AdvMap& advmap) 
{
	PG *pg = context< RecoveryMachine >().pg;
	dout(10) << "Peering advmap" << dendl;
	if (prior_set.get()->affected_by_map(advmap.osdmap, pg)) {
		dout(1) << "Peering, affected_by_map, going to Reset" << dendl;
		post_event(advmap);
		return transit< Reset >();
	}
	
	pg->adjust_need_up_thru(advmap.osdmap);
	
	return forward_event();
}

boost::statechart::result PG::RecoveryState::Started::react(const AdvMap& advmap)
{
	dout(10) << "Started advmap" << dendl;
	PG *pg = context< RecoveryMachine >().pg;
	pg->check_full_transition(advmap.lastmap, advmap.osdmap);
	if (pg->should_restart_peering(
	  advmap.up_primary,
	  advmap.acting_primary,
	  advmap.newup,
	  advmap.newacting,
	  advmap.lastmap,
	  advmap.osdmap)) {
		dout(10) << "should_restart_peering, transitioning to Reset" << dendl;
		post_event(advmap);
		return transit< Reset >();
	}
	pg->remove_down_peer_info(advmap.osdmap);
	return discard_event();
}

// true if the given map affects the prior set
bool PG::PriorSet::affected_by_map(const OSDMapRef osdmap, const PG *debug_pg) const
{
}

{% endhighlight %}
在当前阶段，prior_set->probe为[2,3]， prior_set->down为[]，prior_set->blocked_by为{}。

PG::PriorSet::affected_by_map()函数的作用是判断指定的osdmap会不会影响当前的prior_set，如果返回true，表明会影响，此时应该进入Reset状态，重新peering；否则返回false。

在这里针对PG 11.4，osdmap e2228并不会影响到当前的prior_set，因此调用PG::adjust_need_up_thru()来调整need_up_thru:
{% highlight string %}
bool PG::adjust_need_up_thru(const OSDMapRef osdmap)
{
	epoch_t up_thru = get_osdmap()->get_up_thru(osd->whoami);
	if (need_up_thru &&
	  up_thru >= info.history.same_interval_since) {
		dout(10) << "adjust_need_up_thru now " << up_thru << ", need_up_thru now false" << dendl;
		need_up_thru = false;
		return true;
	}
	return false;
}
{% endhighlight %}
由于当前up_thru为e2227，而info.history.same_interval_since为e2223，因此这里need_up_thru的值设置为false。


之后AdvMap事件会继续向上抛出，调用Started::react(const AdvMap&)来进行处理。这里并不需要重新触发Peering。

2) **处理ActMap事件**

收到了osdmap变动的消息，处理完成AdvMap事件后，接着就会产生一个ActMap事件，如下是对该事件的处理：
{% highlight string %}
boost::statechart::result PG::RecoveryState::WaitUpThru::react(const ActMap& am)
{
	PG *pg = context< RecoveryMachine >().pg;
	if (!pg->need_up_thru) {
		post_event(Activate(pg->get_osdmap()->get_epoch()));
	}
	return forward_event();
}

boost::statechart::result PG::RecoveryState::Primary::react(const ActMap&)
{
	dout(7) << "handle ActMap primary" << dendl;
	PG *pg = context< RecoveryMachine >().pg;
	pg->publish_stats_to_osd();
	pg->take_waiters();
	return discard_event();
}
{% endhighlight %}
上面我们可以看到，如果pg->need_up_thru为false，那么产生Activate()事件，从而进入Active状态。这里针对PG 11.4而言，其并不需要再一次进行up_thru了，因此这里进入Active状态。

### 3.5 进入Active状态
{% highlight string %}
/*---------Active---------*/
PG::RecoveryState::Active::Active(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Active"),
    remote_shards_to_reserve_recovery(
      unique_osd_shard_set(
	context< RecoveryMachine >().pg->pg_whoami,
	context< RecoveryMachine >().pg->actingbackfill)),
    remote_shards_to_reserve_backfill(
      unique_osd_shard_set(
	context< RecoveryMachine >().pg->pg_whoami,
	context< RecoveryMachine >().pg->backfill_targets)),
    all_replicas_activated(false)
{
	context< RecoveryMachine >().log_enter(state_name);
	
	PG *pg = context< RecoveryMachine >().pg;
	
	assert(!pg->backfill_reserving);
	assert(!pg->backfill_reserved);
	assert(pg->is_primary());
	dout(10) << "In Active, about to call activate" << dendl;
	pg->start_flush(
		context< RecoveryMachine >().get_cur_transaction(),
		context< RecoveryMachine >().get_on_applied_context_list(),
		context< RecoveryMachine >().get_on_safe_context_list());

	pg->activate(*context< RecoveryMachine >().get_cur_transaction(),
		pg->get_osdmap()->get_epoch(),
		*context< RecoveryMachine >().get_on_safe_context_list(),
		*context< RecoveryMachine >().get_query_map(),
		context< RecoveryMachine >().get_info_map(),
		context< RecoveryMachine >().get_recovery_ctx());
	
	// everyone has to commit/ack before we are truly active
	pg->blocked_by.clear();
	for (set<pg_shard_t>::iterator p = pg->actingbackfill.begin();p != pg->actingbackfill.end();	++p) {
		if (p->shard != pg->pg_whoami.shard) {
			pg->blocked_by.insert(p->shard);
		}
	}
	pg->publish_stats_to_osd();
	dout(10) << "Activate Finished" << dendl;
}
{% endhighlight %}
在上面Active状态的构造函数中，调用pg->activate()来激活PG，如下是此一阶段的一个日志片段：
{% highlight string %}
54772:2020-09-11 14:10:21.028162 7fba3d925700  5 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] enter Started/Primary/Active
54773:2020-09-11 14:10:21.028170 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2224 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] state<Started/Primary/Active>: In Active, about to call activate
54774:2020-09-11 14:10:21.028179 7fba3d925700 20 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate - purged_snaps [] cached_removed_snaps []
54775:2020-09-11 14:10:21.028186 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate - snap_trimq []
54776:2020-09-11 14:10:21.028193 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate - no missing, moving last_complete 201'1 -> 201'1
54777:2020-09-11 14:10:21.028200 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate peer osd.2 11.4( DNE empty local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0)
54778:2020-09-11 14:10:21.028213 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate peer osd.2 sending log((0'0,201'1], crt=201'1)
54780:2020-09-11 14:10:21.028240 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] activate peer osd.2 11.4( DNE v 201'1 lc 0'0 (0'0,201'1] local-les=0 n=0 ec=0 les/c/f 0/0/0 0/0/0) missing missing(1)
54781:2020-09-11 14:10:21.028251 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] needs_recovery osd.2 has 1 missing
54782:2020-09-11 14:10:21.028258 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] add_batch_sources_info: adding sources in batch 1
54783:2020-09-11 14:10:21.028265 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] build_might_have_unfound
54784:2020-09-11 14:10:21.028274 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] _calc_past_interval_range: already have past intervals back to 2224
54785:2020-09-11 14:10:21.028282 7fba3d925700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 inactive] build_might_have_unfound: built 2
54786:2020-09-11 14:10:21.028288 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 degraded] activate - starting recovery
54787:2020-09-11 14:10:21.028299 7fba3d925700 10 osd.3 2228 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 degraded]
54789:2020-09-11 14:10:21.028307 7fba3d925700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] publish_stats_to_osd 2228:1340
54790:2020-09-11 14:10:21.028315 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] state<Started/Primary/Active>: Activate Finished
{% endhighlight %}

状态Active的构造函数里处理过程如下：

1） 在构造函数里初始化了remote_shards_to_reserve_recovery和remote_shards_to_reserve_backfill，需要Recovery操作和Backfill操作的OSD；

2） 调用函数pg->start_flush()来完成相关数据的flush工作；

3) 调用函数pg->activate()完成最后的激活工作。

此时，由于```PG 11.4```当前的映射为[3,2]，但是osd2缺失了一个object，因此这里显示的状态为```inactive```.

>注： 进入Active状态后，默认进入其初始子状态Started/Primary/Active/Activating

###### 3.5.1 MissingLoc
在讲解pg->activate()之前，我们先介绍一下其中使用到的MissingLoc。类MIssingLoc用来记录处于missing状态对象的位置，也就是缺失对象的正确版本分别在哪些OSD上。恢复时就去这些OSD上去拉取正确对象的对象数据：
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

* add_active_missing()函数： 用于把一个副本中的所有缺失对象添加到MissingLoc的needs_recovery_map结构里
{% highlight string %}
void add_active_missing(const pg_missing_t &missing) {
	...
}
{% endhighlight %}

* add_source_info()函数： 用于计算每个缺失对象是否在本OSD上
{% highlight string %}
/// Adds info about a possible recovery source
bool add_source_info(
	pg_shard_t source,               ///< [in] source
	const pg_info_t &oinfo,         ///< [in] info
	const pg_missing_t &omissing,   ///< [in] (optional) missing
	bool sort_bitwise,             ///< [in] local sort bitwise (vs nibblewise)
	ThreadPool::TPHandle* handle   ///< [in] ThreadPool handle
	); 
{% endhighlight %}

具体实现如下：

遍历needs_recovery_map里的所有对象，对每个对象做如下处理：

1） 如果oinfo.last_update < need(所缺失的对象版本），就跳过；

2） 如果该PG正常的last_backfill指针小于MAX值，说明还处于Backfill阶段，但是sort_bitwise不正确，跳过；

3） 如果该对象大于last_backfill，显然该对象不存在，跳过；

4） 如果该对象大于last_complete，说明该对象或者是上次Peering之后缺失的对象，还没有来得及修复；或者是新创建的对象。检查如果missing记录已存在，就是上次缺失的对象，直接跳过；否则就是新创建的对象，存在该OSD中；

5）经过上述检查后，确认该对象在本OSD上，在missing_loc添加该对象的location为本OSD。

###### 3.5.2 activate操作
PG::activate()函数是主OSD进入Active状态后执行的第一步操作：
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
	
}
{% endhighlight %}
该函数完成以下功能：

* 更新一些pg_info的参数信息

* 给replica发消息，激活副本PG；

* 计算MissingLoc，也就是缺失对象分布在哪些OSD上，用于后续的恢复；

具体的处理过程如下：

1） 如果需要客户回答，就把PG添加到replay_queue队列里；

2） 更新info.last_epoch_started变量，info.last_epoch_started指的是本OSD在完成目前Peering进程后的更新，而info.history.last_epoch_started是PG的所有OSD都确认完成Peering的更新。

3） 更新一些相关的字段；

4） 注册C_PG_ActivateCommitted()回调函数，该函数最终完成activate的工作；

5) 初始化snap_trimq快照相关的变量；

6） 设置info.last_complete指针：

* 如果missing.num_missing()等于0，表明处于clean状态。直接更新info.last_complete等于info.last_update，并调用pg_log.reset_recovery_pointers()调整log的complete_to指针；

* 否则，如果有需要恢复的对象，就调用函数pg_log.activate_not_complete(info)，设置info.last_complete为缺失的第一个对象的前一个版本。

7） 以下都是主OSD的操作，给每个从OSD发送MOSDPGLog类型的消息，激活该PG的从OSD上的副本。分别对应三种不同处理：
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
	for (set<pg_shard_t>::iterator i = actingbackfill.begin();i != actingbackfill.end();++i) {

		...

		if (pi.last_update == info.last_update && !force_restart_backfill){

			...

		}else if (pg_log.get_tail() > pi.last_update || pi.last_backfill == hobject_t() ||
		force_restart_backfill || (backfill_targets.count(*i) && pi.last_backfill.is_max())) {	

			...
		}else{

		}

		// share past_intervals if we are creating the pg on the replica
		// based on whether our info for that peer was dne() *before*
		// updating pi.history in the backfill block above.
		if (needs_past_intervals)
			m->past_intervals = past_intervals;
		
		// update local version of peer's missing list!
		if (m && pi.last_backfill != hobject_t()) {
			for (list<pg_log_entry_t>::iterator p = m->log.log.begin();p != m->log.log.end();++p)
				if (cmp(p->soid, pi.last_backfill, get_sort_bitwise()) <= 0)
					pm.add_next_event(*p);
		}
		
		if (m) {
			dout(10) << "activate peer osd." << peer << " sending " << m->log << dendl;
			//m->log.print(cout);
			osd->send_message_osd_cluster(peer.osd, m, get_osdmap()->get_epoch());
		}
		
		// peer now has 
		pi.last_update = info.last_update;
		
		// update our missing
		if (pm.num_missing() == 0) {
			pi.last_complete = pi.last_update;
			dout(10) << "activate peer osd." << peer << " " << pi << " uptodate" << dendl;
		} else {
			dout(10) << "activate peer osd." << peer << " " << pi << " missing " << pm << dendl;
		}

	}
}
{% endhighlight %}

* 如果pi.last_update等于info.last_update，这种情况下，该OSD本身就是clean的，不需要给该OSD发送其他信息。添加到activator_map只发送pg_info来激活从OSD。其最终的执行在PeeringWQ的线程执行完状态机的事件处理后，在函数OSD::dispatch_context()里调用OSD::do_info()函数实现；

* 需要Backfill操作的OSD，发送pg_info，以及osd_min_pg_log_entries数量的PG日志；

* 需要Recovery操作的OSD，发送pg_info，以及从缺失的日志；

8） 设置MissingLoc，也就是统计缺失的对象，以及缺失的对象所在的OSD，核心就是调用MissingLoc的add_source_info()函数，见MissingLoc的相关分析；
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

	// if primary..
	if (is_primary()) {

		...

		// Set up missing_loc
		set<pg_shard_t> complete_shards;
		for (set<pg_shard_t>::iterator i = actingbackfill.begin();i != actingbackfill.end();++i) {
			if (*i == get_primary()) {
				missing_loc.add_active_missing(missing);
				if (!missing.have_missing())
					complete_shards.insert(*i);
			} else {
				assert(peer_missing.count(*i));
				missing_loc.add_active_missing(peer_missing[*i]);
				if (!peer_missing[*i].have_missing() && peer_info[*i].last_backfill.is_max())
					complete_shards.insert(*i);
			}
		}

		...
	}

	...

}
{% endhighlight %}

在这里针对PG 11.4而言，osd2是需要进行恢复的，因此会加入missing_loc列表中。

9) 如果需要恢复，把该PG加入到osd->queue_for_recovery(this)的恢复队列中；
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

	// if primary..
	if (is_primary()) {

		...

		if (needs_recovery()) {
			// If only one shard has missing, we do a trick to add all others as recovery
			// source, this is considered safe since the PGLogs have been merged locally,
			// and covers vast majority of the use cases, like one OSD/host is down for
			// a while for hardware repairing
			if (complete_shards.size() + 1 == actingbackfill.size()) {
				missing_loc.add_batch_sources_info(complete_shards, ctx->handle);
			} else {
				missing_loc.add_source_info(pg_whoami, info, pg_log.get_missing(),
					get_sort_bitwise(), ctx->handle);

				for (set<pg_shard_t>::iterator i = actingbackfill.begin();i != actingbackfill.end();++i) {
					if (*i == pg_whoami) continue;
					dout(10) << __func__ << ": adding " << *i << " as a source" << dendl;
					assert(peer_missing.count(*i));
					assert(peer_info.count(*i));

					missing_loc.add_source_info(
						*i,
						peer_info[*i],
						peer_missing[*i],
						get_sort_bitwise(),
						ctx->handle);
				}
			}

			for (map<pg_shard_t, pg_missing_t>::iterator i = peer_missing.begin();i != peer_missing.end();++i) {
				if (is_actingbackfill(i->first))
					continue;
				assert(peer_info.count(i->first));
				search_for_missing(
					peer_info[i->first],
					i->second,
					i->first,
					ctx);
			}
		
			build_might_have_unfound();
		
			state_set(PG_STATE_DEGRADED);
			dout(10) << "activate - starting recovery" << dendl;
			osd->queue_for_recovery(this);
			if (have_unfound())
				discover_all_missing(query_map);
		}
	}
}
{% endhighlight %}


10） 如果PG当前acting set的size小于该PG所在pool设置的副本size，也就是当前的OSD不够，就标记PG的状态为```PG_STATE_DEGRADED```和```PG_STATE_UNDERSIZED``` ，最后标记PG为```PG_STATE_ACTIVATING```状态；


这里针对PG 11.4而言，osd2的pg_info.last_update为e0，这里会进入最后的else分支，进行Recovery操作： 发送pg_info以及从缺失的日志信息。并把osd2加入Recovery列表，以进行数据恢复。



###### 3.5.3 Primary激活成功的回调
在上面PG::activate()函数中为对应的RecoveryCtx.transaction注册了一个提交的回调函数：
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
	// write pg info, log
	dirty_info = true;
	dirty_big_info = true; // maybe


	t.register_on_complete(
		new C_PG_ActivateCommitted(
		this,
		get_osdmap()->get_epoch(),
		activation_epoch));

	...
}
{% endhighlight %}
对于RecoveryCtx.transaction事务的提交会在如下函数中进行：
{% highlight string %}
void OSD::process_peering_events(
  const list<PG*> &pgs,
  ThreadPool::TPHandle &handle
  )
{
	...

	PG::RecoveryCtx rctx = create_context();
	rctx.handle = &handle;
	for (list<PG*>::const_iterator i = pgs.begin();i != pgs.end();++i) {

		...
		dispatch_context_transaction(rctx, pg, &handle);
	}

	...
	dispatch_context(rctx, 0, curmap, &handle);
}

void OSD::dispatch_context_transaction(PG::RecoveryCtx &ctx, PG *pg,
                                       ThreadPool::TPHandle *handle)
{
	if (!ctx.transaction->empty()) {
		if (!ctx.created_pgs.empty()) {
			ctx.on_applied->add(new C_OpenPGs(ctx.created_pgs, store));
		}

		int tr = store->queue_transaction(
			pg->osr.get(),
			std::move(*ctx.transaction), ctx.on_applied, ctx.on_safe, NULL,
			TrackedOpRef(), handle);

		delete (ctx.transaction);
		assert(tr == 0);
		ctx.transaction = new ObjectStore::Transaction;
		ctx.on_applied = new C_Contexts(cct);
		ctx.on_safe = new C_Contexts(cct);
	}
}
{% endhighlight %}
如下是此一过程的一个日志片段：
{% highlight string %}
57029:2020-09-11 14:10:21.067621 7fba49ec6700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] _activate_committed 2228 peer_activated now 3 last_epoch_started 2224 same_interval_since 2227
{% endhighlight %}


###### 3.5.4 收到从OSD的MOSDPGLog的应答
当收到从OSD发送的对MOSDPGLog的ACK消息后，触发MInfoRec事件，下面这个函数处理该事件：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Active::react(const MInfoRec& infoevt)
{
	PG *pg = context< RecoveryMachine >().pg;
	assert(pg->is_primary());
	
	assert(!pg->actingbackfill.empty());
	// don't update history (yet) if we are active and primary; the replica
	// may be telling us they have activated (and committed) but we can't
	// share that until _everyone_ does the same.
	if (pg->is_actingbackfill(infoevt.from)) {
		dout(10) << " peer osd." << infoevt.from << " activated and committed" << dendl;
		pg->peer_activated.insert(infoevt.from);
		pg->blocked_by.erase(infoevt.from.shard);
		pg->publish_stats_to_osd();

		if (pg->peer_activated.size() == pg->actingbackfill.size()) {
			pg->all_activated_and_committed();
		}
	}

	return discard_event();
}
{% endhighlight %}
处理过程比较简单：检查该请求的源OSD在本PG的actingbackfill列表中，以及在等待列表中删除该OSD。最后检查，当收集到所有从OSD发送的ACK，就调用函数all_activated_and_committed():
{% highlight string %}
/*
 * update info.history.last_epoch_started ONLY after we and all
 * replicas have activated AND committed the activate transaction
 * (i.e. the peering results are stable on disk).
 */
void PG::all_activated_and_committed()
{
  dout(10) << "all_activated_and_committed" << dendl;
  assert(is_primary());
  assert(peer_activated.size() == actingbackfill.size());
  assert(!actingbackfill.empty());
  assert(blocked_by.empty());

  queue_peering_event(
    CephPeeringEvtRef(
      std::make_shared<CephPeeringEvt>(
        get_osdmap()->get_epoch(),
        get_osdmap()->get_epoch(),
        AllReplicasActivated())));
}
{% endhighlight %}
该函数产生一个AllReplicasActivated()事件。

如下是此一过程的一个日志片段：
{% highlight string %}
60468:2020-09-11 14:10:21.964346 7fba45134700 20 osd.3 2228 _dispatch 0x7fba69a9dfe0 pg_info(1 pgs e2228:11.4) v4
60469:2020-09-11 14:10:21.964354 7fba45134700  7 osd.3 2228 handle_pg_info pg_info(1 pgs e2228:11.4) v4 from osd.2
60471:2020-09-11 14:10:21.964414 7fba45134700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] lock
60474:2020-09-11 14:10:21.964472 7fba3d124700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] lock
60475:2020-09-11 14:10:21.964509 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] handle_peering_event: epoch_sent: 2228 epoch_requested: 2228 MInfoRec from 2 info: 11.4( v 201'1 lc 0'0 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223)
60476:2020-09-11 14:10:21.964530 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] state<Started/Primary/Active>:  peer osd.2 activated and committed
60477:2020-09-11 14:10:21.964549 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.028307
60478:2020-09-11 14:10:21.964575 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] all_activated_and_committed
{% endhighlight %}

>注： Replica收到主OSD发送的MOSDPGLog消息，会进入ReplicaAcitve状态，然后也会调用PG::activate()函数，从而也会调用到PG::_activate_committed()函数，在该函数里向主OSD发出ACK响应。

###### 3.5.5 AllReplicasActivated
如下函数用于处理AllReplicasActivated事件：
{% highlight string %}
boost::statechart::result PG::RecoveryState::Active::react(const AllReplicasActivated &evt)
{
	PG *pg = context< RecoveryMachine >().pg;
	all_replicas_activated = true;
	
	pg->state_clear(PG_STATE_ACTIVATING);
	pg->state_clear(PG_STATE_CREATING);
	if (pg->acting.size() >= pg->pool.info.min_size) {
		pg->state_set(PG_STATE_ACTIVE);
	} else {
		pg->state_set(PG_STATE_PEERED);
	}
	
	// info.last_epoch_started is set during activate()
	pg->info.history.last_epoch_started = pg->info.last_epoch_started;
	pg->dirty_info = true;
	
	pg->share_pg_info();
	pg->publish_stats_to_osd();
	
	pg->check_local();
	
	// waiters
	if (pg->flushes_in_progress == 0) {
		pg->requeue_ops(pg->waiting_for_peered);
	}
	
	pg->on_activate();
	
	return discard_event();
}
{% endhighlight %}

当所有的replica处于activated状态时，进行如下处理：

1) 取消```PG_STATE_ACTIVATING```和```PG_STATE_CREATING```状态，如果该PG上acting状态的OSD数量大于等于pool的min_size，那么设置该PG为```PG_STATE_ACTIVE```状态，否则设置为```PG_STATE_PEERED```状态；

2） 设置pg->info.history.last_epoch_started为pg.info.last_epoch_started；

3） 调用pg->share_pg_info()函数向actingbackfill列表中的Replicas发送最新的pg_info_t信息；

4） 调用ReplicatedPG::check_local()检查本地的stray objects是否被删除；

5） 如果有读写请求在等待peering操作完成，则把该请求添加到处理队列pg->requeue_ops(pg->waiting_for_peered);

>注： 在ReplicatedPG::do_request()函数中，如果发现当前PG没有peering成功，那么将会将相应的请求保存到waiting_for_peered队列中。详细请参看OSD的读写流程。

6） 调用函数ReplicatedPG::on_activate()，如果需要Recovery操作，触发DoRecovery事件；如果需要Backfill操作，触发RequestBackfill事件；否则，触发AllReplicasRecovered事件。
{% highlight string %}
void ReplicatedPG::on_activate()
{
	// all clean?
	if (needs_recovery()) {
		dout(10) << "activate not all replicas are up-to-date, queueing recovery" << dendl;
		queue_peering_event(
		  CephPeeringEvtRef(
			std::make_shared<CephPeeringEvt>(
			get_osdmap()->get_epoch(),
			get_osdmap()->get_epoch(),
			DoRecovery())));
	} else if (needs_backfill()) {
		dout(10) << "activate queueing backfill" << dendl;
		queue_peering_event(
		  CephPeeringEvtRef(
			std::make_shared<CephPeeringEvt>(
			get_osdmap()->get_epoch(),
			get_osdmap()->get_epoch(),
			RequestBackfill())));
	} else {
		dout(10) << "activate all replicas clean, no recovery" << dendl;
		queue_peering_event(
	  	  CephPeeringEvtRef(
			std::make_shared<CephPeeringEvt>(
			get_osdmap()->get_epoch(),
			get_osdmap()->get_epoch(),
			AllReplicasRecovered())));
	}
	
	publish_stats_to_osd();
	
	if (!backfill_targets.empty()) {
		last_backfill_started = earliest_backfill();
		new_backfill = true;
		assert(!last_backfill_started.is_max());

		dout(5) << "on activate: bft=" << backfill_targets<< " from " << last_backfill_started << dendl;
		for (set<pg_shard_t>::iterator i = backfill_targets.begin();i != backfill_targets.end();++i) {
			dout(5) << "target shard " << *i<< " from " << peer_info[*i].last_backfill<< dendl;
		}
	}
	
	hit_set_setup();
	agent_setup();
}
{% endhighlight %}

在on_activate()函数中，如果需要Recovery操作，触发DoRecovery事件；如果需要Backfill操作，触发RequestBackfill事件；否则，触发AllReplicasRecovered事件。之后再调用hit_set_setup()来初始化Cache Tier需要的hit_set对象，调用agent_setup()来初始化Cache Tier需要的agent对象。

如下是此阶段的一个日志片段：
{% highlight string %}
60481:2020-09-11 14:10:21.964670 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2224/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 activating+degraded] handle_peering_event: epoch_sent: 2228 epoch_requested: 2228 AllReplicasActivated
60482:2020-09-11 14:10:21.964685 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] share_pg_info
60484:2020-09-11 14:10:21.964747 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] publish_stats_to_osd 2228:1341
60485:2020-09-11 14:10:21.964771 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] check_local
60486:2020-09-11 14:10:21.964782 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded]  requeue_ops 
60487:2020-09-11 14:10:21.964794 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] needs_recovery osd.2 has 1 missing
60488:2020-09-11 14:10:21.964805 7fba3d124700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] activate not all replicas are up-to-date, queueing recovery
60489:2020-09-11 14:10:21.964828 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.964740
60490:2020-09-11 14:10:21.964867 7fba3d124700 20 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] hit_set_clear
60491:2020-09-11 14:10:21.964881 7fba3d124700 20 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] agent_stop
{% endhighlight %}


###### 3.5.6 进入Active/Activating状态
其实在Active构造函数执行完成之后，默认就进入了Activating状态，并不需要等到上面副本OSD响应MOSDPGLog请求。如下我们简单给出Activating的构造函数：
{% highlight string %}
/*------Activating--------*/
PG::RecoveryState::Activating::Activating(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Active/Activating")
{
	context< RecoveryMachine >().log_enter(state_name);
}
{% endhighlight %}

###### 3.5.7 进入Active/WaitLocalRecoveryReserved状态

在上面ReplicatedPG::on_activate()函数中，由于PG 11.4的osd2需要恢复，因此会产生DoRecovery()事件，从而进入WaitLocalRecoveryReserved状态，如下是此一阶段的日志片段：
{% highlight string %}
60496:2020-09-11 14:10:21.965057 7fba3d124700  5 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+degraded] enter Started/Primary/Active/WaitLocalRecoveryReserved
60497:2020-09-11 14:10:21.965075 7fba3d124700 15 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] publish_stats_to_osd 2228:1342
60621:2020-09-11 14:10:22.049880 7fba49ec6700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
60891:2020-09-11 14:10:22.051626 7fba49ec6700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
60892:2020-09-11 14:10:22.051634 7fba49ec6700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] null
61652:2020-09-11 14:10:22.055162 7fba3d925700 30 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
61659:2020-09-11 14:10:22.055181 7fba3d925700 10 osd.3 pg_epoch: 2228 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_advance_map [3,2]/[3,2] -- 3/3
61664:2020-09-11 14:10:22.055197 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active advmap
61667:2020-09-11 14:10:22.055208 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started>: Started advmap
61670:2020-09-11 14:10:22.055219 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] check_recovery_sources no source osds (3) went down
61675:2020-09-11 14:10:22.055233 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map 
61680:2020-09-11 14:10:22.055248 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active: handling ActMap
61686:2020-09-11 14:10:22.055273 7fba3d925700 10 osd.3 2229 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
61690:2020-09-11 14:10:22.055286 7fba3d925700  7 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary>: handle ActMap primary
61695:2020-09-11 14:10:22.055305 7fba3d925700 15 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.965071
61699:2020-09-11 14:10:22.055323 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] take_waiters
61703:2020-09-11 14:10:22.055333 7fba3d925700 20 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map: Not dirtying info: last_persisted is 2228 while current is 2229
61706:2020-09-11 14:10:22.055344 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2229 epoch_requested: 2229 NullEvt
61849:2020-09-11 14:10:22.055821 7fba37919700 30 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
61851:2020-09-11 14:10:22.055840 7fba37919700 10 osd.3 2229 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
61855:2020-09-11 14:10:22.055850 7fba37919700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] recovery raced and were queued twice, ignoring!
61859:2020-09-11 14:10:22.055862 7fba37919700 10 osd.3 2229 do_recovery started 0/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
63796:2020-09-11 14:10:22.633040 7fba4fd58700 20 osd.3 2229 11.4 heartbeat_peers 2,3
64613:2020-09-11 14:10:23.233528 7fba49ec6700 30 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
64889:2020-09-11 14:10:23.236332 7fba49ec6700 30 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
64891:2020-09-11 14:10:23.236346 7fba49ec6700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] null
65325:2020-09-11 14:10:23.239004 7fba3d925700 30 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
65329:2020-09-11 14:10:23.239029 7fba3d925700 10 osd.3 pg_epoch: 2229 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_advance_map [3,2]/[3,2] -- 3/3
65331:2020-09-11 14:10:23.239052 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active advmap
65334:2020-09-11 14:10:23.239067 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started>: Started advmap
65337:2020-09-11 14:10:23.239085 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] check_recovery_sources no source osds (3) went down
65340:2020-09-11 14:10:23.239104 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map 
65343:2020-09-11 14:10:23.239121 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active: handling ActMap
65346:2020-09-11 14:10:23.239141 7fba3d925700 10 osd.3 2230 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
65348:2020-09-11 14:10:23.239155 7fba3d925700  7 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary>: handle ActMap primary
65353:2020-09-11 14:10:23.239177 7fba3d925700 15 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.965071
65356:2020-09-11 14:10:23.239201 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] take_waiters
65360:2020-09-11 14:10:23.239214 7fba3d925700 20 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map: Not dirtying info: last_persisted is 2228 while current is 2230
65362:2020-09-11 14:10:23.239229 7fba3d925700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2230 epoch_requested: 2230 NullEvt
65624:2020-09-11 14:10:23.242068 7fba37919700 30 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
65625:2020-09-11 14:10:23.242099 7fba37919700 10 osd.3 2230 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
65626:2020-09-11 14:10:23.242111 7fba37919700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] recovery raced and were queued twice, ignoring!
65627:2020-09-11 14:10:23.242124 7fba37919700 10 osd.3 2230 do_recovery started 0/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
66155:2020-09-11 14:10:23.245392 7fba49ec6700 20 osd.3 2230 11.4 heartbeat_peers 2,3
68073:2020-09-11 14:10:24.319966 7fba49ec6700 30 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
68718:2020-09-11 14:10:24.323051 7fba49ec6700 30 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
68726:2020-09-11 14:10:24.323069 7fba49ec6700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] null
68741:2020-09-11 14:10:24.323112 7fba3d124700 30 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
68748:2020-09-11 14:10:24.323142 7fba3d124700 10 osd.3 pg_epoch: 2230 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_advance_map [3,2]/[3,2] -- 3/3
68752:2020-09-11 14:10:24.323158 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active advmap
68755:2020-09-11 14:10:24.323166 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started>: Started advmap
68758:2020-09-11 14:10:24.323174 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] check_recovery_sources no source osds (3) went down
68760:2020-09-11 14:10:24.323183 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map 
68764:2020-09-11 14:10:24.323189 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary/Active>: Active: handling ActMap
68766:2020-09-11 14:10:24.323199 7fba3d124700 10 osd.3 2231 queue_for_recovery queued pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
68769:2020-09-11 14:10:24.323204 7fba3d124700  7 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] state<Started/Primary>: handle ActMap primary
68772:2020-09-11 14:10:24.323212 7fba3d124700 15 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] publish_stats_to_osd 2228: no change since 2020-09-11 14:10:21.965071
68775:2020-09-11 14:10:24.323220 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] take_waiters
68777:2020-09-11 14:10:24.323226 7fba3d124700 20 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_activate_map: Not dirtying info: last_persisted is 2228 while current is 2231
68778:2020-09-11 14:10:24.323232 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2231 epoch_requested: 2231 NullEvt
71105:2020-09-11 14:10:25.746633 7fba4f557700 25 osd.3 2231  sending 11.4 2228:1342
71198:2020-09-11 14:10:25.789587 7fba37919700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
71199:2020-09-11 14:10:25.789593 7fba37919700 10 osd.3 2231 do_recovery starting 1 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
71200:2020-09-11 14:10:25.789598 7fba37919700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] recovery raced and were queued twice, ignoring!
71201:2020-09-11 14:10:25.789602 7fba37919700 10 osd.3 2231 do_recovery started 0/1 on pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded]
71309:2020-09-11 14:10:25.797456 7fba35915700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
71315:2020-09-11 14:10:25.797495 7fba3d124700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
71317:2020-09-11 14:10:25.797531 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2228 epoch_requested: 2228 LocalRecoveryReserved
71318:2020-09-11 14:10:25.797542 7fba3d124700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] exit Started/Primary/Active/WaitLocalRecoveryReserved 3.832485 10 0.000382
{% endhighlight %}

WaitLocalRecoveryReserved构造函数如下：
{% highlight string %}
PG::RecoveryState::WaitLocalRecoveryReserved::WaitLocalRecoveryReserved(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Active/WaitLocalRecoveryReserved")
	{
	context< RecoveryMachine >().log_enter(state_name);
	PG *pg = context< RecoveryMachine >().pg;
	pg->state_set(PG_STATE_RECOVERY_WAIT);
	pg->osd->local_reserver.request_reservation(
		pg->info.pgid,
		new QueuePeeringEvt<LocalRecoveryReserved>(
			pg, pg->get_osdmap()->get_epoch(),
			LocalRecoveryReserved()),
		pg->get_recovery_priority());

	pg->publish_stats_to_osd();
}
{% endhighlight %}
在WaitLocalRecoveryReserved构造函数中，首先设置PG的状态为```Recovery wait```，之后调用AsyncReserver::request_reservation()来请求分配Recovery的资源。

>注1： 每一次PG状态发生改变，都会调用pg::publish_stats_to_osd()来告知osd。

>注2： 上面的日志片段中又出现了多次的handle_advance_map()，可能的原因是其他PG申请pg_temp引起的，这里对于PG 11.4仅仅只会在OSD::advance_pg()中影响pg_epoch，我们可以暂时忽略
{% highlight string %}
bool OSD::advance_pg(
  epoch_t osd_epoch, PG *pg,
  ThreadPool::TPHandle &handle,
  PG::RecoveryCtx *rctx,
  set<boost::intrusive_ptr<PG> > *new_pgs)
{
	service.pg_update_epoch(pg->info.pgid, lastmap->get_epoch());
}
{% endhighlight %}

1） **函数AsyncReserver::request_reservation()**
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
可以看到request_reservation()仅仅是把要获取资源的请求放入队列。

2） **LocalRecoveryReserved事件**
{% highlight string %}
struct WaitLocalRecoveryReserved : boost::statechart::state< WaitLocalRecoveryReserved, Active >, NamedState {
	typedef boost::mpl::list <
		boost::statechart::transition< LocalRecoveryReserved, WaitRemoteRecoveryReserved >
	> reactions;
	> 
	explicit WaitLocalRecoveryReserved(my_context ctx);
	void exit();
};
{% endhighlight %}
当Recovery资源预约好之后，就会产生LocalRecoveryReserved事件，然后直接进入WaitRemoteRecoveryReserved状态。



###### 3.5.8 进入Active/WaitRemoteRecoveryReserved状态
在预约好本地的Recovery资源后，还需要预约远程OSD上的Recovery资源。如下是此一阶段的一个日志片段：
{% highlight string %}
71320:2020-09-11 14:10:25.797552 7fba3d124700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] enter Started/Primary/Active/WaitRemoteRecoveryReserved
71375:2020-09-11 14:10:26.249869 7fba46937700 25 osd.3 2231  ack on 11.4 2228:1342
71484:2020-09-11 14:10:26.635094 7fba4fd58700 20 osd.3 2231 11.4 heartbeat_peers 2,3
94065:2020-09-11 14:13:14.601725 7fba45134700 20 osd.3 2231 _dispatch 0x7fba6c134b40 MRecoveryReserve GRANT  pgid: 11.4, query_epoch: 2231 v2
94067:2020-09-11 14:13:14.601753 7fba45134700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
94070:2020-09-11 14:13:14.601819 7fba3d124700 30 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] lock
94071:2020-09-11 14:13:14.601855 7fba3d124700 10 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] handle_peering_event: epoch_sent: 2231 epoch_requested: 2231 RemoteRecoveryReserved
94072:2020-09-11 14:13:14.601876 7fba3d124700  5 osd.3 pg_epoch: 2231 pg[11.4( v 201'1 (0'0,201'1] local-les=2228 n=1 ec=132 les/c/f 2228/2224/0 2226/2227/2223) [3,2] r=0 lpr=2227 pi=2223-2226/2 crt=201'1 lcod 0'0 mlcod 0'0 active+recovery_wait+degraded] exit Started/Primary/Active/WaitRemoteRecoveryReserved 168.804323 1 0.000031
{% endhighlight %}
如下是WaitRemoteRecoveryReserved构造函数：
{% highlight string %}
PG::RecoveryState::WaitRemoteRecoveryReserved::WaitRemoteRecoveryReserved(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Active/WaitRemoteRecoveryReserved"),
    remote_recovery_reservation_it(context< Active >().remote_shards_to_reserve_recovery.begin())
{
	context< RecoveryMachine >().log_enter(state_name);
	post_event(RemoteRecoveryReserved());
}
{% endhighlight %}
构造函数中产生一个RemoteRecoveryReserved()事件。

1） **RemoteRecoveryReserved事件处理**
{% highlight string %}
boost::statechart::result
PG::RecoveryState::WaitRemoteRecoveryReserved::react(const RemoteRecoveryReserved &evt) {
	PG *pg = context< RecoveryMachine >().pg;
	
	if (remote_recovery_reservation_it != context< Active >().remote_shards_to_reserve_recovery.end()) {
		assert(*remote_recovery_reservation_it != pg->pg_whoami);
		ConnectionRef con = pg->osd->get_con_osd_cluster(
		remote_recovery_reservation_it->osd, pg->get_osdmap()->get_epoch());
		if (con) {
			pg->osd->send_message_osd_cluster(
			new MRecoveryReserve(
			  MRecoveryReserve::REQUEST,
			  spg_t(pg->info.pgid.pgid, remote_recovery_reservation_it->shard),
			  pg->get_osdmap()->get_epoch()),
			con.get());
		}
		++remote_recovery_reservation_it;
	} else {
		post_event(AllRemotesReserved());
	}
	return discard_event();
}

PG::RecoveryState::Active::Active(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/Primary/Active"),
    remote_shards_to_reserve_recovery(
      unique_osd_shard_set(
	context< RecoveryMachine >().pg->pg_whoami,
	context< RecoveryMachine >().pg->actingbackfill)),
    remote_shards_to_reserve_backfill(
      unique_osd_shard_set(
	context< RecoveryMachine >().pg->pg_whoami,
	context< RecoveryMachine >().pg->backfill_targets)),
    all_replicas_activated(false)
{
	...
}
{% endhighlight %}
在我们进入Active状态的时候（参看Active构造函数），就把需要进行Recovery操作的添加到了Active::remote_shards_to_reserve_recovery中了。这里对于PG 11.4而言，需要向osd.2发送MRecoveryReserve::REQUEST，以获取Recovery所需要的资源。

2) **对MRecoveryReserve::REQUEST请求的处理**

对于PG 11.4而言，osd2接收到MRecoveryReserve::REQUEST后，处理流程如下：
{% highlight string %}
void OSD::dispatch_op(OpRequestRef op){
	switch (op->get_req()->get_type()) {

		...

		case MSG_OSD_RECOVERY_RESERVE:
   		  handle_pg_recovery_reserve(op);
   		  break;
	}
}

void OSD::handle_pg_recovery_reserve(OpRequestRef op)
{
	MRecoveryReserve *m = static_cast<MRecoveryReserve*>(op->get_req());
	assert(m->get_type() == MSG_OSD_RECOVERY_RESERVE);
	
	if (!require_osd_peer(op->get_req()))
		return;
	if (!require_same_or_newer_map(op, m->query_epoch, false))
		return;
	
	PG::CephPeeringEvtRef evt;
	if (m->type == MRecoveryReserve::REQUEST) {
		evt = PG::CephPeeringEvtRef(
			new PG::CephPeeringEvt(
			  m->query_epoch,
			  m->query_epoch,
			  PG::RequestRecovery()));
	} else if (m->type == MRecoveryReserve::GRANT) {
		evt = PG::CephPeeringEvtRef(
			new PG::CephPeeringEvt(
			  m->query_epoch,
			  m->query_epoch,
			  PG::RemoteRecoveryReserved()));
	} else if (m->type == MRecoveryReserve::RELEASE) {
		evt = PG::CephPeeringEvtRef(
			new PG::CephPeeringEvt(
			  m->query_epoch,
			  m->query_epoch,
			  PG::RecoveryDone()));
	} else {
		assert(0);
	}
	
	if (service.splitting(m->pgid)) {
		peering_wait_for_split[m->pgid].push_back(evt);
		return;
	}
	
	PG *pg = _lookup_lock_pg(m->pgid);
	if (!pg) {
		dout(10) << " don't have pg " << m->pgid << dendl;
		return;
	}
	
	pg->queue_peering_event(evt);
	pg->unlock();
}
{% endhighlight %}
handle_pg_recovery_reserve()函数产生RequestRecovery()事件。

对于PG 11.4的osd2而言，在进入ReplicaActive状态后默认会进入其子状态RepNotRecovering，因此这里是RepNotRecovering处理RequestRecovery事件：
{% highlight string %}
struct RepNotRecovering : boost::statechart::state< RepNotRecovering, ReplicaActive>, NamedState {
	typedef boost::mpl::list<
		boost::statechart::custom_reaction< RequestBackfillPrio >,
		boost::statechart::transition< RequestRecovery, RepWaitRecoveryReserved >,
		boost::statechart::transition< RecoveryDone, RepNotRecovering >  // for compat with pre-reservation peers
	> reactions;
	> 
	explicit RepNotRecovering(my_context ctx);
	boost::statechart::result react(const RequestBackfillPrio &evt);
	void exit();
};

{% endhighlight %}
RepNotRecovering接收到```RequestRecovery```事件直接进入RepWaitRecoveryReserved状态：
{% highlight string %}
/*---RepWaitRecoveryReserved--*/
PG::RecoveryState::RepWaitRecoveryReserved::RepWaitRecoveryReserved(my_context ctx)
  : my_base(ctx),
    NamedState(context< RecoveryMachine >().pg->cct, "Started/ReplicaActive/RepWaitRecoveryReserved")
{
	context< RecoveryMachine >().log_enter(state_name);
	PG *pg = context< RecoveryMachine >().pg;
	
	pg->osd->remote_reserver.request_reservation(
		pg->info.pgid,
		new QueuePeeringEvt<RemoteRecoveryReserved>(
		  pg, pg->get_osdmap()->get_epoch(),
		  RemoteRecoveryReserved()),
		pg->get_recovery_priority());
}
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
在RepWaitRecoveryReserved构造函数中调用request_reservation()预约Recovery资源，预约成功会回调QueuePeeringEvt，产生RemoteRecoveryReserved事件。

如下是对RemoteRecoveryReserved事件的处理：
{% highlight string %}
boost::statechart::result
PG::RecoveryState::RepWaitRecoveryReserved::react(const RemoteRecoveryReserved &evt)
{
	PG *pg = context< RecoveryMachine >().pg;
	pg->osd->send_message_osd_cluster(
		pg->primary.osd,
		new MRecoveryReserve(
		  MRecoveryReserve::GRANT,
		  spg_t(pg->info.pgid.pgid, pg->primary.shard),
		  pg->get_osdmap()->get_epoch()),
		pg->get_osdmap()->get_epoch());

	return transit<RepRecovering>();
}
{% endhighlight %}
上面函数中会向主OSD（对于PG 11.4而言其主OSD为osd3)发送MRecoveryReserve::GRANT，然后自己直接进入RepRecovering状态。

3) **主OSD对MRecoveryReserve::GRANT的处理**
{% highlight string %}
void OSD::dispatch_op(OpRequestRef op){
	switch (op->get_req()->get_type()) {

		...

		case MSG_OSD_RECOVERY_RESERVE:
   		  handle_pg_recovery_reserve(op);
   		  break;
	}
}

void OSD::handle_pg_recovery_reserve(OpRequestRef op)
{
	MRecoveryReserve *m = static_cast<MRecoveryReserve*>(op->get_req());
	assert(m->get_type() == MSG_OSD_RECOVERY_RESERVE);
	
	...
	else if (m->type == MRecoveryReserve::GRANT) {
		evt = PG::CephPeeringEvtRef(
			new PG::CephPeeringEvt(
			  m->query_epoch,
			  m->query_epoch,
			  PG::RemoteRecoveryReserved()));
	}
	...
}
{% endhighlight %}
主OSD收到响应后，构造一个```RemoteRecoveryReserved```事件。如下是对该事件的处理：
{% highlight string %}
boost::statechart::result
PG::RecoveryState::WaitRemoteRecoveryReserved::react(const RemoteRecoveryReserved &evt) {
	PG *pg = context< RecoveryMachine >().pg;
	
	if (remote_recovery_reservation_it != context< Active >().remote_shards_to_reserve_recovery.end()) {
		assert(*remote_recovery_reservation_it != pg->pg_whoami);
		ConnectionRef con = pg->osd->get_con_osd_cluster(
		remote_recovery_reservation_it->osd, pg->get_osdmap()->get_epoch());
		if (con) {
			pg->osd->send_message_osd_cluster(
			new MRecoveryReserve(
			  MRecoveryReserve::REQUEST,
			  spg_t(pg->info.pgid.pgid, remote_recovery_reservation_it->shard),
			  pg->get_osdmap()->get_epoch()),
			con.get());
		}
		++remote_recovery_reservation_it;
	} else {
		post_event(AllRemotesReserved());
	}
	return discard_event();
}

{% endhighlight %}
可以看到，这里如果所有Remote资源都预约成功，则会产生一个AllRemotesReserved事件，从而进入Recovering状态。




<br />
<br />
<br />

1. [ceph存储 PG的状态机和peering过程](https://blog.csdn.net/skdkjzz/article/details/51579903)

2. [Ceph OSDMap 机制浅析](https://www.jianshu.com/p/8ecd6028f5ff?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)