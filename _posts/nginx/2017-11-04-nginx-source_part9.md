---
layout: post
title: os/unix/ngx_channel.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要介绍一下nginx中ngx_channel，其主要用于master进程与worker进程之间的通信。

<!-- more -->


## 1. os/unix/ngx_channel.h头文件

头文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CHANNEL_H_INCLUDED_
#define _NGX_CHANNEL_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


typedef struct {
    ngx_uint_t  command;
    ngx_pid_t   pid;
    ngx_int_t   slot;
    ngx_fd_t    fd;
} ngx_channel_t;


ngx_int_t ngx_write_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size,
    ngx_log_t *log);
ngx_int_t ngx_read_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size,
    ngx_log_t *log);
ngx_int_t ngx_add_channel_event(ngx_cycle_t *cycle, ngx_fd_t fd,
    ngx_int_t event, ngx_event_handler_pt handler);
void ngx_close_channel(ngx_fd_t *fd, ngx_log_t *log);


#endif /* _NGX_CHANNEL_H_INCLUDED_ */
{% endhighlight %}

<br />

ngx_channel_t是nginx master与worker之间进程之间通信的常用工具，它是使用本机套接字来实现的。```socketpair```方法用于创建一对匿名的，面向连接的指定域socket:
<pre>
int socketpair(int domain, int type, int protocol, int sv[2]);
</pre>
通常会在父子进程之间通信前，调用socketpair()创建一组套接字，然后再调用fork()方法创建出子进程后，在父进程中关闭sv[1]套接字,在子进程中关闭sv[0]套接字。

**1) ngx_channel_t结构体**

ngx_channel_t结构体是nginx定义的master父进程与worker子进程间通信的消息格式。如下所示：
<pre>
typedef struct {
    ngx_uint_t  command;        //发送的指令
    ngx_pid_t   pid;            //进程ID，一般为发送方进程的ID
    ngx_int_t   slot;           //一般为发送方在ngx_process数组中的序号
    ngx_fd_t    fd;             //通信的套接字句柄
} ngx_channel_t;
</pre>

发送的命令一般有如下几个，在os/unix/ngx_process_cycle.h头文件中定义(其含义我们留待后面介绍)：
<pre>
#define NGX_CMD_OPEN_CHANNEL   1
#define NGX_CMD_CLOSE_CHANNEL  2
#define NGX_CMD_QUIT           3
#define NGX_CMD_TERMINATE      4
#define NGX_CMD_REOPEN         5
</pre>

**2) 操作函数**
<pre>
1. 向channel发送命令。
ngx_int_t ngx_write_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size,
    ngx_log_t *log);

2. 从channel中读取命令.
ngx_int_t ngx_read_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size,
    ngx_log_t *log);

3. 将相应的channel事件(读事件、写事件）加入到监听队列。
ngx_int_t ngx_add_channel_event(ngx_cycle_t *cycle, ngx_fd_t fd,
    ngx_int_t event, ngx_event_handler_pt handler);

4. 关闭channel句柄。
void ngx_close_channel(ngx_fd_t *fd, ngx_log_t *log);
</pre>




## 2. os/unix/ngx_channel.c源文件
源文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_channel.h>


ngx_int_t
ngx_write_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size,
    ngx_log_t *log)
{
    ssize_t             n;
    ngx_err_t           err;
    struct iovec        iov[1];
    struct msghdr       msg;

#if (NGX_HAVE_MSGHDR_MSG_CONTROL)

    union {
        struct cmsghdr  cm;
        char            space[CMSG_SPACE(sizeof(int))];
    } cmsg;

    if (ch->fd == -1) {
        msg.msg_control = NULL;
        msg.msg_controllen = 0;

    } else {
        msg.msg_control = (caddr_t) &cmsg;
        msg.msg_controllen = sizeof(cmsg);

        ngx_memzero(&cmsg, sizeof(cmsg));

        cmsg.cm.cmsg_len = CMSG_LEN(sizeof(int));
        cmsg.cm.cmsg_level = SOL_SOCKET;
        cmsg.cm.cmsg_type = SCM_RIGHTS;

        /*
         * We have to use ngx_memcpy() instead of simple
         *   *(int *) CMSG_DATA(&cmsg.cm) = ch->fd;
         * because some gcc 4.4 with -O2/3/s optimization issues the warning:
         *   dereferencing type-punned pointer will break strict-aliasing rules
         *
         * Fortunately, gcc with -O1 compiles this ngx_memcpy()
         * in the same simple assignment as in the code above
         */

        ngx_memcpy(CMSG_DATA(&cmsg.cm), &ch->fd, sizeof(int));
    }

    msg.msg_flags = 0;

#else

    if (ch->fd == -1) {
        msg.msg_accrights = NULL;
        msg.msg_accrightslen = 0;

    } else {
        msg.msg_accrights = (caddr_t) &ch->fd;
        msg.msg_accrightslen = sizeof(int);
    }

#endif

    iov[0].iov_base = (char *) ch;
    iov[0].iov_len = size;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    n = sendmsg(s, &msg, 0);

    if (n == -1) {
        err = ngx_errno;
        if (err == NGX_EAGAIN) {
            return NGX_AGAIN;
        }

        ngx_log_error(NGX_LOG_ALERT, log, err, "sendmsg() failed");
        return NGX_ERROR;
    }

    return NGX_OK;
}


