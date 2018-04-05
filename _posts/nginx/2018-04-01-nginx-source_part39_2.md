---
layout: post
title: core/ngx_file.c源文件分析(1)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节我们讲述一下nginx中对所涉及到的文件操作。


<!-- more -->

## 1. 相关变量定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


static ngx_int_t ngx_test_full_name(ngx_str_t *name);


static ngx_atomic_t   temp_number = 0;
ngx_atomic_t         *ngx_temp_number = &temp_number;
ngx_atomic_int_t      ngx_random_number = 123456;
{% endhighlight %}

* 函数```ngx_test_full_name()```用于判断```name```是否为一个绝对路径。

* 变量```temp_number```作为一个临时的占位符空间

* 变量```ngx_temp_number```刚开始指向```temp_number```,而后在初始化event模块时（ngx_event_module_init()函数)，指向一个共享内存空间，在整个Nginx中作为一个临时值使用

* 变量ngx_random_number作为随机值使用，默认初始化值为123456，而在event模块初始化时，被设置为：
{% highlight string %}
ngx_random_number = (tp->msec << 16) + ngx_pid;
{% endhighlight %}



## 2. 函数ngx_get_full_name()
{% highlight string %}
ngx_int_t
ngx_get_full_name(ngx_pool_t *pool, ngx_str_t *prefix, ngx_str_t *name)
{
    size_t      len;
    u_char     *p, *n;
    ngx_int_t   rc;

    rc = ngx_test_full_name(name);

    if (rc == NGX_OK) {
        return rc;
    }

    len = prefix->len;

#if (NGX_WIN32)

    if (rc == 2) {
        len = rc;
    }

#endif

    n = ngx_pnalloc(pool, len + name->len + 1);
    if (n == NULL) {
        return NGX_ERROR;
    }

    p = ngx_cpymem(n, prefix->data, len);
    ngx_cpystrn(p, name->data, name->len + 1);

    name->len += len;
    name->data = n;

    return NGX_OK;
}
{% endhighlight %}
这里先判断```name```是否是一个绝对路径，如果是则直接返回OK；否则开辟一块空间，将```name```追加到```prefix```后（注意，这里会以'\0'结尾)


## 3. 函数ngx_test_full_name()
{% highlight string %}
static ngx_int_t
ngx_test_full_name(ngx_str_t *name)
{
#if (NGX_WIN32)
    u_char  c0, c1;

    c0 = name->data[0];

    if (name->len < 2) {
        if (c0 == '/') {
            return 2;
        }

        return NGX_DECLINED;
    }

    c1 = name->data[1];

    if (c1 == ':') {
        c0 |= 0x20;

        if ((c0 >= 'a' && c0 <= 'z')) {
            return NGX_OK;
        }

        return NGX_DECLINED;
    }

    if (c1 == '/') {
        return NGX_OK;
    }

    if (c0 == '/') {
        return 2;
    }

    return NGX_DECLINED;

#else

    if (name->data[0] == '/') {
        return NGX_OK;
    }

    return NGX_DECLINED;

#endif
}
{% endhighlight %}
在WIN32上判断是否是一个绝对路径，例如```C:```，又或者是以类似于```<charactor>/```这样开头的路径； 对于在其他操作系统环境上则直接判断是否为绝对路径。在当前环境下，我们并不支持```NGX_WIN32```宏定义。

## 4. 函数ngx_write_chain_to_temp_file()
{% highlight string %}
ssize_t
ngx_write_chain_to_temp_file(ngx_temp_file_t *tf, ngx_chain_t *chain)
{
    ngx_int_t  rc;

    if (tf->file.fd == NGX_INVALID_FILE) {
        rc = ngx_create_temp_file(&tf->file, tf->path, tf->pool,
                                  tf->persistent, tf->clean, tf->access);

        if (rc != NGX_OK) {
            return rc;
        }

        if (tf->log_level) {
            ngx_log_error(tf->log_level, tf->file.log, 0, "%s %V",
                          tf->warn, &tf->file.name);
        }
    }

#if (NGX_THREADS && NGX_HAVE_PWRITEV)

    if (tf->thread_write) {
        return ngx_thread_write_chain_to_file(&tf->file, chain, tf->offset,
                                              tf->pool);
    }

#endif

    return ngx_write_chain_to_file(&tf->file, chain, tf->offset, tf->pool);
}
{% endhighlight %}
这里首先判断```tf->file.fd```是否为一个有效的文件句柄，如果不是则先创建一个临时文件。接着将```chain```中的数据写入到```tf```临时文件中。

