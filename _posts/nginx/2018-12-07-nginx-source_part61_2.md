---
layout: post
title: core/ngx_slab.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们介绍一下Nginx中基于slab的内存分配机制。




<!-- more -->

## 1. 相关宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_SLAB_PAGE_MASK   3
#define NGX_SLAB_PAGE        0
#define NGX_SLAB_BIG         1
#define NGX_SLAB_EXACT       2
#define NGX_SLAB_SMALL       3

#if (NGX_PTR_SIZE == 4)

#define NGX_SLAB_PAGE_FREE   0
#define NGX_SLAB_PAGE_BUSY   0xffffffff
#define NGX_SLAB_PAGE_START  0x80000000

#define NGX_SLAB_SHIFT_MASK  0x0000000f
#define NGX_SLAB_MAP_MASK    0xffff0000
#define NGX_SLAB_MAP_SHIFT   16

#define NGX_SLAB_BUSY        0xffffffff

#else /* (NGX_PTR_SIZE == 8) */

#define NGX_SLAB_PAGE_FREE   0
#define NGX_SLAB_PAGE_BUSY   0xffffffffffffffff
#define NGX_SLAB_PAGE_START  0x8000000000000000

#define NGX_SLAB_SHIFT_MASK  0x000000000000000f
#define NGX_SLAB_MAP_MASK    0xffffffff00000000
#define NGX_SLAB_MAP_SHIFT   32

#define NGX_SLAB_BUSY        0xffffffffffffffff

#endif


#if (NGX_DEBUG_MALLOC)

#define ngx_slab_junk(p, size)     ngx_memset(p, 0xA5, size)

#elif (NGX_HAVE_DEBUG_MALLOC)

#define ngx_slab_junk(p, size)                                                \
    if (ngx_debug_malloc)          ngx_memset(p, 0xA5, size)

#else

#define ngx_slab_junk(p, size)

#endif
{% endhighlight %}

1) **所分配的内存大小尺寸**

这里用```2位```来表示所分配的内存大小尺寸，2位最多可以表示4种尺寸：

* NGX_SLAB_PAGE_MASK: 表示尺寸的掩码

* NGX_SLAB_PAGE: 超大内存，大于等于ngx_slab_max_size

* NGX_SLAB_BIG： 大块内存，大于ngx_slab_exact_size而小于ngx_slab_max_size

* NGX_SLAB_EXACT： 中等内存，等于ngx_slab_exact_size

* NGX_SLAB_SMALL： 小块内存，小于ngx_slab_exact_size

下面是exact size与max size的计算：
<pre>
ngx_slab_max_size = ngx_pagesize / 2;

ngx_slab_exact_size = ngx_pagesize / (8 * sizeof(uintptr_t));
</pre>

2) **掩码设置**

在当前我们使用的32bit Ubuntu环境中，```NGX_PTR_SIZE```的值为4，因此：

* NGX_SLAB_PAGE_FREE: 值为0， 用于指示当前page处于空闲状态

* NGX_SLAB_PAGE_BUSY: 值为0xffffffff，用于指示当前page处于busy状态（针对大于ngx_slab_max_size的内存块）

* NGX_SLAB_PAGE_START: 值为0x80000000，用于指示连续多个页面的开始页

* NGX_SLAB_SHIFT_MASK： 值为0x0000000f， 当所分配的空间大于ngx_slab_exact_size且小于等于ngx_slab_max_size时，page->slab的高NGX_SLAB_MAP_MASK位用作位图，用于标识当前```页```中内存块的分配情况，低NGX_SLAB_SHIFT_MASK位用于标识当前页的内存块大小```移位```的掩码。比如当前内存块大小为256，即```1<<8```，那么page->slab的低NGX_SLAB_SHIFT_MASK位的值就是8。

* NGX_SLAB_MAP_MASK： 值为0xffff0000， 当所分配的空间大于ngx_slab_exact_size且小于等于ngx_slab_max_size时， page->slab的高NGX_SLAB_MAP_MASK位用作位图

* NGX_SLAB_MAP_SHIFT: 值为16，当所分配的空间大于ngx_slab_exact_size且小于等于ngx_slab_max_size时， page->slab的高16位作为位图

* NGX_SLAB_BUSY: 值为0xffffffff, 用于表示当前slab处于busy状态（针对小于等于ngx_slab_max_size的内存块）


3） **调试信息**

我们当前并不支持```NGX_DEBUG_MALLOC```与```NGX_HAVE_DEBUG_MALLOC```，因此执行的是如下宏：
<pre>
#define ngx_slab_junk(p, size)
</pre>


## 2. 相关静态函数声明及变量定义
{% highlight string %}
//用于分配指定数量的页
static ngx_slab_page_t *ngx_slab_alloc_pages(ngx_slab_pool_t *pool,
    ngx_uint_t pages);

//释放指定数量的页
static void ngx_slab_free_pages(ngx_slab_pool_t *pool, ngx_slab_page_t *page,
    ngx_uint_t pages);

//打印slab相关的错误日志
static void ngx_slab_error(ngx_slab_pool_t *pool, ngx_uint_t level,
    char *text);

static ngx_uint_t  ngx_slab_max_size;
static ngx_uint_t  ngx_slab_exact_size;
static ngx_uint_t  ngx_slab_exact_shift;
{% endhighlight %}


## 3. 函数ngx_slab_init()
{% highlight string %}
void
ngx_slab_init(ngx_slab_pool_t *pool)
{
    u_char           *p;
    size_t            size;
    ngx_int_t         m;
    ngx_uint_t        i, n, pages;
    ngx_slab_page_t  *slots;

    /* STUB */
    if (ngx_slab_max_size == 0) {
        ngx_slab_max_size = ngx_pagesize / 2;
        ngx_slab_exact_size = ngx_pagesize / (8 * sizeof(uintptr_t));
        for (n = ngx_slab_exact_size; n >>= 1; ngx_slab_exact_shift++) {
            /* void */
        }
    }
    /**/

    pool->min_size = 1 << pool->min_shift;

    p = (u_char *) pool + sizeof(ngx_slab_pool_t);
    size = pool->end - p;

    ngx_slab_junk(p, size);

    slots = (ngx_slab_page_t *) p;
    n = ngx_pagesize_shift - pool->min_shift;

    for (i = 0; i < n; i++) {
        slots[i].slab = 0;
        slots[i].next = &slots[i];
        slots[i].prev = 0;
    }

    p += n * sizeof(ngx_slab_page_t);

    pages = (ngx_uint_t) (size / (ngx_pagesize + sizeof(ngx_slab_page_t)));

    ngx_memzero(p, pages * sizeof(ngx_slab_page_t));

    pool->pages = (ngx_slab_page_t *) p;

    pool->free.prev = 0;
    pool->free.next = (ngx_slab_page_t *) p;

    pool->pages->slab = pages;
    pool->pages->next = &pool->free;
    pool->pages->prev = (uintptr_t) &pool->free;

    pool->start = (u_char *)
                  ngx_align_ptr((uintptr_t) p + pages * sizeof(ngx_slab_page_t),
                                 ngx_pagesize);

    m = pages - (pool->end - pool->start) / ngx_pagesize;
    if (m > 0) {
        pages -= m;
        pool->pages->slab = pages;
    }

    pool->last = pool->pages + pages;

    pool->log_nomem = 1;
    pool->log_ctx = &pool->zero;
    pool->zero = '\0';
}
{% endhighlight %}

