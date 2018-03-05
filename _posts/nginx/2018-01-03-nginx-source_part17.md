---
layout: post
title: os/unix/ngx_readv_chain.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们讲述一下ngx_readv_chain.c文件，其主要是用于分散读取数据到ngx_chain_t中。
<!-- more -->


## 1. os/unix/ngx_readv_chain.c源文件
{% highlight string %}


/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ssize_t
ngx_readv_chain(ngx_connection_t *c, ngx_chain_t *chain, off_t limit)
{
    u_char        *prev;
    ssize_t        n, size;
    ngx_err_t      err;
    ngx_array_t    vec;
    ngx_event_t   *rev;
    struct iovec  *iov, iovs[NGX_IOVS_PREALLOCATE];

    rev = c->read;

#if (NGX_HAVE_KQUEUE)

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "readv: eof:%d, avail:%d, err:%d",
                       rev->pending_eof, rev->available, rev->kq_errno);

        if (rev->available == 0) {
            if (rev->pending_eof) {
                rev->ready = 0;
                rev->eof = 1;

                ngx_log_error(NGX_LOG_INFO, c->log, rev->kq_errno,
                              "kevent() reported about an closed connection");

                if (rev->kq_errno) {
                    rev->error = 1;
                    ngx_set_socket_errno(rev->kq_errno);
                    return NGX_ERROR;
                }

                return 0;

            } else {
                return NGX_AGAIN;
            }
        }
    }

#endif

    prev = NULL;
    iov = NULL;
    size = 0;

    vec.elts = iovs;
    vec.nelts = 0;
    vec.size = sizeof(struct iovec);
    vec.nalloc = NGX_IOVS_PREALLOCATE;
    vec.pool = c->pool;

    /* coalesce the neighbouring bufs */

    while (chain) {
        n = chain->buf->end - chain->buf->last;

        if (limit) {
            if (size >= limit) {
                break;
            }

            if (size + n > limit) {
                n = (ssize_t) (limit - size);
            }
        }

        if (prev == chain->buf->last) {
            iov->iov_len += n;

        } else {
            if (vec.nelts >= IOV_MAX) {
                break;
            }

            iov = ngx_array_push(&vec);
            if (iov == NULL) {
                return NGX_ERROR;
            }

            iov->iov_base = (void *) chain->buf->last;
            iov->iov_len = n;
        }

        size += n;
        prev = chain->buf->end;
        chain = chain->next;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                   "readv: %ui, last:%uz", vec.nelts, iov->iov_len);

    do {
        n = readv(c->fd, (struct iovec *) vec.elts, vec.nelts);

        if (n == 0) {
            rev->ready = 0;
            rev->eof = 1;

#if (NGX_HAVE_KQUEUE)

            /*
             * on FreeBSD readv() may return 0 on closed socket
             * even if kqueue reported about available data
             */

            if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
                rev->available = 0;
            }

#endif

            return 0;
        }

        if (n > 0) {

#if (NGX_HAVE_KQUEUE)

            if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
                rev->available -= n;

                /*
                 * rev->available may be negative here because some additional
                 * bytes may be received between kevent() and readv()
                 */

                if (rev->available <= 0) {
                    if (!rev->pending_eof) {
                        rev->ready = 0;
                    }

                    rev->available = 0;
                }

                return n;
            }

#endif

            if (n < size && !(ngx_event_flags & NGX_USE_GREEDY_EVENT)) {
                rev->ready = 0;
            }

            return n;
        }

        err = ngx_socket_errno;

        if (err == NGX_EAGAIN || err == NGX_EINTR) {
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "readv() not ready");
            n = NGX_AGAIN;

        } else {
            n = ngx_connection_error(c, err, "readv() failed");
            break;
        }

    } while (err == NGX_EINTR);

    rev->ready = 0;

    if (n == NGX_ERROR) {
        c->read->error = 1;
    }

    return n;
}
{% endhighlight %}

下面我们对ngx_readv_chain()函数做一个简单的解释：

**1) 对kqueue情形下rev->available的处理**
{% highlight string %}
#if (NGX_HAVE_KQUEUE)

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "readv: eof:%d, avail:%d, err:%d",
                       rev->pending_eof, rev->available, rev->kq_errno);

        if (rev->available == 0) {
            if (rev->pending_eof) {
                rev->ready = 0;
                rev->eof = 1;

                ngx_log_error(NGX_LOG_INFO, c->log, rev->kq_errno,
                              "kevent() reported about an closed connection");

                if (rev->kq_errno) {
                    rev->error = 1;
                    ngx_set_socket_errno(rev->kq_errno);
                    return NGX_ERROR;
                }

                return 0;

            } else {
                return NGX_AGAIN;
            }
        }
    }

