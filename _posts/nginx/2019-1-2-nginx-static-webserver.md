---
layout: post
title: 用HTTP核心模块配置一个静态Web服务器
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


静态Web服务器的主要功能由```ngx_http_core_module```模块（HTTP框架的主要成员）实现。当然，一个完整的静态Web服务器还有许多功能是由其他的HTTP模块实现的。本章主要讨论如何配置一个包含基本功能的静态Web服务器，文中会完整的说明ngx_http_core_module模块提供的配置项及变量的用法，但不会过多的说明其他HTTP模块的配置项。

<!-- more -->

一个典型的静态Web服务器还会包含多个```server```块和```location```块，例如：
{% highlight string %}
http {
   gzip on;

   upstream {
     ...
   }

   server {
       listen localhost:80;

       ...

       location /webstatic {
           if ... {
              ...
           }

           root /opt/webresource;
           ...
       }

       location ~* \.(jpg|jpeg|png|jpe|gif)$ {

          ...
       }

   }

   server {

      ....
   }

}
{% endhighlight %}
Nginx为配置一个完整的静态web服务器提供了非常多功能，下面会把这些配置项分为以下8类进行详述：

* 虚拟主机与请求的分发

* 文件路径定义

* 内存及磁盘资源的分配

* 网络连接的设置

* MIME类型的设置

* 对客户端请求的限制

* 文件操作的优化

* 对客户端请求的特殊处理

上面这样的划分只是为了帮助大家从功能上理解这些配置项。在这之后会列出ngx_http_core_module模块提供的变量，以及简单说明它们的含义。



## 1. 虚拟主机与请求分发

配置虚拟主机其实有三种方法：

* 基于域名的虚拟主机： 不同的域名，相同的IP

* 基于端口的虚拟主机： ```不使用```域名、IP来区分不同站点的内容，而是使用不同的TCP端口号

* 基于IP地址的虚拟主机： 不同的域名、不同的IP（需要添加网络接口，应用不广泛）

假设我们当前的Linux主机的IP地址为： 192.168.1.220

### 1.1 基于域名的虚拟主机
我们可以使用```bind-9.8.2-0.17```安装包来搭建DNS服务器，但如果只是为了简单测试，我们可以直接修改```/etc/hosts```文件来达到域名解析。这里我们通过修改```/etc/hosts```文件来实现，在该文件中加入如下：
<pre>
# cat /etc/hosts
www.bt.com  192.168.1.220
www.accp.com 192.168.1.220
</pre>

1) **修改nginx配置文件**

在nginx.conf配置文件的```http```段中加入如下：
{% highlight string %}
http{
   include vhost/*.conf
}
{% endhighlight %}

然后我们在nginx配置文件目录下创建一个```vhost```文件夹，并在该文件夹内添加```vname.conf```文件：
{% highlight string %}
server {
    listen 80;
    server_name www.bt.com;

    location / {
        root /var/www/bt;
        index index.html index.php;
    }
}

server {
     listen 80;

     server_name www.accp.com;

     location / {
         root /var/www.accp;
         index index.html index.php;
     }
}
{% endhighlight %}

2) **创建站点目录和测试页面**

执行如下命令：
{% highlight string %}
# mkdir -p /var/www/bt
# mkdir -p /var/www/accp

# echo "this is bt" >> /var/www/bt/index.html
# echo "this is accp" >> /var/www/accp/index.html 
{% endhighlight %}

重启nginx，并进行测试：
<pre>
# nginx -s reload
# curl -X GET http://www.bt.com/index.html
# curl -X GET http://www.accp.com/index.html
</pre>


### 1.2 基于端口的虚拟主机
1) **步骤和基于域名的虚拟主机相似，修改vname.conf**

{% highlight string %}
server {
    listen 8888;
    server_name _;

    location / {
        root /var/www/bt;
        index index.html index.php;
    }
}

server {
     listen 9999;

     server_name _;

     location / {
         root /var/www.accp;
         index index.html index.php;
     }
}
{% endhighlight %}

2) **重启nginx，并进行测试**
<pre>
# nginx -s reload
# curl -X GET http://192.168.1.220:8888/index.html
# curl -x GET http://192.168.1.220:9999/index.html
</pre>

### 1.3 基于IP地址的虚拟主机

1) **配置虚拟网卡**

我们在VMware虚拟机上，可以通过配置另外一个虚拟网卡来增加一个新的IP地址: 192.168.1.221

2） **步骤和基于域名的虚拟主机相似，修改vname.conf**
{% highlight string %}
server {
    listen 192.168.1.220:80;
    server_name _;

    location / {
        root /var/www/bt;
        index index.html index.php;
    }
}

server {
     listen 192.168.1.221:80;

     server_name _;

     location / {
         root /var/www.accp;
         index index.html index.php;
     }
}
{% endhighlight %}

3) **重启nginx并进行测试**
<pre>
# nginx -s reload 
# curl -X GET http://192.168.1.220:80/index.html
# curl -X GET http://192.168.1.221:80/index.html
</pre>

### 1.4 关于server_name配置项说明
```server_name```后可以跟多个主机名称，如:
<pre>
server_name www.testweb.coms download.testweb.com;
</pre>
在开始处理一个HTTP请求时，Nginx会取出header头中的Host，与每个server中的server_name进行匹配，以此决定到底哪一个server块来处理这个请求。有可能一个Host与多个server块中的server_name都匹配。这时就会根据匹配优先级来选择实际处理的server块。server_name与Host的匹配优先级如下：

1) 首先选择所有字符串完全匹配的server_name，如www.testweb.com

2) 其次选择通配符在前面的server_name， 如*.testweb.com

3) 再次选择通配符在后面的server_name， 如www.testweb.*

4) 最后选择使用正则表达式才匹配的server_name，如```~^\.testweb\.com$```


假如Host与所有的server_name都不匹配，这时将会按如下顺序选择处理的server块：

1) 优先选择在listen配置项后加入[default|default_server]的server块

2） 找到匹配listen端口的第一个server块

如果server_name后跟着空字符串（如server_name "";)，那么表示匹配没有Host这个Http头部的请求。


<br />
<br />

**[参看]**

1. [Nginx虚拟主机配置](https://blog.51cto.com/13630803/2129560)



<br />
<br />
<br />

