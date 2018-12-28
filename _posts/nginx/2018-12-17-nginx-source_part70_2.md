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

			if(rev->active && !rev->ready){
			
				//6.1) 此处添加定时器主要是为了预防读取上游服务器数据超时

			}else if (rev->timer_set){
				//6.2) 将上一次所设置的定时器移除
			}
		}
	}


	if (p->downstream->fd != (ngx_socket_t) -1
		&& p->downstream->data == p->output_ctx){

		//7) 将写事件添加到事件驱动机制
		wev = p->downstream->write;
		if (ngx_handle_write_event(wev, p->send_lowat) != NGX_OK) {
			return NGX_ABORT;
		}

		//8) wev->delayed用于标记由于速率限制(rate limiting)的原因导致IO被延迟，此处表示没有被延迟
		if (!wev->delayed) {

			if (wev->active && !wev->ready) {
			
				//8.1) 此处添加定时器主要是为了预防向下游服务器写数据超时

			} else if (wev->timer_set) {
				//8.2) 将上一次设置的定时器移除
			}
		}

	}

	return NGX_OK;	
}
{% endhighlight %}


## 3. 函数ngx_event_pipe_read_upstream()
{% highlight string %}
static ngx_int_t
ngx_event_pipe_read_upstream(ngx_event_pipe_t *p)
{
    off_t         limit;
    ssize_t       n, size;
    ngx_int_t     rc;
    ngx_buf_t    *b;
    ngx_msec_t    delay;
    ngx_chain_t  *chain, *cl, *ln;

    if (p->upstream_eof || p->upstream_error || p->upstream_done) {
        return NGX_OK;
    }

#if (NGX_THREADS)

    if (p->aio) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "pipe read upstream: aio");
        return NGX_AGAIN;
    }

    if (p->writing) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "pipe read upstream: writing");

        rc = ngx_event_pipe_write_chain_to_temp_file(p);

        if (rc != NGX_OK) {
            return rc;
        }
    }

