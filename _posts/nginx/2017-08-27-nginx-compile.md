---
layout: post
title: nginx源代码编译安装
tags:
- nginx
categories: nginx
description: nginx源代码编译安装
---

本文主要讲述从nginx源代码编译安装nginx的过程。我们这里使用的操作系统环境是32位Ubuntu16.04，所用的nginx版本为nginx1.10.3。 文章主要包括两个部分：

* Configure编译选项的介绍
* Ubuntu16.04(32bit)下nginx1.10.3的安装 

<!-- more -->


## 1. Building nginx from sources
这里主要参看[nginx官方网站](http://nginx.org/en/docs/configure.html)。在从源代码编译nginx时，需要通过```configure```命令来进行编译选项的配置。它定义了系统的许多方面，包括nginx连接所采用的方法。配置完成之后，会生成一个Makefile文件。```configure```命令支持如下的一些参数：

* ```--prefix=path:```它定义了存放服务器文件的路径。configure及nginx.conf配置文件中的所有路径都是以该目录作为一个相对路径（库源文件路径除外）。默认情况下，其会被设置为/usr/local/nginx目录。

* ```--sbin-path=path:``` 设置nginx可执行文件的名字。该名字只在安装过程中会被用到。默认情况下该文件会被命名为prefix/sbin/nginx

* ```--conf-path=path:``` 设置nginx.conf配置文件的名字。假如需要，nginx可以通过命令行参数-c file 来指定一个不同的配置文件。默认情况下，该文件会被设置为prefix/conf/nginx.conf。

* ```--pid-path=path:``` 设置nginx.pid文件的名字，该文件会用于存放主进程的进程ID。在安装之后，该文件的名字可以通过nginx.conf文件中的pid指令进行修改。默认情况下该文件会被命名为prefix/logs/nginx.pid

* ```--error-log-path=path:``` 设置主要的error,warnings及diagnostic文件的名字。在安装之后，该文件的名字也可以通过nginx.conf文件中的error_log指令进行修改。默认情况下，该文件会被命名为prefix/logs/error.log

* ```--http-log-path=path:``` 发送到HTTP Server的请求的日志文件名称。在安装之后，该文件的名称可以通过nginx.conf文件中的access_log指令进行修改。默认情况下，该文件会被命名为prefix/logs/access.log

* ```--build=name:``` 设置nginx构建出来后的名字（可选项）

* ```--user=name:``` 设置nginx工作进程的所属用户。在安装之后，该名字也可以通过nginx.conf文件中的user指令进行修改。默认的用户名为nobody

* ```--group=name:``` 设置nginx工作进程的所属组。在安装之后，该名字也可以通过nginx.conf文件中的user指令进行修改。默认情况下会被设置为非特权用户的用户名称

* ```--with-select_module/without-select_module:``` 使能/禁止一个模块使用select()方法。这个模块会被自动的编译构建假如该平台并不支持一些更合适的方法，例如kqueue，epoll，/dev/poll

* ```--with-poll_module/without-poll_module:``` 使能/禁止一个模块使用poll()方法。该模块会被自动的编译构建假如该平台并不支持一些更合适的方法，例如kqueue，epoll,/dev/poll。

* ```--without-http_gzip_module:``` 禁止构建压缩响应的模块。要想构建及运行此模块，必须依赖与zlib库。

* ```--without-http_rewrite_module:``` 禁止构建允许HTTP Server进行请求重定向及改变请求URI的模块。要构建此模块的话需要PCRE库的支持。

* ```--without-http_proxy_module:``` 禁止构建HTTP Server proxying模块

* ```--without-http_ssl_module:``` 构建HTTP Server对https协议的支持。默认情况下该模块并不会被构建。构建和运行本模块需要OpenSSL的支持

* ```--with-pcre=path:``` 设置PCRE库原文件的路径。该库的发布版本(version 4.4 – 8.40)需要从PCRE官方网站下载然后解压。剩余的操作则由nginx的./configure和make完成。在location指令中的表达式匹配及ngx_http_rewrite_module模块需要依赖该库。

* ```--with-pcre-jit:``` 构建支持“just-in-time compilation”特性的PCRE库。

* ```--with-zlib=path:``` 设置zlib库源文件的路径。该库的发布版本(version 1.1.3 - 1.2.11)需要从zlib官方网站下载然后解压。剩余的操作是由nginx的./configure及make来完成。ngx_http_gzip_module模块需要依赖于该库。

* ```--with-cc-opt=parameters:``` 设置一些额外的参数，这些参数会被添加到CFLAGS变量后。当在FreeBSD系统下使用系统PCRE库的时候，```--with-cc-opt```="-I /usr/local/include" 应该被指定。假如需要指定select()函数支持的文件句柄数也可以通过这样指定：```--with-cc-opt```="-D FD_SETSIZE=2048"。

* ```--with-ld-opt=parameters:``` 设置链接时候的一些额外的参数。当在FreeBSD系统下使用PCRE库时，应该指定```--with-ld-opt```="-L /usr/local/lib"。

* ```--with-debug```: 将Nginx需要打印debug调试级别日志的代码编译进Nginx。这样可以在Nginx运行时通过修改配置文件来使其打印调试日志，这对于研究、定位Nginx问题非常有帮助
<br />
如下是一个例子（所有参数都应该处于同一行）：
{% highlight string %}
# ./configure \
--sbin-path=/usr/local/nginx/nginx \
--conf-path=/usr/local/nginx/nginx.conf \
--pid-path=/usr/local/nginx/nginx.pid \
--with-http_ssl_module \
--with-pcre=../pcre-8.40 \
--with-zlib=../zlib-1.2.11 
{% endhighlight %}

在执行完配置之后，就可以使用make来完成nginx的编译与安装。




## 2. Ubuntu16.04 + nginx1.10.3安装
参看：http://www.cnblogs.com/badboyf/p/6422547.html

**(1) gcc,g++依赖库的安装**
{% highlight string %}
# apt-get install build-essential
# apt-get install libtool
{% endhighlight %}

这里简单介绍一下这两个库：

```build-essential:``` 软件包的作用是提供编译程序必须软件包的列表信息，也就是说编译程序有了这个软件包，它才知道头文件在哪，才知道库函数在哪，还会下载依赖的软件包，最后才组成一个开发环境。

```libtool:``` 软件包是一个用于支持script的通用库。它通过提供一个一致性、可移植性的接口来隐藏使用共享库的复杂性。

我们可以通过如下命令来查看这两个包是否已经安装：
{% highlight string %}
# dpkg -l | grep build-essential
# dpkg -l | grep libtool
{% endhighlight %}

**(2) 安装pcre依赖库**

参看：http://www.pcre.org/

执行如下命令进行安装：
{% highlight string %}
# sudo apt-get install libpcre3 libpcre3-dev
{% endhighlight %}
或源码安装：
{% highlight string %}
# wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.40.tar.gz
# tar -zxvf pcre-8.40.tar.gz
# cd pcre-8.40
# ./configure
# make
# make install
{% endhighlight %}
(注：这里可以暂时解压出来，不用进行安装，后续让nginx脚本来进行安装)


**(3) 安装zlib依赖库**

参看：http://www.zlib.net
{% highlight string %}
# apt-get install zliblg-dev
{% endhighlight %}

或源码安装：
{% highlight string %}
# wget http://zlib.net/zlib-1.2.11.tar.gz
# tar -zxvf zlib-1.2.11.tar.gz
# cd zlib-1.2.11
# ./configure
# make
# make install
{% endhighlight %}
(注：这里可以暂时解压出来，不用进行安装，后续让nginx脚本来进行安装)



**(4) 安装ssl依赖库**

执行如下命令进行安装：
{% highlight string %}
# apt-get install openssl libssl-dev
{% endhighlight %}
也可以通过如下命令来下载二进制安装包：
{% highlight string %}
# apt-get download openssl
{% endhighlight %}


**(5) 安装nginx**

这里我们所用的nginx版本为：nginx-1.10.3
{% highlight string %}
# ./configure \
--prefix=/usr/local/nginx \
--with-http_ssl_module \
--with-pcre=../pcre-8.40 \
--with-zlib=../zlib-1.2.11

# ./configure \
--sbin-path=/usr/local/nginx/nginx \
--conf-path=/usr/local/nginx/nginx.conf \
--pid-path=/usr/local/nginx/nginx.pid \
--with-http_ssl_module \
--with-pcre=../pcre-8.40 \
--with-zlib=../zlib-1.2.11

# ./configure \
--prefix=/usr/local/nginx \
--sbin-path=/usr/sbin/nginx \
--conf-path=/etc/nginx/nginx.conf \
--error-log-path=/var/log/nginx/error.log \
--http-log-path=/var/log/nginx/access.log \
--pid-path=/var/run/nginx.pid \
--lock-path=/var/lock/nginx.lock \

--http-client-body-temp-path=/var/tmp/nginx/client/ \
--http-proxy-temp-path=/var/tmp/nginx/proxy/ \
--http-fastcgi-temp-path=/var/tmp/nginx/fastcgi/ \
--http-uwsgi-temp-path=/var/tmp/nginx/uwsgi \
--http-scgi-temp-path=/var/tmp/nginx/scgi \

--with-http_ssl_module \
--with-pcre=../pcre-8.40 \
--with-zlib=../zlib-1.2.11

# make 

# make install 
{% endhighlight %}
如果我们要打开调试选项，可以使用```--with-debug```。


**(6) 查看nginx是否安装成功**

启动、停止、重载配置、测试配置文件是否正确：

{% highlight string %}
# sudo /usr/local/nginx/nginx             #采用默认的配置文件启动
# sudo /usr/local/nginx/nginx -c /usr/local/nginx/nginx.conf                                #采用指定的配置文件启动 

# sudo /usr/local/nginx/nginx -t         #检测配置文件是否正确

# sudo /usr/local/nginx/nginx -s stop    #停止

# sudo /usr/local/nginx/nginx -s reload  #重载配置文件


# sudo /usr/local/nginx/nginx            #下面采用默认的配置文件启动，并查看各进程的状态
# ps -aux | grep nginx | grep -v grep
root      5088  0.0  0.0   6308   480 ?        Ss   19:52   0:00 nginx: master process /usr/local/nginx/nginx
nobody    5089  0.0  0.2   6448  2064 ?        S    19:52   0:00 nginx: worker process
# ps -ef | grep nginx | grep -v grep
root      5088     1  0 19:52 ?        00:00:00 nginx: master process /usr/local/nginx/nginx
nobody    5089  5088  0 19:52 ?        00:00:00 nginx: worker process
{% endhighlight %}

**7) 安装后目录情况**
{% highlight string %}
# ls /usr/local/nginx/ -al
total 5156
drwxr-xr-x  9 root   root    4096 Jun 25 09:58 .
drwxr-xr-x 12 root   root    4096 Sep  2 07:09 ..
drwx------  2 nobody root    4096 Jun 24 23:06 client_body_temp
-rwxr-xr-x  1 root   root    1077 Jun 24 22:59 fastcgi.conf
-rwxr-xr-x  1 root   root    1077 Jun 24 22:59 fastcgi.conf.default
-rwxr-xr-x  1 root   root    1007 Jun 24 22:59 fastcgi_params
-rwxr-xr-x  1 root   root    1007 Jun 24 22:59 fastcgi_params.default
drwx------  2 nobody root    4096 Jun 24 23:06 fastcgi_temp
drwxr-xr-x  2 root   root    4096 Jun 24 22:59 html
-rwxr-xr-x  1 root   root    2837 Jun 24 22:59 koi-utf
-rwxr-xr-x  1 root   root    2223 Jun 24 22:59 koi-win
drwxr-xr-x  2 root   root    4096 Jun 24 23:06 logs
-rwxr-xr-x  1 root   root    3957 Jun 24 22:59 mime.types
-rwxr-xr-x  1 root   root    3957 Jun 24 22:59 mime.types.default
-rwxr-xr-x  1 root   root 5180748 Jun 24 22:59 nginx
-rwxr-xr-x  1 root   root    2656 Jun 24 22:59 nginx.conf
-rwxr-xr-x  1 root   root    2656 Jun 24 22:59 nginx.conf.default
drwx------  2 nobody root    4096 Jun 24 23:06 proxy_temp
-rwxr-xr-x  1 root   root     636 Jun 24 22:59 scgi_params
-rwxr-xr-x  1 root   root     636 Jun 24 22:59 scgi_params.default
drwx------  2 nobody root    4096 Jun 24 23:06 scgi_temp
-rwxr-xr-x  1 root   root     664 Jun 24 22:59 uwsgi_params
-rwxr-xr-x  1 root   root     664 Jun 24 22:59 uwsgi_params.default
drwx------  2 nobody root    4096 Jun 24 23:06 uwsgi_temp
-rwxr-xr-x  1 root   root    3610 Jun 24 22:59 win-utf

