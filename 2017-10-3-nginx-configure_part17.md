---
layout: post
title: auto/lib/conf脚本分析-part17
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

auto/lib/conf脚本主要用于配置一些nginx所依赖的一些外部库：


<!-- more -->

* PCRE库
* OpenSSL库
* MD5库
* sha1库
* zlib库
* xslt库
* LibGD库
* Perl库
* Geoip库
* Google Perftools库
* LibAtomic库

上面是Nginx当前所支持的一些依赖库配置，但是目前目前我们并没有用到全部。下面我们先对auto/lib/conf脚本进行简要分析，然后再分析一下当前我们所用到的一些库的配置。



## auto/lib/conf脚本
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


if [ $USE_PCRE = YES -o $PCRE != NONE ]; then
    . auto/lib/pcre/conf

else
    if [ $USE_PCRE = DISABLED -a $HTTP_REWRITE = YES ]; then

cat << END

$0: error: the HTTP rewrite module requires the PCRE library.
You can either disable the module by using --without-http_rewrite_module
option or you have to enable the PCRE support.

END
        exit 1
    fi
fi


if [ $USE_OPENSSL = YES ]; then
    . auto/lib/openssl/conf
fi

if [ $USE_MD5 = YES ]; then

    if [ $USE_OPENSSL = YES ]; then
        have=NGX_HAVE_OPENSSL_MD5_H . auto/have
        have=NGX_OPENSSL_MD5 . auto/have
        have=NGX_HAVE_MD5 . auto/have
        MD5=YES
        MD5_LIB=OpenSSL

    else
        . auto/lib/md5/conf
    fi

fi

if [ $USE_SHA1 = YES ]; then

    if [ $USE_OPENSSL = YES ]; then
        have=NGX_HAVE_OPENSSL_SHA1_H . auto/have
        have=NGX_HAVE_SHA1 . auto/have
        SHA1=YES
        SHA1_LIB=OpenSSL

    else
        . auto/lib/sha1/conf
    fi

fi

if [ $USE_ZLIB = YES ]; then
    . auto/lib/zlib/conf
fi

if [ $USE_LIBXSLT != NO ]; then
    . auto/lib/libxslt/conf
fi

if [ $USE_LIBGD != NO ]; then
    . auto/lib/libgd/conf
fi

if [ $USE_PERL != NO ]; then
    . auto/lib/perl/conf
fi

if [ $USE_GEOIP != NO ]; then
    . auto/lib/geoip/conf
fi

if [ $NGX_GOOGLE_PERFTOOLS = YES ]; then
    . auto/lib/google-perftools/conf
fi

if [ $NGX_LIBATOMIC != NO ]; then
    . auto/lib/libatomic/conf
fi

{% endhighlight %}

**1) PCRE库**

在我们配置configure脚本时通过```--with-pcre```启用了```PCRE```,因此这里会调用auto/lib/pcre/conf。

**2) OpenSSL库**

我们通过```--with-http_ssl_module```会启用```HTTP_SSL```，从而```USE_OPENSSL```会置为```YES```，因此这里会调用auto/lib/openssl/conf。

**3) MD5库**

auto/options脚本中默认启用```HTTP_CACHE```，从而导致auto/modules中会启用```USE_MD5```，同时又因为```USE_OPENSSL```值为```YES```，因此会采用OpenSSL中的MD5。

**4) SHA1库**

在auto/options中```HTTP_AUTH_BASIC```默认值为```YES```，因此会导致在auto/modules脚本中启用```USE_SHA1```。这里也会使用OpenSSL中的SHA1.

**5) Zlib库**

在auto/options脚本中```HTTP_GZIP```默认值为```YES```，因此会导致在auto/modules脚本中启用```USE_ZLIB```.


**6) XSLT库**

```USE_LIBXSLT```并不会被启用。

**7) LibGD库**

```USE_LIBGD```并不会被启用。

**8) Perl库**

```USE_PERL```并不会被启用。

**9) GeoIP库**

```USE_GEOIP```并不会被启用。

**10) Google PerfTools库**

```NGX_GOOGLE_PERFTOOLS```并不会被启用。

**11) libatomic库**

```NGX_LIBATOMIC```并不会被启用。


<br />
<br />
<br />

