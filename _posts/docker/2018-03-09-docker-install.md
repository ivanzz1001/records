---
layout: post
title: docker环境的搭建
tags:
- docker
categories: docker
description: docker环境的搭建
---


本文介绍在CentOs7.3操作系统上，docker离线环境下的安装。


<!-- more -->

## 1. 离线环境下安装docker-ce

当前操作系统版本：
<pre>
[root@docker-registry docker]# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 
[root@docker-registry docker]# uname -a
Linux docker-registry 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>

### 1.1 卸载老的docker环境
这里首先卸载老版本的docker环境：
<pre>
# yum remove docker*
# yum list installed | grep "container"   //卸载相关组件container-selinux（必须卸载，不然会报冲突的错误）
container-selinux.noarch               2:2.36-1.gitff95335.el7         installed
# yum remove container-selinux.noarch
</pre>


### 1.2 下载离线安装包
先找一台与当前CentOS版本相同（相似）的主机，通过该主机来下载docker-ce安装包。首先配置yum源:
<pre>
# wget https://download.docker.com/linux/centos/docker-ce.repo
# cp docker-ce.repo /etc/yum.repos.d/
</pre>

docker官方下载地址：[https://download.docker.com/](https://download.docker.com/)

下载docker-ce安装包及其依赖包：
<pre>
# yum clean packages            
# yum clean all
# yum repolist
# yum makecache

# yum list docker-ce --showduplicates
Loaded plugins: fastestmirror, langpacks
Loading mirror speeds from cached hostfile
 * base: centos.ustc.edu.cn
 * extras: mirror.lzu.edu.cn
 * updates: mirror.lzu.edu.cn
Installed Packages
docker-ce.x86_64                                                          17.12.1.ce-1.el7.centos                                                          installed       
Available Packages
docker-ce.x86_64                                                          17.03.0.ce-1.el7.centos                                                          docker-ce-stable
docker-ce.x86_64                                                          17.03.1.ce-1.el7.centos                                                          docker-ce-stable
docker-ce.x86_64                                                          17.03.2.ce-1.el7.centos                                                          docker-ce-stable
docker-ce.x86_64                                                          17.06.0.ce-1.el7.centos                                                          docker-ce-stable
docker-ce.x86_64                                                          17.06.1.ce-1.el7.centos                                                          docker-ce-stable
docker-ce.x86_64                                                          17.06.2.ce-1.el7.centos                                                          docker-ce-stable
docker-ce.x86_64                                                          17.09.0.ce-1.el7.centos                                                          docker-ce-stable
docker-ce.x86_64                                                          17.09.1.ce-1.el7.centos                                                          docker-ce-stable
docker-ce.x86_64                                                          17.12.0.ce-1.el7.centos                                                          docker-ce-stable
docker-ce.x86_64                                                          17.12.1.ce-1.el7.centos                                                          docker-ce-stable


# sudo yum install --downloadonly --downloaddir=./ docker-ce-17.12.0.ce-1.el7.centos
</pre>

另外还会依赖于```libltdl.so.7()(64bit)```与```libseccomp.so.2()(64bit)```，可以按如下方式下载：
{% highlight string %}
//通过如下方法查看rpm包的依赖关系
# rpm -qpR ./docker-ce-17.12.0.ce-1.el7.centos.x86_64.rpm 
warning: ./docker-ce-17.12.0.ce-1.el7.centos.x86_64.rpm: Header V4 RSA/SHA512 Signature, key ID 621e9f35: NOKEY
/bin/sh
/bin/sh
/bin/sh
/bin/sh
/bin/sh
/bin/sh
container-selinux >= 2.9
device-mapper-libs >= 1.02.90-1
iptables
libc.so.6()(64bit)
libc.so.6(GLIBC_2.14)(64bit)
libc.so.6(GLIBC_2.17)(64bit)
libc.so.6(GLIBC_2.2.5)(64bit)
libc.so.6(GLIBC_2.3)(64bit)
libc.so.6(GLIBC_2.4)(64bit)
libcgroup
libdevmapper.so.1.02()(64bit)
libdevmapper.so.1.02(Base)(64bit)
libdevmapper.so.1.02(DM_1_02_97)(64bit)
libdl.so.2()(64bit)
libdl.so.2(GLIBC_2.2.5)(64bit)
libltdl.so.7()(64bit)
libpthread.so.0()(64bit)
libpthread.so.0(GLIBC_2.2.5)(64bit)
libpthread.so.0(GLIBC_2.3.2)(64bit)
libseccomp.so.2()(64bit)
libsystemd.so.0()(64bit)
libsystemd.so.0(LIBSYSTEMD_209)(64bit)
rpmlib(CompressedFileNames) <= 3.0.4-1
rpmlib(FileDigests) <= 4.6.0-1
rpmlib(PayloadFilesHavePrefix) <= 4.0-1
rtld(GNU_HASH)
systemd-units
tar
xz
rpmlib(PayloadIsXz) <= 5.2-1

# yum deplist docker-ce              //此种方法也能查看依赖包
# yumdownloader <package-name>       //用此下载指定名称的包

//http://rpmfind.net/linux/rpm2html/search.php?query=libltdl.so.7()(64bit)
# wget http://rpmfind.net/linux/centos/7.4.1708/os/x86_64/Packages/libtool-ltdl-2.4.2-22.el7_3.x86_64.rpm

//http://www.rpmfind.net/linux/rpm2html/search.php?query=libseccomp.so.2()(64bit)
# wget http://www.rpmfind.net/linux/centos/7.4.1708/os/x86_64/Packages/libseccomp-2.3.1-3.el7.x86_64.rpm
{% endhighlight %}

下载完成后，拷贝到对应的离线主机上进行安装：
<pre>
# yum localinstall *.rpm
</pre>

### 1.3 启动docker
通过如下命令启动docker:
<pre>
# systemctl start docker
# ps -ef | grep docker
root     10904     1  0 15:13 ?        00:00:03 /usr/bin/dockerd
root     10911 10904  0 15:13 ?        00:00:03 docker-containerd --config /var/run/docker/containerd/containerd.toml
</pre>

<br />
<br />

1. [Docker CE安装及配置国内镜像加速教程](http://blog.csdn.net/jackyzhousales/article/details/77995135)

2. [配置docker官方源并用yum安装docker](https://www.cnblogs.com/JiangLe/p/6921320.html)
<br />
<br />
<br />

