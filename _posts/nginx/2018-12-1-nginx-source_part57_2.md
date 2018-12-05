---
layout: post
title: core/ngx_resolver.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本章我们主要介绍一下ngx_resolver.c源文件，其主要是实现对nginx中涉及到的域名的解析。


<!-- more -->

## 1. ngx_resolver_hdr_t数据结构
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define NGX_RESOLVER_UDP_SIZE   4096

#define NGX_RESOLVER_TCP_RSIZE  (2 + 65535)
#define NGX_RESOLVER_TCP_WSIZE  8192


typedef struct {
	//identifier 字段
    u_char  ident_hi;
    u_char  ident_lo;

	
    u_char  flags_hi;
    u_char  flags_lo;

	//查询问题的个数
    u_char  nqs_hi;
    u_char  nqs_lo;

	//'answer section' 段的应答资源记录数
    u_char  nan_hi;
    u_char  nan_lo;

	// 'authority records section' 段授权资源记录数
    u_char  nns_hi;
    u_char  nns_lo;

	//'additional records section'段的附加资源记录数
    u_char  nar_hi;
    u_char  nar_lo;
} ngx_resolver_hdr_t;
{% endhighlight %}
```ngx_resolver_hdr_t```定义了DNS报文结构的头部。关于各字段的含义，请参看[DNS协议详解及报文格式分析](https://blog.csdn.net/tianxuhong/article/details/74922454)


## 2. ngx_resolver_qs_t数据结构
{% highlight string %}
typedef struct {
    u_char  type_hi;
    u_char  type_lo;
    u_char  class_hi;
    u_char  class_lo;
} ngx_resolver_qs_t;
{% endhighlight %}
此结构定义了DNS查询报文中的```Queries区域```。其中```type```用于指明查询类型； ```class```用于指明查询类。

## 3. ngx_resolver_an_t数据结构
{% highlight string %}
typedef struct {
    u_char  type_hi;
    u_char  type_lo;
    u_char  class_hi;
    u_char  class_lo;
    u_char  ttl[4];
    u_char  len_hi;
    u_char  len_lo;
} ngx_resolver_an_t;
{% endhighlight %}
此结构定义了DNS应答报文中的```Answers区域```



## 4. 相关静态函数声明
{% highlight string %}
#define ngx_resolver_node(n)                                                 \
    (ngx_resolver_node_t *)                                                  \
        ((u_char *) (n) - offsetof(ngx_resolver_node_t, node))


ngx_int_t ngx_udp_connect(ngx_resolver_connection_t *rec);
ngx_int_t ngx_tcp_connect(ngx_resolver_connection_t *rec);


static void ngx_resolver_cleanup(void *data);
static void ngx_resolver_cleanup_tree(ngx_resolver_t *r, ngx_rbtree_t *tree);
static ngx_int_t ngx_resolve_name_locked(ngx_resolver_t *r,
    ngx_resolver_ctx_t *ctx, ngx_str_t *name);
static void ngx_resolver_expire(ngx_resolver_t *r, ngx_rbtree_t *tree,
    ngx_queue_t *queue);
static ngx_int_t ngx_resolver_send_query(ngx_resolver_t *r,
    ngx_resolver_node_t *rn);
static ngx_int_t ngx_resolver_send_udp_query(ngx_resolver_t *r,
    ngx_resolver_connection_t *rec, u_char *query, u_short qlen);
static ngx_int_t ngx_resolver_send_tcp_query(ngx_resolver_t *r,
    ngx_resolver_connection_t *rec, u_char *query, u_short qlen);
static ngx_int_t ngx_resolver_create_name_query(ngx_resolver_t *r,
    ngx_resolver_node_t *rn, ngx_str_t *name);

//创建DNS服务查询报文
static ngx_int_t ngx_resolver_create_srv_query(ngx_resolver_t *r,
    ngx_resolver_node_t *rn, ngx_str_t *name);
static ngx_int_t ngx_resolver_create_addr_query(ngx_resolver_t *r,
    ngx_resolver_node_t *rn, ngx_resolver_addr_t *addr);
static void ngx_resolver_resend_handler(ngx_event_t *ev);
static time_t ngx_resolver_resend(ngx_resolver_t *r, ngx_rbtree_t *tree,
    ngx_queue_t *queue);
static ngx_uint_t ngx_resolver_resend_empty(ngx_resolver_t *r);
static void ngx_resolver_udp_read(ngx_event_t *rev);
static void ngx_resolver_tcp_write(ngx_event_t *wev);
static void ngx_resolver_tcp_read(ngx_event_t *rev);
static void ngx_resolver_process_response(ngx_resolver_t *r, u_char *buf,
    size_t n, ngx_uint_t tcp);
static void ngx_resolver_process_a(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t ident, ngx_uint_t code, ngx_uint_t qtype,
    ngx_uint_t nan, ngx_uint_t trunc, ngx_uint_t ans);
static void ngx_resolver_process_srv(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t ident, ngx_uint_t code, ngx_uint_t nan,
    ngx_uint_t trunc, ngx_uint_t ans);
static void ngx_resolver_process_ptr(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t ident, ngx_uint_t code, ngx_uint_t nan);
static ngx_resolver_node_t *ngx_resolver_lookup_name(ngx_resolver_t *r,
    ngx_str_t *name, uint32_t hash);
static ngx_resolver_node_t *ngx_resolver_lookup_srv(ngx_resolver_t *r,
    ngx_str_t *name, uint32_t hash);
static ngx_resolver_node_t *ngx_resolver_lookup_addr(ngx_resolver_t *r,
    in_addr_t addr);
static void ngx_resolver_rbtree_insert_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel);
static ngx_int_t ngx_resolver_copy(ngx_resolver_t *r, ngx_str_t *name,
    u_char *buf, u_char *src, u_char *last);
static void ngx_resolver_timeout_handler(ngx_event_t *ev);
static void ngx_resolver_free_node(ngx_resolver_t *r, ngx_resolver_node_t *rn);
static void *ngx_resolver_alloc(ngx_resolver_t *r, size_t size);
static void *ngx_resolver_calloc(ngx_resolver_t *r, size_t size);
static void ngx_resolver_free(ngx_resolver_t *r, void *p);
static void ngx_resolver_free_locked(ngx_resolver_t *r, void *p);
static void *ngx_resolver_dup(ngx_resolver_t *r, void *src, size_t size);
static ngx_resolver_addr_t *ngx_resolver_export(ngx_resolver_t *r,
    ngx_resolver_node_t *rn, ngx_uint_t rotate);
