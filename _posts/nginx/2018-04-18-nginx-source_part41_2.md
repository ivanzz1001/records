---
layout: post
title: core/ngx_inet.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

本节我们主要分析一下nginx网络地址相关操作。


<!-- more -->


## 1. 相关静态函数声明
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


//解析Unix域url地址
static ngx_int_t ngx_parse_unix_domain_url(ngx_pool_t *pool, ngx_url_t *u);


//解析ipv4类型url地址
static ngx_int_t ngx_parse_inet_url(ngx_pool_t *pool, ngx_url_t *u);


//解析ipv6类型url地址
static ngx_int_t ngx_parse_inet6_url(ngx_pool_t *pool, ngx_url_t *u);

{% endhighlight %}


## 2. 函数ngx_inet_addr()
{% highlight string %}
in_addr_t
ngx_inet_addr(u_char *text, size_t len)
{
    u_char      *p, c;
    in_addr_t    addr;
    ngx_uint_t   octet, n;

    addr = 0;
    octet = 0;
    n = 0;

    for (p = text; p < text + len; p++) {
        c = *p;

        if (c >= '0' && c <= '9') {
            octet = octet * 10 + (c - '0');

            if (octet > 255) {
                return INADDR_NONE;
            }

            continue;
        }

        if (c == '.') {
            addr = (addr << 8) + octet;
            octet = 0;
            n++;
            continue;
        }

        return INADDR_NONE;
    }

    if (n == 3) {
        addr = (addr << 8) + octet;
        return htonl(addr);
    }

    return INADDR_NONE;
}

{% endhighlight %}

此函数用于将字符串表示形式的IPv4地址转换成```in_addr_t```表示形式。转换出错，返回```INADDR_NONE```。

## 2. 函数ngx_inet6_addr()
{% highlight string %}
#if (NGX_HAVE_INET6)

ngx_int_t
ngx_inet6_addr(u_char *p, size_t len, u_char *addr)
{
    u_char      c, *zero, *digit, *s, *d;
    size_t      len4;
    ngx_uint_t  n, nibbles, word;

    if (len == 0) {
        return NGX_ERROR;
    }

    zero = NULL;
    digit = NULL;
    len4 = 0;
    nibbles = 0;
    word = 0;
    n = 8;

    if (p[0] == ':') {
        p++;
        len--;
    }

    for (/* void */; len; len--) {
        c = *p++;

        if (c == ':') {
            if (nibbles) {
                digit = p;
                len4 = len;
                *addr++ = (u_char) (word >> 8);
                *addr++ = (u_char) (word & 0xff);

                if (--n) {
                    nibbles = 0;
                    word = 0;
                    continue;
                }

            } else {
                if (zero == NULL) {
                    digit = p;
                    len4 = len;
                    zero = addr;
                    continue;
                }
            }

            return NGX_ERROR;
        }

        if (c == '.' && nibbles) {
            if (n < 2 || digit == NULL) {
                return NGX_ERROR;
            }

            word = ngx_inet_addr(digit, len4 - 1);
            if (word == INADDR_NONE) {
                return NGX_ERROR;
            }

            word = ntohl(word);
            *addr++ = (u_char) ((word >> 24) & 0xff);
            *addr++ = (u_char) ((word >> 16) & 0xff);
            n--;
            break;
        }

        if (++nibbles > 4) {
            return NGX_ERROR;
        }

        if (c >= '0' && c <= '9') {
            word = word * 16 + (c - '0');
            continue;
        }

        c |= 0x20;

        if (c >= 'a' && c <= 'f') {
            word = word * 16 + (c - 'a') + 10;
            continue;
        }

        return NGX_ERROR;
    }

    if (nibbles == 0 && zero == NULL) {
        return NGX_ERROR;
    }

    *addr++ = (u_char) (word >> 8);
    *addr++ = (u_char) (word & 0xff);

    if (--n) {
        if (zero) {
            n *= 2;
            s = addr - 1;
            d = s + n;
            while (s >= zero) {
                *d-- = *s--;
            }
            ngx_memzero(zero, n);
            return NGX_OK;
        }

    } else {
        if (zero == NULL) {
            return NGX_OK;
        }
    }

    return NGX_ERROR;
}

#endif
{% endhighlight %}


