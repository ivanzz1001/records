---
layout: post
title: os/unix/ngx_files.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节主要分析一下ngx_files.c源文件代码，其主要完成对所有关于文件操作的底层封装。

<!-- more -->


<br />
<br />


## 1. 相关函数声明
在这里主要对一些静态函数及全局变量进行声明：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_THREADS)
#include <ngx_thread_pool.h>
static void ngx_thread_read_handler(void *data, ngx_log_t *log);
static void ngx_thread_write_chain_to_file_handler(void *data, ngx_log_t *log);
#endif

static ngx_chain_t *ngx_chain_to_iovec(ngx_iovec_t *vec, ngx_chain_t *cl);
static ssize_t ngx_writev_file(ngx_file_t *file, ngx_iovec_t *vec,
    off_t offset);


#if (NGX_HAVE_FILE_AIO)

ngx_uint_t  ngx_file_aio = 1;

#endif
{% endhighlight %}
当前，我们并不支持```NGX_THREAD```与```NGX_HAVE_FILE_AIO```。
<pre>
static ngx_chain_t *ngx_chain_to_iovec(ngx_iovec_t *vec, ngx_chain_t *cl);
static ssize_t ngx_writev_file(ngx_file_t *file, ngx_iovec_t *vec,
    off_t offset);
</pre>
其中，```ngx_chain_to_iovec()```函数主要用于将ngx_chain_t中的数据写到ngx_iovec_t中；

```ngx_writev_file()```函数主要用于将ngx_iovec_t中的数据写到ngx_file_t的offset处。


## 2. ngx_read_file()函数
```ngx_read_file()```函数用于从file文件的offset处读取size字节的数据到buf中。
{% highlight string %}
ssize_t
ngx_read_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset)
{
    ssize_t  n;

    ngx_log_debug4(NGX_LOG_DEBUG_CORE, file->log, 0,
                   "read: %d, %p, %uz, %O", file->fd, buf, size, offset);

#if (NGX_HAVE_PREAD)

    n = pread(file->fd, buf, size, offset);

    if (n == -1) {
        ngx_log_error(NGX_LOG_CRIT, file->log, ngx_errno,
                      "pread() \"%s\" failed", file->name.data);
        return NGX_ERROR;
    }

#else

    if (file->sys_offset != offset) {
        if (lseek(file->fd, offset, SEEK_SET) == -1) {
            ngx_log_error(NGX_LOG_CRIT, file->log, ngx_errno,
                          "lseek() \"%s\" failed", file->name.data);
            return NGX_ERROR;
        }

        file->sys_offset = offset;
    }

    n = read(file->fd, buf, size);

    if (n == -1) {
        ngx_log_error(NGX_LOG_CRIT, file->log, ngx_errno,
                      "read() \"%s\" failed", file->name.data);
        return NGX_ERROR;
    }

    file->sys_offset += n;

#endif

    file->offset += n;

    return n;
}
{% endhighlight %}
关于```ngx_file_t```数据结构我们后面会介绍，这里:
<pre>
ngx_file_t->sys_offset: 用于记录当前文件的指针偏移
ngx_file_t->offset: 用于记录当前fd已经读写过的字节总数
</pre>
当前我们支持```NGX_HAVE_PREAD```。这里我们简要介绍一下pread()函数：
{% highlight string %}
#include <unistd.h>

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
{% endhighlight %}
pread()函数从fd的offset处读取count数量的数据到buf中，读取完成后，**该fd对应的offset并不会改变**,因此这里```ngx_file_t->sys_offset```并不会被改变, 这一点与普通的read函数有区别。


## 3. thread read相关
如下我们暂不支持```NGX_THREADS```:
{% highlight string %}
#if (NGX_THREADS)

typedef struct {
    ngx_fd_t       fd;
    ngx_uint_t     write;   /* unsigned  write:1; */

    u_char        *buf;
    size_t         size;
    ngx_chain_t   *chain;
    off_t          offset;

    size_t         nbytes;
    ngx_err_t      err;
} ngx_thread_file_ctx_t;


ssize_t
ngx_thread_read(ngx_file_t *file, u_char *buf, size_t size, off_t offset,
    ngx_pool_t *pool)
{
    ngx_thread_task_t      *task;
    ngx_thread_file_ctx_t  *ctx;

    ngx_log_debug4(NGX_LOG_DEBUG_CORE, file->log, 0,
                   "thread read: %d, %p, %uz, %O",
                   file->fd, buf, size, offset);

    task = file->thread_task;

    if (task == NULL) {
        task = ngx_thread_task_alloc(pool, sizeof(ngx_thread_file_ctx_t));
        if (task == NULL) {
            return NGX_ERROR;
        }

        file->thread_task = task;
    }

    ctx = task->ctx;

    if (task->event.complete) {
        task->event.complete = 0;

        if (ctx->write) {
            ngx_log_error(NGX_LOG_ALERT, file->log, 0,
                          "invalid thread call, read instead of write");
            return NGX_ERROR;
        }

        if (ctx->err) {
            ngx_log_error(NGX_LOG_CRIT, file->log, ctx->err,
                          "pread() \"%s\" failed", file->name.data);
            return NGX_ERROR;
        }

        return ctx->nbytes;
    }

    task->handler = ngx_thread_read_handler;

    ctx->write = 0;

    ctx->fd = file->fd;
    ctx->buf = buf;
    ctx->size = size;
    ctx->offset = offset;

    if (file->thread_handler(task, file) != NGX_OK) {
        return NGX_ERROR;
    }

    return NGX_AGAIN;
}


#if (NGX_HAVE_PREAD)

