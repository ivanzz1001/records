---
layout: post
title: ceph本地对象存储
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本地对象存储模块完成了数据如何原子地写入磁盘，这就涉及事务和日志的概念。对象如何在本地文件系统中组织的代码实现在src/os中。本章将介绍在单个OSD上数据如何写入磁盘中。

目前有4种本地对象存储实现：

* FileStore: 这是目前比较稳定，生产环境上使用的主流对象存储引擎，也是本章重点介绍的对象存储引擎

* BlueStore: 这是目前社区在实现的一个新版本，社区丢弃了本地文件系统，自己写了一个简单的，专门支持RADOS用户态的文件系统

* KStore： 这是以本地KV存储系统实现的对象存储，它基于RADOS的框架用来实现一个分布式的KV存储系统

* Memstore: 它把数据和元数据都保存在内存中，用来测试和验证使用

KStore和Memstore两种存储引擎比较简单，这里就不介绍了（会在后面适当的章节进行详细分析）。BlueStore社区还正在开发之中，这里也暂时不介绍。本章将详细介绍目前在生产环境中使用的FileStore存储的实现。

在了解ceph filestore本地对象对象存储的具体实现之前，建议先参看本章```附录```相关内容，以对object的attr及omap操作有一个直观上的认识。

<!-- more -->









## 8. 附录--ceph存储object的attr和omap操作

在ceph中，所有的存储不管是块存储、对象存储、还是文件存储最后都转化成了底层的对象object，这个object包含3个元素data、xattr、omap。data是保存对象的数据。xattr是保存对象的扩展属性，每个对象文件都可以设置文件的属性，这个属性是一个key/value对，但是受到文件系统的限制，key/value对的个数和每个value的大小都进行了限制。如果要设置的对象的key/value不能存储在文件的扩展属性中，还存在另外一种方式保存:omap，omap实际上是保存到了key/value对的数据库levelDB中，在这里value的值限制要比xattr中好的多。

一开始不太明白key/value是做什么的？在ceph中起到什么作用？这里要说明的是这些key/value是保存对象的元数据相关信息，这些元数据相关信息是可以单独创建和设置的，等于对象存储的扩展，支持属性的键值对存储。这个作用就是提供给ceph内部使用，暂时用处不大。

###### 8.1 块存储设备元数据管理
对于块存储，其元数据主要是块的相关信息。我们在创建一个块设备后，会创建一个默认的rbd(rados block device)元数据文件，用其来存放相应的元数据。

1) 创建块设备

首先执行如下命令查看当前已有的存储池：
<pre>
# rados lspools
.rgw.root
default.rgw.control
default.rgw.data.root
default.rgw.gc
default.rgw.log
default.rgw.users.uid
default.rgw.users.keys
default.rgw.buckets.index
default.rgw.usage
default.rgw.buckets.data
default.rgw.buckets.non-ec
default.rgw.users.swift
benchmark
default.rgw.users.email
</pre>
创建pool的语法为：
{% highlight string %}
# ceph osd pool create {pool-name} {pg-num} [{pgp-num}] [replicated] \
     [crush-ruleset-name] [expected-num-objects]
# ceph osd pool create {pool-name} {pg-num}  {pgp-num}   erasure \
     [erasure-code-profile] [crush-ruleset-name] [expected_num_objects]
{% endhighlight %}
如下我们创建名为```rbd-01```的pool（其pg-num为128):
<pre>
# ceph osd pool create rbd-01 128 128
pool 'rbd-01' created
# ceph osd pool set rbd-01 size 3
set pool 42 size to 3
# ceph osd pool set rbd-01 crush_ruleset 5
set pool 42 crush_ruleset to 5

# ceph osd pool ls
.rgw.root
default.rgw.control
default.rgw.data.root
default.rgw.gc
default.rgw.log
default.rgw.users.uid
default.rgw.users.keys
default.rgw.buckets.index
default.rgw.usage
default.rgw.buckets.data
default.rgw.buckets.non-ec
default.rgw.users.swift
benchmark
default.rgw.users.email
rbd-01
</pre>

之后，通过如下命令在rbd-01上面创建一个大小为100GB的块设备：
<pre>
# rbd create rbd-01/rbd-image01 --size=102400
# rbd info rbd-01/rbd-image01
rbd image 'rbd-image01':
        size 102400 MB in 25600 objects
        order 22 (4096 kB objects)
        block_name_prefix: rbd_data.2f7502ae8944a
        format: 2
        features: layering, exclusive-lock, object-map, fast-diff, deep-flatten
        flags: 
