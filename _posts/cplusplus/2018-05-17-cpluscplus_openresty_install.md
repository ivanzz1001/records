---
layout: post
title: Centos7下OpenResty的安装
tags:
- cplusplus
categories: cplusplus
description: Centos7下OpenResty
---

OpenResty® 是一个基于 Nginx 与 Lua 的高性能 Web 平台，其内部集成了大量精良的 Lua 库、第三方模块以及大多数的依赖项。用于方便地搭建能够处理超高并发、扩展性极高的动态 Web 应用、Web 服务和动态网关。

OpenResty® 通过汇聚各种设计精良的 Nginx 模块（主要由 OpenResty 团队自主开发），从而将 Nginx 有效地变成一个强大的通用 Web 应用平台。这样，Web 开发人员和系统工程师可以使用 Lua 脚本语言调动 Nginx 支持的各种 C 以及 Lua 模块，快速构造出足以胜任 10K 乃至 1000K 以上单机并发连接的高性能 Web 应用系统。

OpenResty® 的目标是让你的Web服务直接跑在 Nginx 服务内部，充分利用 Nginx 的非阻塞 I/O 模型，不仅仅对 HTTP 客户端请求,甚至于对远程后端诸如 MySQL、PostgreSQL、Memcached 以及 Redis 等都进行一致的高性能响应。

本文讲述一下Centos7操作系统环境下OpenResty的安装。当前我们的操作系统环境为：

<!-- more -->

<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 

# uname -a
Linux oss-uat-01 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>


## 1. 二进制包安装

针对Centos7，OpenResty官方提供了预编译包。首先可以在Centos系统中添加```openresty```仓库，这样就可以便于安装或更新我们的软件包（通过yum update命令）。

**1) 添加openresty仓库**

运行下面的命令添加openresty仓库：
<pre>
# sudo yum install yum-utils

# sudo yum-config-manager --add-repo https://openresty.org/package/centos/openresty.repo
</pre>

**2) 安装openresty**

执行如下的命令安装openresty:
<pre>
#  sudo yum install openresty
</pre>

如果想安装命令行工具resty， 那么可以像下面这样安装```openresty-resty```包：
<pre>
# sudo yum install openresty-resty
</pre>
命令行工具 opm 在 openresty-opm 包里，而 restydoc 工具在 openresty-doc 包里头。

**3) 列出所有openresty仓库里的软件包**
<pre>
# sudo yum --disablerepo="*" --enablerepo="openresty" list available
Loaded plugins: fastestmirror, langpacks
openresty/7/x86_64/signature                                                                                                    |  490 B  00:00:00     
Retrieving key from https://openresty.org/package/pubkey.gpg
Importing GPG key 0xD5EDEB74:
 Userid     : "OpenResty Admin <admin@openresty.com>"
 Fingerprint: e522 18e7 0878 97dc 6dea 6d6d 97db 7443 d5ed eb74
 From       : https://openresty.org/package/pubkey.gpg
Is this ok [y/N]: y
openresty/7/x86_64/signature                                                                                                     | 2.9 kB  00:00:06 !!! 
openresty/7/x86_64/primary_db                                                                                                    |  57 kB  00:00:00     
Loading mirror speeds from cached hostfile
Available Packages
openresty.x86_64                                                        1.13.6.2-1.el7.centos                             openresty
openresty-asan.x86_64                                                   1.13.6.2-5.el7.centos                             openresty
openresty-asan-debuginfo.x86_64                                         1.13.6.2-5.el7.centos                             openresty
openresty-debug.x86_64                                                  1.13.6.2-1.el7.centos                             openresty
openresty-debug-debuginfo.x86_64                                        1.13.6.2-1.el7.centos                             openresty
openresty-debuginfo.x86_64                                              1.13.6.2-1.el7.centos                             openresty
openresty-doc.noarch                                                    1.13.6.2-1.el7.centos                             openresty
openresty-openssl.x86_64                                                1.1.0h-3.el7.centos                               openresty
openresty-openssl-asan.x86_64                                           1.1.0h-8.el7.centos                               openresty
openresty-openssl-asan-debuginfo.x86_64                                 1.1.0h-8.el7.centos                               openresty
openresty-openssl-asan-devel.x86_64                                     1.1.0h-8.el7.centos                               openresty
openresty-openssl-debug.x86_64                                          1.1.0h-3.el7.centos                               openresty
openresty-openssl-debug-debuginfo.x86_64                                1.1.0h-3.el7.centos                               openresty
openresty-openssl-debug-devel.x86_64                                    1.1.0h-3.el7.centos                               openresty
openresty-openssl-debuginfo.x86_64                                      1.1.0h-3.el7.centos                               openresty
openresty-openssl-devel.x86_64                                          1.1.0h-3.el7.centos                               openresty
openresty-opm.noarch                                                    1.13.6.2-1.el7.centos                             openresty
openresty-pcre.x86_64                                                   8.42-1.el7.centos                                 openresty
openresty-pcre-asan.x86_64                                              8.42-12.el7.centos                                openresty
openresty-pcre-asan-debuginfo.x86_64                                    8.42-12.el7.centos                                openresty
openresty-pcre-asan-devel.x86_64                                        8.42-12.el7.centos                                openresty
openresty-pcre-debuginfo.x86_64                                         8.42-1.el7.centos                                 openresty
openresty-pcre-devel.x86_64                                             8.42-1.el7.centos                                 openresty
openresty-resty.noarch                                                  1.13.6.2-1.el7.centos                             openresty
openresty-valgrind.x86_64                                               1.13.6.2-1.el7.centos                             openresty
openresty-valgrind-debuginfo.x86_64                                     1.13.6.2-1.el7.centos                             openresty
openresty-zlib.x86_64                                                   1.2.11-3.el7.centos                               openresty
openresty-zlib-asan.x86_64                                              1.2.11-11.el7.centos                              openresty
openresty-zlib-asan-debuginfo.x86_64                                    1.2.11-11.el7.centos                              openresty
openresty-zlib-asan-devel.x86_64                                        1.2.11-11.el7.centos                              openresty
openresty-zlib-debuginfo.x86_64                                         1.2.11-3.el7.centos                               openresty
openresty-zlib-devel.x86_64                                             1.2.11-3.el7.centos                               openresty
perl-Lemplate.noarch                                                    0.15-1.el7.centos                                 openresty
perl-Spiffy.noarch                                                      0.46-3.el7.centos                                 openresty
perl-Test-Base.noarch                                                   0.88-2.el7.centos                                 openresty
perl-Test-LongString.noarch                                             0.17-1.el7.centos                                 openresty
perl-Test-Nginx.noarch                       
</pre>