static void
ngx_thread_read_handler(void *data, ngx_log_t *log)
{
    ngx_thread_file_ctx_t *ctx = data;

    ssize_t  n;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, log, 0, "thread read handler");

    n = pread(ctx->fd, ctx->buf, ctx->size, ctx->offset);

    if (n == -1) {
        ctx->err = ngx_errno;

    } else {
        ctx->nbytes = n;
        ctx->err = 0;
    }

#if 0
    ngx_time_update();
#endif

    ngx_log_debug4(NGX_LOG_DEBUG_CORE, log, 0,
                   "pread: %z (err: %d) of %uz @%O",
                   n, ctx->err, ctx->size, ctx->offset);
}

#else

#error pread() is required!

#endif

#endif /* NGX_THREADS */
{% endhighlight %}

## 4. 
函数内容如下：
{% highlight string %}
ssize_t
ngx_write_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset)
{
    ssize_t    n, written;
    ngx_err_t  err;

    ngx_log_debug4(NGX_LOG_DEBUG_CORE, file->log, 0,
                   "write: %d, %p, %uz, %O", file->fd, buf, size, offset);

    written = 0;

#if (NGX_HAVE_PWRITE)

    for ( ;; ) {
        n = pwrite(file->fd, buf + written, size, offset);

        if (n == -1) {
            err = ngx_errno;

            if (err == NGX_EINTR) {
                ngx_log_debug0(NGX_LOG_DEBUG_CORE, file->log, err,
                               "pwrite() was interrupted");
                continue;
            }

            ngx_log_error(NGX_LOG_CRIT, file->log, err,
                          "pwrite() \"%s\" failed", file->name.data);
            return NGX_ERROR;
        }

        file->offset += n;
        written += n;

        if ((size_t) n == size) {
            return written;
        }

        offset += n;
        size -= n;
    }

#else

    if (file->sys_offset != offset) {
        if (lseek(file->fd, offset, SEEK_SET) == -1) {
            ngx_log_error(NGX_LOG_CRIT, file->log, ngx_errno,
                          "lseek() \"%s\" failed", file->name.data);
            return NGX_ERROR;
        }

        file->sys_offset = offset;
    }

    for ( ;; ) {
        n = write(file->fd, buf + written, size);

        if (n == -1) {
            err = ngx_errno;

            if (err == NGX_EINTR) {
                ngx_log_debug0(NGX_LOG_DEBUG_CORE, file->log, err,
                               "write() was interrupted");
                continue;
            }

            ngx_log_error(NGX_LOG_CRIT, file->log, err,
                          "write() \"%s\" failed", file->name.data);
            return NGX_ERROR;
        }

        file->sys_offset += n;
        file->offset += n;
        written += n;

        if ((size_t) n == size) {
            return written;
        }

        size -= n;
    }
#endif
}
{% endhighlight %}
关于```ngx_file_t```数据结构我们后面会介绍，这里:
<pre>
ngx_file_t->sys_offset: 用于记录当前文件的指针偏移
ngx_file_t->offset: 用于记录当前fd已经读写过的字节总数
</pre>
当前我们支持```NGX_HAVE_PREAD```。这里我们简要介绍一下pread()函数：
{% highlight string %}
#include <unistd.h>

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
{% endhighlight %}
向fd的offset处写入buf中的数据。数据写完后，**该fd对应的offset并不会改变**, 因此这里```ngx_file_t->sys_offset```并不会被改变,这一点与普通的write函数有区别。


## 5. ngx_open_tempfile()函数

{% highlight string %}
ngx_fd_t
ngx_open_tempfile(u_char *name, ngx_uint_t persistent, ngx_uint_t access)
{
    ngx_fd_t  fd;

    fd = open((const char *) name, O_CREAT|O_EXCL|O_RDWR,
              access ? access : 0600);

    if (fd != -1 && !persistent) {
        (void) unlink((const char *) name);
    }

    return fd;
}
{% endhighlight %}
关于```O_EXCL```的解释，请参看如下：
<pre>
Ensure that this call creates the file: if this flag is specified in conjunction with O_CREAT, and pathname already
exists, then open() will fail.

When these two flags are specified, symbolic links are not followed: if pathname is a symbolic link, then open() 
fails regardless of where the  symbolic link points to.
</pre>


## 6. ngx_write_chain_to_file()函数
{% highlight string %}
ssize_t
ngx_write_chain_to_file(ngx_file_t *file, ngx_chain_t *cl, off_t offset,
    ngx_pool_t *pool)
{
    ssize_t        total, n;
    ngx_iovec_t    vec;
    struct iovec   iovs[NGX_IOVS_PREALLOCATE];

    /* use pwrite() if there is the only buf in a chain */

    if (cl->next == NULL) {
        return ngx_write_file(file, cl->buf->pos,
                              (size_t) (cl->buf->last - cl->buf->pos),
                              offset);
    }

    total = 0;

    vec.iovs = iovs;
    vec.nalloc = NGX_IOVS_PREALLOCATE;

    do {
        /* create the iovec and coalesce the neighbouring bufs */
        cl = ngx_chain_to_iovec(&vec, cl);

        /* use pwrite() if there is the only iovec buffer */

        if (vec.count == 1) {
            n = ngx_write_file(file, (u_char *) iovs[0].iov_base,
                               iovs[0].iov_len, offset);

            if (n == NGX_ERROR) {
                return n;
            }

            return total + n;
        }

        n = ngx_writev_file(file, &vec, offset);

        if (n == NGX_ERROR) {
            return n;
        }

        offset += n;
        total += n;

    } while (cl);

    return total;
}
{% endhighlight %}




<br />
<br />
<br />

