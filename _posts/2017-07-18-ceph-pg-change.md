---
layout: post
title: OSD从down到out过程中PG的变化情况
tags:
- ceph
categories: ceph
description: ceph pg
---

本文主要讲述ceph OSD从 (in + up)状态到(in + down)状态，再到(out + down)状态这一整个过程中PG的变化情况。
<!-- more --> 

## 1. 集群环境介绍

当前我们有如下12个OSD组成的ceph集群，每个OSD拥有50G的数据硬盘，拥有的总PG数为744个：
<pre>
[root@ceph001-node1 ~]# ceph osd tree
ID  WEIGHT  TYPE NAME                                UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-10 1.69992 failure-domain sata-00                                                     
 -9 1.69992     replica-domain replica-0                                               
 -8 0.49997         host-domain host-group-0-rack-01                                   
 -6 0.49997             host ceph001-node1                                             
  0 0.14999                 osd.0                         up  1.00000          1.00000 
  2 0.14999                 osd.2                         up  1.00000          1.00000 
  9 0.14999                 osd.9                         up  1.00000          1.00000 
  1 0.04999                 osd.1                         up  1.00000          1.00000 
-11 0.59998         host-domain host-group-0-rack-02                                   
 -2 0.59998             host ceph001-node2                                             
  3 0.14999                 osd.3                         up  1.00000          1.00000 
  4 0.14999                 osd.4                         up  1.00000          1.00000 
  5 0.14999                 osd.5                         up  1.00000          1.00000 
 10 0.14999                 osd.10                        up  1.00000          1.00000 
-12 0.59998         host-domain host-group-0-rack-03                                   
 -4 0.59998             host ceph001-node3                                             
  6 0.14999                 osd.6                         up  1.00000          1.00000 
  7 0.14999                 osd.7                         up  1.00000          1.00000 
  8 0.14999                 osd.8                         up  1.00000          1.00000 
 11 0.14999                 osd.11                        up  1.00000          1.00000 
 -1 1.69992 root default                                                               
 -3 0.59998     rack rack-02                                                           
 -2 0.59998         host ceph001-node2                                                 
  3 0.14999             osd.3                             up  1.00000          1.00000 
  4 0.14999             osd.4                             up  1.00000          1.00000 
  5 0.14999             osd.5                             up  1.00000          1.00000 
 10 0.14999             osd.10                            up  1.00000          1.00000 
 -5 0.59998     rack rack-03                                                           
 -4 0.59998         host ceph001-node3                                                 
  6 0.14999             osd.6                             up  1.00000          1.00000 
  7 0.14999             osd.7                             up  1.00000          1.00000 
  8 0.14999             osd.8                             up  1.00000          1.00000 
 11 0.14999             osd.11                            up  1.00000          1.00000 
 -7 0.49997     rack rack-01                                                           
 -6 0.49997         host ceph001-node1                                                 
  0 0.14999             osd.0                             up  1.00000          1.00000 
  2 0.14999             osd.2                             up  1.00000          1.00000 
  9 0.14999             osd.9                             up  1.00000          1.00000 
  1 0.04999             osd.1                             up  1.00000          1.00000 
</pre>
下面我们以osd.0为例来观察其从(in + up)到(out + down)状态这一整个过程PG的变化情况。

## 2. 初始状态

在整个集群处于HEALTH_OK状态并且达到稳定时，我们此时导出集群的PG(这里我们导出pg_stat,up,up_primary,acting,acting_primary这几列)：
{% highlight string %}
ceph pg dump | awk '{print $1,$15,$16,$17,$18}' > pg_origin.txt
{% endhighlight %}

查看导出的PG信息：
<pre>
[root@ceph001-node1 test]# more pg_origin.txt 
version    
stamp    
last_osdmap_epoch    
last_pg_scan    
full_ratio    
nearfull_ratio    
pg_stat up_primary acting acting_primary last_scrub
25.f6 [6,9,5] 6 [6,9,5] 6
13.e2 [3,6,1] 3 [3,6,1] 3
25.f7 [4,6,2] 4 [4,6,2] 4
13.e3 [0,8,3] 0 [0,8,3] 0
25.f4 [8,2,5] 8 [8,2,5] 8
13.e0 [6,9,3] 6 [6,9,3] 6
25.f5 [5,0,11] 5 [5,0,11] 5
13.e1 [9,4,7] 9 [9,4,7] 9
25.f2 [4,9,8] 4 [4,9,8] 4
...
</pre>

## 3. osd.0处于(in + down)状态

这里手动管理osd.0:
{% highlight string %}
service ceph stop osd.0
{% endhighlight %}

