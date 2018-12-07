---
layout: post
title: core/ngx_shmtx.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们讲述一下nginx中进程之间互斥锁的实现。



<!-- more -->


## 1. ngx_shmtx_sh_t数据结构
{% highlight string %}

#ifndef _NGX_SHMTX_H_INCLUDED_
#define _NGX_SHMTX_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    ngx_atomic_t   lock;
#if (NGX_HAVE_POSIX_SEM)
    ngx_atomic_t   wait;
#endif
} ngx_shmtx_sh_t;
{% endhighlight %}

本结构作为实现```nginx进程间互斥锁```的辅助存在，要求必须存放与共享内存中。

* lock: 作为原子变量，用于实现互斥

* wait: 采用POSIX信号量实现信号通知机制。这里要与```ngx_shmtx_wakeup```联系起来，用于强制解锁。

## 2. ngx_shmtx_t数据结构
{% highlight string %}
typedef struct {
#if (NGX_HAVE_ATOMIC_OPS)
    ngx_atomic_t  *lock;
#if (NGX_HAVE_POSIX_SEM)
    ngx_atomic_t  *wait;
    ngx_uint_t     semaphore;
    sem_t          sem;
#endif
#else
    ngx_fd_t       fd;
    u_char        *name;
#endif
    ngx_uint_t     spin;
} ngx_shmtx_t;
{% endhighlight %}


## 3. 相关函数声明
{% highlight string %}
ngx_int_t ngx_shmtx_create(ngx_shmtx_t *mtx, ngx_shmtx_sh_t *addr,
    u_char *name);
void ngx_shmtx_destroy(ngx_shmtx_t *mtx);
ngx_uint_t ngx_shmtx_trylock(ngx_shmtx_t *mtx);
void ngx_shmtx_lock(ngx_shmtx_t *mtx);
void ngx_shmtx_unlock(ngx_shmtx_t *mtx);
ngx_uint_t ngx_shmtx_force_unlock(ngx_shmtx_t *mtx, ngx_pid_t pid);


#endif /* _NGX_SHMTX_H_INCLUDED_ */
{% endhighlight %}








<br />
<br />

**[参看]**


1. [nginx源码分析1———进程间的通信机制一（信号量）](https://blog.csdn.net/sina_yangyang/article/details/47011303)

2. [Nginx的锁的实现以及惊群的避免](https://www.cnblogs.com/549294286/p/6058811.html)

3. [Nginx源代码分析之锁的实现（十八）](https://blog.csdn.net/namelcx/article/details/52447027)

4. [Nginx---进程锁的实现](https://blog.csdn.net/bytxl/article/details/24580801)

5. [linux进程锁](https://blog.csdn.net/zyembed/article/details/79884211)

6. [进程间同步（进程间互斥锁、文件锁）](https://blog.csdn.net/qq_35396127/article/details/78942245?utm_source=blogxgwz9)

7. [nginx之共享内存](https://blog.csdn.net/evsqiezi/article/details/51785093)

<br />
<br />
<br />

