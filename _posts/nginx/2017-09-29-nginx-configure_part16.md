---
layout: post
title: auto/modules脚本分析-part16
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

本节我们介绍auto/modules脚本，该脚本的主要功能是定义Nginx相应的各个模块：

<!-- more -->

* event模块
* HTTP模块
* CORE模块
* HTTP_FILTER模块
* HTTP_INIT_FILTER模块
* HTTP_AUX_FILTER模块
* MAIL模块
* STREAM模块
* MISC模块

上述是几个大的模块类型，每一个模块类型里面可能会有多个小的子模块组成。其中后边```8种模块```是ngx_module_type可取的类型。


## 1. auto/modules脚本
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


if [ $EVENT_SELECT = NO -a $EVENT_FOUND = NO ]; then
    EVENT_SELECT=YES
fi

if [ $EVENT_SELECT = YES ]; then
    have=NGX_HAVE_SELECT . auto/have
    CORE_SRCS="$CORE_SRCS $SELECT_SRCS"
    EVENT_MODULES="$EVENT_MODULES $SELECT_MODULE"
fi


if [ $EVENT_POLL = NO -a $EVENT_FOUND = NO ]; then
    EVENT_POLL=YES
fi

if [ $EVENT_POLL = YES ]; then
    have=NGX_HAVE_POLL . auto/have
    CORE_SRCS="$CORE_SRCS $POLL_SRCS"
    EVENT_MODULES="$EVENT_MODULES $POLL_MODULE"
fi


if [ $NGX_TEST_BUILD_DEVPOLL = YES ]; then
    have=NGX_HAVE_DEVPOLL . auto/have
    have=NGX_TEST_BUILD_DEVPOLL . auto/have
    EVENT_MODULES="$EVENT_MODULES $DEVPOLL_MODULE"
    CORE_SRCS="$CORE_SRCS $DEVPOLL_SRCS"
fi


if [ $NGX_TEST_BUILD_EVENTPORT = YES ]; then
    have=NGX_HAVE_EVENTPORT . auto/have
    have=NGX_TEST_BUILD_EVENTPORT . auto/have
    EVENT_MODULES="$EVENT_MODULES $EVENTPORT_MODULE"
    CORE_SRCS="$CORE_SRCS $EVENTPORT_SRCS"
fi

if [ $NGX_TEST_BUILD_EPOLL = YES ]; then
    have=NGX_HAVE_EPOLL . auto/have
    have=NGX_HAVE_EPOLLRDHUP . auto/have
    have=NGX_HAVE_EVENTFD . auto/have
    have=NGX_TEST_BUILD_EPOLL . auto/have
    EVENT_MODULES="$EVENT_MODULES $EPOLL_MODULE"
    CORE_SRCS="$CORE_SRCS $EPOLL_SRCS"
fi

if [ $NGX_TEST_BUILD_SOLARIS_SENDFILEV = YES ]; then
    have=NGX_TEST_BUILD_SOLARIS_SENDFILEV . auto/have
    CORE_SRCS="$CORE_SRCS $SOLARIS_SENDFILEV_SRCS"
fi


HTTP_MODULES=
HTTP_DEPS=
HTTP_INCS=

ngx_module_type=HTTP

if :; then
    ngx_module_name="ngx_http_module \
                     ngx_http_core_module \
                     ngx_http_log_module \
                     ngx_http_upstream_module"
    ngx_module_incs="src/http src/http/modules"
    ngx_module_deps="src/http/ngx_http.h \
                     src/http/ngx_http_request.h \
                     src/http/ngx_http_config.h \
                     src/http/ngx_http_core_module.h \
                     src/http/ngx_http_cache.h \
                     src/http/ngx_http_variables.h \
                     src/http/ngx_http_script.h \
                     src/http/ngx_http_upstream.h \
                     src/http/ngx_http_upstream_round_robin.h"
    ngx_module_srcs="src/http/ngx_http.c \
                     src/http/ngx_http_core_module.c \
                     src/http/ngx_http_special_response.c \
                     src/http/ngx_http_request.c \
                     src/http/ngx_http_parse.c \
                     src/http/modules/ngx_http_log_module.c \
                     src/http/ngx_http_request_body.c \
                     src/http/ngx_http_variables.c \
                     src/http/ngx_http_script.c \
                     src/http/ngx_http_upstream.c \
                     src/http/ngx_http_upstream_round_robin.c"
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi


if [ $HTTP != YES ]; then
    have=NGX_CRYPT . auto/nohave
    CRYPT_LIB=
fi


if [ $HTTP_CACHE = YES ]; then
    USE_MD5=YES
    have=NGX_HTTP_CACHE . auto/have
    HTTP_SRCS="$HTTP_SRCS $HTTP_FILE_CACHE_SRCS"
fi


if [ $HTTP_SSI = YES ]; then
    HTTP_POSTPONE=YES
fi


if [ $HTTP_SLICE = YES ]; then
    HTTP_POSTPONE=YES
fi


if [ $HTTP_ADDITION = YES ]; then
    HTTP_POSTPONE=YES
fi


# the module order is important
#     ngx_http_static_module
#     ngx_http_gzip_static_module
#     ngx_http_dav_module
#     ngx_http_autoindex_module
#     ngx_http_index_module
#     ngx_http_random_index_module
#
#     ngx_http_access_module
#     ngx_http_realip_module
#
#
# the filter order is important
#     ngx_http_write_filter
#     ngx_http_header_filter
#     ngx_http_chunked_filter
#     ngx_http_v2_filter
#     ngx_http_range_header_filter
#     ngx_http_gzip_filter
#     ngx_http_postpone_filter
#     ngx_http_ssi_filter
#     ngx_http_charset_filter
#         ngx_http_xslt_filter
#         ngx_http_image_filter
#         ngx_http_sub_filter
#         ngx_http_addition_filter
#         ngx_http_gunzip_filter
#         ngx_http_userid_filter
#         ngx_http_headers_filter
#     ngx_http_copy_filter
#     ngx_http_range_body_filter
#     ngx_http_not_modified_filter
#     ngx_http_slice_filter

ngx_module_type=HTTP_FILTER
HTTP_FILTER_MODULES=

ngx_module_order="ngx_http_static_module \
                  ngx_http_gzip_static_module \
                  ngx_http_dav_module \
                  ngx_http_autoindex_module \
                  ngx_http_index_module \
                  ngx_http_random_index_module \
                  ngx_http_access_module \
                  ngx_http_realip_module \
                  ngx_http_write_filter_module \
                  ngx_http_header_filter_module \
                  ngx_http_chunked_filter_module \
                  ngx_http_v2_filter_module \
                  ngx_http_range_header_filter_module \
                  ngx_http_gzip_filter_module \
                  ngx_http_postpone_filter_module \
                  ngx_http_ssi_filter_module \
                  ngx_http_charset_filter_module \
                  ngx_http_xslt_filter_module \
                  ngx_http_image_filter_module \
                  ngx_http_sub_filter_module \
                  ngx_http_addition_filter_module \
                  ngx_http_gunzip_filter_module \
                  ngx_http_userid_filter_module \
                  ngx_http_headers_filter_module \
                  ngx_http_copy_filter_module \
                  ngx_http_range_body_filter_module \
                  ngx_http_not_modified_filter_module \
                  ngx_http_slice_filter_module"

if :; then
    ngx_module_name=ngx_http_write_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/ngx_http_write_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

if :; then
    ngx_module_name=ngx_http_header_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/ngx_http_header_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

if :; then
    ngx_module_name=ngx_http_chunked_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_chunked_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

if [ $HTTP_V2 = YES ]; then
    ngx_module_name=ngx_http_v2_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/v2/ngx_http_v2_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_V2

    . auto/module
fi

if :; then
    ngx_module_name=ngx_http_range_header_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_range_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

if [ $HTTP_GZIP = YES ]; then
    have=NGX_HTTP_GZIP . auto/have
    USE_ZLIB=YES

    ngx_module_name=ngx_http_gzip_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_gzip_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_GZIP

    . auto/module
fi

if [ $HTTP_POSTPONE = YES ]; then
    ngx_module_name=ngx_http_postpone_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/ngx_http_postpone_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_POSTPONE

    . auto/module
fi

if [ $HTTP_SSI = YES ]; then
    have=NGX_HTTP_SSI . auto/have

    ngx_module_name=ngx_http_ssi_filter_module
    ngx_module_incs=
    ngx_module_deps=src/http/modules/ngx_http_ssi_filter_module.h
    ngx_module_srcs=src/http/modules/ngx_http_ssi_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SSI

    . auto/module
fi

if [ $HTTP_CHARSET = YES ]; then
    ngx_module_name=ngx_http_charset_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_charset_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_CHARSET

    . auto/module
fi

if [ $HTTP_XSLT != NO ]; then
    ngx_module_name=ngx_http_xslt_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_xslt_filter_module.c
    ngx_module_libs=LIBXSLT
    ngx_module_link=$HTTP_XSLT

    . auto/module
fi

if [ $HTTP_IMAGE_FILTER != NO ]; then
    ngx_module_name=ngx_http_image_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_image_filter_module.c
    ngx_module_libs=LIBGD
    ngx_module_link=$HTTP_IMAGE_FILTER

    . auto/module
fi

if [ $HTTP_SUB = YES ]; then
    ngx_module_name=ngx_http_sub_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_sub_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SUB

    . auto/module
fi

if [ $HTTP_ADDITION = YES ]; then
    ngx_module_name=ngx_http_addition_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_addition_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_ADDITION

    . auto/module
fi

if [ $HTTP_GUNZIP = YES ]; then
    have=NGX_HTTP_GZIP . auto/have
    USE_ZLIB=YES

    ngx_module_name=ngx_http_gunzip_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_gunzip_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_GUNZIP

    . auto/module
fi

if [ $HTTP_USERID = YES ]; then
    ngx_module_name=ngx_http_userid_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_userid_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_USERID

    . auto/module
fi

if :; then
    ngx_module_name=ngx_http_headers_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_headers_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi


ngx_module_type=HTTP_INIT_FILTER
HTTP_INIT_FILTER_MODULES=

if :; then
    ngx_module_name=ngx_http_copy_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/ngx_http_copy_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

if :; then
    ngx_module_name=ngx_http_range_body_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

if :; then
    ngx_module_name=ngx_http_not_modified_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_not_modified_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

if [ $HTTP_SLICE = YES ]; then
    ngx_module_name=ngx_http_slice_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_slice_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SLICE

    . auto/module
fi


ngx_module_type=HTTP

if [ $HTTP_V2 = YES ]; then
    have=NGX_HTTP_V2 . auto/have

    ngx_module_name=ngx_http_v2_module
    ngx_module_incs=src/http/v2
    ngx_module_deps="src/http/v2/ngx_http_v2.h src/http/v2/ngx_http_v2_module.h"
    ngx_module_srcs="src/http/v2/ngx_http_v2.c \
                     src/http/v2/ngx_http_v2_table.c \
                     src/http/v2/ngx_http_v2_huff_decode.c \
                     src/http/v2/ngx_http_v2_huff_encode.c \
                     src/http/v2/ngx_http_v2_module.c"
    ngx_module_libs=
    ngx_module_link=$HTTP_V2

    . auto/module
