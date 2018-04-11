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

subjectAltName = IP:10.17.153.196
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

* 对于Centos6/7或者Redhat操作系统，bundle文件的位置在```/etc/pki/tls/certs/ca-bundle.crt```
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



## 3. nginx https代理方式访问docker registry

如下我们配置通过nginx反向代理方式访问docker registry.

### 3.1 获取证书文件

**1) 产生根证书**
<pre>
# mkdir -p /opt/cert
# cd /opt/cert

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
    -subj "/C=CN/ST=Guangdong/L=Shenzhen/O=test_company/OU=IT/CN=10.17.153.196/emailAddress=11111111@qq.com"

# ls
ca.crt  ca.key  harbor-registry.csr  harbor-registry.key
</pre>

**3) 为registry产生证书**
{% highlight string %}
# echo subjectAltName = IP:10.17.153.196 > extfile.cnf

# openssl x509 -req -days 365 -in nginx-registry.csr -CA ca.crt -CAkey ca.key -CAcreateserial -extfile extfile.cnf -out harbor-registry.crt

# ls
ca.crt  ca.key  ca.srl  extfile.cnf  harbor-registry.crt  harbor-registry.csr  harbor-registry.key
{% endhighlight %}


### 3.2 拉取registry镜像
<pre>
# docker search registry
# docker pull registry
# docker tag registry registry:2.6.2
</pre>
这里我们拉取的registry最新镜像版本为```2.6.2```, 我们为其打上version标签。

### 3.3 产生basic auth用户名密码文件
<pre>
# mkdir -p /opt/auth
# cd /opt
# docker run --rm --entrypoint htpasswd registry:2.6.2 -bn testuser testpassword > auth/nginx.htpasswd
# cat auth/

# cat auth/nginx.htpasswd 
testuser:$2y$05$DC1SB8PwbZ63N2YbP914ruQGFssBZm6irLz.b87WmvCb6ai6B..QO
</pre>

```注意```: 在产生htpasswd时，如果不想使用```bcrypt```, 请不要使用```-B```参数(这里nginx中使用```-B```会导致验证不通过)


### 3.4 拉取nginx镜像并配置

1) **拉取nginx镜像并运行**

首先拉取nginx镜像，这里最新版本nginx为```1.13.12```:
<pre>
# docker search nginx
# docker pull nginx
# docker tag nginx nginx:1.13.12
</pre>

运行nginx容器：
{% highlight string %}
# docker run -itd -p 8080:80 --name nginx-1.13.12 nginx:1.13.12
2692b728dbae829b6bce02c07cc0359ce2ba0d22d4178dbb4db1e39efa44a5a0
# netstat -nlp | grep 8080
tcp6       0      0 :::8080                 :::*                    LISTEN      31421/docker-proxy 

# docker ps | grep nginx
2692b728dbae        nginx:1.13.12                  "nginx -g 'daemon of…"   29 minutes ago      Up 2 minutes        0.0.0.0:8080->80/tcp             nginx-1.13.12

# curl -X GET  http://192.168.69.128:8080
<!DOCTYPE html>
<html>
<head>
<title>Welcome to nginx!</title>
<style>
    body {
        width: 35em;
        margin: 0 auto;
        font-family: Tahoma, Verdana, Arial, sans-serif;
    }
</style>
</head>
<body>
<h1>Welcome to nginx!</h1>
<p>If you see this page, the nginx web server is successfully installed and
working. Further configuration is required.</p>

<p>For online documentation and support please refer to
<a href="http://nginx.org/">nginx.org</a>.<br/>
Commercial support is available at
<a href="http://nginx.com/">nginx.com</a>.</p>

<p><em>Thank you for using nginx.</em></p>
</body>
</html>
{% endhighlight %}
可以看到上面nginx目前访问正常。上面默认情况下，nginx所加载的配置文件为```/etc/nginx/nginx.conf```，这我们可以通过进入```nginx-1.13.12```容器中进行查看：
<pre>
# docker exec -it nginx-1.13.12 /bin/sh
# cd /etc/nginx/  
# ls
conf.d          koi-utf  mime.types  nginx.conf   uwsgi_params
fastcgi_params  koi-win  modules     scgi_params  win-utf
# exit
</pre>

2） **配置nginx**

上面默认的nginx配置不太符合我们当前的需求，这里我们进行手动配置。首先在```/opt/nginx-conf/```目录下创建```nginx.conf```文件：
{% highlight string %}
events {
    worker_connections  1024;
}

