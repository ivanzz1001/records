---
layout: post
title: os/unix/ngx_linux_sendfile_chain.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要分析一下ngx_linux_sendfile_chain.c源文件。

<!-- more -->


<br />
<br />


## 1. 相关函数声明
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


static ssize_t ngx_linux_sendfile(ngx_connection_t *c, ngx_buf_t *file,
    size_t size);

#if (NGX_THREADS)
#include <ngx_thread_pool.h>

#if !(NGX_HAVE_SENDFILE64)
#error sendfile64() is required!
#endif

static ngx_int_t ngx_linux_sendfile_thread(ngx_connection_t *c, ngx_buf_t *file,
    size_t size, size_t *sent);
static void ngx_linux_sendfile_thread_handler(void *data, ngx_log_t *log);
#endif
{% endhighlight %}


当前我们并不支持```NGX_THREADS```，系统支持```NGX_HAVE_SENDFILE64```。

## 2. 函数ngx_linux_sendfile_chain()
{% highlight string %}
/*
 * On Linux up to 2.4.21 sendfile() (syscall #187) works with 32-bit
 * offsets only, and the including <sys/sendfile.h> breaks the compiling,
 * if off_t is 64 bit wide.  So we use own sendfile() definition, where offset
 * parameter is int32_t, and use sendfile() for the file parts below 2G only,
 * see src/os/unix/ngx_linux_config.h
 *
 * Linux 2.4.21 has the new sendfile64() syscall #239.
 *
 * On Linux up to 2.6.16 sendfile() does not allow to pass the count parameter
 * more than 2G-1 bytes even on 64-bit platforms: it returns EINVAL,
 * so we limit it to 2G-1 bytes.
 */

#define NGX_SENDFILE_MAXSIZE  2147483647L


