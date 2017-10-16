---
layout: post
title: auto/options脚本解析-part2
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

接上一章，此文我们来解析auto/options脚本。此脚本主要用来设定配置选项。

<br />
<br />

<!-- more -->
参看：

1) ```http://blog.csdn.net/poechant/article/details/7327206```

2) ```http://man.linuxde.net/sed```


## 1. auto/options脚本

在分析脚本之前，我们这里贴出其源代码：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


help=no

NGX_PREFIX=
NGX_SBIN_PATH=
NGX_MODULES_PATH=
NGX_CONF_PREFIX=
NGX_CONF_PATH=
NGX_ERROR_LOG_PATH=
NGX_PID_PATH=
NGX_LOCK_PATH=
NGX_USER=
NGX_GROUP=
NGX_BUILD=

CC=${CC:-cc}
CPP=
NGX_OBJS=objs

NGX_DEBUG=NO
NGX_CC_OPT=
NGX_LD_OPT=
CPU=NO

NGX_RPATH=NO

NGX_TEST_BUILD_DEVPOLL=NO
NGX_TEST_BUILD_EVENTPORT=NO
NGX_TEST_BUILD_EPOLL=NO
NGX_TEST_BUILD_SOLARIS_SENDFILEV=NO

NGX_PLATFORM=
NGX_WINE=

EVENT_FOUND=NO

EVENT_SELECT=NO
EVENT_POLL=NO

USE_THREADS=NO

NGX_FILE_AIO=NO
NGX_IPV6=NO

HTTP=YES

NGX_HTTP_LOG_PATH=
NGX_HTTP_CLIENT_TEMP_PATH=
NGX_HTTP_PROXY_TEMP_PATH=
NGX_HTTP_FASTCGI_TEMP_PATH=
NGX_HTTP_UWSGI_TEMP_PATH=
NGX_HTTP_SCGI_TEMP_PATH=

HTTP_CACHE=YES
HTTP_CHARSET=YES
HTTP_GZIP=YES
HTTP_SSL=NO
HTTP_V2=NO
HTTP_SSI=YES
HTTP_POSTPONE=NO
HTTP_REALIP=NO
HTTP_XSLT=NO
HTTP_IMAGE_FILTER=NO
HTTP_SUB=NO
HTTP_ADDITION=NO
HTTP_DAV=NO
HTTP_ACCESS=YES
HTTP_AUTH_BASIC=YES
HTTP_AUTH_REQUEST=NO
HTTP_USERID=YES
HTTP_SLICE=NO
HTTP_AUTOINDEX=YES
HTTP_RANDOM_INDEX=NO
HTTP_STATUS=NO
HTTP_GEO=YES
HTTP_GEOIP=NO
HTTP_MAP=YES
HTTP_SPLIT_CLIENTS=YES
HTTP_REFERER=YES
HTTP_REWRITE=YES
HTTP_PROXY=YES
HTTP_FASTCGI=YES
HTTP_UWSGI=YES
HTTP_SCGI=YES
HTTP_PERL=NO
HTTP_MEMCACHED=YES
HTTP_LIMIT_CONN=YES
HTTP_LIMIT_REQ=YES
HTTP_EMPTY_GIF=YES
HTTP_BROWSER=YES
HTTP_SECURE_LINK=NO
HTTP_DEGRADATION=NO
HTTP_FLV=NO
HTTP_MP4=NO
HTTP_GUNZIP=NO
HTTP_GZIP_STATIC=NO
HTTP_UPSTREAM_HASH=YES
HTTP_UPSTREAM_IP_HASH=YES
HTTP_UPSTREAM_LEAST_CONN=YES
HTTP_UPSTREAM_KEEPALIVE=YES
HTTP_UPSTREAM_ZONE=YES

# STUB
HTTP_STUB_STATUS=NO

MAIL=NO
MAIL_SSL=NO
MAIL_POP3=YES
MAIL_IMAP=YES
MAIL_SMTP=YES

STREAM=NO
STREAM_SSL=NO
STREAM_LIMIT_CONN=YES
STREAM_ACCESS=YES
STREAM_UPSTREAM_HASH=YES
STREAM_UPSTREAM_LEAST_CONN=YES
STREAM_UPSTREAM_ZONE=YES

DYNAMIC_MODULES=

NGX_ADDONS=
NGX_ADDON_DEPS=
DYNAMIC_ADDONS=

USE_PCRE=NO
PCRE=NONE
PCRE_OPT=
PCRE_CONF_OPT=
PCRE_JIT=NO

USE_OPENSSL=NO
OPENSSL=NONE

