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

下面我们对该函数进行简单的解释：
{% highlight string %}
ngx_connection_t *
ngx_get_connection(ngx_socket_t s, ngx_log_t *log)
{
   //1: socket句柄s不能大于ngx_cycle->files_n，否则是没有地方存放的，也不可能会出现这种情况。若出现，则肯定发生了错误

   //2: 从free_connections中取出一个ngx_connection_t对象，如果当前已经没有空闲，则通过ngx_drain_connections()释放长连接的
   //方式来获得空闲连接。如果还是不能或者，直接返回NULL
   
   //3: 将获取到的长连接存放进ngx_cycle->files[s]中

   //4: 设置instance
   // 一般情况下是用rev->instance与另外保存的一个instance进行对比，如果不相等，则说明是一个
   // stale事件。因此这里对instance进行取反，表面当前指定的这个events是属于过期事件，不应该被处理。

   rev->instance = !instance;
   wev->instance = !instance;
  
   //5: 获取到空闲连接，设置为可写状态
   wev->write = 1;
}
{% endhighlight %}


## 3. 函数ngx_free_connection()
{% highlight string %}
void
ngx_free_connection(ngx_connection_t *c)
{
    c->data = ngx_cycle->free_connections;
    ngx_cycle->free_connections = c;
    ngx_cycle->free_connection_n++;

    if (ngx_cycle->files && ngx_cycle->files[c->fd] == c) {
        ngx_cycle->files[c->fd] = NULL;
    }
}
{% endhighlight %}
此函数用于释放```ngx_connection_t```连接，将其插入到```ngx_cycle->free_connections```链表头，并且如果该connection存放在```ngx_cycle->files[c->fd]```中,则从该位置移除。

## 4. 函数ngx_close_connection()
{% highlight string %}
void
ngx_close_connection(ngx_connection_t *c)
{
    ngx_err_t     err;
    ngx_uint_t    log_error, level;
    ngx_socket_t  fd;

    if (c->fd == (ngx_socket_t) -1) {
        ngx_log_error(NGX_LOG_ALERT, c->log, 0, "connection already closed");
        return;
    }

    if (c->read->timer_set) {
        ngx_del_timer(c->read);
    }

    if (c->write->timer_set) {
        ngx_del_timer(c->write);
    }

    if (!c->shared) {
        if (ngx_del_conn) {
            ngx_del_conn(c, NGX_CLOSE_EVENT);

        } else {
            if (c->read->active || c->read->disabled) {
                ngx_del_event(c->read, NGX_READ_EVENT, NGX_CLOSE_EVENT);
            }

            if (c->write->active || c->write->disabled) {
                ngx_del_event(c->write, NGX_WRITE_EVENT, NGX_CLOSE_EVENT);
            }
        }
    }

    if (c->read->posted) {
        ngx_delete_posted_event(c->read);
    }

    if (c->write->posted) {
        ngx_delete_posted_event(c->write);
    }

    c->read->closed = 1;
    c->write->closed = 1;

    ngx_reusable_connection(c, 0);

    log_error = c->log_error;

    ngx_free_connection(c);

    fd = c->fd;
    c->fd = (ngx_socket_t) -1;

    if (c->shared) {
        return;
    }

    if (ngx_close_socket(fd) == -1) {

        err = ngx_socket_errno;

        if (err == NGX_ECONNRESET || err == NGX_ENOTCONN) {

            switch (log_error) {

            case NGX_ERROR_INFO:
                level = NGX_LOG_INFO;
                break;

            case NGX_ERROR_ERR:
                level = NGX_LOG_ERR;
                break;

            default:
                level = NGX_LOG_CRIT;
            }

        } else {
            level = NGX_LOG_CRIT;
        }

        /* we use ngx_cycle->log because c->log was in c->pool */

        ngx_log_error(level, ngx_cycle->log, err,
                      ngx_close_socket_n " %d failed", fd);
    }
}
{% endhighlight %}
下面我们来简单介绍一下ngx_close_connection()函数的实现：
{% highlight string %}
void
ngx_close_connection(ngx_connection_t *c)
{
    //1: 如果c->fd == -1，说明连接已经关闭

    //2: 移除connection上关联的读写定时器事件

    //3: 如果不是共享connection的话，移除该connection上关联的读写事件

    //4: 移除该连接已经投递到队列中的事件

    //5: 回收连接
    ngx_reusable_connection(c,0);
    ngx_free_connection(c);

    //6: 非共享connection的话，需要关闭对应的fd
}
{% endhighlight %}

