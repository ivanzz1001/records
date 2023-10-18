---
layout: post
title: Ceph亚太峰会RGW议题分享(转)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本文转自[Ceph亚太峰会RGW议题分享](https://cloud.tencent.com/developer/article/1146413)，在此做个记录，以免原文丢失，并方便后续自己阅读学习。


<!-- more -->


## 1. Ceph亚太峰会RGW部分议题分享
本次Ceph亚太峰会干货最实在的的要数Redhat的《Common Support Issues and How to Troubleshoot Them》这里把RGW部分摘出来，和大家分享一下，本次议题主要是涉及到RGW中Object数量过多导致的OSD异常如何处理。


### 1.1 故障现象描述

<pre>
Flapping OSD's when RGW buckets have millions of objects

● Possible causes

○ The first issue here is when RGW buckets have millions of objects their
bucket index shard RADOS objects become very large with high
number OMAP keys stored in leveldb. Then operations like deep-scrub,
bucket index listing etc takes a lot of time to complete and this triggers
OSD's to flap. If sharding is not used this issue become worse because
then only one RADOS index objects will be holding all the OMAP keys.
</pre>

RGW的index数据以omap形式存储在OSD所在节点的leveldb中，当单个bucket存储的Object数量高达百万数量级的时候，deep-scrub和bucket list一类的操作将极大的消耗磁盘资源，导致对应OSD出现异常，如果不对bucket的index进行shard切片操作(shard切片实现了将单个bucket index的LevelDB实例水平切分到多个OSD上)，数据量大了以后很容易出事。

<pre>
○ The second issue is when you have good amount of DELETEs it causes
loads of stale data in OMAP and this triggers leveldb compaction all the
time which is single threaded and non optimal with this kind of workload
and causes osd_op_threads to suicide because it is always compacting
hence OSD’s starts flapping.
</pre>

RGW在处理大量DELETE请求的时候，会导致底层LevelDB频繁进行数据库compaction(数据压缩，对磁盘性能损耗很大)操作，而且刚好整个compaction在LevelDB中又是单线程处理，很容易到达osdopthreads超时上限而导致OSD自杀。

<pre>
● Possible causes contd ...

○ OMAP backend is leveldb in jewel and older clusters. Any luminous
clusters which were upgraded from older releases have leveldb as
OMAP backend.
</pre>

jewel以及之前的版本的OMAP都是以LevelDB作为存储引擎，如果是从旧版本升级到最新的luminous，那么底层OMAP仍然是LevelDB。

<pre>
○ All new luminous clusters have default OMAP backend as rocksdb
which is great because rocksdb has multithreaded compaction and in
Ceph we use 8 compaction thread by default and many other enhanced
features as compare to leveldb.
</pre>
最新版本的Luminous开始，OMAP底层的存储引擎换成了rocksDB，rocksDB采用多线程方式进行compaction(默认8个），所以rocksdb在compaction效率上要比LevelDB强很多。

### 1.2 临时解决方案
* 临时方法1:通过关闭整个集群或者独立的pool的deep-scrub去实现对集群稳定性的提升。

{% highlight string %}
○ The first temporary action should be setting nodeep-scrub flag either
global in the cluster with ceph osd set nodeep-scrub or only to the RGW
index pool with - ceph osd pool set <pool-name> nodeep-scrub 1.


○ Then the second temporary step could be taken if OSD's are not
stopping from hitting suicide timeout. Increase the OSD op threads
normal timeout and suicide timeout values and if filestore op threads are
also hitting timeout then increase normal and suicide timeout for
filestore op threads.
{% endhighlight %}

* 临时方法2：调优一些op相关的timeout参数，减少触发OSD自杀的概率，比如下面的一些参数
{% highlight string %}
○ Add these options in [osd.id] section or in [osd] section to make them
permanent till the time troubleshooting of this issue is going on and use
ceph tell injectargs command to inject them at run time.

osd_op_thread_timeout = 90                      #default is 15
osd_op_thread_suicide_timeout = 2000            #default is 150 

If filestore op threads are hitting timeout
filestore_op_thread_timeout = 180               #default is 60
filestore_op_thread_suicide_timeout = 2000      #default is 180

Same can be done for recovery thread also.
osd_recovery_thread_timeout = 120               #default is 30
osd_recovery_thread_suicide_timeout = 2000      #default is 300
{% endhighlight %}

* 临时方法3：当OMAP目录过大时，手工触发一些osd的Leveldb compaction操作，以压缩OSD的LevelDB体积。
{% highlight string %}
○ The third temporary step could be taken if OSD's have very large OMAP
directories you can verify it with command: du -sh /var/lib/ceph/osd/ceph-$id/current/omap, then do manual leveldb compaction for OSD's.
■ ceph tell osd.$id compact or

■ ceph daemon osd.$id compact or

■ Add leveldb_compact_on_mount = true in [osd.$id] or [osd] section
and restart the OSD.

■ This makes sure that it compacts the leveldb and then bring the
OSD back up/in which really helps.
{% endhighlight %}

* **永久方案**
{% highlight string %}
○ Calculate the bucket index shard RADOS object size
■ Count the OMAP keys in index shard object
● rados -p <rgw index pool name> listomapkeys
<index-shard-object-name> | wc -l
■ Each OMAP key is of 200 bytes for getting the size of object
● <count from above command> * 200 = <value in bytes>
○ If the index shard object is very big like above 20 MB consider resharding
because shard count is not set as per recommendation or sharding is not
used at all.

■ radosgw-admin bucket reshard is the command more details can be
found in upstream documentation. This is offline reshard tool.
■ Because of these issues now Luminous has dynamic resharding.
● http://docs.ceph.com/docs/master/radosgw/dynamicresharding
{% endhighlight %}

按每个index shard object去遍历index pool的对应的omap条目数(最好不要听PPT作者的去进行遍历，很容易雪上加霜)，按每个key占用200byte方式统计每个omap对象的容量大小，当超过20MB的时候去手工进行reshard操作，注意reshard操作过程中bucket有元数据丢失的风险，谨慎使用，具体可以看之前公众号的文章，怎么去备份bucket元数据信息。另外分享中还提到了最新的Luminous可以实现动态的reshard(根据单个bucket当前的Object数量，实时动态调整shard数量），其实这里面也有很大的坑，动态reshard对用户来讲不够透明，而且reshard过程中会造成bucket的读写发生一定时间的阻塞，所以从我的个人经验来看，这个功能最好关闭，能够做到在一开始就设计好单个bucket的shard数量，一步到位是最好。至于如何做好一步到位的设计可以看公众号之前的文章。(《RGW Bucket Shard设计与优化》系列)

{% highlight string %}
Permanent Solutions contd ...

○ If RGW index pool is not backed by SSD or NVME OSD’s and OSD’s are
running above 80% disk util(Disk bound) during leveldb compaction
consider migrating Index pool to new CRUSH ruleset which is backed by
SSD or NVME SSD’s.

○ If RGW index pool OSD’s are always using above 100% CPU(CPU bound)
during leveldb compaction consider converting omap backend to rocksdb
from leveldb.

○ Jewel still do not support omap backend as rocksdb - this jewel pull
request 18010 will bring the rocksdb support in jewel.
{% endhighlight %}
另外可以做到的就是单独使用SSD或者NVME作为index pool的OSD，但是Leveldb从设计上对SSD的支持比较有限，最好能够切换到rocksdb上面去，同时在jewel之前的版本还不支持切换omap引擎到rocksdb，除非打上下面的补丁 https://github.com/ceph/ceph/pull/18010

关于如何切换Leveldb到rocksdb，也给了详细的操作流程，但是简单起见，还是切换配置然后重建OSD要省心很多。具体操作如下:
{% highlight string %}
Permanent Solutions contd ...
○ After rocksdb support in jewel and luminous already has it these
commands can be used to convert omap bakend to rocksdb from leveldb:

■ Stop the OSD
■ mv /var/lib/ceph/osd/ceph-<id>/current/omap
/var/lib/ceph/osd/ceph-<id>/omap.orig
■ ulimit -n 65535
■ ceph-kvstore-tool leveldb /var/lib/ceph/osd/ceph-<id>/omap.orig
store-copy /var/lib/ceph/osd/ceph-<id>/current/omap 10000 rocksdb
■ ceph-osdomap-tool --omap-path
/var/lib/ceph/osd/ceph-<id>/current/omap --command check
■ sed -i s/leveldb/rocksdb/g /var/lib/ceph/osd/ceph-<id>/superblock
■ chown ceph.ceph /var/lib/ceph/osd/ceph-<id>/current/omap -R
■ cd /var/lib/ceph/osd/ceph-<id>; rm -rf omap.orig
■ Start the OSD

○ If you do not want to go with above steps then you can rebuild the OSD
with filestore_omap_backend = "rocksdb".
{% endhighlight %}

### 1.3 总结
{% highlight string %}
In summary:

○ Have RGW index pool backed by SSD or NVME.
○ Have proper bucket index shard count set to a nice value from starting
considering future growth.
○ Have RGW index pool OSD’s using rocksdb with 8 compaction threads,
rocksdb compression disabled and rocksdb_cache_size tuned properly as
per your workload starting point 1G and can be increased more.
○ If you still see index pool OSD’s flapping during deep-scrub you can keep
nodeep-scrub flag set on the index pool and this luminous pull request
luminous: osd: deep-scrub preemption will fix this issue and you can unset
nodeep-scrub after upgrading to fixed luminous version.
{% endhighlight %}
从PPT分享结合个人经验来看，解决这类问题的思路基本上如下:

1) 一定要有SSD作为index pool

2) bucket 的index shard数量提前做好规划，这个可以参考本公众号之前的几篇bucket index shard相关内容。

3) jewel之前的版本LevelDB如果硬件条件允许可以考虑切换到rocksdb同时考虑在业务高峰期关闭deep-scrub。如果是新上的集群用L版本的ceph，放弃Filestore，同时使用Bluestore作为默认的存储引擎。

总而言之bucket index的性能需要有SSD加持，大规模集群一定要做好初期设计，等到数据量大了再做调整，很难做到亡羊补牢！





<br />
<br />

**[参看]**
1. [Ceph亚太峰会RGW议题分享](https://cloud.tencent.com/developer/article/1146413)

2. [leveldb 产生大量ldb文件，导致IO error](https://www.oschina.net/question/2848189_2187722?p=1)

3. [Ceph蹚坑笔记](https://blog.csdn.net/jeegnchen/article/details/50827154)

<br />
<br />
<br />

