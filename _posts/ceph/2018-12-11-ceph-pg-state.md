---
layout: post
title: 监视OSD与PG
tags:
- ceph
categories: ceph
description: 监视OSD与PG
---

要实现高可用与高可靠性，我们就需要有相应的容错方法来管理硬件与软件出现的问题。ceph本身是没有单点故障的，即使处于```degraded```模式下仍可以对外提供服务。我们在[data placement](https://docs.ceph.com/docs/master/rados/operations/data-placement)一章介绍了ceph通过添加一个中间层，从而避免数据与某个OSD地址的直接产生绑定。这就意味着如果我们要从根源上跟踪系统错误的话，就必须要能够找到对应的PG以及底层的OSD。
<pre>
Tip: 集群中的一个错误也许会使得不能访问某一特定的对象，但这并不意味着你不能访问其他对象。
</pre>

通常情况下ceph具有自修复能力。然而，当问题存在之后，我们可以通过查看OSD以及PG的状态来进行定位。


<!-- more -->

## 1. MONITORING OSD
关于OSD的状态有如下描述：

>An OSD’s status is either in the cluster (in) or out of the cluster (out); and, it is either up and running (up), or it is down and not running (down). If an OSD is up, it may be either in the cluster (you can read and write data) or it is out of the cluster. If it was in the cluster and recently moved out of the cluster, Ceph will migrate placement groups to other OSDs. If an OSD is out of the cluster, CRUSH will not assign placement groups to the OSD. If an OSD is down, it should also be out.

<pre>
注： 假如一个OSD的状态时down+in，那么整个集群就会出现unhealthy状态
</pre>
![ceph-monitor-osd](https://ivanzz1001.github.io/records/assets/img/ceph/ceph-monitor-osd.png)

当你执行```ceph health```或者```ceph -s```或者```ceph -w```时，你也许会发现集群并不总是处于HEALTH OK状态。从OSD层面来看，在如下一些情况下也许你并不期望集群报告为HEALTH OK状态：

* 集群未启动（并不能对外部的请求作出响应)

* 集群刚刚启动或重启，并未准备好响应外部请求，因为PG可能正在创建(creating)，OSD可能也正处于peering状态

* 刚刚添加或移除OSD

* 刚刚修改完cluster map

监控OSD的另一个重要方面是确保当集群处于up+running状态时，集群中的所有OSD处于in+up+running状态。如果要查询是否所有的OSD都处于running状态，执行：
<pre>
# ceph osd stat
x osds: y up, z in; epoch: eNNNN
</pre>
查询的结果会告诉你总的OSD个数(x)，当前有多少处于up状态(y)，有多少处于in状态(z)，最后会显示当前的OSD map epoch。

假如当前集群中处于```in```状态的OSD个数大于```up```状态的OSD个数，执行如下命令查看哪些OSD当前处于down状态：
<pre>
# ceph osd tree
#ID CLASS WEIGHT  TYPE NAME             STATUS REWEIGHT PRI-AFF
 -1       2.00000 pool openstack
 -3       2.00000 rack dell-2950-rack-A
 -2       2.00000 host dell-2950-A1
  0   ssd 1.00000      osd.0                up  1.00000 1.00000
  1   ssd 1.00000      osd.1              down  1.00000 1.00000
</pre>

假如某一个osd当前处于down状态，执行如下命令来启动：
<pre>
# sudo systemctl start ceph-osd@1
</pre>

## 2. PG集合
当CRUSH将PG映射到OSD时，其首先会查询对应存储池(pool)的副本策略，之后PG中的每一个副本映射到不同的OSD。例如，假设当前存储池(pool)策略要求每个PG要有3个副本，则CRUSH可能会为其指定osd.1、osd.2、osd.3。CRUSH算法会根据CRUSH map中所设定的失败域(failure domain)来伪随机指定PG中OSD的映射，因此在一个大的ceph集群中你可能会很少看到一个PG内的OSD是相邻的。我们将一个PG中副本OSD集合称为```Acting Set```。在某些情况下，Acting Set中的某个OSD处于```down```状态或者不能处理该PG中的对象请求。在如下一些场景下就很可能会出现这种情况：

* 刚刚添加(add)或移除(remove)了一个osd，之后CRUSH会对PG进行重新映射，在这一过程中可能会更改Acting Set的OSD组成，并通过```backfill```来实现数据的迁移。

* 一个OSD在*过去*某个时间处于down状态，之后重启了，目前处于recovering状态

* Acting Set中的某个OSD*当前*处于down状态或者并不能响应相关的服务请求，其工作由另一个OSD临时接替

ceph使用```Up Set```来处理客户端请求。所谓Up Set，其实就是实际处理客户端请求的一组OSD。在大多数情况下，Up Set列表与Acting Set列表是完全相同的。而当两者不相同时，通常表明‘Ceph正在迁移数据，某一个OSD正在进行恢复’ 或者 ‘当前集群存在问题（比如： pg处于stuck stale状态）’。

要获取当前PG的所有列表，执行：
<pre>
# ceph pg dump
</pre>
如果要查看某一个PG的Acting Set与Up set，执行：
{% highlight string %}
# ceph pg map {pg-num}
osdmap eNNN pg {raw-pg-num} ({pg-num}) -> up [0,1,2] acting [0,1,2]
{% endhighlight %}
注： 假如Up Set与Acting Set不相等的话，通常意味着集群当前正处于```重平衡```状态，或者整个集群出现了潜在的故障

## 3. Peering
在允许向PG写数据之前，该PG必须处于active+clean状态。ceph是通过PG中主OSD与副本OSD之间的peer来获取该PG状态的：

![ceph-pg-peer](https://ivanzz1001.github.io/records/assets/img/ceph/ceph-pg-peer.png)

同时OSD也会向monitor主动报告其自身的状态。

## 4. 监控PG状态




<br />
<br />
**[参看]:**

1. [Monitoring OSDs and PGs](https://docs.ceph.com/docs/master/rados/operations/monitoring-osd-pg/)

2. [Ceph PG介绍及故障状态和修复](https://www.cnblogs.com/luohaixian/p/9693978.html)

3. [ceph PG状态](https://blog.csdn.net/majianting/article/details/86642301)

4. [分布式存储Ceph之PG状态详解](https://www.jianshu.com/p/36c2d5682d87)

<br />
<br />
<br />