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
# rm -rf /data/database
# rm -rf /data/registry
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
默认情况下，registry的数据会被持久化到主机的```/data/```目录。即使在容器被移除或者重新创建的情况下，这些数据都会维持不变。

另外，Harbor使用```rsyslog```来收集每一个容器的日志。默认情况下，这些日志文件都被存储在```/var/log/harbor```目录下，我们可以使用这些日志来处理一些相关问题。

## 6. 定制化Harbor监听端口

默认情况下，Harbor会监听80端口(http)和443端口(假如配置了https)，以此来处理Harbor的后台管理操作及支持docker的相关命令。你也可以对这些端口进行相应的定制。

### 6.1 定制http协议端口

**1) 修改docker-compose.yml文件**

替换第一个```80```端口为一个定制化指定端口，例如```8888:80```: 
<pre>
proxy:
    image: library/nginx:1.11.5
    restart: always
    volumes:
      - ./config/nginx:/etc/nginx
    ports:
      - 8888:80
      - 443:443
    depends_on:
      - mysql
      - registry
      - ui
      - log
    logging:
      driver: "syslog"
      options:  
        syslog-address: "tcp://127.0.0.1:1514"
        tag: "proxy"
</pre>

**2) 修改harbor.cfg文件，添加端口到```hostname```参数**
<pre>
hostname = 192.168.0.2:8888
</pre>

**3) 重新部署Harbor**

请参看前面"Harbor生命周期管理"相关章节。

### 6.2 定制https协议端口

**1) 在Harbor中使能HTTPS**

请参看相关章节。

**2) 修改docker-compose.yml文件**

将第一个```443```端口替换为一个定制化指定端口，例如```8888:80```:
<pre>
proxy:
    image: library/nginx:1.11.5
    restart: always
    volumes:
      - ./config/nginx:/etc/nginx
    ports:
      - 80:80
      - 8888:443
    depends_on:
      - mysql
      - registry
      - ui
      - log
    logging:
      driver: "syslog"
      options:  
        syslog-address: "tcp://127.0.0.1:1514"
        tag: "proxy"
</pre>

**3) 修改harbor.cfg文件，添加端口到```hostname```参数**
<pre>
hostname = 192.168.0.2:8888
</pre>

**4) 重新部署Harbor**

请参看前面"Harbor生命周期管理"相关章节。 



## 7. 性能调优
默认情况下，Harbor会限制Clair容器的的CPU使用率为15000来避免其占用所有的CPU资源。这是在```docker-compose.clair.yml```文件中进行配置的。你可以根据你的硬件配置进行相应的修改。

## 8. Troubleshooting

1） 当Harbor不能正常工作时，通过运行如下的命令来找出是否所有的容器都处于```UP```状态
<pre>
# sudo docker-compose ps
        Name                     Command               State                    Ports                   
  -----------------------------------------------------------------------------------------------------
  harbor-db           docker-entrypoint.sh mysqld      Up      3306/tcp                                 
  harbor-jobservice   /harbor/harbor_jobservice        Up                                               
  harbor-log          /bin/sh -c crond && rsyslo ...   Up      127.0.0.1:1514->514/tcp                    
  harbor-ui           /harbor/harbor_ui                Up                                               
  nginx               nginx -g daemon off;             Up      0.0.0.0:443->443/tcp, 0.0.0.0:80->80/tcp 
  registry            /entrypoint.sh serve /etc/ ...   Up      5000/tcp 
</pre>
假如有一个container不是处于```UP```状态，请检查```/var/log/harbor```目录下该容器的日志。例如，假如```harbor-ui```没有运行的话，你可以查询```ui.log```日志文件。

2) 当在一个Nginx代代理或ELB(elastic load balancing)后端建立Harbor时，请在```common/templates/nginx/nginx.http.conf```文件中查询如下行：
<pre>
proxy_set_header X-Forwarded-Proto $scheme;
</pre>
假如代理中已经有类似于```location /, location /v2/ 与 location /service/```的设置，请将其从所在section移除，然后根据上面```Harbor生命周期管理```相关章节重新部署Harbor。


