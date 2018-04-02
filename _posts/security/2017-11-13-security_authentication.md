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
客户端通过向认证服务器(Authentication Server, AS)上的```KDC```(key distribution center)发送用户身份信息以进行认证。KDC会返回一个```TGT```(ticket-granting ticket)给客户端，该```TGT```中包含时间戳(timestamp)信息，并且经过了```TGS(ticket-granting service)秘钥```的加密。此一过程通常发生并不频繁，一般发生在用户登录过程；TGT通常会在未来的某个时间段内过期，但是可以在过期时间段内由客户端的session manager进行更新。

当客户端需要和另外一个节点上的某个服务(service)进行通信的时候，客户端会发送TGT到TGS(注： TGS通常和KDC处于同一台主机上）。以SPN命名规则(Service Principal Name)命名的service必须在TGT中进行了登记。客户端使用SPN来请求访问这个服务。在成功校验TGT的有效性之后，用户被允许访问所请求的服务，TGS会向客户端传递```ticket```和```session keys```。然后客户端拿着这个ticket向```SS```(Service Server)请求相应的服务。

![security-kerberos](https://ivanzz1001.github.io/records/assets/img/security/security_kerberos.jpg)


如下详细描述该协议：

* **User Client-based Logon**




* **Client Authentication**




* **Client Service Authorization**



* **Client Service Request**

<br />
<br />

**【参看】：**

1. [Kerberos (protocol)](https://en.wikipedia.org/wiki/Kerberos_(protocol))

2. [Kerberos原理和使用](http://blog.csdn.net/kkdelta/article/details/46633557)


<br />
<br />
<br />


