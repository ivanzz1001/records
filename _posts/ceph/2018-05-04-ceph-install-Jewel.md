---
layout: post
title: Jewel版本ceph安装
tags:
- ceph
categories: ceph
description: Jewel版本ceph安装
---

本文我们主要介绍一下Jewel版本的ceph安装。当前安装环境为：

<!-- more -->


[root@ceph001-node1 /]# lsb_release -a
LSB Version:    :core-4.1-amd64:core-4.1-noarch
Distributor ID: CentOS
Description:    CentOS Linux release 7.1.1503 (Core) 
Release:        7.1.1503
Codename:       Core

[root@ceph001-node1 /]# uname -a
Linux ceph001-node1 3.10.0-229.el7.x86_64 #1 SMP Fri Mar 6 11:36:42 
UTC 2015 x86_64 x86_64 x86_64 GNU/Linux
</pre>

这里采用了3台虚拟机：

|        主机IP          |         部署组件             |     主机名      |
|:----------------------:|:--------------------------:|:---------------:|
| 10.133.134.211         |         node1              |  ceph001-node1 |
| 10.133.134.212         |         node2              |  ceph001-node2 |
| 10.133.134.213         |         node3              |  ceph001-node3 |


## 1. 下载软件安装包

因为我们是在无外网环境下安装ceph，因此我们需要在另外一台能够联网的机器上下载到对应的软件安装包。

*注意：这里我们的下载软件包的主机环境最好与实际的安装环境一致，以免后面有些软件依赖出现问题*
<br />

### 1.1 ADD KEYS

添加key到你的系统受信任keys列表来避免一些安全上的警告信息，对于major releases(例如hammer，jewel)请使用release.asc。

我们先从https://download.ceph.com/keys/release.asc 下载对应的release.asc文件，上传到集群的每一个节点上，执行如下命令：
<pre>
sudo rpm --import './release.asc'
</pre>
<br />

### 1.2 DOWNLOAD PACKAGES

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
{% highlight string %}
yum clean packages            #先清除本地可能缓存的一些包
yum clean
yum repolist
yum makecache
sudo yum install --downloadonly --downloaddir=/ceph-cluster/packages/ceph ceph ceph-radosgw
{% endhighlight %}

``NOTE1：``这里我们目前就下载ceph,ceph-radosgw两个包，其依赖的一些包会自动下载下来。如果在实际安装中，仍缺少一些依赖包，我们可以通过yum search ${package-name} 查找到该包，然后再下载下来.

``NOTE2:`` 上面这是下载最新版本的ceph安装包，如果下载其他版本，请携带上版本号


4） 下载ceph-deploy安装包

这里我们是手动安装，可以不用下载。
{% highlight string %}
sudo yum install --downloadonly --downloaddir=/ceph-cluster/packages/ceph-deploy ceph-deploy
{% endhighlight %}

5)	将上述的依赖包分别打包压缩称dependencies.tar.gz、ceph.tar.gz、ceph-deploy.tar.gz，并上传到集群的各个节点上来进行安装

<br />



## 2. 安装软件包

1） 建立相应的构建目录

这里我们统一采用如下目录来完成整个集群的安装：
{% highlight string %}
mkdir -p /ceph-cluster/build/script
mkdir -p /ceph-cluster/packages
mkdir -p /ceph-cluster/test
chmod 777 /ceph-cluster -R
{% endhighlight %}

2） 关闭iptables及SELinux

可以将如下shell命令写成脚本来执行(disable-iptable-selinux.sh)：
{% highlight string %}
systemctl stop firewalld.service 
systemctl disable firewalld.service 
setenforce 0 
sed -i 's/SELINUX=enforcing/SELINUX=disabled/' /etc/selinux/config 
{% endhighlight %}


3） 修改主机名

在上述所有节点上分别修改/etc/sysconfig/network文件，指定其主机名分别为`ceph001-admin、ceph001-node1、ceph001-node2、ceph001-node3`。例如：
<pre>
[root@ceph001-node1 ~]# cat /etc/sysconfig/network
# Created by anaconda
NOZEROCONF=yes
HOSTNAME=ceph001-node1
</pre>

