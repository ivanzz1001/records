---
layout: post
title: core/ngx_resolver.c源文件分析(2)
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本章我们主要介绍一下ngx_resolver.c源文件，其主要是实现对nginx中涉及到的域名的解析。


<!-- more -->


## 1. 函数ngx_resolver_resend_handler()
{% highlight string %}
static void
ngx_resolver_resend_handler(ngx_event_t *ev)
{
    time_t           timer, atimer, stimer, ntimer;
#if (NGX_HAVE_INET6)
    time_t           a6timer;
#endif
    ngx_resolver_t  *r;

    r = ev->data;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, r->log, 0,
                   "resolver resend handler");

    /* lock name mutex */

    ntimer = ngx_resolver_resend(r, &r->name_rbtree, &r->name_resend_queue);

    stimer = ngx_resolver_resend(r, &r->srv_rbtree, &r->srv_resend_queue);

    /* unlock name mutex */

    /* lock addr mutex */

    atimer = ngx_resolver_resend(r, &r->addr_rbtree, &r->addr_resend_queue);

    /* unlock addr mutex */

#if (NGX_HAVE_INET6)

    /* lock addr6 mutex */

    a6timer = ngx_resolver_resend(r, &r->addr6_rbtree, &r->addr6_resend_queue);

    /* unlock addr6 mutex */

#endif

    timer = ntimer;

    if (timer == 0) {
        timer = atimer;

    } else if (atimer) {
        timer = ngx_min(timer, atimer);
    }

    if (timer == 0) {
        timer = stimer;

    } else if (stimer) {
        timer = ngx_min(timer, stimer);
    }

#if (NGX_HAVE_INET6)

    if (timer == 0) {
        timer = a6timer;

    } else if (a6timer) {
        timer = ngx_min(timer, a6timer);
    }

#endif

    if (timer) {
        ngx_add_timer(r->event, (ngx_msec_t) (timer * 1000));
    }
}
{% endhighlight %}
此函数的作用是使用ngx_resolver_t.event来建立定时器事件，从而发发送resend队列中的任务。

## 2. 函数ngx_resolver_resend()
{% highlight string %}
static time_t
ngx_resolver_resend(ngx_resolver_t *r, ngx_rbtree_t *tree, ngx_queue_t *queue)
{
    time_t                now;
    ngx_queue_t          *q;
    ngx_resolver_node_t  *rn;

    now = ngx_time();

    for ( ;; ) {
        if (ngx_queue_empty(queue)) {
            return 0;
        }

        q = ngx_queue_last(queue);

        rn = ngx_queue_data(q, ngx_resolver_node_t, queue);

        if (now < rn->expire) {
            return rn->expire - now;
        }

        ngx_log_debug3(NGX_LOG_DEBUG_CORE, r->log, 0,
                       "resolver resend \"%*s\" %p",
                       (size_t) rn->nlen, rn->name, rn->waiting);

        ngx_queue_remove(q);

        if (rn->waiting) {

            if (++rn->last_connection == r->connections.nelts) {
                rn->last_connection = 0;
            }

            (void) ngx_resolver_send_query(r, rn);

            rn->expire = now + r->resend_timeout;

            ngx_queue_insert_head(queue, q);

            continue;
        }

        ngx_rbtree_delete(tree, &rn->node);

        ngx_resolver_free_node(r, rn);
    }
}
{% endhighlight %}
此函数用于处理resend队列中的到期任务，并返回最近一个没有到期的任务的剩余时间。

## 3. 函数ngx_resolver_resend_empty()
{% highlight string %}
static ngx_uint_t
ngx_resolver_resend_empty(ngx_resolver_t *r)
{
    return ngx_queue_empty(&r->name_resend_queue)
#if (NGX_HAVE_INET6)
           && ngx_queue_empty(&r->addr6_resend_queue)
#endif
           && ngx_queue_empty(&r->addr_resend_queue);
}
{% endhighlight %}
用于判断resend_queue中是否仍有待发送任务。


## 4. 函数ngx_resolver_udp_read()
{% highlight string %}
static void
ngx_resolver_udp_read(ngx_event_t *rev)
{
    ssize_t                     n;
    ngx_connection_t           *c;
    ngx_resolver_connection_t  *rec;
    u_char                      buf[NGX_RESOLVER_UDP_SIZE];

    c = rev->data;
    rec = c->data;

    do {
        n = ngx_udp_recv(c, buf, NGX_RESOLVER_UDP_SIZE);

        if (n < 0) {
            return;
        }

        ngx_resolver_process_response(rec->resolver, buf, n, 0);

    } while (rev->ready);
}
{% endhighlight %}
从rev->data所表示的```ngx_connection_t```中读取UDP数据，然后调用相应的方法处理数据。

## 5. 函数ngx_resolver_tcp_write()
{% highlight string %}
static void
ngx_resolver_tcp_write(ngx_event_t *wev)
{
    off_t                       sent;
    ssize_t                     n;
    ngx_buf_t                  *b;
    ngx_resolver_t             *r;
    ngx_connection_t           *c;
    ngx_resolver_connection_t  *rec;

    c = wev->data;
    rec = c->data;
    b = rec->write_buf;
    r = rec->resolver;

    if (wev->timedout) {
        goto failed;
    }

    sent = c->sent;

    while (wev->ready && b->pos < b->last) {
        n = ngx_send(c, b->pos, b->last - b->pos);

        if (n == NGX_AGAIN) {
            break;
        }

        if (n == NGX_ERROR) {
            goto failed;
        }

        b->pos += n;
    }

    if (b->pos != b->start) {
        b->last = ngx_movemem(b->start, b->pos, b->last - b->pos);
        b->pos = b->start;
    }

    if (c->sent != sent) {
        ngx_add_timer(wev, (ngx_msec_t) (r->tcp_timeout * 1000));
    }

    if (ngx_handle_write_event(wev, 0) != NGX_OK) {
        goto failed;
    }

    return;

failed:

    ngx_close_connection(c);
    rec->tcp = NULL;
}
{% endhighlight %}
此函数用于发送对应write_buf中的数据。下面简要介绍一下实现流程：
{% highlight string %}
static void
ngx_resolver_tcp_write(ngx_event_t *wev)
{
	//1) 如果wev已经处于timeout状态，则关闭对应的连接

	//2) 循环发送TCP数据，直到不能发送更多的数据或发送失败为止

	//3) 调用ngx_handle_write_event()重置相应发送状态，以准备下一次的发送
}
{% endhighlight %}

