---
layout: post
title: ceph peering机制再研究(6)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


在前面的章节中，我们从OSD接收到新的OSDMap开始讲起，然后讲述到其会向状态机投递两个事件：

* AdvMap事件(advance map)

* ActMap事件(activate map)

本章承接上文，从这两个事件入手，引出本文重点： PG Peering。但是在这里我还是想要先给出一张整体架构图，以更好的理解Peering流程。

![ceph-chapter10-10](https://ivanzz1001.github.io/records/assets/img/ceph/sca/ceph_chapter10_1.jpg)


<!-- more -->

## 1. PG状态机接收到AdvMap事件
我们先来看看AdvMap事件数据结构：
{% highlight string %}
struct AdvMap : boost::statechart::event< AdvMap > {
	OSDMapRef osdmap;
	OSDMapRef lastmap;
	vector<int> newup, newacting;
	int up_primary, acting_primary;
};
{% endhighlight %}
各字段含义如下：

* osdmap: 当前PG advance到的osdmap

* lastmap: 当前PG上一版的osdmap

* newup: AdvMap::osdmap版本下的up set

* newacting: AdvMap.osdmap版本下的acting set

* up_primary: AdvMap.osdmap版本下的up primary

* acting_primary: AdvMap.osdmap版本下的acting primary






<br />
<br />

**[参看]**

1. [ceph博客](http://aspirer.wang/)

2. [ceph官网](https://docs.ceph.com/en/latest/dev/internals/)

3. [PEERING](https://docs.ceph.com/en/latest/dev/peering/)

4. [分布式系统](https://blog.csdn.net/chdhust)
<br />
<br />
<br />

