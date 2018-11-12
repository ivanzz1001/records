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
这个是从集群里面删除这个节点的记录，即从OSD map中移除中移除这个节点的记录。

5) 删除节点认证（不删除编号会占住）
<pre>
# ceph auth del osd.47
</pre>
这个是从认证当中去删除这个节点的信息。

如上方式移除OSD，其实会触发两次迁移，一次是在节点osd out以后，一次是在crush remove之后，两次迁移对于集群来说是不好的，其实通过调整步骤是可以避免二次迁移的。

## 1.2 方式2

1） 调整osd的crush weight
<pre>
ceph osd crush reweight osd.47 0.1
</pre>
说明：这个地方如果想慢慢的调整就分几次将crush的weight减低至0，这个过程实际上是让数据不分布在这个节点上，让数据慢慢分布到其他节点上，直至最终不分布到该OSD，并且迁移完成数据。

这个地方不光调整了osd的crush weight，实际上同时调整了host的weight，这样会调整集群的整体crush分布，在osd的crush为0后，再对这个osd的任何删除相关操作都不会影响到集群的数据分布。

2) 停止osd进程
<pre>
# systemctl stop ceph-osd@47

# systemctl status ceph-osd@47
</pre>
停止对应osd的进程，这个是通知集群这个osd进程不在了，不提供服务了。因为本身没有权重，就不会影响到整体的分布，也就没有迁移。

3) 将节点状态标记为out
<pre>
# ceph osd out osd.47
</pre>
将对应OSD移出集群，这个是通知集群这个osd不再映射数据了，不提供服务了，因为本身没有权重，就不会影响到整体的分布，也就没有迁移。


4) 从crush中移除节点
<pre>
# ceph osd crush remove osd.47
</pre>
这个是从crush中删除，因为当前已经是0了，所以不会影响主机的权重，也就没有迁移了。

5） 删除节点
<pre>
# ceph osd rm osd.47
</pre>
这个是从集群里面删除这个节点的记录，即从OSD map中移除中移除这个节点的记录。

6) 删除节点认证(不删除编号会占住）
<pre>
# ceph auth del osd.47
</pre>
这个是从认证当中去删除这个节点的信息。

经过验证，第二种方式只触发了一次迁移，虽然只是一个步骤先后顺序上的调整，但是对于生产环境的集群来说，迁移的量要少一次，实际生产环境当中节点是有自动out的功能，这个可以考虑自己去控制，只是监控的密度需要加大，毕竟这是一个需要监控的集群，完全让其自己处理数据的迁移是不可能的，带来的故障只会更多。


## 2. 替换OSD操作的优化与分析
我们在上面介绍了```删除OSD的正确方式```，里面只是简单的讲了一下删除的方式怎样能减少迁移量。下面我们讲述一下ceph运维过程当中经常出现的坏盘然后换盘的步骤的优化。

这里我们操作的环境是：两台主机，每一台主机8个OSD，总共16个OSD，副本设置为2， PG数设置为800，计算下来平均每一个OSD上的PG数目为100个，下面我们将通过数据来分析不同的处理方法的差别。

开始测试前我们先把环境设置为```noout```，然后通过停止OSD来模拟OSD出现了异常，之后进行不同处理方法。


### 2.1 方式1
这里我们首先out一个OSD，然后剔除OSD，接着增加OSD：
<pre>
1. 停止指定OSD进程

2. out指定OSD

3. crush remove指定OSD

4. 增加一个新的OSD
</pre>
一般生产环境设置为```noout```，当然不设置也可以，那就交给程序去控制节点的```out```，默认是在进程停止后的五分钟，总之这个地方如果有```out```触发，不管是人为触发，还是自动触发数据流是一定的，我们这里为了便于测试，使用的是人为触发，上面提到的预制环境就是设置的```noout```。

1） 获取原始pg分布

开始测试之前，我们首先获取最原始的PG分布：
{% highlight string %}
# ceph pg dump pgs|awk '{print $1,$15}'|grep -v pg > pg1.txt
{% endhighlight %}
上面获取当前的PG分布，保存到文件```pg1.txt```，这个PG分布记录的是PG所在的OSD。这里记录下来，方便后面进行比较，从而得出需要迁移的数据。

2) 停止指定的OSD进程
<pre>
# systemctl stop ceph-osd@15
</pre>
停止进程并不会触发迁移，只会引起PG状态的变化，比如原来主PG在停止的OSD上，那么停止掉OSD以后，原来的副本的那个PG就会升级为主PG了。






<br />
<br />

**[参看]**

1. [删除Ceph OSD节点](https://www.cnblogs.com/schangech/p/8036191.html)

2. [ADDING/REMOVING OSDS](http://docs.ceph.com/docs/master/rados/operations/add-or-rm-osds/)

3. [Ceph osd weight与osd crush weight之间的区别](http://hustcat.github.io/difference_between_osd_weight_and_osd_crush_weight/)

4. [Difference Between ‘Ceph Osd Reweight’ and ‘Ceph Osd Crush Reweight’](https://ceph.com/geen-categorie/difference-between-ceph-osd-reweight-and-ceph-osd-crush-reweight/)

5. [ceph中获取osdmap和monmap的方式](http://www.it610.com/article/5023564.htm)

6. [OSDMAPTOOL – CEPH OSD CLUSTER MAP MANIPULATION TOOL](http://docs.ceph.com/docs/master/man/8/osdmaptool/)

<br />
<br />
<br />