此时集群变成了HEALTH_WARN状态，并且可以看到有184个PG处于降级状态：
<pre>
[root@ceph001-node1 ~]# ceph -s
    cluster ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
     health HEALTH_WARN
            184 pgs degraded
            183 pgs stuck unclean
            184 pgs undersized
            recovery 110/1290 objects degraded (8.527%)
            1/12 in osds are down
     monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
            election epoch 42, quorum 0,1,2 ceph001-node1,ceph001-node2,ceph001-node3
     osdmap e594: 12 osds: 11 up, 12 in
      pgmap v22053: 744 pgs, 16 pools, 1532 MB data, 430 objects
            5243 MB used, 594 GB / 599 GB avail
            110/1290 objects degraded (8.527%)
                 560 active+clean
                 184 active+undersized+degraded

[root@ceph001-node1 ~]# ceph -w
....

2017-07-27 10:21:40.495309 mon.0 [INF] osd.0 marked itself down
2017-07-27 10:21:40.635680 mon.0 [INF] osdmap e593: 12 osds: 11 up, 12 in
2017-07-27 10:21:40.714407 mon.0 [INF] pgmap v22048: 744 pgs: 57 stale+active+clean, 687 active+clean; 1532 MB data, 5247 MB used, 594 GB / 599 GB avail
2017-07-27 10:21:41.652633 mon.0 [INF] osdmap e594: 12 osds: 11 up, 12 in
2017-07-27 10:21:41.727710 mon.0 [INF] pgmap v22049: 744 pgs: 57 stale+active+clean, 687 active+clean; 1532 MB data, 5247 MB used, 594 GB / 599 GB avail
2017-07-27 10:21:42.773254 mon.0 [INF] pgmap v22050: 744 pgs: 57 stale+active+clean, 687 active+clean; 1532 MB data, 5247 MB used, 594 GB / 599 GB avail
2017-07-27 10:21:43.891590 mon.0 [INF] pgmap v22051: 744 pgs: 32 active+undersized+degraded, 46 stale+active+clean, 666 active+clean; 1532 MB data, 5247 MB used, 594 GB / 599 GB avail; 21/1290 objects degraded (1.628%)
2017-07-27 10:21:46.298106 mon.0 [INF] pgmap v22052: 744 pgs: 51 active+undersized+degraded, 38 stale+active+clean, 655 active+clean; 1532 MB data, 5247 MB used, 594 GB / 599 GB avail; 30/1290 objects degraded (2.326%)
2017-07-27 10:21:47.368161 mon.0 [INF] pgmap v22053: 744 pgs: 184 active+undersized+degraded, 560 active+clean; 1532 MB data, 5243 MB used, 594 GB / 599 GB avail; 110/1290 objects degraded (8.527%)
2017-07-27 10:21:47.547796 mon.0 [INF] HEALTH_WARN; 184 pgs degraded; 183 pgs stuck unclean; 184 pgs undersized; recovery 110/1290 objects degraded (8.527%); 1/12 in osds are down
</pre>

但是这时ceph还并没有对pg做重映射，我先将此时的pg map导入到另一个文件pg_osd0_indown.txt文件中：
{% highlight string %}
ceph pg dump | awk '{print $1,$15,$16,$17,$18}' > pg_osd0_indown.txt
{% endhighlight %}