ngx_int_t
ngx_read_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size, ngx_log_t *log)
{
    ssize_t             n;
    ngx_err_t           err;
    struct iovec        iov[1];
    struct msghdr       msg;

#if (NGX_HAVE_MSGHDR_MSG_CONTROL)
    union {
        struct cmsghdr  cm;
        char            space[CMSG_SPACE(sizeof(int))];
    } cmsg;
#else
    int                 fd;
#endif

    iov[0].iov_base = (char *) ch;
    iov[0].iov_len = size;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

#if (NGX_HAVE_MSGHDR_MSG_CONTROL)
    msg.msg_control = (caddr_t) &cmsg;
    msg.msg_controllen = sizeof(cmsg);
#else
    msg.msg_accrights = (caddr_t) &fd;
    msg.msg_accrightslen = sizeof(int);
#endif

    n = recvmsg(s, &msg, 0);

    if (n == -1) {
        err = ngx_errno;
        if (err == NGX_EAGAIN) {
            return NGX_AGAIN;
        }

        ngx_log_error(NGX_LOG_ALERT, log, err, "recvmsg() failed");
        return NGX_ERROR;
    }

    if (n == 0) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, log, 0, "recvmsg() returned zero");
        return NGX_ERROR;
    }

    if ((size_t) n < sizeof(ngx_channel_t)) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
                      "recvmsg() returned not enough data: %z", n);
        return NGX_ERROR;
    }

#if (NGX_HAVE_MSGHDR_MSG_CONTROL)

    if (ch->command == NGX_CMD_OPEN_CHANNEL) {

        if (cmsg.cm.cmsg_len < (socklen_t) CMSG_LEN(sizeof(int))) {
            ngx_log_error(NGX_LOG_ALERT, log, 0,
                          "recvmsg() returned too small ancillary data");
            return NGX_ERROR;
        }

        if (cmsg.cm.cmsg_level != SOL_SOCKET || cmsg.cm.cmsg_type != SCM_RIGHTS)
        {
            ngx_log_error(NGX_LOG_ALERT, log, 0,
                          "recvmsg() returned invalid ancillary data "
                          "level %d or type %d",
                          cmsg.cm.cmsg_level, cmsg.cm.cmsg_type);
            return NGX_ERROR;
        }

        /* ch->fd = *(int *) CMSG_DATA(&cmsg.cm); */

        ngx_memcpy(&ch->fd, CMSG_DATA(&cmsg.cm), sizeof(int));
    }

    if (msg.msg_flags & (MSG_TRUNC|MSG_CTRUNC)) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
                      "recvmsg() truncated data");
    }

#else

    if (ch->command == NGX_CMD_OPEN_CHANNEL) {
        if (msg.msg_accrightslen != sizeof(int)) {
            ngx_log_error(NGX_LOG_ALERT, log, 0,
                          "recvmsg() returned no ancillary data");
            return NGX_ERROR;
        }

        ch->fd = fd;
    }

#endif

    return n;
}


ngx_int_t
ngx_add_channel_event(ngx_cycle_t *cycle, ngx_fd_t fd, ngx_int_t event,
    ngx_event_handler_pt handler)
{
    ngx_event_t       *ev, *rev, *wev;
    ngx_connection_t  *c;

    c = ngx_get_connection(fd, cycle->log);

    if (c == NULL) {
        return NGX_ERROR;
    }

    c->pool = cycle->pool;

    rev = c->read;
    wev = c->write;

    rev->log = cycle->log;
    wev->log = cycle->log;

    rev->channel = 1;
    wev->channel = 1;

    ev = (event == NGX_READ_EVENT) ? rev : wev;

    ev->handler = handler;

    if (ngx_add_conn && (ngx_event_flags & NGX_USE_EPOLL_EVENT) == 0) {
        if (ngx_add_conn(c) == NGX_ERROR) {
            ngx_free_connection(c);
            return NGX_ERROR;
        }

    } else {
        if (ngx_add_event(ev, event, 0) == NGX_ERROR) {
            ngx_free_connection(c);
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


void
ngx_close_channel(ngx_fd_t *fd, ngx_log_t *log)
{
    if (close(fd[0]) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno, "close() channel failed");
    }

    if (close(fd[1]) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno, "close() channel failed");
    }
}

{% endhighlight %}

### 2.1 向channel发送命令

代码整体结构如下：
{% highlight string %}
ngx_int_t
ngx_write_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size,
    ngx_log_t *log)
{
#if (NGX_HAVE_MSGHDR_MSG_CONTROL)
    /*
     * We have to use ngx_memcpy() instead of simple
     *   *(int *) CMSG_DATA(&cmsg.cm) = ch->fd;
     * because some gcc 4.4 with -O2/3/s optimization issues the warning:
     *   dereferencing type-punned pointer will break strict-aliasing rules
     *
     * Fortunately, gcc with -O1 compiles this ngx_memcpy()
     * in the same simple assignment as in the code above
     */
#else

#endif

  n = sendmsg(s, &msg, 0);
}
{% endhighlight %}

