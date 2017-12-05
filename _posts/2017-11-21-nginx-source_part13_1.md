---
layout: post
title: os/unix/ngx_files.h源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本文主要介绍一下ngx_files.h头文件，其主要定义了一些对文件进行操作的数据结构，以及文件操作的一些函数原型。

<!-- more -->

<br />
<br />


## 1. os/unix/ngx_files.h头文件

头文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_FILES_H_INCLUDED_
#define _NGX_FILES_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef int                      ngx_fd_t;
typedef struct stat              ngx_file_info_t;
typedef ino_t                    ngx_file_uniq_t;


typedef struct {
    u_char                      *name;
    size_t                       size;
    void                        *addr;
    ngx_fd_t                     fd;
    ngx_log_t                   *log;
} ngx_file_mapping_t;


typedef struct {
    DIR                         *dir;
    struct dirent               *de;
    struct stat                  info;

    unsigned                     type:8;
    unsigned                     valid_info:1;
} ngx_dir_t;


typedef struct {
    size_t                       n;
    glob_t                       pglob;
    u_char                      *pattern;
    ngx_log_t                   *log;
    ngx_uint_t                   test;
} ngx_glob_t;


#define NGX_INVALID_FILE         -1
#define NGX_FILE_ERROR           -1



#ifdef __CYGWIN__

#ifndef NGX_HAVE_CASELESS_FILESYSTEM
#define NGX_HAVE_CASELESS_FILESYSTEM  1
#endif

#define ngx_open_file(name, mode, create, access)                            \
    open((const char *) name, mode|create|O_BINARY, access)

#else

#define ngx_open_file(name, mode, create, access)                            \
    open((const char *) name, mode|create, access)

#endif

#define ngx_open_file_n          "open()"

#define NGX_FILE_RDONLY          O_RDONLY
#define NGX_FILE_WRONLY          O_WRONLY
#define NGX_FILE_RDWR            O_RDWR
#define NGX_FILE_CREATE_OR_OPEN  O_CREAT
#define NGX_FILE_OPEN            0
#define NGX_FILE_TRUNCATE        (O_CREAT|O_TRUNC)
#define NGX_FILE_APPEND          (O_WRONLY|O_APPEND)
#define NGX_FILE_NONBLOCK        O_NONBLOCK

#if (NGX_HAVE_OPENAT)
#define NGX_FILE_NOFOLLOW        O_NOFOLLOW

#if defined(O_DIRECTORY)
#define NGX_FILE_DIRECTORY       O_DIRECTORY
#else
#define NGX_FILE_DIRECTORY       0
#endif

#if defined(O_SEARCH)
#define NGX_FILE_SEARCH          (O_SEARCH|NGX_FILE_DIRECTORY)

#elif defined(O_EXEC)
#define NGX_FILE_SEARCH          (O_EXEC|NGX_FILE_DIRECTORY)

#elif (NGX_HAVE_O_PATH)
#define NGX_FILE_SEARCH          (O_PATH|O_RDONLY|NGX_FILE_DIRECTORY)

#else
#define NGX_FILE_SEARCH          (O_RDONLY|NGX_FILE_DIRECTORY)
#endif

#endif /* NGX_HAVE_OPENAT */

#define NGX_FILE_DEFAULT_ACCESS  0644
#define NGX_FILE_OWNER_ACCESS    0600


#define ngx_close_file           close
#define ngx_close_file_n         "close()"


#define ngx_delete_file(name)    unlink((const char *) name)
#define ngx_delete_file_n        "unlink()"


ngx_fd_t ngx_open_tempfile(u_char *name, ngx_uint_t persistent,
    ngx_uint_t access);
#define ngx_open_tempfile_n      "open()"


ssize_t ngx_read_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset);
#if (NGX_HAVE_PREAD)
#define ngx_read_file_n          "pread()"
#else
#define ngx_read_file_n          "read()"
#endif

ssize_t ngx_write_file(ngx_file_t *file, u_char *buf, size_t size,
    off_t offset);

ssize_t ngx_write_chain_to_file(ngx_file_t *file, ngx_chain_t *ce,
    off_t offset, ngx_pool_t *pool);


#define ngx_read_fd              read
#define ngx_read_fd_n            "read()"

/*
 * we use inlined function instead of simple #define
 * because glibc 2.3 sets warn_unused_result attribute for write()
 * and in this case gcc 4.3 ignores (void) cast
 */
static ngx_inline ssize_t
ngx_write_fd(ngx_fd_t fd, void *buf, size_t n)
{
    return write(fd, buf, n);
}

#define ngx_write_fd_n           "write()"


#define ngx_write_console        ngx_write_fd


#define ngx_linefeed(p)          *p++ = LF;
#define NGX_LINEFEED_SIZE        1
#define NGX_LINEFEED             "\x0a"


#define ngx_rename_file(o, n)    rename((const char *) o, (const char *) n)
#define ngx_rename_file_n        "rename()"


#define ngx_change_file_access(n, a) chmod((const char *) n, a)
#define ngx_change_file_access_n "chmod()"


