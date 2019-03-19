---
layout: post
title: 为nginx编译第三方动态模块
tags:
- nginx
categories: nginx
description: nginx使用基础
---


本章我们讲述一下如何为Nginx编译第三方动态加载模块。我们当前的操作系统环境为：
<pre>
# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core)
</pre>


<!-- more -->

## 1. 编译安装nginx

这里安装前面我们讲解的nginx 1.10.3版本：
<pre>
# mkdir nginx-inst && cd nginx-inst
# wget http://nginx.org/download/nginx-1.10.3.tar.gz
# wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.40.tar.gz
# wget http://zlib.net/zlib-1.2.11.tar.gz

# yum install openssl
# yum install openssl-devel.x86_64

# ls
nginx-1.10.3.tar.gz  pcre-8.40.tar.gz  zlib-1.2.11.tar.gz
# tar -zxvf pcre-8.40.tar.gz
# tar -zxvf zlib-1.2.11.tar.gz
# tar -zxvf nginx-1.10.3.tar.gz

# cd nginx-1.10.3/
# ./configure \
--prefix=/usr/local/nginx \
--conf-path=/etc/nginx/nginx.conf \
--error-log-path=/var/log/nginx/error.log  \
--http-log-path=/var/log/nginx/access.log  \
--pid-path=/var/run/nginx.pid  \
--lock-path=/var/run/nginx.lock  \
--with-http_ssl_module \
--with-pcre=../pcre-8.40 \
--with-zlib=../zlib-1.2.11
# make 
# make install
</pre>
启动nginx， 并进行简单测试：
{% highlight string %}
# cd /usr/local/nginx/sbin
# ./nginx -t
nginx: the configuration file /etc/nginx/nginx.conf syntax is ok
nginx: configuration file /etc/nginx/nginx.conf test is successful

# curl -X GET http://127.0.0.1:80
{% endhighlight %}

## 2. Dynamic Modules Overview
NGINX 1.9.11开始增加加载动态模块支持，从此不再需要替换nginx文件即可增加第三方扩展。通常我们要使用一个第三方模块，采用如下步骤：

* 获得当前你所运行nginx对应版本的源代码

* 获得第三方模块源代码，并修改该模块对应的config文件

* 编译动态模块

* 加载动态模块

目前nginx官方只有几个模块支持动态加载：
<pre>
# ./configure --help | grep dynamic
  --with-http_xslt_module=dynamic    enable dynamic ngx_http_xslt_module
  --with-http_image_filter_module=dynamic
                                     enable dynamic ngx_http_image_filter_module
  --with-http_geoip_module=dynamic   enable dynamic ngx_http_geoip_module
  --with-http_perl_module=dynamic    enable dynamic ngx_http_perl_module
  --with-mail=dynamic                enable dynamic POP3/IMAP4/SMTP proxy module
  --with-stream=dynamic              enable dynamic TCP/UDP proxy module
  --add-dynamic-module=PATH          enable dynamic external module
</pre>



## 3. 示例

本示例使用一个简单的[Hello World Module](https://github.com/perusio/nginx-hello-world-module)来演示如何为一个module更新源代码并加载到nginx中。```Hello World```实现了一条简单的指令(**hello_world**)来响应客户端的请求。

1） **获得对应版本的nginx源代码**

<pre>
# /usr/local/nginx/sbin/nginx -v
nginx version: nginx/1.10.3
</pre>
这里我们采用nginx 1.10.3版本的源代码来编译我们的动态模块。

2) **获取模块源代码**

我们在nginx安装目录```nginx-inst```处下载我们的```Hello World```模块源代码：
<pre>
# git clone https://github.com/perusio/nginx-hello-world-module.git
# ls
nginx-1.10.3  nginx-1.10.3.tar.gz  nginx-hello-world-module  pcre-8.40  pcre-8.40.tar.gz  zlib-1.2.11  zlib-1.2.11.tar.gz
</pre>

源代码目录nginx-hello-world-module下的```config```脚本定义了如何进行编译。参看如下：
{% highlight string %}
ngx_addon_name=ngx_http_hello_world_module

