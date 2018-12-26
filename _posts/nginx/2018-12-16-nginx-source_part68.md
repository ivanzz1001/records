---
layout: post
title: event/ngx_event_accept.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


通过我们知道ngx_event_core_module模块的init_process函数**ngx_event_process_init()**会为每个监听套接字的读事件注册处理函数ngx_event_accept(TCP)或者ngx_event_recvmsg(UDP)，这里我们就来讲述一下nginx event acceptde的相关实现。

由于Nginx工作在master-worker多进程模式，若所有worker进程在同一时间监听同一个端口，当该端口有新的连接事件出现时，每个worker进程都会调用函数**ngx_event_accep()**试图与新的连接建立通信，即所有worker进程都会被唤醒，这就是所谓的```惊群```问题，这样会导致系统性能下降。

幸好在Nginx中采用了```ngx_accept_mutex```同步锁机制，即只有获得该锁的worker进程才能去处理新的连接事件，也就在同一时间只能有一个worker进程监听某个端口。虽然这样做解决了```惊群```问题，但是随之会出现另一个问题，若每次出现的新连接都被同一个worker进程获得锁的权利并处理该连接事件，这样会导致进程之间出现不均衡的状态，即在所有worker进程中，某些进程处理的连接事件数量庞大，而某些进程基本上不用处理连接事件，一直处于空闲状态。因此，这样会导致worker进程之间的负载不均衡，会影响nginx的整体性能。为了解决负载失衡的问题，Nginx在已经实现同步锁的基础上定义了负载阈值```ngx_accept_disabled```，当某个worker进程的负载阈值大于0时，表示该进程处于负载超重的状态，则Nginx会控制该进程，使其没机会试图与新的连接事件进行通信，这样就会为其他没有负载超重的进程创造处理新连接事件的机会，以此达到进程间的负载均衡。




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


//将可读事件加到监听socket中，这样就可以发现客户端的新连接
static ngx_int_t ngx_enable_accept_events(ngx_cycle_t *cycle);


//将可读事件从监听socket中移除
static ngx_int_t ngx_disable_accept_events(ngx_cycle_t *cycle, ngx_uint_t all);

//关闭已连接的一个connection。一般是监听socket accept一个fd之后，将该fd与一个connection
//关联起来，关联过程中如果出现错误，则需要将该connection关闭
static void ngx_close_accepted_connection(ngx_connection_t *c);


//主要是打印通过debug_connection指令指定的某些调试连接
#if (NGX_DEBUG)
static void ngx_debug_accepted_connection(ngx_event_conf_t *ecf,
    ngx_connection_t *c);
#endif

{% endhighlight %}



## 1. 函数ngx_event_accept()
{% highlight string %}
void
ngx_event_accept(ngx_event_t *ev)
{
    socklen_t          socklen;
    ngx_err_t          err;
    ngx_log_t         *log;
    ngx_uint_t         level;
    ngx_socket_t       s;
    ngx_event_t       *rev, *wev;
    ngx_listening_t   *ls;
    ngx_connection_t  *c, *lc;
    ngx_event_conf_t  *ecf;
    u_char             sa[NGX_SOCKADDRLEN];
#if (NGX_HAVE_ACCEPT4)
    static ngx_uint_t  use_accept4 = 1;
#endif

    if (ev->timedout) {
        if (ngx_enable_accept_events((ngx_cycle_t *) ngx_cycle) != NGX_OK) {
            return;
        }

        ev->timedout = 0;
    }

    ecf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_core_module);

    if (!(ngx_event_flags & NGX_USE_KQUEUE_EVENT)) {
        ev->available = ecf->multi_accept;
    }

    lc = ev->data;
    ls = lc->listening;
    ev->ready = 0;

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "accept on %V, ready: %d", &ls->addr_text, ev->available);

    do {
        socklen = NGX_SOCKADDRLEN;

#if (NGX_HAVE_ACCEPT4)
        if (use_accept4) {
            s = accept4(lc->fd, (struct sockaddr *) sa, &socklen,
                        SOCK_NONBLOCK);
        } else {
            s = accept(lc->fd, (struct sockaddr *) sa, &socklen);
        }
#else
        s = accept(lc->fd, (struct sockaddr *) sa, &socklen);
#endif

        if (s == (ngx_socket_t) -1) {
            err = ngx_socket_errno;

            if (err == NGX_EAGAIN) {
                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, ev->log, err,
                               "accept() not ready");
                return;
            }

            level = NGX_LOG_ALERT;

            if (err == NGX_ECONNABORTED) {
                level = NGX_LOG_ERR;

            } else if (err == NGX_EMFILE || err == NGX_ENFILE) {
                level = NGX_LOG_CRIT;
            }

#if (NGX_HAVE_ACCEPT4)
            ngx_log_error(level, ev->log, err,
                          use_accept4 ? "accept4() failed" : "accept() failed");

            if (use_accept4 && err == NGX_ENOSYS) {
                use_accept4 = 0;
                ngx_inherited_nonblocking = 0;
                continue;
            }
#else
            ngx_log_error(level, ev->log, err, "accept() failed");
#endif

            if (err == NGX_ECONNABORTED) {
                if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
                    ev->available--;
                }

                if (ev->available) {
                    continue;
                }
            }

            if (err == NGX_EMFILE || err == NGX_ENFILE) {
                if (ngx_disable_accept_events((ngx_cycle_t *) ngx_cycle, 1)
                    != NGX_OK)
                {
                    return;
                }

                if (ngx_use_accept_mutex) {
                    if (ngx_accept_mutex_held) {
                        ngx_shmtx_unlock(&ngx_accept_mutex);
                        ngx_accept_mutex_held = 0;
                    }

                    ngx_accept_disabled = 1;

                } else {
                    ngx_add_timer(ev, ecf->accept_mutex_delay);
                }
            }

            return;
        }

