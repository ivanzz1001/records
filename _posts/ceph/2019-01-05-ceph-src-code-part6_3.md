---
layout: post
title: ceph中PGLog处理流程
tags:
- ceph
categories: ceph
description: ceph源代码分析
---

PGLog也是OSD模块的一个重要组成部分。本章我们会先介绍一下PGLog抽象数据结构及操作流程，而对于在PG peering过程中ceph是如何利用PGLog来确保数据的一致性这一问题，我们暂时不会做过深的探讨（请参看后续关于PGLog方面的文章)。



<!-- more -->






<br />
<br />

**[参看]**


1. [PGLog写流程梳理](https://blog.csdn.net/Z_Stand/article/details/100082984)

2. [ceph存储 ceph中pglog处理流程](https://blog.csdn.net/skdkjzz/article/details/51488926)

3. [ceph PGLog处理流程](https://my.oschina.net/linuxhunter/blog/679829?p=1)

4. [ceph基于pglog的一致性协议](https://jingyan.baidu.com/article/fa4125ace14cf028ac7092f4.html)


<br />
<br />
<br />