ngx_int_t ngx_set_file_time(u_char *name, ngx_fd_t fd, time_t s);
#define ngx_set_file_time_n      "utimes()"


#define ngx_file_info(file, sb)  stat((const char *) file, sb)
#define ngx_file_info_n          "stat()"

#define ngx_fd_info(fd, sb)      fstat(fd, sb)
#define ngx_fd_info_n            "fstat()"

#define ngx_link_info(file, sb)  lstat((const char *) file, sb)
#define ngx_link_info_n          "lstat()"

#define ngx_is_dir(sb)           (S_ISDIR((sb)->st_mode))
#define ngx_is_file(sb)          (S_ISREG((sb)->st_mode))
#define ngx_is_link(sb)          (S_ISLNK((sb)->st_mode))
#define ngx_is_exec(sb)          (((sb)->st_mode & S_IXUSR) == S_IXUSR)
#define ngx_file_access(sb)      ((sb)->st_mode & 0777)
#define ngx_file_size(sb)        (sb)->st_size
#define ngx_file_fs_size(sb)     ngx_max((sb)->st_size, (sb)->st_blocks * 512)
#define ngx_file_mtime(sb)       (sb)->st_mtime
#define ngx_file_uniq(sb)        (sb)->st_ino


ngx_int_t ngx_create_file_mapping(ngx_file_mapping_t *fm);
void ngx_close_file_mapping(ngx_file_mapping_t *fm);


#define ngx_realpath(p, r)       (u_char *) realpath((char *) p, (char *) r)
#define ngx_realpath_n           "realpath()"
#define ngx_getcwd(buf, size)    (getcwd((char *) buf, size) != NULL)
#define ngx_getcwd_n             "getcwd()"
#define ngx_path_separator(c)    ((c) == '/')


#if defined(PATH_MAX)

#define NGX_HAVE_MAX_PATH        1
#define NGX_MAX_PATH             PATH_MAX

#else

#define NGX_MAX_PATH             4096

#endif


#define NGX_DIR_MASK_LEN         0


ngx_int_t ngx_open_dir(ngx_str_t *name, ngx_dir_t *dir);
#define ngx_open_dir_n           "opendir()"


#define ngx_close_dir(d)         closedir((d)->dir)
#define ngx_close_dir_n          "closedir()"


ngx_int_t ngx_read_dir(ngx_dir_t *dir);
#define ngx_read_dir_n           "readdir()"


#define ngx_create_dir(name, access) mkdir((const char *) name, access)
#define ngx_create_dir_n         "mkdir()"


#define ngx_delete_dir(name)     rmdir((const char *) name)
#define ngx_delete_dir_n         "rmdir()"


#define ngx_dir_access(a)        (a | (a & 0444) >> 2)


#define ngx_de_name(dir)         ((u_char *) (dir)->de->d_name)
#if (NGX_HAVE_D_NAMLEN)
#define ngx_de_namelen(dir)      (dir)->de->d_namlen
#else
#define ngx_de_namelen(dir)      ngx_strlen((dir)->de->d_name)
#endif

static ngx_inline ngx_int_t
ngx_de_info(u_char *name, ngx_dir_t *dir)
{
    dir->type = 0;
    return stat((const char *) name, &dir->info);
}

#define ngx_de_info_n            "stat()"
#define ngx_de_link_info(name, dir)  lstat((const char *) name, &(dir)->info)
#define ngx_de_link_info_n       "lstat()"

#if (NGX_HAVE_D_TYPE)

/*
 * some file systems (e.g. XFS on Linux and CD9660 on FreeBSD)
 * do not set dirent.d_type
 */

#define ngx_de_is_dir(dir)                                                   \
    (((dir)->type) ? ((dir)->type == DT_DIR) : (S_ISDIR((dir)->info.st_mode)))
#define ngx_de_is_file(dir)                                                  \
    (((dir)->type) ? ((dir)->type == DT_REG) : (S_ISREG((dir)->info.st_mode)))
#define ngx_de_is_link(dir)                                                  \
    (((dir)->type) ? ((dir)->type == DT_LNK) : (S_ISLNK((dir)->info.st_mode)))

#else

#define ngx_de_is_dir(dir)       (S_ISDIR((dir)->info.st_mode))
#define ngx_de_is_file(dir)      (S_ISREG((dir)->info.st_mode))
#define ngx_de_is_link(dir)      (S_ISLNK((dir)->info.st_mode))

#endif

#define ngx_de_access(dir)       (((dir)->info.st_mode) & 0777)
#define ngx_de_size(dir)         (dir)->info.st_size
#define ngx_de_fs_size(dir)                                                  \
    ngx_max((dir)->info.st_size, (dir)->info.st_blocks * 512)
#define ngx_de_mtime(dir)        (dir)->info.st_mtime


ngx_int_t ngx_open_glob(ngx_glob_t *gl);
#define ngx_open_glob_n          "glob()"
ngx_int_t ngx_read_glob(ngx_glob_t *gl, ngx_str_t *name);
void ngx_close_glob(ngx_glob_t *gl);