本函数用于初始化整个ngx_slab_pool_t结构，下面我们简要分析一下函数的实现：
{% highlight string %}
void
ngx_slab_init(ngx_slab_pool_t *pool)
{
	//1) 计算ngx_slab_max_size，ngx_slab_exact_size， ngx_slab_exact_shift
	// 从前面我们知道ngx_pagesize=4096, 因此
	ngx_slab_max_size = 2048;
	ngx_slab_exact_size = 128;
	ngx_slab_exact_shift = 7;

	//2) 计算得到min_size大小及n的值
	pool->min_size = 8;			//(pool->min_shift值为3）
	ngx_pagesize_shift = 12;
	n = 9;

	//3) 初始化slots
	for (i = 0; i < n; i++) {
        slots[i].slab = 0;
        slots[i].next = &slots[i];
        slots[i].prev = 0;
    }

	//4) 计算剩余空间最多能分配多少个页（每一页大小为ngx_pagesize，另外每一页还对应一个ngx_slab_page_t管理结构)
	pages = (ngx_uint_t) (size / (ngx_pagesize + sizeof(ngx_slab_page_t)));

	//5) 初始化pool->pages及pool->free链表
	pool->pages->slab = pages;			//slab用于记录空闲页数

	//6) 计算实际空闲页数
	m = pages - (pool->end - pool->start) / ngx_pagesize;
    if (m > 0) {
        pages -= m;
        pool->pages->slab = pages;
    }

    pool->last = pool->pages + pages;

	//7)将pool->zero的值置为'\0'
	//pool->zero = '\0';

}
{% endhighlight %}
下面我们画出初始化之后的示意图：

![ngx-slab-init](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_slab_init.jpg)


## 4. 函数ngx_slab_alloc()
{% highlight string %}
void *
ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size)
{
    void  *p;

    ngx_shmtx_lock(&pool->mutex);

    p = ngx_slab_alloc_locked(pool, size);

    ngx_shmtx_unlock(&pool->mutex);

    return p;
}
{% endhighlight %}
本函数首先使用```pool->mutex```加锁，然后再调用ngx_slab_alloc_locked()函数分配指定大小的空间。

## 5. 函数ngx_slab_alloc_locked()
{% highlight string %}
void *
ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size)
{
    size_t            s;
    uintptr_t         p, n, m, mask, *bitmap;
    ngx_uint_t        i, slot, shift, map;
    ngx_slab_page_t  *page, *prev, *slots;

    if (size > ngx_slab_max_size) {

        ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, ngx_cycle->log, 0,
                       "slab alloc: %uz", size);

        page = ngx_slab_alloc_pages(pool, (size >> ngx_pagesize_shift)
                                          + ((size % ngx_pagesize) ? 1 : 0));
        if (page) {
            p = (page - pool->pages) << ngx_pagesize_shift;
            p += (uintptr_t) pool->start;

        } else {
            p = 0;
        }

        goto done;
    }

    if (size > pool->min_size) {
        shift = 1;
        for (s = size - 1; s >>= 1; shift++) { /* void */ }
        slot = shift - pool->min_shift;

    } else {
        size = pool->min_size;
        shift = pool->min_shift;
        slot = 0;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, ngx_cycle->log, 0,
                   "slab alloc: %uz slot: %ui", size, slot);

    slots = (ngx_slab_page_t *) ((u_char *) pool + sizeof(ngx_slab_pool_t));
    page = slots[slot].next;

    if (page->next != page) {

        if (shift < ngx_slab_exact_shift) {

            do {
                p = (page - pool->pages) << ngx_pagesize_shift;
                bitmap = (uintptr_t *) (pool->start + p);

                map = (1 << (ngx_pagesize_shift - shift))
                          / (sizeof(uintptr_t) * 8);

                for (n = 0; n < map; n++) {

                    if (bitmap[n] != NGX_SLAB_BUSY) {

                        for (m = 1, i = 0; m; m <<= 1, i++) {
                            if ((bitmap[n] & m)) {
                                continue;
                            }

                            bitmap[n] |= m;

                            i = ((n * sizeof(uintptr_t) * 8) << shift)
                                + (i << shift);

                            if (bitmap[n] == NGX_SLAB_BUSY) {
                                for (n = n + 1; n < map; n++) {
                                    if (bitmap[n] != NGX_SLAB_BUSY) {
                                        p = (uintptr_t) bitmap + i;

                                        goto done;
                                    }
                                }

                                prev = (ngx_slab_page_t *)
                                            (page->prev & ~NGX_SLAB_PAGE_MASK);
                                prev->next = page->next;
                                page->next->prev = page->prev;

                                page->next = NULL;
                                page->prev = NGX_SLAB_SMALL;
                            }

                            p = (uintptr_t) bitmap + i;

                            goto done;
                        }
                    }
                }

                page = page->next;

            } while (page);

        } else if (shift == ngx_slab_exact_shift) {

            do {
                if (page->slab != NGX_SLAB_BUSY) {

                    for (m = 1, i = 0; m; m <<= 1, i++) {
                        if ((page->slab & m)) {
                            continue;
                        }

                        page->slab |= m;

                        if (page->slab == NGX_SLAB_BUSY) {
                            prev = (ngx_slab_page_t *)
                                            (page->prev & ~NGX_SLAB_PAGE_MASK);
                            prev->next = page->next;
                            page->next->prev = page->prev;

                            page->next = NULL;
                            page->prev = NGX_SLAB_EXACT;
                        }

                        p = (page - pool->pages) << ngx_pagesize_shift;
                        p += i << shift;
                        p += (uintptr_t) pool->start;

                        goto done;
                    }
                }

                page = page->next;

            } while (page);

        } else { /* shift > ngx_slab_exact_shift */

            n = ngx_pagesize_shift - (page->slab & NGX_SLAB_SHIFT_MASK);
            n = 1 << n;
            n = ((uintptr_t) 1 << n) - 1;
            mask = n << NGX_SLAB_MAP_SHIFT;

            do {
                if ((page->slab & NGX_SLAB_MAP_MASK) != mask) {

                    for (m = (uintptr_t) 1 << NGX_SLAB_MAP_SHIFT, i = 0;
                         m & mask;
                         m <<= 1, i++)
                    {
                        if ((page->slab & m)) {
                            continue;
                        }

                        page->slab |= m;

                        if ((page->slab & NGX_SLAB_MAP_MASK) == mask) {
                            prev = (ngx_slab_page_t *)
                                            (page->prev & ~NGX_SLAB_PAGE_MASK);
                            prev->next = page->next;
                            page->next->prev = page->prev;

                            page->next = NULL;
                            page->prev = NGX_SLAB_BIG;
                        }

                        p = (page - pool->pages) << ngx_pagesize_shift;
                        p += i << shift;
                        p += (uintptr_t) pool->start;

                        goto done;
                    }
                }

                page = page->next;

            } while (page);
        }
    }

    page = ngx_slab_alloc_pages(pool, 1);

    if (page) {
        if (shift < ngx_slab_exact_shift) {
            p = (page - pool->pages) << ngx_pagesize_shift;
            bitmap = (uintptr_t *) (pool->start + p);

            s = 1 << shift;
            n = (1 << (ngx_pagesize_shift - shift)) / 8 / s;

            if (n == 0) {
                n = 1;
            }

            bitmap[0] = (2 << n) - 1;

            map = (1 << (ngx_pagesize_shift - shift)) / (sizeof(uintptr_t) * 8);

            for (i = 1; i < map; i++) {
                bitmap[i] = 0;
            }

            page->slab = shift;
            page->next = &slots[slot];
            page->prev = (uintptr_t) &slots[slot] | NGX_SLAB_SMALL;

            slots[slot].next = page;

            p = ((page - pool->pages) << ngx_pagesize_shift) + s * n;
            p += (uintptr_t) pool->start;

            goto done;

        } else if (shift == ngx_slab_exact_shift) {

            page->slab = 1;
            page->next = &slots[slot];
            page->prev = (uintptr_t) &slots[slot] | NGX_SLAB_EXACT;

            slots[slot].next = page;

            p = (page - pool->pages) << ngx_pagesize_shift;
            p += (uintptr_t) pool->start;

            goto done;

        } else { /* shift > ngx_slab_exact_shift */

            page->slab = ((uintptr_t) 1 << NGX_SLAB_MAP_SHIFT) | shift;
            page->next = &slots[slot];
            page->prev = (uintptr_t) &slots[slot] | NGX_SLAB_BIG;

            slots[slot].next = page;

            p = (page - pool->pages) << ngx_pagesize_shift;
            p += (uintptr_t) pool->start;

            goto done;
        }
    }

    p = 0;