## 5. 函数ngx_reusable_connection()
{% highlight string %}
void
ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable)
{
    ngx_log_debug1(NGX_LOG_DEBUG_CORE, c->log, 0,
                   "reusable connection: %ui", reusable);

    if (c->reusable) {
        ngx_queue_remove(&c->queue);

#if (NGX_STAT_STUB)
        (void) ngx_atomic_fetch_add(ngx_stat_waiting, -1);
#endif
    }

    c->reusable = reusable;

    if (reusable) {
        /* need cast as ngx_cycle is volatile */

        ngx_queue_insert_head(
            (ngx_queue_t *) &ngx_cycle->reusable_connections_queue, &c->queue);

#if (NGX_STAT_STUB)
        (void) ngx_atomic_fetch_add(ngx_stat_waiting, 1);
#endif
    }
}
{% endhighlight %}
此函数主要用于在```reusable```为true，即表示该连接需要马上被复用，因此这里会先从队列中移除，然后再重新加入到可复用连接队列中。

其中```ngx->reusable_connections_queue```是一个双端队列，如下图所示：

![ngx-cycle](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_reusable_connqueue.jpg)


## 6. 函数ngx_drain_connections()
{% highlight string %}
static void
ngx_drain_connections(void)
{
    ngx_int_t          i;
    ngx_queue_t       *q;
    ngx_connection_t  *c;

    for (i = 0; i < 32; i++) {
        if (ngx_queue_empty(&ngx_cycle->reusable_connections_queue)) {
            break;
        }

        q = ngx_queue_last(&ngx_cycle->reusable_connections_queue);
        c = ngx_queue_data(q, ngx_connection_t, queue);

        ngx_log_debug0(NGX_LOG_DEBUG_CORE, c->log, 0,
                       "reusing connection");

        c->close = 1;
        c->read->handler(c->read);
    }
}
{% endhighlight %}
这里主要是从```ngx_cycle->reusable_connections_queue```中释放长连接，释放完成后加入到空闲连接池，以供后续新连接使用。
<pre>
注意： 这里只有在ngx_http_set_keepalive()中会将connection->reusable置为1，因此这里可复用的连接绑定的read->handler
为ngx_http_keepalive_handler()
</pre>

## 7. 函数ngx_close_idle_connections()
{% highlight string %}
void
ngx_close_idle_connections(ngx_cycle_t *cycle)
{
    ngx_uint_t         i;
    ngx_connection_t  *c;

    c = cycle->connections;

    for (i = 0; i < cycle->connection_n; i++) {

        /* THREAD: lock */

        if (c[i].fd != (ngx_socket_t) -1 && c[i].idle) {
            c[i].close = 1;
            c[i].read->handler(c[i].read);
        }
    }
}
{% endhighlight %}
这里遍历cycle->connections链表，关闭所有空闲连接。

