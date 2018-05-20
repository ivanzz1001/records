---
layout: post
title: core/ngx_murmurhash.c(h)源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要讲述一下nginx中实现的murmurhash算法


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







<br />
<br />


**[参看]**

1. [murmurhash](https://sites.google.com/site/murmurhash/)


2. [Murmurhash算法： 高运算性能、低碰撞率的hash算法](http://comeonbabye.iteye.com/blog/1676199)

<br />
<br />
<br />

