---
layout: post
title: 新版本ceph如何查看osd所使用的磁盘
tags:
- ceph
categories: ceph
description: 新版本ceph如何查看osd所使用的磁盘
---


本文介绍一下新版本ceph如何查看各OSD所使用的磁盘。参考：

- [linux dm-0 dm-1 设备映射 简介](https://blog.csdn.net/whatday/article/details/106354092)


<!-- more -->


## 1. ceph中如何查看osd所使用的磁盘

在Ceph中，可以使用ceph-volume工具或者检查/var/lib/ceph/osd/<id>/目录下的文件来查看OSD所使用的磁盘。

1）使用ceph-volume查看OSD使用的磁盘
{% highlight string %}
# sudo ceph-volume lvm list --osd-id <osd-id>
{% endhighlight %}

替换<osd-id>为您想要查看的OSD的ID。




2) 查看/var/lib/ceph/osd/<id>/目录中的信息
{% highlight string %}
# cat /var/lib/ceph/osd/<osd-id>/type
{% endhighlight %}

这里<osd-id>是您的OSD的ID。

- 如果OSD的类型是bluestore，那么您可以进一步查看使用的磁盘：
{% highlight string %}
# ls /var/lib/ceph/osd/<osd-id>/block
{% endhighlight %}

![osd-disk](https://ivanzz1001.github.io/records/assets/img/ceph/misc/ceph-misc-10000-01.jpg)

![osd-disk](https://ivanzz1001.github.io/records/assets/img/ceph/misc/ceph-misc-10000-02.jpg)

![osd-disk](https://ivanzz1001.github.io/records/assets/img/ceph/misc/ceph-misc-10000-03.jpg)



- 如果是filestore类型的OSD，数据目录通常位于
{% highlight string %}
# ls /var/lib/ceph/osd/<osd-id>/
{% endhighlight %}

## 2. Device Mapper简介
在linux系统中你使用一些命令时，有可能会看到一些名字为dm-xx的设备，那么这些设备到底是什么设备呢，跟磁盘有什么关系呢？以前不了解的时候，我也很纳闷. 其实dm是Device Mapper的缩写，Device Mapper 是 Linux 2.6 内核中提供的一种从逻辑设备到物理设备的映射框架机制，在该机制下，用户可以很方便的根据自己的需要制定实现存储资源的管理策略，当前比较流行的 Linux 下的逻辑卷管理器如 LVM2（Linux Volume Manager 2 version)、EVMS(Enterprise Volume Management System)、dmraid(Device Mapper Raid Tool)等都是基于该机制实现的。



<br />
<br />

**[参看]**


<br />
<br />
<br />




