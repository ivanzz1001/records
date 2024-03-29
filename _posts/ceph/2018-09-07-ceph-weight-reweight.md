---
layout: post
title: ceph weight与reweight的区别
tags:
- ceph
categories: ceph
description: ceph weight与reweight的区别
---


本文主要讲述一下ceph中weight 与reweight的区别。



<!-- more -->


## 1. ceph weight和reweight的区别
用ceph osd tree命令查看ceph集群，会发现有```weight```和```reweight```两个值：
<pre>
# ceph osd tree
ID WEIGHT   TYPE NAME                  UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-3 48.59967 root oss-uat                                                 
-2 16.19989     rack rack-01                                             
-1 16.19989         host ceph001-node1                                   
 0  1.79999             osd.0               up  1.00000          1.00000 
 1  1.79999             osd.1               up  1.00000          1.00000 
 2  1.79999             osd.2               up  1.00000          1.00000 
 3  1.79999             osd.3               up  1.00000          1.00000 
 4  1.79999             osd.4               up  1.00000          1.00000 
 5  1.79999             osd.5               up  1.00000          1.00000 
 6  1.79999             osd.6               up  1.00000          1.00000 
 7  1.79999             osd.7               up  1.00000          1.00000 
 8  1.79999             osd.8               up  1.00000          1.00000 
-5 16.19989     rack rack-02                                             
-4 16.19989         host ceph001-node2                                   
10  1.79999             osd.10              up  1.00000          1.00000 
11  1.79999             osd.11              up  1.00000          1.00000 
12  1.79999             osd.12              up  1.00000          1.00000 
13  1.79999             osd.13              up  1.00000          1.00000 
14  1.79999             osd.14              up  1.00000          1.00000 
15  1.79999             osd.15              up  1.00000          1.00000 
16  1.79999             osd.16              up  1.00000          1.00000 
17  1.79999             osd.17              up  1.00000          1.00000 
 9  1.79999             osd.9               up  1.00000          1.00000 
-7 16.19989     rack rack-03                                             
-6 16.19989         host ceph001-node3                                   
18  1.79999             osd.18              up  1.00000          1.00000 
19  1.79999             osd.19              up  1.00000          1.00000 
20  1.79999             osd.20              up  1.00000          1.00000 
21  1.79999             osd.21              up  1.00000          1.00000 
22  1.79999             osd.22              up  1.00000          1.00000 
23  1.79999             osd.23              up  1.00000          1.00000 
24  1.79999             osd.24              up  1.00000          1.00000 
25  1.79999             osd.25              up  1.00000          1.00000 
26  1.79999             osd.26              up  1.00000          1.00000 
</pre>

```crush weight```权重和磁盘的容量有关，一般```1T```值为**1.000**,```500G```就是**0.5**。其和磁盘的容量有关系，不因磁盘可用空间的减少而变化。其可以通过以下命令设置：
<pre>
# ceph osd crush reweight 
</pre>
例如：
{% highlight string %}
# ceph osd crush reweight osd.47 7.3
{% endhighlight %}


<br />
而```osd reweight```是一个0到1之间的值，可以用以下命令设置：
<pre>
# ceph osd reweight  
</pre>
例如：
{% highlight string %}
# ceph osd reweight 49 0.8
{% endhighlight %}

当```reweight```改变时，weight的值并不会变化。它影响PG到OSD的映射关系。```Reweight```参数的目的，由于ceph的CRUSH算法随机分配，是概率统计意义上的数据均衡，当小规模集群pg数量相对较少时，会产生一些不均匀的情况，通过调整```reweight```参数，达到数据均衡。

需要注意的是，这个参数不会持久化，当该osd out时，reweight的值为0， 当该osd重新up时，该值会恢复到1，而不会保持之前修改过的值。







<br />
<br />

**[参看]**

1. [ceph weight 和 reweight的区别](https://blog.csdn.net/changtao381/article/details/49073631)

2. [Ceph osd weight与osd crush weight之间的区别](http://hustcat.github.io/difference_between_osd_weight_and_osd_crush_weight/)

3. [Difference Between ‘Ceph Osd Reweight’ and ‘Ceph Osd Crush Reweight’](https://ceph.com/geen-categorie/difference-between-ceph-osd-reweight-and-ceph-osd-crush-reweight/)

4. [Ceph Primary Affinity](https://ceph.com/geen-categorie/ceph-primary-affinity/)

<br />
<br />
<br />

