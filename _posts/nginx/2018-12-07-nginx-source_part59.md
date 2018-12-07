---
layout: post
title: core/ngx_sha1.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章主要是对操作系统或```openssl```中的sha1安全哈希算法通过宏定义进行了包装。



<!-- more -->

## 1. ngx_sha1.h头文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SHA1_H_INCLUDED_
#define _NGX_SHA1_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_HAVE_OPENSSL_SHA1_H)
#include <openssl/sha.h>
#else
#include <sha.h>
#endif


typedef SHA_CTX  ngx_sha1_t;


#define ngx_sha1_init    SHA1_Init
#define ngx_sha1_update  SHA1_Update
#define ngx_sha1_final   SHA1_Final


#endif /* _NGX_SHA1_H_INCLUDED_ */

{% endhighlight %}
可以看到这里只是对操作系统或```openssl```的sha1安全哈希算法通过宏定义的方式进行了重命名。当前我们支持如下宏定义：
<pre>
#ifndef NGX_HAVE_OPENSSL_SHA1_H
#define NGX_HAVE_OPENSSL_SHA1_H  1
#endif
</pre>
因此这里默认采用openssl的```sha1```算法。


<br />
<br />

**[参看]**





<br />
<br />
<br />

