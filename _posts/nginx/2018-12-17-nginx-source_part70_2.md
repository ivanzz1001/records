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
		
		//3.2) 如果没有预先读取到包体数据(p->preread_bufs)，并且当前又读取不到新的数据，那么循环跳出
		if (p->preread_bufs == NULL && !p->upstream->read->ready) {
			break;
		}

		if(p->preread_bufs){
			//3.3) 处理预先读取到的包体数据
			chain = p->preread_bufs;
			p->preread_bufs = NULL;
			n = p->preread_size;
			
			ngx_log_debug1(NGX_LOG_DEBUG_EVENT, p->log, 0,
				"pipe preread: %z", n);
			
			if (n) {
				p->read = 1;			//此处p->read表示读取到了上游的响应
			}
		}else{

			#if (NGX_HAVE_KQUEUE)

				//3.4) kqueue notifies about the end of file or a pending error.This test allows not to 
				//allocate a buf on these conditions and not to call c->recv_chain().
          
			#endif

			//3.5) 对速率限制的处理
			if (p->limit_rate) {
				if (p->upstream->read->delayed) {
					break;
				}
			
				//这里我们看到，对速率处理的限制也比较简单，就是计算预计到当前最大接收的数据量：
				// p->limit_rate * (ngx_time() - p->start_sec +1), 减去当前实际的数据接收量 
				limit = (off_t) p->limit_rate * (ngx_time() - p->start_sec + 1) - p->read_length;
			
				//此处小于等于0， 表示超出了速率限制，将p->upstream->read->delayed置为1，进行相应的延迟
				if (limit <= 0) {
					p->upstream->read->delayed = 1;

					//此处计算延迟读取的毫秒数(乘以1000) 
					delay = (ngx_msec_t) (- limit * 1000 / p->limit_rate + 1);
					ngx_add_timer(p->upstream->read, delay);
					break;
				}
			
			} else {
				limit = 0;
			}

			//3.6) 如下用于腾出空间来接收上游服务器的数据
			if (p->free_raw_bufs){

				//3.6.1) 假如有空闲链接的话，那么使用空闲链接的空间来接收上游服务器数据
				if (p->single_buf) {

					//表示接收上游响应时，一次只能接收一个ngx_buf_t缓冲区。通常IOCP事件驱动机制下，此值为1

				}else{

					//表示使用整个空闲链来接收上游服务器返回的数据

				}
			

			}else if (p->allocated < p->bufs.num) {

				//3.6.2) 没有空闲链接，那么尝试从p->pool池中分配一个size大小的缓冲块来接收数据
				b = ngx_create_temp_buf(p->pool, p->bufs.size);
				if (b == NULL) {
					return NGX_ABORT;
				}
				
				p->allocated++;


				//创建一个chain来包装这个ngx_buf_t结构
				chain = ngx_alloc_chain_link(p->pool);
				if (chain == NULL) {
					return NGX_ABORT;
				}
				
				chain->buf = b;
				chain->next = NULL;


			}else if (!p->cacheable
				&& p->downstream->data == p->output_ctx
				&& p->downstream->write->ready
				&& !p->downstream->write->delayed){

				//3.6.3) 假如数据不需要写入到缓存，并且downstream当前已经准备好了写，并且未被延迟，那么
				//此处break跳出循环，以优先处理当前已有的数据
				p->upstream_blocked = 1;
				break;

			}else if (p->cacheable
				|| p->temp_file->offset < p->max_temp_file_size){

				//3.6.4) 可以缓存，那么先写缓存数据以腾出空间
				rc = ngx_event_pipe_write_chain_to_temp_file(p);
				
                if (rc == NGX_BUSY) {
                    break;				//跳出循环，以优先处理当前已经读取到的数据
                }

                if (rc != NGX_OK) {
                    return rc;			//写入失败，退出
                }

				//获取已经腾出的空间
				chain = p->free_raw_bufs;
				if (p->single_buf) {
					p->free_raw_bufs = p->free_raw_bufs->next;
					chain->next = NULL;
				} else {
					p->free_raw_bufs = NULL;
				}
				
			}else{
				
				//3.6.5) 当前已经没有可用空间以接收上游数据

			}

			//3.7) 调用recv_chain()方法接收来自上游服务器的数据。limit用于速率限制，以指定读取多少数据。
			// 值为0，表示不进行限制
			n = p->upstream->recv_chain(p->upstream, chain, limit);
			
			//3.8) 将chain又重新挂到p->free_raw_bufs链表的表头
			if (p->free_raw_bufs) {
				chain->next = p->free_raw_bufs;
			}
			p->free_raw_bufs = chain;


			//3.9) 读取上游数据出错
			if (n == NGX_ERROR) {
				p->upstream_error = 1;
				return NGX_ERROR;
			}

			//3.10) 没有读取到数据
			if (n == NGX_AGAIN) {

				//移除存在于该buf上的shadow
				if (p->single_buf) {
					ngx_event_pipe_remove_shadow_links(chain->buf);
				}
			
				break;
			}

			//3.11) 表示已经读取到了上游的响应
			p->read = 1;

			//3.12) 上游连接已经关闭
            if (n == 0) {
                p->upstream_eof = 1;
                break;
            }

		} //end else

		//运行到此处，表示从上游服务器读取到了相应的数据

		//4) 读取完这一次之后，看是否需要进行速率限制。
		delay = p->limit_rate ? (ngx_msec_t) n * 1000 / p->limit_rate : 0;

		//5) 当前读取到的总的包体数据大小为p->read_length +=n
		p->read_length += n;
		cl = chain;
		p->free_raw_bufs = NULL;


		//6) 处理本次读取到的上游数据 
		while (cl && n > 0) {
			ngx_event_pipe_remove_shadow_links(cl->buf);

            size = cl->buf->end - cl->buf->last;

			if (n >= size) {

				/* STUB */ cl->buf->num = p->num++;			//此处cl->buf->num保存当前已使用的buf块的个数

				//6.1) 调用input_filter处理接收到的上游数据
                if (p->input_filter(p, cl->buf) == NGX_ERROR) {
                    return NGX_ABORT;
                }
			}else{
				
				//6.2) 	此处为接收到的数据所占用的最后一个buf，且只使用了部分空间。（注意，此一部分数据暂时保存在缓存
				//中并未做任何处理）
				cl->buf->last += n;
                n = 0;
	
			}
		}


		//7) 将相应的空闲空间插入到p->free_raw_bufs中。（注意： 这里cl链的第一个节点可能仍保存有部分数据）
		if (cl) {
			for (ln = cl; ln->next; ln = ln->next) { /* void */ }
		
			ln->next = p->free_raw_bufs;
			p->free_raw_bufs = cl;
		}

		//8) 由于速率限制，这里我们需要按时延迟读取上游数据
		if (delay > 0) {
			p->upstream->read->delayed = 1;
			ngx_add_timer(p->upstream->read, delay);
			break;
		}

	} //end for

	//9) 打印各链表的相关信息：p->busy、p->out、p->in、p->free_raw_bufs
	#if NGX_DEBUG
	#endif

	//10) 在符合条件的情况下，处理存在于p->free_raw_bufs链表第一个节点的残留的部分数据（请参看上述步骤6.2)
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

	//11) 表示上游数据读取完毕
	if (p->length == 0) {
		p->upstream_done = 1;
		p->read = 1;
	}

	//12) 读取数据出现异常的情况下，把p->free_raw_bufs链表第一个节点残留的部分数据处理完(请参看上述步骤6.2) 
	if ((p->upstream_eof || p->upstream_error) && p->free_raw_bufs) {
		/* STUB */ p->free_raw_bufs->buf->num = p->num++;
		
		if (p->input_filter(p, p->free_raw_bufs->buf) == NGX_ERROR) {
			return NGX_ABORT;
		}

		//12.1) 指示尽快清理p->free_raw_bufs空间
		if (p->free_bufs && p->buf_to_file == NULL) {
			for (cl = p->free_raw_bufs; cl; cl = cl->next) {
				if (cl->buf->shadow == NULL) {
					ngx_pfree(p->pool, cl->buf->start);
				}
			}
		}
	}

	//13) 将相应的未处理完的数据进行缓存
	if (p->cacheable && (p->in || p->buf_to_file)) {
	
		rc = ngx_event_pipe_write_chain_to_temp_file(p);
	
		if (rc != NGX_OK) {
			return rc;
		}
	}

}
{% endhighlight %}

