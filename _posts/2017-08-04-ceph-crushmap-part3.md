---
layout: post
title: crushmap算法详解
tags:
- ceph
- crushmap
categories: ceph
description: crushmap算法详解
---

本文接上篇《crushmap详解-2》，结合ceph源代码及其他参考资料来详尽的探讨具体的crush算法。为了参看的方便，下面我们继续列出当前的crushmap:

<!-- more -->
<pre>
[root@localhost ceph-test]# cat crushmap.txt 
# begin crush map
tunable choose_local_tries 0
tunable choose_local_fallback_tries 0
tunable choose_total_tries 50
tunable chooseleaf_descend_once 1
tunable straw_calc_version 1

# devices
device 0 osd.0
device 1 osd.1
device 2 osd.2
device 3 osd.3
device 4 osd.4
device 5 osd.5
device 6 osd.6
device 7 osd.7
device 8 osd.8

# types
type 0 osd
type 1 host
type 2 chassis
type 3 rack
type 4 row
type 5 pdu
type 6 pod
type 7 room
type 8 datacenter
type 9 region
type 10 root
type 11 osd-domain
type 12 host-domain
type 13 replica-domain
type 14 failure-domain

# buckets
host node7-1 {
        id -2           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item osd.0 weight 0.150
        item osd.1 weight 0.150
        item osd.2 weight 0.150
}
rack rack-01 {
        id -3           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-1 weight 0.450
}
host node7-2 {
        id -4           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item osd.3 weight 0.150
        item osd.4 weight 0.150
        item osd.5 weight 0.150
}
rack rack-02 {
        id -5           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-2 weight 0.450
}
host node7-3 {
        id -6           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item osd.6 weight 0.150
        item osd.7 weight 0.150
        item osd.8 weight 0.150
}
rack rack-03 {
        id -7           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-3 weight 0.450
}
root default {
        id -1           # do not change unnecessarily
        # weight 1.350
        alg straw
        hash 0  # rjenkins1
        item rack-01 weight 0.450
        item rack-02 weight 0.450
        item rack-03 weight 0.450
}
host-domain host-group-0-rack-01 {
        id -8           # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-1 weight 0.450
}
host-domain host-group-0-rack-02 {
        id -11          # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-2 weight 0.450
}
host-domain host-group-0-rack-03 {
        id -12          # do not change unnecessarily
        # weight 0.450
        alg straw
        hash 0  # rjenkins1
        item node7-3 weight 0.450
}
replica-domain replica-0 {
        id -9           # do not change unnecessarily
        # weight 1.350
        alg straw
        hash 0  # rjenkins1
        item host-group-0-rack-01 weight 0.450
        item host-group-0-rack-02 weight 0.450
        item host-group-0-rack-03 weight 0.450
}
failure-domain sata-00 {
        id -10          # do not change unnecessarily
        # weight 1.350
        alg straw
        hash 0  # rjenkins1
        item replica-0 weight 1.350
}

# rules
rule replicated_ruleset {
        ruleset 0
        type replicated
        min_size 1
        max_size 10
        step take default
        step choose firstn 0 type osd
        step emit
}
rule replicated_rule-5 {
        ruleset 5
        type replicated
        min_size 1
        max_size 10
        step take sata-00
        step choose firstn 1 type replica-domain
        step chooseleaf firstn 0 type host-domain
        step emit
}

# end crush map
</pre>

## 1. CRUSH算法
![crushmap3-alg](https://ivanzz1001.github.io/records/assets/img/ceph/crushmap/crushmap3_alg.png)

上面是CRUSH算法实现的一个伪代码。通常情况下CRUSH函数的输入参数是一个对象名或其他标识符。

1） TAKE(a)操作会在整个存储拓扑结构中选择一个item(通常是一个bucket)，然后会将其指定给一个vector。该vector作为后续操作的一个输入。

2) SELECT(n,t)操作会遍历上述vector中的每一个元素，并且会在以该元素作为根的子树中选择类型为t的n个不同的item。存储设备均有一个已知的、固定的类型，并且系统中的每一个bucket均有一个类型字段以反映bucket的不同层级结构。


crush算法的基本思想就是：从```step take```根开始逐级遍历bucket层级结构，直到找到指定数量的副本节点或者失败退出。 

** Collisions, Failure, and Overload**

在select(n,t)操作当中，为了选出n个t类型的不同的item，其会不断的进行递归。 在该递归操作当中，CRUSH通过一个被修正过的r'来拒绝或选择item。之所有采用修正的r'(而不是当前的简单的副本编号）主要有如下3个原因：
* 该item已经处于当前被选中的集合中（发生了碰撞---select(n,t)算出来的结果必须必须不能冲突）
* 一个设备(device)处于failed状态
* 一个设备处于overloaded状态（负载过重）

Failed及Overloaded状态的设备均会在cluster map中进行标记，但会被保留在设备层级结构中以避免不必要的数据迁移。CRUSH会选择性的转移一部分负载过高的设备上的数据，这可以通过伪随机的拒绝一些的PG映射。针对Failed及Overloaded状态的设备，CRUSH算法会统一的在storage cluster的其他分支（跨storage cluster)来选择最后的映射（参看算法1第11行）；而对于collisions，一个新的r'会被用于下一次的递归搜索（参看算法1第14行），这样可以避免总体数据分步偏离更大可能发生碰撞的子树。









<br />
<br />
<br />


