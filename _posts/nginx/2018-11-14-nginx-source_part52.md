---
layout: post
title: core/ngx_proxy_protocol.c(h)文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本章我们介绍一下ngx_proxy_protocol.c的相关实现。这里我们简要介绍一下```PROXY protocol```。

PROXY protocol使得```Nginx```及```Nginx Plus```能够接收来自代理服务器(proxy server)和负载均衡服务器(LB)传递过来的客户端连接信息。比如```HAProxy```以及```ELB```(Amazon Elastic Load Balancer)会将就会通过PROXY protocol来向Nginx提供客户端的连接信息。

当使用```PROXY protocol```之后，NGINX就可以从HTTP、SSL、HTTP/2、SPDY、WebSocket、TCP请求中获得来自客户端的原始IP(originating IP address)。后台服务程序获得了来自客户端的原始IP地址之后，就可以根据原始IP设置网站的语言，或者是将某个IP加入到黑名单，或者简单的打印日志等。

关于NGINX中如何配置以支持```PROXY protocol```功能，本章我们不做介绍，在后续的章节中我们会专门讨论这一方面。


<!-- more -->

## 1. ngx_proxy_protocol.h头文件
{% highlight string %}

/*
 * Copyright (C) Roman Arutyunyan
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PROXY_PROTOCOL_H_INCLUDED_
#define _NGX_PROXY_PROTOCOL_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_PROXY_PROTOCOL_MAX_HEADER  107


u_char *ngx_proxy_protocol_read(ngx_connection_t *c, u_char *buf,
    u_char *last);
u_char *ngx_proxy_protocol_write(ngx_connection_t *c, u_char *buf,
    u_char *last);


#endif /* _NGX_PROXY_PROTOCOL_H_INCLUDED_ */
{% endhighlight %}

首先这里定义了一个宏： NGX_PROXY_PROTOCOL_MAX_HEADER，用于指示协议头的最长长度。协议头有如下几种格式：

* PROXY TCP4 <ipaddr> <port>

* PROXY TCP6 <ipaddr> <port>

* PROXY UNKNOWN


接着声明了两个函数，第一个用于从缓存buf中读取proxy protocol；第二个用于将PROXY protocol写到buf中。


## 2. ngx_proxy_protocol.c源文件

### 2.1 函数ngx_proxy_protocol_read()
{% highlight string %}

/*
 * Copyright (C) Roman Arutyunyan
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


u_char *
ngx_proxy_protocol_read(ngx_connection_t *c, u_char *buf, u_char *last)
{
    size_t  len;
    u_char  ch, *p, *addr;

    p = buf;
    len = last - buf;

    if (len < 8 || ngx_strncmp(p, "PROXY ", 6) != 0) {
        goto invalid;
    }

    p += 6;
    len -= 6;

    if (len >= 7 && ngx_strncmp(p, "UNKNOWN", 7) == 0) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, c->log, 0,
                       "PROXY protocol unknown protocol");
        p += 7;
        goto skip;
    }

    if (len < 5 || ngx_strncmp(p, "TCP", 3) != 0
        || (p[3] != '4' && p[3] != '6') || p[4] != ' ')
    {
        goto invalid;
    }

    p += 5;
    addr = p;

    for ( ;; ) {
        if (p == last) {
            goto invalid;
        }

        ch = *p++;

        if (ch == ' ') {
            break;
        }

        if (ch != ':' && ch != '.'
            && (ch < 'a' || ch > 'f')
            && (ch < 'A' || ch > 'F')
            && (ch < '0' || ch > '9'))
        {
            goto invalid;
        }
    }

    len = p - addr - 1;
    c->proxy_protocol_addr.data = ngx_pnalloc(c->pool, len);

    if (c->proxy_protocol_addr.data == NULL) {
        return NULL;
    }

    ngx_memcpy(c->proxy_protocol_addr.data, addr, len);
    c->proxy_protocol_addr.len = len;

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, c->log, 0,
                   "PROXY protocol address: \"%V\"", &c->proxy_protocol_addr);

skip:

    for ( /* void */ ; p < last - 1; p++) {
        if (p[0] == CR && p[1] == LF) {
            return p + 2;
        }
    }

invalid:

    ngx_log_error(NGX_LOG_ERR, c->log, 0,
                  "broken header: \"%*s\"", (size_t) (last - buf), buf);

    return NULL;
}


{% endhighlight %}

本函数用于从缓存```buf```中读取出proxy protocol协议，并将读取出的源IP地址(originating IP address)信息设置到 **'connection->proxy_protocol_addr'**中。


### 2.2 函数ngx_proxy_protocol_write()
{% highlight string %}
u_char *
ngx_proxy_protocol_write(ngx_connection_t *c, u_char *buf, u_char *last)
{
    ngx_uint_t  port, lport;

    if (last - buf < NGX_PROXY_PROTOCOL_MAX_HEADER) {
        return NULL;
    }

    if (ngx_connection_local_sockaddr(c, NULL, 0) != NGX_OK) {
        return NULL;
    }

    switch (c->sockaddr->sa_family) {

    case AF_INET:
        buf = ngx_cpymem(buf, "PROXY TCP4 ", sizeof("PROXY TCP4 ") - 1);

        port = ntohs(((struct sockaddr_in *) c->sockaddr)->sin_port);
        lport = ntohs(((struct sockaddr_in *) c->local_sockaddr)->sin_port);

        break;

#if (NGX_HAVE_INET6)
    case AF_INET6:
        buf = ngx_cpymem(buf, "PROXY TCP6 ", sizeof("PROXY TCP6 ") - 1);

        port = ntohs(((struct sockaddr_in6 *) c->sockaddr)->sin6_port);
        lport = ntohs(((struct sockaddr_in6 *) c->local_sockaddr)->sin6_port);

        break;
#endif

    default:
        return ngx_cpymem(buf, "PROXY UNKNOWN" CRLF,
                          sizeof("PROXY UNKNOWN" CRLF) - 1);
    }

    buf += ngx_sock_ntop(c->sockaddr, c->socklen, buf, last - buf, 0);

    *buf++ = ' ';

    buf += ngx_sock_ntop(c->local_sockaddr, c->local_socklen, buf, last - buf,
                         0);

    return ngx_slprintf(buf, last, " %ui %ui" CRLF, port, lport);
}
{% endhighlight %}
本函数使用```connection->sockaddr```保存的远程IP地址来构造PROXY protocol，保存在buf中。



<br />
<br />

**[参看]**

1. [proxy protocol介绍及nginx配置](https://www.jianshu.com/p/cc8d592582c9)

2. [Accepting the PROXY Protocol](https://docs.nginx.com/nginx/admin-guide/load-balancer/using-proxy-protocol/)

3. [The PROXY protocol Versions 1 & 2](https://www.haproxy.org/download/1.8/doc/proxy-protocol.txt)

4. [代理协议Proxy protocol](https://www.sohu.com/a/232767795_575744)


<br />
<br />
<br />

