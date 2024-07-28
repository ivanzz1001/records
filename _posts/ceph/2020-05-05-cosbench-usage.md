---
layout: post
title: cosbench对象存储测试工具
tags:
- ceph
categories: ceph
description: cosbench对象存储测试工具
---


COSBench是一个用于测试对象存储系统的分布式基准测试工具


<!-- more -->

## 1. 用法

1） 下载cosbench

到[cosbench Github官网](https://github.com/intel-cloud/cosbench)下载cosbench工具，放到压力机上，解压。

2）启动cosbench服务

<pre>
# sh start-all.sh
</pre>

这个时候就可以访问你启动的cosbench了： http://your-ip:19088/controller/index.html     

![cosbench-1](https://ivanzz1001.github.io/records/assets/img/ceph/cosbench/381b209413d345e3ab35eec5b726fd8b.png)



<br />
<br />

**[参看]**

1. [cosbench对象存储测试工具](https://blog.csdn.net/bandaoyu/article/details/138051244)

<br />
<br />
<br />