ngx_err_t ngx_trylock_fd(ngx_fd_t fd);
ngx_err_t ngx_lock_fd(ngx_fd_t fd);
ngx_err_t ngx_unlock_fd(ngx_fd_t fd);

#define ngx_trylock_fd_n         "fcntl(F_SETLK, F_WRLCK)"
#define ngx_lock_fd_n            "fcntl(F_SETLKW, F_WRLCK)"
#define ngx_unlock_fd_n          "fcntl(F_SETLK, F_UNLCK)"


#if (NGX_HAVE_F_READAHEAD)

#define NGX_HAVE_READ_AHEAD      1

#define ngx_read_ahead(fd, n)    fcntl(fd, F_READAHEAD, (int) n)
#define ngx_read_ahead_n         "fcntl(fd, F_READAHEAD)"

#elif (NGX_HAVE_POSIX_FADVISE)

#define NGX_HAVE_READ_AHEAD      1

ngx_int_t ngx_read_ahead(ngx_fd_t fd, size_t n);
#define ngx_read_ahead_n         "posix_fadvise(POSIX_FADV_SEQUENTIAL)"

#else

#define ngx_read_ahead(fd, n)    0
#define ngx_read_ahead_n         "ngx_read_ahead_n"

#endif


#if (NGX_HAVE_O_DIRECT)

ngx_int_t ngx_directio_on(ngx_fd_t fd);
#define ngx_directio_on_n        "fcntl(O_DIRECT)"

ngx_int_t ngx_directio_off(ngx_fd_t fd);
#define ngx_directio_off_n       "fcntl(!O_DIRECT)"

#elif (NGX_HAVE_F_NOCACHE)

#define ngx_directio_on(fd)      fcntl(fd, F_NOCACHE, 1)
#define ngx_directio_on_n        "fcntl(F_NOCACHE, 1)"

#elif (NGX_HAVE_DIRECTIO)

#define ngx_directio_on(fd)      directio(fd, DIRECTIO_ON)
#define ngx_directio_on_n        "directio(DIRECTIO_ON)"

#else

#define ngx_directio_on(fd)      0
#define ngx_directio_on_n        "ngx_directio_on_n"

#endif

size_t ngx_fs_bsize(u_char *name);


#if (NGX_HAVE_OPENAT)

#define ngx_openat_file(fd, name, mode, create, access)                      \
    openat(fd, (const char *) name, mode|create, access)

#define ngx_openat_file_n        "openat()"

#define ngx_file_at_info(fd, name, sb, flag)                                 \
    fstatat(fd, (const char *) name, sb, flag)

#define ngx_file_at_info_n       "fstatat()"

#define NGX_AT_FDCWD             (ngx_fd_t) AT_FDCWD

#endif


#define ngx_stdout               STDOUT_FILENO
#define ngx_stderr               STDERR_FILENO
#define ngx_set_stderr(fd)       dup2(fd, STDERR_FILENO)
#define ngx_set_stderr_n         "dup2(STDERR_FILENO)"


#if (NGX_HAVE_FILE_AIO)

ngx_int_t ngx_file_aio_init(ngx_file_t *file, ngx_pool_t *pool);
ssize_t ngx_file_aio_read(ngx_file_t *file, u_char *buf, size_t size,
    off_t offset, ngx_pool_t *pool);

extern ngx_uint_t  ngx_file_aio;

#endif

#if (NGX_THREADS)
ssize_t ngx_thread_read(ngx_file_t *file, u_char *buf, size_t size,
    off_t offset, ngx_pool_t *pool);
ssize_t ngx_thread_write_chain_to_file(ngx_file_t *file, ngx_chain_t *cl,
    off_t offset, ngx_pool_t *pool);
#endif


#endif /* _NGX_FILES_H_INCLUDED_ */
{% endhighlight %}

下面我们分成几个部分来详细解释一下ngx_files.h头文件

### 1.1 相关数据结构声明

**(1) 基本数据类型**
{% highlight string %}
typedef int                      ngx_fd_t;
typedef struct stat              ngx_file_info_t;
typedef ino_t                    ngx_file_uniq_t;
{% endhighlight %}
其中```struct stat```包含了文件当前状态的详细信息，该结构体字段一般类似于如下：
<pre>
struct stat {
   dev_t     st_dev;     /* ID of device containing file */
   ino_t     st_ino;     /* inode number */
   mode_t    st_mode;    /* protection */
   nlink_t   st_nlink;   /* number of hard links */
   uid_t     st_uid;     /* user ID of owner */
   gid_t     st_gid;     /* group ID of owner */
   dev_t     st_rdev;    /* device ID (if special file) */
   off_t     st_size;    /* total size, in bytes */
   blksize_t st_blksize; /* blocksize for file system I/O */
   blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
   time_t    st_atime;   /* time of last access */
   time_t    st_mtime;   /* time of last modification */
   time_t    st_ctime;   /* time of last status change */
};
</pre>

```ino_t```表示为当前inode类型。


**(2) ngx_file_mapping_t数据类型**
{% highlight string %}
typedef struct {
    u_char                      *name;
    size_t                       size;
    void                        *addr;
    ngx_fd_t                     fd;
    ngx_log_t                   *log;
} ngx_file_mapping_t;
{% endhighlight %}
用于将文件映射到内存的一个数据结构。

