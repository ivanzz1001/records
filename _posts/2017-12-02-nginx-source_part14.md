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

### 2.1 限定发送发文件大小
<pre>
#define NGX_SENDFILE_MAXSIZE  2147483647L
</pre>
这里我们限定发送文件的最大大小为2GB-1字节。

### 2.2 ngx_linux_sendfile_chain工作流程




<br />
<br />
<br />

