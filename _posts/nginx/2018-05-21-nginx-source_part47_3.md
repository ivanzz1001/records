---
layout: post
title: core/ngx_open_file_cache.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要讲述一下nginx对静态文件的缓存相关操作。



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
此函数用于打开一个文件，并获取相应的信息保存到参数```of```中。一般情况，当```of->fd```不为-1时，说明文件已经打开，可以直接通过ngx_file_info_wrapper()来获取相应的信息； 否则通过ngx_open_file_wrapper()打开文件，然后再获取相应的文件信息。

## 4. 函数ngx_open_file_add_event()
{% highlight string %}
/*
 * we ignore any possible event setting error and
 * fallback to usual periodic file retests
 */

static void
ngx_open_file_add_event(ngx_open_file_cache_t *cache,
    ngx_cached_open_file_t *file, ngx_open_file_info_t *of, ngx_log_t *log)
{
    ngx_open_file_cache_event_t  *fev;

    if (!(ngx_event_flags & NGX_USE_VNODE_EVENT)
        || !of->events
        || file->event
        || of->fd == NGX_INVALID_FILE
        || file->uses < of->min_uses)
    {
        return;
    }

    file->use_event = 0;

    file->event = ngx_calloc(sizeof(ngx_event_t), log);
    if (file->event== NULL) {
        return;
    }

    fev = ngx_alloc(sizeof(ngx_open_file_cache_event_t), log);
    if (fev == NULL) {
        ngx_free(file->event);
        file->event = NULL;
        return;
    }

    fev->fd = of->fd;
    fev->file = file;
    fev->cache = cache;

    file->event->handler = ngx_open_file_cache_remove;
    file->event->data = fev;

    /*
     * although vnode event may be called while ngx_cycle->poll
     * destruction, however, cleanup procedures are run before any
     * memory freeing and events will be canceled.
     */

    file->event->log = ngx_cycle->log;

    if (ngx_add_event(file->event, NGX_VNODE_EVENT, NGX_ONESHOT_EVENT)
        != NGX_OK)
    {
        ngx_free(file->event->data);
        ngx_free(file->event);
        file->event = NULL;
        return;
    }

    /*
     * we do not set file->use_event here because there may be a race
     * condition: a file may be deleted between opening the file and
     * adding event, so we rely upon event notification only after
     * one file revalidation on next file access
     */

    return;
}
{% endhighlight %}
本函数用于为某个打开的缓存文件绑定一个vnode事件。

## 5. 函数ngx_open_file_cleanup()
{% highlight string %}
static void
ngx_open_file_cleanup(void *data)
{
    ngx_open_file_cache_cleanup_t  *c = data;

    c->file->count--;

    ngx_close_cached_file(c->cache, c->file, c->min_uses, c->log);

    /* drop one or two expired open files */
    ngx_expire_old_cached_files(c->cache, 1, c->log);
}
{% endhighlight %}
本函数用于清除一个文件缓存，并从超时队列中移除一个或两个超时缓存文件。

## 6. 函数ngx_close_cached_file()
{% highlight string %}
static void
ngx_close_cached_file(ngx_open_file_cache_t *cache,
    ngx_cached_open_file_t *file, ngx_uint_t min_uses, ngx_log_t *log)
{
    ngx_log_debug5(NGX_LOG_DEBUG_CORE, log, 0,
                   "close cached open file: %s, fd:%d, c:%d, u:%d, %d",
                   file->name, file->fd, file->count, file->uses, file->close);

    if (!file->close) {

        file->accessed = ngx_time();

        ngx_queue_remove(&file->queue);

        ngx_queue_insert_head(&cache->expire_queue, &file->queue);

        if (file->uses >= min_uses || file->count) {
            return;
        }
    }

    ngx_open_file_del_event(file);

    if (file->count) {
        return;
    }

    if (file->fd != NGX_INVALID_FILE) {

        if (ngx_close_file(file->fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                          ngx_close_file_n " \"%s\" failed", file->name);
        }

        file->fd = NGX_INVALID_FILE;
    }

    if (!file->close) {
        return;
    }

    ngx_free(file->name);
    ngx_free(file);
}
{% endhighlight %}
本函数用于关闭cache文件，但是如果```file->close```为0时，并不真正释放文件名等相关信息

## 7. 函数ngx_open_file_del_event()
{% highlight string %}
static void
ngx_open_file_del_event(ngx_cached_open_file_t *file)
{
    if (file->event == NULL) {
        return;
    }

    (void) ngx_del_event(file->event, NGX_VNODE_EVENT,
                         file->count ? NGX_FLUSH_EVENT : NGX_CLOSE_EVENT);

    ngx_free(file->event->data);
    ngx_free(file->event);
    file->event = NULL;
    file->use_event = 0;
}
{% endhighlight %}
从事件队列中删除该文件所对应的事件。

