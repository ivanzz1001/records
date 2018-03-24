---
layout: post
title: 让docker registry支持https
tags:
- docker
categories: docker
description: 让docker registry支持https
---


本文我们主要讲述一下让docker registry支持https。这里我们用两台机器：

* docker-registry部署机器： 10.17.153.196

* docker拉取镜像测试机器： 192.168.69.128

<!-- more -->


## 1. docker registry支持https(直接IP地址方式）

### 1.1 生成自签名证书

在部署```docker registry```的机器上产生自签名证书。

1) **修改openss.cnf**

因为这里我们支持通过IP地址方式访问，因此，这里需要先修改openss的配置文件： ```/etc/pki/tls/openssl.cnf```，在```[ v3_ca ]```段中添加subjectAltName选项：
<pre>
[ v3_ca ] 

subjectAltName = 10.17.153.196
</pre>

2) **产生自签名证书**
<pre>
# mkdir /opt/docker-certs
# cd /opt
# openssl req -newkey rsa:2048 -nodes -sha256 -keyout docker-certs/rsa_private.key -x509 -days 365 -out docker-certs/cert.crt -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=10.17.153.196/emailAddress=11111111@qq.com"
</pre>

### 1.2 修改/opt/registry-manager.sh脚本
参看上一篇[docker环境搭建]，我们修改registry-manager.sh如下：
{% highlight string %}
#!/bin/sh

registry_name=Test-registry


function docker_registry_start {
    registry_container=`docker ps -a | grep $registry_name`
    
    if [ -z "$registry_container" ]
    then
/usr/bin/docker run -d -p 5000:5000 --privileged=true --restart=always \
 -v /opt/docker-registry:/var/lib/registry \
 -v /opt/docker-auth:/auth \
 -v /opt/docker-certs:/certs \
 -e "REGISTRY_AUTH=htpasswd" \
 -e "REGISTRY_AUTH_HTPASSWD_REALM=Registry Realm" \
 -e REGISTRY_AUTH_HTPASSWD_PATH=/auth/htpasswd \
 -e REGISTRY_HTTP_TLS_CERTIFICATE=/certs/cert.crt \
 -e REGISTRY_HTTP_TLS_KEY=/certs/rsa_private.key \
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

重新启动```docker registry```:
<pre>
# ./registry-manager.sh remove
# ./registry-manager.sh start

# docker ps
CONTAINER ID        IMAGE               COMMAND                  CREATED              STATUS              PORTS                    NAMES
c472e954f01b        registry:2.6.2      "/entrypoint.sh /etc…"   About a minute ago   Up About a minute   0.0.0.0:5000->5000/tcp   Test-registry
</pre>

### 1.3 配置docker

这里我们配置```192.168.69.128```这台主机。我们需要将上面产生的```cert.crt```证书拷贝到docker的默认cert目录： ```/etc/docker/certs.d/<regisry_domain:port>/```或者```/etc/docker/certs.d/<registry_ip:port>```。
<pre>
// 创建相应目录
# mkdir -p /etc/docker/certs.d/10.17.153.196:5000
# scp root@10.17.153.196:/opt/docker-certs/cert.crt /etc/docker/certs.d/10.17.153.196:5000/ca.crt
</pre>
注意这里需要把名称改为```ca.crt```。
<pre>
# docker login 10.17.153.196:5000
Username (admin): admin
Password: 
Login Succeeded
</pre>



**```如下操作不是必须:```**

由于```cert.crt```是自签名证书，要让操作系统信任的话，我们必须要把我们的证书放入操作系统的CA bundle文件中，使操作系统信任我们的自签名证书：

* 对于Centos6/7或者Redhat操作系统，bundle文件的位置在````/etc/pki/tls/certs/ca-bundle.crt```
<pre>
# cat cert.crt >> /etc/pki/tls/certs/ca-bundle.crt
</pre>

* 对于ubuntu或者debian操作系统，bundle文件的位置在```/etc/ssl/certs/ca-certificates.crt```
<pre>
# cat cert.crt >> /etc/ssl/certs/ca-certificates.crt
</pre>

```注意```: 如果之前已经有cat过同样的IP, 需要到ca-bundle.crt中把它删除，再做cat操作。否则后面PUSH时会报：
{% highlight string %}
Get https://129.144.150.111:5000/v1/_ping:x509: certificate signed by unknown authority
{% endhighlight %}
另外，在更改上面bundle文件时，尽量做好备份，以做恢复。


## 2. docker registry支持https(域名方式）





<br />
<br />

**[参看]:**

1. [搭建一个支持HTTPS的私有DOCKER Registry](https://blog.csdn.net/xcjing/article/details/70238273)

2. [docker registry v2 ssl 环境搭建](https://blog.csdn.net/wanglei_storage/article/details/53126690)

3. [docker registry_v2 部署过程中遇到的坑](https://blog.csdn.net/xiaolummhae/article/details/51833354)
<br />
<br />
<br />

