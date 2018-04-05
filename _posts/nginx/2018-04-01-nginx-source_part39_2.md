---
layout: post
title: core/ngx_file.c源文件分析(1)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx中对所涉及到的文件操作。


<!-- more -->

## 1. 相关变量定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


static ngx_int_t ngx_test_full_name(ngx_str_t *name);


static ngx_atomic_t   temp_number = 0;
ngx_atomic_t         *ngx_temp_number = &temp_number;
ngx_atomic_int_t      ngx_random_number = 123456;
{% endhighlight %}

* 函数```ngx_test_full_name()```用于判断```name```是否为一个绝对路径。

* 变量```temp_number```作为一个临时的占位符空间

* 变量```ngx_temp_number```刚开始指向```temp_number```,而后在初始化event模块时（ngx_event_module_init()函数)，指向一个共享内存空间，在整个Nginx中作为一个临时值使用

* 变量ngx_random_number作为随机值使用，默认初始化值为123456，而在event模块初始化时，被设置为：
{% highlight string %}
ngx_random_number = (tp->msec << 16) + ngx_pid;
{% endhighlight %}



## 2. 函数ngx_get_full_name()
{% highlight string %}
ngx_int_t
ngx_get_full_name(ngx_pool_t *pool, ngx_str_t *prefix, ngx_str_t *name)
{
    size_t      len;
    u_char     *p, *n;
    ngx_int_t   rc;

    rc = ngx_test_full_name(name);

    if (rc == NGX_OK) {
        return rc;
    }

    len = prefix->len;

#if (NGX_WIN32)

    if (rc == 2) {
        len = rc;
    }

#endif

    n = ngx_pnalloc(pool, len + name->len + 1);
    if (n == NULL) {
        return NGX_ERROR;
    }

    p = ngx_cpymem(n, prefix->data, len);
    ngx_cpystrn(p, name->data, name->len + 1);

    name->len += len;
    name->data = n;

    return NGX_OK;
}
{% endhighlight %}
这里先判断```name```是否是一个绝对路径，如果是则直接返回OK；否则开辟一块空间，将```name```追加到```prefix```后（注意，这里会以'\0'结尾)


## 3. 函数ngx_test_full_name()
{% highlight string %}
static ngx_int_t
ngx_test_full_name(ngx_str_t *name)
{
#if (NGX_WIN32)
    u_char  c0, c1;

    c0 = name->data[0];

    if (name->len < 2) {
        if (c0 == '/') {
            return 2;
        }

        return NGX_DECLINED;
    }

    c1 = name->data[1];

    if (c1 == ':') {
        c0 |= 0x20;

        if ((c0 >= 'a' && c0 <= 'z')) {
            return NGX_OK;
        }

        return NGX_DECLINED;
    }

    if (c1 == '/') {
        return NGX_OK;
    }

    if (c0 == '/') {
        return 2;
    }

    return NGX_DECLINED;

#else

    if (name->data[0] == '/') {
        return NGX_OK;
    }

    return NGX_DECLINED;

#endif
}
{% endhighlight %}
在WIN32上判断是否是一个绝对路径，例如```C:```，又或者是以类似于```<charactor>/```这样开头的路径； 对于在其他操作系统环境上则直接判断是否为绝对路径。在当前环境下，我们并不支持```NGX_WIN32```宏定义。

## 4. 函数ngx_write_chain_to_temp_file()
{% highlight string %}
ssize_t
ngx_write_chain_to_temp_file(ngx_temp_file_t *tf, ngx_chain_t *chain)
{
    ngx_int_t  rc;

    if (tf->file.fd == NGX_INVALID_FILE) {
        rc = ngx_create_temp_file(&tf->file, tf->path, tf->pool,
                                  tf->persistent, tf->clean, tf->access);

        if (rc != NGX_OK) {
            return rc;
        }

        if (tf->log_level) {
            ngx_log_error(tf->log_level, tf->file.log, 0, "%s %V",
                          tf->warn, &tf->file.name);
        }
    }

#if (NGX_THREADS && NGX_HAVE_PWRITEV)

    if (tf->thread_write) {
        return ngx_thread_write_chain_to_file(&tf->file, chain, tf->offset,
                                              tf->pool);
    }

#endif

    return ngx_write_chain_to_file(&tf->file, chain, tf->offset, tf->pool);
}
{% endhighlight %}


<br />
<br />

**[参考]**

1. [nginx文件结构](https://blog.csdn.net/apelife/article/details/53043275)

2. [Nginx中目录树的遍历](https://blog.csdn.net/weiyuefei/article/details/38313663)

<br />
<br />
<br />

