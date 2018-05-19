---
layout: post
title: core/ngx_log.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们主要讲述一下nginx中日志的相关实现。


<!-- more -->


## 1. 相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LOG_H_INCLUDED_
#define _NGX_LOG_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_LOG_STDERR            0
#define NGX_LOG_EMERG             1
#define NGX_LOG_ALERT             2
#define NGX_LOG_CRIT              3
#define NGX_LOG_ERR               4
#define NGX_LOG_WARN              5
#define NGX_LOG_NOTICE            6
#define NGX_LOG_INFO              7
#define NGX_LOG_DEBUG             8

#define NGX_LOG_DEBUG_CORE        0x010
#define NGX_LOG_DEBUG_ALLOC       0x020
#define NGX_LOG_DEBUG_MUTEX       0x040
#define NGX_LOG_DEBUG_EVENT       0x080
#define NGX_LOG_DEBUG_HTTP        0x100
#define NGX_LOG_DEBUG_MAIL        0x200
#define NGX_LOG_DEBUG_STREAM      0x400

/*
 * do not forget to update debug_levels[] in src/core/ngx_log.c
 * after the adding a new debug level
 */

#define NGX_LOG_DEBUG_FIRST       NGX_LOG_DEBUG_CORE
#define NGX_LOG_DEBUG_LAST        NGX_LOG_DEBUG_STREAM
#define NGX_LOG_DEBUG_CONNECTION  0x80000000
#define NGX_LOG_DEBUG_ALL         0x7ffffff0
{% endhighlight %}

上面宏定义可以分成3个部分：

**1） 定义日志的打印级别**
<pre>
#define NGX_LOG_STDERR            0
#define NGX_LOG_EMERG             1
#define NGX_LOG_ALERT             2
#define NGX_LOG_CRIT              3
#define NGX_LOG_ERR               4
#define NGX_LOG_WARN              5
#define NGX_LOG_NOTICE            6
#define NGX_LOG_INFO              7
#define NGX_LOG_DEBUG             8
</pre>
对于每一个logger，只有那些高于当前所设置的打印级别的信息才会打印出来。

**2) 指示debug模块**
<pre>
#define NGX_LOG_DEBUG_CORE        0x010
#define NGX_LOG_DEBUG_ALLOC       0x020
#define NGX_LOG_DEBUG_MUTEX       0x040
#define NGX_LOG_DEBUG_EVENT       0x080
#define NGX_LOG_DEBUG_HTTP        0x100
#define NGX_LOG_DEBUG_MAIL        0x200
#define NGX_LOG_DEBUG_STREAM      0x400
</pre>
对于debug级别的日志，在进行日志打印时还会检查相应的掩码。
{% highlight string %}
这里注意： 

1） 上面NGX_LOG_STDERR~NGX_LOG_DEBUG表示的是日志打印级别中的大级别，用4个bit位即可表示。
    并且日志级别只能属于上述大级别中的一个。

2） 而NGX_LOG_DEBUG_CORE~NGX_LOG_DEBUG_STREAM,是debug级别中的小级别，可以同时存在多个小
    级别，这样需要7个bit位类表示

3) 如果设置为NGX_LOG_DEBUG这样一个debug大级别，则包含所有NGX_LOG_DEBUG_CORE~NGX_LOG_DEBUG_STREAM
   中的所有小级别，此时log_level会被直接设置为NGX_LOG_DEBUG_ALL
{% endhighlight %}

**3) debug相应掩码总结**

* ```NGX_LOG_DEBUG_FIRST```: 第一个debug模块

