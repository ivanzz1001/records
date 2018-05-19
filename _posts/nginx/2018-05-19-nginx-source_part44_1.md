---
layout: post
title: core/ngx_md5.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们主要讲述一下nginx中md5相关操作：


<!-- more -->


## 1. core/ngx_md5.h头文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_MD5_H_INCLUDED_
#define _NGX_MD5_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_HAVE_MD5)

#if (NGX_HAVE_OPENSSL_MD5_H)
#include <openssl/md5.h>
#else
#include <md5.h>
#endif


typedef MD5_CTX  ngx_md5_t;


#if (NGX_OPENSSL_MD5)

#define ngx_md5_init    MD5_Init
#define ngx_md5_update  MD5_Update
#define ngx_md5_final   MD5_Final

#else

#define ngx_md5_init    MD5Init
#define ngx_md5_update  MD5Update
#define ngx_md5_final   MD5Final

#endif


#else /* !NGX_HAVE_MD5 */


typedef struct {
    uint64_t  bytes;
    uint32_t  a, b, c, d;
    u_char    buffer[64];
} ngx_md5_t;


void ngx_md5_init(ngx_md5_t *ctx);
void ngx_md5_update(ngx_md5_t *ctx, const void *data, size_t size);
void ngx_md5_final(u_char result[16], ngx_md5_t *ctx);


#endif

#endif /* _NGX_MD5_H_INCLUDED_ */

{% endhighlight %}

### 1.1 相关宏定义

在auto/ngx_auto_config.h头文件中，有如下定义：
<pre>
#ifndef NGX_HAVE_OPENSSL_MD5_H
#define NGX_HAVE_OPENSSL_MD5_H  1
#endif


#ifndef NGX_OPENSSL_MD5
#define NGX_OPENSSL_MD5  1
#endif


#ifndef NGX_HAVE_MD5
#define NGX_HAVE_MD5  1
#endif
</pre>
因此这里我们使用的是openssl的md5。

这里总共有3中md5实现：

* openssl md5

* 系统md5实现

* nginx本身md5实现

### 1.2 md5调用步骤

一般md5调用步骤如下：
<pre>
ngx_md5_init(ngx_md5_t *ctx);


//多次调用update()，对长度为size的data数据更新ctx
ngx_md5_update(ngx_md5_t *ctx, const void *data, size_t size);


//产生16字节长的md5值
ngx_md5_final(u_char result[16], ngx_md5_t *ctx);
</pre>







<br />
<br />
<br />