#if (NGX_STAT_STUB)
        (void) ngx_atomic_fetch_add(ngx_stat_accepted, 1);
#endif

        ngx_accept_disabled = ngx_cycle->connection_n / 8
                              - ngx_cycle->free_connection_n;

        c = ngx_get_connection(s, ev->log);

        if (c == NULL) {
            if (ngx_close_socket(s) == -1) {
                ngx_log_error(NGX_LOG_ALERT, ev->log, ngx_socket_errno,
                              ngx_close_socket_n " failed");
            }

            return;
        }

        c->type = SOCK_STREAM;

#if (NGX_STAT_STUB)
        (void) ngx_atomic_fetch_add(ngx_stat_active, 1);
#endif

        c->pool = ngx_create_pool(ls->pool_size, ev->log);
        if (c->pool == NULL) {
            ngx_close_accepted_connection(c);
            return;
        }

        c->sockaddr = ngx_palloc(c->pool, socklen);
        if (c->sockaddr == NULL) {
            ngx_close_accepted_connection(c);
            return;
        }

        ngx_memcpy(c->sockaddr, sa, socklen);

        log = ngx_palloc(c->pool, sizeof(ngx_log_t));
        if (log == NULL) {
            ngx_close_accepted_connection(c);
            return;
        }

        /* set a blocking mode for iocp and non-blocking mode for others */

        if (ngx_inherited_nonblocking) {
            if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
                if (ngx_blocking(s) == -1) {
                    ngx_log_error(NGX_LOG_ALERT, ev->log, ngx_socket_errno,
                                  ngx_blocking_n " failed");
                    ngx_close_accepted_connection(c);
                    return;
                }
            }

        } else {
            if (!(ngx_event_flags & NGX_USE_IOCP_EVENT)) {
                if (ngx_nonblocking(s) == -1) {
                    ngx_log_error(NGX_LOG_ALERT, ev->log, ngx_socket_errno,
                                  ngx_nonblocking_n " failed");
                    ngx_close_accepted_connection(c);
                    return;
                }
            }
        }

        *log = ls->log;

        c->recv = ngx_recv;
        c->send = ngx_send;
        c->recv_chain = ngx_recv_chain;
        c->send_chain = ngx_send_chain;

        c->log = log;
        c->pool->log = log;

        c->socklen = socklen;
        c->listening = ls;
        c->local_sockaddr = ls->sockaddr;
        c->local_socklen = ls->socklen;

        c->unexpected_eof = 1;

#if (NGX_HAVE_UNIX_DOMAIN)
        if (c->sockaddr->sa_family == AF_UNIX) {
            c->tcp_nopush = NGX_TCP_NOPUSH_DISABLED;
            c->tcp_nodelay = NGX_TCP_NODELAY_DISABLED;
#if (NGX_SOLARIS)
            /* Solaris's sendfilev() supports AF_NCA, AF_INET, and AF_INET6 */
            c->sendfile = 0;
#endif
        }
#endif

        rev = c->read;
        wev = c->write;

        wev->ready = 1;

        if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
            rev->ready = 1;
        }

        if (ev->deferred_accept) {
            rev->ready = 1;
#if (NGX_HAVE_KQUEUE)
            rev->available = 1;
#endif
        }

        rev->log = log;
        wev->log = log;

        /*
         * TODO: MT: - ngx_atomic_fetch_add()
         *             or protection by critical section or light mutex
         *
         * TODO: MP: - allocated in a shared memory
         *           - ngx_atomic_fetch_add()
         *             or protection by critical section or light mutex
         */

        c->number = ngx_atomic_fetch_add(ngx_connection_counter, 1);

#if (NGX_STAT_STUB)
        (void) ngx_atomic_fetch_add(ngx_stat_handled, 1);
#endif

        if (ls->addr_ntop) {
            c->addr_text.data = ngx_pnalloc(c->pool, ls->addr_text_max_len);
            if (c->addr_text.data == NULL) {
                ngx_close_accepted_connection(c);
                return;
            }

            c->addr_text.len = ngx_sock_ntop(c->sockaddr, c->socklen,
                                             c->addr_text.data,
                                             ls->addr_text_max_len, 0);
            if (c->addr_text.len == 0) {
                ngx_close_accepted_connection(c);
                return;
            }
        }

#if (NGX_DEBUG)
        {
        ngx_str_t  addr;
        u_char     text[NGX_SOCKADDR_STRLEN];

        ngx_debug_accepted_connection(ecf, c);

        if (log->log_level & NGX_LOG_DEBUG_EVENT) {
            addr.data = text;
            addr.len = ngx_sock_ntop(c->sockaddr, c->socklen, text,
                                     NGX_SOCKADDR_STRLEN, 1);

            ngx_log_debug3(NGX_LOG_DEBUG_EVENT, log, 0,
                           "*%uA accept: %V fd:%d", c->number, &addr, s);
        }

        }
