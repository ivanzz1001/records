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
mmap()函数会在调用进程的虚拟地址空间中创建出一个新的映射(mapping)。该新映射的起始地址有```addr```参数指定，映射的长度由```length```参数指定。

假若```addr```参数为NULL，则内核会帮助我们选择一个合适的地址来创建映射，这也是我们建议的用法，这样可以使程序获得更好的移植性。假若```addr```参数不为NULL的话，则内核会将该地址作为一个参考(hint)来创建映射。在Linux操作系统上，一般创建出的映射的起始地址都是“页”对齐的。mmap()函数返回创建的映射的起始地址。

如果通过mmap()创建的是一个文件映射(与之相对应的是匿名内存映射）的话，则映射的内容会被初始化为```fd```文件从```offset```处开始的length自己的数据。offset也必须是“页”大小的整数倍，“页”大小可以通过如下函数获得：
<pre>
sysconf(_SC_PAGE_SIZE)
</pre>

参数```prot```（protect)用于描述所映射的内存的保护机制（注意：如果是将文件映射到内存，本保护机制不能与文件的open()时的mode相冲突）。其取值可以为```PROT_NONE```或者如下取值的按位或：
<pre>
PROT_EXEC: 映射的“页”可执行

PROT_READ: 映射的“页”可读

PROT_WRITE: 映射的“页”可写

PROT_NONE: 映射的“页”不能被访问
</pre>

参数```flags```用于决定对映射的更新是否对映射同一块区域的其他进程可见，并且对映射的更新是否会反应到底层的文件上。该字段的取值可以为：
<pre>
MAP_SHARED:  共享这一块映射。对映射的更新，其他进程也可以感知到，并且会更新会写到底层的文件上（说明：底层的文件实际的更新可能需要
等到调用msync()或munmap()才会完成）


MAP_PRIVATE:  创建一块私有的copy-on-write(写时复制）映射，对映射的更新并不会反映到映射同一文件的其他进程上，也不会最终写入到底层
的文件中。如果在执行mmap()函数之后，我们对映射的区域进行修改，此时映射是否能感知到我们的修改是不确定的，函数并未对这一情形做出指定。
</pre>
上面```MAP_SHARED```与```MAP_PRIVATE```这两个选项都是POSIX.1所支持的。另外，其还可以**按位或**上以下的取值：
<pre>
MAP_32BIT:  主要是为了使映射的地址在2GB空间内，不常用

MAP_ANON: 与MAP_ANONYMOUS相同含义

MAP_ANONYMOUS: 所创建的映射为匿名映射（而非文件映射)，并且映射的空间会被初始化为0。fd以及offset参数会被忽略，然而有一些系统实现
在此种情况下(MAP_ANONYMOUS或MAP_ANON)还是会要求将fd设置为-1，为了系统的可移植性，建议遵守这一规定。MAP_ANONYMOUS与MAP_SHARED
的组合用法从Linux kernel 2.4开始收到支持

MAP_FIXED: 不要将mmap()中参数addr作为一个映射的参考地址，而是直接映射到该地址。此时传入的addr参数必须是“页”大小的整数倍。此种情
况下，假若addr与len所指定的内存区域与一个已存在的映射相重叠的话，则重叠部分将会被丢弃。而如果指定的地址不可使用的话，则mmap()函数返回错误。一般情况下，不建议使用此选项。

MAP_HUGETLB: 使用"huge pages"来进行映射（since Linux 2.6.32)
</pre>

**说明：** mmap()所创建的映射可以跨越fork()，并且跨越后拥有相同的属性。

下面给出一个示例test.c:
{% highlight string %}
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

int
main(int argc, char *argv[])
{
   char *addr;
   int fd;
   struct stat sb;
   off_t offset, pa_offset;
   size_t length;
   ssize_t s;

   if (argc < 3 || argc > 4) {
	   fprintf(stderr, "%s file offset [length]\n", argv[0]);
	   exit(EXIT_FAILURE);
   }

   fd = open(argv[1], O_RDONLY);
   if (fd == -1)
	   handle_error("open");

   if (fstat(fd, &sb) == -1)           /* To obtain file size */
	   handle_error("fstat");

   offset = atoi(argv[2]);
   pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
	   /* offset for mmap() must be page aligned */

   if (offset >= sb.st_size) {
	   fprintf(stderr, "offset is past end of file\n");
	   exit(EXIT_FAILURE);
   }

   if (argc == 4) {
	   length = atoi(argv[3]);
	   if (offset + length > sb.st_size)
		   length = sb.st_size - offset;
			   /* Can't display bytes past end of file */

   } else {    /* No length arg ==> display to end of file */
	   length = sb.st_size - offset;
   }

   addr = mmap(NULL, length + offset - pa_offset, PROT_READ,
			   MAP_PRIVATE, fd, pa_offset);
   if (addr == MAP_FAILED)
	   handle_error("mmap");

   s = write(STDOUT_FILENO, addr + offset - pa_offset, length);
   if (s != length) {
	   if (s == -1)
		   handle_error("write");

	   fprintf(stderr, "partial write");
	   exit(EXIT_FAILURE);
   }

   exit(EXIT_SUCCESS);
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test helloworld.txt 0
hello,world
</pre>


### 2.2 SystemV共享内存
SystemV共享内存主要用到如下几个API：shmget()、shmat()、shmctl()、shmdt()

**1) 函数shmget()**
{% highlight string %}
#include <sys/ipc.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
{% endhighlight %}
```shmget()```会返回一个SystemV共享内存标识符，该段共享内存与```key```相关联。









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

10. [System V 与 POSIX](http://blog.csdn.net/firstlai/article/details/50705042)

11. [System IPC 与Posix IPC（共享内存）](https://www.cnblogs.com/zhangsf/p/3324169.html)
<br />
<br />
<br />