* ```NGX_LOG_DEBUG_LAST``: 最后一个debug模块

* ```NGX_LOG_DEBUG_CONNECTION```: 用于指示nginx中相应连接的debug信息的掩码

* ```NGX_LOG_DEBUG_ALL```: nginx中所有debug信息的掩码

## 2. ngx_log_s数据结构
{% highlight string %}
typedef u_char *(*ngx_log_handler_pt) (ngx_log_t *log, u_char *buf, size_t len);
typedef void (*ngx_log_writer_pt) (ngx_log_t *log, ngx_uint_t level,
    u_char *buf, size_t len);


struct ngx_log_s {
    ngx_uint_t           log_level;
    ngx_open_file_t     *file;

    ngx_atomic_uint_t    connection;

    time_t               disk_full_time;

    ngx_log_handler_pt   handler;
    void                *data;

    ngx_log_writer_pt    writer;
    void                *wdata;

    /*
     * we declare "action" as "char *" because the actions are usually
     * the static strings and in the "u_char *" case we have to override
     * their types all the time
     */

    char                *action;

    ngx_log_t           *next;
};
{% endhighlight %}
ngx_log_s是对日志数据结构的抽象，下面详细介绍一下各字段：

* ```log_level```: 指示当前的日志打印级别

* ```file```: 指示当前日志对象所关联的文件

* ```connection```: 当前引用该日志对象的连接数(connection number)

* ```disk_full_time```: 指示日志硬盘满的时间

* ```handler```: 日志信息处理器（一般是对日志进行格式化）

* ```data```: 一般作为日志信息处理的一个上下文对象。例如： 当前要用handler格式化一个http request信息等，此时可以用data字段来存放该上下文。（可以通过查询log->data,找到相应的用法）

* ```writer```: 进行日志写操作处理器

* ```wdata```: 作为日志写操作处理器的一个上下文对象。例如： 当writer是写到内存中时，wdata可能关联的就是一个```ngx_log_memory_buf_t```对象。

* ```action```: 用于指示在写日志前，所执行的操作（比如当前正在进行```SSL handshaking```，或者正在```closing request```等）

* ```next```: 指向下一个日志对象




## 3. 日志打印核心函数声明
{% highlight string %}
#define NGX_MAX_ERROR_STR   2048


/*********************************/

#if (NGX_HAVE_C99_VARIADIC_MACROS)

#define NGX_HAVE_VARIADIC_MACROS  1

#define ngx_log_error(level, log, ...)                                        \
    if ((log)->log_level >= level) ngx_log_error_core(level, log, __VA_ARGS__)

void ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, ...);

#define ngx_log_debug(level, log, ...)                                        \
    if ((log)->log_level & level)                                             \
        ngx_log_error_core(NGX_LOG_DEBUG, log, __VA_ARGS__)

/*********************************/

#elif (NGX_HAVE_GCC_VARIADIC_MACROS)

#define NGX_HAVE_VARIADIC_MACROS  1

#define ngx_log_error(level, log, args...)                                    \
    if ((log)->log_level >= level) ngx_log_error_core(level, log, args)

void ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, ...);

#define ngx_log_debug(level, log, args...)                                    \
    if ((log)->log_level & level)                                             \
        ngx_log_error_core(NGX_LOG_DEBUG, log, args)

/*********************************/

#else /* no variadic macros */

#define NGX_HAVE_VARIADIC_MACROS  0

void ngx_cdecl ngx_log_error(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, ...);
void ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, va_list args);
void ngx_cdecl ngx_log_debug_core(ngx_log_t *log, ngx_err_t err,
    const char *fmt, ...);


#endif /* variadic macros */


/*********************************/
{% endhighlight %}

这里首先定义了```NGX_MAX_ERROR_STR```为2048，即单条日志的最大长度为2048。

当前在objs/ngx_auto_config.h头文件中，我们有如下宏定义：
<pre>
#ifndef NGX_HAVE_C99_VARIADIC_MACROS
#define NGX_HAVE_C99_VARIADIC_MACROS  1
#endif


#ifndef NGX_HAVE_GCC_VARIADIC_MACROS
#define NGX_HAVE_GCC_VARIADIC_MACROS  1
#endif
</pre>

我们当前同时支持```C99可变参数宏定义```与```gcc可变参数宏定义```。其中c99可变参数宏定义类似于如下：
<pre>
#define var(dummy, ...)  sprintf(__VA_ARGS__)
</pre>

gcc可变参数宏定义类似于如下：
<pre>
#define var(dummy, args...)  sprintf(args)
</pre>


上面我们定义了三个函数：

* ngx_log_error(): 打印一般的错误日志消息(只有高于对应错误日志级别，才会打印出来）

* ngx_log_error_core(): 打印日志的核心函数

* ngx_log_debug_core(): 打印debug级别日志的函数


## 4. ngx_log_debug辅助函数
{% highlight string %}
#if (NGX_DEBUG)

#if (NGX_HAVE_VARIADIC_MACROS)

#define ngx_log_debug0(level, log, err, fmt)                                  \
        ngx_log_debug(level, log, err, fmt)

#define ngx_log_debug1(level, log, err, fmt, arg1)                            \
        ngx_log_debug(level, log, err, fmt, arg1)

#define ngx_log_debug2(level, log, err, fmt, arg1, arg2)                      \
        ngx_log_debug(level, log, err, fmt, arg1, arg2)

#define ngx_log_debug3(level, log, err, fmt, arg1, arg2, arg3)                \
        ngx_log_debug(level, log, err, fmt, arg1, arg2, arg3)

#define ngx_log_debug4(level, log, err, fmt, arg1, arg2, arg3, arg4)          \
        ngx_log_debug(level, log, err, fmt, arg1, arg2, arg3, arg4)

#define ngx_log_debug5(level, log, err, fmt, arg1, arg2, arg3, arg4, arg5)    \
        ngx_log_debug(level, log, err, fmt, arg1, arg2, arg3, arg4, arg5)

#define ngx_log_debug6(level, log, err, fmt,                                  \
                       arg1, arg2, arg3, arg4, arg5, arg6)                    \
        ngx_log_debug(level, log, err, fmt,                                   \
                       arg1, arg2, arg3, arg4, arg5, arg6)

#define ngx_log_debug7(level, log, err, fmt,                                  \
                       arg1, arg2, arg3, arg4, arg5, arg6, arg7)              \
        ngx_log_debug(level, log, err, fmt,                                   \
                       arg1, arg2, arg3, arg4, arg5, arg6, arg7)

#define ngx_log_debug8(level, log, err, fmt,                                  \
                       arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)        \
        ngx_log_debug(level, log, err, fmt,                                   \
                       arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)


#else /* no variadic macros */

#define ngx_log_debug0(level, log, err, fmt)                                  \
    if ((log)->log_level & level)                                             \
        ngx_log_debug_core(log, err, fmt)

#define ngx_log_debug1(level, log, err, fmt, arg1)                            \
    if ((log)->log_level & level)                                             \
        ngx_log_debug_core(log, err, fmt, arg1)

#define ngx_log_debug2(level, log, err, fmt, arg1, arg2)                      \
    if ((log)->log_level & level)                                             \
        ngx_log_debug_core(log, err, fmt, arg1, arg2)

#define ngx_log_debug3(level, log, err, fmt, arg1, arg2, arg3)                \
    if ((log)->log_level & level)                                             \
        ngx_log_debug_core(log, err, fmt, arg1, arg2, arg3)

#define ngx_log_debug4(level, log, err, fmt, arg1, arg2, arg3, arg4)          \
    if ((log)->log_level & level)                                             \
        ngx_log_debug_core(log, err, fmt, arg1, arg2, arg3, arg4)

#define ngx_log_debug5(level, log, err, fmt, arg1, arg2, arg3, arg4, arg5)    \
    if ((log)->log_level & level)                                             \
        ngx_log_debug_core(log, err, fmt, arg1, arg2, arg3, arg4, arg5)

#define ngx_log_debug6(level, log, err, fmt,                                  \
                       arg1, arg2, arg3, arg4, arg5, arg6)                    \
    if ((log)->log_level & level)                                             \
        ngx_log_debug_core(log, err, fmt, arg1, arg2, arg3, arg4, arg5, arg6)

#define ngx_log_debug7(level, log, err, fmt,                                  \
                       arg1, arg2, arg3, arg4, arg5, arg6, arg7)              \
    if ((log)->log_level & level)                                             \
        ngx_log_debug_core(log, err, fmt,                                     \
                       arg1, arg2, arg3, arg4, arg5, arg6, arg7)

#define ngx_log_debug8(level, log, err, fmt,                                  \
                       arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)        \
    if ((log)->log_level & level)                                             \
        ngx_log_debug_core(log, err, fmt,                                     \
                       arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)

#endif

#else /* !NGX_DEBUG */

#define ngx_log_debug0(level, log, err, fmt)
#define ngx_log_debug1(level, log, err, fmt, arg1)
#define ngx_log_debug2(level, log, err, fmt, arg1, arg2)
#define ngx_log_debug3(level, log, err, fmt, arg1, arg2, arg3)
#define ngx_log_debug4(level, log, err, fmt, arg1, arg2, arg3, arg4)
#define ngx_log_debug5(level, log, err, fmt, arg1, arg2, arg3, arg4, arg5)
#define ngx_log_debug6(level, log, err, fmt, arg1, arg2, arg3, arg4, arg5, arg6)
#define ngx_log_debug7(level, log, err, fmt, arg1, arg2, arg3, arg4, arg5,    \
                       arg6, arg7)
#define ngx_log_debug8(level, log, err, fmt, arg1, arg2, arg3, arg4, arg5,    \
                       arg6, arg7, arg8)

#endif
{% endhighlight %}

如上这些函数只是为了使用方便，针对ngx_log_debug()可变参数函数的特定参数个数时，进行定制。

当前我们并未开启```NGX_DEBUG```，要想开启```NGX_DEBUG```宏，请在编译时添加```--with-debug```选项。

## 5. 相关函数声明 
{% highlight string %}
//1) nginx 日志初始化
ngx_log_t *ngx_log_init(u_char *prefix);


//2) 打印alert级别日志，表示出现严重错误
void ngx_cdecl ngx_log_abort(ngx_err_t err, const char *fmt, ...);

//3) 将相应的日志信息打印到标准输出
void ngx_cdecl ngx_log_stderr(ngx_err_t err, const char *fmt, ...);

//4) 答应相关errno的日志（注意此函数至少保证留有50个字节的空间来打印errno日志）
u_char *ngx_log_errno(u_char *buf, u_char *last, ngx_err_t err);

//5) 打开默认日志文件
ngx_int_t ngx_log_open_default(ngx_cycle_t *cycle);

//6) 重定向标准错误。即将标准错误输出重定向到某一个日志文件
ngx_int_t ngx_log_redirect_stderr(ngx_cycle_t *cycle);

//7) 获得一个文件日志对象
ngx_log_t *ngx_log_get_file_log(ngx_log_t *head);

//8) 根据ngx_conf_t设置日志对象
char *ngx_log_set_log(ngx_conf_t *cf, ngx_log_t **head);
{% endhighlight %}


## 6. 相关静态函数定义
{% highlight string %}
/*
 * ngx_write_stderr() cannot be implemented as macro, since
 * MSVC does not allow to use #ifdef inside macro parameters.
 *
 * ngx_write_fd() is used instead of ngx_write_console(), since
 * CharToOemBuff() inside ngx_write_console() cannot be used with
 * read only buffer as destination and CharToOemBuff() is not needed
 * for ngx_write_stderr() anyway.
 */
static ngx_inline void
ngx_write_stderr(char *text)
{
    (void) ngx_write_fd(ngx_stderr, text, ngx_strlen(text));
}


static ngx_inline void
ngx_write_stdout(char *text)
{
    (void) ngx_write_fd(ngx_stdout, text, ngx_strlen(text));
}

{% endhighlight %}

上述两个函数分别用于将```text```输出到标准错误以及标准输出中。关于为什么```ngx_write_stderr()```不能采用宏来实现，上面注释已经很清楚，不过这主要是针对MSVC上的实现，即Windows版本nginx，对于linux版本并没有此问题。

而关于为什么使用```ngx_write_fd()```函数，上面的注释当前已经过于老旧。目前ngx_write_console()函数实现如下：
<pre>
#define ngx_write_console        ngx_write_fd
</pre>



## 7. 相关变量声明
{% highlight string %}
extern ngx_module_t  ngx_errlog_module;
extern ngx_uint_t    ngx_use_stderr;
{% endhighlight %}

上面声明了两个变量：

* ```ngx_errlog_module```: errlog模块相应数据结构变量

* ```ngx_use_stderr```: 该变量主要用于控制是否输出到标准错误。一般在Nginx启动的时候，会将ngx_use_stderr设置为1，这时用户可以从标准错误中看到相应的启动输出信息，启动之后就会被设置为0，这时nginx以静默方式运行。


<br />
<br />

**[参考]**

1. [git的使用](https://www.yiibai.com/git/git-quick-start.html)

2. [bit book](https://git-scm.com/book/zh/v2)
<br />
<br />
<br />