#endif

        if (ngx_add_conn && (ngx_event_flags & NGX_USE_EPOLL_EVENT) == 0) {
            if (ngx_add_conn(c) == NGX_ERROR) {
                ngx_close_accepted_connection(c);
                return;
            }
        }

        log->data = NULL;
        log->handler = NULL;

        ls->handler(c);

        if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
            ev->available--;
        }

    } while (ev->available);
}
{% endhighlight %}

本函数作为基于TCP```listenning``` connection读事件的处理函数，用于接受来自客户端的新连接。下面我们简要分析一下本函数：
{% highlight string %}
void
ngx_event_accept(ngx_event_t *ev)
{
	//1) 在没有采用ngx_accept_mutex的情况下（通常为单进程，或者显示指明accept_mutex值为off)，我们会在监听socket的读事件
	//(read event)同时加到超时红黑树中（参看本函数后面代码），此处正是为了处理此种超时事件。
	if (ev->timedout) {
		if (ngx_enable_accept_events((ngx_cycle_t *) ngx_cycle) != NGX_OK) {
			return;
		}
	
		ev->timedout = 0;
	}


	//2) 获取event core模块的配置信息。
	ecf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_core_module);
	

	//3) multi_accept表示worker进程是否一次只能accept一个连接，ev->available变量用于控制如下do{}while();循环的次数
	if (!(ngx_event_flags & NGX_USE_KQUEUE_EVENT)) {
		ev->available = ecf->multi_accept;
	}


	//4) 处理来自客户端的连接
	do{
		//4.1) 调用accept函数接收来自客户端的连接
		s = accept(lc->fd, (struct sockaddr *) sa, &socklen);


		//4.2) 处理出错情况
		if (s == (ngx_socket_t) -1) {
			
			err = ngx_socket_errno;

            if (err == NGX_EAGAIN) {
				//4.2.1) 直接返回
			}


			//4.2.2) 当前我们支持NGX_HAVE_ACCEPT4宏，此处NGX_ENOSYS错误表示：Function not implemented 
			// 因此这里标记use_accept4值为0，同时将ngx_inherited_nonblocking设置为0，表示不能继承非阻塞
			//特性
			#if (NGX_HAVE_ACCEPT4)
				ngx_log_error(level, ev->log, err,
					use_accept4 ? "accept4() failed" : "accept() failed");
			
				if (use_accept4 && err == NGX_ENOSYS) {
					use_accept4 = 0;
					ngx_inherited_nonblocking = 0;
					continue;
				}
			#else
				ngx_log_error(level, ev->log, err, "accept() failed");
			#endif


			//4.2.3) NGX_ECONNABORTED表示软件错误，通常我们需要继续进行处理
			if (err == NGX_ECONNABORTED) {
				if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
					ev->available--;
				}
			
				if (ev->available) {
					continue;
				}
			}

			//4.2.4) NGX_EMFILE与NGX_ENFILE通常表示进程fd已经耗尽，此时一般需要禁用当前监听socket的再accept
			//新的连接
			if (err == NGX_EMFILE || err == NGX_ENFILE) {
				if (ngx_disable_accept_events((ngx_cycle_t *) ngx_cycle, 1) != NGX_OK)
				{
					return;
				}
			
				//对于使用accept_mutex情况，我们应该释放锁，以让其他worker进程accept
				if (ngx_use_accept_mutex) {
					if (ngx_accept_mutex_held) {
						ngx_shmtx_unlock(&ngx_accept_mutex);
						ngx_accept_mutex_held = 0;
					}
			
					ngx_accept_disabled = 1;
			
				} else {

					//因为上面执行了ngx_disable_accept_events()，因此这里使用定时器，等过一段时间对应的socket关闭后，
					//有足够的fd时就又可以接收新的连接
					ngx_add_timer(ev, ecf->accept_mutex_delay);
				}
			}

		}	


		//5) 增加当前accept连接的数量
		#if (NGX_STAT_STUB)
        	(void) ngx_atomic_fetch_add(ngx_stat_accepted, 1);
		#endif

		//6) 此处较为关键，前面我们在ngx_event_process_init()函数中预先创建了指定最大数量的ngx_connection_t对象，
		//这里当空闲连接数小于总的ngx_connection_t对象数的1/8时，那么ngx_accept_disabled值将会大于0，表示当前
		//worker工作负载较高，此种情况下后续一段时间Nginx将会放弃抢占accept_mutex锁，这样将不会再有新的客户端连接
		//到此worker进程中; 如果ngx_accept_disabled<=0，那么表示空闲连接较多，后续会与其他worker进程来抢占accept_mutex锁
		ngx_accept_disabled = ngx_cycle->connection_n / 8
			- ngx_cycle->free_connection_n;


		//7) 获取一个ngx_connection_t对象来容纳上面accept的socket
		 c = ngx_get_connection(s, ev->log);

		//8) 用于统计当前处于active状态的客户端连接数，这也包括Waiting状态的连接
		#if (NGX_STAT_STUB)
       		(void) ngx_atomic_fetch_add(ngx_stat_active, 1);
		#endif

		//9) 设置该ngx_connection_t对象的pool、sockaddr等


		//10) 对于当前新accept的socket，如果事件驱动机制是iocp的话，那么需要设置为阻塞模式；
		// 其他事件驱动机制，则设置为非阻塞模式。
		/* set a blocking mode for iocp and non-blocking mode for others */
		
		if (ngx_inherited_nonblocking) {
			if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
				if (ngx_blocking(s) == -1) {
					ngx_log_error(NGX_LOG_ALERT, ev->log, ngx_socket_errno,
					ngx_blocking_n " failed");
					ngx_close_accepted_connection(c);
					return;
				}
			}
		
		} else {
			if (!(ngx_event_flags & NGX_USE_IOCP_EVENT)) {
				if (ngx_nonblocking(s) == -1) {
					ngx_log_error(NGX_LOG_ALERT, ev->log, ngx_socket_errno,
					ngx_nonblocking_n " failed");
					ngx_close_accepted_connection(c);
					return;
				}
			}
		}


		//11) 设置当前新建ngx_connection_t的接收发送函数。这里默认情况下我们采用epoll事件驱动机制，因此
		//ngx_io的值为ngx_os_io
		c->recv = ngx_recv;
        c->send = ngx_send;
        c->recv_chain = ngx_recv_chain;
        c->send_chain = ngx_send_chain;

		//此处当前新建ngx_connection_t对象所关联的listening对象
		c->listening = ls;


		//12) 我们当前支持NGX_HAVE_UNIX_DOMAIN宏，如果当前客户端的连接是来自unix域socket的连接，那么设置
		//禁用NOPUSH与NODELAY

		//13) 设置当前connection所关联的读写事件的相关标志。当前为客户端新连接，因此wev->ready=1，而rev->ready保持
		//默认的值0即可。（注： 在调用ngx_get_connection()函数获取connection时，一般会默认将相应的标志清0）
		rev = c->read;
		wev = c->write;
		
		wev->ready = 1;
		
		if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
			rev->ready = 1;
		}
		
		//注意： 这里如果监听socket支持deferred accept，那么此时可以直接标记为read=1，表示可以马上读取数据了
		if (ev->deferred_accept) {
			rev->ready = 1;
		#if (NGX_HAVE_KQUEUE)
			rev->available = 1;
		#endif
		}

		//14) c->number用于记录到当前为止连接（包括来自客户端以及来自服务器）使用次数,既包括TCP连接，也包括UDP连接。
		//ngx_stat_handled用于统计当前所处理的来自客户端的总的连接数，既包括TCP连接，也包括UDP连接。
	    c->number = ngx_atomic_fetch_add(ngx_connection_counter, 1);
	
		#if (NGX_STAT_STUB)
			(void) ngx_atomic_fetch_add(ngx_stat_handled, 1);
		#endif

		//15) 打印连接的调试信息
		#if (NGX_DEBUG)
		#endif

		//16) 调用某一种监听socket的handler回调函数。查询'ls->handler' 可以看到，对于http模块，其绑定的handler为
		//ngx_http_init_connection; 对于mail模块，其绑定的handler为ngx_mail_init_connection()； 对于stream模块
		//其绑定的handler为ngx_stream_init_connection()
		ls->handler(c);

		//17) 对于kqueue，available减1
		if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
			ev->available--;
		}

	}while(ev->available);
}
{% endhighlight %}


