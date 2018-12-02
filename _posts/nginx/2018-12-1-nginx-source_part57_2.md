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



## 1. 函数ngx_resolver_create()
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


<br />
<br />

**[参看]**

1. [Nginx DNS resolver配置实例](http://m.iis7.com/a/nr/082118.html)


2. [nginx关于域名解析的源码分析](https://blog.csdn.net/ChuiGeDaQiQiu/article/details/78842744?utm_source=blogxgwz7)




<br />
<br />
<br />