done:

    ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, ngx_cycle->log, 0,
                   "slab alloc: %p", (void *) p);

    return (void *) p;
}
{% endhighlight %}
本函数用于分配指定大小的内存空间。这里slab将不等长的内存大小划分为4个大类：

* 小块内存(NGX_SLAB_SMALL): 内存大小 < ngx_slab_exact_size

* 中等内存(NGX_SLAB_EXACT): 内存大小 == ngx_slab_exact_size

* 大块内存(NGX_SLAB_BIG): ngx_slab_exact_size < 内存大小 <= ngx_slab_max_size

* 超大内存(NGX_SLAB_PAGE): ngx_slab_max_size < 内存大小

<pre>
ngx_slab_exact_size = ngx_pagesize / (8 * sizeof(uintptr_t));
</pre>
ngx_slab_exact_size表示```uintptr_t``` slab，此种情况下的slab用作bitmap，表示一页中的内存块使用状况。slab中的所有位(8 * sizeof(uintptr_t))刚好可以一一对应于一页找那个的大小为ngx_slab_exact_size的每一块内存。

下面我们详细分析一下函数的执行流程：
{% highlight string %}
void *
ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size)
{
	//1) 判断当前所请求分配的空间size是否大于ngx_slab_max_size(当前值为2048)，如果大于，那么直接调用
	//ngx_slab_alloc_pages()函数来进行分配
	if (size > ngx_slab_max_size) {

		ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, ngx_cycle->log, 0,
				"slab alloc: %uz", size);

		page = ngx_slab_alloc_pages(pool, (size >> ngx_pagesize_shift)
				+ ((size % ngx_pagesize) ? 1 : 0));
		if (page) {
			
			//1.1) 由返回page在页数组中的偏移量，计算出实际数组地址的偏移量。然后再加上真实可用内存的起始地址pool->start，
			//即可算出本次所分配内存的起始地址
			p = (page - pool->pages) << ngx_pagesize_shift;
			p += (uintptr_t) pool->start;

		} else {
			p = 0;
		}

		goto done;
	}


	//2) 判断从哪一个slot来分配空间（当前pool->min_size=8)
	// 2.1) 如果size > pool->min_size的话，则找到对应的slot;
	// 2.2) 否则，按最小8字节的方式来分配，因此slot=0

	//3) 计算出slots数组的起始位置
	slots = (ngx_slab_page_t *) ((u_char *) pool + sizeof(ngx_slab_pool_t));
	page = slots[slot].next;

	//4) 如果page->next != page，说明指定slot下的page仍有空闲内存块
	if (page->next != page) {
		
		if (shift < ngx_slab_exact_shift) {
			/*
			 * 4.1) 对应于分配小块内存
			 *
			 * 当从一个页中分配小于ngx_slab_exact_shift(ngx_slab_exact_size=128)的内存块时，无法用uintptr_t slab来标识
			 * 一页内所有内存块的使用情况，因此，这里不用page->slab来标识该页内所有内存块的使用情况，而是使用页数据空间的开始
			 * 几个uintptr_t空间来标识
			 */
			do{
				//4.1.1) 计算对应page的实际地址
				p = (page - pool->pages) << ngx_pagesize_shift;
				bitmap = (uintptr_t *) (pool->start + p);

				//4.1.2) 通过(1<<(ngx_pagesize_shift - shift))可以算出一页中可以存放多少内存块，然后再除以sizeof(uintptr_t) *8
				// 则可以计算出需要多少个uintptr_t空间来作为整个页的位图
				map = (1 << (ngx_pagesize_shift - shift))
					/ (sizeof(uintptr_t) * 8);

				//4.1.3) 遍历每一个位图，找出其中的可用内存块
				for (n = 0; n < map; n++) {
					if (bitmap[n] != NGX_SLAB_BUSY) {
						//找出了其中一个bitmap具有可用内存空间
			
						for (m = 1, i = 0; m; m <<= 1, i++) {
							//a) 找出可用内存块

							//b) 将对应bitmap位设为1，并计算出对应的内存块在页内的偏移地址。
							bitmap[n] |= m;
							i = ((n * sizeof(uintptr_t) * 8) << shift)
								+ (i << shift);

							//c) 若bitmap[n] == NGX_SLAB_BUSY，那么说明当前bitmap[n]所表示的一系列内存块已经没有空闲块了，此时
							// 需要检查整个page页是否仍有空闲内存块，如果没有则说明当前页为全满页，要脱离对应链表
							if (bitmap[n] == NGX_SLAB_BUSY) {

								//获得当前页的前一页prev，将当前页脱离链表后会设置page->prev=NGX_SLAB_SMALL;

								 prev = (ngx_slab_page_t *)
										(page->prev & ~NGX_SLAB_PAGE_MASK);
								 prev->next = page->next;
								 page->next->prev = page->prev;

								 page->next = NULL;
								 page->prev = NGX_SLAB_SMALL;
							}
						}

					}
				}

			}while(page);

		}else if (shift == ngx_slab_exact_shift) {

			/*
			 * 4.2) 对应分配中等内存大小的情况
			 *
			 * 此种情况用page->slab作为位图就刚好能够表示已页中的所有内存块
			 */
			do{
				if(page->slab != NGX_SLAB_BUSY)
				{
					//a) 找出可用内存块

					//b) 将对应bitmap位设为1，并计算出对应的内存块在页内的偏移地址。

					//c) 将全满也脱离链表
					
					//d) 计算分配到的内存首地址
					p = (page - pool->pages) << ngx_pagesize_shift;
					p += i << shift;
					p += (uintptr_t) pool->start;
				}
			}while(page);
		}else{
			/*
			 * 4.3) 大块内存(ngx_slab_exact_size < 内存块大小 <= ngx_slab_max_size)的情况 
			 * 当前Ubuntu 32bit系统,ngx_slab_exact_size值为128， ngx_slab_max_size值为2048
			 *
			 * 当需要分配的空间大于ngx_slab_exact_size=128时，我们可以用一个uintptr_t的位（共32位)来表示这些空间
			 * 所以我们依然采用跟 '等于ngx_slab_exact_size' 时类似的情况，用page->slab来标识该page内所有内存块的使用情况
			 * 此时的page->slab同时存储bitmap及表示内存大小的shift，高位为bitmap
			 * 这里会有的内存块大小依次为： 256bytes、 512bytes、 1024bytes、 2048bytes
			 * 对应的shift依次为：          8、        9、        10、       11
			 * 那么采用page->slab的高16位来表示这些空间的占用情况，而最低NGX_SLAB_SHIFT_MASK位用来标识此页分配大小，即保存移位数
			 * 例如：
			 * 比如我们分配256，当分配该页的第一块空间时，此时的page->slab位图情况是：0x00010008
			 * 那分配该页的第二块空间时，page->slab的值就是: 0x00030008。当page->slab的值为0xffff0008时，就表示此页已经分配完毕
			 *
			 * #define NGX_SLAB_SHIFT_MASK  0x0000000f	
			 * page->slab & NGX_SLAB_SHIFT_MASK 即得到最低4位的值，其实就是当前页的分配大小的移位数
			 * 这里用最低4位就足够了，因为shift最大为11(表示内存块大小为2048 bytes)
			 * ngx_pagesize_shift - (page->slab & NGX_SLAB_SHIFT_MASK);就是在一页中标记这些块所需的移位数，也就是块数对应的移位数
			 * 例如：
			 * 当页内所能分配的内存块大小是256bytes时，此时,page->slab & NGX_SLAB_SHIFT_MASK = 8
			 * 因此，n=ngx_pagesize_shift - (page->slab & NGX_SLAB_SHIFT_MASK) = 12 - 8 = 4
			 * 即4096bytes 可以分配为16个256bytes, 因此n = 1<<n = 16
			 *
			 * 其实，是对n = 2^ngx_pagesize_shift/2^(page->slab & NGX_SLAB_SHIFT_MASK) = 2^(ngx_pagesize_shift - (page->slab & NGX_SLAB_SHIFT_MASK))公式的简化
			 */
			n = ngx_pagesize_shift - (page->slab & NGX_SLAB_SHIFT_MASK);
			n = 1 << n;

			
			//得到表示这些块数都用完的bitmap，用现在是低16位的(当前Ubuntu 32bit操作系统）
			n = ((uintptr_t) 1 << n) - 1;

			//将低16位转换成高16位，因为我们是用高16位来表示空间地址的占用情况的，#define NGX_SLAB_MAP_SHIFT   16
			

			do{
				if ((page->slab & NGX_SLAB_MAP_MASK) != mask) {
					//4.3.1) 表示当前页为非全满页，仍有空闲
	
					//遍历整个位图
					for (m = (uintptr_t) 1 << NGX_SLAB_MAP_SHIFT, i = 0; m & mask;m <<= 1, i++){
						//a) 将对应bitmap位设为1，并计算出对应的内存块在页内的偏移地址。

						//b) 将全满也脱离链表
						
						//c) 计算分配到的内存首地址
						p = (page - pool->pages) << ngx_pagesize_shift;
						p += i << shift;
						p += (uintptr_t) pool->start;
					}
				}
			}while(page);
		}
	}

	/*
	 * 5) 在 小块内存、中等内存、大块内存 等三种情况下（不包括超大页面的情况），
	 * 如果当前slab对应的page中没有空间可分配了，则重新从空闲page中分配一个页
	 * /

	//6) 将分配的新页加入到对应的链表
	if(page)
	{
		if (shift < ngx_slab_exact_shift) {
			/*
			 *  6.1) 处理小页情况
			 */

			//6.1.1) 计算位图的位置
			p = (page - pool->pages) << ngx_pagesize_shift;
			bitmap = (uintptr_t *) (pool->start + p);

			/*
			 * 6.1.2） 这里shift代表要分配多大内存块的移位数，因此s为需要分配内存块的大小；
			 * n用于表示需要用多少块大小为s的内存块来作为位图空间
			 */ 
			s = 1 << shift;
			n = (1 << (ngx_pagesize_shift - shift)) / 8 / s;

			
			//6.1.3) 前面n块内存作为位图被占用，另外还有一块内存由本次分配出去，因此位图中要(n+1)位来表示这写内存被占用
			bitmap[0] = (2 << n) - 1;
			
			map = (1 << (ngx_pagesize_shift - shift)) / (sizeof(uintptr_t) * 8);
			
			for (i = 1; i < map; i++) {
				bitmap[i] = 0;
			}

			//6.1.4) 将当前page插入到对应的slot。因为这里是新分配的一页，因此对应slot链表肯定为空，否则不会运行到此处
			page->slab = shift;
			page->next = &slots[slot];
			page->prev = (uintptr_t) &slots[slot] | NGX_SLAB_SMALL;

			slots[slot].next = page;

			//6.1.5) 返回实际分配的空间的地址
			
		}else if(shift == ngx_slab_exact_shift){
			
			/*
			 * 6.2) 处理中等大小内存的情况。
			 * 此种情况直接用page->slab作为位图，很容易进行处理
			 */

		}else{

			/*
			 * 6.3) 用于处理大页
			 * page->slab的高位用于存储位图，低NGX_SLAB_SHIFT_MASK位用于存储当前页中内存块的大小
			 */
			page->slab = ((uintptr_t) 1 << NGX_SLAB_MAP_SHIFT) | shift;
		}
	}