## 3. 函数ngx_event_recvmsg()
{% highlight string %}
#if !(NGX_WIN32)

void
ngx_event_recvmsg(ngx_event_t *ev)
{
    ssize_t            n;
    ngx_log_t         *log;
    ngx_err_t          err;
    ngx_event_t       *rev, *wev;
    struct iovec       iov[1];
    struct msghdr      msg;
    ngx_listening_t   *ls;
    ngx_event_conf_t  *ecf;
    ngx_connection_t  *c, *lc;
    u_char             sa[NGX_SOCKADDRLEN];
    static u_char      buffer[65535];

#if (NGX_HAVE_MSGHDR_MSG_CONTROL)

#if (NGX_HAVE_IP_RECVDSTADDR)
    u_char             msg_control[CMSG_SPACE(sizeof(struct in_addr))];
#elif (NGX_HAVE_IP_PKTINFO)
    u_char             msg_control[CMSG_SPACE(sizeof(struct in_pktinfo))];
#endif

#if (NGX_HAVE_INET6 && NGX_HAVE_IPV6_RECVPKTINFO)
    u_char             msg_control6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
#endif

#endif

    if (ev->timedout) {
        if (ngx_enable_accept_events((ngx_cycle_t *) ngx_cycle) != NGX_OK) {
            return;
        }

        ev->timedout = 0;
    }

    ecf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_core_module);

    if (!(ngx_event_flags & NGX_USE_KQUEUE_EVENT)) {
        ev->available = ecf->multi_accept;
    }

    lc = ev->data;
    ls = lc->listening;
    ev->ready = 0;

    ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                   "recvmsg on %V, ready: %d", &ls->addr_text, ev->available);

    do {
        ngx_memzero(&msg, sizeof(struct msghdr));

        iov[0].iov_base = (void *) buffer;
        iov[0].iov_len = sizeof(buffer);

        msg.msg_name = &sa;
        msg.msg_namelen = sizeof(sa);
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;

#if (NGX_HAVE_MSGHDR_MSG_CONTROL)

        if (ls->wildcard) {

#if (NGX_HAVE_IP_RECVDSTADDR || NGX_HAVE_IP_PKTINFO)
            if (ls->sockaddr->sa_family == AF_INET) {
                msg.msg_control = &msg_control;
                msg.msg_controllen = sizeof(msg_control);
            }
#endif

#if (NGX_HAVE_INET6 && NGX_HAVE_IPV6_RECVPKTINFO)
            if (ls->sockaddr->sa_family == AF_INET6) {
                msg.msg_control = &msg_control6;
                msg.msg_controllen = sizeof(msg_control6);
            }
#endif
        }

#endif

        n = recvmsg(lc->fd, &msg, 0);

        if (n == -1) {
            err = ngx_socket_errno;

            if (err == NGX_EAGAIN) {
                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, ev->log, err,
                               "recvmsg() not ready");
                return;
            }

            ngx_log_error(NGX_LOG_ALERT, ev->log, err, "recvmsg() failed");

            return;
        }

