---
layout: post
title: core/ngx_log.c源文件分析(1)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们主要讲述一下nginx中日志的相关实现。


<!-- more -->


## 1. 相关静态函数声明
{% highlight strig %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


static char *ngx_error_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_log_set_levels(ngx_conf_t *cf, ngx_log_t *log);
static void ngx_log_insert(ngx_log_t *log, ngx_log_t *new_log);


#if (NGX_DEBUG)

static void ngx_log_memory_writer(ngx_log_t *log, ngx_uint_t level,
    u_char *buf, size_t len);
static void ngx_log_memory_cleanup(void *data);


typedef struct {
    u_char        *start;
    u_char        *end;
    u_char        *pos;
    ngx_atomic_t   written;
} ngx_log_memory_buf_t;

#endif
{% endhighlight %}

下面简要介绍一下这些函数：

* ```ngx_error_log()```: 解析error_log指令时相应的处理函数

* ```ngx_log_set_levels()```: 主要是为了设置日志级别

* ```ngx_log_insert()```: 将new_log插入到log链表中，log链表是按```log->log_level```从大到小的顺序排列的。

* ```ngx_log_memory_writer()```: 此函数在```NGX_DEBUG```条件下使用,主要用于在调试环境下将buf数据写到内存中。

* ```ngx_log_memory_cleanup()```: 此函数在```NGX_DEBUG```条件下使用，主要用于清除内存buf

* ```ngx_log_memory_buf_t```： 此数据结构在```NGX_DEBUG```条件下使用，用于在内存中保存日志的buf。这是一个循环buffer内存，下面我们详细介绍一下该数据结构中各字段的含义：
{% highlight string %}
#if (NGX_DEBUG)
typedef struct {
    u_char        *start;      //该内存Buff的开始位置
    u_char        *end;        //该内存buff的结束位置
    u_char        *pos;        //实际写日志的其实位置，这是因为在start后会插入一些相应的内存日志的提示信息
    ngx_atomic_t   written;    //当前总共写了多少日志数据
} ngx_log_memory_buf_t;

#endif
{% endhighlight %}

## 2. 相关变量定义
{% highlight string %}
static ngx_command_t  ngx_errlog_commands[] = {

    { ngx_string("error_log"),
      NGX_MAIN_CONF|NGX_CONF_1MORE,
      ngx_error_log,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_core_module_t  ngx_errlog_module_ctx = {
    ngx_string("errlog"),
    NULL,
    NULL
};


ngx_module_t  ngx_errlog_module = {
    NGX_MODULE_V1,
    &ngx_errlog_module_ctx,                /* module context */
    ngx_errlog_commands,                   /* module directives */
    NGX_CORE_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_log_t        ngx_log;
static ngx_open_file_t  ngx_log_file;
ngx_uint_t              ngx_use_stderr = 1;


static ngx_str_t err_levels[] = {
    ngx_null_string,
    ngx_string("emerg"),
    ngx_string("alert"),
    ngx_string("crit"),
    ngx_string("error"),
    ngx_string("warn"),
    ngx_string("notice"),
    ngx_string("info"),
    ngx_string("debug")
};

static const char *debug_levels[] = {
    "debug_core", "debug_alloc", "debug_mutex", "debug_event",
    "debug_http", "debug_mail", "debug_stream"
};


{% endhighlight %}

这里定义了error_log模块```ngx_errlog_module```，该模块对应的上下文为```ngx_errlog_module_ctx```，该模块对应的指令为```ngx_errlog_commands```。


接着定义了3个变量：

* ```ngx_log```: 作为nginx中初始log对象

* ```ngx_log_file```: nginx初始log对象所关联的日志文件对象

* ```ngx_use_stderr```: 表明是否使用stderr标准错误作为日志对象的输出位置。


最后定义了日志错误级别及debug级别。

## 3. nginx中的error_log指令

这里在介绍nginx log之前，我们再介绍一下```error_log```指令：
<pre>
Syntax: 	error_log file [level];
Default: 	

error_log logs/error.log error;

Context: 	main, http, mail, stream, server, location
</pre>
上述指令用于配置日志。在同一level层级（main/http/mail/stream/server/location)可以配置多个日志。假如在main配置层级并未显示的指定日志的存放位置，那么此时会采用默认的文件来进行存放。

上述指令的第一个参数```file```用于定义log的存放位置。需要指出的是如果file被指定为特殊值```stderr```时，则日志会被输出到标准错误。如果需要将日志写到```syslog```的话，则配置的前缀为```syslog:```。而如果需要将日志写到一个```循环内存buffer```中，则使用```memory```加上一个```size```大小，将日志写到内存一般用在调试阶段。

第二个参数```level```定义日志的打印级别，可取值有:debug, info, notice, warn, error, crit, alert, 或者emerg。前面列出的log级别是按照错误的严重顺序递增排列的，大于等于该指定级别的日志都会被打印出来。例如，将日志的打印级别设置为默认值error，则error，crit,alert，emerg消息都会被记录。当```level```参数被省略时，会采用默认的error级别。

注意：要想使debug级别日志工作，则在nginx编译源代码时必须添加```--with-debug```选项。

## 4. 函数ngx_log_error_core()
{% highlight string %}
#if (NGX_HAVE_VARIADIC_MACROS)

void
ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, ...)

#else

void
ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, va_list args)

#endif
{
#if (NGX_HAVE_VARIADIC_MACROS)
    va_list      args;
#endif
    u_char      *p, *last, *msg;
    ssize_t      n;
    ngx_uint_t   wrote_stderr, debug_connection;
    u_char       errstr[NGX_MAX_ERROR_STR];

    last = errstr + NGX_MAX_ERROR_STR;

    p = ngx_cpymem(errstr, ngx_cached_err_log_time.data,
                   ngx_cached_err_log_time.len);

    p = ngx_slprintf(p, last, " [%V] ", &err_levels[level]);

    /* pid#tid */
    p = ngx_slprintf(p, last, "%P#" NGX_TID_T_FMT ": ",
                    ngx_log_pid, ngx_log_tid);

    if (log->connection) {
        p = ngx_slprintf(p, last, "*%uA ", log->connection);
    }

    msg = p;

#if (NGX_HAVE_VARIADIC_MACROS)

    va_start(args, fmt);
    p = ngx_vslprintf(p, last, fmt, args);
    va_end(args);

#else

    p = ngx_vslprintf(p, last, fmt, args);

#endif

    if (err) {
        p = ngx_log_errno(p, last, err);
    }

    if (level != NGX_LOG_DEBUG && log->handler) {
        p = log->handler(log, p, last - p);
    }

    if (p > last - NGX_LINEFEED_SIZE) {
        p = last - NGX_LINEFEED_SIZE;
    }

    ngx_linefeed(p);

    wrote_stderr = 0;
    debug_connection = (log->log_level & NGX_LOG_DEBUG_CONNECTION) != 0;

    while (log) {

        if (log->log_level < level && !debug_connection) {
            break;
        }

        if (log->writer) {
            log->writer(log, level, errstr, p - errstr);
            goto next;
        }

        if (ngx_time() == log->disk_full_time) {

            /*
             * on FreeBSD writing to a full filesystem with enabled softupdates
             * may block process for much longer time than writing to non-full
             * filesystem, so we skip writing to a log for one second
             */

            goto next;
        }

        n = ngx_write_fd(log->file->fd, errstr, p - errstr);

        if (n == -1 && ngx_errno == NGX_ENOSPC) {
            log->disk_full_time = ngx_time();
        }

        if (log->file->fd == ngx_stderr) {
            wrote_stderr = 1;
        }

    next:

        log = log->next;
    }

    if (!ngx_use_stderr
        || level > NGX_LOG_WARN
        || wrote_stderr)
    {
        return;
    }

    msg -= (7 + err_levels[level].len + 3);

    (void) ngx_sprintf(msg, "nginx: [%V] ", &err_levels[level]);

    (void) ngx_write_console(ngx_stderr, msg, p - msg);
}

{% endhighlight %}

当前我们支持c99可变参数宏。本函数是进行错误日志打印的核心函数，下面我们简要分析一下该函数的实现：
{% highlight string %}
#if (NGX_HAVE_VARIADIC_MACROS)

void
ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, ...)

