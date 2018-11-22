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

## 3. 函数ngx_output_chain_as_is()
{% highlight string %}
static ngx_inline ngx_int_t
ngx_output_chain_as_is(ngx_output_chain_ctx_t *ctx, ngx_buf_t *buf)
{
    ngx_uint_t  sendfile;

    //1) 是否为特殊buf(special buf)，是的话返回1，表示不需要拷贝
    if (ngx_buf_special(buf)) {
        return 1;
    }

    //当期暂未定义NGX_THREADS宏
#if (NGX_THREADS)
    if (buf->in_file) {
        buf->file->thread_handler = ctx->thread_handler;
        buf->file->thread_ctx = ctx->filter_ctx;
    }
#endif

    //2) 如果buf在文件中，并且使用了directio，那么返回0，表示需要拷贝（这是因为directio一般有对齐要求） 
    if (buf->in_file && buf->file->directio) {
        return 0;
    }

    // sendfile标记
    sendfile = ctx->sendfile;

#if (NGX_SENDFILE_LIMIT)

    //如果file_pos大于sendfile的限制，设置标记为0（表示不能用sendfile来发送）
    if (buf->in_file && buf->file_pos >= NGX_SENDFILE_LIMIT) {
        sendfile = 0;
    }

#endif

    if (!sendfile) {

        //如果不走sendfile，而且buf不在内存中，则我们需要复制到内存一份
        if (!ngx_buf_in_memory(buf)) {
            return 0;
        }

        buf->in_file = 0;
    }

    //当前我们并不支持NGX_HAVE_AIO_SENDFILE宏定义
#if (NGX_HAVE_AIO_SENDFILE)
    if (ctx->aio_preload && buf->in_file) {
        (void) ngx_output_chain_aio_setup(ctx, buf->file);
    }
#endif

    //如果ctx要求需要在内存，而当前的buf又不在内存，则我们需要复制到内存一份
    if (ctx->need_in_memory && !ngx_buf_in_memory(buf)) {
        return 0;
    }

    //如果需要内存中有可修改的拷贝，并且buf存在于只读的内存或者mmap中，则返回0，表示需要复制到内存一份
    if (ctx->need_in_temp && (buf->memory || buf->mmap)) {
        return 0;
    }

    return 1;
}
{% endhighlight %}
此函数主要用于判定是否需要复制buf。如果返回值为1，则说明不需要复制，否则说明需要复制。上面我们注意两个标记：

* ctx->need_in_memory: 主要用于当使用sendfile的时候，Nginx并不会将请求文件拷贝到内存中，而有时需要操作文件的内容，此时就需要设置这个标记。然后后面的body filter就能操作内容了。

* ctx->need_in_temp: 这个主要是用于把本来就存在于内存中的buf复制一份可修改的拷贝出来，这里有用到的模块有```charset```，也就是编码filter。


## 4. 函数ngx_output_chain_aio_setup()
{% highlight string %}
#if (NGX_HAVE_AIO_SENDFILE)

static ngx_int_t
ngx_output_chain_aio_setup(ngx_output_chain_ctx_t *ctx, ngx_file_t *file)
{
    ngx_event_aio_t  *aio;

    if (file->aio == NULL && ngx_file_aio_init(file, ctx->pool) != NGX_OK) {
        return NGX_ERROR;
    }

    aio = file->aio;

    aio->data = ctx->filter_ctx;
    aio->preload_handler = ctx->aio_preload;

    return NGX_OK;
}

#endif
{% endhighlight %}
当前我们并不支持```NGX_HAVE_ASIO_SENDFILE```，这里不做介绍。

