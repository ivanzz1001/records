---
layout: post
title: core/ngx_open_file_cache.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要讲述一下nginx对静态文件的缓存相关操作。


<!-- more -->


## 1. nginx静态文件缓存
这里在介绍具体的源代码之前，我们首先简要介绍一下Nginx服务器中静态文件Cache的一个大体处理流程。事实上，整个nginx中只有两个地方会用到这种静态文件缓存：

* ngx_http_core_module

* ngx_http_log_module

nginx静态文件缓存中，可以存储如下信息：

* 打开的文件描述符、文件大小、修改时间

* 目录是否存在等信息

* 与文件查找相关的任何错误消息，例如```file not found```、```no read permission```
<pre>
注意： 如果要缓存错误信息的话，那么还需要单独启用 'open_file_cache_errors' 子令
</pre>


其基本使用示例如下：
<pre>
open_file_cache max=64 inactive=30d;
open_file_cache_min_uses 8;
open_file_cache_valid 3m;
open_file_cache_errors   on;
</pre>
上面 **'max=64'** 表示设置缓冲文件的最大数目为64。超过此数目后，Nginx将按照LRU原则丢弃冷数据。**'inactive=30d'** 与 **'open_file_cache_min_uses 8'** 表示如果在30天内某文件的访问次数低于8次，那就将它从缓存中删除。

**'open_file_cache_valid 3m'**表示每3分钟检查一次缓存中的文件信息是否正确，如果不是则更新之。


### 1.1 原理介绍



### 1.2 文件缓存更新周期
上面提到的配置文件中，若文件在30天内访问的次数低于8次，那么将会从缓存中丢弃。每3分钟做一次信息有效性监测，这里我们暂且把```3分钟```称为```缓存更新周期```。那在这3分钟之内文件发生变化了会怎样呢？

1） **文件被删除**

由于nginx还持有原文件的fd，所以你删除此文件后，文件并不会真正消失，client还是能够通过原路径访问此文件。即便你删除后又新建了一个同名文件，在当前缓存更新周期内能访问到的还是原文件的内容。

2） **文件内容被修改**

文件内容被修改可以分成两种情况：

* 文件大小不变或增大： 由于nginx缓存了文件的size，并且使用缓存中的size调用sendfile(2)，所以此种情况的后果是：
<pre>
1. 从文件开始到原size字节中的变化可以被client看到；

2. 原size之后的内容不会被sendfile(2)发送，因此client看不到此部分内容；
</pre>


* 文件大小减小： 此种情况下，由于同样的原因，nginx在HTTP Header中告诉client文件大小还是原来的尺寸，而sendfile(2)只能发送真正的文件数据，长度小于HTTP Header中设置的大小，所以client会等待到自己超时或者Nginx在epoll_wait超时后关闭连接。


### 1.3 如何设置Nginx静态缓存
我们在使用Nginx静态文件缓存时，可以考虑如下：

* 如果你的静态文件内容变化频繁并且对时效性要求较高，一般应该把 **'open_file_cache_valid'** 设置的小一些，以便及时检测和更新；

* 如果变化相当不频繁的话， 就可以设置大一点。在变化后用reload nginx的方式来强制更新缓存；

* 对静态文件访问的error和access log不关心的话，可以关闭以提升效率。






## 2. 相关静态函数声明
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


/*
 * open file cache caches
 *    open file handles with stat() info;
 *    directories stat() info;
 *    files and directories errors: not found, access denied, etc.
 */


#define NGX_MIN_READ_AHEAD  (128 * 1024)


static void ngx_open_file_cache_cleanup(void *data);
#if (NGX_HAVE_OPENAT)
static ngx_fd_t ngx_openat_file_owner(ngx_fd_t at_fd, const u_char *name,
    ngx_int_t mode, ngx_int_t create, ngx_int_t access, ngx_log_t *log);
#if (NGX_HAVE_O_PATH)
static ngx_int_t ngx_file_o_path_info(ngx_fd_t fd, ngx_file_info_t *fi,
    ngx_log_t *log);
#endif
#endif
static ngx_fd_t ngx_open_file_wrapper(ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_int_t mode, ngx_int_t create,
    ngx_int_t access, ngx_log_t *log);
static ngx_int_t ngx_file_info_wrapper(ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_file_info_t *fi, ngx_log_t *log);
static ngx_int_t ngx_open_and_stat_file(ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_log_t *log);
static void ngx_open_file_add_event(ngx_open_file_cache_t *cache,
    ngx_cached_open_file_t *file, ngx_open_file_info_t *of, ngx_log_t *log);
static void ngx_open_file_cleanup(void *data);
static void ngx_close_cached_file(ngx_open_file_cache_t *cache,
    ngx_cached_open_file_t *file, ngx_uint_t min_uses, ngx_log_t *log);
static void ngx_open_file_del_event(ngx_cached_open_file_t *file);
static void ngx_expire_old_cached_files(ngx_open_file_cache_t *cache,
    ngx_uint_t n, ngx_log_t *log);
static void ngx_open_file_cache_rbtree_insert_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel);
static ngx_cached_open_file_t *
    ngx_open_file_lookup(ngx_open_file_cache_t *cache, ngx_str_t *name,
    uint32_t hash);
static void ngx_open_file_cache_remove(ngx_event_t *ev);
{% endhighlight %}

这里nginx对打开文件的缓存，主要有三种类型：

1) 打开文件的句柄，以及该文件对应额stat()信息

2） 目录的stat()信息；

3） 文件和目录的一些错误信息： not found、拒绝访问等等；








<br />
<br />

**[参看]**

1. [Nginx服务器中静态文件cache的处理流程](https://blog.csdn.net/otion20122/article/details/73332449)

2. [使用Nginx缓存静态文件](https://blog.csdn.net/f529352479/article/details/68484280)

3. [Nginx Open File Cache](https://www.cnblogs.com/cmfwm/p/7659179.html)

4. [Nginx使用教程(五)：使用Nginx缓存之缓存静态内容](https://www.cnblogs.com/felixzh/p/6283896.html)

5. [open_file_cache](http://nginx.org/en/docs/http/ngx_http_core_module.html#open_file_cache)

<br />
<br />
<br />

