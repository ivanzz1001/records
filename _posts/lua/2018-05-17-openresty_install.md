---
layout: post
title: Centos7下OpenResty的安装
tags:
- lua
categories: lua
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

直接```yum install```安装默认配置如下：
{% highlight string %}
# nginx -V
nginx version: openresty/1.13.6.2
built by gcc 4.8.5 20150623 (Red Hat 4.8.5-11) (GCC) 
built with OpenSSL 1.1.0h  27 Mar 2018
TLS SNI support enabled
configure arguments: 
--prefix=/usr/local/openresty/nginx --with-debug \
--with-cc-opt='-DNGX_LUA_USE_ASSERT -DNGX_LUA_ABORT_AT_PANIC -O2 -DNGX_LUA_ABORT_AT_PANIC \
-I/usr/local/openresty/zlib/include -I/usr/local/openresty/pcre/include \
-I/usr/local/openresty/openssl/include' \
--add-module=../ngx_devel_kit-0.3.0 --add-module=../echo-nginx-module-0.61 \
--add-module=../xss-nginx-module-0.06 --add-module=../ngx_coolkit-0.2rc3 \
--add-module=../set-misc-nginx-module-0.32 --add-module=../form-input-nginx-module-0.12 \
--add-module=../encrypted-session-nginx-module-0.08 --add-module=../srcache-nginx-module-0.31 \
--add-module=../ngx_lua-0.10.13 --add-module=../ngx_lua_upstream-0.07 \
--add-module=../headers-more-nginx-module-0.33 --add-module=../array-var-nginx-module-0.05 \
--add-module=../memc-nginx-module-0.19 --add-module=../redis2-nginx-module-0.15 \
--add-module=../redis-nginx-module-0.3.7 --add-module=../ngx_stream_lua-0.0.5 \
--with-ld-opt='-Wl,-rpath,/usr/local/openresty/luajit/lib -L/usr/local/openresty/zlib/lib -L/usr/local/openresty/pcre/lib \
-L/usr/local/openresty/openssl/lib -Wl,-rpath,/usr/local/openresty/zlib/lib:/usr/local/openresty/pcre/lib:/usr/local/openresty/openssl/lib' \
--sbin-path=/usr/sbin/nginx \
--conf-path=/etc/nginx/nginx.conf \
--pid-path=/var/run/nginx.pid \
--error-log-path=/var/log/nginx/error.log \
--lock-path=/var/lock/subsys/nginx \
--with-pcre-jit --with-ipv6 --with-stream \
--with-stream_ssl_module --with-stream_ssl_preread_module \
--with-http_v2_module --with-mail --with-mail_ssl_module \
--without-mail_pop3_module --without-mail_imap_module \
--without-mail_smtp_module --with-http_stub_status_module \
--with-http_realip_module --with-http_addition_module \
--with-http_xslt_module --with-http_image_filter_module \
--with-http_auth_request_module --with-http_secure_link_module \
--with-http_random_index_module --with-http_gzip_static_module \
--with-http_sub_module --with-http_dav_module --with-http_flv_module \
--with-http_mp4_module --with-http_gunzip_module --with-http_gzip_static_module \
--with-http_degradation_module --with-http_slice_module --with-http_stub_status_module \
--with-stream_realip_module --with-stream_geoip_module --with-stream_geoip_module=dynamic \
--with-stream_ssl_preread_module --with-http_geoip_module --with-http_geoip_module=dynamic \
--add-module=/root/rpmbuild/SOURCES/ngx_http_proxy_connect_module --with-select_module \
--with-poll_module --with-threads --with-file-aio --with-dtrace-probes --with-stream \
--with-stream_ssl_module --with-http_ssl_module
{% endhighlight %}


## 2. 源代码编译安装openresty


### 2.1 安装
**1) 下载源代码包**

