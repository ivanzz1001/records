---
layout: post
title: core/ngx_conf_file.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们讲述nginx配置文件相关的一些内容。


<!-- more -->

## 1. 相关静态函数声明

{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>

#define NGX_CONF_BUFFER  4096

// 对指令的解析校验等方面的处理
static ngx_int_t ngx_conf_handler(ngx_conf_t *cf, ngx_int_t last);

// 读取配置文件并进行词法分析
static ngx_int_t ngx_conf_read_token(ngx_conf_t *cf);

// 主要用于nginx配置模块在退出时刷新相应的打开文件
static void ngx_conf_flush_files(ngx_cycle_t *cycle);
{% endhighlight %}

## 2. 相关变量定义

**1) nginx配置模块支持的指令**
{% highlight string %}
static ngx_command_t  ngx_conf_commands[] = {

    { ngx_string("include"),
      NGX_ANY_CONF|NGX_CONF_TAKE1,
      ngx_conf_include,
      0,
      0,
      NULL },

      ngx_null_command
};
{% endhighlight %}

这里nginx配置模块(conf module)当前只支持一个```include```指令。其携带一个参数，可以放置于nginx配置文件的任意位置。在解析```include```指令时，通过调用```ngx_conf_include()```来完成。```include```指令语法如下：
{% highlight string %}
include file | mask
{% endhighlight %}
通过```include```指令将```file```文件，或者满足```mask```匹配的文件包含到配置文件中来。所包含进来的文件必须满足nginx定义的相关语法。例如：
<pre>
include mime.types;

include vhosts/*.conf
</pre>

<br />

**2) nginx配置模块**
{% highlight string %}
ngx_module_t  ngx_conf_module = {
    NGX_MODULE_V1,
    NULL,                                  /* module context */
    ngx_conf_commands,                     /* module directives */
    NGX_CONF_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    ngx_conf_flush_files,                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};
{% endhighlight %}

这里```ngx_conf_module```并不需要任何模块上下文```module context```，所支持的模块指令为```ngx_conf_commands```，模块类型为```NGX_CONF_MODULE```，在进程退出时的回调函数为```ngx_conf_flush_files```。

<br />

**3) 配置指令参数数组**
{% highlight string %}
/* The eight fixed arguments */

static ngx_uint_t argument_number[] = {
    NGX_CONF_NOARGS,
    NGX_CONF_TAKE1,
    NGX_CONF_TAKE2,
    NGX_CONF_TAKE3,
    NGX_CONF_TAKE4,
    NGX_CONF_TAKE5,
    NGX_CONF_TAKE6,
    NGX_CONF_TAKE7
};
{% endhighlight %}

## 3. 配置解析相关函数实现

**1) 函数ngx_conf_param()**
{% highlight string %}
char *
ngx_conf_param(ngx_conf_t *cf)
{
    char             *rv;
    ngx_str_t        *param;
    ngx_buf_t         b;
    ngx_conf_file_t   conf_file;

    param = &cf->cycle->conf_param;

    if (param->len == 0) {
        return NGX_CONF_OK;
    }

    ngx_memzero(&conf_file, sizeof(ngx_conf_file_t));

    ngx_memzero(&b, sizeof(ngx_buf_t));

    b.start = param->data;
    b.pos = param->data;
    b.last = param->data + param->len;
    b.end = b.last;
    b.temporary = 1;

    conf_file.file.fd = NGX_INVALID_FILE;
    conf_file.file.name.data = NULL;
    conf_file.line = 0;

    cf->conf_file = &conf_file;
    cf->conf_file->buffer = &b;

    rv = ngx_conf_parse(cf, NULL);

    cf->conf_file = NULL;

    return rv;
}
{% endhighlight %}



<br />
<br />
**[参看]**

1. [初识nginx——配置解析篇](https://www.cnblogs.com/magicsoar/p/5817734.html)

2. [Nginx 配置项参数解析](http://blog.csdn.net/zhangxiao93/article/details/52979993)

3. [Nginx开发从入门到精通](http://tengine.taobao.org/book/)

4. [Nginx-------配置文件解析ngx_conf_handler](http://blog.csdn.net/jackywgw/article/details/48786429)
<br />
<br />
<br />

