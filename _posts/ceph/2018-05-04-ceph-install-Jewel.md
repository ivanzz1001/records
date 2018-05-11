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

<pre>
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

|        主机IP                  |         部署组件             |     主机名      |
|:------------------------------:|:--------------------------:|:---------------:|
| 10.133.134.211 /10.133.135.211 |         node1              |  ceph001-node1 |
| 10.133.134.212 /10.133.135.212 |         node2              |  ceph001-node2 |
| 10.133.134.213 /10.133.135.213 |         node3              |  ceph001-node3 |

这里我们将```10.133.134.0/24```作为ceph public IP; 而将```10.133.135.0/25```作为ceph cluster IP。我们配置主机名时采用```10.133.134.0/24```段来配置。


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
# For Ceph public IP
10.133.134.211	ceph001-node1
10.133.134.212	ceph001-node2
10.133.134.213	ceph001-node3

# For Ceph cluster IP
10.133.135.211 oss-node1
10.133.135.212 oss-node2
10.133.135.213 oss-node3
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

<br />

10) 修改文件打开句柄数

* 首先查看当前```fs-max```值
<pre>
# cat /proc/sys/fs/file-max
16348794
</pre>
该值当前已经足够大，可以不用修改。

* 再查看```nr_open```值：
<pre>
# cat /proc/sys/fs/nr_open
1048576
</pre>
该值暂时也达到百万级别，可以不用修改。

* 然后再查看```soft limit```及```hard limit```限制：
<pre>
# ulimit -n
65534
</pre>
该值太小，我们做如下设置：
<pre>
# ulimit -SHn 1024000
</pre>
再修改```/etc/security/limits.conf```文件：
{% highlight string %}
# echo "root soft nofile 1024000" >> /etc/security/limits.conf

# echo "root hard nofile 1024000" >> /etc/security/limits.conf
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
[root@ceph001-node1 build]#  lsblk -l
NAME MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
sr0   11:0    1  422K  0 rom
vda  253:0    0   20G  0 disk
vda1 253:1    0   20G  0 part /
vdb  253:16   0   200G  0 disk
vdc  253:32   0  200G  0 disk
vdd  253:48   0  200G  0 disk
vde  253:64   0  15G  0 disk
</pre>

因为需要部署3副本，所以在部署osd之前要先确认是否有相应的磁盘来装载数据和日志。一般来说，虚拟机会配置4个磁盘，其中3个用来装数据，另外一个磁盘用来装3个osd对应的日志。因此，这里我们首先对ceph001-node1,ceph001-node2,ceph001-node3三个节点进行分区并格式化磁盘：
{% highlight string %}
# mkfs -t xfs -f -i size=2048 /dev/vdb
# mkfs -t xfs -f -i size=2048 /dev/vdc
# mkfs -t xfs -f -i size=2048 /dev/vdd
# parted -s /dev/vde mklabel gpt
# parted -s /dev/vde mkpart primary 0% 33%
# parted -s /dev/vde mkpart primary 33% 67%
# parted -s /dev/vde mkpart primary 67% 100%
{% endhighlight %}

查看分区后的状态：
<pre>
[root@ceph001-node1 build]# lsblk -a 
NAME   MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
sr0   11:0    1  422K  0 rom
vda  253:0    0   20G  0 disk
vda1 253:1    0   20G  0 part /
vdb  253:16   0   200G  0 disk
vdc  253:32   0  200G  0 disk
vdd  253:48   0  200G  0 disk
vde    253:48   0   15G  0 disk 
├─vde1 253:49   0  5G  0 part 
├─vde2 253:50   0  5G  0 part 
├─vde3 253:51   0  5G  0 part 
</pre>

**在ceph001-node1上建立3个OSD节点**


1) 在ceph001-node1上建立3个OSD

使用脚本在ceph001-node1上建立OSD(请分步执行如下命令，减少可能的出错机会）：
{% highlight string %}
# chmod 777 ./script/ -R
# ./script/init_osd.sh vdb vde1
# ./script/init_osd.sh vdc vde2
# ./script/init_osd.sh vdd vd33
{% endhighlight %}

2) 启动对应的OSD

我们首先需要知道通过上述脚本建立了哪些OSD。这里有几种方法可以查看：

* 到```/tmp/```目录下查看当前主机所部署的OSD：
<pre>
# ls /tmp/init_osd-*
/tmp/init_osd-0.log  /tmp/init_osd-1.log  /tmp/init_osd-2.log
</pre>

* 到```/var/lib/ceph/osd```目录下查看当前主机部署的OSD
<pre>
# ls /var/lib/ceph/osd/
ceph-0  ceph-1  ceph-2
</pre>

下面我们就启动这几个OSD,分成如下几个步骤：

* 修改osd启动用户

这里我们将osd的启动用户改为了root：
<pre>
# sed -i 's/--setuser ceph --setgroup ceph/--setuser root --setgroup root/g' /usr/lib/systemd/system/ceph-osd@.service
</pre>

* 设置为开机启动
<pre>
# systemctl enable ceph-osd@0
# systemctl enable ceph-osd@1
# systemctl enable ceph-osd@2
</pre>

* 启动osd
<pre>
# systemctl daemon-reload
# systemctl start ceph-osd@0
# systemctl status ceph-osd@0


# systemctl start ceph-osd@1
# systemctl status ceph-osd@1

# systemctl start ceph-osd@2
# systemctl status ceph-osd@2
</pre>


