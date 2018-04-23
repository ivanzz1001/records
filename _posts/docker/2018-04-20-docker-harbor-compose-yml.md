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








<br />
<br />

**[参考]**

1. [harbor官网](https://github.com/vmware/harbor)

2. [vmware harbor](http://vmware.github.io/harbor/)

3. [habor documents](https://github.com/vmware/harbor/tree/master/docs)

<br />
<br />
<br />

