---
layout: post
title: core/ngx_module.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要讲述一下nginx module的实现。


<!-- more -->


## 1. 当前nginx的signature
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Maxim Dounin
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_MODULE_H_INCLUDED_
#define _NGX_MODULE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <nginx.h>


#define NGX_MODULE_UNSET_INDEX  (ngx_uint_t) -1


#define NGX_MODULE_SIGNATURE_0                                                \
    ngx_value(NGX_PTR_SIZE) ","                                               \
    ngx_value(NGX_SIG_ATOMIC_T_SIZE) ","                                      \
    ngx_value(NGX_TIME_T_SIZE) ","

#if (NGX_HAVE_KQUEUE)
#define NGX_MODULE_SIGNATURE_1   "1"
#else
#define NGX_MODULE_SIGNATURE_1   "0"
#endif

#if (NGX_HAVE_IOCP)
#define NGX_MODULE_SIGNATURE_2   "1"
#else
#define NGX_MODULE_SIGNATURE_2   "0"
#endif

#if (NGX_HAVE_FILE_AIO)
#define NGX_MODULE_SIGNATURE_3   "1"
#else
#define NGX_MODULE_SIGNATURE_3   "0"
#endif

#if (NGX_HAVE_AIO_SENDFILE)
#define NGX_MODULE_SIGNATURE_4   "1"
#else
#define NGX_MODULE_SIGNATURE_4   "0"
#endif

#if (NGX_HAVE_EVENTFD)
#define NGX_MODULE_SIGNATURE_5   "1"
#else
#define NGX_MODULE_SIGNATURE_5   "0"
#endif

#if (NGX_HAVE_EPOLL)
#define NGX_MODULE_SIGNATURE_6   "1"
#else
#define NGX_MODULE_SIGNATURE_6   "0"
#endif

#if (NGX_HAVE_KEEPALIVE_TUNABLE)
#define NGX_MODULE_SIGNATURE_7   "1"
#else
#define NGX_MODULE_SIGNATURE_7   "0"
#endif

#if (NGX_HAVE_INET6)
#define NGX_MODULE_SIGNATURE_8   "1"
#else
#define NGX_MODULE_SIGNATURE_8   "0"
#endif

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
#define NGX_MODULE_SIGNATURE_9   "1"
#else
#define NGX_MODULE_SIGNATURE_9   "0"
#endif

#if (NGX_HAVE_REUSEPORT)
#define NGX_MODULE_SIGNATURE_10  "1"
#else
#define NGX_MODULE_SIGNATURE_10  "0"
#endif

#if (NGX_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
#define NGX_MODULE_SIGNATURE_11  "1"
#else
#define NGX_MODULE_SIGNATURE_11  "0"
#endif

#if (NGX_HAVE_DEFERRED_ACCEPT && defined TCP_DEFER_ACCEPT)
#define NGX_MODULE_SIGNATURE_12  "1"
#else
#define NGX_MODULE_SIGNATURE_12  "0"
#endif

#if (NGX_HAVE_SETFIB)
#define NGX_MODULE_SIGNATURE_13  "1"
#else
#define NGX_MODULE_SIGNATURE_13  "0"
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
#define NGX_MODULE_SIGNATURE_14  "1"
#else
#define NGX_MODULE_SIGNATURE_14  "0"
#endif

#if (NGX_HAVE_UNIX_DOMAIN)
#define NGX_MODULE_SIGNATURE_15  "1"
#else
#define NGX_MODULE_SIGNATURE_15  "0"
#endif

#if (NGX_HAVE_VARIADIC_MACROS)
#define NGX_MODULE_SIGNATURE_16  "1"
#else
#define NGX_MODULE_SIGNATURE_16  "0"
#endif

#if (NGX_HAVE_MD5)
#define NGX_MODULE_SIGNATURE_17  "1"
#else
#define NGX_MODULE_SIGNATURE_17  "0"
#endif

#if (NGX_HAVE_SHA1)
#define NGX_MODULE_SIGNATURE_18  "1"
#else
#define NGX_MODULE_SIGNATURE_18  "0"
#endif

#if (NGX_HAVE_OPENAT)
#define NGX_MODULE_SIGNATURE_19  "1"
#else
#define NGX_MODULE_SIGNATURE_19  "0"
#endif

#if (NGX_HAVE_ATOMIC_OPS)
#define NGX_MODULE_SIGNATURE_20  "1"
#else
#define NGX_MODULE_SIGNATURE_20  "0"
#endif

#if (NGX_HAVE_POSIX_SEM)
#define NGX_MODULE_SIGNATURE_21  "1"
#else
#define NGX_MODULE_SIGNATURE_21  "0"
#endif

#if (NGX_THREADS)
#define NGX_MODULE_SIGNATURE_22  "1"
#else
#define NGX_MODULE_SIGNATURE_22  "0"
#endif

#if (NGX_PCRE)
#define NGX_MODULE_SIGNATURE_23  "1"
#else
#define NGX_MODULE_SIGNATURE_23  "0"
#endif

