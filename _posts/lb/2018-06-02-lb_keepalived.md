---
layout: post
title: keepalived实现双机热备
tags:
- lb
categories: lb
description: keepalived实现双机热备
---

本文主要讲述一下keepalived的工作原理，及如何实现双机热备

<!-- more -->

## 1. keepalived简介

keepalved软件完全是由C语言编写的。该项目的主要目标为Linux操作系统是提供一个简便、鲁棒的方式实现负载均衡和高可用性。负载均衡架构依赖于著名并被广泛使用的```LVS```四层负载均衡架构。keepalived实现了一系列的```checkers```,可以通过检查各负载均衡器的健康状况来动态的监测和管理各负载均衡服务。另一方面，高可用性是通过```VRRP```（Virtual Router Redundancy Protocol，虚拟路由冗余协议协议）来实现的。VRRP是一种应对路由失败的基础设施。

此外，keepalived为VRRP有限状态机实现了一系列的钩子(hooks)以提供```底层```（low-level)和```高速```的协议交互。为了提供最快速的网络失败检测，keepalived实现了BFD协议。VRRP状态装换可以把BFD命中考虑在内以更快速的驱动状态机转换。keepalived框架可以被单独的使用，也可以配合LVS等一起使用以提供更富弹性的基础设施。

简而言之，keepalived提供了两个主要功能：

* 对LVS系统进行健康检查

* 实施VRRPv2堆栈以处理负载均衡器故障转移


1） **keepalived使用的内核组件**

keepalived使用四个Linux内核组件：

* LVS框架： 用于实现数据流量的负载均衡

* Netfilter框架： 支持NAT和IP伪装

* Netlink接口： 设置和删除网络接口上的VRRP虚拟IP

* 组播： 将VRRP通告发送到保留的VRRP MULTICAST组(224.0.0.18)

![lvs-keepalived-arch](https://ivanzz1001.github.io/records/assets/img/lb/lb_keepalived_arch.jpg)


## 2. keepalived的安装
当前我们的操作系统环境为：
<pre>
# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
     
# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core) 
</pre>
如下介绍具体的安装步骤：

1） **下载软件安装包并解压**
<pre>
# mkdir keepalived_setup
# cd keepalived_setup
# wget https://www.keepalived.org/software/keepalived-2.0.17.tar.gz
# tar -zxvf keepalived-2.0.17.tar.gz
# cd keepalived-2.017
</pre>


2) **安装依赖文件**

查看```keepalived```的安装说明：
{% highlight string %}
RedHat based systems (RedHat Enterprise/CentOS/Fedora)
------------------------------------------------------
The following build packages are needed:
        make
        autoconf automake (to build from git source tree rather than tarball)
The following libraries need to be installed:
        openssl-devel libnl3-devel ipset-devel iptables-devel
For magic file identification support:
        file-devel
For SNMP support:
        net-snmp-devel
For DBUS support:
        glib2-devel
For JSON support:
        json-c-devel
For PCRE support
        pcre2-devel
For nftables support
        libnftnl-devel libmnl-devel

For building the documentation, the following packages need to be installed:
        Fedora: python-sphinx (will pull in: python2-sphinx_rtd_theme)
        CentOS-and-friends: python-sphinx epel-release python-sphinx_rtd_theme
   For latex or pdf files, the following are also needed:
        Fedora: latexmk python-sphinx-latex
        CentOS-and-friends: latexmk texlive texlive-titlesec texlive-framed texlive-threeparttable texlive-wrapfig texlive-multirow
{% endhighlight %}
这里我们根据需要安装对应的依赖包（如果某些安装包直接采用```yum install```找不到的话，也可以到http://www.rpmfind.net/linux/rpm2html/search.php进行查找)。如下我们执行安装：
<pre>
# yum list installed | grep openssl
Repodata is over 2 weeks old. Install yum-cron? Or run: yum makecache fast
openssl.x86_64                         1:1.0.2k-16.el7_6.1             @updates 
openssl-devel.x86_64                   1:1.0.2k-16.el7_6.1             @updates 
openssl-libs.x86_64                    1:1.0.2k-16.el7_6.1             @updates

