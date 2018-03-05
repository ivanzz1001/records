---
layout: post
title: ceph集群迁移
tags:
- ceph
categories: ceph
description: ceph集群迁移
---

本文主要简单讲述一下ceph集群迁移的一个整体步骤。

<!-- more -->


## 1. 关机前操作

### 1.1 确认集群当前状态
确认当前集群状态的是否为HEALTH_OK， 查看并记录osd所在的domain

在monitor节点执行：
<pre>
$ ceph -s 									#查看ceph集群当前状态
$ ceph osd tree >>/tmp/ceph_osd_tree 		#查看osd所在domain
</pre>

### 1.2 为ceph集群设置标志位
在数据以及CrushMap没有发生变化的情况下，Ceph是不会触发数据迁移以及CrushMap的变化的，但是为了安全起见，我们手动设置上标志
位，防止ceph集群在开关机前后过程中有状态变化以及数据的迁移。

在ceph集群monitor节点执行：
<pre>
$ ceph osd set noout
$ ceph osd set norebalance
$ ceph osd set norecover
</pre>


### 1.3 关闭rgw服务
逐台关闭rgw服务：
<pre>
$ service ceph-radosgw stop
</pre>

确认下rgw服务状态：
<pre>
$ service ceph-radosgw status
</pre>


### 1.4 关闭osd服务
在存储节点逐个执行：
<pre>
$ service ceph stop osd
</pre>
查看下osd状态：
<pre>
$ service ceph status osd
</pre>

### 1.5 关闭mon服务
在monitor节点逐台执行：
<pre>
$ service ceph stop mon
</pre>
查看mon服务：
<pre>
$ service ceph status mon
</pre>


## 2. 迁移机器
Ceph集群节点关机
先关闭存储节点，后关闭monitor节点

做机器迁移

ceph集群节点开机

先开启monitor节点，后开启存储节点


## 3. 开机后操作

### 3.1 开启mon服务
在monitor节点执行：

逐个开启mon服务：
<pre>
$ service ceph start mon
</pre>
查看mon服务状态：
<pre>
$ service ceph status mon
</pre>
查看mon是否在集群内：
<pre>
$ ceph quorum_status
</pre>


### 3.2 开启osd服务
逐个开启存储节点上的osd服务（按domain开启）

在存储节点执行：
<pre>
$ service ceph start osd
</pre>
查看该节点osd是否成功开启：
<pre>
$ ceph osd tree
</pre>

### 3.3 开启rgw服务
逐台开启rgw服务：
<pre>
$ service ceph-radosgw start
</pre>
确认下rgw服务状态：
<pre>
$ service ceph-radosgw status
</pre>


### 3.4 去除ceph标志位
确保所有节点mon和osd启动后，去除ceph集群标志位:

在ceph集群任意节点执行：
<pre>
$ ceph osd unset noout
$ ceph osd unset norebalance
$ ceph osd unset norecover
</pre>
去掉标志位后注意检查ceph集群状态是否恢复至HEALTH_OK
<pre>
$ ceph -s    #检查集群当前状态
</pre>





<br />
<br />
<br />