这里我们暂不支持```NGX_HAVE_INET6```，函数ngx_inet6_addr()用于将字符串表示形式的IPv6地址转换成128bit位的地址。IPv6地址的表示形式一般为：
<pre>
2001:0410:0000:1234:FB00:1400:5000:45FF    //首选格式
2001:0410 :: 1234:FB00:1400:5000:45FF      //压缩格式(注意压缩格式中，只能有一个地方压缩）
0:0:0:0:0:0:138.1.1.1                      //内嵌IPv4
</pre>
下面我们来详细介绍一下ngx_inet6_addr():
{% highlight string %}
ngx_int_t
ngx_inet6_addr(u_char *p, size_t len, u_char *addr)
{
    zero = NULL;      //用于记录IPv6中压缩格式出现的位置
    digit = NULL;     //用于记录一个段中数据出现的起始位置
    len4 = 0;         //用于记录从当前位置到结束位置的长度（主要是处理内嵌IPv4这一情况)
    nibbles = 0;      //主要是用于记录一个段的长度（例如上述首选格式，分成8个段，每段长度为4)
    word = 0;         //主要用于处理每一个段的转换
    n = 8;            //IPv6最长有8个段

   //1) 跳过最前面的压缩
   if (p[0] == ':') {
        p++;
        len--;
    }

   for (/* void */; len; len--) {
       c = *p++;

       //2) 检测完了一段
       if(c == ':')
       {
           if(nibbles)
           {
               //2.1) 将该段进行转换

               //2.2) 判断IPv6最长只能为8段
           }
           else{
               //2.3 记录压缩段的开始
           }
       }

       if (c == '.' && nibbles) {
       {
          //3) 处理IPv6中内嵌IPv4的情况
       }


      //4) 转换一个段中的16进制数据
     
   }

   if (nibbles == 0 && zero == NULL) {
        return NGX_ERROR;
    }

   //5) 转换最后一个段
   *addr++ = (u_char) (word >> 8);
   *addr++ = (u_char) (word & 0xff);

   //6) 处理有压缩情况下的填充（最终要填充到128bit)
}
{% endhighlight %}


## 3. 函数ngx_sock_ntop()
{% highlight string %}
size_t
ngx_sock_ntop(struct sockaddr *sa, socklen_t socklen, u_char *text, size_t len,
    ngx_uint_t port)
{
    u_char               *p;
    struct sockaddr_in   *sin;
#if (NGX_HAVE_INET6)
    size_t                n;
    struct sockaddr_in6  *sin6;
#endif
#if (NGX_HAVE_UNIX_DOMAIN)
    struct sockaddr_un   *saun;
#endif

    switch (sa->sa_family) {

    case AF_INET:

        sin = (struct sockaddr_in *) sa;
        p = (u_char *) &sin->sin_addr;

        if (port) {
            p = ngx_snprintf(text, len, "%ud.%ud.%ud.%ud:%d",
                             p[0], p[1], p[2], p[3], ntohs(sin->sin_port));
        } else {
            p = ngx_snprintf(text, len, "%ud.%ud.%ud.%ud",
                             p[0], p[1], p[2], p[3]);
        }

        return (p - text);

#if (NGX_HAVE_INET6)

    case AF_INET6:

        sin6 = (struct sockaddr_in6 *) sa;

        n = 0;

        if (port) {
            text[n++] = '[';
        }

        n = ngx_inet6_ntop(sin6->sin6_addr.s6_addr, &text[n], len);

        if (port) {
            n = ngx_sprintf(&text[1 + n], "]:%d",
                            ntohs(sin6->sin6_port)) - text;
        }

        return n;
#endif

#if (NGX_HAVE_UNIX_DOMAIN)

    case AF_UNIX:
        saun = (struct sockaddr_un *) sa;

        /* on Linux sockaddr might not include sun_path at all */

        if (socklen <= (socklen_t) offsetof(struct sockaddr_un, sun_path)) {
            p = ngx_snprintf(text, len, "unix:%Z");

        } else {
            p = ngx_snprintf(text, len, "unix:%s%Z", saun->sun_path);
        }

        /* we do not include trailing zero in address length */

        return (p - text - 1);

#endif

    default:
        return 0;
    }
}

{% endhighlight %}
本函数用于将```sockaddr```表示形式的地址（IPv4/IPv6/Unix Domain)，转换成字符串表示形式。（注意：这里如果地址是IPv4/IPv6,也会对port参数进行转换）


## 4. 函数ngx_inet_ntop()
{% highlight string %}
size_t
ngx_inet_ntop(int family, void *addr, u_char *text, size_t len)
{
    u_char  *p;

    switch (family) {

    case AF_INET:

        p = addr;

        return ngx_snprintf(text, len, "%ud.%ud.%ud.%ud",
                            p[0], p[1], p[2], p[3])
               - text;

#if (NGX_HAVE_INET6)

    case AF_INET6:
        return ngx_inet6_ntop(addr, text, len);

#endif

    default:
        return 0;
    }
}
{% endhighlight %}
这里对IPv4/IPv6地址转换成字符串表示形式。

## 5. 函数ngx_inet_ntop()
{% highlight string %}
#if (NGX_HAVE_INET6)

size_t
ngx_inet6_ntop(u_char *p, u_char *text, size_t len)
{
    u_char      *dst;
    size_t       max, n;
    ngx_uint_t   i, zero, last;

    if (len < NGX_INET6_ADDRSTRLEN) {
        return 0;
    }

    zero = (ngx_uint_t) -1;
    last = (ngx_uint_t) -1;
    max = 1;
    n = 0;

    for (i = 0; i < 16; i += 2) {

        if (p[i] || p[i + 1]) {

            if (max < n) {
                zero = last;
                max = n;
            }

            n = 0;
            continue;
        }

        if (n++ == 0) {
            last = i;
        }
    }

    if (max < n) {
        zero = last;
        max = n;
    }

    dst = text;
    n = 16;

    if (zero == 0) {

        if ((max == 5 && p[10] == 0xff && p[11] == 0xff)
            || (max == 6)
            || (max == 7 && p[14] != 0 && p[15] != 1))
        {
            n = 12;
        }

        *dst++ = ':';
    }

    for (i = 0; i < n; i += 2) {

        if (i == zero) {
            *dst++ = ':';
            i += (max - 1) * 2;
            continue;
        }

        dst = ngx_sprintf(dst, "%xd", p[i] * 256 + p[i + 1]);

        if (i < 14) {
            *dst++ = ':';
        }
    }

    if (n == 12) {
        dst = ngx_sprintf(dst, "%ud.%ud.%ud.%ud", p[12], p[13], p[14], p[15]);
    }

    return dst - text;
}

