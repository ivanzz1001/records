---
layout: post
title: 配置Harbor以支持https
tags:
- docker
categories: docker
description: 配置Harbor以支持https
---

因为Harbor发布时默认并不包含certificates，并使用http来提供registry请求服务。然而，我们建议在实际的生产环境中还是要使用安全的https。Harbor有一个Nginx实例以作为其他所有服务的反向代理。可以使用prepare脚本来配置Nginx以支持https。




<!-- more -->


## 1. 停止Harbor
上一章我们使用的是Harbor默认的http方式工作，这里我们首先将Harbor停止，并删除掉原有的一些数据，以恢复到一个干净的环境：
<pre>
# docker-compose down -v
# ls /data/*
# rm -rf /data/database
# rm -rf /data/registry
# rm -rf /data/*
# rm -rf /var/log/harbor*
# 
</pre>

注意，可能是因为目前Harbor工作负载过重，导致上面调用```docker-compose down -v```经常会失败，可用如下方式来停止：
{% highlight string %}
# docker ps | grep -v CONTAINER | grep -v docs | awk '{print $1'} | xargs docker stop
# docker ps | grep -v CONTAINER | grep -v docs | awk '{print $1'} | xargs docker rm -vf
{% endhighlight %}

## 2. 获取证书
这里假设你的registry主机名为```reg.yourdomain.com```，并且通过DNS记录能够找到你运行Harbor的主机。首先你应该从CA处获得一个certificate。该certificate通常包含一个```a.crt```文件和一个```a.key```文件，例如：```yourdomain.com.crt```以及```yourdomain.com.key```。

在测试或开发环境下，你也许会使用一个自签名证书，而不是从CA那里获取。可以通过如下的命令产生你自己的证书：

### 2.1 创建自签名根证书

可以通过如下的方式来产生一个```私钥```及```自签名证书```:
<pre>
# openssl req \
    -newkey rsa:4096 -nodes -sha256 -keyout ca.key \
    -x509 -days 365 -out ca.crt
</pre>

### 2.2 产生证书签名请求
假如你使用类似于```reg.yourdomain.com```的FQDN(Fully Qualified Domain Name)方式来连接registry主机，则你必须使用```reg.yourdomain.com```来作为CN(Common Name)。否则，假如你使用IP地址来连接你的registry主机的话，CN可以指定为任何值（例如指定为你的名字）：
<pre>
# openssl req \
    -newkey rsa:4096 -nodes -sha256 -keyout yourdomain.com.key \
    -out yourdomain.com.csr
</pre>

### 2.3 为registry主机产生证书
假如你使用类似于```reg.yourdomain.com```的FQDN(Full Qualified Domain Name)方式来连接registry主机，你可以使用如下的命令来为registry主机产生证书：
<pre>
# openssl x509 -req -days 365 -in yourdomain.com.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out yourdomain.com.crt
</pre>

假如你是使用ip的话， 比如使用```192.168.1.101```来连接registry主机的话，你需要使用如下命令：
{% highlight string %}
# echo subjectAltName = IP:192.168.1.101 > extfile.cnf

# openssl x509 -req -days 365 -in yourdomain.com.csr -CA ca.crt -CAkey ca.key -CAcreateserial -extfile extfile.cnf -out yourdomain.com
.crt
{% endhighlight %}

## 3. 配置与安装
在你获得```yourdomain.com.crt```与```yourdomain.com.key```文件之后，你可以将它们放到一个目录，比如```/root/cert/```:
<pre>
# cp yourdomain.com.crt /root/cert/
# cp yourdomain.com.key /root/cert/ 
</pre>

然后修改```harbor.cfg```配置文件，更新hostname及protocol，然后更新```ssl_cert```及```ssl_cert_key```：
<pre>
#set hostname
hostname = reg.yourdomain.com
#set ui_url_protocol
ui_url_protocol = https
......
#The path of cert and key files for nginx, they are applied only the protocol is set to https 
ssl_cert = /root/cert/yourdomain.com.crt
ssl_cert_key = /root/cert/yourdomain.com.key
</pre>
然后再执行如下命令为Harbor产生配置文件：
<pre>
 # ./prepare