## 5. 函数ngx_output_chain_add_copy()
{% highlight string %}
static ngx_int_t
ngx_output_chain_add_copy(ngx_pool_t *pool, ngx_chain_t **chain,
    ngx_chain_t *in)
{
    ngx_chain_t  *cl, **ll;
#if (NGX_SENDFILE_LIMIT)
    ngx_buf_t    *b, *buf;
#endif

    ll = chain;

    for (cl = *chain; cl; cl = cl->next) {
        ll = &cl->next;
    }

    while (in) {

        cl = ngx_alloc_chain_link(pool);
        if (cl == NULL) {
            return NGX_ERROR;
        }

#if (NGX_SENDFILE_LIMIT)

        buf = in->buf;

        if (buf->in_file
            && buf->file_pos < NGX_SENDFILE_LIMIT
            && buf->file_last > NGX_SENDFILE_LIMIT)
        {
            /* split a file buf on two bufs by the sendfile limit */

            b = ngx_calloc_buf(pool);
            if (b == NULL) {
                return NGX_ERROR;
            }

            ngx_memcpy(b, buf, sizeof(ngx_buf_t));

            if (ngx_buf_in_memory(buf)) {
                buf->pos += (ssize_t) (NGX_SENDFILE_LIMIT - buf->file_pos);
                b->last = buf->pos;
            }

            buf->file_pos = NGX_SENDFILE_LIMIT;
            b->file_last = NGX_SENDFILE_LIMIT;

            cl->buf = b;

        } else {
            cl->buf = buf;
            in = in->next;
        }

#else
        cl->buf = in->buf;
        in = in->next;

#endif

        cl->next = NULL;
        *ll = cl;
        ll = &cl->next;
    }

    return NGX_OK;
}
{% endhighlight %}
本函数用于将```in```中的数据拷贝到```chain```中。下面我们简要分析一下本函数：
{% highlight string %}
static ngx_int_t
ngx_output_chain_add_copy(ngx_pool_t *pool, ngx_chain_t **chain,
    ngx_chain_t *in)
{
	ngx_chain_t  *cl, **ll;

	1) 找到chain的末尾指针，保存到ll中

	while(in)
	{
		//2) 分配一个空的ngx_chain_t结构
		cl = ngx_alloc_chain_link(pool);

		#if (NGX_SENDFILE_LIMIT)
			//当前我们定义了NGX_SENDFILE_LIMIT宏，因此会走此分支

			buf = in->buf;
			
			if (buf->in_file
			  && buf->file_pos < NGX_SENDFILE_LIMIT
			  && buf->file_last > NGX_SENDFILE_LIMIT)
			{
				//3） 当前的buf在文件中，并且内容横跨NGX_SENDFILE_LIMIT两侧，此时将其分割成两个buf
                //其中： 变量b存放前半段， 变量buf存放后半段
				//然后，将前半段b保存在上面分配的'ngx_chain_t'结构中，后半段buf留作while的下一次循环进行处理（这里我们看到in=in->next)
			}
			else{
				//4) 直接拷贝buf
				cl->buf = buf;
				in = in->next;
			}
		#else
			//直接拷贝buf
			cl->buf = in->buf;
			in = in->next;
		
		#endif

		//5) 将cl追加到chain的末尾
	}	
}
{% endhighlight %}

## 6. 函数ngx_output_chain_align_file_buf()
{% highlight string %}
static ngx_int_t
ngx_output_chain_align_file_buf(ngx_output_chain_ctx_t *ctx, off_t bsize)
{
    size_t      size;
    ngx_buf_t  *in;

    in = ctx->in->buf;

    if (in->file == NULL || !in->file->directio) {
        return NGX_DECLINED;
    }

    ctx->directio = 1;

    size = (size_t) (in->file_pos - (in->file_pos & ~(ctx->alignment - 1)));

    if (size == 0) {

        if (bsize >= (off_t) ctx->bufs.size) {
            return NGX_DECLINED;
        }

        size = (size_t) bsize;

    } else {
        size = (size_t) ctx->alignment - size;

        if ((off_t) size > bsize) {
            size = (size_t) bsize;
        }
    }

    ctx->buf = ngx_create_temp_buf(ctx->pool, size);
    if (ctx->buf == NULL) {
        return NGX_ERROR;
    }

    /*
     * we do not set ctx->buf->tag, because we do not want
     * to reuse the buf via ctx->free list
     */

#if (NGX_HAVE_ALIGNED_DIRECTIO)
    ctx->unaligned = 1;
#endif

    return NGX_OK;
}
{% endhighlight %}

本函数用于对齐file buf，因为使用directio发送file buf时，会有对齐方面的要求。下面我们简单分析下本函数的实现：
{% highlight string %}
static ngx_int_t
ngx_output_chain_align_file_buf(ngx_output_chain_ctx_t *ctx, off_t bsize)
{
	in = ctx->in->buf;
	
	//1） 不是file buf或者不采用directio，则直接返回，不需要对齐
	if (in->file == NULL || !in->file->directio) {
		return NGX_DECLINED;
	}

	//2) 将ctx->directio设置为1，表示采用directio发送
	ctx->directio = 1;
	
	size = (size_t) (in->file_pos - (in->file_pos & ~(ctx->alignment - 1)));

	if (size == 0)
	{
		//3） 当前in->file_pos已经是ctx->alignment对齐了

		//若当前ctx->in->buf的实际大小大于等于ctx事先分配的一个缓存大小，则直接返回相应错误
		if (bsize >= (off_t) ctx->bufs.size) {
			return NGX_DECLINED;
		}
		
		//此处保存后面实际要分配的空间大小
		size = (size_t) bsize;
	}
	else{
		//4) 当前没有对齐，则先求出未对齐部分的大小
		size = (size_t) ctx->alignment - size;
		
		if ((off_t) size > bsize) {

			//此处保存后面实际要分配的空间大小
			size = (size_t) bsize;
		}
	}

	//5) 分配一块指定大小的缓存
	
	//6） 将当前ctx->unaligned标志设置为1（当前我们定义了NGX_HAVE_ALIGNED_DIRECTIO宏)
#if (NGX_HAVE_ALIGNED_DIRECTIO)
    ctx->unaligned = 1;
#endif
}
{% endhighlight %}

