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

上面是Nginx当前所支持的一些依赖库配置，但是目前我们并没有用到全部。下面我们先对auto/lib/conf脚本进行简要分析，然后再分析一下当前我们所用到的一些库的配置。



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

在我们配置configure脚本时通过```--with-pcre=*```启用了```PCRE```,然后在auto/modules中通过```HTTP_REWRITE```会启用```USE_PCRE```,因此这里会调用auto/lib/pcre/conf。

**2) OpenSSL库**

我们通过```--with-http_ssl_module```会启用```HTTP_SSL```，从而```USE_OPENSSL```会置为```YES```，因此这里会调用auto/lib/openssl/conf。

**3) MD5库**

auto/options脚本中默认启用```HTTP_CACHE```，从而导致auto/modules中会启用```USE_MD5```，同时又因为```USE_OPENSSL```值为```YES```，因此会采用OpenSSL中的MD5。

**4) SHA1库**

在auto/options中```HTTP_AUTH_BASIC```默认值为```YES```，因此会导致在auto/modules脚本中启用```USE_SHA1```。这里也会使用OpenSSL中的SHA1.

**5) Zlib库**

在auto/options脚本中```HTTP_GZIP```默认值为```YES```，因此会导致在auto/modules脚本中启用```USE_ZLIB```，因此这里会调用auto/lib/zlib/conf.


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



## 2. auto/lib/pcre/conf脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


if [ $PCRE != NONE ]; then
    CORE_INCS="$CORE_INCS $PCRE"

    case "$NGX_CC_NAME" in

        msvc | owc | bcc)
            have=NGX_PCRE . auto/have
            have=PCRE_STATIC . auto/have
            CORE_DEPS="$CORE_DEPS $PCRE/pcre.h"
            LINK_DEPS="$LINK_DEPS $PCRE/pcre.lib"
            CORE_LIBS="$CORE_LIBS $PCRE/pcre.lib"
        ;;

        icc)
            have=NGX_PCRE . auto/have
            CORE_DEPS="$CORE_DEPS $PCRE/pcre.h"

            LINK_DEPS="$LINK_DEPS $PCRE/.libs/libpcre.a"

            echo $ngx_n "checking for PCRE library ...$ngx_c"

            if [ -f $PCRE/pcre.h ]; then
                ngx_pcre_ver=`grep PCRE_MAJOR $PCRE/pcre.h \
                              | sed -e 's/^.*PCRE_MAJOR.* \(.*\)$/\1/'`

            else if [ -f $PCRE/configure.in ]; then
                ngx_pcre_ver=`grep PCRE_MAJOR= $PCRE/configure.in \
                              | sed -e 's/^.*=\(.*\)$/\1/'`

            else
                ngx_pcre_ver=`grep pcre_major, $PCRE/configure.ac \
                              | sed -e 's/^.*pcre_major,.*\[\(.*\)\].*$/\1/'`
            fi
            fi

            echo " $ngx_pcre_ver major version found"

            # to allow -ipo optimization we link with the *.o but not library

            case "$ngx_pcre_ver" in
                4|5)
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre.o"
                ;;

                6)
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_chartables.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_compile.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_exec.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_fullinfo.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_globals.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_tables.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_try_flipped.o"
                ;;

                *)
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_chartables.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_compile.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_exec.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_fullinfo.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_globals.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_tables.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_try_flipped.o"
                    CORE_LIBS="$CORE_LIBS $PCRE/pcre_newline.o"
                ;;

            esac
        ;;

        *)
            have=NGX_PCRE . auto/have

            if [ "$NGX_PLATFORM" = win32 ]; then
                have=PCRE_STATIC . auto/have
            fi

            CORE_DEPS="$CORE_DEPS $PCRE/pcre.h"
            LINK_DEPS="$LINK_DEPS $PCRE/.libs/libpcre.a"
            CORE_LIBS="$CORE_LIBS $PCRE/.libs/libpcre.a"
        ;;

    esac


    if [ $PCRE_JIT = YES ]; then
        have=NGX_HAVE_PCRE_JIT . auto/have
        PCRE_CONF_OPT="$PCRE_CONF_OPT --enable-jit"
    fi

