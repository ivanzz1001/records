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
本函数主要用于平滑升级时，设置继承而来的socket。在objs/ngx_auto_config.h头文件中我们有如下定义：
<pre>
#ifndef NGX_HAVE_DEFERRED_ACCEPT
#define NGX_HAVE_DEFERRED_ACCEPT  1
#endif

#ifndef NGX_HAVE_TCP_FASTOPEN
#define NGX_HAVE_TCP_FASTOPEN  1
#endif

#ifndef NGX_HAVE_REUSEPORT
#define NGX_HAVE_REUSEPORT  1
#endif
</pre>
并且支持```SO_ACCEPTFILTER```与```TCP_DEFER_ACCEPT```。

本函数是受ngx_add_inherited_sockets()函数调用的，并且在该函数中通过继承已经设置了```ngx_listening_t```对象的fd字段。接下来我们分析一下本函数的实现：
{% highlight string %}
ngx_int_t
ngx_set_inherited_sockets(ngx_cycle_t *cycle)
{
	 for (i = 0; i < cycle->listening.nelts; i++) 
     {
         //1: 通过fd获得绑定的地址信息： 二进制表示形式以及字符串表示形式
         getsockname(fd,&sockaddr,&socklen);

         //2: 获得相应socket的设置选项
         getsockopt(ls[i].fd, SOL_SOCKET, SO_TYPE, (void *) &ls[i].type,&olen); //获取失败，则忽略当前继承过来的这个listening socket

         getsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF, (void *) &ls[i].rcvbuf,&olen);

         getsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF, (void *) &ls[i].sndbuf, &olen);

         #if (NGX_HAVE_REUSEPORT)
             //是否支持端口复用
		     getsockopt(ls[i].fd, SOL_SOCKET, SO_REUSEPORT,(void *) &reuseport, &olen);
         #endif

        //3: 针对TCP的选项设置
        if (ls[i].type != SOCK_STREAM) {
            continue;                 //非TCP类型，则continue，不在进行如下的检测
        }

        #if (NGX_HAVE_TCP_FASTOPEN)
           getsockopt(ls[i].fd, IPPROTO_TCP, TCP_FASTOPEN,(void *) &ls[i].fastopen, &olen);
        #endif
      
        #if (NGX_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
            getsockopt(ls[i].fd, SOL_SOCKET, SO_ACCEPTFILTER, &af, &olen);
        #endif

        #if (NGX_HAVE_DEFERRED_ACCEPT && defined TCP_DEFER_ACCEPT)
            getsockopt(ls[i].fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, &timeout, &olen);
        #endif
     }
}
{% endhighlight %}

## 4. 函数ngx_open_listening_sockets()
{% highlight string %}
ngx_int_t
ngx_open_listening_sockets(ngx_cycle_t *cycle)
{
    int               reuseaddr;
    ngx_uint_t        i, tries, failed;
    ngx_err_t         err;
    ngx_log_t        *log;
    ngx_socket_t      s;
    ngx_listening_t  *ls;

    reuseaddr = 1;
#if (NGX_SUPPRESS_WARN)
    failed = 0;
#endif

    log = cycle->log;

    /* TODO: configurable try number */

    for (tries = 5; tries; tries--) {
        failed = 0;

        /* for each listening socket */

        ls = cycle->listening.elts;
        for (i = 0; i < cycle->listening.nelts; i++) {

            if (ls[i].ignore) {
                continue;
            }

#if (NGX_HAVE_REUSEPORT)

            if (ls[i].add_reuseport) {

                /*
                 * to allow transition from a socket without SO_REUSEPORT
                 * to multiple sockets with SO_REUSEPORT, we have to set
                 * SO_REUSEPORT on the old socket before opening new ones
                 */

                int  reuseport = 1;

                if (setsockopt(ls[i].fd, SOL_SOCKET, SO_REUSEPORT,
                               (const void *) &reuseport, sizeof(int))
                    == -1)
                {
                    ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                                  "setsockopt(SO_REUSEPORT) %V failed, ignored",
                                  &ls[i].addr_text);
                }

                ls[i].add_reuseport = 0;
            }
#endif

            if (ls[i].fd != (ngx_socket_t) -1) {
                continue;
            }

            if (ls[i].inherited) {

                /* TODO: close on exit */
                /* TODO: nonblocking */
                /* TODO: deferred accept */

                continue;
            }

            s = ngx_socket(ls[i].sockaddr->sa_family, ls[i].type, 0);

            if (s == (ngx_socket_t) -1) {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              ngx_socket_n " %V failed", &ls[i].addr_text);
                return NGX_ERROR;
            }

            if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                           (const void *) &reuseaddr, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              "setsockopt(SO_REUSEADDR) %V failed",
                              &ls[i].addr_text);

                if (ngx_close_socket(s) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  ngx_close_socket_n " %V failed",
                                  &ls[i].addr_text);
                }

                return NGX_ERROR;
            }