## 8. 函数ngx_expire_old_cached_files()
{% highlight string %}
static void
ngx_expire_old_cached_files(ngx_open_file_cache_t *cache, ngx_uint_t n,
    ngx_log_t *log)
{
    time_t                   now;
    ngx_queue_t             *q;
    ngx_cached_open_file_t  *file;

    now = ngx_time();

    /*
     * n == 1 deletes one or two inactive files
     * n == 0 deletes least recently used file by force
     *        and one or two inactive files
     */

    while (n < 3) {

        if (ngx_queue_empty(&cache->expire_queue)) {
            return;
        }

        q = ngx_queue_last(&cache->expire_queue);

        file = ngx_queue_data(q, ngx_cached_open_file_t, queue);

        if (n++ != 0 && now - file->accessed <= cache->inactive) {
            return;
        }

        ngx_queue_remove(q);

        ngx_rbtree_delete(&cache->rbtree, &file->node);

        cache->current--;

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
                       "expire cached open file: %s", file->name);

        if (!file->err && !file->is_dir) {
            file->close = 1;
            ngx_close_cached_file(cache, file, 0, log);

        } else {
            ngx_free(file->name);
            ngx_free(file);
        }
    }
}
{% endhighlight %}
当参数```n```取值如下时：

* ```n==0```: 强制从超时队列中淘汰一个最近未使用文件，并继续淘汰1~2个inactive状态的文件；

* ```n==1```: 从超时队列中淘汰1~2个inactive状态的文件；


## 9. 函数ngx_open_file_cache_rbtree_insert_value()
{% highlight string %}
static void
ngx_open_file_cache_rbtree_insert_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel)
{
    ngx_rbtree_node_t       **p;
    ngx_cached_open_file_t    *file, *file_temp;

    for ( ;; ) {

        if (node->key < temp->key) {

            p = &temp->left;

        } else if (node->key > temp->key) {

            p = &temp->right;

        } else { /* node->key == temp->key */

            file = (ngx_cached_open_file_t *) node;
            file_temp = (ngx_cached_open_file_t *) temp;

            p = (ngx_strcmp(file->name, file_temp->name) < 0)
                    ? &temp->left : &temp->right;
        }

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ngx_rbt_red(node);
}
{% endhighlight %}

本函数用于向红黑树中插入节点。注意到，这里插入时比较```key```的值，如果相等时，说明key冲突，那么直接比较文件名。

## 10. 函数ngx_open_file_lookup()
{% highlight string %}
static ngx_cached_open_file_t *
ngx_open_file_lookup(ngx_open_file_cache_t *cache, ngx_str_t *name,
    uint32_t hash)
{
    ngx_int_t                rc;
    ngx_rbtree_node_t       *node, *sentinel;
    ngx_cached_open_file_t  *file;

    node = cache->rbtree.root;
    sentinel = cache->rbtree.sentinel;

    while (node != sentinel) {

        if (hash < node->key) {
            node = node->left;
            continue;
        }

        if (hash > node->key) {
            node = node->right;
            continue;
        }

        /* hash == node->key */

        file = (ngx_cached_open_file_t *) node;

        rc = ngx_strcmp(name->data, file->name);

        if (rc == 0) {
            return file;
        }

        node = (rc < 0) ? node->left : node->right;
    }

    return NULL;
}

{% endhighlight %}
本函数用于从红黑树中查找缓存节点。

## 11. 函数ngx_open_file_cache_remove()
{% highlight string %}
static void
ngx_open_file_cache_remove(ngx_event_t *ev)
{
    ngx_cached_open_file_t       *file;
    ngx_open_file_cache_event_t  *fev;

    fev = ev->data;
    file = fev->file;

    ngx_queue_remove(&file->queue);

    ngx_rbtree_delete(&fev->cache->rbtree, &file->node);

    fev->cache->current--;

    /* NGX_ONESHOT_EVENT was already deleted */
    file->event = NULL;
    file->use_event = 0;

    file->close = 1;

    ngx_close_cached_file(fev->cache, file, 0, ev->log);

    /* free memory only when fev->cache and fev->file are already not needed */

    ngx_free(ev->data);
    ngx_free(ev);
}
{% endhighlight %}
本函数用于从cache中移除一个缓存文件。一般来说，当监听在某个文件上的事件发生时，说明这个文件发生了改变，此时我们只需要将其从缓存中移除即可，等下一次继续访问该文件时，则会重新记载从而获得最新的文件信息。

## 12. direct io方式读写文件（附录）
所谓direct io， 即不通过操作系统缓冲， 使用磁盘IO(或者DMA)直接将硬盘上的数据读入用户空间buffer， 或者将用户空间buffer中的数据通过磁盘IO(或者DMA)直接写到硬盘上。这样避免内核缓冲的消耗与CPU拷贝（数据在内核空间和用户空间之间的拷贝）的消耗。

### 12.1 direct io使用场景

direct io一般是通过DMA的方式来读取文件的。 通过direct io读取文件之前，一般需要初始化DMA, 因此一般使用direct io来读取大文件； 如果是读取小文件，初始化DMA的时间比系统读小文件的时间还长， 所以小文件使用direct io没有优势。对于大文件也只是在只读一次，并且后续没有其他应用再次读取此文件的时候，才有优势， 如果后续还有其他应用需要使用， 这个时候DirectIO也没有优势。


### 12.2 direct io使用示例
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


### 12.3 nginx aio与direct io
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

