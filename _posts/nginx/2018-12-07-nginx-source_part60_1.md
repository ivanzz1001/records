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
本数据结构用于实现```进程间的互斥锁```。当支持**NGX_HAVE_ATOMIC_OPS**宏定义时采用原子锁来实现，否则采用文件锁来实现。当前我们支持GCC的```NGX_HAVE_ATOMIC_OPS```原子操作。

下面我们简要介绍一下各字段的含义:

* lock: 用于实现原子锁

* wait: 可以用于记录当前正在sem_wait()信号量的```请求者数量```。当前我们支持```NGX_HAVE_POSIX_SEM```宏定义

* semaphore: 用于记录信号量是否创建成功

* sem: linux信号量。用于辅助实现NGINX互斥锁，可以获得较高的性能

* fd: 用于实现文件锁的句柄

* name: 文件的名称

* spin: 当不能马上获取到锁时，会采用自旋的方式pause CPU，这里用于控制每一次主动让出CPU之前(通过调用ngx_sched_yield())，进行自旋的次数。


## 3. 相关函数声明
{% highlight string %}
// 创建一把'nginx进程间互斥锁'
ngx_int_t ngx_shmtx_create(ngx_shmtx_t *mtx, ngx_shmtx_sh_t *addr,
    u_char *name);

// 销毁互斥锁
void ngx_shmtx_destroy(ngx_shmtx_t *mtx);

// 尝试获取互斥锁
ngx_uint_t ngx_shmtx_trylock(ngx_shmtx_t *mtx);

//获取互斥锁
void ngx_shmtx_lock(ngx_shmtx_t *mtx);

//解锁
void ngx_shmtx_unlock(ngx_shmtx_t *mtx);

//强制解锁
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