#if (NGX_HAVE_REUSEPORT)

            if (ls[i].reuseport) {
                int  reuseport;

                reuseport = 1;

                if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT,
                               (const void *) &reuseport, sizeof(int))
                    == -1)
                {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  "setsockopt(SO_REUSEPORT) %V failed, ignored",
                                  &ls[i].addr_text);

                    if (ngx_close_socket(s) == -1) {
                        ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                      ngx_close_socket_n " %V failed",
                                      &ls[i].addr_text);
                    }

                    return NGX_ERROR;
                }
            }
#endif

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)

            if (ls[i].sockaddr->sa_family == AF_INET6) {
                int  ipv6only;

                ipv6only = ls[i].ipv6only;

                if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,
                               (const void *) &ipv6only, sizeof(int))
                    == -1)
                {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  "setsockopt(IPV6_V6ONLY) %V failed, ignored",
                                  &ls[i].addr_text);
                }
            }
#endif
            /* TODO: close on exit */

            if (!(ngx_event_flags & NGX_USE_IOCP_EVENT)) {
                if (ngx_nonblocking(s) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  ngx_nonblocking_n " %V failed",
                                  &ls[i].addr_text);

                    if (ngx_close_socket(s) == -1) {
                        ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                      ngx_close_socket_n " %V failed",
                                      &ls[i].addr_text);
                    }

                    return NGX_ERROR;
                }
            }

            ngx_log_debug2(NGX_LOG_DEBUG_CORE, log, 0,
                           "bind() %V #%d ", &ls[i].addr_text, s);

            if (bind(s, ls[i].sockaddr, ls[i].socklen) == -1) {
                err = ngx_socket_errno;

                if (err != NGX_EADDRINUSE || !ngx_test_config) {
                    ngx_log_error(NGX_LOG_EMERG, log, err,
                                  "bind() to %V failed", &ls[i].addr_text);
                }

                if (ngx_close_socket(s) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  ngx_close_socket_n " %V failed",
                                  &ls[i].addr_text);
                }

                if (err != NGX_EADDRINUSE) {
                    return NGX_ERROR;
                }

                if (!ngx_test_config) {
                    failed = 1;
                }

                continue;
            }

#if (NGX_HAVE_UNIX_DOMAIN)

            if (ls[i].sockaddr->sa_family == AF_UNIX) {
                mode_t   mode;
                u_char  *name;

                name = ls[i].addr_text.data + sizeof("unix:") - 1;
                mode = (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

                if (chmod((char *) name, mode) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                                  "chmod() \"%s\" failed", name);
                }

                if (ngx_test_config) {
                    if (ngx_delete_file(name) == NGX_FILE_ERROR) {
                        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                                      ngx_delete_file_n " %s failed", name);
                    }
                }
            }
