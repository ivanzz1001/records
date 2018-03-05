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

### 2.2 Posix内存共享
其实上面讲解的mmap()就是属于posix内存共享。这里我们再介绍几个函数：shm_open()、shm_unlink()、ftruncate()、fstat()。其实这里讲解的posix共享内存只是对mmap()的一个外壳包装。

**1) 函数shm_open()、shm_unlink()**
{% highlight string %}
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

int shm_open(const char *name, int oflag, mode_t mode);

int shm_unlink(const char *name);

//Link with -lrt.
{% endhighlight %}
函数shm_open()用于创建一个新的、或者打开一个已存在的共享内存对象。一个Posix共享内存对象作用于一个handle，该handle可以被无关进程的mmap()所映射，以此来实现共享内存。而函数shm_unlink()执行相反的操作，其将会移除一个由shm_open()所创建的共享内存对象。            

shm_open()函数的操作类似于open()函数。参数```name```用于指定将要创建或打开的共享内存对象，为了系统的可移植性，共享内存对象的名称最好类似于```/somename```，由字符组成的长度最长为```NAME_MAX```的路径名。

参数```oflag```可以为如下值的按位或操作：

* **O_RDONLY**: 打开一个共享内存对象用于读访问。由此标志打开的共享内存对象只能有mmap()函数映射为读操作

* **O_RDWR**: 映射共享内存对象为可读写

* **O_CREAT**: 假如共享内存对象不存在的话则创建。所创建的共享内存对象的所有者和所属组为调用进程的有效用户ID，权限为参数```mode```的低九位（注意umask()的影响）。注意： 一个新创建的共享内存长度为0，可以通过ftruncate()函数来更改大小，一个共享内存新分配的空间会被自动初始化为0。

* **O_EXCL**: 假若O_CREAT也同时被指定的话，假若name所指定的共享内存已存在的话，则返回错误。使用本标志可以完成共享内存对象存在性检查，并且在不存在时完成创建工作。

* **O_TRUNC**: 假如共享内存对象已经存在，则将其截断为0

shm_open()调用成功时返回一个代表该共享内存对象的文件描述符，该文件描述符被确保为当前进程中未使用的文件描述符的最小值。并且该文件描述符会被自动的设置为FD_CLOEXEC。

该文件描述符通常可以被用于ftruncate()函数和mmap()函数。在调用完mmap()函数之后关闭该文件描述符是不会影响到内存映射的。

函数shm_unlink()的操作类似与unlink(): 它移除一个共享内存对象的名称，然后一旦所有的进程unmmap()该共享内存对象之后，就会销毁并释放所关联的共享内存区域。一旦成功调用shm_unlink()成功，试图再次调用shm_open()函数打开相同名称的共享内存对象都将会失败（除非O_CREAT标志存在，这样其实是创建一个新的共享内存）

**返回值：** shm_open()调用成功时，返回一个非负的文件描述符值；否则返回-1。shm_unlink()成功时返回0，失败时返回-1.

<br />

**2) 函数ftruncate()**
{% highlight string %}
#include <unistd.h>
#include <sys/types.h>

int truncate(const char *path, off_t length);
int ftruncate(int fd, off_t length);
{% endhighlight %}
将某一文件截断到某一长度。

<br />


### 2.3 SystemV共享内存
SystemV共享内存主要用到如下几个API：shmget()、ftok()、shmat()、shmctl()、shmdt()