再调用如下命令修改：
{% highlight string %}
hostnamectl --static --transient --pretty set-hostname {host-name}
{% endhighlight %}
例如修改node1节点：
<pre>
hostnamectl --static --transient --pretty set-hostname ceph001-node1
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
{% highlight string %}
modinfo rbd
modprobe rbd       #装载rbd模块
lsmod | grep rbd   #查看模块是否已经装载
{% endhighlight %}

7) 安装ntp，并配置ceph集群节点之间的时间同步

在各节点执行如下命令：
{% highlight string %}
rpm –qa | grep ntp     #查看当前是否已经安装ntp
ps –ef | grep ntp      # 查看ntp服务器是否启动
ntpstat                 #查看当前的同步状态
{% endhighlight %}

在/etc/ntp.conf配置文件中配置时间同步服务器地址

``参看：http://www.centoscn.com/CentosServer/test/2016/0129/6709.html``

```说明：```上面查看每台虚拟机上面的同步状态信息，显示结果中的ip表示上层服务器地址，如果三台虚拟机的上层服务器ip地址一致，说明三台虚拟机已经经过ntp时间同步了，如果需要重新搭建ntp server，请参考：[http://www.178linux.com/9320](http://www.178linux.com/9320)，一般情况下虚拟机之间的时间已经经过同步了。下一步是重新搭建ntp server之后加入crontab。在monitor节点之间通过定时任务指定ntp-server进行时钟同步:
{% highlight string %}
#crontab -e
* * * * * root /usr/sbin/utpdate node7-1;/sbin/hwclock -w &> /dev/null
# service crond restart
{% endhighlight %}
注：这个过程需要在3台虚拟机都进行一遍。


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
{% highlight string %}
sudo yum localinstall *.rpm
{% endhighlight %}

<br />

*安装ceph包*

在所有节点上执行如下命令安装ceph包：
{% highlight string %}
sudo yum localinstall *.rpm
{% endhighlight %}



## 3. 建立集群

### 3.1 建立monitor

我们会在ceph001-node1，ceph001-node2,ceph001-node3上分别部署monitor.请在/ceph-cluster/build目录下完成构建。

1) 生成monitor keyring
{% highlight string %}
ceph-authtool --create-keyring ./ceph.mon.keyring --gen-key -n mon. --cap mon 'allow *'
{% endhighlight %}
查看生成的monitor keyring:
<pre>
[root@ceph001-node1 build]# cat ./ceph.mon.keyring 
[mon.]
        key = AQCG0m5Z9AifFxAAreLxG7PXYPXRNyNlRzrGhQ==
        caps mon = "allow *"
</pre>


2) 生成client.admin keyring
{% highlight string %}
ceph-authtool --create-keyring /etc/ceph/ceph.client.admin.keyring --gen-key -n  client.admin --set-uid=0 --cap mon 'allow *' --cap osd 'allow *' --cap mds 'allow'
{% endhighlight %}
查看生成的client.admin keyring：
<pre>
[root@ceph001-node1 build]# cat /etc/ceph/ceph.client.admin.keyring 
[client.admin]
        key = AQC61W5ZQlCOJRAAkXEE7xZtNiZwudgVqRtvuQ==
        auid = 0
        caps mds = "allow"
        caps mon = "allow *"
        caps osd = "allow *"
</pre>


3) 生成用于集群初始化初始化的cluster.bootstrap keyring
{% highlight string %}
cp ./ceph.mon.keyring ./cluster.bootstrap.keyring
ceph-authtool ./cluster.bootstrap.keyring --import-keyring  /etc/ceph/ceph.client.admin.keyring
{% endhighlight %}
查看生成的集群初始化keyring:
<pre>
[root@ceph001-node1 build]# cat ./cluster.bootstrap.keyring 
[mon.]
        key = AQCG0m5Z9AifFxAAreLxG7PXYPXRNyNlRzrGhQ==
        caps mon = "allow *"
[client.admin]
        key = AQC61W5ZQlCOJRAAkXEE7xZtNiZwudgVqRtvuQ==
        auid = 0
        caps mds = "allow"
        caps mon = "allow *"
        caps osd = "allow *"
</pre>


4) 生成初始化monmap

