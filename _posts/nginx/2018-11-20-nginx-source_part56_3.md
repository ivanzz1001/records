---
layout: post
title: core/ngx_regex.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们主要讲述一下nginx中表达式匹配(ngx_regex)的实现。


<!-- more -->

## 1. 相关静态函数的声明
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>

//对应于nginx配置文件中'pcre_jit'命令
typedef struct {
    ngx_flag_t  pcre_jit;
} ngx_regex_conf_t;


//分配pcre分配内存空间
static void * ngx_libc_cdecl ngx_regex_malloc(size_t size);

//释放pcre申请的空间
static void ngx_libc_cdecl ngx_regex_free(void *p);

//当前我们未定义NGX_HAVE_PCRE_JIT宏，因此不支持'just-in-time compilation'。如果要支持，我们必须在编译时
//加上'--with-pcre-jit'选项
#if (NGX_HAVE_PCRE_JIT)
static void ngx_pcre_free_studies(void *data);
#endif

//初始化nginx regex模块
static ngx_int_t ngx_regex_module_init(ngx_cycle_t *cycle);

//创建对应nginx配置文件regex模块所相关的数据结构
static void *ngx_regex_create_conf(ngx_cycle_t *cycle);

//初始化regex模块相关数据结构
static char *ngx_regex_init_conf(ngx_cycle_t *cycle, void *conf);

//nginx配置文件中'pcre_jit'命令的后置回调函数
static char *ngx_regex_pcre_jit(ngx_conf_t *cf, void *post, void *data);
static ngx_conf_post_t  ngx_regex_pcre_jit_post = { ngx_regex_pcre_jit };
{% endhighlight %}