接着我们查看osd.0数据目录下PG：
<pre>
[root@ceph001-node1 current]# pwd
/var/lib/ceph/osd/ceph-0/current
[root@ceph001-node1 current]# ls
10.0_head   13.13_head  1.33_head   13.75_head  1.3a_head   13.ec_head  1.73_head   25.32_head  25.50_head  25.8b_TEMP  25.c1_head  25.e_head   5.1_TEMP  commit_op_seq
10.7_head   13.14_head  13.3_head   13.7a_head  13.b4_head  13.ee_head  1.8_head    25.32_TEMP  25.50_TEMP  25.93_head  25.c1_TEMP  25.f3_head  5.3_head  config.txt
1.11_head   13.15_head  13.44_head  13.7f_head  13.b5_head  13.fb_head  1.f_head    25.33_head  25.52_head  25.93_TEMP  25.c9_head  25.f5_head  5.3_TEMP  meta
1.12_head   13.17_head  13.4_head   13.80_head  13.bd_head  13.fd_head  25.10_head  25.33_TEMP  25.5d_head  25.94_head  25.c9_TEMP  25.f5_TEMP  5.4_head  nosnap
11.4_head   13.18_head  13.52_head  13.82_head  13.c3_head  1.40_head   25.12_head  25.36_head  25.5d_TEMP  25.94_TEMP  25.cb_head  25.f8_head  5.4_TEMP  omap
1.17_head   13.1b_head  13.55_head  13.84_head  13.c4_head  14.3_head   25.12_TEMP  25.36_TEMP  25.66_head  25.97_head  25.cb_TEMP  25.f8_TEMP  5.6_head
1.1b_head   13.1e_head  13.59_head  13.90_head  13.c7_head  14.5_head   25.13_head  25.38_head  25.66_TEMP  25.97_TEMP  25.d1_head  25.fe_head  5.6_TEMP
1.1c_head   1.31_head   13.5c_head  13.92_head  13.cc_head  14.7_head   25.14_head  25.38_TEMP  25.69_head  25.9a_head  25.d1_TEMP  25.fe_TEMP  5.7_head
1.1c_TEMP   13.21_head  13.5d_head  13.94_head  13.cf_head  1.48_head   25.14_TEMP  25.39_head  25.69_TEMP  25.9f_head  25.d4_head  25.f_head   5.7_TEMP
1.1d_head   13.26_head  13.5e_head  13.95_head  13.d0_head  1.49_head   25.21_head  25.39_TEMP  25.6a_head  25.aa_head  25.d4_TEMP  25.f_TEMP   6.3_head
1.1f_head   13.28_head  13.5_head   13.96_head  13.d2_head  1.4_head    25.21_TEMP  25.3d_head  25.6a_TEMP  25.aa_TEMP  25.d7_head  3.2_head    7.1_head
1.23_head   13.29_head  13.61_head  13.9e_head  13.d6_head  1.51_head   25.22_head  25.3d_TEMP  25.72_head  25.ac_head  25.d7_TEMP  3.3_head    7.3_head
1.28_head   13.2b_head  13.67_head  13.9_head   13.d8_head  1.54_head   25.22_TEMP  25.3f_head  25.72_TEMP  25.ac_TEMP  25.d_head   3.6_head    7.5_head
1.2b_head   13.2e_head  13.6a_head  13.a5_head  13.dc_head  15.5_head   25.2b_head  25.3f_TEMP  25.7d_head  25.b3_head  25.d_TEMP   3.6_TEMP    7.6_head
1.2c_head   13.2f_head  13.6b_head  13.a6_head  1.3d_head   1.57_head   25.2b_TEMP  25.47_head  25.7d_TEMP  25.b3_TEMP  25.e8_head  4.0_head    7.7_head
1.2e_head   13.30_head  1.36_head   13.aa_head  13.e3_head  1.5b_head   25.2c_head  25.4b_head  25.7f_head  25.b4_head  25.e8_TEMP  4.0_TEMP    8.4_head
1.2_head    13.35_head  13.70_head  13.ac_head  13.e4_head  1.5f_head   25.2d_head  25.4f_head  25.84_head  25.b4_TEMP  25.ee_head  4.2_head    8.6_head
13.11_head  13.39_head  13.73_head  13.ae_head  13.e5_head  1.6b_head   25.2d_TEMP  25.4f_TEMP  25.8b_head  25.bf_head  25.ee_TEMP  5.1_head    9.5_head
[root@ceph001-node1 current]# ls | grep head | sort -n | wc -l
184
</pre>

可以发现，处于降级状态的PG总数就是osd.0数据目录下的PG数。

## 4. osd.0处于(out+down)状态
在等待5分钟后，monitor会将osd.0标记为out状态：
<pre>
[root@ceph001-node1 current]# ceph -s
    cluster ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
     health HEALTH_WARN
            70 pgs degraded
            70 pgs stuck degraded
            87 pgs stuck unclean
            70 pgs stuck undersized
            70 pgs undersized
            recovery 41/1290 objects degraded (3.178%)
            recovery 7/1290 objects misplaced (0.543%)
     monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
            election epoch 42, quorum 0,1,2 ceph001-node1,ceph001-node2,ceph001-node3
     osdmap e601: 12 osds: 11 up, 11 in; 17 remapped pgs
      pgmap v22097: 744 pgs, 16 pools, 1532 MB data, 430 objects
            5062 MB used, 544 GB / 549 GB avail
            41/1290 objects degraded (3.178%)
            7/1290 objects misplaced (0.543%)
                 657 active+clean
                  70 active+undersized+degraded
                  17 active+remapped

[root@ceph001-node1 ~]# ceph -w
....