done:
	return p;
}
{% endhighlight %}

## 6. 函数ngx_slab_calloc()
{% highlight string %}
void *
ngx_slab_calloc(ngx_slab_pool_t *pool, size_t size)
{
    void  *p;

    ngx_shmtx_lock(&pool->mutex);

    p = ngx_slab_calloc_locked(pool, size);

    ngx_shmtx_unlock(&pool->mutex);

    return p;
}
{% endhighlight %}
与ngx_slab_alloc()函数类似，不过这里会将分配的内存清0。

## 7. 函数ngx_slab_calloc_locked()
{% highlight string %}
void *
ngx_slab_calloc_locked(ngx_slab_pool_t *pool, size_t size)
{
    void  *p;

    p = ngx_slab_alloc_locked(pool, size);
    if (p) {
        ngx_memzero(p, size);
    }

    return p;
}
{% endhighlight %}
与ngx_slab_alloc_locked()函数类似，不过这里会将分配的内存清0.



## 8. 函数ngx_slab_free()
{% highlight string %}
void
ngx_slab_free(ngx_slab_pool_t *pool, void *p)
{
    ngx_shmtx_lock(&pool->mutex);

    ngx_slab_free_locked(pool, p);

    ngx_shmtx_unlock(&pool->mutex);
}
{% endhighlight %}
本函数首先使用pool->mutex加锁，然后再调用ngx_slab_free_locked()函数来释放对应的内存。