ngx_chain_t *
ngx_linux_sendfile_chain(ngx_connection_t *c, ngx_chain_t *in, off_t limit)
{
    int            tcp_nodelay;
    off_t          send, prev_send;
    size_t         file_size, sent;
    ssize_t        n;
    ngx_err_t      err;
    ngx_buf_t     *file;
    ngx_event_t   *wev;
    ngx_chain_t   *cl;
    ngx_iovec_t    header;
    struct iovec   headers[NGX_IOVS_PREALLOCATE];
#if (NGX_THREADS)
    ngx_int_t      rc;
    ngx_uint_t     thread_handled, thread_complete;
#endif

    wev = c->write;

    if (!wev->ready) {
        return in;
    }


    /* the maximum limit size is 2G-1 - the page size */

    if (limit == 0 || limit > (off_t) (NGX_SENDFILE_MAXSIZE - ngx_pagesize)) {
        limit = NGX_SENDFILE_MAXSIZE - ngx_pagesize;
    }


    send = 0;

    header.iovs = headers;
    header.nalloc = NGX_IOVS_PREALLOCATE;

    for ( ;; ) {
        prev_send = send;
#if (NGX_THREADS)
        thread_handled = 0;
        thread_complete = 0;
#endif

        /* create the iovec and coalesce the neighbouring bufs */

        cl = ngx_output_chain_to_iovec(&header, in, limit - send, c->log);

        if (cl == NGX_CHAIN_ERROR) {
            return NGX_CHAIN_ERROR;
        }

        send += header.size;

        /* set TCP_CORK if there is a header before a file */

        if (c->tcp_nopush == NGX_TCP_NOPUSH_UNSET
            && header.count != 0
            && cl
            && cl->buf->in_file)
        {
            /* the TCP_CORK and TCP_NODELAY are mutually exclusive */

            if (c->tcp_nodelay == NGX_TCP_NODELAY_SET) {

                tcp_nodelay = 0;

                if (setsockopt(c->fd, IPPROTO_TCP, TCP_NODELAY,
                               (const void *) &tcp_nodelay, sizeof(int)) == -1)
                {
                    err = ngx_socket_errno;

                    /*
                     * there is a tiny chance to be interrupted, however,
                     * we continue a processing with the TCP_NODELAY
                     * and without the TCP_CORK
                     */

                    if (err != NGX_EINTR) {
                        wev->error = 1;
                        ngx_connection_error(c, err,
                                             "setsockopt(TCP_NODELAY) failed");
                        return NGX_CHAIN_ERROR;
                    }

                } else {
                    c->tcp_nodelay = NGX_TCP_NODELAY_UNSET;

                    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0,
                                   "no tcp_nodelay");
                }
            }

            if (c->tcp_nodelay == NGX_TCP_NODELAY_UNSET) {

                if (ngx_tcp_nopush(c->fd) == NGX_ERROR) {
                    err = ngx_socket_errno;

                    /*
                     * there is a tiny chance to be interrupted, however,
                     * we continue a processing without the TCP_CORK
                     */

                    if (err != NGX_EINTR) {
                        wev->error = 1;
                        ngx_connection_error(c, err,
                                             ngx_tcp_nopush_n " failed");
                        return NGX_CHAIN_ERROR;
                    }

                } else {
                    c->tcp_nopush = NGX_TCP_NOPUSH_SET;

                    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0,
                                   "tcp_nopush");
                }
            }
        }

        /* get the file buf */

        if (header.count == 0 && cl && cl->buf->in_file && send < limit) {
            file = cl->buf;

            /* coalesce the neighbouring file bufs */

            file_size = (size_t) ngx_chain_coalesce_file(&cl, limit - send);

            send += file_size;
#if 1
            if (file_size == 0) {
                ngx_debug_point();
                return NGX_CHAIN_ERROR;
            }
#endif

#if (NGX_THREADS)
            if (file->file->thread_handler) {
                rc = ngx_linux_sendfile_thread(c, file, file_size, &sent);

                switch (rc) {
                case NGX_OK:
                    thread_handled = 1;
                    break;

                case NGX_DONE:
                    thread_complete = 1;
                    break;

                case NGX_AGAIN:
                    break;

                default: /* NGX_ERROR */
                    return NGX_CHAIN_ERROR;
                }

            } else
#endif
            {
                n = ngx_linux_sendfile(c, file, file_size);

                if (n == NGX_ERROR) {
                    return NGX_CHAIN_ERROR;
                }

                sent = (n == NGX_AGAIN) ? 0 : n;
            }

        } else {
            n = ngx_writev(c, &header);

            if (n == NGX_ERROR) {
                return NGX_CHAIN_ERROR;
            }

            sent = (n == NGX_AGAIN) ? 0 : n;
        }

        c->sent += sent;

        in = ngx_chain_update_sent(in, sent);

        if ((size_t) (send - prev_send) != sent) {
#if (NGX_THREADS)
            if (thread_handled) {
                return in;
            }

            if (thread_complete) {
                send = prev_send + sent;
                continue;
            }
#endif
            wev->ready = 0;
            return in;
        }

        if (send >= limit || in == NULL) {
            return in;
        }
    }
}
{% endhighlight %}

### 1.1 sendfile()版本问题
一直到Linux 2.4.21版本为止，sendfile()函数都只支持32bit的offset,在此种情况下如果off_t为64bit时，包含```<sys/sendfile.h>```头文件会导致编译失败。因此在没有sendfile64()函数的情况下，我们采用extern的方式声明sendfile()。
{% highlight string %}
#if (NGX_HAVE_SENDFILE64)
#include <sys/sendfile.h>
#else
extern ssize_t sendfile(int s, int fd, int32_t *offset, size_t size);
#define NGX_SENDFILE_LIMIT  0x80000000
#endif
{% endhighlight %}

在ngx_auto_config.h头文件中我们有如下定义(在auto/os/conf脚本中进行对应的检测)：
<pre>
#ifndef NGX_HAVE_SENDFILE
#define NGX_HAVE_SENDFILE  1
#endif


#ifndef NGX_HAVE_SENDFILE64
#define NGX_HAVE_SENDFILE64  1
#endif
</pre>

