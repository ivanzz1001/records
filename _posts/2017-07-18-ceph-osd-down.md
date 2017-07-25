---
layout: post
title: 模拟物理机down掉的情况
tags:
- ceph
categories: ceph
description: ceph故障模拟
---

本文主要讲述在ceph运行过程中，物理机由于断电等原因突然down掉的情况下集群的表现，以及针对可能出现的状况的相应处理方法。


<!-- more -->
* 环境介绍
* 故障模拟
* 故障解决


## 1. 环境介绍

当前我们共有12个OSD，每个OSD用50G硬盘空间，分布在3台宿主机上。其逻辑拓扑结构与物理拓扑结构如下：
<pre>
[root@ceph001-node1 ~]# ceph osd tree
ID  WEIGHT  TYPE NAME                                UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-10 1.69992 failure-domain sata-00                                                     
 -9 1.69992     replica-domain replica-0                                               
 -8 0.49997         host-domain host-group-0-rack-01                                   
 -6 0.49997             host ceph001-node1                                             
  0 0.14999                 osd.0                         up  1.00000          1.00000 
  2 0.14999                 osd.2                         up  1.00000          1.00000 
  9 0.14999                 osd.9                         up  1.00000          1.00000 
  1 0.04999                 osd.1                         up  1.00000          1.00000 
-11 0.59998         host-domain host-group-0-rack-02                                   
 -2 0.59998             host ceph001-node2                                             
  3 0.14999                 osd.3                         up  1.00000          1.00000 
  4 0.14999                 osd.4                         up  1.00000          1.00000 
  5 0.14999                 osd.5                         up  1.00000          1.00000 
 10 0.14999                 osd.10                        up  1.00000          1.00000 
-12 0.59998         host-domain host-group-0-rack-03                                   
 -4 0.59998             host ceph001-node3                                             
  6 0.14999                 osd.6                         up  1.00000          1.00000 
  7 0.14999                 osd.7                         up  1.00000          1.00000 
  8 0.14999                 osd.8                         up  1.00000          1.00000 
 11 0.14999                 osd.11                        up  1.00000          1.00000 
 -1 1.69992 root default                                                               
 -3 0.59998     rack rack-02                                                           
 -2 0.59998         host ceph001-node2                                                 
  3 0.14999             osd.3                             up  1.00000          1.00000 
  4 0.14999             osd.4                             up  1.00000          1.00000 
  5 0.14999             osd.5                             up  1.00000          1.00000 
 10 0.14999             osd.10                            up  1.00000          1.00000 
 -5 0.59998     rack rack-03                                                           
 -4 0.59998         host ceph001-node3                                                 
  6 0.14999             osd.6                             up  1.00000          1.00000 
  7 0.14999             osd.7                             up  1.00000          1.00000 
  8 0.14999             osd.8                             up  1.00000          1.00000 
 11 0.14999             osd.11                            up  1.00000          1.00000 
 -7 0.49997     rack rack-01                                                           
 -6 0.49997         host ceph001-node1                                                 
  0 0.14999             osd.0                             up  1.00000          1.00000 
  2 0.14999             osd.2                             up  1.00000          1.00000 
  9 0.14999             osd.9                             up  1.00000          1.00000 
  1 0.04999             osd.1                             up  1.00000          1.00000 
</pre>

## 2. 故障模拟

通常在实际生产环境中，我们会将pool的size设为3或2，min_size设置为2，而在进行故障模拟的时候我们也是针对这一情况来进行。 实际模拟宿主机突然down掉的情况最好是通过断电源等方式，但这里由于云环境的限制，我们采用手动强制关机的方式来模拟。

### 2.1 准备测试数据

1) 创建存储池

这里我们创建两个存储池 ``.simulator.down.fst``和``.simulator.down.snd``：
{% highlight string %}
#创建测试存储池1：
sudo ceph osd pool create .simulator.down.fst 256 256
sudo ceph osd pool set .simulator.down.fst crush_ruleset 5
sudo ceph osd pool set .simulator.down.fst size 3
sudo ceph osd pool set .simulator.down.fst min_size 2
rados -p .simulator.down.fst df

#创建测试存储池2：
sudo ceph osd pool create .simulator.down.snd 256 256
sudo ceph osd pool set .simulator.down.snd crush_ruleset 5
sudo ceph osd pool set .simulator.down.snd size 2
sudo ceph osd pool set .simulator.down.snd min_size 2
rados -p .simulator.down.snd df
{% endhighlight %}

2) 向存储池中写入数据
{% highlight string %}

{% endhighlight %}
