---
layout: post
title: core/ngx_shmtx.c源文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们讲述一下nginx中进程之间互斥锁的实现。


<!-- more -->


## 1. 使用原子锁来实现NGINX互斥锁

当前我们支持GCC的```NGX_HAVE_ATOMIC_OPS```原子操作，因此我们是通过原子锁的方式来实现NGINX互斥锁的。

### 1.1 函数ngx_shmtx_create()
{% highlight string %}
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_HAVE_ATOMIC_OPS)


static void ngx_shmtx_wakeup(ngx_shmtx_t *mtx);


ngx_int_t
ngx_shmtx_create(ngx_shmtx_t *mtx, ngx_shmtx_sh_t *addr, u_char *name)
{
    mtx->lock = &addr->lock;

    if (mtx->spin == (ngx_uint_t) -1) {
        return NGX_OK;
    }

    mtx->spin = 2048;

#if (NGX_HAVE_POSIX_SEM)

    mtx->wait = &addr->wait;

    if (sem_init(&mtx->sem, 1, 0) == -1) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_errno,
                      "sem_init() failed");
    } else {
        mtx->semaphore = 1;
    }

#endif

    return NGX_OK;
}
{% endhighlight %}

本函数用于初始化互斥锁结构。一般要求```addr```是处于共享内存中的变量，否则不能实现```进程间互斥锁```。

这里注意到如果```mtx->spin```的值为-1，那么表示直接采用自旋的方式来获取锁，这时就不会采用```Linux信号量```来作为辅助。

### 1.2 函数ngx_shmtx_destroy()
{% highlight string %}
void
ngx_shmtx_destroy(ngx_shmtx_t *mtx)
{
#if (NGX_HAVE_POSIX_SEM)

    if (mtx->semaphore) {
        if (sem_destroy(&mtx->sem) == -1) {
            ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_errno,
                          "sem_destroy() failed");
        }
    }

#endif
}
{% endhighlight %}

这里主要是当采用了```Linux信号量```作为辅助时，需要调用sem_destroy()来释放相应的信号量资源。

### 1.3 函数ngx_shmtx_trylock()
{% highlight string %}
ngx_uint_t
ngx_shmtx_trylock(ngx_shmtx_t *mtx)
{
    return (*mtx->lock == 0 && ngx_atomic_cmp_set(mtx->lock, 0, ngx_pid));
}

{% endhighlight %}
本函数用于尝试获取互斥锁

### 1.4 函数ngx_shmtx_lock()
{% highlight string %}
void
ngx_shmtx_lock(ngx_shmtx_t *mtx)
{
    ngx_uint_t         i, n;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0, "shmtx lock");

    for ( ;; ) {

        if (*mtx->lock == 0 && ngx_atomic_cmp_set(mtx->lock, 0, ngx_pid)) {
            return;
        }

        if (ngx_ncpu > 1) {

            for (n = 1; n < mtx->spin; n <<= 1) {

                for (i = 0; i < n; i++) {
                    ngx_cpu_pause();
                }

                if (*mtx->lock == 0
                    && ngx_atomic_cmp_set(mtx->lock, 0, ngx_pid))
                {
                    return;
                }
            }
        }

#if (NGX_HAVE_POSIX_SEM)

        if (mtx->semaphore) {
            (void) ngx_atomic_fetch_add(mtx->wait, 1);

            if (*mtx->lock == 0 && ngx_atomic_cmp_set(mtx->lock, 0, ngx_pid)) {
                (void) ngx_atomic_fetch_add(mtx->wait, -1);
                return;
            }

            ngx_log_debug1(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                           "shmtx wait %uA", *mtx->wait);

            while (sem_wait(&mtx->sem) == -1) {
                ngx_err_t  err;

                err = ngx_errno;

                if (err != NGX_EINTR) {
                    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, err,
                                  "sem_wait() failed while waiting on shmtx");
                    break;
                }
            }

            ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                           "shmtx awoke");

            continue;
        }

#endif

        ngx_sched_yield();
    }
}

{% endhighlight %}]
本函数用于获取```进程间的互斥锁```。下面我们简要分析一下本函数的实现：
{% highlight string %}
void
ngx_shmtx_lock(ngx_shmtx_t *mtx)
{
	for ( ;; ) {
		//1) 尝试是否能够直接获取到锁

		//2) 当ngx_ncpu大于1时，优先尝试采用自旋的方式来获取锁

		//3) 当采用了Linux信号量时，采用信号量作为辅助来获取锁

		//4) 主动释放CPU控制权，继续下一次循环
	}
}
{% endhighlight %}

### 1.5 函数ngx_shmtx_unlock()
{% highlight string %}
void
ngx_shmtx_unlock(ngx_shmtx_t *mtx)
{
    if (mtx->spin != (ngx_uint_t) -1) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0, "shmtx unlock");
    }

    if (ngx_atomic_cmp_set(mtx->lock, ngx_pid, 0)) {
        ngx_shmtx_wakeup(mtx);
    }
}
{% endhighlight %}
此函数用于释放```互斥锁```。这里我们注意到，还会调用ngx_shmtx_wakeup()来唤醒正在等待信号量的进程。

