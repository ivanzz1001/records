---
layout: post
title: event/ngx_event_pipe.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---

nginx_event_pipe用于实现upstream对上游服务器包体数据的读取，然后在处理之后，将结果返回给请求端(downstream)。




<!-- more -->


## 1. 相关静态函数声明
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_pipe.h>


//读取来自上游服务器的数据
static ngx_int_t ngx_event_pipe_read_upstream(ngx_event_pipe_t *p);

//将处理后的数据发送到下游客户端
static ngx_int_t ngx_event_pipe_write_to_downstream(ngx_event_pipe_t *p);

//将p->in或p->buf_to_file缓存中的数据写到临时文件
static ngx_int_t ngx_event_pipe_write_chain_to_temp_file(ngx_event_pipe_t *p);

//移除buf上的shadow
static ngx_inline void ngx_event_pipe_remove_shadow_links(ngx_buf_t *buf);

//busy, out, in 三个缓冲chain释放， 同时释放shadow缓冲，并将空闲的buffer加入到p->free_raw_bufs中
static ngx_int_t ngx_event_pipe_drain_chains(ngx_event_pipe_t *p);
{% endhighlight %}


## 2. 函数ngx_event_pipe()
{% highlight string %}
ngx_int_t
ngx_event_pipe(ngx_event_pipe_t *p, ngx_int_t do_write)
{
    ngx_int_t     rc;
    ngx_uint_t    flags;
    ngx_event_t  *rev, *wev;

    for ( ;; ) {
        if (do_write) {
            p->log->action = "sending to client";

            rc = ngx_event_pipe_write_to_downstream(p);

            if (rc == NGX_ABORT) {
                return NGX_ABORT;
            }

            if (rc == NGX_BUSY) {
                return NGX_OK;
            }
        }

        p->read = 0;
        p->upstream_blocked = 0;

        p->log->action = "reading upstream";

        if (ngx_event_pipe_read_upstream(p) == NGX_ABORT) {
            return NGX_ABORT;
        }

        if (!p->read && !p->upstream_blocked) {
            break;
        }

        do_write = 1;
    }

    if (p->upstream->fd != (ngx_socket_t) -1) {
        rev = p->upstream->read;

        flags = (rev->eof || rev->error) ? NGX_CLOSE_EVENT : 0;

        if (ngx_handle_read_event(rev, flags) != NGX_OK) {
            return NGX_ABORT;
        }

        if (!rev->delayed) {
            if (rev->active && !rev->ready) {
                ngx_add_timer(rev, p->read_timeout);

            } else if (rev->timer_set) {
                ngx_del_timer(rev);
            }
        }
    }

    if (p->downstream->fd != (ngx_socket_t) -1
        && p->downstream->data == p->output_ctx)
    {
        wev = p->downstream->write;
        if (ngx_handle_write_event(wev, p->send_lowat) != NGX_OK) {
            return NGX_ABORT;
        }

        if (!wev->delayed) {
            if (wev->active && !wev->ready) {
                ngx_add_timer(wev, p->send_timeout);

            } else if (wev->timer_set) {
                ngx_del_timer(wev);
            }
        }
    }

    return NGX_OK;
}
{% endhighlight %}
用于实现upstream对上游服务器包体数据的读取，然后在处理之后，将结果返回给请求端(downstream)。下面我们仔细分析一下函数的实现：
{% highlight string %}
ngx_int_t
ngx_event_pipe(ngx_event_pipe_t *p, ngx_int_t do_write)
{
	//通常情况下，假如p中原来就还有缓存数据，那么会将参数do_write设置为1，否则要等到ngx_event_pipe_read_upstream()
	//读取到数据之后，才会将do_write置为1
	for(;;){
		
		if (do_write) {

			//1) 包体数据写入到下游请求端
			rc = ngx_event_pipe_write_to_downstream(p);
			
			// 写入出错，直接返回
			if (rc == NGX_ABORT) {
				return NGX_ABORT;
			}
			
			// 来不及处理，此时不会再从上游服务器读数据，也不会再向下游请求端发送数据，直接返回
			if (rc == NGX_BUSY) {
				return NGX_OK;
			}

		}
		
		// 2） 此处p->read置为0，表示暂没有读取到上游服务器的数据； p->upstream_blocked表示暂时阻塞读取上游响应的流程
		p->read = 0;
		p->upstream_blocked = 0;

		//3) 读取upstream数据
		 if (ngx_event_pipe_read_upstream(p) == NGX_ABORT) {
            return NGX_ABORT;
        }
	
		//3) 在上游包体读取未阻塞状态下， 没有读取到数据，break跳出	
		if (!p->read && !p->upstream_blocked) {
			break;
		}

		//4) 将do_write标志位置为1，指示需要向下游请求端发送数据
		do_write = 1;

	}

	if (p->upstream->fd != (ngx_socket_t) -1) {
		//5)上游包体读取出错或者没有数据可读，那么事件会被清理， 否则什么也不做
		flags = (rev->eof || rev->error) ? NGX_CLOSE_EVENT : 0;
		
		if (ngx_handle_read_event(rev, flags) != NGX_OK) {
			return NGX_ABORT;
		}

		//6) rev->delayed用于标记由于速率限制(rate limiting)的原因导致IO被延迟，此处表示没有被延迟
		if (!rev->delayed) {
			if (rev->active && !rev->ready) {
				ngx_add_timer(rev, p->read_timeout);
		
			} else if (rev->timer_set) {
				ngx_del_timer(rev);
			}
		}
	}

	
}
{% endhighlight %}



<br />
<br />

**[参看]**


1. [《深入理解Nginx》笔记之ngx_event_pipe_s结构体](https://blog.csdn.net/yzt33/article/details/47835311)

2. [ngx_event_pipe_write_to_downstream分析](https://blog.csdn.net/kai_ding/article/details/21316493)

3. [ngx_event_pipe_read_upstream分析](https://blog.csdn.net/kai_ding/article/details/20404875)

4. [nginx upstream模块详解（基础结构篇）](https://blog.csdn.net/huzilinitachi/article/details/79543153)

5. [nginx upstream模块详解（处理流程篇二 upstream与event_pipe交互）](https://blog.csdn.net/huzilinitachi/article/details/79565365)

6. [nginx的upstream模块数据转发过程及流量控制分析](https://blog.csdn.net/zhaomangzheng/article/details/24390783)

7. [Nginx upstream原理分析-带buffering给客户端返回数据](http://chenzhenianqing.com/articles/689.html)




<br />
<br />
<br />

