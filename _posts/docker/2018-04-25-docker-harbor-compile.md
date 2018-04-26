---
layout: post
title: Harbor源代码编译
tags:
- docker
categories: docker
description: Harbor源代码编译
---


本章记录一下Harbor源代码的编译流程.



<!-- more -->

## 1. 准备Harbor编译环境
Harbor是由多个Docker容器所组成，并且大部分的源代码都是采用Go语言来编写的。这里Harbor的编译环境需要： Python，Docker，Docker compose和Go开发环境。请安装如下必要环境：

|        Software        |         Required Version          | 
|:----------------------:|:---------------------------------:|
|      docker            |             1.12.0 +              | 
|    docker-compose      |             1.11.0 +              | 
|     python             |             2.7 +                 |  
|     git                |             1.9.1 +               |
|     make               |             3.81 +                |
|     golang*            |             1.7.3 +               |

<pre>
注： golang为可选项，只在你想要使用你自己的golang环境时才需要
</pre>

当前，这些环境我们都已安装好：
<pre>
# docker --version
Docker version 17.12.1-ce, build 7390fc6

# docker-compose --version
docker-compose version 1.20.1, build 5d8c71b

# python --version
Python 2.7.5

# git --version
git version 1.8.3.1

# make --version
GNU Make 3.82
Built for x86_64-redhat-linux-gnu
Copyright (C) 2010  Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

# go version
go version go1.8.3 linux/amd64
</pre>
上面git版本低于所要求版本，应该不存在问题。

## 2. 获取Harbor源代码
<pre>
# git clone https://github.com/vmware/harbor
</pre>

## 3. 构建并安装Harbor

### 3.1 配置
修改```make/harbor.cfg```配置文件，对其中的一些必要配置进行修改。请参看其他相关章节：
<pre>
# cd harbor
# vi make/harbor.cfg
</pre>

### 3.2 编译运行
你可以采用如下两种方式中的一种来对源代码进行编译：

**方式1： 使用官方Golang镜像来编译**

* 从Docker Hub下载官方Golang镜像
<pre>
# docker pull golang:1.9.2
# docker images | grep golang
golang                         1.9.2               138bd936fa29        4 months ago        733MB
</pre>


* 构建，安装并运行不带Notary的Harbor
<pre>
# make install GOBUILDIMAGE=golang:1.9.2 COMPILETAG=compile_golangimage \
 CLARITYIMAGE=vmware/harbor-clarity-ui-builder:1.4.0
</pre>

* 构建，安装并运行带Notary的Harbor 
<pre>
# make install GOBUILDIMAGE=golang:1.9.2 COMPILETAG=compile_golangimage \
 CLARITYIMAGE=vmware/harbor-clarity-ui-builder:1.4.0 NOTARYFLAG=true
</pre>

* 构建，安装并运行带Clair的Harbor 
<pre>
# make install GOBUILDIMAGE=golang:1.9.2 COMPILETAG=compile_golangimage \
 CLARITYIMAGE=vmware/harbor-clarity-ui-builder:1.4.0 CLAIRFLAG=true
</pre>

**方式2： 使用自己的Golang环境来构建Harbor**

* 将源代码移动到```GOPATH```路径
<pre>
# mkdir $GOPATH/src/github.com/vmware/
# cd ..
# mv harbor $GOPATH/src/github.com/vmware/.
</pre>

* 构建，安装并运行不带Notary与Clair的Harbor 
<pre>
# cd $GOPATH/src/github.com/vmware/harbor
# make install
</pre>

* 构建，安装并运行带Notary与Clair的Harbor 
<pre>
# cd $GOPATH/src/github.com/vmware/harbor
# make install -e NOTARYFLAG=true CLAIRFLAG=true
</pre>