## 9. 部署示例
前面我们已经下载并解压好了harbor，这里我们进入解压好的根目录：
<pre>
# cd /opt/harbor-inst/harbor
# ls
common  docker-compose.clair.yml  docker-compose.notary.yml  docker-compose.yml  ha  harbor.cfg  harbor.v1.4.0.tar.gz  install.sh  LICENSE  NOTICE  prepare
</pre>

我们当前ip地址为```192.168.69.128```, 用netstat查看```80```、```443```等端口也没有被占用。

**1) 修改harbor.cfg的```hostname```字段**
<pre>
hostname = 192.168.69.128
</pre>

**2) 执行install.sh脚本**
<pre>
# ./install.sh 

[Step 0]: checking installation environment ...

Note: docker version: 17.12.1

Note: docker-compose version: 1.20.1

[Step 1]: loading Harbor images ...
651f69aef02c: Loading layer [==================================================>]  135.8MB/135.8MB
40a1aad64343: Loading layer [==================================================>]  23.24MB/23.24MB
3fe2713e4072: Loading layer [==================================================>]  12.16MB/12.16MB
ba3a1eb0e375: Loading layer [==================================================>]   17.3MB/17.3MB
447427ec5e1a: Loading layer [==================================================>]  15.87kB/15.87kB
4ccb4026663c: Loading layer [==================================================>]  3.072kB/3.072kB
16faa95946a1: Loading layer [==================================================>]  29.46MB/29.46MB
Loaded image: vmware/notary-server-photon:v0.5.1-v1.4.0
fa7ba9fd42c9: Loading layer [==================================================>]  10.95MB/10.95MB
4e400f9ae23e: Loading layer [==================================================>]   17.3MB/17.3MB
2802fb27c88b: Loading layer [==================================================>]  15.87kB/15.87kB
e6367a4e1e1e: Loading layer [==================================================>]  3.072kB/3.072kB
8ece8dfcdd98: Loading layer [==================================================>]  28.24MB/28.24MB
Loaded image: vmware/notary-signer-photon:v0.5.1-v1.4.0
a7dd1a8afcaf: Loading layer [==================================================>]  396.7MB/396.7MB
05adebbe496f: Loading layer [==================================================>]  9.216kB/9.216kB
86eb534949fa: Loading layer [==================================================>]  9.216kB/9.216kB
d7f127c69380: Loading layer [==================================================>]   7.68kB/7.68kB
5ac1c4dc5ee9: Loading layer [==================================================>]  1.536kB/1.536kB
d0bec56b5b1a: Loading layer [==================================================>]  9.728kB/9.728kB
4bbe83860556: Loading layer [==================================================>]   2.56kB/2.56kB
e526f9e6769f: Loading layer [==================================================>]  3.072kB/3.072kB
Loaded image: vmware/harbor-db:v1.4.0
1cff102bbda2: Loading layer [==================================================>]  154.1MB/154.1MB
04c9f3e07de1: Loading layer [==================================================>]  10.75MB/10.75MB
7b6c7bf54f5c: Loading layer [==================================================>]  2.048kB/2.048kB
42f8acdb7fe3: Loading layer [==================================================>]  48.13kB/48.13kB
5b6299d0a1df: Loading layer [==================================================>]   10.8MB/10.8MB
Loaded image: vmware/clair-photon:v2.0.1-v1.4.0
6534131f457c: Loading layer [==================================================>]  94.76MB/94.76MB
73f582101e4b: Loading layer [==================================================>]  6.656kB/6.656kB
86d847823c48: Loading layer [==================================================>]  6.656kB/6.656kB
Loaded image: vmware/postgresql-photon:v1.4.0
5cd250d5a352: Loading layer [==================================================>]  23.24MB/23.24MB
ad3fd52b54f3: Loading layer [==================================================>]  14.99MB/14.99MB
13b1e24cc368: Loading layer [==================================================>]  14.99MB/14.99MB
Loaded image: vmware/harbor-adminserver:v1.4.0
c26c69706710: Loading layer [==================================================>]  23.24MB/23.24MB
223f6fe02cc8: Loading layer [==================================================>]  23.45MB/23.45MB
1fc843c8698a: Loading layer [==================================================>]  7.168kB/7.168kB
e09293610ee7: Loading layer [==================================================>]  10.39MB/10.39MB
d59f9780b1d8: Loading layer [==================================================>]  23.44MB/23.44MB
Loaded image: vmware/harbor-ui:v1.4.0
dd4753242e59: Loading layer [==================================================>]  73.07MB/73.07MB
95aed61ca251: Loading layer [==================================================>]  3.584kB/3.584kB
1864f9818562: Loading layer [==================================================>]  3.072kB/3.072kB
da2a19f80b81: Loading layer [==================================================>]  4.096kB/4.096kB
058531639e75: Loading layer [==================================================>]  3.584kB/3.584kB
a84e69fb619b: Loading layer [==================================================>]  10.24kB/10.24kB
Loaded image: vmware/harbor-log:v1.4.0
b1056051f246: Loading layer [==================================================>]  23.24MB/23.24MB
07678065e08b: Loading layer [==================================================>]  19.19MB/19.19MB
a2d9bdb8f5fb: Loading layer [==================================================>]  19.19MB/19.19MB
Loaded image: vmware/harbor-jobservice:v1.4.0
7f58ce57cd5e: Loading layer [==================================================>]  4.805MB/4.805MB
Loaded image: vmware/nginx-photon:v1.4.0
4c8965978b77: Loading layer [==================================================>]  23.24MB/23.24MB
1466c942edde: Loading layer [==================================================>]  2.048kB/2.048kB
ac5c17331735: Loading layer [==================================================>]  2.048kB/2.048kB
86824c7c466a: Loading layer [==================================================>]  2.048kB/2.048kB
fd3bd0e70d67: Loading layer [==================================================>]   22.8MB/22.8MB
b02195d77636: Loading layer [==================================================>]   22.8MB/22.8MB
Loaded image: vmware/registry-photon:v2.6.2-v1.4.0
Loaded image: vmware/photon:1.0
Loaded image: vmware/mariadb-photon:v1.4.0
454c81edbd3b: Loading layer [==================================================>]  135.2MB/135.2MB
e99db1275091: Loading layer [==================================================>]  395.4MB/395.4MB
051e4ee23882: Loading layer [==================================================>]  9.216kB/9.216kB
6cca4437b6f6: Loading layer [==================================================>]  9.216kB/9.216kB
1d48fc08c8bc: Loading layer [==================================================>]   7.68kB/7.68kB
0419724fd942: Loading layer [==================================================>]  1.536kB/1.536kB
526b2156bd7a: Loading layer [==================================================>]  637.8MB/637.8MB
9ebf6900ecbd: Loading layer [==================================================>]  78.34kB/78.34kB
Loaded image: vmware/harbor-db-migrator:1.4


