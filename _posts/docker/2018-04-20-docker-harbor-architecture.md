---
layout: post
title: Harbor整体架构
tags:
- docker
categories: docker
description: Harbor整体架构
---


本节我们主要分析一下Harbor中docker-compose.yml文件，以了解Harbor整个系统的一个整体架构及工作状况。



<!-- more -->

## 1. Harbor整体架构
默认情况下，Harbor运行起来后有如下容器：
{% highlight string %}
# docker ps
CONTAINER ID        IMAGE                                  COMMAND                  CREATED             STATUS                            PORTS                                                              NAMES
248ae75cf72b        vmware/nginx-photon:v1.4.0             "nginx -g 'daemon of…"   4 seconds ago       Up 3 seconds                      0.0.0.0:80->80/tcp, 0.0.0.0:443->443/tcp, 0.0.0.0:4443->4443/tcp   nginx
2f4278759096        vmware/harbor-jobservice:v1.4.0        "/harbor/start.sh"       4 seconds ago       Up 4 seconds (health: starting)                                                                      harbor-jobservice
5977ecfd082b        vmware/harbor-ui:v1.4.0                "/harbor/start.sh"       5 seconds ago       Up 4 seconds (health: starting)                                                                      harbor-ui
ff6fc31844a9        vmware/harbor-db:v1.4.0                "/usr/local/bin/dock…"   5 seconds ago       Up 3 seconds (health: starting)   3306/tcp                                                           harbor-db
2ed6ff381ab9        vmware/harbor-adminserver:v1.4.0       "/harbor/start.sh"       5 seconds ago       Up 4 seconds (health: starting)                                                                      harbor-adminserver
d3e1e93bce1b        vmware/registry-photon:v2.6.2-v1.4.0   "/entrypoint.sh serv…"   5 seconds ago       Up 4 seconds (health: starting)   5000/tcp                                                           registry
096310feb030        vmware/harbor-log:v1.4.0               "/bin/sh -c /usr/loc…"   6 seconds ago       Up 5 seconds (health: starting)   127.0.0.1:1514->10514/tcp                                          harbor-log
{% endhighlight %}
名称分别为：```nginx```、```harbor-jobservice```、```harbor-ui```、```harbor-db```、```harbor-adminserver```、```registry```以及```harbor-log```。

结合上面的运行状况，下面我们再给出Harbor的整体架构图（当前实际架构已略有改动）：

![docker-harbor-arch](https://ivanzz1001.github.io/records/assets/img/docker/docker_harbor_arch.png)

如上图所描述，Harbor由6个大的模块所组成：

* **Proxy**: Harbor的registry、UI、token services等组件，都处在一个反向代理后边。该代理将来自浏览器、docker clients的请求转发到后端服务上。

* **Registry**: 负责存储Docker镜像，以及处理Docker push/pull请求。因为Harbor强制要求对镜像的访问做权限控制， 在每一次push/pull请求时，Registry会强制要求客户端从token service那里获得一个有效的token。

* **Core services**: Harbor的核心功能，主要包括如下3个服务:

* **UI**: 作为Registry Webhook, 以图像用户界面的方式辅助用户管理镜像。1) ```WebHook```是在registry中配置的一种机制， 当registry中镜像发生改变时，就可以通知到Harbor的webhook endpoint。Harbor使用webhook来更新日志、初始化同步job等。 2) ```Token service```会根据该用户在一个工程中的角色，为每一次的push/pull请求分配对应的token。假如相应的请求并没有包含token的话，registry会将该请求重定向到token service。 3) ```Database``` 用于存放工程元数据、用户数据、角色数据、同步策略以及镜像元数据。

* **Job services**: 主要用于镜像复制，本地镜像可以被同步到远程Harbor实例上。

* **Log collector**: 负责收集其他模块的日志到一个地方

这里我们与上面运行的7个容器对比，对```harbor-adminserver```感觉有些疑虑。其实这里```harbor-adminserver```主要是作为一个后端的配置数据管理，并没有太多的其他功能。```harbor-ui```所要操作的所有数据都通过harbor-adminserver这样一个数据配置管理中心来完成。

## 2. Harbor实现

Harbor的每一个组件都被包装成一个docker容器。自然，Harbor是通过docker compose来部署的。在[Harbor源代码](https://github.com/vmware/harbor)的make目录下的docker-compose模板会被用于部署Harbor。打开该模板文件，可以看到Harbor由7个容器组件所组成：

* ```proxy```: 通过nginx服务器来做反向代理 

* ```registry```: docker官方发布的一个仓库镜像组件

* ```ui```: 整个架构的核心服务。该容器是Harbor工程的主要部分

* ```adminserver```: 作为Harbor工程的配置数据管理器使用

* ```mysql```: 通过官方Mysql镜像创建的数据库容器

* ```job services```: 通过状态机的形式将镜像复制到远程Harbor实例。镜像删除同样也可以被同步到远程Harbor实例中。

* ```log```: 运行rsyslogd的容器，主要用于收集其他容器的日志

这些容器之间都通过Docker内的DNS服务发现来连接通信。通过这种方式，每一个容器都可以通过相应的容器来进行访问。对于终端用户来说，只有反向代理(Nginx)服务的端口需要对外暴露。

如下两个Docker命令行的例子显示了Harbor各组件之间的交互。

## 3. docker login处理流程

## 4. docker push处理流程



<br />
<br />

**[参考]**

1. [Architecture Overview of Harbor](https://github.com/vmware/harbor/wiki/Architecture-Overview-of-Harbor)

2. [Docker源码分析之容器日志处理与log-driver实现](https://segmentfault.com/a/1190000008166067)

3. [Docker 生产环境之日志 - 配置日志驱动程序](https://blog.csdn.net/kikajack/article/details/79575286)

4. [docker log](https://www.jianshu.com/p/c12622a9f4c1)

5. [Centos7.2下针对LDAP的完整部署记录](https://cloud.tencent.com/developer/article/1026304)

6. [](https://blog.csdn.net/ztq157677114/article/details/50538176)

<br />
<br />
<br />

