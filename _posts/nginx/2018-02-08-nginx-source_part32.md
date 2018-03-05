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



## 1. core/ngx_array.h头文件
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

下面对ngx_array.h头文件各部分做一个简单的解释：

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
//创建一个容量为n，每个元素大小为size的数组
ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);

//销毁数组（这里是将数组占用的空间交还给内存池)
void ngx_array_destroy(ngx_array_t *a);


//增加一个元素到a数组中(这里返回要增加的这个元素的存储位置)
void *ngx_array_push(ngx_array_t *a);

//增加n个元素到a数组中(这里返回这n个新添加元素的起始存储位置)
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);
{% endhighlight %}

### 1.3 函数ngx_array_init()
{% highlight string %}
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
{% endhighlight %}
这里为数组从pool内存池中分配对应的空间，并初始化数组。



## 2. core/ngx_array.c源文件

### 2.1 函数ngx_array_create()
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_array_t *
ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size)
{
    ngx_array_t *a;

    a = ngx_palloc(p, sizeof(ngx_array_t));
    if (a == NULL) {
        return NULL;
    }

    if (ngx_array_init(a, p, n, size) != NGX_OK) {
        return NULL;
    }

    return a;
}
{% endhighlight %}
这里首先分配```ngx_array_t *```数据结构分配内存空间，然后再初始化该数组。

### 2.2 函数ngx_array_destroy()
{% highlight string %}
void
ngx_array_destroy(ngx_array_t *a)
{
    ngx_pool_t  *p;

    p = a->pool;

    if ((u_char *) a->elts + a->size * a->nalloc == p->d.last) {
        p->d.last -= a->size * a->nalloc;
    }

    if ((u_char *) a + sizeof(ngx_array_t) == p->d.last) {
        p->d.last = (u_char *) a;
    }
}
{% endhighlight %}
这里将从内存池分配的数据归还给内存池。

### 2.3 函数ngx_array_push()
{% highlight string %}
void *
ngx_array_push(ngx_array_t *a)
{
    void        *elt, *new;
    size_t       size;
    ngx_pool_t  *p;

    if (a->nelts == a->nalloc) {

        /* the array is full */

        size = a->size * a->nalloc;

        p = a->pool;

        if ((u_char *) a->elts + size == p->d.last
            && p->d.last + a->size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += a->size;
            a->nalloc++;

        } else {
            /* allocate a new array */

            new = ngx_palloc(p, 2 * size);
            if (new == NULL) {
                return NULL;
            }

            ngx_memcpy(new, a->elts, size);
            a->elts = new;
            a->nalloc *= 2;
        }
    }

    elt = (u_char *) a->elts + a->size * a->nelts;
    a->nelts++;

    return elt;
}
{% endhighlight %}

这里主要需要注意的是，在当前数据元素个数达到总容量时，如何分配内存：

* 如果该数组所绑定的内存池可分配的内存正好在原数组空间的后边，且还有剩余的空间可分配，则直接分配即可
{% highlight string %}
if ((u_char *) a->elts + size == p->d.last
        && p->d.last + a->size <= p->d.end)
{
}
{% endhighlight %}

* 如果该数组所绑定的内存池后续没有空间可分配，则需要在该内存池中另寻另一块空间来分配，并且分配的容量扩大一倍

### 1.4 函数ngx_array_push_n()
{% highlight string %}
void *
ngx_array_push_n(ngx_array_t *a, ngx_uint_t n)
{
    void        *elt, *new;
    size_t       size;
    ngx_uint_t   nalloc;
    ngx_pool_t  *p;

    size = n * a->size;

    if (a->nelts + n > a->nalloc) {

        /* the array is full */

        p = a->pool;

        if ((u_char *) a->elts + a->size * a->nalloc == p->d.last
            && p->d.last + size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += size;
            a->nalloc += n;

        } else {
            /* allocate a new array */

            nalloc = 2 * ((n >= a->nalloc) ? n : a->nalloc);

            new = ngx_palloc(p, nalloc * a->size);
            if (new == NULL) {
                return NULL;
            }

            ngx_memcpy(new, a->elts, a->nelts * a->size);
            a->elts = new;
            a->nalloc = nalloc;
        }
    }

    elt = (u_char *) a->elts + a->size * a->nelts;
    a->nelts += n;

    return elt;
}
{% endhighlight %}

本函数与```ngx_array_push()```类似，这里不再赘述。

<br />
<br />
<br />