#if (NGX_STAT_STUB)
        (void) ngx_atomic_fetch_add(ngx_stat_accepted, 1);
#endif

#if (NGX_HAVE_MSGHDR_MSG_CONTROL)
        if (msg.msg_flags & (MSG_TRUNC|MSG_CTRUNC)) {
            ngx_log_error(NGX_LOG_ALERT, ev->log, 0,
                          "recvmsg() truncated data");
            continue;
        }
#endif

        ngx_accept_disabled = ngx_cycle->connection_n / 8
                              - ngx_cycle->free_connection_n;

        c = ngx_get_connection(lc->fd, ev->log);
        if (c == NULL) {
            return;
        }

        c->shared = 1;
        c->type = SOCK_DGRAM;
        c->socklen = msg.msg_namelen;

#if (NGX_STAT_STUB)
        (void) ngx_atomic_fetch_add(ngx_stat_active, 1);
#endif

        c->pool = ngx_create_pool(ls->pool_size, ev->log);
        if (c->pool == NULL) {
            ngx_close_accepted_connection(c);
            return;
        }

        c->sockaddr = ngx_palloc(c->pool, c->socklen);
        if (c->sockaddr == NULL) {
            ngx_close_accepted_connection(c);
            return;
        }

        ngx_memcpy(c->sockaddr, msg.msg_name, c->socklen);

        log = ngx_palloc(c->pool, sizeof(ngx_log_t));
        if (log == NULL) {
            ngx_close_accepted_connection(c);
            return;
        }

        *log = ls->log;

        c->send = ngx_udp_send;

        c->log = log;
        c->pool->log = log;

        c->listening = ls;
        c->local_sockaddr = ls->sockaddr;
        c->local_socklen = ls->socklen;

#if (NGX_HAVE_MSGHDR_MSG_CONTROL)

        if (ls->wildcard) {
            struct cmsghdr   *cmsg;
            struct sockaddr  *sockaddr;

            sockaddr = ngx_palloc(c->pool, c->local_socklen);
            if (sockaddr == NULL) {
                ngx_close_accepted_connection(c);
                return;
            }

            ngx_memcpy(sockaddr, c->local_sockaddr, c->local_socklen);
            c->local_sockaddr = sockaddr;

            for (cmsg = CMSG_FIRSTHDR(&msg);
                 cmsg != NULL;
                 cmsg = CMSG_NXTHDR(&msg, cmsg))
            {

#if (NGX_HAVE_IP_RECVDSTADDR)

                if (cmsg->cmsg_level == IPPROTO_IP
                    && cmsg->cmsg_type == IP_RECVDSTADDR
                    && sockaddr->sa_family == AF_INET)
                {
                    struct in_addr      *addr;
                    struct sockaddr_in  *sin;

                    addr = (struct in_addr *) CMSG_DATA(cmsg);
                    sin = (struct sockaddr_in *) sockaddr;
                    sin->sin_addr = *addr;

                    break;
                }

#elif (NGX_HAVE_IP_PKTINFO)

                if (cmsg->cmsg_level == IPPROTO_IP
                    && cmsg->cmsg_type == IP_PKTINFO
                    && sockaddr->sa_family == AF_INET)
                {
                    struct in_pktinfo   *pkt;
                    struct sockaddr_in  *sin;

                    pkt = (struct in_pktinfo *) CMSG_DATA(cmsg);
                    sin = (struct sockaddr_in *) sockaddr;
                    sin->sin_addr = pkt->ipi_addr;

                    break;
                }

#endif

#if (NGX_HAVE_INET6 && NGX_HAVE_IPV6_RECVPKTINFO)

                if (cmsg->cmsg_level == IPPROTO_IPV6
                    && cmsg->cmsg_type == IPV6_PKTINFO
                    && sockaddr->sa_family == AF_INET6)
                {
                    struct in6_pktinfo   *pkt6;
                    struct sockaddr_in6  *sin6;

                    pkt6 = (struct in6_pktinfo *) CMSG_DATA(cmsg);
                    sin6 = (struct sockaddr_in6 *) sockaddr;
                    sin6->sin6_addr = pkt6->ipi6_addr;

                    break;
                }

#endif

            }
        }

#endif

        c->buffer = ngx_create_temp_buf(c->pool, n);
        if (c->buffer == NULL) {
            ngx_close_accepted_connection(c);
            return;
        }

        c->buffer->last = ngx_cpymem(c->buffer->last, buffer, n);

        rev = c->read;
        wev = c->write;

        wev->ready = 1;

        rev->log = log;
        wev->log = log;

        /*
         * TODO: MT: - ngx_atomic_fetch_add()
         *             or protection by critical section or light mutex
         *
         * TODO: MP: - allocated in a shared memory
         *           - ngx_atomic_fetch_add()
         *             or protection by critical section or light mutex
         */

        c->number = ngx_atomic_fetch_add(ngx_connection_counter, 1);

