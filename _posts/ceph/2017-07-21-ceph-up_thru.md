---
layout: post
title: ceph之up_thru分析
tags:
- ceph
categories: ceph
description: ceph之up_thru分析
---

本文结合源代码来分析一下ceph中up_thru的作用及其要解决的问题。大家都知道，OSDMap的作用之一便是维护Ceph集群OSD的状态信息，所以基于此想先提出一个疑问： Ceph集群中有1个osd down了，那么osdmap会发生什么变化？ osdmap会更新几次？ 带着这个问题，本文深入探讨up_thru。



<!-- more -->


## 1. up_thru简介
### 1.1 引入up_thru目的
up_thru的引入目的是为了解决如下这类极端场景：

比如集群有两个osd(osd.1, osd.2)共同承载着一批PG来服务业务io。如果osd.1在某个时刻down了，并且随后osd.2也down了，再随后osd.1又up了，那么此时osd.1是否能提供服务？

如果osd.1 down掉期间，osd.2有数据更新，那么显然osd.1再次up后是不能服务的；但是如果osd.2没有数据更新，那么osd.1再次up后是可以提供服务的。


### 1.2 up_thru到底是啥？

要想知道up_thru到底是啥，可以先通过其相关数据结构感受一下，如下：
{% highlight string %}
//src/osd/osdmap.h
struct osd_info_t {
	epoch_t last_clean_begin;  // last interval that ended with a clean osd shutdown
	epoch_t last_clean_end;
	epoch_t up_from;          // epoch osd marked up
	epoch_t up_thru;          // lower bound on actual osd death (if > up_from)
	epoch_t down_at;         // upper bound on actual osd death (if > up_from)
	epoch_t lost_at;         // last epoch we decided data was "lost"
};

class OSDMap{
	...

private:
	vector<osd_info_t> osd_info;
	ceph::shared_ptr< map<pg_t,vector<int32_t> > > pg_temp;  // temp pg mapping (e.g. while we rebuild)
	ceph::shared_ptr< map<pg_t,int32_t > > primary_temp;  // temp primary mapping (e.g. while we rebuild)

};
{% endhighlight %}

