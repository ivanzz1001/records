---
layout: post
title: Centos7网络设置
tags:
- LinuxOps
categories: linux
description: linux调试
---


本文我们介绍一下centos7网络接口配置文件。当前系统环境：
<pre>
# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core) 
</pre>


<!-- more -->

## 1. Interface Configuration Files
Interface配置文件用于控制对应的网络接口。在系统启动时，其会使用这些文件来决定启用哪个网络接口以及如何配置这些接口。这些文件通常被称为```ifcfg-name```，其中name对应于相应的配置文件所控制的网卡接口。

### 1.1 Ethernet Interfaces
最常用的接口配置文件就是: /etc/sysconfig/network-scripts/ifcfg-eth0，用于控制系统的第一块以太网网卡。在一个拥有多块网卡的系统中，就会有多个```ifcfg-ethX```文件（其中X对应于网卡编号）。由于每一块网卡都有其对应的配置文件，因此管理员就可以单独控制每一块网卡的工作。

>注： 下面摘抄自[red_hat_enterprise_linux-6-deployment_guide-en-us.pdf](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/pdf/deployment_guide/red_hat_enterprise_linux-6-deployment_guide-en-us.pdf)

如下是使用一个固定IP地址来配置```ifcfg-eth0```的示例：
{% highlight string %}
DEVICE=eth0
BOOTPROTO=none
ONBOOT=yes
NETMASK=255.255.255.0
IPADDR=10.0.1.27
USERCTL=no
{% endhighlight %}

配置文件中相应的值可以根据情况进行修改。比如，ifcfg-eth0配置文件中使用DHCP的话，就可以不用再配置IP信息了：
{% highlight string %}
DEVICE=eth0
BOOTPROTO=dhcp
ONBOOT=yes
{% endhighlight %}
NetworkManager是一个图形配置工具，用于提供便捷的方法来修改多种网络接口配置文件。请参看[NetworkManager](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/networking_guide/index)

然而，有时候我们可能也需要直接手动修改相应的配置文件。

如下是Enternet interface配置文件可配置的参数列表：

1） BONDING_OPTS=parameters

用于bonding设备的配置参数，配置于/etc/sysconfig/network-scripts/ifcfg-bondN配置文件（参看: Channel Bonding Interfaces)。这些参数等价于/sys/class/net/bonding_device/bonding中的相关参数。

我们采用这种配置方法，使得不同的bonding设备有不同的配置文件。在Redhat Enterprise Linux 6系统中，会把所有与网卡接口相关的bonding选项放置于```BONDING_OPTS```后。


2） BOOTPROTO=protocol

其中protocol可取如下值：

* none: No boot-time protocol should be used()

* bootp: The BOOTP protocol should be used

* dhcp: The DHCP protocol should be used.

* static: 静态地址协议

>注：设置固定ip的情况，BOOTPROTO设置为none也行，但是如果要设定多网口绑定bond的时候，必须设成none

3) BROADCAST=address

设置广播地址。该指令已经过时，目前可以自动通过```ipcalc```计算出。

4) DEVICE=name

用于指定物理设备的名称（注：对于动态分配的PPP设备，这里就是logical name)。

5) DHCP_HOSTNAME=name

用于设置发送给DHCP服务器的简短主机名。主要是针对某一些DHCP服务器，其要求客户端在接收一个IP地址之前指定一个主机名，在此情况下会使用此选项。

6） DHCPV6C=answer

其中answer可取如下值：

* yes: Use DHCP to obtain an IPv6 address for this interface

* no: Do not use DHCP to obtain an IPv6 address for this interface. This is the default value.

默认情况下，不管有没有设置此选择，都会被指定一个IPv6 link-local address。link-local地址是基于MAC地址自动计算出来的。

7) DHCPV6C_OPTIONS=answer

其中answer可取如下值：

* -P: 启用IPv6前缀代理

* -S: 只使用DHCP来获取无状态配置，而不获取addresses

* -N: Restore normal operation after using the ```-T``` or ```-P``` options

* -T: 使用DHCP来为本网卡获得一个临时IPv6地址