## 6. 函数ngx_resolver_tcp_read()
{% highlight string %}
static void
ngx_resolver_tcp_read(ngx_event_t *rev)
{
    u_char                     *p;
    size_t                      size;
    ssize_t                     n;
    u_short                     qlen;
    ngx_buf_t                  *b;
    ngx_resolver_t             *r;
    ngx_connection_t           *c;
    ngx_resolver_connection_t  *rec;

    c = rev->data;
    rec = c->data;
    b = rec->read_buf;
    r = rec->resolver;

    while (rev->ready) {
        n = ngx_recv(c, b->last, b->end - b->last);

        if (n == NGX_AGAIN) {
            break;
        }

        if (n == NGX_ERROR || n == 0) {
            goto failed;
        }

        b->last += n;

        for ( ;; ) {
            p = b->pos;
            size = b->last - p;

            if (size < 2) {
                break;
            }

            qlen = (u_short) *p++ << 8;
            qlen += *p++;

            if (size < (size_t) (2 + qlen)) {
                break;
            }

            ngx_resolver_process_response(r, p, qlen, 1);

            b->pos += 2 + qlen;
        }

        if (b->pos != b->start) {
            b->last = ngx_movemem(b->start, b->pos, b->last - b->pos);
            b->pos = b->start;
        }
    }

    if (ngx_handle_read_event(rev, 0) != NGX_OK) {
        goto failed;
    }

    return;

failed:

    ngx_close_connection(c);
    rec->tcp = NULL;
}
{% endhighlight %}
此函数读取TCP数据到buf中，并在读取到足够数据时调用ngx_resolver_process_response()来对DNS返回的数据进行处理。

## 7. 函数ngx_resolver_process_response()
{% highlight string %}
static void
ngx_resolver_process_response(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t tcp)
{
    char                 *err;
    ngx_uint_t            i, times, ident, qident, flags, code, nqs, nan, trunc,
                          qtype, qclass;
#if (NGX_HAVE_INET6)
    ngx_uint_t            qident6;
#endif
    ngx_queue_t          *q;
    ngx_resolver_qs_t    *qs;
    ngx_resolver_hdr_t   *response;
    ngx_resolver_node_t  *rn;

    if (n < sizeof(ngx_resolver_hdr_t)) {
        goto short_response;
    }

    response = (ngx_resolver_hdr_t *) buf;

    ident = (response->ident_hi << 8) + response->ident_lo;
    flags = (response->flags_hi << 8) + response->flags_lo;
    nqs = (response->nqs_hi << 8) + response->nqs_lo;
    nan = (response->nan_hi << 8) + response->nan_lo;
    trunc = flags & 0x0200;

    ngx_log_debug6(NGX_LOG_DEBUG_CORE, r->log, 0,
                   "resolver DNS response %ui fl:%04Xi %ui/%ui/%ud/%ud",
                   ident, flags, nqs, nan,
                   (response->nns_hi << 8) + response->nns_lo,
                   (response->nar_hi << 8) + response->nar_lo);

    /* response to a standard query */
    if ((flags & 0xf870) != 0x8000 || (trunc && tcp)) {
        ngx_log_error(r->log_level, r->log, 0,
                      "invalid %s DNS response %ui fl:%04Xi",
                      tcp ? "TCP" : "UDP", ident, flags);
        return;
    }

    code = flags & 0xf;

    if (code == NGX_RESOLVE_FORMERR) {

        times = 0;

        for (q = ngx_queue_head(&r->name_resend_queue);
             q != ngx_queue_sentinel(&r->name_resend_queue) && times++ < 100;
             q = ngx_queue_next(q))
        {
            rn = ngx_queue_data(q, ngx_resolver_node_t, queue);
            qident = (rn->query[0] << 8) + rn->query[1];

            if (qident == ident) {
                goto dns_error_name;
            }

#if (NGX_HAVE_INET6)
            if (rn->query6) {
                qident6 = (rn->query6[0] << 8) + rn->query6[1];

                if (qident6 == ident) {
                    goto dns_error_name;
                }
            }
#endif
        }

        goto dns_error;
    }

    if (code > NGX_RESOLVE_REFUSED) {
        goto dns_error;
    }

    if (nqs != 1) {
        err = "invalid number of questions in DNS response";
        goto done;
    }

    i = sizeof(ngx_resolver_hdr_t);

    while (i < (ngx_uint_t) n) {
        if (buf[i] == '\0') {
            goto found;
        }

        i += 1 + buf[i];
    }

    goto short_response;

found:

    if (i++ == sizeof(ngx_resolver_hdr_t)) {
        err = "zero-length domain name in DNS response";
        goto done;
    }

    if (i + sizeof(ngx_resolver_qs_t) + nan * (2 + sizeof(ngx_resolver_an_t))
        > (ngx_uint_t) n)
    {
        goto short_response;
    }

    qs = (ngx_resolver_qs_t *) &buf[i];

    qtype = (qs->type_hi << 8) + qs->type_lo;
    qclass = (qs->class_hi << 8) + qs->class_lo;

    ngx_log_debug2(NGX_LOG_DEBUG_CORE, r->log, 0,
                   "resolver DNS response qt:%ui cl:%ui", qtype, qclass);

    if (qclass != 1) {
        ngx_log_error(r->log_level, r->log, 0,
                      "unknown query class %ui in DNS response", qclass);
        return;
    }

    switch (qtype) {

    case NGX_RESOLVE_A:
#if (NGX_HAVE_INET6)
    case NGX_RESOLVE_AAAA:
#endif

        ngx_resolver_process_a(r, buf, n, ident, code, qtype, nan, trunc,
                               i + sizeof(ngx_resolver_qs_t));

        break;

    case NGX_RESOLVE_SRV:

        ngx_resolver_process_srv(r, buf, n, ident, code, nan, trunc,
                                 i + sizeof(ngx_resolver_qs_t));

        break;

    case NGX_RESOLVE_PTR:

        ngx_resolver_process_ptr(r, buf, n, ident, code, nan);

        break;

    default:
        ngx_log_error(r->log_level, r->log, 0,
                      "unknown query type %ui in DNS response", qtype);
        return;
    }

    return;

short_response:

    err = "short DNS response";

done:

    ngx_log_error(r->log_level, r->log, 0, err);

    return;

dns_error_name:

    ngx_log_error(r->log_level, r->log, 0,
                  "DNS error (%ui: %s), query id:%ui, name:\"%*s\"",
                  code, ngx_resolver_strerror(code), ident,
                  (size_t) rn->nlen, rn->name);
    return;

dns_error:

    ngx_log_error(r->log_level, r->log, 0,
                  "DNS error (%ui: %s), query id:%ui",
                  code, ngx_resolver_strerror(code), ident);
    return;
}
{% endhighlight %}
本函数用于解析DNS返回的应答信息。下面我们简单分析一下实现流程：
{% highlight string %}
static void
ngx_resolver_process_response(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t tcp)
{
	//1) 解析头部信息ngx_resolver_hdr_t

	//2) 如果code == NGX_RESOLVE_FORMERR, 表明有DNS格式错误

	//3) 解析ngx_resolver_qs_t头部

	//4) 如下是跳过query区域的name字段
	while (i < (ngx_uint_t) n) {
        if (buf[i] == '\0') {
            goto found;
        }

        i += 1 + buf[i];
    }

	//4) 根据qtype分别调用不同的函数来处理相应的应答
}
{% endhighlight %}