### 2.2 TCP_NODELAY/TCP_CORK

**1) TCP_NODELAY**

默认情况下，发送数据采用Nagle算法。这样虽然提高了网络吞吐量，但是实时性却降低了，对一些交互性很强的应用程序来说是不允许的，使用TCP_NODELAY选项可以禁止Nagle算法。

此时，应用程序向内核递交的每个数据包都会立即发送出去。需要注意的是，虽然禁止了Nagle算法，但网络的传输仍然受到TCP确认延迟机制的影响。

**2) TCP_CORK**

所谓的CORK就是塞子的意思，形象的理解就是用CORK将连接塞住，使数据先不发送出去，等到拔去塞子后再发送出去。设置该选项后，内核会尽量把小数据包拼成一个大的数据包(一个MTU)再发送出去，当然若一定时间后（一般为200ms，该值尚待确认），内核仍然没有组合成一个MTU时也必须发送现有的数据。

然而，TCP_CORK的实现可能并不像你想象的那么完美，CORK并不会将连接完全塞住。内核其实并不知道应用层到底什么时候会发送第二批数据用于和第一批数据拼接以达到MTU的大小，因此内核会给出一个时间限制，在该时间内没有拼接成一个大包（努力接近MTU）的话，内核就会无条件发送。也就是说若应用层程序发送小数据包的间隔不够短时，TCP_CORK就没有一点作用，反而会失去数据的实时性（每个小包数据都会延时一定时间再发送）。

<pre>
这里的TCP_NOPUSH与TCP_CORK含义是相同的。TCP_CORK与TCP_NODELAY为互斥关系。
</pre>


### 2.3 数据发送
ngx_linux_sendfile_chain()函数支持发送两种形式的数据：

* 内存发送

* 文件发送

内存发送和文件发送的区别（writev和sendfile):

1) 文件发送的效率相对内存发送效率要高很多，效率主要高在少了内核层到用户态的拷贝，用户态到内核态的拷贝。直接在磁盘将数据从网卡发送出去。

2) 通常的情况下，程序可能会在多个地方产生不同的buffer。writev是读取多个不连续的buffer然后集中写入。大并发服务器的时候这个效率还是很高的。writev和write函数的区别就在于多个非连续buffer的读取后写入，当负载大的时候就可以很好的提现出性能效果了。当然，如果数据足够小(小于1024)且只有唯一的一个buffer，我们直接用send/write就可以了。

3) 对于静态文件的传输，用sendfile可以减少系统调用。注意：文件发送的场景主要是对文件数据不进行改变，如果数据需要作改变还是得用内存发送。


下面我们仔细分析一下ngx_linux_sendfile_chain()函数：
{% highlight string %}
#define NGX_SENDFILE_MAXSIZE  2147483647L


ngx_chain_t *
ngx_linux_sendfile_chain(ngx_connection_t *c, ngx_chain_t *in, off_t limit)
{
    //1: send用于记录到目前为止发送过的字节数，prev_send用于记录截止到上一次为止发送过的字节数
    //   sent用于记录本次成功发送的字节数

    off_t          send, prev_send;   
    size_t         file_size, sent;

    for(;;)
    {
        prev_send = send;

        //2: 创建iovec并且合并相邻的buf(这里只会合并处于内存中的数据，对于文件中的数据并不会合并)
        cl = ngx_output_chain_to_iovec(&header, in, limit - send, c->log);
        send += header.size;
      
        //3: set TCP_CORK if there is a header before a file

        //4: 发送文件中的数据
        if (header.count == 0 && cl && cl->buf->in_file && send < limit)   
        {
             //5: 合并相邻的文件buf
			 file_size = (size_t) ngx_chain_coalesce_file(&cl, limit - send);
             send += file_size;   

             //6: 发送文件
              #if NGX_THREADS
                   if (file->file->thread_handler)
                   {
                       //多线程文件发送
                        rc = ngx_linux_sendfile_thread(c, file, file_size, &sent);
                   }else
              #endif
                   {
                        //sendfile
                   }
        }
        else{
             //7: ngx_writev内存发送
        }
        
        //8: 根据已经发送的数据，更新发送buf(这里计算下一次应该从什么地方开始发送数据）
        in = ngx_chain_update_sent(in, sent);

        //
        if ((size_t) (send - prev_send) != sent) {
           #if NGX_THREADS
                //9: 这里多线程特别要注意，线程池队列中针对一个ngx_connection_t最多只能存在一个发送任务
                //   如果这里不加限制，上面多次调用ngx_linux_senfile_thread就会导致发送混乱
                 if (thread_complete) {
                     send = prev_send + sent;
                     continue;
                 }
           #endif
           
            //10: 这里重置wev->ready为0，然后return退出循环
            wev->ready = 0;
            return in;
        }

        //11: 数据发送完毕或者没有数据可以发送了，则return退出
        if (send >= limit || in == NULL) {
            return in;
        }
    }
}
{% endhighlight %}


