---
layout: post
title: core/ngx_radix_tree.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们主要讲述一下ngx_radix_tree的实现。


<!-- more -->


## 1. 函数ngx_radix_tree_create()
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


static ngx_radix_node_t *ngx_radix_alloc(ngx_radix_tree_t *tree);


ngx_radix_tree_t *
ngx_radix_tree_create(ngx_pool_t *pool, ngx_int_t preallocate)
{
    uint32_t           key, mask, inc;
    ngx_radix_tree_t  *tree;

    tree = ngx_palloc(pool, sizeof(ngx_radix_tree_t));
    if (tree == NULL) {
        return NULL;
    }

    tree->pool = pool;
    tree->free = NULL;
    tree->start = NULL;
    tree->size = 0;

    tree->root = ngx_radix_alloc(tree);
    if (tree->root == NULL) {
        return NULL;
    }

    tree->root->right = NULL;
    tree->root->left = NULL;
    tree->root->parent = NULL;
    tree->root->value = NGX_RADIX_NO_VALUE;

    if (preallocate == 0) {
        return tree;
    }

    /*
     * Preallocation of first nodes : 0, 1, 00, 01, 10, 11, 000, 001, etc.
     * increases TLB hits even if for first lookup iterations.
     * On 32-bit platforms the 7 preallocated bits takes continuous 4K,
     * 8 - 8K, 9 - 16K, etc.  On 64-bit platforms the 6 preallocated bits
     * takes continuous 4K, 7 - 8K, 8 - 16K, etc.  There is no sense to
     * to preallocate more than one page, because further preallocation
     * distributes the only bit per page.  Instead, a random insertion
     * may distribute several bits per page.
     *
     * Thus, by default we preallocate maximum
     *     6 bits on amd64 (64-bit platform and 4K pages)
     *     7 bits on i386 (32-bit platform and 4K pages)
     *     7 bits on sparc64 in 64-bit mode (8K pages)
     *     8 bits on sparc64 in 32-bit mode (8K pages)
     */

    if (preallocate == -1) {
        switch (ngx_pagesize / sizeof(ngx_radix_node_t)) {

        /* amd64 */
        case 128:
            preallocate = 6;
            break;

        /* i386, sparc64 */
        case 256:
            preallocate = 7;
            break;

        /* sparc64 in 32-bit mode */
        default:
            preallocate = 8;
        }
    }

    mask = 0;
    inc = 0x80000000;

    while (preallocate--) {

        key = 0;
        mask >>= 1;
        mask |= 0x80000000;

        do {
            if (ngx_radix32tree_insert(tree, key, mask, NGX_RADIX_NO_VALUE)
                != NGX_OK)
            {
                return NULL;
            }

            key += inc;

        } while (key);

        inc >>= 1;
    }

    return tree;
}
{% endhighlight %}

本函数用于创建基数树，失败时返回NULL指针。preallocate是预分配的基数树节点数，该值为-1时表示根据当前操作系统中一个页面大小来预分配radix tree节点。下面简要介绍一下本函数的实现：

1） 创建ngx_radix_tree_t数据结构

2） 计算当前的预分配节点数
{% highlight string %}
ngx_radix_tree_t *
ngx_radix_tree_create(ngx_pool_t *pool, ngx_int_t preallocate)
{
    ...

    /*
     * Preallocation of first nodes : 0, 1, 00, 01, 10, 11, 000, 001, etc.
     * increases TLB hits even if for first lookup iterations.
     * On 32-bit platforms the 7 preallocated bits takes continuous 4K,
     * 8 - 8K, 9 - 16K, etc.  On 64-bit platforms the 6 preallocated bits
     * takes continuous 4K, 7 - 8K, 8 - 16K, etc.  There is no sense to
     * to preallocate more than one page, because further preallocation
     * distributes the only bit per page.  Instead, a random insertion
     * may distribute several bits per page.
     *
     * Thus, by default we preallocate maximum
     *     6 bits on amd64 (64-bit platform and 4K pages)
     *     7 bits on i386 (32-bit platform and 4K pages)
     *     7 bits on sparc64 in 64-bit mode (8K pages)
     *     8 bits on sparc64 in 32-bit mode (8K pages)
     */

    if (preallocate == -1) {
        switch (ngx_pagesize / sizeof(ngx_radix_node_t)) {

        /* amd64 */
        case 128:
            preallocate = 6;
            break;

        /* i386, sparc64 */
        case 256:
            preallocate = 7;
            break;

        /* sparc64 in 32-bit mode */
        default:
            preallocate = 8;
        }
    }
    ...
}
{% endhighlight %}
当前```ngx_pagesize```大小为4096，```ngx_radix_node_t```结构体的大小为32，因此计算出preallocate=6，其实就是计算一个页面可以分配多少个预留节点。