上面关于使用多线程写temp文件(ngx_write_chain_to_temp_file())，这里我们再多说一句：

![ngx-eventpipe-thread](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_eventpipe_thread.jpg)


## 4. 函数ngx_event_pipe_write_to_downstream()
{% highlight string %}
static ngx_int_t
ngx_event_pipe_write_to_downstream(ngx_event_pipe_t *p)
{
    u_char            *prev;
    size_t             bsize;
    ngx_int_t          rc;
    ngx_uint_t         flush, flushed, prev_last_shadow;
    ngx_chain_t       *out, **ll, *cl;
    ngx_connection_t  *downstream;

    downstream = p->downstream;

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, p->log, 0,
                   "pipe write downstream: %d", downstream->write->ready);

#if (NGX_THREADS)

    if (p->writing) {
        rc = ngx_event_pipe_write_chain_to_temp_file(p);

        if (rc == NGX_ABORT) {
            return NGX_ABORT;
        }
    }

#endif

    flushed = 0;

    for ( ;; ) {
        if (p->downstream_error) {
            return ngx_event_pipe_drain_chains(p);
        }

        if (p->upstream_eof || p->upstream_error || p->upstream_done) {

            /* pass the p->out and p->in chains to the output filter */

            for (cl = p->busy; cl; cl = cl->next) {
                cl->buf->recycled = 0;
            }

            if (p->out) {
                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "pipe write downstream flush out");

                for (cl = p->out; cl; cl = cl->next) {
                    cl->buf->recycled = 0;
                }

                rc = p->output_filter(p->output_ctx, p->out);

                if (rc == NGX_ERROR) {
                    p->downstream_error = 1;
                    return ngx_event_pipe_drain_chains(p);
                }

                p->out = NULL;
            }

            if (p->writing) {
                break;
            }

            if (p->in) {
                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "pipe write downstream flush in");

                for (cl = p->in; cl; cl = cl->next) {
                    cl->buf->recycled = 0;
                }

                rc = p->output_filter(p->output_ctx, p->in);

                if (rc == NGX_ERROR) {
                    p->downstream_error = 1;
                    return ngx_event_pipe_drain_chains(p);
                }

                p->in = NULL;
            }

            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, p->log, 0,
                           "pipe write downstream done");

            /* TODO: free unused bufs */

            p->downstream_done = 1;
            break;
        }

        if (downstream->data != p->output_ctx
            || !downstream->write->ready
            || downstream->write->delayed)
        {
            break;
        }

        /* bsize is the size of the busy recycled bufs */

        prev = NULL;
        bsize = 0;

        for (cl = p->busy; cl; cl = cl->next) {

            if (cl->buf->recycled) {
                if (prev == cl->buf->start) {
                    continue;
                }

                bsize += cl->buf->end - cl->buf->start;
                prev = cl->buf->start;
            }
        }

        ngx_log_debug1(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "pipe write busy: %uz", bsize);

        out = NULL;

        if (bsize >= (size_t) p->busy_size) {
            flush = 1;
            goto flush;
        }

        flush = 0;
        ll = NULL;
        prev_last_shadow = 1;

        for ( ;; ) {
            if (p->out) {
                cl = p->out;

                if (cl->buf->recycled) {
                    ngx_log_error(NGX_LOG_ALERT, p->log, 0,
                                  "recycled buffer in pipe out chain");
                }

                p->out = p->out->next;

            } else if (!p->cacheable && !p->writing && p->in) {
                cl = p->in;

                ngx_log_debug3(NGX_LOG_DEBUG_EVENT, p->log, 0,
                               "pipe write buf ls:%d %p %z",
                               cl->buf->last_shadow,
                               cl->buf->pos,
                               cl->buf->last - cl->buf->pos);

                if (cl->buf->recycled && prev_last_shadow) {
                    if (bsize + cl->buf->end - cl->buf->start > p->busy_size) {
                        flush = 1;
                        break;
                    }

                    bsize += cl->buf->end - cl->buf->start;
                }

                prev_last_shadow = cl->buf->last_shadow;

                p->in = p->in->next;

            } else {
                break;
            }

            cl->next = NULL;

            if (out) {
                *ll = cl;
            } else {
                out = cl;
            }
            ll = &cl->next;
        }

    flush:

        ngx_log_debug2(NGX_LOG_DEBUG_EVENT, p->log, 0,
                       "pipe write: out:%p, f:%ui", out, flush);

        if (out == NULL) {

            if (!flush) {
                break;
            }

            /* a workaround for AIO */
            if (flushed++ > 10) {
                return NGX_BUSY;
            }
        }

        rc = p->output_filter(p->output_ctx, out);

        ngx_chain_update_chains(p->pool, &p->free, &p->busy, &out, p->tag);

        if (rc == NGX_ERROR) {
            p->downstream_error = 1;
            return ngx_event_pipe_drain_chains(p);
        }

        for (cl = p->free; cl; cl = cl->next) {

            if (cl->buf->temp_file) {
                if (p->cacheable || !p->cyclic_temp_file) {
                    continue;
                }

                /* reset p->temp_offset if all bufs had been sent */

                if (cl->buf->file_last == p->temp_file->offset) {
                    p->temp_file->offset = 0;
                }
            }

            /* TODO: free buf if p->free_bufs && upstream done */

            /* add the free shadow raw buf to p->free_raw_bufs */

            if (cl->buf->last_shadow) {
                if (ngx_event_pipe_add_free_buf(p, cl->buf->shadow) != NGX_OK) {
                    return NGX_ABORT;
                }

                cl->buf->last_shadow = 0;
            }

            cl->buf->shadow = NULL;
        }
    }

    return NGX_OK;
}
{% endhighlight %}

此函数用于向下游请求端发送相应的响应信息。下面我们详细分析一下函数的执行流程：
{% highlight string %}
static ngx_int_t
ngx_event_pipe_write_to_downstream(ngx_event_pipe_t *p)
{
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