**4) 下载最新版openresty**
<pre>
# sudo yum install --downloadonly --downloaddir=./ openresty.x86_64
# ls
openresty-1.13.6.2-1.el7.centos.x86_64.rpm        openresty-pcre-8.42-1.el7.centos.x86_64.rpm
openresty-openssl-1.1.0h-3.el7.centos.x86_64.rpm  openresty-zlib-1.2.11-3.el7.centos.x86_64.rpm
</pre>


## 2. 源代码编译安装openresty


### 2.1 安装
**1) 下载源代码包**

这里我们安装```openresty-1.11.2.5```版本：
<pre>
# https://openresty.org/download/openresty-1.11.2.5.tar.gz
</pre>

**2) 安装pcre依赖库**

参看：http://www.pcre.org/

执行如下命令进行安装：
{% highlight string %}
# yum install pcre.x86_64 pcre-devel.x86_64
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

**3） 安装zlib依赖库**

参看：http://www.zlib.net
{% highlight string %}
# yum install zlib-devel.x86_64
# yum install zlib.x86_64
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

**4) 安装ssl依赖库**

执行如下命令进行安装：
{% highlight string %}
# yum install openssl.x86_64  
# yum install openssl-devel.x86_64
{% endhighlight %}

也可以到```http://rpmfind.net/linux/rpm2html/search.php```找到相应的安装包安装。

**4) 安装openresty**

执行如下的命令进行安装：
{% highlight string %}
# ./configure \
 --with-luajit \
 --with-http_ssl_module \
 --with-pcre=../pcre-8.40 \
 --with-zlib=../zlib-1.2.11
{% endhighlight %}
增加```--with-luajit```以支持lua脚本。

### 2.2 测试
1) 首先配置上面安装的openresty。上面默认安装在```/usr/local/openresty```目录下，修改该目录下的```nginx/conf/nginx.conf```文件：
{% highlight string %}
#user  nobody;
worker_processes  1;

#pid        logs/nginx.pid;


events {
    worker_connections  1024;
}


http {
    include       mime.types;
    default_type  application/octet-stream;

   
    #access_log  logs/access.log  main;

    sendfile        on;
    #tcp_nopush     on;

    #keepalive_timeout  0;
    keepalive_timeout  65;

    #gzip  on;

    server {
        listen       80;
        server_name  localhost;

        #charset koi8-r;

        #access_log  logs/host.access.log  main;

        location / {
            default_type text/html;
            content_by_lua '
                ngx.say("<p>hello, world</p>")
            ';
        }

        #error_page  404              /404.html;

        # redirect server error pages to the static page /50x.html
        #
        error_page   500 502 503 504  /50x.html;
        location = /50x.html {
            root   html;
        }

      
    }

}
{% endhighlight %}


2) 接着执行如下命令启动openresty
<pre>
# /usr/local/openresty/nginx/sbin/nginx 

# ps -ef | grep nginx
root     25588     1  0 21:36 ?        00:00:00 nginx: master process sbin/nginx
nobody   25589 25588  0 21:36 ?        00:00:00 nginx: worker process
root     25591  2957  0 21:36 pts/0    00:00:00 grep --color=auto nginx
</pre>

3) 再接着执行如下命令进行简单测试
{% highlight string %}
# curl -XGET http://10.17.156.68
<p>hello, world</p>
{% endhighlight %}

可以看到上面打印出了lua脚本设置的```hello,world```。

<br />
<br />

**[参考]**

1. [OpenResty® Linux 包](http://openresty.org/cn/linux-packages.html)

2. [OpenResty官方网站](http://openresty.org/cn/)

3. [OpenResty installation](http://openresty.org/cn/installation.html)

<br />
<br />
<br />