#if (NGX_STAT_STUB)
        (void) ngx_atomic_fetch_add(ngx_stat_handled, 1);
#endif

        if (ls->addr_ntop) {
            c->addr_text.data = ngx_pnalloc(c->pool, ls->addr_text_max_len);
            if (c->addr_text.data == NULL) {
                ngx_close_accepted_connection(c);
                return;
            }

            c->addr_text.len = ngx_sock_ntop(c->sockaddr, c->socklen,
                                             c->addr_text.data,
                                             ls->addr_text_max_len, 0);
            if (c->addr_text.len == 0) {
                ngx_close_accepted_connection(c);
                return;
            }
        }

#if (NGX_DEBUG)
        {
        ngx_str_t  addr;
        u_char     text[NGX_SOCKADDR_STRLEN];

        ngx_debug_accepted_connection(ecf, c);

        if (log->log_level & NGX_LOG_DEBUG_EVENT) {
            addr.data = text;
            addr.len = ngx_sock_ntop(c->sockaddr, c->socklen, text,
                                     NGX_SOCKADDR_STRLEN, 1);

            ngx_log_debug4(NGX_LOG_DEBUG_EVENT, log, 0,
                           "*%uA recvmsg: %V fd:%d n:%z",
                           c->number, &addr, c->fd, n);
        }

        }
#endif

        log->data = NULL;
        log->handler = NULL;

        ls->handler(c);

        if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
            ev->available -= n;
        }

    } while (ev->available);
}

#endif
{% endhighlight %}
本函数作为基于UDP```listenning``` connection读事件的处理函数，用于接受来自客户端的UDP连接。下面我们简要分析一下本函数：
{% highlight string %}
void
ngx_event_recvmsg(ngx_event_t *ev)
{
	//当前我们支持NGX_HAVE_MSGHDR_MSG_CONTROL宏定义，不支持NGX_HAVE_IP_RECVDSTADDR， 
	//支持NGX_HAVE_IP_PKTINFO，不支持NGX_HAVE_INET6， 支持NGX_HAVE_IPV6_RECVPKTINFO宏
	#if (NGX_HAVE_MSGHDR_MSG_CONTROL)
	
	#if (NGX_HAVE_IP_RECVDSTADDR)
		u_char             msg_control[CMSG_SPACE(sizeof(struct in_addr))];
	#elif (NGX_HAVE_IP_PKTINFO)
		u_char             msg_control[CMSG_SPACE(sizeof(struct in_pktinfo))];
	#endif
	
	#if (NGX_HAVE_INET6 && NGX_HAVE_IPV6_RECVPKTINFO)
		u_char             msg_control6[CMSG_SPACE(sizeof(struct in6_pktinfo))];
	#endif
	
	#endif

	//1) 在没有采用ngx_accept_mutex的情况下（通常为单进程，或者显示指明accept_mutex值为off)，我们会在监听socket的读事件
	//(read event)同时加到超时红黑树中（参看本函数后面代码），此处正是为了处理此种超时事件。
	if (ev->timedout) {
		if (ngx_enable_accept_events((ngx_cycle_t *) ngx_cycle) != NGX_OK) {
		return;
		}
	
		ev->timedout = 0;
	}

	//2) 获取event core模块的配置信息。
	ecf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_core_module);
	

	//3) multi_accept表示worker进程是否一次只能accept一个连接，ev->available变量用于控制如下do{}while();循环的次数
	if (!(ngx_event_flags & NGX_USE_KQUEUE_EVENT)) {
		ev->available = ecf->multi_accept;
	}


	//4) 处理来自客户端的连接
	do{
		
		//4.1) 调用recvmsg()函数接收来自客户端的连接
		n = recvmsg(lc->fd, &msg, 0);

		//4.2) 处理出错情况
		if (n == -1) {
			err = ngx_socket_errno;

			if (err == NGX_EAGAIN) {
			}
		}

	
		//5) 增加当前accept连接的数量
		#if (NGX_STAT_STUB)
        	(void) ngx_atomic_fetch_add(ngx_stat_accepted, 1);
		#endif

		//6) 当前消息被截断，不做处理
		#if (NGX_HAVE_MSGHDR_MSG_CONTROL)
			if (msg.msg_flags & (MSG_TRUNC|MSG_CTRUNC)) {
				ngx_log_error(NGX_LOG_ALERT, ev->log, 0,
					"recvmsg() truncated data");
				continue;
			}
		#endif

		//7) 此处较为关键，前面我们在ngx_event_process_init()函数中预先创建了指定最大数量的ngx_connection_t对象，
		//这里当空闲连接数小于总的ngx_connection_t对象数的1/8时，那么ngx_accept_disabled值将会大于0，表示当前
		//worker工作负载较高，此种情况下后续一段时间Nginx将会放弃抢占accept_mutex锁，这样将不会再有新的客户端连接
		//到此worker进程中; 如果ngx_accept_disabled<=0，那么表示空闲连接较多，后续会与其他worker进程来抢占accept_mutex锁
		ngx_accept_disabled = ngx_cycle->connection_n / 8
				- ngx_cycle->free_connection_n;

		
		//7) 获取一个ngx_connection_t对象来容纳上面recvmsg()
		c = ngx_get_connection(s, ev->log);

		c->shared = 1;							//此处标记为此连接在进程或线程之间是共享的
        c->type = SOCK_DGRAM;
        c->socklen = msg.msg_namelen;			//保存获取到的对端地址长度

		
		//8) 用于统计当前处于active状态的客户端连接数，这也包括Waiting状态的连接
		#if (NGX_STAT_STUB)
			(void) ngx_atomic_fetch_add(ngx_stat_active, 1);
		#endif


		//9) 设置当前新获取连接的远程地址
		c->pool = ngx_create_pool(ls->pool_size, ev->log);
		if (c->pool == NULL) {
			ngx_close_accepted_connection(c);
			return;
		}
		
		c->sockaddr = ngx_palloc(c->pool, c->socklen);
		if (c->sockaddr == NULL) {
			ngx_close_accepted_connection(c);
			return;
		}
		
		ngx_memcpy(c->sockaddr, msg.msg_name, c->socklen);


		//10) 设置该connection的send函数
		 c->send = ngx_udp_send;

		//11) 获取所创建的UDP连接的本地地址
		#if (NGX_HAVE_MSGHDR_MSG_CONTROL)
			if (ls->wildcard) {
				for (cmsg = CMSG_FIRSTHDR(&msg);cmsg != NULL;cmsg = CMSG_NXTHDR(&msg, cmsg))
				{
					
				}
			
			}
		#endif

		//12) 创建一个临时缓存来存放收到的数据
		c->buffer = ngx_create_temp_buf(c->pool, n);
		if (c->buffer == NULL) {
			ngx_close_accepted_connection(c);
			return;
		}
		
		c->buffer->last = ngx_cpymem(c->buffer->last, buffer, n);

		//13) c->number用于记录到当前为止连接（包括来自客户端以及来自服务器）使用次数,既包括TCP连接，也包括UDP连接。
		//ngx_stat_handled用于统计当前所处理的来自客户端的总的连接数，既包括TCP连接，也包括UDP连接。
	    c->number = ngx_atomic_fetch_add(ngx_connection_counter, 1);
	
		#if (NGX_STAT_STUB)
			(void) ngx_atomic_fetch_add(ngx_stat_handled, 1);
		#endif

		//14) 打印连接的调试信息
		#if (NGX_DEBUG)
		#endif

		//15) 调用某一种监听socket的handler回调函数。（当前代码中貌似没有针对监听UDP socket，绑定相应的handler)
		ls->handler(c);

		//17) 对于kqueue，available减1
		if (ngx_event_flags & NGX_USE_KQUEUE_EVENT) {
			ev->available--;
		}

	}while(ev->available);
}
{% endhighlight %}


