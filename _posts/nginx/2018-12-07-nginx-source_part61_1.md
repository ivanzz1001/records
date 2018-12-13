---
layout: post
title: core/ngx_slab.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们介绍一下Nginx中基于slab的内存分配机制。


这里，我们先简单介绍一下Linux下的slab。```slab```是Linux操作系统的一种内存分配机制。其工作是针对一些经常分配并释放的对象，如进程描述符等，这些对象的大小一般比较小，如果直接采用伙伴系统来进行分配和释放，不仅会造成大量的内存碎片，而且处理速度也太慢。而slab分配器是基于对象进行管理的，相同类型的对象归为一类（如进程描述符就是一类），每当要申请这样一个对象，slab分配器就从一个slab列表中分配一个这样大小的单元出去，而当要释放时，将其重新保存到该列表中，而不是直接返回给伙伴系统，从而避免这些内存碎片。slab分配器并不丢失已分配的对象，而是释放并把它们保存在内存中。当以后又要请求新的对象时，就可以从内存直接获取而不用重复初始化。

<!-- more -->


## 1. ngx_slab_page_s数据结构
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SLAB_H_INCLUDED_
#define _NGX_SLAB_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_slab_page_s  ngx_slab_page_t;

struct ngx_slab_page_s {
    uintptr_t         slab;
    ngx_slab_page_t  *next;
    uintptr_t         prev;
};
{% endhighlight %}
本数据结构对应于Nginx slab内存管理的```页```概念，```页```在slab管理设计中是很核心的概念。**ngx_slab_page_t**中各字段根据```不同内存页类型```有不同的含义，下面我们就分别介绍一下：

1) **小块内存，小于ngx_slab_exact_size**

* slab: 表示该页面上存放的等长内存块大小，当然是用位偏移的方式存放的(存储于slab的低NGX_SLAB_SHIFT_MASK位)；

* next: 指向双链表的下一个元素，如果不在双链表中，则为0

* prev: 低2位为11，以NGX_SLAB_SMALL表示当前页面存放的是小块内存

2) **中等内存， 等于ngx_slab_exact_size**

* slab: 作为bitmap表示页面上的内存块是否已被使用

* next: 指向双链表的下一个元素，如果不在双链表中，则为0

* prev: 低2位为10，以NGX_SLAB_EXACT表示当前页面存放的是中等大小的内存

3) **大块内存，大于ngx_slab_exact_size而小于等于ngx_slab_max_size**

* slab: 高NGX_SLAB_MAP_MASK位表示bitmap，而低NGX_SLAB_SHIFT_MASK位表示存放的内存块大小；

* next: 指向双链表的下一个元素，如果不在双链表中，则为0

* prev: 低2位为01， 以NGX_SLAB_BIG表示当前页面存放的是大块内存

4) **超大内存，大于ngx_slab_max_size**

* slab: 超大内存会使用一页或者多页，这些页都在一起使用。对于这批页面中的第1页，slab的前3位会被设为NGX_SLAB_PAGE_START，其余位表示紧随其后相邻的同批页面数；反之，slab会被设置为NGX_SLAB_PAGE_BUSY

* next: 指向双链表的下一个元素，如果不在双链表中，则为0

* prev: 低2位为00，以NGX_SLAB_PAGE表示当前页面是以整页来使用。

## 2. ngx_slab_pool_t数据结构
{% highlight string %}
typedef struct {
    ngx_shmtx_sh_t    lock;

    size_t            min_size;
    size_t            min_shift;

    ngx_slab_page_t  *pages;
    ngx_slab_page_t  *last;
    ngx_slab_page_t   free;

    u_char           *start;
    u_char           *end;

    ngx_shmtx_t       mutex;

    u_char           *log_ctx;
    u_char            zero;

    unsigned          log_nomem:1;

    void             *data;
    void             *addr;
} ngx_slab_pool_t;
{% endhighlight %}
本数据结构作为nginx slab内存管理的池结构，用于记录及管理整个slab的内存分配情况。每一个slab内存池对应着一块共享内存，这是一段线性的连续的地址空间，这里不止是有将要分配给使用者的应用内存，还包括slab管理结构，事实上从这块内存的首地址开始就是管理结构体ngx_slab_pool_t。下面我们简要介绍一下各字段的含义：

* lock: 为下面的互斥锁```mutex```服务，使用```原子锁```来实现的Nginx互斥锁需要用到本变量；

* min_size: 设定的最小内存块长度；

* min_shift: min_size对应的位偏移，因为slab的算法大量采用位操作，在后面我们可以看出先计算出min_shift很有好处。

* pages: 每一页对应一个ngx_slab_page_t页描述结构体，所有的ngx_slab_page_t存放在连续的内存中构成数组，而pages就是数组首地址。


* last: 

* free: 所有的空闲页组成一个链表挂在free成员上。

* start: 所有的实际页面全部连续地放在一起，第一页的首地址就是start

* end: 指向这段共享内存的尾部