#endif
{% endhighlight %}

这里对128bit的IPv6地址转换成IPv6字符串表示形式。下面详细介绍一下该函数：
{% highlight string %}
size_t
ngx_inet6_ntop(u_char *p, u_char *text, size_t len)
{
   //1) IPv6地址总共16个字节，每2个字节一组，找出最长连续为0的组，
   // 用zero变量记录该组的起始位置，用max记录连续为0的组数目

   //2) 如果最长连续为0的组在头部（即zero==0)，则进一步判断是否是内嵌IPv4

   //3) 将每一组转换成IPv6字符串表示形式
   for (i = 0; i < n; i += 2) {

        if (i == zero) {
            *dst++ = ':';
            i += (max - 1) * 2;
            continue;
        }

        dst = ngx_sprintf(dst, "%xd", p[i] * 256 + p[i + 1]);

        if (i < 14) {
            *dst++ = ':';
        }
    }


    //4) 处理内嵌IPv4情况
}
{% endhighlight %}


## 6. 函数ngx_ptocidr()
{% highlight string %}
ngx_int_t
ngx_ptocidr(ngx_str_t *text, ngx_cidr_t *cidr)
{
    u_char      *addr, *mask, *last;
    size_t       len;
    ngx_int_t    shift;
#if (NGX_HAVE_INET6)
    ngx_int_t    rc;
    ngx_uint_t   s, i;
#endif

    addr = text->data;
    last = addr + text->len;

    mask = ngx_strlchr(addr, last, '/');
    len = (mask ? mask : last) - addr;

    cidr->u.in.addr = ngx_inet_addr(addr, len);

    if (cidr->u.in.addr != INADDR_NONE) {
        cidr->family = AF_INET;

        if (mask == NULL) {
            cidr->u.in.mask = 0xffffffff;
            return NGX_OK;
        }

#if (NGX_HAVE_INET6)
    } else if (ngx_inet6_addr(addr, len, cidr->u.in6.addr.s6_addr) == NGX_OK) {
        cidr->family = AF_INET6;

        if (mask == NULL) {
            ngx_memset(cidr->u.in6.mask.s6_addr, 0xff, 16);
            return NGX_OK;
        }

#endif
    } else {
        return NGX_ERROR;
    }

    mask++;

    shift = ngx_atoi(mask, last - mask);
    if (shift == NGX_ERROR) {
        return NGX_ERROR;
    }

    switch (cidr->family) {

#if (NGX_HAVE_INET6)
    case AF_INET6:
        if (shift > 128) {
            return NGX_ERROR;
        }

        addr = cidr->u.in6.addr.s6_addr;
        mask = cidr->u.in6.mask.s6_addr;
        rc = NGX_OK;

        for (i = 0; i < 16; i++) {

            s = (shift > 8) ? 8 : shift;
            shift -= s;

            mask[i] = (u_char) (0xffu << (8 - s));

            if (addr[i] != (addr[i] & mask[i])) {
                rc = NGX_DONE;
                addr[i] &= mask[i];
            }
        }

        return rc;
#endif

    default: /* AF_INET */
        if (shift > 32) {
            return NGX_ERROR;
        }

        if (shift) {
            cidr->u.in.mask = htonl((uint32_t) (0xffffffffu << (32 - shift)));

        } else {
            /* x86 compilers use a shl instruction that shifts by modulo 32 */
            cidr->u.in.mask = 0;
        }

        if (cidr->u.in.addr == (cidr->u.in.addr & cidr->u.in.mask)) {
            return NGX_OK;
        }

        cidr->u.in.addr &= cidr->u.in.mask;

        return NGX_DONE;
    }
}

{% endhighlight %}

本函数用于将字符串转换成无类域间路由。该函数实现较为简单，我们简单介绍一下：
{% highlight string %}
ngx_int_t
ngx_ptocidr(ngx_str_t *text, ngx_cidr_t *cidr)
{
    //1) 找出无类域间路由中"/"的位置，例如222.80.18.18/25， 该32bit地址中前25位为网络
    // 部分，后7位为主机部分。

    //2) 解析"/"前面部分的IP地址，是属于IPv4还是IPv6类型

    //3) 分别计算出IPv4/IPv6无类域间路由的“网络部分”（addr）及“掩码部分”(mask)
}
{% endhighlight %}