## 4. 函数ngx_trylock_accept_mutex()
{% highlight string %}
ngx_int_t
ngx_trylock_accept_mutex(ngx_cycle_t *cycle)
{
    if (ngx_shmtx_trylock(&ngx_accept_mutex)) {

        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                       "accept mutex locked");

        if (ngx_accept_mutex_held && ngx_accept_events == 0) {
            return NGX_OK;
        }

        if (ngx_enable_accept_events(cycle) == NGX_ERROR) {
            ngx_shmtx_unlock(&ngx_accept_mutex);
            return NGX_ERROR;
        }

        ngx_accept_events = 0;
        ngx_accept_mutex_held = 1;

        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_EVENT, cycle->log, 0,
                   "accept mutex lock failed: %ui", ngx_accept_mutex_held);

    if (ngx_accept_mutex_held) {
        if (ngx_disable_accept_events(cycle, 0) == NGX_ERROR) {
            return NGX_ERROR;
        }

        ngx_accept_mutex_held = 0;
    }

    return NGX_OK;
}
{% endhighlight %}
本函数用于尝试获取```accept_mutex```锁。下面详细分析一下本函数的实现：
{% highlight string %}
ngx_int_t
ngx_trylock_accept_mutex(ngx_cycle_t *cycle)
{
	//1. 尝试获取共享锁
	if (ngx_shmtx_trylock(&ngx_accept_mutex)) {

		//1.1） 这里ngx_accept_mutex_held为1，表示该该锁原来就是由本进程所持有。这种情况下一般不需要重新enable事件，因为这些
		//事件上一次就已经被添加到了本事件驱动机制中（注意： 这里ngx_accept_events主要用于eventport, 对于eventport这
		//一事件驱动机制，每一次调用完后事件都会自动清除，会将ngx_accept_events置为1，需要重新添加）
		if (ngx_accept_mutex_held && ngx_accept_events == 0) {
			return NGX_OK;
		}

		//1.2) 原来accept_mutex并不是由本进程所持有，因此这里需要enable一次
		if (ngx_enable_accept_events(cycle) == NGX_ERROR) {
            ngx_shmtx_unlock(&ngx_accept_mutex);
            return NGX_ERROR;
        }

        ngx_accept_events = 0;
        ngx_accept_mutex_held = 1;
		
	}

	//2. 当前进程没有获取到锁，证明是由别的进程获取到了。如果ngx_accept_mutex_held的值为1，证明该锁原来是由
	//本进程持有，即监听socket原先是加入到了本进程的事件驱动机制当中的。因此这里在进入下一次事件驱动机制(select/
	// poll/eploll)之前，我们需要先disable掉
	if (ngx_accept_mutex_held) {
		if (ngx_disable_accept_events(cycle, 0) == NGX_ERROR) {
			return NGX_ERROR;
		}
		
		ngx_accept_mutex_held = 0;		//标记该锁不再被本进程所持有
	}
	
}
{% endhighlight %}


