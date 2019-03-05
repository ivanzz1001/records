---
layout: post
title: Linux Netfilter/Iptables
tags:
- LinuxOps
categories: linuxOps
description: Linux Netfilter
---

本章主要介绍一下Linux netfilter/iptables相关原理及使用


<!-- more -->



## 1. 基本概念介绍

### 1.1 netfilter与iptables

Netfilter是由Rusty Russell提出的Linux 2.4内核防火墙框架，该框架既简洁又灵活，可实现安全策略应用中的许多功能，如数据包过滤、数据包处理、地址伪装、透明代理、动态网络地址装换(Network Address Translation, NAT)，以及基于用户及媒体访问控制(Media Access Control, MAC)地址的过滤和基于状态的过滤、包速率限制等。Iptables/Netfilter的这些规则可以通过灵活组合，形成非常多的功能、涵盖各个方面，这一切都得益于它的优秀设计思想。

Netfilter是Linux操作系统核心层内部的一个数据包处理模块，它具有如下功能：

* 数据包过滤(防火墙功能)

* 网络地址装换(NAT/NAPT)

* 数据包伪装(packet mangling)

Netfilter是Linux Kernel中的一些钩子(hooks)，以允许内核模块在网络协议栈上注册回调函数。当一个网络协议栈中的数据包遍历到(traverse)对应的钩子(hook)时，相应注册的回调函数就会被调用。

iptables是一个通用的表结构，用于定义规则集(rulesets)。一个```IP table```中的每一个规则(rule)都有```多个classifiers```（iptables matches)和```对应的action```所构成。下面给出一个```iptables```的逻辑结构示意：
{% highlight string %}
                                |------ rule(classifier_1 classififer_2 ... action)
                                |
             |------IP table ---|------ rule(classifier_1 classififer_2 ... action)
             |                  |
             |                  |------ rule(classifier_1 classififer_2 ... action)
             |
             |
             |                  |------ rule(classifier_1 classififer_2 ... action)
             |                  |
iptables ----|------IP table ---|------ rule(classifier_1 classififer_2 ... action)
             |                  |
             |                  |------ rule(classifier_1 classififer_2 ... action)
             |
             |
             |                  |------ rule(classifier_1 classififer_2 ... action)
             |                  |
             |-----IP table---- |------ rule(classifier_1 classififer_2 ... action)
                                |
                                |------ rule(classifier_1 classififer_2 ... action)

{% endhighlight %}










<br />
<br />

**[参看]**

1. [linux netfilter](https://netfilter.org/documentation/index.html#documentation-howto)

2. [iptables常用实例备查](http://seanlook.com/2014/02/26/iptables-example/)

3. [iptables防火墙原理详解](http://seanlook.com/2014/02/23/iptables-understand/)

4. [IPTables](https://www.centos.org/docs/5/html/Deployment_Guide-en-US/ch-iptables.html)

5. [Firewalls](https://www.centos.org/docs/5/html/Deployment_Guide-en-US/ch-fw.html)

6. [iptables 设置端口转发/映射](https://blog.csdn.net/light_jiang2016/article/details/79029661)


<br />
<br />
<br />


