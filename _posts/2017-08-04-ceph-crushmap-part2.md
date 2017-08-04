---
layout: post
title: crushmap详解-2
tags:
- ceph
- crushmap
categories: ceph
description: crushmap详解
---

本文主要通过一个crushmap的例子，来探讨crushmap将PG映射到OSD的过程。
<!-- more -->


## 1. 生成crushmap.bin
我们有如下crushmap.txt:
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

调用如下命令生成crushmap.bin:
{% highlight string %}
crushtool -c crushmap.txt -o crushmap-new.bin
{% endhighlight %}

整个crushmap的层级结构如下：
<pre>
[root@localhost ceph-test]# crushtool --test -i crushmap-new.bin --tree
WARNING: no output selected; use --output-csv or --show-X
ID      WEIGHT  TYPE NAME
-10     1.34999 failure-domain sata-00
-9      1.34999         replica-domain replica-0
-8      0.45000                 host-domain host-group-0-rack-01
-2      0.45000                         host node7-1
0       0.14999                                 osd.0
1       0.14999                                 osd.1
2       0.14999                                 osd.2
-11     0.45000                 host-domain host-group-0-rack-02
-4      0.45000                         host node7-2
3       0.14999                                 osd.3
4       0.14999                                 osd.4
5       0.14999                                 osd.5
-12     0.45000                 host-domain host-group-0-rack-03
-6      0.45000                         host node7-3
6       0.14999                                 osd.6
7       0.14999                                 osd.7
8       0.14999                                 osd.8
-1      1.34999 root default
-3      0.45000         rack rack-01
-2      0.45000                 host node7-1
0       0.14999                         osd.0
1       0.14999                         osd.1
2       0.14999                         osd.2
-5      0.45000         rack rack-02
-4      0.45000                 host node7-2
3       0.14999                         osd.3
4       0.14999                         osd.4
5       0.14999                         osd.5
-7      0.45000         rack rack-03
-6      0.45000                 host node7-3
6       0.14999                         osd.6
7       0.14999                         osd.7
8       0.14999                         osd.8
</pre>


## 2. 测试PG映射到OSD的过程
如下我们使用crushtool工具来测试PG到OSD的映射。上面我们有两个rule,其对应的ruleset分别是ruleset 0与ruleset 5。
{% highlight string %}
 # 方式1： 指定使用rule 1(即ruleset 5),映射[0,10]这11个PG
crushtool --test -i crushmap-new.bin --show-mappings --rule 1 --num-rep=3 --min_x=0 --max_x=10

 # 方式2： 指定使用ruleset 5,映射[0,10]这11个PG
crushtool --test -i crushmap-new.bin --show-mappings --ruleset 5 --num-rep=3 --min_x=0 --max_x=10

 # 方式3： 单独指定映射某个PG
crushtool --test -i crushmap-new.bin --show-mappings --ruleset 5 --num-rep=3 --x=100
{% endhighlight %}

*注意： 这里如果不指定min_x与max_x，则系统默认会映射[0,1023]这1024个PG*

如下我们采用ruleset 5映射PG 0~PG 10:
<pre>
[root@localhost ceph-test]# crushtool --test -i crushmap-new.bin --show-mappings --ruleset 5 --num-rep=3 --min_x=0 --max_x=10
CRUSH rule 1 x 0 [3,0,7]
CRUSH rule 1 x 1 [5,0,7]
CRUSH rule 1 x 2 [8,3,1]
CRUSH rule 1 x 3 [8,0,4]
CRUSH rule 1 x 4 [1,4,7]
CRUSH rule 1 x 5 [3,8,0]
CRUSH rule 1 x 6 [3,6,1]
CRUSH rule 1 x 7 [5,8,2]
CRUSH rule 1 x 8 [7,5,0]
CRUSH rule 1 x 9 [8,3,1]
CRUSH rule 1 x 10 [4,0,8]
</pre>


<br />
<br />
<br />