</pre>


假如当前```Harbor```正在运行的话，停止并移除当前的运行实例。通过如下方式你的```image data```仍会保留在文件系统中：
<pre>
# docker-compose down
</pre>

最后，重启Harbor。

在设置了```https```之后，你可以通过如下的步骤来进行验证：

* 打开浏览器输入访问地址```https://reg.yourdomain.com```，就会显示出Harbor的 UI界面

* 在安装有docker daemon的机器上（请确保没有```--insecure-registry```设置)，你必须拷贝上述步骤所产生的```ca.crt```到```/etc/docker/certs.d/reg.yourdomain.com```目录（或者```registry host IP```目录)。假如该目录并不存在的话，请创建该目录。假如你将nginx 443端口映射到了其他端口的话，则你必须创建```/etc/docker/certs.d/reg.yourdomain.com:port```目录(或者```registry host IP:port```目录），然后运行相应的docker命令行来验证https是否工作正常：
<pre>
# docker login reg.yourdomain.com
</pre>
假如你已经将nginx 443端口映射到了其他端口的话，你需要在登录时添加相应的端口，例如：
<pre>
# docker login reg.yourdomain.com:port
</pre>

## 4. Troubleshooting

1) 你也许从一个certificate issuer处获得了一个intermediate certificate。在这种情况下，你可以将该intermediate certificate与你自己的certificate合并，创建出一个certificate bundle。你可以通过如下命令来实现：
{% highlight string %}
# cat intermediate-certificate.pem >> yourdomain.com.crt 
{% endhighlight %}

2) 在有一些docker daemon运行的操作系统上，你也许需要在操作系统级别信任该证书

* 在Ubuntu操作系统上，你可以通过如下命令来完成
<pre>
# cp youdomain.com.crt /usr/local/share/ca-certificates/reg.yourdomain.com.crt
# update-ca-certificates
</pre>

* 在Redhat(Centos等）操作系统上，你可以通过如下命令来完成
<pre>
# cp yourdomain.com.crt /etc/pki/ca-trust/source/anchors/reg.yourdomain.com.crt
# update-ca-trust
</pre>



## 5. 部署示例

我们当前部署环境ip地址为```192.168.69.128```

### 5.1 获得证书文件

**1) 产生根证书**
<pre>
# openssl req \
    -newkey rsa:4096 -nodes -sha256 -keyout ca.key \
    -x509 -days 365 -out ca.crt \
    -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=test/emailAddress=11111111@qq.com"

# ls
ca.crt  ca.key
</pre>

**2) 产生证书签名请求**
<pre>
# openssl req \
    -newkey rsa:4096 -nodes -sha256 -keyout harbor-registry.key \
    -out harbor-registry.csr \
    -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=192.168.69.128/emailAddress=11111111@qq.com"

# ls
ca.crt  ca.key  harbor-registry.csr  harbor-registry.key
</pre>

**3) 为registry产生证书**
{% highlight string %}
# echo subjectAltName = IP:192.168.69.128 > extfile.cnf

# openssl x509 -req -days 365 -in harbor-registry.csr -CA ca.crt -CAkey ca.key -CAcreateserial -extfile extfile.cnf -out harbor-registry.crt

# ls
ca.crt  ca.key  ca.srl  extfile.cnf  harbor-registry.crt  harbor-registry.csr  harbor-registry.key
{% endhighlight %}


### 5.2 配置及安装

1) 拷贝harbor-registry证书到```/opt/cert```目录
<pre>
# mkdir -p /opt/cert
# cp harbor-registry.crt /opt/cert/
# cp harbor-registry.key /opt/cert/
</pre>

