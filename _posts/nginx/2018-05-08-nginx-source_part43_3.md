---
layout: post
title: core/ngx_log.c源文件分析(2)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们主要讲述一下nginx中日志的相关实现。


<!-- more -->


## 1. 函数ngx_log_init()
{% highlight string %}
ngx_log_t *
ngx_log_init(u_char *prefix)
{
    u_char  *p, *name;
    size_t   nlen, plen;

    ngx_log.file = &ngx_log_file;
    ngx_log.log_level = NGX_LOG_NOTICE;

    name = (u_char *) NGX_ERROR_LOG_PATH;

    /*
     * we use ngx_strlen() here since BCC warns about
     * condition is always false and unreachable code
     */

    nlen = ngx_strlen(name);

    if (nlen == 0) {
        ngx_log_file.fd = ngx_stderr;
        return &ngx_log;
    }

    p = NULL;

#if (NGX_WIN32)
    if (name[1] != ':') {
#else
    if (name[0] != '/') {
#endif

        if (prefix) {
            plen = ngx_strlen(prefix);

        } else {
#ifdef NGX_PREFIX
            prefix = (u_char *) NGX_PREFIX;
            plen = ngx_strlen(prefix);
#else
            plen = 0;
#endif
        }

        if (plen) {
            name = malloc(plen + nlen + 2);
            if (name == NULL) {
                return NULL;
            }

            p = ngx_cpymem(name, prefix, plen);

            if (!ngx_path_separator(*(p - 1))) {
                *p++ = '/';
            }

            ngx_cpystrn(p, (u_char *) NGX_ERROR_LOG_PATH, nlen + 1);

            p = name;
        }
    }

    ngx_log_file.fd = ngx_open_file(name, NGX_FILE_APPEND,
                                    NGX_FILE_CREATE_OR_OPEN,
                                    NGX_FILE_DEFAULT_ACCESS);

    if (ngx_log_file.fd == NGX_INVALID_FILE) {
        ngx_log_stderr(ngx_errno,
                       "[alert] could not open error log file: "
                       ngx_open_file_n " \"%s\" failed", name);
#if (NGX_WIN32)
        ngx_event_log(ngx_errno,
                       "could not open error log file: "
                       ngx_open_file_n " \"%s\" failed", name);
#endif

        ngx_log_file.fd = ngx_stderr;
    }

    if (p) {
        ngx_free(p);
    }

    return &ngx_log;
}
{% endhighlight %}
本函数用于初始化nginx中的第一个log对象。下面我们简要介绍一下该函数：
{% highlight string %}
ngx_log_t *
ngx_log_init(u_char *prefix)
{
    //在调用本函数初始化时,prefix默认取值为ngx_prefix，一般通过-p参数指定，因此这里为NULL

   //1) 初始化ngx_log_file: 这里NGX_ERROR_LOG_PATH值为logs/error.log, NGX_PREFIX值为/usr/local/nginx/，因此
   // 会使用/usr/local/nginx/logs/error.log作为日志文件(注： 如果未指定NGX_ERROR_LOG_PATH，则默认采用ngx_stderr作为输出)

   //2) 打开或创建上述文件
  
}
{% endhighlight %}


## 2. ngx_log_open_default()函数
{% highlight string %}
ngx_int_t
ngx_log_open_default(ngx_cycle_t *cycle)
{
    ngx_log_t         *log;
    static ngx_str_t   error_log = ngx_string(NGX_ERROR_LOG_PATH);

    if (ngx_log_get_file_log(&cycle->new_log) != NULL) {
        return NGX_OK;
    }

    if (cycle->new_log.log_level != 0) {
        /* there are some error logs, but no files */

        log = ngx_pcalloc(cycle->pool, sizeof(ngx_log_t));
        if (log == NULL) {
            return NGX_ERROR;
        }

    } else {
        /* no error logs at all */
        log = &cycle->new_log;
    }

    log->log_level = NGX_LOG_ERR;

    log->file = ngx_conf_open_file(cycle, &error_log);
    if (log->file == NULL) {
        return NGX_ERROR;
    }

    if (log != &cycle->new_log) {
        ngx_log_insert(&cycle->new_log, log);
    }

    return NGX_OK;
}
{% endhighlight %}
本函数用于打开一个默认的log以作为后期日志输出。 这里我们首先介绍一下```cycle->new_log```: 因为在nginx初始化时会默认采用一个log，用于打印相应的提示信息，而在完成初始化之后，会初始化一个新的log作为默认log。下面我们简要介绍一下本函数：
{% highlight string %}
ngx_int_t
ngx_log_open_default(ngx_cycle_t *cycle)
{
   //1) 从cycle->new_log链中查找第一个file类型log，如果找到则把其作为默认log返回； 否则执行如下

  //2) 获得一个log对象，然后将其log_level设置为NGX_LOG_ERR，并将关联到NGX_ERROR_LOG_PATH文件，然后再插入到cycle->new_log链中
  //(说明： 这里NGX_ERROR_LOG_PATH值为logs/error.log)
}
{% endhighlight %}


## 3. 函数ngx_log_redirect_stderr()
{% highlight string %}
ngx_int_t
ngx_log_redirect_stderr(ngx_cycle_t *cycle)
{
    ngx_fd_t  fd;

    if (cycle->log_use_stderr) {
        return NGX_OK;
    }

    /* file log always exists when we are called */
    fd = ngx_log_get_file_log(cycle->log)->file->fd;

    if (fd != ngx_stderr) {
        if (ngx_set_stderr(fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_errno,
                          ngx_set_stderr_n " failed");

            return NGX_ERROR;
        }
    }

    return NGX_OK;
}
{% endhighlight %}
本函数用于将标准错误重定向到某个文件。这里首先会检查```cycle->log_use_stderr```，如果该变量为值为1，则表明当前需要使用stderr，不需要进行重定向；否则，则会将stderr重定向到日志链的一个文件日志中。


## 4. 函数ngx_log_get_file_log() 
{% highlight string %}
ngx_log_t *
ngx_log_get_file_log(ngx_log_t *head)
{
    ngx_log_t  *log;

    for (log = head; log; log = log->next) {
        if (log->file != NULL) {
            return log;
        }
    }

    return NULL;
}
{% endhighlight %}
本函数用于从一个日志链中获取到第一个文件日志。

## 5. 函数ngx_log_set_levels() 
{% highlight string %}
static char *
ngx_log_set_levels(ngx_conf_t *cf, ngx_log_t *log)
{
    ngx_uint_t   i, n, d, found;
    ngx_str_t   *value;

    if (cf->args->nelts == 2) {
        log->log_level = NGX_LOG_ERR;
        return NGX_CONF_OK;
    }

    value = cf->args->elts;

    for (i = 2; i < cf->args->nelts; i++) {
        found = 0;

        for (n = 1; n <= NGX_LOG_DEBUG; n++) {
            if (ngx_strcmp(value[i].data, err_levels[n].data) == 0) {

                if (log->log_level != 0) {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "duplicate log level \"%V\"",
                                       &value[i]);
                    return NGX_CONF_ERROR;
                }

                log->log_level = n;
                found = 1;
                break;
            }
        }

        for (n = 0, d = NGX_LOG_DEBUG_FIRST; d <= NGX_LOG_DEBUG_LAST; d <<= 1) {
            if (ngx_strcmp(value[i].data, debug_levels[n++]) == 0) {
                if (log->log_level & ~NGX_LOG_DEBUG_ALL) {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "invalid log level \"%V\"",
                                       &value[i]);
                    return NGX_CONF_ERROR;
                }

                log->log_level |= d;
                found = 1;
                break;
            }
        }


        if (!found) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid log level \"%V\"", &value[i]);
            return NGX_CONF_ERROR;
        }
    }

    if (log->log_level == NGX_LOG_DEBUG) {
        log->log_level = NGX_LOG_DEBUG_ALL;
    }

    return NGX_CONF_OK;
}
{% endhighlight %}
本函数用于设置日志的级别。下面我们简要分析一下该函数：

