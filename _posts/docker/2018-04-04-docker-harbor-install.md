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

|      Port      |     Protocol       |     Description                                                              |
|:--------------:|:------------------:|------------------------------------------------------------------------------|
|    443         |    HTTPS           | 在https协议下，Harbor UI与API将会在本端口上接收请求                              |
|    4443        |    HTTPS           | Harbor的Docker Content Trust service将会连接到本端口，只在Notary启用时使用       |
|    80          |    HTTP            | 在http协议下，Harbor UI与API将会在本端口上接收请求                               |


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

<br />

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

<br />

#### 2.3.2 配置Harbor
Harbor配置参数处于```harbor.cfg```文件中。在harbor.cfg配置文件中，有两大类参数： ```必填参数```和```可选参数```

* ```required parameters```: 这些参数在配置文件中必须填写。在更新harbor.cfg配置文件后，调用install.sh重新安装Harbor，这些参数就会起作用

* ```optional parameters```: 这些参数在更新时是可选的。例如， 用户可以先让这些参数取默认值，然后在Harbor启动后在Web UI上来进行修改。假如这些参数在harbor.cfg中也进行了配置，那么只在第一次启动harbor有效。后续再对harbor.cfg进行更新将会被忽略。

{% highlight string %}
Note: 假如你选择通过Web UI的方式来更改这些参数，确保在Harbor启动之后马上进行更改。通常，你必须在注册或创建新的用户之前设置auth_mode。
当Harbor系统中有用户之后（出admin管理用户外)，auth_mode是不能被修改的
{% endhighlight %}

如下所描述的参数，你至少需要更改```hostname```属性：

**Required parameters**:

* ```hostname```: 目标主机的hostname名称，被用于访问WebUI和registry服务。其可以被设置为IP地址，或者你目标机器的全限定域名。例如: ```192.168.1.10```或者```reg.yourdomain.com```。注意不要将hostname设置为```localhost```或者```127.0.0.1```, registry服务需要能够被外网访问的到。

* ```ui_url_protocol```: 可以设置为http或者https，默认值为http。该协议被用于访问Web UI和```token/notification```服务。假如```Notary```被使能的话，则必须设置为https。默认情况下采用http协议，要想设置为https,请参看[Configuring Harbor with HTTPS Access](Configuring Harbor with HTTPS Access)

* ```db_password```: 当auth采用db_auth方式时，用于设置MySQL数据库的密码。请在任何实际生产环境中，修改此密码

* ```max_job_workers```: 用于设置job service中```replication```worker的最大数（默认为3）。对于每一个image replication任务，一个worker会同步repository中所有tags到远程目标地址。增大本字段的值，允许在一个系统中有更多的并发复制进程。然而，每个replication worker都会消耗一定数量的network/CPU/IO资源，请基于你当前的硬件环境选择一个合适的值。

