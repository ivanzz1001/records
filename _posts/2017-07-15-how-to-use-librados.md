---
layout: post
title: librados相关介绍
tags:
- ceph
- librados
categories: ceph
description: librados相关介绍及使用范例
---

本文简单介绍librados，并给出相应的使用范例。参看：http://docs.ceph.com/docs/master/rados/api/librados-intro/

<!-- more -->



## 1. librados介绍
ceph存储集群提供的基本存储服务，使ceph能够在一个联合文件系统中独一无二的实现对象存储(object storage)、块存储(block storage)以及文件存储(file storage)。然而，你并不一定需要通过RESTful API、block API或者POSIX文件系统接口才能访问ceph存储集群。依赖于RADOS，你可以直接通过librados API来访问ceph存储集群。

librados API可以访问ceph存储集群的两类守护进程：
* Ceph Monitor: 维持cluster map的一份主拷贝
* Ceph OSD Daemon: 在存储节点上存储对象数据

![librados](https://ivanzz1001.github.io/records/assets/img/ceph/rados/librados.jpg)




## 2. 安装librados开发环境 
客户端应用程序必须通过```librados```来连接到ceph存储集群。在你使用```librados```的时候必须安装librados及其依赖库。 librados API由C++写成，但是也提供了C、python、java、PHP接口。这里我们重点介绍一下C/C++接口：

对于Debian/Unbuntu系统：
{% highlight string %}
sudo apt-get install librados-dev
{% endhighlight %}

对于RHEL/Centos系统：
{% highlight string %}
sudo yum install librados2-devel
{% endhighlight %}

安装之后可以在/usr/include/rados目录下找到。


## 3. 配置一个cluster handler 
通过librados创建的Ceph Client可以直接和OSD来进行交互来存取数据。要想和OSD进行交互，client app必须要调用librados并且连接到Ceph Monitor。一旦连接成功，librados就可以从Ceph Monitor处获得Cluster Map。当client app想要读写数据的时候，其需要创建一个IO context并且绑定到一个pool上。pool是和rule相关联的，其定义了如何将数据存入集群。通过IO Context，client向librados提供object名称，librados采用该名称及获取到的cluster map就能够计算出需要将数据存放到哪个PG和OSD。client app并不需要直接的了解到集群的拓扑结构：
![librados](https://ivanzz1001.github.io/records/assets/img/ceph/rados/rados-rw.jpg)


## 创建一个IO context (step3)


## 关闭sessions (step4)


