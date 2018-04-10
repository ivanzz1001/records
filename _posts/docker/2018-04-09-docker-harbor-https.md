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
# rm -r /data/database
# rm -r /data/registry
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












<br />
<br />

**[参看]**

1. [Configuring Harbor with HTTPS Access](https://github.com/vmware/harbor/blob/master/docs/configure_https.md)



<br />
<br />
<br />

