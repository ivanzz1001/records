---
layout: post
title: Harbor docker-compose文件分析
tags:
- docker
categories: docker
description: Harbor docker-compose文件分析
---


本节我们主要分析一下Harbor中docker-compose.yml文件，以了解Harbor整个系统的一个整体架构及工作状况。



<!-- more -->

## 1. docker-compose.yml文件
{% highlight string %}
version: '2'
services:
  log:
    image: vmware/harbor-log:v1.4.0
    container_name: harbor-log 
    restart: always
    volumes:
      - /var/log/harbor/:/var/log/docker/:z
      - ./common/config/log/:/etc/logrotate.d/:z
    ports:
      - 127.0.0.1:1514:10514
    networks:
      - harbor
  registry:
    image: vmware/registry-photon:v2.6.2-v1.4.0
    container_name: registry
    restart: always
    volumes:
      - /data/registry:/storage:z
      - ./common/config/registry/:/etc/registry/:z
    networks:
      - harbor
    environment:
      - GODEBUG=netdns=cgo
    command:
      ["serve", "/etc/registry/config.yml"]
    depends_on:
      - log
    logging:
      driver: "syslog"
      options:  
        syslog-address: "tcp://127.0.0.1:1514"
        tag: "registry"
  mysql:
    image: vmware/harbor-db:v1.4.0
    container_name: harbor-db
    restart: always
    volumes:
      - /data/database:/var/lib/mysql:z
    networks:
      - harbor
    env_file:
      - ./common/config/db/env
    depends_on:
      - log
    logging:
      driver: "syslog"
      options:  
        syslog-address: "tcp://127.0.0.1:1514"
        tag: "mysql"
  adminserver:
    image: vmware/harbor-adminserver:v1.4.0
    container_name: harbor-adminserver
    env_file:
      - ./common/config/adminserver/env
    restart: always
    volumes:
      - /data/config/:/etc/adminserver/config/:z
      - /data/secretkey:/etc/adminserver/key:z
      - /data/:/data/:z
    networks:
      - harbor
    depends_on:
      - log
    logging:
      driver: "syslog"
      options:  
        syslog-address: "tcp://127.0.0.1:1514"
        tag: "adminserver"
  ui:
    image: vmware/harbor-ui:v1.4.0
    container_name: harbor-ui
    env_file:
      - ./common/config/ui/env
    restart: always
    volumes:
      - ./common/config/ui/app.conf:/etc/ui/app.conf:z
      - ./common/config/ui/private_key.pem:/etc/ui/private_key.pem:z
      - ./common/config/ui/certificates/:/etc/ui/certificates/:z
      - /data/secretkey:/etc/ui/key:z
      - /data/ca_download/:/etc/ui/ca/:z
      - /data/psc/:/etc/ui/token/:z
    networks:
      - harbor
    depends_on:
      - log
      - adminserver
      - registry
    logging:
      driver: "syslog"
      options:  
        syslog-address: "tcp://127.0.0.1:1514"
        tag: "ui"
  jobservice:
    image: vmware/harbor-jobservice:v1.4.0
    container_name: harbor-jobservice
    env_file:
      - ./common/config/jobservice/env
    restart: always
    volumes:
      - /data/job_logs:/var/log/jobs:z
      - ./common/config/jobservice/app.conf:/etc/jobservice/app.conf:z
      - /data/secretkey:/etc/jobservice/key:z
    networks:
      - harbor
    depends_on:
      - ui
      - adminserver
    logging:
      driver: "syslog"
      options:  
        syslog-address: "tcp://127.0.0.1:1514"
        tag: "jobservice"
  proxy:
    image: vmware/nginx-photon:v1.4.0
    container_name: nginx
    restart: always
    volumes:
      - ./common/config/nginx:/etc/nginx:z
    networks:
      - harbor
    ports:
      - 80:80
      - 443:443
      - 4443:4443
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
networks:
  harbor:
    external: false
{% endhighlight %}

这里我们再给出Harbor的整个架构图：