#if (NGX_HTTP_SSL)
#define NGX_MODULE_SIGNATURE_24  "1"
#else
#define NGX_MODULE_SIGNATURE_24  "0"
#endif

#if (NGX_HTTP_V2)
#define NGX_MODULE_SIGNATURE_25  "1"
#else
#define NGX_MODULE_SIGNATURE_25  "0"
#endif

#if (NGX_HTTP_GZIP)
#define NGX_MODULE_SIGNATURE_26  "1"
#else
#define NGX_MODULE_SIGNATURE_26  "0"
#endif

#if (NGX_HTTP_DEGRADATION)
#define NGX_MODULE_SIGNATURE_27  "1"
#else
#define NGX_MODULE_SIGNATURE_27  "0"
#endif

#if (NGX_HTTP_X_FORWARDED_FOR)
#define NGX_MODULE_SIGNATURE_28  "1"
#else
#define NGX_MODULE_SIGNATURE_28  "0"
#endif

#if (NGX_HTTP_REALIP)
#define NGX_MODULE_SIGNATURE_29  "1"
#else
#define NGX_MODULE_SIGNATURE_29  "0"
#endif

#if (NGX_HTTP_HEADERS)
#define NGX_MODULE_SIGNATURE_30  "1"
#else
#define NGX_MODULE_SIGNATURE_30  "0"
#endif

#if (NGX_HTTP_DAV)
#define NGX_MODULE_SIGNATURE_31  "1"
#else
#define NGX_MODULE_SIGNATURE_31  "0"
#endif

#if (NGX_HTTP_CACHE)
#define NGX_MODULE_SIGNATURE_32  "1"
#else
#define NGX_MODULE_SIGNATURE_32  "0"
#endif

#if (NGX_HTTP_UPSTREAM_ZONE)
#define NGX_MODULE_SIGNATURE_33  "1"
#else
#define NGX_MODULE_SIGNATURE_33  "0"
#endif

#define NGX_MODULE_SIGNATURE                                                  \
    NGX_MODULE_SIGNATURE_0 NGX_MODULE_SIGNATURE_1 NGX_MODULE_SIGNATURE_2      \
    NGX_MODULE_SIGNATURE_3 NGX_MODULE_SIGNATURE_4 NGX_MODULE_SIGNATURE_5      \
    NGX_MODULE_SIGNATURE_6 NGX_MODULE_SIGNATURE_7 NGX_MODULE_SIGNATURE_8      \
    NGX_MODULE_SIGNATURE_9 NGX_MODULE_SIGNATURE_10 NGX_MODULE_SIGNATURE_11    \
    NGX_MODULE_SIGNATURE_12 NGX_MODULE_SIGNATURE_13 NGX_MODULE_SIGNATURE_14   \
    NGX_MODULE_SIGNATURE_15 NGX_MODULE_SIGNATURE_16 NGX_MODULE_SIGNATURE_17   \
    NGX_MODULE_SIGNATURE_18 NGX_MODULE_SIGNATURE_19 NGX_MODULE_SIGNATURE_20   \
    NGX_MODULE_SIGNATURE_21 NGX_MODULE_SIGNATURE_22 NGX_MODULE_SIGNATURE_23   \
    NGX_MODULE_SIGNATURE_24 NGX_MODULE_SIGNATURE_25 NGX_MODULE_SIGNATURE_26   \
    NGX_MODULE_SIGNATURE_27 NGX_MODULE_SIGNATURE_28 NGX_MODULE_SIGNATURE_29   \
    NGX_MODULE_SIGNATURE_30 NGX_MODULE_SIGNATURE_31 NGX_MODULE_SIGNATURE_32   \
    NGX_MODULE_SIGNATURE_33