fi

if :; then
    ngx_module_name=ngx_http_static_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_static_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

if [ $HTTP_GZIP_STATIC = YES ]; then
    have=NGX_HTTP_GZIP . auto/have

    ngx_module_name=ngx_http_gzip_static_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_gzip_static_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_GZIP_STATIC

    . auto/module
fi

if [ $HTTP_DAV = YES ]; then
    have=NGX_HTTP_DAV . auto/have

    ngx_module_name=ngx_http_dav_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_dav_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_DAV

    . auto/module
fi

if [ $HTTP_AUTOINDEX = YES ]; then
    ngx_module_name=ngx_http_autoindex_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_autoindex_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_AUTOINDEX

    . auto/module
fi

if :; then
    ngx_module_name=ngx_http_index_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_index_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

if [ $HTTP_RANDOM_INDEX = YES ]; then
    ngx_module_name=ngx_http_random_index_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_random_index_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_RANDOM_INDEX

    . auto/module
fi

if [ $HTTP_AUTH_REQUEST = YES ]; then
    ngx_module_name=ngx_http_auth_request_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_auth_request_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_AUTH_REQUEST

    . auto/module
fi

if [ $HTTP_AUTH_BASIC = YES ]; then
    USE_MD5=YES
    USE_SHA1=YES
    have=NGX_CRYPT . auto/have

    ngx_module_name=ngx_http_auth_basic_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_auth_basic_module.c
    ngx_module_libs=$CRYPT_LIB
    ngx_module_link=$HTTP_AUTH_BASIC

    . auto/module
fi

if [ $HTTP_ACCESS = YES ]; then
    ngx_module_name=ngx_http_access_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_access_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_ACCESS

    . auto/module
fi

if [ $HTTP_LIMIT_CONN = YES ]; then
    ngx_module_name=ngx_http_limit_conn_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_limit_conn_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_LIMIT_CONN

    . auto/module
fi

if [ $HTTP_LIMIT_REQ = YES ]; then
    ngx_module_name=ngx_http_limit_req_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_limit_req_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_LIMIT_REQ

    . auto/module
fi

if [ $HTTP_REALIP = YES ]; then
    have=NGX_HTTP_REALIP . auto/have
    have=NGX_HTTP_X_FORWARDED_FOR . auto/have

    ngx_module_name=ngx_http_realip_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_realip_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_REALIP

    . auto/module
fi

if [ $HTTP_STATUS = YES ]; then
    ngx_module_name=ngx_http_status_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_status_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_STATUS

    . auto/module
fi

if [ $HTTP_GEO = YES ]; then
    have=NGX_HTTP_X_FORWARDED_FOR . auto/have

    ngx_module_name=ngx_http_geo_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_geo_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_GEO

    . auto/module
fi

if [ $HTTP_GEOIP != NO ]; then
    have=NGX_HTTP_X_FORWARDED_FOR . auto/have

    ngx_module_name=ngx_http_geoip_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_geoip_module.c
    ngx_module_libs=GEOIP
    ngx_module_link=$HTTP_GEOIP

    . auto/module
fi

if [ $HTTP_MAP = YES ]; then
    ngx_module_name=ngx_http_map_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_map_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_MAP

    . auto/module
fi

if [ $HTTP_SPLIT_CLIENTS = YES ]; then
    ngx_module_name=ngx_http_split_clients_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_split_clients_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SPLIT_CLIENTS

    . auto/module
fi

if [ $HTTP_REFERER = YES ]; then
    ngx_module_name=ngx_http_referer_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_referer_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_REFERER

    . auto/module
fi

if [ $HTTP_REWRITE = YES -a $USE_PCRE != DISABLED ]; then
    USE_PCRE=YES

    ngx_module_name=ngx_http_rewrite_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_rewrite_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_REWRITE

    . auto/module
fi

if [ $HTTP_SSL = YES ]; then
    USE_OPENSSL=YES
    have=NGX_HTTP_SSL . auto/have

    ngx_module_name=ngx_http_ssl_module
    ngx_module_incs=
    ngx_module_deps=src/http/modules/ngx_http_ssl_module.h
    ngx_module_srcs=src/http/modules/ngx_http_ssl_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SSL

    . auto/module
fi

if [ $HTTP_PROXY = YES ]; then
    have=NGX_HTTP_X_FORWARDED_FOR . auto/have
    #USE_MD5=YES

    ngx_module_name=ngx_http_proxy_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_proxy_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_PROXY

    . auto/module
fi

if [ $HTTP_FASTCGI = YES ]; then
    ngx_module_name=ngx_http_fastcgi_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_fastcgi_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_FASTCGI

    . auto/module
fi

if [ $HTTP_UWSGI = YES ]; then
    ngx_module_name=ngx_http_uwsgi_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_uwsgi_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UWSGI

    . auto/module
fi

if [ $HTTP_SCGI = YES ]; then
    ngx_module_name=ngx_http_scgi_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_scgi_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SCGI

    . auto/module
fi

if [ $HTTP_PERL != NO ]; then
    ngx_module_name=ngx_http_perl_module
    ngx_module_incs=src/http/modules/perl
    ngx_module_deps=src/http/modules/perl/ngx_http_perl_module.h
    ngx_module_srcs=src/http/modules/perl/ngx_http_perl_module.c
    ngx_module_libs=PERL
    ngx_module_link=$HTTP_PERL

    . auto/module
fi

if [ $HTTP_MEMCACHED = YES ]; then
    ngx_module_name=ngx_http_memcached_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_memcached_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_MEMCACHED

    . auto/module
fi

if [ $HTTP_EMPTY_GIF = YES ]; then
    ngx_module_name=ngx_http_empty_gif_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_empty_gif_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_EMPTY_GIF

    . auto/module
fi

if [ $HTTP_BROWSER = YES ]; then
    ngx_module_name=ngx_http_browser_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_browser_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_BROWSER

    . auto/module
fi

if [ $HTTP_SECURE_LINK = YES ]; then
    USE_MD5=YES

    ngx_module_name=ngx_http_secure_link_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_secure_link_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SECURE_LINK

    . auto/module
fi

if [ $HTTP_DEGRADATION = YES ]; then
    have=NGX_HTTP_DEGRADATION . auto/have

    ngx_module_name=ngx_http_degradation_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_degradation_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_DEGRADATION

    . auto/module
fi

if [ $HTTP_FLV = YES ]; then
    ngx_module_name=ngx_http_flv_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_flv_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_FLV

    . auto/module
fi

if [ $HTTP_MP4 = YES ]; then
    ngx_module_name=ngx_http_mp4_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_mp4_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_MP4

    . auto/module
fi

if [ $HTTP_UPSTREAM_HASH = YES ]; then
    ngx_module_name=ngx_http_upstream_hash_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_hash_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_HASH

    . auto/module
fi

if [ $HTTP_UPSTREAM_IP_HASH = YES ]; then
    ngx_module_name=ngx_http_upstream_ip_hash_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_ip_hash_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_IP_HASH

    . auto/module
fi

if [ $HTTP_UPSTREAM_LEAST_CONN = YES ]; then
    ngx_module_name=ngx_http_upstream_least_conn_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_least_conn_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_LEAST_CONN

    . auto/module
fi

if [ $HTTP_UPSTREAM_KEEPALIVE = YES ]; then
    ngx_module_name=ngx_http_upstream_keepalive_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_keepalive_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_KEEPALIVE

    . auto/module
fi

if [ $HTTP_UPSTREAM_ZONE = YES ]; then
    have=NGX_HTTP_UPSTREAM_ZONE . auto/have

    ngx_module_name=ngx_http_upstream_zone_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_zone_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_ZONE

    . auto/module
fi

if [ $HTTP_STUB_STATUS = YES ]; then
    have=NGX_STAT_STUB . auto/have

    ngx_module_name=ngx_http_stub_status_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_stub_status_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_STUB_STATUS

    . auto/module
fi


if [ $MAIL != NO ]; then
    MAIL_MODULES=
    MAIL_DEPS=
    MAIL_INCS=

    ngx_module_type=MAIL
    ngx_module_libs=
    ngx_module_link=YES

    ngx_module_order=

    ngx_module_name="ngx_mail_module ngx_mail_core_module"
    ngx_module_incs="src/mail"
    ngx_module_deps="src/mail/ngx_mail.h"
    ngx_module_srcs="src/mail/ngx_mail.c \
                     src/mail/ngx_mail_core_module.c \
                     src/mail/ngx_mail_handler.c \
                     src/mail/ngx_mail_parse.c"

    . auto/module

    ngx_module_incs=

    if [ $MAIL_SSL = YES ]; then
        USE_OPENSSL=YES
        have=NGX_MAIL_SSL . auto/have

        ngx_module_name=ngx_mail_ssl_module
        ngx_module_deps=src/mail/ngx_mail_ssl_module.h
        ngx_module_srcs=src/mail/ngx_mail_ssl_module.c

        . auto/module
    fi

    if [ $MAIL_POP3 = YES ]; then
        ngx_module_name=ngx_mail_pop3_module
        ngx_module_deps=src/mail/ngx_mail_pop3_module.h
        ngx_module_srcs="src/mail/ngx_mail_pop3_module.c \
                         src/mail/ngx_mail_pop3_handler.c"

        . auto/module
    fi

    if [ $MAIL_IMAP = YES ]; then
        ngx_module_name=ngx_mail_imap_module
        ngx_module_deps=src/mail/ngx_mail_imap_module.h
        ngx_module_srcs="src/mail/ngx_mail_imap_module.c \
                         src/mail/ngx_mail_imap_handler.c"

        . auto/module
    fi

    if [ $MAIL_SMTP = YES ]; then
        ngx_module_name=ngx_mail_smtp_module
        ngx_module_deps=src/mail/ngx_mail_smtp_module.h
        ngx_module_srcs="src/mail/ngx_mail_smtp_module.c \
                         src/mail/ngx_mail_smtp_handler.c"

        . auto/module
    fi

    ngx_module_name=ngx_mail_auth_http_module
    ngx_module_deps=
    ngx_module_srcs=src/mail/ngx_mail_auth_http_module.c

    . auto/module

    ngx_module_name=ngx_mail_proxy_module
    ngx_module_deps=
    ngx_module_srcs=src/mail/ngx_mail_proxy_module.c

    . auto/module
fi


