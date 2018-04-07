---
layout: post
title: core/ngx_hash.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx中hash。nginx中hash有三种类型：

* 不带通配符的hash

* 带前向通配符的hash

* 带后向通配符的hash

<pre>
注意： 不支持同时带有前向通配符与后向通配符的hash
</pre>


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

![ngx-hash-buckets](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_hash_buckets.jpg)


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
* ```key```: nginx的字符串结构，对应<key,value>结构中的key

* ```key_hash```: 由该key计算出的hash值

* ```value```: 该key对应的value值

## 4. ngx_hash_combined_t数据结构
{% highlight string %}
typedef struct {
    ngx_hash_t            hash;
    ngx_hash_wildcard_t  *wc_head;
    ngx_hash_wildcard_t  *wc_tail;
} ngx_hash_combined_t;
{% endhighlight %}
实际使用hash的时候，可能是精准匹配，也可能是前缀匹配，也可能是后缀匹配。因此Nginx封装了一个包含这三种情况的结构：

* ```hash```: 精准匹配的散列表

* ```wc_head```: 查询前置通配符的散列表

* ```wc_tail```: 查询后置通配符的散列表

## 5. ngx_hash_init_t数据结构
{% highlight string %}
typedef struct {
    ngx_hash_t       *hash;
    ngx_hash_key_pt   key;

    ngx_uint_t        max_size;
    ngx_uint_t        bucket_size;

    char             *name;
    ngx_pool_t       *pool;
    ngx_pool_t       *temp_pool;
} ngx_hash_init_t;
{% endhighlight %}
```ngx_hash_init_t```数据结构用于辅助我们建立起一个初始化的hash结构：

* ```hash```: 指向普通的完全匹配散列表

* ```key```: 散列方法

* ```max_size```: hash表中桶的最大值,实际桶的个数存放在前面ngx_hash_t中的size字段中

* ```bucket_size```: 每个bucket的空间，即ngx_hash_elt_t的空间大小

* ```name```: hash表的名称(仅在错误日志中使用)

* ```pool```: 该hash结构从pool指向的内存池中分配

* ```temp_pool```: 分配临时数据空间的内存池

## 6. ngx_hash_keys_arrays_t结构
{% highlight string %}
#define NGX_HASH_SMALL            1
#define NGX_HASH_LARGE            2

#define NGX_HASH_LARGE_ASIZE      16384
#define NGX_HASH_LARGE_HSIZE      10007

#define NGX_HASH_WILDCARD_KEY     1
#define NGX_HASH_READONLY_KEY     2

typedef struct {
    ngx_uint_t        hsize;

    ngx_pool_t       *pool;
    ngx_pool_t       *temp_pool;

    ngx_array_t       keys;
    ngx_array_t      *keys_hash;

    ngx_array_t       dns_wc_head;
    ngx_array_t      *dns_wc_head_hash;

    ngx_array_t       dns_wc_tail;
    ngx_array_t      *dns_wc_tail_hash;
} ngx_hash_keys_arrays_t;
{% endhighlight %}

在介绍```ngx_hash_keys_arrays_t```数据结构之前，我们先介绍一下各宏定义：

* ```NGX_HASH_SMALL```: 基于数组的Hash,用于指定当前hash的体量类别（小)

* ```NGX_HASH_LARGE```: 基于数组的Hash,用于指定当前hash的体量类别（大)

* ```NGX_HASH_LARGE_ASIZE```: 基于数组的Hash，用于指定```ngx_hash_keys_arrays_t```结构中keys、dns_wc_head、dns_wc_tail数组的容量

* ```NGX_HASH_LARGE_HSIZE```: 基于数组的Hash，用于指定桶的数量

* ```NGX_HASH_WILDCARD_KEY```: hash key类型

* ```NGX_HASH_READONLY_KEY```: hash key类型

下面再简要介绍```ngx_hash_keys_arrays_t```数据结构：

* ```hsize```: 用于指定桶大小

* ```pool```: 该hash表所关联的内存池

* ```temp_pool```： 临时内存池，下面的临时动态数组是由临时内存池分配的

* ```keys```: 存放所有非通配符key的数组。数据类型为```ngx_hash_key_t```