2017-07-27 10:26:41.302510 mon.0 [INF] osd.0 out (down for 300.708743)
2017-07-27 10:26:41.359975 mon.0 [INF] osdmap e595: 12 osds: 11 up, 11 in
2017-07-27 10:26:41.435926 mon.0 [INF] pgmap v22061: 744 pgs: 184 active+undersized+degraded, 560 active+clean; 1532 MB data, 4854 MB used, 544 GB / 549 GB avail; 110/1290 objects degraded (8.527%)
2017-07-27 10:26:42.539382 mon.0 [INF] osdmap e596: 12 osds: 11 up, 11 in
2017-07-27 10:26:42.608586 mon.0 [INF] pgmap v22062: 744 pgs: 184 active+undersized+degraded, 560 active+clean; 1532 MB data, 4854 MB used, 544 GB / 549 GB avail; 110/1290 objects degraded (8.527%)
2017-07-27 10:26:43.676203 mon.0 [INF] osdmap e597: 12 osds: 11 up, 11 in
2017-07-27 10:26:43.763114 mon.0 [INF] pgmap v22063: 744 pgs: 184 active+undersized+degraded, 560 active+clean; 1532 MB data, 4854 MB used, 544 GB / 549 GB avail; 110/1290 objects degraded (8.527%)
2017-07-27 10:26:43.696806 osd.7 [INF] 5.4 restarting backfill on osd.2 from (0'0,0'0] MAX to 592'4318
2017-07-27 10:26:43.700017 osd.7 [INF] 5.6 restarting backfill on osd.2 from (0'0,0'0] MAX to 592'6042
2017-07-27 10:26:43.701745 osd.7 [INF] 5.6 restarting backfill on osd.10 from (0'0,0'0] MAX to 592'6042
2017-07-27 10:26:47.553561 mon.0 [INF] HEALTH_WARN; 184 pgs degraded; 184 pgs stuck degraded; 184 pgs stuck unclean; 184 pgs stuck undersized; 184 pgs undersized; recovery 110/1290 objects degraded (8.527%)
2017-07-27 10:26:47.932205 mon.0 [INF] osdmap e598: 12 osds: 11 up, 11 in
</pre>

这个过期时间可以通过如下命令查出：
<pre>
[root@ceph001-node1 current]# ceph daemon mon.ceph001-node1 config show | grep mon_osd_down_out_interval
    "mon_osd_down_out_interval": "300",
</pre>

可以看到300s，刚好是5分钟。然后使用ceph osd tree查看osd map:
<pre>
[root@ceph001-node1 current]# ceph osd tree
ID  WEIGHT  TYPE NAME                                UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-10 1.79993 failure-domain sata-00                                                     
 -9 1.79993     replica-domain replica-0                                               
 -8 0.59998         host-domain host-group-0-rack-01                                   
 -6 0.59998             host ceph001-node1                                             
  0 0.14999                 osd.0                       down        0          1.00000 
  2 0.14999                 osd.2                         up  1.00000          1.00000 
  9 0.14999                 osd.9                         up  1.00000          1.00000 
  1 0.14999                 osd.1                         up  1.00000          1.00000 
-11 0.59998         host-domain host-group-0-rack-02                                   
 -2 0.59998             host ceph001-node2                                             
  3 0.14999                 osd.3                         up  1.00000          1.00000 
  4 0.14999                 osd.4                         up  1.00000          1.00000 
  5 0.14999                 osd.5                         up  1.00000          1.00000 
 10 0.14999                 osd.10                        up  1.00000          1.00000 
-12 0.59998         host-domain host-group-0-rack-03                                   
 -4 0.59998             host ceph001-node3                                             
  6 0.14999                 osd.6                         up  1.00000          1.00000 
  7 0.14999                 osd.7                         up  1.00000          1.00000 
  8 0.14999                 osd.8                         up  1.00000          1.00000 
 11 0.14999                 osd.11                        up  1.00000          1.00000 
 -1 1.79993 root default                                                               
 -3 0.59998     rack rack-02                                                           
 -2 0.59998         host ceph001-node2                                                 
  3 0.14999             osd.3                             up  1.00000          1.00000 
  4 0.14999             osd.4                             up  1.00000          1.00000 
  5 0.14999             osd.5                             up  1.00000          1.00000 
 10 0.14999             osd.10                            up  1.00000          1.00000 
 -5 0.59998     rack rack-03                                                           
 -4 0.59998         host ceph001-node3                                                 
  6 0.14999             osd.6                             up  1.00000          1.00000 
  7 0.14999             osd.7                             up  1.00000          1.00000 
  8 0.14999             osd.8                             up  1.00000          1.00000 
 11 0.14999             osd.11                            up  1.00000          1.00000 
 -7 0.59998     rack rack-01                                                           
 -6 0.59998         host ceph001-node1                                                 
  0 0.14999             osd.0                           down        0          1.00000 
  2 0.14999             osd.2                             up  1.00000          1.00000 
  9 0.14999             osd.9                             up  1.00000          1.00000 
  1 0.14999             osd.1                             up  1.00000          1.00000 
