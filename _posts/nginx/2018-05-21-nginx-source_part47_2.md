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


## 1. nginx静态文件缓存
这里在介绍具体的源代码之前，我们首先简要介绍一下Nginx服务器中静态文件Cache的一个大体处理流程。事实上，整个nginx中只有两个地方会用到这种静态文件缓存：

* ngx_http_core_module

* ngx_http_log_module

nginx静态文件缓存中，可以存储如下信息：

* 打开的文件描述符、文件大小、修改时间

* 目录是否存在等信息

* 与文件查找相关的任何错误消息，例如```file not found```、```no read permission```
<pre>
注意： 如果要缓存错误信息的话，那么还需要单独启用 'open_file_cache_errors' 子令
</pre>


其基本使用示例如下：
<pre>
open_file_cache max=64 inactive=30d;
open_file_cache_min_uses 8;
open_file_cache_valid 3m;
open_file_cache_errors   on;
</pre>
上面 **'max=64'** 表示设置缓冲文件的最大数目为64。超过此数目后，Nginx将按照LRU原则丢弃冷数据。**'inactive=30d'** 与 **'open_file_cache_min_uses 8'** 表示如果在30天内某文件的访问次数低于8次，那就将它从缓存中删除。

**'open_file_cache_valid 3m'**表示每3分钟检查一次缓存中的文件信息是否正确，如果不是则更新之。


### 1.1 原理介绍

这里我们主要介绍一下Linux下Nginx的静态文件缓存的实现。对于BSD版本，由于其可以通过kqueue来监听文件改变的事件，因此其实现会有一个不同的做法，这里我们不对其做详细介绍。

在Linux下，Nginx并没有使用```Inotify```，而是每次都会判断文件的st_ino来得到文件是否被修改，不过这样会有个缺点就是如果你是使用open()，然后write()来修改文件的话，因为是属于修改同一个文件，因此```st_ino```是相同的，此时Nginx是无法感知到文件被修改了。因此要修改文件的话，最好使用先删除再覆盖的命令（比如cp命令)。


Nginx中的Cache只是cache文件句柄，因为静态文件的发送，一般来说，Nginx都是尽量使用sendfile()来进行发送的。因此只需要cache文件句柄就足够了。

所有的Cache对象包含在两个数据结构里面，整个机制最关键的也是这两个东西，一个是红黑树，另一个就是队列。其中红黑树是为了方便查找（能根据文件名迅速得到fd)，而队列是为了方便超时管理（按照读取顺序插入，在队列头的就是最近存取的文件），由于所有的句柄的超时时间都是一样的，因此每次只需要判断最后几个元素就够了，因为这个超时并不需要那么实时。如下画出cache存储结构的整体图景：

![ngx-file-cache](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_file_cache.jpg)



假如现在客户端的请求是```GET test.html HTTP/1.1```，则Nginx是这么处理的，如果test.html在cache中存在，则从cache中取得这个句柄，然后正常返回；如果test.html不存在，则是打开这个文件，然后插入到cache中。不过这里面有很多细节需要处理，比如超时，比如红黑树的插入等等。我们会在后面的章节来较为详细的介绍这些。





### 1.2 文件缓存更新周期
上面提到的配置文件中，若文件在30天内访问的次数低于8次，那么将会从缓存中丢弃。每3分钟做一次信息有效性监测，这里我们暂且把```3分钟```称为```缓存更新周期```。那在这3分钟之内文件发生变化了会怎样呢？

1） **文件被删除**

由于nginx还持有原文件的fd，所以你删除此文件后，文件并不会真正消失，client还是能够通过原路径访问此文件。即便你删除后又新建了一个同名文件，在当前缓存更新周期内能访问到的还是原文件的内容。

2） **文件内容被修改**

文件内容被修改可以分成两种情况：

* 文件大小不变或增大： 由于nginx缓存了文件的size，并且使用缓存中的size调用sendfile(2)，所以此种情况的后果是：
<pre>
1. 从文件开始到原size字节中的变化可以被client看到；

2. 原size之后的内容不会被sendfile(2)发送，因此client看不到此部分内容；
</pre>


