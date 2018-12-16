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
```error_log```与```access_log```指令都支持将日志写到syslog中。如下的一些参数用于配置nginx的syslog:

* server=address: 用于指定syslog服务器的地址。该地址可以是一个```域名```或```IP地址```（端口号可选），或者是通过```unix:```前缀指定的一个unix域socket路径。假如未指定端口的话，则默认情况下会使用514 UDP端口。另外，假如一个域名对应多个IP地址的话，将会使用解析到的第一个IP。

* facility=string: 用于指明syslog消息的facility（请参看[RFC3164](https://tools.ietf.org/html/rfc3164#section-4.1.1))，可选的值有
<pre>
“kern”, “user”, “mail”, “daemon”, 
“auth”, “intern”, “lpr”, “news”, 
“uucp”, “clock”, “authpriv”, “ftp”,
“ntp”, “audit”, “alert”, “cron”,
 “local0”..“local7”
</pre>
默认值为```local7```。

* severity=string: 用于为```access_log```指令指明日志消息的优先级（
* facility=string: 用于指明syslog消息的facility（请参看[RFC3164](https://tools.ietf.org/html/rfc3164#section-4.1.1)。其实与```error_log```指令的第二个参数的取值范围是一样的。默认值是```info```。
<pre>
注意：当在error_log指令中使用syslog时，由于错误消息的级别是由nginx本身所指定的，因此
在这种情况下，会忽略syslog中定义的级别
</pre>

* tag=string: 用于为syslog消息指定tag。默认值为```nginx```

* nohostname: 用于禁止添加hostname字段到syslog消息的header部分

下面给出几个syslog配置的示例：
<pre>
error_log syslog:server=192.168.1.1 debug;

access_log syslog:server=unix:/var/log/nginx.sock,nohostname;
access_log syslog:server=[2001:db8::1]:12345,facility=local7,tag=nginx,severity=info combined;
</pre>

上面注意到error_log使用的是其自身的级别，并不会采用syslog中的severity。


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

