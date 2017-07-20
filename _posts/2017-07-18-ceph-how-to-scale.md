---
layout: post
title: 当磁盘容量接近或者超过 mon_osd_full_ratio 时，该怎么去扩容？
tags:
- ceph
categories: ceph
description: ceph 扩容
---

本文主要讲述在ceph运行过程中，当磁盘容量接近或者超过mon_osd_full_ratio时，在整个ceph集群拒绝任何读写的情况下如何去扩容这样一个问题。文章主要包括如下几个部分：

<!-- more -->
* 环境介绍
* 故障模拟
* 故障解决


## 1. 环境介绍
当前我们共有9个OSD，每一个OSD有50G的硬盘空间。
<pre>
[root@ceph001-node1 build]# ceph osd tree
ID  WEIGHT  TYPE NAME                                UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-10 1.34999 failure-domain sata-00                                                     
 -9 1.34999     replica-domain replica-0                                               
 -8 0.45000         host-domain host-group-0-rack-01                                   
 -6 0.45000             host ceph001-node1                                             
  0 0.14999                 osd.0                         up  1.00000          1.00000 
  1 0.14999                 osd.1                         up  1.00000          1.00000 
  2 0.14999                 osd.2                         up  1.00000          1.00000 
-11 0.45000         host-domain host-group-0-rack-02                                   
 -2 0.45000             host ceph001-node2                                             
  3 0.14999                 osd.3                         up  1.00000          1.00000 
  4 0.14999                 osd.4                         up  1.00000          1.00000 
  5 0.14999                 osd.5                         up  1.00000          1.00000 
-12 0.45000         host-domain host-group-0-rack-03                                   
 -4 0.45000             host ceph001-node3                                             
  6 0.14999                 osd.6                         up  1.00000          1.00000 
  7 0.14999                 osd.7                         up  1.00000          1.00000 
  8 0.14999                 osd.8                         up  1.00000          1.00000 
 -1 1.34999 root default                                                               
 -3 0.45000     rack rack-02                                                           
 -2 0.45000         host ceph001-node2                                                 
  3 0.14999             osd.3                             up  1.00000          1.00000 
  4 0.14999             osd.4                             up  1.00000          1.00000 
  5 0.14999             osd.5                             up  1.00000          1.00000 
 -5 0.45000     rack rack-03                                                           
 -4 0.45000         host ceph001-node3                                                 
  6 0.14999             osd.6                             up  1.00000          1.00000 
  7 0.14999             osd.7                             up  1.00000          1.00000 
  8 0.14999             osd.8                             up  1.00000          1.00000 
 -7 0.45000     rack rack-01                                                           
 -6 0.45000         host ceph001-node1                                                 
  0 0.14999             osd.0                             up  1.00000          1.00000 
  1 0.14999             osd.1                             up  1.00000          1.00000 
  2 0.14999             osd.2                             up  1.00000          1.00000 
</pre>




## 2. 故障模拟
在故障模拟之前，我们仔细分析故障产生的本质原因是：ceph存储集群中数据达到或超过了mon_osd_full_ratio。 而与ceph存储集群的大小，磁盘的绝对容量是没有关系的。后面我们会看到，故障的解决虽然与mon_osd_full_ratio值的大小有一定关系，但是与绝对容量是不相关的，因此这里模拟故障时，可以不用考虑集群的大小。

如下是整个故障的模拟步骤：

### 2.1 调整阈值