#endif

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, p->log, 0,
                   "pipe read upstream: %d", p->upstream->read->ready);

    for ( ;; ) {

        if (p->upstream_eof || p->upstream_error || p->upstream_done) {
            break;
        }

        if (p->preread_bufs == NULL && !p->upstream->read->ready) {
            break;
        }

        if (p->preread_bufs) {

            /* use the pre-read bufs if they exist */

            chain = p->preread_bufs;
            p->preread_bufs = NULL;
            n = p->preread_size;

            ngx_log_debug1(NGX_LOG_DEBUG_EVENT, p->log, 0,
                           "pipe preread: %z", n);

            if (n) {
                p->read = 1;
            }

        } else {

#if (NGX_HAVE_KQUEUE)

            /*
             * kqueue notifies about the end of file or a pending error.
             * This test allows not to allocate a buf on these conditions
             * and not to call c->recv_chain().
             */

            if (p->upstream->read->available == 0
                && p->upstream->read->pending_eof)
            {
                p->upstream->read->ready = 0;
                p->upstream->read->eof = 1;
                p->upstream_eof = 1;
                p->read = 1;

                if (p->upstream->read->kq_errno) {
                    p->upstream->read->error = 1;
                    p->upstream_error = 1;
                    p->upstream_eof = 0;

                    ngx_log_error(NGX_LOG_ERR, p->log,
                                  p->upstream->read->kq_errno,
                                  "kevent() reported that upstream "
                                  "closed connection");
                }

                break;
            }
#endif

            if (p->limit_rate) {
                if (p->upstream->read->delayed) {
                    break;
                }

                limit = (off_t) p->limit_rate * (ngx_time() - p->start_sec + 1)
                        - p->read_length;

                if (limit <= 0) {
                    p->upstream->read->delayed = 1;
                    delay = (ngx_msec_t) (- limit * 1000 / p->limit_rate + 1);
                    ngx_add_timer(p->upstream->read, delay);
                    break;
                }

            } else {
                limit = 0;
            }

            if (p->free_raw_bufs) {

                /* use the free bufs if they exist */

                chain = p->free_raw_bufs;
                if (p->single_buf) {
                    p->free_raw_bufs = p->free_raw_bufs->next;
                    chain->next = NULL;
                } else {
                    p->free_raw_bufs = NULL;
                }

            } else if (p->allocated < p->bufs.num) {

                /* allocate a new buf if it's still allowed */

                b = ngx_create_temp_buf(p->pool, p->bufs.size);
                if (b == NULL) {
                    return NGX_ABORT;
                }

                p->allocated++;

                chain = ngx_alloc_chain_link(p->pool);
                if (chain == NULL) {
                    return NGX_ABORT;
                }

                chain->buf = b;
                chain->next = NULL;

            } else if (!p->cacheable
                       && p->downstream->data == p->output_ctx
                       && p->downstream->write->ready
                       && !p->downstream->write->delayed)
            {
                /*
                 * if the bufs are not needed to be saved in a cache and
                 * a downstream is ready then write the bufs to a downstream
                 */

                p->upstream_blocked = 1;

                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "pipe downstream ready");

                break;

            } else if (p->cacheable
                       || p->temp_file->offset < p->max_temp_file_size)
            {

                /*
                 * if it is allowed, then save some bufs from p->in
                 * to a temporary file, and add them to a p->out chain
                 */

                rc = ngx_event_pipe_write_chain_to_temp_file(p);

                ngx_log_debug1(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "pipe temp offset: %O", p->temp_file->offset);

                if (rc == NGX_BUSY) {
                    break;
                }

                if (rc != NGX_OK) {
                    return rc;
                }

                chain = p->free_raw_bufs;
                if (p->single_buf) {
                    p->free_raw_bufs = p->free_raw_bufs->next;
                    chain->next = NULL;
                } else {
                    p->free_raw_bufs = NULL;
                }

            } else {

                /* there are no bufs to read in */

                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "no pipe bufs to read in");

                break;
            }

            n = p->upstream->recv_chain(p->upstream, chain, limit);

            ngx_log_debug1(NGX_LOG_DEBUG_EVENT, p->log, 0,
                           "pipe recv chain: %z", n);

            if (p->free_raw_bufs) {
                chain->next = p->free_raw_bufs;
            }
            p->free_raw_bufs = chain;

            if (n == NGX_ERROR) {
                p->upstream_error = 1;
                return NGX_ERROR;
            }

            if (n == NGX_AGAIN) {
                if (p->single_buf) {
                    ngx_event_pipe_remove_shadow_links(chain->buf);
                }

                break;
            }

            p->read = 1;

            if (n == 0) {
                p->upstream_eof = 1;
                break;
            }
        }

        delay = p->limit_rate ? (ngx_msec_t) n * 1000 / p->limit_rate : 0;

        p->read_length += n;
        cl = chain;
        p->free_raw_bufs = NULL;

        while (cl && n > 0) {

            ngx_event_pipe_remove_shadow_links(cl->buf);

            size = cl->buf->end - cl->buf->last;

            if (n >= size) {
                cl->buf->last = cl->buf->end;

                /* STUB */ cl->buf->num = p->num++;

                if (p->input_filter(p, cl->buf) == NGX_ERROR) {
                    return NGX_ABORT;
                }

                n -= size;
                ln = cl;
                cl = cl->next;
                ngx_free_chain(p->pool, ln);

            } else {
                cl->buf->last += n;
                n = 0;
            }
        }

        if (cl) {
            for (ln = cl; ln->next; ln = ln->next) { /* void */ }

            ln->next = p->free_raw_bufs;
            p->free_raw_bufs = cl;
        }

        if (delay > 0) {
            p->upstream->read->delayed = 1;
            ngx_add_timer(p->upstream->read, delay);
            break;
        }
    }