**(3) ngx_dir_t数据类型**
{% highlight string %}
typedef struct {
    DIR                         *dir;
    struct dirent               *de;
    struct stat                  info;

    unsigned                     type:8;
    unsigned                     valid_info:1;
} ngx_dir_t;
{% endhighlight %}
其中```DIR *dir```一般表示一个打开的目录句柄：
<pre>
DIR *opendir(const char *name);
</pre>

```struct dirent *de```主要用于保存一个打开的目录的详细信息，结构一般如下：
<pre>
struct dirent {
   ino_t          d_ino;       /* inode number */
   off_t          d_off;       /* not an offset; see NOTES */
   unsigned short d_reclen;    /* length of this record */
   unsigned char  d_type;      /* type of file; not supported
                                  by all file system types */
   char           d_name[256]; /* filename */
};
</pre>

```unsigned type:8```字段一般表示文件的类型。一般等于```de->d_type```值。

**(4) ngx_glob_t数据类型**

{% highlight string %}
typedef struct {
    size_t                       n;
    glob_t                       pglob;
    u_char                      *pattern;
    ngx_log_t                   *log;
    ngx_uint_t                   test;
} ngx_glob_t;
{% endhighlight %}
```glob_t pglob```一般用于基于模式的文件查找，其结构一般如下：
<pre>
int glob(const char *pattern, int flags,
        int (*errfunc) (const char *epath, int eerrno),
        glob_t *pglob);
void globfree(glob_t *pglob);

typedef struct {
   size_t   gl_pathc;    /* Count of paths matched so far  */
   char   **gl_pathv;    /* List of matched pathnames.  */
   size_t   gl_offs;     /* Slots to reserve in gl_pathv.  */
} glob_t;
</pre>

**(5) 相关错误码**
<pre>
#define NGX_INVALID_FILE         -1
#define NGX_FILE_ERROR           -1
</pre>

<br />

### 1.2 open()函数
主要是定义open函数原型及标志：
{% highlight string %}
#ifdef __CYGWIN__

#ifndef NGX_HAVE_CASELESS_FILESYSTEM
#define NGX_HAVE_CASELESS_FILESYSTEM  1
#endif

#define ngx_open_file(name, mode, create, access)                            \
    open((const char *) name, mode|create|O_BINARY, access)

#else

#define ngx_open_file(name, mode, create, access)                            \
    open((const char *) name, mode|create, access)

#endif

#define ngx_open_file_n          "open()"

#define NGX_FILE_RDONLY          O_RDONLY
#define NGX_FILE_WRONLY          O_WRONLY
#define NGX_FILE_RDWR            O_RDWR
#define NGX_FILE_CREATE_OR_OPEN  O_CREAT
#define NGX_FILE_OPEN            0
#define NGX_FILE_TRUNCATE        (O_CREAT|O_TRUNC)
#define NGX_FILE_APPEND          (O_WRONLY|O_APPEND)
#define NGX_FILE_NONBLOCK        O_NONBLOCK

#if (NGX_HAVE_OPENAT)
#define NGX_FILE_NOFOLLOW        O_NOFOLLOW

#if defined(O_DIRECTORY)
#define NGX_FILE_DIRECTORY       O_DIRECTORY
#else
#define NGX_FILE_DIRECTORY       0
#endif

#if defined(O_SEARCH)
#define NGX_FILE_SEARCH          (O_SEARCH|NGX_FILE_DIRECTORY)

#elif defined(O_EXEC)
#define NGX_FILE_SEARCH          (O_EXEC|NGX_FILE_DIRECTORY)

#elif (NGX_HAVE_O_PATH)
#define NGX_FILE_SEARCH          (O_PATH|O_RDONLY|NGX_FILE_DIRECTORY)

#else
#define NGX_FILE_SEARCH          (O_RDONLY|NGX_FILE_DIRECTORY)
#endif

#endif /* NGX_HAVE_OPENAT */

#define NGX_FILE_DEFAULT_ACCESS  0644
#define NGX_FILE_OWNER_ACCESS    0600
{% endhighlight %}

```open()```函数部分主要包括：函数的定义、相应的打开创建模式、文件创建时候的权限。当前在 ngx_auto_config.h头文件中，具有如下定义：
<pre>
#ifndef NGX_HAVE_OPENAT
#define NGX_HAVE_OPENAT  1
#endif
</pre>
对应我们当前的操作系统环境(32位Ubuntu16.04)，我们进行如下测试(test.c):
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>


