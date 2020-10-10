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

	...
	
	AdvMap evt(
		osdmap, lastmap, newup, up_primary,
		newacting, acting_primary);
	recovery_state.handle_event(evt, rctx);
	...
}
{% endhighlight %}
这里```PG11.4```对应的up set为[3,2], acting set也为[3]，up_primary为3，acting_primary也为3。之后会产生一个AdvMap事件，交由recovery_state来进行处理，从而触发peering进程。

3) **函数handle_activate_map()**
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
函数handle_activate_map()用于激活

### 2.1 Clean状态对AdvMap事件的处理
由于在当前状态，PG 11.4已经处于clean状态，且osd3对于PG 11.4而言是主OSD，因此接收到AdvMap事件时，处理流程如下：
{% highlight string %}
{% endhighlight }


<br />
<br />
<br />

1. [ceph存储 PG的状态机和peering过程](https://blog.csdn.net/skdkjzz/article/details/51579903)