## 8. 函数ngx_resolver_process_a()
{% highlight string %}
static void
ngx_resolver_process_a(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t ident, ngx_uint_t code, ngx_uint_t qtype,
    ngx_uint_t nan, ngx_uint_t trunc, ngx_uint_t ans)
{
    char                       *err;
    u_char                     *cname;
    size_t                      len;
    int32_t                     ttl;
    uint32_t                    hash;
    in_addr_t                  *addr;
    ngx_str_t                   name;
    ngx_uint_t                  type, class, qident, naddrs, a, i, j, start;
#if (NGX_HAVE_INET6)
    struct in6_addr            *addr6;
#endif
    ngx_resolver_an_t          *an;
    ngx_resolver_ctx_t         *ctx, *next;
    ngx_resolver_node_t        *rn;
    ngx_resolver_addr_t        *addrs;
    ngx_resolver_connection_t  *rec;

    if (ngx_resolver_copy(r, &name, buf,
                          buf + sizeof(ngx_resolver_hdr_t), buf + n)
        != NGX_OK)
    {
        return;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0, "resolver qs:%V", &name);

    hash = ngx_crc32_short(name.data, name.len);

    /* lock name mutex */

    rn = ngx_resolver_lookup_name(r, &name, hash);

    if (rn == NULL) {
        ngx_log_error(r->log_level, r->log, 0,
                      "unexpected response for %V", &name);
        ngx_resolver_free(r, name.data);
        goto failed;
    }

    switch (qtype) {

#if (NGX_HAVE_INET6)
    case NGX_RESOLVE_AAAA:

        if (rn->query6 == NULL || rn->naddrs6 != (u_short) -1) {
            ngx_log_error(r->log_level, r->log, 0,
                          "unexpected response for %V", &name);
            ngx_resolver_free(r, name.data);
            goto failed;
        }

        if (trunc && rn->tcp6) {
            ngx_resolver_free(r, name.data);
            goto failed;
        }

        qident = (rn->query6[0] << 8) + rn->query6[1];

        break;
#endif

    default: /* NGX_RESOLVE_A */

        if (rn->query == NULL || rn->naddrs != (u_short) -1) {
            ngx_log_error(r->log_level, r->log, 0,
                          "unexpected response for %V", &name);
            ngx_resolver_free(r, name.data);
            goto failed;
        }

        if (trunc && rn->tcp) {
            ngx_resolver_free(r, name.data);
            goto failed;
        }

        qident = (rn->query[0] << 8) + rn->query[1];
    }

    if (ident != qident) {
        ngx_log_error(r->log_level, r->log, 0,
                      "wrong ident %ui response for %V, expect %ui",
                      ident, &name, qident);
        ngx_resolver_free(r, name.data);
        goto failed;
    }

    ngx_resolver_free(r, name.data);

    if (trunc) {

        ngx_queue_remove(&rn->queue);

        if (rn->waiting == NULL) {
            ngx_rbtree_delete(&r->name_rbtree, &rn->node);
            ngx_resolver_free_node(r, rn);
            goto next;
        }

        rec = r->connections.elts;
        rec = &rec[rn->last_connection];

        switch (qtype) {

#if (NGX_HAVE_INET6)
        case NGX_RESOLVE_AAAA:

            rn->tcp6 = 1;

            (void) ngx_resolver_send_tcp_query(r, rec, rn->query6, rn->qlen);

            break;
#endif

        default: /* NGX_RESOLVE_A */

            rn->tcp = 1;

            (void) ngx_resolver_send_tcp_query(r, rec, rn->query, rn->qlen);
        }

        rn->expire = ngx_time() + r->resend_timeout;

        ngx_queue_insert_head(&r->name_resend_queue, &rn->queue);

        goto next;
    }

    if (code == 0 && rn->code) {
        code = rn->code;
    }

    if (code == 0 && nan == 0) {

#if (NGX_HAVE_INET6)
        switch (qtype) {

        case NGX_RESOLVE_AAAA:

            rn->naddrs6 = 0;

            if (rn->naddrs == (u_short) -1) {
                goto next;
            }

            if (rn->naddrs) {
                goto export;
            }

            break;

        default: /* NGX_RESOLVE_A */

            rn->naddrs = 0;

            if (rn->naddrs6 == (u_short) -1) {
                goto next;
            }

            if (rn->naddrs6) {
                goto export;
            }
        }
#endif

        code = NGX_RESOLVE_NXDOMAIN;
    }

    if (code) {

#if (NGX_HAVE_INET6)
        switch (qtype) {

        case NGX_RESOLVE_AAAA:

            rn->naddrs6 = 0;

            if (rn->naddrs == (u_short) -1) {
                rn->code = (u_char) code;
                goto next;
            }

            break;

        default: /* NGX_RESOLVE_A */

            rn->naddrs = 0;

            if (rn->naddrs6 == (u_short) -1) {
                rn->code = (u_char) code;
                goto next;
            }
        }
#endif

        next = rn->waiting;
        rn->waiting = NULL;

        ngx_queue_remove(&rn->queue);

        ngx_rbtree_delete(&r->name_rbtree, &rn->node);

        /* unlock name mutex */

        while (next) {
            ctx = next;
            ctx->state = code;
            ctx->valid = ngx_time() + (r->valid ? r->valid : 10);
            next = ctx->next;

            ctx->handler(ctx);
        }

        ngx_resolver_free_node(r, rn);

        return;
    }

    i = ans;
    naddrs = 0;
    cname = NULL;

    for (a = 0; a < nan; a++) {

        start = i;

        while (i < n) {

            if (buf[i] & 0xc0) {
                i += 2;
                goto found;
            }

            if (buf[i] == 0) {
                i++;
                goto test_length;
            }

            i += 1 + buf[i];
        }

        goto short_response;

    test_length:

        if (i - start < 2) {
            err = "invalid name in DNS response";
            goto invalid;
        }

    found:

        if (i + sizeof(ngx_resolver_an_t) >= n) {
            goto short_response;
        }

        an = (ngx_resolver_an_t *) &buf[i];

        type = (an->type_hi << 8) + an->type_lo;
        class = (an->class_hi << 8) + an->class_lo;
        len = (an->len_hi << 8) + an->len_lo;
        ttl = (an->ttl[0] << 24) + (an->ttl[1] << 16)
            + (an->ttl[2] << 8) + (an->ttl[3]);

        if (class != 1) {
            ngx_log_error(r->log_level, r->log, 0,
                          "unexpected RR class %ui", class);
            goto failed;
        }

        if (ttl < 0) {
            ttl = 0;
        }

        rn->ttl = ngx_min(rn->ttl, (uint32_t) ttl);

        i += sizeof(ngx_resolver_an_t);

        switch (type) {

        case NGX_RESOLVE_A:

            if (qtype != NGX_RESOLVE_A) {
                err = "unexpected A record in DNS response";
                goto invalid;
            }

            if (len != 4) {
                err = "invalid A record in DNS response";
                goto invalid;
            }

            if (i + 4 > n) {
                goto short_response;
            }

            naddrs++;

            break;

#if (NGX_HAVE_INET6)
        case NGX_RESOLVE_AAAA:

            if (qtype != NGX_RESOLVE_AAAA) {
                err = "unexpected AAAA record in DNS response";
                goto invalid;
            }

            if (len != 16) {
                err = "invalid AAAA record in DNS response";
                goto invalid;
            }

            if (i + 16 > n) {
                goto short_response;
            }

            naddrs++;

            break;
#endif

        case NGX_RESOLVE_CNAME:

            cname = &buf[i];

            break;

        case NGX_RESOLVE_DNAME:

            break;

        default:

            ngx_log_error(r->log_level, r->log, 0,
                          "unexpected RR type %ui", type);
        }

        i += len;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_CORE, r->log, 0,
                   "resolver naddrs:%ui cname:%p ttl:%uD",
                   naddrs, cname, rn->ttl);

    if (naddrs) {

        switch (qtype) {

#if (NGX_HAVE_INET6)
        case NGX_RESOLVE_AAAA:

            if (naddrs == 1) {
                addr6 = &rn->u6.addr6;
                rn->naddrs6 = 1;

            } else {
                addr6 = ngx_resolver_alloc(r, naddrs * sizeof(struct in6_addr));
                if (addr6 == NULL) {
                    goto failed;
                }

                rn->u6.addrs6 = addr6;
                rn->naddrs6 = (u_short) naddrs;
            }

#if (NGX_SUPPRESS_WARN)
            addr = NULL;
#endif

            break;
#endif

        default: /* NGX_RESOLVE_A */

            if (naddrs == 1) {
                addr = &rn->u.addr;
                rn->naddrs = 1;

            } else {
                addr = ngx_resolver_alloc(r, naddrs * sizeof(in_addr_t));
                if (addr == NULL) {
                    goto failed;
                }

                rn->u.addrs = addr;
                rn->naddrs = (u_short) naddrs;
            }

#if (NGX_HAVE_INET6 && NGX_SUPPRESS_WARN)
            addr6 = NULL;
#endif
        }

        j = 0;
        i = ans;

        for (a = 0; a < nan; a++) {

            for ( ;; ) {

                if (buf[i] & 0xc0) {
                    i += 2;
                    break;
                }

                if (buf[i] == 0) {
                    i++;
                    break;
                }

                i += 1 + buf[i];
            }

            an = (ngx_resolver_an_t *) &buf[i];

            type = (an->type_hi << 8) + an->type_lo;
            len = (an->len_hi << 8) + an->len_lo;

            i += sizeof(ngx_resolver_an_t);

            if (type == NGX_RESOLVE_A) {

                addr[j] = htonl((buf[i] << 24) + (buf[i + 1] << 16)
                                + (buf[i + 2] << 8) + (buf[i + 3]));

                if (++j == naddrs) {

#if (NGX_HAVE_INET6)
                    if (rn->naddrs6 == (u_short) -1) {
                        goto next;
                    }
#endif

                    break;
                }
            }

#if (NGX_HAVE_INET6)
            else if (type == NGX_RESOLVE_AAAA) {

                ngx_memcpy(addr6[j].s6_addr, &buf[i], 16);

                if (++j == naddrs) {

                    if (rn->naddrs == (u_short) -1) {
                        goto next;
                    }

                    break;
                }
            }
#endif

            i += len;
        }
    }

    switch (qtype) {

#if (NGX_HAVE_INET6)
    case NGX_RESOLVE_AAAA:

        if (rn->naddrs6 == (u_short) -1) {
            rn->naddrs6 = 0;
        }

        break;
#endif

    default: /* NGX_RESOLVE_A */

        if (rn->naddrs == (u_short) -1) {
            rn->naddrs = 0;
        }
    }

    if (rn->naddrs != (u_short) -1
#if (NGX_HAVE_INET6)
        && rn->naddrs6 != (u_short) -1
#endif
        && rn->naddrs
#if (NGX_HAVE_INET6)
           + rn->naddrs6
#endif
           > 0)
    {

#if (NGX_HAVE_INET6)
    export:
#endif

        naddrs = rn->naddrs;
#if (NGX_HAVE_INET6)
        naddrs += rn->naddrs6;
#endif

        if (naddrs == 1 && rn->naddrs == 1) {
            addrs = NULL;

        } else {
            addrs = ngx_resolver_export(r, rn, 0);
            if (addrs == NULL) {
                goto failed;
            }
        }

        ngx_queue_remove(&rn->queue);

        rn->valid = ngx_time() + (r->valid ? r->valid : (time_t) rn->ttl);
        rn->expire = ngx_time() + r->expire;

        ngx_queue_insert_head(&r->name_expire_queue, &rn->queue);

        next = rn->waiting;
        rn->waiting = NULL;

        /* unlock name mutex */

        while (next) {
            ctx = next;
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
        }

        if (addrs != NULL) {
            ngx_resolver_free(r, addrs->sockaddr);
            ngx_resolver_free(r, addrs);
        }

        ngx_resolver_free(r, rn->query);
        rn->query = NULL;
#if (NGX_HAVE_INET6)
        rn->query6 = NULL;
#endif

        return;
    }

    if (cname) {

        /* CNAME only */

        if (rn->naddrs == (u_short) -1
#if (NGX_HAVE_INET6)
            || rn->naddrs6 == (u_short) -1
#endif
            )
        {
            goto next;
        }

        if (ngx_resolver_copy(r, &name, buf, cname, buf + n) != NGX_OK) {
            goto failed;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0,
                       "resolver cname:\"%V\"", &name);

        ngx_queue_remove(&rn->queue);

        rn->cnlen = (u_short) name.len;
        rn->u.cname = name.data;

        rn->valid = ngx_time() + (r->valid ? r->valid : (time_t) rn->ttl);
        rn->expire = ngx_time() + r->expire;

        ngx_queue_insert_head(&r->name_expire_queue, &rn->queue);

        ngx_resolver_free(r, rn->query);
        rn->query = NULL;
#if (NGX_HAVE_INET6)
        rn->query6 = NULL;
#endif

        ctx = rn->waiting;
        rn->waiting = NULL;

        if (ctx) {

            if (ctx->recursion++ >= NGX_RESOLVER_MAX_RECURSION) {

                /* unlock name mutex */

                do {
                    ctx->state = NGX_RESOLVE_NXDOMAIN;
                    next = ctx->next;

                    ctx->handler(ctx);

                    ctx = next;
                } while (ctx);

                return;
            }

            for (next = ctx; next; next = next->next) {
                next->node = NULL;
            }

            (void) ngx_resolve_name_locked(r, ctx, &name);
        }

        /* unlock name mutex */

        return;
    }

    ngx_log_error(r->log_level, r->log, 0,
                  "no A or CNAME types in DNS response");
    return;

short_response:

    err = "short DNS response";

invalid:

    /* unlock name mutex */

    ngx_log_error(r->log_level, r->log, 0, err);

    return;

failed:

next:

    /* unlock name mutex */

    return;
}
{% endhighlight %}

