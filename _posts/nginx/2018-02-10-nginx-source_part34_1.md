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

* ```NGX_CONF_FLAG```: 配置指令可以接受的值是```on```或者```off```，最终会被转成bool值

* ```NGX_CONF_ANY```: 配置指令可以接受任意参数值。一个或者多个，或者```on```，或者```off```，或者是配置块

* ```NGX_CONF_1MORE```: 配置指令至少接受1个参数

* ```NGX_CONF_2MORE```: 配置指令至少接受2个参数

* ```NGX_CONF_MULTI```: 配置指令可以接受多个参数，即个数不定（但是应确保不超过```NGX_CONF_MAX_ARGS```)

<br />

下面介绍一组说明配置指令可以出现的位置的属性：

* ```NGX_DIRECT_CONF```: 配置指令只能出现在主配置文件中

* ```NGX_MAIN_CONF```: 配置指令只能出现在主配置级别，例如http、mail、events、error_log等配置指令

* ```NGX_ANY_CONF```: 该配置指令可以出现在任意配置级别上。
<pre>
注意： 除此之外，还有如下一些用于指定配置指令出现位置的宏

NGX_HTTP_MAIN_CONF: 配置指令只能出现在http-server主配置级别

NGX_HTTP_SRV_CONF: 配置指令只能出现在http-server的虚拟主机配置级别

NGX_HTTP_LOC_CONF: 配置指令只能出现在http-server的location配置级别

NGX_HTTP_LMT_CONF: 配置指令只能出现在limit_except块中

NGX_HTTP_LIF_CONF: 配置指令只能出现在if()块中
</pre>

<br />

**2) 其他配置相关宏定义**
{% highlight string %}
// 表示当前某一种类型的配置项未设置
#define NGX_CONF_UNSET       -1
#define NGX_CONF_UNSET_UINT  (ngx_uint_t) -1
#define NGX_CONF_UNSET_PTR   (void *) -1
#define NGX_CONF_UNSET_SIZE  (size_t) -1
#define NGX_CONF_UNSET_MSEC  (ngx_msec_t) -1


#define NGX_CONF_OK          NULL
#define NGX_CONF_ERROR       (void *) -1


// 表示配置信息块的开始/结束，整个配置文件的结束， 主要是用于解析配置文件时使用
#define NGX_CONF_BLOCK_START 1
#define NGX_CONF_BLOCK_DONE  2
#define NGX_CONF_FILE_DONE   3

// 模块类型的magic值
#define NGX_CORE_MODULE      0x45524F43  /* "CORE" */
#define NGX_CONF_MODULE      0x464E4F43  /* "CONF" */

//配置文件最长错误字符串长度
#define NGX_MAX_CONF_ERRSTR  1024
{% endhighlight %}

属于```NGX_CORE_MODULE```类型的模块主要有：

* **ngx_events_module**

* **ngx_openssl_module**

* **ngx_google_perftools_module**

* **ngx_http_module**

* **ngx_errlog_module**

* **ngx_mail_module**

* **ngx_regex_module**

* **ngx_stream_module**

* **ngx_thread_pool_module**

<pre>
注意： 

nginx模块虽然有很多，但是基本类型只有5种： CORE、CONF、EVENT、HTTP、MAIL

#define NGX_CORE_MODULE      0x45524F43  /* "CORE" */
#define NGX_CONF_MODULE      0x464E4F43  /* "CONF" */
#define NGX_EVENT_MODULE     0x544E5645  /* "EVNT" */
#define NGX_HTTP_MODULE      0x50545448  /* "HTTP" */
#define NGX_MAIL_MODULE      0x4C49414D  /* "MAIL" */
</pre>

<br />

## 2. 相关数据结构定义

**1) ngx_command_s数据结构**
{% highlight string %}
struct ngx_command_s {
    ngx_str_t             name;
    ngx_uint_t            type;
    char               *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
    ngx_uint_t            conf;
    ngx_uint_t            offset;
    void                 *post;
};
{% endhighlight %}
各成员含义如下：

* ```name```: 本条指令的名字，例如```worker_processes 1;```对应的```ngx_command_s.name```就是```worker_processes```

