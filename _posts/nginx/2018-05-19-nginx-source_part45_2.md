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



## 6. 函数ngx_add_module()
{% highlight string %}
ngx_int_t
ngx_add_module(ngx_conf_t *cf, ngx_str_t *file, ngx_module_t *module,
    char **order)
{
    void               *rv;
    ngx_uint_t          i, m, before;
    ngx_core_module_t  *core_module;

    if (cf->cycle->modules_n >= ngx_max_module) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "too many modules loaded");
        return NGX_ERROR;
    }

    if (module->version != nginx_version) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "module \"%V\" version %ui instead of %ui",
                           file, module->version, (ngx_uint_t) nginx_version);
        return NGX_ERROR;
    }

    if (ngx_strcmp(module->signature, NGX_MODULE_SIGNATURE) != 0) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "module \"%V\" is not binary compatible",
                           file);
        return NGX_ERROR;
    }

    for (m = 0; cf->cycle->modules[m]; m++) {
        if (ngx_strcmp(cf->cycle->modules[m]->name, module->name) == 0) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "module \"%s\" is already loaded",
                               module->name);
            return NGX_ERROR;
        }
    }

    /*
     * if the module wasn't previously loaded, assign an index
     */

    if (module->index == NGX_MODULE_UNSET_INDEX) {
        module->index = ngx_module_index(cf->cycle);

        if (module->index >= ngx_max_module) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "too many modules loaded");
            return NGX_ERROR;
        }
    }

    /*
     * put the module into the cycle->modules array
     */

    before = cf->cycle->modules_n;

    if (order) {
        for (i = 0; order[i]; i++) {
            if (ngx_strcmp(order[i], module->name) == 0) {
                i++;
                break;
            }
        }

        for ( /* void */ ; order[i]; i++) {

#if 0
            ngx_log_debug2(NGX_LOG_DEBUG_CORE, cf->log, 0,
                           "module: %s before %s",
                           module->name, order[i]);
#endif

            for (m = 0; m < before; m++) {
                if (ngx_strcmp(cf->cycle->modules[m]->name, order[i]) == 0) {

                    ngx_log_debug3(NGX_LOG_DEBUG_CORE, cf->log, 0,
                                   "module: %s before %s:%i",
                                   module->name, order[i], m);

                    before = m;
                    break;
                }
            }
        }
    }

    /* put the module before modules[before] */

    if (before != cf->cycle->modules_n) {
        ngx_memmove(&cf->cycle->modules[before + 1],
                    &cf->cycle->modules[before],
                    (cf->cycle->modules_n - before) * sizeof(ngx_module_t *));
    }

    cf->cycle->modules[before] = module;
    cf->cycle->modules_n++;

    if (module->type == NGX_CORE_MODULE) {

        /*
         * we are smart enough to initialize core modules;
         * other modules are expected to be loaded before
         * initialization - e.g., http modules must be loaded
         * before http{} block
         */

        core_module = module->ctx;

        if (core_module->create_conf) {
            rv = core_module->create_conf(cf->cycle);
            if (rv == NULL) {
                return NGX_ERROR;
            }

            cf->cycle->conf_ctx[module->index] = rv;
        }
    }

    return NGX_OK;
}

{% endhighlight %}

本函数用于处理```load_module```指令， 添加一个动态模块。下面我们简要分析一下本函数:
{% highlight string %}
ngx_int_t
ngx_add_module(ngx_conf_t *cf, ngx_str_t *file, ngx_module_t *module,
    char **order)
{
     //1) 进行相应的检查： 是否已经超过支持的最大模块数、版本是否合规、特征signature是否匹配、
     // 是否已经存在该模块

    //2) 为该新添加的module指定index


    //3) 将当前module插入到cycle->modules数组的适当位置， 具体方法如下：

        //3.1) 找出当前module在order数组中的位置，这里假设位置为order[i]， 则order[i+1]之后的模块都应该在当前要插入模块的后边

        //3.2) 遍历order[i+1]之后的所有模块，找出其应该在cycle->modules数组中应该存放的位置

        //3.3 找出存放位置之后，将该位置起的所有模块都往后移动以腾出空间存放当前模块

    
    //4) 如果是core模块的话，则会调用该模块的module->ctx->create_conf来初始化相应的上下文对象

}
{% endhighlight %}


## 7: 函数ngx_module_index()
{% highlight string %}
static ngx_uint_t
ngx_module_index(ngx_cycle_t *cycle)
{
    ngx_uint_t     i, index;
    ngx_module_t  *module;

    index = 0;

again:

    /* find an unused index */

    for (i = 0; cycle->modules[i]; i++) {
        module = cycle->modules[i];

        if (module->index == index) {
            index++;
            goto again;
        }
    }

    /* check previous cycle */

    if (cycle->old_cycle && cycle->old_cycle->modules) {

        for (i = 0; cycle->old_cycle->modules[i]; i++) {
            module = cycle->old_cycle->modules[i];

            if (module->index == index) {
                index++;
                goto again;
            }
        }
    }

    return index;
}

{% endhighlight %}

本函数较为简单，从```cycle->modules```以及```cycle->old_cycle->modules```数组中挑选出一个最小的未被使用的Index。这里之所以还要在```cycle->old_cycle->modules```未被使用，主要是因为用到了一些与modules相关的全局变量。

## 8. 函数ngx_module_ctx_index()
{% highlight string %}
static ngx_uint_t
ngx_module_ctx_index(ngx_cycle_t *cycle, ngx_uint_t type, ngx_uint_t index)
{
    ngx_uint_t     i;
    ngx_module_t  *module;

again:

    /* find an unused ctx_index */

    for (i = 0; cycle->modules[i]; i++) {
        module = cycle->modules[i];

        if (module->type != type) {
            continue;
        }

        if (module->ctx_index == index) {
            index++;
            goto again;
        }
    }

    /* check previous cycle */

    if (cycle->old_cycle && cycle->old_cycle->modules) {

        for (i = 0; cycle->old_cycle->modules[i]; i++) {
            module = cycle->old_cycle->modules[i];

            if (module->type != type) {
                continue;
            }

            if (module->ctx_index == index) {
                index++;
                goto again;
            }
        }
    }

    return index;
}
{% endhighlight %}
本函数较为简单，从```cycle->modules```以及```cycle->old_cycle->modules```数组中挑选出一个最小的未被使用的ctx_index。这里之所以还要在```cycle->old_cycle->modules```未被使用，主要是因为用到了一些与modules相关的全局变量。

<br />
<br />

**[参看]**

1. [nginx-module-t数据结构](https://blog.csdn.net/u014082714/article/details/46125135)

<br />
<br />
<br />