## 3. 函数ngx_linux_sendfile()
{% highlight string %}
static ssize_t
ngx_linux_sendfile(ngx_connection_t *c, ngx_buf_t *file, size_t size)
{
#if (NGX_HAVE_SENDFILE64)
    off_t      offset;
#else
    int32_t    offset;
#endif
    ssize_t    n;
    ngx_err_t  err;

#if (NGX_HAVE_SENDFILE64)
    offset = file->file_pos;
#else
    offset = (int32_t) file->file_pos;
#endif

eintr:

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "sendfile: @%O %uz", file->file_pos, size);

    n = sendfile(c->fd, file->file->fd, &offset, size);

    if (n == -1) {
        err = ngx_errno;

        switch (err) {
        case NGX_EAGAIN:
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "sendfile() is not ready");
            return NGX_AGAIN;

        case NGX_EINTR:
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "sendfile() was interrupted");
            goto eintr;

        default:
            c->write->error = 1;
            ngx_connection_error(c, err, "sendfile() failed");
            return NGX_ERROR;
        }
    }

    if (n == 0) {
        /*
         * if sendfile returns zero, then someone has truncated the file,
         * so the offset became beyond the end of the file
         */

        ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                      "sendfile() reported that \"%s\" was truncated at %O",
                      file->file->name.data, file->file_pos);

        return NGX_ERROR;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0, "sendfile: %z of %uz @%O",
                   n, size, file->file_pos);

    return n;
}
{% endhighlight %}
此函数调用sendfile()发送文件中的数据。注意如下几个方面：

**1) offset变量的定义**
{% highlight string %}
#if (NGX_HAVE_SENDFILE64)
    off_t      offset;
#else
    int32_t    offset;
#endif
    ssize_t    n;
    ngx_err_t  err;

#if (NGX_HAVE_SENDFILE64)
    offset = file->file_pos;
#else
    offset = (int32_t) file->file_pos;
#endif
{% endhighlight %} 
这里主要是由于sendfile()函数在32bit环境下被我们定义成了：
<pre>
extern ssize_t sendfile(int s, int fd, int32_t *offset, size_t size);
</pre>


**2) 对中断的处理**

若发送过程中被INT信号中断，返回继续处理。

**3) 对sendfile()返回值0的处理**
{% highlight string %}
if (n == 0) {
    /*
     * if sendfile returns zero, then someone has truncated the file,
     * so the offset became beyond the end of the file
     */

    ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                  "sendfile() reported that \"%s\" was truncated at %O",
                  file->file->name.data, file->file_pos);

    return NGX_ERROR;
}
{% endhighlight %}
这里返回0表示文件产生了截断。


## 4. 多线程数据发送
{% highlight string %}
#if (NGX_THREADS)

typedef struct {
    ngx_buf_t     *file;
    ngx_socket_t   socket;
    size_t         size;

    size_t         sent;
    ngx_err_t      err;
} ngx_linux_sendfile_ctx_t;


