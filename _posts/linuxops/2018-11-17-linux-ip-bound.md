---
layout: post
title: Linux一个网卡上绑定多个IP
tags:
- LinuxOps
categories: linuxOps
description: Linux一个网卡上绑定多个IP
---


本文介绍一下Linux环境下如何在一个网卡上绑定多个IP。当前的操作系统环境：
<pre>
# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core) 
</pre>


<!-- more -->


## 1. 给网卡绑定多个IP
在添加虚拟网卡之前，我们先来看一下我们当前的网络情况：
{% highlight string %}
# ifconfig
ens33: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.79.128  netmask 255.255.255.0  broadcast 192.168.79.255
        inet6 fe80::2a0f:9dce:2a6d:9278  prefixlen 64  scopeid 0x20<link>
        ether 00:0c:29:15:61:68  txqueuelen 1000  (Ethernet)
        RX packets 12137  bytes 1372269 (1.3 MiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 8232  bytes 1324880 (1.2 MiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 72  bytes 6256 (6.1 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 72  bytes 6256 (6.1 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

virbr0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 192.168.122.1  netmask 255.255.255.0  broadcast 192.168.122.255
        ether 52:54:00:f5:32:d3  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
{% endhighlight %}
下面我们在ens33上绑定一个新的IP地址: 192.168.79.180

1) **临时绑定**

添加虚拟网卡ens33:0(这里不一定非得从0开始，只要是0-255范围的任何数字都可以）
{% highlight string %}
# ifconfig ens33:0 192.168.79.180 broadcast 192.168.79.180 netmask 255.255.255.255
# ifconfig
ens33: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.79.128  netmask 255.255.255.0  broadcast 192.168.79.255
        inet6 fe80::2a0f:9dce:2a6d:9278  prefixlen 64  scopeid 0x20<link>
        ether 00:0c:29:15:61:68  txqueuelen 1000  (Ethernet)
        RX packets 13689  bytes 1528113 (1.4 MiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 9409  bytes 1478784 (1.4 MiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

ens33:0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.79.180  netmask 255.255.255.255  broadcast 192.168.79.180
        ether 00:0c:29:15:61:68  txqueuelen 1000  (Ethernet)

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 76  bytes 6596 (6.4 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 76  bytes 6596 (6.4 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

virbr0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 192.168.122.1  netmask 255.255.255.0  broadcast 192.168.122.255
        ether 52:54:00:f5:32:d3  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

{% endhighlight %}

然后，我们可以在另外一台机器上ping 192.168.78.180这个IP：
{% highlight string %}
# ping 192.168.79.180
PING 192.168.79.180 (192.168.79.180) 56(84) bytes of data.
64 bytes from 192.168.79.180: icmp_seq=1 ttl=64 time=0.635 ms
64 bytes from 192.168.79.180: icmp_seq=2 ttl=64 time=0.277 ms
64 bytes from 192.168.79.180: icmp_seq=3 ttl=64 time=0.289 ms
64 bytes from 192.168.79.180: icmp_seq=4 ttl=64 time=0.292 ms
{% endhighlight %}
通过上面我们可以看到，```ens33```与```ens33:0```是共用同一块网卡的，其对应的MAC地址相同。

如果要禁用虚拟网卡，操作方式和禁用物理网卡是一样的。例如：
{% highlight string %}
# ifconfig ens33:0 down
# ifconfig
ens33: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.79.128  netmask 255.255.255.0  broadcast 192.168.79.255
        inet6 fe80::2a0f:9dce:2a6d:9278  prefixlen 64  scopeid 0x20<link>
        ether 00:0c:29:15:61:68  txqueuelen 1000  (Ethernet)
        RX packets 13756  bytes 1534701 (1.4 MiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 9454  bytes 1486401 (1.4 MiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 76  bytes 6596 (6.4 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 76  bytes 6596 (6.4 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

virbr0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 192.168.122.1  netmask 255.255.255.0  broadcast 192.168.122.255
        ether 52:54:00:f5:32:d3  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
{% endhighlight %}
这里注意，使用ifconfig临时加的网卡一旦down就没了，再也up不起来。如果重启网络服务、重启系统，临时加的虚拟网卡也会消失。

2） **永久绑定**

永久生效的办法只有一个，修改配置文件。参看如下：
{% highlight string %}
# cd /etc/sysconfig/network-scripts/
# cp ifcfg-ens33 ifcfg-ens33:0
# cat ifcfg-ens33:0
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
NAME="ens33"
UUID="e1aa4838-3ecc-4b38-b44e-2095e6f6a7d5"
DEVICE="ens33"
ONBOOT="yes"
{% endhighlight %}
然后修改```ifcfg-ens33:0```这一配置文件如下：
<pre>
# cat ifcfg-ens33:0
TYPE="Ethernet"
BOOTPROTO="static"
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
NAME="ens33"
UUID="e1aa4838-3ecc-4b38-b44e-2095e6f6a7d5"
DEVICE="ens33:0"
ONBOOT="yes"
IPADDR="192.168.79.180"
NETMASK="255.255.255.255"
</pre>
修改完成之后执行如下命令重启该网卡：
{% highlight string %}
# ifup ens33:0
# ifconfig
ens33: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.79.128  netmask 255.255.255.0  broadcast 192.168.79.255
        inet6 fe80::2a0f:9dce:2a6d:9278  prefixlen 64  scopeid 0x20<link>
        ether 00:0c:29:15:61:68  txqueuelen 1000  (Ethernet)
        RX packets 14696  bytes 1619257 (1.5 MiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 10096  bytes 1573047 (1.5 MiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

ens33:0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.79.180  netmask 255.255.255.255  broadcast 192.168.79.180
        ether 00:0c:29:15:61:68  txqueuelen 1000  (Ethernet)

lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 76  bytes 6596 (6.4 KiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 76  bytes 6596 (6.4 KiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

virbr0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 192.168.122.1  netmask 255.255.255.0  broadcast 192.168.122.255
        ether 52:54:00:f5:32:d3  txqueuelen 1000  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
{% endhighlight %}
注意： 有些人在这一步喜欢用**service network restart**来重启网络，其实这是没必要的，只需要启用这张网卡就可以了。




<br />
<br />

**[参看]**

1. [Linux高级网络设置——给网卡绑定多个IP](https://www.cnblogs.com/kelamoyujuzhen/p/9126022.html)

<br />
<br />
<br />