* -D: 当选择使用DHCP Unique Identifier(DUID)时覆盖默认的

>注：默认情况下，假如DHCP客户端(dhclient)运行在stateless模式(```-S```选项）下时，其会基于link-layer address(DUID-LL)创建一个DHCP Unique Identifier(DUID)；假如DHCP客户端运行在stateful模式下(不使用```-S```选项）时，其会基于link-layer address plus a timestamp(DUID-LLT)来创建一个DHCP Unique Identifier。采用```-D```选项就可以覆盖此默认行为，```-D```选项值可为```LL```或者```LLT```

8) DNS{1,2}=address

用于设置DNS服务器地址。当```PEERDNS```设置为no时，其会被写入到/etc/resolv.conf文件中

9） ETHTOOL_OPTS=options

由ethtool所支持的与设备相关的选项。例如，假如你想要强制设置为全双工100Mb:
{% highlight string %}
# ETHTOOL_OPTS="autoneg off speed 100 duplex full"
{% endhighlight %}

作为替换自定义初始化脚本的方法，我们可以使用```ETHTOOL_OPTS```来设置网卡接口速度和duplex。

10） HOTPLUG=answer

answer的可选值为：

* yes: This device should be activated when it is hot-plugged (this is the default option).

* no: This device should not be activated when it is hot-plugged

当一个bonding内核模块被加载的时候，我们可以通过```HOTPLUG=no```选项来阻止channel bonding接口被激活。

11） HWADDR=MAC-address

用于设置以太网设备的硬件地址，设置样式为```AA:BB:CC:DD:EE:FF```。该指令被用于含有多块网卡的主机上，来确保不管每一块网卡的加载顺序如何，都能够被指定一个正确的设备名称。此指令不能与```MACADDR```指令一起使用

>注： 
> 1） 持久化的设备名称现在是由/etc/udev/rules.d/70-persistent-net.rules处理
> 2)  HWADDR不能用于System z网络设备
> 3）Mapping subchannels and network device names

12） IPADDRn=address

用于设置IPv4地址，其中n为从0开始的连续正整数（例如IPADDR0)。主要用于在一个网络接口上配置多个IPv4地址，假如只配置一个IP地址的话，那么```n```是可以省略的

13) IPV6ADDR=address

用于配置网卡的第一个static或primary IPv6地址。

格式为：Address/Prefix-length。假如不指定Prefix-length的话，则默认为```/64```。注意，此项设置依赖于IPV6INIT的启用。

14） IPV6ADDR_SECONDARIES=address

用于设置一个或多个附加IPv6地址，以空格作为分割。

格式为：Address/Prefix-length。假如不指定Prefix-length的话，则默认为```/64```。注意，此项设置依赖于IPV6INIT的启用。

15） IPV6INIT=answer

answer的可选值为：

* yes: 用于初始化网卡的IPv6地址

* no: 不要初始化网卡的IPv6地址。此为默认值

在设置静态IPv6地址，或者DHCP设置IPv6地址时都要求启用本选项。

16) IPV6_AUTOCONF=answer

answer的可选值为：

* yes: 为网卡启用IPv6 autoconf配置

* no： 为网卡禁用IPv6 autoconf配置

假如启用了的话，那么将会通过运行于route中的radvd进程使用Neighbor Discovery(ND)来设置IPv6。

需要指出的是，IPV6_AUTOCONF的默认值取决于IPV6FORWARDING：

* 假如IPV6FORWARDING=yes，则IPV6_AUTOCONF默认值为no

* 假如IPV6FORWARDING=yes，则IPV6_AUTOCONF默认值为yes，且IPV6_ROUTER将不会生效


17） IPV6_MTU=value

此项是一个可选项，用于决定网卡的MTU

18) IPV6_PRIVACY=rfc3041

设置网络接口所支持的rfc3041相关的选项值。此项设置依赖于```IPV6INIT```的启用。

这里不做详细介绍。

19) LINKDELAY=time