这里我们为了方便，一开始就将ceph001-node1,ceph001-node2,ceph001-node3同时作为初始化monitor。这可以减少操作步骤，但是必须要等到3个monitor同时建立完成之后monitor集群才能正常工作。
{% highlight string %}
UUID=`uuidgen`
echo $UUID
monmaptool --create --add ceph001-node1 10.133.134.211 --add ceph001-node2 10.133.134.212 --add ceph001-node3 10.133.134.213 --fsid $UUID ./bootstrap-monmap.bin
{% endhighlight %}

查看生成的bootstrap-monmap.bin文件：
<pre>
[root@ceph001-node1 build]# monmaptool --print ./bootstrap-monmap.bin 
monmaptool: monmap file ./bootstrap-monmap.bin
epoch 0
fsid ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
last_changed 2017-07-19 12:27:17.374031
created 2017-07-19 12:27:17.374031
0: 10.133.134.211:6789/0 mon.ceph001-node1
1: 10.133.134.212:6789/0 mon.ceph001-node2
2: 10.133.134.213:6789/0 mon.ceph001-node3
</pre>

5) 对ceph001-node1节点monitor初始化
{% highlight string %}
sudo ceph-mon --mkfs -i ceph001-node1 --monmap ./bootstrap-monmap.bin --keyring ./cluster.bootstrap.keyring
touch /var/lib/ceph/mon/ceph-ceph001-node1/{done,sysvinit}  #在数据目录建立done及sysvinit两个文件
{% endhighlight %}

初始化完成后，查看数据目录：
<pre>
[root@ceph001-node1 build]# ls -al /var/lib/ceph/mon/ceph-ceph001-node1/
total 4
drwxr-xr-x 3 root root 61 Jul 19 13:54 .
drwxr-xr-x 3 root root 31 Jul 19 13:54 ..
-rw-r--r-- 1 root root  0 Jul 19 13:54 done
-rw------- 1 root root 77 Jul 19 13:54 keyring
drwxr-xr-x 2 root root 80 Jul 19 13:54 store.db
-rw-r--r-- 1 root root  0 Jul 19 13:54 sysvinit
</pre>


6） 创建monitor进程配置文件

创建/etc/ceph/ceph.conf文件：
<pre>
[global] 
fsid = ba47fcbc-b2f7-4071-9c37-be859d8c7e6e 
mon_initial_members = ceph001-node1,ceph001-node2,ceph001-node3
mon_host = 10.133.134.211,10.133.134.212,10.133.134.213
auth_supported = cephx
auth_cluster_required = cephx
auth_client_required = cephx
auth_service_required = cephx
osd_pool_default_crush_rule = 2
osd_pool_default_size = 2
osd_pool_default_pg_num = 2048
osd_pool_default_pgp_num = 2048
osd_crush_chooseleaf_type = 0

[mon.ceph001-node1] 
host = ceph001-node1
mon_data = /var/lib/ceph/mon/ceph-ceph001-node1
mon_addr = 10.133.134.211:6789 
</pre>

7） 启动monitor


这里由于```J版本```使用```ceph```用户启动mon，osd, 因此这里需要将/var/lib/ceph的属组改为ceph用户：
<pre>
# chown ceph.ceph -R /var/lib/ceph/
# systemctl start ceph-mon@ceph001-node1
# systemctl status ceph-mon@ceph001-node1 
</pre>

8) 查看monitor状态

此时，因为我们配置文件中同时指定了3个initial monitors，但是目前我们只启动了1个，因此monitor会出现如下状况：
<pre>
[root@ceph001-node1 build]# ceph -s
2017-07-19 14:14:11.167910 7f7a10217700  0 -- :/1124433248 >> 10.133.134.213:6789/0 pipe(0x7f7a0c068550 sd=3 :0 s=1 pgs=0 cs=0 l=1 c=0x7f7a0c05bb80).fault
2017-07-19 14:14:14.151613 7f7a10116700  0 -- :/1124433248 >> 10.133.134.212:6789/0 pipe(0x7f7a00000c00 sd=4 :0 s=1 pgs=0 cs=0 l=1 c=0x7f7a00004ef0).fault
2017-07-19 14:14:17.152012 7f7a10217700  0 -- :/1124433248 >> 10.133.134.213:6789/0 pipe(0x7f7a000081b0 sd=3 :0 s=1 pgs=0 cs=0 l=1 c=0x7f7a0000c450).fault
</pre>


9）分发mon keyring,mon map及admin keyring