# rados -p rbd-01 ls
rbd_id.rbd-image01
rbd_object_map.2f7502ae8944a
rbd_directory
rbd_header.2f7502ae8944a
</pre>

2) 查看块设备存被映射到了哪些OSD上

采用如下的命令查看```rbd-image01```这个块设备映射到了哪些OSD上：
{% highlight string %}
# ceph osd map rbd-01 rbd-image01
osdmap e1262 pool 'rbd-01' (42) object 'rbd-image01' -> pg 42.f11b8ea3 (42.23) -> up ([5,15,23], p5) acting ([5,15,23], p5)
{% endhighlight %}

3) 设置xattr元数据信息
<pre>
# rados -p rbd-01 setxattr rbd-image01 creator ivanzz1001
# rados -p rbd-01 setxattr rbd-image01 description 'just for test'
# rados -p rbd-01 listxattr rbd-image01
creator
description
# rados -p rbd-01 getxattr rbd-image01 creator
ivanzz1001
</pre>

4) 查看rbd-image01对象

通过上面我们发现rbd-image01被存储在pg 42.23内，因此我们可以进入osd.5相应的目录下查看```rbd-image01```的相关信息：
<pre>
# pwd
/var/lib/ceph/osd/ceph-5/current
# cd 42.23_head
# ls
__head_00000023__2a  rbd-image01__head_F11B8EA3__2a
</pre>

获取对象的扩展属性信息：
<pre>
# getfattr ./rbd-image01__head_F11B8EA3__2a 
# file: rbd-image01__head_F11B8EA3__2a
user.ceph._
user.ceph._@1
user.ceph._creator
user.ceph._description
user.ceph.snapset
user.cephos.spill_out

# getfattr -n user.ceph._creator ./rbd-image01__head_F11B8EA3__2a 
# file: rbd-image01__head_F11B8EA3__2a
user.ceph._creator="ivanzz1001"

# getfattr -n user.ceph._description ./rbd-image01__head_F11B8EA3__2a 
# file: rbd-image01__head_F11B8EA3__2a
user.ceph._description="just for test"
</pre>
通过上面我们可以直观的看到刚才我们所设置的扩展属性信息。


###### 8.2 对象存储元数据管理

1) 上传文件到benchmark存储池
<pre>
# rados lspools
.rgw.root
default.rgw.control
default.rgw.data.root
default.rgw.gc
default.rgw.log
default.rgw.users.uid
default.rgw.users.keys
default.rgw.buckets.index
default.rgw.usage
default.rgw.buckets.data
default.rgw.buckets.non-ec
default.rgw.users.swift
benchmark
default.rgw.users.email

# pwd
/root/ceph_cluster/test
# cat  hello.txt 
hello,world

# rados -p benchmark put hello.txt /root/ceph_cluster/test/hello.txt
# rados -p benchmark ls | grep hello
hello.txt
</pre>

2) 查看上传的hello.txt对象
{% highlight string %}
# ceph osd map benchmark hello.txt
osdmap e1262 pool 'benchmark' (39) object 'hello.txt' -> pg 39.d92fd82b (39.2b) -> up ([18,12,7], p18) acting ([18,12,7], p18)

# ls
hello.txt__head_D92FD82B__27

# cat hello.txt__head_D92FD82B__27 
hello,world
{% endhighlight %}

3) 设置对象的扩展属性
<pre>
# rados -p benchmark setxattr hello.txt creator ivanzz1001
# rados -p benchmark setxattr hello.txt description 'just for test'
# rados -p benchmark listxattr hello.txt
creator
description
# rados -p benchmark getxattr hello.txt creator
ivanzz1001
</pre>

4) 通过linux相关命令查看设置的扩展属性
<pre>
# getfattr ./hello.txt__head_D92FD82B__27 
# file: hello.txt__head_D92FD82B__27
user.ceph._
user.ceph._@1
user.ceph._creator
user.ceph._description
user.ceph.snapset
user.cephos.spill_out

# getfattr -n user.ceph._creator ./hello.txt__head_D92FD82B__27 
# file: hello.txt__head_D92FD82B__27
user.ceph._creator="ivanzz1001"
</pre>





<br />
<br />

**[参看]**


1. [ceph存储 object的attr和omap操作](https://blog.csdn.net/skdkjzz/article/details/51579520)

2. [Ceph Pool操作总结](https://blog.csdn.net/hxpjava1/article/details/80167792)

3. [linux下文件文件夹扩展属性操作](https://blog.csdn.net/liuhong1123/article/details/7247744)

<br />
<br />
<br />