#endif

            if (ls[i].type != SOCK_STREAM) {
                ls[i].fd = s;
                continue;
            }

            if (listen(s, ls[i].backlog) == -1) {
                err = ngx_socket_errno;

                /*
                 * on OpenVZ after suspend/resume EADDRINUSE
                 * may be returned by listen() instead of bind(), see
                 * https://bugzilla.openvz.org/show_bug.cgi?id=2470
                 */

                if (err != NGX_EADDRINUSE || !ngx_test_config) {
                    ngx_log_error(NGX_LOG_EMERG, log, err,
                                  "listen() to %V, backlog %d failed",
                                  &ls[i].addr_text, ls[i].backlog);
                }

                if (ngx_close_socket(s) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  ngx_close_socket_n " %V failed",
                                  &ls[i].addr_text);
                }

                if (err != NGX_EADDRINUSE) {
                    return NGX_ERROR;
                }

                if (!ngx_test_config) {
                    failed = 1;
                }

                continue;
            }

            ls[i].listen = 1;

            ls[i].fd = s;
        }

        if (!failed) {
            break;
        }

        /* TODO: delay configurable */

        ngx_log_error(NGX_LOG_NOTICE, log, 0,
                      "try again to bind() after 500ms");

        ngx_msleep(500);
    }

    if (failed) {
        ngx_log_error(NGX_LOG_EMERG, log, 0, "still could not bind()");
        return NGX_ERROR;
    }

    return NGX_OK;
}
{% endhighlight %}
本函数用于打开cycle->listening.elts中符合要求的所有监听sockets。我们在objs/ngx_auto_config.h头文件中有如下定义：
<pre>
#ifndef NGX_HAVE_REUSEPORT
#define NGX_HAVE_REUSEPORT  1
#endif
</pre>
下面我们来简单分析一下该函数：
{% highlight string %}
ngx_int_t
ngx_open_listening_sockets(ngx_cycle_t *cycle)
{
    // 循环检查5次，直到所有都已经打开
    for (tries = 5; tries; tries--) 
    {
        for (i = 0; i < cycle->listening.nelts; i++)
        {
            //1: 对于标记为ignore的ngx_listening_t对象，则直接忽略
             
            //2: 这里主要是为了将一个未被设置为SO_REUSEPORT的socket转化为REUSEPORT类型的socket
            #if (NGX_HAVE_REUSEPORT)
                if (ls[i].add_reuseport)
                {
                    setsockopt(ls[i].fd, SOL_SOCKET, SO_REUSEPORT,(const void *) &reuseport, sizeof(int));
                    ls[i].add_reuseport = 0;
                }
            #endif
          
           //3: 如果fd不为-1，则表示此socket已经处于监听状态，不再继续后续的流程
           if (ls[i].fd != (ngx_socket_t) -1) {
                continue;
            }

           //4: 如果是继承而来的socket，也不再继续后面的流程
           if (ls[i].inherited) {
                continue;
            }

           //5: 创建socket，并设置相关属性
           s = ngx_socket(ls[i].sockaddr->sa_family, ls[i].type, 0);
           setsockopt(s, SOL_SOCKET, SO_REUSEADDR,(const void *) &reuseaddr, sizeof(int))
        
           #if (NGX_HAVE_REUSEPORT)
               setsockopt(s, SOL_SOCKET, SO_REUSEPORT,(const void *) &reuseport, sizeof(int));
           #endif
 
           #if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
               setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,(const void *) &ipv6only, sizeof(int))
           #endif

           ngx_nonblocking();        //如果采用的事件模型不是IOCP模型，则这里设置为非阻塞

           bind(s, ls[i].sockaddr, ls[i].socklen)；   //绑定

           //6: unix domain socket的处理
           // 对于unix domain socket，会在本地创建出一个unix域文件，这里更改相应的权限设置

           //7: 对于非TCP类型socket，则不再需要后续操作
           if (ls[i].type != SOCK_STREAM) {
                ls[i].fd = s;
                continue;
            }

           //8: 监听
           listen(s, ls[i].backlog);
           ls[i].listen = 1;
           ls[i].fd = s;
        }
    }
}
{% endhighlight %}

