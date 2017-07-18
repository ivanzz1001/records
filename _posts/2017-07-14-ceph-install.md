---
layout: post
title: ceph手动安装
tags:
- ceph
categories: ceph
description: ceph手动安装
---

本文主要讲述在无外网条件下安装ceph存储集群的过程。具体的安装环境如下：

<!-- more -->
<pre>
[root@ceph001-node1 /]# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.1.1503 (Core) 
Release:        7.1.1503
Codename:       Core

[root@ceph001-node1 /]# uname -a
Linux ceph001-node1 3.10.0-229.el7.x86_64 #1 SMP Fri Mar 6 11:36:42 UTC 2015 x86_64 x86_64 x86_64 GNU/Linux
</pre>

这里采用了3台虚拟机：

|        主机IP          |         部署组件             |     主机名      |
|:----------------------:|:--------------------------:|:---------------:|
| 10.133.134.211         |         node1              |  ceph001-node1 |
| 10.133.134.212         |         node2              |  ceph001-node2 |
| 10.133.134.213         |         node3              |  ceph001-node3 |



## 下载软件安装包

因为我们是在无外网环境下安装ceph，因此我们需要在另外一台能够联网的机器上下载到对应的软件安装包。

*注意：这里我们的下载软件包的主机环境最好与实际的安装环境一致，以免后面有些软件依赖出现问题*
<br />

### ADD KEYS

添加key到你的系统受信任keys列表来避免一些安全上的警告信息，对于major releases(例如hammer，jewel)请使用release.asc。

我们先从https://download.ceph.com/keys/release.asc 下载对应的release.asc文件，上传到集群的每一个节点上，执行如下命令：
<pre>
sudo rpm --import './release.asc'
</pre>
<br />

### DOWNLOAD PACKAGES

假如你是需要在无网络访问的防火墙后安装ceph集群，在安装之前你必须要获得相应的安装包。

``注意，你的连接外网的下载ceph安装包的机器环境与你实际安装ceph集群的环境最好一致，否则可能出现安装包版本不一致的情况而出现错误。``。

**RPM PACKAGES**

先新建三个文件夹dependencies、ceph、ceph-deploy分别存放下面下载的安装包。

1)	Ceph需要一些额外的第三方库。添加EPEL repository，执行如下命令：
<pre>
sudo yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
</pre>

Ceph需要如下一些依赖包：

* snappy
* leveldb
* gdisk
* python-argparse
* gperftools-libs

现在一台可以连接外网的主机上下载这些依赖包，存放在dependencies文件夹：
<pre>
sudo yumdownloader snappy
sudo yumdownloader leveldb
sudo yumdownloader gdisk
sudo yumdownloader python-argparse
sudo yumdownloader gperftools-libs
</pre>

2)	安装yum-plugin-priorities
<pre>
sudo yum install yum-plugin-priorities
</pre>
>
修改/etc/yum/pluginconf.d/priorities.conf文件：
<pre>
[main]
enabled = 1
</pre>


3)	通过如下的命令下载适当版本的ceph安装包
<pre>
su -c 'rpm -Uvh https://download.ceph.com/rpm-{release-name}/{distro}/noarch/ceph-{version}.{distro}.noarch.rpm'
</pre>

也可以直接到官方对应的网站去下载：`https://download.ceph.com/`


这里我们在CentOS7.1上配置如下：
<pre>
su -c 'rpm -Uvh https://download.ceph.com/rpm-jewel/el7/noarch/ceph-release-1-0.el7.noarch.rpm'
</pre>

修改/etc/yum.repos.d/ceph.repo文件，添加priority项：

![manual-inst-priority](https://ivanzz1001.github.io/records/assets/img/ceph/manual-inst/manual-inst-priority.jpg)


下载ceph安装包：
<pre>
yum clean packages            #先清除本地可能缓存的一些包
yum clean
yum repolist
yum makecache
sudo yum install --downloadonly --downloaddir=/ceph-cluster/packages/ceph ceph ceph-radosgw
</pre>

``NOTE1：``这里我们目前就下载ceph,ceph-radosgw两个包，其依赖的一些包会自动下载下来。如果在实际安装中，仍缺少一些依赖包，我们可以通过yum search ${package-name} 查找到该包，然后再下载下来.

``NOTE2:`` 上面这是下载最新版本的ceph安装包，如果下载其他版本，请携带上版本号


4） 下载ceph-deploy安装包

这里我们是手动安装，可以不用下载。
<pre>
sudo yum install --downloadonly --downloaddir=/ceph-cluster/packages/ceph-deploy ceph-deploy
</pre>

5)	将上述的依赖包分别打包压缩称dependencies.tar.gz、ceph.tar.gz、ceph-deploy.tar.gz，并上传到集群的各个节点上来进行安装

<br />



## 安装软件包

1） 建立相应的构建目录

