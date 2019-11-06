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
假如你执行```ceph health```或者```ceph -s```或者```ceph -w```命令，你可能会发现ceph集群并不总是处于HEALTH OK状态。在你检查完OSD是否正处于运行状态之后，你也应该再检查一下PG的状态。在PG处于peering相关场景下，集群的状态可能就不是```HEALTH OK```了:

* 刚刚创建完一个pool，pg还为peer完成

* PG当前正处于recovering状态

* 刚刚向集群中添加一个OSD或者从集群中移除一个OSD

* 刚刚修改完crush map，PG正处于迁移过程

* 在PG中不同副本之间的数据出现了不一致

* 一个PG的副本之间正处于scrubbing状态

* ceph没有足够的空间来完成backfilling操作

假设出现上面的情况之一，整个集群将会处于```HEALTH WARN```状态。在大部分情况下，整个ceph集群都可以完成自我修复。但在有一些情况下，可能就需要人工介入来进行处理。监控PG运行情况的另一个方面就是确保在ceph集群处于up+running状态时，所有的PG处于active+clean状态。如果要查询所有PG的状态，请执行：
<pre>
# ceph pg stat
v61459532: x pgs: y active+clean, z active+clean+scrubbing+deep; 63243 GB data, 187 TB used, 450 TB / 638 TB avail; 21507 kB/s rd, 2751 kB/s wr, 362 op/s
</pre>
上面的结果告诉我们当前总的PG个数为x，其中有y个PG处于active+clean状态，z个PG处于active+clean+scrubbing+deep状态

>注： 大部分情况下，ceph集群中的PG状态都可能会有多种

另外，通过上面的命令在查询PG状态时也会打印出当前数据的使用容量，剩余容量以及总容量。这些数据在某一些场景下很有用：

* ceph集群快达要到near full ratio或者full ratio

* 由于CRUSH的配置错误使得数据并没有均衡的分布到整个集群

1） **PG ID**

PG ID由三个部分组成： 存储池ID(pool number)、period(.)、pg ID(十六进制数表示)：
{% highlight string %}
{pool-num}.{pg-id}
{% endhighlight %}

你可以通过执行*ceph osd lspools*命令来查看存储池ID以及它们的名称。比如ceph集群创建的第一个pool，其pool id为1，因此该存储池中的PG ID就类似于```1.1f```。


2） **PG查询相关命令**

如果要获取整个PG列表，请执行如下命令：
<pre>
# ceph pg dump
</pre>

如果想格式化输出(json格式)到一个指定文件，则可以执行如下命令：
<pre>
# ceph pg dump -o {filename} --format=json
</pre>

如果要查询某一个特定PG，可以执行如下的命令：
<pre>
# ceph pg {poolnum}.{pg-id} query
</pre>
上面命令执行之后将会以json格式打印出该PG的详细信息。


### 4.1 PG状态

如下我们将详细地介绍PG的常见状态。

###### 1） CREATING

当创建存储池(pool)时，将会一并创建所指定个数的PG。在PG正处于创建过程中时，该PG的状态为```creating```。一旦PG创建完成，该PG Acting Set中的OSD将会开始执行peer操作。peer完成后，pg的状态变为active+clean，此时就表明ceph客户端可以开始向该PG写入数据了：