* ```type```: 配置指令属性的集合。例如，```worker_processes```这条指令对应的type定义为：
<pre>
NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE1
</pre>
这就表示该指令用于main上下文; 且是属于main上下文的简单指令; 该指令后跟一个参数,例如: ```worker_processes 1;```

* ```set```: 函数指针set用来表示，当nginx解析配置文件碰到此指令时，该执行怎样的操作。而该操作本身，自然是用来设置本模块所对应的```ngx_<module_name>_conf_t```结构体。

* ```conf```: 这个变量只在```NGX_HTTP_MODULE```类型模块的```ngx_command_t```使用，指定当前配置项存储的内存位置。实际上是使用哪个内存池的问题。因为http模块对所有该模块要保存的信息划分了main、server、location三个地方进行存储，每个地方都有一个内存池用来分配存储这些信息的内存。这里可能的取值为：
<pre>
GX_HTTP_MAIN_CONF_OFFSET、NGX_HTTP_SRV_CONF_OFFSET或NGX_HTTP_LOC_CONF_OFFSET
</pre>

* ```offset```: 这个变量用来标记```ngx_<module_name>_conf_t```中某成员变量的偏移量，纯粹是为了使用方便

* ```post```: 可以指向任何一个在读取配置过程中需要的数据，以便进行配置读取的数据。大多数时候不需要，直接设置为0即可

<br />

**2) ngx_null_command宏**
{% highlight string %}
#define ngx_null_command  { ngx_null_string, 0, NULL, 0, 0, NULL }
{% endhighlight %}
一般作为ngx_command_s配置数组的结束标志

<br />

**3) ngx_open_file_s数据结构**
{% highlight string %}
struct ngx_open_file_s {
    ngx_fd_t              fd;
    ngx_str_t             name;

    void                (*flush)(ngx_open_file_t *file, ngx_log_t *log);
    void                 *data;
};
{% endhighlight %}
```ngx_open_file_s```代表一个已打开的文件，各成员含义如下：

* ```fd```: 已打开文件句柄

* ```name```: 已打开文件的文件名

* ```flush```: 函数指针flush，用于指定当有数据需要写入到文件时，所进行的操作（此过程可能不是简单的文件写入操作，可能还涉及到其他变量的更新）。

* ```data```: 辅助数据

<br />

**4) ngx_conf_file_t数据结构**
{% highlight string %}
typedef struct {
    ngx_file_t            file;
    ngx_buf_t            *buffer;
    ngx_buf_t            *dump;
    ngx_uint_t            line;
} ngx_conf_file_t;
{% endhighlight %}
各成员含义如下：

* ```file```: 该配置所对应的文件

* ```buffer```: 该配置文件所关联的缓冲

* ```dump```: 主要用于在执行```./nginx -T```命令时，用于指定dump时所用的缓冲

* ```line```: 用于指定当前解析到的行数

<br />

**5) ngx_conf_dump_t数据结构**
{% highlight string %}
typedef struct {
    ngx_str_t             name;
    ngx_buf_t            *buffer;
} ngx_conf_dump_t;
{% endhighlight %}
dump配置文件时用到。

<br />

**6) ngx_conf_s数据结构**
{% highlight string %}
typedef char *(*ngx_conf_handler_pt)(ngx_conf_t *cf,
    ngx_command_t *dummy, void *conf);


struct ngx_conf_s {
    char                 *name;
    ngx_array_t          *args;

    ngx_cycle_t          *cycle;
    ngx_pool_t           *pool;
    ngx_pool_t           *temp_pool;
    ngx_conf_file_t      *conf_file;
    ngx_log_t            *log;

    void                 *ctx;
    ngx_uint_t            module_type;
    ngx_uint_t            cmd_type;

    ngx_conf_handler_pt   handler;
    char                 *handler_conf;
};

{% endhighlight %}
各成员含义如下：

* ```name```: 存放当前所解析到的指令

* ```args```: 存放该指令包含的所有参数。args[0]存放的是指令本身

* ```cycle```: 所关联的全局ngx_cycle_t变量

* ```pool```: 所关联的内存池

* ```temp_pool```: 用于解析配置文件的临时内存池，解析完后释放

* ```conf_file```: 存放nginx配置文件相关信息

* ```log```: 描述日志文件的相关属性