![docker-harbor-arch](https://ivanzz1001.github.io/records/assets/img/docker/docker_harbor_arch.png)


上面各组件之间构建了一个内部网络```harbor```, 相互之间通过harbor网络进行通信。各组件之间的依赖关系如下：


![harbor-components-reply](https://ivanzz1001.github.io/records/assets/img/docker/harbor_components_reply.jpg)

上面如果admin server要将相应的配置存放到数据库，其也需要依赖于mysql。

另外还有两个地方需要注意：

1） admin server、ui、job service之间交互会共用一个公共的secretkey, 其存放在/data/secretkey文件中

2) ui与registry之间，采用数字签名的方式来进行认证。ui采用private_key.pem对token进行数字签名，在registry后续接收到token后采用root.crt来进行校验。



## 2. 手动启动Harbor个组件
这里先首先停止Harbor所有组件，删除所有已有数据：
<pre>
# cd /opt/harbor-inst/harbor/ && ls
bakup-harbor.cfg  docker-compose.clair.yml   docker-compose.yml  harbor.cfg            install.sh  NOTICE
common            docker-compose.notary.yml  ha                  harbor.v1.4.0.tar.gz  LICENSE     prepare

# docker-compose down -v
# ls /data/*
# rm -rf /data/database
# rm -rf /data/registry
# rm -rf /data/*
# rm -rf /var/log/harbor*
# rm -rf common/config/

# docker ps
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES

</pre>

下面我们重新产生配置文件：
<pre>
# ./prepare 
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

# ls common/config/
adminserver  db  jobservice  log  nginx  registry  ui
# ls /data/
secretkey
</pre>

如下是启动Harbor时所需要的一些镜像：
{% highlight string %}
# docker images | grep vmware
vmware/clair-photon           v2.0.1-v1.4.0       a1df3526fe43        2 months ago        300MB
vmware/notary-server-photon   v0.5.1-v1.4.0       3edfddb8ece2        2 months ago        211MB
vmware/notary-signer-photon   v0.5.1-v1.4.0       cc70a05cdb6a        2 months ago        209MB
vmware/registry-photon        v2.6.2-v1.4.0       8920f621ddd1        2 months ago        198MB
vmware/nginx-photon           v1.4.0              20c8a01ac6ab        2 months ago        135MB
vmware/harbor-log             v1.4.0              9e818c7a27ab        2 months ago        200MB
vmware/harbor-jobservice      v1.4.0              29c14d91b043        2 months ago        191MB
vmware/harbor-ui              v1.4.0              6cb4318eda6a        2 months ago        210MB
vmware/harbor-adminserver     v1.4.0              8145970fa013        2 months ago        182MB
vmware/harbor-db              v1.4.0              c38da34727f0        2 months ago        521MB
vmware/mariadb-photon         v1.4.0              8457013cf6e3        2 months ago        521MB
vmware/postgresql-photon      v1.4.0              59aa61520094        2 months ago        221MB
vmware/harbor-db-migrator     1.4                 7a4d871b612e        3 months ago        1.15GB
vmware/photon                 1.0                 9b411d78ad9e        3 months ago        130MB
{% endhighlight %}

如下在Harbor主目录下执行。

### 2.1 启动log组件
{% highlight string %}
# docker run -itd --name harbor-log \
 -v /var/log/harbor/:/var/log/docker/:z -v /opt/harbor-inst/harbor/common/config/log/:/etc/logrotate.d/:z \
 -p 1514:10514 vmware/harbor-log:v1.4.0
a6737c82dd1fa14c848b4a4d77b367485a55c78e34dd644b0b9be85e2b1c1ea7

# docker ps
CONTAINER ID        IMAGE                      COMMAND                  CREATED             STATUS                            PORTS                     NAMES
9292c4ac73b6        vmware/harbor-log:v1.4.0   "/bin/sh -c /usr/loc…"   3 seconds ago       Up 2 seconds (health: starting)   0.0.0.0:1514->10514/tcp   harbor-log

# docker inspect harbor-log | grep IPAddress
            "SecondaryIPAddresses": null,
            "IPAddress": "172.17.0.2",
                    "IPAddress": "172.17.0.2", 

# docker exec -it harbor-log /bin/sh
sh-4.3# ls
bin   dev  home  lib64  mnt   root  sbin  sys  usr
boot  etc  lib   media  proc  run   srv   tmp  var
sh-4.3# cd /etc/logrotate.d
sh-4.3# ls
logrotate.conf

# ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:ac:11:00:02  
          inet addr:172.17.0.2  Bcast:172.17.255.255  Mask:255.255.0.0
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:8 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0 
          RX bytes:648 (648.0 B)  TX bytes:0 (0.0 B)

lo        Link encap:Local Loopback  
          inet addr:127.0.0.1  Mask:255.0.0.0
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1 
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)
{% endhighlight %}
如上，我们运行起harbor-log组件了。其内部IP地址为```172.17.0.2```。后续我们可以通过如下命令来停止并删除harbor-log容器：
<pre>
# docker rm -vf harbor-log
</pre>

### 2.2 启动mysql
{% highlight string %}
# docker run -itd --name harbor-db \
 -v /data/database:/var/lib/mysql:z \
 --env-file /opt/harbor-inst/harbor/common/config/db/env \
 --log-driver syslog --log-opt syslog-address=tcp://127.0.0.1:1514 --log-opt tag=mysql\
 vmware/harbor-db:v1.4.0
a28716bfb7a547222c2f7918ed87a23bf51d9a155c973357548a72d9165772a3