### 3.3 验证
假如一切进展顺利的话，你将会得到如下的打印消息：
<pre>
...
Start complete. You can visit harbor now.
</pre>
这时Harbor会启动，你可以根据[ Installation and Configuration Guide](https://github.com/vmware/harbor/blob/master/docs/installation_guide.md#managing-harbors-lifecycle)来对Harbor进行验证。


## 4. Harbor编译示例

下面我们以一个实际的例子来讲述一下Harbor源代码的编译过程

### 4.1 安装Go编译环境
这里我们安装Go编译环境：
<pre>
# cd /opt
# wget https://storage.googleapis.com/golang/go1.9.linux-amd64.tar.gz
# tar -zxvf go1.9.linux-amd64.tar.gz -C /usr/local/
</pre>
在```/etc/profile```文件中导出如下变量：
<pre>
export GOROOT=/usr/local/go/
export GOBIN=/usr/local/go/bin/
export GOPATH=/opt/gowork/
export GOOS=linux
export GOARCH=amd64
export PATH=$PATH:$GOROOT:$GOBIN
</pre>
使相应变量生效，并创建go工作目录,然后再检查是否安装成功：
<pre>
# source /etc/profile

# mkdir -p /opt/gowork/src

# which go
/usr/local/go/bin/go
# go version
go version go1.9 linux/amd64

# echo $GOROOT $GOPATH
/usr/local/go/ /opt/gowork/
</pre>

### 4.2 清理Harbor环境
这里我们把原来的Harbor环境全部清除掉：
<pre>
# cd /opt/harbor-inst/harbor
# docker-compose down -v
# rm -rf /data/*
# rm -rf /var/log/harbor*
# rm -rf common/config

# docker ps
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
</pre>

然后再清除掉原来下载下来的Harbor相关镜像：
<pre>
# docker images | grep vmware | awk '{print $1":"$2}'
vmware/clair-photon:v2.0.1-v1.4.0
vmware/notary-server-photon:v0.5.1-v1.4.0
vmware/notary-signer-photon:v0.5.1-v1.4.0
vmware/registry-photon:v2.6.2-v1.4.0
vmware/nginx-photon:v1.4.0
vmware/harbor-log:v1.4.0
vmware/harbor-jobservice:v1.4.0
vmware/harbor-ui:v1.4.0
vmware/harbor-adminserver:v1.4.0
vmware/harbor-db:v1.4.0
vmware/mariadb-photon:v1.4.0
vmware/postgresql-photon:v1.4.0
vmware/harbor-db-migrator:1.4
vmware/photon:1.0
# docker images | grep vmware | awk '{print $1":"$2}' | xargs docker rmi
</pre>

### 4.3 下载Harbor源代码
这里我们在实际使用过程中，并不会采用最新的Harbor源代码，而是采用相对较新的稳定版本。我们下载```v1.4.0```版本到```$GOPATH/src```目录下：
<pre>
# echo $GOPATH
/opt/gowork/

# cd /opt/gowork/src
# wget https://github.com/vmware/harbor/archive/v1.4.0.tar.gz
# tar -zxvf v1.4.0.tar.gz
# ls
harbor-1.4.0  v1.4.0.tar.gz
</pre>

### 4.4 下载相关依赖镜像
这里我们会依赖到两个镜像： ```harbor-clarity-ui-builder```与```golang```。

* 这里我们首先拉取harbor-clarity-ui-build镜像，当前所用版本为```1.4.0```
<pre>
# docker pull vmware/harbor-clarity-ui-builder:1.4.0
# docker images | grep clarity
vmware/harbor-clarity-ui-builder   1.4.0               937eb5e24878        7 days ago          1.54GB
</pre>

* 然后再拉取golang镜像，当前使用版本为```1.9.2```
<pre>
# docker pull golang:1.9.2
# docker tag golang:1.9.2 vmware/harborgo:1.9.2
# docker images | grep harborgo
vmware/harborgo                    1.9.2               138bd936fa29        4 months ago        733MB
</pre>


### 4.5 编译并运行Harbor1.4

* **将上面下载的Harbor源代码移动到```$GOPATH```目录下的相应位置**
<pre>
# pwd
/opt/gowork/src

# mv harbor-1.4.0 $GOPATH/src/github.com/vmware/.
# cd $GOPATH/src/github.com/vmware/. && mv harbor-1.4.0 harbor
</pre>
注意，上面必须把名字```harbor-1.4.0```改为harbor，否则后面编译时可能导致相应的错误。


* **修改相关配置文件**

进入harbor源代码目录：
<pre>
# cd cd $GOPATH/src/github.com/vmware/harbor
</pre>

1) 修改```make/harbor.cfg```文件
<pre>
hostname = 192.168.69.128
ui_url_protocol = https
max_job_workers = 1
ssl_cert = /opt/cert/harbor-registry.crt
ssl_cert_key = /opt/cert/harbor-registry.key
</pre>