## 7. 函数ngx_output_chain_get_buf()
{% highlight string %}
static ngx_int_t
ngx_output_chain_get_buf(ngx_output_chain_ctx_t *ctx, off_t bsize)
{
    size_t       size;
    ngx_buf_t   *b, *in;
    ngx_uint_t   recycled;

    in = ctx->in->buf;
    size = ctx->bufs.size;
    recycled = 1;

    if (in->last_in_chain) {

        if (bsize < (off_t) size) {

            /*
             * allocate a small temp buf for a small last buf
             * or its small last part
             */

            size = (size_t) bsize;
            recycled = 0;

        } else if (!ctx->directio
                   && ctx->bufs.num == 1
                   && (bsize < (off_t) (size + size / 4)))
        {
            /*
             * allocate a temp buf that equals to a last buf,
             * if there is no directio, the last buf size is lesser
             * than 1.25 of bufs.size and the temp buf is single
             */

            size = (size_t) bsize;
            recycled = 0;
        }
    }

    b = ngx_calloc_buf(ctx->pool);
    if (b == NULL) {
        return NGX_ERROR;
    }

    if (ctx->directio) {

        /*
         * allocate block aligned to a disk sector size to enable
         * userland buffer direct usage conjunctly with directio
         */

        b->start = ngx_pmemalign(ctx->pool, size, (size_t) ctx->alignment);
        if (b->start == NULL) {
            return NGX_ERROR;
        }

    } else {
        b->start = ngx_palloc(ctx->pool, size);
        if (b->start == NULL) {
            return NGX_ERROR;
        }
    }

    b->pos = b->start;
    b->last = b->start;
    b->end = b->last + size;
    b->temporary = 1;
    b->tag = ctx->tag;
    b->recycled = recycled;

    ctx->buf = b;
    ctx->allocated++;

    return NGX_OK;
}
{% endhighlight %}
本函数用于分配一块指定大小的空间给ctx->buf。下面简要分析一下本函数：
{% highlight string %}
static ngx_int_t
ngx_output_chain_get_buf(ngx_output_chain_ctx_t *ctx, off_t bsize)
{
	in = ctx->in->buf; 	

	/* 可以看到这里分配的buf，每个buf的大小是配置文件中设置的size */  
	size = ctx->bufs.size;  

	/*
	 * 这里recycled标志表示当前的buf是否需要被回收。一般情况下Nginx(比如在非last_buf)会缓存一部分buf(默认是1460字节），然后再发送，
	 * 而设置了recycled的话，就不会让它缓存buf，也就是尽量发送出去，然后以供回收使用。因此如果是最后一个buf,则不需要设置recycled域，
	 * 否则的话，需要设置recycled域
	 *
	 * 这里默认设置为不需要缓存
	 */
	recycled = 1;

	1) 当前buf是属于最后一个chain的时候，需要特殊处理
	if (in->last_in_chain){

		if (bsize < (off_t) size){
			//实际要分配的缓存大小小于配置文件所设置的每块缓存大小
			size = bsize;
			recycled = 0;
		}else if (!ctx->directio
				&& ctx->bufs.num == 1
				&& (bsize < (off_t) (size + size / 4)))
		{
			size = bsize;
			recycled = 0;
		}
	}

	//2) 分配一个ngx_buf_t结构
	b = ngx_calloc_buf(ctx->pool);

	//3) 为b分配实际的内存空间
	
	//4)将分配的空间赋值给ctx->buf
	ctx->buf = b;
	ctx->allocated++;
}
{% endhighlight %}


