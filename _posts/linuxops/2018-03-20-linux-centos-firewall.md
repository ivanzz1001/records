---
layout: post
title: Centos7 中防火墙的关闭问题
tags:
- LinuxOps
categories: linuxOps
description: Centos7 中防火墙的关闭问题
---


本文主要记录一下Centos7中防火墙关闭问题的处理方法。

<!-- more -->

## 1. SELinux、Netfilter、iptables、firewall和UFW
在使用Linux的过程中，我们经常会碰到上面这些与防火墙有关的名词，那这5者到底是什么呢？

* SELinux是美国国家安全局发布的一个强制访问控制系统

* Netfilter是Linux2.4.x引入的一个子系统，作为一个通用的、抽象的框架，提供一整套的hook函数的管理机制

* iptables是Linux下功能强大的应用层防火墙工具

* firewall是Centos7里新增的默认防火墙管理命令

* ufw是Ubuntu下的一个简易的防火墙配置工具

### 1.1 五者之间的关系
1） SELinux是本地访问控制的一整套安全机制，而Netfilter主要是管理网络方面

2） iptables是用于设置防火墙，防范来自网络的入侵和实现网络地址转发、QoS等功能，而SELinux则可以理解为Linux文件权限控制(即我们知道的read/write/execute)的补充存在

3) ufw是自2.4版本以后的Linux内核中一个简易的防火墙配置工具，底层还是调用iptables来处理。iptables可以灵活的定义防火墙规则，功能非常强大。但是产生的副作用就是配置过于复杂，因此产生了一个相对iptalbes简单很多的防火墙配置工具```ufw```

4) firewall是centos7里面新的防火墙管理命令，底层还是调用iptables来处理，主要区别是iptables服务每次更改都意味着刷新所有就规则并从*/etc/sysconfig/iptables*读取所有新规则；而firewall可以在运行时更改配置，而不丢失现有连接

5） iptables是Linux下功能强大的应用层防火墙工具，说到iptables必然提到Netfilter。iptables是应用层的，其实质是定义规则的配置工具，而核心的数据包拦截和转发是Netfilter。Netfilter是Linux操作系统核心层内部的一个数据包处理模块。

* iptables与Netfilter关系图

![linux-iptables-netfilter](https://ivanzz1001.github.io/records/assets/img/linuxops/linuxops_iptables_netfilter.png)


* iptables与firewall关系图

![linux-iptables-firewall](https://ivanzz1001.github.io/records/assets/img/linuxops/linuxops_iptables_firewall.png)



<br />
<br />

**[参看]:**

1. [centos 7 中防火墙的关闭问题](http://blog.csdn.net/song_csdn1550/article/details/51768671)

2. [CentOS7.3下的一个iptables配置](https://www.cnblogs.com/alwu007/p/6693822.html)

3. [Linux查看、修改SELinux的状态](http://blog.csdn.net/l18637220680/article/details/70231989)

4. [SELinux、Netfilter、iptables、firewall和UFW五者关系](https://blog.csdn.net/qq_34870631/article/details/78581891)

5. [一文彻底明白linux中的selinux到底是什么](https://blog.csdn.net/yanjun821126/article/details/80828908)
<br />
<br />
<br />