* ```ctx```: 描述指令的上下文信息

* ```module_type```: 当前指令所属模块类型，core、http、event和mail中的一种

* ```cmd_type```: 指令的类型

* ```handler```: 指令自定义的处理函数

* ```handler_conf```: 自定义处理函数需要的相关配置

<br />

**7) ngx_conf_deprecated_t数据结构**
{% highlight string %}
typedef char *(*ngx_conf_post_handler_pt) (ngx_conf_t *cf,
    void *data, void *conf);

typedef struct {
    ngx_conf_post_handler_pt  post_handler;
} ngx_conf_post_t;


typedef struct {
    ngx_conf_post_handler_pt  post_handler;
    char                     *old_name;
    char                     *new_name;
} ngx_conf_deprecated_t;
{% endhighlight %}
主要用于处理nginx中的过时指令

<br />

**8) ngx_conf_num_bounds_t数据结构**
{% highlight string %}
typedef struct {
    ngx_conf_post_handler_pt  post_handler;
    ngx_int_t                 low;
    ngx_int_t                 high;
} ngx_conf_num_bounds_t;
{% endhighlight %}
对nginx配置指令取值的上下界的封装

<br />

**9) ngx_conf_enum_t数据结构**
{% highlight string %}
typedef struct {
    ngx_str_t                 name;
    ngx_uint_t                value;
} ngx_conf_enum_t;
{% endhighlight %}
配置中的枚举结构。

<br />

**9) ngx_conf_bitmask_t数据结构**
{% highlight string %}
#define NGX_CONF_BITMASK_SET  1

typedef struct {
    ngx_str_t                 name;
    ngx_uint_t                mask;
} ngx_conf_bitmask_t;
{% endhighlight %}
配置中的位掩码结构

<br />

**10) 相关函数**
{% highlight string %}
//处理过时指令
char * ngx_conf_deprecated(ngx_conf_t *cf, void *post, void *data);

//检测配置指令中的上下界
char *ngx_conf_check_num_bounds(ngx_conf_t *cf, void *post, void *data);
{% endhighlight %}

## 3. 配置的默认初始化相关宏定义
{% highlight string %}
// 获得配置上下文中的对应模块的配置
#define ngx_get_conf(conf_ctx, module)  conf_ctx[module.index]


// 采用default值初始化当前conf变量（conf一般为bool类型，因此default一般取0或1）
// 注意： C语言中bool类型一般用int表示
#define ngx_conf_init_value(conf, default)                                   \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = default;                                                      \
    }

// 初始化指针类型变量
#define ngx_conf_init_ptr_value(conf, default)                               \
    if (conf == NGX_CONF_UNSET_PTR) {                                        \
        conf = default;                                                      \
    }

// 初始化uint类型变量
#define ngx_conf_init_uint_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_UINT) {                                       \
        conf = default;                                                      \
    }

// 初始化size类型变量
#define ngx_conf_init_size_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_SIZE) {                                       \
        conf = default;                                                      \
    }

// 初始化时间类型变量
#define ngx_conf_init_msec_value(conf, default)                              \
    if (conf == NGX_CONF_UNSET_MSEC) {                                       \
        conf = default;                                                      \
    }

// 在conf当前未设置的情况下，如果prev值为NGX_CONF_UNSET,则将conf设置为default；否则设置为prev
#define ngx_conf_merge_value(conf, prev, default)                            \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

// 
#define ngx_conf_merge_ptr_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET_PTR) {                                        \
        conf = (prev == NGX_CONF_UNSET_PTR) ? default : prev;                \
    }

#define ngx_conf_merge_uint_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_UINT) {                                       \
        conf = (prev == NGX_CONF_UNSET_UINT) ? default : prev;               \
    }

#define ngx_conf_merge_msec_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_MSEC) {                                       \
        conf = (prev == NGX_CONF_UNSET_MSEC) ? default : prev;               \
    }

#define ngx_conf_merge_sec_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

#define ngx_conf_merge_size_value(conf, prev, default)                       \
    if (conf == NGX_CONF_UNSET_SIZE) {                                       \
        conf = (prev == NGX_CONF_UNSET_SIZE) ? default : prev;               \
    }