**1) 函数shmget()**
{% highlight string %}
#include <sys/ipc.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
{% endhighlight %}
```shmget()```会返回一段与```key```相关联的SystemV共享内存标识符。在如下两种情况下会创建一个新的```size```(size会进行N倍的PAGE_SIZE上对齐）大小的共享内存段：

* key取值为```IPC_PRIVATE```，且shmflag标志被指定为```IPC_CREAT```

* key取值为其他值，并且当前系统中与该key相对应的共享内存段不存在，且shmflg标志被指定为```IPC_CREAT```

假如shmflg标志同时指定了```IPC_CREAT```和```IPC_EXCL```，并且与key所关联的共享内存段已经存在的话，则shmget()函数返回错误并将errno设置为```EEXIST```（这与open()函数的O_CREAT|O_EXCL类似）。

shmflg可以取如下值：

* **IPC_CREAT**: 创建一段新的共享内存段。假如并未使用本标志，则shmget()会查找key所关联的共享内存段，然后检查用户是否是否有权限访问该共享内存段。

* **IPC_EXCL**： 搭配```IPC_CREAT```一起使用，用于确保当key所指定的内存段已经存在时，返回错误

* **mode_flags**: (shmflg的低9位)用于指定所有者、所属组、其他人对该共享内存段的访问权限。这些bits与open()函数的mode参数具有相同的格式、相同的含义。注意：当前execute权限并不会被系统所使用。

* **SHM_HUGETLB**: (since Linux 2.6)使用"huge pages"分配共享内存段

* **SHM_NORESERVE**： (since Linux 2.6.15)本标志与mmap()的```MAP_NORESERVE```标志一致，表示并不为本共享内存段保留swap空间。当swap空间被保留，这样就能够确保可以修改共享内存段；假如不保留swap空间的话，则在没有物理内存可用的情况下写共享内存段可能会产生```SIGSEGV```错误。

假如成功的创建了一块新的共享内存段之后，则其内容就会被初始化为0，而与其相关联的数据结构shmid_ds将会被初始化为如下值：

* shm_perm.cuid与shm_perm.uid会被设置为调用进程的有效用户ID

* shm_perm.cgid和shm_perm.gid会被设置为调用进程的有效组ID

* shm_perm.mode的最低9bits会被设置为shmflg的最低9bits

* shm_segsz会被设置为参数```size```的值

* shm_lpid、shm_nattch、shm_atime、shm_dtime会被设置为0

* shm_ctime会被设置为当前时间

假如共享内存段已经存在，则会检查相应的权限，然后还会检查其是否还被标记为```销毁状态```。

函数在成功时返回共享内存标识符；失败时返回-1。

<br />

下面举一个例子test.c:
{% highlight string %}
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#define KEY_ID    0x1000
#define SIZE      128

int main(int argc,char *argv[])
{
    int shmid;
    shmid = shmget(KEY_ID, SIZE, 0x0);
    if(shmid < 0)
    {
         printf("shmget(KEY_ID, SIZE, 0x0) failure\n");
    }

    shmid = shmget(KEY_ID, SIZE, IPC_CREAT);
    if(shmid < 0)
    {
         printf("shmget(KEY_ID, SIZE, IPC_CREATE) failure(1)\n");
    }

    shmid = shmget(KEY_ID, SIZE, IPC_CREAT);
    if(shmid < 0)
    {
        printf("shmget(KEY_ID, SIZE, IPC_CREAT) failure(2)\n");
    }
    
    shmid = shmget(KEY_ID, SIZE, IPC_CREAT|0600);
    if(shmid < 0)
    {
        printf("shmget(KEY_ID, SIZE, IPC_CREATE|0600) failure\n");
    }
   
    shmid = shmget(KEY_ID, SIZE, IPC_CREAT|IPC_EXCL|0600);
    if(shmid < 0)
    {
        printf("shmget(KEY_ID, SIZE, IPC_CREATE|IPC_EXCL|0600) failure\n");
    }

    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
//第一次运行：
# gcc -o test test.c
# ./test
shmget(KEY_ID, SIZE, 0x0) failure
shmget(KEY_ID, SIZE, IPC_CREAT|IPC_EXCL|0600) failure
# ipcs -m

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x00001000 131072     root       0          128        0                       


//第二次运行
# ./test
shmget(KEY_ID, SIZE, IPC_CREAT|IPC_EXCL|0600) failure
# ipcs -m

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      
0x00001000 131072     root       0          128        0                       

# ipcrm -m 131072
# ipcs -m

------ Shared Memory Segments --------
key        shmid      owner      perms      bytes      nattch     status      

</pre>
从上面我们可以看到，当指定key的共享内存段不存在时，必须要携带```IPC_CREAT```标识；当存在时加不加```IPC_CREAT```都无所谓，都会返回该已创建的共享内存段

同时我们可以用```ipcs -m```命令来查看当前系统上创建的所有共享内存段；用```ipcrm -m```来删除共享内存段。


**2) 函数ftok()**
{% highlight string %}
#include <sys/types.h>
#include <sys/ipc.h>

key_t ftok(const char *pathname, int proj_id);
{% endhighlight %}
ftok()函数使用path_name标识的文件(注意此文件必须存在，且能被访问）以及proj_id的低8bit（必须为非0）来产生一个SystemV IPC key。这个key可用于msgget()、semget()、和shmget()。

当proj_id相同时，对于同一个文件的所有不同路径（相对路径、绝对路径等）ftok()函数都会返回相同的值。假如路径或者proj_id不同时，ftok()函数会返回不同的值。

<br />
下面举一个例子test2.c:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>


int main(int argc,char *argv[])
{
   key_t key;
   char buf[1024];
   
   key = ftok("/root",123);
   printf("%d\n",key);

   getcwd(buf,sizeof(buf));
   printf("buf:%s\n",buf);

   key = ftok("../",123);
   printf("%d\n",key);
   return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test2 test2.c
# ./test2
2063805345
buf:/root/test-src
2063805345
</pre>

**3) 函数shmat()**
{% highlight string %}
#include <sys/types.h>
#include <sys/shm.h>

void *shmat(int shmid, const void *shmaddr, int shmflg);
{% endhighlight %}
shmat()函数将```shmid```标识的共享内存段绑定到当前调用进程的地址空间中。绑定的地址通过参数```shmaddr```指定，并且遵循如下准则：

* 假如shmaddr为NULL，系统会选择一个合适的未使用的地址绑定到该共享内存段

* 假如shmaddr不为NULL，并且```SHM_RND```在shmflg中被指定，则绑定的地址为参数shmaddr指定地址的```SHMLBA```字节下对齐；否则参数shmaddr必须指定为“页”对齐的地址。

如果在shmflg中被指定了```SHM_RDONLY```，则被绑定的共享内存段只允许读，并且进程必须要有对该共享内存段的读权限。否则被绑定的共享内存段具有读写权限，并且进程必须要有对该共享内存段的读写权限。并没有```只写```共享内存段这一概念。

此外如果shmflg中被指定了```SHM_REMAP```(本选项为Linux专属），则意味着当前映射的共享内存段应该替换任何在该位置已经存在的共享内存段（如果不用此选项的话，则在该地址空间若已经存在一个共享内存段会返回```EINVAL```错误）。
<pre>
说明： 在进程退出的时候，该被绑定的共享内存段会自动解绑。同一个共享内存段，可以在一个进程中可以被多次的绑定
</pre>

函数shmat()映射成功之后，会更新与该共享内存段所关联的shmid_ds结构：

* **shm_atime**： 会被设置为当前时间

* **shm_lpid**: 会被设置为当前调用进程的进程ID

* **shm_nattch**: 在原来的基础上+1

shmat()函数在成功时会返回映射的共享内存地址；在失败时返回-1.

注意： fork()函数调用之后，子进程会继承该绑定的共享内存段。而在执行exec函数族后，所有该进程绑定的共享内存段都会被解绑。在调用_exit()函数后，所有该进程绑定的共享内存段也会被解绑。

<br />

**4) 函数shmdt()**
{% highlight string %}
#include <sys/types.h>
#include <sys/shm.h>

int shmdt(const void *shmaddr);
{% endhighlight %}
函数shmdt()从当前调用进程中解绑shmaddr地址处的共享内存段。此处将要解绑的共享内存段的地址shmaddr必须为shmat()函数的返回值。

在shmdt()调用成功时，系统会更新与该共享内存段所关联的shmid_ds数据结构：

* **shm_dtime**: 会被设置为当前时间

* **shm_lpid**： 会被设置为当前调用进程的PID

* **shm_nattch**: 会在原来的基础上-1。如果该值变为0的时候，则会将该共享内存段标记为```deletion```状态，该共享内存段就会被删除。

函数shmdt()在成功时返回0；失败时返回-1.

<br />

**5) 函数shmctl()**
{% highlight string %}
#include <sys/ipc.h>
#include <sys/shm.h>

int shmctl(int shmid, int cmd, struct shmid_ds *buf);
{% endhighlight %}
函数shmctl()会在shmid所指定的SystemV共享内存段上执行参数cmd所表示的控制命令。其中buf参数是一个```shmid_ds```类型的数据结构，其在```<sys/shm.h>```中被定义为：
{% highlight string %}
struct shmid_ds {
   struct ipc_perm shm_perm;    /* Ownership and permissions */
   size_t          shm_segsz;   /* Size of segment (bytes) */
   time_t          shm_atime;   /* Last attach time */
   time_t          shm_dtime;   /* Last detach time */
   time_t          shm_ctime;   /* Last change time */
   pid_t           shm_cpid;    /* PID of creator */
   pid_t           shm_lpid;    /* PID of last shmat(2)/shmdt(2) */
   shmatt_t        shm_nattch;  /* No. of current attaches */
   ...
};

struct ipc_perm {
   key_t          __key;    /* Key supplied to shmget(2) */
   uid_t          uid;      /* Effective UID of owner */
   gid_t          gid;      /* Effective GID of owner */
   uid_t          cuid;     /* Effective UID of creator */
   gid_t          cgid;     /* Effective GID of creator */
   unsigned short mode;     /* Permissions + SHM_DEST and
                               SHM_LOCKED flags */
   unsigned short __seq;    /* Sequence number */
};
{% endhighlight %}

cmd支持的值有如下：

* **IPC_STAT**: 将shmid所关联的共享内存段的内核数据结构拷贝到buf中，调用者必须要有该共享内存段的读权限。

* **IPC_SET**: 将buf所指定的shmid_ds数据结构中的一些值写入到shmid所指定的共享内存的内核数据结构中，同时也会更新该内核结构的```shm_ctime```字段。如下的一下字段可以被改变： shm_perm.uid、shm_perm.gid以及shm_perm.mode的低9位。要执行此命令，调用进程的有效用户ID必须为该共享内存段的所有者(shm_perm.uid)或者创建者(shm_perm.cuid),又或者调用进程为特权进程。

* **IPC_RMID**: 将该共享内存段标记为```销毁状态```。而一个共享内存段真正被销毁是发生在最后一个进程对该共享内存段解绑之后（即shm_attch变为0）。要调用此命令，调用进程必须为该共享内存段的所有者或创建者，或者该调用进程为特权进程。假如一个共享内存段被标记为销毁状态，则shm_perm.mode字段的```SHM_DEST```标志会被设置。

* **IPC_INFO**: (Linux specific)通过buf返回系统级别的共享内存的限制与参数。此时buf结构应该为shminfo类型（需要做强制类型转换），该类型定义在<sys/shm.h>头文件中，并且需要定义```_GNU_SOURCE```宏：
{% highlight string %}
struct  shminfo {
	 unsigned long shmmax; /* Maximum segment size */
	 unsigned long shmmin; /* Minimum segment size;
	                          always 1 */
	 unsigned long shmmni; /* Maximum number of segments */
	 unsigned long shmseg; /* Maximum number of segments
	                          that a process can attach;
	                          unused within kernel */
	 unsigned long shmall; /* Maximum number of pages of
	                          shared memory, system-wide */
};
{% endhighlight %}
可以通过修改/proc目录下的shmmin、shmmax、shmall文件来更改相应的限制。

* **SHM_INFO**: (Linux specific)返回一个shm_info结构体，该结构体的相应字段反应了当前系统共享内存所消耗的系统资源。该结构体定义在<sys/shm.h>头文件中，并且需要定义```_GNU_SOURCE```宏：
{% highlight string %}
struct shm_info {
 int           used_ids;         /* # of currently existing segments */

 unsigned long shm_tot;          /* Total number of shared memory pages */

 unsigned long shm_rss;          /* # of resident shared memory pages */

 unsigned long shm_swp;          /* # of swapped shared memory pages */

 unsigned long swap_attempts;    /* Unused since Linux 2.4 */

 unsigned long swap_successes;   /* Unused since Linux 2.4 */
};
{% endhighlight %}

针对上述两个字段，我们举个例子test.c:
{% highlight string %}
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>


int main(int argc,char *argv[])
{
     struct shminfo ipcinfo;
     struct shm_info info;
     int shmid;


     if(shmctl(0,IPC_INFO,(struct shmid_ds *)&ipcinfo) == -1)
     {
         printf("get ipc info failure\n");
         exit(-1);
     }


     printf("IPC_INFO.shmmax: %lu\n",ipcinfo.shmmax);
     printf("IPC_INFO.shmmin: %lu\n",ipcinfo.shmmin);
     printf("IPC_INFO.shmmni: %lu\n",ipcinfo.shmmni);
     printf("IPC_INFO.shmseg: %lu\n",ipcinfo.shmseg);
     printf("IPC_INFO.shmall: %lu\n",ipcinfo.shmall);


     shmid = shmget(IPC_PRIVATE, 8192,IPC_CREAT);
     if(shmid < 0)
     {
         printf("create share memory failure\n");
         exit(-2);
     }
     if(shmctl(0,SHM_INFO,(struct shmid_ds *)&info) == -1)
     {
         printf("get share memory info failure\n");
         exit(-1);
     }

     printf("shm_info.used_ids:%d\n",info.used_ids);
     printf("shm_info.shm_tot:%lu\n",info.shm_tot);
     printf("shm_info.shm_rss:%lu\n",info.shm_rss);
     printf("shm_info.shm_swp:%lu\n",info.shm_swp);
     printf("shm_info.swap_attempts:%lu\n",info.swap_attempts);
     printf("shm_info.swap_successes:%lu\n",info.swap_successes);

     return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test
IPC_INFO.shmmax: 18446744073692774399
IPC_INFO.shmmin: 1
IPC_INFO.shmmni: 4096
IPC_INFO.shmseg: 4096
IPC_INFO.shmall: 18446744073692774399
shm_info.used_ids:1
shm_info.shm_tot:2
shm_info.shm_rss:0
shm_info.shm_swp:0
shm_info.swap_attempts:0
shm_info.swap_successes:0

[root@localhost test-src]# ./test
IPC_INFO.shmmax: 18446744073692774399
IPC_INFO.shmmin: 1
IPC_INFO.shmmni: 4096
IPC_INFO.shmseg: 4096
IPC_INFO.shmall: 18446744073692774399
shm_info.used_ids:2
shm_info.shm_tot:4
shm_info.shm_rss:0
shm_info.shm_swp:0
shm_info.swap_attempts:0
shm_info.swap_successes:0

//如下清除相应的共享内存
# ids=`ipcs -m | awk '{print $2}' | grep -v Shared | grep -v shmid`
# for id in $ids; do echo "remove share memory: $id"; ipcrm -m $id; done
</pre>

* **SHM_STAT**: (Linux specific)与IPC_STAT类似，返回一个shmid_ds数据结构。然而，这里参数shmid并不是一个共享内存段标识符，而是该共享内存段在系统内核共内存数组中的索引值。

此外，我们可以在```cmd```参数中添加如下值来允许或禁止共享内存段使用交换分区：

* **SHM_LOCK**: (Linux specific)禁止共享内存使用交换分区。此时，调用者必须确保在锁定后共享内存都处于内存中，而不是在交换分区中。

* **SHM_UNLOCK**: (Linux specific) 解锁被锁定的共享内存段，以使其可以放到交换分区中去

<br />
**函数返回值：** ```IPC_INFO```及```SHM_INFO```操作成功时返回系统中共享内存段所对应的内核数组的最大索引值(该值可被用于```SHM_STAT```操作，以获取所有共享内存内存的相关信息)。```SHM_STAT```成功时返回shmid所指定索引处的共享内存标识符。对于其他操作，成功时返回0；失败时返回-1。

### 2.4 ngx_shmem.c源代码分析

**1) mmap匿名内存映射**
{% highlight string %}
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
#endif
{% endhighlight %}
这里比较简单，直接采用mmap()的匿名内存映射```MAP_ANON```。

<br />

**2) mmap命名内存映射**
{% highlight string %}
#if (NGX_HAVE_MAP_DEVZERO) 
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
#endif
{% endhighlight %}
有些只支持mmap()命名内存映射，因此这里采用打开/dev/zero文件来实现命名内存映射。

<br />

**3) SystemV共享内存**
{% highlight string %}
#if (NGX_HAVE_SYSVSHM)

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
这里注意在调用shmat()成功之后，立马就调用shmctl(id,IPC_RMID,NULL)将该共享内存段标记为```销毁状态```，这样当最后一个进程分离之后该共享内存段就会真正被销毁。


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

