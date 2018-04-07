---
layout: post
title: core/ngx_hash.c源文件分析
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

## 1. 函数ngx_hash_find()
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


void *
ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len)
{
    ngx_uint_t       i;
    ngx_hash_elt_t  *elt;

#if 0
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "hf:\"%*s\"", len, name);
#endif

    elt = hash->buckets[key % hash->size];

    if (elt == NULL) {
        return NULL;
    }

    while (elt->value) {
        if (len != (size_t) elt->len) {
            goto next;
        }

        for (i = 0; i < len; i++) {
            if (name[i] != elt->name[i]) {
                goto next;
            }
        }

        return elt->value;

    next:

        elt = (ngx_hash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len,
                                               sizeof(void *));
        continue;
    }

    return NULL;
}
{% endhighlight %}

这里函数实现较为简单，我们主要看一下ngx_align_ptr宏，该宏的core/ngx_config.h头文件中：
<pre>
#define ngx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
</pre>
主要是实现```a```字节对齐，这里是进行```sizeof(void *)```字节对齐。





<br />
<br />


<br />
<br />
<br />

