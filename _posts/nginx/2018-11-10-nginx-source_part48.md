---
layout: post
title: core/ngx_output_chain.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


nginx使用output chain来发送数据，这里主要有两个原因：

1） 数据可能并不是一次性产生，而是分散在多个步骤，这样客观上形成了一个等待发送的链式数据；

2） 数据量太大，单个小块内存存放不下，并且一次也发送不完


因此，这里我们首先需要以一个链式结构保存起要发送的数据，其次我们还需要保存发送上下文，我们用到前面介绍的
<pre>
typedef struct ngx_output_chain_ctx_s  ngx_output_chain_ctx_t;
</pre>
结构。


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


#if 0
#define NGX_SENDFILE_LIMIT  4096
#endif

/*
 * When DIRECTIO is enabled FreeBSD, Solaris, and MacOSX read directly
 * to an application memory from a device if parameters are aligned
 * to device sector boundary (512 bytes).  They fallback to usual read
 * operation if the parameters are not aligned.
 * Linux allows DIRECTIO only if the parameters are aligned to a filesystem
 * sector boundary, otherwise it returns EINVAL.  The sector size is
 * usually 512 bytes, however, on XFS it may be 4096 bytes.
 */

#define NGX_NONE            1


static ngx_inline ngx_int_t
    ngx_output_chain_as_is(ngx_output_chain_ctx_t *ctx, ngx_buf_t *buf);
#if (NGX_HAVE_AIO_SENDFILE)
static ngx_int_t ngx_output_chain_aio_setup(ngx_output_chain_ctx_t *ctx,
    ngx_file_t *file);
#endif
static ngx_int_t ngx_output_chain_add_copy(ngx_pool_t *pool,
    ngx_chain_t **chain, ngx_chain_t *in);
static ngx_int_t ngx_output_chain_align_file_buf(ngx_output_chain_ctx_t *ctx,
    off_t bsize);
static ngx_int_t ngx_output_chain_get_buf(ngx_output_chain_ctx_t *ctx,
    off_t bsize);
static ngx_int_t ngx_output_chain_copy_buf(ngx_output_chain_ctx_t *ctx);
{% endhighlight %}

这里声明了一些output chain发送的辅助函数：

* 函数ngx_output_chain_as_is(): 主要用来判断是否需要复制buf。如果返回值为1，表示不需要拷贝； 否则表示需要拷贝

* 函数ngx_output_chain_aio_setup(): 建立aio来发送文件，当前我们并不支持此功能（即 **'NGX_HAVE_AIO_SENDFILE'**宏未定义）

* 函数ngx_output_chain_add_copy(): 拷贝```in```到```chain```中

* 函数ngx_output_chain_align_file_buf(): 此函数主要用于对齐file buf，某些发送函数需要进行相应对齐。

* 函数ngx_output_chain_get_buf(): 获得指定大小的buf，赋值给ctx->buf

* 函数ngx_output_chain_copy_buf(): 拷贝数据。我们一般输出的话都是从ctx->in直接拷贝到ctx->buf中，然后发送出去



