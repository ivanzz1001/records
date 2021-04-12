---
layout: post
title: ceph解读之PGLog(转)
tags:
- ceph
categories: ceph
description: ceph源代码分析
---


ceph的PGLog是由PG来维护，记录了该PG的所有操作。其作用类似于数据库里的undo log。PGLog通常只保存近千条的操作记录（默认是3000条），但是当PG处于降级状态时，就会保存更多的日志（默认时10000条），这样就可以在故障的PG重新上线后用来恢复PG的数据。本文主要从PGLog的格式、存储方式、如何参与恢复来解析PGLLog。





<!-- more -->














<br />
<br />

**[参看]**

1. [ceph解读之PGLog](http://sysnote.github.io/2015/12/18/ceph-pglog/)


<br />
<br />
<br />