## 9. 函数ngx_slab_free_locked()
{% highlight string %}
void
ngx_slab_free_locked(ngx_slab_pool_t *pool, void *p)
{
    size_t            size;
    uintptr_t         slab, m, *bitmap;
    ngx_uint_t        n, type, slot, shift, map;
    ngx_slab_page_t  *slots, *page;

    ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, ngx_cycle->log, 0, "slab free: %p", p);

    if ((u_char *) p < pool->start || (u_char *) p > pool->end) {
        ngx_slab_error(pool, NGX_LOG_ALERT, "ngx_slab_free(): outside of pool");
        goto fail;
    }

    n = ((u_char *) p - pool->start) >> ngx_pagesize_shift;
    page = &pool->pages[n];
    slab = page->slab;
    type = page->prev & NGX_SLAB_PAGE_MASK;

    switch (type) {

    case NGX_SLAB_SMALL:

        shift = slab & NGX_SLAB_SHIFT_MASK;
        size = 1 << shift;

        if ((uintptr_t) p & (size - 1)) {
            goto wrong_chunk;
        }

        n = ((uintptr_t) p & (ngx_pagesize - 1)) >> shift;
        m = (uintptr_t) 1 << (n & (sizeof(uintptr_t) * 8 - 1));
        n /= (sizeof(uintptr_t) * 8);
        bitmap = (uintptr_t *)
                             ((uintptr_t) p & ~((uintptr_t) ngx_pagesize - 1));

        if (bitmap[n] & m) {

            if (page->next == NULL) {
                slots = (ngx_slab_page_t *)
                                   ((u_char *) pool + sizeof(ngx_slab_pool_t));
                slot = shift - pool->min_shift;

                page->next = slots[slot].next;
                slots[slot].next = page;

                page->prev = (uintptr_t) &slots[slot] | NGX_SLAB_SMALL;
                page->next->prev = (uintptr_t) page | NGX_SLAB_SMALL;
            }

            bitmap[n] &= ~m;

            n = (1 << (ngx_pagesize_shift - shift)) / 8 / (1 << shift);

            if (n == 0) {
                n = 1;
            }

            if (bitmap[0] & ~(((uintptr_t) 1 << n) - 1)) {
                goto done;
            }

            map = (1 << (ngx_pagesize_shift - shift)) / (sizeof(uintptr_t) * 8);

            for (n = 1; n < map; n++) {
                if (bitmap[n]) {
                    goto done;
                }
            }

            ngx_slab_free_pages(pool, page, 1);

            goto done;
        }

        goto chunk_already_free;

    case NGX_SLAB_EXACT:

        m = (uintptr_t) 1 <<
                (((uintptr_t) p & (ngx_pagesize - 1)) >> ngx_slab_exact_shift);
        size = ngx_slab_exact_size;

        if ((uintptr_t) p & (size - 1)) {
            goto wrong_chunk;
        }

        if (slab & m) {
            if (slab == NGX_SLAB_BUSY) {
                slots = (ngx_slab_page_t *)
                                   ((u_char *) pool + sizeof(ngx_slab_pool_t));
                slot = ngx_slab_exact_shift - pool->min_shift;

                page->next = slots[slot].next;
                slots[slot].next = page;

                page->prev = (uintptr_t) &slots[slot] | NGX_SLAB_EXACT;
                page->next->prev = (uintptr_t) page | NGX_SLAB_EXACT;
            }

            page->slab &= ~m;

            if (page->slab) {
                goto done;
            }

            ngx_slab_free_pages(pool, page, 1);

            goto done;
        }

        goto chunk_already_free;

    case NGX_SLAB_BIG:

        shift = slab & NGX_SLAB_SHIFT_MASK;
        size = 1 << shift;

        if ((uintptr_t) p & (size - 1)) {
            goto wrong_chunk;
        }

        m = (uintptr_t) 1 << ((((uintptr_t) p & (ngx_pagesize - 1)) >> shift)
                              + NGX_SLAB_MAP_SHIFT);

        if (slab & m) {

            if (page->next == NULL) {
                slots = (ngx_slab_page_t *)
                                   ((u_char *) pool + sizeof(ngx_slab_pool_t));
                slot = shift - pool->min_shift;

                page->next = slots[slot].next;
                slots[slot].next = page;

                page->prev = (uintptr_t) &slots[slot] | NGX_SLAB_BIG;
                page->next->prev = (uintptr_t) page | NGX_SLAB_BIG;
            }

            page->slab &= ~m;

            if (page->slab & NGX_SLAB_MAP_MASK) {
                goto done;
            }

            ngx_slab_free_pages(pool, page, 1);

            goto done;
        }

        goto chunk_already_free;

    case NGX_SLAB_PAGE:

        if ((uintptr_t) p & (ngx_pagesize - 1)) {
            goto wrong_chunk;
        }

        if (slab == NGX_SLAB_PAGE_FREE) {
            ngx_slab_error(pool, NGX_LOG_ALERT,
                           "ngx_slab_free(): page is already free");
            goto fail;
        }

        if (slab == NGX_SLAB_PAGE_BUSY) {
            ngx_slab_error(pool, NGX_LOG_ALERT,
                           "ngx_slab_free(): pointer to wrong page");
            goto fail;
        }

        n = ((u_char *) p - pool->start) >> ngx_pagesize_shift;
        size = slab & ~NGX_SLAB_PAGE_START;

        ngx_slab_free_pages(pool, &pool->pages[n], size);

        ngx_slab_junk(p, size << ngx_pagesize_shift);

        return;
    }

    /* not reached */

    return;

