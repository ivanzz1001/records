---
layout: post
title: core/ngx_inet.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx inet相关数据结构定义及操作函数。


<!-- more -->


## 1. socket地址长度相关定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_INET_H_INCLUDED_
#define _NGX_INET_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * TODO: autoconfigure NGX_SOCKADDRLEN and NGX_SOCKADDR_STRLEN as
 *       sizeof(struct sockaddr_storage)
 *       sizeof(struct sockaddr_un)
 *       sizeof(struct sockaddr_in6)
 *       sizeof(struct sockaddr_in)
 */

#define NGX_INET_ADDRSTRLEN   (sizeof("255.255.255.255") - 1)
#define NGX_INET6_ADDRSTRLEN                                                 \
    (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") - 1)
#define NGX_UNIX_ADDRSTRLEN                                                  \
    (sizeof(struct sockaddr_un) - offsetof(struct sockaddr_un, sun_path))

#if (NGX_HAVE_UNIX_DOMAIN)
#define NGX_SOCKADDR_STRLEN   (sizeof("unix:") - 1 + NGX_UNIX_ADDRSTRLEN)
#else
#define NGX_SOCKADDR_STRLEN   (NGX_INET6_ADDRSTRLEN + sizeof("[]:65535") - 1)
#endif

#if (NGX_HAVE_UNIX_DOMAIN)
#define NGX_SOCKADDRLEN       sizeof(struct sockaddr_un)
#else
#define NGX_SOCKADDRLEN       512
#endif
{% endhighlight %}
这里首先定义字符串表示形式下ipv4、ipv6以及unix domain socket的地址长度。在objs/ngx_auto_config.h头文件中我们有如下定义：
<pre>
#ifndef NGX_HAVE_UNIX_DOMAIN
#define NGX_HAVE_UNIX_DOMAIN  1
#endif
</pre>

## 2. 相关数据结构定义
{% highlight string %}
typedef struct {
    in_addr_t                 addr;
    in_addr_t                 mask;
} ngx_in_cidr_t;


#if (NGX_HAVE_INET6)

typedef struct {
    struct in6_addr           addr;
    struct in6_addr           mask;
} ngx_in6_cidr_t;

#endif


typedef struct {
    ngx_uint_t                family;
    union {
        ngx_in_cidr_t         in;
#if (NGX_HAVE_INET6)
        ngx_in6_cidr_t        in6;
#endif
    } u;
} ngx_cidr_t;


typedef struct {
    struct sockaddr          *sockaddr;
    socklen_t                 socklen;
    ngx_str_t                 name;
} ngx_addr_t;
{% endhighlight %}

* ```ngx_in_cidr_t```: 是ipv4无类域间ip地址

* ```ngx_in6_cidr_t```: 是ipv6无类域间ip地址。当前我们并不支持```NGX_HAVE_INET6```

* ```ngx_cidr_t```: 无类域间ip地址

* ```ngx_addr_t```: socket ip地址封装 


## 3. ngx_url_t数据结构
{% highlight string %}
typedef struct {
    ngx_str_t                 url;
    ngx_str_t                 host;
    ngx_str_t                 port_text;
    ngx_str_t                 uri;

    in_port_t                 port;
    in_port_t                 default_port;
    int                       family;

    unsigned                  listen:1;
    unsigned                  uri_part:1;
    unsigned                  no_resolve:1;
    unsigned                  one_addr:1;  /* compatibility */

    unsigned                  no_port:1;
    unsigned                  wildcard:1;

    socklen_t                 socklen;
    u_char                    sockaddr[NGX_SOCKADDRLEN];

    ngx_addr_t               *addrs;
    ngx_uint_t                naddrs;

    char                     *err;
} ngx_url_t;
{% endhighlight %}

在介绍```ngx_url_t```数据结构之前，我们先来了解一下nginx中所配置url大体是什么样式。搜索```ngx_parse_url()```函数，并找到所使用```ngx_url_t```的一些位置。




<br />
<br />

**[参考]**

1. [IPv6地址表示方法及其简化方法](http://www.sohu.com/a/132928812_470081)

2. [无类域间路由](https://baike.baidu.com/item/%E6%97%A0%E7%B1%BB%E5%9F%9F%E9%97%B4%E8%B7%AF%E7%94%B1/240168?fr=aladdin&fromid=3695195&fromtitle=CIDR)

3. [Nginx中发送udp请求](http://blog.aka-cool.net/blog/2014/02/24/udp-in-nginx/)

4. [nginx源码初读](https://blog.csdn.net/wuchunlai_2012/article/category/6098793)

5. [Module ngx_http_core_module - listen](http://nginx.org/en/docs/http/ngx_http_core_module.html#listen)

6. [Module ngx_http_core_module - debug_connection](http://nginx.org/en/docs/ngx_core_module.html#debug_connection)

7. [memcached_pass](http://nginx.org/en/docs/http/ngx_http_memcached_module.html#memcached_pass)

8. [resolver](http://nginx.org/en/docs/http/ngx_http_core_module.html#resolver)

9. [stream module - listen](http://nginx.org/en/docs/stream/ngx_stream_core_module.html#listen)

10. [error_log - syslog](http://nginx.org/en/docs/ngx_core_module.html#error_log)

<br />
<br />
<br />