这里我们在后续建立其他monitor/osd节点时，都会用到上述生成的/ceph-cluster/build/bootstrap-monmap.bin , /ceph-cluster/build/ceph.mon.keyring 以及 /etc/ceph/ceph.client.admin.keyring 三个文件，因此这里先把这三个文件分别推送到ceph001-node2,ceph001-node3节点的对应目录里。

采用如下命令拷贝文件：
{% highlight string %}
scp /etc/ceph/ceph.client.admin.keyring root@10.133.134.212:/etc/ceph/
scp /ceph-cluster/build/bootstrap-monmap.bin root@10.133.134.212:/ceph-cluster/build/
scp /ceph-cluster/build/ceph.mon.keyring root@10.133.134.212:/ceph-cluster/build/


scp /etc/ceph/ceph.client.admin.keyring root@10.133.134.213:/etc/ceph/
scp /ceph-cluster/build/bootstrap-monmap.bin root@10.133.134.213:/ceph-cluster/build/
scp /ceph-cluster/build/ceph.mon.keyring root@10.133.134.213:/ceph-cluster/build/
{% endhighlight %}
注： 我们可以通过如下的方式从当前正常运行的集群中导出mon.keyring, mon map以及admin.keyring
{% highlight string %}
# ceph mon getmap -o ./new_monmap.bin
# monmaptool --print ./new_monmap.bin

# ceph auth get mon. -o ./new_keyfile
# cat ./new_keyfile 

// admin.keyring直接从/etc/ceph/ceph.client.admin.keyring处获取即可
{% endhighlight %}

**在ceph001-node2上建立monitor**

在上面我们已经完成了第一个monitor的初始化，建立第二个monitor就会简单很多了。并且我们已经有了用于初始化的bootstrap-monmap.bin以及ceph.mon.keyring。在/ceph-cluster/build目录下进行如下步骤：

1） 初始化monitor
{% highlight string %}
ceph-mon -i ceph001-node2 --mkfs --monmap ./bootstrap-monmap.bin --keyring ./ceph.mon.keyring
{% endhighlight %}

2) 初始化数据目录
{% highlight string %}
ceph-mon --inject-monmap ./bootstrap-monmap.bin --mon-data /var/lib/ceph/mon/ceph-ceph001-node2/
touch /var/lib/ceph/mon/ceph-ceph001-node2/{done,sysvinit} 
{% endhighlight %}

初始化后，查看monitor默认的数据目录:
<pre>
[root@ceph001-node2 build]# ls -al /var/lib/ceph/mon/ceph-ceph001-node2/
total 4
drwxr-xr-x 3 root root  61 Jul 19 14:42 .
drwxr-xr-x 3 root root  31 Jul 19 14:41 ..
-rw-r--r-- 1 root root   0 Jul 19 14:42 done
-rw------- 1 root root  77 Jul 19 14:41 keyring
drwxr-xr-x 2 root root 111 Jul 19 14:42 store.db
-rw-r--r-- 1 root root   0 Jul 19 14:42 sysvinit
</pre>

3) 修改ceph配置文件

可以从ceph001-node1节点将/etc/ceph/ceph.conf文件拷贝到ceph001-node2节点对应的目录，然后对[mon] section进行修改：
<pre>
[global] 
fsid = ba47fcbc-b2f7-4071-9c37-be859d8c7e6e 
mon_initial_members = ceph001-node1,ceph001-node2,ceph001-node3
mon_host = 10.133.134.211,10.133.134.212,10.133.134.213
auth_supported = cephx
auth_cluster_required = cephx
auth_client_required = cephx
auth_service_required = cephx
osd_pool_default_crush_rule = 2
osd_pool_default_size = 2
osd_pool_default_pg_num = 2048
osd_pool_default_pgp_num = 2048
osd_crush_chooseleaf_type = 0

[mon.ceph001-node2] 
host = ceph001-node2
mon_data = /var/lib/ceph/mon/ceph-ceph001-node2
mon_addr = 10.133.134.212:6789 
</pre>

注意上面对[mon]段的修改。

3) 启动monitor

这里由于```J版本```使用```ceph```用户启动mon，osd, 因此这里需要将/var/lib/ceph的属组改为ceph用户：
<pre>
# chown ceph.ceph -R /var/lib/ceph/
# systemctl start ceph-mon@ceph001-node2
# systemctl status ceph-mon@ceph001-node2 
</pre>