## 8. 函数ngx_output_chain_copy_buf()
{% highlight string %}
static ngx_int_t
ngx_output_chain_copy_buf(ngx_output_chain_ctx_t *ctx)
{
    off_t        size;
    ssize_t      n;
    ngx_buf_t   *src, *dst;
    ngx_uint_t   sendfile;

    src = ctx->in->buf;
    dst = ctx->buf;

    size = ngx_buf_size(src);
    size = ngx_min(size, dst->end - dst->pos);

    sendfile = ctx->sendfile & !ctx->directio;

#if (NGX_SENDFILE_LIMIT)

    if (src->in_file && src->file_pos >= NGX_SENDFILE_LIMIT) {
        sendfile = 0;
    }

#endif

    if (ngx_buf_in_memory(src)) {
        ngx_memcpy(dst->pos, src->pos, (size_t) size);
        src->pos += (size_t) size;
        dst->last += (size_t) size;

        if (src->in_file) {

            if (sendfile) {
                dst->in_file = 1;
                dst->file = src->file;
                dst->file_pos = src->file_pos;
                dst->file_last = src->file_pos + size;

            } else {
                dst->in_file = 0;
            }

            src->file_pos += size;

        } else {
            dst->in_file = 0;
        }

        if (src->pos == src->last) {
            dst->flush = src->flush;
            dst->last_buf = src->last_buf;
            dst->last_in_chain = src->last_in_chain;
        }

    } else {

#if (NGX_HAVE_ALIGNED_DIRECTIO)

        if (ctx->unaligned) {
            if (ngx_directio_off(src->file->fd) == NGX_FILE_ERROR) {
                ngx_log_error(NGX_LOG_ALERT, ctx->pool->log, ngx_errno,
                              ngx_directio_off_n " \"%s\" failed",
                              src->file->name.data);
            }
        }

#endif

#if (NGX_HAVE_FILE_AIO)
        if (ctx->aio_handler) {
            n = ngx_file_aio_read(src->file, dst->pos, (size_t) size,
                                  src->file_pos, ctx->pool);
            if (n == NGX_AGAIN) {
                ctx->aio_handler(ctx, src->file);
                return NGX_AGAIN;
            }

        } else
#endif
#if (NGX_THREADS)
        if (ctx->thread_handler) {
            src->file->thread_task = ctx->thread_task;
            src->file->thread_handler = ctx->thread_handler;
            src->file->thread_ctx = ctx->filter_ctx;

            n = ngx_thread_read(src->file, dst->pos, (size_t) size,
                                src->file_pos, ctx->pool);
            if (n == NGX_AGAIN) {
                ctx->thread_task = src->file->thread_task;
                return NGX_AGAIN;
            }

        } else
#endif
        {
            n = ngx_read_file(src->file, dst->pos, (size_t) size,
                              src->file_pos);
        }

#if (NGX_HAVE_ALIGNED_DIRECTIO)

        if (ctx->unaligned) {
            ngx_err_t  err;

            err = ngx_errno;

            if (ngx_directio_on(src->file->fd) == NGX_FILE_ERROR) {
                ngx_log_error(NGX_LOG_ALERT, ctx->pool->log, ngx_errno,
                              ngx_directio_on_n " \"%s\" failed",
                              src->file->name.data);
            }

            ngx_set_errno(err);

            ctx->unaligned = 0;
        }

#endif

        if (n == NGX_ERROR) {
            return (ngx_int_t) n;
        }

        if (n != size) {
            ngx_log_error(NGX_LOG_ALERT, ctx->pool->log, 0,
                          ngx_read_file_n " read only %z of %O from \"%s\"",
                          n, size, src->file->name.data);
            return NGX_ERROR;
        }

        dst->last += n;

        if (sendfile) {
            dst->in_file = 1;
            dst->file = src->file;
            dst->file_pos = src->file_pos;
            dst->file_last = src->file_pos + n;

        } else {
            dst->in_file = 0;
        }

        src->file_pos += n;

        if (src->file_pos == src->file_last) {
            dst->flush = src->flush;
            dst->last_buf = src->last_buf;
            dst->last_in_chain = src->last_in_chain;
        }
    }

    return NGX_OK;
}
{% endhighlight %}
本函数用于将```ctx->in```中的buf拷贝到```ctx->buf```这样一个临时缓存中进行处理。下面我们简要分析：
{% highlight string %}
static ngx_int_t
ngx_output_chain_copy_buf(ngx_output_chain_ctx_t *ctx)
{
	src = ctx->in->buf;
    dst = ctx->buf;

    size = ngx_buf_size(src);
    size = ngx_min(size, dst->end - dst->pos);

	//是否真正的sendfile()文件的标志
    sendfile = ctx->sendfile & !ctx->directio;

	if (ngx_buf_in_memory(src)){
		//1) src属于内存buf

		//1.1) 调用ngx_memcpy()进行内存文件复制

		//1.2) 假如src本身是属于文件（这里不冲突，因为内存buf有可能是通过mmap等映射而来的）
		if(src->in_file)
		{
			// 处理文件发送情况
		}else{
			dst->in_file = 0;
		}

		

	}else{
		//2) 非内存buf

		#if (NGX_HAVE_ALIGNED_DIRECTIO)
			//当前我们定义了此宏
			if (ctx->unaligned) {
				//这里关闭主要是为了下面的read读取
			}
		#endif

		#if(NGX_HAVE_FILE_AIO)
			//当前我们暂时未定义此宏
		#endif

		#if(NGX_THREADS)
			//当前也并未定义
		#endif

		//2.1) 读取文件到dst
		n = ngx_read_file(src->file, dst->pos, (size_t) size,src->file_pos);

		#if (NGX_HAVE_ALIGNED_DIRECTIO)
			if (ctx->unaligned) {
				//开启directio
			}
		#endif
		
		//2.2) 设置dst变量的相应标志值
	}

	return NGX_OK;
}
{% endhighlight %}


