---
layout: post
title: 编写第一个nginx模块
tags:
- nginx
categories: nginx
description: nginx使用基础
---


本章来编写我们的第一个nginx模块。当前环境为：
<pre>
# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core)
</pre>


<!-- more -->

## 1. 编写hello_world模块

在上一节中，我们编译了一个```Hello World```第三方模块，在这一章我们只是自己手动来写一下，以便加深对这一过程的理解。这里我们将模块名称指定为: ngx_http_hello_world_module


### 1.1 编写模块源代码

编写ngx_http_hello_world_module.c文件：
{% highlight string %}
/**
 * @file   ngx_http_hello_world_module.c
 * @author António P. P. Almeida <appa@perusio.net>
 * @date   Wed Aug 17 12:06:52 2011
 *
 * @brief  A hello world module for Nginx.
 *
 * @section LICENSE
 *
 * Copyright (C) 2011 by Dominic Fallows, António P. P. Almeida <appa@perusio.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#define HELLO_WORLD "hello world\r\n"

static char *ngx_http_hello_world(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r);

/**
 * This module provided directive: hello world.
 *
 */
static ngx_command_t ngx_http_hello_world_commands[] = {

    { ngx_string("hello_world"), /* directive */
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS, /* location context and takes
                                            no arguments*/
      ngx_http_hello_world, /* configuration setup function */
      0, /* No offset. Only one context is supported. */
      0, /* No offset when storing the module configuration on struct. */
      NULL},

    ngx_null_command /* command termination */
};

/* The hello world string. */
static u_char ngx_hello_world[] = HELLO_WORLD;

/* The module context. */
static ngx_http_module_t ngx_http_hello_world_module_ctx = {
    NULL, /* preconfiguration */
    NULL, /* postconfiguration */

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    NULL, /* create location configuration */
    NULL /* merge location configuration */
};

/* Module definition. */
ngx_module_t ngx_http_hello_world_module = {
    NGX_MODULE_V1,
    &ngx_http_hello_world_module_ctx, /* module context */
    ngx_http_hello_world_commands, /* module directives */
    NGX_HTTP_MODULE, /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};

/**
 * Content handler.
 *
 * @param r
 *   Pointer to the request structure. See http_request.h.
 * @return
 *   The status of the response generation.
 */
static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r)
{
    ngx_buf_t *b;
    ngx_chain_t out;

    /* Set the Content-Type header. */
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *) "text/plain";

    /* Allocate a new buffer for sending out the reply. */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    /* Insertion in the buffer chain. */
    out.buf = b;
    out.next = NULL; /* just one buffer */

    b->pos = ngx_hello_world; /* first position in memory of the data */
    b->last = ngx_hello_world + sizeof(ngx_hello_world) - 1; /* last position in memory of the data */
    b->memory = 1; /* content is in read-only memory */
    b->last_buf = 1; /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = sizeof(ngx_hello_world) - 1;
    ngx_http_send_header(r); /* Send the headers */

    /* Send the body, and return the status code of the output filter chain. */
    return ngx_http_output_filter(r, &out);
} /* ngx_http_hello_world_handler */

/**
 * Configuration setup function that installs the content handler.
 *
 * @param cf
 *   Module configuration structure pointer.
 * @param cmd
 *   Module directives structure pointer.
 * @param conf
 *   Module configuration structure pointer.
 * @return string
 *   Status of the configuration setup.
 */
static char *ngx_http_hello_world(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf; /* pointer to core location configuration */

    /* Install the hello world handler. */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_hello_world_handler;

    return NGX_CONF_OK;
} /* ngx_http_hello_world */
{% endhighlight %}

这里我们看到，整体实现过程较为简单。

### 1.2 编译模块

这里我们介绍两种方法，一种是利用nginx的自动编译脚的```--add-module```或者```--add-dynamic-module```选项来协助我们编译；另外一种是直接通过更改Makefile的方式来实现。假设我们当前的路径结构为：
<pre>
# tree -d
.
├── nginx-1.10.3
│   ├── objs
│   │   ├── addon
├── nginx-hello-world-module
</pre>

1） **通过配置脚本来编译**

通常我们需要在模块(nginx-hello-world-module)的同一级目录创建一个```config```文件。该文件通常需要如下三个字段：

* ngx_addon_name: 仅在configure执行时使用，一般设置为模块名即可

* HTTP_MODULES： 保存所有的HTTP模块名称，每个HTTP模块间由空格符相连。在重新设置HTTP_MODULES变量时，不要直接覆盖它，因为configure调用到自定义的config脚本前，已经将各个HTTP模块设置到HTTP_MODULES变量中了，因此，要像如下这样设
<pre>
HTTP_MODULES="$HTTP_MODULES ngx_http_hello_world_module"
</pre>

* NGX_ADDON_SRCS: 用于指定新增模块的源代码，多个待编译的源代码间以空格符相连。注意，在设置NGX_ADDON_SRCS时可以使用$ngx_addon_dir变量，它等价于configure执行时```--add-module=PATH```的PATH参数

因此，这里我们编写config脚本如下：
{% highlight string %}
ngx_addon_name=ngx_http_hello_world_module
 
HTTP_MODULES="$HTTP_MODULES ngx_http_hello_world_module"
 
NGX_ADDON_SRCS="$NGX_ADDON_SRCS $ngx_addon_dir/ngx_http_hello_world_module.c"
{% endhighlight %}


2) **通过修改Makefile来编译**

上面第一种方法无疑是最方便的，因为大量的工作已由Nginx中的configure脚本帮我们做好了。在使用第三方模块时，一般也推荐使用该方法。

