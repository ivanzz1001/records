---
layout: post
title: auto/stubs及auto/summary脚本分析-part21
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---


本章主要介绍一下auto/stubs及auto/summary两个脚本。前一个脚本要定义两个宏；后一个主要是打印出相关的一些重要信息。




<!-- more -->

## 1. auto/stubs脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


have=NGX_SUPPRESS_WARN . auto/have

have=NGX_SMP . auto/have

{% endhighlight %}

向```$NGX_AUTO_CONFIG_H```头文件(这里为objs/ngx_auto_config.h)中写入如下宏定义：
<pre>
#ifndef NGX_SUPPRESS_WARN
#define NGX_SUPPRESS_WARN  1
#endif

#ifndef NGX_SMP
#define NGX_SMP  1
#endif
</pre>

前一个宏定义表示```抑制相应警告信息```;后一个主要用于多处理器原子锁.



## 2. auto/summary脚本

脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


echo
echo "Configuration summary"


if [ $USE_THREADS = YES ]; then
    echo "  + using threads"
fi

if [ $USE_PCRE = DISABLED ]; then
    echo "  + PCRE library is disabled"

else
    case $PCRE in
        YES)   echo "  + using system PCRE library" ;;
        NONE)  echo "  + PCRE library is not used" ;;
        *)     echo "  + using PCRE library: $PCRE" ;;
    esac
fi

case $OPENSSL in
    YES)   echo "  + using system OpenSSL library" ;;
    NONE)  echo "  + OpenSSL library is not used" ;;
    *)     echo "  + using OpenSSL library: $OPENSSL" ;;
esac

case $MD5 in
    YES)   echo "  + md5: using $MD5_LIB library" ;;
    NONE)  echo "  + md5 library is not used" ;;
    NO)    echo "  + using builtin md5 code" ;;
    *)     echo "  + using md5 library: $MD5" ;;
esac

case $SHA1 in
    YES)   echo "  + sha1: using $SHA1_LIB library" ;;
    NONE)  echo "  + sha1 library is not used" ;;
    NO)    echo "  + sha1 library is not found" ;;
    *)     echo "  + using sha1 library: $SHA1" ;;
esac

case $ZLIB in
    YES)   echo "  + using system zlib library" ;;
    NONE)  echo "  + zlib library is not used" ;;
    *)     echo "  + using zlib library: $ZLIB" ;;
esac

case $NGX_LIBATOMIC in
    YES)   echo "  + using system libatomic_ops library" ;;
    NO)    ;; # not used
    *)     echo "  + using libatomic_ops library: $NGX_LIBATOMIC" ;;
esac

echo


cat << END
  nginx path prefix: "$NGX_PREFIX"
  nginx binary file: "$NGX_SBIN_PATH"
  nginx modules path: "$NGX_MODULES_PATH"
  nginx configuration prefix: "$NGX_CONF_PREFIX"
  nginx configuration file: "$NGX_CONF_PATH"
  nginx pid file: "$NGX_PID_PATH"
END

if test -n "$NGX_ERROR_LOG_PATH"; then
    echo "  nginx error log file: \"$NGX_ERROR_LOG_PATH\""
else
    echo "  nginx logs errors to stderr"
fi

cat << END
  nginx http access log file: "$NGX_HTTP_LOG_PATH"
  nginx http client request body temporary files: "$NGX_HTTP_CLIENT_TEMP_PATH"
END

if [ $HTTP_PROXY = YES ]; then
    echo "  nginx http proxy temporary files: \"$NGX_HTTP_PROXY_TEMP_PATH\""
fi

if [ $HTTP_FASTCGI = YES ]; then
    echo "  nginx http fastcgi temporary files: \"$NGX_HTTP_FASTCGI_TEMP_PATH\""
fi

if [ $HTTP_UWSGI = YES ]; then
    echo "  nginx http uwsgi temporary files: \"$NGX_HTTP_UWSGI_TEMP_PATH\""
fi

if [ $HTTP_SCGI = YES ]; then
    echo "  nginx http scgi temporary files: \"$NGX_HTTP_SCGI_TEMP_PATH\""
