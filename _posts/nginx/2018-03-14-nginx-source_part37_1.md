---
layout: post
title: core/ngx_crypt.c(h)源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们主要讲述一下nginx中加密的处理。

<!-- more -->


## 1. core/ngx_crypt.h源文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CRYPT_H_INCLUDED_
#define _NGX_CRYPT_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


ngx_int_t ngx_crypt(ngx_pool_t *pool, u_char *key, u_char *salt,
    u_char **encrypted);


#endif /* _NGX_CRYPT_H_INCLUDED_ */
{% endhighlight %}

本头文件主要是ngx_crypt()函数的声明。

## 2. core/ngx_crypt.c源文件

本文件主要是实现ngx_crypt()功能：

### 2.1 相关函数声明
{% highlight string %}

/*
 * Copyright (C) Maxim Dounin
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_crypt.h>
#include <ngx_md5.h>
#if (NGX_HAVE_SHA1)
#include <ngx_sha1.h>
#endif


#if (NGX_CRYPT)

static ngx_int_t ngx_crypt_apr1(ngx_pool_t *pool, u_char *key, u_char *salt,
    u_char **encrypted);
static ngx_int_t ngx_crypt_plain(ngx_pool_t *pool, u_char *key, u_char *salt,
    u_char **encrypted);

#if (NGX_HAVE_SHA1)

static ngx_int_t ngx_crypt_ssha(ngx_pool_t *pool, u_char *key, u_char *salt,
    u_char **encrypted);
static ngx_int_t ngx_crypt_sha(ngx_pool_t *pool, u_char *key, u_char *salt,
    u_char **encrypted);

#endif


static u_char *ngx_crypt_to64(u_char *p, uint32_t v, size_t n);
{% endhighlight %}
在objs/ngx_auto_config.h头文件中，我们有如下定义：
<pre>
#ifndef NGX_HAVE_SHA1
#define NGX_HAVE_SHA1  1
#endif

#ifndef NGX_CRYPT
#define NGX_CRYPT  1
#endif
</pre>
关于各个函数的大体作用，我们后面会进行说明。

### 2.2 函数ngx_crypt()
{% highlight string %}
ngx_int_t
ngx_crypt(ngx_pool_t *pool, u_char *key, u_char *salt, u_char **encrypted)
{
    if (ngx_strncmp(salt, "$apr1$", sizeof("$apr1$") - 1) == 0) {
        return ngx_crypt_apr1(pool, key, salt, encrypted);

    } else if (ngx_strncmp(salt, "{PLAIN}", sizeof("{PLAIN}") - 1) == 0) {
        return ngx_crypt_plain(pool, key, salt, encrypted);

#if (NGX_HAVE_SHA1)
    } else if (ngx_strncmp(salt, "{SSHA}", sizeof("{SSHA}") - 1) == 0) {
        return ngx_crypt_ssha(pool, key, salt, encrypted);

    } else if (ngx_strncmp(salt, "{SHA}", sizeof("{SHA}") - 1) == 0) {
        return ngx_crypt_sha(pool, key, salt, encrypted);
#endif
    }

    /* fallback to libc crypt() */

    return ngx_libc_crypt(pool, key, salt, encrypted);
}

{% endhighlight %}
ngx_crypt()函数主要是根据```salt```值选择合适的加密算法对```key```进行加密，加密后的结果存放在```encrypted```参数中，并通过函数返回值返回加密后子串的长度。

* 当salt以```$apr1$```开头时： 采用ngx_crypt_apr1()进行加密

* 当salt以```{PLAIN}```开头时: 采用ngx_crypt_plain()进行加密

* 当salt以```{SSHA}```开头时: 采用ngx_crypt_ssha()进行加密

* 当salt以```{SHA}```开头时： 采用ngx_crypt_sha()进行加密

* 默认情况下，采用ngx_libc_crypt()进行加密

### 2.3 函数ngx_crypt_apr1()
{% highlight string %}
static ngx_int_t
ngx_crypt_apr1(ngx_pool_t *pool, u_char *key, u_char *salt, u_char **encrypted)
{
    ngx_int_t          n;
    ngx_uint_t         i;
    u_char            *p, *last, final[16];
    size_t             saltlen, keylen;
    ngx_md5_t          md5, ctx1;

    /* Apache's apr1 crypt is Poul-Henning Kamp's md5 crypt with $apr1$ magic */

    keylen = ngx_strlen(key);

    /* true salt: no magic, max 8 chars, stop at first $ */

    salt += sizeof("$apr1$") - 1;
    last = salt + 8;
    for (p = salt; *p && *p != '$' && p < last; p++) { /* void */ }
    saltlen = p - salt;

    /* hash key and salt */

    ngx_md5_init(&md5);
    ngx_md5_update(&md5, key, keylen);
    ngx_md5_update(&md5, (u_char *) "$apr1$", sizeof("$apr1$") - 1);
    ngx_md5_update(&md5, salt, saltlen);

    ngx_md5_init(&ctx1);
    ngx_md5_update(&ctx1, key, keylen);
    ngx_md5_update(&ctx1, salt, saltlen);
    ngx_md5_update(&ctx1, key, keylen);
    ngx_md5_final(final, &ctx1);

    for (n = keylen; n > 0; n -= 16) {
        ngx_md5_update(&md5, final, n > 16 ? 16 : n);
    }

    ngx_memzero(final, sizeof(final));

    for (i = keylen; i; i >>= 1) {
        if (i & 1) {
            ngx_md5_update(&md5, final, 1);

        } else {
            ngx_md5_update(&md5, key, 1);
        }
    }

    ngx_md5_final(final, &md5);

    for (i = 0; i < 1000; i++) {
        ngx_md5_init(&ctx1);

        if (i & 1) {
            ngx_md5_update(&ctx1, key, keylen);

        } else {
            ngx_md5_update(&ctx1, final, 16);
        }

        if (i % 3) {
            ngx_md5_update(&ctx1, salt, saltlen);
        }

        if (i % 7) {
            ngx_md5_update(&ctx1, key, keylen);
        }

        if (i & 1) {
            ngx_md5_update(&ctx1, final, 16);

        } else {
            ngx_md5_update(&ctx1, key, keylen);
        }

        ngx_md5_final(final, &ctx1);
    }

    /* output */

    *encrypted = ngx_pnalloc(pool, sizeof("$apr1$") - 1 + saltlen + 1 + 22 + 1);
    if (*encrypted == NULL) {
        return NGX_ERROR;
    }

    p = ngx_cpymem(*encrypted, "$apr1$", sizeof("$apr1$") - 1);
    p = ngx_copy(p, salt, saltlen);
    *p++ = '$';

    p = ngx_crypt_to64(p, (final[ 0]<<16) | (final[ 6]<<8) | final[12], 4);
    p = ngx_crypt_to64(p, (final[ 1]<<16) | (final[ 7]<<8) | final[13], 4);
    p = ngx_crypt_to64(p, (final[ 2]<<16) | (final[ 8]<<8) | final[14], 4);
    p = ngx_crypt_to64(p, (final[ 3]<<16) | (final[ 9]<<8) | final[15], 4);
    p = ngx_crypt_to64(p, (final[ 4]<<16) | (final[10]<<8) | final[ 5], 4);
    p = ngx_crypt_to64(p, final[11], 2);
    *p = '\0';

    return NGX_OK;
}
{% endhighlight %}



<br />
<br />
<br />