</pre>
通过上面我们发现，osd map中也将osd.0标记为down而且权重为0即out状态。然后mon开始将新的osd map广播到集群内的所有服务器，osd服务器接收到新的osd map之后，与之前的osd map进行比较发现osd.0为out状态，这时所有744个PG中只要有映射到osd.0的PG都会进行remap，然后开始进行数据迁移,而不包含osd.0的PG则不做任何改变，这个结论下面会继续验证：
<pre>
[root@ceph001-node1 ~]# ceph -w
....
2017-07-27 10:26:43.696806 osd.7 [INF] 5.4 restarting backfill on osd.2 from (0'0,0'0] MAX to 592'4318
2017-07-27 10:26:43.700017 osd.7 [INF] 5.6 restarting backfill on osd.2 from (0'0,0'0] MAX to 592'6042
2017-07-27 10:26:43.701745 osd.7 [INF] 5.6 restarting backfill on osd.10 from (0'0,0'0] MAX to 592'6042
2017-07-27 10:26:47.553561 mon.0 [INF] HEALTH_WARN; 184 pgs degraded; 184 pgs stuck degraded; 184 pgs stuck unclean; 184 pgs stuck undersized; 184 pgs undersized; recovery 110/1290 objects degraded (8.527%)
2017-07-27 10:26:47.932205 mon.0 [INF] osdmap e598: 12 osds: 11 up, 11 in
2017-07-27 10:26:47.947244 mon.0 [INF] pgmap v22064: 744 pgs: 131 active+undersized+degraded, 1 active+undersized+degraded+remapped, 1 active+undersized+degraded+remapped+backfilling, 3 active+remapped, 605 active+clean, 3 active+recovering+degraded; 1532 MB data, 4916 MB used, 544 GB / 549 GB avail; 105/1297 objects degraded (8.096%); 27/1297 objects misplaced (2.082%); 35703 kB/s, 10 objects/s recovering
2017-07-27 10:26:48.070421 mon.0 [INF] pgmap v22065: 744 pgs: 112 active+undersized+degraded, 1 active+undersized+degraded+remapped, 1 active+undersized+degraded+remapped+backfilling, 11 active+remapped, 616 active+clean, 3 active+recovering+degraded; 1532 MB data, 4920 MB used, 544 GB / 549 GB avail; 103/1297 objects degraded (7.941%); 32/1297 objects misplaced (2.467%); 51853 kB/s, 14 objects/s recovering
2017-07-27 10:26:43.699258 osd.3 [INF] 5.7 restarting backfill on osd.1 from (0'0,0'0] MAX to 592'3462
2017-07-27 10:26:43.700994 osd.3 [INF] 5.7 restarting backfill on osd.5 from (0'0,0'0] MAX to 592'3462
</pre>

注意，有时候（通常是只有几台主机的“小”集群，比如小型测试集群）out某一个osd可能会使CRUSH进入临界状态,某些PG会一直卡在active + remapped状态。在实验时我们就遇到了这种状态：
<pre>
[root@ceph001-node1 ~]# ceph -w
....
2017-07-27 10:59:05.779734 mon.0 [INF] pgmap v22116: 744 pgs: 70 active+undersized+degraded, 17 active+remapped, 657 active+clean; 1532 MB data, 5062 MB used, 544 GB / 549 GB avail; 41/1290 objects degraded (3.178%); 7/1290 objects misplaced (0.543%)
2017-07-27 10:59:37.916361 mon.0 [INF] pgmap v22117: 744 pgs: 70 active+undersized+degraded, 17 active+remapped, 657 active+clean; 1532 MB data, 5062 MB used, 544 GB / 549 GB avail; 41/1290 objects degraded (3.178%); 7/1290 objects misplaced (0.543%)
2017-07-27 11:00:00.000516 mon.0 [INF] HEALTH_WARN; 70 pgs degraded; 70 pgs stuck degraded; 87 pgs stuck unclean; 70 pgs stuck undersized; 70 pgs undersized; recovery 41/1290 objects degraded (3.178%); recovery 7/1290 objects misplaced (0.543%)
2017-07-27 11:01:05.793245 mon.0 [INF] pgmap v22118: 744 pgs: 70 active+undersized+degraded, 17 active+remapped, 657 active+clean; 1532 MB data, 5062 MB used, 544 GB / 549 GB avail; 41/1290 objects degraded (3.178%); 7/1290 objects misplaced (0.543%)
2017-07-27 11:02:57.223977 mon.0 [INF] pgmap v22119: 744 pgs: 70 active+undersized+degraded, 17 active+remapped, 657 active+clean; 1532 MB data, 5062 MB used, 544 GB / 549 GB avail; 41/1290 objects degraded (3.178%); 7/1290 objects misplaced (0.543%)
2017-07-27 11:02:58.261469 mon.0 [INF] pgmap v22120: 744 pgs: 70 active+undersized+degraded, 17 active+remapped, 657 active+clean; 1532 MB data, 5062 MB used, 544 GB / 549 GB avail; 41/1290 objects degraded (3.178%); 7/1290 objects misplaced (0.543%)
2017-07-27 11:02:59.369553 mon.0 [INF] pgmap v22121: 744 pgs: 70 active+undersized+degraded, 17 active+remapped, 657 active+clean; 1532 MB data, 5062 MB used, 544 GB / 549 GB avail; 41/1290 objects degraded (3.178%); 7/1290 objects misplaced (0.543%)
</pre>