#else

void
ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, va_list args)

#endif
{
    //1) 将缓存的日志时间存入errstr

    //2) 格式化日志级别到errstr

    //3) 将当前进程ID(pid)及线程ID(tid)格式化进errstr
    
    //4) 将当前引用该log的connection数目格式化进errstr中
   
    //5) 格式化函数传递进来的参数到errstr

    //6) 格式化err参数

    //7) 对log->data进行格式化

    //8) 换行符

    //9) 循环遍历log链，将日志输出到log->fd

    //10) 将相应的参数日志信息输出到标准错误(stderr)


}
{% endhighlight %}

## 5. 普通日志及debug日志输出函数
{% highlight string %} 
#if !(NGX_HAVE_VARIADIC_MACROS)

void ngx_cdecl
ngx_log_error(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, ...)
{
    va_list  args;

    if (log->log_level >= level) {
        va_start(args, fmt);
        ngx_log_error_core(level, log, err, fmt, args);
        va_end(args);
    }
}


void ngx_cdecl
ngx_log_debug_core(ngx_log_t *log, ngx_err_t err, const char *fmt, ...)
{
    va_list  args;

    va_start(args, fmt);
    ngx_log_error_core(NGX_LOG_DEBUG, log, err, fmt, args);
    va_end(args);
}

