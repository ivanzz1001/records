---
layout: post
title: Linux下路由及网关的配置
tags:
- LinuxOps
categories: linuxOps
description: Linux下路由及网关的配置
---

本文主要讲述一下Linux环境下路由及网关的配置。


<!-- more -->


## 1. Linux路由表设置

### 1.1 route命令的基本用法

我们通过```route --help```查看route命令的基本用法：
{% highlight string %}
# route --help
Usage: route [-nNvee] [-FC] [<AF>]           List kernel routing tables
       route [-v] [-FC] {add|del|flush} ...  Modify routing table for AF.

       route {-h|--help} [<AF>]              Detailed usage syntax for specified AF.
       route {-V|--version}                  Display version/author and exit.

        -v, --verbose            be verbose
        -n, --numeric            don't resolve names
        -e, --extend             display other/more information
        -F, --fib                display Forwarding Information Base (default)
        -C, --cache              display routing cache instead of FIB

  <AF>=Use -4, -6, '-A <af>' or '--<af>'; default: inet
  List of possible address families (which support routing):
    inet (DARPA Internet) inet6 (IPv6) ax25 (AMPR AX.25) 
    netrom (AMPR NET/ROM) ipx (Novell IPX) ddp (Appletalk DDP) 
    x25 (CCITT X.25) 
{% endhighlight %}
上面可以看到，我们可以使用```route```命令来列出或修改当前路由表信息。

下面我们来看一下当前系统的Linux内核路由表信息：
<pre>
# route
Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
default         bogon           0.0.0.0         UG    100    0        0 ens33
192.168.79.0    0.0.0.0         255.255.255.0   U     100    0        0 ens33
192.168.122.0   0.0.0.0         255.255.255.0   U     0      0        0 virbr0


# route -n
Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
0.0.0.0         192.168.79.2    0.0.0.0         UG    100    0        0 ens33
192.168.79.0    0.0.0.0         255.255.255.0   U     100    0        0 ens33
192.168.122.0   0.0.0.0         255.255.255.0   U     0      0        0 virbr0
</pre>

这里我们简单介绍一下route命令的各输出项：

* **Destination**: 目标网络(network)或者目标主机(host)

* **Gateway**: 网关地址，```*```表示并未设置网关地址；

* **Genmask**: 目标网络。其中'255.255.255'用于指示单一目标主机，'0.0.0.0'用于指示默认路由
<pre>
这里再说明一下网络掩码为0.0.0.0和255.255.255.255时表示的含义：

1） 因为0.0.0.0表示所有地址，例如默认路由0.0.0.0  192.168.79.2  0.0.0.0表示所有地址通过192.168.79.2转发；

2） 255.255.255.255是受限广播地址，但在实际使用中，你会看到用255.255.255.255作为子网掩码，此时它表示单一主机IP地址；  
</pre>

* **Flags**: 标记，可能的标记如下
<pre>
U (route is up)： 路由是活动的

H (target is a host)： 目标是一个主机而非网络

G (use gateway)： 需要透过外部的主机（gateway)来转递封包

R (reinstate route for dynamic routing)： 使用动态路由时，对动态路由进行复位设置的标志

D (dynamically installed by daemon or redirect)： 由后台程序或转发程序动态安装的路由

M (modified from routing daemon or redirect)： 由后台程序或转发程序修改的路由

A (installed by addrconf)： 由addrconf安装的路由

C (cache entry)： 缓存的路由信息

!  (reject route)： 这个路由将不会被接受（主要用来抵御不安全的网络）
</pre>

* **Metric**: 路由距离，到达指定网络所需的中转数。当前Linux内核并未使用，但是routing daemons可能会需要这个值

* **Ref**: 路由项引用次数（linux内核中没有使用）

* **Use**: 此路由项被路由软件查找的次数

* **Iface**: 当前路由会使用哪个接口来发送数据
 

### 1.2 三种路由类型

1） **主机路由**

主机路由是路由选择表中指向单个IP地址或主机名的路由记录。主机路由的Flags字段为```H```。例如，在下面的示例中，本地主机通过IP地址192.168.1.1的路由器到达IP地址为10.0.0.10的主机：
<pre>
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
-----------     -----------     --------------  ----- ------ ---    --- ------
10.0.0.10       192.168.1.1     255.255.255.255 UH    100    0        0 eth0
</pre>

2) **网络路由**