# docker ps 
CONTAINER ID        IMAGE                          COMMAND                  CREATED              STATUS                        PORTS                            NAMES
fe15b507a2f6        vmware/harbor-db:v1.4.0        "/usr/local/bin/dock…"   About a minute ago   Up 59 seconds (healthy)       3306/tcp                         harbor-db
80fe63bdbe34        vmware/harbor-log:v1.4.0       "/bin/sh -c /usr/loc…"   About a minute ago   Up About a minute (healthy)   0.0.0.0:1514->10514/tcp          harbor-log

# docker inspect harbor-db | grep IPAddress
            "SecondaryIPAddresses": null,
            "IPAddress": "172.17.0.3",
                    "IPAddress": "172.17.0.3",
{% endhighlight %}
如上所示，mysql已经正常启动起来了。

### 2.3 启动admin server
{% highlight string %}
# docker run -itd --name harbor-adminserver \
 -v /data/config/:/etc/adminserver/config/:z \
 -v /data/secretkey:/etc/adminserver/key:z \
 -v /data/:/data/:z \
 --env-file /opt/harbor-inst/harbor/common/config/adminserver/env \
 --log-driver syslog --log-opt syslog-address=tcp://127.0.0.1:1514 --log-opt tag=adminserver\
 --add-host mysql:172.17.0.3 \
 vmware/harbor-adminserver:v1.4.0

# docker ps
CONTAINER ID        IMAGE                              COMMAND                  CREATED             STATUS                    PORTS                     NAMES
9e4044e9d73f        vmware/harbor-adminserver:v1.4.0   "/harbor/start.sh"       3 minutes ago       Up 3 minutes (healthy)                              harbor-adminserver
a28716bfb7a5        vmware/harbor-db:v1.4.0            "/usr/local/bin/dock…"   14 minutes ago      Up 14 minutes (healthy)   3306/tcp                  harbor-db
7c9ad816e6ab        vmware/harbor-log:v1.4.0           "/bin/sh -c /usr/loc…"   14 minutes ago      Up 14 minutes (healthy)   0.0.0.0:1514->10514/tcp   harbor-log

# docker inspect harbor-adminserver | grep IPAddress
            "SecondaryIPAddresses": null,
            "IPAddress": "172.17.0.4",
                    "IPAddress": "172.17.0.4",
{% endhighlight %}
注意上面需要添加```--add-host```选项来设置mysql的地址。

##  2.4 启动Registry
{% highlight string %}
# docker run -itd --name harbor-registry \
 -v /data/registry:/storage:z -v /opt/harbor-inst/harbor/common/config/registry/:/etc/registry/:z \
 -e "GODEBUG=netdns=cgo" \
 --log-driver syslog --log-opt syslog-address=tcp://127.0.0.1:1514 --log-opt tag=registry\
 vmware/registry-photon:v2.6.2-v1.4.0 /bin/registry serve /etc/registry/config.yml
{% endhighlight %}

### 2.5 启动ui
{% highlight string %}
# docker run -itd --name harbor-ui \
 -v /opt/harbor-inst/harbor/common/config/ui/app.conf:/etc/ui/app.conf:z \
 -v /opt/harbor-inst/harbor/common/config/ui/private_key.pem:/etc/ui/private_key.pem:z \
 -v /opt/harbor-inst/harbor/common/config/ui/certificates/:/etc/ui/certificates/:z \
 -v /data/secretkey:/etc/ui/key:z \
 -v /data/ca_download/:/etc/ui/ca/:z \
 -v /data/psc/:/etc/ui/token/:z \
 --add-host mysql:172.17.0.3 \
 --add-host adminserver:172.17.0.4 \
 --env-file /opt/harbor-inst/harbor/common/config/ui/env \
 --log-driver syslog --log-opt syslog-address=tcp://127.0.0.1:1514 --log-opt tag=ui\
 vmware/harbor-ui:v1.4.0

# docker ps
CONTAINER ID        IMAGE                              COMMAND                  CREATED              STATUS                                 PORTS                     NAMES
839d48d6c389        vmware/harbor-ui:v1.4.0            "/harbor/start.sh"       About a minute ago   Up About a minute (health: starting)                             harbor-ui
9e4044e9d73f        vmware/harbor-adminserver:v1.4.0   "/harbor/start.sh"       27 minutes ago       Up 27 minutes (healthy)                                          harbor-adminserver
a28716bfb7a5        vmware/harbor-db:v1.4.0            "/usr/local/bin/dock…"   39 minutes ago       Up 39 minutes (healthy)                3306/tcp                  harbor-db
7c9ad816e6ab        vmware/harbor-log:v1.4.0           "/bin/sh -c /usr/loc…"   39 minutes ago       Up 39 minutes (healthy)                0.0.0.0:1514->10514/tcp   harbor-log
{% endhighlight %}
















<br />
<br />

**[参考]**

1. [harbor官网](https://github.com/vmware/harbor)

2. [vmware harbor](http://vmware.github.io/harbor/)

3. [habor documents](https://github.com/vmware/harbor/tree/master/docs)

<br />
<br />
<br />

