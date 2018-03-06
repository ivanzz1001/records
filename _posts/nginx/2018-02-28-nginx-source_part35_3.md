---
layout: post
title: core/ngx_connection.c源文件分析(2)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们介绍一下nginx中对socket connection的相关操作.主要包括两个方面的内容：

* 监听socket对象(ngx_listening_t)相关操作

* 连接socket对象(ngx_connection_t)相关操作


<!-- more -->


## 1. 函数ngx_close_listening_sockets()
{% highlight string %}
void
ngx_close_listening_sockets(ngx_cycle_t *cycle)
{
    ngx_uint_t         i;
    ngx_listening_t   *ls;
    ngx_connection_t  *c;

    if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
        return;
    }

    ngx_accept_mutex_held = 0;
    ngx_use_accept_mutex = 0;

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {

        c = ls[i].connection;

        if (c) {
            if (c->read->active) {
                if (ngx_event_flags & NGX_USE_EPOLL_EVENT) {

                    /*
                     * it seems that Linux-2.6.x OpenVZ sends events
                     * for closed shared listening sockets unless
                     * the events was explicitly deleted
                     */

                    ngx_del_event(c->read, NGX_READ_EVENT, 0);

                } else {
                    ngx_del_event(c->read, NGX_READ_EVENT, NGX_CLOSE_EVENT);
                }
            }

            ngx_free_connection(c);

            c->fd = (ngx_socket_t) -1;
        }

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, cycle->log, 0,
                       "close listening %V #%d ", &ls[i].addr_text, ls[i].fd);

        if (ngx_close_socket(ls[i].fd) == -1) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                          ngx_close_socket_n " %V failed", &ls[i].addr_text);
        }

#if (NGX_HAVE_UNIX_DOMAIN)

        if (ls[i].sockaddr->sa_family == AF_UNIX
            && ngx_process <= NGX_PROCESS_MASTER
            && ngx_new_binary == 0)
        {
            u_char *name = ls[i].addr_text.data + sizeof("unix:") - 1;

            if (ngx_delete_file(name) == NGX_FILE_ERROR) {
                ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                              ngx_delete_file_n " %s failed", name);
            }
        }

#endif

        ls[i].fd = (ngx_socket_t) -1;
    }

    cycle->listening.nelts = 0;
}
{% endhighlight %}
本函数用于关闭cycle->listening中的所有监听sockets。下面我们来简要分析：
{% highlight string %}
void
ngx_close_listening_sockets(ngx_cycle_t *cycle)
{
    //1： 对于IOCP事件模型的socket，这里不做任何处理

    //2: 因为这里关闭了所有监听sockets，因此这里不再持有互斥锁的任何相关变量
    ngx_accept_mutex_held = 0;
    ngx_use_accept_mutex = 0;

    for (i = 0; i < cycle->listening.nelts; i++) 
    {
        //3: 释放连接
        //c->read->active为真，说明该事件已经被注册用于接收IO通知，因此这里需要将该事件删除。
        //说明： 在Linux2.6.X系统下，open操作似乎会引发向一个已关闭的共享监听socket发送事件
        ngx_free_connection(c);

        //4: 关闭socket
        ngx_close_socket(ls[i].fd);

        //5: 对于unix域socket,如果是属于最后一个进程退出，则需要删除本地产生的unix域文件
    }
}
{% endhighlight %}

## 2. 函数ngx_get_connection()
{% highlight string %}
ngx_connection_t *
ngx_get_connection(ngx_socket_t s, ngx_log_t *log)
{
    ngx_uint_t         instance;
    ngx_event_t       *rev, *wev;
    ngx_connection_t  *c;

    /* disable warning: Win32 SOCKET is u_int while UNIX socket is int */

    if (ngx_cycle->files && (ngx_uint_t) s >= ngx_cycle->files_n) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
                      "the new socket has number %d, "
                      "but only %ui files are available",
                      s, ngx_cycle->files_n);
        return NULL;
    }

    c = ngx_cycle->free_connections;

    if (c == NULL) {
        ngx_drain_connections();
        c = ngx_cycle->free_connections;
    }

    if (c == NULL) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
                      "%ui worker_connections are not enough",
                      ngx_cycle->connection_n);

        return NULL;
    }

    ngx_cycle->free_connections = c->data;
    ngx_cycle->free_connection_n--;

    if (ngx_cycle->files && ngx_cycle->files[s] == NULL) {
        ngx_cycle->files[s] = c;
    }

    rev = c->read;
    wev = c->write;

    ngx_memzero(c, sizeof(ngx_connection_t));

    c->read = rev;
    c->write = wev;
    c->fd = s;
    c->log = log;

    instance = rev->instance;

    ngx_memzero(rev, sizeof(ngx_event_t));
    ngx_memzero(wev, sizeof(ngx_event_t));

    rev->instance = !instance;
    wev->instance = !instance;

    rev->index = NGX_INVALID_INDEX;
    wev->index = NGX_INVALID_INDEX;

    rev->data = c;
    wev->data = c;

    wev->write = 1;

    return c;
}
{% endhighlight %}
在讲解本函数之前，我们必须先明白如下几点：

* ```ngx_cycle->files```: 本字段是nginx事件模块初始化时预先分配的一个足够大的空间，用于将来存放所有正在使用的连接（指针）。并且可以通过socket fd来索引该```ngx_connection_t```连接对象。

* ```ngx_cycle->connections```: 预先分配了一个足够大的空间来在这空间分配```ngx_connection_t```对象

* ```ngx_cycle->free_connections```: 本字段存放了所有空闲状态的```ngx_connection_t```对象，通过ngx_connection_t.data字段连接起来。

* ```cycle->read_events```: 预先分配的足够大的```ngx_event_t```对象空间

下面我们简要画出在初状态下，大概的一副场景：

![ngx-cycle](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_get_connection.jpg)



<br />
<br />

**[参看]:**

1. [setsockopt](https://www.freebsd.org/cgi/man.cgi?query=setsockopt&sektion=2)

<br />
<br />
<br />

