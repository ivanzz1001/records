---
layout: post
title: os/unix/ngx_send.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们讲述一下ngx_send.c文件,其主要用于向TCP socket发送一段连续的内存数据。
<!-- more -->


## 1. os/unix/ngx_send.c源文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ssize_t
ngx_unix_send(ngx_connection_t *c, u_char *buf, size_t size)
{
    ssize_t       n;
    ngx_err_t     err;
    ngx_event_t  *wev;

    wev = c->write;

#if (NGX_HAVE_KQUEUE)

    if ((ngx_event_flags & NGX_USE_KQUEUE_EVENT) && wev->pending_eof) {
        (void) ngx_connection_error(c, wev->kq_errno,
                               "kevent() reported about an closed connection");
        wev->error = 1;
        return NGX_ERROR;
    }

#endif

    for ( ;; ) {
        n = send(c->fd, buf, size, 0);

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "send: fd:%d %z of %uz", c->fd, n, size);

        if (n > 0) {
            if (n < (ssize_t) size) {
                wev->ready = 0;
            }

            c->sent += n;

            return n;
        }

        err = ngx_socket_errno;

        if (n == 0) {
            ngx_log_error(NGX_LOG_ALERT, c->log, err, "send() returned zero");
            wev->ready = 0;
            return n;
        }

        if (err == NGX_EAGAIN || err == NGX_EINTR) {
            wev->ready = 0;

            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                           "send() not ready");

            if (err == NGX_EAGAIN) {
                return NGX_AGAIN;
            }

        } else {
            wev->error = 1;
            (void) ngx_connection_error(c, err, "send() failed");
            return NGX_ERROR;
        }
    }
}
{% endhighlight %}

下面对ngx_unix_send()函数做一个简单的解释：

**1) 对kqueue情形下pending_eof的处理**
{% highlight string %}
#if (NGX_HAVE_KQUEUE)

    if ((ngx_event_flags & NGX_USE_KQUEUE_EVENT) && wev->pending_eof) {
        (void) ngx_connection_error(c, wev->kq_errno,
                               "kevent() reported about an closed connection");
        wev->error = 1;
        return NGX_ERROR;
    }

#endif
{% endhighlight %}

我们当前不支持```NGX_HAVE_KQUEUE```。在kqueue模型下，wev->pending_eof表示当前socket 连接已经关闭，但是当前还未被处理。


**2) 函数send()发送数据**
<pre>
n = send(c->fd, buf, size, 0);
</pre>
根据返回值n做不同的处理：

* **返回值>0**: 返回发送的字节数，如果```n < size```，将wev->ready置为0，表示socket未准备好（如发送缓存区已满)
{% highlight string %}
if (n > 0) {
    if (n < (ssize_t) size) {
        wev->ready = 0;
    }

    c->sent += n;

    return n;
}
{% endhighlight %}

* **返回值为0**: 将wev->ready置为0，返回
{% highlight string %}
if (n == 0) {
        ngx_log_error(NGX_LOG_ALERT, c->log, err, "send() returned zero");
        wev->ready = 0;
        return n;
    }
{% endhighlight %}

* **返回值<0**
{% highlight string %}
if (err == NGX_EAGAIN || err == NGX_EINTR) {
    wev->ready = 0;

    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, err,
                   "send() not ready");

    if (err == NGX_EAGAIN) {
        return NGX_AGAIN;
    }

} else {
    wev->error = 1;
    (void) ngx_connection_error(c, err, "send() failed");
    return NGX_ERROR;
}
{% endhighlight %}
此种情况下，有两种特例： ```NGX_EAGAIN```以及```NGX_EINTR```。

```NGX_EINTR```这种情况是受到信号中断的影响，一般重新发送即可。对于```NGX_EAGAIN```一般表示当前尚未准备好数据的发送，可能需要再等待一段时间以使socket就绪(此种情况一般并不表示真正出错)。

<br />
<br />
<br />