## 5. 函数ngx_create_temp_file()
{% highlight string %}
ngx_int_t
ngx_create_temp_file(ngx_file_t *file, ngx_path_t *path, ngx_pool_t *pool,
    ngx_uint_t persistent, ngx_uint_t clean, ngx_uint_t access)
{
    uint32_t                  n;
    ngx_err_t                 err;
    ngx_pool_cleanup_t       *cln;
    ngx_pool_cleanup_file_t  *clnf;

    file->name.len = path->name.len + 1 + path->len + 10;

    file->name.data = ngx_pnalloc(pool, file->name.len + 1);
    if (file->name.data == NULL) {
        return NGX_ERROR;
    }

#if 0
    for (i = 0; i < file->name.len; i++) {
        file->name.data[i] = 'X';
    }
#endif

    ngx_memcpy(file->name.data, path->name.data, path->name.len);

    n = (uint32_t) ngx_next_temp_number(0);

    cln = ngx_pool_cleanup_add(pool, sizeof(ngx_pool_cleanup_file_t));
    if (cln == NULL) {
        return NGX_ERROR;
    }

    for ( ;; ) {
        (void) ngx_sprintf(file->name.data + path->name.len + 1 + path->len,
                           "%010uD%Z", n);

        ngx_create_hashed_filename(path, file->name.data, file->name.len);

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, file->log, 0,
                       "hashed path: %s", file->name.data);

        file->fd = ngx_open_tempfile(file->name.data, persistent, access);

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, file->log, 0,
                       "temp fd:%d", file->fd);

        if (file->fd != NGX_INVALID_FILE) {

            cln->handler = clean ? ngx_pool_delete_file : ngx_pool_cleanup_file;
            clnf = cln->data;

            clnf->fd = file->fd;
            clnf->name = file->name.data;
            clnf->log = pool->log;

            return NGX_OK;
        }

        err = ngx_errno;

        if (err == NGX_EEXIST_FILE) {
            n = (uint32_t) ngx_next_temp_number(1);
            continue;
        }

        if ((path->level[0] == 0) || (err != NGX_ENOPATH)) {
            ngx_log_error(NGX_LOG_CRIT, file->log, err,
                          ngx_open_tempfile_n " \"%s\" failed",
                          file->name.data);
            return NGX_ERROR;
        }

        if (ngx_create_path(file, path) == NGX_ERROR) {
            return NGX_ERROR;
        }
    }
}
{% endhighlight %}
本函数用于创建一个临时文件，具体创建步骤如下：

* 为file->name分配空间，空间大小为：
<pre>
file->name.len = path->name.len + 1 + path->len + 10;
</pre>
这里path->len为文件的多级目录结构长度，之所以要再加10，是因为创建临时文件会加一个随机值到文件名后（uint32_t所表示的数最大长度为10个字节)

* 接着创建一个ngx_pool_cleanup_t数据结构，以再后续对临时文件清除时调用

* 产生临时文件名，并创建临时文件
{% highlight string %}
(void) ngx_sprintf(file->name.data + path->name.len + 1 + path->len,
                           "%010uD%Z", n);

ngx_create_hashed_filename(path, file->name.data, file->name.len);

ngx_log_debug1(NGX_LOG_DEBUG_CORE, file->log, 0,
               "hashed path: %s", file->name.data);

file->fd = ngx_open_tempfile(file->name.data, persistent, access);
{% endhighlight %}
这里首先将产生的整数```n```格式化为宽度为10的字符串，以```'\0'```结尾，到目前为止,```file->name```为如下：

![ngx-file-data](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_file_data.jpg)

然后创建hash文件名，最后再创建临时文件。

## 6. 函数ngx_create_hashed_filename()
{% highlight string %}
void
ngx_create_hashed_filename(ngx_path_t *path, u_char *file, size_t len)
{
    size_t      i, level;
    ngx_uint_t  n;

    i = path->name.len + 1;

    file[path->name.len + path->len]  = '/';

    for (n = 0; n < 3; n++) {
        level = path->level[n];

        if (level == 0) {
            break;
        }

        len -= level;
        file[i - 1] = '/';
        ngx_memcpy(&file[i], &file[len], level);
        i += level + 1;
    }
}
{% endhighlight %}

这里首先在```path->name.len+path->len```为止处添加'/'字符，然后分别产生每一级的Hash路径。

## 7. 函数ngx_create_path()
{% highlight string %}
ngx_int_t
ngx_create_path(ngx_file_t *file, ngx_path_t *path)
{
    size_t      pos;
    ngx_err_t   err;
    ngx_uint_t  i;

    pos = path->name.len;

    for (i = 0; i < 3; i++) {
        if (path->level[i] == 0) {
            break;
        }

        pos += path->level[i] + 1;

        file->name.data[pos] = '\0';

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, file->log, 0,
                       "temp file: \"%s\"", file->name.data);

        if (ngx_create_dir(file->name.data, 0700) == NGX_FILE_ERROR) {
            err = ngx_errno;
            if (err != NGX_EEXIST) {
                ngx_log_error(NGX_LOG_CRIT, file->log, err,
                              ngx_create_dir_n " \"%s\" failed",
                              file->name.data);
                return NGX_ERROR;
            }
        }

        file->name.data[pos] = '/';
    }

    return NGX_OK;
}

{% endhighlight %}
这里分别创建path中的每一级路径。


## 8. 函数ngx_create_full_path()
{% highlight string %}
ngx_err_t
ngx_create_full_path(u_char *dir, ngx_uint_t access)
{
    u_char     *p, ch;
    ngx_err_t   err;

    err = 0;

#if (NGX_WIN32)
    p = dir + 3;
#else
    p = dir + 1;
#endif

    for ( /* void */ ; *p; p++) {
        ch = *p;

        if (ch != '/') {
            continue;
        }

        *p = '\0';

        if (ngx_create_dir(dir, access) == NGX_FILE_ERROR) {
            err = ngx_errno;

            switch (err) {
            case NGX_EEXIST:
                err = 0;
            case NGX_EACCES:
                break;

            default:
                return err;
            }
        }

        *p = '/';
    }

    return err;
}
{% endhighlight %}

这里以'/'作为分界符，分别创建每一级路径。

## 9. 函数ngx_next_temp_number()
{% highlight string %}
ngx_atomic_uint_t
ngx_next_temp_number(ngx_uint_t collision)
{
    ngx_atomic_uint_t  n, add;

    add = collision ? ngx_random_number : 1;

    n = ngx_atomic_fetch_add(ngx_temp_number, add);

    return n + add;
}
{% endhighlight %}

这里根据collision的条件，每次对```ngx_temp_number```增加一个值，然后返回.


<br />
<br />

**[参考]**

1. [nginx文件结构](https://blog.csdn.net/apelife/article/details/53043275)

2. [Nginx中目录树的遍历](https://blog.csdn.net/weiyuefei/article/details/38313663)

<br />
<br />
<br />