## 2. 相关静态变量的定义
{% highlight string %}
static ngx_command_t  ngx_regex_commands[] = {

    { ngx_string("pcre_jit"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      0,
      offsetof(ngx_regex_conf_t, pcre_jit),
      &ngx_regex_pcre_jit_post },

      ngx_null_command
};


static ngx_core_module_t  ngx_regex_module_ctx = {
    ngx_string("regex"),
    ngx_regex_create_conf,
    ngx_regex_init_conf
};


ngx_module_t  ngx_regex_module = {
    NGX_MODULE_V1,
    &ngx_regex_module_ctx,                 /* module context */
    ngx_regex_commands,                    /* module directives */
    NGX_CORE_MODULE,                       /* module type */
    NULL,                                  /* init master */
    ngx_regex_module_init,                 /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_pool_t  *ngx_pcre_pool;
static ngx_list_t  *ngx_pcre_studies;
{% endhighlight %}

下面简单介绍一下各变量：

* ngx_regex_commands： 定义了nginx regex模块支持的所有命令

* ngx_regex_module_ctx: regex模块上下文

* ngx_regex_module: 对应的regex模块

* ngx_pcre_pool: 用于保存regex模块中pcre所用的内存池

* ngx_pcre_studies: 保存```ngx_regex_elt_t```结构的链表

## 3. pcre内存管理相关函数
{% highlight string %}
void
ngx_regex_init(void)
{
    pcre_malloc = ngx_regex_malloc;
    pcre_free = ngx_regex_free;
}


static ngx_inline void
ngx_regex_malloc_init(ngx_pool_t *pool)
{
    ngx_pcre_pool = pool;
}


static ngx_inline void
ngx_regex_malloc_done(void)
{
    ngx_pcre_pool = NULL;
}
{% endhighlight %}
这里函数较为简单，只是替换了默认的pcre的内存分配与释放

## 4. 函数ngx_regex_compile()
{% highlight string %}
ngx_int_t
ngx_regex_compile(ngx_regex_compile_t *rc)
{
    int               n, erroff;
    char             *p;
    pcre             *re;
    const char       *errstr;
    ngx_regex_elt_t  *elt;

    ngx_regex_malloc_init(rc->pool);

    re = pcre_compile((const char *) rc->pattern.data, (int) rc->options,
                      &errstr, &erroff, NULL);

    /* ensure that there is no current pool */
    ngx_regex_malloc_done();

    if (re == NULL) {
        if ((size_t) erroff == rc->pattern.len) {
           rc->err.len = ngx_snprintf(rc->err.data, rc->err.len,
                              "pcre_compile() failed: %s in \"%V\"",
                               errstr, &rc->pattern)
                      - rc->err.data;

        } else {
           rc->err.len = ngx_snprintf(rc->err.data, rc->err.len,
                              "pcre_compile() failed: %s in \"%V\" at \"%s\"",
                               errstr, &rc->pattern, rc->pattern.data + erroff)
                      - rc->err.data;
        }

        return NGX_ERROR;
    }

    rc->regex = ngx_pcalloc(rc->pool, sizeof(ngx_regex_t));
    if (rc->regex == NULL) {
        goto nomem;
    }

    rc->regex->code = re;

    /* do not study at runtime */

    if (ngx_pcre_studies != NULL) {
        elt = ngx_list_push(ngx_pcre_studies);
        if (elt == NULL) {
            goto nomem;
        }

        elt->regex = rc->regex;
        elt->name = rc->pattern.data;
    }

    n = pcre_fullinfo(re, NULL, PCRE_INFO_CAPTURECOUNT, &rc->captures);
    if (n < 0) {
        p = "pcre_fullinfo(\"%V\", PCRE_INFO_CAPTURECOUNT) failed: %d";
        goto failed;
    }

    if (rc->captures == 0) {
        return NGX_OK;
    }

    n = pcre_fullinfo(re, NULL, PCRE_INFO_NAMECOUNT, &rc->named_captures);
    if (n < 0) {
        p = "pcre_fullinfo(\"%V\", PCRE_INFO_NAMECOUNT) failed: %d";
        goto failed;
    }

    if (rc->named_captures == 0) {
        return NGX_OK;
    }

    n = pcre_fullinfo(re, NULL, PCRE_INFO_NAMEENTRYSIZE, &rc->name_size);
    if (n < 0) {
        p = "pcre_fullinfo(\"%V\", PCRE_INFO_NAMEENTRYSIZE) failed: %d";
        goto failed;
    }

    n = pcre_fullinfo(re, NULL, PCRE_INFO_NAMETABLE, &rc->names);
    if (n < 0) {
        p = "pcre_fullinfo(\"%V\", PCRE_INFO_NAMETABLE) failed: %d";
        goto failed;
    }

    return NGX_OK;

failed:

    rc->err.len = ngx_snprintf(rc->err.data, rc->err.len, p, &rc->pattern, n)
                  - rc->err.data;
    return NGX_ERROR;

nomem:

    rc->err.len = ngx_snprintf(rc->err.data, rc->err.len,
                               "regex \"%V\" compilation failed: no memory",
                               &rc->pattern)
                  - rc->err.data;
    return NGX_ERROR;
}
{% endhighlight %}
本函数较为简单，基本上是对pcre_compile()函数的封装，下面我们简单介绍一下：
{% highlight string %}
ngx_int_t
ngx_regex_compile(ngx_regex_compile_t *rc)
{
	//1） 初始化pcre结构。因为一般我们都会在nginx启动的时候事先调用ngx_regex_init()，这时候已经将pcre的内部内存分配
	//与释放函数进行了替换，因此这里在调用完pcre_compile()函数后立即调用ngx_regex_malloc_done()
	ngx_regex_malloc_init(rc->pool);
	
	re = pcre_compile((const char *) rc->pattern.data, (int) rc->options,
	                  &errstr, &erroff, NULL);
	
	/* ensure that there is no current pool */
	ngx_regex_malloc_done();

	//2) 如果ngx_pcre_studies不为NULL，那么将re加入到学习链表中

	//2) 调用pcre_fullinfo()获得相应的信息，其实这里主要是验证我们设置的模式pattern是否有效
}
{% endhighlight %}

## 5. 函数ngx_regex_exec_array()
{% highlight string %}
ngx_int_t
ngx_regex_exec_array(ngx_array_t *a, ngx_str_t *s, ngx_log_t *log)
{
    ngx_int_t         n;
    ngx_uint_t        i;
    ngx_regex_elt_t  *re;

    re = a->elts;

    for (i = 0; i < a->nelts; i++) {

        n = ngx_regex_exec(re[i].regex, s, NULL, 0);

        if (n == NGX_REGEX_NO_MATCHED) {
            continue;
        }

        if (n < 0) {
            ngx_log_error(NGX_LOG_ALERT, log, 0,
                          ngx_regex_exec_n " failed: %i on \"%V\" using \"%s\"",
                          n, s, re[i].name);
            return NGX_ERROR;
        }

        /* match */

        return NGX_OK;
    }

    return NGX_DECLINED;
}
{% endhighlight %}
用字符串```s```分别匹配```a```数组中的所有pattern，直到找出一个匹配，或出现相应的错误退出。

## 6. pcre库中涉及到的内存分配与释放
{% highlight string %}
static void * ngx_libc_cdecl
ngx_regex_malloc(size_t size)
{
    ngx_pool_t      *pool;
    pool = ngx_pcre_pool;

    if (pool) {
        return ngx_palloc(pool, size);
    }

    return NULL;
}


static void ngx_libc_cdecl
ngx_regex_free(void *p)
{
    return;
}
{% endhighlight %}

在系统初始化时，会替换掉pcre库中的默认的内存分配与释放函数。

## 7. 函数ngx_pcre_free_studies()
{% highlight string %}
#if (NGX_HAVE_PCRE_JIT)

static void
ngx_pcre_free_studies(void *data)
{
    ngx_list_t *studies = data;

    ngx_uint_t        i;
    ngx_list_part_t  *part;
    ngx_regex_elt_t  *elts;

    part = &studies->part;
    elts = part->elts;

    for (i = 0 ; /* void */ ; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }

            part = part->next;
            elts = part->elts;
            i = 0;
        }

        if (elts[i].regex->extra != NULL) {
            pcre_free_study(elts[i].regex->extra);
        }
    }
}

#endif
{% endhighlight %}
当前我们未定义```NGX_HAVE_PCRE_JIT```宏，所以并不会调用到此函数。本函数的作用是释放```ngx_pcre_studies```链表中的extra数据。

