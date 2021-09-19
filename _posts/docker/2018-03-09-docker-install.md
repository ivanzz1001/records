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



## 2. docker启动配置

**1） 设置docker开机自动启动**
<pre>
# systemctl enable docker
</pre>

**2） 配置开放管理端口映射**

管理端口在 /lib/systemd/system/docker.service 文件中：
{% highlight string %}
[Unit]
Description=Docker Application Container Engine
Documentation=https://docs.docker.com
After=network-online.target firewalld.service
Wants=network-online.target

[Service]
Type=notify
# the default is not to use systemd for cgroups because the delegate issues still
# exists and systemd currently does not support the cgroup feature set required
# for containers run by docker
ExecStart=/usr/bin/dockerd
ExecReload=/bin/kill -s HUP $MAINPID
# Having non-zero Limit*s causes performance problems due to accounting overhead
# in the kernel. We recommend using cgroups to do container-local accounting.
LimitNOFILE=infinity
LimitNPROC=infinity
LimitCORE=infinity
# Uncomment TasksMax if your systemd version supports it.
# Only systemd 226 and above support this version.
#TasksMax=infinity
TimeoutStartSec=0
# set delegate yes so that systemd does not reset the cgroups of docker containers
Delegate=yes
# kill only the docker process, not all processes in the cgroup
KillMode=process
# restart the docker process if it exits prematurely
Restart=on-failure
StartLimitBurst=3
StartLimitInterval=60s

[Install]
WantedBy=multi-user.target
{% endhighlight %}
将上面的```ExecStart=/usr/bin/dockerd``` 替换为：
<pre>
ExecStart=/usr/bin/dockerd -H tcp://0.0.0.0:2375 -H unix://var/run/docker.sock -H tcp://0.0.0.0:7654
</pre>
（此处默认2375为主管理端口，unix:///var/run/docker.sock用于本地管理，7654是备用的端口）
    
配置完成之后，重新启动：
<pre>
# systemctl daemon-reload && systemctl restart docker

//查看docker进程，发现docker守护进程在已经监听2375的tcp端口
# ps -ef | grep docker
root      9431     1  0 20:30 ?        00:00:00 /usr/bin/dockerd -H tcp://0.0.0.0:2375 -H unix://var/run/docker.sock -H tcp://0.0.0.0:7654
root      9438  9431  0 20:30 ?        00:00:00 docker-containerd --config /var/run/docker/containerd/containerd.toml


//查看系统的网络端口，发现tcp的2375端口，的确是docker的守护进程在监听
# netstat -nlp | grep docker
tcp6       0      0 :::7654                 :::*                    LISTEN      9431/dockerd        
tcp6       0      0 :::2375                 :::*                    LISTEN      9431/dockerd        
unix  2      [ ACC ]     STREAM     LISTENING     30464    9431/dockerd         /run/docker/libnetwork/a83e1600984942bbacfc8587ff54aee0625550bb918af4a05ed26ba9a1eed24d.sock
unix  2      [ ACC ]     STREAM     LISTENING     31763    9431/dockerd         var/run/docker.sock
unix  2      [ ACC ]     STREAM     LISTENING     31770    9431/dockerd         /var/run/docker/metrics.sock
unix  2      [ ACC ]     STREAM     LISTENING     30455    9438/docker-contain  /var/run/docker/containerd/docker-containerd-debug.sock
unix  2      [ ACC ]     STREAM     LISTENING     30457    9438/docker-contain  /var/run/docker/containerd/docker-containerd.sock
</pre>



## 3. 搭建私有镜像仓库

我们当前私有镜像仓库搭建在：```10.17.153.196```。 

**1) 下载私有镜像仓库**