fi

echo "$NGX_POST_CONF_MSG"

{% endhighlight %}

这里:

* USE_THREADS:在auto/options脚本中初始设置为```NO```
* PCRE: 被设置为```--with-pcre=../pcre-8.40```
* OPENSSL: 被设置为使用系统Openssl
* MD5：被设置为```YES```，使用Openssl库中的MD5,```MD5_LIB```在auto/lib/conf脚本中被设置为OpenSSL.
* SHA1： 被设置为```YES```，使用Openssl库中的SHA1,```SHA1_LIB```在auto/lib/conf脚本中被设置为OpenSSL.
* ZLIB： 被设置为```--with-zlib=../zlib-1.2.11```
* NGX_LIBATOMIC: 值为```NO```，并未使用

<br />

然后执行如下命令：
{% highlight string %}
cat << END
  nginx path prefix: "$NGX_PREFIX"
  nginx binary file: "$NGX_SBIN_PATH"
  nginx modules path: "$NGX_MODULES_PATH"
  nginx configuration prefix: "$NGX_CONF_PREFIX"
  nginx configuration file: "$NGX_CONF_PATH"
  nginx pid file: "$NGX_PID_PATH"
END

{% endhighlight %}
打印出路径相关信息：
<pre>
NGX_PREFIX: /usr/local/nginx
NGX_SBIN_PATH: /usr/local/nginx/nginx
NGX_MODULES_PATH:/usr/local/nginx/modules
NGX_CONF_PREFIX: (empty)
NGX_CONF_PATH: /usr/local/nginx/nginx.conf
NGX_PID_PATH: /usr/local/nginx/nginx.pid


NGX_ERROR_LOG_PATH: /usr/local/nginx/logs/error.log

NGX_HTTP_LOG_PATH: /usr/local/nginx/logs/access.log

NGX_HTTP_CLIENT_TEMP_PATH: client_body_temp


HTTP_PROXY: 在auto/options脚本中默认设置为YES, NGX_HTTP_PROXY_TEMP_PATH使用默认值proxy_temp

HTTP_FASTCGI: 在auto/options脚本中默认设置为YES, NGX_HTTP_FASTCGI_TEMP_PATH使用默认值fastcgi_temp。

HTTP_UWSGI: 在auto/options脚本中默认设置为YES, NGX_HTTP_UWSGI_TEMP_PATH使用默认值uwsgi_temp。

HTTP_SCGI: 在auto/options脚本中默认设置为YES, NGX_HTTP_SCGI_TEMP_PATH使用默认值scgi_temp。

NGX_POST_CONF_MSG： (empty)
</pre>

最后给出一个整体输出结果，以作参考：
{% highlight string %}
Configuration summary
  + using PCRE library: ../pcre-8.40
  + using system OpenSSL library
  + md5: using OpenSSL library
  + sha1: using OpenSSL library
  + using zlib library: ../zlib-1.2.11

  nginx path prefix: "/usr/local/nginx"
  nginx binary file: "/usr/local/nginx/nginx"
  nginx modules path: "/usr/local/nginx/modules"
  nginx configuration prefix: "/usr/local/nginx"
  nginx configuration file: "/usr/local/nginx/nginx.conf"
  nginx pid file: "/usr/local/nginx/nginx.pid"
  nginx error log file: "/usr/local/nginx/logs/error.log"
  nginx http access log file: "/usr/local/nginx/logs/access.log"
  nginx http client request body temporary files: "client_body_temp"
  nginx http proxy temporary files: "proxy_temp"
  nginx http fastcgi temporary files: "fastcgi_temp"
  nginx http uwsgi temporary files: "uwsgi_temp"
  nginx http scgi temporary files: "scgi_temp"
{% endhighlight %}



<br />
<br />
参考：

1. [Nginx工作原理和优化](http://www.cnblogs.com/linguoguo/p/5511293.html)

2. [【linux】spinlock 的实现](http://www.cnblogs.com/chenpingzhao/archive/2015/12/13/5043746.html)
<br />
<br />
<br />