static ngx_int_t
ngx_linux_sendfile_thread(ngx_connection_t *c, ngx_buf_t *file, size_t size,
    size_t *sent)
{
    ngx_event_t               *wev;
    ngx_thread_task_t         *task;
    ngx_linux_sendfile_ctx_t  *ctx;

    ngx_log_debug3(NGX_LOG_DEBUG_CORE, c->log, 0,
                   "linux sendfile thread: %d, %uz, %O",
                   file->file->fd, size, file->file_pos);

    task = c->sendfile_task;

    if (task == NULL) {
        task = ngx_thread_task_alloc(c->pool, sizeof(ngx_linux_sendfile_ctx_t));
        if (task == NULL) {
            return NGX_ERROR;
        }

        task->handler = ngx_linux_sendfile_thread_handler;

        c->sendfile_task = task;
    }

    ctx = task->ctx;
    wev = c->write;

    if (task->event.complete) {
        task->event.complete = 0;

        if (ctx->err == NGX_EAGAIN) {
            *sent = 0;

            if (wev->complete) {
                return NGX_DONE;
            }

            return NGX_AGAIN;
        }

        if (ctx->err) {
            wev->error = 1;
            ngx_connection_error(c, ctx->err, "sendfile() failed");
            return NGX_ERROR;
        }

        if (ctx->sent == 0) {
            /*
             * if sendfile returns zero, then someone has truncated the file,
             * so the offset became beyond the end of the file
             */

            ngx_log_error(NGX_LOG_ALERT, c->log, 0,
                          "sendfile() reported that \"%s\" was truncated at %O",
                          file->file->name.data, file->file_pos);

            return NGX_ERROR;
        }

        *sent = ctx->sent;

        if (ctx->sent == ctx->size || wev->complete) {
            return NGX_DONE;
        }

        return NGX_AGAIN;
    }

    if (task->event.active && ctx->file == file) {
        /*
         * tolerate duplicate calls; they can happen due to subrequests
         * or multiple calls of the next body filter from a filter
         */

        *sent = 0;

        return NGX_OK;
    }

    ctx->file = file;
    ctx->socket = c->fd;
    ctx->size = size;

    wev->complete = 0;

    if (file->file->thread_handler(task, file->file) != NGX_OK) {
        return NGX_ERROR;
    }

    *sent = 0;

    return NGX_OK;
}


static void
ngx_linux_sendfile_thread_handler(void *data, ngx_log_t *log)
{
    ngx_linux_sendfile_ctx_t *ctx = data;

    off_t       offset;
    ssize_t     n;
    ngx_buf_t  *file;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, log, 0, "linux sendfile thread handler");

    file = ctx->file;
    offset = file->file_pos;

again:

    n = sendfile(ctx->socket, file->file->fd, &offset, ctx->size);

    if (n == -1) {
        ctx->err = ngx_errno;

    } else {
        ctx->sent = n;
        ctx->err = 0;
    }

#if 0
    ngx_time_update();
#endif

    ngx_log_debug4(NGX_LOG_DEBUG_EVENT, log, 0,
                   "sendfile: %z (err: %d) of %uz @%O",
                   n, ctx->err, ctx->size, file->file_pos);

    if (ctx->err == NGX_EINTR) {
        goto again;
    }
}

#endif /* NGX_THREADS */
{% endhighlight %}

这里两个函数都较为简单。```ngx_linux_sendfile_thread```主要是为connection分配一个sendfile_task,然后调用thread_handler将该任务添加到队列中。


<br />
<br />
**[参看]:**

1. [TCP_NODELAY与TCP_CORK](http://blog.csdn.net/gexiao/article/details/50722431)

2. [关于TCP_NODELAY和TCP_CORK选项](https://www.2cto.com/net/201308/238322.html)


3. [ngx_linux_sendfile_chain](http://blog.csdn.net/wu5215080/article/details/53021965)
<br />
<br />
<br />