从函数的注释中，我们了解到：预分配的节点的key(隐含，因为节点中并未有此字段）分别为0、1、00、01、10、11等，这样是为了提高查询时的TLB命中。在32-bit平台上，preallocate的值一般为7，即一页可以分配254个节点； 在64-bit平台上，preallocate的值一般为6,即一页可以分配126个节点； 否则默认分配510个节点。

3) 向radix tree中插入预分配的节点
{% highlight string %}
ngx_radix_tree_t *
ngx_radix_tree_create(ngx_pool_t *pool, ngx_int_t preallocate)
{
	mask = 0;
    inc = 0x80000000;

	//这里我们preallocate值为6， 最后所建的radix tree节点(包括root节点）总个数为2^(preallocate + 1)-1，
	// 每一层的个数为2^(6-preallocate)
	
	//下面我们给出每一次循环各变量的变化情况
	/*
	 * preallocate:      6             5            4             3            2              1
	 *   mask     : 0x80000000    0xc0000000    0xe0000000    0xf0000000    0xf8000000     0xfc000000
	 *   inc      : 0x80000000    0x40000000    0x20000000    0x10000000    0x08000000     0x04000000
	 * 添加节点个数：     2             4            8             16            32             64
	 * 插入的key值：  0x0          0x0           0x0           0x0           0x0            0x0
	 *              0x80000000    0x40000000    0x20000000    0x10000000    0x08000000     0x04000000
	 *                            0x80000000    0x40000000    0x20000000    0x10000000     0x08000000
	 *                            0x0           0x60000000    0x30000000    0x18000000     0x0c000000
	 *                                          0x80000000    0x40000000    0x20000000     0x0e000000
	 *                                          0xa0000000    0x50000000    0x28000000     0x10000000
	 *                                          0xc0000000    0x60000000    0x30000000     0x14000000
	 *                                          0xe0000000    0x70000000    0x38000000     0x18000000
     *                                                        ...           ...            ...
	 */

	while (preallocate--) {
	
		key = 0;
		mask >>= 1;
		mask |= 0x80000000;
	
		do {
			if (ngx_radix32tree_insert(tree, key, mask, NGX_RADIX_NO_VALUE)
				!= NGX_OK)
			{
				return NULL;
			}
		
			key += inc;
		
		} while (key);
		
		inc >>= 1;
	}

}
{% endhighlight %}

## 2. 函数ngx_radix32tree_insert()
{% highlight string %}
ngx_int_t
ngx_radix32tree_insert(ngx_radix_tree_t *tree, uint32_t key, uint32_t mask,
    uintptr_t value)
{
    uint32_t           bit;
    ngx_radix_node_t  *node, *next;

    bit = 0x80000000;

    node = tree->root;
    next = tree->root;

    while (bit & mask) {
        if (key & bit) {
            next = node->right;

        } else {
            next = node->left;
        }

        if (next == NULL) {
            break;
        }

        bit >>= 1;
        node = next;
    }

    if (next) {
        if (node->value != NGX_RADIX_NO_VALUE) {
            return NGX_BUSY;
        }

        node->value = value;
        return NGX_OK;
    }

    while (bit & mask) {
        next = ngx_radix_alloc(tree);
        if (next == NULL) {
            return NGX_ERROR;
        }

        next->right = NULL;
        next->left = NULL;
        next->parent = node;
        next->value = NGX_RADIX_NO_VALUE;

        if (key & bit) {
            node->right = next;

        } else {
            node->left = next;
        }

        bit >>= 1;
        node = next;
    }

    node->value = value;

    return NGX_OK;
}
{% endhighlight %}
本函数用于向radix tree中插入节点，首先查找相应的插入位置，然后再从此位置插入mask中剩余位为1的节点。下面我们给出当```preallocate```为6和5时构造radix tree的过程：