* 文件大小减小： 此种情况下，由于同样的原因，nginx在HTTP Header中告诉client文件大小还是原来的尺寸，而sendfile(2)只能发送真正的文件数据，长度小于HTTP Header中设置的大小，所以client会等待到自己超时或者Nginx在epoll_wait超时后关闭连接。


### 1.3 如何设置Nginx静态缓存
我们在使用Nginx静态文件缓存时，可以考虑如下：

* 如果你的静态文件内容变化频繁并且对时效性要求较高，一般应该把 **'open_file_cache_valid'** 设置的小一些，以便及时检测和更新；

* 如果变化相当不频繁的话， 就可以设置大一点。在变化后用reload nginx的方式来强制更新缓存；

* 对静态文件访问的error和access log不关心的话，可以关闭以提升效率。



## 3. 相关静态函数声明
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


/*
 * open file cache caches
 *    open file handles with stat() info;
 *    directories stat() info;
 *    files and directories errors: not found, access denied, etc.
 */


#define NGX_MIN_READ_AHEAD  (128 * 1024)


//用于清除超时队列中的所有
static void ngx_open_file_cache_cleanup(void *data);


#if (NGX_HAVE_OPENAT)
//使用openat()来打开一个文件，因为openat()打开的文件并不能判定是不是链接文件，所以需要进一步判断
static ngx_fd_t ngx_openat_file_owner(ngx_fd_t at_fd, const u_char *name,
    ngx_int_t mode, ngx_int_t create, ngx_int_t access, ngx_log_t *log);
#if (NGX_HAVE_O_PATH)

//使用O_PATH选项来打开一个文件或目录以获得相应的文件信息
static ngx_int_t ngx_file_o_path_info(ngx_fd_t fd, ngx_file_info_t *fi,
    ngx_log_t *log);
#endif
#endif

//打开一个文件，主要是封装了普通打开方式与openat()
static ngx_fd_t ngx_open_file_wrapper(ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_int_t mode, ngx_int_t create,
    ngx_int_t access, ngx_log_t *log);

//获取文件信息
static ngx_int_t ngx_file_info_wrapper(ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_file_info_t *fi, ngx_log_t *log);

//打开并stat一个文件
static ngx_int_t ngx_open_and_stat_file(ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_log_t *log);

//打开一个文件并添加相应的事件
static void ngx_open_file_add_event(ngx_open_file_cache_t *cache,
    ngx_cached_open_file_t *file, ngx_open_file_info_t *of, ngx_log_t *log);

//清除一个打开的文件信息
static void ngx_open_file_cleanup(void *data);

//关闭缓存文件
static void ngx_close_cached_file(ngx_open_file_cache_t *cache,
    ngx_cached_open_file_t *file, ngx_uint_t min_uses, ngx_log_t *log);

//删除关联在文件上的事件
static void ngx_open_file_del_event(ngx_cached_open_file_t *file);

//淘汰超时的、老旧的缓存文件
static void ngx_expire_old_cached_files(ngx_open_file_cache_t *cache,
    ngx_uint_t n, ngx_log_t *log);

//将缓存文件信息插入到红黑树中
static void ngx_open_file_cache_rbtree_insert_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel);

//从红黑树中查找文件
static ngx_cached_open_file_t *
    ngx_open_file_lookup(ngx_open_file_cache_t *cache, ngx_str_t *name,
    uint32_t hash);

//移除事件所关联的文件缓存信息
static void ngx_open_file_cache_remove(ngx_event_t *ev);
{% endhighlight %}

这里nginx对打开文件的缓存，主要有三种类型：

1) 打开文件的句柄，以及该文件对应额stat()信息

2） 目录的stat()信息；

3） 与文件查找相关的任何错误消息，例如```file not found```、```no read permission```


## 4. 函数ngx_open_file_cache_init()
{% highlight string %}
ngx_open_file_cache_t *
ngx_open_file_cache_init(ngx_pool_t *pool, ngx_uint_t max, time_t inactive)
{
    ngx_pool_cleanup_t     *cln;
    ngx_open_file_cache_t  *cache;

    cache = ngx_palloc(pool, sizeof(ngx_open_file_cache_t));
    if (cache == NULL) {
        return NULL;
    }

    ngx_rbtree_init(&cache->rbtree, &cache->sentinel,
                    ngx_open_file_cache_rbtree_insert_value);

    ngx_queue_init(&cache->expire_queue);

    cache->current = 0;
    cache->max = max;
    cache->inactive = inactive;

    cln = ngx_pool_cleanup_add(pool, 0);
    if (cln == NULL) {
        return NULL;
    }

    cln->handler = ngx_open_file_cache_cleanup;
    cln->data = cache;

    return cache;
}