## 8. 函数ngx_regex_module_init()
{% highlight string %}
static ngx_int_t
ngx_regex_module_init(ngx_cycle_t *cycle)
{
    int               opt;
    const char       *errstr;
    ngx_uint_t        i;
    ngx_list_part_t  *part;
    ngx_regex_elt_t  *elts;

    opt = 0;

#if (NGX_HAVE_PCRE_JIT)
    {
    ngx_regex_conf_t    *rcf;
    ngx_pool_cleanup_t  *cln;

    rcf = (ngx_regex_conf_t *) ngx_get_conf(cycle->conf_ctx, ngx_regex_module);

    if (rcf->pcre_jit) {
        opt = PCRE_STUDY_JIT_COMPILE;

        /*
         * The PCRE JIT compiler uses mmap for its executable codes, so we
         * have to explicitly call the pcre_free_study() function to free
         * this memory.
         */

        cln = ngx_pool_cleanup_add(cycle->pool, 0);
        if (cln == NULL) {
            return NGX_ERROR;
        }

        cln->handler = ngx_pcre_free_studies;
        cln->data = ngx_pcre_studies;
    }
    }
#endif

    ngx_regex_malloc_init(cycle->pool);

    part = &ngx_pcre_studies->part;
    elts = part->elts;

    for (i = 0 ; /* void */ ; i++) {

        if (i >= part->nelts) {
            if (part->next == NULL) {
                break;
            }

            part = part->next;
            elts = part->elts;
            i = 0;
        }

        elts[i].regex->extra = pcre_study(elts[i].regex->code, opt, &errstr);

        if (errstr != NULL) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, 0,
                          "pcre_study() failed: %s in \"%s\"",
                          errstr, elts[i].name);
        }

#if (NGX_HAVE_PCRE_JIT)
        if (opt & PCRE_STUDY_JIT_COMPILE) {
            int jit, n;

            jit = 0;
            n = pcre_fullinfo(elts[i].regex->code, elts[i].regex->extra,
                              PCRE_INFO_JIT, &jit);

            if (n != 0 || jit != 1) {
                ngx_log_error(NGX_LOG_INFO, cycle->log, 0,
                              "JIT compiler does not support pattern: \"%s\"",
                              elts[i].name);
            }
        }
#endif
    }

    ngx_regex_malloc_done();

    ngx_pcre_studies = NULL;

    return NGX_OK;
}
{% endhighlight %}

这里用于初始化nginx regex模块。这里我们再说明一下nginx中模块的初始化流程： 首先解析配置文件，调用模块对应的context上下文回调函数，接着再调用```init_module```回调函数。因此，在调用本函数之前，链表```ngx_pcre_studies```就已经建立，然后调用本函数时完成相应pattern的学习。

## 9. 函数ngx_regex_create_conf()
{% highlight string %}
static void *
ngx_regex_create_conf(ngx_cycle_t *cycle)
{
    ngx_regex_conf_t  *rcf;

    rcf = ngx_pcalloc(cycle->pool, sizeof(ngx_regex_conf_t));
    if (rcf == NULL) {
        return NULL;
    }

    rcf->pcre_jit = NGX_CONF_UNSET;

    ngx_pcre_studies = ngx_list_create(cycle->pool, 8, sizeof(ngx_regex_elt_t));
    if (ngx_pcre_studies == NULL) {
        return NULL;
    }

    return rcf;
}
{% endhighlight %}
初始化nginx regex模块上下文的回调函数， 主要是分配相应的空间。

## 10. 函数ngx_regex_init_conf()
{% highlight string %}
static char *
ngx_regex_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_regex_conf_t *rcf = conf;

    ngx_conf_init_value(rcf->pcre_jit, 0);

    return NGX_CONF_OK;
}
{% endhighlight %}
初始化nginx regex模块上下文的回调函数，主要是进行相应数据赋默认值。

## 11. 函数ngx_regex_pcre_jit()
{% highlight string %}
static char *
ngx_regex_pcre_jit(ngx_conf_t *cf, void *post, void *data)
{
    ngx_flag_t  *fp = data;

    if (*fp == 0) {
        return NGX_CONF_OK;
    }

#if (NGX_HAVE_PCRE_JIT)
    {
    int  jit, r;

    jit = 0;
    r = pcre_config(PCRE_CONFIG_JIT, &jit);

    if (r != 0 || jit != 1) {
        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                           "PCRE library does not support JIT");
        *fp = 0;
    }
    }
#else
    ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                       "nginx was built without PCRE JIT support");
    *fp = 0;
#endif

    return NGX_CONF_OK;
}
{% endhighlight %}

在nginx配置文件中，解析到```pcre_jit```命令的回调函数。





<br />
<br />

**[参看]**

1. [Nginx模块开发中使用PCRE正则表达式匹配](https://blog.x-speed.cc/archives/38.html)

2. [nx单独使用pcre的一个小坑](http://dinic.iteye.com/blog/2057150)

3. [深入解析Nginx的pcre库及相关注意事项](https://blog.csdn.net/deltatang/article/details/8754002)


<br />
<br />
<br />