static void ngx_resolver_report_srv(ngx_resolver_t *r, ngx_resolver_ctx_t *ctx);
static u_char *ngx_resolver_log_error(ngx_log_t *log, u_char *buf, size_t len);
static void ngx_resolver_resolve_srv_names(ngx_resolver_ctx_t *ctx,
    ngx_resolver_node_t *rn);
static void ngx_resolver_srv_names_handler(ngx_resolver_ctx_t *ctx);
static ngx_int_t ngx_resolver_cmp_srvs(const void *one, const void *two);

#if (NGX_HAVE_INET6)
static void ngx_resolver_rbtree_insert_addr6_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel);
static ngx_resolver_node_t *ngx_resolver_lookup_addr6(ngx_resolver_t *r,
    struct in6_addr *addr, uint32_t hash);
#endif
{% endhighlight %}

## 5. 函数ngx_resolver_create()
{% highlight string %}
ngx_resolver_t *
ngx_resolver_create(ngx_conf_t *cf, ngx_str_t *names, ngx_uint_t n)
{
    ngx_str_t                   s;
    ngx_url_t                   u;
    ngx_uint_t                  i, j;
    ngx_resolver_t             *r;
    ngx_pool_cleanup_t         *cln;
    ngx_resolver_connection_t  *rec;

    cln = ngx_pool_cleanup_add(cf->pool, 0);
    if (cln == NULL) {
        return NULL;
    }

    cln->handler = ngx_resolver_cleanup;

    r = ngx_calloc(sizeof(ngx_resolver_t), cf->log);
    if (r == NULL) {
        return NULL;
    }

    cln->data = r;

    r->event = ngx_calloc(sizeof(ngx_event_t), cf->log);
    if (r->event == NULL) {
        return NULL;
    }

    ngx_rbtree_init(&r->name_rbtree, &r->name_sentinel,
                    ngx_resolver_rbtree_insert_value);

    ngx_rbtree_init(&r->srv_rbtree, &r->srv_sentinel,
                    ngx_resolver_rbtree_insert_value);

    ngx_rbtree_init(&r->addr_rbtree, &r->addr_sentinel,
                    ngx_rbtree_insert_value);

    ngx_queue_init(&r->name_resend_queue);
    ngx_queue_init(&r->srv_resend_queue);
    ngx_queue_init(&r->addr_resend_queue);

    ngx_queue_init(&r->name_expire_queue);
    ngx_queue_init(&r->srv_expire_queue);
    ngx_queue_init(&r->addr_expire_queue);

#if (NGX_HAVE_INET6)
    r->ipv6 = 1;

    ngx_rbtree_init(&r->addr6_rbtree, &r->addr6_sentinel,
                    ngx_resolver_rbtree_insert_addr6_value);

    ngx_queue_init(&r->addr6_resend_queue);

    ngx_queue_init(&r->addr6_expire_queue);
#endif

    r->event->handler = ngx_resolver_resend_handler;
    r->event->data = r;
    r->event->log = &cf->cycle->new_log;
    r->ident = -1;

    r->resend_timeout = 5;
    r->tcp_timeout = 5;
    r->expire = 30;
    r->valid = 0;

    r->log = &cf->cycle->new_log;
    r->log_level = NGX_LOG_ERR;

    if (n) {
        if (ngx_array_init(&r->connections, cf->pool, n,
                           sizeof(ngx_resolver_connection_t))
            != NGX_OK)
        {
            return NULL;
        }
    }

    for (i = 0; i < n; i++) {
        if (ngx_strncmp(names[i].data, "valid=", 6) == 0) {
            s.len = names[i].len - 6;
            s.data = names[i].data + 6;

            r->valid = ngx_parse_time(&s, 1);

            if (r->valid == (time_t) NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid parameter: %V", &names[i]);
                return NULL;
            }

            continue;
        }

#if (NGX_HAVE_INET6)
        if (ngx_strncmp(names[i].data, "ipv6=", 5) == 0) {

            if (ngx_strcmp(&names[i].data[5], "on") == 0) {
                r->ipv6 = 1;

            } else if (ngx_strcmp(&names[i].data[5], "off") == 0) {
                r->ipv6 = 0;

            } else {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid parameter: %V", &names[i]);
                return NULL;
            }

            continue;
        }
#endif

        ngx_memzero(&u, sizeof(ngx_url_t));

        u.url = names[i];
        u.default_port = 53;

        if (ngx_parse_url(cf->pool, &u) != NGX_OK) {
            if (u.err) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "%s in resolver \"%V\"",
                                   u.err, &u.url);
            }

            return NULL;
        }

        rec = ngx_array_push_n(&r->connections, u.naddrs);
        if (rec == NULL) {
            return NULL;
        }

        ngx_memzero(rec, u.naddrs * sizeof(ngx_resolver_connection_t));

        for (j = 0; j < u.naddrs; j++) {
            rec[j].sockaddr = u.addrs[j].sockaddr;
            rec[j].socklen = u.addrs[j].socklen;
            rec[j].server = u.addrs[j].name;
            rec[j].resolver = r;
        }
    }

    return r;
}
{% endhighlight %}
本函数用于创建一个```ngx_resolver_t```对象。在介绍本函数的具体实现之前，我们先来大体看一下resolver的配置：
<pre>
resolver 223.5.5.5 223.6.6.6 1.2.4.8 114.114.114.114 valid=3600s;
</pre>
因此，如果解析到此```resolver```命令时调用ngx_resolver_create()函数，那么```names```将为后面所有以空格分隔的字符串。

下面介绍一下本函数的大体实现流程：
{% highlight string %}
ngx_resolver_t *
ngx_resolver_create(ngx_conf_t *cf, ngx_str_t *names, ngx_uint_t n)
{
	//1) 为要创建的ngx_resolver_t对象创建一个pool cleanup，这样后续该对象就可以随池的销毁而自动释放相应的空间。

	//2) 创建ngx_resolver_t对象，并初始化该对象的event、name_rbtree、srv_rbtree、addr_rbtree等方面的属性

	//3) 设置ngx_resolver_t对象event的事件处理器ngx_resolver_resend_handler，相应的TCP超时时间tcp_timeout，
	// 以及对DNS结果的缓存时间valid

	//4) 若n>0，则创建对应的connections对象数组，数组中的每一个元素表示与DNS服务器的连接（注意，对于一个DNS域名
	// 可能会对应多个不同的IP地址)
}
{% endhighlight %}