3） 查看OSD的启动状况
{% highlight string %}
ps -ef | grep osd
ceph -s
{% endhighlight %}
查看信息如下：
<pre>
[root@ceph001-node1 build]# ceph -s
    cluster ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
     health HEALTH_WARN
            too few PGs per OSD (0 < min 30)
     monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
            election epoch 4, quorum 0,1,2 ceph001-node1,ceph001-node2,ceph001-node3
     osdmap e10: 3 osds: 3 up, 3 in
      pgmap v14: 0 pgs, 0 pools, 0 bytes data, 0 objects
            101136 kB used, 149 GB / 149 GB avail
[root@ceph001-node1 build]# ps -ef | grep osd
root     17392     1  0 15:52 ?        00:00:00 /bin/bash -c ulimit -n 32768;  /usr/bin/ceph-osd -i 0 --pid-file /var/run/ceph/osd.0.pid -c /etc/ceph/ceph.conf --cluster ceph -f
root     17393 17392  0 15:52 ?        00:00:00 /usr/bin/ceph-osd -i 0 --pid-file /var/run/ceph/osd.0.pid -c /etc/ceph/ceph.conf --cluster ceph -f
root     17809     1  0 15:52 ?        00:00:00 /bin/bash -c ulimit -n 32768;  /usr/bin/ceph-osd -i 1 --pid-file /var/run/ceph/osd.1.pid -c /etc/ceph/ceph.conf --cluster ceph -f
root     17810 17809  0 15:52 ?        00:00:00 /usr/bin/ceph-osd -i 1 --pid-file /var/run/ceph/osd.1.pid -c /etc/ceph/ceph.conf --cluster ceph -f
root     18213     1  0 15:53 ?        00:00:00 /bin/bash -c ulimit -n 32768;  /usr/bin/ceph-osd -i 2 --pid-file /var/run/ceph/osd.2.pid -c /etc/ceph/ceph.conf --cluster ceph -f
root     18215 18213  0 15:53 ?        00:00:00 /usr/bin/ceph-osd -i 2 --pid-file /var/run/ceph/osd.2.pid -c /etc/ceph/ceph.conf --cluster ceph -f
root     18370 16930  0 15:53 pts/0    00:00:00 grep --color=auto osd
[root@ceph001-node1 build]# ceph -s
    cluster ba47fcbc-b2f7-4071-9c37-be859d8c7e6e
     health HEALTH_WARN
            too few PGs per OSD (0 < min 30)
     monmap e1: 3 mons at {ceph001-node1=10.133.134.211:6789/0,ceph001-node2=10.133.134.212:6789/0,ceph001-node3=10.133.134.213:6789/0}
            election epoch 4, quorum 0,1,2 ceph001-node1,ceph001-node2,ceph001-node3
     osdmap e10: 3 osds: 3 up, 3 in
      pgmap v14: 0 pgs, 0 pools, 0 bytes data, 0 objects
            101136 kB used, 149 GB / 149 GB avail
</pre>

**在ceph001-node2上建立3个OSD节点**

与ceph001-node1类似，只需要注意少许参数的修改，这里不在赘述。

**在ceph001-node3上建立3个OSD节点**

与ceph001-node1类似，只需要注意少许参数的修改，这里不再赘述。

### 3.3 构建crush map

在上面3.1节建立好OSD之后，默认的crush map如下图所示：
<pre>
[root@ceph001-node1 build]# ceph osd tree
ID WEIGHT  TYPE NAME              UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-1 0.44989 root default                                             
-2 0.14996     host ceph001-node1                                   
 0 0.04999         osd.0               up  1.00000          1.00000 
 1 0.04999         osd.1               up  1.00000          1.00000 
 2 0.04999         osd.2               up  1.00000          1.00000 
-3 0.14996     host ceph001-node2                                   
 3 0.04999         osd.3               up  1.00000          1.00000 
 4 0.04999         osd.4               up  1.00000          1.00000 
 5 0.04999         osd.5               up  1.00000          1.00000 
-4 0.14996     host ceph001-node3                                   
 6 0.04999         osd.6               up  1.00000          1.00000 
 7 0.04999         osd.7               up  1.00000          1.00000 
 8 0.04999         osd.8               up  1.00000          1.00000
</pre>

这不适用与我们的生产环境。下面我们就来构建我们自己的crush map.

1) 删除默认的crush map结构
{% highlight string %}
for i in {0..8}; do ceph osd crush rm osd.$i; done
for i in {1..3}; do ceph osd crush rm ceph001-node$i; done
ceph osd tree
{% endhighlight %}

执行完后，当前的ceph集群视图如下：
<pre>
[root@ceph001-node1 build]# ceph osd tree
ID WEIGHT TYPE NAME    UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-1      0 root default                                   
 0      0 osd.0             up  1.00000          1.00000 
 1      0 osd.1             up  1.00000          1.00000 
 2      0 osd.2             up  1.00000          1.00000 
 3      0 osd.3             up  1.00000          1.00000 
 4      0 osd.4             up  1.00000          1.00000 
 5      0 osd.5             up  1.00000          1.00000 
 6      0 osd.6             up  1.00000          1.00000 
 7      0 osd.7             up  1.00000          1.00000 
 8      0 osd.8             up  1.00000          1.00000 
</pre>

2) 修改crush rule规则内容
{% highlight string %}
ceph osd getcrushmap -o ./old_crushmap.bin
crushtool -d ./old_crushmap.bin -o ./old_crushmap.txt
cp old_crushmap.txt new_crushmap.txt
{% endhighlight %}

下面修改new_crushmap.txt:
<pre>
# 在type 10 root下面添加逻辑拓扑中的bucket类型, 其中数值越大, 表示在crush map中的层级越大
type 11 osd-domain
type 12 host-domain
type 13 replica-domain
type 14 failure-domain
# 将crush map中所有的 alg straw2 修改为 alg starw
</pre>

