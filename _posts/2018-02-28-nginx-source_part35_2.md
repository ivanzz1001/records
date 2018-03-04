---
layout: post
title: core/ngx_connection.c源文件分析 
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们介绍一下nginx中对socket connection的相关操作.主要包括两个方面的内容：

* 监听socket对象(ngx_listening_t)相关操作

* 连接socket对象(ngx_connection_t)相关操作


<!-- more -->

<br />
<br />

## 1. 函数ngx_create_listening()
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


ngx_os_io_t  ngx_io;

//从reusable_connections_queue队列中释放一些长连接
static void ngx_drain_connections(void);


ngx_listening_t *
ngx_create_listening(ngx_conf_t *cf, void *sockaddr, socklen_t socklen)
{
    size_t            len;
    ngx_listening_t  *ls;
    struct sockaddr  *sa;
    u_char            text[NGX_SOCKADDR_STRLEN];

    ls = ngx_array_push(&cf->cycle->listening);
    if (ls == NULL) {
        return NULL;
    }

    ngx_memzero(ls, sizeof(ngx_listening_t));

    sa = ngx_palloc(cf->pool, socklen);
    if (sa == NULL) {
        return NULL;
    }

    ngx_memcpy(sa, sockaddr, socklen);

    ls->sockaddr = sa;
    ls->socklen = socklen;

    len = ngx_sock_ntop(sa, socklen, text, NGX_SOCKADDR_STRLEN, 1);
    ls->addr_text.len = len;

    switch (ls->sockaddr->sa_family) {
#if (NGX_HAVE_INET6)
    case AF_INET6:
        ls->addr_text_max_len = NGX_INET6_ADDRSTRLEN;
        break;
#endif
#if (NGX_HAVE_UNIX_DOMAIN)
    case AF_UNIX:
        ls->addr_text_max_len = NGX_UNIX_ADDRSTRLEN;
        len++;
        break;
#endif
    case AF_INET:
        ls->addr_text_max_len = NGX_INET_ADDRSTRLEN;
        break;
    default:
        ls->addr_text_max_len = NGX_SOCKADDR_STRLEN;
        break;
    }

    ls->addr_text.data = ngx_pnalloc(cf->pool, len);
    if (ls->addr_text.data == NULL) {
        return NULL;
    }

    ngx_memcpy(ls->addr_text.data, text, len);

    ls->fd = (ngx_socket_t) -1;
    ls->type = SOCK_STREAM;

    ls->backlog = NGX_LISTEN_BACKLOG;
    ls->rcvbuf = -1;
    ls->sndbuf = -1;

#if (NGX_HAVE_SETFIB)
    ls->setfib = -1;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    ls->fastopen = -1;
#endif

    return ls;
}
{% endhighlight %}

这里主要是根据```sockaddr```地址构建一个```ngx_listening_t```结构体对象，存放于cycle->listening数组中。这里并未真正创建socket：
{% highlight string %}
ls->fd = (ngx_socket_t) -1;
ls->type = SOCK_STREAM;

ls->backlog = NGX_LISTEN_BACKLOG;
ls->rcvbuf = -1;
ls->sndbuf = -1;
{% endhighlight %}
注意这里将二进制形式表示的socket地址```sockaddr```转换为字符串形式时长度的确定。


## 2. 函数ngx_clone_listening()
{% highlight string %}
ngx_int_t
ngx_clone_listening(ngx_conf_t *cf, ngx_listening_t *ls)
{
#if (NGX_HAVE_REUSEPORT)

    ngx_int_t         n;
    ngx_core_conf_t  *ccf;
    ngx_listening_t   ols;

    if (!ls->reuseport) {
        return NGX_OK;
    }

    ols = *ls;

    ccf = (ngx_core_conf_t *) ngx_get_conf(cf->cycle->conf_ctx,
                                           ngx_core_module);

    for (n = 1; n < ccf->worker_processes; n++) {

        /* create a socket for each worker process */

        ls = ngx_array_push(&cf->cycle->listening);
        if (ls == NULL) {
            return NGX_ERROR;
        }

        *ls = ols;
        ls->worker = n;
    }

#endif

    return NGX_OK;
}
{% endhighlight %}
我们在objs/ngx_auto_config.h头文件中有如下定义：
<pre>
#ifndef NGX_HAVE_REUSEPORT
#define NGX_HAVE_REUSEPORT  1
#endif
</pre>
一般是在worker0创建完成一个监听socket后，通过此函数再复制到其他worker进程。

