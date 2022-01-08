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


所以，一般keepalived是实现前端高可用，常用的前端高可用组合有：LVS+Keeplived、Nginx+Keepalived、HAproxy+keepalived。而Heartbeat或Corosync是实现服务的高可用，常见的组合有Heartbeat v3(Corosync) + Pacemaker + NFS + Httpd实现Web服务的高可用、Heartbeat v3(Corosync) + Pacemaker + NFS + MySQL实现MySQL服务的高可用。总结一下，Keepalived实现轻量级的高可用，一般用于前端高可用，且不需要共享存储，一般常用于两个节点之间的高可用； 而Heartbeat或Corosync一般用于服务的高可用，且需要共享存储，一般用于多个节点的高可用。

<pre>
补充：

有博友可能会问，那heartbeat与corosync我们又应该选哪个好啊？ 我想说我们一般选用corosync，因为corosync的运行机制更优于heartbeat，
就连从heartbeat分离出来的pacemaker都说在以后的开发当中更倾向于corosync，所以现在corosync+pacemaker是最佳组合。
</pre>


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
# cd keepalived-2.0.17
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
这里将*/etc/keepalived/keepalived/*目录下的文件移动到*/etc/keepalived*目录，以使后面通过```systemd```能够找到：
<pre>
# mv /etc/keepalived/keepalived/* /etc/keepalived/
# rm -rf /etc/keepalived/keepalived/
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
这里我们简单说一下keepalived的配置文件：

* 注释以```#```或者```!```开头，直到该行的结尾

* 通常由global_defs、vrrp_instance、virtual_server这3大模块组成

上面的示例中，在real_server里面加上了针对HTTP的健康检查，另外其实我们可以加上针对TCP的监看检查，例如：
{% highlight string %}
TCP_CHECK {
    connect_timeout 10
    nb_get_retry 3
    delay_before_retry 3
    connect_port 17443
}
{% endhighlight %}


## 5. Keepalived实现双机热备
keepalived的作用是检测后端TCP服务的状态。如果有一台提供TCP服务的后端节点死机，或者出现工作故障，keepalived会及时检测到，并将有故障的节点从系统中剔除；当提供TCP服务的节点恢复并且正常提供服务后keepalived会自动将TCP服务的节点加入到集群中。这些工作都是keepalived自动完成，不需要人工干涉，需要人工做的只是修复发生故障的服务器，以下通过示例来演示。测试环境如下：
<pre>
keepalived主机：  192.168.79.128
keepalived备机：  192.168.79.129

http服务器1：     192.168.79.128
http服务器2：     192.168.79.129
http服务器3：     192.168.79.131

vip:             192.168.79.180
</pre>

在进行具体工作之前，我们最好先关闭```SELinux```。执行如下命令查看当前SELinux状态：
<pre>
# getenforce
Enabled
</pre>
有两种方式来执行关闭： 临时关闭与永久关闭

* **临时关闭SELinux**
<pre>
# setenforce 0
setenforce: SELinux is disabled
</pre>

* **永久关闭**

修改*/etc/selinux/config*文件， 将*SELINUX=enforcing*改为*SELINUX=disabled*，然后重启操作系统即可。

### 5.1 安装keepalived及nginx服务器

1) **安装keepalived**

在*192.168.79.128*以及*192.168.79.129*这两台主机上安装keepalived，具体安装方法参看本文前面章节。

2） **安装nginx**

在*192.168.79.128*、*192.168.79.129*、*192.168.79.131*这三台主机上安装nginx，具体安装方法这里不做介绍。安装完成之后启动nginx服务。



### 5.2 keepalived配置

* keepalived master配置

在```192.168.79.128```主机上备份原来的```keepalived.conf```文件，然后将配置修改为如下：
{% highlight string %}
! Configuration File for keepalived 
 