修改后重新设置到ceph集群中：
{% highlight string %}
crushtool -c new_crushmap.txt -o new_crushmap.bin
ceph osd setcrushmap -i new_crushmap.bin
ceph osd crush dump 
{% endhighlight %}

3) 重新构建crush map中的物理拓扑

请分步执行如下命令：
{% highlight string %}
for i in {0..2}; do ceph osd crush create-or-move osd.$i 0.15 host=ceph001-node1  rack=rack-01 root=default; done

for i in {3..5}; do ceph osd crush create-or-move osd.$i 0.15 host=ceph001-node2  rack=rack-02 root=default; done

for i in {6..8}; do ceph osd crush create-or-move osd.$i 0.15 host=ceph001-node3  rack=rack-03 root=default; done

ceph osd tree
{% endhighlight %}

构建完成后，查看对应的物理拓扑结构：
<pre>
[root@ceph001-node1 build]# ceph osd tree
ID WEIGHT  TYPE NAME                  UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-1 1.34995 root default                                                 
-3 0.44998     rack rack-02                                             
-2 0.44998         host ceph001-node2                                   
 3 0.14999             osd.3               up  1.00000          1.00000 
 4 0.14999             osd.4               up  1.00000          1.00000 
 5 0.14999             osd.5               up  1.00000          1.00000 
-5 0.44998     rack rack-03                                             
-4 0.44998         host ceph001-node3                                   
 6 0.14999             osd.6               up  1.00000          1.00000 
 7 0.14999             osd.7               up  1.00000          1.00000 
 8 0.14999             osd.8               up  1.00000          1.00000 
-7 0.44998     rack rack-01                                             
-6 0.44998         host ceph001-node1                                   
 0 0.14999             osd.0               up  1.00000          1.00000 
 1 0.14999             osd.1               up  1.00000          1.00000 
 2 0.14999             osd.2               up  1.00000          1.00000 
</pre>

4） 重新构建crush map中的逻辑拓扑

请分步执行如下命令：
{% highlight string %}
ceph osd crush link ceph001-node1 host-domain=host-group-0-rack-01  replica-domain=replica-0 failure-domain=sata-00

ceph osd crush link ceph001-node2 host-domain=host-group-0-rack-02  replica-domain=replica-0 failure-domain=sata-00

ceph osd crush link ceph001-node3 host-domain=host-group-0-rack-03  replica-domain=replica-0 failure-domain=sata-00

ceph osd tree
{% endhighlight %}

构建完成后，查看对应的逻辑拓扑结构：
<pre>
[root@ceph001-node1 build]# ceph osd tree
ID  WEIGHT  TYPE NAME                                UP/DOWN REWEIGHT PRIMARY-AFFINITY 
-10 1.34995 failure-domain sata-00                                                     
 -9 1.34995     replica-domain replica-0                                               
 -8 0.44998         host-domain host-group-0-rack-01                                   
 -6 0.44998             host ceph001-node1                                             
  0 0.14999                 osd.0                         up  1.00000          1.00000 
  1 0.14999                 osd.1                         up  1.00000          1.00000 
  2 0.14999                 osd.2                         up  1.00000          1.00000 
-11 0.44998         host-domain host-group-0-rack-02                                   
 -2 0.44998             host ceph001-node2                                             
  3 0.14999                 osd.3                         up  1.00000          1.00000 
  4 0.14999                 osd.4                         up  1.00000          1.00000 
  5 0.14999                 osd.5                         up  1.00000          1.00000 
-12 0.44998         host-domain host-group-0-rack-03                                   
 -4 0.44998             host ceph001-node3                                             
  6 0.14999                 osd.6                         up  1.00000          1.00000 
  7 0.14999                 osd.7                         up  1.00000          1.00000 
  8 0.14999                 osd.8                         up  1.00000          1.00000 
 -1 1.34995 root default                                                               
 -3 0.44998     rack rack-02                                                           
 -2 0.44998         host ceph001-node2                                                 
  3 0.14999             osd.3                             up  1.00000          1.00000 
  4 0.14999             osd.4                             up  1.00000          1.00000 
  5 0.14999             osd.5                             up  1.00000          1.00000 
 -5 0.44998     rack rack-03                                                           
 -4 0.44998         host ceph001-node3                                                 
  6 0.14999             osd.6                             up  1.00000          1.00000 
  7 0.14999             osd.7                             up  1.00000          1.00000 
  8 0.14999             osd.8                             up  1.00000          1.00000 
 -7 0.44998     rack rack-01                                                           
 -6 0.44998         host ceph001-node1                                                 
  0 0.14999             osd.0                             up  1.00000          1.00000 
  1 0.14999             osd.1                             up  1.00000          1.00000 
  2 0.14999             osd.2                             up  1.00000          1.00000 
</pre>

5） 构建crush rule规则
{% highlight string %}
ceph osd getcrushmap -o origin_crushmap.bin
crushtool -d origin_crushmap.bin -o origin_crushmap.txt
cp origin_crushmap.txt tobuild_crushmap.txt
{% endhighlight %}

修改tobuild_crushmap.txt文件,手动添加如下内容：
<pre>
rule replicated_rule-5 {
     ruleset 5
     type replicated
     min_size 1
     max_size 10
     step take sata-00
     step choose firstn 1 type replica-domain
     step chooseleaf firstn 0 type host-domain 
     step emit
}
</pre>
对上面```step chooseleaf firstn 0 type host-domain```说明一下： 由于当前crush map中的逻辑拓扑的层级结构为: failure-domain --> replica-domain --> host-domain, 此处就要使用host-domain而不是osd-domain。

