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





<br />
<br />
<br />

