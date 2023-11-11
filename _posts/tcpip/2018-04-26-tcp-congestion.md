---
layout: post
title: 万字详文：TCP 拥塞控制详解(转)
tags:
- tcpip
categories: tcpip
description: 万字详文：TCP 拥塞控制详解
---

本文主要介绍 TCP 拥塞控制算法，内容多来自网上各个大佬的博客及《TCP/IP 详解》一书，在此基础上进行梳理总结，与大家分享。因水平有限，内容多有不足之处， 敬请谅解。

>本文转自知乎[《万字详文：TCP 拥塞控制详解》](https://zhuanlan.zhihu.com/p/144273871?utm_oi=26757679808512&utm_id=0)，在此做个记录，主要是为了防止原文丢失，并便于后续自身阅读学习。




<!-- more -->


## 1. TCP 首部格式

在了解 TCP 的拥塞控制之前，先来看看 TCP 的首部格式和一些基本概念。

TCP 头部标准长度是 20 字节。包含源端口、目的端口、序列号、确认号、数据偏移、保留位、控制位、窗口大小、校验和、紧急指针、选项等。


![tcp format](https://ivanzz1001.github.io/records/assets/img/tcpip/group1/tcp_format.png)

### 1.1 数据偏移（Data Offset）

该字段长 4 位，单位为 4 字节。表示为 TCP 首部的长度。所以 TCP 首部长度最多为 60 字节。

### 1.2 控制位

目前的 TCP 控制位如下，其中 CWR 和 ECE 用于拥塞控制，ACK、RST、SYN、FIN 用于连接管理及数据传输。

* CWR：用于 IP 首部的 ECN 字段。ECE 为 1 时，则通知对方已将拥塞窗口缩小。

* ECE：在收到数据包的 IP 首部中 ECN 为 1 时将 TCP 首部中的 ECE 设置为 1，表示从对方到这边的网络有拥塞。

* URG：紧急模式

* ACK：确认

* PSH：推送，接收方应尽快给应用程序传送这个数据。没用到

* RST：该位为 1 表示 TCP 连接中出现异常必须强制断开连接。

* SYN：初始化一个连接的同步序列号

* FIN：该位为 1 表示今后不会有数据发送，希望断开连接。

### 1.3  窗口大小（Window）

该字段长度位 16 位，即 TCP 数据包长度位 64KB。可以通过 Options 字段的 WSOPT 选项扩展到 1GB。



<br />
<br />

参看:

1. []()



<br />
<br />
<br />