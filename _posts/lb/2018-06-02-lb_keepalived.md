---
layout: post
title: keepalived实现双机热备
tags:
- lb
categories: lb
description: keepalived实现双机热备
---

本文主要讲述一下keepalived的工作原理，及如何实现双机热备

<!-- more -->

## 1. keepalived简介

keepalved软件完全是由C语言编写的。该项目的主要目标为Linux操作系统是提供一个简便、鲁棒的方式实现负载均衡和高可用性。负载均衡架构依赖于著名并被广泛使用的```LVS```四层负载均衡。keepalived实现了一系列的```checkers```,可以通过检查各负载均衡器的健康状况来动态的监测和管理各负载均衡服务。另一方面，高可用性是通过```VRRP```协议来实现的。VRRP是一种应对路由失败的基础设施。

此外，keepalived为VRRP有限状态机实现了一系列的钩子(hooks)以提供low-level和高速的协议交互。为了提供最快速的网络失败检测，keepalived实现了BFD协议。VRRP状态装换可以把BFD命中考虑在内以更快速的驱动状态机转换。keepalived框架可以被单独的使用，也可以配合LVS等一起使用以提供更富弹性的基础设施。




<br />
<br />

**[参看]**

1. [keepalived官网](https://www.keepalived.org/)

2. [keepalived实现双机热备](https://www.cnblogs.com/jefflee168/p/7442127.html)

<br />
<br />
<br />