## 3. 函数ngx_set_inherited_sockets()
{% highlight string %}
ngx_int_t
ngx_set_inherited_sockets(ngx_cycle_t *cycle)
{
    size_t                     len;
    ngx_uint_t                 i;
    ngx_listening_t           *ls;
    socklen_t                  olen;
#if (NGX_HAVE_DEFERRED_ACCEPT || NGX_HAVE_TCP_FASTOPEN)
    ngx_err_t                  err;
#endif
#if (NGX_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
    struct accept_filter_arg   af;
#endif
#if (NGX_HAVE_DEFERRED_ACCEPT && defined TCP_DEFER_ACCEPT)
    int                        timeout;
#endif
#if (NGX_HAVE_REUSEPORT)
    int                        reuseport;
#endif

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {

        ls[i].sockaddr = ngx_palloc(cycle->pool, NGX_SOCKADDRLEN);
        if (ls[i].sockaddr == NULL) {
            return NGX_ERROR;
        }

        ls[i].socklen = NGX_SOCKADDRLEN;
        if (getsockname(ls[i].fd, ls[i].sockaddr, &ls[i].socklen) == -1) {
            ngx_log_error(NGX_LOG_CRIT, cycle->log, ngx_socket_errno,
                          "getsockname() of the inherited "
                          "socket #%d failed", ls[i].fd);
            ls[i].ignore = 1;
            continue;
        }

        switch (ls[i].sockaddr->sa_family) {

#if (NGX_HAVE_INET6)
        case AF_INET6:
            ls[i].addr_text_max_len = NGX_INET6_ADDRSTRLEN;
            len = NGX_INET6_ADDRSTRLEN + sizeof("[]:65535") - 1;
            break;
#endif

#if (NGX_HAVE_UNIX_DOMAIN)
        case AF_UNIX:
            ls[i].addr_text_max_len = NGX_UNIX_ADDRSTRLEN;
            len = NGX_UNIX_ADDRSTRLEN;
            break;
#endif

        case AF_INET:
            ls[i].addr_text_max_len = NGX_INET_ADDRSTRLEN;
            len = NGX_INET_ADDRSTRLEN + sizeof(":65535") - 1;
            break;

        default:
            ngx_log_error(NGX_LOG_CRIT, cycle->log, ngx_socket_errno,
                          "the inherited socket #%d has "
                          "an unsupported protocol family", ls[i].fd);
            ls[i].ignore = 1;
            continue;
        }

        ls[i].addr_text.data = ngx_pnalloc(cycle->pool, len);
        if (ls[i].addr_text.data == NULL) {
            return NGX_ERROR;
        }

        len = ngx_sock_ntop(ls[i].sockaddr, ls[i].socklen,
                            ls[i].addr_text.data, len, 1);
        if (len == 0) {
            return NGX_ERROR;
        }

        ls[i].addr_text.len = len;

        ls[i].backlog = NGX_LISTEN_BACKLOG;

        olen = sizeof(int);

        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_TYPE, (void *) &ls[i].type,
                       &olen)
            == -1)
        {
            ngx_log_error(NGX_LOG_CRIT, cycle->log, ngx_socket_errno,
                          "getsockopt(SO_TYPE) %V failed", &ls[i].addr_text);
            ls[i].ignore = 1;
            continue;
        }

        olen = sizeof(int);

        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF, (void *) &ls[i].rcvbuf,
                       &olen)
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "getsockopt(SO_RCVBUF) %V failed, ignored",
                          &ls[i].addr_text);

            ls[i].rcvbuf = -1;
        }

        olen = sizeof(int);

        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF, (void *) &ls[i].sndbuf,
                       &olen)
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "getsockopt(SO_SNDBUF) %V failed, ignored",
                          &ls[i].addr_text);

            ls[i].sndbuf = -1;
        }

