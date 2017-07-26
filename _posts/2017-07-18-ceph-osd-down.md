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

2） OSD状态

在monitor接收不到OSD的报告信息，或收到其相邻OSD对其的故障报告时，monitor就会将该OSD置为down状态。接着经过一段时间```mon osd down out interval```,ceph集群就会将该down状态的OSD out出整个集群。然后会导致整个PG的重新映射，触发数据的迁移。

事实上，在我们上面的测试环境中重启ceph001-node1宿主机，却观察不到osd.0,osd.1,osd.2,osd.9被out出整个集群的状况。这是因为这里osd out还受到```mon_osd_down_out_subtree_limit```的影响。
<pre>
[root@ceph001-node1 ~]# ceph --show-config | grep mon_osd_down_out_subtree_limit
mon_osd_down_out_subtree_limit = rack
</pre>
如上所示，默认情况下ceph最大可以把rack大小的crush单元置为out。而这里我们关闭ceph001-node1宿主机已经达到了该最大单元，因此这里会限制将这些OSD置为out。

3）pg状态

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

一旦ceph完成了peering进程之后，该PG就会变成 active 状态。active状态意味着该PG(primary和replicas)内的数据可以进行读写操作。

**```CLEAN```**

当PG处于clean状态时，意味着该PG的master OSD和replica OSDs之间已经成功完成了peer，并且master与replicas之间数据完全一致。ceph对PG内的所有对象都复制到正确的副本数。


**```DEGRADED```**

当client将对象数据写到主OSD上时，该主OSD负责将对象数据副本写到副本OSD上。在主OSD将对象数据写到硬盘之后，在副本数据未成功写入副OSD之前整个PG都将处于DEGRADED状态。

一个PG之所有会存在active + degraded状态，是因为一个OSD即使在未保存有所有对象的情况下其仍可以处于active状态。假如一个OSD变成down状态之后，ceph就会将映射到该OSD的PG标志为degraded状态。在该down OSD回来时，其必须要进行重新的peering动作。然而只要该PG仍是active的，即使其处于degraded状态，其仍然可以进行写操作。

假如OSD down之后并且degraded状态一直持续，Ceph可以将该down状态的OSD out出集群，然后将处于该down OSD上的数据remap到其他的OSD上。在一个OSD被标记为down之后，经过```mon osd down out interval```时间之后就会被标记为out(默认值为300)。

注：
{% highlight string %}
#通过如下命令查看默认值
ceph --show-config --conf /dev/null 
{% endhighlight %}

当ceph认为应当处于某PG中的对象却并未发现时，PG也会被认为是degraded状态。即使处于degraded状态的PG中有些对象并不能读写，但是你仍可以对其他对象进行正常读写操作。

**```RECOVERING```**

ceph的设计使得其在软硬件出现故障时具有很好的容灾作用。当一个OSD down之后，其数据版本也许会落后与该PG的其他副本。而当该OSD重新回来变成up状态后，该PG的数据就必须要更新到当前状态。在这一过程中，该OSD就会处于recovering状态.

恢复操作很可能会经常发生，因为一个物理故障通常都会导致多个OSD同时down掉。比如，一个机架路由器故障就会导致该机架上的所有宿主机上的OSD 数据版本落后与集群的最新版本。这样当故障解决的时候，这些OSD就必须进行数据恢复。

ceph定义了很多变量来平衡当前的当前的服务请求与数据恢复时所造成的系统压力。```osd recovery delay start```允许该OSD在数据进行recovering之前,进行restart,re-peer,replay等操作。```osd recovery thread timeout```设置线程超时时间。``` osd recovery max active ```设置一个OSD同时处理的recovering请求数。```osd recovery max chunk```设置恢复时每一个chunk的数据大小，以避免网络拥塞。

**```REMAPPED```**

当PG的acting set发生改变的时候，数据就要从旧的acting set迁移到新acting set中。这在新的primary OSD接收请求之前会耗费一定时间。因此其仍会用旧的primary OSD来接收请求，直到数据迁移完成。一旦数据迁移完成之后，就会使用新acting set的primary OSD。

**```STALE```**

ceph使用心跳来确保host与daemons正处于运行状态，然而ceph-osd daemon并不能实时报告数据的话，其仍然会被置为stuck状态。默认情况下，OSD daemon会每隔0.5秒报告一次PG、up thru、boot、failure数据，其频率一般高于心跳频率。假如PG acting set中的primary OSD向monitor报告运行状况数据失败，又或者是其他的OSD报告该primary OSD处于down状态之后，该monitor会将该PG置为stale状态。

