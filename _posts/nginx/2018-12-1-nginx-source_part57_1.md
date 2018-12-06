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

## 1. 相关宏定义
{% highlight string %}
#include <ngx_core.h>


#ifndef _NGX_RESOLVER_H_INCLUDED_
#define _NGX_RESOLVER_H_INCLUDED_


//由域名转换成IP地址
#define NGX_RESOLVE_A         1

//查询规范名称
#define NGX_RESOLVE_CNAME     5

//将IP地址转换成域名
#define NGX_RESOLVE_PTR       12
#define NGX_RESOLVE_MX        15
#define NGX_RESOLVE_TXT       16
#if (NGX_HAVE_INET6)
#define NGX_RESOLVE_AAAA      28
#endif

//根据服务名查询目标服务器的域名
#define NGX_RESOLVE_SRV       33
#define NGX_RESOLVE_DNAME     39

#define NGX_RESOLVE_FORMERR   1
#define NGX_RESOLVE_SERVFAIL  2


// 表示并未指定domain域，一般表示不需要进行DNS查询
#define NGX_RESOLVE_NXDOMAIN  3
#define NGX_RESOLVE_NOTIMP    4
#define NGX_RESOLVE_REFUSED   5
#define NGX_RESOLVE_TIMEDOUT  NGX_ETIMEDOUT


#define NGX_NO_RESOLVER       (void *) -1

#define NGX_RESOLVER_MAX_RECURSION    50
{% endhighlight %}


## 2. ngx_resolver_connection_t数据结构
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


## 2. ngx_resolver_srv_t数据结构
{% highlight string %}
typedef struct {
    ngx_str_t                 name;
    u_short                   priority;
    u_short                   weight;
    u_short                   port;
} ngx_resolver_srv_t;
{% endhighlight %}
在进行```NGX_RESOLVE_SRV```查询时，返回目标服务器的相关信息。下面简要介绍一下各字段的含义(rfc2782)：

* name: 目标服务器的域名信息

* priority: 优先级。一般值越低，优先级越高

* weight: 目标服务器所占用的权重

* port: 该不服务在目标服务器上所占用的端口


## 3. ngx_resolver_srv_name_t数据结构
{% highlight string %}
typedef struct {
    ngx_str_t                 name;
    u_short                   priority;
    u_short                   weight;
    u_short                   port;

    ngx_resolver_ctx_t       *ctx;

    ngx_uint_t                naddrs;
    ngx_addr_t               *addrs;
} ngx_resolver_srv_name_t;
{% endhighlight %}
本数据结构用于表示某一个域名(name)下所指定的服务的IP地址，用于服务查询。下面我们简要介绍一下各字段的含义：

* name: 服务对应的域名

* priority: 服务在```name```域名下的优先级

* weight： 权重

* port: 对应的端口

* ctx: 所关联的ngx_resolver_ctx_t上下文

* naddrs: ip地址个数

* addrs: 所返回的IP地址

## 4. ngx_resolver_node_t数据结构
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
        in_addr_t             addr;		//表示单个IP地址
        in_addr_t            *addrs;	//表示IP地址的数组
        u_char               *cname;	//用于表示查询返回的规范名称（canonical name)
        ngx_resolver_srv_t   *srvs;		//用于SRV查询时，返回的目标服务器的相关信息
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

* node: 作为红黑树的一个节点存在，将```ngx_resolver_node_t```数据结构作为红黑树的一个节点来进行管理

* queue: 作为一个辅助元素，用于实现ngx_resolver_t中的超时队列。

* name: 当执行的是DNS ```A```类型查询时，本字段保存的是需要解析的域名； 当执行的是DNS ```PTR```类型的查询时，本字段保存的是逆解析后得到的域名

* nlen: 用于指明```name```的长度

* qlen: 用于指明```query```的长度

* query: 用于存放当前DNS的查询请求报文

* query6: 用于存放当前DNS的查询请求报文(适用于IPv6, 当前我们并不支持```NGX_HAVE_INET6```宏定义)

* u: 用于保存当前DNS查询到的地址信息(IPv4)。

* naddrs: 用于指明当前解析到的```u.addrs```的个数。 -1表示未开始解析，或解析出错。

* nsrvs: 用于指明当前解析大的```u.srvs```的个数。 -1表示未开始解析，或解析出错

* cnlen: 用于指明u.cname的长度

* u6: 用于保存当前DNS查询到的地址信息(IPv6)

* naddrs6:当前DNS解析到的IPv6地址信息的个数。 -1表示未开始解析，或解析出错

* expire: 用于指示当前节点的过期```时刻```，主要是用于控制超时队列使用，适时的淘汰```ngx_resolver_t```中```name_rbtree```、```srv_rbtree```或```addr_rbtree```中的节点，对应的超时队列分别是ngx_resolver_t中的name_expire_queue、srv_expire_queue或addr_expire_queue;

* valid： 用于指示当前节点的有效期```时刻```。注意： 本字段含义上与expire相似，但expire主要用于控制超时队列。