可以到如下两个地址：
<pre>
1. https://hub.docker.com/explore/
2. https://store.docker.com/
</pre>
去看一下当前官方仓库镜像。通过如下命令下载：
<pre>
# docker search registry
NAME                                    DESCRIPTION                                     STARS               OFFICIAL            AUTOMATED
registry                                The Docker Registry 2.0 implementation for s…   1903                [OK]                
konradkleine/docker-registry-frontend   Browse and modify your Docker registry in a …   181                                     [OK]
hyper/docker-registry-web               Web UI, authentication service and event rec…   126                                     [OK]
atcol/docker-registry-ui                A web UI for easy private/local Docker Regis…   98                                      [OK]
distribution/registry                   WARNING: NOT the registry official image!!! …   54                                      [OK]
marvambass/nginx-registry-proxy         Docker Registry Reverse Proxy with Basic Aut…   43                                      [OK]
google/docker-registry                  Docker Registry w/ Google Cloud Storage driv…   32                                      
jhipster/jhipster-registry              JHipster Registry, based on Netflix Eureka a…   22                                      [OK]
confluentinc/cp-schema-registry         Official Confluent Docker Images for Schema …   16                                      
deis/registry                           Docker image registry for the Deis open sour…   12                                      
openshift/origin-docker-registry        The integrated OpenShift V3 registry            8                                       
joxit/docker-registry-ui                Docker registry v2 web User Interface           6                                       [OK]
landoop/schema-registry-ui              UI for Confluent's Schema Registry              5                                       [OK]
cblomart/rpi-registry                   docker registry 2 for raspbery pi               5                                       
elasticio/docker-registry-ecs           Docker image to run Docker private registry …   4                                       [OK]
allingeek/registry                      A specialization of registry:2 configured fo…   4                                       [OK]
pallet/registry-swift                   Add swift storage support to the official do…   4                                       [OK]
aibaars/docker-registry2-gcs            Docker Registry2 w/ Google Cloud Storage dri…   1                                       
webhippie/registry                      Docker images for registry                      1                                       [OK]
conjurinc/registry-oauth-server         Docker registry authn/authz server backed by…   1                                       
metadata/registry                       Metadata Registry is a tool which helps you …   1                                       [OK]
convox/registry                                                                         0                                       
kontena/registry                        Kontena Registry                                0                                       
lorieri/registry-ceph                   Ceph Rados Gateway (and any other S3 compati…   0                                       
databus23/docker-registry-proxy         A ssl enabled nginx reverse proxy for the do…   0  

# docker pull registry
# docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
registry            latest              d1fd7d86a825        8 weeks ago         33.3MB

# docker tag registry registry:2.6.2      //因为当前registry最新版本是2.6.2，这里我们将其打上tag，以方便后续辨别
# docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
registry            2.6.2               d1fd7d86a825        8 weeks ago         33.3MB
registry            latest              d1fd7d86a825        8 weeks ago         33.3MB

# docker save -o registry-2.6.2.tar.gz registry:2.6.2       //打包

# ls 
registry-2.6.2.tar.gz
</pre>
然后将上述打包好的镜像仓库拷贝到对应的安装机器上。

<br />

**2) 安装registry镜像仓库**

<pre>
# docker load -i registry-2.6.2.tar.gz
# docker images
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
registry            2.6.2               3ebefe7c539b        5 months ago        33.2MB
</pre>

**3) 为镜像仓库挂载一块硬盘**

如下先将一块硬盘格式化，然后挂载到我们指定的目录
<pre>
# lsblk         
# mkfs.ext4 /dev/vdb
# mkdir /opt/docker-registry
# mount -t ext4 /dev/vdb /registry

# blkid
/dev/sr0: UUID="2018-03-09-10-24-55-00" LABEL="config-2" TYPE="iso9660" 
/dev/vda1: UUID="f31c1eba-0729-4c4a-959f-d1bb6a2623aa" TYPE="ext4" 
/dev/vdb: UUID="639cc105-d6a9-43af-8036-3462b450ea5c" TYPE="ext4"
</pre>
然后在操作系统/etc/fstab文件中添加如下：
<pre>
UUID=639cc105-d6a9-43af-8036-3462b450ea5c /opt/docker-registry    ext4    defaults        2 2
</pre>


**4) 设置开机自动启动私有镜像仓库**

编写docker-registry.service文件：
{% highlight string %}
[Unit]
Description=Docker Registry
After=network-online.target docker.service

[Service]
Type=oneshot
# ExecStart=/usr/bin/docker run -d -p 5000:5000 --privileged=true \
#       -v /opt/docker-registry:/var/lib/registry --name Test-registry registry:2.6.2
ExecStart=/opt/registry-manager.sh start
ExecReload=/opt/registry-manager.sh restart
ExecStop=/opt/registry-manager.sh stop
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
{% endhighlight %}
说明：