{% highlight string %}
static char *
ngx_log_set_levels(ngx_conf_t *cf, ngx_log_t *log)
{
    //从cf->args[2]开始就是error_log的级别

    //1) 如果没有设置日志级别，则设置为默认的NGX_LOG_ERR

    
    //2) 遍历日志参数
    for (i = 2; i < cf->args->nelts; i++) {
    {
        //2.1) 检查设置的是否是emerg/alert/crit/error/warn/notice/info/debug这样的大级别

        //2.2) 检查设置是否是debug中的小级别：debug_core/debug_alloc/debug_mutex/
        //debug_event/debug_http/debug_mail/debug_stream

    }

    //3) 如果log->log_level被设置为NGX_LOG_DEBUG这样一个大级别，则会被修改为NGX_LOG_DEBUG_ALL
}
{% endhighlight %}



## 6. 函数ngx_error_log()
{% highlight string %}
static char *
ngx_error_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_log_t  *dummy;

    dummy = &cf->cycle->new_log;

    return ngx_log_set_log(cf, &dummy);
}
{% endhighlight %}

在解析到```error_log```指令时，调用本函数用于设置log

## 7. 函数ngx_log_set_log()
{% highlight string %}
char *
ngx_log_set_log(ngx_conf_t *cf, ngx_log_t **head)
{
    ngx_log_t          *new_log;
    ngx_str_t          *value, name;
    ngx_syslog_peer_t  *peer;

    if (*head != NULL && (*head)->log_level == 0) {
        new_log = *head;

    } else {

        new_log = ngx_pcalloc(cf->pool, sizeof(ngx_log_t));
        if (new_log == NULL) {
            return NGX_CONF_ERROR;
        }

        if (*head == NULL) {
            *head = new_log;
        }
    }

    value = cf->args->elts;

    if (ngx_strcmp(value[1].data, "stderr") == 0) {
        ngx_str_null(&name);
        cf->cycle->log_use_stderr = 1;

        new_log->file = ngx_conf_open_file(cf->cycle, &name);
        if (new_log->file == NULL) {
            return NGX_CONF_ERROR;
        }

    } else if (ngx_strncmp(value[1].data, "memory:", 7) == 0) {

#if (NGX_DEBUG)
        size_t                 size, needed;
        ngx_pool_cleanup_t    *cln;
        ngx_log_memory_buf_t  *buf;

        value[1].len -= 7;
        value[1].data += 7;

        needed = sizeof("MEMLOG  :" NGX_LINEFEED)
                 + cf->conf_file->file.name.len
                 + NGX_SIZE_T_LEN
                 + NGX_INT_T_LEN
                 + NGX_MAX_ERROR_STR;

        size = ngx_parse_size(&value[1]);

        if (size == (size_t) NGX_ERROR || size < needed) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid buffer size \"%V\"", &value[1]);
            return NGX_CONF_ERROR;
        }

        buf = ngx_pcalloc(cf->pool, sizeof(ngx_log_memory_buf_t));
        if (buf == NULL) {
            return NGX_CONF_ERROR;
        }

        buf->start = ngx_pnalloc(cf->pool, size);
        if (buf->start == NULL) {
            return NGX_CONF_ERROR;
        }

        buf->end = buf->start + size;

        buf->pos = ngx_slprintf(buf->start, buf->end, "MEMLOG %uz %V:%ui%N",
                                size, &cf->conf_file->file.name,
                                cf->conf_file->line);

        ngx_memset(buf->pos, ' ', buf->end - buf->pos);

        cln = ngx_pool_cleanup_add(cf->pool, 0);
        if (cln == NULL) {
            return NGX_CONF_ERROR;
        }

        cln->data = new_log;
        cln->handler = ngx_log_memory_cleanup;

        new_log->writer = ngx_log_memory_writer;
        new_log->wdata = buf;