[Step 2]: preparing environment ...
Generated and saved secret to file: /data/secretkey
Generated configuration file: ./common/config/nginx/nginx.conf
Generated configuration file: ./common/config/adminserver/env
Generated configuration file: ./common/config/ui/env
Generated configuration file: ./common/config/registry/config.yml
Generated configuration file: ./common/config/db/env
Generated configuration file: ./common/config/jobservice/env
Generated configuration file: ./common/config/log/logrotate.conf
Generated configuration file: ./common/config/jobservice/app.conf
Generated configuration file: ./common/config/ui/app.conf
Generated certificate, key file: ./common/config/ui/private_key.pem, cert file: ./common/config/registry/root.crt
The configuration files are ready, please use docker-compose to start the service.


[Step 3]: checking existing instance of Harbor ...


[Step 4]: starting Harbor ...
Creating network "harbor_harbor" with the default driver
Creating harbor-log ... done
Creating harbor-adminserver ... done
Creating harbor-db          ... done
Creating registry           ... done
Creating harbor-ui          ... done
Creating harbor-jobservice  ... done
Creating nginx              ... done

✔ ----Harbor has been installed and started successfully.----

Now you should be able to visit the admin portal at http://192.168.69.128. 
For more details, please visit https://github.com/vmware/harbor .