USE_MD5=NO
MD5=NONE
MD5_OPT=
MD5_ASM=NO

USE_SHA1=NO
SHA1=NONE
SHA1_OPT=
SHA1_ASM=NO

USE_ZLIB=NO
ZLIB=NONE
ZLIB_OPT=
ZLIB_ASM=NO

USE_PERL=NO
NGX_PERL=perl

USE_LIBXSLT=NO
USE_LIBGD=NO
USE_GEOIP=NO

NGX_GOOGLE_PERFTOOLS=NO
NGX_CPP_TEST=NO

NGX_LIBATOMIC=NO

NGX_CPU_CACHE_LINE=

NGX_POST_CONF_MSG=

opt=

for option
do
    opt="$opt `echo $option | sed -e \"s/\(--[^=]*=\)\(.* .*\)/\1'\2'/\"`"

    case "$option" in
        -*=*) value=`echo "$option" | sed -e 's/[-_a-zA-Z0-9]*=//'` ;;
           *) value="" ;;
    esac

    case "$option" in
        --help)                          help=yes                   ;;

        --prefix=)                       NGX_PREFIX="!"             ;;
        --prefix=*)                      NGX_PREFIX="$value"        ;;
        --sbin-path=*)                   NGX_SBIN_PATH="$value"     ;;
        --modules-path=*)                NGX_MODULES_PATH="$value"  ;;
        --conf-path=*)                   NGX_CONF_PATH="$value"     ;;
        --error-log-path=*)              NGX_ERROR_LOG_PATH="$value";;
        --pid-path=*)                    NGX_PID_PATH="$value"      ;;
        --lock-path=*)                   NGX_LOCK_PATH="$value"     ;;
        --user=*)                        NGX_USER="$value"          ;;
        --group=*)                       NGX_GROUP="$value"         ;;

        --crossbuild=*)                  NGX_PLATFORM="$value"      ;;

        --build=*)                       NGX_BUILD="$value"         ;;
        --builddir=*)                    NGX_OBJS="$value"          ;;

        --with-select_module)            EVENT_SELECT=YES           ;;
        --without-select_module)         EVENT_SELECT=NONE          ;;
        --with-poll_module)              EVENT_POLL=YES             ;;
        --without-poll_module)           EVENT_POLL=NONE            ;;

        --with-threads)                  USE_THREADS=YES            ;;

        --with-file-aio)                 NGX_FILE_AIO=YES           ;;
        --with-ipv6)                     NGX_IPV6=YES               ;;

        --without-http)                  HTTP=NO                    ;;
        --without-http-cache)            HTTP_CACHE=NO              ;;

        --http-log-path=*)               NGX_HTTP_LOG_PATH="$value" ;;
        --http-client-body-temp-path=*)  NGX_HTTP_CLIENT_TEMP_PATH="$value" ;;
        --http-proxy-temp-path=*)        NGX_HTTP_PROXY_TEMP_PATH="$value" ;;
        --http-fastcgi-temp-path=*)      NGX_HTTP_FASTCGI_TEMP_PATH="$value" ;;
        --http-uwsgi-temp-path=*)        NGX_HTTP_UWSGI_TEMP_PATH="$value" ;;
        --http-scgi-temp-path=*)         NGX_HTTP_SCGI_TEMP_PATH="$value" ;;

        --with-http_ssl_module)          HTTP_SSL=YES               ;;
        --with-http_v2_module)           HTTP_V2=YES                ;;
        --with-http_realip_module)       HTTP_REALIP=YES            ;;
        --with-http_addition_module)     HTTP_ADDITION=YES          ;;
        --with-http_xslt_module)         HTTP_XSLT=YES              ;;
        --with-http_xslt_module=dynamic) HTTP_XSLT=DYNAMIC          ;;
        --with-http_image_filter_module) HTTP_IMAGE_FILTER=YES      ;;
        --with-http_image_filter_module=dynamic)
                                         HTTP_IMAGE_FILTER=DYNAMIC  ;;
        --with-http_geoip_module)        HTTP_GEOIP=YES             ;;
        --with-http_geoip_module=dynamic)
                                         HTTP_GEOIP=DYNAMIC         ;;
        --with-http_sub_module)          HTTP_SUB=YES               ;;
        --with-http_dav_module)          HTTP_DAV=YES               ;;
        --with-http_flv_module)          HTTP_FLV=YES               ;;
        --with-http_mp4_module)          HTTP_MP4=YES               ;;
        --with-http_gunzip_module)       HTTP_GUNZIP=YES            ;;
        --with-http_gzip_static_module)  HTTP_GZIP_STATIC=YES       ;;
        --with-http_auth_request_module) HTTP_AUTH_REQUEST=YES      ;;
        --with-http_random_index_module) HTTP_RANDOM_INDEX=YES      ;;
        --with-http_secure_link_module)  HTTP_SECURE_LINK=YES       ;;
        --with-http_degradation_module)  HTTP_DEGRADATION=YES       ;;
        --with-http_slice_module)        HTTP_SLICE=YES             ;;

        --without-http_charset_module)   HTTP_CHARSET=NO            ;;
        --without-http_gzip_module)      HTTP_GZIP=NO               ;;
        --without-http_ssi_module)       HTTP_SSI=NO                ;;
        --without-http_userid_module)    HTTP_USERID=NO             ;;
        --without-http_access_module)    HTTP_ACCESS=NO             ;;
        --without-http_auth_basic_module) HTTP_AUTH_BASIC=NO        ;;
        --without-http_autoindex_module) HTTP_AUTOINDEX=NO          ;;
        --without-http_status_module)    HTTP_STATUS=NO             ;;
        --without-http_geo_module)       HTTP_GEO=NO                ;;
        --without-http_map_module)       HTTP_MAP=NO                ;;
        --without-http_split_clients_module) HTTP_SPLIT_CLIENTS=NO  ;;
        --without-http_referer_module)   HTTP_REFERER=NO            ;;
        --without-http_rewrite_module)   HTTP_REWRITE=NO            ;;
        --without-http_proxy_module)     HTTP_PROXY=NO              ;;
        --without-http_fastcgi_module)   HTTP_FASTCGI=NO            ;;
        --without-http_uwsgi_module)     HTTP_UWSGI=NO              ;;
        --without-http_scgi_module)      HTTP_SCGI=NO               ;;
        --without-http_memcached_module) HTTP_MEMCACHED=NO          ;;
        --without-http_limit_conn_module) HTTP_LIMIT_CONN=NO        ;;
        --without-http_limit_req_module) HTTP_LIMIT_REQ=NO         ;;
        --without-http_empty_gif_module) HTTP_EMPTY_GIF=NO          ;;
        --without-http_browser_module)   HTTP_BROWSER=NO            ;;
        --without-http_upstream_hash_module) HTTP_UPSTREAM_HASH=NO  ;;
        --without-http_upstream_ip_hash_module) HTTP_UPSTREAM_IP_HASH=NO ;;
        --without-http_upstream_least_conn_module)
                                         HTTP_UPSTREAM_LEAST_CONN=NO ;;
        --without-http_upstream_keepalive_module) HTTP_UPSTREAM_KEEPALIVE=NO ;;
        --without-http_upstream_zone_module) HTTP_UPSTREAM_ZONE=NO  ;;

        --with-http_perl_module)         HTTP_PERL=YES              ;;
        --with-http_perl_module=dynamic) HTTP_PERL=DYNAMIC          ;;
        --with-perl_modules_path=*)      NGX_PERL_MODULES="$value"  ;;
        --with-perl=*)                   NGX_PERL="$value"          ;;

        # STUB
        --with-http_stub_status_module)  HTTP_STUB_STATUS=YES       ;;

        --with-mail)                     MAIL=YES                   ;;
        --with-mail=dynamic)             MAIL=DYNAMIC               ;;
        --with-mail_ssl_module)          MAIL_SSL=YES               ;;
        # STUB
        --with-imap)
            MAIL=YES
            NGX_POST_CONF_MSG="$NGX_POST_CONF_MSG