2) 修改harbor目录下的Makefile文件

首先将如下行：
<pre>
CLARITYIMAGE=vmware/harbor-clarity-ui-builder[:tag]
</pre>
修改为
{% highlight string %}
CLARITYIMAGE=vmware/harbor-clarity-ui-builder:1.4.0
{% endhighlight %}

再将如下行：
<pre>
GOBUILDIMAGE=reg.mydomain.com/library/harborgo[:tag]
</pre>
修改为：
<pre>
GOBUILDIMAGE=vmware/harborgo:1.9.2
</pre>



* **编译Harbor源代码，并运行**
<pre>
# make install
</pre>
上面Harbor源代码编译会耗费一段时间，编译完成之后就会自动运行Harbor:
<pre>
# docker ps
CONTAINER ID        IMAGE                               COMMAND                  CREATED             STATUS                 PORTS                                                              NAMES
98bf7bff91db        vmware/harbor-jobservice:dev        "/harbor/start.sh"       3 hours ago         Up 3 hours (healthy)                                                                      harbor-jobservice
feb4c66b8e75        vmware/nginx-photon:dev             "nginx -g 'daemon of…"   3 hours ago         Up 3 hours             0.0.0.0:80->80/tcp, 0.0.0.0:443->443/tcp, 0.0.0.0:4443->4443/tcp   nginx
6c1d9e8f3e80        vmware/harbor-ui:dev                "/harbor/start.sh"       3 hours ago         Up 3 hours (healthy)                                                                      harbor-ui
fff4eab5c159        vmware/harbor-db:dev                "/usr/local/bin/dock…"   3 hours ago         Up 3 hours (healthy)   3306/tcp                                                           harbor-db
8d598f0a2d6e        vmware/harbor-adminserver:dev       "/harbor/start.sh"       3 hours ago         Up 3 hours (healthy)                                                                      harbor-adminserver
557b2b17f928        vmware/registry-photon:v2.6.2-dev   "/entrypoint.sh serv…"   3 hours ago         Up 3 hours (healthy)   5000/tcp                                                           registry
c02ab8aa35cd        vmware/harbor-log:dev               "/bin/sh -c /usr/loc…"   3 hours ago         Up 3 hours (healthy)   127.0.0.1:1514->10514/tcp                                          harbor-log
e4cdabfee168        golang:1.9.2                        "bash"                   4 hours ago         Up 4 hours                                                                                harborgo
</pre>


### 4.6 测试
在上述编译完成后，我们可以进行简单的测试，看Harbor是否工作正常:
<pre>
# docker login 192.168.69.128
# docker push 192.168.69.128/library/nginx
</pre>


### 4.7 查看编译结果
进入harbor目录下的```make/dev```目录，查看：
<pre>
# cd make/dev
# tree
.
├── adminserver
│   ├── Dockerfile
│   └── harbor_adminserver
├── docker-compose.yml
├── jobservice
│   ├── Dockerfile
│   └── harbor_jobservice
├── nodeclarity
│   ├── angular-cli.json
│   ├── Dockerfile
│   ├── entrypoint.sh
│   └── index.html
└── ui
    ├── Dockerfile
    └── harbor_ui

4 directories, 11 files
</pre>
我们看到这里编译出了```adminserver```、```jobservice```、```nodeclarity```、```ui```四个目录。其中```nodeclarity```我们暂时不会用到。后续我们需要进行扩展开发也主要是针对：

* harbor_adminserver

* harbor_jobservice

* harbor_ui


## 5. 附录

### 5.1 Harbor Makefile的使用

根目录下的Makefile文件包括如下的一些配置参数：