else

    if [ "$NGX_PLATFORM" != win32 ]; then

        PCRE=NO

        ngx_feature="PCRE library"
        ngx_feature_name="NGX_PCRE"
        ngx_feature_run=no
        ngx_feature_incs="#include <pcre.h>"
        ngx_feature_path=
        ngx_feature_libs="-lpcre"
        ngx_feature_test="pcre *re;
                          re = pcre_compile(NULL, 0, NULL, 0, NULL);
                          if (re == NULL) return 1"
        . auto/feature

        if [ $ngx_found = no ]; then

            # FreeBSD port

            ngx_feature="PCRE library in /usr/local/"
            ngx_feature_path="/usr/local/include"

            if [ $NGX_RPATH = YES ]; then
                ngx_feature_libs="-R/usr/local/lib -L/usr/local/lib -lpcre"
            else
                ngx_feature_libs="-L/usr/local/lib -lpcre"
            fi

            . auto/feature
        fi

        if [ $ngx_found = no ]; then

            # RedHat RPM, Solaris package

            ngx_feature="PCRE library in /usr/include/pcre/"
            ngx_feature_path="/usr/include/pcre"
            ngx_feature_libs="-lpcre"

            . auto/feature
        fi

        if [ $ngx_found = no ]; then

            # NetBSD port

            ngx_feature="PCRE library in /usr/pkg/"
            ngx_feature_path="/usr/pkg/include"

            if [ $NGX_RPATH = YES ]; then
                ngx_feature_libs="-R/usr/pkg/lib -L/usr/pkg/lib -lpcre"
            else
                ngx_feature_libs="-L/usr/pkg/lib -lpcre"
            fi

            . auto/feature
        fi

        if [ $ngx_found = no ]; then

            # MacPorts

            ngx_feature="PCRE library in /opt/local/"
            ngx_feature_path="/opt/local/include"

            if [ $NGX_RPATH = YES ]; then
                ngx_feature_libs="-R/opt/local/lib -L/opt/local/lib -lpcre"
            else
                ngx_feature_libs="-L/opt/local/lib -lpcre"
            fi

            . auto/feature
        fi

        if [ $ngx_found = yes ]; then
            CORE_INCS="$CORE_INCS $ngx_feature_path"
            CORE_LIBS="$CORE_LIBS $ngx_feature_libs"
            PCRE=YES
        fi

        if [ $PCRE = YES ]; then
            ngx_feature="PCRE JIT support"
            ngx_feature_name="NGX_HAVE_PCRE_JIT"
            ngx_feature_test="int jit = 0;
                              pcre_free_study(NULL);
                              pcre_config(PCRE_CONFIG_JIT, &jit);
                              if (jit != 1) return 1;"
            . auto/feature

            if [ $ngx_found = yes ]; then
                PCRE_JIT=YES
            fi
        fi
    fi

    if [ $PCRE != YES ]; then
cat << END

$0: error: the HTTP rewrite module requires the PCRE library.
You can either disable the module by using --without-http_rewrite_module
option, or install the PCRE library into the system, or build the PCRE library
statically from the source with nginx by using --with-pcre=<path> option.

END
        exit 1
    fi

fi

{% endhighlight %}

这里```PCRE```值为```../pcre-8.40```，因此执行第一个if条件分支。```NGX_CC_NAME```为gcc,因此执行如下：
{% highlight string %}
*)
    have=NGX_PCRE . auto/have

    if [ "$NGX_PLATFORM" = win32 ]; then
        have=PCRE_STATIC . auto/have
    fi

    CORE_DEPS="$CORE_DEPS $PCRE/pcre.h"
    LINK_DEPS="$LINK_DEPS $PCRE/.libs/libpcre.a"
    CORE_LIBS="$CORE_LIBS $PCRE/.libs/libpcre.a"
;;
{% endhighlight %}
将对应的pcre头文件、库文件添加到相应的变量中。

```PCRE_JIT```值为```NO```，因此并没有开启pcre jit特性。

## 3. auto/lib/openssl/conf脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


if [ $OPENSSL != NONE ]; then

    case "$CC" in

        cl | bcc32)
            have=NGX_OPENSSL . auto/have
            have=NGX_SSL . auto/have

            CFLAGS="$CFLAGS -DNO_SYS_TYPES_H"

            CORE_INCS="$CORE_INCS $OPENSSL/openssl/include"
            CORE_DEPS="$CORE_DEPS $OPENSSL/openssl/include/openssl/ssl.h"
            CORE_LIBS="$CORE_LIBS $OPENSSL/openssl/lib/ssleay32.lib"
            CORE_LIBS="$CORE_LIBS $OPENSSL/openssl/lib/libeay32.lib"

            # libeay32.lib requires gdi32.lib
            CORE_LIBS="$CORE_LIBS gdi32.lib"
            # OpenSSL 1.0.0 requires crypt32.lib
            CORE_LIBS="$CORE_LIBS crypt32.lib"
        ;;

        *)
            have=NGX_OPENSSL . auto/have
            have=NGX_SSL . auto/have

            CORE_INCS="$CORE_INCS $OPENSSL/.openssl/include"
            CORE_DEPS="$CORE_DEPS $OPENSSL/.openssl/include/openssl/ssl.h"
            CORE_LIBS="$CORE_LIBS $OPENSSL/.openssl/lib/libssl.a"
            CORE_LIBS="$CORE_LIBS $OPENSSL/.openssl/lib/libcrypto.a"
            CORE_LIBS="$CORE_LIBS $NGX_LIBDL"

            if [ "$NGX_PLATFORM" = win32 ]; then
                CORE_LIBS="$CORE_LIBS -lgdi32 -lcrypt32 -lws2_32"
            fi
        ;;
    esac