这里我们安装```openresty-1.11.2.5```版本：
<pre>
# wget https://openresty.org/download/openresty-1.11.2.5.tar.gz
</pre>

该版本所使用的LuaJIT版本为v2.1-20170808

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

# make
# make install
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
其实在/usr/local/openresty/bin目录下建立了openresty的软链接，我们也可以使用如下命令来启动：
{% highlight string %}
# cd /usr/local/openresty
# ls bin/
total 148
drwxr-xr-x. 2 root root   123 Jul  2 16:30 .
drwxr-xr-x. 8 root root   117 Jul  2 16:30 ..
-rwxr-xr-x. 1 root root 19109 Jul  2 16:30 md2pod.pl
-rwxr-xr-x. 1 root root 15655 Jul  2 16:30 nginx-xml2pod
lrwxrwxrwx. 1 root root    37 Jul  2 16:30 openresty -> /usr/local/openresty/nginx/sbin/nginx
-rwxr-xr-x. 1 root root 63463 Jul  2 16:30 opm
-rwxr-xr-x. 1 root root 17165 Jul  2 16:30 resty
-rwxr-xr-x. 1 root root 14957 Jul  2 16:30 restydoc
-rwxr-xr-x. 1 root root  8452 Jul  2 16:30 restydoc-index

#pwd
/usr/local/openresty
# bin/openresty
{% endhighlight %}

3) 再接着执行如下命令进行简单测试
{% highlight string %}
# curl -XGET http://10.17.156.68
<p>hello, world</p>
{% endhighlight %}

可以看到上面打印出了lua脚本设置的```hello,world```。

### 2.4 luajit与lualib
我们进入/usr/local/openresy目录：
<pre>
# pwd
/usr/local/openresty
# ls
bin  COPYRIGHT  luajit  lualib  nginx  pod  resty.index  site
</pre>
发现该目录下有```luajit```和```lualib```两个文件夹。

1) **luajit**

LuaJIT是采用C语言写的Lua即时编译器。LuaJIT被设计成完全兼容Lua5.1标准，因此LuaJIT代码的语法和标准Lua的语法并没有多大的区别。打我们大概来看一下luajit这个目录：
{% highlight string %}
# tree luajit
luajit
├── bin
│   ├── luajit -> luajit-2.1.0-beta3
│   └── luajit-2.1.0-beta3
├── include
│   └── luajit-2.1
│       ├── lauxlib.h
│       ├── luaconf.h
│       ├── lua.h
│       ├── lua.hpp
│       ├── luajit.h
│       └── lualib.h
├── lib
│   ├── libluajit-5.1.a
│   ├── libluajit-5.1.so -> libluajit-5.1.so.2.1.0
│   ├── libluajit-5.1.so.2 -> libluajit-5.1.so.2.1.0
│   ├── libluajit-5.1.so.2.1.0
│   ├── lua
│   │   └── 5.1
│   └── pkgconfig
│       └── luajit.pc
└── share
    ├── lua
    │   └── 5.1
    ├── luajit-2.1.0-beta3
    │   └── jit
    │       ├── bc.lua
    │       ├── bcsave.lua
    │       ├── dis_arm64be.lua
    │       ├── dis_arm64.lua
    │       ├── dis_arm.lua
    │       ├── dis_mips64el.lua
    │       ├── dis_mips64.lua
    │       ├── dis_mipsel.lua
    │       ├── dis_mips.lua
    │       ├── dis_ppc.lua
    │       ├── dis_x64.lua
    │       ├── dis_x86.lua
    │       ├── dump.lua
    │       ├── p.lua
    │       ├── v.lua
    │       ├── vmdef.lua
    │       └── zone.lua
    └── man
        └── man1
            └── luajit.1
{% endhighlight %}
在OpenResty中就是采用该编译器来即时解析我们的lua脚本的。

2) **lualib**