![ngx-radix-insert](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_radix_insert.jpg)

## 3. 函数ngx_radix32tree_delete()
{% highlight string %}
ngx_int_t
ngx_radix32tree_delete(ngx_radix_tree_t *tree, uint32_t key, uint32_t mask)
{
    uint32_t           bit;
    ngx_radix_node_t  *node;

    bit = 0x80000000;
    node = tree->root;

    while (node && (bit & mask)) {
        if (key & bit) {
            node = node->right;

        } else {
            node = node->left;
        }

        bit >>= 1;
    }

    if (node == NULL) {
        return NGX_ERROR;
    }

    if (node->right || node->left) {
        if (node->value != NGX_RADIX_NO_VALUE) {
            node->value = NGX_RADIX_NO_VALUE;
            return NGX_OK;
        }

        return NGX_ERROR;
    }

    for ( ;; ) {
        if (node->parent->right == node) {
            node->parent->right = NULL;

        } else {
            node->parent->left = NULL;
        }

        node->right = tree->free;
        tree->free = node;

        node = node->parent;

        if (node->right || node->left) {
            break;
        }

        if (node->value != NGX_RADIX_NO_VALUE) {
            break;
        }

        if (node->parent == NULL) {
            break;
        }
    }

    return NGX_OK;
}
{% endhighlight %}
此函数用于删除radix tree中的一个节点。下面我们简要介绍一下删除的实现：
{% highlight string %}
ngx_int_t
ngx_radix32tree_delete(ngx_radix_tree_t *tree, uint32_t key, uint32_t mask)
{
	//1) 查询要删除的节点

    //2) 
	//  2.1） 如果该节点仍有孩子节点了，那么直接将该节点的value标志为NGX_RADIX_NO_VALUE
    //  2.2) 否则，从node节点开始向root处回收相应的节点，并将回收后的节点存放到root->free链表的头部	
	//       (注意root->free链是一个单链表，采用right来作为next指针）
}
{% endhighlight %}

## 4. 查找ngx_radix32tree_find()
{% highlight string %}
uintptr_t
ngx_radix32tree_find(ngx_radix_tree_t *tree, uint32_t key)
{
    uint32_t           bit;
    uintptr_t          value;
    ngx_radix_node_t  *node;

    bit = 0x80000000;
    value = NGX_RADIX_NO_VALUE;
    node = tree->root;

    while (node) {
        if (node->value != NGX_RADIX_NO_VALUE) {
            value = node->value;
        }

        if (key & bit) {
            node = node->right;

        } else {
            node = node->left;
        }

        bit >>= 1;
    }

    return value;
}
{% endhighlight %}
这里查找节点较为简单，不详细介绍。

## 5. 函数ngx_radix128tree_insert()
{% highlight string %}
#if (NGX_HAVE_INET6)

ngx_int_t
ngx_radix128tree_insert(ngx_radix_tree_t *tree, u_char *key, u_char *mask,
    uintptr_t value)
{
    u_char             bit;
    ngx_uint_t         i;
    ngx_radix_node_t  *node, *next;

    i = 0;
    bit = 0x80;

    node = tree->root;
    next = tree->root;

    while (bit & mask[i]) {
        if (key[i] & bit) {
            next = node->right;

        } else {
            next = node->left;
        }

        if (next == NULL) {
            break;
        }

        bit >>= 1;
        node = next;

        if (bit == 0) {
            if (++i == 16) {
                break;
            }

            bit = 0x80;
        }
    }

    if (next) {
        if (node->value != NGX_RADIX_NO_VALUE) {
            return NGX_BUSY;
        }

        node->value = value;
        return NGX_OK;
    }

    while (bit & mask[i]) {
        next = ngx_radix_alloc(tree);
        if (next == NULL) {
            return NGX_ERROR;
        }

        next->right = NULL;
        next->left = NULL;
        next->parent = node;
        next->value = NGX_RADIX_NO_VALUE;

        if (key[i] & bit) {
            node->right = next;

        } else {
            node->left = next;
        }

        bit >>= 1;
        node = next;

        if (bit == 0) {
            if (++i == 16) {
                break;
            }

            bit = 0x80;
        }
    }

    node->value = value;

    return NGX_OK;
}
#endif
{% endhighlight %}
与ngx_radix32tree_insert()类似，这里不再赘述。