$0: warning: the \"--with-imap\" option is deprecated, \
use the \"--with-mail\" option instead"
        ;;
        --with-imap_ssl_module)
            MAIL_SSL=YES
            NGX_POST_CONF_MSG="$NGX_POST_CONF_MSG
$0: warning: the \"--with-imap_ssl_module\" option is deprecated, \
use the \"--with-mail_ssl_module\" option instead"
        ;;
        --without-mail_pop3_module)      MAIL_POP3=NO               ;;
        --without-mail_imap_module)      MAIL_IMAP=NO               ;;
        --without-mail_smtp_module)      MAIL_SMTP=NO               ;;

        --with-stream)                   STREAM=YES                 ;;
        --with-stream=dynamic)           STREAM=DYNAMIC             ;;
        --with-stream_ssl_module)        STREAM_SSL=YES             ;;
        --without-stream_limit_conn_module)
                                         STREAM_LIMIT_CONN=NO       ;;
        --without-stream_access_module)  STREAM_ACCESS=NO           ;;
        --without-stream_upstream_hash_module)
                                         STREAM_UPSTREAM_HASH=NO    ;;
        --without-stream_upstream_least_conn_module)
                                         STREAM_UPSTREAM_LEAST_CONN=NO ;;
        --without-stream_upstream_zone_module)
                                         STREAM_UPSTREAM_ZONE=NO    ;;

        --with-google_perftools_module)  NGX_GOOGLE_PERFTOOLS=YES   ;;
        --with-cpp_test_module)          NGX_CPP_TEST=YES           ;;

        --add-module=*)                  NGX_ADDONS="$NGX_ADDONS $value" ;;
        --add-dynamic-module=*)          DYNAMIC_ADDONS="$DYNAMIC_ADDONS $value" ;;

        --with-cc=*)                     CC="$value"                ;;
        --with-cpp=*)                    CPP="$value"               ;;
        --with-cc-opt=*)                 NGX_CC_OPT="$value"        ;;
        --with-ld-opt=*)                 NGX_LD_OPT="$value"        ;;
        --with-cpu-opt=*)                CPU="$value"               ;;
        --with-debug)                    NGX_DEBUG=YES              ;;

        --without-pcre)                  USE_PCRE=DISABLED          ;;
        --with-pcre)                     USE_PCRE=YES               ;;
        --with-pcre=*)                   PCRE="$value"              ;;
        --with-pcre-opt=*)               PCRE_OPT="$value"          ;;
        --with-pcre-jit)                 PCRE_JIT=YES               ;;

        --with-openssl=*)                OPENSSL="$value"           ;;
        --with-openssl-opt=*)            OPENSSL_OPT="$value"       ;;

        --with-md5=*)                    MD5="$value"               ;;
        --with-md5-opt=*)                MD5_OPT="$value"           ;;
        --with-md5-asm)                  MD5_ASM=YES                ;;

        --with-sha1=*)                   SHA1="$value"              ;;
        --with-sha1-opt=*)               SHA1_OPT="$value"          ;;
        --with-sha1-asm)                 SHA1_ASM=YES               ;;

        --with-zlib=*)                   ZLIB="$value"              ;;
        --with-zlib-opt=*)               ZLIB_OPT="$value"          ;;
        --with-zlib-asm=*)               ZLIB_ASM="$value"          ;;

        --with-libatomic)                NGX_LIBATOMIC=YES          ;;
        --with-libatomic=*)              NGX_LIBATOMIC="$value"     ;;

        --test-build-devpoll)            NGX_TEST_BUILD_DEVPOLL=YES ;;
        --test-build-eventport)          NGX_TEST_BUILD_EVENTPORT=YES ;;
        --test-build-epoll)              NGX_TEST_BUILD_EPOLL=YES   ;;
        --test-build-solaris-sendfilev)  NGX_TEST_BUILD_SOLARIS_SENDFILEV=YES ;;

        *)
            echo "$0: error: invalid option \"$option\""
            exit 1
        ;;
    esac