#else
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "nginx was built without debug support");
        return NGX_CONF_ERROR;
#endif

    } else if (ngx_strncmp(value[1].data, "syslog:", 7) == 0) {
        peer = ngx_pcalloc(cf->pool, sizeof(ngx_syslog_peer_t));
        if (peer == NULL) {
            return NGX_CONF_ERROR;
        }

        if (ngx_syslog_process_conf(cf, peer) != NGX_CONF_OK) {
            return NGX_CONF_ERROR;
        }

        new_log->writer = ngx_syslog_writer;
        new_log->wdata = peer;

    } else {
        new_log->file = ngx_conf_open_file(cf->cycle, &value[1]);
        if (new_log->file == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    if (ngx_log_set_levels(cf, new_log) != NGX_CONF_OK) {
        return NGX_CONF_ERROR;
    }

    if (*head != new_log) {
        ngx_log_insert(*head, new_log);
    }

    return NGX_CONF_OK;
}

{% endhighlight %}
本函数用于在解析到```error_log```指令时，向nginx log链中插入一个新的```ngx_log_t```对象。下面我们简要分析一下该函数：
{% highlight string %}
char *
ngx_log_set_log(ngx_conf_t *cf, ngx_log_t **head)
{
    //1) 获得一个ngx_log_t对象（新构造一个或者采用一个head链符合条件的一个）

    //2) 分别处理以下几种不同的日志类型
    if (ngx_strcmp(value[1].data, "stderr") == 0) 
    {
       //3) 日志输出到标准错误，将log_use_stderr设置为1，并且cf->log->file关联一个空的ngx_open_file_t对象
    } 
    else if(ngx_strncmp(value[1].data, "memory:", 7) == 0)
    {
       //4) 只在NGX_DEBUG条件下使用，用于调试。关于memory循环内存buffer，一般配置为如下：
       // error_log memory:32m debug;
     
       //这里首先解析上述指令，然后分配一块足够大的内存buf，并将该buf分成两个部分：
       // buf->start~buf->pos部分，这一段存放一些标示信息
       // buf->pos~buf->end部分作为一个循环内存，用于存放日志

       //对内存日志会关联一个内存池清理函数： ngx_log_memory_cleanup()
       //还会关联一个内存写函数：ngx_log_memory_writer()
      
    }
    else if(ngx_strncmp(value[1].data, "syslog:", 7) == 0)
    {
       //5） 关于syslog日志，一般配置情形如下：

       //error_log syslog:server=192.168.1.1 debug;

       //access_log syslog:server=unix:/var/log/nginx.sock,nohostname;
       //access_log syslog:server=[2001:db8::1]:12345,facility=local7,tag=nginx,severity=info combined;

       //我们后面会对syslog日志进行详细讲解
    }
    else{

       //6） 普通的文件日志，关联一个普通的文件对象
    }

    //7) 设置日志级别

    //8) 插入到日志链中 
}
{% endhighlight %}

## 8. 函数ngx_log_insert()
{% highlight string %}
static void
ngx_log_insert(ngx_log_t *log, ngx_log_t *new_log)
{
    ngx_log_t  tmp;

    if (new_log->log_level > log->log_level) {

        /*
         * list head address is permanent, insert new log after
         * head and swap its contents with head
         */

        tmp = *log;
        *log = *new_log;
        *new_log = tmp;

        log->next = new_log;
        return;
    }

    while (log->next) {
        if (new_log->log_level > log->next->log_level) {
            new_log->next = log->next;
            log->next = new_log;
            return;
        }

        log = log->next;
    }

    log->next = new_log;
}

{% endhighlight %}
log链是按```log->log_level```从大到小的顺序排列的，本函数用于将一个新的```ngx_log_t```对象插入到一个已存在的log链中。（注意： 一般log链的头是一个固定地址，因此这里插入到头部时用了一个小技巧）



## 9. 函数ngx_log_memory_writer()
{% highlight string %}
#if (NGX_DEBUG)

static void
ngx_log_memory_writer(ngx_log_t *log, ngx_uint_t level, u_char *buf,
    size_t len)
{
    u_char                *p;
    size_t                 avail, written;
    ngx_log_memory_buf_t  *mem;

    mem = log->wdata;

    if (mem == NULL) {
        return;
    }

    written = ngx_atomic_fetch_add(&mem->written, len);

    p = mem->pos + written % (mem->end - mem->pos);

    avail = mem->end - p;

    if (avail >= len) {
        ngx_memcpy(p, buf, len);

    } else {
        ngx_memcpy(p, buf, avail);
        ngx_memcpy(mem->pos, buf + avail, len - avail);
    }
}

#endif
{% endhighlight %}

上面函数较为简单，就是往循环memory buf中写日志数据。这里需要注意的一点是：
<pre>
written = ngx_atomic_fetch_add(&mem->written, len);
</pre>
当多线程向buf中写时，通过如上实现原子操作。

![ngx-memory-log](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_memory_log.jpg)

上面```memory log指示```区域大概内容如下：
{% highlight string %}
MEMLOG <total_bufsize> <nginx_conf_filename>:<errorlog_line_in_ngxconf><feedline>
{% endhighlight %}


## 10. 函数ngx_log_memory_cleanup()
{% highlight string %}
#if (NGX_DEBUG)
static void
ngx_log_memory_cleanup(void *data)
{
    ngx_log_t *log = data;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, log, 0, "destroy memory log buffer");

    log->wdata = NULL;
}

#endif

{% endhighlight %}
用于销毁memory log缓存。这里注意，由于目前我们使用的是基于pool的内存，这里我们并不会真正释放内存。实际上，nginx内部基本上只有在整个进程退出时由操作系统来自动释放内存。



<br />
<br />

**[参考]**

1. [nginx error log](http://nginx.org/en/docs/ngx_core_module.html#error_log)




<br />
<br />
<br />