# ls /usr/local/nginx/logs/
access.log  error.log   
{% endhighlight %}



## 3. 二进制安装nginx
一般直接执行```yum install```安装命令即可：
{% highlight string %}
# nginx -V
nginx version: nginx/1.12.2
built by gcc 4.8.5 20150623 (Red Hat 4.8.5-36) (GCC) 
built with OpenSSL 1.0.2k-fips  26 Jan 2017
TLS SNI support enabled
configure arguments: 
--prefix=/usr/share/nginx --sbin-path=/usr/sbin/nginx \
--modules-path=/usr/lib64/nginx/modules --conf-path=/etc/nginx/nginx.conf \
--error-log-path=/var/log/nginx/error.log --http-log-path=/var/log/nginx/access.log \
--http-client-body-temp-path=/var/lib/nginx/tmp/client_body \
--http-proxy-temp-path=/var/lib/nginx/tmp/proxy \
--http-fastcgi-temp-path=/var/lib/nginx/tmp/fastcgi \
--http-uwsgi-temp-path=/var/lib/nginx/tmp/uwsgi \
--http-scgi-temp-path=/var/lib/nginx/tmp/scgi \
--pid-path=/run/nginx.pid \
--lock-path=/run/lock/subsys/nginx \
--user=nginx --group=nginx \
--with-file-aio --with-ipv6 \
--with-http_auth_request_module --with-http_ssl_module \
--with-http_v2_module --with-http_realip_module \
--with-http_addition_module --with-http_xslt_module=dynamic \
--with-http_image_filter_module=dynamic --with-http_geoip_module=dynamic \
--with-http_sub_module --with-http_dav_module --with-http_flv_module \
--with-http_mp4_module --with-http_gunzip_module --with-http_gzip_static_module \
--with-http_random_index_module --with-http_secure_link_module \
--with-http_degradation_module --with-http_slice_module --with-http_stub_status_module \
--with-http_perl_module=dynamic --with-mail=dynamic --with-mail_ssl_module \
--with-pcre --with-pcre-jit --with-stream=dynamic --with-stream_ssl_module \
--with-google_perftools_module --with-debug \
--with-cc-opt='-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong \
--param=ssp-buffer-size=4 -grecord-gcc-switches -specs=/usr/lib/rpm/redhat/redhat-hardened-cc1 \
-m64 -mtune=generic' --with-ld-opt='-Wl,-z,relro -specs=/usr/lib/rpm/redhat/redhat-hardened-ld -Wl,-E'
{% endhighlight %}

<br />
<br />

**[参看]：**

1. [Nginx快速入门-菜鸟笔记](https://www.cnblogs.com/wylhome/p/6057198.html)


<br />
<br />
<br />