int main(int argc,char *argv[])
{
    #if defined(O_DIRECTORY)
        printf("have O_DIRECTORY flags\n");
    #endif

    #if defined(O_SEARCH)
        printf("have O_SEARCH flags\n");
    #endif

    #if defined(O_EXEC)
        printf("have o_EXEC flags\n");
    #endif
    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test
have O_DIRECTORY flags
</pre>

<br />

### 1.3 关闭文件句柄
{% highlight string %}
#define ngx_close_file           close
#define ngx_close_file_n         "close()"
{% endhighlight %}

### 1.4 删除文件
{% highlight string %}
#define ngx_delete_file(name)    unlink((const char *) name)
#define ngx_delete_file_n        "unlink()"
{% endhighlight %}
```unlink```删除目录项，并将由name所引用的文件的链接数减1。假如该名字是所引用文件的最后一个链接，且没有进程打开了该文件，则该文件会被删除并将所占用的空间释放；假如该名字是所引用文件的最后一个链接，但是仍有其他进程打开了该文件，则等到所有进程关闭文件句柄之后该文件就会被删除；假若该名字引用的是一个软链接的话，则该软链接会被删除。看如下示例test.c:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc,char *argv[])
{
    int fd1 = -1;
    struct stat finfo;
    int ret = -1;

    fd1 = open("hello.txt",O_RDONLY);

    if(fd1 < 0)
       goto END;

    if(stat("hello.txt",&finfo) < 0)
       goto END;
    printf("1) st_nlink: %d\n",(intptr_t)finfo.st_nlink);



    if(link("hello.txt","new.txt") < 0)
       goto END;
    if(stat("new.txt",&finfo) < 0)
       goto END;    
    printf("2) st_nlink: %d\n",(intptr_t)finfo.st_nlink);


    unlink("new.txt");
    if(stat("hello.txt",&finfo) < 0)
       goto END;
    printf("3) st_nlink: %d\n",(intptr_t)finfo.st_nlink);


    unlink("hello.txt");
    if(stat("hello.txt",&finfo) < 0)
       goto END;
    printf("4) st_nlink: %d\n",(intptr_t)finfo.st_nlink);


    unlink("hello.txt");
    if(stat("hello.txt",&finfo) < 0)  
       goto END;
    printf("5) st_nlink: %d\n",(intptr_t)finfo.st_nlink);
   

    ret = 0;


END:
    close(fd1);
    return ret;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# echo "hello,world" > hello.txt
[root@localhost test-src]# ./test
1) st_nlink: 1
2) st_nlink: 2
3) st_nlink: 1
[root@localhost test-src]# ls 
test  test.c
</pre>
这里注意与```remove()```函数的区别：remove函数用于从文件系统中删除一个文件。当该文件是一个普通文件时，其调用unlink()函数来删除；当该文件是一个目录文件时，其通过调用rmdir()函数来删除。

<br />

### 1.5 读写文件
这一部分代码如下：
{% highlight string %}
ngx_fd_t ngx_open_tempfile(u_char *name, ngx_uint_t persistent,
    ngx_uint_t access);
#define ngx_open_tempfile_n      "open()"


ssize_t ngx_read_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset);
#if (NGX_HAVE_PREAD)
#define ngx_read_file_n          "pread()"
#else
#define ngx_read_file_n          "read()"
#endif

ssize_t ngx_write_file(ngx_file_t *file, u_char *buf, size_t size,
    off_t offset);

ssize_t ngx_write_chain_to_file(ngx_file_t *file, ngx_chain_t *ce,
    off_t offset, ngx_pool_t *pool);


#define ngx_read_fd              read
#define ngx_read_fd_n            "read()"

/*
 * we use inlined function instead of simple #define
 * because glibc 2.3 sets warn_unused_result attribute for write()
 * and in this case gcc 4.3 ignores (void) cast
 */
static ngx_inline ssize_t
ngx_write_fd(ngx_fd_t fd, void *buf, size_t n)
{
    return write(fd, buf, n);
}

#define ngx_write_fd_n           "write()"


#define ngx_write_console        ngx_write_fd


#define ngx_linefeed(p)          *p++ = LF;
#define NGX_LINEFEED_SIZE        1
#define NGX_LINEFEED             "\x0a"
{% endhighlight %}

**(1) 创建临时文件**
<pre>
ngx_fd_t ngx_open_tempfile(u_char *name, ngx_uint_t persistent,
    ngx_uint_t access);
</pre>

**(2) 读文件**
<pre>
ssize_t ngx_read_file(ngx_file_t *file, u_char *buf, size_t size, off_t offset);
</pre>
从```ngx_file_t *file```中读取数据到buf中。

**(3) 写文件**
<pre>
ssize_t ngx_write_file(ngx_file_t *file, u_char *buf, size_t size,
    off_t offset);
ssize_t ngx_write_chain_to_file(ngx_file_t *file, ngx_chain_t *ce,
    off_t offset, ngx_pool_t *pool);

/*
 * we use inlined function instead of simple #define
 * because glibc 2.3 sets warn_unused_result attribute for write()
 * and in this case gcc 4.3 ignores (void) cast
 */
static ngx_inline ssize_t
ngx_write_fd(ngx_fd_t fd, void *buf, size_t n)
{
    return write(fd, buf, n);
}

#define ngx_write_console        ngx_write_fd
</pre>
前面两个函数只是申明了相应的原型。第3个函数主要是由于编译器在对函数返回值的处理方面的原因，导致并未使用如下写法：
<pre>
#define ngx_write_fd(fd,buf,n) write(fd,buf,n)
</pre>

