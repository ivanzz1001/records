---
layout: post
title: 删除osd节点
tags:
- ceph
categories: ceph
description: 删除osd节点
---


本文主要记录一下删除osd节点的步骤， 并对几种不同的方式进行对比


<!-- more -->


## 1. 删除osd的正确方式

如下我们先介绍两种移除OSD的方式，关于性能及数据迁移我们会在后面来进行分析，这里只列出相应的操作步骤：


### 1.1 方式1

1） 停止osd进程
<pre>
# systemctl stop ceph-osd@47

# systemctl status ceph-osd@47
</pre>

这一步是停止OSD的进程，让其他的OSD知道这个节点不提供服务了。

2) 将节点状态标记为```out```
<pre>
# ceph osd out osd.47

# ceph osd tree
</pre>

这一步是告诉mon，这个节点已经不能服务了，需要在其他的OSD上进行数据的恢复了。

3) 从crush中移除节点
<pre>
# ceph osd crush remove osd.47
</pre>

从crush中删除是告诉集群这个节点回不来了，完全从集群的分布当中剔除掉，让集群的crush进行一次重新计算，之前节点还占着这个crush的weight，会影响到当前主机的host crush weight。

4） 删除节点
<pre>
# ceph osd rm osd.47
</pre>
这个是从集群里面删除这个节点的记录。









<br />
<br />

**[参看]**

1. [删除Ceph OSD节点](https://www.cnblogs.com/schangech/p/8036191.html)

2. [ADDING/REMOVING OSDS](http://docs.ceph.com/docs/master/rados/operations/add-or-rm-osds/)

3. [Ceph osd weight与osd crush weight之间的区别](http://hustcat.github.io/difference_between_osd_weight_and_osd_crush_weight/)

4. [Difference Between ‘Ceph Osd Reweight’ and ‘Ceph Osd Crush Reweight’](https://ceph.com/geen-categorie/difference-between-ceph-osd-reweight-and-ceph-osd-crush-reweight/)

<br />
<br />
<br />

