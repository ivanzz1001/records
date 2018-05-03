---
layout: post
title: rgw简要介绍
tags:
- ceph
categories: ceph
description: rgw简要介绍
---

本文简要介绍一下RGW。

<!-- more -->


## 1. RGW层次结构


![ceph-rgw](https://ivanzz1001.github.io/records/assets/img/ceph/rgw/ceph-rgw-level.png)

RGW是建立在librados上的一个对象存储接口，支持两种类型的接口： s3-compatible、swift-compatible。由于RGW兼容s3/swift类型接口，因此RGW本身提供了其自身的用户管理功能。


## 2. MULTI-SITE

```MULTI-SITE```功能是从```Jewel```版本开始支持的。一个```single zone```配置通常是由一个```zone group```组成，该zone group包含一个zone和多个用于负载均衡的RGW实例。然而,Kraken为RGW提供了一些multi-site的配置选项。

* **Multi-zone**: A more advanced configuration consists of one zone group and multiple zones，每一个zone都包含着多个rgw实例。每一个zone后端都对应着其自己的Ceph Storage Cluster。一个zone group中的多个zone提供了容灾恢复。在Kraken中，zone group中的每一个zone都是active状态的，并且都会接受写操作。另外在灾难恢复时，所有处于active状态的zone都可以作为恢复源。

* **Multi-zone-group**: 在以前的话叫做```regions```。RGW也支持多个zone groups，每一个zone group包含多个zone。如果两个zone group的realm相同的话，则所存储的对象也会使用相同的全局namespace。	

* **Multiple Realms**： 在Kraken中, 支持每个zone group都有独立的realm； 也支持多个zone group共享一个realm； 也支持全局一个realm。

![rgw-multi-zone](https://ivanzz1001.github.io/records/assets/img/ceph/rgw/rgw-multi-zone.jpg)


## 3 配置一个Multi-site
一个multi-site配置通常需要至少两个ceph storage cluster，建议每一个集群都采用不同的名字。而针对每一个ceph storage cluster都需至少一个rgw。本文假设在两个不同的地理位置有至少两个ceph集群，每个集群对应着一个rgw，分别为: rgw1和rgw2。

一个multi-site配置需要有一个master zone group和一个master zone， 在本例子中rgw1在master zone group的master zone中提供相应的服务； 而rgw2在master zone group的secondary zone中提供相应的服务。

![zone-sync2](https://ivanzz1001.github.io/records/assets/img/ceph/rgw/zone-sync2.png)

### 3.1 配置一个master zone
在一个multi-site配置当中的所有rgw都会接收来自master zone group中的mater zone对应的ceph-radosgw的相关配置。因此首先我们必须先配置一个master zone group和一个master zone。

**1) 创建REALM**

一个realm包含multi-site配置中的```zone groups```以及```zones```， 并且在该realm中作为全局唯一的名称空间。我们在需要被设为```master zone group```的宿主机上执行如下命令：
<pre>
# radosgw-admin realm create --rgw-realm={realm-name} [--default]
</pre>
例如：
<pre>
# radosgw-admin realm create --rgw-realm=movies --default
</pre>

假如该realm只用于本集群的话，则添加```--default```选项。假如添加了```--default```选项，则radosgw-admin就默认会使用该realm。假如```--default```选项没有被设置的话，就可以通过```--rgw-realm```选项或者```--realm-id```选项添加其他的```zone-groups```和```zones```.

**2) 创建MASTER ZONE GROUP**

一个realm必须至少有一个master zone group。我们在需要被设为```master zone group```的宿主机上执行如下命令：
<pre>
# radosgw-admin zonegroup create --rgw-zonegroup={name} --endpoints={url} [--rgw-realm={realm-name}|--realm-id={realm-id}] --master --default
</pre>

例如：
<pre>
# radosgw-admin zonegroup create --rgw-zonegroup=us --endpoints=http://rgw1:80 --rgw-realm=movies --master --default
</pre>

假如该realm只有一个zone group的话，则指定```--default```选项。假如```--default```选项被指定了的话，则在添加新的```zones```时就会默认添加到该zone group中。假如```--default```选项未被指定的话，可以通过```--rgw-zonegroup```选项或者```--zonegroup-id```选项来指定需要添加到哪一个group。

**3） 创建Master Zone**

为一个multi-site配置添加新的master zone，可以采用如下命令：
<pre>
# radosgw-admin zone create --rgw-zonegroup={zone-group-name} \
                            --rgw-zone={zone-name} \
                            --master --default \
                            --endpoints={http://fqdn}[,{http://fqdn}]
</pre>
例如：
<pre>
# radosgw-admin zone create --rgw-zonegroup=us --rgw-zone=us-east \
                            --master --default \
                            --endpoints={http://fqdn}[,{http://fqdn}]
</pre>










<br />
<br />

**[参看]**

1. [MULTI-SITE](http://docs.ceph.com/docs/master/radosgw/multisite/)


<br />
<br />
<br />