**(4) 相关换行符的定义**
<pre>
#define ngx_linefeed(p)          *p++ = LF;
#define NGX_LINEFEED_SIZE        1
#define NGX_LINEFEED             "\x0a"
</pre>
其中```LF```在src/core/ngx_core.h头文件中被定义为：
<pre>
#define LF     (u_char) '\n'
#define CR     (u_char) '\r'
#define CRLF   "\r\n"
</pre>

<br />

### 1.6 文件属性操作
{% highlight string %}
#define ngx_rename_file(o, n)    rename((const char *) o, (const char *) n)
#define ngx_rename_file_n        "rename()"


#define ngx_change_file_access(n, a) chmod((const char *) n, a)
#define ngx_change_file_access_n "chmod()"


ngx_int_t ngx_set_file_time(u_char *name, ngx_fd_t fd, time_t s);
#define ngx_set_file_time_n      "utimes()"


#define ngx_file_info(file, sb)  stat((const char *) file, sb)
#define ngx_file_info_n          "stat()"

#define ngx_fd_info(fd, sb)      fstat(fd, sb)
#define ngx_fd_info_n            "fstat()"

#define ngx_link_info(file, sb)  lstat((const char *) file, sb)
#define ngx_link_info_n          "lstat()"

#define ngx_is_dir(sb)           (S_ISDIR((sb)->st_mode))
#define ngx_is_file(sb)          (S_ISREG((sb)->st_mode))
#define ngx_is_link(sb)          (S_ISLNK((sb)->st_mode))
#define ngx_is_exec(sb)          (((sb)->st_mode & S_IXUSR) == S_IXUSR)
#define ngx_file_access(sb)      ((sb)->st_mode & 0777)
#define ngx_file_size(sb)        (sb)->st_size
#define ngx_file_fs_size(sb)     ngx_max((sb)->st_size, (sb)->st_blocks * 512)
#define ngx_file_mtime(sb)       (sb)->st_mtime
#define ngx_file_uniq(sb)        (sb)->st_ino
{% endhighlight %}
主要包括以下几个部分：

* 重命名文件

* 改变文件访问属性

* 设置文件的```访问时间```（access time) 与```修改时间```(modification time)

* 获得文件相关信息(stat,fstat,lstat)

<br />

## 1.7 文件内存映射
{% highlight string %}
ngx_int_t ngx_create_file_mapping(ngx_file_mapping_t *fm);
void ngx_close_file_mapping(ngx_file_mapping_t *fm);


#define ngx_realpath(p, r)       (u_char *) realpath((char *) p, (char *) r)
#define ngx_realpath_n           "realpath()"
#define ngx_getcwd(buf, size)    (getcwd((char *) buf, size) != NULL)
#define ngx_getcwd_n             "getcwd()"
#define ngx_path_separator(c)    ((c) == '/')
{% endhighlight %}
```realpath()```获得一个规范的绝对路径值；```getcwd()```返回调用进程当前工作目录的绝对路径。

<br />

## 1.8 目录操作
{% highlight string %}
#if defined(PATH_MAX)

#define NGX_HAVE_MAX_PATH        1
#define NGX_MAX_PATH             PATH_MAX

#else

#define NGX_MAX_PATH             4096

#endif


#define NGX_DIR_MASK_LEN         0


ngx_int_t ngx_open_dir(ngx_str_t *name, ngx_dir_t *dir);
#define ngx_open_dir_n           "opendir()"


#define ngx_close_dir(d)         closedir((d)->dir)
#define ngx_close_dir_n          "closedir()"


ngx_int_t ngx_read_dir(ngx_dir_t *dir);
#define ngx_read_dir_n           "readdir()"


#define ngx_create_dir(name, access) mkdir((const char *) name, access)
#define ngx_create_dir_n         "mkdir()"


#define ngx_delete_dir(name)     rmdir((const char *) name)
#define ngx_delete_dir_n         "rmdir()"


#define ngx_dir_access(a)        (a | (a & 0444) >> 2)


#define ngx_de_name(dir)         ((u_char *) (dir)->de->d_name)
#if (NGX_HAVE_D_NAMLEN)
#define ngx_de_namelen(dir)      (dir)->de->d_namlen
#else
#define ngx_de_namelen(dir)      ngx_strlen((dir)->de->d_name)
#endif

static ngx_inline ngx_int_t
ngx_de_info(u_char *name, ngx_dir_t *dir)
{
    dir->type = 0;
    return stat((const char *) name, &dir->info);
}

#define ngx_de_info_n            "stat()"
#define ngx_de_link_info(name, dir)  lstat((const char *) name, &(dir)->info)
#define ngx_de_link_info_n       "lstat()"

#if (NGX_HAVE_D_TYPE)

/*
 * some file systems (e.g. XFS on Linux and CD9660 on FreeBSD)
 * do not set dirent.d_type
 */

#define ngx_de_is_dir(dir)                                                   \
    (((dir)->type) ? ((dir)->type == DT_DIR) : (S_ISDIR((dir)->info.st_mode)))
#define ngx_de_is_file(dir)                                                  \
    (((dir)->type) ? ((dir)->type == DT_REG) : (S_ISREG((dir)->info.st_mode)))
