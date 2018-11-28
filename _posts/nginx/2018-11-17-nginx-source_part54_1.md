---
layout: post
title: core/ngx_radix_tree.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们主要讲述一下ngx_radix_tree的实现。


<!-- more -->


## 1. ngx_radix_not_t数据结构 
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_RADIX_TREE_H_INCLUDED_
#define _NGX_RADIX_TREE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_RADIX_NO_VALUE   (uintptr_t) -1

typedef struct ngx_radix_node_s  ngx_radix_node_t;

struct ngx_radix_node_s {
    ngx_radix_node_t  *right;
    ngx_radix_node_t  *left;
    ngx_radix_node_t  *parent;
    uintptr_t          value;
};
{% endhighlight %}
这里首先定义了一个宏```NGX_RADIX_NO_VALUE```，其主要用于标志```ngx_radix_node_t.value```当前并未设置。接下来，我们简要介绍一下ngx_radix_node_t，其用于标识一个radix tree的节点：

* right: 指向右子树

* left: 指向左子树

* parent: 指向父节点

* value: 存储的是指针的值，指向用户定义的数据结构。如果这个节点还未使用，value的值将是```NGX_RADIX_NO_VALUE```。

## 2. 结构体ngx_radix_tree_t
{% highlight string %}
typedef struct {
    ngx_radix_node_t  *root;
    ngx_pool_t        *pool;
    ngx_radix_node_t  *free;
    char              *start;
    size_t             size;
} ngx_radix_tree_t;
{% endhighlight %}

ngx_radix_tree_t用于标识一棵基数树，我们简要介绍一下各个字段的含义：

* root: 指向整棵基数树的根节点

* pool: 与基数树关联的内存池结构

* free: 管理已经分配但暂时未使用（不在树中）的节点，free实际上是所有不在树中节点的单链表。单链表使用right来作为'next'指针

* start: 已分配内存中还未使用的内存首地址

* size: 已分配内存中还未使用的内存大小

## 3. 相关函数声明
{% highlight string %}
// 创建一棵radix树，preallocate是预分配的基数树节点数
ngx_radix_tree_t *ngx_radix_tree_create(ngx_pool_t *pool,
    ngx_int_t preallocate);

//向radix树中插入一个节点
ngx_int_t ngx_radix32tree_insert(ngx_radix_tree_t *tree,
    uint32_t key, uint32_t mask, uintptr_t value);

//从radix树中删除一个节点
ngx_int_t ngx_radix32tree_delete(ngx_radix_tree_t *tree,
    uint32_t key, uint32_t mask);

//从radix树中查找一个节点
uintptr_t ngx_radix32tree_find(ngx_radix_tree_t *tree, uint32_t key);

#if (NGX_HAVE_INET6)
ngx_int_t ngx_radix128tree_insert(ngx_radix_tree_t *tree,
    u_char *key, u_char *mask, uintptr_t value);
ngx_int_t ngx_radix128tree_delete(ngx_radix_tree_t *tree,
    u_char *key, u_char *mask);
uintptr_t ngx_radix128tree_find(ngx_radix_tree_t *tree, u_char *key);
#endif
{% endhighlight %}
当前，我们暂时不支持```NGX_HAVE_INET6```宏定义。









<br />
<br />

**[参看]**

1. [基数树结构ngx_radix_tree_t](https://blog.csdn.net/u012819339/article/details/53581203?utm_source=blogxgwz0)

2. [nginx 学习八 高级数据结构之基数树ngx_radix_tree_t](https://www.cnblogs.com/blfshiye/p/5269679.html)


<br />
<br />
<br />

