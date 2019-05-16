---
layout: post
title: ceph数据一致性浅析
tags:
- ceph
categories: ceph
description: ceph数据一致性浅析
---



ceph作为一个分布式存储系统，保证数据的一致性是很重要的一个方面。ceph的数据存储是以PG为单位来进行的，而数据的一致性也是通过PG的相关操作来实现的。本章涉及到的内容包括：

* PGMap的映射

* Ceph Peering机制

<!-- more -->




<br />
<br />

**[参看]**

1. [PG 的创建过程](http://www.it610.com/article/2077868.htm)

2. [ceph——创建pg](https://blog.csdn.net/w007d/article/details/80906250)

3. [解析Ceph: 恢复与数据一致性](https://blog.csdn.net/xingkong_678/article/details/51485077)

4. [错误的状况下，ceph IO 的一致性如何保证](https://blog.csdn.net/guzyguzyguzy/article/details/53608829)

5. [ceph数据一致性浅析](https://max.book118.com/html/2017/0618/116348354.shtm)

6. [Ceph源码解析：PG peering](https://www.cnblogs.com/chenxianpao/p/5565286.html)

7. [ceph存储 PG的状态机和peering过程](https://blog.csdn.net/skdkjzz/article/details/51579903)

8. [Ceph中一些PG相关的状态说明和基本概念说明、故障模拟](https://blog.csdn.net/pansaky/article/details/86700301)

<br />
<br />
<br />

