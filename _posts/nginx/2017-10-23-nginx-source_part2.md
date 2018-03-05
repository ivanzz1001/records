---
layout: post
title: os/unix/ngx_os.h源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


os/unix文件夹下源代码与操作系统关系紧密，与具体的业务及上层依赖也较少，因此我们这里首先来分析这一部分。ngx_os.h头文件是作为此部分的一个接口文件，我们会在本文进行分析。


<!-- more -->

## 1. os/unix/ngx_linux.h头文件

头文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LINUX_H_INCLUDED_
#define _NGX_LINUX_H_INCLUDED_


ngx_chain_t *ngx_linux_sendfile_chain(ngx_connection_t *c, ngx_chain_t *in,
    off_t limit);


#endif /* _NGX_LINUX_H_INCLUDED_ */

{% endhighlight %}

ngx_linux.h头文件会包含在ngx_os.h头文件中，因此我们这里做简单介绍。

## 2. os/unix/ngx_os.h头文件

头文件内容如下：

{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_OS_H_INCLUDED_
#define _NGX_OS_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_IO_SENDFILE    1


typedef ssize_t (*ngx_recv_pt)(ngx_connection_t *c, u_char *buf, size_t size);
typedef ssize_t (*ngx_recv_chain_pt)(ngx_connection_t *c, ngx_chain_t *in,
    off_t limit);
typedef ssize_t (*ngx_send_pt)(ngx_connection_t *c, u_char *buf, size_t size);
typedef ngx_chain_t *(*ngx_send_chain_pt)(ngx_connection_t *c, ngx_chain_t *in,
    off_t limit);

typedef struct {
    ngx_recv_pt        recv;
    ngx_recv_chain_pt  recv_chain;
    ngx_recv_pt        udp_recv;
    ngx_send_pt        send;
    ngx_send_pt        udp_send;
    ngx_send_chain_pt  send_chain;
    ngx_uint_t         flags;
} ngx_os_io_t;


ngx_int_t ngx_os_init(ngx_log_t *log);
void ngx_os_status(ngx_log_t *log);
ngx_int_t ngx_os_specific_init(ngx_log_t *log);
void ngx_os_specific_status(ngx_log_t *log);
ngx_int_t ngx_daemon(ngx_log_t *log);
ngx_int_t ngx_os_signal_process(ngx_cycle_t *cycle, char *sig, ngx_pid_t pid);


ssize_t ngx_unix_recv(ngx_connection_t *c, u_char *buf, size_t size);
ssize_t ngx_readv_chain(ngx_connection_t *c, ngx_chain_t *entry, off_t limit);
ssize_t ngx_udp_unix_recv(ngx_connection_t *c, u_char *buf, size_t size);
ssize_t ngx_unix_send(ngx_connection_t *c, u_char *buf, size_t size);
ngx_chain_t *ngx_writev_chain(ngx_connection_t *c, ngx_chain_t *in,
    off_t limit);
ssize_t ngx_udp_unix_send(ngx_connection_t *c, u_char *buf, size_t size);


#if (IOV_MAX > 64)
#define NGX_IOVS_PREALLOCATE  64
#else
#define NGX_IOVS_PREALLOCATE  IOV_MAX
#endif


typedef struct {
    struct iovec  *iovs;
    ngx_uint_t     count;
    size_t         size;
    ngx_uint_t     nalloc;
} ngx_iovec_t;

ngx_chain_t *ngx_output_chain_to_iovec(ngx_iovec_t *vec, ngx_chain_t *in,
    size_t limit, ngx_log_t *log);


ssize_t ngx_writev(ngx_connection_t *c, ngx_iovec_t *vec);


extern ngx_os_io_t  ngx_os_io;
extern ngx_int_t    ngx_ncpu;
extern ngx_int_t    ngx_max_sockets;
extern ngx_uint_t   ngx_inherited_nonblocking;
extern ngx_uint_t   ngx_tcp_nodelay_and_tcp_nopush;


#if (NGX_FREEBSD)
#include <ngx_freebsd.h>


#elif (NGX_LINUX)
#include <ngx_linux.h>


#elif (NGX_SOLARIS)
#include <ngx_solaris.h>


#elif (NGX_DARWIN)
#include <ngx_darwin.h>
#endif


#endif /* _NGX_OS_H_INCLUDED_ */

{% endhighlight %}


文件主要定义了如下几个方面：

* 操作系统初始化及状态相关接口

* 进程处理相关接口

* 信号处理相关接口

* socket发送接收相关接口


<br />
<br />
<br />

