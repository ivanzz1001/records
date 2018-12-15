---
layout: post
title: core/ngx_syslog.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们介绍一下nginx中syslog的实现。nginx中的syslog并未直接采用类Unix系统中的syslog函数来实现，而是自己封装了一套相似的API来实现对应的功能。关于Linux syslog请参看附录。



<!-- more -->

## 1. ngx_syslog_peer_t结构体
{% highlight string %}

/*
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SYSLOG_H_INCLUDED_
#define _NGX_SYSLOG_H_INCLUDED_



typedef struct {
    ngx_pool_t       *pool;
    ngx_uint_t        facility;
    ngx_uint_t        severity;
    ngx_str_t         tag;

    ngx_addr_t        server;
    ngx_connection_t  conn;
    unsigned          busy:1;
    unsigned          nohostname:1;
} ngx_syslog_peer_t;
{% endhighlight %}
本结构体用于表示与远程syslog服务器的一个连接。下面简要介绍一下各字段的含义：

* pool: 所关联的内存池结构

* facility: 与标准的syslog相似，但是这里Nginx定义了一套自己的facility实现
<pre>
"kern",	"user", "mail", "daemon", 
"auth", "intern", "lpr","news",
"uucp","clock", "authpriv", "ftp", 
"ntp","audit","alert", "cron", 
"local0","local1", "local2","local3", 
"local4", "local5", "local6","local7",
<pre>

* severity: 用于指示日志的优先级

* tag: 对应于标准syslog中的TAG标识

* server: 远程syslog接收服务器的地址信息

* conn: 与远程syslog接收服务器的连接

* busy: 标志当前是否正处于写日志的状态

* nohostname：标志是否在header部分加上hostname信息


## 2. 相关函数声明
{% highlight string %}
//用于解析配置文件中关于syslog的相关配置
char *ngx_syslog_process_conf(ngx_conf_t *cf, ngx_syslog_peer_t *peer);


//向日志中添加HEADER信息
u_char *ngx_syslog_add_header(ngx_syslog_peer_t *peer, u_char *buf);


//将buf中的内容写到syslog
void ngx_syslog_writer(ngx_log_t *log, ngx_uint_t level, u_char *buf,
    size_t len);

//将buf中的内容发送到远程syslog服务器
ssize_t ngx_syslog_send(ngx_syslog_peer_t *peer, u_char *buf, size_t len);
#endif /* _NGX_SYSLOG_H_INCLUDED_ */
{% endhighlight %}


## 3. syslog配置文件



<br />
<br />

**[参看]**

1. [Logging to syslog](http://nginx.org/en/docs/syslog.html)


2. [syslog日志服务](https://blog.csdn.net/llzk_/article/details/69945366)

3. [syslog](https://baike.baidu.com/item/syslog/2802901)

4. [syslog 详解](https://blog.csdn.net/zhezhebie/article/details/75222667)



<br />
<br />
<br />