修改完成后，重新设置到ceph集群中。
{% highlight string %}
crushtool -c tobuild_crushmap.txt -o tobuild_crushmap.bin

ceph osd setcrushmap -i tobuild_crushmap.bin

ceph osd crush dump 
{% endhighlight %}

6) 创建pool, 并将pool绑定之指定crush_ruleset
{% highlight string %}
ceph osd pool delete rbd rbd --yes-i-really-really-mean-it 

ceph osd pool create rbd-01 128 128

ceph osd pool set rbd-01 size 3

ceph osd pool set rbd-01 crush_ruleset 5
{% endhighlight %}

7) 使用rbd命令简单测试创建的pool是否能够正常使用
{% highlight string %}
rbd create rbd-01/test-image --size 4096

rbd info rbd-01/test-image

rbd rm rbd-01/test-image
{% endhighlight %}

到此为止，crush map就已经构建完毕。

## 4. 构建RGW

这里我们构建一个```multi-site```类型RGW。下面我们以realm为```oss```, master zone group为```cn```，  master zone为```cn-oss```的例子来说明：

### 4.1 配置一个master zone
1) 创建REALM
<pre>
# radosgw-admin realm create --rgw-realm=oss --default
{
    "id": "78e7a255-d6cb-440e-bd82-124d34116f95",
    "name": "oss",
    "current_period": "b376eed3-ddc5-4ea8-8c03-391013d5c021",
    "epoch": 1
}
</pre>

创建之后，可以通过如下命令获取realm信息：
<pre>
2. 获取 realm 的信息
# radosgw-admin realm get --rgw-realm=oss
{
    "id": "78e7a255-d6cb-440e-bd82-124d34116f95",
    "name": "oss",
    "current_period": "b376eed3-ddc5-4ea8-8c03-391013d5c021",
    "epoch": 1
}
</pre>

2) 创建master zone group
<pre>
# radosgw-admin zonegroup create --rgw-zonegroup=cn --rgw-realm=oss --master --default
</pre>

3) 创建master zone
<pre>
radosgw-admin zone create --rgw-zonegroup=cn --rgw-zone=cn-oss \
                            --master --default 
</pre>

4) 删除默认的zone-group,zone,realm
<pre>
# radosgw-admin zonegroup remove --rgw-zonegroup=default --rgw-zone=default
# radosgw-admin period update --commit
# radosgw-admin zone delete --rgw-zone=default
# radosgw-admin period update --commit
# radosgw-admin zonegroup delete --rgw-zonegroup=default
# radosgw-admin period update --commit

# radosgw-admin realm remove --rgw-realm=default --rgw-zonegroup=default
# radosgw-admin period update --commit
# radosgw-admin realm delete --rgw-realm=default
# radosgw-admin period update --commit
</pre>



5) 删除默认的pool

我们首先来看一下默认的pool:
<pre>
# rados lspools
.rgw.root
default.rgw.control
default.rgw.data.root
default.rgw.gc
default.rgw.log
default.rgw.users.uid
default.rgw.users.keys
default.rgw.buckets.index
default.rgw.usage
default.rgw.buckets.data
default.rgw.buckets.non-ec
</pre>
下面删除这些默认的pool(注意不要删除```.rgw.root```):

<pre>
# rados rmpool default.rgw.control default.rgw.control --yes-i-really-really-mean-it
# rados rmpool default.rgw.data.root default.rgw.data.root --yes-i-really-really-mean-it
# rados rmpool default.rgw.gc default.rgw.gc --yes-i-really-really-mean-it
# rados rmpool default.rgw.log default.rgw.log --yes-i-really-really-mean-it
# rados rmpool default.rgw.users.uid default.rgw.users.uid --yes-i-really-really-mean-it
# rados rmpool default.rgw.users.keys default.rgw.users.keys --yes-i-really-really-mean-it
# rados rmpool default.rgw.buckets.index default.rgw.buckets.index --yes-i-really-really-mean-it
# rados rmpool default.rgw.usage default.rgw.usage --yes-i-really-really-mean-it
# rados rmpool default.rgw.buckets.data default.rgw.buckets.data --yes-i-really-really-mean-it
# rados rmpool default.rgw.buckets.non-ec default.rgw.buckets.non-ec --yes-i-really-really-mean-it
</pre>

6） 配置zonegroup

首先通过命令获取到当前的zonegroup:
{% highlight string %}
# radosgw-admin zonegroup get --rgw-zonegroup=cn > zonegroup.json
{% endhighlight %}
接着修改```zonegroup.json```,将```bucket_index_max_shards```设置为8，然后在导入到rgw中：
<pre>
# radosgw-admin zonegroup set --rgw-zonegroup=cn --infile=zonegroup.json
</pre>


7) 创建一个用于跨zone同步的系统用户
<pre>
# radosgw-admin user create --uid="synchronization-user" --display-name="Synchronization User" --system
{
    "user_id": "synchronization-user",
    "display_name": "Synchronization User",
    "email": "",
    "suspended": 0,
    "max_buckets": 1000,
    "auid": 0,
    "subusers": [],
    "keys": [
        {
            "user": "synchronization-user",
            "access_key": "V9O1G3E80KBF2P280YDS",
            "secret_key": "LzmQ7y0sBvPjuQA23EQLqy8R4sgBbNz4FOfNUzDN"
        }
    ],
    "swift_keys": [],
    "caps": [],
    "op_mask": "read, write, delete",
    "system": "true",
    "default_placement": "",
    "placement_tags": [],
    "bucket_quota": {
        "enabled": false,
        "max_size_kb": -1,
        "max_objects": -1
    },
    "user_quota": {
        "enabled": false,
        "max_size_kb": -1,
        "max_objects": -1
    },
    "temp_url_keys": []
}
</pre>
这里说明一下，当secondary zones需要和master zone完成认证时，需要access_key与secret_key。