这里使用sendmsg()函数向channel发送命令。在ngx_auto_config.h头文件中，我们有如下定义：
<pre>
#ifndef NGX_HAVE_MSGHDR_MSG_CONTROL
#define NGX_HAVE_MSGHDR_MSG_CONTROL  1
#endif
</pre>
因此，这里采用我们常用的msghdr.cmsghdr的方式来传递文件描述符。关于```strict-aliasing```,请参看：[C/C++ Strict Alias 小记](http://blog.csdn.net/dbzhang800/article/details/6720141)

另外这里说明一下，旧的unix系统使用的是```msg_accrights```域来传递文件描述符，因此在不支持```NGX_HAVE_MSGHDR_MSG_CONTROL```时采用如下方式：
{% highlight string %}
if (ch->fd == -1) {
    msg.msg_accrights = NULL;
    msg.msg_accrightslen = 0;

} else {
    msg.msg_accrights = (caddr_t) &ch->fd;
    msg.msg_accrightslen = sizeof(int);
}
{% endhighlight %}


### 2.2 从channel读取命令

代码整体结构如下：
{% highlight string %}
ngx_int_t
ngx_read_channel(ngx_socket_t s, ngx_channel_t *ch, size_t size, ngx_log_t *log)
{
    ...

    n = recvmsg(s, &msg, 0);

	....
}
{% endhighlight %}

这里调用recvmsg()来接收channel消息。这里我们不需要接收地址信息，因此可以：
<pre>
msghdr.msg_name = NULL;
msghdr.msg_namelen = 0;
</pre>
这里注意对recvmsg()返回值n为 -1 时候的处理。


### 2.3 添加channel事件到监听队列

代码整体结构如下：
{% highlight string %}
ngx_int_t
ngx_add_channel_event(ngx_cycle_t *cycle, ngx_fd_t fd, ngx_int_t event,
    ngx_event_handler_pt handler)
{
	...
	
	ngx_connection_t  *c;
	c = ngx_get_connection(fd, cycle->log);
	
	...
	
	if (ngx_add_conn && (ngx_event_flags & NGX_USE_EPOLL_EVENT) == 0) {
	    if (ngx_add_conn(c) == NGX_ERROR) {
	        ngx_free_connection(c);
	        return NGX_ERROR;
	    }
	
	} else {
	    if (ngx_add_event(ev, event, 0) == NGX_ERROR) {
	        ngx_free_connection(c);
	        return NGX_ERROR;
	    }
	}

}
{% endhighlight %}

这里首先从ngx_cycle_t中获取到一个connection,然后将其添加到事件监听队列中。这里对channel的一个fd不会同时进行读写操作(channel[0]用于写，channel[1]用于读），因此这里会进行二选一操作：
<pre>
ev = (event == NGX_READ_EVENT) ? rev : wev;
</pre>


### 2.4 关闭channel
代码如下：
{% highlight string %}
void
ngx_close_channel(ngx_fd_t *fd, ngx_log_t *log)
{
    if (close(fd[0]) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno, "close() channel failed");
    }

    if (close(fd[1]) == -1) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno, "close() channel failed");
    }
}
{% endhighlight %}

关于关闭channel,这里有一个情况说明一下： 当前master通过ngx_spawn_process()函数创建出一对匿名channel,然后通过fork()自动传递给了子进程，在子进程的初始化中将channel[0]关闭，用channel[1]读取来自父进程中通过channel[0]发送过来的消息，但是在父进程中却没有关闭channel[1]。关于这个问题的解释，请参看 [nginx-ticket 1426](https://trac.nginx.org/nginx/ticket/1426)



<br />
<br />

**[参看]:**

1. [nginx进程间的通信](http://blog.csdn.net/walkerkalr/article/details/38237147)

2. [Nginx源码分析-master和worker进程间的通信](http://blog.csdn.net/marcky/article/details/6014733)

3. [nginx源码分析--高性能服务器开发 常见进程模型](http://blog.csdn.net/yusiguyuan/article/details/40924757)

4. [ngx_worker_process_cycle子进程执行](http://blog.csdn.net/lengzijian/article/details/7591025)

<br />
<br />
<br />