## 5. 函数ngx_resolver_cleanup()
{% highlight string %}
static void
ngx_resolver_cleanup(void *data)
{
    ngx_resolver_t  *r = data;

    ngx_uint_t                  i;
    ngx_resolver_connection_t  *rec;

    if (r) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                       "cleanup resolver");

        ngx_resolver_cleanup_tree(r, &r->name_rbtree);

        ngx_resolver_cleanup_tree(r, &r->srv_rbtree);

        ngx_resolver_cleanup_tree(r, &r->addr_rbtree);

#if (NGX_HAVE_INET6)
        ngx_resolver_cleanup_tree(r, &r->addr6_rbtree);
#endif

        if (r->event) {
            ngx_free(r->event);
        }


        rec = r->connections.elts;

        for (i = 0; i < r->connections.nelts; i++) {
            if (rec[i].udp) {
                ngx_close_connection(rec[i].udp);
            }

            if (rec[i].tcp) {
                ngx_close_connection(rec[i].tcp);
            }

            if (rec[i].read_buf) {
                ngx_resolver_free(r, rec[i].read_buf->start);
                ngx_resolver_free(r, rec[i].read_buf);
            }

            if (rec[i].write_buf) {
                ngx_resolver_free(r, rec[i].write_buf->start);
                ngx_resolver_free(r, rec[i].write_buf);
            }
        }

        ngx_free(r);
    }
}

{% endhighlight %}
此函数是ngx_resolver_t对象的pool被销毁时的回调函数，主要完成相关内存资源的回收以及关闭对应的连接：
{% highlight string %}
static void
ngx_resolver_cleanup(void *data)
{
	//1) 回收相应的红黑树结构: name_rbtree、srv_rbtree、addr_rbtree、addr6_rbtree

	//2) 释放ngx_resolver_t的event对象

	//3) 关闭与DNS的连接，并释放连接相关的内存占用

	//4) 释放ngx_resolver_t对象本身
}
{% endhighlight %}

## 6. 函数ngx_resolver_cleanup_tree()
{% highlight string %}
static void
ngx_resolver_cleanup_tree(ngx_resolver_t *r, ngx_rbtree_t *tree)
{
    ngx_resolver_ctx_t   *ctx, *next;
    ngx_resolver_node_t  *rn;

    while (tree->root != tree->sentinel) {

        rn = ngx_resolver_node(ngx_rbtree_min(tree->root, tree->sentinel));

        ngx_queue_remove(&rn->queue);

        for (ctx = rn->waiting; ctx; ctx = next) {
            next = ctx->next;

            if (ctx->event) {
                ngx_resolver_free(r, ctx->event);
            }

            ngx_resolver_free(r, ctx);
        }

        ngx_rbtree_delete(tree, &rn->node);

        ngx_resolver_free_node(r, rn);
    }
}
{% endhighlight %}
本函数按从小到大的顺序依次删除```ngx_resolver_t```中相应红黑树的节点。并将该节点所关联的```rn->waiting```链表中的上下文对象进行删除。


## 7. 函数ngx_resolve_start()
{% highlight string %}
ngx_resolver_ctx_t *
ngx_resolve_start(ngx_resolver_t *r, ngx_resolver_ctx_t *temp)
{
    in_addr_t            addr;
    ngx_resolver_ctx_t  *ctx;

    if (temp) {
        addr = ngx_inet_addr(temp->name.data, temp->name.len);

        if (addr != INADDR_NONE) {
            temp->resolver = r;
            temp->state = NGX_OK;
            temp->naddrs = 1;
            temp->addrs = &temp->addr;
            temp->addr.sockaddr = (struct sockaddr *) &temp->sin;
            temp->addr.socklen = sizeof(struct sockaddr_in);
            ngx_memzero(&temp->sin, sizeof(struct sockaddr_in));
            temp->sin.sin_family = AF_INET;
            temp->sin.sin_addr.s_addr = addr;
            temp->quick = 1;

            return temp;
        }
    }

    if (r->connections.nelts == 0) {
        return NGX_NO_RESOLVER;
    }

    ctx = ngx_resolver_calloc(r, sizeof(ngx_resolver_ctx_t));

    if (ctx) {
        ctx->resolver = r;
    }

    return ctx;
}
{% endhighlight %}
本函数用于创建```ngx_resolver_t```的上下文。如果传入的参数temp不为NULL，且```temp->name```能够被解析为一个IPv4地址，则复用temp，将其作为```r```的上下文对象； 否则新建一个新的上下文对象。

这里注意，当```temp->name```能够成功解析为IP地址时，会将temp->quick置为1，这样后续就不再需要请求DNS来解析了。


## 8. 函数ngx_resolve_name()
{% highlight string %}
ngx_int_t
ngx_resolve_name(ngx_resolver_ctx_t *ctx)
{
    size_t           slen;
    ngx_int_t        rc;
    ngx_str_t        name;
    ngx_resolver_t  *r;

    r = ctx->resolver;

    if (ctx->name.len > 0 && ctx->name.data[ctx->name.len - 1] == '.') {
        ctx->name.len--;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0,
                   "resolve: \"%V\"", &ctx->name);

    if (ctx->quick) {
        ctx->handler(ctx);
        return NGX_OK;
    }

    if (ctx->service.len) {
        slen = ctx->service.len;

        if (ngx_strlchr(ctx->service.data,
                        ctx->service.data + ctx->service.len, '.')
            == NULL)
        {
            slen += sizeof("_._tcp") - 1;
        }

        name.len = slen + 1 + ctx->name.len;

        name.data = ngx_resolver_alloc(r, name.len);
        if (name.data == NULL) {
            return NGX_ERROR;
        }

        if (slen == ctx->service.len) {
            ngx_sprintf(name.data, "%V.%V", &ctx->service, &ctx->name);

        } else {
            ngx_sprintf(name.data, "_%V._tcp.%V", &ctx->service, &ctx->name);
        }

        /* lock name mutex */

        rc = ngx_resolve_name_locked(r, ctx, &name);

        ngx_resolver_free(r, name.data);

    } else {
        /* lock name mutex */

        rc = ngx_resolve_name_locked(r, ctx, &ctx->name);
    }

    if (rc == NGX_OK) {
        return NGX_OK;
    }

    /* unlock name mutex */

    if (rc == NGX_AGAIN) {
        return NGX_OK;
    }

    /* NGX_ERROR */

    if (ctx->event) {
        ngx_resolver_free(r, ctx->event);
    }

    ngx_resolver_free(r, ctx);

    return NGX_ERROR;
}
{% endhighlight %}

