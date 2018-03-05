---
layout: post
title: configure脚本分析(末)-part22
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---


configure脚本作为nginx配置的整体入口，在最先开始以此作为起点进行了初步的讲解。这里在我们完成了所有其他依赖脚本的分析之后，我们再对此脚本进行一个详细的分析。




<!-- more -->

## 1. configure脚本
脚本内容如下：
{% highlight string %}
#!/bin/sh

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


LC_ALL=C
export LC_ALL

. auto/options
. auto/init
. auto/sources

test -d $NGX_OBJS || mkdir -p $NGX_OBJS

echo > $NGX_AUTO_HEADERS_H
echo > $NGX_AUTOCONF_ERR

echo "#define NGX_CONFIGURE \"$NGX_CONFIGURE\"" > $NGX_AUTO_CONFIG_H


if [ $NGX_DEBUG = YES ]; then
    have=NGX_DEBUG . auto/have
fi


if test -z "$NGX_PLATFORM"; then
    echo "checking for OS"

    NGX_SYSTEM=`uname -s 2>/dev/null`
    NGX_RELEASE=`uname -r 2>/dev/null`
    NGX_MACHINE=`uname -m 2>/dev/null`

    echo " + $NGX_SYSTEM $NGX_RELEASE $NGX_MACHINE"

    NGX_PLATFORM="$NGX_SYSTEM:$NGX_RELEASE:$NGX_MACHINE";

    case "$NGX_SYSTEM" in
        MINGW32_*)
            NGX_PLATFORM=win32
        ;;
    esac

else
    echo "building for $NGX_PLATFORM"
    NGX_SYSTEM=$NGX_PLATFORM
fi

. auto/cc/conf

if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/headers
fi

. auto/os/conf

if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/unix
fi

. auto/threads
. auto/modules
. auto/lib/conf

case ".$NGX_PREFIX" in
    .)
        NGX_PREFIX=${NGX_PREFIX:-/usr/local/nginx}
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;

    .!)
        NGX_PREFIX=
    ;;

    *)
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;
esac

if [ ".$NGX_CONF_PREFIX" != "." ]; then
    have=NGX_CONF_PREFIX value="\"$NGX_CONF_PREFIX/\"" . auto/define
fi

have=NGX_SBIN_PATH value="\"$NGX_SBIN_PATH\"" . auto/define
have=NGX_CONF_PATH value="\"$NGX_CONF_PATH\"" . auto/define
have=NGX_PID_PATH value="\"$NGX_PID_PATH\"" . auto/define
have=NGX_LOCK_PATH value="\"$NGX_LOCK_PATH\"" . auto/define
have=NGX_ERROR_LOG_PATH value="\"$NGX_ERROR_LOG_PATH\"" . auto/define

have=NGX_HTTP_LOG_PATH value="\"$NGX_HTTP_LOG_PATH\"" . auto/define
have=NGX_HTTP_CLIENT_TEMP_PATH value="\"$NGX_HTTP_CLIENT_TEMP_PATH\""
. auto/define
have=NGX_HTTP_PROXY_TEMP_PATH value="\"$NGX_HTTP_PROXY_TEMP_PATH\""
. auto/define
have=NGX_HTTP_FASTCGI_TEMP_PATH value="\"$NGX_HTTP_FASTCGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_UWSGI_TEMP_PATH value="\"$NGX_HTTP_UWSGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_SCGI_TEMP_PATH value="\"$NGX_HTTP_SCGI_TEMP_PATH\""
. auto/define

. auto/make
. auto/lib/make
. auto/install

# STUB
. auto/stubs

have=NGX_USER value="\"$NGX_USER\"" . auto/define
have=NGX_GROUP value="\"$NGX_GROUP\"" . auto/define

if [ ".$NGX_BUILD" != "." ]; then
    have=NGX_BUILD value="\"$NGX_BUILD\"" . auto/define
fi

. auto/summary

{% endhighlight %}


**1) 初始化工作**
{% highlight string %}
LC_ALL=C
export LC_ALL

. auto/options
. auto/init
. auto/sources

test -d $NGX_OBJS || mkdir -p $NGX_OBJS

echo > $NGX_AUTO_HEADERS_H
echo > $NGX_AUTOCONF_ERR

echo "#define NGX_CONFIGURE \"$NGX_CONFIGURE\"" > $NGX_AUTO_CONFIG_H


if [ $NGX_DEBUG = YES ]; then
    have=NGX_DEBUG . auto/have
fi
{% endhighlight %}

这里,

* ```NGX_OBJS```: 在auto/options中被初始化为objs；

* ```NGX_AUTO_HEADERS_H```: 在auto/init中被初始化为objs/ngx_auto_headers.h;

* ```NGX_AUTOCONF_ERR```: 在auto/init中被初始化为objs/autoconf.err;

* ```NGX_CONFIGURE```: 被设置为
<pre>
#define NGX_CONFIGURE " --sbin-path=/usr/local/nginx/nginx --conf-path=/usr/local/nginx/nginx.conf --pid-path=/usr/local/nginx/nginx.pid --with-http_ssl_module --with-pcre=../pcre-8.40 --with-zlib=../zlib-1.2.11"
</pre>

* ```NGX_DEBUG```: 在auto/options中被初始化为```NO```.

<br />

**2) 获得当前Nginx运行平台**
{% highlight string %}
if test -z "$NGX_PLATFORM"; then
    echo "checking for OS"

    NGX_SYSTEM=`uname -s 2>/dev/null`
    NGX_RELEASE=`uname -r 2>/dev/null`
    NGX_MACHINE=`uname -m 2>/dev/null`

    echo " + $NGX_SYSTEM $NGX_RELEASE $NGX_MACHINE"

    NGX_PLATFORM="$NGX_SYSTEM:$NGX_RELEASE:$NGX_MACHINE";

    case "$NGX_SYSTEM" in
        MINGW32_*)
            NGX_PLATFORM=win32
        ;;
    esac