有编译器原则上就可以执行符合相关标准的lua脚本了，但这里为什么还有一个lualib呢？举个例子，在C语言中，我们有gcc编译器，但是假如我们不使用posix C库的话，我们可能只能编写类似如下的最简单的程序(main.c)：
{% highlight string %}
int main(int argc, char *argv[])
{
        int a = 1;
        int b = 2;

        int c = a + b;

        return c;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o main main.c
# ./main
</pre>
如果我们不使用任何库，可能连最基本的```打印输出结果```都做不到。

这里lualib就是OpenResty为我们准备的一个基础lua库，我们后续编写的程序都可以基于该库来完成。现在我们大体看一下该基础库：
{% highlight string %}
# tree lualib/
lualib/
├── cjson.so
├── ngx
│   ├── balancer.lua
│   ├── errlog.lua
│   ├── ocsp.lua
│   ├── process.lua
│   ├── re.lua
│   ├── semaphore.lua
│   ├── ssl
│   │   └── session.lua
│   └── ssl.lua
├── rds
│   └── parser.so
├── redis
│   └── parser.so
└── resty
    ├── aes.lua
    ├── core
    │   ├── base64.lua
    │   ├── base.lua
    │   ├── ctx.lua
    │   ├── exit.lua
    │   ├── hash.lua
    │   ├── misc.lua
    │   ├── regex.lua
    │   ├── request.lua
    │   ├── response.lua
    │   ├── shdict.lua
    │   ├── time.lua
    │   ├── uri.lua
    │   ├── var.lua
    │   └── worker.lua
    ├── core.lua
    ├── dns
    │   └── resolver.lua
    ├── limit
    │   ├── conn.lua
    │   ├── req.lua
    │   └── traffic.lua
    ├── lock.lua
    ├── lrucache
    │   └── pureffi.lua
    ├── lrucache.lua
    ├── md5.lua
    ├── memcached.lua
    ├── mysql.lua
    ├── random.lua
    ├── redis.lua
    ├── sha1.lua
    ├── sha224.lua
    ├── sha256.lua
    ├── sha384.lua
    ├── sha512.lua
    ├── sha.lua
    ├── string.lua
    ├── upload.lua
    ├── upstream
    │   └── healthcheck.lua
    └── websocket
        ├── client.lua
        ├── protocol.lua
        └── server.lua
{% endhighlight %}
对于在该默认路径下的```基础lua库```，openresy可以自动找到，我们不必在nginx.conf的http配置段中通过如下命令来显式指定：
<pre>
lua_package_path "/usr/local/openresty/lualib/?.lua;;";
lua_package_cpath "/usr/local/openresty/lualib/?.so;;";
</pre>

下面我们来测试一下使用```基础lua库```中的string，修改nginx.conf:
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

    init_by_lua_block { str = require "resty.string" }

    server {
        listen       80;
        server_name  localhost;

        #charset koi8-r;

        #access_log  logs/host.access.log  main;

        location / {
            default_type text/html;
            content_by_lua '
                local s = str.to_hex("aaa")
                local s2 = string.sub("aaabbbccc",1,4)
                ngx.say("<p>hello, world</p>", "hex(aaa):", s, s2)
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
测试运行：
{% highlight string %}
# bin/openresty -s reload
# curl -X GET http://127.0.0.1
<p>hello, world</p>hex(aaa):616161aaab
{% endhighlight %}






<br />
<br />

**[参考]**

1. [OpenResty® Linux 包](http://openresty.org/cn/linux-packages.html)

2. [OpenResty官方网站](http://openresty.org/cn/)

3. [OpenResty installation](http://openresty.org/cn/installation.html)

4. [openresty github](https://github.com/openresty/lua-nginx-module/blob/master/src/ngx_http_lua_string.c)

5. [openresty的unescape_uri函数处理百分号后面字符的小特性](https://www.cnxct.com/openresty-unescape_uri-feature-to-decode-char-after-percent-sign/)

<br />
<br />
<br />