此函数用于处理```由域名获得IPv4（或IPv6)地址```的解析。下面简单介绍一下函数的处理流程：
{% highlight string %}
static void
ngx_resolver_process_a(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t ident, ngx_uint_t code, ngx_uint_t qtype,
    ngx_uint_t nan, ngx_uint_t trunc, ngx_uint_t ans)
{
	//1) 调用ngx_resolver_copy()分离出查询的域名name

	//2) 调用ngx_resolver_lookup_name()查找对应的ngx_resolver_node_t节点

	//3) 根据qtype获得当前的查询标识符(qident), 并判断我们本地保存的查询标识符与DNS返回的标识符是否一样，不一样的话表示响应有问题

	//4) 处理trunc情况，表示有截断，此时可能需要重新发起查询请求

	//5) 如果code==0 且 nan==0(即应答资源记录数为0），则一般情况表示出现了异常，此种情况会将ctx->state设置为相应的异常状态，
	// 直接调用ctx->handler()来处理

	//6) 如果有相应的资源记录数，即nan不为0，那么
	if nan > 0{
		for(i = 0; i<nan; i++)
		{
			//6.1) 解析应答头部ngx_resolver_an_t
			//6.2) 获得返回的IP地址数
		}
		// 6.3 然后, 解析返回的IP地址
	}
	
	//调用ngx_resolver_export()导出相应的IP地址到ctx->addrs

	//7) 处理cname不为NULL的情况， 此时表示还返回了所查询域名的规范名称，这种情况下会再调用
	// ngx_resolve_name_locked()进行递归查询
}
{% endhighlight %}