* ```-d```: 作为daemon进程启动，也就是后台启动

* ```-v```: 默认情况下，早期docker会将仓库存放于容器的/tmp/registry目录下，而当前版本docker存放的位置是/var/lib/registry,因此这里我们将我们的外部硬盘/opt/docker-registry挂在到/var/lib/registry目录下。

* ```-p```: 前一个5000是host的端口，后一个是容器的端口。这里将容器的5000端口映射到host的5000端口.

/opt/registry-manager.sh代码如下：
{% highlight string %}
#!/bin/sh

registry_name=Test-registry

function docker_registry_start {
    registry_container=`docker ps -a | grep $registry_name`
    
    if [ -z "$registry_container" ]
    then
       /usr/bin/docker run -d -p 5000:5000 --privileged=true  --restart=always -v /opt/docker-registry:/var/lib/registry --name $registry_name registry:2.6.2
    else
       /usr/bin/docker start $registry_name
    fi 
   
}

function docker_registry_stop {
    /usr/bin/docker stop $registry_name
}

if [ $# -ne 1 ]
then
   echo "invalid parameter number"
   exit 1
fi

if [ $1 = "start" ]
then
   docker_registry_start
elif [ $1 = "stop" ]
then 
    docker_registry_stop
elif [ $1 = "restart" ]
then
    docker_registry_stop
    docker_registry_start
else
    echo "unsupported command"
fi
{% endhighlight %}


接下来，将该文件拷贝到/lib/systemd/system/目录下，执行：
<pre>
# cp docker-registry.service /lib/systemd/system/
# systemctl daemon-reload
# systemctl enable docker-registry
# sudo journalctl -n 20                          //查看日志
# systemctl status docker-registry.service
</pre>

这里配置iptables，以允许5000端口：
<pre>
# iptables -I INPUT 1 -p tcp --dport 5000 -j ACCEPT

# iptables -L -n -v                                       //可以看到iptables添加成功
Chain INPUT (policy ACCEPT 43 packets, 3432 bytes)
 pkts bytes target     prot opt in     out     source               destination         
    0     0 ACCEPT     tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp dpt:5000

//这里iptables配置保存有些问题(service iptables save)，需要安装iptables-services
# sudo yum install --downloadonly --downloaddir=./ iptables-services
# ls
iptables-1.4.21-18.3.el7_4.x86_64.rpm           iptables-services-1.4.21-18.3.el7_4.x86_64.rpm  

# yum localinstall *.rpm

# service iptables save

# systemctl is-active iptables
inactive
# systemctl unmask iptables
Removed symlink /etc/systemd/system/iptables.service.
# systemctl enable iptables
Created symlink from /etc/systemd/system/basic.target.wants/iptables.service to /usr/lib/systemd/system/iptables.service.
</pre>



## 3. 测试

**1) push镜像到私有镜像仓库**

接下来，我们就要操作把一个本地镜像push到私有仓库中。在内网的任意一台装有docker机器下pull一个比较小的镜像来测试（此处使用busybox):
<pre>
# docker pull busybox
</pre>

然后再修改一下该镜像的tag:
<pre>
//此处前缀为我们的私有镜像库地址
# docker tag busybox 10.17.153.196:5000/busybox   

# docker images
REPOSITORY                   TAG                 IMAGE ID            CREATED             SIZE
10.17.153.196:5000/busybox   latest              f6e427c148a7        9 days ago          1.15MB
busybox                      latest              f6e427c148a7        9 days ago          1.15MB         
</pre>

接着再push到我们的私有镜像仓库：
<pre>
# docker push 10.17.153.196:5000/busybox
The push refers to repository [10.17.153.196:5000/busybox]
Get https://10.17.153.196:5000/v2/: http: server gave HTTP response to HTTPS client
</pre>
上面我们看到，出现了错误，提示说： 服务端给了一个http响应到客户端。

<br />

**2） 配置docker以支持http方式push**

