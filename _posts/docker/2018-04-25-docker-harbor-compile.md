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
<pre>

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

## 2. 获得Harbor源代码

这里我们的```GOPATH```路径为：
<pre>
# echo $GOPATH
/opt/gowork
<pre>
因此我们在```/opt/gowork```目录下创建harbor目录，然后将源代码git clone到那里：
<pre>
# cd /opt/gowork/
# mkdir -p src/github.com/vmware/ && src/github.com/vmware/
# git clone https://github.com/vmware/harbor
# ls && cd harbor
harbor  tes
</pre>

## 3. 构建并安装Harbor
这里我们原来的Harbor环境全部清除掉：
<pre>
# cd /opt/harbor-inst/harbor
# docker-compose down -v
# ls /data/*
# rm -rf /data/database
# rm -rf /data/registry
# rm -rf /data/*
# rm -rf /var/log/harbor*
# rm -rf common/config/

# docker ps
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES


//把原来的harbor相关镜像删除
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
# 
</pre>

### 3.1 配置
修改```make/harbor.cfg```配置文件，对其中的一些必要配置进行修改。请参看其他相关章节：
<pre>
# cd /opt/workdir/harbor
# vi make/harbor.cfg
</pre>

这里，我们修改如下几个字段：
<pre>
hostname = 192.168.69.128
ui_url_protocol = https
max_job_workers = 1
ssl_cert = /opt/cert/harbor-registry.crt
ssl_cert_key = /opt/cert/harbor-registry.key

#http_proxy =
#https_proxy =
#no_proxy = 127.0.0.1,localhost,ui
</pre>
将```http_proxy```，```https_proxy```，```no_proxy```这三行注释掉.

### 3.2 编译、运行

修改Makefile:
<pre>
CLARITYIMAGE=vmware/harbor-clarity-ui-builder[:tag]
</pre>
这里将```[:tag]```修改为```:1.4.0```， 再拉取```vmware/harbor-clarity-ui-builder:1.4.0```镜像
<pre>
# docker pull vmware/harbor-clarity-ui-builder：1.4.0
# docker images | grep clarity
vmware/harbor-clarity-ui-builder   1.4.0               937eb5e24878        7 days ago          1.54GB
</pre>

修改Makefile:
<pre>
GOBUILDIMAGE=reg.mydomain.com/library/harborgo[:tag]
</pre>
改为```GOBUILDIMAGE=vmware/harborgo:1.6.2```。


你可以采用如下三种方式中的一种来对源代码进行编译：

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



<br />
<br />

**[参考]**

1. [harbor官网](https://github.com/vmware/harbor)

2. [harbor compile](https://github.com/vmware/harbor/blob/master/docs/compile_guide.md)



<br />
<br />
<br />

