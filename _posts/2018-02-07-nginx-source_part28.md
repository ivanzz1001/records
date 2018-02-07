---
layout: post
title: os/unix/ngx_udp_recv.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节主要讲述一下nginx接收udp数据包。


<!-- more -->

## 1. os/unix/ngx_udp_recv.c源代码
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ssize_t
ngx_udp_unix_recv(ngx_connection_t *c, u_char *buf, size_t size)
{
    ssize_t       n;
    ngx_err_t     err;
    ngx_event_t  *rev;

    rev = c->read;

    do {
        n = recv(c->fd, buf, size, 0);

        ngx_log_debug3(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "recv: fd:%d %z of %uz", c->fd, n, size);

        if (n >= 0) {

#if (NGX_HAVE_KQUEUE)

            if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
                rev->available -= n;

                /*
                 * rev->available may be negative here because some additional
                 * bytes may be received between kevent() and recv()
                 */

                if (rev->available <= 0) {
                    rev->ready = 0;
                    rev->available = 0;
                }
            }

#endif

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
这里对于接收udp数据包的处理比较简单，主要是注意一下几点：

* 接收函数recv()返回值>=0，表示接收成功，直接返回即可。

* 接收函数recv()返回值<0，此时如果是受中断信号导致的，则继续接收数据；否则设置设置好接收事件的相应状态位，然后返回
{% highlight string %}
// 接收错误，将ready置为0，表示没有准备好接收数据

rev->ready = 0;
{% endhighlight %}


<br />
<br />
<br />

