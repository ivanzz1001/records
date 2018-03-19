---
layout: post
title: core/ngx_cycle.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们讲述一下nginx运行的一个总控型数据结构及相关操作函数。

<!-- more -->


## 1. 相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2
{% endhighlight %}

下面我们对上面几个宏定义进行简单的说明：

* **NGX_CYCLE_POOL_SIZE**: 这里定义nginx cycle所关联的pool大小，默认值```NGX_DEFAULT_POOL_SIZE```，即16KB（该变量定义在core/palloc.h头文件中)

* **NGX_DEBUG_POINTS_STOP**: 定义程序在执行到一些关键错误点时，产生```SIGSTOP```信号。

* **NGX_DEBUG_POINTS_ABORT**: 定义程序在执行到一些关键错误点时，执行abort()函数。

<pre>
可以通过在nginx.conf配置文件使用debug_points指令，指定在一些关键错误点处的行为。
</pre>


## 2. ngx_shm_zone_t数据结构
{% highlight string %}
typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};
{% endhighlight %}
```ngx_shm_zone_s```代表一块内存共享区域。下面我们对其中各个字段做一个简要的说明：

* **data**: 这里```data```可以指向自定义的一个数据结构，主要是为了在数据初始化的时候用到，或通过共享内存直接拿到与共享内存相关的数据，它不一定指向共享内存中的地址。

* **shm**: 实际的共享内存结构

* **init**: 共享内存初始化函数

* **tag**: 这里tag是一个标志，区别于shm.name。 shm.name没法让nginx区分到底是想新创建一个共享内存，还是使用已存在的旧的共享内存，因此这里引入tag字段来解决该问题。tag一般指向当前模块的```ngx_module_t```变量，例如：
<pre>
ngx_shared_memory_add(cf, &value[1], 0, &ngx_http_fastcgi_module);
</pre>

这里再对tag字段解释一下，因为看上去它和name字段有点重复，而事实上，name字段主要用作共享内存的唯一标识，它能让nginx知道我想使用哪个共享内存，但它没办法让nginx区分到底是想新创建一个共享内存，还是使用那个已存在的旧的共享内存。举个例子，模块A创建了共享内存sa,模块A或者另外一个模块B再以同样的名称sa去获取共享内存，那么此时nginx是返回模块A已创建的那个共享内存sa给模块A/模块B，还是直接以共享内存名重复提示模块A/模块B出错呢？ 不管nginx采用哪种做法都有另外一种情况出错，所以新增一个tag字段做冲突标识，该字段一般指向当前模块的 ```ngx_module_t```变量即可。这样在上面的例子中，通过tag字段的帮助，如果模块A/模块B再以同样的名称去获取模块A已创建的共享内存sa，则模块A将获得它之前创建的共享内存的引用（因为模块A前后两次请求的tag相同），而模块B将获得共享内存已作他用的错误提示（因为模块B请求的tag与之前模块A请求的tag不同）。

* **noreuse**: 取值为0时，则表示可以对此共享内存进行复用；否则不能对此共享内存进行复用。一般用在系统升级时，表示是否可以复用前面创建的共享内存。

## 3. ngx_cycle_s数据结构
{% highlight string %}
struct ngx_cycle_s {
    void                  ****conf_ctx;
    ngx_pool_t               *pool;

    ngx_log_t                *log;
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    ngx_connection_t        **files;
    ngx_connection_t         *free_connections;
    ngx_uint_t                free_connection_n;

    ngx_module_t            **modules;
    ngx_uint_t                modules_n;
    ngx_uint_t                modules_used;    /* unsigned  modules_used:1; */

    ngx_queue_t               reusable_connections_queue;

    ngx_array_t               listening;
    ngx_array_t               paths;
    ngx_array_t               config_dump;
    ngx_list_t                open_files;
    ngx_list_t                shared_memory;

    ngx_uint_t                connection_n;
    ngx_uint_t                files_n;

    ngx_connection_t         *connections;
    ngx_event_t              *read_events;
    ngx_event_t              *write_events;

    ngx_cycle_t              *old_cycle;

    ngx_str_t                 conf_file;
    ngx_str_t                 conf_param;
    ngx_str_t                 conf_prefix;
    ngx_str_t                 prefix;
    ngx_str_t                 lock_file;
    ngx_str_t                 hostname;
};
{% endhighlight %}
```ngx_cycle_s```是一个总控型数据结构。一个```cycle```对象存放着从某个配置创建而来的nginx运行时上下文。我们可以通过全局变量```ngx_cycle```来引用到当前进程的cycle上下文（对于worker进程，在创建时也会继承得到该上下文）。每一次重新加载nginx配置文件时，都会从该配置文件重新创建出一个新的cycle对象；而原来老的cycle对象则会在新的cycle成功创建之后被删除掉。

一个cycle对象通常是由```ngx_init_cycle()```函数所创建，该函数以一个```previous cycle```作为其参数。函数首先会定位到```previous cycle```的配置文件，然后尽可能的从previous cycle继承相应的资源。在nginx启动时，首先会创建一个```init_cycle```占位符，然后会用一个从配置文件创建而来的cycle来替换该```init_cycle```。

下面我们再针对各个字段，做一个简要的介绍：

* **conf_ctx**: 本字段是一个4级指针结构，实际使用时可能当做2级指针来用，也可能当做4级指针来用。下面给出一个```conf_ctx```指针的一个大体结构：
![ngx-conf-ctx](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_conf_ctx.jpg)

* **pool**: 本cycle所关联的内存池对象。针对每一个新的cycle对象，都会创建一个相应的内存池对象。

* **log**: 本cycle所关联的日志对象。初始时，会从原来老的cycle继承而来；而当配置文件成功读取完成之后，```log```指针会指向```new_log```。

* **new_log**: 本cycle所关联的日志对象，在读取完配置文件时进行创建。其受到nginx配置文件一级配置(root-scope)error_log指令的影响。

* **log_use_stderr**: 主要用于指示当前配置文件中error_log指令的日志输出是否配置为stderr。```error_log```指令用法如下：
<pre>
Syntax:	error_log file [level];
Default:	error_log logs/error.log error;
Context:	main, http, mail, stream, server, location
</pre>


<br />
<br />

1. [nginx共享内存：共享内存的实现](http://blog.csdn.net/wgwgnihao/article/details/37838837)

2. [Nginx内存管理及数据结构浅析–共享内存的实现](http://www.colaghost.net/web-server/246)

3. [nginx之共享内存](http://blog.csdn.net/evsqiezi/article/details/51785093)

4. [Nginx Cycle](http://nginx.org/en/docs/dev/development_guide.html#cycle)

5. [Nginx源码分析： 3张图看懂启动及进程工作原理](http://www.360doc.com/content/16/0220/10/30291625_535903478.shtml)

6. [nginx学习十 ngx_cycle_t 、ngx_connection_t 和ngx_listening_t](http://blog.csdn.net/xiaoliangsky/article/details/39831035)

7. [ngx_cycle_s](http://blog.csdn.net/yzt33/article/details/47087943)
<br />
<br />
<br />