{% endhighlight %}
当前nginx模块共有34个signature，动态添加进入的模块(通过```load_module```指令添加）的signature必须和这里的signature一致，否则会添加失败。

首先```NGX_MODULE_SIGNATURE_0```是最基本的signature，基本反映了当前nginx可执行文件所匹配的机器字长、原子整形数据字长以及```time_t```数据类型长度。 当前这些长度在我们32为Ubuntu系统上都是4。

剩余的一些特性我们这里并不详细介绍。


## 2. nginx module相关宏定义
{% highlight string %}
#define NGX_MODULE_V1                                                         \
    NGX_MODULE_UNSET_INDEX, NGX_MODULE_UNSET_INDEX,                           \
    NULL, 0, 0, nginx_version, NGX_MODULE_SIGNATURE

#define NGX_MODULE_V1_PADDING  0, 0, 0, 0, 0, 0, 0, 0
{% endhighlight %}
这里我们简要介绍一下这两个宏定义：

* 宏```NGX_MODULE_V1```： 用于初始化一个module

* 宏```NGX_MODULE_V1_PADDING```: 作为一个module中后面几个字段的填充使用


## 3. ngx_module_s数据结构
{% highlight string %}
struct ngx_module_s {
    ngx_uint_t            ctx_index;
    ngx_uint_t            index;

    char                 *name;

    ngx_uint_t            spare0;
    ngx_uint_t            spare1;

    ngx_uint_t            version;
    const char           *signature;

    void                 *ctx;
    ngx_command_t        *commands;
    ngx_uint_t            type;

    ngx_int_t           (*init_master)(ngx_log_t *log);

    ngx_int_t           (*init_module)(ngx_cycle_t *cycle);

    ngx_int_t           (*init_process)(ngx_cycle_t *cycle);
    ngx_int_t           (*init_thread)(ngx_cycle_t *cycle);
    void                (*exit_thread)(ngx_cycle_t *cycle);
    void                (*exit_process)(ngx_cycle_t *cycle);

    void                (*exit_master)(ngx_cycle_t *cycle);

    uintptr_t             spare_hook0;
    uintptr_t             spare_hook1;
    uintptr_t             spare_hook2;
    uintptr_t             spare_hook3;
    uintptr_t             spare_hook4;
    uintptr_t             spare_hook5;
    uintptr_t             spare_hook6;
    uintptr_t             spare_hook7;
};

{% endhighlight %}
```ngx_module_s```数据结构是对Nginx 模块的一个抽象。下面我们简要介绍一下各字段的含义：

* ```ctx_index```: 分类的模块计数器。nginx模块可以分为四种： core、event、http和mail，每个模块都会有各自计数,ctx_index就是每个模块在其所属类组的计数。

* ```index```: 用于指定本模块在```ngx_modules```数组中的索引值

* ```name```: 用于指示模块的名称

* ```spare0/spare1```: 暂时保留，不做使用

* ```version```: 用于指定版本号，当前值为```1010003```，即```1.10.3```版本

* ```commands```: 用于指定该模块的指令集

* ```type```: 用于指定模块的类型

* ```init_master```: 初始化master时执行的回调函数，当前在系统中并未有任何地方用到

* ```init_module```: 在对module执行初始化时执行, 当前会在```ngx_init_modules()```函数调用时用到

* ```init_process```: 在子进程初始化时，会调用module的init_process回调函数

* ```init_thread```: 在线程初始化时，执行本回调函数， 当前在系统中并未有任何地方用到

* ```exit_thread```: 在线程退出时，执行本回调函数，当前在系统中并未有任何地方用到

* ```exit_process```: 在子进程退出时，会执行本回调函数

* ```exit_master```: 在master进程退出时，会执行本回调函数。当前在系统ngx_master_process_exit()函数中会被调用。

* ```spare_hook0/1/2/3/4/5/6/7```: 其他一些hook函数，当前在系统中暂未被使用到

## 4. ngx_core_module_t数据结构
{% highlight string %}
typedef struct {
    ngx_str_t             name;
    void               *(*create_conf)(ngx_cycle_t *cycle);
    char               *(*init_conf)(ngx_cycle_t *cycle, void *conf);
} ngx_core_module_t;
{% endhighlight %}
此数据结构作为核心模块的一个context类型，即```ngx_module_s.ctx```。对于core、event、http和mail， 其都属于核心模块，各自都拥有一个context对象。下面介绍一下本数据结构的各字段：

* ```name```: context名称。对于core模块，本字段取值一般为```core```，对于event模块，本字段取值一般为```events```。

* ```create_conf```: 一般在系统初始化调用本回调函数创建相应的上下文结构。例如在整个系统初始化时，会分别调用core、event、http、mail等核心模块的create_conf函数； 在解析到```events```指令时, 又会调用本函数创建整个events模块的上下文结构。

* ```init_conf```: 一般在解析完成相应的指令后，调用本回调函数对一些配置中未设置的变量完成相应的初始化



## 5. 相关函数声明
{% highlight string %}

//1) 预先对module进行适当的初始化
ngx_int_t ngx_preinit_modules(void);

//2) 为cycle->modules变量分配相应的空间
ngx_int_t ngx_cycle_modules(ngx_cycle_t *cycle);


//3) 初始化cycle->modules, 即回调ngx_module_s.init_module()函数
ngx_int_t ngx_init_modules(ngx_cycle_t *cycle);


//4) 用于计算某一种类的module的个数, 并位模块指定ctx_index值
ngx_int_t ngx_count_modules(ngx_cycle_t *cycle, ngx_uint_t type);

//5) 在执行到load_module指令时，会调用本函数添加一个模块
ngx_int_t ngx_add_module(ngx_conf_t *cf, ngx_str_t *file,
    ngx_module_t *module, char **order);

{% endhighlight %}


## 6. 相关变量声明
{% highlight string %}
extern ngx_module_t  *ngx_modules[];
extern ngx_uint_t     ngx_max_module;

extern char          *ngx_module_names[];
{% endhighlight %}
下面我们简要介绍一下各变量：

* ```ngx_modules```: 全局变量，在nginx编译时将当前所有编译到的模块添加到该数组中

* ```ngx_max_module```: 当前最大模块数，包括编译时所有的静态模块数以及后续可添加的最大动态模块数。

* ```ngx_module_names```: 全局变量，在Nginx编译时将当前所有编译到的模块的名称存放到该数组中


<br />
<br />

**[参看]**

1. [nginx-module-t数据结构](https://blog.csdn.net/u014082714/article/details/46125135)

<br />
<br />
<br />