4) 查看monitor状态
<pre>
[root@ceph001-node2 build]# ceph -s
2017-07-19 14:47:11.705625 7f4f60124700  0 -- :/749124637 >> 10.133.134.213:6789/0 pipe(0x7f4f5c068550 sd=3 :0 s=1 pgs=0 cs=0 l=1 c=0x7f4f5c05bb80).fault
    cluster ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
     health HEALTH_ERR
            no osds
            1 mons down, quorum 0,1 ceph001-node1,ceph001-node2
     monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
            election epoch 2, quorum 0,1 ceph001-node1,ceph001-node2
     osdmap e1: 0 osds: 0 up, 0 in
      pgmap v2: 0 pgs, 0 pools, 0 bytes data, 0 objects
            0 kB used, 0 kB / 0 kB avail 
</pre>

上面我们可以看到，尽管我们的mon_initial_members中设置了3个monitor，但是只要启动两个就达到了PAXOS协议的法定人数，monitor集群这时已经可以工作了。但是因为我们还没有建立OSD，因此当前集群状态为HEALTH_ERR


**在ceph001-node3上建立monitor**

与ceph001-node2节点建立monitor类似，这里直接简要列出：

1）初始化monitor及数据目录
{% highlight string %}
ceph-mon -i ceph001-node3 --mkfs --monmap ./bootstrap-monmap.bin --keyring ./ceph.mon.keyring
ceph-mon --inject-monmap ./bootstrap-monmap.bin --mon-data /var/lib/ceph/mon/ceph-ceph001-node3/
touch /var/lib/ceph/mon/ceph-ceph001-node3/{done,sysvinit} 
ls -al /var/lib/ceph/mon/ceph-ceph001-node3/
{% endhighlight %}

2) 修改ceph配置文件
<pre>
[global] 
fsid = ba47fcbc-b2f7-4071-9c37-be859d8c7e6e 
mon_initial_members = ceph001-node1,ceph001-node2,ceph001-node3
mon_host = 10.133.134.211,10.133.134.212,10.133.134.213
auth_supported = cephx
auth_cluster_required = cephx
auth_client_required = cephx
auth_service_required = cephx
osd_pool_default_crush_rule = 2
osd_pool_default_size = 2
osd_pool_default_pg_num = 2048
osd_pool_default_pgp_num = 2048
osd_crush_chooseleaf_type = 0

[mon.ceph001-node3] 
host = ceph001-node3
mon_data = /var/lib/ceph/mon/ceph-ceph001-node3
mon_addr = 10.133.134.213:6789 
</pre>

3) 启动及查看相应状态

这里由于```J版本```使用```ceph```用户启动mon，osd, 因此这里需要将/var/lib/ceph的属组改为ceph用户：
<pre>
# chown ceph.ceph -R /var/lib/ceph/
# systemctl start ceph-mon@ceph001-node3
# systemctl status ceph-mon@ceph001-node3 

# ceph -s
    cluster ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
     health HEALTH_ERR
            no osds
     monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
            election epoch 4, quorum 0,1,2 ceph001-node1,ceph001-node2,ceph001-node3
     osdmap e1: 0 osds: 0 up, 0 in
      pgmap v2: 0 pgs, 0 pools, 0 bytes data, 0 objects
            0 kB used, 0 kB / 0 kB avail
</pre>
如上所示，3个monitor已经正常启动，只是因为我们还未添加任何OSD，导致当前集群处于HEALTH_ERR状态。


### 3.2 建立OSD

我们目前会在每一个节点上部署3个OSD，总共3个节点则一共会部署9个OSD。首先查看我们当前的硬盘信息：
<pre>
[root@ceph001-node1 build]# lsblk -a
NAME   MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
sr0     11:0    1  436K  0 rom  
vda    253:0    0   20G  0 disk 
└─vda1 253:1    0   20G  0 part /
vdb    253:16   0  100G  0 disk 
vdc    253:32   0  100G  0 disk 
vdd    253:48   0   15G  0 disk 
</pre>





<br />
<br />

**[参看]**

1. [MULTI-SITE](http://docs.ceph.com/docs/master/radosgw/multisite/)


<br />
<br />
<br />

