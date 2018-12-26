---
layout: post
title: event/ngx_event_connect.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


在前面相关章节，我们知道nginx定义了```ngx_connection_t```数据结构来表示连接，这种连接通常表示由客户端主动发起、Nginx服务器被动接收的TCP连接（当然UDP连接也会用该结构来表示），称为被动连接。还有一类连接，在某些请求的处理过程中，Nginx会试图主动向其他上游服务器建立连接，并以此连接与上游服务器进行通信，Nginx定义**ngx_peer_connection_t**结构来表示，这类可以称为```主动连接```。本质上来说，```主动连接```是以**ngx_connection_t**结构体为基础实现的。


<!-- more -->

## 1. 函数ngx_event_connect_peer()
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>


ngx_int_t
ngx_event_connect_peer(ngx_peer_connection_t *pc)
{
    int                rc, type;
    ngx_int_t          event;
    ngx_err_t          err;
    ngx_uint_t         level;
    ngx_socket_t       s;
    ngx_event_t       *rev, *wev;
    ngx_connection_t  *c;

    rc = pc->get(pc, pc->data);
    if (rc != NGX_OK) {
        return rc;
    }

    type = (pc->type ? pc->type : SOCK_STREAM);

    s = ngx_socket(pc->sockaddr->sa_family, type, 0);

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, pc->log, 0, "%s socket %d",
                   (type == SOCK_STREAM) ? "stream" : "dgram", s);

    if (s == (ngx_socket_t) -1) {
        ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                      ngx_socket_n " failed");
        return NGX_ERROR;
    }


    c = ngx_get_connection(s, pc->log);

    if (c == NULL) {
        if (ngx_close_socket(s) == -1) {
            ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                          ngx_close_socket_n "failed");
        }

        return NGX_ERROR;
    }

    c->type = type;

    if (pc->rcvbuf) {
        if (setsockopt(s, SOL_SOCKET, SO_RCVBUF,
                       (const void *) &pc->rcvbuf, sizeof(int)) == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                          "setsockopt(SO_RCVBUF) failed");
            goto failed;
        }
    }

    if (ngx_nonblocking(s) == -1) {
        ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                      ngx_nonblocking_n " failed");

        goto failed;
    }

    if (pc->local) {
        if (bind(s, pc->local->sockaddr, pc->local->socklen) == -1) {
            ngx_log_error(NGX_LOG_CRIT, pc->log, ngx_socket_errno,
                          "bind(%V) failed", &pc->local->name);

            goto failed;
        }
    }

    if (type == SOCK_STREAM) {
        c->recv = ngx_recv;
        c->send = ngx_send;
        c->recv_chain = ngx_recv_chain;
        c->send_chain = ngx_send_chain;

        c->sendfile = 1;

        if (pc->sockaddr->sa_family == AF_UNIX) {
            c->tcp_nopush = NGX_TCP_NOPUSH_DISABLED;
            c->tcp_nodelay = NGX_TCP_NODELAY_DISABLED;

#if (NGX_SOLARIS)
            /* Solaris's sendfilev() supports AF_NCA, AF_INET, and AF_INET6 */
            c->sendfile = 0;
#endif
        }

    } else { /* type == SOCK_DGRAM */
        c->recv = ngx_udp_recv;
        c->send = ngx_send;
    }

    c->log_error = pc->log_error;

    rev = c->read;
    wev = c->write;

    rev->log = pc->log;
    wev->log = pc->log;

    pc->connection = c;

    c->number = ngx_atomic_fetch_add(ngx_connection_counter, 1);

    if (ngx_add_conn) {
        if (ngx_add_conn(c) == NGX_ERROR) {
            goto failed;
        }
    }

    ngx_log_debug3(NGX_LOG_DEBUG_EVENT, pc->log, 0,
                   "connect to %V, fd:%d #%uA", pc->name, s, c->number);

    rc = connect(s, pc->sockaddr, pc->socklen);

    if (rc == -1) {
        err = ngx_socket_errno;


        if (err != NGX_EINPROGRESS
#if (NGX_WIN32)
            /* Winsock returns WSAEWOULDBLOCK (NGX_EAGAIN) */
            && err != NGX_EAGAIN
#endif
            )
        {
            if (err == NGX_ECONNREFUSED
#if (NGX_LINUX)
                /*
                 * Linux returns EAGAIN instead of ECONNREFUSED
                 * for unix sockets if listen queue is full
                 */
                || err == NGX_EAGAIN
#endif
                || err == NGX_ECONNRESET
                || err == NGX_ENETDOWN
                || err == NGX_ENETUNREACH
                || err == NGX_EHOSTDOWN
                || err == NGX_EHOSTUNREACH)
            {
                level = NGX_LOG_ERR;

            } else {
                level = NGX_LOG_CRIT;
            }

            ngx_log_error(level, c->log, err, "connect() to %V failed",
                          pc->name);

            ngx_close_connection(c);
            pc->connection = NULL;

            return NGX_DECLINED;
        }
    }

    if (ngx_add_conn) {
        if (rc == -1) {

            /* NGX_EINPROGRESS */

            return NGX_AGAIN;
        }

        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, pc->log, 0, "connected");

        wev->ready = 1;

        return NGX_OK;
    }

    if (ngx_event_flags & NGX_USE_IOCP_EVENT) {

        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, pc->log, ngx_socket_errno,
                       "connect(): %d", rc);

        if (ngx_blocking(s) == -1) {
            ngx_log_error(NGX_LOG_ALERT, pc->log, ngx_socket_errno,
                          ngx_blocking_n " failed");
            goto failed;
        }

        /*
         * FreeBSD's aio allows to post an operation on non-connected socket.
         * NT does not support it.
         *
         * TODO: check in Win32, etc. As workaround we can use NGX_ONESHOT_EVENT
         */

        rev->ready = 1;
        wev->ready = 1;

        return NGX_OK;
    }

    if (ngx_event_flags & NGX_USE_CLEAR_EVENT) {

        /* kqueue */

        event = NGX_CLEAR_EVENT;

    } else {

        /* select, poll, /dev/poll */

        event = NGX_LEVEL_EVENT;
    }

    if (ngx_add_event(rev, NGX_READ_EVENT, event) != NGX_OK) {
        goto failed;
    }

    if (rc == -1) {

        /* NGX_EINPROGRESS */

        if (ngx_add_event(wev, NGX_WRITE_EVENT, event) != NGX_OK) {
            goto failed;
        }

        return NGX_AGAIN;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, pc->log, 0, "connected");

    wev->ready = 1;

    return NGX_OK;

