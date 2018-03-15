---
layout: post
title: Wireshark如何抓取本机包
tags:
- LinuxOps
categories: linux
description: wireshark如何抓取本机包
---

本章简要讲述一下wireshark的简单使用，及如何利用wireshark来抓取本机包。


<!-- more -->


## 1. wireshark的简单使用

windows wireshark的使用较为简单，这里主要讲述一下相应过滤规则的书写。wireshark中的过滤规则的书写格式基本与[pcap filter表达式](http://www.tcpdump.org/manpages/pcap-filter.7.html) 的书写规则一致，这里只举几个例子：
<pre>
1: 抓取HTTP包
http

2: 抓取IP地址为192.0.2.1并且tcp端口不为80和25的数据包
ip.addr == 192.0.2.1 and not tcp.port in {80 25}

3: 抓取目标IP地址为10.133.146.63，且tcp端口为8088的数据包
ip.dst == 10.133.146.63 && tcp.port == 8088

4: 抓取端口为80的tcp或udp数据包
tcp.port == 80 || udp.port == 80

5: 抓取目标IP地址为192.168.101.8的数据包
ip.dst == 192.168.101.8
</pre>


## 2. wireshark如何抓取本机包(转）

在进行通信开发的过程中，我们往往会把本机既作为客户端又作为服务端来调试代码，使得本机自己和自己通信。但是wireshark此时是无法抓取到数据包的，需要通过简单的设置才可以。有两种方法，我们下面介绍。

### 2.1 方法一

1) 以管理员身份运行cmd程序

2) **route add 本机ip mask 255.255.255.255 网关ip**，例如：
<pre>
route add 10.133.146.63 mask 255.255.255.255 10.133.146.1
</pre>

3) 使用完毕后用**route delete 本机ip mask 255.255.255.255 网关ip**删除，否则所有本机报文都经过网卡出去走一圈回来很耗性能。例如：
<pre>
route delete 10.133.146.63 mask 255.255.255.255 10.133.146.1
</pre>

<br />
此时再利用wireshark进行抓包便可以抓到本机同自己的通信包，这样配置的原因是将发往本机的包发送到网关，而此时wireshark可以捕获到网卡驱动的报文实现抓包。但这样有一个缺点，那就是本地请求的URL的IP只能写本地的IP地址，不能写localhost或127.0.0.1，写localhost或127.0.0.1还是抓取不到数据包。

### 2.2 方法二
windows系统没有提供本地回环网络的接口，用wireshark监控网络的话只能看到经过网卡的流量，看不到访问localhost的流量，因为wireshark在windows系统上默认使用的是WinPcap来抓包的，现在可以用Npcap来替换掉WinPcap，Npcap是基于WinPcap 4.1.3开发的，api兼容WinPcap。

**1) 下载安装包**

[Npcap项目主页](https://nmap.org/npcap/guide/)，它采用的是MIT开源协议，[Npcap下载](https://nmap.org/npcap/#download)。

**2) 安装**

安装时要勾选 Use DLT_NULL protocol sa Loopback ... 和 install npcap in winpcap api-compat mode，如下所示:

![npcap setup](https://ivanzz1001.github.io/records/assets/img/linux/wireshark-npcap-setup.png)

如果你已经安装了WireShark，安装前请先卸载WinPcap:

![winpcap uninstall](https://ivanzz1001.github.io/records/assets/img/linux/wireshark_winpcap_uninstall.png)

如果还提示WinPcap has been detected之类的，那就将C:\Windows\SysWOW64目录下的wpcap.dll修改为wpcap.dll.old，packet.dll修改为packet.dll.old，也可参考[[Solved] – WinPCap 4.12 install error](https://nicolask.wordpress.com/2012/09/23/solved-winpcap-4-12-install-error/)。

当然，如果还没有安装wireshark，那么在安装wireshark时就不要安装WinPcap了。

安装完成启动wireshark，可以看到在网络接口列表中，多了一项Npcap Loopback adapter，这个就是来抓本地回环包的网络接口了，打开后如下图：

![winpcap startup](https://ivanzz1001.github.io/records/assets/img/linux/wireshark_npcap_startup.png)

<br />


**3) 抓取localhost数据包**

它不仅可以抓URL是**localhost**的，也可以是**127.0.0.1**:

![capture localhost](https://ivanzz1001.github.io/records/assets/img/linux/wireshark_localhost.png)

<br />



**4) 抓取本机数据包**

当然，抓**本机IP**也是可以的：

![capture local ip](https://ivanzz1001.github.io/records/assets/img/linux/wireshark_local_ip.png)








<br />
<br />

[参看]:

1. [wireshark如何抓取本机包](https://www.cnblogs.com/lvdongjie/p/6110183.html)



<br />
<br />
<br />