{% endhighlight %}
本函数较为简单，主要是初始化了```ngx_open_file_cache_t```数据结构，并将其加入到pool cleanup中，来让其自动的管理cache的回收工作。

## 5. 函数ngx_open_file_cache_cleanup()
{% highlight string %}
static void
ngx_open_file_cache_cleanup(void *data)
{
    ngx_open_file_cache_t  *cache = data;

    ngx_queue_t             *q;
    ngx_cached_open_file_t  *file;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                   "open file cache cleanup");

    for ( ;; ) {

        if (ngx_queue_empty(&cache->expire_queue)) {
            break;
        }

        q = ngx_queue_last(&cache->expire_queue);

        file = ngx_queue_data(q, ngx_cached_open_file_t, queue);

        ngx_queue_remove(q);

        ngx_rbtree_delete(&cache->rbtree, &file->node);

        cache->current--;

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                       "delete cached open file: %s", file->name);

        if (!file->err && !file->is_dir) {
            file->close = 1;
            file->count = 0;
            ngx_close_cached_file(cache, file, 0, ngx_cycle->log);

        } else {
            ngx_free(file->name);
            ngx_free(file);
        }
    }

    if (cache->current) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                      "%ui items still left in open file cache",
                      cache->current);
    }

    if (cache->rbtree.root != cache->rbtree.sentinel) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                      "rbtree still is not empty in open file cache");

    }
}
{% endhighlight %}
本函数用于从超时队列及红黑树中删除相应的缓存文件数据。一般来说，把所有的缓存数据从超时队列中移除之后，**cache->current**的值就会为0，如果不为0，表示出现了相应的错误，这里我们也只是打印相应的错误日志消息。