</pre>

**3) 查询Harbor运行状态**
{% highlight string %}
# docker ps
CONTAINER ID        IMAGE                                  COMMAND                  CREATED             STATUS                            PORTS                                                              NAMES
10b95448f80f        vmware/nginx-photon:v1.4.0             "nginx -g 'daemon of…"   5 seconds ago       Up 4 seconds                      0.0.0.0:80->80/tcp, 0.0.0.0:443->443/tcp, 0.0.0.0:4443->4443/tcp   nginx
64893e6ba9d3        vmware/harbor-jobservice:v1.4.0        "/harbor/start.sh"       5 seconds ago       Up 4 seconds (health: starting)                                                                      harbor-jobservice
62220b07e57f        vmware/harbor-ui:v1.4.0                "/harbor/start.sh"       5 seconds ago       Up 5 seconds (health: starting)                                                                      harbor-ui
ce166d26724e        vmware/harbor-db:v1.4.0                "/usr/local/bin/dock…"   7 seconds ago       Up 6 seconds (health: starting)   3306/tcp                                                           harbor-db
a62d8f460c35        vmware/registry-photon:v2.6.2-v1.4.0   "/entrypoint.sh serv…"   7 seconds ago       Up 5 seconds (health: starting)   5000/tcp                                                           registry
5e5e4bcee123        vmware/harbor-adminserver:v1.4.0       "/harbor/start.sh"       7 seconds ago       Up 6 seconds (health: starting)                                                                      harbor-adminserver
cb6dbc564382        vmware/harbor-log:v1.4.0               "/bin/sh -c /usr/loc…"   7 seconds ago       Up 6 seconds (health: starting)   127.0.0.1:1514->10514/tcp                                          harbor-log
{% endhighlight %}

**4) 访问**

首先我们用curl命令访问一下：
<pre>
# curl -X GET http://192.168.69.128 -k -IL
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 02:43:20 GMT
Content-Type: text/html; charset=utf-8
Content-Length: 810
Connection: keep-alive
Set-Cookie: beegosessionID=1720767232a3cdcb58a54cd13eead058; Path=/; HttpOnly
</pre>

然后我们再用浏览器访问。

**5） 向Harbor push/pull镜像**

* 停止Harbor
<pre>
# docker-compose stop
Stopping nginx              ... done
Stopping harbor-jobservice  ... done
Stopping harbor-ui          ... done
Stopping harbor-db          ... done
Stopping registry           ... 
Stopping registry           ... done
Stopping harbor-adminserver ... done
Stopping harbor-log         ... done
</pre>

* 修改dockerd启动脚本

这里修改```/lib/systemd/system/docker.service```文件，将```ExecStart```修改为：
<pre>
ExecStart=/usr/bin/dockerd \
        --insecure-registry=192.168.69.128 \
        -H tcp://0.0.0.0:2375 \
        -H unix://var/run/docker.sock \
        -H tcp://0.0.0.0:7654
</pre>
上面添加了```--insecure-registry```选项。然后执行再执行如下命令重启dockerd:
<pre>
# systemctl daemon-reload

# systemctl restart docker
</pre>