if [ $STREAM != NO ]; then
    STREAM_MODULES=
    STREAM_DEPS=
    STREAM_INCS=

    ngx_module_type=STREAM
    ngx_module_libs=
    ngx_module_link=YES

    ngx_module_order=

    ngx_module_name="ngx_stream_module \
                     ngx_stream_core_module \
                     ngx_stream_proxy_module \
                     ngx_stream_upstream_module"
    ngx_module_incs="src/stream"
    ngx_module_deps="src/stream/ngx_stream.h \
                     src/stream/ngx_stream_upstream.h \
                     src/stream/ngx_stream_upstream_round_robin.h"
    ngx_module_srcs="src/stream/ngx_stream.c \
                     src/stream/ngx_stream_handler.c \
                     src/stream/ngx_stream_core_module.c \
                     src/stream/ngx_stream_proxy_module.c \
                     src/stream/ngx_stream_upstream.c \
                     src/stream/ngx_stream_upstream_round_robin.c"

    . auto/module

    ngx_module_incs=

    if [ $STREAM_SSL = YES ]; then
        USE_OPENSSL=YES
        have=NGX_STREAM_SSL . auto/have

        ngx_module_name=ngx_stream_ssl_module
        ngx_module_deps=src/stream/ngx_stream_ssl_module.h
        ngx_module_srcs=src/stream/ngx_stream_ssl_module.c

        . auto/module
    fi

    if [ $STREAM_LIMIT_CONN = YES ]; then
        ngx_module_name=ngx_stream_limit_conn_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_limit_conn_module.c

        . auto/module
    fi

    if [ $STREAM_ACCESS = YES ]; then
        ngx_module_name=ngx_stream_access_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_access_module.c

        . auto/module
    fi

    if [ $STREAM_UPSTREAM_HASH = YES ]; then
        ngx_module_name=ngx_stream_upstream_hash_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_upstream_hash_module.c

        . auto/module
    fi

    if [ $STREAM_UPSTREAM_LEAST_CONN = YES ]; then
        ngx_module_name=ngx_stream_upstream_least_conn_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_upstream_least_conn_module.c

        . auto/module
    fi

    if [ $STREAM_UPSTREAM_ZONE = YES ]; then
        have=NGX_STREAM_UPSTREAM_ZONE . auto/have

        ngx_module_name=ngx_stream_upstream_zone_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_upstream_zone_module.c

        . auto/module
    fi
fi


#if [ -r $NGX_OBJS/auto ]; then
#    . $NGX_OBJS/auto
#fi


if test -n "$NGX_ADDONS"; then

    echo configuring additional modules

    for ngx_addon_dir in $NGX_ADDONS
    do
        echo "adding module in $ngx_addon_dir"

        ngx_module_type=
        ngx_module_name=
        ngx_module_incs=
        ngx_module_deps=
        ngx_module_srcs=
        ngx_module_libs=
        ngx_module_order=
        ngx_module_link=ADDON

        if test -f $ngx_addon_dir/config; then
            . $ngx_addon_dir/config

            echo " + $ngx_addon_name was configured"

        else
            echo "$0: error: no $ngx_addon_dir/config was found"
            exit 1
        fi
    done
fi


if test -n "$DYNAMIC_ADDONS"; then

    echo configuring additional dynamic modules

    for ngx_addon_dir in $DYNAMIC_ADDONS
    do
        echo "adding module in $ngx_addon_dir"

        ngx_module_type=
        ngx_module_name=
        ngx_module_incs=
        ngx_module_deps=
        ngx_module_srcs=
        ngx_module_libs=
        ngx_module_order=
        ngx_module_link=DYNAMIC

        if test -f $ngx_addon_dir/config; then
            . $ngx_addon_dir/config

            echo " + $ngx_addon_name was configured"

        else
            echo "$0: error: no $ngx_addon_dir/config was found"
            exit 1
        fi
    done
fi


if [ $USE_OPENSSL = YES ]; then
    ngx_module_type=CORE
    ngx_module_name=ngx_openssl_module
    ngx_module_incs=
    ngx_module_deps=src/event/ngx_event_openssl.h
    ngx_module_srcs="src/event/ngx_event_openssl.c
                     src/event/ngx_event_openssl_stapling.c"
    ngx_module_libs=
    ngx_module_link=YES
    ngx_module_order=

    . auto/module
fi


if [ $USE_PCRE = YES ]; then
    ngx_module_type=CORE
    ngx_module_name=ngx_regex_module
    ngx_module_incs=
    ngx_module_deps=src/core/ngx_regex.h
    ngx_module_srcs=src/core/ngx_regex.c
    ngx_module_libs=
    ngx_module_link=YES
    ngx_module_order=

    . auto/module
fi


modules="$CORE_MODULES $EVENT_MODULES"


# thread pool module should be initialized after events
if [ $USE_THREADS = YES ]; then
    modules="$modules $THREAD_POOL_MODULE"
fi


if [ $HTTP = YES ]; then
    modules="$modules $HTTP_MODULES $HTTP_FILTER_MODULES \
             $HTTP_AUX_FILTER_MODULES $HTTP_INIT_FILTER_MODULES"

    NGX_ADDON_DEPS="$NGX_ADDON_DEPS \$(HTTP_DEPS)"
fi


if [ $MAIL != NO ]; then

    if [ $MAIL = YES ]; then
        modules="$modules $MAIL_MODULES"

    elif [ $MAIL = DYNAMIC ]; then
        ngx_module_name=$MAIL_MODULES
        ngx_module_incs=
        ngx_module_deps=$MAIL_DEPS
        ngx_module_srcs=$MAIL_SRCS
        ngx_module_libs=
        ngx_module_link=DYNAMIC

        . auto/module
    fi

    NGX_ADDON_DEPS="$NGX_ADDON_DEPS \$(MAIL_DEPS)"
fi


if [ $STREAM != NO ]; then

    if [ $STREAM = YES ]; then
        modules="$modules $STREAM_MODULES"

    elif [ $STREAM = DYNAMIC ]; then
        ngx_module_name=$STREAM_MODULES
        ngx_module_incs=
        ngx_module_deps=$STREAM_DEPS
        ngx_module_srcs=$STREAM_SRCS
        ngx_module_libs=
        ngx_module_link=DYNAMIC

        . auto/module
    fi

    NGX_ADDON_DEPS="$NGX_ADDON_DEPS \$(STREAM_DEPS)"
fi


ngx_module_type=MISC
MISC_MODULES=

if [ $NGX_GOOGLE_PERFTOOLS = YES ]; then
    ngx_module_name=ngx_google_perftools_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/misc/ngx_google_perftools_module.c
    ngx_module_libs=
    ngx_module_link=$NGX_GOOGLE_PERFTOOLS

    . auto/module
fi

if [ $NGX_CPP_TEST = YES ]; then
    ngx_module_name=
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/misc/ngx_cpp_test_module.cpp
    ngx_module_libs=-lstdc++
    ngx_module_link=$NGX_CPP_TEST

    . auto/module
fi

modules="$modules $MISC_MODULES"


cat << END                                    > $NGX_MODULES_C

#include <ngx_config.h>
#include <ngx_core.h>

$NGX_PRAGMA

END

for mod in $modules
do
    echo "extern ngx_module_t  $mod;"         >> $NGX_MODULES_C
done

echo                                          >> $NGX_MODULES_C
echo 'ngx_module_t *ngx_modules[] = {'        >> $NGX_MODULES_C

for mod in $modules
do
    echo "    &$mod,"                         >> $NGX_MODULES_C
done

cat << END                                    >> $NGX_MODULES_C
    NULL
};

END

echo 'char *ngx_module_names[] = {'           >> $NGX_MODULES_C

for mod in $modules
do
    echo "    \"$mod\","                      >> $NGX_MODULES_C
done

cat << END                                    >> $NGX_MODULES_C
    NULL
};

END
{% endhighlight %}



## 2. event模块
{% highlight string %}
if [ $EVENT_SELECT = NO -a $EVENT_FOUND = NO ]; then
    EVENT_SELECT=YES
fi

if [ $EVENT_SELECT = YES ]; then
    have=NGX_HAVE_SELECT . auto/have
    CORE_SRCS="$CORE_SRCS $SELECT_SRCS"
    EVENT_MODULES="$EVENT_MODULES $SELECT_MODULE"
fi


if [ $EVENT_POLL = NO -a $EVENT_FOUND = NO ]; then
    EVENT_POLL=YES
fi

if [ $EVENT_POLL = YES ]; then
    have=NGX_HAVE_POLL . auto/have
    CORE_SRCS="$CORE_SRCS $POLL_SRCS"
    EVENT_MODULES="$EVENT_MODULES $POLL_MODULE"
fi


if [ $NGX_TEST_BUILD_DEVPOLL = YES ]; then
    have=NGX_HAVE_DEVPOLL . auto/have
    have=NGX_TEST_BUILD_DEVPOLL . auto/have
    EVENT_MODULES="$EVENT_MODULES $DEVPOLL_MODULE"
    CORE_SRCS="$CORE_SRCS $DEVPOLL_SRCS"
fi


if [ $NGX_TEST_BUILD_EVENTPORT = YES ]; then
    have=NGX_HAVE_EVENTPORT . auto/have
    have=NGX_TEST_BUILD_EVENTPORT . auto/have
    EVENT_MODULES="$EVENT_MODULES $EVENTPORT_MODULE"
    CORE_SRCS="$CORE_SRCS $EVENTPORT_SRCS"
fi

if [ $NGX_TEST_BUILD_EPOLL = YES ]; then
    have=NGX_HAVE_EPOLL . auto/have
    have=NGX_HAVE_EPOLLRDHUP . auto/have
    have=NGX_HAVE_EVENTFD . auto/have
    have=NGX_TEST_BUILD_EPOLL . auto/have
    EVENT_MODULES="$EVENT_MODULES $EPOLL_MODULE"
    CORE_SRCS="$CORE_SRCS $EPOLL_SRCS"
fi
{% endhighlight %}

1) 首先判断```EVENT_SELECT```有没有被设置，并且是否```EVENT_FOUNT```变量是否找到了更先进的事件模型。如果两者都为否的话，则直接设置```EVENT_SELECT=YES```，从这里我们可以看出nginx做的很聪明，它会先检查是否有更高级的事件模型，没有的话才会选择大部分系统都自持的相对来说性能较低的select模型。

<pre>
默认情况下，在auto/options脚本中将EVENT_SELECT设置为了NO。

对于EVENT_FOUND，首先我们在auto/options脚本中初始设置为NO。然后在auto/os/linux脚本中对epoll
事件模型做了检测，接着在auto/unix脚本中对/dev/poll模型做了检测，后面又在auto/unix脚本中对
kqueue模型做了检测。

这里我们发现存在一个问题，对于中等效率的poll模型，其并没有通过EVENT_FOUND这一变量来指示。
但是nginx引入了另外一个变量EVENT_POLL，其首先在auto/options脚本中被设置为NO,然后在auto/unix
脚本中对poll模型进行检测，如果没有发现poll模型，则将EVENT_POLL设置为NONE。因此只要EVENT_POLL没有
被设置为NONE，就说明本身操作系统是支持poll模型的.
</pre>

<br />

2) 接着判断```EVENT_POLL```是否被设置，并且是否```EVENT_FOUND```找到了更高级的事件模型。如果两者都为NO的话，则直接将```EVENT_POLL```设置为YES。这是因为我们在上面已经讲到，如果系统本身不支持poll模型的话，```EVENT_POLL```会被设置为```NONE```，因此这里我们这样设置是没有问题的。

从这里我们可以看到对event模块的事件处理模型的选择，主要是通过```EVENT_SELECT```,```EVENT_POLL```,```EVENT_FOUND```这三个变量控制的。则三个变量可以确保nginx能找到一个当前系统所支持的最先进的事件模型.

