---
layout: post
title: core/ngx_syslog.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们主要介绍一下nginx中syslog的实现。



<!-- more -->

## 1. 相关静态函数声明
{% highlight string %}
/*
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define NGX_SYSLOG_MAX_STR                                                    \
    NGX_MAX_ERROR_STR + sizeof("<255>Jan 01 00:00:00 ") - 1                   \
    + (NGX_MAXHOSTNAMELEN - 1) + 1 /* space */                                \
    + 32 /* tag */ + 2 /* colon, space */


//用于解析syslog配置参数
static char *ngx_syslog_parse_args(ngx_conf_t *cf, ngx_syslog_peer_t *peer);

//用于初始化与syslog服务器的连接
static ngx_int_t ngx_syslog_init_peer(ngx_syslog_peer_t *peer);

//作为syslog所关联内存池的清理函数
static void ngx_syslog_cleanup(void *data);
{% endhighlight %}

宏定义```NGX_SYSLOG_MAX_STR```用于计算最长的syslog日志的长度，从上面可以看到，其中MSG的```Content```部分的最长长度为```NGX_MAX_ERROR_STR```，及2048；接下来是```PRI```及```HEADER```部分的长度。


## 2. 相关静态变量的定义
{% highlight string %}
static char  *facilities[] = {
    "kern", "user", "mail", "daemon", "auth", "intern", "lpr", "news", "uucp",
    "clock", "authpriv", "ftp", "ntp", "audit", "alert", "cron", "local0",
    "local1", "local2", "local3", "local4", "local5", "local6", "local7",
    NULL
};

/* note 'error/warn' like in nginx.conf, not 'err/warning' */
static char  *severities[] = {
    "emerg", "alert", "crit", "error", "warn", "notice", "info", "debug", NULL
};

static ngx_log_t    ngx_syslog_dummy_log;
static ngx_event_t  ngx_syslog_dummy_event;
{% endhighlight %}

* facilities: 用于定义syslog的facility(模块）

* severities: 用于定义优先级

* ngx_syslog_dummy_log: 由于nginx syslog也是作为nginx普通日志模块(ngx_log_t)的一部分，因此这里作为dummy存在。


* ngx_syslog_dummy_event: 作为绑定syslog UDP发送数据的一个```伪event```存在。


## 3. 函数ngx_syslog_process_conf()
{% highlight string %}
char *
ngx_syslog_process_conf(ngx_conf_t *cf, ngx_syslog_peer_t *peer)
{
    peer->pool = cf->pool;
    peer->facility = NGX_CONF_UNSET_UINT;
    peer->severity = NGX_CONF_UNSET_UINT;

    if (ngx_syslog_parse_args(cf, peer) != NGX_CONF_OK) {
        return NGX_CONF_ERROR;
    }

    if (peer->server.sockaddr == NULL) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "no syslog server specified");
        return NGX_CONF_ERROR;
    }

    if (peer->facility == NGX_CONF_UNSET_UINT) {
        peer->facility = 23; /* local7 */
    }

    if (peer->severity == NGX_CONF_UNSET_UINT) {
        peer->severity = 6; /* info */
    }

    if (peer->tag.data == NULL) {
        ngx_str_set(&peer->tag, "nginx");
    }

    peer->conn.fd = (ngx_socket_t) -1;

    return NGX_CONF_OK;
}

{% endhighlight %}
此函数用于解析nginx配置文件中的syslog配置。下面我们给出几个syslog配置的示例，再来分析函数的实现：
<pre>
error_log syslog:server=192.168.1.1 debug;

access_log syslog:server=unix:/var/log/nginx.sock,nohostname;
access_log syslog:server=[2001:db8::1]:12345,facility=local7,tag=nginx,severity=info combined;
</pre>
本函数较为简单，首先直接调用ngx_syslog_parse_args()来进行解析，然后对未设置的值(facility、severity、tag)设置默认值。