## 7. 函数ngx_parse_addr()
{% highlight string %}
ngx_int_t
ngx_parse_addr(ngx_pool_t *pool, ngx_addr_t *addr, u_char *text, size_t len)
{
    in_addr_t             inaddr;
    ngx_uint_t            family;
    struct sockaddr_in   *sin;
#if (NGX_HAVE_INET6)
    struct in6_addr       inaddr6;
    struct sockaddr_in6  *sin6;

    /*
     * prevent MSVC8 warning:
     *    potentially uninitialized local variable 'inaddr6' used
     */
    ngx_memzero(&inaddr6, sizeof(struct in6_addr));
#endif

    inaddr = ngx_inet_addr(text, len);

    if (inaddr != INADDR_NONE) {
        family = AF_INET;
        len = sizeof(struct sockaddr_in);

#if (NGX_HAVE_INET6)
    } else if (ngx_inet6_addr(text, len, inaddr6.s6_addr) == NGX_OK) {
        family = AF_INET6;
        len = sizeof(struct sockaddr_in6);

#endif
    } else {
        return NGX_DECLINED;
    }

    addr->sockaddr = ngx_pcalloc(pool, len);
    if (addr->sockaddr == NULL) {
        return NGX_ERROR;
    }

    addr->sockaddr->sa_family = (u_char) family;
    addr->socklen = len;

    switch (family) {

#if (NGX_HAVE_INET6)
    case AF_INET6:
        sin6 = (struct sockaddr_in6 *) addr->sockaddr;
        ngx_memcpy(sin6->sin6_addr.s6_addr, inaddr6.s6_addr, 16);
        break;
#endif

    default: /* AF_INET */
        sin = (struct sockaddr_in *) addr->sockaddr;
        sin->sin_addr.s_addr = inaddr;
        break;
    }

    return NGX_OK;
}

{% endhighlight %}
本函数用于将字符串表示的IPv4/IPv6地址转换成ngx_addr_t形式。

## 8. 函数ngx_parse_url()
{% highlight string %}
ngx_int_t
ngx_parse_url(ngx_pool_t *pool, ngx_url_t *u)
{
    u_char  *p;
    size_t   len;

    p = u->url.data;
    len = u->url.len;

    if (len >= 5 && ngx_strncasecmp(p, (u_char *) "unix:", 5) == 0) {
        return ngx_parse_unix_domain_url(pool, u);
    }

    if (len && p[0] == '[') {
        return ngx_parse_inet6_url(pool, u);
    }

    return ngx_parse_inet_url(pool, u);
}
{% endhighlight %}
本函数用于解析url，分三种类型：

* unix域URL: 调用ngx_parse_unix_domain_url()，例如```unix:/var/run/nginx.sock```

* IPv6 URL: 调用ngx_parse_inet6_url()，例如```[::1]:5353```

* IPv4 URL: 调用ngx_parse_inet_url()，例如```127.0.0.1:12345```

## 9. 函数ngx_parse_unix_domain_url()
{% highlight string %}
static ngx_int_t
ngx_parse_unix_domain_url(ngx_pool_t *pool, ngx_url_t *u)
{
#if (NGX_HAVE_UNIX_DOMAIN)
    u_char              *path, *uri, *last;
    size_t               len;
    struct sockaddr_un  *saun;

    len = u->url.len;
    path = u->url.data;

    path += 5;
    len -= 5;

    if (u->uri_part) {

        last = path + len;
        uri = ngx_strlchr(path, last, ':');

        if (uri) {
            len = uri - path;
            uri++;
            u->uri.len = last - uri;
            u->uri.data = uri;
        }
    }

    if (len == 0) {
        u->err = "no path in the unix domain socket";
        return NGX_ERROR;
    }

    u->host.len = len++;
    u->host.data = path;

    if (len > sizeof(saun->sun_path)) {
        u->err = "too long path in the unix domain socket";
        return NGX_ERROR;
    }

    u->socklen = sizeof(struct sockaddr_un);
    saun = (struct sockaddr_un *) &u->sockaddr;
    saun->sun_family = AF_UNIX;
    (void) ngx_cpystrn((u_char *) saun->sun_path, path, len);

    u->addrs = ngx_pcalloc(pool, sizeof(ngx_addr_t));
    if (u->addrs == NULL) {
        return NGX_ERROR;
    }

    saun = ngx_pcalloc(pool, sizeof(struct sockaddr_un));
    if (saun == NULL) {
        return NGX_ERROR;
    }

    u->family = AF_UNIX;
    u->naddrs = 1;

    saun->sun_family = AF_UNIX;
    (void) ngx_cpystrn((u_char *) saun->sun_path, path, len);

    u->addrs[0].sockaddr = (struct sockaddr *) saun;
    u->addrs[0].socklen = sizeof(struct sockaddr_un);
    u->addrs[0].name.len = len + 4;
    u->addrs[0].name.data = u->url.data;

    return NGX_OK;

#else

    u->err = "the unix domain sockets are not supported on this platform";

    return NGX_ERROR;

#endif
}
{% endhighlight %}

本函数用于解析```ngx_url_t.url```，主要会解析成以下几个部分：

* ```ngx_url_t.uri```: 如果url中uri部分存在的话，解析后存放于此

* ```ngx_url_t.host```: 解析后的主机名存放于此