然后将系统用户添加到master zone中：
<pre>
# radosgw-admin zone modify --rgw-zone=cn-oss --access-key=V9O1G3E80KBF2P280YDS --secret=LzmQ7y0sBvPjuQA23EQLqy8R4sgBbNz4FOfNUzDN
# radosgw-admin period update --commit
</pre>


8) 更新period

在更新完master zone配置之后，需要更新period:
<pre>
# radosgw-admin period update --commit
</pre>
注意： 执行上述更新命令时，会更新epoch，然后确保其他的zone也会收到该配置。


### 4.2 在ceph001-node1上部署RGW

1) 创建存储池

在创建存储池之前，执行如下命令确保当前集群处于正常工作状态：
<pre>
# ceph -s
</pre>

创建新的pool:
<pre>
ceph osd pool create .rgw.root 8 8
ceph osd pool create default.rgw.control 8 8
ceph osd pool create default.rgw.data.root 16 16
ceph osd pool create default.rgw.gc 8 8
ceph osd pool create default.rgw.log 8 8
ceph osd pool create default.rgw.intent-log 8 8
ceph osd pool create default.rgw.usage 8 8
ceph osd pool create default.rgw.users.keys 8 8
ceph osd pool create default.rgw.users.email 8 8
ceph osd pool create default.rgw.users.swift 8 8
ceph osd pool create default.rgw.users.uid 8 8
ceph osd pool create default.rgw.buckets.index 256 256
ceph osd pool create default.rgw.buckets.data 256 256
ceph osd pool create default.rgw.meta 8 8
ceph osd pool create default.rgw.buckets.non-ec 8 8
</pre>

然后更改pool所用的crush 规则：
<pre>
ceph osd pool set .rgw.root crush_ruleset 5
ceph osd pool set default.rgw.control crush_ruleset 5
ceph osd pool set default.rgw.data.root crush_ruleset 5
ceph osd pool set default.rgw.gc crush_ruleset 5
ceph osd pool set default.rgw.log  crush_ruleset 5
ceph osd pool set default.rgw.intent-log crush_ruleset 5
ceph osd pool set default.rgw.usage crush_ruleset 5
ceph osd pool set default.rgw.users.keys crush_ruleset 5
ceph osd pool set default.rgw.users.email crush_ruleset 5
ceph osd pool set default.rgw.users.swift crush_ruleset 5
ceph osd pool set default.rgw.users.uid crush_ruleset 5
ceph osd pool set default.rgw.buckets.index crush_ruleset 5
ceph osd pool set default.rgw.buckets.data crush_ruleset 5
ceph osd pool set default.rgw.meta crush_ruleset 5
ceph osd pool set default.rgw.buckets.non-ec crush_ruleset 5
</pre>

2) 为当前节点RGW创建用户
<pre>
# ceph-authtool --create-keyring /etc/ceph/ceph.client.radosgw.keyring --gen-key -n client.radosgw.ceph001-node1 --cap mon 'allow rwx' --cap osd 'allow rwx'

# ceph -k /etc/ceph/ceph.client.admin.keyring auth add client.radosgw.ceph001-node1 -i /etc/ceph/ceph.client.radosgw.keyring

# ceph auth list
</pre>

3) 修改当前节点RGW配置

当前节点配置文件为```/etc/ceph/ceph.conf```,添加rgw配置：
<pre>
[client.radosgw.ceph001-node1]
host = ceph001-node1
log_file = /var/log/ceph/radosgw-ceph001-node1.log
rgw_s3_auth_use_keystone = False
rgw_frontends = civetweb port=7480
rgw_socket_path = /var/run/ceph/ceph.radosgw.ceph001-node1.sock
user = root
keyring = /etc/ceph/ceph.client.radosgw.keyring
rgw_override_bucket_index_max_shards = 0
rgw_swift_token_expiration = 86400
rgw_enable_usage_log = True
rgw_cache_lru_size = 10000
rgw_print_continue = False
rgw_cache_enabled = True
admin_socket = /var/run/ceph/radosgw-node7-1.asok
rgw_thread_pool_size=512
rgw_num_rados_handles=512
</pre>

4) 启动RGW
{% highlight string %}
radosgw -c /etc/ceph/ceph.conf -n client.radosgw.ceph001-node1
{% endhighlight %}

查看是否启动成功：
<pre>
[root@ceph001-node1 build]# netstat -lpn | grep radosgw
tcp        0      0 0.0.0.0:7480            0.0.0.0:*               LISTEN      21229/radosgw       
unix  2      [ ACC ]     STREAM     LISTENING     403819   21229/radosgw        /var/run/ceph/radosgw-ceph001-node1.asok
</pre>



5) 创建一个用于管理rgw的超级用户

这里我们为RGW创建一个管理员用户，用于后台管理。
{% highlight string %}
radosgw-admin user create --uid=admin --display-name="admin"

radosgw-admin caps add --uid=admin --caps="buckets=*"
radosgw-admin caps add --uid=admin --caps="data=*"
radosgw-admin caps add --uid=admin --caps="metadata=*"
radosgw-admin caps add --uid=admin --caps="usage=*"
radosgw-admin caps add --uid=admin --caps="users=*"
radosgw-admin caps add --uid=admin --caps="zone=*"
{% endhighlight %}