#endif
{% endhighlight %}
当前我们采用的是epoll模型，因此这里并不会执行。

**2) 合并ngx_chain_t中相邻的buf**
{% highlight string %}
ssize_t
ngx_readv_chain(ngx_connection_t *c, ngx_chain_t *chain, off_t limit)
{
    ...
    
    prev = NULL;
    iov = NULL;
    size = 0;

    vec.elts = iovs;
    vec.nelts = 0;
    vec.size = sizeof(struct iovec);
    vec.nalloc = NGX_IOVS_PREALLOCATE;
    vec.pool = c->pool;

    /* coalesce the neighbouring bufs */

    while (chain) {
        n = chain->buf->end - chain->buf->last;

        if (limit) {
            if (size >= limit) {
                break;
            }

            if (size + n > limit) {
                n = (ssize_t) (limit - size);
            }
        }

        if (prev == chain->buf->last) {
            iov->iov_len += n;

        } else {
            if (vec.nelts >= IOV_MAX) {
                break;
            }

            iov = ngx_array_push(&vec);
            if (iov == NULL) {
                return NGX_ERROR;
            }

            iov->iov_base = (void *) chain->buf->last;
            iov->iov_len = n;
        }

        size += n;
        prev = chain->buf->end;
        chain = chain->next;
    }

    ...
}
{% endhighlight %}
这里我们首先再次给出```ngx_chain_t```的结构：

![ngx-chain-t](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_chain_t.jpg)

当前空闲的空间为ngx_buf_s->last到ngx_buf_s->end之间的那一部分空间。

**3) 读取fd中的数据**

这里我们看到调用readv()分散读取数据：
<pre>
n = readv(c->fd, (struct iovec *) vec.elts, vec.nelts);
</pre>
根据返回值n做不同的处理：

* **返回值为0**
{% highlight string %}
if (n == 0) {
    rev->ready = 0;
    rev->eof = 1;

#if (NGX_HAVE_KQUEUE)

    /*
     * on FreeBSD readv() may return 0 on closed socket
     * even if kqueue reported about available data
     */

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
        rev->available = 0;
    }

#endif

    return 0;
}
{% endhighlight %}
此中情况一般表示为读到了文件的结尾。对于```NGX_HAVE_KQUEUE```这种情况，即使kqueue报告有可用数据，readv()在已关闭的socket上也会返回0.

* **返回值>0**
{% highlight string %}
if (n > 0) {

#if (NGX_HAVE_KQUEUE)

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
        rev->available -= n;

        /*
         * rev->available may be negative here because some additional
         * bytes may be received between kevent() and readv()
         */

        if (rev->available <= 0) {
            if (!rev->pending_eof) {
                rev->ready = 0;
            }

            rev->available = 0;
        }

        return n;
    }

#endif

    if (n < size && !(ngx_event_flags & NGX_USE_GREEDY_EVENT)) {
        rev->ready = 0;
    }

    return n;
}
{% endhighlight %}
此种情况成功读取到了数据。对于```NGX_HAVE_KQUEUE```这种情况下，因为在kevent()报告拥有数据可读到真正读取数据这段时间内，可能会有新的数据到来，因此实际读取的数据可能会比报告的更多，导致rev->available小于0的情况出现。

对于epoll模型用到```NGX_USE_GREEDY_EVENT```标志，则表示需要一直进行读取数据，直到遇到EAGAIN错误为止。

* **返回值<0**
{% highlight string %}
err = ngx_socket_errno;

if (err == NGX_EAGAIN || err == NGX_EINTR) {
    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                   "readv() not ready");
    n = NGX_AGAIN;

} else {
    n = ngx_connection_error(c, err, "readv() failed");
    break;
}
{% endhighlight %}
此种情况下，有两种特例： ```NGX_EAGAIN```以及```NGX_EINTR```。

```NGX_EINTR```这种情况是受到信号中断的影响，一般重新读取即可。对于```NGX_EAGAIN```一般表示当前并没有数据，此时不应该再进行继续读取数据操作了（但是此种情况并不真正表示数据读取出错了）。






<br />
<br />
<br />