done


NGX_CONFIGURE="$opt"


if [ $help = yes ]; then

cat << END

  --help                             print this message

  --prefix=PATH                      set installation prefix
  --sbin-path=PATH                   set nginx binary pathname
  --modules-path=PATH                set modules path
  --conf-path=PATH                   set nginx.conf pathname
  --error-log-path=PATH              set error log pathname
  --pid-path=PATH                    set nginx.pid pathname
  --lock-path=PATH                   set nginx.lock pathname

  --user=USER                        set non-privileged user for
                                     worker processes
  --group=GROUP                      set non-privileged group for
                                     worker processes

  --build=NAME                       set build name
  --builddir=DIR                     set build directory

  --with-select_module               enable select module
  --without-select_module            disable select module
  --with-poll_module                 enable poll module
  --without-poll_module              disable poll module

  --with-threads                     enable thread pool support

  --with-file-aio                    enable file AIO support
  --with-ipv6                        enable IPv6 support

  --with-http_ssl_module             enable ngx_http_ssl_module
  --with-http_v2_module              enable ngx_http_v2_module
  --with-http_realip_module          enable ngx_http_realip_module
  --with-http_addition_module        enable ngx_http_addition_module
  --with-http_xslt_module            enable ngx_http_xslt_module
  --with-http_xslt_module=dynamic    enable dynamic ngx_http_xslt_module
  --with-http_image_filter_module    enable ngx_http_image_filter_module
  --with-http_image_filter_module=dynamic
                                     enable dynamic ngx_http_image_filter_module
  --with-http_geoip_module           enable ngx_http_geoip_module
  --with-http_geoip_module=dynamic   enable dynamic ngx_http_geoip_module
  --with-http_sub_module             enable ngx_http_sub_module
  --with-http_dav_module             enable ngx_http_dav_module
  --with-http_flv_module             enable ngx_http_flv_module
  --with-http_mp4_module             enable ngx_http_mp4_module
  --with-http_gunzip_module          enable ngx_http_gunzip_module
  --with-http_gzip_static_module     enable ngx_http_gzip_static_module
  --with-http_auth_request_module    enable ngx_http_auth_request_module
  --with-http_random_index_module    enable ngx_http_random_index_module
  --with-http_secure_link_module     enable ngx_http_secure_link_module
  --with-http_degradation_module     enable ngx_http_degradation_module
  --with-http_slice_module           enable ngx_http_slice_module
  --with-http_stub_status_module     enable ngx_http_stub_status_module

  --without-http_charset_module      disable ngx_http_charset_module
  --without-http_gzip_module         disable ngx_http_gzip_module
  --without-http_ssi_module          disable ngx_http_ssi_module
  --without-http_userid_module       disable ngx_http_userid_module
  --without-http_access_module       disable ngx_http_access_module
  --without-http_auth_basic_module   disable ngx_http_auth_basic_module
  --without-http_autoindex_module    disable ngx_http_autoindex_module
  --without-http_geo_module          disable ngx_http_geo_module
  --without-http_map_module          disable ngx_http_map_module
  --without-http_split_clients_module disable ngx_http_split_clients_module
  --without-http_referer_module      disable ngx_http_referer_module
  --without-http_rewrite_module      disable ngx_http_rewrite_module
  --without-http_proxy_module        disable ngx_http_proxy_module
  --without-http_fastcgi_module      disable ngx_http_fastcgi_module
  --without-http_uwsgi_module        disable ngx_http_uwsgi_module
  --without-http_scgi_module         disable ngx_http_scgi_module
  --without-http_memcached_module    disable ngx_http_memcached_module
  --without-http_limit_conn_module   disable ngx_http_limit_conn_module
  --without-http_limit_req_module    disable ngx_http_limit_req_module
  --without-http_empty_gif_module    disable ngx_http_empty_gif_module
  --without-http_browser_module      disable ngx_http_browser_module
  --without-http_upstream_hash_module
                                     disable ngx_http_upstream_hash_module
  --without-http_upstream_ip_hash_module
                                     disable ngx_http_upstream_ip_hash_module
  --without-http_upstream_least_conn_module
                                     disable ngx_http_upstream_least_conn_module
  --without-http_upstream_keepalive_module
                                     disable ngx_http_upstream_keepalive_module
  --without-http_upstream_zone_module
                                     disable ngx_http_upstream_zone_module

  --with-http_perl_module            enable ngx_http_perl_module
  --with-http_perl_module=dynamic    enable dynamic ngx_http_perl_module
  --with-perl_modules_path=PATH      set Perl modules path
  --with-perl=PATH                   set perl binary pathname

  --http-log-path=PATH               set http access log pathname
  --http-client-body-temp-path=PATH  set path to store
                                     http client request body temporary files
  --http-proxy-temp-path=PATH        set path to store
                                     http proxy temporary files
  --http-fastcgi-temp-path=PATH      set path to store
                                     http fastcgi temporary files
  --http-uwsgi-temp-path=PATH        set path to store
                                     http uwsgi temporary files
  --http-scgi-temp-path=PATH         set path to store
                                     http scgi temporary files

  --without-http                     disable HTTP server
  --without-http-cache               disable HTTP cache

  --with-mail                        enable POP3/IMAP4/SMTP proxy module
  --with-mail=dynamic                enable dynamic POP3/IMAP4/SMTP proxy module
  --with-mail_ssl_module             enable ngx_mail_ssl_module
  --without-mail_pop3_module         disable ngx_mail_pop3_module
  --without-mail_imap_module         disable ngx_mail_imap_module
  --without-mail_smtp_module         disable ngx_mail_smtp_module

  --with-stream                      enable TCP/UDP proxy module
  --with-stream=dynamic              enable dynamic TCP/UDP proxy module
  --with-stream_ssl_module           enable ngx_stream_ssl_module
  --without-stream_limit_conn_module disable ngx_stream_limit_conn_module
  --without-stream_access_module     disable ngx_stream_access_module
  --without-stream_upstream_hash_module
                                     disable ngx_stream_upstream_hash_module
  --without-stream_upstream_least_conn_module
                                     disable ngx_stream_upstream_least_conn_module
  --without-stream_upstream_zone_module
                                     disable ngx_stream_upstream_zone_module

  --with-google_perftools_module     enable ngx_google_perftools_module
  --with-cpp_test_module             enable ngx_cpp_test_module

  --add-module=PATH                  enable external module
  --add-dynamic-module=PATH          enable dynamic external module

  --with-cc=PATH                     set C compiler pathname
  --with-cpp=PATH                    set C preprocessor pathname
  --with-cc-opt=OPTIONS              set additional C compiler options
  --with-ld-opt=OPTIONS              set additional linker options
  --with-cpu-opt=CPU                 build for the specified CPU, valid values:
                                     pentium, pentiumpro, pentium3, pentium4,
                                     athlon, opteron, sparc32, sparc64, ppc64

  --without-pcre                     disable PCRE library usage
  --with-pcre                        force PCRE library usage
  --with-pcre=DIR                    set path to PCRE library sources
  --with-pcre-opt=OPTIONS            set additional build options for PCRE
  --with-pcre-jit                    build PCRE with JIT compilation support

  --with-md5=DIR                     set path to md5 library sources
  --with-md5-opt=OPTIONS             set additional build options for md5
  --with-md5-asm                     use md5 assembler sources

  --with-sha1=DIR                    set path to sha1 library sources
  --with-sha1-opt=OPTIONS            set additional build options for sha1
  --with-sha1-asm                    use sha1 assembler sources

  --with-zlib=DIR                    set path to zlib library sources
  --with-zlib-opt=OPTIONS            set additional build options for zlib
  --with-zlib-asm=CPU                use zlib assembler sources optimized
                                     for the specified CPU, valid values:
                                     pentium, pentiumpro

  --with-libatomic                   force libatomic_ops library usage
  --with-libatomic=DIR               set path to libatomic_ops library sources

  --with-openssl=DIR                 set path to OpenSSL library sources
  --with-openssl-opt=OPTIONS         set additional build options for OpenSSL

  --with-debug                       enable debug logging

