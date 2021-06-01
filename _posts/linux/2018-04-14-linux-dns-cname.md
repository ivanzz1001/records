---
layout: post
title: DNS中的cname记录
tags:
- LinuxOps
categories: linux
description: linux调试
---

本文介绍一下DNS中的```cname记录```和```A记录```，及其使用方法。


<!-- more -->

## 1. 什么是CNAME记录？
CNAME记录即Canonical Name Record，是域名解析系统(DNS)中的一种资源记录，用于将一个域名(an alias)映射到另一个域名(canonical name)。

当一个IP同时提供多种服务(比如FTP服务和Web服务，分别占用不同的端口)的时候，通过使用CNAME记录可以为我们带来极大的方便。例如，可以
通过CNAME来将```example.com```指向两个不同的DNS entry: ftp.example.com和www.example.com，反过来```example.com```也有一个指向实际
IP地址的```A Record```。之后，假如IP地址发生改变，则只需要对DNS的```A Record```进行修改即可。

>注：CNAME records总是指向另一个域名，不能直接指向一个IP地址。

根据[CNAME wiki](https://en.wikipedia.org/wiki/CNAME_record)，在上面的例子中，我们将ftp.example.com和www.example.com成为别名(alias name)，
而example.com才是真正的Canonical Name。


### 1.1 CNAME记录如何使用？

例如，假设你有几个子域：

* www.mydomain.com (alias name)

* ftp.mydomain.com (alias name)

* mail.mydomain.com (alias name)

并且你希望这些子域指向主域名```mydomain.com```(Canonical Name)，那么可以创建CNAME Record，而不是为每个子域创建A Record来绑定到实际的IP地址。

如下所示，如果服务器的IP地址发生更改，则只需要更新一个A记录，并且所有子域都会自动更新，因为所有CNAMES都指向带有A Record的主域：
<pre>
(子)域名/主机名           记录类型                目标/目的地
------------------------------------------------------------------
 mydomain.com               A                     111.222.100.101
 
ftp.mydomain.com           CNAME                   mydomain.com 
mail.mydomain.com          CNAME                   mydomain.com 
www.mydomain.com           CNAME                   mydomain.com 
</pre>
mydomain.com指向服务器IP地址，并通过www.mydomain.com、ftp.mydomain.com、mail.mydomain.com指向相同的地址mydomain.com。如果IP地址发生更改，
则只需要在一个地方进行更新即可： 修改A记录```mydomain.com```，那么www.mydomain.com将自动继承更改。

CNAME记录必须始终指向另一个域名，永远不要直接指向IP地址。如果您尝试将CNAME记录指向IP地址，DNSimple的记录编辑器会警告您。



 
 





<br />
<br />
**[参看]:**

1. [cname记录是什么](https://www.zhihu.com/question/22916306)

2. [what is CNAME?](https://www.pythonthree.com/what-is-cname/)

3. [CNAME record](https://en.wikipedia.org/wiki/CNAME_record)

<br />
<br />
<br />