## 9. 函数ngx_resolver_process_srv()
{% highlight string %}
static void
ngx_resolver_process_srv(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t ident, ngx_uint_t code, ngx_uint_t nan,
    ngx_uint_t trunc, ngx_uint_t ans)
{
    char                       *err;
    u_char                     *cname;
    size_t                      len;
    int32_t                     ttl;
    uint32_t                    hash;
    ngx_str_t                   name;
    ngx_uint_t                  type, qident, class, start, nsrvs, a, i, j;
    ngx_resolver_an_t          *an;
    ngx_resolver_ctx_t         *ctx, *next;
    ngx_resolver_srv_t         *srvs;
    ngx_resolver_node_t        *rn;
    ngx_resolver_connection_t  *rec;

    if (ngx_resolver_copy(r, &name, buf,
                          buf + sizeof(ngx_resolver_hdr_t), buf + n)
        != NGX_OK)
    {
        return;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0, "resolver qs:%V", &name);

    hash = ngx_crc32_short(name.data, name.len);

    rn = ngx_resolver_lookup_srv(r, &name, hash);

    if (rn == NULL || rn->query == NULL) {
        ngx_log_error(r->log_level, r->log, 0,
                      "unexpected response for %V", &name);
        ngx_resolver_free(r, name.data);
        goto failed;
    }

    if (trunc && rn->tcp) {
        ngx_resolver_free(r, name.data);
        goto failed;
    }

    qident = (rn->query[0] << 8) + rn->query[1];

    if (ident != qident) {
        ngx_log_error(r->log_level, r->log, 0,
                      "wrong ident %ui response for %V, expect %ui",
                      ident, &name, qident);
        ngx_resolver_free(r, name.data);
        goto failed;
    }

    ngx_resolver_free(r, name.data);

    if (trunc) {

        ngx_queue_remove(&rn->queue);

        if (rn->waiting == NULL) {
            ngx_rbtree_delete(&r->srv_rbtree, &rn->node);
            ngx_resolver_free_node(r, rn);
            return;
        }

        rec = r->connections.elts;
        rec = &rec[rn->last_connection];

        rn->tcp = 1;

        (void) ngx_resolver_send_tcp_query(r, rec, rn->query, rn->qlen);

        rn->expire = ngx_time() + r->resend_timeout;

        ngx_queue_insert_head(&r->srv_resend_queue, &rn->queue);

        return;
    }

    if (code == 0 && rn->code) {
        code = rn->code;
    }

    if (code == 0 && nan == 0) {
        code = NGX_RESOLVE_NXDOMAIN;
    }

    if (code) {
        next = rn->waiting;
        rn->waiting = NULL;

        ngx_queue_remove(&rn->queue);

        ngx_rbtree_delete(&r->srv_rbtree, &rn->node);

        while (next) {
            ctx = next;
            ctx->state = code;
            ctx->valid = ngx_time() + (r->valid ? r->valid : 10);
            next = ctx->next;

            ctx->handler(ctx);
        }

        ngx_resolver_free_node(r, rn);

        return;
    }

    i = ans;
    nsrvs = 0;
    cname = NULL;

    for (a = 0; a < nan; a++) {

        start = i;

        while (i < n) {

            if (buf[i] & 0xc0) {
                i += 2;
                goto found;
            }

            if (buf[i] == 0) {
                i++;
                goto test_length;
            }

            i += 1 + buf[i];
        }

        goto short_response;

    test_length:

        if (i - start < 2) {
            err = "invalid name DNS response";
            goto invalid;
        }

    found:

        if (i + sizeof(ngx_resolver_an_t) >= n) {
            goto short_response;
        }

        an = (ngx_resolver_an_t *) &buf[i];

        type = (an->type_hi << 8) + an->type_lo;
        class = (an->class_hi << 8) + an->class_lo;
        len = (an->len_hi << 8) + an->len_lo;
        ttl = (an->ttl[0] << 24) + (an->ttl[1] << 16)
            + (an->ttl[2] << 8) + (an->ttl[3]);

        if (class != 1) {
            ngx_log_error(r->log_level, r->log, 0,
                          "unexpected RR class %ui", class);
            goto failed;
        }

        if (ttl < 0) {
            ttl = 0;
        }

        rn->ttl = ngx_min(rn->ttl, (uint32_t) ttl);

        i += sizeof(ngx_resolver_an_t);

        switch (type) {

        case NGX_RESOLVE_SRV:

            if (i + 6 > n) {
                goto short_response;
            }

            if (ngx_resolver_copy(r, NULL, buf, &buf[i + 6], buf + n)
                != NGX_OK)
            {
                goto failed;
            }

            nsrvs++;

            break;

        case NGX_RESOLVE_CNAME:

            cname = &buf[i];

            break;

        case NGX_RESOLVE_DNAME:

            break;

        default:

            ngx_log_error(r->log_level, r->log, 0,
                          "unexpected RR type %ui", type);
        }

        i += len;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_CORE, r->log, 0,
                   "resolver nsrvs:%ui cname:%p ttl:%uD",
                   nsrvs, cname, rn->ttl);

    if (nsrvs) {

        srvs = ngx_resolver_calloc(r, nsrvs * sizeof(ngx_resolver_srv_t));
        if (srvs == NULL) {
            goto failed;
        }

        rn->u.srvs = srvs;
        rn->nsrvs = (u_short) nsrvs;

        j = 0;
        i = ans;

        for (a = 0; a < nan; a++) {

            for ( ;; ) {

                if (buf[i] & 0xc0) {
                    i += 2;
                    break;
                }

                if (buf[i] == 0) {
                    i++;
                    break;
                }

                i += 1 + buf[i];
            }

            an = (ngx_resolver_an_t *) &buf[i];

            type = (an->type_hi << 8) + an->type_lo;
            len = (an->len_hi << 8) + an->len_lo;

            i += sizeof(ngx_resolver_an_t);

            if (type == NGX_RESOLVE_SRV) {

                srvs[j].priority = (buf[i] << 8) + buf[i + 1];
                srvs[j].weight = (buf[i + 2] << 8) + buf[i + 3];

                if (srvs[j].weight == 0) {
                    srvs[j].weight = 1;
                }

                srvs[j].port = (buf[i + 4] << 8) + buf[i + 5];

                if (ngx_resolver_copy(r, &srvs[j].name, buf, &buf[i + 6],
                                      buf + n)
                    != NGX_OK)
                {
                    goto failed;
                }

                j++;
            }

            i += len;
        }

        ngx_sort(srvs, nsrvs, sizeof(ngx_resolver_srv_t),
                 ngx_resolver_cmp_srvs);

        ngx_resolver_free(r, rn->query);
        rn->query = NULL;

        ngx_queue_remove(&rn->queue);

        rn->valid = ngx_time() + (r->valid ? r->valid : (time_t) rn->ttl);
        rn->expire = ngx_time() + r->expire;

        ngx_queue_insert_head(&r->srv_expire_queue, &rn->queue);

        next = rn->waiting;
        rn->waiting = NULL;

        while (next) {
            ctx = next;
            next = ctx->next;

            ngx_resolver_resolve_srv_names(ctx, rn);
        }

        return;
    }

    rn->nsrvs = 0;

    if (cname) {

        /* CNAME only */

        if (ngx_resolver_copy(r, &name, buf, cname, buf + n) != NGX_OK) {
            goto failed;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0,
                       "resolver cname:\"%V\"", &name);

        ngx_queue_remove(&rn->queue);

        rn->cnlen = (u_short) name.len;
        rn->u.cname = name.data;

        rn->valid = ngx_time() + (r->valid ? r->valid : (time_t) rn->ttl);
        rn->expire = ngx_time() + r->expire;

        ngx_queue_insert_head(&r->srv_expire_queue, &rn->queue);

        ngx_resolver_free(r, rn->query);
        rn->query = NULL;
#if (NGX_HAVE_INET6)
        rn->query6 = NULL;
#endif

        ctx = rn->waiting;
        rn->waiting = NULL;

        if (ctx) {

            if (ctx->recursion++ >= NGX_RESOLVER_MAX_RECURSION) {

                /* unlock name mutex */

                do {
                    ctx->state = NGX_RESOLVE_NXDOMAIN;
                    next = ctx->next;

                    ctx->handler(ctx);

                    ctx = next;
                } while (ctx);

                return;
            }

            for (next = ctx; next; next = next->next) {
                next->node = NULL;
            }

            (void) ngx_resolve_name_locked(r, ctx, &name);
        }

        /* unlock name mutex */

        return;
    }

    ngx_log_error(r->log_level, r->log, 0, "no SRV type in DNS response");

    return;

short_response:

    err = "short DNS response";

invalid:

    /* unlock name mutex */

    ngx_log_error(r->log_level, r->log, 0, err);

    return;

failed:

    /* unlock name mutex */

    return;
}
{% endhighlight %}