else

    if [ "$NGX_PLATFORM" != win32 ]; then

        OPENSSL=NO

        ngx_feature="OpenSSL library"
        ngx_feature_name="NGX_OPENSSL"
        ngx_feature_run=no
        ngx_feature_incs="#include <openssl/ssl.h>"
        ngx_feature_path=
        ngx_feature_libs="-lssl -lcrypto $NGX_LIBDL"
        ngx_feature_test="SSL_CTX_set_options(NULL, 0)"
        . auto/feature

        if [ $ngx_found = no ]; then

            # FreeBSD port

            ngx_feature="OpenSSL library in /usr/local/"
            ngx_feature_path="/usr/local/include"

            if [ $NGX_RPATH = YES ]; then
                ngx_feature_libs="-R/usr/local/lib -L/usr/local/lib -lssl -lcrypto $NGX_LIBDL"
            else
                ngx_feature_libs="-L/usr/local/lib -lssl -lcrypto $NGX_LIBDL"
            fi

            . auto/feature
        fi

        if [ $ngx_found = no ]; then

            # NetBSD port

            ngx_feature="OpenSSL library in /usr/pkg/"
            ngx_feature_path="/usr/pkg/include"

            if [ $NGX_RPATH = YES ]; then
                ngx_feature_libs="-R/usr/pkg/lib -L/usr/pkg/lib -lssl -lcrypto $NGX_LIBDL"
            else
                ngx_feature_libs="-L/usr/pkg/lib -lssl -lcrypto $NGX_LIBDL"
            fi

            . auto/feature
        fi

        if [ $ngx_found = no ]; then

            # MacPorts

            ngx_feature="OpenSSL library in /opt/local/"
            ngx_feature_path="/opt/local/include"

            if [ $NGX_RPATH = YES ]; then
                ngx_feature_libs="-R/opt/local/lib -L/opt/local/lib -lssl -lcrypto $NGX_LIBDL"
            else
                ngx_feature_libs="-L/opt/local/lib -lssl -lcrypto $NGX_LIBDL"
            fi

            . auto/feature
        fi

        if [ $ngx_found = yes ]; then
            have=NGX_SSL . auto/have
            CORE_INCS="$CORE_INCS $ngx_feature_path"
            CORE_LIBS="$CORE_LIBS $ngx_feature_libs"
            OPENSSL=YES
        fi
    fi

    if [ $OPENSSL != YES ]; then

cat << END

$0: error: SSL modules require the OpenSSL library.
You can either do not enable the modules, or install the OpenSSL library
into the system, or build the OpenSSL library statically from the source
with nginx by using --with-openssl=<path> option.

END
        exit 1
    fi

fi

{% endhighlight %}


此处我们虽然通过```--with-http_ssl_module```以间接的方式启用了```USE_OPENSSL```，但是```OPENSSL```的值仍为```NONE```。我们这里是采用自动链接我们事先手动安装的openssl库的方式。因此这里执行if条件的else分支：
{% highlight string %}
OPENSSL=NO

ngx_feature="OpenSSL library"
ngx_feature_name="NGX_OPENSSL"
ngx_feature_run=no
ngx_feature_incs="#include <openssl/ssl.h>"
ngx_feature_path=
ngx_feature_libs="-lssl -lcrypto $NGX_LIBDL"
ngx_feature_test="SSL_CTX_set_options(NULL, 0)"
. auto/feature
{% endhighlight %}
将```OPENSSL```置为```NO```，然后调用auto/feature检测当前系统是否有openssl特性(此外这里会根据一些特殊的操作系统进行特别查找)。如果没有找到，则直接报错退出；否则执行如下：
{% highlight string %}
if [ $ngx_found = yes ]; then
    have=NGX_SSL . auto/have
    CORE_INCS="$CORE_INCS $ngx_feature_path"
    CORE_LIBS="$CORE_LIBS $ngx_feature_libs"
    OPENSSL=YES