<br />

3) 最后再对三种先进的事件模型进行处理。分别判断如下三个变量是否被设置：

* ```NGX_TEST_BUILD_DEVPOLL```
* ```NGX_TEST_BUILD_EVENTPORT```
* ```NGX_TEST_BUILD_EPOLL```

默认情况下这三个变量都被设置为```NO```,一般是用户明确你当前系统支持相应特性时可以显示通过如下参数进行设置：
*  ```--test-build-devpoll```
*  ```--test-build-eventport```
*  ```--test-build-epoll```

否则一般情况下，应该让系统自己检测然后选择使用哪一种事件模型。

<br />

从如上脚本我们看到，发现相应的事件模型后，其都是定义好对应的宏定义；然后在往```CORE_SRCS```，```CORE_MODULES```添加相应的源文件及对应的事件模块名。


## 3. Solaris sendfilev特性
{% highlight string %}
if [ $NGX_TEST_BUILD_SOLARIS_SENDFILEV = YES ]; then
    have=NGX_TEST_BUILD_SOLARIS_SENDFILEV . auto/have
    CORE_SRCS="$CORE_SRCS $SOLARIS_SENDFILEV_SRCS"
fi
{% endhighlight %}
如果显示设置了```NGX_TEST_BUILD_SOLARIS_SENDFILEV```，则将对应的源代码文件加入到CORE_SRCS变量中保存起来。

## 4. HTTP模块
{% highlight string %}
HTTP_MODULES=
HTTP_DEPS=
HTTP_INCS=

ngx_module_type=HTTP

if :; then
    ngx_module_name="ngx_http_module \
                     ngx_http_core_module \
                     ngx_http_log_module \
                     ngx_http_upstream_module"
    ngx_module_incs="src/http src/http/modules"
    ngx_module_deps="src/http/ngx_http.h \
                     src/http/ngx_http_request.h \
                     src/http/ngx_http_config.h \
                     src/http/ngx_http_core_module.h \
                     src/http/ngx_http_cache.h \
                     src/http/ngx_http_variables.h \
                     src/http/ngx_http_script.h \
                     src/http/ngx_http_upstream.h \
                     src/http/ngx_http_upstream_round_robin.h"
    ngx_module_srcs="src/http/ngx_http.c \
                     src/http/ngx_http_core_module.c \
                     src/http/ngx_http_special_response.c \
                     src/http/ngx_http_request.c \
                     src/http/ngx_http_parse.c \
                     src/http/modules/ngx_http_log_module.c \
                     src/http/ngx_http_request_body.c \
                     src/http/ngx_http_variables.c \
                     src/http/ngx_http_script.c \
                     src/http/ngx_http_upstream.c \
                     src/http/ngx_http_upstream_round_robin.c"
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi


if [ $HTTP != YES ]; then
    have=NGX_CRYPT . auto/nohave
    CRYPT_LIB=
fi


if [ $HTTP_CACHE = YES ]; then
    USE_MD5=YES
    have=NGX_HTTP_CACHE . auto/have
    HTTP_SRCS="$HTTP_SRCS $HTTP_FILE_CACHE_SRCS"
fi


if [ $HTTP_SSI = YES ]; then
    HTTP_POSTPONE=YES
fi


if [ $HTTP_SLICE = YES ]; then
    HTTP_POSTPONE=YES
fi


if [ $HTTP_ADDITION = YES ]; then
    HTTP_POSTPONE=YES
fi
{% endhighlight %}


首先将```HTTP_MODULES```,```HTTP_DEPS```,```HTTP_INCS```都置为空，然后将ngx_module_type变量置为```HTTP```。

接着设置```HTTP```模块所需要的源文件、头文件、库文件等，调用auto/module脚本将相应的信息设置到模块变量中。

然后再执行如下：
{% highlight string %}
if [ $HTTP != YES ]; then
    have=NGX_CRYPT . auto/nohave
    CRYPT_LIB=
fi
{% endhighlight %}
默认情况下，```HTTP```在auto/options中被置为```YES```.


执行如下处理```HTTP_CACHE```:
{% highlight string %}
if [ $HTTP_CACHE = YES ]; then
    USE_MD5=YES
    have=NGX_HTTP_CACHE . auto/have
    HTTP_SRCS="$HTTP_SRCS $HTTP_FILE_CACHE_SRCS"
fi
{% endhighlight %}
```HTTP_CACHE```默认在auto/options中被设置为```YES```，即启用缓存。


执行如下：
{% highlight string %}
if [ $HTTP_SSI = YES ]; then
    HTTP_POSTPONE=YES
fi


if [ $HTTP_SLICE = YES ]; then
    HTTP_POSTPONE=YES
fi


if [ $HTTP_ADDITION = YES ]; then
    HTTP_POSTPONE=YES
fi
{% endhighlight %}
默认情况下,```HTTP_SSI```被设置为```YES```；而```HTTP_SLICE```与```HTTP_ADDITION```被设置为```NO```。


## 5. HTTP_FILTER模块

**1) 初始化相关变量**
{% highlight string %}
# the module order is important
#     ngx_http_static_module
#     ngx_http_gzip_static_module
#     ngx_http_dav_module
#     ngx_http_autoindex_module
#     ngx_http_index_module
#     ngx_http_random_index_module
#
#     ngx_http_access_module
#     ngx_http_realip_module
#
#
# the filter order is important
#     ngx_http_write_filter
#     ngx_http_header_filter
#     ngx_http_chunked_filter
#     ngx_http_v2_filter
#     ngx_http_range_header_filter
#     ngx_http_gzip_filter
#     ngx_http_postpone_filter
#     ngx_http_ssi_filter
#     ngx_http_charset_filter
#         ngx_http_xslt_filter
#         ngx_http_image_filter
#         ngx_http_sub_filter
#         ngx_http_addition_filter
#         ngx_http_gunzip_filter
#         ngx_http_userid_filter
#         ngx_http_headers_filter
#     ngx_http_copy_filter
#     ngx_http_range_body_filter
#     ngx_http_not_modified_filter
#     ngx_http_slice_filter

ngx_module_type=HTTP_FILTER
HTTP_FILTER_MODULES=
{% endhighlight %}
这里将ngx_module_type初始化为```HTTP_FILTER```,```HTTP_FILTER_MODULES```被初始化为空。

**2) 定义ngx_module_order**
{% highlight string %}
ngx_module_order="ngx_http_static_module \
                  ngx_http_gzip_static_module \
                  ngx_http_dav_module \
                  ngx_http_autoindex_module \
                  ngx_http_index_module \
                  ngx_http_random_index_module \
                  ngx_http_access_module \
                  ngx_http_realip_module \
                  ngx_http_write_filter_module \
                  ngx_http_header_filter_module \
                  ngx_http_chunked_filter_module \
                  ngx_http_v2_filter_module \
                  ngx_http_range_header_filter_module \
                  ngx_http_gzip_filter_module \
                  ngx_http_postpone_filter_module \
                  ngx_http_ssi_filter_module \
                  ngx_http_charset_filter_module \
                  ngx_http_xslt_filter_module \
                  ngx_http_image_filter_module \
                  ngx_http_sub_filter_module \
                  ngx_http_addition_filter_module \
                  ngx_http_gunzip_filter_module \
                  ngx_http_userid_filter_module \
                  ngx_http_headers_filter_module \
                  ngx_http_copy_filter_module \
                  ngx_http_range_body_filter_module \
                  ngx_http_not_modified_filter_module \
                  ngx_http_slice_filter_module"
{% endhighlight %}
这是因为在auto/options中，如下几个module都有可能会被设置为dynamic:

* HTTP_XSLT
* HTTP_IMAGE_FILTER
* HTTP_GEOIP
* HTTP_PERL
* MAIL
* STREAM

因此在下面有可能会处理这些模块时，首先定义好ngx_module_order。


**3) 处理ngx_http_write_filter_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_write_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/ngx_http_write_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi
{% endhighlight %}
ngx_http_write_filter_module作为内部静态模块处理。

**4) 处理ngx_http_header_filter_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_header_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/ngx_http_header_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi
{% endhighlight %}
ngx_http_header_filter_module作为内部静态模块处理。

**5) 处理ngx_http_chunked_filter_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_chunked_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_chunked_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi
{% endhighlight %}
ngx_http_chunked_filter_module作为内部静态模块处理。

**6) 处理ngx_http_v2_filter_module**
{% highlight string %}
if [ $HTTP_V2 = YES ]; then
    ngx_module_name=ngx_http_v2_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/v2/ngx_http_v2_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_V2

    . auto/module
fi
{% endhighlight %}
默认情况下```HTTP_V2```被设置为```NO```. ```HTTP_V2```主要用于提供对http/2的支持。

参看:[Module ngx_http_v2_module](http://nginx.org/en/docs/http/ngx_http_v2_module.html)


**7) 处理ngx_http_range_header_filter_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_range_header_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_range_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi
{% endhighlight %}

ngx_http_range_header_filter_module作为内部静态模块处理。

**8) 处理ngx_http_gzip_filter_module**
{% highlight string %}
if [ $HTTP_GZIP = YES ]; then
    have=NGX_HTTP_GZIP . auto/have
    USE_ZLIB=YES

    ngx_module_name=ngx_http_gzip_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_gzip_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_GZIP

    . auto/module
fi
{% endhighlight %}
auto/options中默认会启用```HTTP_GZIP```压缩。

**9) 处理ngx_http_postpone_filter_module**
{% highlight string %}
if [ $HTTP_POSTPONE = YES ]; then
    ngx_module_name=ngx_http_postpone_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/ngx_http_postpone_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_POSTPONE

    . auto/module
fi
{% endhighlight %}
上面我们提到这里```HTTP_POSTPONE```会被置为```YES```，因此会启用postpone filter模块。

**10) 处理ngx_http_ssi_filter_module**
{% highlight string %}
if [ $HTTP_SSI = YES ]; then
    have=NGX_HTTP_SSI . auto/have

    ngx_module_name=ngx_http_ssi_filter_module
    ngx_module_incs=
    ngx_module_deps=src/http/modules/ngx_http_ssi_filter_module.h
    ngx_module_srcs=src/http/modules/ngx_http_ssi_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SSI

    . auto/module
fi
{% endhighlight %}
默认情况下```HTTP_SSI```会被置为```YES```，因此这里会启用ssi filter模块。

**11) 处理ngx_http_charset_filter_module**
{% highlight string %}
if [ $HTTP_CHARSET = YES ]; then
    ngx_module_name=ngx_http_charset_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_charset_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_CHARSET

    . auto/module
fi
{% endhighlight %}
默认情况下```HTTP_CHARSET```会被置为```YES```,因此这里会启用charset filter模块。

**12) 处理ngx_http_xslt_filter_module**
{% highlight string %}
if [ $HTTP_XSLT != NO ]; then
    ngx_module_name=ngx_http_xslt_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_xslt_filter_module.c
    ngx_module_libs=LIBXSLT
    ngx_module_link=$HTTP_XSLT

    . auto/module