## 6. 函数ngx_open_cached_file()
{% highlight string %}
ngx_int_t
ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool)
{
    time_t                          now;
    uint32_t                        hash;
    ngx_int_t                       rc;
    ngx_file_info_t                 fi;
    ngx_pool_cleanup_t             *cln;
    ngx_cached_open_file_t         *file;
    ngx_pool_cleanup_file_t        *clnf;
    ngx_open_file_cache_cleanup_t  *ofcln;

    of->fd = NGX_INVALID_FILE;
    of->err = 0;

    if (cache == NULL) {

        if (of->test_only) {

            if (ngx_file_info_wrapper(name, of, &fi, pool->log)
                == NGX_FILE_ERROR)
            {
                return NGX_ERROR;
            }

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

        cln = ngx_pool_cleanup_add(pool, sizeof(ngx_pool_cleanup_file_t));
        if (cln == NULL) {
            return NGX_ERROR;
        }

        rc = ngx_open_and_stat_file(name, of, pool->log);

        if (rc == NGX_OK && !of->is_dir) {
            cln->handler = ngx_pool_cleanup_file;
            clnf = cln->data;

            clnf->fd = of->fd;
            clnf->name = name->data;
            clnf->log = pool->log;
        }

        return rc;
    }

    cln = ngx_pool_cleanup_add(pool, sizeof(ngx_open_file_cache_cleanup_t));
    if (cln == NULL) {
        return NGX_ERROR;
    }

    now = ngx_time();

    hash = ngx_crc32_long(name->data, name->len);

    file = ngx_open_file_lookup(cache, name, hash);

    if (file) {

        file->uses++;

        ngx_queue_remove(&file->queue);

        if (file->fd == NGX_INVALID_FILE && file->err == 0 && !file->is_dir) {

            /* file was not used often enough to keep open */

            rc = ngx_open_and_stat_file(name, of, pool->log);

            if (rc != NGX_OK && (of->err == 0 || !of->errors)) {
                goto failed;
            }

            goto add_event;
        }

        if (file->use_event
            || (file->event == NULL
                && (of->uniq == 0 || of->uniq == file->uniq)
                && now - file->created < of->valid
#if (NGX_HAVE_OPENAT)
                && of->disable_symlinks == file->disable_symlinks
                && of->disable_symlinks_from == file->disable_symlinks_from
#endif
            ))
        {
            if (file->err == 0) {

                of->fd = file->fd;
                of->uniq = file->uniq;
                of->mtime = file->mtime;
                of->size = file->size;

                of->is_dir = file->is_dir;
                of->is_file = file->is_file;
                of->is_link = file->is_link;
                of->is_exec = file->is_exec;
                of->is_directio = file->is_directio;

                if (!file->is_dir) {
                    file->count++;
                    ngx_open_file_add_event(cache, file, of, pool->log);
                }

            } else {
                of->err = file->err;
#if (NGX_HAVE_OPENAT)
                of->failed = file->disable_symlinks ? ngx_openat_file_n
                                                    : ngx_open_file_n;
#else
                of->failed = ngx_open_file_n;
#endif
            }

            goto found;
        }

        ngx_log_debug4(NGX_LOG_DEBUG_CORE, pool->log, 0,
                       "retest open file: %s, fd:%d, c:%d, e:%d",
                       file->name, file->fd, file->count, file->err);

        if (file->is_dir) {

            /*
             * chances that directory became file are very small
             * so test_dir flag allows to use a single syscall
             * in ngx_file_info() instead of three syscalls
             */

            of->test_dir = 1;
        }

        of->fd = file->fd;
        of->uniq = file->uniq;

        rc = ngx_open_and_stat_file(name, of, pool->log);

        if (rc != NGX_OK && (of->err == 0 || !of->errors)) {
            goto failed;
        }

        if (of->is_dir) {

            if (file->is_dir || file->err) {
                goto update;
            }

            /* file became directory */

        } else if (of->err == 0) {  /* file */

            if (file->is_dir || file->err) {
                goto add_event;
            }

            if (of->uniq == file->uniq) {

                if (file->event) {
                    file->use_event = 1;
                }

                of->is_directio = file->is_directio;

                goto update;
            }

            /* file was changed */

        } else { /* error to cache */

            if (file->err || file->is_dir) {
                goto update;
            }

            /* file was removed, etc. */
        }

        if (file->count == 0) {

            ngx_open_file_del_event(file);

            if (ngx_close_file(file->fd) == NGX_FILE_ERROR) {
                ngx_log_error(NGX_LOG_ALERT, pool->log, ngx_errno,
                              ngx_close_file_n " \"%V\" failed", name);
            }

            goto add_event;
        }

        ngx_rbtree_delete(&cache->rbtree, &file->node);

        cache->current--;

        file->close = 1;

        goto create;
    }

    /* not found */

    rc = ngx_open_and_stat_file(name, of, pool->log);

    if (rc != NGX_OK && (of->err == 0 || !of->errors)) {
        goto failed;
    }

create:

    if (cache->current >= cache->max) {
        ngx_expire_old_cached_files(cache, 0, pool->log);
    }

    file = ngx_alloc(sizeof(ngx_cached_open_file_t), pool->log);

    if (file == NULL) {
        goto failed;
    }

    file->name = ngx_alloc(name->len + 1, pool->log);

    if (file->name == NULL) {
        ngx_free(file);
        file = NULL;
        goto failed;
    }

    ngx_cpystrn(file->name, name->data, name->len + 1);

    file->node.key = hash;

    ngx_rbtree_insert(&cache->rbtree, &file->node);

    cache->current++;

    file->uses = 1;
    file->count = 0;
    file->use_event = 0;
    file->event = NULL;

add_event:

    ngx_open_file_add_event(cache, file, of, pool->log);

update:

    file->fd = of->fd;
    file->err = of->err;
#if (NGX_HAVE_OPENAT)
    file->disable_symlinks = of->disable_symlinks;
    file->disable_symlinks_from = of->disable_symlinks_from;
#endif

    if (of->err == 0) {
        file->uniq = of->uniq;
        file->mtime = of->mtime;
        file->size = of->size;

        file->close = 0;

        file->is_dir = of->is_dir;
        file->is_file = of->is_file;
        file->is_link = of->is_link;
        file->is_exec = of->is_exec;
        file->is_directio = of->is_directio;

        if (!of->is_dir) {
            file->count++;
        }
    }

    file->created = now;

found:

    file->accessed = now;

    ngx_queue_insert_head(&cache->expire_queue, &file->queue);

    ngx_log_debug5(NGX_LOG_DEBUG_CORE, pool->log, 0,
                   "cached open file: %s, fd:%d, c:%d, e:%d, u:%d",
                   file->name, file->fd, file->count, file->err, file->uses);

    if (of->err == 0) {

        if (!of->is_dir) {
            cln->handler = ngx_open_file_cleanup;
            ofcln = cln->data;

            ofcln->cache = cache;
            ofcln->file = file;
            ofcln->min_uses = of->min_uses;
            ofcln->log = pool->log;
        }

        return NGX_OK;
    }

    return NGX_ERROR;

failed:

    if (file) {
        ngx_rbtree_delete(&cache->rbtree, &file->node);

        cache->current--;

        if (file->count == 0) {

            if (file->fd != NGX_INVALID_FILE) {
                if (ngx_close_file(file->fd) == NGX_FILE_ERROR) {
                    ngx_log_error(NGX_LOG_ALERT, pool->log, ngx_errno,
                                  ngx_close_file_n " \"%s\" failed",
                                  file->name);
                }
            }

            ngx_free(file->name);
            ngx_free(file);

        } else {
            file->close = 1;
        }
    }

    if (of->fd != NGX_INVALID_FILE) {
        if (ngx_close_file(of->fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, pool->log, ngx_errno,
                          ngx_close_file_n " \"%V\" failed", name);
        }
    }

    return NGX_ERROR;
}
{% endhighlight %}

nginx静态文件缓存的主要代码都包含在本函数中。下面我们详细介绍一下：

1） **处理cache为NULL时的特殊情况**
{% highlight string %}
ngx_int_t
ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool)
{
	...
	if (cache == NULL) {

        if (of->test_only) {

            if (ngx_file_info_wrapper(name, of, &fi, pool->log)
                == NGX_FILE_ERROR)
            {
                return NGX_ERROR;
            }

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

        cln = ngx_pool_cleanup_add(pool, sizeof(ngx_pool_cleanup_file_t));
        if (cln == NULL) {
            return NGX_ERROR;
        }

        rc = ngx_open_and_stat_file(name, of, pool->log);

        if (rc == NGX_OK && !of->is_dir) {
            cln->handler = ngx_pool_cleanup_file;
            clnf = cln->data;

            clnf->fd = of->fd;
            clnf->name = name->data;
            clnf->log = pool->log;
        }

        return rc;
    }
	...
}
{% endhighlight %}
一般只有在临时情况下传递的cache参数才会为NULL，这种情况通常只是简单获取```ngx_open_file_info_t```信息。此种情况下我们看到也会从pool池中申请一块小内存来管理打开文件的清除操作，但是注意这里绑定的handler也只是单个文件的关闭，与上面我们介绍的 **'清除cache中的所有打开的缓存文件'**是不同的。


2) **查找文件**
{% highlight string %}
ngx_int_t
ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool)
{
	...
	cln = ngx_pool_cleanup_add(pool, sizeof(ngx_open_file_cache_cleanup_t));
    if (cln == NULL) {
        return NGX_ERROR;
    }

    now = ngx_time();

    hash = ngx_crc32_long(name->data, name->len);

    file = ngx_open_file_lookup(cache, name, hash);

	...
}
{% endhighlight %}
这里首先会从池中申请一小块```ngx_open_file_cache_cleanup_t```空间，用于清除单个打开的文件，我们可以从后边所绑定的handler看出其中的不同。接着通过要打开的文件名```name```来从红黑树中查找相应的缓存。这里我们注意到，是对文件名做crc32之后，来进行查找的，也即是说红黑树中所存储信息的key是crc32(file_name)。