## 6. 函数ngx_radix128tree_delete()
{% highlight string %}

#if (NGX_HAVE_INET6)
ngx_int_t
ngx_radix128tree_delete(ngx_radix_tree_t *tree, u_char *key, u_char *mask)
{
    u_char             bit;
    ngx_uint_t         i;
    ngx_radix_node_t  *node;

    i = 0;
    bit = 0x80;
    node = tree->root;

    while (node && (bit & mask[i])) {
        if (key[i] & bit) {
            node = node->right;

        } else {
            node = node->left;
        }

        bit >>= 1;

        if (bit == 0) {
            if (++i == 16) {
                break;
            }

            bit = 0x80;
        }
    }

    if (node == NULL) {
        return NGX_ERROR;
    }

    if (node->right || node->left) {
        if (node->value != NGX_RADIX_NO_VALUE) {
            node->value = NGX_RADIX_NO_VALUE;
            return NGX_OK;
        }

        return NGX_ERROR;
    }

    for ( ;; ) {
        if (node->parent->right == node) {
            node->parent->right = NULL;

        } else {
            node->parent->left = NULL;
        }

        node->right = tree->free;
        tree->free = node;

        node = node->parent;

        if (node->right || node->left) {
            break;
        }

        if (node->value != NGX_RADIX_NO_VALUE) {
            break;
        }

        if (node->parent == NULL) {
            break;
        }
    }

    return NGX_OK;
}
#endif
{% endhighlight %}
与ngx_radix32tree_delete()类似，这里不再赘述。

## 7. 函数
{% highlight string %}
#if (NGX_HAVE_INET6)
uintptr_t
ngx_radix128tree_find(ngx_radix_tree_t *tree, u_char *key)
{
    u_char             bit;
    uintptr_t          value;
    ngx_uint_t         i;
    ngx_radix_node_t  *node;

    i = 0;
    bit = 0x80;
    value = NGX_RADIX_NO_VALUE;
    node = tree->root;

    while (node) {
        if (node->value != NGX_RADIX_NO_VALUE) {
            value = node->value;
        }

        if (key[i] & bit) {
            node = node->right;

        } else {
            node = node->left;
        }

        bit >>= 1;

        if (bit == 0) {
            i++;
            bit = 0x80;
        }
    }

    return value;
}
#endif
{% endhighlight %}
与ngx_radix32tree_find()类似，这里不再赘述。


## 8. 函数ngx_radix_alloc()
{% highlight string %}
static ngx_radix_node_t *
ngx_radix_alloc(ngx_radix_tree_t *tree)
{
    ngx_radix_node_t  *p;

    if (tree->free) {
        p = tree->free;
        tree->free = tree->free->right;
        return p;
    }

    if (tree->size < sizeof(ngx_radix_node_t)) {
        tree->start = ngx_pmemalign(tree->pool, ngx_pagesize, ngx_pagesize);
        if (tree->start == NULL) {
            return NULL;
        }

        tree->size = ngx_pagesize;
    }

    p = (ngx_radix_node_t *) tree->start;
    tree->start += sizeof(ngx_radix_node_t);
    tree->size -= sizeof(ngx_radix_node_t);

    return p;
}
{% endhighlight %}
分配一页(page)大小的空间，然后从该空间中分配出一个```ngx_radix_not_t```节点，剩余的空间保存在tree->start中。



<br />
<br />

**[参看]**

1. [基数树结构ngx_radix_tree_t](https://blog.csdn.net/u012819339/article/details/53581203?utm_source=blogxgwz0)

2. [nginx 学习八 高级数据结构之基数树ngx_radix_tree_t](https://www.cnblogs.com/blfshiye/p/5269679.html)


<br />
<br />
<br />