http {

  upstream docker-registry {
    server registry:5000;
  }

  ## Set a variable to help us decide if we need to add the
  ## 'Docker-Distribution-Api-Version' header.
  ## The registry always sets this header.
  ## In the case of nginx performing auth, the header is unset
  ## since nginx is auth-ing before proxying.
  map $upstream_http_docker_distribution_api_version $docker_distribution_api_version {
    '' 'registry/2.0';
  }

  server {
    listen 443 ssl;
    server_name myregistrydomain.com;

    # SSL
    ssl_certificate /opt/cert/harbor-registry.crt;
    ssl_certificate_key /opt/cert/harbor-registry.key;

    # Recommendations from https://raymii.org/s/tutorials/Strong_SSL_Security_On_nginx.html
    ssl_protocols TLSv1.1 TLSv1.2;
    ssl_ciphers 'EECDH+AESGCM:EDH+AESGCM:AES256+EECDH:AES256+EDH';
    ssl_prefer_server_ciphers on;
    ssl_session_cache shared:SSL:10m;

    # disable any limits to avoid HTTP 413 for large image uploads
    client_max_body_size 0;

    # required to avoid HTTP 411: see Issue #1486 (https://github.com/moby/moby/issues/1486)
    chunked_transfer_encoding on;

    location /v2/ {
      # Do not allow connections from docker 1.5 and earlier
      # docker pre-1.6.0 did not properly set the user agent on ping, catch "Go *" user agents
      if ($http_user_agent ~ "^(docker\/1\.(3|4|5(?!\.[0-9]-dev))|Go ).*$" ) {
        return 404;
      }

      # To add basic authentication to v2 use auth_basic setting.
      auth_basic "Registry realm";
      auth_basic_user_file /opt/auth/nginx.htpasswd;

      ## If $docker_distribution_api_version is empty, the header is not added.
      ## See the map directive above where this variable is defined.
      add_header 'Docker-Distribution-Api-Version' $docker_distribution_api_version always;

      proxy_pass                          http://docker-registry;
      proxy_set_header  Host              $http_host;   # required for docker client's sake
      proxy_set_header  X-Real-IP         $remote_addr; # pass on real client's IP
      proxy_set_header  X-Forwarded-For   $proxy_add_x_forwarded_for;
      proxy_set_header  X-Forwarded-Proto $scheme;
      proxy_read_timeout                  900;
    }
  }
}
{% endhighlight %}

3) **重新启动nginx**

这里我们首先将上面原来启动的nginx停掉：
{% highlight string %}
# docker ps | grep nginx-1.13.12
2692b728dbae        nginx:1.13.12                  "nginx -g 'daemon of…"   2 hours ago         Up About an hour    0.0.0.0:8080->80/tcp             nginx-1.13.12

# docker rm -vf nginx-1.13.12
nginx-1.13.12

# docker ps -a | grep nginx-1.13.12
{% endhighlight %}

重新移动加载我们配置文件的nginx:
{% highlight string %}
//首先启动一个registry，因为下面nginx反向代理registry
# docker run -itd --name registry-2.6.2 -p 5000:5000 registry:2.6.2
# docker ps 
CONTAINER ID        IMAGE                          COMMAND                  CREATED             STATUS              PORTS                            NAMES
98b168b0aa86        registry:2.6.2                 "/entrypoint.sh /etc…"   10 seconds ago      Up 8 seconds        0.0.0.0:5000->5000/tcp           registry-2.6.2

# docker exec -it registry-2.6.2 /sbin/ifconfig
eth0      Link encap:Ethernet  HWaddr 02:42:AC:11:00:03  
          inet addr:172.17.0.3  Bcast:172.17.255.255  Mask:255.255.0.0
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


# docker run -itd --name nginx-1.13.12 -p 443:443 --add-host registry:172.17.0.3 \
 -v /opt/cert/:/opt/cert/ -v /opt/auth/:/opt/auth -v /opt/nginx-conf/nginx.conf:/etc/nginx/nginx.conf \
 nginx:1.13.12

# docker ps
CONTAINER ID        IMAGE                          COMMAND                  CREATED             STATUS              PORTS                            NAMES
096f1b2688c0        nginx:1.13.12                  "nginx -g 'daemon of…"   3 seconds ago       Up 2 seconds        80/tcp, 0.0.0.0:443->443/tcp     nginx-1.13.12
98b168b0aa86        registry:2.6.2                 "/entrypoint.sh /etc…"   9 minutes ago       Up 9 minutes        0.0.0.0:5000->5000/tcp           registry-2.6.2
{% endhighlight %}

5) **测试nginx**
{% highlight string %}
# curl -X GET -iL https://192.168.69.128/v2 --cacert /opt/cert/ca.crt
HTTP/1.1 301 Moved Permanently
Server: nginx/1.13.12
Date: Wed, 11 Apr 2018 04:54:49 GMT
Content-Type: text/html
Content-Length: 186
Location: https://192.168.69.128/v2/
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0

HTTP/1.1 401 Unauthorized
Server: nginx/1.13.12
Date: Wed, 11 Apr 2018 04:54:49 GMT
Content-Type: text/html
Content-Length: 196
Connection: keep-alive
WWW-Authenticate: Basic realm="Registry realm"
Docker-Distribution-Api-Version: registry/2.0

<html>
<head><title>401 Authorization Required</title></head>
<body bgcolor="white">
<center><h1>401 Authorization Required</h1></center>
<hr><center>nginx/1.13.12</center>
</body>
</html>