done:

    ngx_slab_junk(p, size);

    return;

wrong_chunk:

    ngx_slab_error(pool, NGX_LOG_ALERT,
                   "ngx_slab_free(): pointer to wrong chunk");

    goto fail;

chunk_already_free:

    ngx_slab_error(pool, NGX_LOG_ALERT,
                   "ngx_slab_free(): chunk is already free");

fail:

    return;
}
{% endhighlight %}
本函数用于将对应的空间释放回内存池。下面我们简要分析一下函数的执行流程：
{% highlight string %}
void
ngx_slab_free_locked(ngx_slab_pool_t *pool, void *p)
{
	//1) 判定对应的内存p是否属于内存池所管理
	 if ((u_char *) p < pool->start || (u_char *) p > pool->end) {}

	//2) 计算空间p所属的page以及page的类型
	n = ((u_char *) p - pool->start) >> ngx_pagesize_shift;
	page = &pool->pages[n];
	slab = page->slab;
	type = page->prev & NGX_SLAB_PAGE_MASK;

	//3) 根据不同类型的内存，做不同的处理
	switch(type){
	case NGX_SLAB_SMALL:
		/*
		 * 3.1) 小块内存
		 *
		 * 对于小块内存无法用uintptr_t slab;来标识一页内所有内存块的使用情况，因此，这里不用
		 * page->slab来标识该页内所有内存块的使用情况，而是使用页数据空间的开始几个uintptr_t
		 * 空间来表示了
		 *
		 * 3.1.1) 在 '小内存块' 中，page->slab存放的是等长内存块的大小（用位偏移的方式存储）
		 * 因此，size为当前页小块内存的大小
		 */
		shift = slab & NGX_SLAB_SHIFT_MASK;
		size = 1 << shift;


		//3.1.2) 因为从pool中分配的空间首先是ngx_pagesize对齐的，自然也是这里的size对齐的，因此 (uintptr_t)p & (size -1)的值应该为0
		if ((uintptr_t) p & (size - 1)) {
			goto wrong_chunk;
		}

		/*
		 * 3.1.3) 这里很巧妙：由于前面对页进行了内存对齐处理，因此下面的式子可直接
		 *
		 * 求出 p 对应的slot块的位置，即 p 对应的小块内存位于page中的第几个块：
		 * 首先((uintptr_t) p & (ngx_pagesize -1))可以计算出页内的偏移地址p_offset，然后将页内偏移地址p_offset除以每一块的大小即可
		 * 求出是属于page内的第几块
		 */
		 n = ((uintptr_t) p & (ngx_pagesize - 1)) >> shift;

		//3.1.4) m是计算page中第n块内存对应于uintptr_t中的第几位
		m = (uintptr_t) 1 << (n & (sizeof(uintptr_t) * 8 - 1));

		//3.1.5) 用于计算page中第n块内存对应于位图中的第几个uintptr_t
		n /= (sizeof(uintptr_t) * 8);

		//3.1.6) 用于计算位图的起始位置
		bitmap = (uintptr_t *)
				((uintptr_t) p & ~((uintptr_t) ngx_pagesize - 1));
		

		/*
		 * 3.1.7) bitmap[n] & m 值不等于0，表明了此处了内存处于busy状态，即表示分配了出去。这种情况才有释放内存一说，
		 * 否则直接 go chunk_already_free，表明此块内存已经处于free状态，不需要释放
		 */
		if(bitmap[n] & m){

			/*
			 * a) 如果页面的当前状态是全部已使用（全满页，全满页不在任何链表中；全满页释放一个内存块后变为半满页），
			 * 则把它重新加入到slot链表的表头
			 */
			if(page->next == NULL){}


			//b) 将对应的内存块的位图置为空闲状态
			bitmap[n] &= ~m;

			/*
			 * c) 如下是计算要多少个内存块来存放位图
			 * (1<<(ngx_pagesize_shift -shift))用于计算一页内总共有多少块内存块，再除以8用于计算要多少个字节来作为位图表示这些内存块；
			 * 最后再除以(1<<shift)用于计算要用多少块内存块来作为位图
			 */
			n = (1 << (ngx_pagesize_shift - shift)) / 8 / (1 << shift);
			if (n == 0) {
				n = 1;
			}

			/*
			 * d) 从上面步骤c)可知道，要用n个内存块来存放位图。因此这里我们要判定bitmap[0]中除用来存放位图的内存块外，是否处于busy状态
			 * ((uintptr_t)1 << n) -1)用于表示存储位图的内存块的掩码，再取反，则可以得出前32个内存块中除用作位图的内存块外的掩码，然后
			 * 在按位与上bitmap[0]，则可以知道前32块内存块中实际用于存储数据的内存块是否使用。
			 * 如果仍处于busy状态，那么直接goto done;
			 */
			if (bitmap[0] & ~(((uintptr_t) 1 << n) - 1)) {
				goto done;
			}

			//e) 计算有多少个uintptr_t对象才能表示整个位图
			map = (1 << (ngx_pagesize_shift - shift)) / (sizeof(uintptr_t) * 8);

			//f) 判断剩余的位图是否处于busy状态

			//g) 到此步骤时，表明当前整个页都处于空闲状态，此时需要调用ngx_slab_free_pages()来释放整个页。
			ngx_slab_free_pages(pool, page, 1);
		}

		goto chunk_already_free;
	case NGX_SLAB_EXACT:
		/*
		 * 用于释放中等大小内存块。
		 *
		 * 此种情况下，page->slab作为当前页的使用情况的位图
		 */

		/*
		 * a) 
		 * ((uintptr_t) p & (ngx_pagesize -1))可以计算出当前内存块的页内偏移地址
		 * (((uintptr_t) p & (ngx_pagesize - 1)) >> ngx_slab_exact_shift)可以计算出当前内存块是属于页内的第几块内存块
		 * 再接着左移，则可以计算出该块内存块对应与位图(page->slab)中的哪一位
		 */
		m = (uintptr_t) 1 <<
		 	(((uintptr_t) p & (ngx_pagesize - 1)) >> ngx_slab_exact_shift);

		//b) 因为从pool中分配的空间首先是ngx_pagesize对齐的，自然也是这里的size对齐的，因此 (uintptr_t)p & (size -1)的值应该为0
		if ((uintptr_t) p & (size - 1)) {
			goto wrong_chunk;
		}

		/*
		 * c) slab & m 值不等于0，表明了此处了内存处于busy状态，即表示分配了出去。这种情况才有释放内存一说，
		 * 否则直接 go chunk_already_free，表明此块内存已经处于free状态，不需要释放
		 */
		if (slab & m) {
			if(slab == NGX_SLAB_BUSY)
			{
				//d) 表明当前整个page中的内存块都处于busy状态。此种情况即使释放了当前块，整个page仍处于半满页状态，需要
				//加入到对应slot链表的表头
			}

			//e) 将当前内存块为空闲状态
			page->slab &= ~m;

			//f) 说明当前page页并不是空闲页，直接goto done;
			if(page->slab){
			}

			//g) 整个page处于空闲状态，需要释放整个page
			 ngx_slab_free_pages(pool, page, 1);
		}

	case NGX_SLAB_BIG:
		/*
		 * 用于释放大内存页的情况
		 */
		
		//a) slab的低NGX_SLAB_SHIFT_MASK位用于表示当前页内存块大小的移位，然后就可以算出当前页内存块的大小
		shift = slab & NGX_SLAB_SHIFT_MASK;
		size = 1 << shift;

		//b) 因为从pool中分配的空间首先是ngx_pagesize对齐的，自然也是这里的size对齐的，因此 (uintptr_t)p & (size -1)的值应该为0
		if ((uintptr_t) p & (size - 1)) {
			goto wrong_chunk;
		}

		/*
		 * c) 用于计算当前内存块的位图
		 * ((uintptr_t) p & (ngx_pagesize - 1))用于计算页内偏移地址
		 * (((uintptr_t) p & (ngx_pagesize - 1)) >> shift) 用于计算出当前是属于页内的第几个内存块
		 * ((((uintptr_t) p & (ngx_pagesize - 1)) >> shift)+ NGX_SLAB_MAP_SHIFT) 用于计算出在位图的第几位
		 */
		 m = (uintptr_t) 1 << ((((uintptr_t) p & (ngx_pagesize - 1)) >> shift)
			+ NGX_SLAB_MAP_SHIFT);


		/*
		 * d) slab & m 值不等于0，表明了此处了内存处于busy状态，即表示分配了出去。这种情况才有释放内存一说，
		 * 否则直接 go chunk_already_free，表明此块内存已经处于free状态，不需要释放
		 */
		if (slab & m) {
			/*
			 * e) 如果页面的当前状态是全部已使用（全满页，全满页不在任何链表中；全满页释放一个内存块后变为半满页），
			 * 则把它重新加入到slot链表的表头
			 */
			if(page->next == NULL){}
			
			//f) 将当前内存块置为空闲状态
			page->slab &= ~m;

			//g) 说明当前page页并不是空闲页，直接goto done;
			if (page->slab & NGX_SLAB_MAP_MASK) {
				goto done;
			}

			//h) 整个page处于空闲状态，需要释放整个page
			 ngx_slab_free_pages(pool, page, 1);
		}

	case NGX_SLAB_PAGE:
		/*
		 * 释放超大内存
		 */

		//a) 因为从pool中分配的空间首先是ngx_pagesize对齐的，自然也是这里的size对齐的，因此 (uintptr_t)p & (size -1)的值应该为0
		if ((uintptr_t) p & (ngx_pagesize - 1)) {
			goto wrong_chunk;
		}

		//b) 当前页的slab处于free状态，表明不需要进行释放
		if (slab == NGX_SLAB_PAGE_FREE) {}

		//c) 对于按页分配的内存，第一页的page->slab的值为 pages | NGX_SLAB_PAGE_START;即NGX_SLAB_PAGE_START标识再按位或上连续的页数
		//而后续页才会设置为NGX_SLAB_PAGE_BUSY
		if (slab == NGX_SLAB_PAGE_BUSY) {}

		//d) 用于计算页偏移，及当前内存块所包含的页数
		n = ((u_char *) p - pool->start) >> ngx_pagesize_shift;
		size = slab & ~NGX_SLAB_PAGE_START;

		//e) 用于释放对应的页
		ngx_slab_free_pages(pool, &pool->pages[n], size);
	}
}
{% endhighlight %}