#endif
{% endhighlight %}

上面两个函数主要是为了处理编译器不支持可变参数宏的情况下普通日志及debug日志的输出。

## 6. 函数ngx_log_abort()
{% highlight string %}
void ngx_cdecl
ngx_log_abort(ngx_err_t err, const char *fmt, ...)
{
    u_char   *p;
    va_list   args;
    u_char    errstr[NGX_MAX_CONF_ERRSTR];

    va_start(args, fmt);
    p = ngx_vsnprintf(errstr, sizeof(errstr) - 1, fmt, args);
    va_end(args);

    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, err,
                  "%*s", p - errstr, errstr);
}
{% endhighlight %}
用于输出alert级别的日志。

## 7. 函数ngx_log_stderr()
{% highlight string %}
void ngx_cdecl
ngx_log_stderr(ngx_err_t err, const char *fmt, ...)
{
    u_char   *p, *last;
    va_list   args;
    u_char    errstr[NGX_MAX_ERROR_STR];

    last = errstr + NGX_MAX_ERROR_STR;

    p = ngx_cpymem(errstr, "nginx: ", 7);

    va_start(args, fmt);
    p = ngx_vslprintf(p, last, fmt, args);
    va_end(args);

    if (err) {
        p = ngx_log_errno(p, last, err);
    }

    if (p > last - NGX_LINEFEED_SIZE) {
        p = last - NGX_LINEFEED_SIZE;
    }

    ngx_linefeed(p);

    (void) ngx_write_console(ngx_stderr, errstr, p - errstr);
}
{% endhighlight %}
将相应的日志输出到stderr

## 8. 函数ngx_log_errno()
{% highlight string %}
u_char *
ngx_log_errno(u_char *buf, u_char *last, ngx_err_t err)
{
    if (buf > last - 50) {

        /* leave a space for an error code */

        buf = last - 50;
        *buf++ = '.';
        *buf++ = '.';
        *buf++ = '.';
    }

#if (NGX_WIN32)
    buf = ngx_slprintf(buf, last, ((unsigned) err < 0x80000000)
                                       ? " (%d: " : " (%Xd: ", err);
#else
    buf = ngx_slprintf(buf, last, " (%d: ", err);
#endif

    buf = ngx_strerror(err, buf, last - buf);

    if (buf < last) {
        *buf++ = ')';
    }

    return buf;
}
{% endhighlight %}
将```ngx_err_t```类型的错误码格式化到buf中。


<br />
<br />

**[参考]**

1. [git的使用](https://www.yiibai.com/git/git-quick-start.html)

2. [bit book](https://git-scm.com/book/zh/v2)

3. [nginx error log](http://nginx.org/en/docs/ngx_core_module.html#error_log)
<br />
<br />
<br />