之所以出现上面的错误提示，是因为docker从1.3.x之后，与docker registry交互默认使用的是https，然而此处搭建的私有仓库只提供http服务，所以当与私有仓库交互时就会报上面的错误。

这里修改docker的启动配置文件```/lib/systemd/system/docker.service```，在其中添加:
<pre>
--insecure-registry=10.17.153.196:5000
</pre>
修改后如下所示：
{% highlight string %}
[Unit]
Description=Docker Application Container Engine
Documentation=https://docs.docker.com
After=network-online.target firewalld.service
Wants=network-online.target

[Service]
Type=notify
# the default is not to use systemd for cgroups because the delegate issues still
# exists and systemd currently does not support the cgroup feature set required
# for containers run by docker
ExecStart=/usr/bin/dockerd \
		--insecure-registry=10.17.153.196:5000 \
		-H tcp://0.0.0.0:2375 \ 
		-H unix://var/run/docker.sock \ 
		-H tcp://0.0.0.0:7654

ExecReload=/bin/kill -s HUP $MAINPID

# Having non-zero Limit*s causes performance problems due to accounting overhead
# in the kernel. We recommend using cgroups to do container-local accounting.
LimitNOFILE=infinity
LimitNPROC=infinity
LimitCORE=infinity
# Uncomment TasksMax if your systemd version supports it.
# Only systemd 226 and above support this version.
#TasksMax=infinity
TimeoutStartSec=0
# set delegate yes so that systemd does not reset the cgroups of docker containers
Delegate=yes
# kill only the docker process, not all processes in the cgroup
KillMode=process
# restart the docker process if it exits prematurely
Restart=on-failure
StartLimitBurst=3
StartLimitInterval=60s

[Install]
WantedBy=multi-user.target
{% endhighlight %}
修改完成后(注意上面ExecStart分多行写时，前面为tab键)，重启docker，然后再push镜像：
<pre>
# systemctl daemon-reload && systemctl restart docker

# docker push 10.17.153.196:5000/busybox
The push refers to repository [10.17.153.196:5000/busybox]
c5183829c43c: Pushed 
latest: digest: sha256:c7b0a24019b0e6eda714ec0fa137ad42bc44a754d9cea17d14fba3a80ccc1ee4 size: 527
</pre>

**3） 查看我们push到私有镜像仓库中的镜像**

登录到私有镜像仓库服务器，查看我们push上去的busybox镜像:
<pre>
# docker exec -it MyRegistry /bin/sh

# find / -name busybox
/bin/busybox
/var/lib/registry/docker/registry/v2/repositories/busybox
</pre>
上面我们看到，我们push上去的busybox镜像存放在/var/lib/registry目录下。

通过curl命令查看当前镜像仓库中的镜像：
<pre>
# curl -XGET http://10.17.153.196:5000/v2/_catalog  
{"repositories":["busybox"]}

# curl -XGET http://10.17.153.196:5000/v2/busybox/tags/list 
{"name":"busybox","tags":["latest"]}
</pre>


## 4. 配置仓库认证

私有仓库搭建以后其他所有客户端均可以push、pull，docker官方提供认证方法对docker仓库进行权限保护.我们这里只添加用户权限限制：

**1) 删除原来启动的仓库容器**
{% highlight string %}
# docker ps -a
CONTAINER ID        IMAGE                            COMMAND                  CREATED             STATUS                         PORTS                    NAMES
cba959ec5852        registry:2.6.2                   "/entrypoint.sh /etc…"   43 seconds ago      Up 42 seconds                  0.0.0.0:5000->5000/tcp   Test-registry

# docker stop Test-registry
# docker rm Test-registry
{% endhighlight %}

**2) 创建存放账号的文件**
{% highlight string %}
# mkdir /opt/docker-auth

//这里添加一个admin账号，密码设置为123
# docker run --entrypoint htpasswd registry:2.6.2 -Bbn admin 123 >> /opt/docker-auth/htpasswd
# cat /opt/docker-auth/htpasswd
admin:$2y$05$AWp/QM9DPRvOgwv9iNjxlO3juhHqj/ANzmxsPvV3nsQBZpcy1x62C
{% endhighlight %}

**3) 修改docker registry的启动参数，并启动**

