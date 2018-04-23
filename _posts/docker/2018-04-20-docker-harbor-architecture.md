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
假设Harbor被部署在```192.168.1.10```宿主机上。用户通过如下命令行发送一个登录Harbor的请求：
<pre>
docker login 192.168.1.10
</pre>

在用户输入所需要的```Credentials```之后，Docker Client会发送一个HTTP GET请求到```192.168.1.10/v2/```地址处，Harbor的不同容器组件将会按照如下步骤进行处理：

![harbor-doker-login](https://ivanzz1001.github.io/records/assets/img/docker/harbor_docker_login.png)

(a) 首先，该请求将会被监听在80端口上的代理容器所接收到。容器中的Nginx将会把该请求转发给后端的Registry容器

(b) 由于registry容器已经被配置为基于```token```的认证，因此其会返回一个401错误码，用于通知docker客户端从一个指定的URL处获得一个有效的token。在Harbor中，该URL会指向Core service中的token service。

(c) 当Docker Client接收到这个错误码，其就会发送一个请求到token service URL，会根据HTTP基本认证协议在请求头中内嵌username和password相关信息

(d) 在该请求被发送到代理的80端口上后，Nginx会根据预先所配置的规则将请求转发到UI容器上。UI容器中的token service接收到该请求之后，其就会对该请求进行解码然后获得相应的用户名及密码

(e) 在成功获得用户名及密码之后，token Service就会检查mysql数据库以完成用户的认证。当token service被配置为LDAP/AD认证的时候，其就会通过外部的LDAP/AD服务来完成认证。在成功认证之后，token Service就会返回一个认证成功的http code， Http body部分会返回一个通过private key所产生的token


到这里为止，Docker login就处理完成。Docker client会将步骤(c)所产生的username及password编码后保存到一个隐藏的文件中

## 4. docker push处理流程

![harbor-doker-push](https://ivanzz1001.github.io/records/assets/img/docker/harbor_docker_push.png)


(注： 上述我们已经省略了Nginx代理转发的步骤。上图展示了docker push过程中不同组件之间的交互流程）

在用户成功登录之后，就会通过Docker push命令向Harbor发送一个Docker Image:
<pre>
# docker push 192.168.1.10/library/hello-world
</pre>

(a) 首先，docker client执行类似登录时的流程发送一个请求到registry，然后返回一个token service的URL

(b) 然后，docker client通过提供一些额外的信息与ui/token交互以获得push镜像library/hello-world的token

(c) 在成功获得来自Nginx转发的请求之后，Token Service查询数据库以寻找用户推送镜像的```角色```及```权限```。假如用户有相应的权限，token service就会编码相应的push操作信息，并用一个private key进行签名。然后返回一个token给Docker client

(d) 在docker client获得token之后，其就会发送一个push请求到registry，在该push请求的头中包含有上面返回的token信息。一旦registry收到了该请求，其就会使用public key来解码该token，然后再校验其内容。该public key对应于token service处的private key。假如registry发现该token有效，则会开启镜像的传输流程。





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