## 2. 函数ngx_output_chain()
{% highlight string %}
ngx_int_t
ngx_output_chain(ngx_output_chain_ctx_t *ctx, ngx_chain_t *in)
{
    off_t         bsize;
    ngx_int_t     rc, last;
    ngx_chain_t  *cl, *out, **last_out;

    if (ctx->in == NULL && ctx->busy == NULL
#if (NGX_HAVE_FILE_AIO || NGX_THREADS)
        && !ctx->aio
#endif
       )
    {
        /*
         * the short path for the case when the ctx->in and ctx->busy chains
         * are empty, the incoming chain is empty too or has the single buf
         * that does not require the copy
         */

        if (in == NULL) {
            return ctx->output_filter(ctx->filter_ctx, in);
        }

        if (in->next == NULL
#if (NGX_SENDFILE_LIMIT)
            && !(in->buf->in_file && in->buf->file_last > NGX_SENDFILE_LIMIT)
#endif
            && ngx_output_chain_as_is(ctx, in->buf))
        {
            return ctx->output_filter(ctx->filter_ctx, in);
        }
    }

    /* add the incoming buf to the chain ctx->in */

    if (in) {
        if (ngx_output_chain_add_copy(ctx->pool, &ctx->in, in) == NGX_ERROR) {
            return NGX_ERROR;
        }
    }

    out = NULL;
    last_out = &out;
    last = NGX_NONE;

    for ( ;; ) {

#if (NGX_HAVE_FILE_AIO || NGX_THREADS)
        if (ctx->aio) {
            return NGX_AGAIN;
        }
#endif

        while (ctx->in) {

            /*
             * cycle while there are the ctx->in bufs
             * and there are the free output bufs to copy in
             */

            bsize = ngx_buf_size(ctx->in->buf);

            if (bsize == 0 && !ngx_buf_special(ctx->in->buf)) {

                ngx_log_error(NGX_LOG_ALERT, ctx->pool->log, 0,
                              "zero size buf in output "
                              "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                              ctx->in->buf->temporary,
                              ctx->in->buf->recycled,
                              ctx->in->buf->in_file,
                              ctx->in->buf->start,
                              ctx->in->buf->pos,
                              ctx->in->buf->last,
                              ctx->in->buf->file,
                              ctx->in->buf->file_pos,
                              ctx->in->buf->file_last);

                ngx_debug_point();

                ctx->in = ctx->in->next;

                continue;
            }

            if (ngx_output_chain_as_is(ctx, ctx->in->buf)) {

                /* move the chain link to the output chain */

                cl = ctx->in;
                ctx->in = cl->next;

                *last_out = cl;
                last_out = &cl->next;
                cl->next = NULL;

                continue;
            }

            if (ctx->buf == NULL) {

                rc = ngx_output_chain_align_file_buf(ctx, bsize);

                if (rc == NGX_ERROR) {
                    return NGX_ERROR;
                }

                if (rc != NGX_OK) {

                    if (ctx->free) {

                        /* get the free buf */

                        cl = ctx->free;
                        ctx->buf = cl->buf;
                        ctx->free = cl->next;

                        ngx_free_chain(ctx->pool, cl);

                    } else if (out || ctx->allocated == ctx->bufs.num) {

                        break;

                    } else if (ngx_output_chain_get_buf(ctx, bsize) != NGX_OK) {
                        return NGX_ERROR;
                    }
                }
            }

            rc = ngx_output_chain_copy_buf(ctx);

            if (rc == NGX_ERROR) {
                return rc;
            }

            if (rc == NGX_AGAIN) {
                if (out) {
                    break;
                }

                return rc;
            }

            /* delete the completed buf from the ctx->in chain */

            if (ngx_buf_size(ctx->in->buf) == 0) {
                ctx->in = ctx->in->next;
            }

            cl = ngx_alloc_chain_link(ctx->pool);
            if (cl == NULL) {
                return NGX_ERROR;
            }

            cl->buf = ctx->buf;
            cl->next = NULL;
            *last_out = cl;
            last_out = &cl->next;
            ctx->buf = NULL;
        }

        if (out == NULL && last != NGX_NONE) {

            if (ctx->in) {
                return NGX_AGAIN;
            }

            return last;
        }

        last = ctx->output_filter(ctx->filter_ctx, out);

        if (last == NGX_ERROR || last == NGX_DONE) {
            return last;
        }

        ngx_chain_update_chains(ctx->pool, &ctx->free, &ctx->busy, &out,
                                ctx->tag);
        last_out = &out;
    }
}
{% endhighlight %}

此函数的目的是发送```in```中的数据，```ctx```用来保存发送的上下文。因为通常情况下，不能一次发送完成。nginx因为使用了```ET```模式，在网络编程事件管理上简单了，但是编程中处理事件复杂了，需要不停的循环做处理； 事件的函数回调，次数也不确定，因此需要使用context上下文对象来保存发送到什么环节了。

此函数当前只在如下两个模块中会调用到：

* ngx_http_copy_filter_module： 是响应体过滤链（body filter）中非常重要的一个模块，这个filter模块主要是来将一些需要复制的buf（可能在文件中，也可能在内存中）重新复制一份交给后面的filter模块处理

* nginx的upstream模块

下面我们分成几个部分来讲解本函数

1） **简单发送(short path)**
{% highlight string %}
ngx_int_t
ngx_output_chain(ngx_output_chain_ctx_t *ctx, ngx_chain_t *in)
{
	    if (ctx->in == NULL && ctx->busy == NULL
#if (NGX_HAVE_FILE_AIO || NGX_THREADS)
        && !ctx->aio
#endif
       )
    {
        /*
         * the short path for the case when the ctx->in and ctx->busy chains
         * are empty, the incoming chain is empty too or has the single buf
         * that does not require the copy
         */

        if (in == NULL) {
            return ctx->output_filter(ctx->filter_ctx, in);
        }

        if (in->next == NULL
#if (NGX_SENDFILE_LIMIT)
            && !(in->buf->in_file && in->buf->file_last > NGX_SENDFILE_LIMIT)
#endif
            && ngx_output_chain_as_is(ctx, in->buf))
        {
            return ctx->output_filter(ctx->filter_ctx, in);
        }
    }
}
{% endhighlight %}
这里我们看到假如```ctx->in```及```ctx->busy```均为NULL时，说明当前ctx中并未有太多的数据需要发送。此时如果参数```in```传递进来的要发送的数据也较少，那么就可以直接调用ctx->output_filter()来进行处理。也就是说当能直接确定所有in chain都不需要复制时，可以直接调用output_filter来交给剩下的filter去处理。