此时，应该把此OSD标记为in，采用如下命令：
{% highlight string %}
ceph osd in {osd-num}
{% endhighlight %}
等到回到最初的状态后，把它的CRUSH MAP权重设置为0，而不是标记为out，采用如下命令：
{% highlight string %}
ceph osd crush reweight osd.{osd-num} 0
{% endhighlight %}

等迁移完成后，在重新out出去：
{% highlight string %}
ceph osd out {osd-num}
{% endhighlight %}

执行后，你可以观察数据迁移过程，应该可以正常结束。把某一个OSD标记为out和将权重改为0的区别在于，前者包含此OSD的桶其权重没变；而后一种情况下桶的权重变了（降低了此OSD的权重）。某些情况下，reweight命令更适合“小”集群。

在我们执行上述命令后，通过ceph -w观察数据迁移：
<pre>
2017-07-27 11:11:15.356801 mon.0 [INF] osdmap e612: 12 osds: 11 up, 12 in
2017-07-27 11:11:15.879497 mon.0 [INF] pgmap v22146: 744 pgs: 6 active+degraded, 3 active+remapped, 724 active+clean, 11 active+recovering+degraded; 1532 MB data, 5231 MB used, 544 GB / 549 GB avail; 57/1311 objects degraded (4.348%); 37/1311 objects misplaced (2.822%); 97263 kB/s, 30 objects/s recovering
2017-07-27 11:11:17.504589 mon.0 [INF] osdmap e613: 12 osds: 11 up, 12 in
2017-07-27 11:11:17.534262 mon.0 [INF] pgmap v22147: 744 pgs: 6 active+degraded, 3 active+remapped, 724 active+clean, 11 active+recovering+degraded; 1532 MB data, 5231 MB used, 544 GB / 549 GB avail; 57/1311 objects degraded (4.348%); 37/1311 objects misplaced (2.822%)
2017-07-27 11:11:18.580482 mon.0 [INF] osdmap e614: 12 osds: 11 up, 12 in
2017-07-27 11:11:18.612214 mon.0 [INF] pgmap v22148: 744 pgs: 6 active+degraded, 3 active+remapped, 724 active+clean, 11 active+recovering+degraded; 1532 MB data, 5231 MB used, 544 GB / 549 GB avail; 57/1311 objects degraded (4.348%); 37/1311 objects misplaced (2.822%)
2017-07-27 11:11:11.020942 osd.8 [INF] 1.57 deep-scrub ok
2017-07-27 11:11:19.654127 mon.0 [INF] pgmap v22149: 744 pgs: 3 active+degraded, 2 active+remapped, 733 active+clean, 6 active+recovering+degraded; 1532 MB data, 5207 MB used, 544 GB / 549 GB avail; 30/1306 objects degraded (2.297%); 32/1306 objects misplaced (2.450%); 25152 kB/s, 3 objects/s recovering
2017-07-27 11:11:20.755809 mon.0 [INF] pgmap v22150: 744 pgs: 2 active+degraded, 2 active+remapped, 739 active+clean, 1 active+recovering+degraded; 1532 MB data, 5239 MB used, 544 GB / 549 GB avail; 10/1306 objects degraded (0.766%); 32/1306 objects misplaced (2.450%); 47253 kB/s, 9 objects/s recovering
2017-07-27 11:11:23.240040 mon.0 [INF] pgmap v22151: 744 pgs: 744 active+clean; 1532 MB data, 5227 MB used, 544 GB / 549 GB avail; 18293 kB/s, 8 objects/s recovering
2017-07-27 11:11:47.601116 mon.0 [INF] HEALTH_WARN; 1/12 in osds are down
2017-07-27 11:11:48.739331 mon.0 [INF] pgmap v22152: 744 pgs: 744 active+clean; 1532 MB data, 5227 MB used, 544 GB / 549 GB avail; 682 B/s rd, 0 B/s wr, 1 op/s
2017-07-27 11:11:49.781253 mon.0 [INF] pgmap v22153: 744 pgs: 744 active+clean; 1532 MB data, 5184 MB used, 544 GB / 549 GB avail; 3317 B/s rd, 0 B/s wr, 5 op/s
2017-07-27 11:11:53.237482 mon.0 [INF] pgmap v22154: 744 pgs: 744 active+clean; 1532 MB data, 5184 MB used, 544 GB / 549 GB avail; 17976 B/s rd, 0 B/s wr, 29 op/s
2017-07-27 11:13:14.797855 mon.0 [INF] pgmap v22155: 744 pgs: 744 active+clean; 1532 MB data, 5196 MB used, 544 GB / 549 GB avail
2017-07-27 11:13:19.175359 mon.0 [INF] pgmap v22156: 744 pgs: 744 active+clean; 1532 MB data, 5196 MB used, 544 GB / 549 GB avail
</pre>
可以看到最后数据迁移完成,集群也重新回到HEALTH_OK状态：
<pre>
[root@ceph001-node1 test]# ceph -s
    cluster ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
     health HEALTH_OK
     monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
            election epoch 42, quorum 0,1,2 ceph001-node1,ceph001-node2,ceph001-node3
     osdmap e615: 12 osds: 11 up, 11 in
      pgmap v22169: 744 pgs, 16 pools, 1532 MB data, 430 objects
            5197 MB used, 544 GB / 549 GB avail
                 744 active+clean