## 4. 函数ngx_syslog_parse_args()
{% highlight string %}
static char *
ngx_syslog_parse_args(ngx_conf_t *cf, ngx_syslog_peer_t *peer)
{
    u_char      *p, *comma, c;
    size_t       len;
    ngx_str_t   *value;
    ngx_url_t    u;
    ngx_uint_t   i;

    value = cf->args->elts;

    p = value[1].data + sizeof("syslog:") - 1;

    for ( ;; ) {
        comma = (u_char *) ngx_strchr(p, ',');

        if (comma != NULL) {
            len = comma - p;
            *comma = '\0';

        } else {
            len = value[1].data + value[1].len - p;
        }

        if (ngx_strncmp(p, "server=", 7) == 0) {

            if (peer->server.sockaddr != NULL) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "duplicate syslog \"server\"");
                return NGX_CONF_ERROR;
            }

            ngx_memzero(&u, sizeof(ngx_url_t));

            u.url.data = p + 7;
            u.url.len = len - 7;
            u.default_port = 514;

            if (ngx_parse_url(cf->pool, &u) != NGX_OK) {
                if (u.err) {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "%s in syslog server \"%V\"",
                                       u.err, &u.url);
                }

                return NGX_CONF_ERROR;
            }

            peer->server = u.addrs[0];

        } else if (ngx_strncmp(p, "facility=", 9) == 0) {

            if (peer->facility != NGX_CONF_UNSET_UINT) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "duplicate syslog \"facility\"");
                return NGX_CONF_ERROR;
            }

            for (i = 0; facilities[i] != NULL; i++) {

                if (ngx_strcmp(p + 9, facilities[i]) == 0) {
                    peer->facility = i;
                    goto next;
                }
            }

            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "unknown syslog facility \"%s\"", p + 9);
            return NGX_CONF_ERROR;

        } else if (ngx_strncmp(p, "severity=", 9) == 0) {

            if (peer->severity != NGX_CONF_UNSET_UINT) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "duplicate syslog \"severity\"");
                return NGX_CONF_ERROR;
            }

            for (i = 0; severities[i] != NULL; i++) {

                if (ngx_strcmp(p + 9, severities[i]) == 0) {
                    peer->severity = i;
                    goto next;
                }
            }

            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "unknown syslog severity \"%s\"", p + 9);
            return NGX_CONF_ERROR;

        } else if (ngx_strncmp(p, "tag=", 4) == 0) {

            if (peer->tag.data != NULL) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "duplicate syslog \"tag\"");
                return NGX_CONF_ERROR;
            }

            /*
             * RFC 3164: the TAG is a string of ABNF alphanumeric characters
             * that MUST NOT exceed 32 characters.
             */
            if (len - 4 > 32) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "syslog tag length exceeds 32");
                return NGX_CONF_ERROR;
            }

            for (i = 4; i < len; i++) {
                c = ngx_tolower(p[i]);

                if (c < '0' || (c > '9' && c < 'a' && c != '_') || c > 'z') {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "syslog \"tag\" only allows "
                                       "alphanumeric characters "
                                       "and underscore");
                    return NGX_CONF_ERROR;
                }
            }

            peer->tag.data = p + 4;
            peer->tag.len = len - 4;

        } else if (len == 10 && ngx_strncmp(p, "nohostname", 10) == 0) {
            peer->nohostname = 1;

        } else {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "unknown syslog parameter \"%s\"", p);
            return NGX_CONF_ERROR;
        }

    next:

        if (comma == NULL) {
            break;
        }

        p = comma + 1;
    }

    return NGX_CONF_OK;
}
{% endhighlight %}
下面我们简要分析一下函数的实现：
{% highlight string %}
static char *
ngx_syslog_parse_args(ngx_conf_t *cf, ngx_syslog_peer_t *peer)
{
	//1) 定位到syslog:位置处
	p = value[1].data + sizeof("syslog:") - 1;

	for(;;){
		//2) 以逗号作分隔，解析syslog配置的各个部分
		
		if (ngx_strncmp(p, "server=", 7) == 0){

			//2.1) 解析server部分

		} else if (ngx_strncmp(p, "facility=", 9) == 0) {
			
			//2.2) 解析facility部分

		} else if (ngx_strncmp(p, "severity=", 9) == 0) {

			//2.3） 解析severity部分

		} else if (ngx_strncmp(p, "tag=", 4) == 0) {

			//2.4) 解析tag部分

		}else if (len == 10 && ngx_strncmp(p, "nohostname", 10) == 0) {
		
			//2.5) 解析nohostname部分

		}
	}
}
{% endhighlight %}


## 5. 函数ngx_syslog_add_header()
{% highlight string %}
u_char *
ngx_syslog_add_header(ngx_syslog_peer_t *peer, u_char *buf)
{
    ngx_uint_t  pri;

    pri = peer->facility * 8 + peer->severity;

    if (peer->nohostname) {
        return ngx_sprintf(buf, "<%ui>%V %V: ", pri, &ngx_cached_syslog_time,
                           &peer->tag);
    }

    return ngx_sprintf(buf, "<%ui>%V %V %V: ", pri, &ngx_cached_syslog_time,
                       &ngx_cycle->hostname, &peer->tag);
}
{% endhighlight %}
本函数用于构造syslog的header部分。可以注意到这里用了```ngx_cached_syslog_time```，因为nginx中很多地方都需要用到时间戳信息，但是如果每一次都调用系统函数去获取的话，则会造成系统性能降低，因此这里会采用缓存的时间。（缓存时间按一定频率进行更新）