3) **缓存文件查找成功**
{% highlight string %}
ngx_int_t
ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool)
{
	...
	    if (file) {

        file->uses++;

        ngx_queue_remove(&file->queue);

        if (file->fd == NGX_INVALID_FILE && file->err == 0 && !file->is_dir) {

            /* file was not used often enough to keep open */

            rc = ngx_open_and_stat_file(name, of, pool->log);

            if (rc != NGX_OK && (of->err == 0 || !of->errors)) {
                goto failed;
            }

            goto add_event;
        }

        if (file->use_event
            || (file->event == NULL
                && (of->uniq == 0 || of->uniq == file->uniq)
                && now - file->created < of->valid
#if (NGX_HAVE_OPENAT)
                && of->disable_symlinks == file->disable_symlinks
                && of->disable_symlinks_from == file->disable_symlinks_from
#endif
            ))
        {
            if (file->err == 0) {

                of->fd = file->fd;
                of->uniq = file->uniq;
                of->mtime = file->mtime;
                of->size = file->size;

                of->is_dir = file->is_dir;
                of->is_file = file->is_file;
                of->is_link = file->is_link;
                of->is_exec = file->is_exec;
                of->is_directio = file->is_directio;

                if (!file->is_dir) {
                    file->count++;
                    ngx_open_file_add_event(cache, file, of, pool->log);
                }

            } else {
                of->err = file->err;
#if (NGX_HAVE_OPENAT)
                of->failed = file->disable_symlinks ? ngx_openat_file_n
                                                    : ngx_open_file_n;
#else
                of->failed = ngx_open_file_n;
#endif
            }

            goto found;
        }

        ngx_log_debug4(NGX_LOG_DEBUG_CORE, pool->log, 0,
                       "retest open file: %s, fd:%d, c:%d, e:%d",
                       file->name, file->fd, file->count, file->err);

        if (file->is_dir) {

            /*
             * chances that directory became file are very small
             * so test_dir flag allows to use a single syscall
             * in ngx_file_info() instead of three syscalls
             */

            of->test_dir = 1;
        }

        of->fd = file->fd;
        of->uniq = file->uniq;

        rc = ngx_open_and_stat_file(name, of, pool->log);

        if (rc != NGX_OK && (of->err == 0 || !of->errors)) {
            goto failed;
        }

        if (of->is_dir) {

            if (file->is_dir || file->err) {
                goto update;
            }

            /* file became directory */

        } else if (of->err == 0) {  /* file */

            if (file->is_dir || file->err) {
                goto add_event;
            }

            if (of->uniq == file->uniq) {

                if (file->event) {
                    file->use_event = 1;
                }

                of->is_directio = file->is_directio;

                goto update;
            }

            /* file was changed */

        } else { /* error to cache */

            if (file->err || file->is_dir) {
                goto update;
            }

            /* file was removed, etc. */
        }

        if (file->count == 0) {

            ngx_open_file_del_event(file);

            if (ngx_close_file(file->fd) == NGX_FILE_ERROR) {
                ngx_log_error(NGX_LOG_ALERT, pool->log, ngx_errno,
                              ngx_close_file_n " \"%V\" failed", name);
            }

            goto add_event;
        }

        ngx_rbtree_delete(&cache->rbtree, &file->node);

        cache->current--;

        file->close = 1;

        goto create;
    }
	...
}
{% endhighlight %}
在第2）步中，我们会从红黑树中查找文件。如果查找成功，那么使用对应的fd来获取文件的最新信息，然后更新缓存并返回。另外：
{% highlight string %}
if (file->use_event
            || (file->event == NULL
                && (of->uniq == 0 || of->uniq == file->uniq)
                && now - file->created < of->valid
#if (NGX_HAVE_OPENAT)
                && of->disable_symlinks == file->disable_symlinks
                && of->disable_symlinks_from == file->disable_symlinks_from
#endif
            ))
{
	//处理支持文件事件的情况，即VNODE事件
	goto found;
}


