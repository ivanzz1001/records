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

健康检查和失败切换是```keepalived```的两大核心功能。所谓的健康检查，就是采用tcp三次握手、http请求、udp echo请求等方式对负载均衡器后面的实际服务器进行保活检测；而失败切换主要是应用于配置了主备模式的负载均衡器，利用VRRP(虚拟路由冗余协议，可参考[RFC文档](http://tools.ietf.org/html/rfc5798) )维持主备负载均衡器的心跳，当主负载均衡器出现问题时，由备负载均衡器承载对应的业务，从而在最大限度上减少流量损失，并保证服务的稳定性。

### 1.1 keepalived使用的内核组件

keepalived使用四个Linux内核组件：

* LVS框架： 用于实现数据流量的负载均衡

* Netfilter框架： 支持NAT和IP伪装

* Netlink接口： 设置和删除网络接口上的VRRP虚拟IP

* 组播： 将VRRP通告发送到保留的VRRP MULTICAST组(224.0.0.18)

![lvs-keepalived-arch](https://ivanzz1001.github.io/records/assets/img/lb/lb_keepalived_arch.jpg)

上图是```keepalived```的功能体系结构，大致分为两层： 用户空间(user space)和内核空间(kernel space)。

1） **内核空间**

主要包括```IPVS```(IP虚拟服务器，用于实现网络服务的负载均衡）和```NETLINK```(提供高级路由及其他相关的网络功能）两个部分。

2） **用户空间**

* WatchDog: 负责监控checkers以及VRRP进程的状况

* VRRP Stack: 负责负载均衡器之间的失败切换Failover，如果只用一个负载均衡器，则VRRP不是必须的。

* Checkers： 负责真实服务器的健康检查health checking，是keepalived最主要的功能。换言之，可以没有VRRP Stack，但健康检查health checking是一定要有的。

* IPVS wrapper: 发送用户设置的规则到内核ipvs代码

* Netlink Reflector: 用来设定vrrp的vip地址等  

## 2. VRRP协议
VRRP全称Virtual Router Redundancy Protocol，即虚拟路由冗余协议。可以认为它是实现路由器高可用的容错协议，即将N台提供相同功能的路由器组成一个```路由器组```(Router Group)，这个组里有一个master和多个backup，但在外界看来就像一台一样，构成虚拟路由器，拥有一个虚拟IP(vip，也就是路由器所在局域网内其他机器的默认路由），占有这个IP的```master```实际负责```ARP```响应和转发IP数据包，组中的其他路由器作为备份的角色处于待命状态。master会发送组播消息，当backup在超时时间内收不到vrrp包时就认为master宕掉了，这时就需要根据VRRP的优先级来选举一个backup充当新的master，保证路由器的高可用。

在VRRP协议实现里，虚拟路由器使用```00-00-5E-00-01-XX```作为虚拟MAC地址，```XX```就是唯一的VRID(Virtual Router IDentifier)，这个地址同一时间只有一个物理路由器占用。在虚拟路由器里面的物理路由器组通过多播IP地址```224.0.0.18```来定时发送通告消息。每个Router都有一个1-255之间的优先级别，级别最高的（highest priority)将成为主控(master)路由器。通过降低master的优先权可以让处于backup状态的路由器抢占主路由器的状态，两个backup优先级相同时IP地址较大者为master，接管虚拟IP。

![lvs-keepalived-vrrp](https://ivanzz1001.github.io/records/assets/img/lb/lb_keeplived_vrrp.jpg)



## 3. 与heartbeat/corosync等比较
Heartbeat、Corosync、Keepalived这三个集群组件到底选哪个好？ 首先我想说明的是，Keepalived与Heartbeat、Corosync根本不是同一类型的(Heartbeat、Corosync是属于同一类型)。Keepalived使用的是vrrp协议方式，即虚拟路由冗余协议(Virtual Router Redundancy Protocol，简写为VRRP); Heartbeat或CoroSync是基于主机或网络服务的高可用方式。简单的说就是，**Keepalived的目的是模拟路由器的高可用，Heartbeat或Corosync的目的是实现Service的高可用**。


所以，一般keepalived是实现前端高可用，常用的前端高可用组合有：LVS+Keeplived、Nginx+Keepalived、HAproxy+keepalived。而Heartbeat或Corosync是实现服务的高可用，常见的组合有Heartbeat v3(Corosync) + Pacemaker + NFS + Httpd实现Web服务的高可用、Heartbeat v3(Corosync) + Pacemaker + NFS + MySQL实现MySQL服务的高可用。总结一下，


## 4. keepalived的安装
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

5） **keepalived的配置**

这里我们暂时不讲解具体的keepalived各项配置，仅给出如下我们安装好后的默认配置，以作参考：
{% highlight string %}
# cat /etc/keepalived/keepalived/keepalived.conf 
! Configuration File for keepalived

global_defs {
   notification_email {
     acassen@firewall.loc
     failover@firewall.loc
     sysadmin@firewall.loc
   }
   notification_email_from Alexandre.Cassen@firewall.loc
   smtp_server 192.168.200.1
   smtp_connect_timeout 30
   router_id LVS_DEVEL
   vrrp_skip_check_adv_addr
   vrrp_strict
   vrrp_garp_interval 0
   vrrp_gna_interval 0
}

vrrp_instance VI_1 {
    state MASTER
    interface eth0
    virtual_router_id 51
    priority 100
    advert_int 1
    authentication {
        auth_type PASS
        auth_pass 1111
    }
    virtual_ipaddress {
        192.168.200.16
        192.168.200.17
        192.168.200.18
    }
}

virtual_server 192.168.200.100 443 {
    delay_loop 6
    lb_algo rr
    lb_kind NAT
    persistence_timeout 50
    protocol TCP

    real_server 192.168.201.100 443 {
        weight 1
        SSL_GET {
            url {
              path /
              digest ff20ad2481f97b1754ef3e12ecd3a9cc
            }
            url {
              path /mrtg/
              digest 9b3a0c85a887a256d6939da88aabd8cd
            }
            connect_timeout 3
            retry 3
            delay_before_retry 3
        }
    }
}

virtual_server 10.10.10.2 1358 {
    delay_loop 6
    lb_algo rr
    lb_kind NAT
    persistence_timeout 50
    protocol TCP

    sorry_server 192.168.200.200 1358

    real_server 192.168.200.2 1358 {
        weight 1
        HTTP_GET {
            url {
              path /testurl/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334d
            }
            url {
              path /testurl2/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334d
            }
            url {
              path /testurl3/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334d
            }
            connect_timeout 3
            retry 3
            delay_before_retry 3
        }
    }

    real_server 192.168.200.3 1358 {
        weight 1
        HTTP_GET {
            url {
              path /testurl/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334c
            }
            url {
              path /testurl2/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334c
            }
            connect_timeout 3
            retry 3
            delay_before_retry 3
        }
    }
}

virtual_server 10.10.10.3 1358 {
    delay_loop 3
    lb_algo rr
    lb_kind NAT
    persistence_timeout 50
    protocol TCP

    real_server 192.168.200.4 1358 {
        weight 1
        HTTP_GET {
            url {
              path /testurl/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334d
            }
            url {
              path /testurl2/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334d
            }
            url {
              path /testurl3/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334d
            }
            connect_timeout 3
            retry 3
            delay_before_retry 3
        }
    }

    real_server 192.168.200.5 1358 {
        weight 1
        HTTP_GET {
            url {
              path /testurl/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334d
            }
            url {
              path /testurl2/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334d
            }
            url {
              path /testurl3/test.jsp
              digest 640205b7b0fc66c1ea91c463fac6334d
            }
            connect_timeout 3
            retry 3
            delay_before_retry 3
        }
    }
}
{% endhighlight %}





<br />
<br />

**[参看]**

1. [keepalived官网](https://www.keepalived.org/)

2. [keepalived实现双机热备](https://www.cnblogs.com/jefflee168/p/7442127.html)

3. [VRRP协议与keepalived原理及功能实例演示](https://blog.51cto.com/13322786/2162618)

<br />
<br />
<br />