fi
{% endhighlight %}
在auto/options中```HTTP_XSLT```默认被设置为```NO```，但其可以通过如下：
<pre>
--with-http_xslt_module)         HTTP_XSLT=YES              ;;
--with-http_xslt_module=dynamic) HTTP_XSLT=DYNAMIC          ;;
</pre>
来设置编译为```内部静态模块```或```外部动态模块```。

**13) 处理ngx_http_image_filter_module**
{% highlight string %}
if [ $HTTP_IMAGE_FILTER != NO ]; then
    ngx_module_name=ngx_http_image_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_image_filter_module.c
    ngx_module_libs=LIBGD
    ngx_module_link=$HTTP_IMAGE_FILTER

    . auto/module
fi
{% endhighlight %}
在auto/options中```HTTP_IMAGE_FILTER```默认被设置为```NO```，但是其可以通过如下:
<pre>
--with-http_image_filter_module) HTTP_IMAGE_FILTER=YES      ;;
--with-http_image_filter_module=dynamic)
                                 HTTP_IMAGE_FILTER=DYNAMIC  ;;
</pre>
来设置编译为```内部静态模块```或```外部动态模块```.


**14) 处理ngx_http_sub_filter_module**
{% highlight string %}
if [ $HTTP_SUB = YES ]; then
    ngx_module_name=ngx_http_sub_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_sub_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SUB

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中```HTTP_SUB```默认被设置为```NO```。

**15) 处理ngx_http_addition_filter_module**
{% highlight string %}
if [ $HTTP_ADDITION = YES ]; then
    ngx_module_name=ngx_http_addition_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_addition_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_ADDITION

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中```HTTP_ADDITION```默认被设置为```NO```.

**16) 处理ngx_http_gunzip_filter_module**
{% highlight string %}
if [ $HTTP_GUNZIP = YES ]; then
    have=NGX_HTTP_GZIP . auto/have
    USE_ZLIB=YES

    ngx_module_name=ngx_http_gunzip_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_gunzip_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_GUNZIP

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中```HTTP_GUNZIP```默认被设置为```NO```。

**17) 处理ngx_http_userid_filter_module**
{% highlight string %}
if [ $HTTP_USERID = YES ]; then
    ngx_module_name=ngx_http_userid_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_userid_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_USERID

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中```HTTP_USERID```默认被设置为```YES```。

**18) 处理ngx_http_headers_filter_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_headers_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_headers_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi

{% endhighlight %}
ngx_http_headers_filter_module作为内部静态模块处理.


## 6. HTTP_INIT_FILTER模块

**1) 初始化相关变量**
{% highlight string %}
ngx_module_type=HTTP_INIT_FILTER
HTTP_INIT_FILTER_MODULES=
{% endhighlight %}
这里将ngx_module_type初始化为```HTTP_INIT_FILTER```,```HTTP_INIT_FILTER_MODULES```被初始化为空。

**2) 处理ngx_http_copy_filter_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_copy_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/ngx_http_copy_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi
{% endhighlight %}
ngx_http_copy_filter_module作为内部静态模块处理。

**3) 处理ngx_http_range_body_filter_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_range_body_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi
{% endhighlight %}
ngx_http_range_body_filter_module作为内部静态模块处理。

**4) 处理ngx_http_not_modified_filter_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_not_modified_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_not_modified_filter_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi
{% endhighlight %}
ngx_http_not_modified_filter_module作为内部静态模块处理。

**5) 处理ngx_http_slice_filter_module**
{% highlight string %}
if [ $HTTP_SLICE = YES ]; then
    ngx_module_name=ngx_http_slice_filter_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_slice_filter_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SLICE

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中```HTTP_SLICE```默认被设置为```NO```。


## 7. HTTP模块(续)

**1) 初始化相关变量**
{% highlight string %}
ngx_module_type=HTTP
{% endhighlight %}

将ngx_module_type初始化为```HTTP```。

**2) 处理ngx_http_v2_module**
{% highlight string %}
if [ $HTTP_V2 = YES ]; then
    have=NGX_HTTP_V2 . auto/have

    ngx_module_name=ngx_http_v2_module
    ngx_module_incs=src/http/v2
    ngx_module_deps="src/http/v2/ngx_http_v2.h src/http/v2/ngx_http_v2_module.h"
    ngx_module_srcs="src/http/v2/ngx_http_v2.c \
                     src/http/v2/ngx_http_v2_table.c \
                     src/http/v2/ngx_http_v2_huff_decode.c \
                     src/http/v2/ngx_http_v2_huff_encode.c \
                     src/http/v2/ngx_http_v2_module.c"
    ngx_module_libs=
    ngx_module_link=$HTTP_V2

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中```HTTP_V2```默认设置为```NO```.

**3) 处理ngx_http_static_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_static_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_static_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi
{% endhighlight %}
ngx_http_static_module作为内部静态模块处理.

**4) 处理ngx_http_gzip_static_module**
{% highlight string %}
if [ $HTTP_GZIP_STATIC = YES ]; then
    have=NGX_HTTP_GZIP . auto/have

    ngx_module_name=ngx_http_gzip_static_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_gzip_static_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_GZIP_STATIC

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_GZIP_STATIC```默认被设置为```NO```。


**5) 处理ngx_http_dav_module**
{% highlight string %}
if [ $HTTP_DAV = YES ]; then
    have=NGX_HTTP_DAV . auto/have

    ngx_module_name=ngx_http_dav_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_dav_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_DAV

    . auto/module
fi
{% endhighlight %}

在auto/options脚本中，```HTTP_DAV```默认被设置为```NO```。

**6) 处理ngx_http_autoindex_module**
{% highlight string %}
if [ $HTTP_AUTOINDEX = YES ]; then
    ngx_module_name=ngx_http_autoindex_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_autoindex_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_AUTOINDEX

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_AUTOINDEX```默认被设置为```YES```。

**7) 处理ngx_http_index_module**
{% highlight string %}
if :; then
    ngx_module_name=ngx_http_index_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_index_module.c
    ngx_module_libs=
    ngx_module_link=YES

    . auto/module
fi
{% endhighlight %}

ngx_http_index_module作为内部静态模块处理。

**8) 处理ngx_http_random_index_module**
{% highlight string %}
if [ $HTTP_RANDOM_INDEX = YES ]; then
    ngx_module_name=ngx_http_random_index_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_random_index_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_RANDOM_INDEX

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_RANDOM_INDEX```默认被设置为```NO```。

**9) 处理ngx_http_auth_request_module**
{% highlight string %}
if [ $HTTP_AUTH_REQUEST = YES ]; then
    ngx_module_name=ngx_http_auth_request_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_auth_request_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_AUTH_REQUEST

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_AUTH_REQUEST```默认被设置为```NO```。

**10) 处理ngx_http_auth_basic_module**
{% highlight string %}
if [ $HTTP_AUTH_BASIC = YES ]; then
    USE_MD5=YES
    USE_SHA1=YES
    have=NGX_CRYPT . auto/have

    ngx_module_name=ngx_http_auth_basic_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_auth_basic_module.c
    ngx_module_libs=$CRYPT_LIB
    ngx_module_link=$HTTP_AUTH_BASIC

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_AUTH_BASIC```默认被设置为```YES```.

**11) 处理ngx_http_access_module**
{% highlight string %}
if [ $HTTP_ACCESS = YES ]; then
    ngx_module_name=ngx_http_access_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_access_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_ACCESS

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_ACCESS```默认被设置为```YES```。

**12) 处理ngx_http_limit_conn_module**
{% highlight string %}
if [ $HTTP_LIMIT_CONN = YES ]; then
    ngx_module_name=ngx_http_limit_conn_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_limit_conn_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_LIMIT_CONN

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_LIMIT_CONN```默认被设置为```YES```。

**13) 处理ngx_http_limit_req_module**
{% highlight string %}
if [ $HTTP_LIMIT_REQ = YES ]; then
    ngx_module_name=ngx_http_limit_req_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_limit_req_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_LIMIT_REQ

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_LIMIT_REQ```默认被设置为```YES```。

**14) 处理ngx_http_realip_module**
{% highlight string %}
if [ $HTTP_REALIP = YES ]; then
    have=NGX_HTTP_REALIP . auto/have
    have=NGX_HTTP_X_FORWARDED_FOR . auto/have

    ngx_module_name=ngx_http_realip_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_realip_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_REALIP

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_REALIP```默认被设置为```NO```。

**15) 处理ngx_http_status_module**
{% highlight string %}
if [ $HTTP_STATUS = YES ]; then
    ngx_module_name=ngx_http_status_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_status_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_STATUS

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_STATUS```默认被设置为```NO```。


**16) 处理ngx_http_geo_module**
{% highlight string %}
if [ $HTTP_GEO = YES ]; then
    have=NGX_HTTP_X_FORWARDED_FOR . auto/have

    ngx_module_name=ngx_http_geo_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_geo_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_GEO

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_GEO```默认被设置为```YES```。

**17) 处理ngx_http_geoip_module**
{% highlight string %}
if [ $HTTP_GEOIP != NO ]; then
    have=NGX_HTTP_X_FORWARDED_FOR . auto/have

    ngx_module_name=ngx_http_geoip_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_geoip_module.c
    ngx_module_libs=GEOIP
    ngx_module_link=$HTTP_GEOIP

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_GEOIP```默认被设置为```NO```。

**18) 处理ngx_http_map_module**
{% highlight string %}
if [ $HTTP_MAP = YES ]; then
    ngx_module_name=ngx_http_map_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_map_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_MAP

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_MAP```默认被设置为```YES```。

**19) 处理ngx_http_split_clients_module**
{% highlight string %}
if [ $HTTP_SPLIT_CLIENTS = YES ]; then
    ngx_module_name=ngx_http_split_clients_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_split_clients_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SPLIT_CLIENTS

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_SPLIT_CLIENTS```默认被设置为```YES```。

**20) 处理ngx_http_referer_module**
{% highlight string %}
if [ $HTTP_REFERER = YES ]; then
    ngx_module_name=ngx_http_referer_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_referer_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_REFERER

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_REFERER```默认被设置为```YES```。

**21) 处理ngx_http_rewrite_module**
{% highlight string %}
if [ $HTTP_REWRITE = YES -a $USE_PCRE != DISABLED ]; then
    USE_PCRE=YES

    ngx_module_name=ngx_http_rewrite_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_rewrite_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_REWRITE

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_REWRITE```默认被设置为```YES```，```USE_PCRE```默认被设置为```NO```。

**22) 处理ngx_http_ssl_module**
{% highlight string %}
if [ $HTTP_SSL = YES ]; then
    USE_OPENSSL=YES
    have=NGX_HTTP_SSL . auto/have

    ngx_module_name=ngx_http_ssl_module
    ngx_module_incs=
    ngx_module_deps=src/http/modules/ngx_http_ssl_module.h
    ngx_module_srcs=src/http/modules/ngx_http_ssl_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SSL

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_SSL```默认被设置为```NO```。

**23) 处理ngx_http_proxy_module**
{% highlight string %}
if [ $HTTP_PROXY = YES ]; then
    have=NGX_HTTP_X_FORWARDED_FOR . auto/have
    #USE_MD5=YES

    ngx_module_name=ngx_http_proxy_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_proxy_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_PROXY

    . auto/module
fi
{% endhighlight %}

在auto/options脚本中，```HTTP_PROXY```默认被设置为```YES```。


**24) 处理ngx_http_fastcgi_module**
{% highlight string %}
if [ $HTTP_FASTCGI = YES ]; then
    ngx_module_name=ngx_http_fastcgi_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_fastcgi_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_FASTCGI

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_FASTCGI```默认被设置为```YES```。

