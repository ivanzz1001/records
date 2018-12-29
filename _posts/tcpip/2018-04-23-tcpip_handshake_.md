---
layout: post
title: TCP三次握手四次挥手详解
tags:
- tcpip
categories: tcpip
description: TCP三次握手四次挥手详解
---

本文讲述一下TCP/IP的三次握手与四次挥手。转自 [TCP三次握手四次挥手详解](https://www.cnblogs.com/zmlctt/p/3690998.html)


<!-- more -->





## 2. TCP连接过程补充


**1) 为什么建立连接协议是三次握手，而关闭连接却是四次握手呢？**

这是因为服务端的```listen```状态下的socket当收到```SYNC```报文的连接请求后，它可以把```ACK```和```SYN```(ACK起应答作用，而SYNC起同步作用）放在一个报文里发送。但关闭连接时，当收到对方的```FIN```报文通知时，它仅仅表示对方没有数据发送给你了； 但未必你所有的数据都全部发送给了对方了，所以你可能未必会马上关闭socket，也即你可能还需要发送一些数据给对方之后，再发送```FIN```报文给对方来表示你同意现在可以关闭连接了，所以它这里的```ACK报文```和```FIN报文```多数情况下都是分开发送的。



<br />
<br />

**[参看]**

1. [TCP三次握手四次挥手详解](https://www.cnblogs.com/zmlctt/p/3690998.html)



<br />
<br />
<br />