上面我们看到，该管理员用户具有十分大的权限。

至此，ceph001-node1节点的radosgw已经部署完成。


### 4.3 在ceph001-node2上部署RGW

此处只需要创建相应的rgw用户并加入集群，然后配置ceph.conf文件，再启动rgw即可。下面是详细步骤：

1） 创建RGW秘钥并加入集群
{% highlight string %}
ceph-authtool --create-keyring /etc/ceph/ceph.client.radosgw.keyring --gen-key -n client.radosgw.ceph001-node2 --cap mon 'allow rwx' --cap osd 'allow rwx'

ceph -k /etc/ceph/ceph.client.admin.keyring auth add client.radosgw.ceph001-node2 -i /etc/ceph/ceph.client.radosgw.keyring

ceph auth list
{% endhighlight %}

2) 修改ceph配置文件

修改/etc/ceph/ceph.conf配置文件，在其中添加：
<pre>
[client.radosgw.ceph001-node2]
host = ceph001-node2
log_file = /var/log/ceph/radosgw-ceph001-node2.log
rgw_s3_auth_use_keystone = False
rgw_frontends = civetweb port=7480
rgw_socket_path = /var/run/ceph/ceph.radosgw.ceph001-node2.sock
user = root
keyring = /etc/ceph/ceph.client.radosgw.keyring
rgw_override_bucket_index_max_shards = 0
rgw_swift_token_expiration = 86400
rgw_enable_usage_log = True
rgw_cache_lru_size = 10000
rgw_print_continue = False
rgw_cache_enabled = True
admin_socket = /var/run/ceph/radosgw-ceph001-node2.asok
rgw_thread_pool_size=512
rgw_num_rados_handles=512
</pre>

3) 启动RGW
{% highlight string %}
radosgw -c /etc/ceph/ceph.conf -n client.radosgw.ceph001-node2
{% endhighlight %}

查看是否启动成功：
<pre>
[root@ceph001-node2 build]# netstat -lpn | grep radosgw
tcp        0      0 0.0.0.0:7480            0.0.0.0:*               LISTEN      9756/radosgw        
unix  2      [ ACC ]     STREAM     LISTENING     312548   9756/radosgw         /var/run/ceph/radosgw-ceph001-node2.asok
</pre>

因为节点ceph001-node1已经创建好了admin账号以及初始化权限，所以之后的节点都不需要再进行创建了。

至此，节点ceph001-node2部署rgw完毕。


### 4.4 在ceph001-node3上部署RGW

此处只需要创建相应的rgw用户并加入集群，然后配置ceph.conf文件，再启动rgw即可。下面是详细步骤：

1） 创建RGW秘钥并加入集群
{% highlight string %}
ceph-authtool --create-keyring /etc/ceph/ceph.client.radosgw.keyring --gen-key -n client.radosgw.ceph001-node3 --cap mon 'allow rwx' --cap osd 'allow rwx'

ceph -k /etc/ceph/ceph.client.admin.keyring auth add client.radosgw.ceph001-node3 -i /etc/ceph/ceph.client.radosgw.keyring

ceph auth list
{% endhighlight %}

2) 修改ceph配置文件

修改/etc/ceph/ceph.conf配置文件，在其中添加：
<pre>
[client.radosgw.ceph001-node3]
host = ceph001-node3
log_file = /var/log/ceph/radosgw-ceph001-node3.log
rgw_s3_auth_use_keystone = False
rgw_frontends = civetweb port=7480
rgw_socket_path = /var/run/ceph/ceph.radosgw.ceph001-node3.sock
user = root
keyring = /etc/ceph/ceph.client.radosgw.keyring
rgw_override_bucket_index_max_shards = 0
rgw_swift_token_expiration = 86400
rgw_enable_usage_log = True
rgw_cache_lru_size = 10000
rgw_print_continue = False
rgw_cache_enabled = True
admin_socket = /var/run/ceph/radosgw-ceph001-node3.asok
rgw_thread_pool_size=512
rgw_num_rados_handles=512
</pre>

3) 启动RGW
{% highlight string %}
radosgw -c /etc/ceph/ceph.conf -n client.radosgw.ceph001-node3
{% endhighlight %}

查看是否启动成功：
<pre>
[root@ceph001-node3 build]# netstat -lpn | grep radosgw
tcp        0      0 0.0.0.0:7480            0.0.0.0:*               LISTEN      15626/radosgw       
unix  2      [ ACC ]     STREAM     LISTENING     326358   15626/radosgw        /var/run/ceph/radosgw-ceph001-node3.asok
</pre>

因为节点ceph001-node1已经创建好了admin账号以及初始化权限，所以之后的节点都不需要再进行创建了。

至此，节点ceph001-node3部署rgw完毕。
<br />
<br />


最后，到此为止整个集群已经部署完毕.


## 5. ceph的卸载

在有些时候，由于ceph安装失败或其他原因，我们希望把ceph整个环境卸载，以回到一个干净的操作系统环境。


### 5.1 kill掉ceph相关的所有进程
<pre>
# ps -aux | grep ceph | grep -v grep | awk '{print $2}' | xargs kill -9
# ps -aux | grep ceph | grep -v grep | awk '{print $2}' | xargs kill -9   //执行多次
# ps -ef | grep ceph


# ps -aux | grep nginx | grep -v grep | awk '{print $2}' | xargs kill -9
# ps -aux | grep nginx | grep -v grep | awk '{print $2}' | xargs kill -9
# ps -ef | grep nginx

# ps -aux | grep zabbix | grep -v grep | awk '{print $2}' | xargs kill -9
# ps -aux | grep zabbix | grep -v grep | awk '{print $2}' | xargs kill -9
# ps -ef | grep zabbix
</pre>

