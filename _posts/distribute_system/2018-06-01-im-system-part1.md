---
layout: post
title: IM服务器设计--基础(转)
tags:
- 分布式系统
categories: distribute-systems
description: IM服务器设计
---

IM作为非常经典的服务器系统，其设计时候的考量具备代表性，所以这一次花几个篇幅讨论其相关设计。

主要内容相当部分参考了[一套海量在线用户的移动端IM架构设计实践分享](http://www.52im.net/thread-812-1-1.html)一文，在此之上补充了更好的消息存储设计以及集群设计。


>说明： 工作多年，自己也亲身参与过一款IM系统相关模块的设计，但是对比本文有一些地方还是略有不足。本文转载自[IM服务器设计-基础](https://www.codedump.info/post/20190608-im-design-base/)，主要是为了进一步从更高层次理解IM；另一方面也方便自己的后续查找，防止文章丢失。

<!-- more -->

## 1. 整体架构






<br />
<br />

**[参看]:**

1. [IM服务器设计-基础](https://www.codedump.info/post/20190608-im-design-base/)

<br />
<br />
<br />