这里我们调小阀值的原因是为了后面可以通过相应的工具填充数据以尽快达到该阀值(在磁盘容量较小的情况下，也可以不必调整)。我们主要调整```mon_osd_nearfull_ratio``` 和 ```mon_osd_full_ratio```两个参数。结合我们的实际环境，将mon_osd_nearfull_ratio调整为0.1(50*9*0.1=45G时产生警告），mon_osd_full_ratio调整为0.2(50*9*0.2=90G时集群为HEALTH_ERR)。

有两种方式两种不同的方式来调整这两个值，下面分别介绍：

**修改方式1**

修改/etc/ceph/ceph.conf配置文件，在[global] section下添加或修改这两个参数：
<pre>
mon_osd_nearfull_ratio = 0.1
mon_osd_full_ratio = 0.2
</pre>

修改完成后，重启所有节点上的OSD,Monitor,RGW。重启完成之后，采用如下命令查看是否更改过来：
{% highlight string %}
ceph daemon mon.{mon-id} config show | grep ratio
ceph daemon osd.{num} config show | grep ratio
ceph daemon client.radosgw.{rgw-name} config show | grep ratio
{% endhighlight %}

此处请根据集群的实际情况替换{mon-id}、{num}、{rgw-name}.
例如：
<pre>
[root@ceph001-node1 ~]# ceph daemon mon.ceph001-node1 config show | grep ratio
    "mon_osd_min_up_ratio": "0.3",
    "mon_osd_min_in_ratio": "0.3",
    "mon_cache_target_full_warn_ratio": "0.66",
    "mon_osd_full_ratio": "0.2",
    "mon_osd_nearfull_ratio": "0.1",
    "mds_op_history_duration": "600",
    "osd_backfill_full_ratio": "0.85",
    "osd_pool_default_cache_target_dirty_ratio": "0.4",
    "osd_pool_default_cache_target_full_ratio": "0.8",
    "osd_heartbeat_min_healthy_ratio": "0.33",
    "osd_scrub_interval_randomize_ratio": "0.5",
    "osd_debug_drop_ping_duration": "0",
    "osd_debug_drop_pg_create_duration": "1",
    "osd_op_history_duration": "600",
    "osd_failsafe_full_ratio": "0.97",
    "osd_failsafe_nearfull_ratio": "0.9",
    "osd_bench_duration": "30",
    "rgw_swift_token_expiration": "86400",
</pre>


**修改方式2**

此种方式不需要进行重启，但一定要注意命令的正确执行，不能遗漏。

1） 修改pg，mon,osd的阈值
{% highlight string %}
ceph pg set_nearfull_ratio 0.1
ceph pg set_full_ratio 0.2

ceph tell osd.* injectargs '--mon-osd-nearfull-ratio 0.1'
ceph tell osd.* injectargs '--mon-osd-full-ratio 0.2'

ceph tell mon.* injectargs '--mon-osd-nearfull-ratio 0.1'
ceph tell mon.* injectargs '--mon-osd-full-ratio 0.2'
{% endhighlight %}

或通过如下命令修改：
{% highlight string %}
ceph pg set_nearfull_ratio 0.1
ceph pg set_full_ratio 0.2

sudo ceph daemon mon.{mon-id} config set mon_osd_nearfull_ratio 0.1
sudo ceph daemon mon.{mon-id} config set mon_osd_full_ratio 0.2

sudo ceph daemon osd.{num} config set mon_osd_nearfull_ratio 0.1
sudo ceph daemon osd.{num} config set mon_osd_full_ratio 0.2
{% endhighlight %}
此处请根据集群的实际情况替换{mon-id}、{num}。这里需要调整所有节点上的OSD，MON的这两个阈值。


实际上，这里最重要的是```ceph pg``这两个命令的执行，其他的设置并不会作为最终的受控变量参看如下:

```代码片段1：```

![set_ratio_code1](https://ivanzz1001.github.io/records/assets/img/ceph/scale/set_ratio_code1.jpg)

```代码片段2：```

![set_ratio_code1](https://ivanzz1001.github.io/records/assets/img/ceph/scale/mon_ratio_code2.jpg)
从上面我们可以看到，pending_inc是作为最终的受控变量。


2） 修改rgw的阈值
{% highlight string %}
sudo ceph daemon client.radosgw.{rgw-name} config set mon_osd_nearfull_ratio 0.1
sudo ceph daemon client.radosgw.{rgw-name} config set mon_osd_full_ratio 0.2
{% endhighlight %}
此处请根据集群的实际情况替换{rgw-name}。这里需要调整所有节点上的RGW的这两个阈值。


3) 查看阈值是否修改成功
{% highlight string %}
ceph daemon mon.{mon-id} config show | grep ratio
ceph daemon osd.{num} config show | grep ratio
ceph daemon client.radosgw.{rgw-name} config show | grep ratio
{% endhighlight %}
此处请根据集群的实际情况替换{mon-id}、{num}、{rgw-name}:
<pre>
[root@ceph001-node1 ~]# ceph daemon mon.ceph001-node1 config show | grep ratio
    "mon_osd_min_up_ratio": "0.3",
    "mon_osd_min_in_ratio": "0.3",
    "mon_cache_target_full_warn_ratio": "0.66",
    "mon_osd_full_ratio": "0.2",
    "mon_osd_nearfull_ratio": "0.1",
    "mds_op_history_duration": "600",
    "osd_backfill_full_ratio": "0.85",
    "osd_pool_default_cache_target_dirty_ratio": "0.4",
    "osd_pool_default_cache_target_full_ratio": "0.8",
    "osd_heartbeat_min_healthy_ratio": "0.33",
    "osd_scrub_interval_randomize_ratio": "0.5",
    "osd_debug_drop_ping_duration": "0",
    "osd_debug_drop_pg_create_duration": "1",
    "osd_op_history_duration": "600",
    "osd_failsafe_full_ratio": "0.97",
    "osd_failsafe_nearfull_ratio": "0.9",
    "osd_bench_duration": "30",
    "rgw_swift_token_expiration": "86400",
</pre>

### 2.2 




## 故障解决






