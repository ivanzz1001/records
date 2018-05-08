---
layout: post
title: core/ngx_list.c(h)文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx中list的实现。


<!-- more -->

## 1. core/ngx_list.h
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_list_part_s  ngx_list_part_t;

struct ngx_list_part_s {
    void             *elts;
    ngx_uint_t        nelts;
    ngx_list_part_t  *next;
};


typedef struct {
    ngx_list_part_t  *last;
    ngx_list_part_t   part;
    size_t            size;
    ngx_uint_t        nalloc;
    ngx_pool_t       *pool;
} ngx_list_t;


ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */


void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */

{% endhighlight %}

下面对ngx_list.h头文件各部分做一个简单的解释：


### 1.1 相关数据结构定义

**1) ngx_list_part_t结构**

```ngx_list_part_t```代表一个链表中的一个part，下面对各字段做个简要介绍：

* ```elts```: 一个part中存放元素的基址

* ```nelts```: 一个part中当前元素个数

* ```next```： 指向链表的下一个part

**2) ngx_list_t结构**

代表着总个链表结构，下面对各字段做个简要介绍：

* ```last```: 指向链表的最后一个part

* ```part```: 链表的第一个part

* ```size```: 链表中每个元素的大小

* ```nalloc```: 每一个part的容量

* ```pool```: 该链表结构对应的内存池

下面给出```ngx_list_t```的一个整体结构图：

![ngx-list](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_list.jpg)

### 1.2 相关函数声明

{% highlight string %}
//用于创建一个链表
ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);


//向链表中push一个元素
void *ngx_list_push(ngx_list_t *list);
{% endhighlight %}

### 1.3 函数ngx_list_init()
{% highlight string %}
static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}

{% endhighlight %}
本函数较为简单，就是对一个链表结构进行简单的初始化。

## 2. core/ngx_list.c源文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_list_t *
ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    ngx_list_t  *list;

    list = ngx_palloc(pool, sizeof(ngx_list_t));
    if (list == NULL) {
        return NULL;
    }

    if (ngx_list_init(list, pool, n, size) != NGX_OK) {
        return NULL;
    }

    return list;
}


void *
ngx_list_push(ngx_list_t *l)
{
    void             *elt;
    ngx_list_part_t  *last;

    last = l->last;

    if (last->nelts == l->nalloc) {

        /* the last part is full, allocate a new list part */

        last = ngx_palloc(l->pool, sizeof(ngx_list_part_t));
        if (last == NULL) {
            return NULL;
        }

        last->elts = ngx_palloc(l->pool, l->nalloc * l->size);
        if (last->elts == NULL) {
            return NULL;
        }

        last->nelts = 0;
        last->next = NULL;

        l->last->next = last;
        l->last = last;
    }

    elt = (char *) last->elts + l->size * last->nelts;
    last->nelts++;

    return elt;
}

{% endhighlight %}


下面我们简单介绍一下这两个函数：

**1） 函数ngx_list_create()**

本函数主要是创建一个```ngx_list_t```结构，然后调用ngx_list_init()进行链表的初始化。

**2) 函数ngx_list_push()**

本函数用于从链表尾部找出一个可用于插入的位置。


<br />
<br />
<br />