在介绍本函数之前，我们先大体说明一下DNS的服务查询(SRV)的格式：
<pre>
_ldap._tcp.example.com
</pre>
下面我们再来简单介绍本函数的实现：
{% highlight string %}
ngx_int_t
ngx_resolve_name(ngx_resolver_ctx_t *ctx)
{
	//1) 如果ctx->quick为1，那么直接回调handler()即可

	//2) 若指定了ctx->service，表示的是查询某一个服务，这时需要做特定的处理，然后调用ngx_resolve_name_locked()来完成'服务名到IP的映射'

	//3) 否则，直接调用ngx_resolve_name_locked()来完成'域名到IP的映射'
}
{% endhighlight %}

## 9. 函数ngx_resolve_name_done()
{% highlight string %}
void
ngx_resolve_name_done(ngx_resolver_ctx_t *ctx)
{
    ngx_uint_t            i;
    ngx_resolver_t       *r;
    ngx_resolver_ctx_t   *w, **p;
    ngx_resolver_node_t  *rn;

    r = ctx->resolver;

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0,
                   "resolve name done: %i", ctx->state);

    if (ctx->quick) {
        return;
    }

    if (ctx->event && ctx->event->timer_set) {
        ngx_del_timer(ctx->event);
    }

    /* lock name mutex */

    if (ctx->nsrvs) {
        for (i = 0; i < ctx->nsrvs; i++) {
            if (ctx->srvs[i].ctx) {
                ngx_resolve_name_done(ctx->srvs[i].ctx);
            }

            if (ctx->srvs[i].addrs) {
                ngx_resolver_free(r, ctx->srvs[i].addrs->sockaddr);
                ngx_resolver_free(r, ctx->srvs[i].addrs);
            }

            ngx_resolver_free(r, ctx->srvs[i].name.data);
        }

        ngx_resolver_free(r, ctx->srvs);
    }

    if (ctx->state == NGX_AGAIN || ctx->state == NGX_RESOLVE_TIMEDOUT) {

        rn = ctx->node;

        if (rn) {
            p = &rn->waiting;
            w = rn->waiting;

            while (w) {
                if (w == ctx) {
                    *p = w->next;

                    goto done;
                }

                p = &w->next;
                w = w->next;
            }

            ngx_log_error(NGX_LOG_ALERT, r->log, 0,
                          "could not cancel %V resolving", &ctx->name);
        }
    }

done:

    if (ctx->service.len) {
        ngx_resolver_expire(r, &r->srv_rbtree, &r->srv_expire_queue);

    } else {
        ngx_resolver_expire(r, &r->name_rbtree, &r->name_expire_queue);
    }

    /* unlock name mutex */

    /* lock alloc mutex */

    if (ctx->event) {
        ngx_resolver_free_locked(r, ctx->event);
    }

    ngx_resolver_free_locked(r, ctx);

    /* unlock alloc mutex */

    if (r->event->timer_set && ngx_resolver_resend_empty(r)) {
        ngx_del_timer(r->event);
    }
}
{% endhighlight %}

此函数用于处理当解析完成（可能成功，也可能失败），进行相应的收尾工作。下面我们先简要介绍一下```通过服务名来查询IP```返回报文的基本格式：
<pre>
_Service._Proto.Name TTL Class SRV Priority Weight Port Target
</pre>

下面是本函数的基本流程：
{% highlight string %}
void
ngx_resolve_name_done(ngx_resolver_ctx_t *ctx)
{
	//1) 如果ctx->quick为真，此种情况根本没有向DNS发出解析请求，因此这里可以直接返回

	//2) 如果ctx->event上仍还有定时器在运行，那么清除相应定时器

	//3) 若是 SRV查询，即ctx->nsrvs>0，那么释放服务所占用的空间

	//4) 如果状态为NGX_AGAIN或者NGX_RESOLVE_TIMEDOUT，那么将该上下文从waiting链表中移除

	//5) 从超时队列中移除1~2个超时节点（这里注意到，一次只会移除少数几个超时节点，从而把整个超时处理过程分摊到
	// 整个程序运行过程中，此种处理方法在很多程序设计中值得借鉴）

	//6) 删除ctx->event

	//7) 删除ctx本身

	//8) 若对应ngx_resolver_t的resend_queue并没有重发任务时，则移除ctx对应的resolver上的事件定时器
}
{% endhighlight %}


