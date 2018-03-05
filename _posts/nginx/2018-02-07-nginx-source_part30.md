---
layout: post
title: os/unix/ngx_user.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要讲述一下Linux中自带的crypt()函数，nginx采用该函数来实现自己的加密函数。


<!-- more -->

## 1. crypt()函数介绍
{% highlight string %}
#define _XOPEN_SOURCE       /* See feature_test_macros(7) */
#include <unistd.h>

char *crypt(const char *key, const char *salt);

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <crypt.h>

char *crypt_r(const char *key, const char *salt,
             struct crypt_data *data);

Link with -lcrypt.
{% endhighlight %}
crypt()函数是一个密码加密函数。它是基于DES加密算法的一种变体来实现加密的。加密后返回一个具有13个可打印字符的字符串。

参数```key```为用户输入的明文密码（最多只会处理前8个字节。如果超出部分被忽略）

参数```salt```是一个两个字节的字串（字符取自[a-zA-Z0-9./]，长度超过2的部分被忽略)，用于扰乱该加密算法。

```crypt_r()```函数是crypt()函数的可重入版本，参数crypt_data只是用于存储加密后的数据，其内部并不会分配空间。在调用前，需要将crypt_data->initialized置为0.

**返回值**： 函数成功时返回加密后的字符串；否则返回NULL，并设置相应的错误码

<pre>
注意： 一般启用_GNU_SOURCE后，就自动的启用了_XOPEN_SOURCE，因此我们可以看到，在程序中我们并没有再去显示的开启_XOPEN_SOURCE
      这个宏定义。
</pre>

参看如下示例test2.c:
{% highlight string %}
#define _GNU_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "%s salt key\n", argv[0]);
        return 1;
    }
    char *encrypted = NULL;

    if((encrypted = crypt(argv[2], argv[1])) == NULL)
    {
        fprintf(stderr, "crypt error:%s\n", strerror(errno));
    }

    printf("%s encrypted:%s\n", argv[2], encrypted);

    return 0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test2 test2.c -lcrypt

# ./test2 aa world66677
world66677 encrypted:aaElDg5KDwBQg

# ./test2 aa world66688
world66688 encrypted:aaElDg5KDwBQg

# ./test2 aabb world66688
world66688 encrypted:aaElDg5KDwBQg

# ./test2 "\$6\$y9cP0qlmDYgBk6OZ\$" world6666          //这里也支持其他算法，请参看相应文档（这里反斜杠为转义作用)
world6666 encrypted:$6$y9cP0qlmDYgBk6OZ$9Jnxb/9sqKnr4hofUO1y0T4ireWiQPuj37QYjEahjn1oRIAkWUNcuTBnipQNF6.O5U1YPMmS.1FqA7JphoEnY0
</pre>

## 2. os/unix/ngx_user.h头文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_USER_H_INCLUDED_
#define _NGX_USER_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef uid_t  ngx_uid_t;
typedef gid_t  ngx_gid_t;


ngx_int_t ngx_libc_crypt(ngx_pool_t *pool, u_char *key, u_char *salt,
    u_char **encrypted);


#endif /* _NGX_USER_H_INCLUDED_ */
{% endhighlight %}

这里只是声明ngx_libc_crypt()函数。

## 3. os/unix/ngx_user.c源文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * Solaris has thread-safe crypt()
 * Linux has crypt_r(); "struct crypt_data" is more than 128K
 * FreeBSD needs the mutex to protect crypt()
 *
 * TODO:
 *     ngx_crypt_init() to init mutex
 */


#if (NGX_CRYPT)

#if (NGX_HAVE_GNU_CRYPT_R)

ngx_int_t
ngx_libc_crypt(ngx_pool_t *pool, u_char *key, u_char *salt, u_char **encrypted)
{
    char               *value;
    size_t              len;
    struct crypt_data   cd;

    cd.initialized = 0;
#ifdef __GLIBC__
    /* work around the glibc bug */
    cd.current_salt[0] = ~salt[0];
#endif

    value = crypt_r((char *) key, (char *) salt, &cd);

    if (value) {
        len = ngx_strlen(value) + 1;

        *encrypted = ngx_pnalloc(pool, len);
        if (*encrypted == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(*encrypted, value, len);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_CRIT, pool->log, ngx_errno, "crypt_r() failed");

    return NGX_ERROR;
}

#else

ngx_int_t
ngx_libc_crypt(ngx_pool_t *pool, u_char *key, u_char *salt, u_char **encrypted)
{
    char       *value;
    size_t      len;
    ngx_err_t   err;

    value = crypt((char *) key, (char *) salt);

    if (value) {
        len = ngx_strlen(value) + 1;

        *encrypted = ngx_pnalloc(pool, len);
        if (*encrypted == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(*encrypted, value, len);
        return NGX_OK;
    }

    err = ngx_errno;

    ngx_log_error(NGX_LOG_CRIT, pool->log, err, "crypt() failed");

    return NGX_ERROR;
}

#endif

#endif /* NGX_CRYPT */
{% endhighlight %}

在ngx_auto_config.h头文件中我们有如下定义：
<pre>
#ifndef NGX_CRYPT
#define NGX_CRYPT  1
#endif

#ifndef NGX_HAVE_GNU_CRYPT_R
#define NGX_HAVE_GNU_CRYPT_R  1
#endif
</pre>


<br />
<br />

1. [linux crypt函数](http://blog.csdn.net/liuxingen/article/details/46673305)

2. [关于Linux下的crypt加密](http://blog.csdn.net/yjp19871013/article/details/8425356)


<br />
<br />
<br />