#define ngx_conf_merge_off_value(conf, prev, default)                        \
    if (conf == NGX_CONF_UNSET) {                                            \
        conf = (prev == NGX_CONF_UNSET) ? default : prev;                    \
    }

// 对字符串类型进行合并设置
#define ngx_conf_merge_str_value(conf, prev, default)                        \
    if (conf.data == NULL) {                                                 \
        if (prev.data) {                                                     \
            conf.len = prev.len;                                             \
            conf.data = prev.data;                                           \
        } else {                                                             \
            conf.len = sizeof(default) - 1;                                  \
            conf.data = (u_char *) default;                                  \
        }                                                                    \
    }

// 对buf类型进行合并设置
#define ngx_conf_merge_bufs_value(conf, prev, default_num, default_size)     \
    if (conf.num == 0) {                                                     \
        if (prev.num) {                                                      \
            conf.num = prev.num;                                             \
            conf.size = prev.size;                                           \
        } else {                                                             \
            conf.num = default_num;                                          \
            conf.size = default_size;                                        \
        }                                                                    \
    }


#define ngx_conf_merge_bitmask_value(conf, prev, default)                    \
    if (conf == 0) {                                                         \
        conf = (prev == 0) ? default : prev;                                 \
    }
{% endhighlight %}

## 4. 相关函数声明
{% highlight string %}
// 主要是用来处理通过命令行-g选项传递进来的“全局配置指令”
char *ngx_conf_param(ngx_conf_t *cf);

// 用于解析配置信息
char *ngx_conf_parse(ngx_conf_t *cf, ngx_str_t *filename);

// 用于解析include指令
char *ngx_conf_include(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 获取配置文件的全路径名称
ngx_int_t ngx_conf_full_name(ngx_cycle_t *cycle, ngx_str_t *name,
    ngx_uint_t conf_prefix);

// 打开配置文件中指定的一个文件
ngx_open_file_t *ngx_conf_open_file(ngx_cycle_t *cycle, ngx_str_t *name);

// 处理配置文件中的log_error指令
void ngx_cdecl ngx_conf_log_error(ngx_uint_t level, ngx_conf_t *cf,
    ngx_err_t err, const char *fmt, ...);

// 用来设置flag类型（bool类型）的变量
char *ngx_conf_set_flag_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用来设置字符串类型的变量
char *ngx_conf_set_str_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用来设置字符串数组类型的变量
char *ngx_conf_set_str_array_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);

// 用来设置key/value类型的数组变量
char *ngx_conf_set_keyval_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用于设置数字类型变量
char *ngx_conf_set_num_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用于设置size_t类型变量
char *ngx_conf_set_size_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用于设置offset类型变量
char *ngx_conf_set_off_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用于设置毫秒类型变量
char *ngx_conf_set_msec_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用于设置秒类型变量
char *ngx_conf_set_sec_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用于设置buf类型的变量
char *ngx_conf_set_bufs_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用于设置枚举类型变量
char *ngx_conf_set_enum_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

// 用于设置掩码类型变量
char *ngx_conf_set_bitmask_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
{% endhighlight %}

<br />
<br />

**[参看]:**

1. [handler模块(100%)](http://tengine.taobao.org/book/chapter_03.html)

2. [Nginx配置参数说明](https://www.cnblogs.com/fansik/p/6952453.html)

3. [Nginx 源码分析：从模块到配置（下）](https://segmentfault.com/a/1190000002780254)

4. [图解Nginx 中的4级指针](http://blog.chinaunix.net/uid-27767798-id-3840094.html)

5. [nginx源码分析之配置图解](https://my.oschina.net/fqing/blog/80867)

6. [Difference between NGX_DIRECT_CONF and NGX_MAIN_CONF](http://nginx-devel.nginx.narkive.com/va8Fwi2S/difference-between-ngx-direct-conf-and-ngx-main-conf)

7. [Nginx配置文件详解](https://www.cnblogs.com/hunttown/p/5759959.html)

8. [Nginx简介及配置文件详解](http://blog.csdn.net/hzsunshine/article/details/63687054)

9. [Configuration directives](http://www.nginxguts.com/2011/09/configuration-directives/)

10. [Core functionality](http://nginx.org/en/docs/ngx_core_module.html)
<br />
<br />
<br />

