---
layout: post
title: os/unix/ngx_udp_send.c源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节主要讲述一下nginx发送udp数据包。


<!-- more -->

## 1. os/unix/ngx_udp_send.c源代码
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ssize_t
ngx_udp_unix_send(ngx_connection_t *c, u_char *buf, size_t size)
{
    ssize_t       n;
    ngx_err_t     err;
    ngx_event_t  *wev;

    wev = c->write;

    for ( ;; ) {
        n = sendto(c->fd, buf, size, 0, c->sockaddr, c->socklen);

        ngx_log_debug4(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "sendto: fd:%d %z of %uz to \"%V\"",
                       c->fd, n, size, &c->addr_text);

        if (n >= 0) {
            if ((size_t) n != size) {
                wev->error = 1;
                (void) ngx_connection_error(c, 0, "sendto() incomplete");
                return NGX_ERROR;
            }

            c->sent += n;

            return n;
        }

        err = ngx_socket_errno;

        if (err == NGX_EAGAIN) {
            wev->ready = 0;
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, NGX_EAGAIN,
                           "sendto() not ready");
            return NGX_AGAIN;
        }

        if (err != NGX_EINTR) {
            wev->error = 1;
            (void) ngx_connection_error(c, err, "sendto() failed");
            return NGX_ERROR;
        }
    }
}
{% endhighlight %}

下面我们简单分析一下发送流程：
{% highlight string %}
ssize_t
ngx_udp_unix_send(ngx_connection_t *c, u_char *buf, size_t size)
{
      for(;;)
      {
           n = sendto(c->fd, buf, size, 0, c->sockaddr, c->socklen);
          
           if(n >= 0)
           {
                if(n != size)
                {
                     //发送错误，将wev写事件的error位置为1，返回
                }
                
                //返回
           }

          //若errno为NGX_EAGAIN，则表示当前socket句柄暂时为准备好发送数据，一般出现此情况是当前发送缓冲区已满

          //在errno为NGX_EINTR时，表示受信号中断影响，此时继续发送数据即可。而若errno为其他错误时，设置wev->error为1，返回错误
      }
}
{% endhighlight %}




<br />
<br />
<br />