**25) 处理ngx_http_uwsgi_module**
{% highlight string %}
if [ $HTTP_UWSGI = YES ]; then
    ngx_module_name=ngx_http_uwsgi_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_uwsgi_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UWSGI

    . auto/module
fi
{% endhighlight %}

在auto/options脚本中，```HTTP_UWSGI```默认被设置为```YES```。

**26) 处理ngx_http_scgi_module**
{% highlight string %}
if [ $HTTP_SCGI = YES ]; then
    ngx_module_name=ngx_http_scgi_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_scgi_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SCGI

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_SCGI```默认被设置为```YES```。

**27) 处理ngx_http_perl_module**
{% highlight string %}
if [ $HTTP_PERL != NO ]; then
    ngx_module_name=ngx_http_perl_module
    ngx_module_incs=src/http/modules/perl
    ngx_module_deps=src/http/modules/perl/ngx_http_perl_module.h
    ngx_module_srcs=src/http/modules/perl/ngx_http_perl_module.c
    ngx_module_libs=PERL
    ngx_module_link=$HTTP_PERL

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_PERL```默认被设置为```NO```。

**28) 处理ngx_http_memcached_module**
{% highlight string %}
if [ $HTTP_MEMCACHED = YES ]; then
    ngx_module_name=ngx_http_memcached_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_memcached_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_MEMCACHED

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_MEMCACHED```默认被设置为```YES```。

**29) 处理ngx_http_empty_gif_module**
{% highlight string %}
if [ $HTTP_EMPTY_GIF = YES ]; then
    ngx_module_name=ngx_http_empty_gif_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_empty_gif_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_EMPTY_GIF

    . auto/module
fi
{% endhighlight %}

在auto/options脚本中，```HTTP_EMPTY_GIF```默认被设置为```YES```。

**30) 处理ngx_http_browser_module**
{% highlight string %}
if [ $HTTP_BROWSER = YES ]; then
    ngx_module_name=ngx_http_browser_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_browser_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_BROWSER

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_BROWSER```默认被设置为```YES```。

**31) 处理ngx_http_secure_link_module**
{% highlight string %}
if [ $HTTP_SECURE_LINK = YES ]; then
    USE_MD5=YES

    ngx_module_name=ngx_http_secure_link_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_secure_link_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_SECURE_LINK

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_SECURE_LINK```默认被设置为```NO```。

**32) 处理ngx_http_degradation_module**
{% highlight string %}
if [ $HTTP_DEGRADATION = YES ]; then
    have=NGX_HTTP_DEGRADATION . auto/have

    ngx_module_name=ngx_http_degradation_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_degradation_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_DEGRADATION

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_DEGRADATION```默认被设置为```NO```。

**33) 处理ngx_http_flv_module**
{% highlight string %}
if [ $HTTP_FLV = YES ]; then
    ngx_module_name=ngx_http_flv_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_flv_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_FLV

    . auto/module
fi
{% endhighlight %}

在auto/options脚本中,```HTTP_FLV```默认被设置为```NO```。


**34) 处理ngx_http_mp4_module**
{% highlight string %}
if [ $HTTP_MP4 = YES ]; then
    ngx_module_name=ngx_http_mp4_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_mp4_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_MP4

    . auto/module
fi
{% endhighlight %}

在auto/options脚本中,```HTTP_MP4```默认被设置为```NO```.

**35) 处理ngx_http_upstream_hash_module**
{% highlight string %}
if [ $HTTP_UPSTREAM_HASH = YES ]; then
    ngx_module_name=ngx_http_upstream_hash_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_hash_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_HASH

    . auto/module
fi
{% endhighlight %}

在auto/options脚本中,```HTTP_UPSTREAM_HASH```默认被设置为```YES```。

**36) 处理ngx_http_upstream_ip_hash_module**
{% highlight string %}
if [ $HTTP_UPSTREAM_IP_HASH = YES ]; then
    ngx_module_name=ngx_http_upstream_ip_hash_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_ip_hash_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_IP_HASH

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_UPSTREAM_IP_HASH```默认被设置为```YES```。

**37) 处理ngx_http_upstream_least_conn_module**
{% highlight string %}
if [ $HTTP_UPSTREAM_LEAST_CONN = YES ]; then
    ngx_module_name=ngx_http_upstream_least_conn_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_least_conn_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_LEAST_CONN

    . auto/module
fi
{% endhighlight %}

在auto/options脚本中,```HTTP_UPSTREAM_LEAST_CONN```默认被设置为```YES```。

**38) 处理ngx_http_upstream_keepalive_module**
{% highlight string %}
if [ $HTTP_UPSTREAM_KEEPALIVE = YES ]; then
    ngx_module_name=ngx_http_upstream_keepalive_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_keepalive_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_KEEPALIVE

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```HTTP_UPSTREAM_KEEPALIVE```默认被设置为```YES```。

**39) 处理ngx_http_upstream_zone_module**
{% highlight string %}
if [ $HTTP_UPSTREAM_ZONE = YES ]; then
    have=NGX_HTTP_UPSTREAM_ZONE . auto/have

    ngx_module_name=ngx_http_upstream_zone_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_upstream_zone_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_UPSTREAM_ZONE

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_UPSTREAM_ZONE```默认被设置为```YES```。


**40) 处理ngx_http_stub_status_module**
{% highlight string %}
if [ $HTTP_STUB_STATUS = YES ]; then
    have=NGX_STAT_STUB . auto/have

    ngx_module_name=ngx_http_stub_status_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/http/modules/ngx_http_stub_status_module.c
    ngx_module_libs=
    ngx_module_link=$HTTP_STUB_STATUS

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中，```HTTP_STUB_STATUS```默认被设置为```NO```。

## 7. MAIL模块

**1) 处理MAIL模块**
{% highlight string %}
if [ $MAIL != NO ]; then

# 略

fi
{% endhighlight %}
在auto/options脚本中，```MAIL```默认被初始化为```NO```。所以如下```MAIL```模块其实并不会执行.

**2) 初始化相关变量**
{% highlight string %}
    MAIL_MODULES=
    MAIL_DEPS=
    MAIL_INCS=

    ngx_module_type=MAIL
    ngx_module_libs=
    ngx_module_link=YES

    ngx_module_order=
{% endhighlight %}

**3) 处理ngx_mail_module**
{% highlight string %}
    ngx_module_name="ngx_mail_module ngx_mail_core_module"
    ngx_module_incs="src/mail"
    ngx_module_deps="src/mail/ngx_mail.h"
    ngx_module_srcs="src/mail/ngx_mail.c \
                     src/mail/ngx_mail_core_module.c \
                     src/mail/ngx_mail_handler.c \
                     src/mail/ngx_mail_parse.c"

    . auto/module
{% endhighlight %}
ngx_mail_module作为内部静态模块处理。

**4) 处理ngx_mail_ssl_module**
{% highlight string %}
    ngx_module_incs=

    if [ $MAIL_SSL = YES ]; then
        USE_OPENSSL=YES
        have=NGX_MAIL_SSL . auto/have

        ngx_module_name=ngx_mail_ssl_module
        ngx_module_deps=src/mail/ngx_mail_ssl_module.h
        ngx_module_srcs=src/mail/ngx_mail_ssl_module.c

        . auto/module
    fi
{% endhighlight %}

在auto/options脚本中,```MAIL_SSL```默认被初始化为```NO```.

```注意：ngx_module_incs被初始化为空```

**5) 处理ngx_mail_pop3_module**
{% highlight string %}
    if [ $MAIL_POP3 = YES ]; then
        ngx_module_name=ngx_mail_pop3_module
        ngx_module_deps=src/mail/ngx_mail_pop3_module.h
        ngx_module_srcs="src/mail/ngx_mail_pop3_module.c \
                         src/mail/ngx_mail_pop3_handler.c"

        . auto/module
    fi
{% endhighlight %}
在auto/options脚本中，```MAIL_POP3```默认被初始化为```YES```。

```注意：ngx_module_incs被初始化为空```


**6) 处理ngx_mail_imap_module**
{% highlight string %}
    if [ $MAIL_IMAP = YES ]; then
        ngx_module_name=ngx_mail_imap_module
        ngx_module_deps=src/mail/ngx_mail_imap_module.h
        ngx_module_srcs="src/mail/ngx_mail_imap_module.c \
                         src/mail/ngx_mail_imap_handler.c"

        . auto/module
    fi
{% endhighlight %}
在auto/options脚本中，```MAIL_IMAP```默认被初始化为```YES```.

```注意：ngx_module_incs被初始化为空```


**7) 处理ngx_mail_smtp_module**
{% highlight string %}
    if [ $MAIL_SMTP = YES ]; then
        ngx_module_name=ngx_mail_smtp_module
        ngx_module_deps=src/mail/ngx_mail_smtp_module.h
        ngx_module_srcs="src/mail/ngx_mail_smtp_module.c \
                         src/mail/ngx_mail_smtp_handler.c"

        . auto/module
    fi
{% endhighlight %}
在auto/options脚本中，```MAIL_SMTP```默认被初始化为```YES```。


```注意：ngx_module_incs被初始化为空```


**8) ngx_mail_auth_http_module**
{% highlight string %}
    ngx_module_name=ngx_mail_auth_http_module
    ngx_module_deps=
    ngx_module_srcs=src/mail/ngx_mail_auth_http_module.c

    . auto/module
{% endhighlight %}
ngx_mail_auth_http_module作为内部静态模块处理。

```注意：ngx_module_incs被初始化为空```


**9) 处理ngx_mail_proxy_module**
{% highlight string %}
    ngx_module_name=ngx_mail_proxy_module
    ngx_module_deps=
    ngx_module_srcs=src/mail/ngx_mail_proxy_module.c

    . auto/module
{% endhighlight %}
ngx_mail_proxy_module作为内部静态模块处理。


```注意：ngx_module_incs被初始化为空```



## 8. STREAM模块

**1) 处理STREAM模块**
{% highlight string %}
if [ $STREAM != NO ]; then

# 略

fi
{% endhighlight %}
在auto/options脚本中，```STREAM```默认被设置为```NO```，所以如下```STREAM```模块其实并不会被执行。

**2) 初始化相关便利**
{% highlight string %}
    STREAM_MODULES=
    STREAM_DEPS=
    STREAM_INCS=

    ngx_module_type=STREAM
    ngx_module_libs=
    ngx_module_link=YES

    ngx_module_order=
{% endhighlight %}


