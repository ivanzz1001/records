---
layout: post
title: core/ngx_cycle.c源文件分析(2)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本文我们讲述一下Nginx cycle运行上下文相关的实现。


<!-- more -->

 
## 1. 函数ngx_destroy_cycle_pools()
{% highlight string %}
static void
ngx_destroy_cycle_pools(ngx_conf_t *conf)
{
    ngx_destroy_pool(conf->temp_pool);
    ngx_destroy_pool(conf->pool);
}
{% endhighlight %}
这里销毁conf->temp_pool临时内存池与conf->pool.

## 2. 函数ngx_init_zone_pool()
{% highlight string %}
static ngx_int_t
ngx_init_zone_pool(ngx_cycle_t *cycle, ngx_shm_zone_t *zn)
{
    u_char           *file;
    ngx_slab_pool_t  *sp;

    sp = (ngx_slab_pool_t *) zn->shm.addr;

    if (zn->shm.exists) {

        if (sp == sp->addr) {
            return NGX_OK;
        }

#if (NGX_WIN32)

        /* remap at the required address */

        if (ngx_shm_remap(&zn->shm, sp->addr) != NGX_OK) {
            return NGX_ERROR;
        }

        sp = (ngx_slab_pool_t *) zn->shm.addr;

        if (sp == sp->addr) {
            return NGX_OK;
        }

#endif

        ngx_log_error(NGX_LOG_EMERG, cycle->log, 0,
                      "shared zone \"%V\" has no equal addresses: %p vs %p",
                      &zn->shm.name, sp->addr, sp);
        return NGX_ERROR;
    }

    sp->end = zn->shm.addr + zn->shm.size;
    sp->min_shift = 3;
    sp->addr = zn->shm.addr;

#if (NGX_HAVE_ATOMIC_OPS)

    file = NULL;

#else

    file = ngx_pnalloc(cycle->pool, cycle->lock_file.len + zn->shm.name.len);
    if (file == NULL) {
        return NGX_ERROR;
    }

    (void) ngx_sprintf(file, "%V%V%Z", &cycle->lock_file, &zn->shm.name);

#endif

    if (ngx_shmtx_create(&sp->mutex, &sp->lock, file) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_slab_init(sp);

    return NGX_OK;
}
{% endhighlight %}
注意这里```cycle->shared_memory```只适合于大块内存共享，因为这里会用一个相对复杂的结构来管理该共享内存，该结构本身也会占用共享内存的一部分空间。下面画出一个大体示意图：

![ngx-share-memory](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_share_memory.jpg)



<br />
<br />
<br />

