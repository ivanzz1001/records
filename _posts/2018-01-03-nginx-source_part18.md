---
layout: post
title: os/unix/ngx_recv.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们讲述一下ngx_recv.c文件。本文件与ngx_readv_chain.c类似，只不过是这里是将数据读取到连续的地址空间，而不是分散读。
<!-- more -->


## 1. os/unix/ngx_recv.c源文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ssize_t
ngx_unix_recv(ngx_connection_t *c, u_char *buf, size_t size)
{
    ssize_t       n;
    ngx_err_t     err;
    ngx_event_t  *rev;

    rev = c->read;

#if (NGX_HAVE_KQUEUE)

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "recv: eof:%d, avail:%d, err:%d",
                       rev->pending_eof, rev->available, rev->kq_errno);

        if (rev->available == 0) {
            if (rev->pending_eof) {
                rev->ready = 0;
                rev->eof = 1;

                if (rev->kq_errno) {
                    rev->error = 1;
                    ngx_set_socket_errno(rev->kq_errno);

                    return ngx_connection_error(c, rev->kq_errno,
                               "kevent() reported about an closed connection");
                }

                return 0;

            } else {
                rev->ready = 0;
                return NGX_AGAIN;
            }
        }
    }

#endif

    do {
        n = recv(c->fd, buf, size, 0);

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "recv: fd:%d %z of %uz", c->fd, n, size);

        if (n == 0) {
            rev->ready = 0;
            rev->eof = 1;

#if (NGX_HAVE_KQUEUE)

            /*
             * on FreeBSD recv() may return 0 on closed socket
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
                 * bytes may be received between kevent() and recv()
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

            if ((size_t) n < size
                && !(ngx_event_flags & NGX_USE_GREEDY_EVENT))
            {
                rev->ready = 0;
            }

            return n;
        }

        err = ngx_socket_errno;

        if (err == NGX_EAGAIN || err == NGX_EINTR) {
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "recv() not ready");
            n = NGX_AGAIN;

        } else {
            n = ngx_connection_error(c, err, "recv() failed");
            break;
        }

    } while (err == NGX_EINTR);

    rev->ready = 0;

    if (n == NGX_ERROR) {
        rev->error = 1;
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
                       "recv: eof:%d, avail:%d, err:%d",
                       rev->pending_eof, rev->available, rev->kq_errno);

        if (rev->available == 0) {
            if (rev->pending_eof) {
                rev->ready = 0;
                rev->eof = 1;

                if (rev->kq_errno) {
                    rev->error = 1;
                    ngx_set_socket_errno(rev->kq_errno);

                    return ngx_connection_error(c, rev->kq_errno,
                               "kevent() reported about an closed connection");
                }

                return 0;

            } else {
                rev->ready = 0;
                return NGX_AGAIN;
            }
        }
    }

#endif
{% endhighlight %}
当前我们采用的是epoll模型，因此这里并不会执行。


**2) 读取fd中的数据**

这里我们看到调用readv()分散读取数据：
<pre>
n = recv(c->fd, buf, size, 0);
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