END

    exit 1
fi


if [ $HTTP = NO ]; then
    HTTP_CHARSET=NO
    HTTP_GZIP=NO
    HTTP_SSI=NO
    HTTP_USERID=NO
    HTTP_ACCESS=NO
    HTTP_STATUS=NO
    HTTP_REWRITE=NO
    HTTP_PROXY=NO
    HTTP_FASTCGI=NO
fi


if [ ".$NGX_PLATFORM" = ".win32" ]; then
    NGX_WINE=$WINE
fi


NGX_SBIN_PATH=${NGX_SBIN_PATH:-sbin/nginx}
NGX_MODULES_PATH=${NGX_MODULES_PATH:-modules}
NGX_CONF_PATH=${NGX_CONF_PATH:-conf/nginx.conf}
NGX_CONF_PREFIX=`dirname $NGX_CONF_PATH`
NGX_PID_PATH=${NGX_PID_PATH:-logs/nginx.pid}
NGX_LOCK_PATH=${NGX_LOCK_PATH:-logs/nginx.lock}

if [ ".$NGX_ERROR_LOG_PATH" = ".stderr" ]; then
    NGX_ERROR_LOG_PATH=
else
    NGX_ERROR_LOG_PATH=${NGX_ERROR_LOG_PATH:-logs/error.log}