### 5.2 删除与ceph相关的定时任务

首先通过如下命令查看：
<pre>
# crontab -l
#* * * * * /usr/sbin/ntpdate ntp.ustack.in 1 >/dev/null 2>&1
* * * * * /apps/zabbix/ceph/ceph_check.py > /apps/zabbix/ceph/ceph_check_tmp.a;mv /apps/zabbix//ceph/ceph_check_tmp.a /apps/zabbix/ceph/ceph_check_tmp &
* * * * * /apps/zabbix/ceph/ceph_usage.py > /apps/zabbix/ceph/ceph_usage_tmp.a 2>/dev/null;mv /apps/zabbix/ceph/ceph_usage_tmp.a /apps/zabbix/ceph/ceph_usage_tmp &
</pre>

然后删除相关任务：
<pre>
# crontab -e
# service crond restart 
</pre>


### 5.3 卸载已挂载硬盘
1) 首先通过如下命令查看当前挂载了哪些硬盘:
<pre>
# lsblk -l
NAME                 MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
sda                    8:0    0   1.8T  0 disk 
└─sda1                 8:1    0   1.8T  0 part /var/lib/ceph/osd/ceph-0
sdb                    8:16   0   1.8T  0 disk 
└─sdb1                 8:17   0   1.8T  0 part /var/lib/ceph/osd/ceph-1
sdc                    8:32   0   1.8T  0 disk 
└─sdc1                 8:33   0   1.8T  0 part /var/lib/ceph/osd/ceph-2
sdd                    8:48   0   1.8T  0 disk 
└─sdd1                 8:49   0   1.8T  0 part /var/lib/ceph/osd/ceph-3
sde                    8:64   0   1.8T  0 disk 
└─sde1                 8:65   0   1.8T  0 part /var/lib/ceph/osd/ceph-12
sdf                    8:80   0   1.8T  0 disk 
└─sdf1                 8:81   0   1.8T  0 part /var/lib/ceph/osd/ceph-13
sdg                    8:96   0 447.1G  0 disk 
├─sdg1                 8:97   0    14G  0 part 
├─sdg2                 8:98   0    14G  0 part 
├─sdg3                 8:99   0    14G  0 part 
├─sdg4                 8:100  0    14G  0 part 
├─sdg5                 8:101  0    14G  0 part 
├─sdg6                 8:102  0    14G  0 part 
├─sdg7                 8:103  0    14G  0 part 
├─sdg8                 8:104  0    14G  0 part 
└─sdg9                 8:105  0    14G  0 part 
sdh                    8:112  0   1.8T  0 disk 
└─sdh1                 8:113  0   1.8T  0 part /var/lib/ceph/osd/ceph-14
sdi                    8:128  0   1.8T  0 disk 
└─sdi1                 8:129  0   1.8T  0 part /var/lib/ceph/osd/ceph-15
sdj                    8:144  0   1.8T  0 disk 
└─sdj1                 8:145  0   1.8T  0 part /var/lib/ceph/osd/ceph-16
sdk                    8:160  0   1.8T  0 disk 
├─sdk1                 8:161  0  1000M  0 part /boot
├─sdk2                 8:162  0   3.9G  0 part [SWAP]
└─sdk3                 8:163  0   1.8T  0 part 
  ├─company_pv-rootvol 253:0    0  19.5G  0 lvm  /
  └─company_pv-varvol  253:1    0   1.8T  0 lvm  /var
</pre>
上面我们看到，有sda~sdk共11个硬盘， 这里除了sdk硬盘几个分区作为系统分区使用，其他都作为一般分区使用。

2) 这里卸载挂载的硬盘：
<pre>
# umount /var/lib/ceph/osd/ceph-0
# umount /var/lib/ceph/osd/ceph-1
# umount /var/lib/ceph/osd/ceph-2
# umount /var/lib/ceph/osd/ceph-3
# umount /var/lib/ceph/osd/ceph-12
# umount /var/lib/ceph/osd/ceph-13
# umount /var/lib/ceph/osd/ceph-14
# umount /var/lib/ceph/osd/ceph-15
# umount /var/lib/ceph/osd/ceph-16

或直接如下命令：
# umount /var/lib/ceph/osd/*
</pre>

3) 卸载完成后，我们看当前硬盘挂载情况：
<pre>
# lsblk 
NAME                 MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
sda                    8:0    0   1.8T  0 disk 
└─sda1                 8:1    0   1.8T  0 part 
sdb                    8:16   0   1.8T  0 disk 
└─sdb1                 8:17   0   1.8T  0 part 
sdc                    8:32   0   1.8T  0 disk 
└─sdc1                 8:33   0   1.8T  0 part 
sdd                    8:48   0   1.8T  0 disk 
└─sdd1                 8:49   0   1.8T  0 part 
sde                    8:64   0   1.8T  0 disk 
└─sde1                 8:65   0   1.8T  0 part 
sdf                    8:80   0   1.8T  0 disk 
└─sdf1                 8:81   0   1.8T  0 part 
sdg                    8:96   0 447.1G  0 disk 
├─sdg1                 8:97   0    14G  0 part 
├─sdg2                 8:98   0    14G  0 part 
├─sdg3                 8:99   0    14G  0 part 
├─sdg4                 8:100  0    14G  0 part 
├─sdg5                 8:101  0    14G  0 part 
├─sdg6                 8:102  0    14G  0 part 
├─sdg7                 8:103  0    14G  0 part 
├─sdg8                 8:104  0    14G  0 part 
└─sdg9                 8:105  0    14G  0 part 
sdh                    8:112  0   1.8T  0 disk 
└─sdh1                 8:113  0   1.8T  0 part 
sdi                    8:128  0   1.8T  0 disk 
└─sdi1                 8:129  0   1.8T  0 part 
sdj                    8:144  0   1.8T  0 disk 
└─sdj1                 8:145  0   1.8T  0 part 
sdk                    8:160  0   1.8T  0 disk 
├─sdk1                 8:161  0  1000M  0 part /boot
├─sdk2                 8:162  0   3.9G  0 part [SWAP]
└─sdk3                 8:163  0   1.8T  0 part 
  ├─company_pv-rootvol 253:0    0  19.5G  0 lvm  /
  └─company_pv-varvol  253:1    0   1.8T  0 lvm  /var
