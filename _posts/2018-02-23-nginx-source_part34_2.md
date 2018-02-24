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

本函数主要用于处理通过nginx命令行的```-g```选项传递进来的参数。```-g directives```是用于设置全局配置指令的，例如：
<pre>
# nginx -g "pid /var/run/nginx.pid; worker_processes `sysctl -n hw.ncpu`;"
</pre>

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

通过nginx命令行```-g```选项传递进来的参数保存在```cf->cycle->conf_param```中。函数首先构造一个conf_file对象，用于表示当前的一个配置文件：
{% highlight string %}
typedef struct {
    ngx_file_t            file;      // 当前数据存在于内存，因此此字段暂时不用
    ngx_buf_t            *buffer;    // 指向一个临时的内存
    ngx_buf_t            *dump;      // NULL
    ngx_uint_t            line;      // 0
} ngx_conf_file_t;
{% endhighlight %}

再接着调用```ngx_conf_parse()```当前的配置对象.

<br />

**2) 函数ngx_conf_parse()**
{% highlight string %}
char *
ngx_conf_parse(ngx_conf_t *cf, ngx_str_t *filename)
{
    char             *rv;
    u_char           *p;
    off_t             size;
    ngx_fd_t          fd;
    ngx_int_t         rc;
    ngx_buf_t         buf, *tbuf;
    ngx_conf_file_t  *prev, conf_file;
    ngx_conf_dump_t  *cd;
    enum {
        parse_file = 0,
        parse_block,
        parse_param
    } type;

#if (NGX_SUPPRESS_WARN)
    fd = NGX_INVALID_FILE;
    prev = NULL;
#endif

    if (filename) {

        /* open configuration file */

        fd = ngx_open_file(filename->data, NGX_FILE_RDONLY, NGX_FILE_OPEN, 0);
        if (fd == NGX_INVALID_FILE) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, ngx_errno,
                               ngx_open_file_n " \"%s\" failed",
                               filename->data);
            return NGX_CONF_ERROR;
        }

        prev = cf->conf_file;

        cf->conf_file = &conf_file;

        if (ngx_fd_info(fd, &cf->conf_file->file.info) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_EMERG, cf->log, ngx_errno,
                          ngx_fd_info_n " \"%s\" failed", filename->data);
        }

        cf->conf_file->buffer = &buf;

        buf.start = ngx_alloc(NGX_CONF_BUFFER, cf->log);
        if (buf.start == NULL) {
            goto failed;
        }

        buf.pos = buf.start;
        buf.last = buf.start;
        buf.end = buf.last + NGX_CONF_BUFFER;
        buf.temporary = 1;

        cf->conf_file->file.fd = fd;
        cf->conf_file->file.name.len = filename->len;
        cf->conf_file->file.name.data = filename->data;
        cf->conf_file->file.offset = 0;
        cf->conf_file->file.log = cf->log;
        cf->conf_file->line = 1;

        type = parse_file;

        if (ngx_dump_config
#if (NGX_DEBUG)
            || 1
#endif
           )
        {
            p = ngx_pstrdup(cf->cycle->pool, filename);
            if (p == NULL) {
                goto failed;
            }

            size = ngx_file_size(&cf->conf_file->file.info);

            tbuf = ngx_create_temp_buf(cf->cycle->pool, (size_t) size);
            if (tbuf == NULL) {
                goto failed;
            }

            cd = ngx_array_push(&cf->cycle->config_dump);
            if (cd == NULL) {
                goto failed;
            }

            cd->name.len = filename->len;
            cd->name.data = p;
            cd->buffer = tbuf;

            cf->conf_file->dump = tbuf;

        } else {
            cf->conf_file->dump = NULL;
        }

    } else if (cf->conf_file->file.fd != NGX_INVALID_FILE) {

        type = parse_block;

    } else {
        type = parse_param;
    }


    for ( ;; ) {
        rc = ngx_conf_read_token(cf);

        /*
         * ngx_conf_read_token() may return
         *
         *    NGX_ERROR             there is error
         *    NGX_OK                the token terminated by ";" was found
         *    NGX_CONF_BLOCK_START  the token terminated by "{" was found
         *    NGX_CONF_BLOCK_DONE   the "}" was found
         *    NGX_CONF_FILE_DONE    the configuration file is done
         */

        if (rc == NGX_ERROR) {
            goto done;
        }

        if (rc == NGX_CONF_BLOCK_DONE) {

            if (type != parse_block) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "unexpected \"}\"");
                goto failed;
            }

            goto done;
        }

        if (rc == NGX_CONF_FILE_DONE) {

            if (type == parse_block) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "unexpected end of file, expecting \"}\"");
                goto failed;
            }

            goto done;
        }

        if (rc == NGX_CONF_BLOCK_START) {

            if (type == parse_param) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "block directives are not supported "
                                   "in -g option");
                goto failed;
            }
        }

        /* rc == NGX_OK || rc == NGX_CONF_BLOCK_START */

        if (cf->handler) {

            /*
             * the custom handler, i.e., that is used in the http's
             * "types { ... }" directive
             */

            if (rc == NGX_CONF_BLOCK_START) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "unexpected \"{\"");
                goto failed;
            }

            rv = (*cf->handler)(cf, NULL, cf->handler_conf);
            if (rv == NGX_CONF_OK) {
                continue;
            }

            if (rv == NGX_CONF_ERROR) {
                goto failed;
            }

            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, rv);

            goto failed;
        }


        rc = ngx_conf_handler(cf, rc);

        if (rc == NGX_ERROR) {
            goto failed;
        }
    }

failed:

    rc = NGX_ERROR;

done:

    if (filename) {
        if (cf->conf_file->buffer->start) {
            ngx_free(cf->conf_file->buffer->start);
        }

        if (ngx_close_file(fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, cf->log, ngx_errno,
                          ngx_close_file_n " %s failed",
                          filename->data);
            rc = NGX_ERROR;
        }

        cf->conf_file = prev;
    }

    if (rc == NGX_ERROR) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}
{% endhighlight %}


<br />
<br />
**[参看]**

1. [初识nginx——配置解析篇](https://www.cnblogs.com/magicsoar/p/5817734.html)

2. [Nginx 配置项参数解析](http://blog.csdn.net/zhangxiao93/article/details/52979993)

3. [Nginx开发从入门到精通](http://tengine.taobao.org/book/)

4. [Nginx-------配置文件解析ngx_conf_handler](http://blog.csdn.net/jackywgw/article/details/48786429)

5. [Command-line parameters](http://nginx.org/en/docs/switches.html)
<br />
<br />
<br />