</pre>

此时，OSD个数由12个变成11个，也就是说osd.0的数据已经全部被复制到了集群的其他地方。这时我们将集群的pg导出到pg_osd_outdown.txt文件中：
{% highlight string %}
ceph pg dump | awk '{print $1,$15,$16,$17,$18}' > pg_osd0_outdown.txt
{% endhighlight %}


## 5. 集群PG变化情况分析
此时我们有了3个pg dump文件,分别是：
* pg_origin.txt                      
* pg_osd0_indown.txt                
* pg_osd0_outdown.txt

为了方便比较，我们将这3个文件中不相干字段剔除，只保留pg_stat,up,up_primary,acting,acting_primary这5个字段。其中pg_stat表示pgid；acting set为某一特定PG所映射的OSD；up set是当前PG处理客户端请求时所映射的一组OSD。在大多数情况下acting set与up set是一致的，如果不同则说明ceph在迁移数据或OSD在恢复，也可能此时ceph集群出现了其他未知故障。

下面我们对比pg_origin.txt与pg_osd0_indown.txt文件：
<pre>
[root@ceph001-node1 test]# diff pg_origin.txt pg_osd0_indown.txt -y --suppress-common-lines
13.e3 [0,8,3] 0 [0,8,3] 0                                     | 13.e3 [8,3] 8 [8,3] 8
25.f5 [5,0,11] 5 [5,0,11] 5                                   | 25.f5 [5,11] 5 [5,11] 5
25.f3 [6,4,0] 6 [6,4,0] 6                                     | 25.f3 [6,4] 6 [6,4] 6
13.e4 [5,6,0] 5 [5,6,0] 5                                     | 13.e4 [5,6] 5 [5,6] 5
13.e5 [10,0,8] 10 [10,0,8] 10                                 | 13.e5 [10,8] 10 [10,8] 10
25.fe [0,3,8] 0 [0,3,8] 0                                     | 25.fe [3,8] 3 [3,8] 3
13.ee [8,3,0] 8 [8,3,0] 8                                     | 13.ee [8,3] 8 [8,3] 8
25.f8 [10,0,7] 10 [10,0,7] 10                                 | 25.f8 [10,7] 10 [10,7] 10
13.ec [3,8,0] 3 [3,8,0] 3                                     | 13.ec [3,8] 3 [3,8] 3
13.d2 [3,8,0] 3 [3,8,0] 3                                     | 13.d2 [3,8] 3 [3,8] 3
13.d0 [0,8,3] 0 [0,8,3] 0                                     | 13.d0 [8,3] 8 [8,3] 8
13.d6 [10,0,6] 10 [10,0,6] 10                                 | 13.d6 [10,6] 10 [10,6] 10
25.c1 [0,6,3] 0 [0,6,3] 0                                     | 25.c1 [6,3] 6 [6,3] 6
13.d8 [0,8,5] 0 [0,8,5] 0                                     | 13.d8 [8,5] 8 [8,5] 8
25.cb [6,0,4] 6 [6,0,4] 6                                     | 25.cb [6,4] 6 [6,4] 6
13.dc [0,10,7] 0 [0,10,7] 0                                   | 13.dc [10,7] 10 [10,7] 10
25.c9 [0,10,6] 0 [0,10,6] 0                                   | 25.c9 [10,6] 10 [10,6] 10
25.d7 [8,3,0] 8 [8,3,0] 8                                     | 25.d7 [8,3] 8 [8,3] 8
13.c3 [0,11,4] 0 [0,11,4] 0                                   | 13.c3 [11,4] 11 [11,4] 11
25.d4 [11,5,0] 11 [11,5,0] 11                                 | 25.d4 [11,5] 11 [11,5] 11
13.c7 [5,11,0] 5 [5,11,0] 5                                   | 13.c7 [5,11] 5 [5,11] 5