</pre>

4) 删除分区

这里我们删除```除sdk1、sdk2、sdk3```之外的所有分区：
<pre>
# parted /dev/sda rm 1
Information: You may need to update /etc/fstab.

# parted /dev/sdb rm 1
# parted /dev/sdc rm 1
# parted /dev/sdd rm 1
# parted /dev/sde rm 1
# parted /dev/sdf rm 1

# parted /dev/sdg rm 1
# parted /dev/sdg rm 2
# parted /dev/sdg rm 3
# parted /dev/sdg rm 4
# parted /dev/sdg rm 5
# parted /dev/sdg rm 6
# parted /dev/sdg rm 7
# parted /dev/sdg rm 8
# parted /dev/sdg rm 9


# parted /dev/sdh rm 1
# parted /dev/sdi rm 1
# parted /dev/sdj rm 1
</pre>

删除后查看当前分区情况：
<pre>
# lsblk                                                 
NAME                 MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
sda                    8:0    0   1.8T  0 disk 
sdb                    8:16   0   1.8T  0 disk 
sdc                    8:32   0   1.8T  0 disk 
sdd                    8:48   0   1.8T  0 disk 
sde                    8:64   0   1.8T  0 disk 
sdf                    8:80   0   1.8T  0 disk 
sdg                    8:96   0 447.1G  0 disk 
sdh                    8:112  0   1.8T  0 disk 
sdi                    8:128  0   1.8T  0 disk 
sdj                    8:144  0   1.8T  0 disk 
sdk                    8:160  0   1.8T  0 disk 
├─sdk1                 8:161  0  1000M  0 part /boot
├─sdk2                 8:162  0   3.9G  0 part [SWAP]
└─sdk3                 8:163  0   1.8T  0 part 
  ├─company_pv-rootvol 253:0    0  19.5G  0 lvm  /
  └─company_pv-varvol  253:1    0   1.8T  0 lvm  /var
</pre>


5) 删除自动挂载相关配置

这里根据如下的执行结果：
<pre>
# systemctl status network.service
</pre>
可能会把硬盘自动挂载写入到```/etc/fstab```文件或者```/etc/rc.d/rc.local```中， 因此这里需要检查这两个文件，把相应的信息删除。



### 5.4 删除ceph相关所有数据
<pre>
# ls /var/lib/ceph/
bootstrap-mds  bootstrap-osd  bootstrap-rgw  mon  osd
# rm -rf /var/lib/ceph/*
# rm -rf /var/lib/ceph/*

#  ls /etc/ceph/
ceph.client.openstack.keyring  ceph.conf      ceph.conf.rgw  rbdmap
ceph.client.admin.keyring    ceph.client.radosgw.keyring      ceph.conf.cluster  keyfile
# rm -rf /etc/ceph/*
# rm -rf /etc/ceph/*

# ls /var/run/ceph/
# rm -rf /var/run/ceph/
</pre>

### 5.5 卸载ceph

首先用如下命令查看当前安装的与ceph相关的软件包：
<pre>
# rpm -qa|grep ceph
libcephfs1-10.2.3-0.el7.x86_64
ceph-common-10.2.3-0.el7.x86_64
ceph-selinux-10.2.3-0.el7.x86_64
ceph-osd-10.2.3-0.el7.x86_64
ceph-mds-10.2.3-0.el7.x86_64
ceph-radosgw-10.2.3-0.el7.x86_64
python-cephfs-10.2.3-0.el7.x86_64
ceph-base-10.2.3-0.el7.x86_64
ceph-mon-10.2.3-0.el7.x86_64
ceph-10.2.3-0.el7.x86_64
</pre>

然后调用如下命令进行卸载：
<pre>
# yum remove ceph
</pre>

卸载完成后再调用：
<pre>
# rpm -qa | grep ceph
</pre>
如果仍残留有```ceph```相关包，则调用如下命令卸载：
{% highlight string %}
# yum remove <ceph-package-name>
{% endhighlight %}

### 5.6 卸载其他相关软件
<pre>
# rpm -qa | grep zabbix
# rpm -qa | grep s3cmd
# rpm -qa | grep haproxy

# yum remove zabbix
# yum remove s3cmd
# yum remove haproxy
</pre>

### 5.7 查看当前开机启动相关
<pre>
# systemctl status
</pre>




<br />
<br />

**[参看]**

1. [MULTI-SITE](http://docs.ceph.com/docs/master/radosgw/multisite/)

2. [ceph的卸载](https://www.cnblogs.com/nulige/articles/8475907.html)

3. [不卸载ceph重新获取一个干净的集群环境](https://www.cnblogs.com/sisimi/p/7693237.html)

4. [Linux下进行硬盘挂载、分区、删除分区，格式化，卸载方法](https://www.cnblogs.com/zishengY/p/7137671.html)
<br />
<br />
<br />