2) **将传递进来的in chain拷贝到ctx->in中**
{% highlight string %}
ngx_int_t
ngx_output_chain(ngx_output_chain_ctx_t *ctx, ngx_chain_t *in)
{
	...

    /* add the incoming buf to the chain ctx->in */

    if (in) {
        if (ngx_output_chain_add_copy(ctx->pool, &ctx->in, in) == NGX_ERROR) {
            return NGX_ERROR;
        }
    }	
	...
}
{% endhighlight %}

这里将参数传递进来的in chain buf拷贝到ctx->in的结尾，以等待后续的发送处理；

**3） 处理主流程**

主流程所完成的基本功能是： 将```in```中的数据处理后放到ctx->buf临时缓冲链中，然后再调用ctx->output_filter对该临时buf进行后续的发送处理。没有处理完成的chain放到ctx->busy中，而已经发送完毕的就放到ctx->free中。

在这里主处理逻辑阶段，nginx做的非常巧妙也非常复杂，首先是chain的重用，然后是buf的重用。

3.1） chain的重用

这里我们首先来看chain的重用，涉及到的几个关键的结构及域： ```ctx->free```、```ctx->busy```、```ctx->pool->chain```。其中每次没有处理完的chain:
<pre>
ngx_buf_size(cl->buf) != 0
</pre>
会被放到```ctx->busy```链中； 而已经处理完成chain会被放到```ctx->free```中（注：若```tag```已经修改了，则会调用ngx_free_chain()来将该chain直接放入到```ctx->pool->chain```中）。调用ngx_alloc_chain_link()时（参见core/ngx_buf.c中）：
{% highlight string %}
/*
 * 链接c1到pool->chain中
 */  
#define ngx_free_chain(pool, cl)                                             \  
    cl->next = pool->chain;                                                  \  
    pool->chain = cl  

/*
 * 从pool中分配chain
 */
ngx_chain_t *
ngx_alloc_chain_link(ngx_pool_t *pool)
{
    ngx_chain_t  *cl;

    cl = pool->chain;

    if (cl) {
        pool->chain = cl->next;
        return cl;
    }

    cl = ngx_palloc(pool, sizeof(ngx_chain_t));
    if (cl == NULL) {
        return NULL;
    }

    return cl;
}
{% endhighlight %}
上面我们看到如果pool->chain中存在chain的话，就不用malloc了，而是直接返回pool->chain。


3.2） buf重用

严格意义上来说，buf的重用是从free中的chain中取得的，当free中的buf被重用，则这个buf对应的chain就会被链接到ctx->pool中，从而这个chain就会被重用。也就是说首先考虑的是buf的重用，只有当这个chain的buf确定不需要被重用的时候，chain才会被链接到ctx->pool中被重用。 

另外还有一个就是```ctx->allocated```域，这个field表示了当前的上下文中已经分配了多少个buf，```output_buffer```命令用来设置output的buf的大小以及buf的个数。而```allocated```如果比```output_buffer```大的话，则需要先处理完已存在的buf，然后才能重新分配buf。

<br />