## 10. 函数ngx_resolve_name_locked()
{% highlight string %}
static ngx_int_t
ngx_resolve_name_locked(ngx_resolver_t *r, ngx_resolver_ctx_t *ctx,
    ngx_str_t *name)
{
    uint32_t              hash;
    ngx_int_t             rc;
    ngx_str_t             cname;
    ngx_uint_t            i, naddrs;
    ngx_queue_t          *resend_queue, *expire_queue;
    ngx_rbtree_t         *tree;
    ngx_resolver_ctx_t   *next, *last;
    ngx_resolver_addr_t  *addrs;
    ngx_resolver_node_t  *rn;

    ngx_strlow(name->data, name->data, name->len);

    hash = ngx_crc32_short(name->data, name->len);

    if (ctx->service.len) {
        rn = ngx_resolver_lookup_srv(r, name, hash);

        tree = &r->srv_rbtree;
        resend_queue = &r->srv_resend_queue;
        expire_queue = &r->srv_expire_queue;

    } else {
        rn = ngx_resolver_lookup_name(r, name, hash);

        tree = &r->name_rbtree;
        resend_queue = &r->name_resend_queue;
        expire_queue = &r->name_expire_queue;
    }

    if (rn) {

        /* ctx can be a list after NGX_RESOLVE_CNAME */
        for (last = ctx; last->next; last = last->next);

        if (rn->valid >= ngx_time()) {

            ngx_log_debug0(NGX_LOG_DEBUG_CORE, r->log, 0, "resolve cached");

            ngx_queue_remove(&rn->queue);

            rn->expire = ngx_time() + r->expire;

            ngx_queue_insert_head(expire_queue, &rn->queue);

            naddrs = (rn->naddrs == (u_short) -1) ? 0 : rn->naddrs;
#if (NGX_HAVE_INET6)
            naddrs += (rn->naddrs6 == (u_short) -1) ? 0 : rn->naddrs6;
#endif

            if (naddrs) {

                if (naddrs == 1 && rn->naddrs == 1) {
                    addrs = NULL;

                } else {
                    addrs = ngx_resolver_export(r, rn, 1);
                    if (addrs == NULL) {
                        return NGX_ERROR;
                    }
                }

                last->next = rn->waiting;
                rn->waiting = NULL;

                /* unlock name mutex */

                do {
                    ctx->state = NGX_OK;
                    ctx->valid = rn->valid;
                    ctx->naddrs = naddrs;

                    if (addrs == NULL) {
                        ctx->addrs = &ctx->addr;
                        ctx->addr.sockaddr = (struct sockaddr *) &ctx->sin;
                        ctx->addr.socklen = sizeof(struct sockaddr_in);
                        ngx_memzero(&ctx->sin, sizeof(struct sockaddr_in));
                        ctx->sin.sin_family = AF_INET;
                        ctx->sin.sin_addr.s_addr = rn->u.addr;

                    } else {
                        ctx->addrs = addrs;
                    }

                    next = ctx->next;

                    ctx->handler(ctx);

                    ctx = next;
                } while (ctx);

                if (addrs != NULL) {
                    ngx_resolver_free(r, addrs->sockaddr);
                    ngx_resolver_free(r, addrs);
                }

                return NGX_OK;
            }

            if (rn->nsrvs) {
                last->next = rn->waiting;
                rn->waiting = NULL;

                /* unlock name mutex */

                do {
                    next = ctx->next;

                    ngx_resolver_resolve_srv_names(ctx, rn);

                    ctx = next;
                } while (ctx);

                return NGX_OK;
            }

            /* NGX_RESOLVE_CNAME */

            if (ctx->recursion++ < NGX_RESOLVER_MAX_RECURSION) {

                cname.len = rn->cnlen;
                cname.data = rn->u.cname;

                return ngx_resolve_name_locked(r, ctx, &cname);
            }

            last->next = rn->waiting;
            rn->waiting = NULL;

            /* unlock name mutex */

            do {
                ctx->state = NGX_RESOLVE_NXDOMAIN;
                ctx->valid = ngx_time() + (r->valid ? r->valid : 10);
                next = ctx->next;

                ctx->handler(ctx);

                ctx = next;
            } while (ctx);

            return NGX_OK;
        }

        if (rn->waiting) {

            if (ctx->event == NULL && ctx->timeout) {
                ctx->event = ngx_resolver_calloc(r, sizeof(ngx_event_t));
                if (ctx->event == NULL) {
                    return NGX_ERROR;
                }

                ctx->event->handler = ngx_resolver_timeout_handler;
                ctx->event->data = ctx;
                ctx->event->log = r->log;
                ctx->ident = -1;

                ngx_add_timer(ctx->event, ctx->timeout);
            }

            last->next = rn->waiting;
            rn->waiting = ctx;
            ctx->state = NGX_AGAIN;

            do {
                ctx->node = rn;
                ctx = ctx->next;
            } while (ctx);

            return NGX_AGAIN;
        }

        ngx_queue_remove(&rn->queue);

        /* lock alloc mutex */

        if (rn->query) {
            ngx_resolver_free_locked(r, rn->query);
            rn->query = NULL;
#if (NGX_HAVE_INET6)
            rn->query6 = NULL;
#endif
        }

        if (rn->cnlen) {
            ngx_resolver_free_locked(r, rn->u.cname);
        }

        if (rn->naddrs > 1 && rn->naddrs != (u_short) -1) {
            ngx_resolver_free_locked(r, rn->u.addrs);
        }

#if (NGX_HAVE_INET6)
        if (rn->naddrs6 > 1 && rn->naddrs6 != (u_short) -1) {
            ngx_resolver_free_locked(r, rn->u6.addrs6);
        }
#endif

        if (rn->nsrvs) {
            for (i = 0; i < rn->nsrvs; i++) {
                if (rn->u.srvs[i].name.data) {
                    ngx_resolver_free_locked(r, rn->u.srvs[i].name.data);
                }
            }

            ngx_resolver_free_locked(r, rn->u.srvs);
        }

        /* unlock alloc mutex */

    } else {

        rn = ngx_resolver_alloc(r, sizeof(ngx_resolver_node_t));
        if (rn == NULL) {
            return NGX_ERROR;
        }

        rn->name = ngx_resolver_dup(r, name->data, name->len);
        if (rn->name == NULL) {
            ngx_resolver_free(r, rn);
            return NGX_ERROR;
        }

        rn->node.key = hash;
        rn->nlen = (u_short) name->len;
        rn->query = NULL;
#if (NGX_HAVE_INET6)
        rn->query6 = NULL;
#endif

        ngx_rbtree_insert(tree, &rn->node);
    }

    if (ctx->service.len) {
        rc = ngx_resolver_create_srv_query(r, rn, name);

    } else {
        rc = ngx_resolver_create_name_query(r, rn, name);
    }

    if (rc == NGX_ERROR) {
        goto failed;
    }

    if (rc == NGX_DECLINED) {
        ngx_rbtree_delete(tree, &rn->node);

        ngx_resolver_free(r, rn->query);
        ngx_resolver_free(r, rn->name);
        ngx_resolver_free(r, rn);

        do {
            ctx->state = NGX_RESOLVE_NXDOMAIN;
            next = ctx->next;

            ctx->handler(ctx);

            ctx = next;
        } while (ctx);

        return NGX_OK;
    }

    rn->last_connection = r->last_connection++;
    if (r->last_connection == r->connections.nelts) {
        r->last_connection = 0;
    }

    rn->naddrs = (u_short) -1;
    rn->tcp = 0;
#if (NGX_HAVE_INET6)
    rn->naddrs6 = r->ipv6 ? (u_short) -1 : 0;
    rn->tcp6 = 0;
#endif
    rn->nsrvs = 0;

    if (ngx_resolver_send_query(r, rn) != NGX_OK) {
        goto failed;
    }

    if (ctx->event == NULL && ctx->timeout) {
        ctx->event = ngx_resolver_calloc(r, sizeof(ngx_event_t));
        if (ctx->event == NULL) {
            goto failed;
        }

        ctx->event->handler = ngx_resolver_timeout_handler;
        ctx->event->data = ctx;
        ctx->event->log = r->log;
        ctx->ident = -1;

        ngx_add_timer(ctx->event, ctx->timeout);
    }

    if (ngx_queue_empty(resend_queue)) {
        ngx_add_timer(r->event, (ngx_msec_t) (r->resend_timeout * 1000));
    }

    rn->expire = ngx_time() + r->resend_timeout;

    ngx_queue_insert_head(resend_queue, &rn->queue);

    rn->code = 0;
    rn->cnlen = 0;
    rn->valid = 0;
    rn->ttl = NGX_MAX_UINT32_VALUE;
    rn->waiting = ctx;

    ctx->state = NGX_AGAIN;

    do {
        ctx->node = rn;
        ctx = ctx->next;
    } while (ctx);

    return NGX_AGAIN;

failed:

    ngx_rbtree_delete(tree, &rn->node);

    if (rn->query) {
        ngx_resolver_free(r, rn->query);
    }

    ngx_resolver_free(r, rn->name);

    ngx_resolver_free(r, rn);

    return NGX_ERROR;
}
{% endhighlight %}
本函数实现的主要功能就是向DNS服务器发起```域名查询```或```服务名查询```。下面我们简要分析一下函数的实现步骤：
{% highlight string %}
static ngx_int_t
ngx_resolve_name_locked(ngx_resolver_t *r, ngx_resolver_ctx_t *ctx,
    ngx_str_t *name)
{
	//1) 首先检查r->srv_rbtree或者r->name_rbtree，看是否有已经缓存有对应的信息

	//2) 如果检查到有对应的缓存信息
	//  2.1) 如果查找到的节点仍在有效期内
			2.1.1） 如果当前已经有解析到的地址信息了，那么直接调用ctx所绑定的handler()回调函数即可
	//		2.1.2） 否则，根据相应的情况重新发起 '域名查询' 或 '服务名查询', 或将状态设置为NGX_RESOLVE_NXDOMAIN,
	//       		然后直接回调ctx绑定的handler()
	//  2.2) 否则，检查rn->waiting链表，如果不为NULL，表示当前仍有等待解析的任务，直接将ctx->state设置为NGX_AGAIN，然后返回
	

	//3) 否则，构造相应的查询报文，然后发起查询请求
	 
	//4) 如果当前ctx->event为NULL，并且ctx又具有timeout属性，那么默认为此context绑定一个超时回调事件
	
}
{% endhighlight %}



