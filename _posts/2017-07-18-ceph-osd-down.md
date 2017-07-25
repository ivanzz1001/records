---
layout: post
title: 模拟物理机down掉的情况
tags:
- ceph
categories: ceph
description: ceph故障模拟
---

本文主要讲述在ceph运行过程中，物理机由于断电等原因突然down掉的情况下集群的表现，以及针对可能出现的状况的相应处理方法。


<!-- more -->
* 环境介绍
* 故障模拟
* 故障解决


## 1. 环境介绍

当前我们共有12个OSD，每个OSD用50G硬盘空间，分布在3台宿主机上,同时每台宿主机上还各部署一个monitor。其逻辑拓扑结构与物理拓扑结构如下：
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

## 2. 故障模拟

通常在实际生产环境中，我们会将pool的size设为3，min_size设置为2，而在进行故障模拟的时候我们也是针对这一情况来进行。 实际模拟宿主机突然down掉的情况最好是通过断电源等方式，但这里由于云环境的限制，我们采用手动强制关机的方式来模拟。

### 2.1 准备测试数据

1) 创建存储池

这里创建存储池``.simulator.down``:
{% highlight string %}
sudo ceph osd pool create .simulator.down 256 256
sudo ceph osd pool set .simulator.down crush_ruleset 5
sudo ceph osd pool set .simulator.down size 3
sudo ceph osd pool set .simulator.down min_size 2
rados -p .simulator.down df
{% endhighlight %}
建立后，用如下命令查看：
<pre>
[root@ceph001-node1 ~]# ceph osd dump | grep .simulator.down
pool 25 '.simulator.down' replicated size 3 min_size 2 crush_ruleset 5 object_hash rjenkins pg_num 256 pgp_num 256 last_change 378 flags hashpspool stripe_width 0
</pre>


2) 向存储池中写入数据
{% highlight string %}
rados bench -p .simulator.down 60  write --no-cleanup
{% endhighlight %}


### 2.2 ceph001-node1节点down掉

这里直接将``ceph001-node1``节点进行关机以模拟宿主机down掉的情况：

查看集群的状态：
<pre>
[root@ceph001-node3 ~]# ceph -s
    cluster ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
     health HEALTH_WARN
            744 pgs degraded
            307 pgs stuck degraded
            744 pgs stuck unclean
            307 pgs stuck undersized
            744 pgs undersized
            recovery 430/1290 objects degraded (33.333%)
            4/12 in osds are down
            1 mons down, quorum 1,2 ceph001-node2,ceph001-node3
     monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
            election epoch 26, quorum 1,2 ceph001-node2,ceph001-node3
     osdmap e384: 12 osds: 8 up, 12 in
      pgmap v18247: 744 pgs, 16 pools, 1532 MB data, 430 objects
            5198 MB used, 594 GB / 599 GB avail
            430/1290 objects degraded (33.333%)
                 744 active+undersized+degraded
</pre>

集群的动态监控状况：
<pre>
[root@ceph001-node2 ~]# ceph -w
    cluster ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
     health HEALTH_OK
     monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
            election epoch 24, quorum 0,1,2 ceph001-node1,ceph001-node2,ceph001-node3
     osdmap e378: 12 osds: 12 up, 12 in
      pgmap v18226: 744 pgs, 16 pools, 1532 MB data, 430 objects
            5192 MB used, 594 GB / 599 GB avail
                 744 active+clean

