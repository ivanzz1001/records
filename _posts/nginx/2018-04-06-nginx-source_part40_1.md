---
layout: post
title: core/ngx_hash.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx中hash


<!-- more -->


## 1. ngx_hash_t数据结构
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HASH_H_INCLUDED_
#define _NGX_HASH_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    void             *value;
    u_short           len;
    u_char            name[1];
} ngx_hash_elt_t;


typedef struct {
    ngx_hash_elt_t  **buckets;
    ngx_uint_t        size;
} ngx_hash_t;
{% endhighlight %}
这里涉及到两个数据结构，我们先来讲述```ngx_hash_elt_t```数据结构：

* ```value```: 某个key对一个的值，即<key,value>中的value

* ```len```: name长度

* ```name```: 占位符，某个要hash的数据（在nginx中表现为字符串)，即<key,value>中的key

{% highlight string %}
注： 关于0长度数组，请参看《Arrays of Length Zero-gcc》
{% endhighlight %}

对于```ngx_hash_t```数据结构：

* ```buckets```: hash桶首地址

* ```size```: hash桶个数

## 2. ngx_hash_wildcard_t数据结构
{% highlight string %}
typedef struct {
    ngx_hash_t        hash;
    void             *value;
} ngx_hash_wildcard_t;
{% endhighlight %}
nginx支持带有通配符的散列表，用本结构代表前置或后置通配符散列表（注意： nginx不能同时包含在前与在后都有通配符的情况）：

* ```hash```: 基本散列表结构

* ```value```: 指向用户数据

## 3. ngx_hash_key_t数据结构
{% highlight string %}
typedef struct {
    ngx_str_t         key;
    ngx_uint_t        key_hash;
    void             *value;
} ngx_hash_key_t;


typedef ngx_uint_t (*ngx_hash_key_pt) (u_char *data, size_t len);
{% endhighlight %}



<br />
<br />

1. [Nginx 源码分析：ngx_hash_t（上）](https://segmentfault.com/a/1190000002770345)

2. [Nginx 源码分析：ngx_hash_t（下）](https://segmentfault.com/a/1190000002771908)

3. [Nginx源码分析之ngx_hash_t](https://blog.csdn.net/zhangxiao93/article/details/53844203)

4. [hash结构ngx_hash_t](https://blog.csdn.net/livelylittlefish/article/details/6636229)

5. [从基本hash表到支持通配符的hash表](https://blog.csdn.net/a987073381/article/details/52357990)

<br />
<br />
<br />