然而，我们有时可能需要更灵活的方式，比如重新决定 ngx_module_t *ngx_modules[] 数组中各个模块的顺序，或者在编译源代码时需要加入一些独特的编译选项，那么可以在执行完configure后，对生成的objs/ngx_modules.c和objs/Makefile文件直接进行修改。

在修改objs/ngx_modules.c文件时，通常需要修改如下三处：
<pre>
# grep -rnw "ngx_http_hello_world_module" ./
./objs/ngx_modules.c:44:extern ngx_module_t  ngx_http_hello_world_module;
./objs/ngx_modules.c:97:    &ngx_http_hello_world_module,
./objs/ngx_modules.c:152:    "ngx_http_hello_world_module",
</pre>

即首先我们要定义个extern ngx_module_t类型的全局变量， 接着将该变量添加到```ngx_modules```数组中的适当位置，再接着在```ngx_module_names```数组的适当位置加上模块对应的名称。

在修改objs/Makefile文件时，通常我们需要修改如下三个部分：
<pre>
# grep -rnw "ngx_http_hello_world_module" ./
./objs/Makefile:233:    objs/addon/nginx-hello-world-module/ngx_http_hello_world_module.o \
./objs/Makefile:355:    objs/addon/nginx-hello-world-module/ngx_http_hello_world_module.o \
./objs/Makefile:1183:objs/addon/nginx-hello-world-module/ngx_http_hello_world_module.o: $(ADDON_DEPS) \
./objs/Makefile:1184:   ../nginx-hello-world-module/ngx_http_hello_world_module.c
./objs/Makefile:1186:           -o objs/addon/nginx-hello-world-module/ngx_http_hello_world_module.o \
./objs/Makefile:1187:           ../nginx-hello-world-module/ngx_http_hello_world_module.c
</pre>
首先是需要在```objs/nginx:```这个target中将我们要编译的```.o```文件添加进去，接着还需要把目标文件链接到nginx中，最后就是编译该```.o```文件的方法。下面给出大体示例：
{% highlight string %}
build:	binary modules manpage

binary:	objs/nginx

objs/nginx:	objs/src/core/nginx.o \
	objs/src/core/ngx_log.o \
	objs/src/core/ngx_palloc.o \
	...
	objs/addon/nginx-hello-world-module/ngx_http_hello_world_module.o \
	objs/ngx_modules.o \
	../pcre-8.40/.libs/libpcre.a \
	../zlib-1.2.11/libz.a

	$(LINK) -o objs/nginx \
	objs/src/core/nginx.o \
	objs/src/core/ngx_log.o \
	objs/addon/nginx-hello-world-module/ngx_http_hello_world_module.o \
	objs/ngx_modules.o \
	-ldl -lpthread -lcrypt ../pcre-8.40/.libs/libpcre.a -lssl -lcrypto -ldl ../zlib-1.2.11/libz.a \
	-Wl,-E

objs/addon/nginx-hello-world-module/ngx_http_hello_world_module.o:	$(ADDON_DEPS) \
	../nginx-hello-world-module/ngx_http_hello_world_module.c
	$(CC) -c $(CFLAGS)  $(ALL_INCS) \
		-o objs/addon/nginx-hello-world-module/ngx_http_hello_world_module.o \
		../nginx-hello-world-module/ngx_http_hello_world_module.c
{% endhighlight %}


### 1.3 验证
编译安装完之后，我们重新配置nginx.conf文件：
<pre>
worker_processes  1;

#error_log  logs/error.log  notice;


#pid        logs/nginx.pid;

load_module modules/ngx_http_hello_world_module.so;

events {
    worker_connections  1024;
}


http {
    include       mime.types;
    default_type  application/octet-stream;

    #log_format  main  '$remote_addr - $remote_user [$time_local] "$request" '
    #                  '$status $body_bytes_sent "$http_referer" '
    #                  '"$http_user_agent" "$http_x_forwarded_for"';

    #access_log  logs/access.log  main;

    sendfile        on;
    #tcp_nopush     on;

    #keepalive_timeout  0;
    keepalive_timeout  65;

    #gzip  on;

    server {
        listen       80;
        server_name  localhost;

        #access_log  logs/host.access.log  main;

        location / {
            hello_world;
        }

        #error_page  404              /404.html;

        # redirect server error pages to the static page /50x.html
        #
        error_page   500 502 503 504  /50x.html;
        location = /50x.html {
            root   html;
        }

    }
}
</pre>
重新加载nginx，并测试验证：
<pre>
# /usr/local/nginx/sbin/nginx -t
nginx: the configuration file /etc/nginx/nginx.conf syntax is ok
nginx: configuration file /etc/nginx/nginx.conf test is successful

# /usr/local/nginx/sbin/nginx -s reload
# curl -X GET http://127.0.0.1:80/
hello world
</pre>


<br />
<br />

**[参看]**

1. [Compiling Third-Party Dynamic Modules for NGINX and NGINX Plus](https://www.nginx.com/blog/page/50/)

2. [Extending NGINX](https://www.nginx.com/resources/wiki/extending/)

3. [编写第一个Nginx模块](https://segmentfault.com/a/1190000016856451)

4. [Nginx 虚拟主机配置的三种方式](https://blog.csdn.net/liupeifeng3514/article/details/79007051)

5. [Nginx-编写自己的http模块](https://blog.csdn.net/weixin_36816337/article/details/85175602)

6. [nginx](https://cloud.tencent.com/developer/section/1259213)

7. [nginx的11个http请求处理阶段](https://blog.51cto.com/wenxi123/2296295)

<br />
<br />
<br />