2) 修改harbor.cfg配置文件
<pre>
#set hostname
hostname = 192.168.69.128
#set ui_url_protocol
ui_url_protocol = https
......
#The path of cert and key files for nginx, they are applied only the protocol is set to https 
ssl_cert = /opt/cert/harbor-registry.crt
ssl_cert_key = /opt/cert/harbor-registry.key
</pre>

3) 重新产生配置文件
<pre>
# ./prepare
</pre>

4) 关闭harbor
<pre>
# docker-compose down 
</pre>

5) 查看docker daemon是否有```--insecure-registry```选项

如果仍有该选项，请将其去掉，并执行如下命令重启docker daemon:
<pre>
# systemctl daemon-reload
# systemctl restart docker
</pre>

6) 重启Harbor
<pre>
# docker-compose up -d
Creating network "harbor_harbor" with the default driver
Creating harbor-log ... done
Creating registry           ... done
Creating harbor-adminserver ... done
Creating harbor-db          ... done
Creating harbor-ui          ... done
Creating harbor-jobservice  ... done
Creating nginx              ... done

# docker ps
CONTAINER ID        IMAGE                                  COMMAND                  CREATED             STATUS                            PORTS                                                              NAMES
c7b4d837fefc        vmware/nginx-photon:v1.4.0             "nginx -g 'daemon of…"   6 seconds ago       Up 3 seconds                      0.0.0.0:80->80/tcp, 0.0.0.0:443->443/tcp, 0.0.0.0:4443->4443/tcp   nginx
257ec984fc98        vmware/harbor-jobservice:v1.4.0        "/harbor/start.sh"       6 seconds ago       Up 4 seconds (health: starting)                                                                      harbor-jobservice
331fe98b1623        vmware/harbor-ui:v1.4.0                "/harbor/start.sh"       8 seconds ago       Up 5 seconds (health: starting)                                                                      harbor-ui
d155d8a3cf00        vmware/harbor-db:v1.4.0                "/usr/local/bin/dock…"   10 seconds ago      Up 7 seconds (health: starting)   3306/tcp                                                           harbor-db
183a8f508491        vmware/harbor-adminserver:v1.4.0       "/harbor/start.sh"       10 seconds ago      Up 7 seconds (health: starting)                                                                      harbor-adminserver
579642c3cecc        vmware/registry-photon:v2.6.2-v1.4.0   "/entrypoint.sh serv…"   10 seconds ago      Up 7 seconds (health: starting)   5000/tcp                                                           registry
06a1618f789e        vmware/harbor-log:v1.4.0               "/bin/sh -c /usr/loc…"   10 seconds ago      Up 9 seconds (health: starting)   127.0.0.1:1514->10514/tcp                                          harbor-log
</pre>

7) 通过https形式访问Harbor

* 通过浏览器访问

这里首先需要将上面产生的```ca.crt```导入到浏览器的```受信任的根证书```中。然后就可以通过https进行访问（这里经过测试，Chrome浏览器、IE浏览器可以正常访问，但360浏览器不能正常访问）

* 通过docker命令来访问

首先新建```/etc/docker/certs.d/192.168.69.128```目录，然后将上面产生的```ca.crt```拷贝到该目录:
<pre>
# mkdir -p /etc/docker/certs.d/192.168.69.128
# cp ca.crt /etc/docker/certs.d/192.168.69.128/
</pre>

然后登录到docker registry:
<pre>
# docker login 192.168.69.128
Username (admin): admin
Password: 
Login Succeeded
</pre>

用向```registry```中上传一个镜像：
<pre>
# docker images
192.168.69.128/library/redis   alpine              c27f56585938        3 weeks ago         27.7MB