else
    echo "building for $NGX_PLATFORM"
    NGX_SYSTEM=$NGX_PLATFORM
fi

{% endhighlight %}

当前平台信息如下：
<pre>
# uname -s
Linux

# uname -r
4.10.0-35-generic

# uname -m
i686

# NGX_SYSTEM=`uname -s 2>/dev/null`
# NGX_RELEASE=`uname -r 2>/dev/null`
# NGX_MACHINE=`uname -m 2>/dev/null`
# echo " + $NGX_SYSTEM $NGX_RELEASE $NGX_MACHINE"
 + Linux 4.10.0-35-generic i686


# NGX_PLATFORM="$NGX_SYSTEM:$NGX_RELEASE:$NGX_MACHINE"
# echo $NGX_PLATFORM
Linux:4.10.0-35-generic:i686
</pre>


**3) 执行相关配置脚本**
{% highlight string %}
. auto/cc/conf

if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/headers
fi

. auto/os/conf

if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/unix
fi

. auto/threads
. auto/modules
. auto/lib/conf
{% endhighlight %}

<br />

**4) 处理路径相关**
{% highlight string %}
case ".$NGX_PREFIX" in
    .)
        NGX_PREFIX=${NGX_PREFIX:-/usr/local/nginx}
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;

    .!)
        NGX_PREFIX=
    ;;

    *)
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;
esac

if [ ".$NGX_CONF_PREFIX" != "." ]; then
    have=NGX_CONF_PREFIX value="\"$NGX_CONF_PREFIX/\"" . auto/define
fi

have=NGX_SBIN_PATH value="\"$NGX_SBIN_PATH\"" . auto/define
have=NGX_CONF_PATH value="\"$NGX_CONF_PATH\"" . auto/define
have=NGX_PID_PATH value="\"$NGX_PID_PATH\"" . auto/define
have=NGX_LOCK_PATH value="\"$NGX_LOCK_PATH\"" . auto/define
have=NGX_ERROR_LOG_PATH value="\"$NGX_ERROR_LOG_PATH\"" . auto/define

have=NGX_HTTP_LOG_PATH value="\"$NGX_HTTP_LOG_PATH\"" . auto/define
have=NGX_HTTP_CLIENT_TEMP_PATH value="\"$NGX_HTTP_CLIENT_TEMP_PATH\""
. auto/define
have=NGX_HTTP_PROXY_TEMP_PATH value="\"$NGX_HTTP_PROXY_TEMP_PATH\""
. auto/define
have=NGX_HTTP_FASTCGI_TEMP_PATH value="\"$NGX_HTTP_FASTCGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_UWSGI_TEMP_PATH value="\"$NGX_HTTP_UWSGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_SCGI_TEMP_PATH value="\"$NGX_HTTP_SCGI_TEMP_PATH\""
. auto/define
{% endhighlight %}

这里我们并未设置```NGX_PREFIX```，因此会设置默认值/usr/local/nginx; 然后会想objs/ngx_auto_config.h头文件定义如下宏：
<pre>
#ifndef NGX_PREFIX
#define NGX_PREFIX  "/usr/local/nginx/"
#endif


#ifndef NGX_CONF_PREFIX
#define NGX_CONF_PREFIX  "/usr/local/nginx/"
#endif


#ifndef NGX_SBIN_PATH
#define NGX_SBIN_PATH  "/usr/local/nginx/nginx"
#endif


#ifndef NGX_CONF_PATH
#define NGX_CONF_PATH  "/usr/local/nginx/nginx.conf"
#endif


#ifndef NGX_PID_PATH
#define NGX_PID_PATH  "/usr/local/nginx/nginx.pid"
#endif


#ifndef NGX_LOCK_PATH
#define NGX_LOCK_PATH  "logs/nginx.lock"
#endif


#ifndef NGX_ERROR_LOG_PATH
#define NGX_ERROR_LOG_PATH  "logs/error.log"
#endif


#ifndef NGX_HTTP_LOG_PATH
#define NGX_HTTP_LOG_PATH  "logs/access.log"
#endif


#ifndef NGX_HTTP_CLIENT_TEMP_PATH
#define NGX_HTTP_CLIENT_TEMP_PATH  "client_body_temp"
#endif


#ifndef NGX_HTTP_PROXY_TEMP_PATH
#define NGX_HTTP_PROXY_TEMP_PATH  "proxy_temp"
#endif


#ifndef NGX_HTTP_FASTCGI_TEMP_PATH
#define NGX_HTTP_FASTCGI_TEMP_PATH  "fastcgi_temp"
#endif


#ifndef NGX_HTTP_UWSGI_TEMP_PATH
#define NGX_HTTP_UWSGI_TEMP_PATH  "uwsgi_temp"
#endif


#ifndef NGX_HTTP_SCGI_TEMP_PATH
#define NGX_HTTP_SCGI_TEMP_PATH  "scgi_temp"
#endif
</pre>


**5) 其他**
{% highlight string %}
. auto/make
. auto/lib/make
. auto/install

# STUB
. auto/stubs

have=NGX_USER value="\"$NGX_USER\"" . auto/define
have=NGX_GROUP value="\"$NGX_GROUP\"" . auto/define

if [ ".$NGX_BUILD" != "." ]; then
    have=NGX_BUILD value="\"$NGX_BUILD\"" . auto/define
fi

. auto/summary
{% endhighlight %}

这里```NGX_USER```会在auto/unix脚本中设置为nobody; ```NGX_GROUP```会在auto/unix脚本中设置为nogroup; ```NGX_BUILD```值为空。







<br />
<br />
<br />