* ```customize_crt```: 可以被设置为on或者off，默认值为on。当本属性设置为on时，prepare脚本会创建一个```private key```及```root certificate```,以用于registry token的验证。假如本属性被设置为off的话，你可以自己手动来产生```private key```及```root certificate```。请参看:[Customize Key and Certificate of Harbor Token Service](https://github.com/vmware/harbor/blob/master/docs/customize_token_service.md)

* ```ssl_cert```: SSL certificate路径，当协议被设置为https时使用

* ```ssl_cert_key```: SSL key路径，当协议被设置为https时使用

* ```secretkey_path```: 用于加密和机密远程registry密码的key路径

* ```log_rotate_count```: 用于设置日志在回滚多少次之后被删除。假如被设置为0，则日志不会被回滚，而是会被直接删除

* ```log_rotate_size```: 用于设置日志在多大时会进行回滚，单位可以是K/M/G，分别表示KB/MB/GB。

**Optional parameters**:

* ```Email settings```: 这些信息主要是为了重置Harbor密码时使用，通常情况下我们并不需要。 

* ```harbor_admin_password```: 用于设置管理员初始密码。该密码只在Harbor第一次启动时有效。启动之后该密码将会被忽略，Administrator的密码应该在UI中进行设置。注意，默认的用户名/密码为```admin/Harbor12345```。

* ```auth_mode```: 用户认证的类型，默认情况下为```db_auth```，这种情况下用户名密码被存放在数据库中。如果要使用```LDAP```认证的话，请将此字段设置为```ldap_auth```。
{% highlight string %}
IMPORTANT: 当要从一个已存在的Harbor实例升级的时候，你必须确保在harbor.cfg中配置的auth_mode是相同的，否则在更新后可能会造成用户不能正常登录
{% endhighlight %}

* ```ldap_url```: LDAP端点的URL(例如：ldaps://ldap.mydomain.com)。只在auth_mode被设置为```ldap_auth```时使用

* ```ldap_searchdn```、```ldap_search_pwd```、```ldap_basedn```、```ldap_filter```、```ldap_uid```、```ldap_scope```

* ```self_registration```: 可选值为on/off，默认为on。本选项用于使能或禁止注册成为本系统的账户。当被禁止时，新用户只能由admin用户来创建，在Harbor中只有admin用户可以创建新用户。注意： 当auth_mode被设置为```ldap_auth```时，self-registration功能总是会被禁止，并且此选项会被忽略。

* ```token_expiration```: token创建多长时间之后会过期，默认是30min

* ```project_creation_restriction```: 本flag用于控制哪些用户有权限来创建projects。默认情况下，任何用户都可以创建project，假如设置为```adminonly```，则只有admin用户可以创建project。

<br />

#### 2.3.3 配置存储后端(可选）
默认情况下，Harbor存储镜像到本地文件系统。在实际的生产环境下，你可以采用其他的存储后端来代替本地文件系统，例如可以采用S3、OpenStack Swift、Ceph等。而这你需要修改的文件是```common/templates/registry/config.yml```的```storage```字段。例如，假如你需要配置存储后端为Openstack swift，则storage段类似如下：
<pre>
storage:
  swift:
    username: admin
    password: ADMIN_PASS
    authurl: http://keystone_addr:35357/v3/auth
    tenant: admin
    domain: default
    region: regionOne
    container: docker_images
</pre>

想要了解详细的后端存储配置，请参看[Registry Configuration Reference ](https://docs.docker.com/registry/configuration/)

<br />

### 2.4 完成安装并启动Harbor
一旦harbor.cfg及存储后端(可选）完成配置，使用```install.sh```脚本完成安装并启动Harbor。

**1) 默认安装(without Notary/Clair)**

Harbor已经集成了Notary/Clair(用于vulnerability scanning）。然而，默认的安装并不包含Notary/Clair:
<pre>
# sudo ./install.sh
</pre>

假如一切工作正常的话，你可以打开一个用户界面，然后访问后台管理页面```http://reg.yourdomain.com/```(注意这里请将reg.yourdomain.com替换为你在harbor.cfg中配置的hostname字段的值），默认的后台管理username/password为admin/Harbor12345

登录admin管理页面，然后创建一个新的工程，例如```myproject```，你可以使用docker命令来登录并push镜像(默认情况下，registry server监听在80端口上）:
<pre>
# docker login reg.yourdomain.com
# docker push reg.yourdomain.com/myproject/myrepo:mytag
</pre>

```IMPORTANT```: 默认情况下安装Harbor，使用的是http协议。这样你必须为docker daemon添加```--insecure-registry```，并重启docker daemon服务


**2) Installation with Notary**

要安装带```Notary```服务的Harbor，你可以在运行```install.sh```脚本时添加一个参数：
<pre>
# sudo ./install.sh --with-notary
</pre>

```NOTE```: 要让Harbor支持Notary服务的话，```ui_url_protocol```必须配置为```https```。要配置https,请参考另外的章节

要了解更多关于```Notary```及```Docker Content Trust```相关信息，请参看docker相关文档：[Docker Content Trust](https://docs.docker.com/engine/security/trust/content_trust/)


**3) Installation with Clair**

要安装带```Clair```服务的Harbor，你可以在运行```install.sh```脚本时添加一个参数：
<pre>
# sudo ./install.sh --with-clair
</pre>

要想了解更多```Clair```相关信息，请参看[Clair文档](https://coreos.com/clair/docs/2.0.1/)

```注意```： 假如要同时支持Notary与Clair，你必须在同一个命令中同时指定这两个参数：
<pre>
# sudo ./install.sh --with-notary --with-clair
</pre>

欲了解更多Harbor的使用，请参看[User Guide of Harbor](https://github.com/vmware/harbor/blob/master/docs/user_guide.md)



## 3. 配置Harbor以支持https访问
Harbor本身在发布时并不提供任何证书，默认情况下，其使用http来提供相应服务。这使得Harbor可以相对容易来建立及运行，特别是在开发及测试环境中，这很重要。然而在实际的生产环境中，并不建议采用http。要使能https，请参看[Configuring Harbor with HTTPS Access](https://github.com/vmware/harbor/blob/master/docs/configure_https.md)


## 4. Harbor生命周期管理
你可以使用docker-compose来管理Harbor的生命周期，下面列出一些常用的命令（说明必须在docker-compose.yml文件所在目录运行)：

**1) 停止Harbor**
{% highlight string %}
# sudo docker-compose stop
Stopping nginx ... done
Stopping harbor-jobservice ... done
Stopping harbor-ui ... done
Stopping harbor-db ... done
Stopping registry ... done
Stopping harbor-log ... done
{% endhighlight %} 
 

**2) 在Harbor停止后，重启Harbor**
{% highlight string %}
# sudo docker-compose start
Starting log ... done
Starting ui ... done
Starting mysql ... done
Starting jobservice ... done
Starting registry ... done
Starting proxy ... done
{% endhighlight %}

**3) 如果要改变Harbor的配置，首先要停止当前已存在的Harbor实例，然后更新harbor.cfg。然后再运行```prepare```脚本更新配置文件，最后再重新创建并启动Harbor实例**
{% highlight string %}
# sudo docker-compose down -v
# vim harbor.cfg
# sudo prepare
# sudo docker-compose up -d
{% endhighlight %}

**4) 移除Harbor容器，但保留文件系统上的image data及Harbor数据库**
{% highlight string %}
# sudo docker-compose down -v
{% endhighlight %}

**5) 移除Harbor数据库及image data(用于干净环境下Harbor重装)**
{% highlight string %}
# rm -r /data/database
# rm -r /data/registry
{% endhighlight %}


### 4.1 Harbor with Notary生命周期管理
当Harbor被安装支持Notary服务时，需要给docker-compose提供一个额外的模板文件```docker-compose.notary.yml```。docker-compose管理Harbor生命周期的命令：
<pre>
# sudo docker-compose -f ./docker-compose.yml -f ./docker-compose.notary.yml [ up|down|ps|stop|start ]
</pre>
例如，假如你想要改变```harbor.cfg```配置文件，并重新部署带Notary服务的Harbor，那么你可以用如下的命令：
<pre>
# sudo docker-compose -f ./docker-compose.yml -f ./docker-compose.notary.yml down -v
# vim harbor.cfg
# sudo prepare --with-notary
# sudo docker-compose -f ./docker-compose.yml -f ./docker-compose.notary.yml up -d
</pre>

### 4.2 Harbor with Clair生命周期管理
当Harbor被安装支持Clair服务时，需要给docker-compose提供一个额外的模板文件```docker-compose.clair.yml```。docker-compose管理Clair生命周期的命令：
<pre>
#  sudo docker-compose -f ./docker-compose.yml -f ./docker-compose.clair.yml [ up|down|ps|stop|start ]
</pre>

例如，假如你想要改变```harbor.cfg```配置文件，并重新部署带Clair服务的Harbor，那么你可以用如下的命令：
<pre>
# sudo docker-compose -f ./docker-compose.yml -f ./docker-compose.clair.yml down -v
# vim harbor.cfg
# sudo prepare --with-clair
# sudo docker-compose -f ./docker-compose.yml -f ./docker-compose.clair.yml up -d
</pre>

### 4.3 Harbor with Notary and Clair生命周期管理
假如你安装了同时支持Notary及Clair服务的Harbor，你应该在docker-compose命令中包含两个组件：
<pre>
# sudo docker-compose -f ./docker-compose.yml -f ./docker-compose.notary.yml -f ./docker-compose.clair.yml down -v
# vim harbor.cfg
# sudo prepare --with-notary --with-clair
# sudo docker-compose -f ./docker-compose.yml -f ./docker-compose.notary.yml -f ./docker-compose.clair.yml up -d
</pre>
请参看[Docker Compose command-line reference](https://docs.docker.com/compose/reference/)以了解更多docker-compose的用法。

## 5. 持久化数据及日志文件






<br />
<br />

**[参考]**

1. [harbor官网](https://github.com/vmware/harbor)

2. [Centos7上Docker仓库Harbor的搭建](https://blog.csdn.net/felix_yujing/article/details/54694294)
<br />
<br />
<br />

