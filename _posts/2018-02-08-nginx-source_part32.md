---
layout: post
title: core/ngx_array.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要讲述一下nginx中数组的实现。


<!-- more -->

<br />
<br />

## 1. core_ngx_array.h头文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    void        *elts;
    ngx_uint_t   nelts;
    size_t       size;
    ngx_uint_t   nalloc;
    ngx_pool_t  *pool;
} ngx_array_t;


ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
void ngx_array_destroy(ngx_array_t *a);
void *ngx_array_push(ngx_array_t *a);
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);


static ngx_inline ngx_int_t
ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;
    array->size = size;
    array->nalloc = n;
    array->pool = pool;

    array->elts = ngx_palloc(pool, n * size);
    if (array->elts == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


#endif /* _NGX_ARRAY_H_INCLUDED_ */
{% endhighlight %}

### 1.1 nginx中数组数据结构
{% highlight string %}
typedef struct {
    void        *elts;               //存储数据元素的基址
    ngx_uint_t   nelts;              //当前实际的元素个数
    size_t       size;               //每个元素的大小
    ngx_uint_t   nalloc;             //当前所分配的可以容纳的元素个数
    ngx_pool_t  *pool;               //所关联的内存池
} ngx_array_t;
{% endhighlight %}
请参看下图：

![ngx-array](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_array.jpg)


### 1.2 相关函数声明
{% highlight string %}
ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
void ngx_array_destroy(ngx_array_t *a);
void *ngx_array_push(ngx_array_t *a);
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);
{% endhighlight %}

<br />
<br />
<br />