## 10. 函数ngx_slab_alloc_pages()
{% highlight string %}
static ngx_slab_page_t *
static ngx_slab_page_t *
ngx_slab_alloc_pages(ngx_slab_pool_t *pool, ngx_uint_t pages)
{
    ngx_slab_page_t  *page, *p;

    for (page = pool->free.next; page != &pool->free; page = page->next) {

        if (page->slab >= pages) {

            if (page->slab > pages) {
                page[page->slab - 1].prev = (uintptr_t) &page[pages];

                page[pages].slab = page->slab - pages;
                page[pages].next = page->next;
                page[pages].prev = page->prev;

                p = (ngx_slab_page_t *) page->prev;
                p->next = &page[pages];
                page->next->prev = (uintptr_t) &page[pages];

            } else {
                p = (ngx_slab_page_t *) page->prev;
                p->next = page->next;
                page->next->prev = page->prev;
            }

            page->slab = pages | NGX_SLAB_PAGE_START;
            page->next = NULL;
            page->prev = NGX_SLAB_PAGE;

            if (--pages == 0) {
                return page;
            }

            for (p = page + 1; pages; pages--) {
                p->slab = NGX_SLAB_PAGE_BUSY;
                p->next = NULL;
                p->prev = NGX_SLAB_PAGE;
                p++;
            }

            return page;
        }
    }

    if (pool->log_nomem) {
        ngx_slab_error(pool, NGX_LOG_CRIT,
                       "ngx_slab_alloc() failed: no memory");
    }

    return NULL;
}
{% endhighlight %}
本函数用于从pool中分配出指定的 n 页。下面我们简单分析一下函数的执行流程：
{% highlight string %}
static ngx_slab_page_t *
ngx_slab_alloc_pages(ngx_slab_pool_t *pool, ngx_uint_t pages)
{
	//遍历整个空闲链表
	for (page = pool->free.next; page != &pool->free; page = page->next) {
		//找出连续空闲页大于指定大小的内存块
		if (page->slab >= pages) {

			 if (page->slab > pages){
				//说明分配之后，还有剩余的空闲页，此时需要重新设置链表
			 }else{
				//说明刚好够分配，此种情况只需要将这一系列页从free链表摘除即可
			 }

			//将新分配出的这些page设置如下标识
			page->slab = pages | NGX_SLAB_PAGE_START;
			page->next = NULL;
			page->prev = NGX_SLAB_PAGE;

			//若请求分配的页数是1页，则直接返回

			//否则，从第2页起将page->slab都设置为NGX_SLAB_PAGE_BUSY
			for (p = page + 1; pages; pages--) {
				p->slab = NGX_SLAB_PAGE_BUSY;
				p->next = NULL;
				p->prev = NGX_SLAB_PAGE;
				p++;
			}
		}
	}
}
{% endhighlight %}