**3) 处理ngx_stream_module**
{% highlight string %}
    ngx_module_name="ngx_stream_module \
                     ngx_stream_core_module \
                     ngx_stream_proxy_module \
                     ngx_stream_upstream_module"
    ngx_module_incs="src/stream"
    ngx_module_deps="src/stream/ngx_stream.h \
                     src/stream/ngx_stream_upstream.h \
                     src/stream/ngx_stream_upstream_round_robin.h"
    ngx_module_srcs="src/stream/ngx_stream.c \
                     src/stream/ngx_stream_handler.c \
                     src/stream/ngx_stream_core_module.c \
                     src/stream/ngx_stream_proxy_module.c \
                     src/stream/ngx_stream_upstream.c \
                     src/stream/ngx_stream_upstream_round_robin.c"

    . auto/module
{% endhighlight %}
ngx_stream_module作为内部静态模块处理。

**4) 处理ngx_stream_ssl_module**
{% highlight string %}
    ngx_module_incs=

    if [ $STREAM_SSL = YES ]; then
        USE_OPENSSL=YES
        have=NGX_STREAM_SSL . auto/have

        ngx_module_name=ngx_stream_ssl_module
        ngx_module_deps=src/stream/ngx_stream_ssl_module.h
        ngx_module_srcs=src/stream/ngx_stream_ssl_module.c

        . auto/module
    fi
{% endhighlight %}
在auto/options脚本中，```STREAM_SSL```默认被初始化为```NO```。


```注意：ngx_module_incs被初始化为空```

**5) 处理ngx_stream_limit_conn_module**
{% highlight string %}
    if [ $STREAM_LIMIT_CONN = YES ]; then
        ngx_module_name=ngx_stream_limit_conn_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_limit_conn_module.c

        . auto/module
    fi
{% endhighlight %}
在auto/options脚本中，```STREAM_LIMIT_CONN```默认被初始化为```YES```。


```注意：ngx_module_incs被初始化为空```

**6) 处理ngx_stream_access_module**
{% highlight string %}
    if [ $STREAM_ACCESS = YES ]; then
        ngx_module_name=ngx_stream_access_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_access_module.c

        . auto/module
    fi
{% endhighlight %}

在auto/options脚本中，```STREAM_ACCESS```默认被初始化为```YES```。


```注意：ngx_module_incs被初始化为空```



**7) 处理ngx_stream_upstream_hash_module**
{% highlight string %}
    if [ $STREAM_UPSTREAM_HASH = YES ]; then
        ngx_module_name=ngx_stream_upstream_hash_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_upstream_hash_module.c

        . auto/module
    fi
{% endhighlight %}
在auto/options脚本中，```STREAM_UPSTREAM_HASH```默认被初始化为```YES```.


```注意：ngx_module_incs被初始化为空```

**8) 处理ngx_stream_upstream_least_conn_module**
{% highlight string %}
    if [ $STREAM_UPSTREAM_LEAST_CONN = YES ]; then
        ngx_module_name=ngx_stream_upstream_least_conn_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_upstream_least_conn_module.c

        . auto/module
    fi
{% endhighlight %}

在auto/options脚本中，```STREAM_UPSTREAM_LEAST_CONN```默认被初始化为```YES```。


```注意：ngx_module_incs被初始化为空```


**9) 处理ngx_stream_upstream_zone_module**
{% highlight string %}
    if [ $STREAM_UPSTREAM_ZONE = YES ]; then
        have=NGX_STREAM_UPSTREAM_ZONE . auto/have

        ngx_module_name=ngx_stream_upstream_zone_module
        ngx_module_deps=
        ngx_module_srcs=src/stream/ngx_stream_upstream_zone_module.c

        . auto/module
    fi
{% endhighlight %}
在auto/options脚本中，```STREAM_UPSTREAM_ZONE```默认被初始化为```YES```。


```注意：ngx_module_incs被初始化为空```


## 9. 处理外部静态模块(ADDONS)
{% highlight string %}
if test -n "$NGX_ADDONS"; then

    echo configuring additional modules

    for ngx_addon_dir in $NGX_ADDONS
    do
        echo "adding module in $ngx_addon_dir"

        ngx_module_type=
        ngx_module_name=
        ngx_module_incs=
        ngx_module_deps=
        ngx_module_srcs=
        ngx_module_libs=
        ngx_module_order=
        ngx_module_link=ADDON

        if test -f $ngx_addon_dir/config; then
            . $ngx_addon_dir/config

            echo " + $ngx_addon_name was configured"

        else
            echo "$0: error: no $ngx_addon_dir/config was found"
            exit 1
        fi
    done
fi
{% endhighlight %}


由于我们并没有设置```NGX_ADDONS```，因此本段脚本其实并不会被执行。这里我们简单分析一下脚本：首先遍历```NGX_ADDONS```，将ngx_module_link置为```ADDON```,然后调用对应目录下的config脚本文件。

例如，我们有ngx_http_hello_world_module,其config一般类似于如下:
{% highlight string %}
ngx_addon_name=ngx_http_hello_world_module

if test -n "$ngx_module_link"; then
    ngx_module_type=HTTP
    ngx_module_name=ngx_http_hello_world_module
    ngx_module_srcs="$ngx_addon_dir/ngx_http_hello_world_module.c"

    . auto/module
else
    HTTP_MODULES="$HTTP_MODULES ngx_http_hello_world_module"
    NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_hello_world_module.c"
fi
{% endhighlight %}

## 10. 处理外部动态加载模块(DYNAMIC ADDONS)
{% highlight string %}
if test -n "$DYNAMIC_ADDONS"; then

    echo configuring additional dynamic modules

    for ngx_addon_dir in $DYNAMIC_ADDONS
    do
        echo "adding module in $ngx_addon_dir"

        ngx_module_type=
        ngx_module_name=
        ngx_module_incs=
        ngx_module_deps=
        ngx_module_srcs=
        ngx_module_libs=
        ngx_module_order=
        ngx_module_link=DYNAMIC

        if test -f $ngx_addon_dir/config; then
            . $ngx_addon_dir/config

            echo " + $ngx_addon_name was configured"

        else
            echo "$0: error: no $ngx_addon_dir/config was found"
            exit 1
        fi
    done
fi
{% endhighlight %}
由于我们并没有设置```DYNAMIC_ADDONS```，因此本段脚本其实并不会被执行。关于脚本的分析与上述“处理外部静态模块(ADDONS)”类似，这里不再赘述。

## 11. 处理CORE_MODULES

```CORE_MODULES```在auto/sources中被初始化为：
<pre>
CORE_MODULES="ngx_core_module ngx_errlog_module ngx_conf_module"
</pre>
它是属于Nginx框架最核心、最基础的模块。我们后续分析时也会首要分析这三个模块。

**1) 处理ngx_openssl_module**
{% highlight string %}
if [ $USE_OPENSSL = YES ]; then
    ngx_module_type=CORE
    ngx_module_name=ngx_openssl_module
    ngx_module_incs=
    ngx_module_deps=src/event/ngx_event_openssl.h
    ngx_module_srcs="src/event/ngx_event_openssl.c
                     src/event/ngx_event_openssl_stapling.c"
    ngx_module_libs=
    ngx_module_link=YES
    ngx_module_order=

    . auto/module
fi
{% endhighlight %}
在auto/options脚本中,```USE_OPENSSL```默认被初始化为```NO```，但是因为我们SSL的开启还会受到如下三个模块的影响：

* HTTP_SSL
* MAIL_SSL
* STREAM_SSL

这里我们在执行congfigure脚本时通过```--with-http_ssl_module```选项启用了http ssl，因此也会自动使能```USE_OPENSSL```，从而将ngx_openssl_module模块也纳入到核心模块中。


**2) 处理ngx_regex_module**
{% highlight string %}
if [ $USE_PCRE = YES ]; then
    ngx_module_type=CORE
    ngx_module_name=ngx_regex_module
    ngx_module_incs=
    ngx_module_deps=src/core/ngx_regex.h
    ngx_module_srcs=src/core/ngx_regex.c
    ngx_module_libs=
    ngx_module_link=YES
    ngx_module_order=

    . auto/module
fi
{% endhighlight %}
这里我们在执行configure脚本时通过```--with-pcre```选项启用了pcre，因此```USE_PCRE```的值为```YES```，从而将ngx_regex_module纳入到核心模块之中。

## 12. 合并MODULES模块
前面我们提到的```ngx_module_type```所支持的8种模块，我们已经处理完成了其中7种（还有```MISC```模块未介绍），这里我们先对这些模块进行合并。

**1) 初始化modules**
{% highlight string %}
modules="$CORE_MODULES $EVENT_MODULES"
{% endhighlight %}


**2) 处理THREAD_POOL_MODULE**
{% highlight string %}
# thread pool module should be initialized after events
if [ $USE_THREADS = YES ]; then
    modules="$modules $THREAD_POOL_MODULE"
fi
{% endhighlight %}
在auto/options脚本中,```USE_THREADS```默认被初始化为```NO```，因此这里其实并不会被执行。```THREAD_POOL_MODULE```其实是与```EVENT_MODULE```类似，其在auto/sources脚本中被设置为:
<pre>
THREAD_POOL_MODULE=ngx_thread_pool_module
</pre>
但是最终并不会被加入到编译源代码中。

**3) 合并HTTP相关的模块**
{% highlight string %}
if [ $HTTP = YES ]; then
    modules="$modules $HTTP_MODULES $HTTP_FILTER_MODULES \
             $HTTP_AUX_FILTER_MODULES $HTTP_INIT_FILTER_MODULES"

    NGX_ADDON_DEPS="$NGX_ADDON_DEPS \$(HTTP_DEPS)"
fi
{% endhighlight %}


**4) 合并MAIL相关模块**
{% highlight string %}
if [ $MAIL != NO ]; then

    if [ $MAIL = YES ]; then
        modules="$modules $MAIL_MODULES"

    elif [ $MAIL = DYNAMIC ]; then
        ngx_module_name=$MAIL_MODULES
        ngx_module_incs=
        ngx_module_deps=$MAIL_DEPS
        ngx_module_srcs=$MAIL_SRCS
        ngx_module_libs=
        ngx_module_link=DYNAMIC

        . auto/module
    fi

    NGX_ADDON_DEPS="$NGX_ADDON_DEPS \$(MAIL_DEPS)"
fi
{% endhighlight %}

在auto/options脚本中，```MAIL```默认被初始化为```NO```，所以如下```MAIL```模块其实并不会执行，但是我们来分析一下```DYNAMIC```的情形：

```MAIL```模块代表着整邮件模块，其中包含几个子模块。在```MAIL```模块内部是以```ngx_module_link=YES```链接在一起的，但是作为```MAIL```模块整体其可以作为外部动态加载模块来工作的，因此这里会有如上elif分支。

**5) 合并STREAM相关模块**
{% highlight string %}
if [ $STREAM != NO ]; then

    if [ $STREAM = YES ]; then
        modules="$modules $STREAM_MODULES"

    elif [ $STREAM = DYNAMIC ]; then
        ngx_module_name=$STREAM_MODULES
        ngx_module_incs=
        ngx_module_deps=$STREAM_DEPS
        ngx_module_srcs=$STREAM_SRCS
        ngx_module_libs=
        ngx_module_link=DYNAMIC

        . auto/module
    fi

    NGX_ADDON_DEPS="$NGX_ADDON_DEPS \$(STREAM_DEPS)"