.....
</pre>
通过上述比较发现，所有的PG中存在映射到OSD.0的PG，都是active + undersized + degraded状态。并且，PG的映射关系也是直接将osd.0剔除了。然后进行计数统计：
<pre>
[root@ceph001-node1 test]# diff pg_origin.txt pg_osd0_indown.txt -y --suppress-common-lines |grep -v '^\s' |  wc -l
184
</pre>
结果刚好等于ceph -w 显示的不正常的PG数，也就是osd.0下面包含的PG总数。

然后我们将osd.0目录下的PG全部导入pg_osd0_data.txt文件：
<pre>
ls /var/lib/ceph/osd/ceph-0/current/ | grep head | sort -n | awk -F '_' '{print $1}' | sort -n > pg_osd0_data.txt
</pre>

然后对比pg_origin.txt与pg_osd0_outdown.txt:
<pre>
[root@ceph001-node1 test]# diff pg_origin.txt pg_osd0_outdown.txt -y --suppress-common-lines | grep -v '^\s' | grep -v '@@' | grep -v 'txt'
13.e3 [0,8,3] 0 [0,8,3] 0                                     | 13.e3 [9,8,3] 9 [9,8,3] 9
25.f5 [5,0,11] 5 [5,0,11] 5                                   | 25.f5 [5,1,11] 5 [5,1,11] 5
13.e1 [9,4,7] 9 [9,4,7] 9                                     | 13.e1 [3,9,7] 3 [3,9,7] 3
25.f3 [6,4,0] 6 [6,4,0] 6                                     | 25.f3 [6,4,2] 6 [6,4,2] 6
13.e7 [1,10,6] 1 [1,10,6] 1                                   | 13.e7 [1,8,3] 1 [1,8,3] 1
13.e4 [5,6,0] 5 [5,6,0] 5                                     | 13.e4 [5,6,2] 5 [5,6,2] 5
13.e5 [10,0,8] 10 [10,0,8] 10                                 | 13.e5 [10,1,8] 10 [10,1,8] 10
25.fe [0,3,8] 0 [0,3,8] 0                                     | 25.fe [1,3,8] 1 [1,3,8] 1
13.ea [2,8,10] 2 [2,8,10] 2                                   | 13.ea [3,8,2] 3 [3,8,2] 3
25.fc [2,7,10] 2 [2,7,10] 2                                   | 25.fc [3,7,2] 3 [3,7,2] 3
25.fd [7,2,3] 7 [7,2,3] 7                                     | 25.fd [7,5,2] 7 [7,5,2] 7
13.ee [8,3,0] 8 [8,3,0] 8                                     | 13.ee [8,3,9] 8 [8,3,9] 8
25.f8 [10,0,7] 10 [10,0,7] 10                                 | 25.f8 [10,6,9] 10 [10,6,9] 10
13.ec [3,8,0] 3 [3,8,0] 3                                     | 13.ec [3,8,1] 3 [3,8,1] 3
25.f9 [1,3,8] 1 [1,3,8] 1                                     | 25.f9 [5,7,2] 5 [5,7,2] 5
</pre>
通过上面我们发现，不管有没有涉及到OSD.0的PG，其最后映射关系都可能发生了改变。

然后我们将pg_origin.txt与pg_osd0_outdown.txt中改变了的PG与osd.0下的PG做比较：
<pre>
[root@ceph001-node1 test]# diff pg_origin.txt pg_osd0_outdown.txt -y --suppress-common-lines | grep -v '^\s' | grep -v '@@' | grep -v 'txt'| awk '{print $1}' | sort -n | diff - pg_osd0_data.txt -y 
1.f                                                             1.f
1.1b                                                            1.1b
1.1c                                                            1.1c
1.1d                                                            1.1d
1.1f                                                            1.1f
1.11                                                            1.11
1.12                                                            1.12
1.13                                                          <
1.14                                                          <
1.15                                                          <
1.17                                                            1.17
1.2                                                             1.2
1.2b                                                            1.2b
....
</pre>
可以看到，这里确实是不管有没有涉及到osd.0的PG，其最后均发生了改变。

值得指出的是，由于我们这里在上面是通过了```ceph osd crush reweight```方式来解除PG一直处于stuck状态。如果osd.0可以直接out出去并且集群可以自动的完成数据迁移的话，则结果会完全不同。此种情况下，PG的迁移只会发生在与osd.0相关的PG上。

这里顺便解释一下backfill和recovering的区别：
* backfill: 需要进行全量恢复操作
* recovering: 需要增量恢复的操作

<br />
<br />
<br />