## 11. 函数ngx_slab_free_pages()
{% highlight string %}
static void
ngx_slab_free_pages(ngx_slab_pool_t *pool, ngx_slab_page_t *page,
    ngx_uint_t pages)
{
    ngx_uint_t        type;
    ngx_slab_page_t  *prev, *join;

    page->slab = pages--;

    if (pages) {
        ngx_memzero(&page[1], pages * sizeof(ngx_slab_page_t));
    }

    if (page->next) {
        prev = (ngx_slab_page_t *) (page->prev & ~NGX_SLAB_PAGE_MASK);
        prev->next = page->next;
        page->next->prev = page->prev;
    }

    join = page + page->slab;

    if (join < pool->last) {
        type = join->prev & NGX_SLAB_PAGE_MASK;

        if (type == NGX_SLAB_PAGE) {

            if (join->next != NULL) {
                pages += join->slab;
                page->slab += join->slab;

                prev = (ngx_slab_page_t *) (join->prev & ~NGX_SLAB_PAGE_MASK);
                prev->next = join->next;
                join->next->prev = join->prev;

                join->slab = NGX_SLAB_PAGE_FREE;
                join->next = NULL;
                join->prev = NGX_SLAB_PAGE;
            }
        }
    }

    if (page > pool->pages) {
        join = page - 1;
        type = join->prev & NGX_SLAB_PAGE_MASK;

        if (type == NGX_SLAB_PAGE) {

            if (join->slab == NGX_SLAB_PAGE_FREE) {
                join = (ngx_slab_page_t *) (join->prev & ~NGX_SLAB_PAGE_MASK);
            }

            if (join->next != NULL) {
                pages += join->slab;
                join->slab += page->slab;

                prev = (ngx_slab_page_t *) (join->prev & ~NGX_SLAB_PAGE_MASK);
                prev->next = join->next;
                join->next->prev = join->prev;

                page->slab = NGX_SLAB_PAGE_FREE;
                page->next = NULL;
                page->prev = NGX_SLAB_PAGE;

                page = join;
            }
        }
    }

    if (pages) {
        page[pages].prev = (uintptr_t) page;
    }

    page->prev = (uintptr_t) &pool->free;
    page->next = pool->free.next;

    page->next->prev = (uintptr_t) page;

    pool->free.next = page;
}
{% endhighlight %}
此函数用于释放指定页数的page。下面我们简要分析一下函数的执行流程：
{% highlight string %}
static void
ngx_slab_free_pages(ngx_slab_pool_t *pool, ngx_slab_page_t *page,
    ngx_uint_t pages)
{
	/*
	 * 将page->slab设置为指定的页数，
	 */
	page->slab = pages--;

	//1) 如果释放的页数大于1，那么会将从第1叶开始的后续所有的页清0
	if (pages) {
		ngx_memzero(&page[1], pages * sizeof(ngx_slab_page_t));
	}

	//2) 如果page->next不为空，表示当前page还没有脱离原来的slot链表（因为对于全满页，早已脱离了链表）
	

	//3) 检查当前所释放page(s)是否能够和后续页进行合并
	if (join < pool->last) {
		type = join->prev & NGX_SLAB_PAGE_MASK;
	
		/*
		 * 3.1) 表明此处是一个完整的页，并不是slot中对应的页（因为只有全满页 和 全空页 的type值会是NGX_SLAB_PAGE)
		 */
		if(type == NGX_SLAB_PAGE){

			if (join->next != NULL) {
				/*
				 * join->next不为NULL，表明不是全满页，因为全满页已经脱离了任何的链表，其next一定为NULL
				 *
				 * 将join脱离链表，并加入到当前释放的page中
				 */
				
			}
		}

	}

	//4) 检查当前所释放的page(s)是否能够和前面的页进行合并
	if (page > pool->pages) {
		join = page - 1;
		type = join->prev & NGX_SLAB_PAGE_MASK;

		/*
		 * 4.1) 表明此处是一个完整的页，并不是slot中对应的页（因为只有全满页 和 全空页 的type值会是NGX_SLAB_PAGE)
		 */
		if (type == NGX_SLAB_PAGE) {
			
			//a) 找出该空闲内存页（组）的首页
			if (join->slab == NGX_SLAB_PAGE_FREE) {
				join = (ngx_slab_page_t *) (join->prev & ~NGX_SLAB_PAGE_MASK);
			}

			if (join->next != NULL){
				/*
				 * join->next不为NULL，表明不是全满页，因为全满页已经脱离了任何的链表，其next一定为NULL
				 *
				 * 将join脱离链表，并加入到当前释放的page中
				 */
			}

		}
	}


	//5)将所释放的内存页组的最后一页的prev指向该组的第一页，这样方便后续合并
	if (pages) {
		page[pages].prev = (uintptr_t) page;
	}

	//6) 将page加入到free链表的首部
}
{% endhighlight %}
下面给出一张释放pages的示意图：


![ngx-slab-freepages](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_slab_freepages.jpg)


## 12. 函数ngx_slab_error()
{% highlight string %}
static void
ngx_slab_error(ngx_slab_pool_t *pool, ngx_uint_t level, char *text)
{
    ngx_log_error(level, ngx_cycle->log, 0, "%s%s", text, pool->log_ctx);
}
{% endhighlight %}
用于打印slab内存分配相关的错误消息




<br />
<br />

**[参看]**

1. [slab](https://baike.baidu.com/item/slab/5803993?fr=aladdin)

2. [Linux内存管理中的slab分配器](https://www.cnblogs.com/pengdonglin137/p/3878552.html)

3. [共享内存管理之slab机制](https://blog.csdn.net/hnudlz/article/details/50972596)

4. [nginx slab内存管理](http://www.cnblogs.com/doop-ymc/p/3412572.html)

5. [Nginx开发从入门到精通](http://tengine.taobao.org/book/index.html#)

<br />
<br />
<br />