## 9. 函数ngx_chain_writer()
{% highlight string %}
ngx_int_t
ngx_chain_writer(void *data, ngx_chain_t *in)
{
    ngx_chain_writer_ctx_t *ctx = data;

    off_t              size;
    ngx_chain_t       *cl, *ln, *chain;
    ngx_connection_t  *c;

    c = ctx->connection;

    for (size = 0; in; in = in->next) {

#if 1
        if (ngx_buf_size(in->buf) == 0 && !ngx_buf_special(in->buf)) {

            ngx_log_error(NGX_LOG_ALERT, ctx->pool->log, 0,
                          "zero size buf in chain writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          in->buf->temporary,
                          in->buf->recycled,
                          in->buf->in_file,
                          in->buf->start,
                          in->buf->pos,
                          in->buf->last,
                          in->buf->file,
                          in->buf->file_pos,
                          in->buf->file_last);

            ngx_debug_point();

            continue;
        }
#endif

        size += ngx_buf_size(in->buf);

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, c->log, 0,
                       "chain writer buf fl:%d s:%uO",
                       in->buf->flush, ngx_buf_size(in->buf));

        cl = ngx_alloc_chain_link(ctx->pool);
        if (cl == NULL) {
            return NGX_ERROR;
        }

        cl->buf = in->buf;
        cl->next = NULL;
        *ctx->last = cl;
        ctx->last = &cl->next;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, c->log, 0,
                   "chain writer in: %p", ctx->out);

    for (cl = ctx->out; cl; cl = cl->next) {

#if 1
        if (ngx_buf_size(cl->buf) == 0 && !ngx_buf_special(cl->buf)) {

            ngx_log_error(NGX_LOG_ALERT, ctx->pool->log, 0,
                          "zero size buf in chain writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();

            continue;
        }
#endif

        size += ngx_buf_size(cl->buf);
    }

    if (size == 0 && !c->buffered) {
        return NGX_OK;
    }

    chain = c->send_chain(c, ctx->out, ctx->limit);

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, c->log, 0,
                   "chain writer out: %p", chain);

    if (chain == NGX_CHAIN_ERROR) {
        return NGX_ERROR;
    }

    for (cl = ctx->out; cl && cl != chain; /* void */) {
        ln = cl;
        cl = cl->next;
        ngx_free_chain(ctx->pool, ln);
    }

    ctx->out = chain;

    if (ctx->out == NULL) {
        ctx->last = &ctx->out;

        if (!c->buffered) {
            return NGX_OK;
        }
    }

    return NGX_AGAIN;
}
{% endhighlight %}
此函数是用来真正调用```connection```来将chain中的数据发送出去。下面我们简要分析一下该函数的实现：
{% highlight string %}
ngx_int_t
ngx_chain_writer(void *data, ngx_chain_t *in)
{
	ngx_chain_writer_ctx_t *ctx = data;
	c = ctx->connection;

	//遍历in链
	for (size = 0; in; in = in->next) {
		if (ngx_buf_size(in->buf) == 0 && !ngx_buf_special(in->buf)) {
			//1) 当buf大小为0时，则跳过并打印相应的调试信息
			continue;
		}

		//2) 将in中的数据追加到ctx->out这个缓存中
	}

	//3) 计算ctx->out总的数据的大小

	//4) 当前没有数据要发送，直接返回
	if (size == 0 && !c->buffered) {
		return NGX_OK;
	}

	//5) 发送数据,返回值chain表示当前已经发送到哪一个chain了
	chain = c->send_chain(c, ctx->out, ctx->limit);

	//6) 后续空间回收等相关操作
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

