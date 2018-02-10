---
layout: post
title: core/ngx_conf_file.h源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们讲述nginx配置文件相关的一些内容。


<!-- more -->



## 1. 配置类型相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONF_FILE_H_INCLUDED_
#define _NGX_CONF_FILE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 *        AAAA  number of arguments
 *      FF      command flags
 *    TT        command type, i.e. HTTP "location" or "server" command
 */

#define NGX_CONF_NOARGS      0x00000001
#define NGX_CONF_TAKE1       0x00000002
#define NGX_CONF_TAKE2       0x00000004
#define NGX_CONF_TAKE3       0x00000008
#define NGX_CONF_TAKE4       0x00000010
#define NGX_CONF_TAKE5       0x00000020
#define NGX_CONF_TAKE6       0x00000040
#define NGX_CONF_TAKE7       0x00000080

#define NGX_CONF_MAX_ARGS    8

#define NGX_CONF_TAKE12      (NGX_CONF_TAKE1|NGX_CONF_TAKE2)
#define NGX_CONF_TAKE13      (NGX_CONF_TAKE1|NGX_CONF_TAKE3)

#define NGX_CONF_TAKE23      (NGX_CONF_TAKE2|NGX_CONF_TAKE3)

#define NGX_CONF_TAKE123     (NGX_CONF_TAKE1|NGX_CONF_TAKE2|NGX_CONF_TAKE3)
#define NGX_CONF_TAKE1234    (NGX_CONF_TAKE1|NGX_CONF_TAKE2|NGX_CONF_TAKE3   \
                              |NGX_CONF_TAKE4)

#define NGX_CONF_ARGS_NUMBER 0x000000ff
#define NGX_CONF_BLOCK       0x00000100
#define NGX_CONF_FLAG        0x00000200
#define NGX_CONF_ANY         0x00000400
#define NGX_CONF_1MORE       0x00000800
#define NGX_CONF_2MORE       0x00001000
#define NGX_CONF_MULTI       0x00000000  /* compatibility */

#define NGX_DIRECT_CONF      0x00010000

#define NGX_MAIN_CONF        0x01000000
#define NGX_ANY_CONF         0x1F000000



#define NGX_CONF_UNSET       -1
#define NGX_CONF_UNSET_UINT  (ngx_uint_t) -1
#define NGX_CONF_UNSET_PTR   (void *) -1
#define NGX_CONF_UNSET_SIZE  (size_t) -1
#define NGX_CONF_UNSET_MSEC  (ngx_msec_t) -1


#define NGX_CONF_OK          NULL
#define NGX_CONF_ERROR       (void *) -1

#define NGX_CONF_BLOCK_START 1
#define NGX_CONF_BLOCK_DONE  2
#define NGX_CONF_FILE_DONE   3

#define NGX_CORE_MODULE      0x45524F43  /* "CORE" */
#define NGX_CONF_MODULE      0x464E4F43  /* "CONF" */


#define NGX_MAX_CONF_ERRSTR  1024
{% endhighlight %}
下面我们介绍一下各宏定义的相关含义：

**1) 配置指令属性**
 
* ```NGX_CONF_NOARGS```: 配置指令不接受任何参数

* ```NGX_CONF_TAKE1```: 配置指令接受1个参数

* ```NGX_CONF_TAKE2```: 配置指令接受2个参数

* ```NGX_CONF_TAKE3```: 配置指令接受3个参数

* ```NGX_CONF_TAKE4```: 配置指令接受4个参数

* ```NGX_CONF_TAKE5```: 配置指令接受5个参数

* ```NGX_CONF_TAKE6```: 配置指令接受6个参数

* ```NGX_CONF_TAKE7```: 配置指令接受7个参数

* ```NGX_CONF_MAX_ARGS```: nginx配置指令最大参数大小，目前该值被定义为8，也就是不能超过8个参数值

关于参数个数，可以组合多个属性。比如一个指令可以不填参数，也可以接受1个或者2个参数。那么就是：
{% highlight string %}
NGX_CONF_NOARGS | NGX_CONF_TAKE1 | NGX_CONF_TAKE2
{% endhighlight %}
如果写上面3个属性在一起，可能会觉得麻烦，因此nginx提供了一些定义，使用起来更简洁：

* ```NGX_CONF_TAKE12```: 配置指令接受1个或2个参数

* ```NGX_CONF_TAKE13```: 配置指令接受1个或3个参数

* ```NGX_CONF_TAKE23```: 配置指令接受2个或3个参数

* ```NGX_CONF_TAKE123```: 配置指令接受1个或2个或3个参数

* ```NGX_CONF_TAKE1234```: 配置指令接受1个或2个或3个参数

* ```NGX_CONF_ARGS_NUMBER```: 用于取参数个数的宏定义

* ```NGX_CONF_BLOCK```: 配置指令可以接受的值是一个配置信息块。也就是一对大括号括起来的内容。里面可以再包括很多的配置指令，比如常见的server指令就是这个属性的：
<pre>
http {

    ...

    server {
        listen       8000;
        server_name  somename  alias  another.alias;

        location / {
            root   html;
            index  index.html index.htm;
        }
    }

    ....
}
</pre>

* ```NGX_CONF_FLAG```: 配置指令可以接受的值是```on```或者```off``，最终会被转成bool值

* ```NGX_CONF_ANY```: 配置指令可以接受任意参数值。一个或者多个，或者```on```，或者```off```，或者是配置块

* ```NGX_CONF_1MORE```: 配置指令至少接受1个参数

* ```NGX_CONF_2MORE```: 配置指令至少接受2个参数

* ```NGX_CONF_MULTI```: 配置指令可以接受多个参数，即个数不定（但是应确保不超过```NGX_CONF_MAX_ARGS```)







<br />
<br />

**[参看]:**

1. [handler模块(100%)](http://tengine.taobao.org/book/chapter_03.html)

2. [Nginx配置参数说明](https://www.cnblogs.com/fansik/p/6952453.html)

<br />
<br />
<br />

