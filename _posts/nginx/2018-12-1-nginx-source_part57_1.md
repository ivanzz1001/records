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

## 1. ngx_resolver_connection_t数据结构
{% highlight string %}
typedef struct {
    ngx_connection_t         *udp;
    ngx_connection_t         *tcp;
    struct sockaddr          *sockaddr;
    socklen_t                 socklen;
    ngx_str_t                 server;
    ngx_log_t                 log;
    ngx_buf_t                *read_buf;
    ngx_buf_t                *write_buf;
    ngx_resolver_t           *resolver;
} ngx_resolver_connection_t;
{% endhighlight %}
本数据结构用于表示resolver实例与DNS服务器的一条连接。下面简单介绍一下各字段的含义：

* udp: 代表resolver与DNS服务器的udp连接

* tcp: 代表resolver与DNS服务器的tcp连接

* sockaddr: 存放DNS服务器的IP地址

* socklen: DNS服务器IP地址的长度

* server: DNS服务器IP地址的字符串表示形式

* log: 与当前connection关联的日志对象

* read_buf: 当前connection的读取缓冲（用于TCP）

* write_buf: 当前connection的写缓冲（用于TCP）

* resolver: 当前connection对象所属的resolver





## 2. ngx_resolver_node_t数据结构
{% highlight string %}
typedef struct {
    ngx_rbtree_node_t         node;
    ngx_queue_t               queue;

    /* PTR: resolved name, A: name to resolve */
    u_char                   *name;

#if (NGX_HAVE_INET6)
    /* PTR: IPv6 address to resolve (IPv4 address is in rbtree node key) */
    struct in6_addr           addr6;
#endif

    u_short                   nlen;
    u_short                   qlen;

    u_char                   *query;
#if (NGX_HAVE_INET6)
    u_char                   *query6;
#endif

    union {
        in_addr_t             addr;
        in_addr_t            *addrs;
        u_char               *cname;
        ngx_resolver_srv_t   *srvs;
    } u;

    u_char                    code;
    u_short                   naddrs;
    u_short                   nsrvs;
    u_short                   cnlen;

#if (NGX_HAVE_INET6)
    union {
        struct in6_addr       addr6;
        struct in6_addr      *addrs6;
    } u6;

    u_short                   naddrs6;
#endif

    time_t                    expire;
    time_t                    valid;
    uint32_t                  ttl;

    unsigned                  tcp:1;
#if (NGX_HAVE_INET6)
    unsigned                  tcp6:1;
#endif

    ngx_uint_t                last_connection;

    ngx_resolver_ctx_t       *waiting;
} ngx_resolver_node_t;
{% endhighlight %}
下面简要介绍一下各字段的含义：

* query: 用于存放当前DNS的查询请求报文



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
ngx_resolver_t数据结构用于表示nginx中的一个DNS解析器。下面简单介绍一下各字段的含义：

* event: 与当前resolver关联的event

* log: 与当前resolver关联的日志对象

* connections: 是一个```ngx_resolver_connection_t```类型的数组，数组的每一个元素代表与DNS的一条连接。这里涉及到两个方面，首先一个resolver可能对应多个不同域名(或IP）的DNS， 其次即使是同一个DNS域名，也可能对应多个IP地址, 因此这里需要用数组来保存。

* name_rbtree: 用于保存从DNS查询到的```域名到IP地址的映射```的红黑树，

* name_sentinel: name_rbtree红黑树的叶子终节点

* addr_rbtree: 用于保存从DNS逆查询得到的```IP地址域名的映射```的红黑树。红黑树中的value是```ngx_resolver_node_t```结构

* addr_sentinel: addr_rbtree红黑树的叶子终节点

* tcp_timeout: 代表与DNS服务连接的TCP超时时间

* expire: 用于指示当前resolver缓存DNS解析结果的时间，这样就可以不需要每一次请求都去请求DNS

* valid: 


## 4. ngx_resolver_ctx_t数据结构
{% highlight string %}
typedef struct ngx_resolver_ctx_s  ngx_resolver_ctx_t;
struct ngx_resolver_ctx_s {
    ngx_resolver_ctx_t       *next;
    ngx_resolver_t           *resolver;
    ngx_resolver_node_t      *node;

    /* event ident must be after 3 pointers as in ngx_connection_t */
    ngx_int_t                 ident;

    ngx_int_t                 state;
    ngx_str_t                 name;
    ngx_str_t                 service;

    time_t                    valid;
    ngx_uint_t                naddrs;
    ngx_resolver_addr_t      *addrs;
    ngx_resolver_addr_t       addr;
    struct sockaddr_in        sin;

    ngx_uint_t                count;
    ngx_uint_t                nsrvs;
    ngx_resolver_srv_name_t  *srvs;

    ngx_resolver_handler_pt   handler;
    void                     *data;
    ngx_msec_t                timeout;

    ngx_uint_t                quick;  /* unsigned  quick:1; */
    ngx_uint_t                recursion;
    ngx_event_t              *event;
};
{% endhighlight %}
本数据结构用于表示resolver对象实例所关联的上下文对象。下面简单介绍一下各字段的含义:

* next: 用于指示ngx_resolver_ctx_t链表中的下一个上下文对象；

* resolver: 本上下文对象所关联的resolver

* addr: 当前需要进行DNS逆查询的IP地址



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