vrrp_instance VI_180{
	state MASTER             #指定该节点为主节点，备用节点设置为BACKUP
	interface ens33          #绑定虚拟IP的网络接口
	
	virtual_router_id 180    #VRRP组名，两个节点设置一样，以指明各个节点同属一VRRP组
	
	priority 102             #主节点优先级，数值在1~255，注意从节点必须必主节点的优先级低
	advert_int 1             #组播信息发送间隔，两个节点需一致
	
	#设置验证信息，两个节点需一致
	authentication{
	    auth_type PASS
		auth_pass 1111
	}
	
	#指定虚拟IP，两个节点需设置一样
	virtual_ipaddress{
	    192.168.79.180
	}
} 

!include conf/*.conf

#虚拟IP服务
virtual_server 192.168.79.180 80{
	delay_loop 6             #设定检查间隔
	lb_algo rr               #指定LVS算法
	lb_kind DR               #指定LVS模式
	persistence_timeout 10   #指定LVS持久连接设置
	protocol TCP             #转发协议为TCP
	
	#后端实际TCP服务配置
	real_server 192.168.79.128 80{
	    weight 1
	}
	
	real_server 192.168.79.129 80{
	    weight 1
	}
	
	real_server 192.168.79.131 80{
	    weight 1
	}
}
{% endhighlight %}

* keepalived backup配置

在```192.168.79.129```主机上备份原来的```keepalived.conf```文件，然后将配置修改为如下(主要修改了```state```以及```priority```两个字段）：
{% highlight string %}
! Configuration File for keepalived 
 
vrrp_instance VI_180{
	state BACKUP             #指定该节点为主节点，备用节点设置为BACKUP
	interface ens33          #绑定虚拟IP的网络接口
	
	virtual_router_id 180    #VRRP组名，两个节点设置一样，以指明各个节点同属一VRRP组
	
	priority 80             #主节点优先级，数值在1~255，注意从节点必须必主节点的优先级低
	advert_int 1             #组播信息发送间隔，两个节点需一致
	
	#设置验证信息，两个节点需一致
	authentication{
	    auth_type PASS
		auth_pass 1111
	}
	
	#指定虚拟IP，两个节点需设置一样
	virtual_ipaddress{
	    192.168.79.180
	}
} 

!include conf/*.conf

#虚拟IP服务
virtual_server 192.168.79.180 80{
	delay_loop 6             #设定检查间隔
	lb_algo rr               #指定LVS算法
	lb_kind DR               #指定LVS模式
	persistence_timeout 10   #指定LVS持久连接设置
	protocol TCP             #转发协议为TCP
	
	#后端实际TCP服务配置
	real_server 192.168.79.128 80{
	    weight 1
	}
	
	real_server 192.168.79.129 80{
	    weight 1
	}
	
	real_server 192.168.79.131 80{
	    weight 1
	}
}
{% endhighlight %}


### 5.3 启动keepalived服务

1) **启动keepalived**

执行如下命令启动主备keepalived服务，并查看启动状态：
<pre>
# systemctl start keepalived             //启动主
# systemctl status keepalived
● keepalived.service - LVS and VRRP High Availability Monitor
   Loaded: loaded (/usr/lib/systemd/system/keepalived.service; enabled; vendor preset: disabled)
   Active: active (running) since Mon 2019-07-08 03:10:48 PDT; 6s ago
  Process: 103385 ExecStart=/usr/local/sbin/keepalived $KEEPALIVED_OPTIONS (code=exited, status=0/SUCCESS)
 Main PID: 103388 (keepalived)
   CGroup: /system.slice/keepalived.service
           ├─103388 /usr/local/sbin/keepalived -D
           ├─103389 /usr/local/sbin/keepalived -D
           └─103390 /usr/local/sbin/keepalived -D

Jul 08 03:10:48 localhost.localdomain Keepalived_vrrp[103390]: VRRP sockpool: [ifindex(2), family(IPv4), proto(112), unicast(0), fd(11,12)]
Jul 08 03:10:52 localhost.localdomain Keepalived_vrrp[103390]: (VI_180) Receive advertisement timeout
Jul 08 03:10:52 localhost.localdomain Keepalived_vrrp[103390]: (VI_180) Entering MASTER STATE
Jul 08 03:10:52 localhost.localdomain Keepalived_vrrp[103390]: (VI_180) setting VIPs.
Jul 08 03:10:52 localhost.localdomain Keepalived_vrrp[103390]: Sending gratuitous ARP on ens33 for 192.168.79.180
Jul 08 03:10:52 localhost.localdomain Keepalived_vrrp[103390]: (VI_180) Sending/queueing gratuitous ARPs on ens33 for 192.168.79.180
Jul 08 03:10:52 localhost.localdomain Keepalived_vrrp[103390]: Sending gratuitous ARP on ens33 for 192.168.79.180
Jul 08 03:10:52 localhost.localdomain Keepalived_vrrp[103390]: Sending gratuitous ARP on ens33 for 192.168.79.180
Jul 08 03:10:52 localhost.localdomain Keepalived_vrrp[103390]: Sending gratuitous ARP on ens33 for 192.168.79.180
Jul 08 03:10:52 localhost.localdomain Keepalived_vrrp[103390]: Sending gratuitous ARP on ens33 for 192.168.79.180

# systemctl start keepalived             //启动备
# systemctl status keepalived
● keepalived.service - LVS and VRRP High Availability Monitor
   Loaded: loaded (/usr/lib/systemd/system/keepalived.service; enabled; vendor preset: disabled)
   Active: active (running) since Mon 2019-07-08 18:17:45 CST; 2min 21s ago
  Process: 48763 ExecStart=/usr/local/sbin/keepalived $KEEPALIVED_OPTIONS (code=exited, status=0/SUCCESS)
 Main PID: 48767 (keepalived)
   Memory: 932.0K
   CGroup: /system.slice/keepalived.service
           ├─48767 /usr/local/sbin/keepalived -D
           ├─48768 /usr/local/sbin/keepalived -D
           └─48769 /usr/local/sbin/keepalived -D

Jul 08 18:17:43 localhost.localdomain Keepalived_vrrp[48769]: Registering Kernel netlink command channel
Jul 08 18:17:43 localhost.localdomain Keepalived_vrrp[48769]: Opening file '/etc/keepalived/keepalived.conf'.
Jul 08 18:17:43 localhost.localdomain Keepalived_vrrp[48769]: Assigned address 192.168.79.129 for interface ens33
Jul 08 18:17:43 localhost.localdomain Keepalived_vrrp[48769]: Assigned address fe80::7e75:c1ed:6f41:49d4 for interface ens33
Jul 08 18:17:43 localhost.localdomain Keepalived_vrrp[48769]: Registering gratuitous ARP shared channel
Jul 08 18:17:43 localhost.localdomain Keepalived_vrrp[48769]: (VI_180) removing VIPs.
Jul 08 18:17:43 localhost.localdomain Keepalived_vrrp[48769]: (VI_180) Entering BACKUP STATE (init)
Jul 08 18:17:43 localhost.localdomain Keepalived_vrrp[48769]: VRRP sockpool: [ifindex(2), family(IPv4), proto(112), unicast(0), fd(11,12)]
Jul 08 18:17:44 localhost.localdomain Keepalived_healthcheckers[48768]: Gained quorum 1+0=1 <= 3 for VS [192.168.79.180]:tcp:80
Jul 08 18:17:45 localhost.localdomain systemd[1]: Started LVS and VRRP High Availability Monitor.
</pre>

2) **查看keepalived主机IP**

在```192.168.79.128```上查看主机IP：
{% highlight string %}
# ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: ens33: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:15:61:68 brd ff:ff:ff:ff:ff:ff
    inet 192.168.79.128/24 brd 192.168.79.255 scope global dynamic ens33
       valid_lft 1444sec preferred_lft 1444sec
    inet 192.168.79.180/32 scope global ens33
       valid_lft forever preferred_lft forever
    inet6 fe80::2a0f:9dce:2a6d:9278/64 scope link 
       valid_lft forever preferred_lft forever
{% endhighlight %}
可以看到在```master keepalived```主机上绑定了vip，同样我们可以查看```backup keepalived```主机，我们看到此时并没有绑定vip。

### 5.4 测试keepalived

我们通过浏览器请求VIP上面的http服务：
<pre>
# curl -X GET http://192.168.79.180/
</pre>
可以看到服务正常返回。

接着我们关掉```master keepalived```,即关掉*192.168.79.128*上的keepalived服务，执行如下命令：
<pre>
# systemctl stop keepalived
# systemctl status keepalived
● keepalived.service - LVS and VRRP High Availability Monitor
   Loaded: loaded (/usr/lib/systemd/system/keepalived.service; enabled; vendor preset: disabled)
   Active: inactive (dead) since Mon 2019-07-08 03:34:39 PDT; 7s ago
  Process: 103385 ExecStart=/usr/local/sbin/keepalived $KEEPALIVED_OPTIONS (code=exited, status=0/SUCCESS)
 Main PID: 103388 (code=exited, status=0/SUCCESS)

Jul 08 03:34:38 localhost.localdomain systemd[1]: Stopping LVS and VRRP High Availability Monitor...
Jul 08 03:34:38 localhost.localdomain Keepalived[103388]: Stopping
Jul 08 03:34:38 localhost.localdomain Keepalived_vrrp[103390]: (VI_180) sent 0 priority
Jul 08 03:34:38 localhost.localdomain Keepalived_vrrp[103390]: (VI_180) removing VIPs.
Jul 08 03:34:38 localhost.localdomain Keepalived_healthcheckers[103389]: Shutting down service [192.168.79.128]:tcp:80 from VS [192.168.79.180]:tcp:80
Jul 08 03:34:38 localhost.localdomain Keepalived_healthcheckers[103389]: Shutting down service [192.168.79.129]:tcp:80 from VS [192.168.79.180]:tcp:80
Jul 08 03:34:38 localhost.localdomain Keepalived_healthcheckers[103389]: Shutting down service [192.168.79.131]:tcp:80 from VS [192.168.79.180]:tcp:80
Jul 08 03:34:39 localhost.localdomain Keepalived_vrrp[103390]: Stopped - used 0.019591 user time, 0.489784 system time
Jul 08 03:34:39 localhost.localdomain Keepalived[103388]: Stopped Keepalived v2.0.17 (06/25,2019)
Jul 08 03:34:39 localhost.localdomain systemd[1]: Stopped LVS and VRRP High Availability Monitor.
</pre>
之后我们查看备机*192.168.79.129*主机上的keepalived服务：
{% highlight string %}
# ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: ens33: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:6f:14:dc brd ff:ff:ff:ff:ff:ff
    inet 192.168.79.129/24 brd 192.168.79.255 scope global dynamic ens33
       valid_lft 1695sec preferred_lft 1695sec
    inet 192.168.79.180/32 scope global ens33
       valid_lft forever preferred_lft forever
    inet6 fe80::7e75:c1ed:6f41:49d4/64 scope link 
       valid_lft forever preferred_lft forever
{% endhighlight %}
可以看到vip绑定到了```192.168.79.129```主机上。然后我们再请求nginx服务，发现仍可以正常工作。


之后我们重启```192.168.79.128```主机上的keepalived服务，可以发现vip又回到了*192.168.79.128*这台master keepalived主机上，而在*192.168.79.129*这台backup keepalived主机上的vip解绑了。

### 5.5 查看vrrp数据包
我们在局域网内的三台主机上抓包：

* keepalived主机： 192.168.79.128

* keepalived备机： 192.168.79.129

* http服务器1： 192.168.79.128

执行以下命令抓包(keepalived默认多播地址是```224.0.0.18```，可以通过vrrp_mcast_group4选项进行修改）：
{% highlight string %}
# tcpdump -i ens33 -nn host 224.0.0.18
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on ens33, link-type EN10MB (Ethernet), capture size 65535 bytes
03:47:52.775392 IP 192.168.79.128 > 224.0.0.18: VRRPv2, Advertisement, vrid 180, prio 102, authtype simple, intvl 1s, length 20
03:47:53.776295 IP 192.168.79.128 > 224.0.0.18: VRRPv2, Advertisement, vrid 180, prio 102, authtype simple, intvl 1s, length 20
03:47:54.777258 IP 192.168.79.128 > 224.0.0.18: VRRPv2, Advertisement, vrid 180, prio 102, authtype simple, intvl 1s, length 20
03:47:55.778509 IP 192.168.79.128 > 224.0.0.18: VRRPv2, Advertisement, vrid 180, prio 102, authtype simple, intvl 1s, length 20
03:47:56.779592 IP 192.168.79.128 > 224.0.0.18: VRRPv2, Advertisement, vrid 180, prio 102, authtype simple, intvl 1s, length 20
{% endhighlight %}
可以看到每秒中产生一个多播数据包。
 
## 6. 补充
很多时候keepalived/fwmark还会搭配iptables来使用。一般我们可以先在iptables的mangle表对数据流量进行标记，然后在keepalived中根据相应的标记进行流量转发。例如：
{% highlight string %}
# iptables -t mangle -D PREROUTING 1          //删除规则
# iptables -A PREROUTING -d 10.4.18.69/32 -p tcp -m tcp --dport 17443 -m mac ! --mac-source F8:98:EF:7E:9E:8B -j MARK --set-xmark 0x96be3/0xffffffff

# iptables -t mangle -L
Chain PREROUTING (policy ACCEPT)
target     prot opt source               destination         
MARK       tcp  --  anywhere             server-ceph01         tcp dpt:17443 MAC ! F8:98:EF:7E:9E:8B MARK set 0x96be3
MARK       tcp  --  anywhere             server-ceph01         tcp dpt:pcsync-https MAC ! F8:98:EF:7E:9E:8B MARK set 0x10b5b
MARK       tcp  --  anywhere             server-ceph01         tcp dpt:https MAC ! F8:98:EF:7E:9E:8B MARK set 0x192b
MARK       tcp  --  anywhere             server-ceph01         tcp dpt:17480 MAC ! F8:98:EF:7E:9E:8B MARK set 0x96c08
MARK       tcp  --  anywhere             server-ceph01         tcp dpt:irdmi MAC ! F8:98:EF:7E:9E:8B MARK set 0x109a0
MARK       tcp  --  anywhere             server-ceph01         tcp dpt:http MAC ! F8:98:EF:7E:9E:8B MARK set 0x2a8
# iptables-save > /etc/sysconfig/iptables
# systemctl status iptables
{% endhighlight %}
上面第一条表示： 对于发送到目标端口为17443的数据流量（源网卡地址不为F8:98:EF:7E:9E:8B，通常为另一个副本lvs的网卡地址)，将其标志为617443(0x96be3)

>注：上面10.4.18.69应该是需要设置的vip地址

那么此时，我们可以在keepalived做如下配置以处理带有该标志的流量：
{% highlight string %}
virtual_server fwmark 617443 17443 {
    delay_loop 5
    lb_algo rr
    lb_kind DR
    persistence_timeout 0
    protocol TCP

    real_server 192.168.79.129 17443 {
		weight 1
	}

	real_server 192.168.79.131 17443 {
		weight 1
	}
}
{% endhighlight %}




<br />
<br />

**[参看]**

1. [keepalived官网](https://www.keepalived.org/)

2. [Keepalived 服务器安装与配置](https://blog.csdn.net/liupeifeng3514/article/details/79018116)

3. [keepalived实现双机热备](https://www.cnblogs.com/jefflee168/p/7442127.html)

4. [VRRP协议与keepalived原理及功能实例演示](https://blog.51cto.com/13322786/2162618)

5. [LVS + keepalived问题](https://blog.csdn.net/qq_32523587/article/details/83931550)

6. [Keepalived部署与配置详解](https://www.cnblogs.com/rexcheny/p/10778567.html)

7. [Keepalived LVS+DR合设配置方法以及存在的问题](https://www.jianshu.com/p/612a55652ae8)

<br />
<br />
<br />