* ```ngx_url_t.sockaddr```: 解析后的字符串表示形式

* ```ngx_url_t.addrs```: 解析后的地址

## 9. 函数ngx_parse_inet_url()
{% highlight string %}
static ngx_int_t
ngx_parse_inet_url(ngx_pool_t *pool, ngx_url_t *u)
{
    u_char               *p, *host, *port, *last, *uri, *args;
    size_t                len;
    ngx_int_t             n;
    struct sockaddr_in   *sin;
#if (NGX_HAVE_INET6)
    struct sockaddr_in6  *sin6;
#endif

    u->socklen = sizeof(struct sockaddr_in);
    sin = (struct sockaddr_in *) &u->sockaddr;
    sin->sin_family = AF_INET;

    u->family = AF_INET;

    host = u->url.data;

    last = host + u->url.len;

    port = ngx_strlchr(host, last, ':');

    uri = ngx_strlchr(host, last, '/');

    args = ngx_strlchr(host, last, '?');

    if (args) {
        if (uri == NULL || args < uri) {
            uri = args;
        }
    }

    if (uri) {
        if (u->listen || !u->uri_part) {
            u->err = "invalid host";
            return NGX_ERROR;
        }

        u->uri.len = last - uri;
        u->uri.data = uri;

        last = uri;

        if (uri < port) {
            port = NULL;
        }
    }

    if (port) {
        port++;

        len = last - port;

        n = ngx_atoi(port, len);

        if (n < 1 || n > 65535) {
            u->err = "invalid port";
            return NGX_ERROR;
        }

        u->port = (in_port_t) n;
        sin->sin_port = htons((in_port_t) n);

        u->port_text.len = len;
        u->port_text.data = port;

        last = port - 1;

    } else {
        if (uri == NULL) {

            if (u->listen) {

                /* test value as port only */

                n = ngx_atoi(host, last - host);

                if (n != NGX_ERROR) {

                    if (n < 1 || n > 65535) {
                        u->err = "invalid port";
                        return NGX_ERROR;
                    }

                    u->port = (in_port_t) n;
                    sin->sin_port = htons((in_port_t) n);

                    u->port_text.len = last - host;
                    u->port_text.data = host;

                    u->wildcard = 1;

                    return NGX_OK;
                }
            }
        }

        u->no_port = 1;
        u->port = u->default_port;
        sin->sin_port = htons(u->default_port);
    }

    len = last - host;

    if (len == 0) {
        u->err = "no host";
        return NGX_ERROR;
    }

    u->host.len = len;
    u->host.data = host;

    if (u->listen && len == 1 && *host == '*') {
        sin->sin_addr.s_addr = INADDR_ANY;
        u->wildcard = 1;
        return NGX_OK;
    }

    sin->sin_addr.s_addr = ngx_inet_addr(host, len);

    if (sin->sin_addr.s_addr != INADDR_NONE) {

        if (sin->sin_addr.s_addr == INADDR_ANY) {
            u->wildcard = 1;
        }

        u->naddrs = 1;

        u->addrs = ngx_pcalloc(pool, sizeof(ngx_addr_t));
        if (u->addrs == NULL) {
            return NGX_ERROR;
        }

        sin = ngx_pcalloc(pool, sizeof(struct sockaddr_in));
        if (sin == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(sin, u->sockaddr, sizeof(struct sockaddr_in));

        u->addrs[0].sockaddr = (struct sockaddr *) sin;
        u->addrs[0].socklen = sizeof(struct sockaddr_in);

        p = ngx_pnalloc(pool, u->host.len + sizeof(":65535") - 1);
        if (p == NULL) {
            return NGX_ERROR;
        }

        u->addrs[0].name.len = ngx_sprintf(p, "%V:%d",
                                           &u->host, u->port) - p;
        u->addrs[0].name.data = p;

        return NGX_OK;
    }

    if (u->no_resolve) {
        return NGX_OK;
    }

    if (ngx_inet_resolve_host(pool, u) != NGX_OK) {
        return NGX_ERROR;
    }

    u->family = u->addrs[0].sockaddr->sa_family;
    u->socklen = u->addrs[0].socklen;
    ngx_memcpy(u->sockaddr, u->addrs[0].sockaddr, u->addrs[0].socklen);

    switch (u->family) {

#if (NGX_HAVE_INET6)
    case AF_INET6:
        sin6 = (struct sockaddr_in6 *) &u->sockaddr;

        if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
            u->wildcard = 1;
        }

        break;
#endif

    default: /* AF_INET */
        sin = (struct sockaddr_in *) &u->sockaddr;

        if (sin->sin_addr.s_addr == INADDR_ANY) {
            u->wildcard = 1;
        }

        break;
    }

    return NGX_OK;
}
{% endhighlight %}
本函数用于解析IPv4类型的URL。例如：
<pre>
localhost:8088/query?id=1001
listen 8000;
</pre>