这里我们修改上文的registry-manager.sh:
{% highlight string %}
# cat registry-manager.sh 
#!/bin/sh

registry_name=test-registry


function docker_registry_start {
    registry_container=`docker ps -a | grep $registry_name`
    
    if [ -z "$registry_container" ]
    then
/usr/bin/docker run -d -p 5000:5000 --privileged=true --restart=always \
 -v /opt/docker-registry:/var/lib/registry -v /opt/docker-auth:/auth \
 -e "REGISTRY_AUTH=htpasswd" \
 -e "REGISTRY_AUTH_HTPASSWD_REALM=Registry Realm" \
 -e REGISTRY_AUTH_HTPASSWD_PATH=/auth/htpasswd \
 --name $registry_name \
 registry:2.6.2
    
    else
       /usr/bin/docker start $registry_name
    fi 
   
}

function docker_registry_stop {
    /usr/bin/docker stop $registry_name
}

function docker_registry_remove {
    /usr/bin/docker stop $registry_name
    /usr/bin/docker rm $registry_name
}

if [ $# -ne 1 ]
then
   echo "invalid parameter number"
   exit 1
fi

if [ $1 = "start" ]
then
   docker_registry_start
elif [ $1 = "stop" ]
then 
    docker_registry_stop
elif [ $1 = "restart" ]
then
    docker_registry_stop
    docker_registry_start
elif [ $1 == "remove" ]
then
    docker_registry_remove
else
    echo "unsupported command"
fi
{% endhighlight %}

**4) 测试**
{% highlight string %}
# docker pull 10.17.153.196:5000/busybox
Using default tag: latest
Error response from daemon: Get http://10.17.153.196:5000/v2/busybox/manifests/latest: no basic auth credentials

# docker login http://10.17.153.196:5000 
Username: admin
Password: 
Login Succeeded

# docker pull  10.17.153.196:5000/busybox
Using default tag: latest
latest: Pulling from busybox
d070b8ef96fc: Pull complete 
Digest: sha256:c7b0a24019b0e6eda714ec0fa137ad42bc44a754d9cea17d14fba3a80ccc1ee4
Status: Downloaded newer image for 10.17.153.196:5000/busybox:latest
{% endhighlight %}




<br />
<br />

**[参看]:**

1. [Docker CE安装及配置国内镜像加速教程](http://blog.csdn.net/jackyzhousales/article/details/77995135)

2. [配置docker官方源并用yum安装docker](https://www.cnblogs.com/JiangLe/p/6921320.html)

3. [centos7安装docker并设置开机启动](https://www.cnblogs.com/rwxwsblog/p/5436445.html)

4. [CentOS7安装Docker全程并启动](http://blog.csdn.net/wangfei0904306/article/details/62046753)

5. [CentOS 7 : Docker私有仓库搭建和使用](http://blog.csdn.net/fgf00/article/details/52040492)

6. [CentOS环境下Docker私有仓库搭建](https://www.cnblogs.com/kangoroo/p/7994801.html)

7. [docker 私有镜像仓库搭建](http://blog.csdn.net/wu_di_xiao_wei/article/details/54755475)

8. [Docker搭建本地私有仓库](http://blog.csdn.net/ronnyjiang/article/details/71189392)

9. [centos7 Docker私有仓库搭建及删除镜像](https://www.cnblogs.com/Tempted/p/7768694.html)

9. [Centos7创建支持ssh服务器的docker容器](http://blog.csdn.net/xizaihui/article/details/52960604)

10. [[docker]privileged参数](http://blog.csdn.net/halcyonbaby/article/details/43499409)

11. [Docker之Centos7 Docker私有仓库搭建](http://blog.csdn.net/mmd0308/article/details/77162004)

12. [Docker学习笔记六：Docker搭建企业级私有仓库](https://www.cnblogs.com/sishang/p/6511420.html)

13. [Docker搭建带有访问认证的私有仓库](http://blog.csdn.net/yuhaitao8922/article/details/72996993)

14. [docker私有仓库搭建并且配置仓库认证](https://www.jianshu.com/p/7918c9af45a3)

15. [docker github官方源码](https://github.com/docker/docker-ce)
<br />
<br />
<br />

