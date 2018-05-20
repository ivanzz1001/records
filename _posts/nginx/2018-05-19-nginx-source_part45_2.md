---
layout: post
title: core/ngx_module.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要讲述一下nginx module的实现。


<!-- more -->


## 1. 相关函数声明及变量定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Maxim Dounin
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_MAX_DYNAMIC_MODULES  128


static ngx_uint_t ngx_module_index(ngx_cycle_t *cycle);
static ngx_uint_t ngx_module_ctx_index(ngx_cycle_t *cycle, ngx_uint_t type,
    ngx_uint_t index);


ngx_uint_t         ngx_max_module;
static ngx_uint_t  ngx_modules_n;
{% endhighlight %}

上面声明了两个静态函数：

* ```ngx_module_index()```: 用于在```cycle->modules```以及```cycle->old_cycle->modules```中查找一个未被使用的```module->index```

* ```ngx_module_ctx_index```: 用于在```cycle->modules```以及```cycle->old_cycle->modules```中查找一个未被使用的```module->ctx_index```。


接着定义了两个变量：

* ```ngx_max_module```: 用于记录当前nginx可接受的最大的nginx模块数

* ```ngx_modules_n```: 用于记录当前的静态加载模块数，即在编译nginx时静态编译的nginx模块数




## 2. 函数ngx_preinit_modules()
{% highlight string %}
ngx_int_t
ngx_preinit_modules(void)
{
    ngx_uint_t  i;

    for (i = 0; ngx_modules[i]; i++) {
        ngx_modules[i]->index = i;
        ngx_modules[i]->name = ngx_module_names[i];
    }

    ngx_modules_n = i;
    ngx_max_module = ngx_modules_n + NGX_MAX_DYNAMIC_MODULES;

    return NGX_OK;
}

{% endhighlight %}
本函数用于预先初始化全局```ngx_modules```数组元素对应的index以及name， 并统计当前静态编译的模块个数，算出允许的最大模块数。

## 3. 函数ngx_cycle_modules()

{% highlight string %}
ngx_int_t
ngx_cycle_modules(ngx_cycle_t *cycle)
{
    /*
     * create a list of modules to be used for this cycle,
     * copy static modules to it
     */

    cycle->modules = ngx_pcalloc(cycle->pool, (ngx_max_module + 1)
                                              * sizeof(ngx_module_t *));
    if (cycle->modules == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(cycle->modules, ngx_modules,
               ngx_modules_n * sizeof(ngx_module_t *));

    cycle->modules_n = ngx_modules_n;

    return NGX_OK;
}


{% endhighlight %}
此函数初始化指定```cycle```的modules变量。


## 4. 函数ngx_init_modules()
{% highlight string %}
ngx_int_t
ngx_init_modules(ngx_cycle_t *cycle)
{
    ngx_uint_t  i;

    for (i = 0; cycle->modules[i]; i++) {
        if (cycle->modules[i]->init_module) {
            if (cycle->modules[i]->init_module(cycle) != NGX_OK) {
                return NGX_ERROR;
            }
        }
    }

    return NGX_OK;
}

{% endhighlight %}

此函数在```cycle```初始化的后期，即在配置文件处理完成、socket已经建立、共享内存已经建立后调用本函数初始化module。

## 5. 函数ngx_count_modules()
{% highlight string %}
ngx_int_t
ngx_count_modules(ngx_cycle_t *cycle, ngx_uint_t type)
{
    ngx_uint_t     i, next, max;
    ngx_module_t  *module;

    next = 0;
    max = 0;

    /* count appropriate modules, set up their indices */

    for (i = 0; cycle->modules[i]; i++) {
        module = cycle->modules[i];

        if (module->type != type) {
            continue;
        }

        if (module->ctx_index != NGX_MODULE_UNSET_INDEX) {

            /* if ctx_index was assigned, preserve it */

            if (module->ctx_index > max) {
                max = module->ctx_index;
            }

            if (module->ctx_index == next) {
                next++;
            }

            continue;
        }

        /* search for some free index */

        module->ctx_index = ngx_module_ctx_index(cycle, type, next);

        if (module->ctx_index > max) {
            max = module->ctx_index;
        }

        next = module->ctx_index + 1;
    }

    /*
     * make sure the number returned is big enough for previous
     * cycle as well, else there will be problems if the number
     * will be stored in a global variable (as it's used to be)
     * and we'll have to roll back to the previous cycle
     */

    if (cycle->old_cycle && cycle->old_cycle->modules) {

        for (i = 0; cycle->old_cycle->modules[i]; i++) {
            module = cycle->old_cycle->modules[i];

            if (module->type != type) {
                continue;
            }

            if (module->ctx_index > max) {
                max = module->ctx_index;
            }
        }
    }

    /* prevent loading of additional modules */

    cycle->modules_used = 1;

    return max + 1;
}


{% endhighlight %}

这里我们首先讲述一下nginx的```cycle```回滚（rollback)： 在nginx调用ngx_init_cycle()出现较为严重的错误而失败时，会回滚到前一个已经初始化过的old_cycle状态， 这样保证系统仍能够正常的运行。接下来我们简要介绍一下本函数的实现：

{% highlight string %}
ngx_int_t
ngx_count_modules(ngx_cycle_t *cycle, ngx_uint_t type)
{
    
    for (i = 0; cycle->modules[i]; i++) 
    {
       //1) 找出当前最大的ctx_index，保存在max变量中

       //2) 调用ngx_module_ctx_index()函数找出一个当前未被使用的index作为当前module的ctx_index
       module->ctx_index = ngx_module_ctx_index(cycle, type, next);

    }

    //3） 确保max值大于old_cycle->modules中对应type的最大ctx_index
    // 关于这一点，我们在这里说明一下： 因为ngx_count_modules()函数的返回值会被保存到一个
    // 全局变量中(请参看本函数调用）， 因此如果在调用ngx_init_cycle()出现失败时需要回滚，
    // 此时只会对cycle变量进行回滚，并不会对全局变量进行回滚，在此情况下就要求当前该全局变量所
    // 表示空间必须能够容纳其原来所表示的空间

   
    //4) 返回max+1
    // 最后一个位置作为结尾使用
}
{% endhighlight %}


<br />
<br />

**[参看]**

1. [nginx-module-t数据结构](https://blog.csdn.net/u014082714/article/details/46125135)

<br />
<br />
<br />

