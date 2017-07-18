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


## 环境介绍




## 故障模拟
在故障模拟之前，我们仔细分析故障产生的本质原因是：ceph存储集群中数据达到或超过了mon_osd_full_ratio。 而与ceph存储集群的大小，磁盘的绝对容量是没有关系的。后面我们会看到，故障的解决虽然与mon_osd_full_ratio值的大小有一定关系，但是与绝对容量是不相关的，因此这里模拟故障时，可以不用考虑集群的大小。

如下是整个故障的模拟步骤：

**(1) 新建新建用于故障演练的pool**
{% highlight string %}
sudo ceph osd pool create benchmark 256 256
rados -p benchmark df                      # 查看该池占用的资源
{% endhighlight %}


**(2) 调整mon中相应阀值的设置**

这里我们调小阀值的原因是为了后面可以通过相应的工具填充数据以尽快达到该阀值(在磁盘容量较小的情况下，也可以不必调整)。这里我们主要调整```mon_osd_nearfull_ratio``` 和 ```mon_osd_full_ratio```两个参数。在所有monitor上执行如下命令：
{% highlight string %}
sudo ceph daemon mon.{mon-name} config set mon_osd_nearfull_ratio 0.03
sudo ceph daemon mon.{mon-name} config set mon_osd_full_ratio 0.05
{% endhighlight %}

这里{mon-name}是具体monitor的名字。注意这里我们可以先用如下命令获取到原来的值并保存，以便我们在故障演练完成之后可以进行恢复。
<pre>
ceph daemon mon.{mon-name} config show | grep ratio
</pre>

**(3) 向benchmark池写入数据**

重复执行如下命令向benchmark池中写入数据，直到触发mon_osd_full_ratio条件，数据不能写入为止：
{% highlight string %}
sudo rados bench -p benchmark 60  write --no-cleanup
{% endhighlight %}









## 故障解决






