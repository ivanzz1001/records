---
layout: post
title: 基于Harbor搭建docker registry
tags:
- docker
categories: docker
description: 基于Harbor搭建docker registry
---


本文记录一下在Centos7.3操作系统上，基于Harbor来搭建docker registry。当前环境为：
<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 

# uname -a
Linux bogon 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

# docker --version
Docker version 17.12.1-ce, build 7390fc6

# docker-compose --version
docker-compose version 1.20.1, build 5d8c71b
</pre>


<!-- more -->

## 1. Harbor简介
Harbor工程是一个企业级的镜像服务器，用于存储和分发Docker镜像。Harbor扩展了开源软件```Docker Distribution```，添加了如```security```、```identity```和```management```等功能。作为一个企业级的私有镜像仓库，Harbor提供了更好的性能和安全性。Harbor支持建立多个```registries```,并提供这些仓库间镜像的复制能力。Harbor也提供了更加先进的安全特性，比如用户管理、访问控制、活动审计。

Harbor特性：

* **基于角色的访问控制**: ```users```和```repositories ```都是以projects的方式组织的。在一个project下面，每一个用户对镜像有不同的全向。

* **基于策略的镜像复制**: 在多个registry之间镜像可以同步，并且在出现错误的时候可以进行自动重试。在负载均衡、高可用性、多数据中心和异构云环境下都表现出色。

* **脆弱性扫描(Vulnerability Scanning)**: Harbor会周期性的扫描镜像，然后警告用户相应的脆弱性

* **LDAP/AD支持**: Harbor可以和已存在的企业版LDAP/AD系统集成，以提供用户认证和管理

* **镜像删除 & 垃圾回收**: Images可以被删除，然后回收它们所占用的空间

* **可信任(Notary)**: 可以确保镜像的真实性

* **用户界面(Graphical user portal)**: 用户可以人容易的浏览、搜索仓库和管理工程

* **审计(Auditing)**: 所有对仓库的操作都会被跟踪记录

* **RESTful API**: 对于大部分的管理操作都提供了RESTful API， 很容易和外部系统进行集成

* **易部署**: 提供了离线和在线安装

## 2. Harbor的安装

这里介绍的是通过```Harbor安装文件```的方式来安装Harbor。在Linux操作系统上至少需要如下环境：
<pre>
docker 1.10.0+ and docker-compose 1.6.0+
</pre>

### 2.1 下载Harbor离线安装包
到[Harbor Release](https://github.com/vmware/harbor/releases)页面下载对应的离线安装包，目前我们下载最新版本```v1.4.0```:
<pre>
# mkdir /opt/harbor-inst
# cd /opt/harbor-inst

# wget https://storage.googleapis.com/harbor-releases/release-1.4.0/harbor-offline-installer-v1.4.0.tgz
</pre>

### 2.2 目标主机相关配置推荐

Harbor部署完后会运行多个Docker containers，因此可以部署在任何支持docker的Linux发布版本上。部署的目标主机需要安装```Python```, ```Docker```和```Docker Compose```。

**硬件环境：**

|        Resource        |         Capacity           |     Description      |
|:----------------------:|:--------------------------:|:--------------------:|
|        CPU             |         minimal 2 CPU      |  4 CPU is prefered   |
|        Mem             |         minimal 4GB        |  8GB is preffered    |
|        Disk            |         minimal 40GB       |  160GB is preffered  |


**软件环境**

|      Software      |         Version           |     Description                                                             |
|:------------------:|:-------------------------:|-----------------------------------------------------------------------------|
|    Python          | version 2.7 or higher     | 注意： 在有一些Linux发布版本(Gentoo、Arch)默认没有安装Python，此时你必须手动安装   |
|    Docker Engine   | version 1.10 or higher    | 具体安装手册，请参看相关文档:https://docs.docker.com/engine/installation/       |
|    Docker Compose  | version 1.6.0 or higher   | 具体安装手册，请参看相关文档:https://docs.docker.com/compose/install/           |
|    Openssl         | latest is preffered       | 用于为Harbor产生证书和秘钥                                                     |

**网络端口**

|      Port      |     Protocol       |     Description                                                             |
|:--------------:|:------------------:|-----------------------------------------------------------------------------|
|    443         |    Https           | 注意： 在有一些Linux发布版本(Gentoo、Arch)默认没有安装Python，此时你必须手动安装   |
|    4443        |    Https           | 具体安装手册，请参看相关文档:https://docs.docker.com/engine/installation/       |
|    80          |    Http            | 具体安装手册，请参看相关文档:https://docs.docker.com/compose/install/           |
|    Openssl     |latest is preffered | 用于为Harbor产生证书和秘钥                                                     |


我们当前的硬件环境：
<pre>
//物理CPU个数
# cat /proc/cpuinfo| grep "physical id"| sort| uniq| wc -l
1

//每个CPU核数
# cat /proc/cpuinfo| grep "cpu cores"| uniq
cpu cores       : 4

//逻辑CPU个数
# cat /proc/cpuinfo  | grep processor
processor       : 0
processor       : 1
processor       : 2
processor       : 3

# cat /proc/meminfo | grep MemTotal
MemTotal:       10058704 kB

#  fdisk -l | grep Disk
Disk /dev/sda: 85.9 GB, 85899345920 bytes, 167772160 sectors
Disk label type: dos
Disk identifier: 0x000c3eb0
</pre>

我们当前软件环境：
<pre>
# python --version
Python 2.7.5

# docker --version
Docker version 17.12.1-ce, build 7390fc6

# docker-compose --version
docker-compose version 1.20.1, build 5d8c71b

# openssl version -v
OpenSSL 1.0.2k-fips  26 Jan 2017
</pre>

### 2.3 安装步骤
安装Harbor一般遵循如下步骤：

* 下载Harbor installer

* 配置harbor.cfg

* 运行```install.sh```脚本进行安装并启动harbor

#### 2.3.1 解压harbor安装包

我们在上面下载了harbor安装包，这里解压：
<pre>
# pwd
/opt/harbor-inst

# ls
harbor-offline-installer-v1.4.0.tgz

# tar -zxvf harbor-offline-installer-v1.4.0.tgz 
# cd harbor
</pre>

#### 2.3.2 配置Harbor
Harbor配置参数处于```harbor.cfg```文件中。在harbor.cfg配置文件中，有两大类参数： ```必填参数```和```可选参数```

* ```required parameters```: 这些参数在配置文件中必须填写。在更新harbor.cfg配置文件后，调用install.sh重新安装Harbor，这些参数就会起作用

* ```optional parameters```: 这些参数在更新时是可选的。例如， 用户可以先让这些参数取默认值，然后在Harbor启动后在Web UI上来进行修改。假如这些参数在harbor.cfg中也进行了配置，那么只在第一次启动harbor有效。后续再对harbor.cfg进行更新将会被忽略。

<pre>
Note: 假如你选择通过Web UI的方式来更改这些参数，确保在Harbor启动之后马上进行更改。通常，你必须在注册或创建新的用户之前设置auth_mode。
当Harbor系统中有用户之后（出admin管理用户外)，auth_mode是不能被修改的
</pre>

如下所描述的参数，你至少需要更改```hostname```属性：

**Required parameters**:

* ```hostname```: 


<br />
<br />

**[参考]**

1. [harbor官网](https://github.com/vmware/harbor)

2. [Centos7上Docker仓库Harbor的搭建](https://blog.csdn.net/felix_yujing/article/details/54694294)
<br />
<br />
<br />