## 11. 函数ngx_resolve_addr()
{% highlight string %}
ngx_int_t
ngx_resolve_addr(ngx_resolver_ctx_t *ctx)
{
    u_char               *name;
    in_addr_t             addr;
    ngx_queue_t          *resend_queue, *expire_queue;
    ngx_rbtree_t         *tree;
    ngx_resolver_t       *r;
    struct sockaddr_in   *sin;
    ngx_resolver_node_t  *rn;
#if (NGX_HAVE_INET6)
    uint32_t              hash;
    struct sockaddr_in6  *sin6;
#endif

#if (NGX_SUPPRESS_WARN)
    addr = 0;
#if (NGX_HAVE_INET6)
    hash = 0;
    sin6 = NULL;
#endif
#endif

    r = ctx->resolver;

    switch (ctx->addr.sockaddr->sa_family) {

#if (NGX_HAVE_INET6)
    case AF_INET6:
        sin6 = (struct sockaddr_in6 *) ctx->addr.sockaddr;
        hash = ngx_crc32_short(sin6->sin6_addr.s6_addr, 16);

        /* lock addr mutex */

        rn = ngx_resolver_lookup_addr6(r, &sin6->sin6_addr, hash);

        tree = &r->addr6_rbtree;
        resend_queue = &r->addr6_resend_queue;
        expire_queue = &r->addr6_expire_queue;

        break;
#endif

    default: /* AF_INET */
        sin = (struct sockaddr_in *) ctx->addr.sockaddr;
        addr = ntohl(sin->sin_addr.s_addr);

        /* lock addr mutex */

        rn = ngx_resolver_lookup_addr(r, addr);

        tree = &r->addr_rbtree;
        resend_queue = &r->addr_resend_queue;
        expire_queue = &r->addr_expire_queue;
    }

    if (rn) {

        if (rn->valid >= ngx_time()) {

            ngx_log_debug0(NGX_LOG_DEBUG_CORE, r->log, 0, "resolve cached");

            ngx_queue_remove(&rn->queue);

            rn->expire = ngx_time() + r->expire;

            ngx_queue_insert_head(expire_queue, &rn->queue);

            name = ngx_resolver_dup(r, rn->name, rn->nlen);
            if (name == NULL) {
                goto failed;
            }

            ctx->name.len = rn->nlen;
            ctx->name.data = name;

            /* unlock addr mutex */

            ctx->state = NGX_OK;
            ctx->valid = rn->valid;

            ctx->handler(ctx);

            ngx_resolver_free(r, name);

            return NGX_OK;
        }

        if (rn->waiting) {

            if (ctx->event == NULL && ctx->timeout) {
                ctx->event = ngx_resolver_calloc(r, sizeof(ngx_event_t));
                if (ctx->event == NULL) {
                    return NGX_ERROR;
                }

                ctx->event->handler = ngx_resolver_timeout_handler;
                ctx->event->data = ctx;
                ctx->event->log = r->log;
                ctx->ident = -1;

                ngx_add_timer(ctx->event, ctx->timeout);
            }

            ctx->next = rn->waiting;
            rn->waiting = ctx;
            ctx->state = NGX_AGAIN;
            ctx->node = rn;

            /* unlock addr mutex */

            return NGX_OK;
        }

        ngx_queue_remove(&rn->queue);

        ngx_resolver_free(r, rn->query);
        rn->query = NULL;
#if (NGX_HAVE_INET6)
        rn->query6 = NULL;
#endif

    } else {
        rn = ngx_resolver_alloc(r, sizeof(ngx_resolver_node_t));
        if (rn == NULL) {
            goto failed;
        }

        switch (ctx->addr.sockaddr->sa_family) {

#if (NGX_HAVE_INET6)
        case AF_INET6:
            rn->addr6 = sin6->sin6_addr;
            rn->node.key = hash;
            break;
#endif

        default: /* AF_INET */
            rn->node.key = addr;
        }

        rn->query = NULL;
#if (NGX_HAVE_INET6)
        rn->query6 = NULL;
#endif

        ngx_rbtree_insert(tree, &rn->node);
    }

    if (ngx_resolver_create_addr_query(r, rn, &ctx->addr) != NGX_OK) {
        goto failed;
    }

    rn->last_connection = r->last_connection++;
    if (r->last_connection == r->connections.nelts) {
        r->last_connection = 0;
    }

    rn->naddrs = (u_short) -1;
    rn->tcp = 0;
#if (NGX_HAVE_INET6)
    rn->naddrs6 = (u_short) -1;
    rn->tcp6 = 0;
#endif
    rn->nsrvs = 0;

    if (ngx_resolver_send_query(r, rn) != NGX_OK) {
        goto failed;
    }

    if (ctx->event == NULL && ctx->timeout) {
        ctx->event = ngx_resolver_calloc(r, sizeof(ngx_event_t));
        if (ctx->event == NULL) {
            goto failed;
        }

        ctx->event->handler = ngx_resolver_timeout_handler;
        ctx->event->data = ctx;
        ctx->event->log = r->log;
        ctx->ident = -1;

        ngx_add_timer(ctx->event, ctx->timeout);
    }

    if (ngx_queue_empty(resend_queue)) {
        ngx_add_timer(r->event, (ngx_msec_t) (r->resend_timeout * 1000));
    }

    rn->expire = ngx_time() + r->resend_timeout;

    ngx_queue_insert_head(resend_queue, &rn->queue);

    rn->code = 0;
    rn->cnlen = 0;
    rn->name = NULL;
    rn->nlen = 0;
    rn->valid = 0;
    rn->ttl = NGX_MAX_UINT32_VALUE;
    rn->waiting = ctx;

    /* unlock addr mutex */

    ctx->state = NGX_AGAIN;
    ctx->node = rn;

    return NGX_OK;

failed:

    if (rn) {
        ngx_rbtree_delete(tree, &rn->node);

        if (rn->query) {
            ngx_resolver_free(r, rn->query);
        }

        ngx_resolver_free(r, rn);
    }

    /* unlock addr mutex */

    if (ctx->event) {
        ngx_resolver_free(r, ctx->event);
    }

    ngx_resolver_free(r, ctx);

    return NGX_ERROR;
}
{% endhighlight %}
本函数进行DNS逆查询，即通过DNS，查询```ctx->addr```地址所对应的域名。下面简要介绍一下本函数的实现：
{% highlight string %}
ngx_int_t
ngx_resolve_addr(ngx_resolver_ctx_t *ctx)
{
	//1) 从红黑树中查询当前所缓存的IP地址到域名的映射

	//2) 如果红黑树r->addr_rbtree中有对应的缓存信息
	// 2.1) 如果缓存信息并未过期，则将该节点加入到addr_expire_queue或addr6_expire_queue中，同时调用对应的handler()回调函数
	// 2.2) 如果对应节点的waiting链表不为NULL，则将当前上下文加入到waiting链表中， 并将ctx状态设置为NGX_AGAIN

	//3) 如果在红黑树r->addr_rbtree中没有对应的缓存信息，创建一个ngx_resolver_node_t对象加入到ctx->resolver相应的红黑树中

	//4) 创建DNS逆解析报文，并向对应的DNS服务器发送请求

	//5) 将ctx->state设置为NGX_AGAIN，等待返回相应的处理结果
}
{% endhighlight %}

