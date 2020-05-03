---
layout: post
title: CRUSH数据分布算法
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

本章我们介绍一下ceph的数据分布算法CRUSH，它是一个相对比较独立的模块，和其他模块的耦合性比较少，功能比较清晰，比较容易理解。在客户端和服务器都有CRUSH的计算，了解它可以更好地理解后面的章节。

CRUSH算法解决了PG的副本如何分布在集群的OSD上的问题。我们首先会介绍CRUSH算法的原理，并给出相应的示例，然后进一步分析其实现的一些核心代码。


<!-- more -->


## 1. 数据分布算法的挑战
存储系统的数据分布算法要解决数据如何分布到集群中的各个节点和磁盘上，其面临如下的挑战：

* 数据分布和负载均衡： 首先是数据分布均衡，使数据能均匀地分布在各个节点和磁盘上。其次是负载均衡，使数据访问（读写等操作）的负载在各个节点和磁盘上均衡。

* 灵活应对集群伸缩： 系统可以方便地增加或者删除存储设备（包括节点和设备失效的处理）。当增加或者删除存储设备后，能自动实现数据的均衡，并且迁移的数据尽可能地少。

* 支持大规模集群： 为了支持大规模的存储集群，就要求数据分布算法维护的元数据相对较小，并且计算量不能太大。随着集群规模的增加，数据分布算法的开销比较小。

在分布式存储系统中，数据分布算法对于分布式存储系统至关重要。目前有两种基本实现方法，一种是基于集中式的元数据查询的方式，如HDFS的实现： 文件的分布信息(layout信息）是通过访问集中式元数据服务器获得；另一种是基于分布式算法以计算获得。例如一致性哈希算法(DHT)等。Ceph的数据分布算法CRUSH就属于后者。

## 2. CRUSH算法的原理
CRUSH算法的全称为： Controlled、Scalable、Decentralized Placement of Replicated Data，可控的、可扩展的、分布式的副本数据放置算法。

由前面介绍过的RADOS对象寻址过程可知，CRUSH算法解决PG如何映射到OSD列表中。其过程可以看成函数：
{% highlight string %}
CRUSH(X) -> (OSDi, OSDj, OSDk)
{% endhighlight %}

输入参数：

* X为要计算的PG的pg_id

* Hierachical Cluster Map为Ceph集群的拓扑结构

* Placement Rules为选择策略

输出一组可用的OSD列表。

下面将分别详细介绍Hierachical Cluster Map的定义和组织方式。Placement rules定义了副本选择的规则。最后介绍Bucket随机选择算法的实现。

### 2.1 层次化的Cluster Map
层次化的Cluster Map定义了OSD集群具有层级关系的静态拓扑结构。OSD的层级使得CRUSH算法在选择OSD时实现了机架感知(rack awareness)的能力，也就是通过规则定义，使得副本可以分布在不同的机架、不同的机房中，提供数据的安全性。

层级化的Cluster Map的一些基本概念如下：

* Device： 最基本的存储设备，也就是OSD，一个OSD对应一个磁盘存储设备；

* Bucket： 设备的容器，可以递归的包含多个设备或者子类型的bucket。Bucket的类型可以有很多种，例如host就代表了一个节点，可以包含多个device；rack就是机架，包含多个host等。在ceph里，默认的有root、datacenter、room、row、rack、host六个等级。用户也可以自己定义新的类型。每个device都设置了自己的权重，和自己的存储空间相关。bucket的权重就是子bucket（或者device)的权重之和。

下面举例说明bucket的用法：

```例4-1``` Cluster Map的定义
{% highlight string %}
host test1{                          //类型host，名字为test1
	id -2                            //bucket的id，一般为负值
	# weight 3.000                   //权重，默认为子item的权重之和
	alg straw                        //bucket随机选择的算法
	hash 0                           //bucket随机选择的算法使用的hash函数，这里0代表使用hash函数jenkinsl

	item osd.1 weight 1.000          //item1: osd.1和权重
	item osd.2 weight 1.000
	item osd.3 weight 1.000
}


host test2{                          
	id -3                            
	# weight 3.000                   
	alg straw                        
	hash 0                           

	item osd.3 weight 1.000          
	item osd.4 weight 1.000
	item osd.5 weight 1.000
}


root default{                   //root的类型为bucket，名字为default
	id -1                       //id号
	# weight 6.000                 
	alg straw
	hash 0

	item test1 weight 3.000
	item test2 weight 3.000
}

{% endhighlight %}
根据上面Cluster Map的语法定义，下图4-1给出了比较直观的层级化的树形结构。





<br />
<br />

**[参看]**

1. [非常详细的 Ceph 介绍、原理、架构](https://blog.csdn.net/mingongge/article/details/100788388)





<br />
<br />
<br />