此函数用于处理```由服务名获得IPv4(或IPv6)地址```的解析。下面简要介绍一下函数的处理流程：
{% highlight string %}
static void
ngx_resolver_process_srv(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t ident, ngx_uint_t code, ngx_uint_t nan,
    ngx_uint_t trunc, ngx_uint_t ans)
{
	//1) 调用ngx_resolver_copy()分离出查询的域名name

	//2) 调用ngx_resolver_lookup_srv()查找对应的ngx_resolver_node_t节点

	//3) 根据qtype获得当前的查询标识符(qident), 并判断我们本地保存的查询标识符与DNS返回的标识符是否一样，不一样的话表示响应有问题

	//4) 处理trunc情况，表示有截断，此时可能需要重新发起查询请求

	//5) 如果code==0 且 nan==0(即应答资源记录数为0），则一般情况表示出现了异常，此种情况会将ctx->state设置为相应的异常状态，
	// 直接调用ctx->handler()来处理

	//6) 解析应答报文，获取srvs的个数

	//7) 假如srvs个数大于0， 
	// 7.1) 循环解析应答报文，获得所查询服务的domain name、优先级、权重、端口等等
	// 7.2) 对返回的srvs进行排序，并将该ngx_resolver_node_t节点插入到resolver的srv_expire_queue队列中
	// 7.3) 接着再根据服务的domain name等查询具体的IP地址等信息（通过调用ngx_resolver_resolve_srv_names())

	//8) 如果cname不为NULL，将rn->nsrvs设置为0，表示返回的是规范化名称CNAME，此时可能需要再采用该canonical name
	//来进行继续的解析(注意： 在解析时可能产生递归，这里判别当出现NGX_RESOLVER_MAX_RECURSION次递归时，直接结束查询)
	
}
{% endhighlight %}

## 10. 函数ngx_resolver_resolve_srv_names()
{% highlight string %}
static void
ngx_resolver_resolve_srv_names(ngx_resolver_ctx_t *ctx, ngx_resolver_node_t *rn)
{
    ngx_uint_t                i;
    ngx_resolver_t           *r;
    ngx_resolver_ctx_t       *cctx;
    ngx_resolver_srv_name_t  *srvs;

    r = ctx->resolver;

    ctx->node = NULL;
    ctx->state = NGX_OK;
    ctx->valid = rn->valid;
    ctx->count = rn->nsrvs;

    srvs = ngx_resolver_calloc(r, rn->nsrvs * sizeof(ngx_resolver_srv_name_t));
    if (srvs == NULL) {
        goto failed;
    }

    ctx->srvs = srvs;
    ctx->nsrvs = rn->nsrvs;

    for (i = 0; i < rn->nsrvs; i++) {
        srvs[i].name.data = ngx_resolver_alloc(r, rn->u.srvs[i].name.len);
        if (srvs[i].name.data == NULL) {
            goto failed;
        }

        srvs[i].name.len = rn->u.srvs[i].name.len;
        ngx_memcpy(srvs[i].name.data, rn->u.srvs[i].name.data,
                   srvs[i].name.len);

        cctx = ngx_resolve_start(r, NULL);
        if (cctx == NULL) {
            goto failed;
        }

        cctx->name = srvs[i].name;
        cctx->handler = ngx_resolver_srv_names_handler;
        cctx->data = ctx;
        cctx->srvs = &srvs[i];
        cctx->timeout = 0;

        srvs[i].priority = rn->u.srvs[i].priority;
        srvs[i].weight = rn->u.srvs[i].weight;
        srvs[i].port = rn->u.srvs[i].port;
        srvs[i].ctx = cctx;

        if (ngx_resolve_name(cctx) == NGX_ERROR) {
            srvs[i].ctx = NULL;
            goto failed;
        }
    }

    return;

failed:

    ctx->state = NGX_ERROR;
    ctx->valid = ngx_time() + (r->valid ? r->valid : 10);

    ctx->handler(ctx);
}
{% endhighlight %}
此函数用于解析```rn->u.srvs```，即将```规范化服务名称解析为IP地址```。下面简要讲解一下本函数的实现流程：
{% highlight string %}
static void
ngx_resolver_resolve_srv_names(ngx_resolver_ctx_t *ctx, ngx_resolver_node_t *rn)
{
	//循环rn->nsrvs，重新创建ngx_resolver_ctx_t来进行'规范化服务名称解析为IP地址'
	for (i = 0; i < rn->nsrvs; i++) {
		//创建新的ctx对象
		cctx = ngx_resolve_start(r, NULL);

		//使用新的ctx完成解析
		ngx_resolve_name()
	}
}
{% endhighlight %}