## 6. 函数ngx_syslog_writer()
{% highlight string %}
void
ngx_syslog_writer(ngx_log_t *log, ngx_uint_t level, u_char *buf,
    size_t len)
{
    u_char             *p, msg[NGX_SYSLOG_MAX_STR];
    ngx_uint_t          head_len;
    ngx_syslog_peer_t  *peer;

    peer = log->wdata;

    if (peer->busy) {
        return;
    }

    peer->busy = 1;
    peer->severity = level - 1;

    p = ngx_syslog_add_header(peer, msg);
    head_len = p - msg;

    len -= NGX_LINEFEED_SIZE;

    if (len > NGX_SYSLOG_MAX_STR - head_len) {
        len = NGX_SYSLOG_MAX_STR - head_len;
    }

    p = ngx_snprintf(p, len, "%s", buf);

    (void) ngx_syslog_send(peer, msg, p - msg);

    peer->busy = 0;
}
{% endhighlight %}

本函数用于格式化```buf```中的内容： 添加上header信息，加上末尾的换行信息，对过长的日志进行截断。然后将格式化好的日志内容发送到syslog接收服务器

## 7. 函数ngx_syslog_send()
{% highlight string %}
ssize_t
ngx_syslog_send(ngx_syslog_peer_t *peer, u_char *buf, size_t len)
{
    ssize_t  n;

    if (peer->conn.fd == (ngx_socket_t) -1) {
        if (ngx_syslog_init_peer(peer) != NGX_OK) {
            return NGX_ERROR;
        }
    }

    /* log syslog socket events with valid log */
    peer->conn.log = ngx_cycle->log;

    if (ngx_send) {
        n = ngx_send(&peer->conn, buf, len);

    } else {
        /* event module has not yet set ngx_io */
        n = ngx_os_io.send(&peer->conn, buf, len);
    }

#if (NGX_HAVE_UNIX_DOMAIN)

    if (n == NGX_ERROR && peer->server.sockaddr->sa_family == AF_UNIX) {

        if (ngx_close_socket(peer->conn.fd) == -1) {
            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_socket_errno,
                          ngx_close_socket_n " failed");
        }

        peer->conn.fd = (ngx_socket_t) -1;
    }

#endif

    return n;
}
{% endhighlight %}
本函数用于向syslog接收服务器发送日志。


## 8. 函数ngx_syslog_init_peer()
{% highlight string %}
static ngx_int_t
ngx_syslog_init_peer(ngx_syslog_peer_t *peer)
{
    ngx_socket_t         fd;
    ngx_pool_cleanup_t  *cln;

    peer->conn.read = &ngx_syslog_dummy_event;
    peer->conn.write = &ngx_syslog_dummy_event;

    ngx_syslog_dummy_event.log = &ngx_syslog_dummy_log;

    fd = ngx_socket(peer->server.sockaddr->sa_family, SOCK_DGRAM, 0);
    if (fd == (ngx_socket_t) -1) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_socket_errno,
                      ngx_socket_n " failed");
        return NGX_ERROR;
    }

    if (ngx_nonblocking(fd) == -1) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_socket_errno,
                      ngx_nonblocking_n " failed");
        goto failed;
    }

    if (connect(fd, peer->server.sockaddr, peer->server.socklen) == -1) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_socket_errno,
                      "connect() failed");
        goto failed;
    }

    cln = ngx_pool_cleanup_add(peer->pool, 0);
    if (cln == NULL) {
        goto failed;
    }

    cln->data = peer;
    cln->handler = ngx_syslog_cleanup;

    peer->conn.fd = fd;

    /* UDP sockets are always ready to write */
    peer->conn.write->ready = 1;

    return NGX_OK;

failed:

    if (ngx_close_socket(fd) == -1) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_socket_errno,
                      ngx_close_socket_n " failed");
    }

    return NGX_ERROR;
}
{% endhighlight %}
本函数用于建立非阻塞的UDP socket，稍后采用该socket来发送日志信息。注意这里为该socket绑定了ngx_syslog_dummy_event。

## 9. 函数ngx_syslog_cleanup()
{% highlight string %}
static void
ngx_syslog_cleanup(void *data)
{
    ngx_syslog_peer_t  *peer = data;

    /* prevents further use of this peer */
    peer->busy = 1;

    if (peer->conn.fd == (ngx_socket_t) -1) {
        return;
    }

    if (ngx_close_socket(peer->conn.fd) == -1) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_socket_errno,
                      ngx_close_socket_n " failed");
    }
}

{% endhighlight %}
本函数用于处理在syslog绑定的内存池销毁时，如果对应的发送socket未关闭，则这里进行关闭。



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