failed:

    ngx_close_connection(c);
    pc->connection = NULL;

    return NGX_ERROR;
}
{% endhighlight %}
本函数用于Nginx向上游服务器发起连接。下面我们简要分析一下函数的执行流程：
{% highlight string %}
ngx_int_t
ngx_event_connect_peer(ngx_peer_connection_t *pc)
{
	//1) 检查是否能够直接从连接池中获取
	rc = pc->get(pc, pc->data);
	if (rc != NGX_OK) {
		return rc;
	}

	//2) 建立socket
	type = (pc->type ? pc->type : SOCK_STREAM);
	
	s = ngx_socket(pc->sockaddr->sa_family, type, 0);

	//3) 获取一个ngx_connection_t对象
	c = ngx_get_connection(s, pc->log);

	//4) 设置接收缓冲区大小、设置为非阻塞、绑定本地地址
	if (pc->rcvbuf) {
		if (setsockopt(s, SOL_SOCKET, SO_RCVBUF,(const void *) &pc->rcvbuf, sizeof(int)) == -1)
		{
	
		}
	}
	if (ngx_nonblocking(s) == -1) {
	
	}
	if (pc->local) {
		if (bind(s, pc->local->sockaddr, pc->local->socklen) == -1) {
	
		}
	}

	//5) 设置相应的发送接收函数
	if (type == SOCK_STREAM) {
		c->recv = ngx_recv;
		c->send = ngx_send;
		c->recv_chain = ngx_recv_chain;
		c->send_chain = ngx_send_chain;
	
		c->sendfile = 1;
	
		if (pc->sockaddr->sa_family == AF_UNIX) {
			c->tcp_nopush = NGX_TCP_NOPUSH_DISABLED;
			c->tcp_nodelay = NGX_TCP_NODELAY_DISABLED;
	
		#if (NGX_SOLARIS)
			/* Solaris's sendfilev() supports AF_NCA, AF_INET, and AF_INET6 */
			c->sendfile = 0;
		#endif
		}
	
	} else { /* type == SOCK_DGRAM */
		c->recv = ngx_udp_recv;
		c->send = ngx_send;
	}


	//6) 用于统计当前的连接数
	c->number = ngx_atomic_fetch_add(ngx_connection_counter, 1);

	//7) 将一个connection(连接）添加到事件驱动机制中，连接上的读、写事件即被加入到了事件驱动机制中
	 if (ngx_add_conn) {
        if (ngx_add_conn(c) == NGX_ERROR) {
            goto failed;
        }
    }

	//8) 向上游服务器发起连接
	rc = connect(s, pc->sockaddr, pc->socklen);

	//9) 经典错误处理
	if(rc == -1){
		//由于是非阻塞连接，因此可能返回NGX_EINPROGRESS(linux)或NGX_EAGAIN(windows)
	}

	//通过上面及后面的一些代码，我们知道当状态为NGX_EINPROGRESS时，会向该socket句柄上添加'写事件'。
	//另外，对于wev及rev绑定handler，是在调用本函数之后的外部代码中来进行的


	//10) 将一个connection(连接）添加到事件驱动机制中，连接上的读、写事件即被加入到了事件驱动机制中
	if (ngx_add_conn){
		return NGX_OK;
	}

	//11) IOCP事件驱动机制，我们这里不做介绍 
	if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
	}

	//12) 将读事件添加到事件驱动机制中(这里还会先觉得使用边沿触发，还是水平触发)
	if (ngx_event_flags & NGX_USE_CLEAR_EVENT) {
	
		/* kqueue */
	
		event = NGX_CLEAR_EVENT;
	
	} else {
	
		/* select, poll, /dev/poll */
	
		event = NGX_LEVEL_EVENT;
	}
	if (ngx_add_event(rev, NGX_READ_EVENT, event) != NGX_OK) {
		goto failed;
	}

	//12) 对于NGX_EINPROGRESS状态，需要添加'写事件'(请注意这一点）
	if (rc == -1) {
	
		/* NGX_EINPROGRESS */
	
		if (ngx_add_event(wev, NGX_WRITE_EVENT, event) != NGX_OK) {
			goto failed;
		}
	
		return NGX_AGAIN;
	}
	wev->ready = 1;
	
	return NGX_OK;


//13) 出错情况，关闭连接
failed:

	ngx_close_connection(c);
	pc->connection = NULL;
	
	return NGX_ERROR;
}
{% endhighlight %}

## 2. 函数ngx_event_get_peer()
{% highlight string %}
ngx_int_t
ngx_event_get_peer(ngx_peer_connection_t *pc, void *data)
{
    return NGX_OK;
}
{% endhighlight %}
//此函数作为一个dummy函数，一般如果我们不需要从连接池中获取连接，则可将```pc->get```设置为此dummy函数




<br />
<br />

**[参看]**

1. [Nginx学习笔记(十九)：Nginx连接](https://blog.csdn.net/fzy0201/article/details/23782397)

2. [linux中使用select和epoll确定异步connect连接是否成功](https://blog.csdn.net/m08090420/article/details/52081333)

3. [linux 客户端 Socket 非阻塞connect编程](https://www.cnblogs.com/GameDeveloper/p/3406565.html)

<br />
<br />
<br />