## 5. 函数ngx_configure_listening_sockets()
{% highlight string %}
void
ngx_configure_listening_sockets(ngx_cycle_t *cycle)
{
    int                        value;
    ngx_uint_t                 i;
    ngx_listening_t           *ls;

#if (NGX_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
    struct accept_filter_arg   af;
#endif

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {

        ls[i].log = *ls[i].logp;

        if (ls[i].rcvbuf != -1) {
            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF,
                           (const void *) &ls[i].rcvbuf, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(SO_RCVBUF, %d) %V failed, ignored",
                              ls[i].rcvbuf, &ls[i].addr_text);
            }
        }

        if (ls[i].sndbuf != -1) {
            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF,
                           (const void *) &ls[i].sndbuf, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(SO_SNDBUF, %d) %V failed, ignored",
                              ls[i].sndbuf, &ls[i].addr_text);
            }
        }

        if (ls[i].keepalive) {
            value = (ls[i].keepalive == 1) ? 1 : 0;

            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_KEEPALIVE,
                           (const void *) &value, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(SO_KEEPALIVE, %d) %V failed, ignored",
                              value, &ls[i].addr_text);
            }
        }

#if (NGX_HAVE_KEEPALIVE_TUNABLE)

        if (ls[i].keepidle) {
            value = ls[i].keepidle;

#if (NGX_KEEPALIVE_FACTOR)
            value *= NGX_KEEPALIVE_FACTOR;
#endif

            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPIDLE,
                           (const void *) &value, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(TCP_KEEPIDLE, %d) %V failed, ignored",
                              value, &ls[i].addr_text);
            }
        }

        if (ls[i].keepintvl) {
            value = ls[i].keepintvl;

#if (NGX_KEEPALIVE_FACTOR)
            value *= NGX_KEEPALIVE_FACTOR;
#endif

            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPINTVL,
                           (const void *) &value, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                             "setsockopt(TCP_KEEPINTVL, %d) %V failed, ignored",
                             value, &ls[i].addr_text);
            }
        }

        if (ls[i].keepcnt) {
            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPCNT,
                           (const void *) &ls[i].keepcnt, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(TCP_KEEPCNT, %d) %V failed, ignored",
                              ls[i].keepcnt, &ls[i].addr_text);
            }
        }

#endif

#if (NGX_HAVE_SETFIB)
        if (ls[i].setfib != -1) {
            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_SETFIB,
                           (const void *) &ls[i].setfib, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(SO_SETFIB, %d) %V failed, ignored",
                              ls[i].setfib, &ls[i].addr_text);
            }
        }
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
        if (ls[i].fastopen != -1) {
            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_FASTOPEN,
                           (const void *) &ls[i].fastopen, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(TCP_FASTOPEN, %d) %V failed, ignored",
                              ls[i].fastopen, &ls[i].addr_text);
            }
        }
#endif

#if 0
        if (1) {
            int tcp_nodelay = 1;

            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_NODELAY,
                       (const void *) &tcp_nodelay, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(TCP_NODELAY) %V failed, ignored",
                              &ls[i].addr_text);
            }
        }
#endif

        if (ls[i].listen) {

            /* change backlog via listen() */

            if (listen(ls[i].fd, ls[i].backlog) == -1) {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "listen() to %V, backlog %d failed, ignored",
                              &ls[i].addr_text, ls[i].backlog);
            }
        }

        /*
         * setting deferred mode should be last operation on socket,
         * because code may prematurely continue cycle on failure
         */

#if (NGX_HAVE_DEFERRED_ACCEPT)

