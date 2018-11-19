---
layout: post
title: core/ngx_open_file_cache.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---





<!-- more -->


## 1. 函数ngx_open_file_wrapper()
{% highlight string %}
static ngx_fd_t
ngx_open_file_wrapper(ngx_str_t *name, ngx_open_file_info_t *of,
    ngx_int_t mode, ngx_int_t create, ngx_int_t access, ngx_log_t *log)
{
    ngx_fd_t  fd;

#if !(NGX_HAVE_OPENAT)

    fd = ngx_open_file(name->data, mode, create, access);

    if (fd == NGX_INVALID_FILE) {
        of->err = ngx_errno;
        of->failed = ngx_open_file_n;
        return NGX_INVALID_FILE;
    }

    return fd;

#else

    u_char           *p, *cp, *end;
    ngx_fd_t          at_fd;
    ngx_str_t         at_name;

    if (of->disable_symlinks == NGX_DISABLE_SYMLINKS_OFF) {
        fd = ngx_open_file(name->data, mode, create, access);

        if (fd == NGX_INVALID_FILE) {
            of->err = ngx_errno;
            of->failed = ngx_open_file_n;
            return NGX_INVALID_FILE;
        }

        return fd;
    }

    p = name->data;
    end = p + name->len;

    at_name = *name;

    if (of->disable_symlinks_from) {

        cp = p + of->disable_symlinks_from;

        *cp = '\0';

        at_fd = ngx_open_file(p, NGX_FILE_SEARCH|NGX_FILE_NONBLOCK,
                              NGX_FILE_OPEN, 0);

        *cp = '/';

        if (at_fd == NGX_INVALID_FILE) {
            of->err = ngx_errno;
            of->failed = ngx_open_file_n;
            return NGX_INVALID_FILE;
        }

        at_name.len = of->disable_symlinks_from;
        p = cp + 1;

    } else if (*p == '/') {

        at_fd = ngx_open_file("/",
                              NGX_FILE_SEARCH|NGX_FILE_NONBLOCK,
                              NGX_FILE_OPEN, 0);

        if (at_fd == NGX_INVALID_FILE) {
            of->err = ngx_errno;
            of->failed = ngx_openat_file_n;
            return NGX_INVALID_FILE;
        }

        at_name.len = 1;
        p++;

    } else {
        at_fd = NGX_AT_FDCWD;
    }

    for ( ;; ) {
        cp = ngx_strlchr(p, end, '/');
        if (cp == NULL) {
            break;
        }

        if (cp == p) {
            p++;
            continue;
        }

        *cp = '\0';

        if (of->disable_symlinks == NGX_DISABLE_SYMLINKS_NOTOWNER) {
            fd = ngx_openat_file_owner(at_fd, p,
                                       NGX_FILE_SEARCH|NGX_FILE_NONBLOCK,
                                       NGX_FILE_OPEN, 0, log);

        } else {
            fd = ngx_openat_file(at_fd, p,
                           NGX_FILE_SEARCH|NGX_FILE_NONBLOCK|NGX_FILE_NOFOLLOW,
                           NGX_FILE_OPEN, 0);
        }

        *cp = '/';

        if (fd == NGX_INVALID_FILE) {
            of->err = ngx_errno;
            of->failed = ngx_openat_file_n;
            goto failed;
        }

        if (at_fd != NGX_AT_FDCWD && ngx_close_file(at_fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                          ngx_close_file_n " \"%V\" failed", &at_name);
        }

        p = cp + 1;
        at_fd = fd;
        at_name.len = cp - at_name.data;
    }

    if (p == end) {

        /*
         * If pathname ends with a trailing slash, assume the last path
         * component is a directory and reopen it with requested flags;
         * if not, fail with ENOTDIR as per POSIX.
         *
         * We cannot rely on O_DIRECTORY in the loop above to check
         * that the last path component is a directory because
         * O_DIRECTORY doesn't work on FreeBSD 8.  Fortunately, by
         * reopening a directory, we don't depend on it at all.
         */

        fd = ngx_openat_file(at_fd, ".", mode, create, access);
        goto done;
    }

    if (of->disable_symlinks == NGX_DISABLE_SYMLINKS_NOTOWNER
        && !(create & (NGX_FILE_CREATE_OR_OPEN|NGX_FILE_TRUNCATE)))
    {
        fd = ngx_openat_file_owner(at_fd, p, mode, create, access, log);

    } else {
        fd = ngx_openat_file(at_fd, p, mode|NGX_FILE_NOFOLLOW, create, access);
    }

done:

    if (fd == NGX_INVALID_FILE) {
        of->err = ngx_errno;
        of->failed = ngx_openat_file_n;
    }

failed:

    if (at_fd != NGX_AT_FDCWD && ngx_close_file(at_fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                      ngx_close_file_n " \"%V\" failed", &at_name);
    }

    return fd;
#endif
}