在OSD处于alive + healthy阶段，我们通常会跟踪两个interval。最近的一个interval就是[up_from, up_thru)，这里up_thru（假设大于up_from的话）就是OSD处于```_started_```状态时的最后一个epoch，通常是osd实际down掉时的一个下边界值， 而down_at(假设其大于up_from的话）则是osd实际down掉时的一个上边界值。

另一个interval则是last_clean interval[first, last]。在这种情况下，last即为该OSD cleanly shutdown时的epoch值。

lost_at的作用在于不必等待OSD恢复，即可允许调用build_prior()来进行处理。在很多情况下，在osd处于down状态时，progress可能会被阻塞（这些操作中可能就包含一些更新操作）。假设OSD并不能回到online状态，我们可以强制继续进行处理，尽管这可能会丢失一些已经```应答```的write操作。稍后假设OSD重新回到线上后，则这些写操作仍然会被丢弃（the divergent objects will be thrown out）。

通过上面的数据结构我们便可以知道，up_thru是作为osd的信息保存在osdmap里的，其类型便是osdmap的版本号(epoch_t)。

既然其是保存在osdmap里面，那么我们可以通过把osdmap dump出来看看，如下：
<pre>
# ceph osd dump
epoch 2222
fsid 5341b139-15dc-4c68-925a-179797d894d3
created 2018-05-11 16:11:51.479785
modified 2020-09-11 12:13:01.076048
flags sortbitwise,require_jewel_osds
pool 11 '.rgw.root' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 170 flags hashpspool stripe_width 0
pool 12 'default.rgw.control' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 172 flags hashpspool stripe_width 0
pool 13 'default.rgw.data.root' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 16 pgp_num 16 last_change 174 flags hashpspool stripe_width 0
pool 14 'default.rgw.gc' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 16 pgp_num 16 last_change 1142 flags hashpspool stripe_width 0
pool 15 'default.rgw.log' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 178 flags hashpspool stripe_width 0
pool 16 'default.rgw.intent-log' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 180 flags hashpspool stripe_width 0
pool 17 'default.rgw.usage' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 182 flags hashpspool stripe_width 0
pool 18 'default.rgw.users.keys' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 184 flags hashpspool stripe_width 0
pool 19 'default.rgw.users.email' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 186 flags hashpspool stripe_width 0
pool 20 'default.rgw.users.swift' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 188 flags hashpspool stripe_width 0
pool 21 'default.rgw.users.uid' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 190 flags hashpspool stripe_width 0
pool 22 'default.rgw.buckets.index' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 256 pgp_num 256 last_change 192 flags hashpspool stripe_width 0
pool 23 'default.rgw.buckets.data' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 256 pgp_num 256 last_change 194 flags hashpspool stripe_width 0
pool 24 'default.rgw.meta' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 197 flags hashpspool stripe_width 0
pool 25 'default.rgw.buckets.non-ec' replicated size 2 min_size 1 crush_ruleset 5 object_hash rjenkins pg_num 8 pgp_num 8 last_change 200 flags hashpspool stripe_width 0
max_osd 10
osd.0 up   in  weight 1 up_from 2220 up_thru 2221 down_at 2212 last_clean_interval [2206,2211) 10.17.155.113:6800/29412 10.17.155.113:6801/29412 10.17.155.113:6802/29412 10.17.155.113:6803/29412 exists,up 67990973-0316-4f57-b721-ce61d886572c
osd.1 up   in  weight 1 up_from 2119 up_thru 2220 down_at 2114 last_clean_interval [2045,2113) 10.17.155.113:6808/5393 10.17.155.113:6809/5393 10.17.155.113:6810/5393 10.17.155.113:6811/5393 exists,up e6c55932-820c-474d-aff7-0ed4081f2a33
osd.2 up   in  weight 1 up_from 2118 up_thru 2220 down_at 2093 last_clean_interval [2045,2092) 10.17.155.113:6804/5391 10.17.155.113:6805/5391 10.17.155.113:6806/5391 10.17.155.113:6807/5391 exists,up dfe500d8-5778-4379-9c48-1f1390fa8f0a
osd.3 up   in  weight 1 up_from 2123 up_thru 2221 down_at 2095 last_clean_interval [2041,2093) 10.17.155.114:6800/894 10.17.155.114:6801/894 10.17.155.114:6802/894 10.17.155.114:6803/894 exists,up e00c8fe5-d49e-42d1-9bfb-4965b9ab75b3
osd.4 up   in  weight 1 up_from 2125 up_thru 2221 down_at 2121 last_clean_interval [2042,2120) 10.17.155.114:6804/6327 10.17.155.114:6805/6327 10.17.155.114:6806/6327 10.17.155.114:6807/6327 exists,up dede8fcc-0b34-4296-83e2-a48966b22c36
osd.5 up   in  weight 1 up_from 2127 up_thru 2221 down_at 2121 last_clean_interval [2042,2120) 10.17.155.114:6808/6391 10.17.155.114:6809/6391 10.17.155.114:6810/6391 10.17.155.114:6811/6391 exists,up c2cde97e-c27a-4560-a46c-68695be79ff1
osd.6 up   in  weight 1 up_from 2040 up_thru 2221 down_at 2039 last_clean_interval [2020,2038) 10.17.155.115:6800/820 10.17.155.115:6801/820 10.17.155.115:6802/820 10.17.155.115:6803/820 exists,up d12f28d8-8fff-4a77-b344-d5d0b3e6949c
osd.7 up   in  weight 1 up_from 2072 up_thru 2221 down_at 2051 last_clean_interval [2015,2038) 10.17.155.115:6804/26346 10.17.155.115:6805/26346 10.17.155.115:6806/26346 10.17.155.115:6807/26346 exists,up 26035aa5-6759-4856-8bdb-1507f5b052e6
osd.8 up   in  weight 1 up_from 2087 up_thru 2221 down_at 2085 last_clean_interval [2074,2086) 10.17.155.115:6808/26484 10.17.155.115:6812/1026484 10.17.155.115:6813/1026484 10.17.155.115:6814/1026484 exists,up bca13e21-d64c-433f-87e4-4d5ea309f28a
</pre>

## 2. up_thru的来龙去脉
up_thru的整个生成以及是如何起作用的？整个流程是非常长的，为了让文章变得短小精悍一点，与up_thru不是强相关的流程就不加入代码分析了，只是一笔带过。

为了形象说明up_thru的来龙去脉，我们就沿着上文那两个osd的例子展开说。

### 2.1 up_thru的更新

###### osd.1挂了后发生了什么？

osd.1挂了之后，整个集群反应如下：

* osd上报mon

osd.1挂了后，或者osd.1主动上报，或是其他osd向mon上报osd.1挂了，此时mon已经感知到osd.1挂了。

* 




<br />
<br />

**[参看]:**

1. [分布式存储架构：Ceph之up_thru来龙去脉分析](https://zhuanlan.zhihu.com/p/166527885)


<br />
<br />
<br />