fi

NGX_HTTP_LOG_PATH=${NGX_HTTP_LOG_PATH:-logs/access.log}
NGX_HTTP_CLIENT_TEMP_PATH=${NGX_HTTP_CLIENT_TEMP_PATH:-client_body_temp}
NGX_HTTP_PROXY_TEMP_PATH=${NGX_HTTP_PROXY_TEMP_PATH:-proxy_temp}
NGX_HTTP_FASTCGI_TEMP_PATH=${NGX_HTTP_FASTCGI_TEMP_PATH:-fastcgi_temp}
NGX_HTTP_UWSGI_TEMP_PATH=${NGX_HTTP_UWSGI_TEMP_PATH:-uwsgi_temp}
NGX_HTTP_SCGI_TEMP_PATH=${NGX_HTTP_SCGI_TEMP_PATH:-scgi_temp}

case ".$NGX_PERL_MODULES" in
    ./*)
    ;;

    .)
    ;;

    *)
        NGX_PERL_MODULES=$NGX_PREFIX/$NGX_PERL_MODULES
    ;;
esac

{% endhighlight %}


## 2. 读取configure配置参数

**(1) 变量定义**

我们注意到在auto/options脚本的最开始定义了很多变量，有些设定了初始值，有些则没有。其他变量都很稀疏平常，但是我们注意到有如下一个比较特别：
{% highlight string %}
CC=${CC:-cc}
{% endhighlight %} 
则句话的含义为如果```CC```变量没有定义，则```CC```定义为cc；否则为```CC```定义的变量值。

**(2) 解析参数选项**

接着在auto/options中，有如下一段：
{% highlight string %}
opt=

for option
do
    opt="$opt `echo $option | sed -e \"s/\(--[^=]*=\)\(.* .*\)/\1'\2'/\"`"

    case "$option" in
        -*=*) value=`echo "$option" | sed -e 's/[-_a-zA-Z0-9]*=//'` ;;
           *) value="" ;;
    esac
done
{% endhighlight %}

这段实际上是处理./configure携带的参数选项，for循环每次对应一个参数选项option。要注意的是for循环体上面有一个全局的opt变量。这个循环体内第一个语句是最重要的：
{% highlight string %}
opt="$opt `echo $option | sed -e \"s/\(--[^=]*=\)\(.* .*\)/\1'\2'/\"`"
{% endhighlight %}
它的实际作用是匹配configure的参数选项（把匹配到的第一部分作为参数1，匹配到的第二部分作为参数2）。因此，通过循环该语句后，最后opt的值就是一个由空格分隔的参数列表。参看：[正则表达式括号、中括号、大括号的区别小结](http://blog.csdn.net/u010552788/article/details/51019367)

<br />

接下来是：
{% highlight string %}
case "$option" in
    -*=*) value=`echo "$option" | sed -e 's/[-_a-zA-Z0-9]*=//'` ;;
       *) value="" ;;
esac
{% endhighlight %}
其含义是将value赋值为参数选项值。如果该选项不符合匹配```-*=*```,则将value值设置为"".

<br>

再接着是匹配各参数选项：
{% highlight string %}
case "$option" in
    --help)                          help=yes                   ;;

    --prefix=)                       NGX_PREFIX="!"             ;;
    --prefix=*)                      NGX_PREFIX="$value"        ;;
    --sbin-path=*)                   NGX_SBIN_PATH="$value"     ;;
    --modules-path=*)                NGX_MODULES_PATH="$value"  ;;
    --conf-path=*)                   NGX_CONF_PATH="$value"     ;;
    --error-log-path=*)              NGX_ERROR_LOG_PATH="$value";;
    --pid-path=*)                    NGX_PID_PATH="$value"      ;;
    --lock-path=*)                   NGX_LOCK_PATH="$value"     ;;
    --user=*)                        NGX_USER="$value"          ;;
    --group=*)                       NGX_GROUP="$value"         ;;

  // 后续省略
esac
{% endhighlight %}
各匹配的分支语句中进行配置变量的赋值。这些变量在auto/options脚本的最开始处赋予默认值。其中那些模块配置变量被赋予```YES```的表示默认开启，赋予```NO```的表示默认关闭。但他们开启与否是由这个auto/options中的case-esac语句来决定的。

还有一些安装相关的选项变量也在这里被赋值，比如:

* prefix参数值被赋予NGX_PREFIX
* sbin-path参数值被赋予NGX_SBIN_PATH
* conf-path参数值被赋予NGX_CONF_PATH
* error-log-path参数值被赋予NGX_ERROR_LOG_PATH
* pid-path参数值被赋予NGX_PID_PATH
* lock-path参数值被赋予NGX_LOCK_PATH

<br />


如果option并不符合预设的这些匹配，也就是用户使用configure脚本的时候携带的参数错误，则auto/options会匹配该语句：
{% highlight string %}
*)
    echo "$0: error: invalid option \"$option\""
    exit 1
;;
{% endhighlight %}

从而提示用户参数错误，并使脚本退出运行。经过多次循环，for-do-done就结束。


## 3. 设定NGX_CONFIGURE变量
处理完所有option后，opt就如我们上面提到的，成为由空格分隔的配置项值，并被赋给NGX_CONFIGURE变量：
{% highlight string %}
NGX_CONFIGURE="$opt"
{% endhighlight %}



## 4. 是否显示configure的帮助信息
再看下面这句：
{% highlight string %}
if [ $help = yes ]; then

cat << END

  --help                             print this message

  --prefix=PATH                      set installation prefix
  --sbin-path=PATH                   set nginx binary pathname
  --modules-path=PATH                set modules path
  --conf-path=PATH                   set nginx.conf pathname

 //后续省略
END

exit
fi
{% endhighlight %}
默认情况下，```$help```变量值在初始化时就为no. 如果configure选项中指定了help参数，则```$help```参数为yes，则会运行cat命令，显示大段的帮助信息，然后退出。


## 5. 是否关闭HTTP功能

默认情况下HTTP的一些基本功能是被开启的，如果用户指定了```--without-http```参数，则变量HTTP会被赋值为NO，则下面这段代码会被被执行：
{% highlight string %}
if [ $HTTP = NO ]; then
    HTTP_CHARSET=NO
    HTTP_GZIP=NO
    HTTP_SSI=NO
    HTTP_USERID=NO
    HTTP_ACCESS=NO
    HTTP_STATUS=NO
    HTTP_REWRITE=NO
    HTTP_PROXY=NO
    HTTP_FASTCGI=NO
fi
{% endhighlight %}

## 6.是否指定运行于Windows平台

如果显示指定了```--crossbuild```参数，则变量NGX_PLATFORM会被赋予当前for-do-done循环中的```$value```值，也就是```--crossbuild```的参数值，一般考虑在Windows平台使用时才会用到，看下面的语句：
{% highlight string %}
if [ ".$NGX_PLATFORM" = ".win32" ]; then
    NGX_WINE=$WINE
fi
{% endhighlight %}
如果指定了```--crossbuild=win32```，则NGX_WINE就会被赋值了。


## 7. nginx配置文件路径
在加载configure的参数时，如果没有指定```--config-path```参数，则```$NGX_CONF_PATH```变量是没有值的，下面的语句会为NGX_CONF_PATH赋予conf/nginx.conf的缺省值。不过觉得完全可以再auto/options开始处和其他参数一样先指定NGX_CONF_PATH的默认值。

{% highlight string %}
NGX_SBIN_PATH=${NGX_SBIN_PATH:-sbin/nginx}
NGX_MODULES_PATH=${NGX_MODULES_PATH:-modules}
NGX_CONF_PATH=${NGX_CONF_PATH:-conf/nginx.conf}
NGX_CONF_PREFIX=`dirname $NGX_CONF_PATH`
NGX_PID_PATH=${NGX_PID_PATH:-logs/nginx.pid}
NGX_LOCK_PATH=${NGX_LOCK_PATH:-logs/nginx.lock}
{% endhighlight %}

上面如果指定参数```--conf-path```=/home/michael/nginx/conf/nginx.conf，则NGX_CONF_PREFIX的值就是/home/michael/nginx/conf.


## 8. 错误日志文件路径
如果指定了参数```--error-log-path```，则NGX_ERROR_LOG_PATH变量的值会被指定。根据下面的语句，如果指定的是stderr则将NGX_ERROR_LOG_PATH修改为空，即不需要错误日志文件。如果不是标准输出，且其值为空，则设置为缺省logs/error.log。
{% highlight string %}
if [ ".$NGX_ERROR_LOG_PATH" = ".stderr" ]; then
    NGX_ERROR_LOG_PATH=
else
    NGX_ERROR_LOG_PATH=${NGX_ERROR_LOG_PATH:-logs/error.log}
fi
{% endhighlight %}


## 9. HTTP相关路径
{% highlight string %}
NGX_HTTP_LOG_PATH=${NGX_HTTP_LOG_PATH:-logs/access.log}
NGX_HTTP_CLIENT_TEMP_PATH=${NGX_HTTP_CLIENT_TEMP_PATH:-client_body_temp}
NGX_HTTP_PROXY_TEMP_PATH=${NGX_HTTP_PROXY_TEMP_PATH:-proxy_temp}
NGX_HTTP_FASTCGI_TEMP_PATH=${NGX_HTTP_FASTCGI_TEMP_PATH:-fastcgi_temp}
NGX_HTTP_UWSGI_TEMP_PATH=${NGX_HTTP_UWSGI_TEMP_PATH:-uwsgi_temp}
NGX_HTTP_SCGI_TEMP_PATH=${NGX_HTTP_SCGI_TEMP_PATH:-scgi_temp}
{% endhighlight %}

## 10. Perl模块
如果指定了```--with-perl_modules_path```参数，则NGX_PERL_MODULES变量被设定。如果NGX_PERL_MODULES变量未设定值或者值为一个绝对路径，那么不做任何处理；而如果指定的是一个相对路径，则最后设定为```$NGX_PREFIX/$NGX_PERL_MODULES```。

{% highlight string %}
case ".$NGX_PERL_MODULES" in
    ./*)
    ;;

    .)
    ;;

    *)
        NGX_PERL_MODULES=$NGX_PREFIX/$NGX_PERL_MODULES
    ;;
esac
{% endhighlight %}



<br />
<br />
<br />