## 11. 函数ngx_resolver_srv_names_handler()
{% highlight string %}
static void
ngx_resolver_srv_names_handler(ngx_resolver_ctx_t *cctx)
{
    ngx_uint_t                 i;
    u_char                   (*sockaddr)[NGX_SOCKADDRLEN];
    ngx_addr_t                *addrs;
    ngx_resolver_t            *r;
    struct sockaddr_in        *sin;
    ngx_resolver_ctx_t        *ctx;
    ngx_resolver_srv_name_t   *srv;
#if (NGX_HAVE_INET6)
    struct sockaddr_in6       *sin6;
#endif

    r = cctx->resolver;
    ctx = cctx->data;
    srv = cctx->srvs;

    ctx->count--;

    srv->ctx = NULL;

    if (cctx->naddrs) {

        ctx->valid = ngx_min(ctx->valid, cctx->valid);

        addrs = ngx_resolver_calloc(r, cctx->naddrs * sizeof(ngx_addr_t));
        if (addrs == NULL) {
            ngx_resolve_name_done(cctx);

            ctx->state = NGX_ERROR;
            ctx->valid = ngx_time() + (r->valid ? r->valid : 10);

            ctx->handler(ctx);
            return;
        }

        sockaddr = ngx_resolver_alloc(r, cctx->naddrs * NGX_SOCKADDRLEN);
        if (sockaddr == NULL) {
            ngx_resolver_free(r, addrs);
            ngx_resolve_name_done(cctx);

            ctx->state = NGX_ERROR;
            ctx->valid = ngx_time() + (r->valid ? r->valid : 10);

            ctx->handler(ctx);
            return;
        }

        for (i = 0; i < cctx->naddrs; i++) {
            addrs[i].sockaddr = (struct sockaddr *) sockaddr[i];
            addrs[i].socklen = cctx->addrs[i].socklen;

            ngx_memcpy(sockaddr[i], cctx->addrs[i].sockaddr,
                       addrs[i].socklen);

            switch (addrs[i].sockaddr->sa_family) {
#if (NGX_HAVE_INET6)
            case AF_INET6:
                sin6 = (struct sockaddr_in6 *) addrs[i].sockaddr;
                sin6->sin6_port = htons(srv->port);
                break;
#endif
            default: /* AF_INET */
                sin = (struct sockaddr_in *) addrs[i].sockaddr;
                sin->sin_port = htons(srv->port);
            }
        }

        srv->addrs = addrs;
        srv->naddrs = cctx->naddrs;
    }

    ngx_resolve_name_done(cctx);

    if (ctx->count == 0) {
        ngx_resolver_report_srv(r, ctx);
    }
}
{% endhighlight %}
本函数是将```规范化服务名称解析为IP地址```完成后的回调函数。涉及到两个context: 首先```context_1```将尝试将服务名解析为```IP地址```，但是只返回了```规范化服务名称```； 接着会创建一个新的```context_2```来将该规范化服务名成解析为```IP地址```。这里可以认为context_1是context_2的父上下文。

基本实现步骤如下：
{% highlight string %}
static void
ngx_resolver_srv_names_handler(ngx_resolver_ctx_t *cctx)
{
	//1) 如果成功解析到了IP地址
	if (cctx->naddrs) {
		
		//将解析到的IP地址保存到cctx->srvs->addrs里面(一个规范化名称，可能会对应多个IP地址)

	}

	//2) 执行ngx_resolve_name_done(cctx)，表明当前context已经处理完，进行相应的收尾工作

	//3) 如果cctx->data所表示的context的count为0，表示已经完成了所有规范名称的IP地址转换，此时ngx_resolver_report_srv()
}
{% endhighlight %}