#ifdef SO_ACCEPTFILTER

        if (ls[i].delete_deferred) {
            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_ACCEPTFILTER, NULL, 0)
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(SO_ACCEPTFILTER, NULL) "
                              "for %V failed, ignored",
                              &ls[i].addr_text);

                if (ls[i].accept_filter) {
                    ngx_log_error(NGX_LOG_ALERT, cycle->log, 0,
                                  "could not change the accept filter "
                                  "to \"%s\" for %V, ignored",
                                  ls[i].accept_filter, &ls[i].addr_text);
                }

                continue;
            }

            ls[i].deferred_accept = 0;
        }

        if (ls[i].add_deferred) {
            ngx_memzero(&af, sizeof(struct accept_filter_arg));
            (void) ngx_cpystrn((u_char *) af.af_name,
                               (u_char *) ls[i].accept_filter, 16);

            if (setsockopt(ls[i].fd, SOL_SOCKET, SO_ACCEPTFILTER,
                           &af, sizeof(struct accept_filter_arg))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(SO_ACCEPTFILTER, \"%s\") "
                              "for %V failed, ignored",
                              ls[i].accept_filter, &ls[i].addr_text);
                continue;
            }

            ls[i].deferred_accept = 1;
        }

#endif

#ifdef TCP_DEFER_ACCEPT

        if (ls[i].add_deferred || ls[i].delete_deferred) {

            if (ls[i].add_deferred) {
                /*
                 * There is no way to find out how long a connection was
                 * in queue (and a connection may bypass deferred queue at all
                 * if syncookies were used), hence we use 1 second timeout
                 * here.
                 */
                value = 1;

            } else {
                value = 0;
            }

            if (setsockopt(ls[i].fd, IPPROTO_TCP, TCP_DEFER_ACCEPT,
                           &value, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(TCP_DEFER_ACCEPT, %d) for %V failed, "
                              "ignored",
                              value, &ls[i].addr_text);

                continue;
            }
        }

        if (ls[i].add_deferred) {
            ls[i].deferred_accept = 1;
        }

#endif

#endif /* NGX_HAVE_DEFERRED_ACCEPT */

#if (NGX_HAVE_IP_RECVDSTADDR)

        if (ls[i].wildcard
            && ls[i].type == SOCK_DGRAM
            && ls[i].sockaddr->sa_family == AF_INET)
        {
            value = 1;

            if (setsockopt(ls[i].fd, IPPROTO_IP, IP_RECVDSTADDR,
                           (const void *) &value, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(IP_RECVDSTADDR) "
                              "for %V failed, ignored",
                              &ls[i].addr_text);
            }
        }

#elif (NGX_HAVE_IP_PKTINFO)

        if (ls[i].wildcard
            && ls[i].type == SOCK_DGRAM
            && ls[i].sockaddr->sa_family == AF_INET)
        {
            value = 1;

            if (setsockopt(ls[i].fd, IPPROTO_IP, IP_PKTINFO,
                           (const void *) &value, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(IP_PKTINFO) "
                              "for %V failed, ignored",
                              &ls[i].addr_text);
            }
        }

#endif

#if (NGX_HAVE_INET6 && NGX_HAVE_IPV6_RECVPKTINFO)

        if (ls[i].wildcard
            && ls[i].type == SOCK_DGRAM
            && ls[i].sockaddr->sa_family == AF_INET6)
        {
            value = 1;

            if (setsockopt(ls[i].fd, IPPROTO_IPV6, IPV6_RECVPKTINFO,
                           (const void *) &value, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                              "setsockopt(IPV6_RECVPKTINFO) "
                              "for %V failed, ignored",
                              &ls[i].addr_text);
            }
        }

#endif
    }

    return;
}
{% endhighlight %}
本函数用于配置监听sockets。在objs/ngx_auto_config.h头文件中我们有如下宏定义：
<pre>
#ifndef NGX_HAVE_DEFERRED_ACCEPT
#define NGX_HAVE_DEFERRED_ACCEPT  1
#endif

#ifndef NGX_HAVE_KEEPALIVE_TUNABLE
#define NGX_HAVE_KEEPALIVE_TUNABLE  1
#endif

