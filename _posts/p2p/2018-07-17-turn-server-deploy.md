---
layout: post
title: turn服务器的部署
tags:
- p2p
categories: p2p
description: turn服务器的部署
---

本章主要讲述一下turn服务器的部署。

<!-- more -->

## 1. nat的分类

根据NAT对外映射地址的不同工作方式，NAT可以分为如下4种：

* 全锥形(Full Cone): NAT把所有来自相同内部IP地址和端口的请求映射到相同的IP地址和端口。任何一个外部主机都可以通过该映射发送IP包到内部主机。

* 限制性锥形（Restricted Cone）:NAT把所有来自相同内部IP地址和端口的请求映射到相同的外部IP地址和端口。但是，只有当内部主机先对IP地址为X的外部主机发送过IP包，该外部主机才能够向该内部主机发送IP包，而其他主机向该内部主机发送的IP数据包都会被NAT拦截掉。


* 端口限制性锥形(Port Restricted Cone):端口限制性锥形与限制性锥形类似，只是多了端口号的限制。即只有当内部主机先向IP地址为X，端口为P的外部主机发送过IP包，该外部主机才能够把源端口号为P,IP地址为X的数据包发送给该内部主机。

* 对称形（Symmetric NAT）:每一个来自相同内部IP和端口的请求到一个特定的目的IP地址和端口，NAT都会将其映射到不同的IP地址和端口，而只有收到过内部主机封包的外部主机才能够将封包发送回来。

![nat-type](https://ivanzz1001.github.io/records/assets/img/p2p/p2p_nat_type.jpg)


<br />
<br />

参看:

1. [turnserver穿透服务器详细配置](https://blog.csdn.net/tst116/article/details/62217782?locationNum=9&fps=1)

2. [几种UDP网络库的整理Raknet,UDT,ENet,lidgren-network-gen3](https://blog.csdn.net/andyhebear/article/details/51210752)

3. [Linux ipip隧道及实现](https://www.cnblogs.com/weifeng1463/p/6805856.html)





<br />
<br />
<br />

