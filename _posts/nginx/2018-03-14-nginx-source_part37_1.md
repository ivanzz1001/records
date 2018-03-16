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
这是Apache服务器能够识别的一种专用的加密算法，称作Apache apr1加密.

### 2.4 函数ngx_crypt_to64()
{% highlight string %}
static u_char *
ngx_crypt_to64(u_char *p, uint32_t v, size_t n)
{
    static u_char   itoa64[] =
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    while (n--) {
        *p++ = itoa64[v & 0x3f];
        v >>= 6;
    }

    return p;
}
{% endhighlight %}
本函数主要用于将```uint32_t```类型的整数转换成长度为```n```的64进制字串。

### 2.5 函数ngx_crypt_plain()
{% highlight string %}
static ngx_int_t
ngx_crypt_plain(ngx_pool_t *pool, u_char *key, u_char *salt, u_char **encrypted)
{
    size_t   len;
    u_char  *p;

    len = ngx_strlen(key);

    *encrypted = ngx_pnalloc(pool, sizeof("{PLAIN}") - 1 + len + 1);
    if (*encrypted == NULL) {
        return NGX_ERROR;
    }

    p = ngx_cpymem(*encrypted, "{PLAIN}", sizeof("{PLAIN}") - 1);
    ngx_memcpy(p, key, len + 1);

    return NGX_OK;
}
{% endhighlight %}
函数ngx_crypt_plain()主要是再```key```前面加上```{PLAIN}```头。

### 2.6 函数ngx_crypt_ssha()
{% highlight string %}
#if (NGX_HAVE_SHA1)
static ngx_int_t
ngx_crypt_ssha(ngx_pool_t *pool, u_char *key, u_char *salt, u_char **encrypted)
{
    size_t       len;
    ngx_int_t    rc;
    ngx_str_t    encoded, decoded;
    ngx_sha1_t   sha1;

    /* "{SSHA}" base64(SHA1(key salt) salt) */

    /* decode base64 salt to find out true salt */

    encoded.data = salt + sizeof("{SSHA}") - 1;
    encoded.len = ngx_strlen(encoded.data);

    len = ngx_max(ngx_base64_decoded_length(encoded.len), 20);

    decoded.data = ngx_pnalloc(pool, len);
    if (decoded.data == NULL) {
        return NGX_ERROR;
    }

    rc = ngx_decode_base64(&decoded, &encoded);

    if (rc != NGX_OK || decoded.len < 20) {
        decoded.len = 20;
    }

    /* update SHA1 from key and salt */

    ngx_sha1_init(&sha1);
    ngx_sha1_update(&sha1, key, ngx_strlen(key));
    ngx_sha1_update(&sha1, decoded.data + 20, decoded.len - 20);
    ngx_sha1_final(decoded.data, &sha1);

    /* encode it back to base64 */

    len = sizeof("{SSHA}") - 1 + ngx_base64_encoded_length(decoded.len) + 1;

    *encrypted = ngx_pnalloc(pool, len);
    if (*encrypted == NULL) {
        return NGX_ERROR;
    }

    encoded.data = ngx_cpymem(*encrypted, "{SSHA}", sizeof("{SSHA}") - 1);
    ngx_encode_base64(&encoded, &decoded);
    encoded.data[encoded.len] = '\0';

    return NGX_OK;
}
#endif
{% endhighlight %}
当前我们支持```NGX_HAVE_SHA1```宏定义。ssha加密算法的基本步骤是：```base64(SHA1(key salt) salt)```。

### 2.7 函数
{% highlight string %}
#if (NGX_HAVE_SHA1)
static ngx_int_t
ngx_crypt_sha(ngx_pool_t *pool, u_char *key, u_char *salt, u_char **encrypted)
{
    size_t      len;
    ngx_str_t   encoded, decoded;
    ngx_sha1_t  sha1;
    u_char      digest[20];

    /* "{SHA}" base64(SHA1(key)) */

    decoded.len = sizeof(digest);
    decoded.data = digest;

    ngx_sha1_init(&sha1);
    ngx_sha1_update(&sha1, key, ngx_strlen(key));
    ngx_sha1_final(digest, &sha1);

    len = sizeof("{SHA}") - 1 + ngx_base64_encoded_length(decoded.len) + 1;

    *encrypted = ngx_pnalloc(pool, len);
    if (*encrypted == NULL) {
        return NGX_ERROR;
    }

    encoded.data = ngx_cpymem(*encrypted, "{SHA}", sizeof("{SHA}") - 1);
    ngx_encode_base64(&encoded, &decoded);
    encoded.data[encoded.len] = '\0';

    return NGX_OK;
}

#endif /* NGX_HAVE_SHA1 */
{% endhighlight %}
当前我们支持```NGX_HAVE_SHA1```宏定义。nginx中```SHA```加密算法的基本步骤是：```base64(SHA1(key))```。


<br />
<br />
<br />