2017-07-25 14:56:04.066435 mon.0 [INF] pgmap v18226: 744 pgs: 744 active+clean; 1532 MB data, 5192 MB used, 594 GB / 599 GB avail
2017-07-25 14:56:35.605112 mon.1 [INF] mon.ceph001-node2 calling new monitor election
2017-07-25 14:56:35.606511 mon.2 [INF] mon.ceph001-node3 calling new monitor election
2017-07-25 14:56:40.667486 mon.1 [INF] mon.ceph001-node2@1 won leader election with quorum 1,2
2017-07-25 14:56:40.711851 mon.1 [INF] HEALTH_WARN; 1 mons down, quorum 1,2 ceph001-node2,ceph001-node3
2017-07-25 14:56:40.723789 mon.1 [INF] monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
2017-07-25 14:56:40.723872 mon.1 [INF] pgmap v18226: 744 pgs: 744 active+clean; 1532 MB data, 5192 MB used, 594 GB / 599 GB avail
2017-07-25 14:56:40.723954 mon.1 [INF] mdsmap e1: 0/0/0 up
2017-07-25 14:56:40.724052 mon.1 [INF] osdmap e378: 12 osds: 12 up, 12 in
2017-07-25 14:56:43.288638 mon.1 [INF] osd.1 10.133.134.211:6804/3306 failed (3 reports from 3 peers after 20.014342 >= grace 20.000000)
2017-07-25 14:56:43.395532 mon.1 [INF] osdmap e379: 12 osds: 11 up, 12 in
2017-07-25 14:56:43.482454 mon.1 [INF] pgmap v18227: 744 pgs: 25 stale+active+clean, 719 active+clean; 1532 MB data, 5192 MB used, 594 GB / 599 GB avail
2017-07-25 14:56:44.539657 mon.1 [INF] osdmap e380: 12 osds: 11 up, 12 in
2017-07-25 14:56:44.698687 mon.1 [INF] pgmap v18229: 744 pgs: 21 stale+active+clean, 11 peering, 712 active+clean; 1532 MB data, 5192 MB used, 594 GB / 599 GB avail
2017-07-25 14:56:48.641278 mon.1 [INF] pgmap v18230: 744 pgs: 9 active+undersized+degraded, 17 stale+active+clean, 11 peering, 707 active+clean; 1532 MB data, 5192 MB used, 594 GB / 599 GB avail; 3/1290 objects degraded (0.233%)
2017-07-25 14:56:49.731555 mon.1 [INF] pgmap v18231: 744 pgs: 75 active+undersized+degraded, 669 active+clean; 1532 MB data, 5193 MB used, 594 GB / 599 GB avail; 42/1290 objects degraded (3.256%)
2017-07-25 14:56:58.637901 mon.1 [INF] pgmap v18232: 744 pgs: 75 active+undersized+degraded, 669 active+clean; 1532 MB data, 5193 MB used, 594 GB / 599 GB avail; 42/1290 objects degraded (3.256%)
2017-07-25 14:56:59.739817 mon.1 [INF] pgmap v18233: 744 pgs: 75 active+undersized+degraded, 669 active+clean; 1532 MB data, 5193 MB used, 594 GB / 599 GB avail; 42/1290 objects degraded (3.256%)
2017-07-25 14:57:13.909836 mon.1 [INF] osd.0 10.133.134.211:6800/1393 failed (49 reports from 8 peers after 47.406060 >= grace 47.384897)
2017-07-25 14:57:14.912197 mon.1 [INF] osdmap e381: 12 osds: 10 up, 12 in
2017-07-25 14:57:14.976806 mon.1 [INF] pgmap v18234: 744 pgs: 75 active+undersized+degraded, 67 stale+active+clean, 602 active+clean; 1532 MB data, 5193 MB used, 594 GB / 599 GB avail; 42/1290 objects degraded (3.256%)
2017-07-25 14:57:15.945309 mon.1 [INF] osdmap e382: 12 osds: 10 up, 12 in
2017-07-25 14:57:15.993132 mon.1 [INF] pgmap v18235: 744 pgs: 75 active+undersized+degraded, 67 stale+active+clean, 602 active+clean; 1532 MB data, 5193 MB used, 594 GB / 599 GB avail; 42/1290 objects degraded (3.256%)
2017-07-25 14:57:20.068080 mon.1 [INF] pgmap v18236: 744 pgs: 106 active+undersized+degraded, 62 stale+active+clean, 576 active+clean; 1532 MB data, 5193 MB used, 594 GB / 599 GB avail; 56/1290 objects degraded (4.341%)
2017-07-25 14:57:21.146178 mon.1 [INF] pgmap v18237: 744 pgs: 307 active+undersized+degraded, 437 active+clean; 1532 MB data, 5195 MB used, 594 GB / 599 GB avail; 183/1290 objects degraded (14.186%)
2017-07-25 14:57:40.712218 mon.1 [INF] HEALTH_WARN; 307 pgs degraded; 142 pgs stuck unclean; 307 pgs undersized; recovery 183/1290 objects degraded (14.186%); 2/12 in osds are down; 1 mons down, quorum 1,2 ceph001-node2,ceph001-node3
2017-07-25 14:57:42.274080 mon.1 [INF] osd.9 10.133.134.211:6812/2598 failed (92 reports from 8 peers after 75.770088 >= grace 75.464902)
2017-07-25 14:57:44.917580 mon.1 [INF] osd.2 10.133.134.211:6808/2147 failed (93 reports from 8 peers after 78.413654 >= grace 76.470944)
2017-07-25 14:57:45.154120 mon.1 [INF] osdmap e383: 12 osds: 8 up, 12 in
2017-07-25 14:57:45.276166 mon.1 [INF] pgmap v18238: 744 pgs: 307 active+undersized+degraded, 136 stale+active+clean, 301 active+clean; 1532 MB data, 5195 MB used, 594 GB / 599 GB avail; 183/1290 objects degraded (14.186%)
2017-07-25 14:57:46.398289 mon.1 [INF] osdmap e384: 12 osds: 8 up, 12 in
2017-07-25 14:57:46.471501 mon.1 [INF] pgmap v18239: 744 pgs: 307 active+undersized+degraded, 136 stale+active+clean, 301 active+clean; 1532 MB data, 5195 MB used, 594 GB / 599 GB avail; 183/1290 objects degraded (14.186%)
2017-07-25 14:57:50.392073 mon.1 [INF] pgmap v18240: 744 pgs: 355 active+undersized+degraded, 116 stale+active+clean, 273 active+clean; 1532 MB data, 5195 MB used, 594 GB / 599 GB avail; 215/1290 objects degraded (16.667%)
2017-07-25 14:57:51.538346 mon.1 [INF] pgmap v18241: 744 pgs: 744 active+undersized+degraded; 1532 MB data, 5198 MB used, 594 GB / 599 GB avail; 430/1290 objects degraded (33.333%)
</pre>