|        Variable        |                     Description                                    | 
|------------------------|--------------------------------------------------------------------|
|      BASEIMAGE         |            容器的基础镜像，默认为：photon                             | 
|    CLARITYIMAGE        |  Clarity UI(抗脆弱性)构建镜像，默认为:harbor-clarity-ui-builder       | 
|     DEVFLAG            |  构建模型标签，默认为:dev(实际发布时，我们可以修改为对应的版本号)         |  
|     COMPILETAG         |  编译模型标签，默认为: compile_normal(使用本地golang来构建)            |
|     NOTARYFLAG         |  Notary模型标签，默认为： false                                      |
|     CLAIRFLAG          |  Clair模型标签，默认为： false                                       |
|     HTTPPROXY          |  针对Clarity UI builder的NPM http代理                               | 
|    REGISTRYSERVER      |  远程Registry服务的地址                                              | 
|    REGISTRYUSER        |  远程Registry服务的用户名                                            |  
|  REGISTRYPASSWORD      |  远程Registry服务的密码                                              |
| REGISTRYPROJECTNAME    |  远程Registry服务上的一个工程的名称(Project Name)                      |
|     VERSIONTAG         |  Harbor镜像的版本标志，默认为:dev(实际发布时，我们可以修改为对应的版本号)  |
|     PKGVERSIONTAG      |  Harbor online与offline版本标志                                      |



### 5.2 Makefile中预先定义的target

根目录下的Makefile文件中预先定义了大量的target:

|        Target          |                     Description                                                      | 
|------------------------|--------------------------------------------------------------------------------------|
|          all           |     用于准备env, 编译Harbor各组件相应二进制文件，构建镜像，然后运行镜像                     | 
|       prepare          |     用于准备env                                                                       | 
|     compile            |     编译ui及jobservice源代码                                                           |  
|     compile_ui         |     编译ui二进制文件                                                                   |
|  compile_jobservice    |     编译jobservice二进制文件                                                           |
|     compile_clarity    |     编译clarity二进制文件                                                              |
|         build          |     构建Harbor docker镜像（默认：使用build_photon)                                     | 
|      build_photon      |     以Photon OS base image作为基础镜像来构建Harbor镜像                                  | 
|         install        | 编译二进制文件，构建镜像，准备特定版本的compose文件并且启动Harbor示例                       |  
|         start          | 启动Harbor实例（当要启动Notary时，请设置NOTARYFLAG=true                                  |
|          down          | 停止Harbor实例（当Harbor实例是以Notary方式启动时，停止时也应加上NOTARYFLAG=true            |
|     package_online     | 构建Harbor在线安装包                                                                   |
|     package_offline    | 构建Harbor离线安装包                                                                   |
|      pushimage         | 推送harbor镜像到一个指定的镜像仓库                                                       |
|      clean all         | 移除二进制文件，Harbor镜像，特定版本的docker-compose文件，特定版本tag以及online/offline安装包|
|     cleanbinary        | 移除ui及jobservice二进制文件                                                            |
|        cleanimage      | 移除Harbor镜像文 件                                                                     |
| cleandockercomposefile | 移除指定版本的doc ker-compose文件                                                       |
|     cleanversiontag    | 移除指定版本的tag                                                                      |
|     cleanpackage       | 移除online/offline安装包                                                               |


### 5.3 示例

**1) 推送harbor镜像到指定的registry服务器**
<pre>
make pushimage -e DEVFLAG=false REGISTRYSERVER=[$SERVERADDRESS] REGISTRYUSER=[$USERNAME] \
 REGISTRYPASSWORD=[$PASSWORD] REGISTRYPROJECTNAME=[$PROJECTNAME]
</pre> 

注意：需要添加一个```/```到```REGISTRYSERVER```后。假如```REGISTRYSERVER```并未设置的话，镜像将会被直接push到Docker Hub上
<pre>
# make pushimage -e DEVFLAG=false REGISTRYUSER=[$USERNAME] \
 REGISTRYPASSWORD=[$PASSWORD] REGISTRYPROJECTNAME=[$PROJECTNAME]
</pre>


**2) 清除指定版本的二进制文件及镜像**
<pre>
# make clean -e VERSIONTAG=[TAG]
</pre>

注意： 假如有新的代码传递到Github上，则```TAG```可能会发生改变。使用此命令一般是清除以前版本的TAG镜像及文件

默认情况下，Make进程会创建一个```development build```。假如要构建一个发布版本的```Harbor```，可以设置```DEVFLAG```为false:
<pre>
# make XXXX -e DEVFLAG=false
</pre>


<br />
<br />

**[参考]**

1. [harbor官网](https://github.com/vmware/harbor)

2. [harbor compile](https://github.com/vmware/harbor/blob/master/docs/compile_guide.md)



<br />
<br />
<br />

