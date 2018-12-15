---
layout: post
title: core/ngx_syslog.c源文件分析（附录）
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本文主要介绍一些Linux中syslog的实现。



<!-- more -->


## 1. syslog协议介绍

### 1.1 标准协议
<pre>
在Unix类操作系统上，syslog广泛应用于系统日志。syslog日志消息既可以记录在本地文件中，也可以通过网络发送到接收syslog
的服务器上。接收syslog的服务器可以对多个设备的syslog消息进行统一的存储，或者解析其中的内容做相应的处理。常见的应用场
景是网络管理工具、安全管理系统、日志审计系统。完整的syslog日志中包含产生日志的程序模块(Facility)、严重性(Severity或
Level)、时间、主机名或IP、进程名、进程ID和正文。

在Unix类操作系统上，能够按Facility和Severity的组合来决定什么样的日志消息是否需要记录，记录到什么地方，是否需要发送到
一个接收syslog的服务器等。由于syslog简单而灵活的特性，syslog不再仅限于Unix类主机的日志记录，任何需要记录和发送日志的
场景，都可能会使用syslog。长期以来，没有一个标准来规范syslog的格式，导致syslog的格式是非常随意的。最坏的情况下，根本
就没有任何格式，导致程序不能对syslog消息进行解析，只能将它看作是一个字符串。
</pre>

在2001年定义的[RFC3164](http://www.ietf.org/rfc/rfc3164.txt)中,描述了BSD syslog协议。不过这个规范的很多内容都不是强制性的，常常是“建议”或者“约定”，也由于这个规范出的比较晚，很多设备并不遵守或不完全遵守这个规范。接下来我们就简单介绍一下这个规范：

约定发送syslog的设备为Device，转发syslog的设备为Relay，接收syslog的设备为Collector。Relay本身也可以发送自身的syslog给Collector，这个时候它表现为一个Device。Relay也可以只转发部分接收到的syslog消息，这个时候它同时表现为Relay和Collector。syslog消息发送到Collector的UDP 514端口，不需要接收方应答，RFC3164建议Device也使用514作为源端口。规定syslog消息的UDP报文不能超过1024字节，并且全部由可打印的字符组成。完整的syslog消息由3部分组成，分别是```PRI```、```HEADER```和```MSG```。大部分syslog都包含```PRI```和```MSG```部分，而```HEADER```可能没有。


### 1.2 syslog的格式
下面是一个syslog消息：
{% highlight string %}
{% endhighlight %}






<br />
<br />

**[参看]**

1. [Logging to syslog](http://nginx.org/en/docs/syslog.html)


2. [syslog日志服务](https://blog.csdn.net/llzk_/article/details/69945366)

3. [syslog](https://baike.baidu.com/item/syslog/2802901)

4. [syslog 详解](https://blog.csdn.net/zhezhebie/article/details/75222667)


<br />
<br />
<br />