下面我们就来简要分析一下该函数：
{% highlight string %}
static ngx_int_t
ngx_parse_inet_url(ngx_pool_t *pool, ngx_url_t *u)
{
   //1) 找出port, uri以及args字段的起始位置

   //2) 解析uri

   //3) 解析port: 这里如果url中本身携带有port，则直接转换； 否则进行如下解析：
   //   若uri为NULL并且u->listen为1(即类似于listen 8000)，调用ngx_atoi()进行转换；
   //   否则设置u->no_port=1， 然后采用默认的端口

   
   //4) 转换u->host部分： 如果host直接是ip地址表示形式，可以直接转换；否则
   //调用ngx_inet_resolve_host()进行域名解析
}
{% endhighlight %}


## 10. 函数ngx_parse_inet6_url()
{% highlight string %}
static ngx_int_t
ngx_parse_inet6_url(ngx_pool_t *pool, ngx_url_t *u)
{
#if (NGX_HAVE_INET6)
    u_char               *p, *host, *port, *last, *uri;
    size_t                len;
    ngx_int_t             n;
    struct sockaddr_in6  *sin6;

    u->socklen = sizeof(struct sockaddr_in6);
    sin6 = (struct sockaddr_in6 *) &u->sockaddr;
    sin6->sin6_family = AF_INET6;

    host = u->url.data + 1;

    last = u->url.data + u->url.len;

    p = ngx_strlchr(host, last, ']');

    if (p == NULL) {
        u->err = "invalid host";
        return NGX_ERROR;
    }

    if (last - p) {

        port = p + 1;

        uri = ngx_strlchr(port, last, '/');

        if (uri) {
            if (u->listen || !u->uri_part) {
                u->err = "invalid host";
                return NGX_ERROR;
            }

            u->uri.len = last - uri;
            u->uri.data = uri;

            last = uri;
        }

        if (*port == ':') {
            port++;

            len = last - port;

            n = ngx_atoi(port, len);

            if (n < 1 || n > 65535) {
                u->err = "invalid port";
                return NGX_ERROR;
            }

            u->port = (in_port_t) n;
            sin6->sin6_port = htons((in_port_t) n);

            u->port_text.len = len;
            u->port_text.data = port;

        } else {
            u->no_port = 1;
            u->port = u->default_port;
            sin6->sin6_port = htons(u->default_port);
        }
    }

    len = p - host;

    if (len == 0) {
        u->err = "no host";
        return NGX_ERROR;
    }

    u->host.len = len + 2;
    u->host.data = host - 1;

    if (ngx_inet6_addr(host, len, sin6->sin6_addr.s6_addr) != NGX_OK) {
        u->err = "invalid IPv6 address";
        return NGX_ERROR;
    }

    if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
        u->wildcard = 1;
    }

    u->family = AF_INET6;
    u->naddrs = 1;

    u->addrs = ngx_pcalloc(pool, sizeof(ngx_addr_t));
    if (u->addrs == NULL) {
        return NGX_ERROR;
    }

    sin6 = ngx_pcalloc(pool, sizeof(struct sockaddr_in6));
    if (sin6 == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(sin6, u->sockaddr, sizeof(struct sockaddr_in6));

    u->addrs[0].sockaddr = (struct sockaddr *) sin6;
    u->addrs[0].socklen = sizeof(struct sockaddr_in6);

    p = ngx_pnalloc(pool, u->host.len + sizeof(":65535") - 1);
    if (p == NULL) {
        return NGX_ERROR;
    }

    u->addrs[0].name.len = ngx_sprintf(p, "%V:%d",
                                       &u->host, u->port) - p;
    u->addrs[0].name.data = p;

    return NGX_OK;

#else

    u->err = "the INET6 sockets are not supported on this platform";

    return NGX_ERROR;

#endif
}
{% endhighlight %}
本函数用于解析IPv6形式的Url。首先我们给出一个IPv6形式的url的例子：
<pre>
listen [::1]:12345
</pre>
下面我们再来简要分析一下本函数：
{% highlight string %}
static ngx_int_t
ngx_parse_inet6_url(ngx_pool_t *pool, ngx_url_t *u)
{
   //1) IPv6形式的url以“["开始，首先分析出其中的uri, port

   //2) 转换IPv6的host部分
}
{% endhighlight %}



## 11. 函数ngx_inet_resolve_host()
{% highlight string %}
#if (NGX_HAVE_GETADDRINFO && NGX_HAVE_INET6)

