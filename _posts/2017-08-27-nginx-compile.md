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

* ```--prefix=path:``` 它定义了存放服务器文件的路径。configure及nginx.conf配置文件中的所有路径都是以该目录作为一个相对路径（库源文件路径除外）。默认情况下，其会被设置为/usr/local/nginx目录。

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

* ```--with-zlib=path:``` 设置zlib库源文件的路径。该库的发布版本(version 1.1.3-1.2.11)需要从zlib官方网站下载然后解压。剩余的操作是由nginx的./configure及make来完成。ngx_http_gzip_module模块需要依赖于该库。

* ```--with-cc-opt=parameters:``` 设置一些额外的参数，这些参数会被添加到CFLAGS变量后。当在FreeBSD系统下使用系统PCRE库的时候，--with-cc-opt="-I /usr/local/include" 应该被指定。假如需要指定select()函数支持的文件句柄数也可以通过这样指定：--with-cc-opt="-D FD_SETSIZE=2048"。

* ```--with-ld-opt=parameters:``` 设置链接时候的一些额外的参数。当在FreeBSD系统下使用PCRE库时，应该指定--with-ld-opt="-L /usr/local/lib"。










<br />
<br />
<br />