//不支持文件事件，则根据需要打开文件以获得相应的信息
rc = ngx_open_and_stat_file(name, of, pool->log);
{% endhighlight %}


**4） cache中文件未找到**
{% highlight string %}
ngx_int_t
ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool)
{
	...
	    /* not found */

    rc = ngx_open_and_stat_file(name, of, pool->log);

    if (rc != NGX_OK && (of->err == 0 || !of->errors)) {
        goto failed;
    }

create:

    if (cache->current >= cache->max) {
        ngx_expire_old_cached_files(cache, 0, pool->log);
    }

    file = ngx_alloc(sizeof(ngx_cached_open_file_t), pool->log);

    if (file == NULL) {
        goto failed;
    }

    file->name = ngx_alloc(name->len + 1, pool->log);

    if (file->name == NULL) {
        ngx_free(file);
        file = NULL;
        goto failed;
    }

    ngx_cpystrn(file->name, name->data, name->len + 1);

    file->node.key = hash;

    ngx_rbtree_insert(&cache->rbtree, &file->node);

    cache->current++;

    file->uses = 1;
    file->count = 0;
    file->use_event = 0;
    file->event = NULL;
	...
}
{% endhighlight %}
此处，cache中未找到对应的缓存文件，那么Nginx就会打开该文件，然后将其存入缓存中。这里我们注意到```cache->current```达到了指定的最大值时，Nginx就会强制淘汰若干个（3个以下）文件。

