---
layout: post
title: os/unix/ngx_shmem.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本章我们主要讲述一下nginx底层对共享内存的创建与销毁相关实现。


<!-- more -->

<br />
<br />


## 1. os/unix/ngx_shmem.h头文件
头文件内容如下：
{% highlight string %} 

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SHMEM_H_INCLUDED_
#define _NGX_SHMEM_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    u_char      *addr;
    size_t       size;
    ngx_str_t    name;
    ngx_log_t   *log;
    ngx_uint_t   exists;   /* unsigned  exists:1;  */
} ngx_shm_t;


ngx_int_t ngx_shm_alloc(ngx_shm_t *shm);
void ngx_shm_free(ngx_shm_t *shm);


#endif /* _NGX_SHMEM_H_INCLUDED_ */
{% endhighlight %}

下面我们简单讲述一下各部分：

### 1.1 ngx_shm_t数据结构
{% highlight string %}
typedef struct {
    u_char      *addr;
    size_t       size;
    ngx_str_t    name;
    ngx_log_t   *log;
    ngx_uint_t   exists;   /* unsigned  exists:1;  */
} ngx_shm_t;
{% endhighlight %}
该数据结构用于描述一块共享内存：

* **addr**: 指向共享内存的起始地址

* **size**: 共享内存的长度

* **name**: 这块共享内存的名称

* **log**: 记录日志的ngx_log_t对象

* **exists**: 表示共享内存是否分配过的标志位，如果为1表示该共享内存确实已经存在（即addr已经映射到了实际地址） 


### 1.2 相关函数声明
<pre>
//1: 申请共享内存空间
ngx_int_t ngx_shm_alloc(ngx_shm_t *shm);

//2: 释放共享内存空间
void ngx_shm_free(ngx_shm_t *shm);
</pre>


## 2. os/unix/ngx_shmem.c源文件
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_HAVE_MAP_ANON)

ngx_int_t
ngx_shm_alloc(ngx_shm_t *shm)
{
    shm->addr = (u_char *) mmap(NULL, shm->size,
                                PROT_READ|PROT_WRITE,
                                MAP_ANON|MAP_SHARED, -1, 0);

    if (shm->addr == MAP_FAILED) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "mmap(MAP_ANON|MAP_SHARED, %uz) failed", shm->size);
        return NGX_ERROR;
    }

    return NGX_OK;
}


void
ngx_shm_free(ngx_shm_t *shm)
{
    if (munmap((void *) shm->addr, shm->size) == -1) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "munmap(%p, %uz) failed", shm->addr, shm->size);
    }
}

#elif (NGX_HAVE_MAP_DEVZERO)

ngx_int_t
ngx_shm_alloc(ngx_shm_t *shm)
{
    ngx_fd_t  fd;

    fd = open("/dev/zero", O_RDWR);

    if (fd == -1) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "open(\"/dev/zero\") failed");
        return NGX_ERROR;
    }

    shm->addr = (u_char *) mmap(NULL, shm->size, PROT_READ|PROT_WRITE,
                                MAP_SHARED, fd, 0);

    if (shm->addr == MAP_FAILED) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "mmap(/dev/zero, MAP_SHARED, %uz) failed", shm->size);
    }

    if (close(fd) == -1) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "close(\"/dev/zero\") failed");
    }

    return (shm->addr == MAP_FAILED) ? NGX_ERROR : NGX_OK;
}


void
ngx_shm_free(ngx_shm_t *shm)
{
    if (munmap((void *) shm->addr, shm->size) == -1) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "munmap(%p, %uz) failed", shm->addr, shm->size);
    }
}

#elif (NGX_HAVE_SYSVSHM)

#include <sys/ipc.h>
#include <sys/shm.h>


ngx_int_t
ngx_shm_alloc(ngx_shm_t *shm)
{
    int  id;

    id = shmget(IPC_PRIVATE, shm->size, (SHM_R|SHM_W|IPC_CREAT));

    if (id == -1) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "shmget(%uz) failed", shm->size);
        return NGX_ERROR;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, shm->log, 0, "shmget id: %d", id);

    shm->addr = shmat(id, NULL, 0);

    if (shm->addr == (void *) -1) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno, "shmat() failed");
    }

    if (shmctl(id, IPC_RMID, NULL) == -1) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "shmctl(IPC_RMID) failed");
    }

    return (shm->addr == (void *) -1) ? NGX_ERROR : NGX_OK;
}


void
ngx_shm_free(ngx_shm_t *shm)
{
    if (shmdt(shm->addr) == -1) {
        ngx_log_error(NGX_LOG_ALERT, shm->log, ngx_errno,
                      "shmdt(%p) failed", shm->addr);
    }
}

#endif
{% endhighlight %}

我们在ngx_auto_config.h头文件中有如下定义：
<pre>
#ifndef NGX_HAVE_MAP_ANON
#define NGX_HAVE_MAP_ANON  1
#endif


#ifndef NGX_HAVE_MAP_DEVZERO
#define NGX_HAVE_MAP_DEVZERO  1
#endif


#ifndef NGX_HAVE_SYSVSHM
#define NGX_HAVE_SYSVSHM  1
#endif
</pre>
表示对这三种类型的共享内存都支持，这里我们实际上用到的是POSIX匿名内存映射(```NGX_HAVE_MAP_ANON```)。虽然我们只用到一种，但是这里我们还是会把这三种方式都介绍一下。

### 2.1 mmap()函数介绍
{% highlight string %}
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags,
          int fd, off_t offset);
int munmap(void *addr, size_t length);
{% endhighlight %}





<br />
<br />
**[参看]:**

1. [nginx进程间的通信](https://www.cnblogs.com/cxchanpin/p/7241346.html)

2. [Nginx源码分析（1）之——共享内存的配置、分配及初始化](http://blog.csdn.net/hnudlz/article/details/50964065)

3. [nginx 进程通信--共享内存](https://www.cnblogs.com/fll369/archive/2012/11/26/2789233.html)

4. [nginx之共享内存](http://blog.csdn.net/evsqiezi/article/details/51785093)

5. [绝对详细！Nginx基本配置、性能优化指南](http://www.chinaz.com/web/2015/0424/401323.shtml)

6. [Nginx有哪些有趣的玩法？](https://www.zhihu.com/question/34429320)

7. [Nginx 多进程连接请求/事件分发流程分析](https://www.cnblogs.com/NerdWill/p/4992345.html)

8. [system v和posix的共享内存对比 & 共享内存位置](http://www.cnblogs.com/charlesblc/p/6261469.html)

9. [linux进程间通信-----System V共享内存总结实例](http://blog.csdn.net/Linux_ever/article/details/50372573)

<br />
<br />
<br />

