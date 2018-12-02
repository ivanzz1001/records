---
layout: post
title: core/ngx_resolver.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本章我们主要介绍一下ngx_resolver.h头文件，其主要是实现对nginx中涉及到的域名的解析。


<!-- more -->



## 1. ngx_resolver_t数据结构
{% highlight string %}
struct ngx_resolver_s {
    /* has to be pointer because of "incomplete type" */
    ngx_event_t              *event;
    void                     *dummy;
    ngx_log_t                *log;

    /* event ident must be after 3 pointers as in ngx_connection_t */
    ngx_int_t                 ident;

    /* simple round robin DNS peers balancer */
    ngx_array_t               connections;
    ngx_uint_t                last_connection;

    ngx_rbtree_t              name_rbtree;
    ngx_rbtree_node_t         name_sentinel;

    ngx_rbtree_t              srv_rbtree;
    ngx_rbtree_node_t         srv_sentinel;

    ngx_rbtree_t              addr_rbtree;
    ngx_rbtree_node_t         addr_sentinel;

    ngx_queue_t               name_resend_queue;
    ngx_queue_t               srv_resend_queue;
    ngx_queue_t               addr_resend_queue;

    ngx_queue_t               name_expire_queue;
    ngx_queue_t               srv_expire_queue;
    ngx_queue_t               addr_expire_queue;

#if (NGX_HAVE_INET6)
    ngx_uint_t                ipv6;                 /* unsigned  ipv6:1; */
    ngx_rbtree_t              addr6_rbtree;
    ngx_rbtree_node_t         addr6_sentinel;
    ngx_queue_t               addr6_resend_queue;
    ngx_queue_t               addr6_expire_queue;
#endif

    time_t                    resend_timeout;
    time_t                    tcp_timeout;
    time_t                    expire;
    time_t                    valid;

    ngx_uint_t                log_level;
};
{% endhighlight %}




<br />
<br />

**[参看]**

1. [Nginx的DNS解析过程](http://www.360doc.com/content/14/0102/10/15064667_341883468.shtml)

2. [nginx关于域名解析的源码分析](https://blog.csdn.net/wan706364166/article/details/8525192)

3. [nginx resolver](http://nginx.org/en/docs/http/ngx_http_core_module.html#resolver)


4. [Nginx displayed by LXR](http://lxr.nginx.org/ident?_i=ngx_resolve_start)
<br />
<br />
<br />