## 5. 函数ngx_enable_accept_events()
{% highlight string %}
static ngx_int_t
ngx_enable_accept_events(ngx_cycle_t *cycle)
{
    ngx_uint_t         i;
    ngx_listening_t   *ls;
    ngx_connection_t  *c;

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {

        c = ls[i].connection;

        if (c == NULL || c->read->active) {
            continue;
        }

        if (ngx_add_event(c->read, NGX_READ_EVENT, 0) == NGX_ERROR) {
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}
{% endhighlight %}
此函数较为简单，把监听socket加入到对应的事件驱动机制中

## 6. 函数ngx_disable_accept_events()
{% highlight string %}
static ngx_int_t
ngx_disable_accept_events(ngx_cycle_t *cycle, ngx_uint_t all)
{
    ngx_uint_t         i;
    ngx_listening_t   *ls;
    ngx_connection_t  *c;

    ls = cycle->listening.elts;
    for (i = 0; i < cycle->listening.nelts; i++) {

        c = ls[i].connection;

        if (c == NULL || !c->read->active) {
            continue;
        }

#if (NGX_HAVE_REUSEPORT)

        /*
         * do not disable accept on worker's own sockets
         * when disabling accept events due to accept mutex
         */

        if (ls[i].reuseport && !all) {
            continue;
        }

#endif

        if (ngx_del_event(c->read, NGX_READ_EVENT, NGX_DISABLE_EVENT)
            == NGX_ERROR)
        {
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}

{% endhighlight %}
本函数用于从```事件驱动机制```中移除监听socket的读事件。这里注意，对于```ls[i].reuseport```，表示复用端口，此时该socket属于worker进程，因此并不会从事件驱动机制中移除。


## 7. 函数ngx_close_accepted_connection()
{% highlight string %}
static void
ngx_close_accepted_connection(ngx_connection_t *c)
{
    ngx_socket_t  fd;

    ngx_free_connection(c);

    fd = c->fd;
    c->fd = (ngx_socket_t) -1;

    if (!c->shared && ngx_close_socket(fd) == -1) {
        ngx_log_error(NGX_LOG_ALERT, c->log, ngx_socket_errno,
                      ngx_close_socket_n " failed");
    }

    if (c->pool) {
        ngx_destroy_pool(c->pool);
    }

#if (NGX_STAT_STUB)
    (void) ngx_atomic_fetch_add(ngx_stat_active, -1);
#endif
}
{% endhighlight %}

本函数用于关闭accept到的连接对象，并将ngx_connection_t连接对象放回到ngx_cycle->free_connections中。注意，如果该connection所对应的socket fd在多个线程或进程之间共享，那么这里并不真正关闭fd。

## 8. 函数ngx_accept_log_error()
{% highlight string %}
u_char *
ngx_accept_log_error(ngx_log_t *log, u_char *buf, size_t len)
{
    return ngx_snprintf(buf, len, " while accepting new connection on %V",
                        log->data);
}
{% endhighlight %}
本函数用于格式化accept日志输出。

## 9. 函数ngx_debug_accepted_connection()
{% highlight string %}
#if (NGX_DEBUG)

static void
ngx_debug_accepted_connection(ngx_event_conf_t *ecf, ngx_connection_t *c)
{
    struct sockaddr_in   *sin;
    ngx_cidr_t           *cidr;
    ngx_uint_t            i;
#if (NGX_HAVE_INET6)
    struct sockaddr_in6  *sin6;
    ngx_uint_t            n;
#endif

    cidr = ecf->debug_connection.elts;
    for (i = 0; i < ecf->debug_connection.nelts; i++) {
        if (cidr[i].family != (ngx_uint_t) c->sockaddr->sa_family) {
            goto next;
        }

        switch (cidr[i].family) {

#if (NGX_HAVE_INET6)
        case AF_INET6:
            sin6 = (struct sockaddr_in6 *) c->sockaddr;
            for (n = 0; n < 16; n++) {
                if ((sin6->sin6_addr.s6_addr[n]
                    & cidr[i].u.in6.mask.s6_addr[n])
                    != cidr[i].u.in6.addr.s6_addr[n])
                {
                    goto next;
                }
            }
            break;
#endif

#if (NGX_HAVE_UNIX_DOMAIN)
        case AF_UNIX:
            break;
#endif

        default: /* AF_INET */
            sin = (struct sockaddr_in *) c->sockaddr;
            if ((sin->sin_addr.s_addr & cidr[i].u.in.mask)
                != cidr[i].u.in.addr)
            {
                goto next;
            }
            break;
        }

        c->log->log_level = NGX_LOG_DEBUG_CONNECTION|NGX_LOG_DEBUG_ALL;
        break;

    next:
        continue;
    }
}

#endif
{% endhighlight %}
nginx在accept到一个新的连接之后，会将该新连接的远程地址与nginx配置指令```debug_connection```所配置的地址向比较，如果相等，那么调制该连接的日志打印级别，从而实现对某些客户端连接的打印。



<br />
<br />

**[参看]**

1. [Nginx监听套接字读事件处理函数ngx_event_accept](https://blog.csdn.net/oyw5201314ck/article/details/78491865)

2. [Nginx 事件驱动模块连接处理](https://www.xuebuyuan.com/2225396.html)

3. [从EMFILE和ENFILE说起，fd limit的问题（一）](https://blog.csdn.net/sdn_prc/article/details/28661661)


<br />
<br />
<br />