# curl -X GET -iL -H "Authorization: Basic dGVzdHVzZXI6dGVzdHBhc3N3b3Jk" https://192.168.69.128/v2 --cacert /opt/cert/ca.crt
HTTP/1.1 301 Moved Permanently
Server: nginx/1.13.12
Date: Wed, 11 Apr 2018 06:26:14 GMT
Content-Type: text/html
Content-Length: 186
Location: https://192.168.69.128/v2/
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0

HTTP/1.1 200 OK
Server: nginx/1.13.12
Date: Wed, 11 Apr 2018 06:26:14 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 2
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
X-Content-Type-Options: nosniff

{} 

# curl -X GET -iL -u testuser https://192.168.69.128/v2/_catalog --cacert /opt/cert/ca.crt
Enter host password for user 'testuser':
HTTP/1.1 200 OK
Server: nginx/1.13.12
Date: Wed, 11 Apr 2018 06:36:18 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 27
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
X-Content-Type-Options: nosniff

{"repositories":["nginx"]}
{% endhighlight %}

### 3.5 使用docker-compose来启动
上面我们手动启动，比较繁琐，也容易出错。这里我们使用docker-compose来启动。编写```docker-compose.yml```文件:
{% highlight string %}
nginx:
  # Note : Only nginx:alpine supports bcrypt.
  # If you don't need to use bcrypt, you can use a different tag.
  # Ref. https://github.com/nginxinc/docker-nginx/issues/29
  image: "nginx:1.13.12"
  ports:
    - 443:443
  links:
    - registry:registry
  volumes:
    - /opt/auth:/opt/auth
    - /opt/cert:/opt/cert
    - /opt/nginx-conf/nginx.conf:/etc/nginx/nginx.conf:ro

registry:
  image: registry:2.6.2
  ports:
    - 5000:5000
{% endhighlight %}

然后我们先停掉上面```3.4```步骤启动的nginx及registry:
<pre>
# docker rm -vf nginx-1.13.12 registry-2.6.2
nginx-1.13.12
registry-2.6.2
</pre>

再用docker-compose启动：
{% highlight string %}
# docker ps
CONTAINER ID        IMAGE                          COMMAND                  CREATED             STATUS              PORTS                            NAMES
6a4d6be411c5        nginx:1.13.12                  "nginx -g 'daemon of…"   2 seconds ago       Up 2 seconds        80/tcp, 0.0.0.0:443->443/tcp     nginxconf_nginx_1
1766c8735ba1        registry:2.6.2                 "/entrypoint.sh /etc…"   3 seconds ago       Up 2 seconds        0.0.0.0:5000->5000/tcp           nginxconf_registry_1
{% endhighlight %}

用如下命令测试：
{% highlight string %}
# curl -X GET -iL -H "Authorization: Basic dGVzdHVzZXI6dGVzdHBhc3N3b3Jk" https://192.168.69.128/v2 --cacert /opt/cert/ca.crt
HTTP/1.1 301 Moved Permanently
Server: nginx/1.13.12
Date: Wed, 11 Apr 2018 06:45:17 GMT
Content-Type: text/html
Content-Length: 186
Location: https://192.168.69.128/v2/
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0

HTTP/1.1 200 OK
Server: nginx/1.13.12
Date: Wed, 11 Apr 2018 06:45:17 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 2
Connection: keep-alive
Docker-Distribution-Api-Version: registry/2.0
X-Content-Type-Options: nosniff

{}
{% endhighlight %}


停止：
<pre>
# docker-compose down -v
Stopping nginxconf_nginx_1    ... done
Stopping nginxconf_registry_1 ... done
Removing nginxconf_nginx_1    ... done
Removing nginxconf_registry_1 ... done
</pre>

<br />
<br />

**[参看]:**

1. [搭建一个支持HTTPS的私有DOCKER Registry](https://blog.csdn.net/xcjing/article/details/70238273)

2. [docker registry v2 ssl 环境搭建](https://blog.csdn.net/wanglei_storage/article/details/53126690)

3. [docker registry_v2 部署过程中遇到的坑](https://blog.csdn.net/xiaolummhae/article/details/51833354)

4. [Token Authentication Specification](https://docs.docker.com/registry/spec/auth/token/)

5. [Docker glossary](https://docs.docker.com/glossary/?term=on-prem)

6. [Docker Registry + docker_auth 使用mongodb 存储](https://blog.csdn.net/qq_21398167/article/details/54616186)

7. [Docker Registry v2 + Token Auth Server （Registry v2 认证）实例 ](http://dockone.io/article/845)

8. [从源码看Docker Registry v2中的Token认证实现机制](https://blog.csdn.net/horsefoot/article/details/51657322)

9. [Docker Register部署与基本认证](https://www.cnblogs.com/feinian/p/7857430.html)

10. [Docker Registry服务器部署配置](https://www.myfreax.com/deploying-a-registry-server/)

11. [docker私有仓库搭建及认证](https://www.cnblogs.com/wsy1030/p/8431837.html)

12. [Deploy a registry server](https://docs.docker.com/registry/deploying/)

13. [Docker--Harbor registry安全认证搭建](http://www.mamicode.com/info-detail-1855980.html)
<br />
<br />
<br />