#if 0
        /* SO_SETFIB is currently a set only option */

#if (NGX_HAVE_SETFIB)

        olen = sizeof(int);

        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_SETFIB,
                       (void *) &ls[i].setfib, &olen)
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "getsockopt(SO_SETFIB) %V failed, ignored",
                          &ls[i].addr_text);

            ls[i].setfib = -1;
        }

#endif
#endif

#if (NGX_HAVE_REUSEPORT)

        reuseport = 0;
        olen = sizeof(int);

        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_REUSEPORT,
                       (void *) &reuseport, &olen)
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "getsockopt(SO_REUSEPORT) %V failed, ignored",
                          &ls[i].addr_text);

        } else {
            ls[i].reuseport = reuseport ? 1 : 0;
        }

#endif

        if (ls[i].type != SOCK_STREAM) {
            continue;
        }

#if (NGX_HAVE_TCP_FASTOPEN)

        olen = sizeof(int);

        if (getsockopt(ls[i].fd, IPPROTO_TCP, TCP_FASTOPEN,
                       (void *) &ls[i].fastopen, &olen)
            == -1)
        {
            err = ngx_socket_errno;

            if (err != NGX_EOPNOTSUPP && err != NGX_ENOPROTOOPT) {
                ngx_log_error(NGX_LOG_NOTICE, cycle->log, err,
                              "getsockopt(TCP_FASTOPEN) %V failed, ignored",
                              &ls[i].addr_text);
            }

            ls[i].fastopen = -1;
        }

#endif

#if (NGX_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)

        ngx_memzero(&af, sizeof(struct accept_filter_arg));
        olen = sizeof(struct accept_filter_arg);

        if (getsockopt(ls[i].fd, SOL_SOCKET, SO_ACCEPTFILTER, &af, &olen)
            == -1)
        {
            err = ngx_socket_errno;

            if (err == NGX_EINVAL) {
                continue;
            }

            ngx_log_error(NGX_LOG_NOTICE, cycle->log, err,
                          "getsockopt(SO_ACCEPTFILTER) for %V failed, ignored",
                          &ls[i].addr_text);
            continue;
        }

        if (olen < sizeof(struct accept_filter_arg) || af.af_name[0] == '\0') {
            continue;
        }

        ls[i].accept_filter = ngx_palloc(cycle->pool, 16);
        if (ls[i].accept_filter == NULL) {
            return NGX_ERROR;
        }

        (void) ngx_cpystrn((u_char *) ls[i].accept_filter,
                           (u_char *) af.af_name, 16);
#endif

#if (NGX_HAVE_DEFERRED_ACCEPT && defined TCP_DEFER_ACCEPT)

        timeout = 0;
        olen = sizeof(int);

        if (getsockopt(ls[i].fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, &olen)
            == -1)
        {
            err = ngx_socket_errno;

            if (err == NGX_EOPNOTSUPP) {
                continue;
            }

            ngx_log_error(NGX_LOG_NOTICE, cycle->log, err,
                          "getsockopt(TCP_DEFER_ACCEPT) for %V failed, ignored",
                          &ls[i].addr_text);
            continue;
        }

        if (olen < sizeof(int) || timeout == 0) {
            continue;
        }

        ls[i].deferred_accept = 1;
#endif
    }

    return NGX_OK;
}
{% endhighlight %}




<br />
<br />
<br />