if test -n "$ngx_module_link"; then
  ngx_module_type=HTTP
  ngx_module_name=ngx_http_hello_world_module
  ngx_module_srcs="$ngx_addon_dir/ngx_http_hello_world_module.c"
  . auto/module
else
        HTTP_MODULES="$HTTP_MODULES ngx_http_hello_world_module"
        NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_hello_world_module.c"
fi
{% endhighlight %}



3） **编译动态模块**

这里我们可以先获取当前我们正在运行的nginx的一个编译参数：
<pre>
# /usr/local/nginx/sbin/nginx -V
nginx version: nginx/1.10.3
built by gcc 4.8.5 20150623 (Red Hat 4.8.5-28) (GCC) 
built with OpenSSL 1.0.2k-fips  26 Jan 2017
TLS SNI support enabled
configure arguments: --prefix=/usr/local/nginx --conf-path=/etc/nginx/nginx.conf --error-log-path=/var/log/nginx/error.log \
--http-log-path=/var/log/nginx/access.log --pid-path=/var/run/nginx.pid --lock-path=/var/run/nginx.lock --with-http_ssl_module \
--with-pcre=../pcre-8.40 --with-zlib=../zlib-1.2.11
</pre>
这里因为我们的```Hello World```要工作在该环境下，因此编译时最好保持参数完全一致，以减少不必要的麻烦：
<pre>
# cd nginx-1.10.3
# make clean
# ./configure \
--prefix=/usr/local/nginx \
--conf-path=/etc/nginx/nginx.conf \
--error-log-path=/var/log/nginx/error.log  \
--http-log-path=/var/log/nginx/access.log  \
--pid-path=/var/run/nginx.pid  \
--lock-path=/var/run/nginx.lock  \
--with-http_ssl_module \
--with-pcre=../pcre-8.40 \
--with-zlib=../zlib-1.2.11 \
--add-dynamic-module=../nginx-hello-world-module
# make modules
# ls objs/
addon         Makefile           ngx_auto_headers.h                     ngx_http_hello_world_module_modules.o  ngx_modules.c
autoconf.err  ngx_auto_config.h  ngx_http_hello_world_module_modules.c  ngx_http_hello_world_module.so         src
</pre>
上面我们可以看到编译出了一个```.so```的动态链接库文件。

4）**加载并使用模块**

我们将上述生成的```.so```动态链接库文件拷贝到nginx的安装目录下的```modules```文件夹：
<pre>
#  mkdir -p /usr/local/nginx/modules
# cp objs/ngx_http_hello_world_module.so /usr/local/nginx/modules/
# chmod 777 /usr/local/nginx/modules/ngx_http_hello_world_module.so
# ls /usr/local/nginx/modules/
ngx_http_hello_world_module.so
</pre>

修改nginx配置文件(/etc/nginx/nginx.conf)，使用```load_module```指令在top-level(main)加载该动态链接库：
<pre>
# vi /etc/nginx/nginx.conf
worker_processes  1;

load_module modules/ngx_http_hello_world_module.so;

events {
    worker_connections  1024;
}
</pre>

然后再修改nginx配置文件，在**http**上下文添加一个**location**块，并使用```Hello World```模块的**hello_world**指令：
<pre>
server {
    listen 80;

    location / {
         hello_world;
    }
}
</pre>


最后重新加载nginx，发送请求进行验证：
<pre>
# /usr/local/nginx/sbin/nginx -t
nginx: the configuration file /etc/nginx/nginx.conf syntax is ok
nginx: configuration file /etc/nginx/nginx.conf test is successful

# /usr/local/nginx/sbin/nginx -s reload
# curl -X GET http://127.0.0.1:80/
hello world
</pre>



<br />
<br />

**[参看]**

1. [Compiling Third-Party Dynamic Modules for NGINX and NGINX Plus](https://www.nginx.com/blog/page/50/)

2. [Extending NGINX](https://www.nginx.com/resources/wiki/extending/)

3. [nginx代理tcp协议连接mysql](https://www.cnblogs.com/heruiguo/p/8962243.html)

<br />
<br />
<br />