## 12. 函数ngx_resolve_addr_done()
{% highlight string %}
void
ngx_resolve_addr_done(ngx_resolver_ctx_t *ctx)
{
    ngx_queue_t          *expire_queue;
    ngx_rbtree_t         *tree;
    ngx_resolver_t       *r;
    ngx_resolver_ctx_t   *w, **p;
    ngx_resolver_node_t  *rn;

    r = ctx->resolver;

    switch (ctx->addr.sockaddr->sa_family) {

#if (NGX_HAVE_INET6)
    case AF_INET6:
        tree = &r->addr6_rbtree;
        expire_queue = &r->addr6_expire_queue;
        break;
#endif

    default: /* AF_INET */
        tree = &r->addr_rbtree;
        expire_queue = &r->addr_expire_queue;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0,
                   "resolve addr done: %i", ctx->state);

    if (ctx->event && ctx->event->timer_set) {
        ngx_del_timer(ctx->event);
    }

    /* lock addr mutex */

    if (ctx->state == NGX_AGAIN || ctx->state == NGX_RESOLVE_TIMEDOUT) {

        rn = ctx->node;

        if (rn) {
            p = &rn->waiting;
            w = rn->waiting;

            while (w) {
                if (w == ctx) {
                    *p = w->next;

                    goto done;
                }

                p = &w->next;
                w = w->next;
            }
        }

        {
            u_char     text[NGX_SOCKADDR_STRLEN];
            ngx_str_t  addrtext;

            addrtext.data = text;
            addrtext.len = ngx_sock_ntop(ctx->addr.sockaddr, ctx->addr.socklen,
                                         text, NGX_SOCKADDR_STRLEN, 0);

            ngx_log_error(NGX_LOG_ALERT, r->log, 0,
                          "could not cancel %V resolving", &addrtext);
        }
    }

done:

    ngx_resolver_expire(r, tree, expire_queue);

    /* unlock addr mutex */

    /* lock alloc mutex */

    if (ctx->event) {
        ngx_resolver_free_locked(r, ctx->event);
    }

    ngx_resolver_free_locked(r, ctx);

    /* unlock alloc mutex */

    if (r->event->timer_set && ngx_resolver_resend_empty(r)) {
        ngx_del_timer(r->event);
    }
}
{% endhighlight %}
此函数用于处理当解析完成（可能成功，也可能失败），进行相应的收尾工作：
{% highlight string %}
1) 如果状态为NGX_AGAIN或者NGX_RESOLVE_TIMEDOUT，那么将该上下文从waiting链表中移除

2) 从ctx->resolver的addr_expire_queue或addr6_expire_queue队列中移除相应的节点

3) 如果ctx->resolver上并没有相应的resend事件了，则将ctx->resolver上相应的定时器移除
{% endhighlight %}


