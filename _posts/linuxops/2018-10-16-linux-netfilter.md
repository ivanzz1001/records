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

* 网络地址装换(NAT/NAPT): 源网络地址装换(SNAT -- Source Network Address Translation)和目标地址转换(DNAT -- Destination Network Address Translation) 

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

**iptables** 和**netfilter**主要工作在```OSI网络7层参考模型```中的网络层和传输层。



### 1.2 ip filtering 介绍
本章我们会介绍一下ip filter的理论细节： IP filter工作原理、firewall的工作层级、policies等。

1) **What is an IP filter**

很重要的一点是我们必须要完全弄明白一个```IP filter```到底是什么。其实```Iptables```就是一个IP Filter，假如不能完全理解这一点的话，当我们在未来设计防火墙的时候将会碰到严重的问题。

一个```IP Filter```主要工作在TCP/IP网络协议栈中的**传输层**，而```iptables```可以工作在**传输层**和**网络层**。假如IP Filter的实现严格按照定义来的话，其只能够基于IP头(IP Headers)来进行包过滤。然而，由于iptables并不是严格的按照定义来实现的，导致IP filter也能够基于其他的Header来进行数据包过滤，例如TCP头、UDP头或者是MAC源地址。


iptables并不能跟踪数据包之间的联系，也不会对数据包的内容进行解析，因为这可能会耗费大量的内存及CPU。


2) **IP filtering相关术语**

如下我们介绍一下```IP Filtering```中的一些常用术语：

* Drop/Deny: 当一个数据包被dropped或denied，则该数据包被简单的删除，而并不会再采取其他的后续动作(actions)。既不会通知源端数据包丢失，也不会通知目的端数据包被丢掉。


* Reject: 基本上与Drop/Deny策略类似，只不过Reject会通知源端主机相应的数据包被丢掉。向源端主机返回的响应消息可以被指定或自动的计算为某个值。注意到目前为止，Reject并不会通知目的端主机相应的数据包被丢掉。

* State: 数据流中某个数据包的指定状态。例如，假如某数据包是防火墙首次遇见，则会被认为是一个新包(TCP连接中的SYN数据包）；假如某个数据包是一条已建立连接(established connection)流的一部分，则防火墙会认为其状态为```established```。这种状态信息可以通过**连接跟踪系统**(connection tracking system)来获得，其会跟踪所有的会话信息。


* Chain: 一个chain包含了一个```规则集```(ruleset)，规则集会被应用在所有经过该chain上的数据包上。每一条chain都有特定的目的（例如： 该chain被连接到哪一个```表```上）和特定的应用场景（例如： 只作用在```转发数据包```上，或者只作用于发往本主机的数据包）。在**iptables**中有多条不同的chain，我们后面会详细的介绍。

* Table: 每一个table都有特定的目的，在**iptables**中有4张不同的table，分别是**raw table**、**nat table**、**mangle table**和**filter table**。例如，```filter table```设计的初衷就是为了过滤数据包，而```nat table```设计的目的就是为了处理NAT包。

* Match: 在IP Filtering中，Match这一单词有两个不同的含义。第一个含义是指一个```单独匹配```(single match)，用于指示某一个规则(rule)其所匹配的数据包头(header)必须包含指定的信息，例如：```--source```匹配用于指示数据包的源地址必须为指定的```网络范围```(network range)或主机地址； 第二个含义是指一个规则的全量匹配(whole match)。假如某个数据包匹配全量规则,则会执行对应的jump或target指令

* Target: 通常一个规则集中的每一个规则都有一个对应的target。假若某个规则完全匹配成功，则target会指示我们如何处理该packet。例如，可以指示```drop```掉该数据包，或者```accept```该数据包，或者NAT该数据包。另外，也可以是一个```Jump```(请参看如下）

* Rule: 通常一个规则(rule)是由一系列的匹配(match)和一个的target所组成

* Ruleset: 一个```规则集```(ruleset)是由一系列的rule所组成，存放于IP Filter的实现中。在iptables中包含了filter表、nat表、raw表以及mangle表对应chain中的所有规则集。

* Jump： Jump指令与Target有很紧密的关系。在Iptables中，jump指令的写法与target的写法完全相同，除了其指定的是一个```chain```，而target所指定的是一个```target name```。假如匹配对应的规则，该数据包就会被发送到第二条chain中继续进行处理

* Connection Tracking: 实现了连接跟踪的防火墙能够跟踪```connctions/streams```。连接跟踪通常要耗费大量的CPU及内存资源。

* Accept： 接受一个数据包，并让其穿过防火墙规则。这与drop/deny/reject这样的target是完全相反的

* Policy： 当我们实现一个防火墙的时候，通常会说到两种不同类型的策略(policy)。首先我们有```链路策略```(chain policies)，用于指示防火墙的默认行文（即一个数据包并没有对应的规则匹配时的动作）；第二种policy就是指某种```安全策略```(security policy)，通常我们在构建一个防火墙之前需要全盘的想好一套安全策略。

3) **如何规划一个IP Filter**

当我们在规划一个防火墙方案的时候，首先就是要想好应该将防火墙架设在什么地方。通常情况下，这是相对容易的一个步骤，因为我们的网络是早已规划好的。首先我们考虑架设防火墙的位置就是在网关(gateway)上，该位置对内连接着本地网络，对外连接着Internet，因此其安全性一般要求较高。另外，在一个大型的网络中，也可以通过防火墙来做多个不同的划分。

此外，假如在你当前的网络中有一些服务器需要对外提供服务，我们也可以划设一个DMZ区域。一个DMZ区域是一个由服务器组成的小型物理网络，对外部进行一定的隔离，降低了外部用户对DMZ区域服务器访问的风险。


## 2. ```tables```和```chains```之旅
在本章，我们会讨论数据包是如何通过不同的```chains```的，以及相应的通过顺序。我们也会讨论数据包通过对应```table```的顺序。


### 2.1 General

当一个数据包刚进入防火墙(filewall)时，它首先会遇到相应的硬件设备(hardware)，然后再被传递到内核中相应的设备驱动程序。接着数据包会在内核中经过多个步骤，然后才会被发送到对应的```应用程序```(locally)或者被转发到其他主机或者丢弃等。

首先我们来看一下，当一个数据包的目的地址是我们的本地主机时，在我们的上层应用程序(application)收到该数据包之前其所经过的步骤：

![incomming-pkg](https://ivanzz1001.github.io/records/assets/img/linuxops/netfilter_incomming_pkg.jpg)


![tables-traverse](https://ivanzz1001.github.io/records/assets/img/linuxops/tables_traverse.jpg)


<br />
<br />

**[参看]**

1. [linux netfilter](https://netfilter.org/documentation/index.html#documentation-howto)

2. [iptables常用实例备查](http://seanlook.com/2014/02/26/iptables-example/)

3. [iptables防火墙原理详解](http://seanlook.com/2014/02/23/iptables-understand/)

4. [IPTables](https://www.centos.org/docs/5/html/Deployment_Guide-en-US/ch-iptables.html)

5. [Firewalls](https://www.centos.org/docs/5/html/Deployment_Guide-en-US/ch-fw.html)

6. [iptables 设置端口转发/映射](https://blog.csdn.net/light_jiang2016/article/details/79029661)

7. [iptables详解](https://www.cnblogs.com/metoy/p/4320813.html)

<br />
<br />
<br />


