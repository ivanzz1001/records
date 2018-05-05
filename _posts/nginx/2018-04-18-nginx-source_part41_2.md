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

   //2) 处理
}
{% endhighlight %}




<br />
<br />
<br />

