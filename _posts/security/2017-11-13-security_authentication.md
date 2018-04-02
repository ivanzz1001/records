---
layout: post
title: 安全认证之kerberos协议
tags:
- security
categories: security
description: 安全认证之kerberos协议
---


本文主要介绍一下在安全认证方面经常用到的kerberos协议。


<!-- more -->


## 1. Kerberos协议
客户端通过向认证服务器(Authentication Server, AS)上的```KDC```(key distribution center)发送用户身份信息以进行认证。KDC会返回一个```TGT```(ticket-granting ticket)给客户端，该```TGT```中包含时间戳(timestamp)信息，并且经过了``TGS```(ticket-granting service)秘钥的加密。此一过程通常发生并不频繁，一般发生在用户登录过程；TGT通常会在未来的某个时间段内过期，但是可以在过期时间段内由客户端的session manager进行更新。

当客户端需要和






<br />
<br />

**【参看】：**

1. [Kerberos (protocol)](https://en.wikipedia.org/wiki/Kerberos_(protocol))

2. [Kerberos原理和使用](http://blog.csdn.net/kkdelta/article/details/46633557)


<br />
<br />
<br />