#if (NGX_DEBUG)

    for (cl = p->busy; cl; cl = cl->next) {
        ngx_log_debug8(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "pipe buf busy s:%d t:%d f:%d "
                       "%p, pos %p, size: %z "
                       "file: %O, size: %O",
                       (cl->buf->shadow ? 1 : 0),
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);
    }

    for (cl = p->out; cl; cl = cl->next) {
        ngx_log_debug8(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "pipe buf out  s:%d t:%d f:%d "
                       "%p, pos %p, size: %z "
                       "file: %O, size: %O",
                       (cl->buf->shadow ? 1 : 0),
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);
    }

    for (cl = p->in; cl; cl = cl->next) {
        ngx_log_debug8(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "pipe buf in   s:%d t:%d f:%d "
                       "%p, pos %p, size: %z "
                       "file: %O, size: %O",
                       (cl->buf->shadow ? 1 : 0),
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);
    }

    for (cl = p->free_raw_bufs; cl; cl = cl->next) {
        ngx_log_debug8(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "pipe buf free s:%d t:%d f:%d "
                       "%p, pos %p, size: %z "
                       "file: %O, size: %O",
                       (cl->buf->shadow ? 1 : 0),
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);
    }

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, p->log, 0,
                   "pipe length: %O", p->length);

#endif

    if (p->free_raw_bufs && p->length != -1) {
        cl = p->free_raw_bufs;

        if (cl->buf->last - cl->buf->pos >= p->length) {

            p->free_raw_bufs = cl->next;

            /* STUB */ cl->buf->num = p->num++;

            if (p->input_filter(p, cl->buf) == NGX_ERROR) {
                return NGX_ABORT;
            }

            ngx_free_chain(p->pool, cl);
        }
    }

    if (p->length == 0) {
        p->upstream_done = 1;
        p->read = 1;
    }

    if ((p->upstream_eof || p->upstream_error) && p->free_raw_bufs) {

        /* STUB */ p->free_raw_bufs->buf->num = p->num++;

        if (p->input_filter(p, p->free_raw_bufs->buf) == NGX_ERROR) {
            return NGX_ABORT;
        }

        p->free_raw_bufs = p->free_raw_bufs->next;

        if (p->free_bufs && p->buf_to_file == NULL) {
            for (cl = p->free_raw_bufs; cl; cl = cl->next) {
                if (cl->buf->shadow == NULL) {
                    ngx_pfree(p->pool, cl->buf->start);
                }
            }
        }
    }

    if (p->cacheable && (p->in || p->buf_to_file)) {

        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "pipe write chain");

        rc = ngx_event_pipe_write_chain_to_temp_file(p);

        if (rc != NGX_OK) {
            return rc;
        }
    }

    return NGX_OK;
}
{% endhighlight %}
本函数用于读取上游服务器的包体数据，然后调用相应的方法进行处理。下面我们简要分析一下函数的实现：
{% highlight string %}
static ngx_int_t
ngx_event_pipe_read_upstream(ngx_event_pipe_t *p)
{
	//1) p->upstream_eof为1，表示上游服务器关闭了连接； p->upstream_error表示读取上游服务器数据出错； p->upstream_done
	//为1表示Nginx与上游交互已经结束，即上游包体数据读取完毕
	if (p->upstream_eof || p->upstream_error || p->upstream_done) {
        return NGX_OK;
    }

	//2) 多线程情况处理，目前我们不支持NGX_THREADS宏定义
	#if (NGX_THREADS)
	#endif

	//3) 从上游服务器读取数据
	for(;;){
		//3.1) 同上
		if (p->upstream_eof || p->upstream_error || p->upstream_done) {
			break;
		}
		
		if (p->preread_bufs == NULL && !p->upstream->read->ready) {
			break;
		}
	}
}
{% endhighlight %}

上面关于使用多线程写temp文件(ngx_write_chain_to_temp_file())，这里我们再多说一句：

![ngx-eventpipe-thread](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_eventpipe_thread.jpg)




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

