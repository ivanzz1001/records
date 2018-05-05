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

**1） core模块中的listen指令**

可以在```listen```指令后为IP设置```address```以及```port```，或者指定一个unix域socket。服务器将会在这些地址上接收外部请求。可以同时指定```address```以及```port```，也可只指定```address```或```port```。而对于```address```也可以用主机名代替。例如：
<pre>
listen 127.0.0.1:8000;
listen 127.0.0.1;
listen 8000;
listen *:8000;
listen localhost:8000;
</pre>

对于IPV6地址，可以在方括号中指定。例如：
<pre>
listen [::]:8000;
listen [::1];
</pre>

对于unix域socket，则必须添加```unix:```前缀：
<pre>
listen unix:/var/run/nginx.sock;
</pre>

上面如果只指定了IP地址，没有指明端口的话，则默认会采用80端口。


**2） events模块debug_connection指令**
<pre>
events {
    debug_connection 127.0.0.1;
    debug_connection localhost;
    debug_connection 192.0.2.0/24;
    debug_connection ::1;
    debug_connection 2001:0db8::/32;
    debug_connection unix:;
    ...
}
</pre>

```debug_connection```指令用于为指定的客户端使能调试日志。而对于其他的连接，则会使用```error_log```指令的相应设置来打印日志。指定的debug connection可以是ipv4或者ipv6形式，也可以是hostname形式表示的地址，也可以是一个unix域socket形式。

**3） http模块memcached_pass指令**

用于设置memcached服务器地址。该地址可以通过一个domain name或IP地址，和一个port的形式来指定。例如：
<pre>
memcached_pass localhost:11211;
</pre>
而对于一个unix域socket，可以通过如下方式指定：
<pre>
memcached_pass unix:/tmp/memcached.socket;
</pre>

**4) http核心模块resolver指令**

用于配置将upstream servers解析成IP地址的一个服务地址。例如：
<pre>
resolver 127.0.0.1 [::1]:5353;
</pre>
假如没有指定端口的话，默认使用53端口

**5) stream模块listen指令**

用于指定服务器在给定```address```以及```port```处接受连接请求。可以单独指定port。```address```也可以是一个Hostname。例如：
<pre>
listen 127.0.0.1:12345;
listen *:12345;
listen 12345;     # same as *:12345
listen localhost:12345;
</pre>

也可以通过方括号的形式指定IPv6地址：
<pre>
listen [::1]:12345;
listen [::]:12345;
</pre>
也可以通过```unix:```前缀来指定unix域socket:
<pre>
listen unix:/var/run/nginx.sock;
</pre>

**6) 核心模块error_log指令**

```error_log```指令用于配置日志输出。当配置为输出到```syslog```中时，配置如下：
<pre>
error_log syslog:server=192.168.1.1 debug;

access_log syslog:server=unix:/var/log/nginx.sock,nohostname;
access_log syslog:server=[2001:db8::1]:12345,facility=local7,tag=nginx,severity=info combined;
</pre>

通过上面6个url配置示例，我们可以看到url的一个大体样式。包含```ipv4```、```ipv6```以及```unix域socket```这3种形式。这里注意在进行解析```ngx_url_t```时，一般都会去掉前缀，如```http```、```https```、以及```syslog:server=```这样的前缀。

下面我们再来分析```ngx_url_t```数据结构：

* ```url```: 该url的字符串表示形式

* ```host```: 在有些地方是通过主机名的形式来指定某一项服务，此时会设置此字段。一般只是作为服务的一个标识。例如http upstream模块下的server指令：
<pre>
server backend.example.com service=_http._tcp resolve;
</pre>
此种情况一般不需要经过```ngx_parse_url()```函数来解析。

* ```port_text```: 端口的字符串表示形式

* ```uri```: uri标识，一般为url中最后一个```/```后的内容。

* ```port```: 网络字节序表示的端口

* ```default_port```: 如果在配置中端口没有指定的话，会采用系统所指定的一个默认端口。

* ```family```: 所指定的协议类型(IPv4/IPv7/Unix domain)

* ```listen```: 是否需要建立监听socket。针对有一些配置可能需要建立，而另外一些如events模块中的debug_connection指令，则不需要建立专门的监听socket。

* ```uri_part```: 是否具有uri部分

* ```no_resolve```: 需不需要进行DNS解析

* ```one_addr```: 本字段暂时未使用

* ```no_port```: 表明当前url中是否配置了端口（如果没有配置，且需要端口的话，则会采用默认端口）

* ```wildcard```: 是否为一个通配地址

* ```socklen```: 所对应的socket长度

* ```sockaddr```: 存放socket地址的内存空间（此地址存放的一般是选作为默认的socket地址，请参看如下字段）

* ```addrs```: 存放所有socket的地址的数组空间（有时配置是一个通配地址，或者是一个域名地址，在完成DNS解析后可能会解析出多个地址）

* ```naddrs```: 上面addrs数组元素的个数

* ```err```: 用于存放对应的错误信息字符串

## 4. 相关函数声明
{% highlight string %}
in_addr_t ngx_inet_addr(u_char *text, size_t len);
#if (NGX_HAVE_INET6)
ngx_int_t ngx_inet6_addr(u_char *p, size_t len, u_char *addr);
size_t ngx_inet6_ntop(u_char *p, u_char *text, size_t len);
#endif
size_t ngx_sock_ntop(struct sockaddr *sa, socklen_t socklen, u_char *text,
    size_t len, ngx_uint_t port);
size_t ngx_inet_ntop(int family, void *addr, u_char *text, size_t len);
ngx_int_t ngx_ptocidr(ngx_str_t *text, ngx_cidr_t *cidr);
ngx_int_t ngx_parse_addr(ngx_pool_t *pool, ngx_addr_t *addr, u_char *text,
    size_t len);
ngx_int_t ngx_parse_url(ngx_pool_t *pool, ngx_url_t *u);
ngx_int_t ngx_inet_resolve_host(ngx_pool_t *pool, ngx_url_t *u);
ngx_int_t ngx_cmp_sockaddr(struct sockaddr *sa1, socklen_t slen1,
    struct sockaddr *sa2, socklen_t slen2, ngx_uint_t cmp_port);
{% endhighlight %}


* ```ngx_inet_addr()```: 用于将字符串表示的IPv4地址转换成in_addr_t类型表示

* ```ngx_inet6_addr()```: 用于将字符串表示的IPv6地址转换成ipv6相应结构，存放在addr所指向的内存中。当前我们并不支持```NGX_HAVE_INET6```宏定义。

* ```ngx_inet6_ntop()```: 用于将ipv6相应内存结构转换成字符串表示形式

* ```ngx_sock_ntop()```: 用于将ipv4、ipv6、unix域socket地址转换成字符串表示形式，存放在text中

* ```ngx_inet_ntop()```: 用于将ipv4、ipv6表示的相应结构转换成字符串表示形式，存放在text中

* ```ngx_ptocidr()```: 将字符串表示形式的ip地址(ipv4/ipv6)转换成cidr表示形式

* ```ngx_parse_addr()```: 将```text```表示形式的IP地址转换成ngx_addr_t结构

* ```ngx_parse_url()```: 解析```ngx_url_t```数据结构

* ```ngx_inet_resolve()```: 将主机名转换成socket地址结构

* ```ngx_cmp_sockaddr()```: 比较两个IP地址

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