{% endhighlight %}
此函数主要是封装了普通文件打开，与openat()这样一个相对路径的打开。关于```disable_symlinks_from```与```disable_symlinks```字段，主要用于决定当打开文件的时候如何处理符号链接(symbolic links)。其中前一个字段用于指定从哪一个字段开始检查符号链接， 而后一个字段用于指定是否禁止符号链接。

本函数返回代表该文件的文件句柄。

## 2. 函数ngx_file_info_wrapper()
{% highlight string %}
static ngx_int_t
ngx_file_info_wrapper(ngx_str_t *name, ngx_open_file_info_t *of,
    ngx_file_info_t *fi, ngx_log_t *log)
{
    ngx_int_t  rc;

#if !(NGX_HAVE_OPENAT)

    rc = ngx_file_info(name->data, fi);

    if (rc == NGX_FILE_ERROR) {
        of->err = ngx_errno;
        of->failed = ngx_file_info_n;
        return NGX_FILE_ERROR;
    }

    return rc;

#else

    ngx_fd_t  fd;

    if (of->disable_symlinks == NGX_DISABLE_SYMLINKS_OFF) {

        rc = ngx_file_info(name->data, fi);

        if (rc == NGX_FILE_ERROR) {
            of->err = ngx_errno;
            of->failed = ngx_file_info_n;
            return NGX_FILE_ERROR;
        }

        return rc;
    }

    fd = ngx_open_file_wrapper(name, of, NGX_FILE_RDONLY|NGX_FILE_NONBLOCK,
                               NGX_FILE_OPEN, 0, log);

    if (fd == NGX_INVALID_FILE) {
        return NGX_FILE_ERROR;
    }

    rc = ngx_fd_info(fd, fi);

    if (rc == NGX_FILE_ERROR) {
        of->err = ngx_errno;
        of->failed = ngx_fd_info_n;
    }

    if (ngx_close_file(fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                      ngx_close_file_n " \"%V\" failed", name);
    }

    return rc;
#endif
}
{% endhighlight %}

本函数用于获取文件的```ngx_file_info_t```信息，保存到输出参数```fi```中。注意在不支持```NGX_HAVE_OPENAT```的操作系统中，会把符号链接文件也当成普通文件来进行处理。


## 3. 函数ngx_open_and_stat_file()
{% highlight string %}
static ngx_int_t
ngx_open_and_stat_file(ngx_str_t *name, ngx_open_file_info_t *of,
    ngx_log_t *log)
{
    ngx_fd_t         fd;
    ngx_file_info_t  fi;

    if (of->fd != NGX_INVALID_FILE) {

        if (ngx_file_info_wrapper(name, of, &fi, log) == NGX_FILE_ERROR) {
            of->fd = NGX_INVALID_FILE;
            return NGX_ERROR;
        }

        if (of->uniq == ngx_file_uniq(&fi)) {
            goto done;
        }

    } else if (of->test_dir) {

        if (ngx_file_info_wrapper(name, of, &fi, log) == NGX_FILE_ERROR) {
            of->fd = NGX_INVALID_FILE;
            return NGX_ERROR;
        }

        if (ngx_is_dir(&fi)) {
            goto done;
        }
    }

    if (!of->log) {

        /*
         * Use non-blocking open() not to hang on FIFO files, etc.
         * This flag has no effect on a regular files.
         */

        fd = ngx_open_file_wrapper(name, of, NGX_FILE_RDONLY|NGX_FILE_NONBLOCK,
                                   NGX_FILE_OPEN, 0, log);

    } else {
        fd = ngx_open_file_wrapper(name, of, NGX_FILE_APPEND,
                                   NGX_FILE_CREATE_OR_OPEN,
                                   NGX_FILE_DEFAULT_ACCESS, log);
    }

    if (fd == NGX_INVALID_FILE) {
        of->fd = NGX_INVALID_FILE;
        return NGX_ERROR;
    }

    if (ngx_fd_info(fd, &fi) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_CRIT, log, ngx_errno,
                      ngx_fd_info_n " \"%V\" failed", name);

        if (ngx_close_file(fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                          ngx_close_file_n " \"%V\" failed", name);
        }

        of->fd = NGX_INVALID_FILE;

        return NGX_ERROR;
    }

    if (ngx_is_dir(&fi)) {
        if (ngx_close_file(fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                          ngx_close_file_n " \"%V\" failed", name);
        }

        of->fd = NGX_INVALID_FILE;

    } else {
        of->fd = fd;

        if (of->read_ahead && ngx_file_size(&fi) > NGX_MIN_READ_AHEAD) {
            if (ngx_read_ahead(fd, of->read_ahead) == NGX_ERROR) {
                ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                              ngx_read_ahead_n " \"%V\" failed", name);
            }
        }

        if (of->directio <= ngx_file_size(&fi)) {
            if (ngx_directio_on(fd) == NGX_FILE_ERROR) {
                ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                              ngx_directio_on_n " \"%V\" failed", name);

            } else {
                of->is_directio = 1;
            }
        }
    }