网络路由是代表主机可以到达的网络。网络路由的Flags字段为```G```。例如，在下面的示例中，本地主机将发送到网络192.19.12.0的数据包转发到IP地址为192.168.1.1的路由器上：
<pre>
Destination    Gateway       Genmask        Flags    Metric    Ref     Use    Iface
-----------    -------       -------        -----    -----     ---     ---    -----
192.19.12.0   192.168.1.1    255.255.255.0   UG      0         0       0      eth0
</pre>

3) **默认路由**

当主机不能在路由表中查找到目标主机的IP地址或网络路由时，数据包就被发送到默认路由（默认网关）上。默认路由的Flags字段为G。例如，在下面的示例中，默认路由是指IP地址为192.168.1.1的路由器。
<pre>
Destination    Gateway       Genmask    Flags     Metric    Ref    Use    Iface
-----------    -------       -------    -----    ------     ---    ---    -----
default       192.168.1.1    0.0.0.0    UG        0         0      0      eth0
<pre>

### 1.3 配置静态路由

1） **route命令**

设置和查看路由表都可以用route命令，设置内核路由表的命令格式是：
<pre>
# route  [add|del] [-net|-host] target [netmask Nm] [gw Gw] [[dev] If]
</pre>

其中：

* add: 添加一条路由规则

* del: 删除一条路由规则

* -net: 目的地址是一个网络

* -host: 目的地址是一台主机

* target: 目的网络或主机

* netmask: 目的地址的网络掩码

* gw: 路由数据包通过的网关

* dev If: 强制路由关联到指定的设备接口，否则的话内核会其自身的相应规则决定选用那个设备接口。在大多数正常的网络中你可能并不需要指定本项。假如```dev If```是命令行的最后一个选项的话，那么关键字```dev```可以省略。


2） **route命令使用举例**

* 添加到主机的路由
<pre>
# route add -host 192.168.1.2 dev eth0 

# route add -host 10.20.30.148 gw 10.20.30.40
</pre>
一般来说，对于主机路由可以不用指定```netmask```，因为其默认就是255.255.255.255。而网关如果不指定的话，则默认为0.0.0.0(表示未指定）


* 添加到网络的路由
<pre>
# route add -net 10.20.30.40 netmask 255.255.255.248 eth0   #添加10.20.30.40的网络

# route add -net 10.20.30.48 netmask 255.255.255.248 gw 10.20.30.41 #添加10.20.30.48的网络

# route add -net 192.168.1.0/24 eth1
</pre>


* 添加默认路由
<pre>
# route add default gw 192.168.1.1
</pre>


* 删除路由
<pre>
# route del -host 192.168.1.2 dev eth0:0

# route del -host 10.20.30.148 gw 10.20.30.40

# route del -net 10.20.30.40 netmask 255.255.255.248 eth0

# route del -net 10.20.30.48 netmask 255.255.255.248 gw 10.20.30.41

# route del -net 192.168.1.0/24 eth1

# route del default gw 192.168.1.1
</pre>

3) **设置包转发**

在Centos中默认的内核配置已经包含了路由功能，但默认并没有在系统启动时启用此功能。开启Linux的路由功能可以通过调整内核的网络参数来实现。要配置和调整内核参数可以使用```sysctl```命令。例如： 要开启Linux内核的数据包转发功能可以使用如下命令
<pre>
# sysctl -a     # 查看所有内核参数
# sysctl -w net.ipv4.ip_forward=1
</pre>

这样设置之后，当前系统就能实现包转发，但下次启动计算机时将失效。为了使在下次启动计算机时仍然有效，需要将下面的行写入配置文件'/etc/sysctl.conf'文件中：
<pre>
# vi /etc/sysctl.conf
net.ipv4.ip_forward = 1
</pre>

用户还可以使用如下命令查看当前系统是否支持包转发：
<pre>
# sysctl net.ipv4.ip_forward
</pre>





<br />
<br />

**[参看]**

1. [LINUX下网关地址配置](https://blog.csdn.net/KgdYsg/article/details/78102439)

2. [linux 路由表设置 之 route 指令详解](https://blog.csdn.net/vevenlcf/article/details/48026965)

3. [linux_下IP、网关、DNS地址配置](http://www.cnblogs.com/xuzhiwei/p/3560553.html)

4. [linux route命令的使用详解](https://www.cnblogs.com/snake-hand/p/3143041.html)

<br />
<br />
<br />