接下来我们分析代码，上面所说的重用以及buf的控制，代码里面都可以看的比较清晰，下面这段主要是拷贝buf前所做的一些工作，比如判断是否拷贝，以及给```buf```分配内存等，然后将整理好的数据交给```output_filter```来进行下一步处理（及发送)：
{% highlight string %}
ngx_int_t
ngx_output_chain(ngx_output_chain_ctx_t *ctx, ngx_chain_t *in)
{
	...

	out = NULL;

	/*
	* last_out在经过内层while循环之后，会执行out链中的最后一个节点
	*/
	last_out = &out;
	last = NGX_NONE;

	for ( ;; ){

		//开始遍历chain: 当ctx->in不为NULL，并且有足够的空闲buf的话，则不断将in拷贝到ctx->buf中
		//(注意前面的步骤我们已经将'in' 中的数据拷贝到了ctx->in中）
		while(ctx->in){

			//获取当前buf的大小
			bsize = ngx_buf_size(ctx->in->buf);	

			//当buf大小为0时，则跳过并打印相应的调试信息
			if (bsize == 0 && !ngx_buf_special(ctx->in->buf)) {
				ctx->in = ctx->in->next;
				continue;
			}

			//判断是否需要复制buf
			if (ngx_output_chain_as_is(ctx, ctx->in->buf)) {

			//不需要复制，直接将当前ctx->in追加到out链的末尾，同时ctx->in = ctx->in->next;

			continue;
			}

			/*
			* 如下是需要进行buf复制的情况
			*/

			//因为ctx->buf是属于一个临时用的buf，所以大多数情况下ctx->buf均为NULL
			if(ctx->buf == NULL){
		 
				/*
				 * 此函数用于处理文件buf(file buf), 因为有些发送函数可能有对齐要求。
				 * 一般来说，如果没有开启directio的话，这个函数都会返回NGX_DECLINED
				 */
				rc = ngx_output_chain_align_file_buf(ctx, bsize);
				if (rc == NGX_ERROR){
					return NGX_ERROR;
				}
				
				if (rc != NGX_OK){
					if(ctx->free){
					/*
					 * 从ctx->free中获取一个空闲的buf，并将该buf所对应的chain释放回pool->chain中，
					 * 这样就实现了上面介绍的chain复用。
					 */
					}else if(out || ctx->allocated == ctx->bufs.num){
						/*
						 * 如果已经等于buf的个数限制，则跳出循环，发送已经存在的buf。这里可以看到如果out
						 * 存在的话，nginx也会跳出循环，然后发送out，等发送完成后会再次处理，这里很好的
						 * 体现了nginx的流式处理
						 */
						 
						break;
					}else if (ngx_output_chain_get_buf(ctx, bsize) != NGX_OK){
						//上面这个函数也比较关键，它用来取得buf。接下来会详细看这个函数
						return NGX_ERROR;
					}
				}
				
			}
			
			/*从原来的buf中拷贝内容或者从文件中读取内容*/
			rc = ngx_output_chain_copy_buf(ctx);
			if (rc == NGX_ERROR){
				return rc;
			}

			if (rc == NGX_AGAIN){
				if(out){
					break;
				}
				return rc;
			}

			//删除当前ctx->in中已经处理完成的buf
			if (ngx_buf_size(ctx->in->buf) == 0) {
				ctx->in = ctx->in->next;
			}

			//将当前处理好的ctx->buf追加到out chain的末尾
			cl = ngx_alloc_chain_link(ctx->pool);
			if (cl == NULL) {
				return NGX_ERROR;
			}
			
			cl->buf = ctx->buf;
			cl->next = NULL;
			*last_out = cl;
			last_out = &cl->next;
			ctx->buf = NULL;
		}

		//继续处理上次没有完成的数据
		if (out == NULL && last != NGX_NONE) {
		
			if (ctx->in) {
				return NGX_AGAIN;
			}
		
			return last;
		}

		/*
		 * 调用output_filter来进行数据处理，并更新ctx->free链以及ctx->busy链
		 */
		last = ctx->output_filter(ctx->filter_ctx, out);

		if (last == NGX_ERROR || last == NGX_DONE) {
			return last;
		}
		
		ngx_chain_update_chains(ctx->pool, &ctx->free, &ctx->busy, &out,ctx->tag);
		last_out = &out;

	}
}
{% endhighlight %}



<br />
<br />

**[参看]**

1. [ngx_output_chain 函数分析](http://ju.outofmemory.cn/entry/137930)

2. [Nginx filter分析)](https://blog.csdn.net/fengmo_q/article/details/12494781)

3. [Nginx filter 模块解析（filter调用顺序)](https://yq.aliyun.com/ziliao/279082)

4. [nginx处理post之转发](https://m.aliyun.com/wanwang/info/1536018.html)

5. [nginx HTTP处理流程](https://www.cnblogs.com/improvement/p/6517814.html)

6. [nginx的十一个阶段处理](https://blog.csdn.net/esion23011/article/details/24057633)

7. [Development Guide](https://nginx.org/en/docs/dev/development_guide.html)

8. [nginx phase handler的原理和选择](https://blog.csdn.net/liujiyong7/article/details/38817135)

9. [nginx模块执行顺序分析](http://www.it165.net/admin/html/201212/590.html)

10. [Emiller’s Guide To Nginx Module Development](http://www.evanmiller.org/nginx-modules-guide.html)
<br />
<br />
<br />

