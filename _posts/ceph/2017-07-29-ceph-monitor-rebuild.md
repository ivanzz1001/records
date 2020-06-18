---
layout: post
title: ceph monitor重建
tags:
- ceph
categories: ceph
description: ceph monitor重建
---


本文介绍一下如何从OSD来恢复monitor。

<!-- more -->


在所有host上执行如下脚本(collect_mon_store.sh)：
{% highlight string %}
#!/bin/sh

for osd in /var/lib/ceph/osd/ceph-*; do
  ceph-objectstore-tool --data-path $osd --no-mon-config --op update-mon-db --mon-store-path /root/mon-store/
done
{% endhighlight %}



<br />
<br />

**参考**:

1. [ceph从osd恢复mon](https://blog.csdn.net/qq_23929673/article/details/98176377)

2. [RECOVERY USING OSDS](https://docs.ceph.com/docs/master/rados/troubleshooting/troubleshooting-mon/)

3. [ceph 鉴权相关问题](https://www.cnblogs.com/yajun2019/p/11635906.html)

<br />
<br />
<br />