#define ngx_de_is_link(dir)                                                  \
    (((dir)->type) ? ((dir)->type == DT_LNK) : (S_ISLNK((dir)->info.st_mode)))

#else

#define ngx_de_is_dir(dir)       (S_ISDIR((dir)->info.st_mode))
#define ngx_de_is_file(dir)      (S_ISREG((dir)->info.st_mode))
#define ngx_de_is_link(dir)      (S_ISLNK((dir)->info.st_mode))

#endif

#define ngx_de_access(dir)       (((dir)->info.st_mode) & 0777)
#define ngx_de_size(dir)         (dir)->info.st_size
#define ngx_de_fs_size(dir)                                                  \
    ngx_max((dir)->info.st_size, (dir)->info.st_blocks * 512)
#define ngx_de_mtime(dir)        (dir)->info.st_mtime
{% endhighlight %}

对于```PATH_MAX```,我们在当前环境采用如下程序测试：
{% highlight string %}
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

int main(int argc,char *argv[])
{
    #if defined(PATH_MAX)
       printf("PATH_MAX: %d\n",PATH_MAX);
    #endif
   
    return 0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ./test
PATH_MAX: 4096
</pre>
可以看到，```PATH_MAX```在我们当前环境中被定义，且默认值为4096.

```ngx_dir_access()```: 对于目录来说，执行位的作用是控制能否进入或者通过该目录，而不是控制能否列出它的内容。读取位和执行位的组合的作用才是控制是否列出目录中的内容。写入位和执行位的组合则是允许在目录中创建，删除，和重命名文件。

```NGX_HAVE_D_NAMLEN```在我们当前环境并未定义。

而对于```NGX_HAVE_D_TYPE```我们在ngx_auto_config.h头文件中具有如下定义(另请参看： man 3 readdir)：
<pre>
#ifndef NGX_HAVE_D_TYPE
#define NGX_HAVE_D_TYPE  1
#endif
</pre>

### 1.9 文件查找
{% highlight string %}
ngx_int_t ngx_open_glob(ngx_glob_t *gl);
#define ngx_open_glob_n          "glob()"
ngx_int_t ngx_read_glob(ngx_glob_t *gl, ngx_str_t *name);
void ngx_close_glob(ngx_glob_t *gl);
{% endhighlight %}

这里给出一个glob使用的示例(test.c):
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <glob.h>

int main(int argc,char *argv)
{
   glob_t globbuf;

   globbuf.gl_offs = 2;
   glob("*.c", GLOB_DOOFFS, NULL, &globbuf);
   glob("../*.c", GLOB_DOOFFS | GLOB_APPEND, NULL, &globbuf);
   globbuf.gl_pathv[0] = "ls";
   globbuf.gl_pathv[1] = "-l";
   execvp("ls", &globbuf.gl_pathv[0]);
  
   return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c
[root@localhost test-src]# ls 
test  test1.c  test2.c  test3.c  test4.c  test.c
[root@localhost test-src]# ./test
-rw-r--r--. 1 root root   0 Dec  1 18:45 test1.c
-rw-r--r--. 1 root root   0 Dec  1 18:45 test2.c
-rw-r--r--. 1 root root   0 Dec  1 18:45 test3.c
-rw-r--r--. 1 root root   0 Dec  1 18:45 test4.c
-rw-r--r--. 1 root root 405 Dec  1 18:47 test.c
</pre>


### 1.10 文件读写锁
{% highlight string %}
ngx_err_t ngx_trylock_fd(ngx_fd_t fd);
ngx_err_t ngx_lock_fd(ngx_fd_t fd);
ngx_err_t ngx_unlock_fd(ngx_fd_t fd);

#define ngx_trylock_fd_n         "fcntl(F_SETLK, F_WRLCK)"
#define ngx_lock_fd_n            "fcntl(F_SETLKW, F_WRLCK)"
#define ngx_unlock_fd_n          "fcntl(F_SETLK, F_UNLCK)"
{% endhighlight %}
通过上述函数设置文件的读写锁，请参看：[fcntl函数说明 F_SETLK/F_SETLKW例子](http://blog.csdn.net/wangyin159/article/details/48467315)


### 1.11 文件预先读取
{% highlight string %}
#if (NGX_HAVE_F_READAHEAD)

#define NGX_HAVE_READ_AHEAD      1

#define ngx_read_ahead(fd, n)    fcntl(fd, F_READAHEAD, (int) n)
#define ngx_read_ahead_n         "fcntl(fd, F_READAHEAD)"

#elif (NGX_HAVE_POSIX_FADVISE)

#define NGX_HAVE_READ_AHEAD      1

ngx_int_t ngx_read_ahead(ngx_fd_t fd, size_t n);
#define ngx_read_ahead_n         "posix_fadvise(POSIX_FADV_SEQUENTIAL)"

#else

#define ngx_read_ahead(fd, n)    0
#define ngx_read_ahead_n         "ngx_read_ahead_n"

#endif
{% endhighlight %}

在ngx_auto_config.h头文件中，我们有如下定义：
<pre>
#ifndef NGX_HAVE_POSIX_FADVISE
#define NGX_HAVE_POSIX_FADVISE  1
#endif
</pre>

这里主要是通过操作系统内核的支持加快对文件的访问，属于系统优化的部分。
<pre>
Programs  can  use posix_fadvise() to announce an intention to access file data in a specific pattern in the
future, thus allowing the kernel to perform appropriate optimizations.
</pre>
更多请参看```man posix_fadvise```。




### 1.12 direct io支持
{% highlight string %}
#if (NGX_HAVE_O_DIRECT)

ngx_int_t ngx_directio_on(ngx_fd_t fd);
#define ngx_directio_on_n        "fcntl(O_DIRECT)"

ngx_int_t ngx_directio_off(ngx_fd_t fd);
#define ngx_directio_off_n       "fcntl(!O_DIRECT)"

#elif (NGX_HAVE_F_NOCACHE)

#define ngx_directio_on(fd)      fcntl(fd, F_NOCACHE, 1)
#define ngx_directio_on_n        "fcntl(F_NOCACHE, 1)"

#elif (NGX_HAVE_DIRECTIO)

#define ngx_directio_on(fd)      directio(fd, DIRECTIO_ON)
#define ngx_directio_on_n        "directio(DIRECTIO_ON)"

#else

#define ngx_directio_on(fd)      0
#define ngx_directio_on_n        "ngx_directio_on_n"

#endif
{% endhighlight %}

在ngx_auto_config.h头文件中，我们有如下定义：
<pre>
#ifndef NGX_HAVE_O_DIRECT
#define NGX_HAVE_O_DIRECT  1
#endif
</pre>
主要是为了建议操作系统尽快将对文件的写操作刷新到硬盘。请参看```man 2 open```:
<pre>
O_DIRECT (Since Linux 2.4.10)
  Try to minimize cache effects of the I/O to and from this file.  In general this will degrade performance, but it 
  is useful in special situations,  such as  when  applications  do  their  own  caching.  File I/O is done directly
  to/from user-space buffers.  The O_DIRECT flag on its own makes an effort to transfer data synchronously, but does
  not give the guarantees of the O_SYNC flag that data and necessary metadata are transferred. To  guarantee synchronous
  I/O, O_SYNC must be used in addition to O_DIRECT.  See NOTES below for further discussion.
</pre>





### 1.13 open at支持
{% highlight string %}
size_t ngx_fs_bsize(u_char *name);


#if (NGX_HAVE_OPENAT)

#define ngx_openat_file(fd, name, mode, create, access)                      \
    openat(fd, (const char *) name, mode|create, access)

#define ngx_openat_file_n        "openat()"

#define ngx_file_at_info(fd, name, sb, flag)                                 \
    fstatat(fd, (const char *) name, sb, flag)

#define ngx_file_at_info_n       "fstatat()"

#define NGX_AT_FDCWD             (ngx_fd_t) AT_FDCWD

#endif


#define ngx_stdout               STDOUT_FILENO
#define ngx_stderr               STDERR_FILENO
#define ngx_set_stderr(fd)       dup2(fd, STDERR_FILENO)
#define ngx_set_stderr_n         "dup2(STDERR_FILENO)"
{% endhighlight %}

```ngx_fs_bsize()```用于获取文件系统块大小，我们稍后会进行讲解；在ngx_auto_config.h头文件中我们有如下定义：
<pre>
#ifndef NGX_HAVE_OPENAT
#define NGX_HAVE_OPENAT  1
#endif
</pre>
```openat()```函数与open()函数类似，只不过是在相对路径处理方面有些许不同：
<pre>
If  the pathname given in pathname is relative, then it is interpreted relative to the directory referred to by the file 
descriptor dirfd (rather than relative to the current working directory of the calling process, as is done by open(2) for
a relative pathname).

If pathname is relative and dirfd is the special value AT_FDCWD, then pathname is interpreted relative to the current working
directory of the calling  process(like open(2)).

If pathname is absolute, then dirfd is ignored.
</pre>




### 1.14 aio及多线程读写支持
{% highlight string %}
#if (NGX_HAVE_FILE_AIO)

ngx_int_t ngx_file_aio_init(ngx_file_t *file, ngx_pool_t *pool);
ssize_t ngx_file_aio_read(ngx_file_t *file, u_char *buf, size_t size,
    off_t offset, ngx_pool_t *pool);

extern ngx_uint_t  ngx_file_aio;

#endif

#if (NGX_THREADS)
ssize_t ngx_thread_read(ngx_file_t *file, u_char *buf, size_t size,
    off_t offset, ngx_pool_t *pool);
ssize_t ngx_thread_write_chain_to_file(ngx_file_t *file, ngx_chain_t *cl,
    off_t offset, ngx_pool_t *pool);
#endif
{% endhighlight %}
当前我们并不支持```NGX_HAVE_FILE_AIO```及```NGX_THREAD```。


<br />
<br />

**[参看]:**

1. [吕涛博客](https://www.lvtao.net/tag/nginx/5/)







<br />
<br />
<br />

