---
layout: post
title: core/ngx_rwlock.c(h)文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章讲述nginx中读写锁的实现。


<!-- more -->


## 1. ngx_rwlock.h头文件
{% highlight string %}

/*
 * Copyright (C) Ruslan Ermilov
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_RWLOCK_H_INCLUDED_
#define _NGX_RWLOCK_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


void ngx_rwlock_wlock(ngx_atomic_t *lock);
void ngx_rwlock_rlock(ngx_atomic_t *lock);
void ngx_rwlock_unlock(ngx_atomic_t *lock);


#endif /* _NGX_RWLOCK_H_INCLUDED_ */
{% endhighlight %} 
这里我们声明了3个函数，分别用于实现nginx的读锁、写锁以及锁释放操作。

## 2. ngx_rwlock.c源文件

当前我们支持GCC的```NGX_HAVE_ATOMIC_OPS```原子操作。

### 2.1 函数ngx_rwlock_wlock()
{% highlight string %}

/*
 * Copyright (C) Ruslan Ermilov
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_HAVE_ATOMIC_OPS)


#define NGX_RWLOCK_SPIN   2048
#define NGX_RWLOCK_WLOCK  ((ngx_atomic_uint_t) -1)


void
ngx_rwlock_wlock(ngx_atomic_t *lock)
{
    ngx_uint_t  i, n;

    for ( ;; ) {

        if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, NGX_RWLOCK_WLOCK)) {
            return;
        }

        if (ngx_ncpu > 1) {

            for (n = 1; n < NGX_RWLOCK_SPIN; n <<= 1) {

                for (i = 0; i < n; i++) {
                    ngx_cpu_pause();
                }

                if (*lock == 0
                    && ngx_atomic_cmp_set(lock, 0, NGX_RWLOCK_WLOCK))
                {
                    return;
                }
            }
        }

        ngx_sched_yield();
    }
}

{% endhighlight %}

此处采用自旋的方式获取一把些锁。因为写锁是独占的，因此获取写锁时```*lock```的值必须为0，才能够获取到。下面简要分析一下函数流程：
{% highlight string %}
void
ngx_rwlock_wlock(ngx_atomic_t *lock)
{
	for(;;)
	{
		//1） 如果当前锁是处于空闲状态，那么可以直接获取

		//2) 如果ngx_ncpu > 1，通过自旋的方式以让出CPU，以等待获取到锁

		//3) 直接通过ngx_sched_yield()显示让出CPU
	}
}
{% endhighlight %}

### 2.2 函数ngx_rwlock_rlock()
{% highlight string %}
void
ngx_rwlock_rlock(ngx_atomic_t *lock)
{
    ngx_uint_t         i, n;
    ngx_atomic_uint_t  readers;

    for ( ;; ) {
        readers = *lock;

        if (readers != NGX_RWLOCK_WLOCK
            && ngx_atomic_cmp_set(lock, readers, readers + 1))
        {
            return;
        }

        if (ngx_ncpu > 1) {

            for (n = 1; n < NGX_RWLOCK_SPIN; n <<= 1) {

                for (i = 0; i < n; i++) {
                    ngx_cpu_pause();
                }

                readers = *lock;

                if (readers != NGX_RWLOCK_WLOCK
                    && ngx_atomic_cmp_set(lock, readers, readers + 1))
                {
                    return;
                }
            }
        }

        ngx_sched_yield();
    }
}
{% endhighlight %}
这里用于获取```读锁```，与上面获取```写锁```类似。但读锁可以共享，因此只要当前锁没有被```write lock```独占，就能够获取到。注意到，如果一直获取读锁，可能造成```*lock```值达到最大，而变成些锁。但一般不可能会有如此多的线程来同时请求读锁。

### 2.3 函数ngx_rwlock_unlock()
{% highlight string %}
void
ngx_rwlock_unlock(ngx_atomic_t *lock)
{
    ngx_atomic_uint_t  readers;

    readers = *lock;

    if (readers == NGX_RWLOCK_WLOCK) {
        *lock = 0;
        return;
    }

    for ( ;; ) {

        if (ngx_atomic_cmp_set(lock, readers, readers - 1)) {
            return;
        }

        readers = *lock;
    }
}


#else

#if (NGX_HTTP_UPSTREAM_ZONE || NGX_STREAM_UPSTREAM_ZONE)

#error ngx_atomic_cmp_set() is not defined!

#endif

#endif
{% endhighlight %}
此函数用于释放锁。从代码我们可以看到，释放读锁时，是将对应的值减```1```。


<br />
<br />

**[参看]**

1. [如何设计一个真正高性能的spin_lock?](https://www.zhihu.com/question/55764216/answer/318433668)



<br />
<br />
<br />

