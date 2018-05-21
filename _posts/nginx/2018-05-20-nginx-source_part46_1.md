---
layout: post
title: core/ngx_murmurhash.c(h)源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要讲述一下nginx中实现的murmurhash算法。murmurhash算法具有高运算性能、低碰撞率的特征。由Austin Appleby创建于2008年，现已应用到Hadoop、libstdc++、nginx、libmemcached等开源系统。2011年Appleby被Google雇佣，随后Google推出其变种的CityHash算法。


<!-- more -->


## 1. core/ngx_murmurhash.h源文件
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_MURMURHASH_H_INCLUDED_
#define _NGX_MURMURHASH_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


uint32_t ngx_murmur_hash2(u_char *data, size_t len);


#endif /* _NGX_MURMURHASH_H_INCLUDED_ */

{% endhighlight %}

头文件只是声明了```ngx_murmur_hash2()```函数。


## 2. core/ngx_murmurhash.c源文件
{% highlight string %}

/*
 * Copyright (C) Austin Appleby
 */


#include <ngx_config.h>
#include <ngx_core.h>


uint32_t
ngx_murmur_hash2(u_char *data, size_t len)
{
    uint32_t  h, k;

    h = 0 ^ len;

    while (len >= 4) {
        k  = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;

        k *= 0x5bd1e995;
        k ^= k >> 24;
        k *= 0x5bd1e995;

        h *= 0x5bd1e995;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
    case 3:
        h ^= data[2] << 16;
    case 2:
        h ^= data[1] << 8;
    case 1:
        h ^= data[0];
        h *= 0x5bd1e995;
    }

    h ^= h >> 13;
    h *= 0x5bd1e995;
    h ^= h >> 15;

    return h;
}

{% endhighlight %}

对于本函数的实现，这里暂不做介绍。



<br />
<br />


**[参看]**

1. [murmurhash](https://sites.google.com/site/murmurhash/)


2. [Murmurhash算法： 高运算性能、低碰撞率的hash算法](http://comeonbabye.iteye.com/blog/1676199)

<br />
<br />
<br />

