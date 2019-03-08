---
layout: post
title: 深入浅出HTTPS工作原理(转)
tags:
- http
categories: http
description: https协议
---

HTTP协议由于是明文传送，所以存在三大风险：

* 被窃听的风险： 第三方可以截获并查看你的内容

* 被篡改的风险： 第三方可以截获并修改你的内容

* 被冒充的风险： 第三方可以伪装成通信方与你通信

HTTP因为存在以上三大安全风险，所以才有了HTTPS的出现。HTTPS涉及到了很多概念，比如SSL(Secure Sockets Layer)/TLS(Transport Layer Security)、数字证书、数字签名、加密、认证、公钥和私钥等，比较容易混淆。我们先从一次简单的安全通信故事讲起吧，其中穿插复习一些密码学的概念。



<!-- more -->








<br />
<br />

**[参考]**


1. [深入浅出HTTPS工作原理](https://blog.csdn.net/wangtaomtk/article/details/80917081)



<br />
<br />
<br />

