---
layout: post
title: 什么是EPEL?
tags:
- LinuxOps
categories: linuxOps
description: 什么是EPEL？
---


>文章转载自[什么是EPEL](https://www.jianshu.com/p/6a71714342e3)，在此做一个记录，以便后续查阅

背景：在了解EPEL之前，我们先来了解一下在Linux系统安装第三方库的痛楚。之前我在阿里云的ECS以及本地安装的CentOS 7系统中安装```Supervisor```时遇到了兼容问题。比如，阿里云ECS Centos7的python版本过低，以及Python的一些依赖命令也没有，导致我安装过程中受尽折磨。于是，我们今天祭出```EPEL```。


<!-- more -->

## 1. 什么是EPEL?

EPEL的全称叫Extra Packages for Enterprise Linux。```EPEL```是由Fedora社区打造，为```RHEL```及衍生发行版如CentOS、Scientific Linux等提供高质量软件包的项目。装上了```EPEL```之后，就相当于添加了一个第三方源。


为什么需要```EPEL```?

那是因为CentOS源包含的大多数的库都是比较旧的。并且，很多流行的库也不存在。EPEL在其基础上不仅全，而且够新。

EPEL这两个优点，解决了很多人安装库的烦恼。

## 2. 安装EPEL

```EPEL```在CentOS系统安装相当简单：
<pre>
# yum install -y epel-release
</pre>

这种方式我验证过。

在其他系统安装我没有验证。但是，```EPEL```毕竟是大厂出品，在这方面应该不存在问题。大家可以自行在 Google 搜索了解。

>以后再也不用为了安装某个软件的依赖而耗时大量脑细胞了。





**参考**:

1. [什么是EPEL？](https://www.jianshu.com/p/6a71714342e3)

<br />
<br />
<br />