在此过程中我们通过``ceph -s``命令可以看到，集群处于 HEALTH_WARN状态，并且提示有1个monitor及4个OSD已经处于down状态；通过``ceph -w``监控到在宿主机ceph001-node1 突然down掉时集群所做的反应：

1） monitor节点

在集群检测到宿主机down掉后，剩余monitor节点首先迅速进行协商选出新的leader monitor,并提示集群处于HEALTH_WARN状态
<pre>
2017-07-25 14:56:35.605112 mon.1 [INF] mon.ceph001-node2 calling new monitor election
2017-07-25 14:56:35.606511 mon.2 [INF] mon.ceph001-node3 calling new monitor election
2017-07-25 14:56:40.667486 mon.1 [INF] mon.ceph001-node2@1 won leader election with quorum 1,2
2017-07-25 14:56:40.711851 mon.1 [INF] HEALTH_WARN; 1 mons down, quorum 1,2 ceph001-node2,ceph001-node3
</pre>

如上，mon.ceph001-node1被选举作为leader.

2）pg状态

这里我们首先来介绍pg整个生命周期中可能的状态，然后再来分析在一台宿主机down掉时PG的表现：

**```CREATING```**

当你在创建一个pool的时候就会创建指定数量的PG。当集群正在创建PG的时候就处于``creating``状态。一旦一个PG创建成功，处于PG Acting Set中的OSD就会进行peer。而一旦peering完成之后，PG的状态就会变为active + clean，这就代表着ceph客户端可以对PG执行写操作了。

![pg-state](https://ivanzz1001.github.io/records/assets/img/ceph/osd-down/pg-state.jpg)


**```PEERING```**

当ceph正在peering一个PG的时候，目的是要让存储该PG各个副本的OSD之间相互了解到当前PG中所存储的对象和元数据的相应状态。当peering完成之后，各OSD就会针对该PG得出一个一致同意的状态。然而，peering的完成并不代表PG的各个副本之间都有了数据的最新版本。
<pre>
权威历史：
在PG Acting Set所映射的所有OSD均同意写之前，ceph会拒绝对对该PG进行写操作。这就可以确保自上一次peering成功完成之后,
PG acting set中的至少一个有一个成员保存有每次写操作的一份记录。

拥有一份精确的写记录之后，ceph就可以构建出该PG的一份详尽的历史。这样通过这份完整的、有序的操作记录，ceph就能确保将该PG
的OSD副本更新到最新。
</pre>

**```ACTIVE```**