* mutex: nginx对互斥锁结构的封装

* log_ctx: slab操作失败时会记录日志，为区别是哪个slab共享内存出错，可以在slab中分配一段内存存放描述的字符串，然后再用log_ctx指向这个字符串；

* zero: 实际上就是'\0'，当log_ctx没有赋值时，将直接指向zero，表示空字符串防止出错；

* data: 由各个使用slab的模块自由使用，slab管理内存时不会用到它

* addr: 指向所属的ngx_shm_zone_t里的ngx_shm_t成员的addr成员，一般用于指示一段共享内存块的起始位置

<pre>
注： slab中每一个页大小为ngx_pagesize，这个大小用于实际的内存分配，但另外还需要一个ngx_slab_page_t结构
来管理该页。该结构所占用的空间是24个字节(我们当前是32bit的ubuntu系统），即管理一页需要额外付出24字节的空间。
</pre>
下面给出```ngx_slab_pool_t```数据结构的一个整体视图：

![ngx-slab-pool](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_slab_pool.jpg)




## 3. 相关函数声明
{% highlight string %}
//用于初始化slab管理池
void ngx_slab_init(ngx_slab_pool_t *pool);

//用于从slab管理池中分配出一块指定大小的内存
void *ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);

//与ngx_slab_alloc()类似，只是默认认为在调用此函数前我们已经加了锁，不会出现互斥性问题
void *ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size);

//从slab管理池中分配出一块指定大小的内存，并初始化为0
void *ngx_slab_calloc(ngx_slab_pool_t *pool, size_t size);

//与ngx_slab_alloc_locked()相似
void *ngx_slab_calloc_locked(ngx_slab_pool_t *pool, size_t size);

//用于将一块内存归还回slab管理池
void ngx_slab_free(ngx_slab_pool_t *pool, void *p);

//与ngx_slab_free()类似，只是默认认为在调用此函数前我们已经加了锁，不会出现互斥性问题
void ngx_slab_free_locked(ngx_slab_pool_t *pool, void *p);
{% endhighlight %}

## 4. nginx slab内存管理补充
这里对nginx slab内存管理再做一些补充介绍。通常，我们要动态的管理内存，需要面对的两个主要问题：

* 在时间上，使用者会随机地申请分配、释放内存；

* 在空间上，每次申请分配的内存大小也是随机的；

这两个问题将给内存分配算法带来很大的挑战： 当多次分配、释放不同大小的内存后，将不可避免的造成内存碎片，而内存碎片会造成内存浪费、执行速度变慢！常见的算法有2个设计方向： first-fit和best-fit。用最简单的实现方式来描述这2个算法就是：若已使用的内存之间有许多不等长的空闲内存，那么分配内存时，first-fit将从头遍历空闲内存块构成的链表，当找到的第1块空间大于请求size的内存块时，就把它返回给申请者； best-fit则不然，它也会遍历空闲链表，但如果一块空闲内存的空间远大于请求size，为了避免浪费，它会继续向后遍历，看看有没有恰好适合申请大小的空闲内存块，这个算法将试图返回最合适(例如内存块大小等于或略大于申请size)的内存块。这样first-fit和best-fit的优劣仿佛已一目了然： 前者分配的速度更快，但内存浪费的多；后者的分配速度慢一些，内存利用率上却更划算。而且，前者造成内存碎片的几率似乎要大于后者。


Nginx的slab内存分配方式是基于best-fit思路的，即当我们申请一块内存时，它只会返回恰好符合请求大小的内存块。但是，怎样可以更快速地找到best-fit内存块呢？ Nginx首先有一个假定： 所有需要使用slab内存的模块请求分配的内存都是比较小的（绝大部分小于4KB）。有了这个假定，就有了一种快速找到最合适内存块的方法。主要包括5个要点：

1） 把整块内存按4KB分为许多页，这样，如果每一页只存放一种固定大小的内存块，由于一页上能够分配的内存块数量是很有限的，所以可以在页首用bitmap方式，按二进制位表示页上对应位置的内存块是否在使用中。只是遍历bitmap二进制位去寻找页上的空闲内存块，使得消耗的时间很有限。例如bitmap占用的内存空间小导致CPU缓存命中率高，可以按32或64位这样的总线长度去寻找空闲位以减少访问次数等。

2） 基于空间换时间的思想，slab内存分配器会把请求分配的内存大小简化为极为有限的几种（简化的方法有很多，例如可以按照fibonacci方法进行），而Nginx slab是按2的倍数，将内存块分为8、16、32、64...字节，当申请的空间大于8小于等于16时，就会使用16字节的内存块，以此类推。所以，一种页面若存放的内存块大小为N，那么使用者申请的空间在(N/2,N]这一区间时，都将使用这种页面。这样最多会造成一倍内存的浪费，但使得页种类大大减少了，这会降低碎片的产生，提高内存的利用率。