fi
{% endhighlight %}
将相应的头文件、库文件保存到对应的变量中，同时将```OPENSSL```置为```YES```。

## 4. MD5库
这里在auto/lib/conf脚本中，采用的是Openssl中的MD5库：
{% highlight string %}
have=NGX_HAVE_OPENSSL_MD5_H . auto/have
have=NGX_OPENSSL_MD5 . auto/have
have=NGX_HAVE_MD5 . auto/have
MD5=YES
MD5_LIB=OpenSSL
{% endhighlight %}


## 5. SHA1库
这里在auto/lib/conf脚本中，采用的是Openssl中的Sha1库：
{% highlight string %}
have=NGX_HAVE_OPENSSL_SHA1_H . auto/have
have=NGX_HAVE_SHA1 . auto/have
SHA1=YES
SHA1_LIB=OpenSSL
{% endhighlight %}

## 6. auto/lib/zlib/conf脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


if [ $ZLIB != NONE ]; then
    CORE_INCS="$CORE_INCS $ZLIB"

    case "$NGX_CC_NAME" in

        msvc | owc | bcc)
            have=NGX_ZLIB . auto/have
            LINK_DEPS="$LINK_DEPS $ZLIB/zlib.lib"
            CORE_LIBS="$CORE_LIBS $ZLIB/zlib.lib"
        ;;

        icc)
            have=NGX_ZLIB . auto/have
            LINK_DEPS="$LINK_DEPS $ZLIB/libz.a"

            # to allow -ipo optimization we link with the *.o but not library
            CORE_LIBS="$CORE_LIBS $ZLIB/adler32.o"
            CORE_LIBS="$CORE_LIBS $ZLIB/crc32.o"
            CORE_LIBS="$CORE_LIBS $ZLIB/deflate.o"
            CORE_LIBS="$CORE_LIBS $ZLIB/trees.o"
            CORE_LIBS="$CORE_LIBS $ZLIB/zutil.o"
            CORE_LIBS="$CORE_LIBS $ZLIB/compress.o"

            if [ $ZLIB_ASM != NO ]; then
                CORE_LIBS="$CORE_LIBS $ZLIB/match.o"
            fi
        ;;

        *)
            have=NGX_ZLIB . auto/have
            LINK_DEPS="$LINK_DEPS $ZLIB/libz.a"
            CORE_LIBS="$CORE_LIBS $ZLIB/libz.a"
            #CORE_LIBS="$CORE_LIBS -L $ZLIB -lz"
        ;;

    esac

else

    if [ "$NGX_PLATFORM" != win32 ]; then
        ZLIB=NO

        # FreeBSD, Solaris, Linux

        ngx_feature="zlib library"
        ngx_feature_name="NGX_ZLIB"
        ngx_feature_run=no
        ngx_feature_incs="#include <zlib.h>"
        ngx_feature_path=
        ngx_feature_libs="-lz"
        ngx_feature_test="z_stream z; deflate(&z, Z_NO_FLUSH)"
        . auto/feature


        if [ $ngx_found = yes ]; then
            CORE_LIBS="$CORE_LIBS $ngx_feature_libs"
            ZLIB=YES
            ngx_found=no
        fi
    fi

    if [ $ZLIB != YES ]; then
cat << END

$0: error: the HTTP gzip module requires the zlib library.
You can either disable the module by using --without-http_gzip_module
option, or install the zlib library into the system, or build the zlib library
statically from the source with nginx by using --with-zlib=<path> option.

END
        exit 1
    fi

fi

{% endhighlight %}
在auto/options脚本中通过```--with-zlib=*```将```ZLIB```值置为了```../zlib-1.2.11```，因此会执行if条件的第一个分支,将```ZLIB```头文件保存:
{% highlight string %}
CORE_INCS="$CORE_INCS $ZLIB"
{% endhighlight %}

这里```NGX_CC_NAME```为gcc，因此执行如下：
{% highlight string %}
have=NGX_ZLIB . auto/have
LINK_DEPS="$LINK_DEPS $ZLIB/libz.a"
CORE_LIBS="$CORE_LIBS $ZLIB/libz.a"
#CORE_LIBS="$CORE_LIBS -L $ZLIB -lz"
{% endhighlight %}

将对应的库文件保存到相应变量中。


<br />
<br />
<br />