fi
{% endhighlight %}

在auto/options脚本中，```MSTREAMAIL```默认被初始化为```NO```，所以如下```STREAM```模块其实并不会执行。其他与上面```MAIL```模块合并类似，这里不再赘述。


## 13. MISC模块
到此为止，我们上面说的8大模块中就只剩下这里要介绍的```MISC```模块了：
{% highlight string %}
ngx_module_type=MISC
MISC_MODULES=

if [ $NGX_GOOGLE_PERFTOOLS = YES ]; then
    ngx_module_name=ngx_google_perftools_module
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/misc/ngx_google_perftools_module.c
    ngx_module_libs=
    ngx_module_link=$NGX_GOOGLE_PERFTOOLS

    . auto/module
fi

if [ $NGX_CPP_TEST = YES ]; then
    ngx_module_name=
    ngx_module_incs=
    ngx_module_deps=
    ngx_module_srcs=src/misc/ngx_cpp_test_module.cpp
    ngx_module_libs=-lstdc++
    ngx_module_link=$NGX_CPP_TEST

    . auto/module
fi

modules="$modules $MISC_MODULES"
{% endhighlight %}

**1) 初始化相关变量**


**2) 处理ngx_google_perftools_module**

在auto/options脚本中，```NGX_GOOGLE_PERFTOOLS```默认被初始化为```NO```。perftools模块主要是用于相应的性能分析。

**3) 处理test module**

本模块其实ngx_module_name为空，并没有为其命名。其主要用于对nginx相关模块的一个测试。

**4) 合并MISC模块**

上面 第12节 我们提到了模块的合并，这里也对```MISC```模块进行合并。


## 14. 生成ngx_modules.c文件

在前面我们已经合并了所有的```MODULES```，此时遍历这些模块，然后生成ngx_modules.c文件：
{% highlight string %}
cat << END                                    > $NGX_MODULES_C

#include <ngx_config.h>
#include <ngx_core.h>

$NGX_PRAGMA

END

for mod in $modules
do
    echo "extern ngx_module_t  $mod;"         >> $NGX_MODULES_C
done

echo                                          >> $NGX_MODULES_C
echo 'ngx_module_t *ngx_modules[] = {'        >> $NGX_MODULES_C

for mod in $modules
do
    echo "    &$mod,"                         >> $NGX_MODULES_C
done

cat << END                                    >> $NGX_MODULES_C
    NULL
};

END

echo 'char *ngx_module_names[] = {'           >> $NGX_MODULES_C

for mod in $modules
do
    echo "    \"$mod\","                      >> $NGX_MODULES_C
done

cat << END                                    >> $NGX_MODULES_C
    NULL
};

END
{% endhighlight %}


整个脚本比较简单，```NGX_PRAGMA```对于我们的当前的环境来说为空，生成的脚本基本样式如下：
<pre>
extern ngx_module_t  $mod;

ngx_module_t *ngx_modules[] = {
};

char *ngx_module_names[] = {
    NULL
};
</pre>

下面我们给出当前我们配置所生成的objs/ngx_modules.c源文件：
{% highlight string %}

#include <ngx_config.h>
#include <ngx_core.h>



extern ngx_module_t  ngx_core_module;
extern ngx_module_t  ngx_errlog_module;
extern ngx_module_t  ngx_conf_module;
extern ngx_module_t  ngx_openssl_module;
extern ngx_module_t  ngx_regex_module;
extern ngx_module_t  ngx_events_module;
extern ngx_module_t  ngx_event_core_module;
extern ngx_module_t  ngx_epoll_module;
extern ngx_module_t  ngx_http_module;
extern ngx_module_t  ngx_http_core_module;
extern ngx_module_t  ngx_http_log_module;
extern ngx_module_t  ngx_http_upstream_module;
extern ngx_module_t  ngx_http_static_module;
extern ngx_module_t  ngx_http_autoindex_module;
extern ngx_module_t  ngx_http_index_module;
extern ngx_module_t  ngx_http_auth_basic_module;
extern ngx_module_t  ngx_http_access_module;
extern ngx_module_t  ngx_http_limit_conn_module;
extern ngx_module_t  ngx_http_limit_req_module;
extern ngx_module_t  ngx_http_geo_module;
extern ngx_module_t  ngx_http_map_module;
extern ngx_module_t  ngx_http_split_clients_module;
extern ngx_module_t  ngx_http_referer_module;
extern ngx_module_t  ngx_http_rewrite_module;
extern ngx_module_t  ngx_http_ssl_module;
extern ngx_module_t  ngx_http_proxy_module;
extern ngx_module_t  ngx_http_fastcgi_module;
extern ngx_module_t  ngx_http_uwsgi_module;
extern ngx_module_t  ngx_http_scgi_module;
extern ngx_module_t  ngx_http_memcached_module;
extern ngx_module_t  ngx_http_empty_gif_module;
extern ngx_module_t  ngx_http_browser_module;
extern ngx_module_t  ngx_http_upstream_hash_module;
extern ngx_module_t  ngx_http_upstream_ip_hash_module;
extern ngx_module_t  ngx_http_upstream_least_conn_module;
extern ngx_module_t  ngx_http_upstream_keepalive_module;
extern ngx_module_t  ngx_http_upstream_zone_module;
extern ngx_module_t  ngx_http_write_filter_module;
extern ngx_module_t  ngx_http_header_filter_module;
extern ngx_module_t  ngx_http_chunked_filter_module;
extern ngx_module_t  ngx_http_range_header_filter_module;
extern ngx_module_t  ngx_http_gzip_filter_module;
extern ngx_module_t  ngx_http_postpone_filter_module;
extern ngx_module_t  ngx_http_ssi_filter_module;
extern ngx_module_t  ngx_http_charset_filter_module;
extern ngx_module_t  ngx_http_userid_filter_module;
extern ngx_module_t  ngx_http_headers_filter_module;
extern ngx_module_t  ngx_http_copy_filter_module;
extern ngx_module_t  ngx_http_range_body_filter_module;
extern ngx_module_t  ngx_http_not_modified_filter_module;

ngx_module_t *ngx_modules[] = {
    &ngx_core_module,
    &ngx_errlog_module,
    &ngx_conf_module,
    &ngx_openssl_module,
    &ngx_regex_module,
    &ngx_events_module,
    &ngx_event_core_module,
    &ngx_epoll_module,
    &ngx_http_module,
    &ngx_http_core_module,
    &ngx_http_log_module,
    &ngx_http_upstream_module,
    &ngx_http_static_module,
    &ngx_http_autoindex_module,
    &ngx_http_index_module,
    &ngx_http_auth_basic_module,
    &ngx_http_access_module,
    &ngx_http_limit_conn_module,
    &ngx_http_limit_req_module,
    &ngx_http_geo_module,
    &ngx_http_map_module,
    &ngx_http_split_clients_module,
    &ngx_http_referer_module,
    &ngx_http_rewrite_module,
    &ngx_http_ssl_module,
    &ngx_http_proxy_module,
    &ngx_http_fastcgi_module,
    &ngx_http_uwsgi_module,
    &ngx_http_scgi_module,
    &ngx_http_memcached_module,
    &ngx_http_empty_gif_module,
    &ngx_http_browser_module,
    &ngx_http_upstream_hash_module,
    &ngx_http_upstream_ip_hash_module,
    &ngx_http_upstream_least_conn_module,
    &ngx_http_upstream_keepalive_module,
    &ngx_http_upstream_zone_module,
    &ngx_http_write_filter_module,
    &ngx_http_header_filter_module,
    &ngx_http_chunked_filter_module,
    &ngx_http_range_header_filter_module,
    &ngx_http_gzip_filter_module,
    &ngx_http_postpone_filter_module,
    &ngx_http_ssi_filter_module,
    &ngx_http_charset_filter_module,
    &ngx_http_userid_filter_module,
    &ngx_http_headers_filter_module,
    &ngx_http_copy_filter_module,
    &ngx_http_range_body_filter_module,
    &ngx_http_not_modified_filter_module,
    NULL
};

char *ngx_module_names[] = {
    "ngx_core_module",
    "ngx_errlog_module",
    "ngx_conf_module",
    "ngx_openssl_module",
    "ngx_regex_module",
    "ngx_events_module",
    "ngx_event_core_module",
    "ngx_epoll_module",
    "ngx_http_module",
    "ngx_http_core_module",
    "ngx_http_log_module",
    "ngx_http_upstream_module",
    "ngx_http_static_module",
    "ngx_http_autoindex_module",
    "ngx_http_index_module",
    "ngx_http_auth_basic_module",
    "ngx_http_access_module",
    "ngx_http_limit_conn_module",
    "ngx_http_limit_req_module",
    "ngx_http_geo_module",
    "ngx_http_map_module",
    "ngx_http_split_clients_module",
    "ngx_http_referer_module",
    "ngx_http_rewrite_module",
    "ngx_http_ssl_module",
    "ngx_http_proxy_module",
    "ngx_http_fastcgi_module",
    "ngx_http_uwsgi_module",
    "ngx_http_scgi_module",
    "ngx_http_memcached_module",
    "ngx_http_empty_gif_module",
    "ngx_http_browser_module",
    "ngx_http_upstream_hash_module",
    "ngx_http_upstream_ip_hash_module",
    "ngx_http_upstream_least_conn_module",
    "ngx_http_upstream_keepalive_module",
    "ngx_http_upstream_zone_module",
    "ngx_http_write_filter_module",
    "ngx_http_header_filter_module",
    "ngx_http_chunked_filter_module",
    "ngx_http_range_header_filter_module",
    "ngx_http_gzip_filter_module",
    "ngx_http_postpone_filter_module",
    "ngx_http_ssi_filter_module",
    "ngx_http_charset_filter_module",
    "ngx_http_userid_filter_module",
    "ngx_http_headers_filter_module",
    "ngx_http_copy_filter_module",
    "ngx_http_range_body_filter_module",
    "ngx_http_not_modified_filter_module",
    NULL
};


{% endhighlight %}
ngx_modules.c文件就是用来定义ngx_modules数组的。

ngx_modules是非常关键的数组，它指明了每个模块在Nginx中的优先级，当一个请求同时符合多个模块的处理规则时，它将按照它们在ngx_modules数组中的顺序选择最靠前的模块优先处理。对于HTTP过滤模块而言则相反，因为HTTP框架在初始化时，会在ngx_modules数组中将过滤模块按先后顺序向过滤链表中添加，但每次都添加到链表表头，因此，对HTTP过滤模块而言，在ngx_modules数组中越靠后的模块反而会首先处理HTTP响应。

因此，ngx_modules中模块的先后顺序非常重要，不正确的顺序会导致nginx无法工作，上面是auto/modules脚本执行后的结果。可以体会一下上面的ngx_modules中同一类型下各个模块的顺序以及这种顺序带来的意义。


<br />
<br />
<br />