在配置设备之前，需要等待多长时间(单位： 秒）来做链接协商。默认为5秒。

20） MACADDR=MAC-address

用于设置以太网设备的硬件地址，格式为：AA:BB:CC:DD:EE:FF

此指令用于为网络接口设置一个Mac地址，以替换由NIC所设置的值。注意，此指令不能与HWADDR指令同时使用

21） MASTER=bond-interface

指定所绑定的以太网接口。此指令搭配```SLAVE```指令一起使用

22） NETMASKn=mask

用于设置掩码值，其中n为从0开始的连续正整数值（例如NETMASK0)。其用于配置绑定多IP地址的网卡。

假如网卡只配置了一个IP地址的话，那么```n```可以省略。

23) NETWORK=address

用于设置网络地址。该指令目前已经过时，其可以通过ipcalc自动计算出来。

24） NM_CONTROLLED=answer

answer的可选值为：

* yes: 允许通过NetworkManager来配置此设备。这是默认行为，可生省略不配置

* no：不允许通过NetworkManager来配置此设备。

>注： 在RedHat 6.3中，NM_CONTROLLED指令依赖于/etc/sysconfig/network中的NM_BOND_VLAN_ENABLED。当且仅当NM_BOND_VLAN_ENABLED被配置为yes/y/true时，NetworkManager才会侦测和管理VLAN接口

25） ONBOOT=answer

answer的可选值为：

* yes: 假如DNS指令被设置，或者启用了DHCP，或者使用微软RFC 1877 PPP协议的IPCP扩展，那么会将对应的DNS信息写到/etc/resolv.conf配置文件中。在所有前面列的这些情况下，ONBOOT选项的默认值都为yes

* no: 不修改/etc/resolv.conf

26) SLAVE=answer

answer的可选值为：

* yes: 表明设备由MASTER指令所指定的绑定接口所控制

* no: 表明设备不受由MASTER指令所指定的绑定接口所控制

此指令搭配```MASTER```指令一起使用。

27) SRCADDR=address

用于设置发出去的数据包的源IP地址

28） USERCTL=answer

answer的可选值为：

* yes: 允许非root用户控制此设备

* no: 禁止非root用户控制此设备。

### 1.2 通过network-scripts配置网络示例

1） 示例1： 为ifcfg-eth0配置静态IP
{% highlight string %}
TYPE=Ethernet
DEVICE=eth0
BOOTPROTO=none
ONBOOT=yes
IPADDR=10.0.1.27
NETMASK=255.255.255.0
GATEWAY=10.0.1.1
BROADCAST=10.10.1.255
HWADDR=00:0C:29:13:5D:74
PEERDNS=yes
DNS1=10.0.1.41
USERCTL=no
NM_CONTROLLED=no
IPV6INIT=yes
IPV6ADDR=FD55:faaf:e1ab:1B0D:10:14:24:106/64
{% endhighlight %}
配置后，执行如下命令重启网络：
<pre>
# service network restart
</pre>

2) 示例2： 为ifcfg-eth1配置静态IP
{% highlight string %}
DEVICE="eth1"
BOOTPROTO="static"
BROADCAST="192.168.0.255"
HWADDR="00:16:36:1B:BB:74"
IPADDR="192.168.0.100"
NETMASK="255.255.255.0"
ONBOOT="yes"
{% endhighlight %}
配置后，执行如下命令重启网络：
<pre>
# service network restart
</pre>

<br />
<br />
**[参看]:**

1. [Centos7网络文件说明](https://www.cnblogs.com/splenday/articles/7668589.html)

2. [Linux Static IP Configuration](https://www.linode.com/docs/guides/linux-static-ip-configuration/)

3. [Using NetworkManager with sysconfig files](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/networking_guide/sec-using_networkmanager_with_sysconfig_files)

4. [Linux网络接口配置文件ifcfg-eth0解析](https://blog.csdn.net/jmyue/article/details/17288467)

6. [redhat enterprise linux6 deployment guide](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/6/pdf/deployment_guide/red_hat_enterprise_linux-6-deployment_guide-en-us.pdf)

7. [redhat文档入口](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7)

<br />
<br />
<br />