done:

    of->uniq = ngx_file_uniq(&fi);
    of->mtime = ngx_file_mtime(&fi);
    of->size = ngx_file_size(&fi);
    of->fs_size = ngx_file_fs_size(&fi);
    of->is_dir = ngx_is_dir(&fi);
    of->is_file = ngx_is_file(&fi);
    of->is_link = ngx_is_link(&fi);
    of->is_exec = ngx_is_exec(&fi);

    return NGX_OK;
}
{% endhighlight %}




## 13. direct io方式读写文件（附录）
所谓direct io， 即不通过操作系统缓冲， 使用磁盘IO(或者DMA)直接将硬盘上的数据读入用户空间buffer， 或者将用户空间buffer中的数据通过磁盘IO(或者DMA)直接写到硬盘上。这样避免内核缓冲的消耗与CPU拷贝（数据在内核空间和用户空间之间的拷贝）的消耗。

### 1.1 direct io使用场景

direct io一般是通过DMA的方式来读取文件的。 通过direct io读取文件之前，一般需要初始化DMA, 因此一般使用direct io来读取大文件； 如果是读取小文件，初始化DMA的时间比系统读小文件的时间还长， 所以小文件使用direct io没有优势。对于大文件也只是在只读一次，并且后续没有其他应用再次读取此文件的时候，才有优势， 如果后续还有其他应用需要使用， 这个时候DirectIO也没有优势。


### 1.2 direct io使用示例
direct io方式读写文件， 只需在打开文件时选上```O_DIRECT```选项就行， 但必须在所有```include```前加上:
<pre>
#define _GNU_SOURCE
</pre> 
另外，以direct io方式读写时，开辟的buffer必须是系统页大小的整数倍而且必须以页大小为标准对齐， 例如linux2.6下每页大小是4096字节（函数```getpagesize()```)，申请的buffer大小只能是4096的整数倍才能获得最大的性能。 参看如下例子test.c:
{% highlight string %}

#define _GNU_SOURCE

#define BUFFER_SIZE 8192

int fd = open("testfile", O_CREAT|O_RDWR|O_DIRECT);
int pagesize = getpagesize();

char *readbuf = (char *)malloc(BUFFER_SIZE + pagesize);

char *alignedReadBuf = (char *) ((((uintptr)readbuf + pagesize -1)/pagesize) * pagesize);

read(fd, alignedReadBuf, BUFFER_SIZE);

free(readbuf);

{% endhighlight %}


### 1.3 nginx aio与direct io
关于nginx aio， 有如下一段说明：
<pre>
On Linux, AIO can be used starting from kernel version 2.6.22. Also, it is necessary to enable directio, or otherwise reading will be blocking:

location /video/ {
    aio            on;
    directio       512;
    output_buffers 1 128k;
}
</pre>


而当AIO与sendfile一起使用时， 有如下一段说明：
<pre>
When both AIO and sendfile are enabled on Linux, AIO is used for files that are larger than or equal to the size specified in the directio directive, while sendfile is used for files of smaller sizes or when directio is disabled.

location /video/ {
    sendfile       on;
    aio            on;
    directio       8m;
}
</pre>

当aio为```threads```时，有如下说明：
<pre>
Finally, files can be read and sent using multi-threading (1.7.11), without blocking a worker process:

location /video/ {
    sendfile       on;
    aio            threads;
}
</pre>



<br />
<br />

**[参看]**

1. [DirectIO方式读写文件](https://blog.csdn.net/zhangxinrun/article/details/6874143)

2. [nginx aio](http://nginx.org/en/docs/http/ngx_http_core_module.html#aio)



<br />
<br />
<br />

