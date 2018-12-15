---
layout: post
title: core/ngx_spinlock.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们讲述一下nginx中spinlock(自旋锁）的实现。



<!-- more -->


## 1. 函数ngx_spinlock()
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


void
ngx_spinlock(ngx_atomic_t *lock, ngx_atomic_int_t value, ngx_uint_t spin)
{

#if (NGX_HAVE_ATOMIC_OPS)

    ngx_uint_t  i, n;

    for ( ;; ) {

        if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
            return;
        }

        if (ngx_ncpu > 1) {

            for (n = 1; n < spin; n <<= 1) {

                for (i = 0; i < n; i++) {
                    ngx_cpu_pause();
                }

                if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, value)) {
                    return;
                }
            }
        }

        ngx_sched_yield();
    }

#else

#if (NGX_THREADS)

#error ngx_spinlock() or ngx_atomic_cmp_set() are not defined !

#endif

#endif

}

{% endhighlight %}
从上面函数的实现我们看到，如果当前Nginx支持```NGX_THREADS```，但是又不支持原子操作的话，则在编译时就会直接报错。因为大部分系统或编译器都实现了各自的原子操作函数，因此这里这样实现也无问题。但是假如真的要确保在没有原子操作的情况下实现```NGX_THREADS```，则可能要用到线程库本身的```互斥```相关函数。

下面简单分析一下函数的实现：
{% highlight string %}
void
ngx_spinlock(ngx_atomic_t *lock, ngx_atomic_int_t value, ngx_uint_t spin)
{
	for ( ;; ) {
		//1) 尝试是否能够直接获取到锁

		//2) 当ngx_ncpu大于1时，优先尝试采用自旋的方式来获取锁

		//3) 主动释放CPU控制权，继续下一次循环
	}
}
{% endhighlight %}




<br />
<br />

**[参看]**



<br />
<br />
<br />