## 1.6 函数ngx_shmtx_force_unlock()
{% highlight string %}
ngx_uint_t
ngx_shmtx_force_unlock(ngx_shmtx_t *mtx, ngx_pid_t pid)
{
    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                   "shmtx forced unlock");

    if (ngx_atomic_cmp_set(mtx->lock, pid, 0)) {
        ngx_shmtx_wakeup(mtx);
        return 1;
    }

    return 0;
}
{% endhighlight %}
强制解锁。这里需要指定参数```pid```.

### 1.7 函数
{% highlight string %}
static void
ngx_shmtx_wakeup(ngx_shmtx_t *mtx)
{
#if (NGX_HAVE_POSIX_SEM)
    ngx_atomic_uint_t  wait;

    if (!mtx->semaphore) {
        return;
    }

    for ( ;; ) {

        wait = *mtx->wait;

        if ((ngx_atomic_int_t) wait <= 0) {
            return;
        }

        if (ngx_atomic_cmp_set(mtx->wait, wait, wait - 1)) {
            break;
        }
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                   "shmtx wake %uA", wait);

    if (sem_post(&mtx->sem) == -1) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_errno,
                      "sem_post() failed while wake shmtx");
    }

#endif
}
{% endhighlight %}

这里针对采用```Posix信号量```时，需要调用sem_post()来释放信号量。

## 2. 使用文件锁来实现NGINX互斥锁
如下是采用文件锁的方式来实现NGINX互斥锁。默认是不会执行到如下代码的。

### 2.1 函数ngx_shmtx_create()
{% highlight string %}
ngx_int_t
ngx_shmtx_create(ngx_shmtx_t *mtx, ngx_shmtx_sh_t *addr, u_char *name)
{
    if (mtx->name) {

        if (ngx_strcmp(name, mtx->name) == 0) {
            mtx->name = name;
            return NGX_OK;
        }

        ngx_shmtx_destroy(mtx);
    }

    mtx->fd = ngx_open_file(name, NGX_FILE_RDWR, NGX_FILE_CREATE_OR_OPEN,
                            NGX_FILE_DEFAULT_ACCESS);

    if (mtx->fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, ngx_errno,
                      ngx_open_file_n " \"%s\" failed", name);
        return NGX_ERROR;
    }

    if (ngx_delete_file(name) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_errno,
                      ngx_delete_file_n " \"%s\" failed", name);
    }

    mtx->name = name;

    return NGX_OK;
}

{% endhighlight %}
这里通过简单的打开一个文件的方式来创建互斥锁。

### 2.2 函数ngx_shmtx_destroy()
{% highlight string %}
void
ngx_shmtx_destroy(ngx_shmtx_t *mtx)
{
    if (ngx_close_file(mtx->fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", mtx->name);
    }
}
{% endhighlight %}
销毁互斥锁，只需要关闭对应的文件句柄即可。

### 2.3 函数ngx_shmtx_trylock()
{% highlight string %}
ngx_uint_t
ngx_shmtx_trylock(ngx_shmtx_t *mtx)
{
    ngx_err_t  err;

    err = ngx_trylock_fd(mtx->fd);

    if (err == 0) {
        return 1;
    }

    if (err == NGX_EAGAIN) {
        return 0;
    }

#if __osf__ /* Tru64 UNIX */

    if (err == NGX_EACCES) {
        return 0;
    }

#endif

    ngx_log_abort(err, ngx_trylock_fd_n " %s failed", mtx->name);

    return 0;
}
{% endhighlight %}
使用```F_SETLK```,尝试获取文件锁。

### 2.4 函数ngx_shmtx_lock()
{% highlight string %}
void
ngx_shmtx_lock(ngx_shmtx_t *mtx)
{
    ngx_err_t  err;

    err = ngx_lock_fd(mtx->fd);

    if (err == 0) {
        return;
    }

    ngx_log_abort(err, ngx_lock_fd_n " %s failed", mtx->name);
}
{% endhighlight %}

使用```F_SETLKW```，用于获取文件锁。

### 2.5 函数ngx_shmtx_unlock()
{% highlight string %}
void
ngx_shmtx_unlock(ngx_shmtx_t *mtx)
{
    ngx_err_t  err;

    err = ngx_unlock_fd(mtx->fd);

    if (err == 0) {
        return;
    }

    ngx_log_abort(err, ngx_unlock_fd_n " %s failed", mtx->name);
}

{% endhighlight %}
用于释放文件锁。

### 2.6 函数ngx_shmtx_force_unlock()
{% highlight string %}
ngx_uint_t
ngx_shmtx_force_unlock(ngx_shmtx_t *mtx, ngx_pid_t pid)
{
    return 0;
}
{% endhighlight %}
当前不支持。


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