当你重启ceph集群的时候，在peering状态完成之前都通常会看到stale状态。而当集群运行一会之后，如果PG仍处于stale状态，则该PG的primary OSD处于down状态或者并没有将PG的相应数据报告给monitor。
<br />


**标识troubled PG**

如上所述，当一个PG的状态不是active + clean时不表示其一定有问题。通常，当PG变成stuck状态之后并不能通过自修复功能恢复。stuck状态包括：

* Unclean : PG中的一些对象数据并没有复制到所期望的副本数。它们应该正处于恢复中.
* Inactive: PG并不能进行读写操作，因为其正在等待拥有最新版本数据的OSD重新归来
* Stale   : PG当前处于未知状态，因为OSD并未在规定的时间内向monitor报告相应的信息（由```mon osd report timeout```配置)

执行如下命令可以查看stuck状态的PG：
{% highlight string %}
ceph pg dump_stuck [unclean|inactive|stale|undersized|degraded]
{% endhighlight %}


<br />
<br />
**集群PG状态分析**

接下来，我们结合上面的介绍来分析PG状态的变化。
<pre>
2017-07-25 14:56:43.288638 mon.1 [INF] osd.1 10.133.134.211:6804/3306 failed (3 reports from 3 peers after 20.014342 >= grace 20.000000)
2017-07-25 14:56:43.395532 mon.1 [INF] osdmap e379: 12 osds: 11 up, 12 in
2017-07-25 14:56:43.482454 mon.1 [INF] pgmap v18227: 744 pgs: 25 stale+active+clean, 719 active+clean; 1532 MB data, 5192 MB used, 594 GB / 599 GB avail
2017-07-25 14:56:44.539657 mon.1 [INF] osdmap e380: 12 osds: 11 up, 12 in
2017-07-25 14:56:44.698687 mon.1 [INF] pgmap v18229: 744 pgs: 21 stale+active+clean, 11 peering, 712 active+clean; 1532 MB data, 5192 MB used, 594 GB / 599 GB avail
2017-07-25 14:56:48.641278 mon.1 [INF] pgmap v18230: 744 pgs: 9 active+undersized+degraded, 17 stale+active+clean, 11 peering, 707 active+clean; 1532 MB data, 5192 MB used, 594 GB / 599 GB avail; 3/1290 objects degraded (0.233%)
2017-07-25 14:56:49.731555 mon.1 [INF] pgmap v18231: 744 pgs: 75 active+undersized+degraded, 669 active+clean; 1532 MB data, 5193 MB used, 594 GB / 599 GB avail; 42/1290 objects degraded (3.256%)
2017-07-25 14:56:58.637901 mon.1 [INF] pgmap v18232: 744 pgs: 75 active+undersized+degraded, 669 active+clean; 1532 MB data, 5193 MB used, 594 GB / 599 GB avail; 42/1290 objects degraded (3.256%)
2017-07-25 14:56:59.739817 mon.1 [INF] pgmap v18233: 744 pgs: 75 active+undersized+degraded, 669 active+clean; 1532 MB data, 5193 MB used, 594 GB / 599 GB avail; 42/1290 objects degraded (3.256%)
</pre>

首先monitor与osd.1之间通过peers发现其已经failed，我们查看```/var/lib/ceph/osd/ceph-1/current```目录发现PG数为75,因为我们设置的pool默认副本数是3，则这75个PG中以osd.1作为primary OSD的PG个数约为25个，正好符合osd.1 down掉时最先报告有25个PG处于stale + active + clean状态。

再接着由于PG之间的peering操作，会发现映射到osd.1上的所有75个PG都变成active + undersized + degraded状态。而由于目前我们crush map的设置，这些被降级的PG均不能向其他机架的OSD进行迁移，而其最小副本为2，因此只在ceph001-node1节点down掉的情况下这些PG一定会保持active + undersized + degraded状态。

后续osd.0,osd.2,osd.9均会报告类似信息，最后整个集群744个PG均处于active + undersized + degraded状态。
<pre>
2017-07-26 10:33:39.390934 mon.1 [INF] pgmap v19417: 744 pgs: 744 active+undersized+degraded; 1532 MB data, 5191 MB used, 594 GB / 599 GB avail; 430/1290 objects degraded (33.333%)
</pre>


## 故障解决

一般情况下，针对宿主机故障导致该宿主机上所有OSD down掉的情况下，我们只需要将这些down掉的OSD逐个重新启动并加入集群即可。在进行集群恢复时请用```ceph -w```命令监控集群的恢复状态，一般等一个OSD恢复完成，再恢复下一个。

在OSD处于down状态但仍为out出集群时，恢复一般只涉及到数据的recovering；而如果已经out出集群，一般会涉及到PG的remap，数据会重新backfill + recover.

<br />
<br />