**5) 更新相应文件信息，并插入到超时队列**
{% highlight string %}
ngx_int_t
ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool)
{
	...
add_event:

    ngx_open_file_add_event(cache, file, of, pool->log);

update:

    file->fd = of->fd;
    file->err = of->err;
#if (NGX_HAVE_OPENAT)
    file->disable_symlinks = of->disable_symlinks;
    file->disable_symlinks_from = of->disable_symlinks_from;
#endif

    if (of->err == 0) {
        file->uniq = of->uniq;
        file->mtime = of->mtime;
        file->size = of->size;

        file->close = 0;

        file->is_dir = of->is_dir;
        file->is_file = of->is_file;
        file->is_link = of->is_link;
        file->is_exec = of->is_exec;
        file->is_directio = of->is_directio;

        if (!of->is_dir) {
            file->count++;
        }
    }

    file->created = now;

found:

    file->accessed = now;

    ngx_queue_insert_head(&cache->expire_queue, &file->queue);

    ngx_log_debug5(NGX_LOG_DEBUG_CORE, pool->log, 0,
                   "cached open file: %s, fd:%d, c:%d, e:%d, u:%d",
                   file->name, file->fd, file->count, file->err, file->uses);

    if (of->err == 0) {

        if (!of->is_dir) {
            cln->handler = ngx_open_file_cleanup;
            ofcln = cln->data;

            ofcln->cache = cache;
            ofcln->file = file;
            ofcln->min_uses = of->min_uses;
            ofcln->log = pool->log;
        }

        return NGX_OK;
    }

    return NGX_ERROR;
	...
}
{% endhighlight %}
此处，更新前面获取到的文件信息到```file```变量，然后再插入到超时队列。值得注意的是，这里同样会为该打开的缓存文件绑定一个```缓存清除回调```函数。

**6） 处理失败情况**
{% highlight string %}
ngx_int_t
ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool)
{
	...
failed:

    if (file) {
        ngx_rbtree_delete(&cache->rbtree, &file->node);

        cache->current--;

        if (file->count == 0) {

            if (file->fd != NGX_INVALID_FILE) {
                if (ngx_close_file(file->fd) == NGX_FILE_ERROR) {
                    ngx_log_error(NGX_LOG_ALERT, pool->log, ngx_errno,
                                  ngx_close_file_n " \"%s\" failed",
                                  file->name);
                }
            }

            ngx_free(file->name);
            ngx_free(file);

        } else {
            file->close = 1;
        }
    }

    if (of->fd != NGX_INVALID_FILE) {
        if (ngx_close_file(of->fd) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, pool->log, ngx_errno,
                          ngx_close_file_n " \"%V\" failed", name);
        }
    }

    return NGX_ERROR;
	...
}
{% endhighlight %}
处理失败时，从缓存中清除相应的缓存信息，并关闭对应的文件句柄。