## 13. 函数ngx_resolver_expire()
{% highlight string %}
static void
ngx_resolver_expire(ngx_resolver_t *r, ngx_rbtree_t *tree, ngx_queue_t *queue)
{
    time_t                now;
    ngx_uint_t            i;
    ngx_queue_t          *q;
    ngx_resolver_node_t  *rn;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, r->log, 0, "resolver expire");

    now = ngx_time();

    for (i = 0; i < 2; i++) {
        if (ngx_queue_empty(queue)) {
            return;
        }

        q = ngx_queue_last(queue);

        rn = ngx_queue_data(q, ngx_resolver_node_t, queue);

        if (now <= rn->expire) {
            return;
        }

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, r->log, 0,
                       "resolver expire \"%*s\"", (size_t) rn->nlen, rn->name);

        ngx_queue_remove(q);

        ngx_rbtree_delete(tree, &rn->node);

        ngx_resolver_free_node(r, rn);
    }
}
{% endhighlight %}
此函数用于淘汰相应超时队列上的1~2个超时节点。

## 14. 函数ngx_resolver_send_query()
{% highlight string %}
static ngx_int_t
ngx_resolver_send_query(ngx_resolver_t *r, ngx_resolver_node_t *rn)
{
    ngx_int_t                   rc;
    ngx_resolver_connection_t  *rec;

    rec = r->connections.elts;
    rec = &rec[rn->last_connection];

    if (rec->log.handler == NULL) {
        rec->log = *r->log;
        rec->log.handler = ngx_resolver_log_error;
        rec->log.data = rec;
        rec->log.action = "resolving";
    }

    if (rn->naddrs == (u_short) -1) {
        rc = rn->tcp ? ngx_resolver_send_tcp_query(r, rec, rn->query, rn->qlen)
                     : ngx_resolver_send_udp_query(r, rec, rn->query, rn->qlen);

        if (rc != NGX_OK) {
            return rc;
        }
    }

#if (NGX_HAVE_INET6)

    if (rn->query6 && rn->naddrs6 == (u_short) -1) {
        rc = rn->tcp6
                    ? ngx_resolver_send_tcp_query(r, rec, rn->query6, rn->qlen)
                    : ngx_resolver_send_udp_query(r, rec, rn->query6, rn->qlen);

        if (rc != NGX_OK) {
            return rc;
        }
    }

#endif

    return NGX_OK;
}

{% endhighlight %}
此函数用于向DNS服务器发送相应的查询报文。


## 15. 函数ngx_resolver_send_udp_query()
{% highlight string %}
static ngx_int_t
ngx_resolver_send_udp_query(ngx_resolver_t *r, ngx_resolver_connection_t  *rec,
    u_char *query, u_short qlen)
{
    ssize_t  n;

    if (rec->udp == NULL) {
        if (ngx_udp_connect(rec) != NGX_OK) {
            return NGX_ERROR;
        }

        rec->udp->data = rec;
        rec->udp->read->handler = ngx_resolver_udp_read;
        rec->udp->read->resolver = 1;
    }

    n = ngx_send(rec->udp, query, qlen);

    if (n == -1) {
        return NGX_ERROR;
    }

    if ((size_t) n != (size_t) qlen) {
        ngx_log_error(NGX_LOG_CRIT, &rec->log, 0, "send() incomplete");
        return NGX_ERROR;
    }

    return NGX_OK;
}
{% endhighlight %}
采用UDP方式向DNS服务器发送查询报文

## 16. 函数ngx_resolver_send_tcp_query()
{% highlight string %}
static ngx_int_t
ngx_resolver_send_tcp_query(ngx_resolver_t *r, ngx_resolver_connection_t *rec,
    u_char *query, u_short qlen)
{
    ngx_buf_t  *b;
    ngx_int_t   rc;

    rc = NGX_OK;

    if (rec->tcp == NULL) {
        b = rec->read_buf;

        if (b == NULL) {
            b = ngx_resolver_calloc(r, sizeof(ngx_buf_t));
            if (b == NULL) {
                return NGX_ERROR;
            }

            b->start = ngx_resolver_alloc(r, NGX_RESOLVER_TCP_RSIZE);
            if (b->start == NULL) {
                ngx_resolver_free(r, b);
                return NGX_ERROR;
            }

            b->end = b->start + NGX_RESOLVER_TCP_RSIZE;

            rec->read_buf = b;
        }

        b->pos = b->start;
        b->last = b->start;

        b = rec->write_buf;

        if (b == NULL) {
            b = ngx_resolver_calloc(r, sizeof(ngx_buf_t));
            if (b == NULL) {
                return NGX_ERROR;
            }

            b->start = ngx_resolver_alloc(r, NGX_RESOLVER_TCP_WSIZE);
            if (b->start == NULL) {
                ngx_resolver_free(r, b);
                return NGX_ERROR;
            }

            b->end = b->start + NGX_RESOLVER_TCP_WSIZE;

            rec->write_buf = b;
        }

        b->pos = b->start;
        b->last = b->start;

        rc = ngx_tcp_connect(rec);
        if (rc == NGX_ERROR) {
            return NGX_ERROR;
        }

        rec->tcp->data = rec;
        rec->tcp->write->handler = ngx_resolver_tcp_write;
        rec->tcp->read->handler = ngx_resolver_tcp_read;
        rec->tcp->read->resolver = 1;

        ngx_add_timer(rec->tcp->write, (ngx_msec_t) (r->tcp_timeout * 1000));
    }

    b = rec->write_buf;

    if (b->end - b->last <  2 + qlen) {
        ngx_log_error(NGX_LOG_CRIT, &rec->log, 0, "buffer overflow");
        return NGX_ERROR;
    }

    *b->last++ = (u_char) (qlen >> 8);
    *b->last++ = (u_char) qlen;
    b->last = ngx_cpymem(b->last, query, qlen);

    if (rc == NGX_OK) {
        ngx_resolver_tcp_write(rec->tcp->write);
    }

    return NGX_OK;
}
{% endhighlight %}

以TCP方式向DNS服务器发送查询报文。如果连接没有建立，则创建对应的连接，并设置好读写缓冲。


<br />
<br />

**[参看]**

1. [Nginx DNS resolver配置实例](http://m.iis7.com/a/nr/082118.html)


2. [nginx关于域名解析的源码分析](https://blog.csdn.net/ChuiGeDaQiQiu/article/details/78842744?utm_source=blogxgwz7)

3. [DNS协议详解及报文格式分析](https://blog.csdn.net/tianxuhong/article/details/74922454)

4. [DNS系统SRV和NAPTR记录类型说明](https://blog.csdn.net/zhangmingcai/article/details/81126632)

5. [DNS中的协议字段定义](https://www.cnblogs.com/cobbliu/p/3691119.html)

6. [DNS RFC文档](https://www.isc.org/community/rfcs/dns/)
<br />
<br />
<br />