ngx_int_t
ngx_inet_resolve_host(ngx_pool_t *pool, ngx_url_t *u)
{
    u_char               *p, *host;
    size_t                len;
    in_port_t             port;
    ngx_uint_t            i;
    struct addrinfo       hints, *res, *rp;
    struct sockaddr_in   *sin;
    struct sockaddr_in6  *sin6;

    port = htons(u->port);

    host = ngx_alloc(u->host.len + 1, pool->log);
    if (host == NULL) {
        return NGX_ERROR;
    }

    (void) ngx_cpystrn(host, u->host.data, u->host.len + 1);

    ngx_memzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
#ifdef AI_ADDRCONFIG
    hints.ai_flags = AI_ADDRCONFIG;
#endif

    if (getaddrinfo((char *) host, NULL, &hints, &res) != 0) {
        u->err = "host not found";
        ngx_free(host);
        return NGX_ERROR;
    }

    ngx_free(host);

    for (i = 0, rp = res; rp != NULL; rp = rp->ai_next) {

        switch (rp->ai_family) {

        case AF_INET:
        case AF_INET6:
            break;

        default:
            continue;
        }

        i++;
    }

    if (i == 0) {
        u->err = "host not found";
        goto failed;
    }

    /* MP: ngx_shared_palloc() */

    u->addrs = ngx_pcalloc(pool, i * sizeof(ngx_addr_t));
    if (u->addrs == NULL) {
        goto failed;
    }

    u->naddrs = i;

    i = 0;

    /* AF_INET addresses first */

    for (rp = res; rp != NULL; rp = rp->ai_next) {

        if (rp->ai_family != AF_INET) {
            continue;
        }

        sin = ngx_pcalloc(pool, rp->ai_addrlen);
        if (sin == NULL) {
            goto failed;
        }

        ngx_memcpy(sin, rp->ai_addr, rp->ai_addrlen);

        sin->sin_port = port;

        u->addrs[i].sockaddr = (struct sockaddr *) sin;
        u->addrs[i].socklen = rp->ai_addrlen;

        len = NGX_INET_ADDRSTRLEN + sizeof(":65535") - 1;

        p = ngx_pnalloc(pool, len);
        if (p == NULL) {
            goto failed;
        }

        len = ngx_sock_ntop((struct sockaddr *) sin, rp->ai_addrlen, p, len, 1);

        u->addrs[i].name.len = len;
        u->addrs[i].name.data = p;

        i++;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {

        if (rp->ai_family != AF_INET6) {
            continue;
        }

        sin6 = ngx_pcalloc(pool, rp->ai_addrlen);
        if (sin6 == NULL) {
            goto failed;
        }

        ngx_memcpy(sin6, rp->ai_addr, rp->ai_addrlen);

        sin6->sin6_port = port;

        u->addrs[i].sockaddr = (struct sockaddr *) sin6;
        u->addrs[i].socklen = rp->ai_addrlen;

        len = NGX_INET6_ADDRSTRLEN + sizeof("[]:65535") - 1;

        p = ngx_pnalloc(pool, len);
        if (p == NULL) {
            goto failed;
        }

        len = ngx_sock_ntop((struct sockaddr *) sin6, rp->ai_addrlen, p,
                            len, 1);

        u->addrs[i].name.len = len;
        u->addrs[i].name.data = p;

        i++;
    }

    freeaddrinfo(res);
    return NGX_OK;

failed:

    freeaddrinfo(res);
    return NGX_ERROR;
}

#else /* !NGX_HAVE_GETADDRINFO || !NGX_HAVE_INET6 */

ngx_int_t
ngx_inet_resolve_host(ngx_pool_t *pool, ngx_url_t *u)
{
    u_char              *p, *host;
    size_t               len;
    in_port_t            port;
    in_addr_t            in_addr;
    ngx_uint_t           i;
    struct hostent      *h;
    struct sockaddr_in  *sin;

    /* AF_INET only */

    port = htons(u->port);

    in_addr = ngx_inet_addr(u->host.data, u->host.len);

    if (in_addr == INADDR_NONE) {
        host = ngx_alloc(u->host.len + 1, pool->log);
        if (host == NULL) {
            return NGX_ERROR;
        }

        (void) ngx_cpystrn(host, u->host.data, u->host.len + 1);

        h = gethostbyname((char *) host);

        ngx_free(host);

        if (h == NULL || h->h_addr_list[0] == NULL) {
            u->err = "host not found";
            return NGX_ERROR;
        }

        for (i = 0; h->h_addr_list[i] != NULL; i++) { /* void */ }

        /* MP: ngx_shared_palloc() */

        u->addrs = ngx_pcalloc(pool, i * sizeof(ngx_addr_t));
        if (u->addrs == NULL) {
            return NGX_ERROR;
        }

        u->naddrs = i;

        for (i = 0; i < u->naddrs; i++) {

            sin = ngx_pcalloc(pool, sizeof(struct sockaddr_in));
            if (sin == NULL) {
                return NGX_ERROR;
            }

            sin->sin_family = AF_INET;
            sin->sin_port = port;
            sin->sin_addr.s_addr = *(in_addr_t *) (h->h_addr_list[i]);

            u->addrs[i].sockaddr = (struct sockaddr *) sin;
            u->addrs[i].socklen = sizeof(struct sockaddr_in);

            len = NGX_INET_ADDRSTRLEN + sizeof(":65535") - 1;

            p = ngx_pnalloc(pool, len);
            if (p == NULL) {
                return NGX_ERROR;
            }

            len = ngx_sock_ntop((struct sockaddr *) sin,
                                sizeof(struct sockaddr_in), p, len, 1);

            u->addrs[i].name.len = len;
            u->addrs[i].name.data = p;
        }

    } else {

        /* MP: ngx_shared_palloc() */

        u->addrs = ngx_pcalloc(pool, sizeof(ngx_addr_t));
        if (u->addrs == NULL) {
            return NGX_ERROR;
        }

        sin = ngx_pcalloc(pool, sizeof(struct sockaddr_in));
        if (sin == NULL) {
            return NGX_ERROR;
        }

        u->naddrs = 1;

        sin->sin_family = AF_INET;
        sin->sin_port = port;
        sin->sin_addr.s_addr = in_addr;

        u->addrs[0].sockaddr = (struct sockaddr *) sin;
        u->addrs[0].socklen = sizeof(struct sockaddr_in);

        p = ngx_pnalloc(pool, u->host.len + sizeof(":65535") - 1);
        if (p == NULL) {
            return NGX_ERROR;
        }

        u->addrs[0].name.len = ngx_sprintf(p, "%V:%d",
                                           &u->host, ntohs(port)) - p;
        u->addrs[0].name.data = p;
    }

    return NGX_OK;
}

#endif /* NGX_HAVE_GETADDRINFO && NGX_HAVE_INET6 */

{% endhighlight %}

本函数用于将主机名解析成url的IP地址。在objs/ngx_auto_config.h头文件中我们有如下定义：
<pre>
#ifndef NGX_HAVE_GETADDRINFO
#define NGX_HAVE_GETADDRINFO  1
#endif
</pre>
但是当前并不支持```NGX_HAVE_INET6```。下面我们对该函数进行简要说明：
{% highlight string %}
#if (NGX_HAVE_GETADDRINFO && NGX_HAVE_INET6)

ngx_int_t
ngx_inet_resolve_host(ngx_pool_t *pool, ngx_url_t *u)
{
    //1) 调用getaddrinfo()解析主机名

    //2) 对IPv4及IPv6形式的IP地址保存到ngx_url_t.addrs中
}
#else
ngx_int_t
ngx_inet_resolve_host(ngx_pool_t *pool, ngx_url_t *u)
{
   // 1) 调用ngx_inet_addr()转换url中的Host主机

   //2) 如果转换结果为INADDR_NONE，表明不知直接IPv4表示形式的主机，此时调用
   // gethostbyname()函数来获得主机地址； 否则直接保存对应的ip即可
}
#endif
{% endhighlight %}


## 12. 函数ngx_cmp_sockaddr()
{% highlight string %}
ngx_int_t
ngx_cmp_sockaddr(struct sockaddr *sa1, socklen_t slen1,
    struct sockaddr *sa2, socklen_t slen2, ngx_uint_t cmp_port)
{
    struct sockaddr_in   *sin1, *sin2;
#if (NGX_HAVE_INET6)
    struct sockaddr_in6  *sin61, *sin62;
#endif
#if (NGX_HAVE_UNIX_DOMAIN)
    size_t                len;
    struct sockaddr_un   *saun1, *saun2;
#endif

    if (sa1->sa_family != sa2->sa_family) {
        return NGX_DECLINED;
    }

    switch (sa1->sa_family) {

#if (NGX_HAVE_INET6)
    case AF_INET6:

        sin61 = (struct sockaddr_in6 *) sa1;
        sin62 = (struct sockaddr_in6 *) sa2;

        if (cmp_port && sin61->sin6_port != sin62->sin6_port) {
            return NGX_DECLINED;
        }

        if (ngx_memcmp(&sin61->sin6_addr, &sin62->sin6_addr, 16) != 0) {
            return NGX_DECLINED;
        }

        break;
#endif

#if (NGX_HAVE_UNIX_DOMAIN)
    case AF_UNIX:

        saun1 = (struct sockaddr_un *) sa1;
        saun2 = (struct sockaddr_un *) sa2;

        if (slen1 < slen2) {
            len = slen1 - offsetof(struct sockaddr_un, sun_path);

        } else {
            len = slen2 - offsetof(struct sockaddr_un, sun_path);
        }

        if (len > sizeof(saun1->sun_path)) {
            len = sizeof(saun1->sun_path);
        }

        if (ngx_memcmp(&saun1->sun_path, &saun2->sun_path, len) != 0) {
            return NGX_DECLINED;
        }

        break;
#endif

    default: /* AF_INET */

        sin1 = (struct sockaddr_in *) sa1;
        sin2 = (struct sockaddr_in *) sa2;

        if (cmp_port && sin1->sin_port != sin2->sin_port) {
            return NGX_DECLINED;
        }

        if (sin1->sin_addr.s_addr != sin2->sin_addr.s_addr) {
            return NGX_DECLINED;
        }

        break;
    }

    return NGX_OK;
}

{% endhighlight %}
此函数用于比较两个socket地址是否相同。函数较为简单，这里不再进行细讲。


<pre>
//IPv4形式的socket 地址
struct sockaddr_in   *sin1, *sin2;

//IPv6形式的socket 地址
struct sockaddr_in6  *sin61, *sin62;

//unix域socket地址
struct sockaddr_un   *saun1, *saun2;
</pre>


<br />
<br />

**[参看]**

1. [无类域间路由](https://baike.baidu.com/item/%E6%97%A0%E7%B1%BB%E5%9F%9F%E9%97%B4%E8%B7%AF%E7%94%B1/240168?fr=aladdin&fromid=3695195&fromtitle=CIDR)

2. [子网掩码](https://baike.baidu.com/item/%E5%AD%90%E7%BD%91%E6%8E%A9%E7%A0%81/100207?fr=aladdin)

<br />
<br />
<br />

