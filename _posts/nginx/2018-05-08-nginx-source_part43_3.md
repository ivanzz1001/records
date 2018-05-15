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
本函数用于设置日志的级别。


<br />
<br />

**[参考]**

1. [git的使用](https://www.yiibai.com/git/git-quick-start.html)

2. [bit book](https://git-scm.com/book/zh/v2)

3. [nginx error log](http://nginx.org/en/docs/ngx_core_module.html#error_log)
<br />
<br />
<br />

