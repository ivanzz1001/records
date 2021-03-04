---
layout: post
title: vmware虚拟机三种网络模式原理详解
tags:
- LinuxOps
categories: linux
description: vmware虚拟机三种网络模式原理详解
---

本文记录一下vmware虚拟机三种网络模式：

* 桥接模式

* NAT模式

* Host Only模式

的基本工作原理，之后在介绍相关的配置步骤。

<!-- more -->

## 1. 概述
VMware为我们提供了三种网络工作模式：Bridged(桥接模式)、NAT(网络地址转换模式)、Host-Only(仅主机模式)。

打开VMWare虚拟机，我们可以在选项栏的```“编辑”```下的```“虚拟网络编辑器”```中看到VMnet0(桥接模式）、VMnet1(仅主机模式)、VMnet8(NAT模式)，那么这些都有什么作用呢？其实，我们现在看到的```VMnet0```表示的是用于桥接模式下的虚拟交换机；VMnet1表示的是仅用于主机模式下的虚拟交换机；VMnet8表示的是用与NAT模式下的虚拟交换机。

![vmware-network](https://ivanzz1001.github.io/records/assets/img/linux/vmware_network.jpg)

同时，在主机上对应的有VMware Network Adapter VMnet1和VMware Network Adapter VMnet8两块虚拟网卡，它们分别作用于仅主机模式与NAT模式下。在“网络连接”中我们可以看到这两块虚拟网卡，如果将这两块网卡卸载了，可以在vmware的```“编辑”```下的```“虚拟网络编辑器”```中点击“还原默认设置”，可重新将虚拟网卡还原。

![vmware-network-interface](https://ivanzz1001.github.io/records/assets/img/linux/vmware_windows_interface.png)


## 2. Bridged(桥接模式)

什么是桥接模式？桥接模式就是将主机网卡与虚拟机的虚拟网卡利用```虚拟网桥```进行通信。在桥接的作用下，类似于把物理主机虚拟为一个交换机，所有桥接设置的虚拟机均连接到这个交换机的一个接口上，物理主机也同样插在这个交换机当中，所以所有桥接下的网卡与网卡都是交换模式的，相互可以访问而不干扰。在桥接模式下，虚拟机IP地址需要与主机在同一个网段，如果需要联网，则网关与DNS需要与主机网卡一致。其网络结构如下所示：

![vmware-bridge](https://ivanzz1001.github.io/records/assets/img/linux/vmware_bridge.jpg)

### 2.1 如何设置桥接模式
首先，安装完系统之后，在开启系统之前，点击“编辑虚拟机设置”来设置网卡模式：

![vmware-bridge-step1](https://ivanzz1001.github.io/records/assets/img/linux/vmware_bridge_step1.jpg)


点击“网络适配器”，选择“桥接模式”，然后“确定”:

![vmware-bridge-step2](https://ivanzz1001.github.io/records/assets/img/linux/vmware_bridge_step2.jpg)

在进入系统之前，我们先确认一下主机的ip地址、网关、DNS等信息。

![vmware-bridge-step3](https://ivanzz1001.github.io/records/assets/img/linux/vmware_bridge_step3.jpg)

然后，进入系统编辑网卡配置文件，命令为：
<pre>
# vi /etc/sysconfig/network-scripts/ifcfg-eth0
</pre>
![vmware-bridge-step4](https://ivanzz1001.github.io/records/assets/img/linux/vmware_bridge_step4.jpg)

添加内容如下：

![vmware-bridge-step5](https://ivanzz1001.github.io/records/assets/img/linux/vmware_bridge_step5.jpg)

>注：上面我们采用的是静态配置IP，我们也可以采用如下方式动态获取IP
<pre>
TYPE="Ethernet"
BOOTPROTO="dhcp"
DEFROUTE="yes"
PEERDNS="yes"
PEERROUTES="yes"
IPV4_FAILURE_FATAL="no"
IPV6INIT="yes"
IPV6_AUTOCONF="yes"
IPV6_DEFROUTE="yes"
IPV6_PEERDNS="yes"
IPV6_PEERROUTES="yes"
IPV6_FAILURE_FATAL="no"
IPV6_ADDR_GEN_MODE="stable-privacy"
NAME="eth0"
UUID="612fd539-db07-4c9c-9e1d-4c6d41c12652"
DEVICE="eth0"
ONBOOT="yes"
</pre>

编辑完成，保存退出，然后重启虚拟机网卡，使用ping命令ping外网ip，测试能否联网。

![vmware-bridge-step6](https://ivanzz1001.github.io/records/assets/img/linux/vmware_bridge_step6.jpg)

主机与虚拟机通信正常。

### 2.2 Linux下重启网络的几种方式

1. service network restart
<pre>
1) 首先用CRT工具连bai接到Linux命令行界面。du 或者进入操作系统界面，选zhi择终端输入；

2) 如对所有的网卡进行重启操作， 可以尝试输入：service network restart 命令进行操作；

3) 完成了用service network restart命令重启网卡的操作。
</pre>


2. ifconfig eth0 down / ifconfig eth0 up
<pre>
1) 连接到命令行界面，输入ifconfig查看网卡的基本信息；

2) 查看到eth3的网卡信息。 输入ifconfig eth3 down ,卸载eth3网卡；

3) 输入ifconfig eth3 up,重新加载eth3网卡。
</pre>


3. ifdown eth0 / ifup eth0
<pre>
1) 连接到命令行界面。输入ifdown eth3,对网卡eth3进行卸载；

2) 输入ifup eth3,对网卡eth3进行重新加载；

3) 这样就完成了对网卡的重启操作。
</pre>


<br />
<br />

**[参看]:**

1. [vmware 虚拟机三种网络模式、桥接、NAT仅主机工作原理及配置详解](https://blog.csdn.net/weixin_44786530/article/details/89509875)

2. [linux中如何重启指定网卡](https://zhidao.baidu.com/question/1900165177221518940.html)