**6） 重启Harbor**
{% highlight string %}
# docker-compose start
Starting log         ... done
Starting registry    ... done
Starting mysql       ... done
Starting adminserver ... done
Starting ui          ... done
Starting jobservice  ... done
Starting proxy       ... done
# docker ps
CONTAINER ID        IMAGE                                  COMMAND                  CREATED             STATUS                            PORTS                                                              NAMES
10b95448f80f        vmware/nginx-photon:v1.4.0             "nginx -g 'daemon of…"   21 minutes ago      Up Less than a second             0.0.0.0:80->80/tcp, 0.0.0.0:443->443/tcp, 0.0.0.0:4443->4443/tcp   nginx
64893e6ba9d3        vmware/harbor-jobservice:v1.4.0        "/harbor/start.sh"       21 minutes ago      Up 4 seconds (health: starting)                                                                      harbor-jobservice
62220b07e57f        vmware/harbor-ui:v1.4.0                "/harbor/start.sh"       21 minutes ago      Up 4 seconds (health: starting)                                                                      harbor-ui
ce166d26724e        vmware/harbor-db:v1.4.0                "/usr/local/bin/dock…"   21 minutes ago      Up 57 seconds (healthy)           3306/tcp                                                           harbor-db
a62d8f460c35        vmware/registry-photon:v2.6.2-v1.4.0   "/entrypoint.sh serv…"   21 minutes ago      Up 57 seconds (healthy)           5000/tcp                                                           registry
5e5e4bcee123        vmware/harbor-adminserver:v1.4.0       "/harbor/start.sh"       21 minutes ago      Up 4 seconds (health: starting)                                                                      harbor-adminserver
cb6dbc564382        vmware/harbor-log:v1.4.0               "/bin/sh -c /usr/loc…"   21 minutes ago      Up 57 seconds (healthy)           127.0.0.1:1514->10514/tcp                                          harbor-log
{% endhighlight %}

**7) 往Harbor中push/pull镜像**

* 登录
<pre>
# docker login 192.168.69.128
Username: admin
Password: 
Login Succeeded
</pre>

* 重新为镜像打tag
<pre>
# docker images
REPOSITORY                    TAG                 IMAGE ID            CREATED             SIZE
test-image                    latest              fe9c46d12863        7 days ago          195MB
friendlyhello                 latest              f2ae8dec6267        9 days ago          150MB
redis                         alpine              c27f56585938        3 weeks ago         27.7MB

//这里我们用Harbor中的默认库
# docker tag redis:alpine 192.168.69.128/library/redis:alpine
# docker images | grep redis
192.168.69.128/library/redis          alpine              c27f56585938        3 weeks ago         27.7MB
redis                         alpine              c27f56585938        3 weeks ago         27.7MB
</pre>



* 上传镜像到Harbor
<pre>
# docker push 192.168.69.128/library/redis:alpine
The push refers to repository [192.168.69.128/library/redis]
f6b9463783dc: Pushed 
222a85888a99: Pushed 
1925395eabdd: Pushed 
c3d278563734: Pushed 
ad9247fe8c63: Pushed 
cd7100a72410: Pushed 
alpine: digest: sha256:9d017f829df3d0800f2a2582c710143767f6dda4df584b708260e73b1a1b6db3 size: 1568
</pre>

然后我们登录网站，可以看到镜像上传成功。（注： 这里Harbor默认采用```Www-Authenticate: Bearer```认证）

* 下载镜像
<pre>
//这里我们先把原来本地的镜像删除
# docker rmi 192.168.69.128/library/redis:alpine

//从Harbor镜像库拉取镜像
# docker pull 192.168.69.128/library/redis:alpine
alpine: Pulling from library/redis
Digest: sha256:9d017f829df3d0800f2a2582c710143767f6dda4df584b708260e73b1a1b6db3
Status: Downloaded newer image for 192.168.69.128/library/redis:alpine
</pre>



<br />
<br />

**[参考]**

1. [harbor官网](https://github.com/vmware/harbor)

2. [Centos7上Docker仓库Harbor的搭建](https://blog.csdn.net/felix_yujing/article/details/54694294)
<br />
<br />
<br />

