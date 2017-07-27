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




## 安装librados开发环境 (step1)


## 配置一个cluster handler (step2)



## 创建一个IO context (step3)


## 关闭sessions (step4)