这里我们统一采用如下目录来完成整个集群的安装：
<pre>
mkdir -p /ceph-cluster/build/script
mkdir -p /ceph-cluster/packages
mkdir -p /ceph-cluster/test
chmod 777 /ceph-cluster -R
</pre>

2） 关闭iptables及SELinux

可以将如下shell命令写成脚本来执行(disable-iptable-selinux.sh)：

<pre>
systemctl stop firewalld.service 
systemctl disable firewalld.service 
setenforce 0 
sed -i 's/SELINUX=enforcing/SELINUX=disabled/' /etc/selinux/config 
</pre>

3） 修改主机名

在上述所有节点上分别修改/etc/sysconfig/network文件，指定其主机名分别为`ceph001-admin、ceph001-node1、ceph001-node2、ceph001-node3`。例如：
<pre>
[root@ceph001-node1 ~]# cat /etc/sysconfig/network
# Created by anaconda
NOZEROCONF=yes
HOSTNAME=ceph001-node1
</pre>


上述修改需要在系统下次重启时才生效。

此外，我们需要分别在每一个节点上执行hostname命令来立即更改主机名称。例如：
<pre>
[root@ceph001-node1 ~]# sudo hostname ceph001-node1
[root@ceph001-node1 ~]# hostname -s
ceph001-node1
</pre>

4) 修改/etc/hosts文件

分别修改3台宿主机节点上的/etc/hosts文件
<pre>
# For Ceph Cluster
10.133.134.211	ceph001-node1
10.133.134.212	ceph001-node2
10.133.134.213	ceph001-node3
</pre>

5) 通过主机名测试各节点之间是否联通

分别测试各个主机节点是否通过主机名ping通。例如：
<pre>
[root@ceph001-node1 ~]# ping ceph001-node2
PING ceph001-node2 (10.133.134.212) 56(84) bytes of data.
64 bytes from ceph001-node2 (10.133.134.212): icmp_seq=1 ttl=64 time=0.869 ms
64 bytes from ceph001-node2 (10.133.134.212): icmp_seq=2 ttl=64 time=0.524 ms
64 bytes from ceph001-node2 (10.133.134.212): icmp_seq=3 ttl=64 time=0.363 ms
64 bytes from ceph001-node2 (10.133.134.212): icmp_seq=4 ttl=64 time=0.416 ms
</pre>

6) 检查当前CentOS内核版本是否支持rbd，并装载rbd模块
<pre>
modinfo rbd
modprobe rbd       #装载rbd模块
lsmod | grep rbd   #查看模块是否已经装载
</pre>

7) 安装ntp，并配置ceph集群节点之间的时间同步

在各节点执行如下命令：
<pre>
rpm –qa | grep ntp     #查看当前是否已经安装ntp
ps –ef | grep ntp      # 查看ntp服务器是否启动
ntpstat                 #查看当前的同步状态
</pre>

在/etc/ntp.conf配置文件中配置时间同步服务器地址

``参看：http://www.centoscn.com/CentosServer/test/2016/0129/6709.html``

8) TTY

在CentOS及RHEL上，当你尝试执行ceph-deploy时，你也许会收到一个错误。假如requiretty在你的Ceph节点上默认被设置了的话，可以执行``sudo visudo``然后定位到Defaults requiretty的设置部分，将其改变为Defaults:ceph !requiretty或者直接将其注释掉

**``NOTE:假如直接修改/etc/sudoers，确保使用sudo visudo，而不要用其他的文本编辑器``**


9) 安装ceph软件包

*安装pre-requisite 包*

在所有节点上安装如下包:
* snappy
* leveldb
* gdisk
* python-argparse
* gperftools-libs

执行如下命令进行安装：
<pre>
sudo yum localinstall *.rpm
</pre>

<br />

*安装ceph包*

在所有节点上执行如下命令安装ceph包：
<pre>
sudo yum localinstall *.rpm
</pre>

<br />


## 建立集群

### 建立monitor

我们会在ceph001-node1，ceph001-node2,ceph001-node3上分别部署monitor.请在/ceph-cluster/build目录下完成构建。


**在ceph001-node1上建立monitor**


1) 生成monitor keyring及client.admin keyring

执行如下命令生成monitor生成monitor keyring及client.admin keyring:

{% highlight string %}
ceph-authtool --create-keyring ceph.mon.keyring --gen-key -n mon. --cap mon 'allow *'
ceph-authtool --create-keyring ceph.client.admin.keyring --gen-key -n  client.admin --set-uid=0 --cap mon 'allow *' --cap osd 'allow *' --cap mds 'allow'
ceph-authtool ./ceph.mon.keyring --import-keyring  ./ceph.client.admin.keyring
cp ./ceph.client.admin.keyring /etc/ceph/ceph.client.admin.keyring
{% endhighlight %}




