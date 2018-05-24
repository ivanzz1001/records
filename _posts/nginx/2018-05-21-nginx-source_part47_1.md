---
layout: post
title: core/ngx_open_file_cache.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要讲述一下nginx对静态文件的缓存相关操作。


<!-- more -->


## 1. directio_off宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef _NGX_OPEN_FILE_CACHE_H_INCLUDED_
#define _NGX_OPEN_FILE_CACHE_H_INCLUDED_


#define NGX_OPEN_FILE_DIRECTIO_OFF  NGX_MAX_OFF_T_VALUE

{% endhighlight %}

在objs/ngx_auto_config.h头文件中，有如下定义：
<pre>
#ifndef NGX_MAX_OFF_T_VALUE
#define NGX_MAX_OFF_T_VALUE  9223372036854775807LL
#endif
</pre>
这里定义```NGX_OPEN_FILE_DIRECTIO_OFF```为long long的最大值，一般文件大小都不会超过此值，因此可以将此值作为```directio_off```来用。




## 2. ngx_open_file_info_t数据结构
{% highlight string %}
typedef struct {
    ngx_fd_t                 fd;                     //该文件所对应的文件句柄
    ngx_file_uniq_t          uniq;                   //该文件所对应的全局唯一标识， 一般取值为inode节点号
    time_t                   mtime;                  //该文件的最后修改时间
    off_t                    size;                   //该文件的大小
    off_t                    fs_size;                //文件所占用的硬盘层面块数block * 512
    off_t                    directio;               //文件对应的directio大小值， 小于该值采用sendfile来发送， 大于该值采用aio来发送
    size_t                   read_ahead;             //预先由内核读取的字节数

    ngx_err_t                err;                    //该打开的文件所关联的错误信息码
    char                    *failed;                 //所关联的错误信息

    time_t                   valid;                  //文件的有效时间

    ngx_uint_t               min_uses;               //

#if (NGX_HAVE_OPENAT)
    size_t                   disable_symlinks_from;
    unsigned                 disable_symlinks:2;
#endif

    unsigned                 test_dir:1;
    unsigned                 test_only:1;
    unsigned                 log:1;
    unsigned                 errors:1;
    unsigned                 events:1;

    unsigned                 is_dir:1;
    unsigned                 is_file:1;
    unsigned                 is_link:1;
    unsigned                 is_exec:1;
    unsigned                 is_directio:1;
} ngx_open_file_info_t;
{% endhighlight %}







<br />
<br />

**[参看]**

1. [第二章 OpenResty(Nginx+Lua)开发入门](http://jinnianshilongnian.iteye.com/blog/2186448)

2. [nginx open_file_cache指令影响静态文件更新时间](https://www.cnblogs.com/sunsweet/p/3338684.html)

<br />
<br />
<br />