# yum list installed | grep pcre
Repodata is over 2 weeks old. Install yum-cron? Or run: yum makecache fast
pcre.x86_64                            8.32-17.el7                     @base    
pcre-devel.x86_64                      8.32-17.el7                     @base  

# yum list installed | grep iptables
Repodata is over 2 weeks old. Install yum-cron? Or run: yum makecache fast
iptables.x86_64                        1.4.21-17.el7                   @anaconda

# yum list installed | grep "snmp"
Repodata is over 2 weeks old. Install yum-cron? Or run: yum makecache fast
net-snmp-libs.x86_64                   1:5.7.2-24.el7_2.1              @anaconda
</pre>

3) **安装keepalived**
{% highlight string %}
# mkdir -p /usr/local/keepalived
# mkdir -p /etc/keepalived
# ./configure --prefix=/usr/local/keepalived --sbindir=/usr/local/sbin --sysconfdir=/etc/keepalived/
# make
# make install
{% endhighlight %}
安装完成之后，可以看到如下：
<pre>
# tree /usr/local/sbin/
/usr/local/sbin/
└── keepalived

0 directories, 1 file

# tree /usr/local/keepalived/
/usr/local/keepalived/
├── bin
│   └── genhash
└── share
    ├── doc
    │   └── keepalived
    │       └── README
    ├── man
    │   ├── man1
    │   │   └── genhash.1
    │   ├── man5
    │   │   └── keepalived.conf.5
    │   └── man8
    │       └── keepalived.8
    └── snmp
        └── mibs

10 directories, 5 files

# tree /etc/keepalived/
/etc/keepalived/
├── keepalived
│   ├── keepalived.conf
│   └── samples
│       ├── client.pem
│       ├── dh1024.pem
│       ├── keepalived.conf.conditional_conf
│       ├── keepalived.conf.fwmark
│       ├── keepalived.conf.HTTP_GET.port
│       ├── keepalived.conf.inhibit
│       ├── keepalived.conf.IPv6
│       ├── keepalived.conf.misc_check
│       ├── keepalived.conf.misc_check_arg
│       ├── keepalived.conf.quorum
│       ├── keepalived.conf.sample
│       ├── keepalived.conf.SMTP_CHECK
│       ├── keepalived.conf.SSL_GET
│       ├── keepalived.conf.status_code
│       ├── keepalived.conf.track_interface
│       ├── keepalived.conf.virtualhost
│       ├── keepalived.conf.virtual_server_group
│       ├── keepalived.conf.vrrp
│       ├── keepalived.conf.vrrp.localcheck
│       ├── keepalived.conf.vrrp.lvs_syncd
│       ├── keepalived.conf.vrrp.routes
│       ├── keepalived.conf.vrrp.rules
│       ├── keepalived.conf.vrrp.scripts
│       ├── keepalived.conf.vrrp.static_ipaddress
│       ├── keepalived.conf.vrrp.sync
│       ├── root.pem
│       ├── sample.misccheck.smbcheck.sh
│       └── sample_notify_fifo.sh
└── sysconfig
    └── keepalived

3 directories, 30 files
</pre>

4) **设置keepalived开机启动**

默认情况下，安装完成之后，已经在/usr/lib/systemd/system目录下为我们生成了一个```keeepalived.service```，如果没有可以在keepalived的源代码目录下将```keepalived.server```文件拷贝到该目录：
<pre>
# ls -al /usr/lib/systemd/system/keepalived.service
-rw-r--r-- 1 root root 383 Jul  7 20:05 /usr/lib/systemd/system/keepalived.service
</pre>
执行如下命令设置为开机启动：
<pre>
# systemctl enable keepalived
Created symlink from /etc/systemd/system/multi-user.target.wants/keepalived.service to /usr/lib/systemd/system/keepalived.service.
[root@localhost keepalived-2.0.17]# systemctl daemon-reload
</pre>








<br />
<br />

**[参看]**

1. [keepalived官网](https://www.keepalived.org/)

2. [keepalived实现双机热备](https://www.cnblogs.com/jefflee168/p/7442127.html)

<br />
<br />
<br />