## 8. 函数ngx_connection_local_sockaddr()
{% highlight string %}
ngx_int_t
ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port)
{
    socklen_t             len;
    ngx_uint_t            addr;
    u_char                sa[NGX_SOCKADDRLEN];
    struct sockaddr_in   *sin;
#if (NGX_HAVE_INET6)
    ngx_uint_t            i;
    struct sockaddr_in6  *sin6;
#endif

    addr = 0;

    if (c->local_socklen) {
        switch (c->local_sockaddr->sa_family) {

#if (NGX_HAVE_INET6)
        case AF_INET6:
            sin6 = (struct sockaddr_in6 *) c->local_sockaddr;

            for (i = 0; addr == 0 && i < 16; i++) {
                addr |= sin6->sin6_addr.s6_addr[i];
            }

            break;
#endif

#if (NGX_HAVE_UNIX_DOMAIN)
        case AF_UNIX:
            addr = 1;
            break;
#endif

        default: /* AF_INET */
            sin = (struct sockaddr_in *) c->local_sockaddr;
            addr = sin->sin_addr.s_addr;
            break;
        }
    }

    if (addr == 0) {

        len = NGX_SOCKADDRLEN;

        if (getsockname(c->fd, (struct sockaddr *) &sa, &len) == -1) {
            ngx_connection_error(c, ngx_socket_errno, "getsockname() failed");
            return NGX_ERROR;
        }

        c->local_sockaddr = ngx_palloc(c->pool, len);
        if (c->local_sockaddr == NULL) {
            return NGX_ERROR;
        }

        ngx_memcpy(c->local_sockaddr, &sa, len);

        c->local_socklen = len;
    }

    if (s == NULL) {
        return NGX_OK;
    }

    s->len = ngx_sock_ntop(c->local_sockaddr, c->local_socklen,
                           s->data, s->len, port);

    return NGX_OK;
}
{% endhighlight %}
在当前我们的配置当中，并不支持```NGX_HAVE_INET6```，但是在obs/ngx_auto_config.h头文件中有如下宏定义：
<pre>
#ifndef NGX_HAVE_UNIX_DOMAIN
#define NGX_HAVE_UNIX_DOMAIN  1
#endif
</pre>
本函数用于获取```ngx_connection_t```所绑定的本地socket地址，将其转化成字符串表示形式返回。下面简要解释一下：
{% highlight string %}
ngx_int_t
ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port)
{
    //1: 收件检查c->local_sockaddr保存的是否是一个有效的IP地址(addr不为0）

    //2: 如果是无效的IP地址，则通过getsockname()来获取，保存到c->local_sockaddr

    //3: 将c->local_sockaddr地址通过ngx_sock_ntop()函数转换成字符串表示形式，返回
}
{% endhighlight %}


## 9. 函数ngx_connection_error()
{% highlight string %}
ngx_int_t
ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text)
{
    ngx_uint_t  level;

    /* Winsock may return NGX_ECONNABORTED instead of NGX_ECONNRESET */

    if ((err == NGX_ECONNRESET
#if (NGX_WIN32)
         || err == NGX_ECONNABORTED
#endif
        ) && c->log_error == NGX_ERROR_IGNORE_ECONNRESET)
    {
        return 0;
    }

#if (NGX_SOLARIS)
    if (err == NGX_EINVAL && c->log_error == NGX_ERROR_IGNORE_EINVAL) {
        return 0;
    }
#endif

    if (err == 0
        || err == NGX_ECONNRESET
#if (NGX_WIN32)
        || err == NGX_ECONNABORTED
#else
        || err == NGX_EPIPE
#endif
        || err == NGX_ENOTCONN
        || err == NGX_ETIMEDOUT
        || err == NGX_ECONNREFUSED
        || err == NGX_ENETDOWN
        || err == NGX_ENETUNREACH
        || err == NGX_EHOSTDOWN
        || err == NGX_EHOSTUNREACH)
    {
        switch (c->log_error) {

        case NGX_ERROR_IGNORE_EINVAL:
        case NGX_ERROR_IGNORE_ECONNRESET:
        case NGX_ERROR_INFO:
            level = NGX_LOG_INFO;
            break;

        default:
            level = NGX_LOG_ERR;
        }

    } else {
        level = NGX_LOG_ALERT;
    }

    ngx_log_error(level, c->log, err, text);

    return NGX_ERROR;
}
{% endhighlight %}
当前我们运行在Linux环境下，因此不支持```NGX_WIN32```与```NGX_SOLARIS```。此函数主要用于打印```ngx_connection_t```中的日志信息。```ngx_connection_t.log_error```定义了一个连接中的日志打印级别。



<br />
<br />

**[参看]:**

1. [setsockopt](https://www.freebsd.org/cgi/man.cgi?query=setsockopt&sektion=2)

2. [nginx源码初读（8）--让烦恼从数据结构开始(ngx_event)](http://blog.csdn.net/wuchunlai_2012/article/details/50731037)

3. [nginx keepalive连接回收机制](http://blog.csdn.net/brainkick/article/details/7321894)

<br />
<br />
<br />