## 7. 函数
{% highlight string %}
#if (NGX_HAVE_OPENAT)
static ngx_fd_t
ngx_openat_file_owner(ngx_fd_t at_fd, const u_char *name,
    ngx_int_t mode, ngx_int_t create, ngx_int_t access, ngx_log_t *log)
{
    ngx_fd_t         fd;
    ngx_err_t        err;
    ngx_file_info_t  fi, atfi;

    /*
     * To allow symlinks with the same owner, use openat() (followed
     * by fstat()) and fstatat(AT_SYMLINK_NOFOLLOW), and then compare
     * uids between fstat() and fstatat().
     *
     * As there is a race between openat() and fstatat() we don't
     * know if openat() in fact opened symlink or not.  Therefore,
     * we have to compare uids even if fstatat() reports the opened
     * component isn't a symlink (as we don't know whether it was
     * symlink during openat() or not).
     */

    fd = ngx_openat_file(at_fd, name, mode, create, access);

    if (fd == NGX_INVALID_FILE) {
        return NGX_INVALID_FILE;
    }

    if (ngx_file_at_info(at_fd, name, &atfi, AT_SYMLINK_NOFOLLOW)
        == NGX_FILE_ERROR)
    {
        err = ngx_errno;
        goto failed;
    }

#if (NGX_HAVE_O_PATH)
    if (ngx_file_o_path_info(fd, &fi, log) == NGX_ERROR) {
        err = ngx_errno;
        goto failed;
    }
#else
    if (ngx_fd_info(fd, &fi) == NGX_FILE_ERROR) {
        err = ngx_errno;
        goto failed;
    }
#endif

    if (fi.st_uid != atfi.st_uid) {
        err = NGX_ELOOP;
        goto failed;
    }

    return fd;

failed:

    if (ngx_close_file(fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", name);
    }

    ngx_set_errno(err);

    return NGX_INVALID_FILE;
}
#endif
{% endhighlight %}

这里由于```openat()```并不能判断出所打开的文件是否是一个链接文件，因此必须要比较fstat()与fstatat()获取到的owner信息。

## 8. 函数ngx_file_o_path_info()
{% highlight string %}
#if (NGX_HAVE_OPENAT)
#if (NGX_HAVE_O_PATH)

static ngx_int_t
ngx_file_o_path_info(ngx_fd_t fd, ngx_file_info_t *fi, ngx_log_t *log)
{
    static ngx_uint_t  use_fstat = 1;

    /*
     * In Linux 2.6.39 the O_PATH flag was introduced that allows to obtain
     * a descriptor without actually opening file or directory.  It requires
     * less permissions for path components, but till Linux 3.6 fstat() returns
     * EBADF on such descriptors, and fstatat() with the AT_EMPTY_PATH flag
     * should be used instead.
     *
     * Three scenarios are handled in this function:
     *
     * 1) The kernel is newer than 3.6 or fstat() with O_PATH support was
     *    backported by vendor.  Then fstat() is used.
     *
     * 2) The kernel is newer than 2.6.39 but older than 3.6.  In this case
     *    the first call of fstat() returns EBADF and we fallback to fstatat()
     *    with AT_EMPTY_PATH which was introduced at the same time as O_PATH.
     *
     * 3) The kernel is older than 2.6.39 but nginx was build with O_PATH
     *    support.  Since descriptors are opened with O_PATH|O_RDONLY flags
     *    and O_PATH is ignored by the kernel then the O_RDONLY flag is
     *    actually used.  In this case fstat() just works.
     */

    if (use_fstat) {
        if (ngx_fd_info(fd, fi) != NGX_FILE_ERROR) {
            return NGX_OK;
        }

        if (ngx_errno != NGX_EBADF) {
            return NGX_ERROR;
        }

        ngx_log_error(NGX_LOG_NOTICE, log, 0,
                      "fstat(O_PATH) failed with EBADF, "
                      "switching to fstatat(AT_EMPTY_PATH)");

        use_fstat = 0;
    }

    if (ngx_file_at_info(fd, "", fi, AT_EMPTY_PATH) != NGX_FILE_ERROR) {
        return NGX_OK;
    }

    return NGX_ERROR;
}

#endif

#endif /* NGX_HAVE_OPENAT */
{% endhighlight %}
此函数也主要用于支持使用openat()函数来获取文件信息。




<br />
<br />

**[参看]**

1. [Nginx服务器中静态文件cache的处理流程](https://blog.csdn.net/otion20122/article/details/73332449)

2. [使用Nginx缓存静态文件](https://blog.csdn.net/f529352479/article/details/68484280)

3. [Nginx Open File Cache](https://www.cnblogs.com/cmfwm/p/7659179.html)

4. [Nginx使用教程(五)：使用Nginx缓存之缓存静态内容](https://www.cnblogs.com/felixzh/p/6283896.html)

5. [open_file_cache](http://nginx.org/en/docs/http/ngx_http_core_module.html#open_file_cache)

<br />
<br />
<br />