[root@localhost test]# docker push 192.168.69.128/library/redis:alpine
The push refers to repository [192.168.69.128/library/redis]
f6b9463783dc: Pushed 
222a85888a99: Pushed 
1925395eabdd: Pushed 
c3d278563734: Pushed 
ad9247fe8c63: Pushed 
cd7100a72410: Pushed 
alpine: digest: sha256:9d017f829df3d0800f2a2582c710143767f6dda4df584b708260e73b1a1b6db3 size: 1568
</pre>

* 通过curl命令来访问 registry API版本号

查询registry API版本号：
<pre>
# curl -iL -X GET https://192.168.69.128/v2 --cacert ca.crt
HTTP/1.1 301 Moved Permanently
Server: nginx
Date: Tue, 10 Apr 2018 09:33:39 GMT
Content-Type: text/html
Content-Length: 178
Location: https://192.168.69.128/v2/
Connection: keep-alive

HTTP/1.1 401 Unauthorized
Server: nginx
Date: Tue, 10 Apr 2018 09:33:39 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 87
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=575f32ac760f52c8cf1cdb748e48ab5e; Path=/; HttpOnly
Www-Authenticate: Bearer realm="https://192.168.69.128/service/token",service="harbor-registry"

{"errors":[{"code":"UNAUTHORIZED","message":"authentication required","detail":null}]}

# curl -iL -X GET -u admin:Harbor12345 https://192.168.69.128/service/token?account=admin\&service=harbor-registry --cacert ca.crt
HTTP/1.1 200 OK
Server: nginx
Date: Tue, 10 Apr 2018 09:34:39 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 1100
Connection: keep-alive
Set-Cookie: beegosessionID=77bc62dcdc4a810a0e208487a89f069a; Path=/; HttpOnly

{
  "token": "nHLZqMPw",
  "expires_in": 1800,
  "issued_at": "2018-04-10T09:34:39Z"
}

# curl -iL -X GET -H "Content-Type: application/json" -H "Authorization: Bearer nHLZqMPw" https://192.168.69.128/v2 --cacert ca.crt
HTTP/1.1 301 Moved Permanently
Server: nginx
Date: Tue, 10 Apr 2018 09:36:48 GMT
Content-Type: text/html
Content-Length: 178
Location: https://192.168.69.128/v2/
Connection: keep-alive

HTTP/1.1 200 OK
Server: nginx
Date: Tue, 10 Apr 2018 09:36:48 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 2
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=e651b65d891617a999254ec875c1c63c; Path=/; HttpOnly
</pre>
上面为了显示，我们对返回过来的```token```做了适当的裁剪。此外这里```curl```命令不适用```-k```选项，表示需要对服务器证书进行检查。


* 通过curl来访问registry中的镜像列表
<pre>
# curl -iL -X GET -u admin:Harbor12345 https://192.168.69.128/service/token?account=admin\&service=harbor-registry\&scope=registry:catalog:* --cacert ca.crt
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:33:52 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 1166
Connection: keep-alive
Set-Cookie: beegosessionID=648fd5a5ec4f06389d45c02f7f5971b4; Path=/; HttpOnly

{
  "token": "A7yfEdUBYD3bDhLM",
  "expires_in": 1800,
  "issued_at": "2018-04-09T09:33:52Z"
}

# curl -iL -X GET -H "Content-Type: application/json" -H "Authorization: Bearer LA7yfEdUBYD3bDhLM" http://192.168.69.128/v2/_catalog --cacert ca.crt
HTTP/1.1 200 OK
Server: nginx
Date: Mon, 09 Apr 2018 09:36:35 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 34
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
Set-Cookie: beegosessionID=1b84e760ab0234045f06680e56e28818; Path=/; HttpOnly

{"repositories":["library/redis"]}
</pre>
上面为了显示，我们对返回过来的```token```做了适当的裁剪。此外这里```curl```命令不适用```-k```选项，表示需要对服务器证书进行检查。

<br />
<br />

**[参看]**

1. [Configuring Harbor with HTTPS Access](https://github.com/vmware/harbor/blob/master/docs/configure_https.md)



<br />
<br />
<br />