## 12. 函数ngx_resolver_process_ptr()
{% highlight string %}
static void
ngx_resolver_process_ptr(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t ident, ngx_uint_t code, ngx_uint_t nan)
{
    char                 *err;
    size_t                len;
    in_addr_t             addr;
    int32_t               ttl;
    ngx_int_t             octet;
    ngx_str_t             name;
    ngx_uint_t            mask, type, class, qident, a, i, start;
    ngx_queue_t          *expire_queue;
    ngx_rbtree_t         *tree;
    ngx_resolver_an_t    *an;
    ngx_resolver_ctx_t   *ctx, *next;
    ngx_resolver_node_t  *rn;
#if (NGX_HAVE_INET6)
    uint32_t              hash;
    ngx_int_t             digit;
    struct in6_addr       addr6;
#endif

    if (ngx_resolver_copy(r, &name, buf,
                          buf + sizeof(ngx_resolver_hdr_t), buf + n)
        != NGX_OK)
    {
        return;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0, "resolver qs:%V", &name);

    /* AF_INET */

    addr = 0;
    i = sizeof(ngx_resolver_hdr_t);

    for (mask = 0; mask < 32; mask += 8) {
        len = buf[i++];

        octet = ngx_atoi(&buf[i], len);
        if (octet == NGX_ERROR || octet > 255) {
            goto invalid_in_addr_arpa;
        }

        addr += octet << mask;
        i += len;
    }

    if (ngx_strcasecmp(&buf[i], (u_char *) "\7in-addr\4arpa") == 0) {
        i += sizeof("\7in-addr\4arpa");

        /* lock addr mutex */

        rn = ngx_resolver_lookup_addr(r, addr);

        tree = &r->addr_rbtree;
        expire_queue = &r->addr_expire_queue;

        goto valid;
    }

invalid_in_addr_arpa:

#if (NGX_HAVE_INET6)

    i = sizeof(ngx_resolver_hdr_t);

    for (octet = 15; octet >= 0; octet--) {
        if (buf[i++] != '\1') {
            goto invalid_ip6_arpa;
        }

        digit = ngx_hextoi(&buf[i++], 1);
        if (digit == NGX_ERROR) {
            goto invalid_ip6_arpa;
        }

        addr6.s6_addr[octet] = (u_char) digit;

        if (buf[i++] != '\1') {
            goto invalid_ip6_arpa;
        }

        digit = ngx_hextoi(&buf[i++], 1);
        if (digit == NGX_ERROR) {
            goto invalid_ip6_arpa;
        }

        addr6.s6_addr[octet] += (u_char) (digit * 16);
    }

    if (ngx_strcasecmp(&buf[i], (u_char *) "\3ip6\4arpa") == 0) {
        i += sizeof("\3ip6\4arpa");

        /* lock addr mutex */

        hash = ngx_crc32_short(addr6.s6_addr, 16);
        rn = ngx_resolver_lookup_addr6(r, &addr6, hash);

        tree = &r->addr6_rbtree;
        expire_queue = &r->addr6_expire_queue;

        goto valid;
    }

invalid_ip6_arpa:
#endif

    ngx_log_error(r->log_level, r->log, 0,
                  "invalid in-addr.arpa or ip6.arpa name in DNS response");
    ngx_resolver_free(r, name.data);
    return;

valid:

    if (rn == NULL || rn->query == NULL) {
        ngx_log_error(r->log_level, r->log, 0,
                      "unexpected response for %V", &name);
        ngx_resolver_free(r, name.data);
        goto failed;
    }

    qident = (rn->query[0] << 8) + rn->query[1];

    if (ident != qident) {
        ngx_log_error(r->log_level, r->log, 0,
                      "wrong ident %ui response for %V, expect %ui",
                      ident, &name, qident);
        ngx_resolver_free(r, name.data);
        goto failed;
    }

    ngx_resolver_free(r, name.data);

    if (code == 0 && nan == 0) {
        code = NGX_RESOLVE_NXDOMAIN;
    }

    if (code) {
        next = rn->waiting;
        rn->waiting = NULL;

        ngx_queue_remove(&rn->queue);

        ngx_rbtree_delete(tree, &rn->node);

        /* unlock addr mutex */

        while (next) {
            ctx = next;
            ctx->state = code;
            ctx->valid = ngx_time() + (r->valid ? r->valid : 10);
            next = ctx->next;

            ctx->handler(ctx);
        }

        ngx_resolver_free_node(r, rn);

        return;
    }

    i += sizeof(ngx_resolver_qs_t);

    for (a = 0; a < nan; a++) {

        start = i;

        while (i < n) {

            if (buf[i] & 0xc0) {
                i += 2;
                goto found;
            }

            if (buf[i] == 0) {
                i++;
                goto test_length;
            }

            i += 1 + buf[i];
        }

        goto short_response;

    test_length:

        if (i - start < 2) {
            err = "invalid name in DNS response";
            goto invalid;
        }

    found:

        if (i + sizeof(ngx_resolver_an_t) >= n) {
            goto short_response;
        }

        an = (ngx_resolver_an_t *) &buf[i];

        type = (an->type_hi << 8) + an->type_lo;
        class = (an->class_hi << 8) + an->class_lo;
        len = (an->len_hi << 8) + an->len_lo;
        ttl = (an->ttl[0] << 24) + (an->ttl[1] << 16)
            + (an->ttl[2] << 8) + (an->ttl[3]);

        if (class != 1) {
            ngx_log_error(r->log_level, r->log, 0,
                          "unexpected RR class %ui", class);
            goto failed;
        }

        if (ttl < 0) {
            ttl = 0;
        }

        ngx_log_debug3(NGX_LOG_DEBUG_CORE, r->log, 0,
                      "resolver qt:%ui cl:%ui len:%uz",
                      type, class, len);

        i += sizeof(ngx_resolver_an_t);

        switch (type) {

        case NGX_RESOLVE_PTR:

            goto ptr;

        case NGX_RESOLVE_CNAME:

            break;

        default:

            ngx_log_error(r->log_level, r->log, 0,
                          "unexpected RR type %ui", type);
        }

        i += len;
    }

    /* unlock addr mutex */

    ngx_log_error(r->log_level, r->log, 0,
                  "no PTR type in DNS response");
    return;

ptr:

    if (ngx_resolver_copy(r, &name, buf, buf + i, buf + n) != NGX_OK) {
        goto failed;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, r->log, 0, "resolver an:%V", &name);

    if (name.len != (size_t) rn->nlen
        || ngx_strncmp(name.data, rn->name, name.len) != 0)
    {
        if (rn->nlen) {
            ngx_resolver_free(r, rn->name);
        }

        rn->nlen = (u_short) name.len;
        rn->name = name.data;

        name.data = ngx_resolver_dup(r, rn->name, name.len);
        if (name.data == NULL) {
            goto failed;
        }
    }

    ngx_queue_remove(&rn->queue);

    rn->valid = ngx_time() + (r->valid ? r->valid : ttl);
    rn->expire = ngx_time() + r->expire;

    ngx_queue_insert_head(expire_queue, &rn->queue);

    next = rn->waiting;
    rn->waiting = NULL;

    /* unlock addr mutex */

    while (next) {
        ctx = next;
        ctx->state = NGX_OK;
        ctx->valid = rn->valid;
        ctx->name = name;
        next = ctx->next;

        ctx->handler(ctx);
    }

    ngx_resolver_free(r, name.data);

    return;

short_response:

    err = "short DNS response";

invalid:

    /* unlock addr mutex */

    ngx_log_error(r->log_level, r->log, 0, err);

    return;

failed:

    /* unlock addr mutex */

    return;
}
{% endhighlight %}
此函数用于处理```将IP地址转换为域名```的响应。下面简要介绍一下函数的实现流程：
{% highlight string %}
static void
ngx_resolver_process_ptr(ngx_resolver_t *r, u_char *buf, size_t n,
    ngx_uint_t ident, ngx_uint_t code, ngx_uint_t nan)
{
	//1) 调用ngx_resolver_copy()分离出查询的域名name,

	//2) 从response的buf中解析出IP地址

	//3) 通过ngx_resolver_lookup_addr查询出本地保存的标识符qident与查询返回的标识符是否一致，如果不一样的话表示响应有问题

	//4) 如果code不为0， 则一般情况表示出现了异常， 打印相应的错误退出

	//5) 若nan>0，表示有相应的应答资源记录数，那么解析出对应的name，接着回调rn->waiting上下文列表中的handler()
	ngx_resolver_copy(r, &name, buf, buf + i, buf + n)；
}
{% endhighlight %}

<br />
<br />

**[参看]**

1. [Nginx DNS resolver配置实例](http://m.iis7.com/a/nr/082118.html)


2. [nginx关于域名解析的源码分析](https://blog.csdn.net/ChuiGeDaQiQiu/article/details/78842744?utm_source=blogxgwz7)

3. [DNS协议详解及报文格式分析](https://blog.csdn.net/tianxuhong/article/details/74922454)




<br />
<br />
<br />