3） 让有限的几种页面构成链表，而各链表按序保存在数组中，这样一来，用直接寻址法就可以快速找到。在Nginx slab中，用slot数组来存放链表首页。例如，如果申请的内存大小为30字节，那么根据最小的内存块8为字节，可以算出从小到大第3种内存块存放的内存大小为32字节，符合要求，从slots数组中取出第3个元素则可以寻找到32字节的页面。

4） 这些页面分为空闲页、半满页、全满页。为什么要这么划分呢？因为上述的同种页面链表不应当包含太多元素，否则内存分配时遍历链表一样非常耗时。所以全满页应当脱离链表，分配内存时不应当再访问到它。空闲页应当是超然的，如果这个页面曾经为32字节的内存块服务，在它又称为空闲页时，下次便可以为128字节的内存块服务。因此，所有的空闲页会单独构成一个空闲页链表。这里slots数组采用散列表的思想，用快速的直接寻址方式将半满页展现在使用者面前。

5） 虽然大部分情况下申请分配的内存块是小于4KB的，但极个别可能会有一些大于4KB的内存分配请求，如果直接拒绝它，则可能太过于粗暴了。对于此，可以用遍历空闲页链表寻找地址连续的空闲页来分配。例如，需要分配11KB的内存时，则遍历到3个地址连续的空闲页即可。

以上5点，就是Nginx slab内存管理方法的主要思想，如下图所示：

![ngx-slab-page](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_slab_page.jpg)

如上图所示，每一页都会有一个ngx_slab_page_t结构体描述，object是申请分配到的内存存放的对象，阴影方块是已经分配出去的内存块，空白方块则是未分配的内存块。


### 4.1 slab池中的空闲页与全满页
我们从上图中可以看到，页分为空闲页和已使用页，而已使用页又分为还有空闲空间可供分配的半满页和完全分配完毕的全满页。每一页的大小由```ngx_pagesize```变量指定，同时为方便大量的位操作，还定义了页大小对应的位移变量ngx_pagesize_shift。如下：
{% highlight string %}
//ngx_alloc.c文件
ngx_uint_t  ngx_pagesize;
ngx_uint_t  ngx_pagesize_shift;

//ngx_posix_init.c文件
ngx_int_t
ngx_os_init(ngx_log_t *log)
{
	...
	ngx_pagesize = getpagesize();
    ngx_cacheline_size = NGX_CPU_CACHE_LINE;

    for (n = ngx_pagesize; n >>= 1; ngx_pagesize_shift++) { /* void */ }
	...
}
{% endhighlight %}

在我们当前调试的系统32bit Ubuntu系统中，ngx_pagesize为4096，因此ngx_pagesize_shift的值为12。

全满页和空闲页较为简单。全满页不在任何链表中，它对应的ngx_slab_page_t中的next和prev成员没有任何链表功能。

所有的空闲页链表构成一个双向链表，ngx_slab_pool_t中的free指向这个链表。然而需要注意的是，并不是每一个空闲页都是该双向链表中的元素，可能存在多个相邻的页面中，仅首页面在链表中的情况，故而首页面的slab成员大于1的时候则表示其后面有相邻的页面，这些相邻的多个页面作为一个链表元素存在。但是，也并不是相邻的页面一定作为一个链表元素存在。如下图所示：

![ngx-freefull-page](https://ivanzz1001.github.io/records/assets/img/nginx/ngx_freefull_page.jpg)

在上图中，有5个连续的页面，中间是描述页面的ngx_slab_page_t结构体，右边则是真正的页面，它们是一一对应的。其中，第1、2、4、5都是空闲页，第3页则是全满页。而free链表的第一个元素是第5页，第2个元素是第4页，可见，虽然第4、5页时连续的，但是由于分配页面与回收页面时的时序不同，导致这第4、5两个页面间出现了相见不相识的现象，只能作为2个链表元素存在。这会造成未来分配不出占用2个页面的大块内存，虽然原本是可以分配出的。第3个元素是第1页。第2页附在第1页上，则还是与分配、回收页面的时机有关，事实上，当slab内存池刚刚初始化完毕时，free链表中只有1个元素，就是第1个页面，该页面的slab成员值为总页数。第3页时全满页，其next指针为NULL，而prev也没有指针的含义。


对于半满页，存放相同大小内存块的页面会构成双向链表，挂在slots数组的相应位置上。

<br />
<br />

**[参看]**

1. [slab](https://baike.baidu.com/item/slab/5803993?fr=aladdin)

2. [Linux内存管理中的slab分配器](https://www.cnblogs.com/pengdonglin137/p/3878552.html)

3. [nginx中slab实现](https://www.cnblogs.com/fll369/archive/2012/11/26/2789704.html)

4. [共享内存管理之slab机制](https://blog.csdn.net/hnudlz/article/details/50972596)

5. [nginx slab内存管理](http://www.cnblogs.com/doop-ymc/p/3412572.html)

6. [Nginx开发从入门到精通](http://tengine.taobao.org/book/index.html#)
<br />
<br />
<br />