* ttl: 用于保存DNS返回的TTL时间

* last_connection: 用于表示适用resolver连接数组中的哪一个连接（这里采用round-robin算法）

* waiting: 用于指示当前节点所对应的ngx_resolver_ctx_t类型的等待链表

## 5. ngx_resolver_t数据结构
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

* event: 与当前resolver关联的event， 一般作为定时事件来执行resend_queue中相应的任务

* log: 与当前resolver关联的日志对象

* connections: 是一个```ngx_resolver_connection_t```类型的数组，数组的每一个元素代表与DNS的一条连接。这里涉及到两个方面，首先一个resolver可能对应多个不同域名(或IP）的DNS， 其次即使是同一个DNS域名，也可能对应多个IP地址, 因此这里需要用数组来保存。

* last_connection：采用round-robin算法时，用于控制采用连接数组中的哪一个连接

* name_rbtree: 用于保存从DNS查询到的```域名到IP地址的映射```的红黑树，

* name_sentinel: name_rbtree红黑树的叶子终节点

* srv_rbtree: 用于保存从DNS查询到的```服务名到IP地址的映射```的红黑树（实际上，当前程序中并未有任何地方使用到此功能）

* srv_sentinel: srv_rbtree红黑树的叶子终节点

* addr_rbtree: 用于保存从DNS逆查询得到的```IP地址到域名的映射```的红黑树。红黑树中的value是```ngx_resolver_node_t```结构

* addr_sentinel: addr_rbtree红黑树的叶子终节点

* name_resend_queue：一般作为name查询服务类型ngx_resolver_node_t的waiting context所关联事件的```resend```队列

* srv_resend_queue： 一般作为srv查询服务类型ngx_resolver_node_t的waiting context所关联事件的```resend```队列

* addr_resend_queue： 一般作为addr查询服务类型ngx_resolver_node_t的waiting context所关联事件的```resend```队列


* name_expire_queue: 用于控制name_rbtree中节点超时的队列

* srv_expire_queue： 用于控制srv_rbtree中节点超时的队列

* addr_expire_queue； 用于控制addr_rbtree中节点超时的队列

* IPv6相关
<pre>
#if (NGX_HAVE_INET6)
    ngx_uint_t                ipv6;                 /* unsigned  ipv6:1; */
    ngx_rbtree_t              addr6_rbtree;
    ngx_rbtree_node_t         addr6_sentinel;
    ngx_queue_t               addr6_resend_queue;
    ngx_queue_t               addr6_expire_queue;
#endif
</pre>
在当前，我们暂未支持```NGX_HAVE_INET6```这一宏定义。这里除了协议版本与ipv4不同外，字段含义与ipv4中对应字段均相同。

* resend_timeout: 用于指示请求重发的超时时间


* tcp_timeout: 代表与DNS服务连接的TCP超时时间

* expire: 用于指示当前resolver缓存DNS解析结果的时间，这样就可以不需要每一次请求都去请求DNS

* valid: 用于指明resolver的有效事件

<pre>
注意： 上面介绍的DNS逆地址解析，一般只会在邮件模块用到
</pre>


## 6. ngx_resolver_ctx_t数据结构
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

* resolver: 本上下文对象所关联的resolver（注意： 本身ngx_resolver_t并不直接与ctx关联，而是通过ngx_resolver_t中相应的节点来关联的）

* node： 本上下文所关联的ngx_resolver_node_t数据结构。

* state: 用于指示当前上下文对DNS域名解析，或DNS逆解析的状态

* name: DNS ```A```类型解析时保存的是将要解析的域名； DNS ```PTR```类型解析时，保存的是IP地址逆解析到的域名

* service： 用于保存要解析的服务名称

* valid: 当前context的有效期时间

* addr: 当前需要进行DNS逆查询的IP地址（即IP地址到域名的映射)

* count: 用于保存当前未处理完的srvs的个数。（srv表示将```服务名转换为IP地址```，有时可能不能直接转换，会先返回规范化的名称，然后我们需要再根据返回的规范化名称再去查询DNS，以返回IP地址。此处用于指明当前我们未处理的规范化名称个数)

* nsrvs: 当前ctx所要解析的```srvs```的个数

* srvs: 当前ctx所要解析的```规范化服务名称```(canonical name)

* handler: 本context对象绑定的回调函数

* data: 一般用于保存handler()回调函数的参数

* timeout: 当前context的超时时间，主要用于event事件的超时管理

* quick： 一般情况下，当我们并不需要调用DNS服务器进行解析时会将本字段设置为1，这时直接调用ngx_resolver_ctx_t的handler回调函数即可。

* recursion: 因为DNS解析时可能会产生递归，这里用于记录递归的次数。超过了一定的次数之后，一般可以认为出现了问题，此时不应该继续查询DNS让递归一直进行下去了


下面我们给出nginx resolver各数据结构的一个整体图景：

![ngx-resolver-pic](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_resolver_pic.jpg)










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

