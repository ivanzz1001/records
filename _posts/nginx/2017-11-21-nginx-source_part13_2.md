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


## 3. thread 读操作
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

## 4. ngx_write_file()函数
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


static ngx_chain_t *
ngx_chain_to_iovec(ngx_iovec_t *vec, ngx_chain_t *cl)
{
    size_t         total, size;
    u_char        *prev;
    ngx_uint_t     n;
    struct iovec  *iov;

    iov = NULL;
    prev = NULL;
    total = 0;
    n = 0;

    for ( /* void */ ; cl; cl = cl->next) {

        if (ngx_buf_special(cl->buf)) {
            continue;
        }

        size = cl->buf->last - cl->buf->pos;

        if (prev == cl->buf->pos) {
            iov->iov_len += size;

        } else {
            if (n == vec->nalloc) {
                break;
            }

            iov = &vec->iovs[n++];

            iov->iov_base = (void *) cl->buf->pos;
            iov->iov_len = size;
        }

        prev = cl->buf->pos + size;
        total += size;
    }

    vec->count = n;
    vec->size = total;

    return cl;
}


static ssize_t
ngx_writev_file(ngx_file_t *file, ngx_iovec_t *vec, off_t offset)
{
    ssize_t    n;
    ngx_err_t  err;

    ngx_log_debug3(NGX_LOG_DEBUG_CORE, file->log, 0,
                   "writev: %d, %uz, %O", file->fd, vec->size, offset);

#if (NGX_HAVE_PWRITEV)

eintr:

    n = pwritev(file->fd, vec->iovs, vec->count, offset);

    if (n == -1) {
        err = ngx_errno;

        if (err == NGX_EINTR) {
            ngx_log_debug0(NGX_LOG_DEBUG_CORE, file->log, err,
                           "pwritev() was interrupted");
            goto eintr;
        }

        ngx_log_error(NGX_LOG_CRIT, file->log, err,
                      "pwritev() \"%s\" failed", file->name.data);
        return NGX_ERROR;
    }

    if ((size_t) n != vec->size) {
        ngx_log_error(NGX_LOG_CRIT, file->log, 0,
                      "pwritev() \"%s\" has written only %z of %uz",
                      file->name.data, n, vec->size);
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

eintr:

    n = writev(file->fd, vec->iovs, vec->count);

    if (n == -1) {
        err = ngx_errno;

        if (err == NGX_EINTR) {
            ngx_log_debug0(NGX_LOG_DEBUG_CORE, file->log, err,
                           "writev() was interrupted");
            goto eintr;
        }

        ngx_log_error(NGX_LOG_CRIT, file->log, err,
                      "writev() \"%s\" failed", file->name.data);
        return NGX_ERROR;
    }

    if ((size_t) n != vec->size) {
        ngx_log_error(NGX_LOG_CRIT, file->log, 0,
                      "writev() \"%s\" has written only %z of %uz",
                      file->name.data, n, vec->size);
        return NGX_ERROR;
    }

    file->sys_offset += n;

#endif

    file->offset += n;

    return n;
}
{% endhighlight %}

这三个函数是相关的，我们放到一起来讲解：
<pre>
ssize_t
ngx_write_chain_to_file(ngx_file_t *file, ngx_chain_t *cl, off_t offset,
    ngx_pool_t *pool);

static ngx_chain_t *
ngx_chain_to_iovec(ngx_iovec_t *vec, ngx_chain_t *cl);

static ssize_t
ngx_writev_file(ngx_file_t *file, ngx_iovec_t *vec, off_t offset);
</pre>

首先我们来看一下ngx_iovec_t数据结构(os/unix/ngx_os.h)：
{% highlight string %}
typedef struct {
    struct iovec  *iovs;
    ngx_uint_t     count;      //当前所使用的iovs的个数
    size_t         size;       //当前存放的总的字节数
    ngx_uint_t     nalloc;     //总的iovs个数
} ngx_iovec_t;
{% endhighlight %}
而对于```ngx_chain_t```数据结构有如下定义：
{% highlight string %}
typedef struct ngx_chain_s       ngx_chain_t;
struct ngx_chain_s {
    ngx_buf_t    *buf;
    ngx_chain_t  *next;
};
{% endhighlight %}
关于```ngx_buf_t```我们后续会详细介绍，这里我们只给出一个大体的示意图：

![ngx-chain-t](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_chain_t.jpg)

**(1) 函数ngx_chain_to_iovec()**

该函数的主要作用是将```ngx_chain_t```缓存链转换为```ngx_iovec_t```,转换时进行适当的内存合并。

<br />

**(2) 函数ngx_writev_file()**

当前环境支持```NGX_HAVE_PWRITEV```，因此这里采用```pwritev()```函数进行分散文件的写入。

<br />

**(3) 函数ngx_write_chain_to_file()**
<pre>
ssize_t
ngx_write_chain_to_file(ngx_file_t *file, ngx_chain_t *cl, off_t offset,
    ngx_pool_t *pool);
</pre>
该函数主要是循环遍历```ngx_chain_t```，然后将数据发送到ngx_file_t保存的文件句柄中。

<br />
在这里文件的读写采用了如下两组函数：

* 读函数: read()、readv()、pread()、preadv()

* 写函数: write()、writev()、pwrite()、pwritev()

请注意上述两组函数中各个函数的区别。


## 7. thread 写操作

当前我们并不支持```NGX_THREADS```:
{% highlight string %}
#if (NGX_THREADS)

ssize_t
ngx_thread_write_chain_to_file(ngx_file_t *file, ngx_chain_t *cl, off_t offset,
    ngx_pool_t *pool)
{
    ngx_thread_task_t      *task;
    ngx_thread_file_ctx_t  *ctx;

    ngx_log_debug3(NGX_LOG_DEBUG_CORE, file->log, 0,
                   "thread write chain: %d, %p, %O",
                   file->fd, cl, offset);

    task = file->thread_task;

    if (task == NULL) {
        task = ngx_thread_task_alloc(pool,
                                     sizeof(ngx_thread_file_ctx_t));
        if (task == NULL) {
            return NGX_ERROR;
        }

        file->thread_task = task;
    }

    ctx = task->ctx;

    if (task->event.complete) {
        task->event.complete = 0;

        if (!ctx->write) {
            ngx_log_error(NGX_LOG_ALERT, file->log, 0,
                          "invalid thread call, write instead of read");
            return NGX_ERROR;
        }

        if (ctx->err || ctx->nbytes == 0) {
            ngx_log_error(NGX_LOG_CRIT, file->log, ctx->err,
                          "pwritev() \"%s\" failed", file->name.data);
            return NGX_ERROR;
        }

        file->offset += ctx->nbytes;
        return ctx->nbytes;
    }

    task->handler = ngx_thread_write_chain_to_file_handler;

    ctx->write = 1;

    ctx->fd = file->fd;
    ctx->chain = cl;
    ctx->offset = offset;

    if (file->thread_handler(task, file) != NGX_OK) {
        return NGX_ERROR;
    }

    return NGX_AGAIN;
}


static void
ngx_thread_write_chain_to_file_handler(void *data, ngx_log_t *log)
{
    ngx_thread_file_ctx_t *ctx = data;

#if (NGX_HAVE_PWRITEV)

    off_t          offset;
    ssize_t        n;
    ngx_err_t      err;
    ngx_chain_t   *cl;
    ngx_iovec_t    vec;
    struct iovec   iovs[NGX_IOVS_PREALLOCATE];

    vec.iovs = iovs;
    vec.nalloc = NGX_IOVS_PREALLOCATE;

    cl = ctx->chain;
    offset = ctx->offset;

    ctx->nbytes = 0;
    ctx->err = 0;

    do {
        /* create the iovec and coalesce the neighbouring bufs */
        cl = ngx_chain_to_iovec(&vec, cl);

eintr:

        n = pwritev(ctx->fd, iovs, vec.count, offset);

        if (n == -1) {
            err = ngx_errno;

            if (err == NGX_EINTR) {
                ngx_log_debug0(NGX_LOG_DEBUG_CORE, log, err,
                               "pwritev() was interrupted");
                goto eintr;
            }

            ctx->err = err;
            return;
        }

        if ((size_t) n != vec.size) {
            ctx->nbytes = 0;
            return;
        }

        ctx->nbytes += n;
        offset += n;
    } while (cl);

#else

    ctx->err = NGX_ENOSYS;
    return;

#endif
}

#endif /* NGX_THREADS */
{% endhighlight %}


## 8. 设置文件```最近访问时间```和```最近修改时间```
{% highlight string %}
ngx_int_t
ngx_set_file_time(u_char *name, ngx_fd_t fd, time_t s)
{
    struct timeval  tv[2];

    tv[0].tv_sec = ngx_time();
    tv[0].tv_usec = 0;
    tv[1].tv_sec = s;
    tv[1].tv_usec = 0;

    if (utimes((char *) name, tv) != -1) {
        return NGX_OK;
    }

    return NGX_ERROR;
}
{% endhighlight %}

这里采用utimes()函数设置文件的```access time```和```modification time```:
{% highlight string %}
#include <sys/time.h>

int utimes(const char *filename, const struct timeval times[2]);
{% endhighlight %}
其中times[0]用于修改```access time```，times[1]用于修改```modification time```。

## 9. 文件mmap映射
{% highlight string %}
ngx_int_t
ngx_create_file_mapping(ngx_file_mapping_t *fm)
{
    fm->fd = ngx_open_file(fm->name, NGX_FILE_RDWR, NGX_FILE_TRUNCATE,
                           NGX_FILE_DEFAULT_ACCESS);
    if (fm->fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_CRIT, fm->log, ngx_errno,
                      ngx_open_file_n " \"%s\" failed", fm->name);
        return NGX_ERROR;
    }

    if (ftruncate(fm->fd, fm->size) == -1) {
        ngx_log_error(NGX_LOG_CRIT, fm->log, ngx_errno,
                      "ftruncate() \"%s\" failed", fm->name);
        goto failed;
    }

    fm->addr = mmap(NULL, fm->size, PROT_READ|PROT_WRITE, MAP_SHARED,
                    fm->fd, 0);
    if (fm->addr != MAP_FAILED) {
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_CRIT, fm->log, ngx_errno,
                  "mmap(%uz) \"%s\" failed", fm->size, fm->name);

failed:

    if (ngx_close_file(fm->fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, fm->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", fm->name);
    }

    return NGX_ERROR;
}


void
ngx_close_file_mapping(ngx_file_mapping_t *fm)
{
    if (munmap(fm->addr, fm->size) == -1) {
        ngx_log_error(NGX_LOG_CRIT, fm->log, ngx_errno,
                      "munmap(%uz) \"%s\" failed", fm->size, fm->name);
    }

    if (ngx_close_file(fm->fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, fm->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", fm->name);
    }
}
{% endhighlight %}

函数```ngx_create_file_mapping()```首先创建一个文件(如果文件已存在，则NGX_FILE_TRUNCATE标志将其截断为0），然后调用ftruncate()将其截断成指定大小的文件，然后再进行映射。

函数```ngx_close_file_mapping```首先解除映射，然后关闭文件。

## 10. 目录的操作
这里封装了打开目录与读取目录的相关操作:
{% highlight string %}
ngx_int_t
ngx_open_dir(ngx_str_t *name, ngx_dir_t *dir)
{
    dir->dir = opendir((const char *) name->data);

    if (dir->dir == NULL) {
        return NGX_ERROR;
    }

    dir->valid_info = 0;

    return NGX_OK;
}


ngx_int_t
ngx_read_dir(ngx_dir_t *dir)
{
    dir->de = readdir(dir->dir);

    if (dir->de) {
#if (NGX_HAVE_D_TYPE)
        dir->type = dir->de->d_type;
#else
        dir->type = 0;
#endif
        return NGX_OK;
    }

    return NGX_ERROR;
}
{% endhighlight %}


## 11. 文件查找
{% highlight string %}
ngx_int_t
ngx_open_glob(ngx_glob_t *gl)
{
    int  n;

    n = glob((char *) gl->pattern, 0, NULL, &gl->pglob);

    if (n == 0) {
        return NGX_OK;
    }

#ifdef GLOB_NOMATCH

    if (n == GLOB_NOMATCH && gl->test) {
        return NGX_OK;
    }

#endif

    return NGX_ERROR;
}


ngx_int_t
ngx_read_glob(ngx_glob_t *gl, ngx_str_t *name)
{
    size_t  count;

#ifdef GLOB_NOMATCH
    count = (size_t) gl->pglob.gl_pathc;
#else
    count = (size_t) gl->pglob.gl_matchc;
#endif

    if (gl->n < count) {

        name->len = (size_t) ngx_strlen(gl->pglob.gl_pathv[gl->n]);
        name->data = (u_char *) gl->pglob.gl_pathv[gl->n];
        gl->n++;

        return NGX_OK;
    }

    return NGX_DONE;
}


void
ngx_close_glob(ngx_glob_t *gl)
{
    globfree(&gl->pglob);
}
{% endhighlight %}
函数```ngx_open_glob()```用于基于模式匹配的文件查找，查找到的结果保存在```ngx_glob_t```数据结构中。

函数```ngx_read_glob()```用于从上述查找结果中读取一个查找结果。

函数```ngx_close_glob()```释放相关的资源。

通过如下程序代码测试(test.c):
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <glob.h>

int main(int argc,char *argv[])
{
    #ifdef GLOB_NOMATCH
       printf("support GLOB_NOMATCH\n");
    #endif
    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# ./test
support GLOB_NOMATCH
</pre>

## 12. 文件锁
{% highlight string %}
ngx_err_t
ngx_trylock_fd(ngx_fd_t fd)
{
    struct flock  fl;

    ngx_memzero(&fl, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        return ngx_errno;
    }

    return 0;
}


ngx_err_t
ngx_lock_fd(ngx_fd_t fd)
{
    struct flock  fl;

    ngx_memzero(&fl, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;

    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        return ngx_errno;
    }

    return 0;
}


ngx_err_t
ngx_unlock_fd(ngx_fd_t fd)
{
    struct flock  fl;

    ngx_memzero(&fl, sizeof(struct flock));
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        return  ngx_errno;
    }

    return 0;
}
{% endhighlight %}
 
上面代码比较简单，这里我们主要介绍一下Linux环境下flock和lockf。

### 12.1 flock 和 lockf

从底层实现上来说，Linux的文件锁主要有两种：```flock```和```lockf```。需要额外对lockf说明的是，**它只是fcntl系统调用的一个封装**。从使用角度讲，lockf或fcntl实现了更细粒度的文件锁，即：记录锁。我们可以使用lockf或fcntl对文件的部分字节上锁，而**flock只能对整个文件上锁**。这两种文件锁是从历史上不同的标准中起源的，flock来自BSD，而lockf来自POSIX，所有lockf或fcntl实现的锁在类型上又叫做```POSIX```锁。

除了这个区别外，```fcntl```系统调用还可以支持强制锁（Mandatory locking）。强制锁的概念是传统```UNIX```为了强制应用程序遵守锁规则而引入的一个概念，与之对应的概念就是建议锁(Advisory locking)。我们日常使用的基本都是建议锁，它并不强制生效。这里的不强制生效的意思是，**如果某一个进程对一个文件持有一把锁之后，其他进程仍然可以直接对文件进行各种操作的**，比如open、read、write。只有当多个进程在操作文件前都去检查和对相关锁进行锁操作的时候，文件锁的规则才会生效。这就是一般建议锁行为。而强制锁试图实现一套内核级的锁操作。当有进程对某个文件上锁之后，其他进程即使不在操作之前检查锁，也会在open()、read()或write()等文件操作时发生错误。内核将对有锁的文件在任何情况下的锁规则都生效，这就是强制锁的行为。由此可以理解，如果内核想要支持强制锁，将需要在内核实现open()、read()或write()等系统调用内部进行支持。

从应用角度来说，**Linux内核虽然号称具备了锁的能力，但其对强制锁的实现是不可靠的，建议大家还是不要在Linux下使用强制锁**。鉴于此，我们就不在此介绍如何在Linux环境中打开所谓的强制锁支持了。我们只需知道，在Linux环境下的应用程序，flock和lockf在锁类型方面没有本质区别，它们都是建议锁，而非强制锁。


flock和lockf的另一个差别是它们实现锁的方式不同。这在应用的时候表现在flock的语义是针对文件的锁，而lockf是针对文件描述符(fd)的锁。

### 12.2 内核文件结构
我们先来通过如下的程序来理解一下一个打开的文件的内核结构：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>


int main(int argc,char *argv[])
{
    int fd = -1;
    int fd2 = -1;
    int fd3 = -1;
    pid_t pid;
    char buf[16];
    size_t sz;

    fd = open("test.txt",O_RDWR,0);


    pid = fork();
    if(pid == 0)
    {
        sz = read(fd,buf,4);
        buf[sz] = 0;
        printf("child buf: %s\n",buf);
        exit(0);
    }
    sz = read(fd,buf,4);
    buf[sz] = 0;
    printf("parent buf: %s\n",buf);

    wait(NULL);

    fd2 = dup(fd);
    sz = read(fd2,buf,4);
    buf[sz] = 0;
    printf("parent buf2: %s\n",buf);


    fd3 = open("test.txt",O_RDWR,0);
    sz = read(fd3,buf,4);
    buf[sz] = 0;
    printf("parent buf3: %s\n",buf);


    return 0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# ./test
parent buf: hell
child buf: o,wo
parent buf2: rld.
parent buf3: hell
</pre>
可以看到，通过fork()传递的文件描述符共享同一个```文件表项```(dup()函数类似)，而两次通过open()函数打开同一个文件获得的是不同的```文件表项```。如下图所示：

![file table entry](https://ivanzz1001.github.io/records/assets/img/nginx/linux_file_table.jpg)

```flock()```锁是作用于```文件表项```上的递归锁，因此可以对同一个文件表项多次进行加锁，例如（test2.c)：
{% highlight string %}
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/file.h>
int main (int argc, char ** argv)
{
    int ret;
    int fd1 = open("test.txt",O_RDWR);

    ret = flock(fd1,LOCK_EX);
    printf("get lock1, ret: %d\n", ret);
    ret = flock(fd1,LOCK_EX);
    printf("get lock2, ret: %d\n", ret);

    return 0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# ./test2
get lock1, ret: 0
get lock2, ret: 0
</pre>

注意：
<pre>
flock()递归锁允许对同一个文件表项在解锁之前进行多次加锁，其内部会维持锁的计数，在解锁次数和加锁次数不相同的情况下，
不会释放锁。所以，如果对一个递归锁加锁两次，然后解锁一次，那么该文件表项仍然处于加锁状态，对它再次解锁以前不能释放
该锁。
</pre>

而fcntl()的```F_WRLCK```锁在跨进程之间互斥的，flock()是跨文件表项互斥。


## 13. 函数ngx_read_ahead()
{% highlight string %}
#if (NGX_HAVE_POSIX_FADVISE) && !(NGX_HAVE_F_READAHEAD)

ngx_int_t
ngx_read_ahead(ngx_fd_t fd, size_t n)
{
    int  err;

    err = posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);

    if (err == 0) {
        return 0;
    }

    ngx_set_errno(err);
    return NGX_FILE_ERROR;
}

#endif
{% endhighlight %}
当前```NGX_HAVE_POSIX_FADVISE```为1，```NGX_HAVE_F_READAHEAD```值为0。这里主要是通过操作系统内核的支持加快对文件的访问，属于系统优化的部分。


## 14. direct io支持
{% highlight string %}
#if (NGX_HAVE_O_DIRECT)

ngx_int_t
ngx_directio_on(ngx_fd_t fd)
{
    int  flags;

    flags = fcntl(fd, F_GETFL);

    if (flags == -1) {
        return NGX_FILE_ERROR;
    }

    return fcntl(fd, F_SETFL, flags | O_DIRECT);
}


ngx_int_t
ngx_directio_off(ngx_fd_t fd)
{
    int  flags;

    flags = fcntl(fd, F_GETFL);

    if (flags == -1) {
        return NGX_FILE_ERROR;
    }

    return fcntl(fd, F_SETFL, flags & ~O_DIRECT);
}

#endif
{% endhighlight %}

当前```NGX_HAVE_O_DIRECT```值为1。这里打开与关闭direct io的支持。

## 14. 函数ngx_fs_bsize()
{% highlight string %}
#if (NGX_HAVE_STATFS)

size_t
ngx_fs_bsize(u_char *name)
{
    struct statfs  fs;

    if (statfs((char *) name, &fs) == -1) {
        return 512;
    }

    if ((fs.f_bsize % 512) != 0) {
        return 512;
    }

    return (size_t) fs.f_bsize;
}

#elif (NGX_HAVE_STATVFS)

size_t
ngx_fs_bsize(u_char *name)
{
    struct statvfs  fs;

    if (statvfs((char *) name, &fs) == -1) {
        return 512;
    }

    if ((fs.f_frsize % 512) != 0) {
        return 512;
    }

    return (size_t) fs.f_frsize;
}

#else

size_t
ngx_fs_bsize(u_char *name)
{
    return 512;
}

#endif
{% endhighlight %}
在ngx_auto_config.h头文件中我们有如下定义：
<pre>
#ifndef NGX_HAVE_STATFS
#define NGX_HAVE_STATFS  1
#endif
</pre>
```fs.f_bsize```获取一个文件块的大小。读写文件时按块大小来进行读写，能获得最高的效率。


<br />
<br />

**[参看]:**

1. [多进程之间的文件锁](http://www.jianshu.com/p/eb57a467f702)

2. [Linux 中 fcntl()、lockf、flock 的区别](http://blog.jobbole.com/104331/)

<br />
<br />
<br />