* ```key_hash```: 这是个二维数组，第一个维度代表的是bucket编号，那么keys_hash[i]中存放的是所有key求Hash后对hsize取模值为i的key列表。假设有3个key, 分别是key1、key2和key3，假设hash算出来以后对hsize去模的值都是i，那么这三个key就顺序存放在keys_hash[i][0]、keys_hash[i][1]、keys_hash[i][2]。该值在调用的过程中用来保存和检测是否有冲突的key值，也就是是否有重复


* ```dns_wc_head```: 存放前向通配符key被处理完成以后的值。比如```*.abc.com```被处理完成以后，变成```com.abc.```被存放在此数组中。数据类型为```ngx_hash_key_t```

* ```dns_wc_head_hash```: 这是一个二维数组。该值在调用的过程中用来保存和检测是否有冲突的key值，也就是是否有重复

* ```dns_wc_tail```: 存放后向通配符key被处理完成以后的值。比如：```mail.xxx.*```被处理完成以后，变成```mail.xxx.```被存放在此数组中。数据类型为```ngx_hash_key_t```

* ```dns_wc_tail_hash```: 这是一个二维数组。该值在调用的过程中用来保存和检测是否有冲突的key值，也就是是否有重复



## 7. ngx_table_elt_t数据结构
{% highlight string %}
typedef struct {
    ngx_uint_t        hash;
    ngx_str_t         key;
    ngx_str_t         value;
    u_char           *lowcase_key;
} ngx_table_elt_t;
{% endhighlight %}
ngx_table_elt_t是一个key/value对，ngx_str_t类型的key、value成员分别存储的是名字、值字符串。

hash成员表明ngx_table_elt_t也可以是某个散列表数据结构中的成员。ngx_uint_t类型的hash成员可以在ngx_hash_t中更快地找到相同key的ngx_table_elt_t数据。lowcase_key指向的是全小写的key字符串。


ngx_table_elt_t是为HTTP头部量身定制的，其中key存储头部名称，value存储对应的值，lowcase_key是为了忽悠HTTP头部名称的大小写，hash用于快速检索到头部。比如：Content-Length: 1024

## 8. 相关函数声明
{% highlight string %}
//用于从hash表中查找指定key和name的元素
void *ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len);

//查找带前向通配符的元素
void *ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);

//查找带后向通配符的元素
void *ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);

//从combined中查找指定key和name的元素
void *ngx_hash_find_combined(ngx_hash_combined_t *hash, ngx_uint_t key,
    u_char *name, size_t len);


//hash表初始化
ngx_int_t ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);

//wildcard哈希表初始化
ngx_int_t ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);

//单个字符的哈希算法
#define ngx_hash(key, c)   ((ngx_uint_t) key * 31 + c)

//求data的hash算法
ngx_uint_t ngx_hash_key(u_char *data, size_t len);

//首先将data转换成小写，再求hash
ngx_uint_t ngx_hash_key_lc(u_char *data, size_t len);

//首先将src转换成小写(转换后的结果存放在dst中)，再求hash
ngx_uint_t ngx_hash_strlow(u_char *dst, u_char *src, size_t n);

// 基于数组的hash keys初始化
ngx_int_t ngx_hash_keys_array_init(ngx_hash_keys_arrays_t *ha, ngx_uint_t type);

//向基于数组的hash keys中添加元素
ngx_int_t ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key,
    void *value, ngx_uint_t flags);
{% endhighlight %}

<br />
<br />

1. [Nginx 源码分析：ngx_hash_t（上）](https://segmentfault.com/a/1190000002770345)

2. [Nginx 源码分析：ngx_hash_t（下）](https://segmentfault.com/a/1190000002771908)

3. [Nginx源码分析之ngx_hash_t](https://blog.csdn.net/zhangxiao93/article/details/53844203)

4. [hash结构ngx_hash_t](https://blog.csdn.net/livelylittlefish/article/details/6636229)

5. [从基本hash表到支持通配符的hash表](https://blog.csdn.net/a987073381/article/details/52357990)

6. [Nginx基础数据结构分析-ngx_hash_keys_arrays_t](http://blog.chinaunix.net/uid-686647-id-3392072.html)

7. [nginx源码剖析数据结构-哈希表](http://www.bkjia.com/ASPjc/905190.html)

8. [了解 Nginx 基本概念](https://segmentfault.com/a/1190000007172005)

9. [建立hash表的前提条件](https://blog.csdn.net/a987073381/article/details/52389939)
<br />
<br />
<br />