![ceph-pg-create](https://ivanzz1001.github.io/records/assets/img/ceph/ceph-pg-create.png)


###### 2） PEERING

当ceph正在对一个PG进行peering时，将尝试使PG中各数据副本达成一致的状态。当PG完成peering之后，这就意味着该PG中OSD之间针对当前PG的状态达成了一致。然而，peering完成之后并不意味着当前PG中各个副本都拥有当前最新的数据。

>**权威历史(Authoritative History)**
>
>在客户端进行写操作时，必须要等到PG acting set中的所有OSD都写成功之后才会向客户端返回ACK响应。这就保证了自上次peering操作完成后，acting set中至少有一个成员拥有每一次*成功写操作*(acknowledged write operation)的完整历史记录。
>
>对每一个acknowledged write operation都有精确的记录，这就使得ceph可以构建出该PG的一个完整且有序的操作历史，然后就可以将该PG的所有OSD副本都更新到最新的一致状态。

###### 3） ACTIVE
一旦ceph完成Peering操作之后，pg就会变为active状态。active状态意味着PG中的数据是可以执行读写操作的。

###### 4) CLEAN
当一个PG处于clean状态时，说明PG的主OSD与副本OSD已经完成了peer，并且主副本OSD上的数据也已经是一致的了。ceph会对PG中的所有数据都复制到正确的副本数。

###### 5) DEGRADED
当一个客户端向主OSD写入一个对象时，主OSD负责将该对象也写入到副本OSD。在主OSD将对象写入到硬盘之后，PG将会处于*degraded*状态，直到主OSD收到了所有副本OSD的ack应答为止。

一个PG处于active+degraded状态的原因是： 某一个OSD虽然处于active状态，但该OSD当前却并不含有全部的对象。假如某一个OSD进入down状态，ceph将会把与该OSD相关联的PG都置位*degraded*状态。之后如果该OSD重新上线，则相关的PG必须重新peer。然而，即使某一个pg处于*degraded*状态，只要该PG仍是active的，那么仍可以向该PG写入新的数据。

假如某一个OSD处于down状态，并且PG处于*degraded*状态一直持续的话，ceph就可能将该down状态的OSD移出集群，标记为out状态。

假如某一个OSD下线(处于down状态)，那么相关的PG会处于*degraded*状态，在该状态持续一段时间之后（通常是300s），ceph就有可能将该down状态的OSD标记为out状态，指示该OSD已经移出集群， 然后开始将该down OSD上面的数据remap到另一个OSD。一个OSD从down状态变为out状态的时间由*mon osd down out interval*来控制，默认值为600s。

此外，如果ceph在某一个PG中找不到对应的object的话，ceph也可能会将该pg标记为*degraded*状态。对于unfound的对象，将不能够进行读写，但是你仍然可以访问处于*degraded*状态下PG中的其他对象。

###### 6） RECOVERING
ceph具有高容错性，能够应对运行过程中出现的软件或硬件相关的问题。当一个osd下线(down)之后，该OSD上的数据就有可能会落后于其他副本。之后该OSD重新上线，相关的PG就必须进行更新以反应当前的最新状态。在这一过程中，OSD可能就会显示为*recovering*状态。

Recovery发生的概率还是很高的，因为硬件的错误会导致多个OSD都失效，之后肯定就需要对数据进行恢复。例如，某个rack的路由器出现了故障，这就可能导致多台主机上的OSD落后于当前的集群状态。之后当故障解决之后，相关的OSD就必须进行恢复。

ceph提供了许多的配置参数来平衡相应的资源，使得在数据恢复期间也能够尽量正常的为外部提供服务。*osd recovery delay start*配置参数用于设置在restart、re-peer多长时间之后开始进行recovery操作； *osd recovery thread timeout*用于设置线程的超时时间，因为多个OSD都可能失效，这样最好是错开来进行restart、repeer，以防止消耗太多的资源。*osd recovery max active*配置参数限制了每个OSD能够同时处理的recovery请求个数，以防止因消耗资源过多而不能对外提供服务； *The osd recovery max chunk*配置参数限制了在进行数据恢复时chunk的大小，用以降低网络拥塞。

###### 7） BACKFILLING
当一个新的OSD加入到集群中之后，CRUSH会对PG进行重新映射，以将一些数据能够存放到新添加的OSD上。强制新添加的OSD马上就接受PG的指派可能会导致该OSD瞬间压力过高。而backfilling操作可以在后台进行，一旦backfilling操作完成，新添加的OSD就能够服务外部的请求。

在backfill操作过程中，你可能会看到多种状态：*backfill_wait*状态表明当前有一个backfill操作被挂起；*backfilling*状态表明backfill操作正在进行中；*backfill_toofull*表明s收到了一个backfill操作请求，但是由于存储容量的限制，使得该backfill操作不能够完成。当一个PG不能够backfilled时，就会显示为*incomplete*状态

*backfill_toofull*状态有可能只是暂时性的，因为当PG中相关的数据有可能会被移除，这样后面就又有可用的存储空间了。*backfill_toofull*状态类似于*backfill_wait*，只要后续相关的条件满足之后backfill就可以成功完成。

ceph提供了一系列的设置用于管理由于PG重新映射(特别是重新映射到一个新的OSD)导致的高负载。默认情况下，*osd_max_backfills*设置一个OSD可以同时进行的backfill数(包括backfill to与backfill from)为1。*backfill full ratio*用于设置一个OSD 在backfill时硬盘容量达到full ratio时就会解决相关的backfill请求，我们可以通过*osd set-backfillfull-ratio*命令来更改相关的值；假如某一个OSD拒绝了一个backfill请求，* osd backfill retry interval*使得一个OSD会在其指定的时间之后再次发起backfill请求。OSD也可以通过设置*osd backfill scan min*以及*osd backfill scan max*来管理扫描周期（默认情况下值分别为64/512）。


###### 8) REMAPPED




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