#ifndef NGX_HAVE_TCP_FASTOPEN
#define NGX_HAVE_TCP_FASTOPEN  1
#endif

#ifndef NGX_HAVE_IP_PKTINFO
#define NGX_HAVE_IP_PKTINFO  1
#endif
</pre>
接下来我们来简要分析一下该函数：
{% highlight string %}
void
ngx_configure_listening_sockets(ngx_cycle_t *cycle)
{
	for (i = 0; i < cycle->listening.nelts; i++) 
    {
		//1: 根据ls[i].rcvbuf条件，设置接收缓冲区
        setsockopt(ls[i].fd, SOL_SOCKET, SO_RCVBUF,(const void *) &ls[i].rcvbuf, sizeof(int))

        //2: 根据ls[i].sndbuf条件，设置发送缓冲区
        setsockopt(ls[i].fd, SOL_SOCKET, SO_SNDBUF,(const void *) &ls[i].sndbuf, sizeof(int))

        //3: 根据ls[i].keepalive条件，设置socket的keepalive属性
        setsockopt(ls[i].fd, SOL_SOCKET, SO_KEEPALIVE,(const void *) &value, sizeof(int))
        #if (NGX_HAVE_KEEPALIVE_TUNABLE)
           setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPIDLE,(const void *) &value, sizeof(int))
           
           setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPINTVL,(const void *) &value, sizeof(int))

           setsockopt(ls[i].fd, IPPROTO_TCP, TCP_KEEPCNT,(const void *) &ls[i].keepcnt, sizeof(int))
        #endif 
      
        //4: 当前并不支持setfib: 设置该socket的关联路由表
        #if (NGX_HAVE_SETFIB)
           setsockopt(ls[i].fd, SOL_SOCKET, SO_SETFIB,(const void *) &ls[i].setfib, sizeof(int))
        #endif

        //5: 根据ls[i].fastopen条件，设置socket快速打开
        #if (NGX_HAVE_TCP_FASTOPEN)
            setsockopt(ls[i].fd, IPPROTO_TCP, TCP_FASTOPEN,(const void *) &ls[i].fastopen, sizeof(int))
        #endif

        //6: 根据ls[i].listen条件，更改listen的backlog队列
        listen(ls[i].fd, ls[i].backlog)

        //7: 设置延迟接收,设置失败则continue，跳过本次循环
        #if (NGX_HAVE_DEFERRED_ACCEPT)
           if (ls[i].delete_deferred) {
              setsockopt(ls[i].fd, SOL_SOCKET, SO_ACCEPTFILTER, NULL, 0)
           }
           if (ls[i].add_deferred) {
              setsockopt(ls[i].fd, SOL_SOCKET, SO_ACCEPTFILTER,&af, sizeof(struct accept_filter_arg))
           }

           setsockopt(ls[i].fd, IPPROTO_TCP, TCP_DEFER_ACCEPT,&value, sizeof(int))
        #endif

        //8: 获得路由目的地址选项设置
        #if (NGX_HAVE_IP_RECVDSTADDR)
            setsockopt(ls[i].fd, IPPROTO_IP, IP_RECVDSTADDR,(const void *) &value, sizeof(int))
        #elif (NGX_HAVE_IP_PKTINFO)
            setsockopt(ls[i].fd, IPPROTO_IP, IP_PKTINFO,(const void *) &value, sizeof(int))
        #endif

        #if (NGX_HAVE_INET6 && NGX_HAVE_IPV6_RECVPKTINFO)   //当前并未定义NGX_HAVE_INET6宏定义，
           setsockopt(ls[i].fd, IPPROTO_IPV6, IPV6_RECVPKTINFO, (const void *) &value, sizeof(int))
        #endif
    }
}
{% endhighlight %}



<br />
<br />

**[参看]:**

1. [setsockopt](https://www.freebsd.org/cgi/man.cgi?query=setsockopt&sektion=2)

<br />
<br />
<br />

