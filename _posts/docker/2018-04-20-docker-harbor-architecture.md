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




<br />
<br />

**[参考]**

1. [Architecture Overview of Harbor](https://github.com/vmware/harbor/wiki/Architecture-Overview-of-Harbor)

<br />
<br />
<br />

