---
layout: post
title: Nginx支持https
tags:
- nginx
categories: nginx
description: Nginx支持https
---

本文主要讲述一下nginx对https的支持及相关的配置（本文针对CentOS7.4操作系统)。


<!-- more -->



## 1. 编译支持https nginx
这里因为前面我们讲述了nginx的编译（Ubuntu12.04)，这里虽然系统不一样，但是差异不大。
<pre>
# wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.40.tar.gz
# tar -zxvf pcre-8.40.tar.gz

# wget http://zlib.net/zlib-1.2.11.tar.gz
# tar -zxvf zlib-1.2.11.tar.gz

# yum install openssl openssl-devel      //头文件默认安装在/usr/include/openssl目录下

# tar -zxvf nginx-1.10.3.tar.gz
# cd nginx-1.10.3/

# ./configure \
--prefix=/usr/local/nginx \
--with-http_ssl_module \
--with-pcre=../pcre-8.40 \
--with-zlib=../zlib-1.2.11 \
--with-http_gzip_static_module \
--with-http_stub_status_module \
--with-http_sub_module

# make
# make install
</pre>
这里要支持https，则必须添加```--with-http_ssl_module```; 另外这里还添加了```sub_status```模块，用于查看相应的访问记录。








<br />
<br />

**[参看]:**

1. [http module参看](http://nginx.org/en/docs/)

<br />
<br />
<br />

