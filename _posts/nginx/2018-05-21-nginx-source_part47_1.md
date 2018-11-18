---
layout: post
title: core/ngx_open_file_cache.h头文件分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本节主要讲述一下nginx对静态文件的缓存相关操作。


<!-- more -->


## 1. directio_off宏定义
{% highlight string %}

/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef _NGX_OPEN_FILE_CACHE_H_INCLUDED_
#define _NGX_OPEN_FILE_CACHE_H_INCLUDED_


#define NGX_OPEN_FILE_DIRECTIO_OFF  NGX_MAX_OFF_T_VALUE

{% endhighlight %}

在objs/ngx_auto_config.h头文件中，有如下定义：
<pre>
#ifndef NGX_MAX_OFF_T_VALUE
#define NGX_MAX_OFF_T_VALUE  9223372036854775807LL
#endif
</pre>
这里定义```NGX_OPEN_FILE_DIRECTIO_OFF```为long long的最大值，一般文件大小都不会超过此值，因此可以将此值作为```directio_off```来用。




## 2. ngx_open_file_info_t数据结构
{% highlight string %}
typedef struct {
    ngx_fd_t                 fd;
    ngx_file_uniq_t          uniq;
    time_t                   mtime;
    off_t                    size;
    off_t                    fs_size;
    off_t                    directio;
    size_t                   read_ahead;

    ngx_err_t                err;
    char                    *failed;

    time_t                   valid;

    ngx_uint_t               min_uses;

#if (NGX_HAVE_OPENAT)
    size_t                   disable_symlinks_from;
    unsigned                 disable_symlinks:2;
#endif

    unsigned                 test_dir:1;
    unsigned                 test_only:1;
    unsigned                 log:1;
    unsigned                 errors:1;
    unsigned                 events:1;

    unsigned                 is_dir:1;
    unsigned                 is_file:1;
    unsigned                 is_link:1;
    unsigned                 is_exec:1;
    unsigned                 is_directio:1;
} ngx_open_file_info_t;
{% endhighlight %}

```ngx_open_file_info_t```用于nginx中描述一个打开的缓存文件， 下面我们简要分析一下各字段的含义：

* ```fd```: 该文件所对应的文件句柄

* ```uniq```: 该文件所对应的全局唯一标识， 一般取值为inode节点号

* ```mtime```: 该文件的最后修改时间

* ```size```: 该文件的大小

* ```fs_size```: 该文件所占用的硬盘层面的块数block * 512

这里关于size与fs_size的区别，简单说明一下。 首先执行如下命令：
{% highlight string %}
# man 2 stat

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
The st_size field gives the size of the file (if it is a regular file or a symbolic link) in bytes.  The size of a symbolic
link is the length of the  pathname it contains, without a terminating null byte.

The st_blocks field indicates the number of blocks allocated to the file, 512-byte units.  (This may be smaller than st_size/512 
when the file has holes.)
{% endhighlight %}
这里求fs_size一般采用如下宏：
<pre>
#define ngx_file_fs_size(sb)     ngx_max((sb)->st_size, (sb)->st_blocks * 512)
</pre>
一般在文件有```hole```的情况下， 文件的```size```大于```fs_size```， 否则```fs_size```小于等于```size```。


* ```directio```: 文件对应的directio大小值， 小于该值采用sendfile来发送， 大于该值采用aio来发送, 这样可以防止进程被阻塞。

* ```read_ahead```: 预先由内核读取的字节数。关于read_ahead， 有如下一段说明：
<pre>
Syntax:	read_ahead size;
Default:	read_ahead 0;
Context:	http, server, location
Sets the amount of pre-reading for the kernel when working with file.

On Linux, the posix_fadvise(0, 0, 0, POSIX_FADV_SEQUENTIAL) system call is used, and so the size parameter is ignored.

On FreeBSD, the fcntl(O_READAHEAD, size) system call, supported since FreeBSD 9.0-CURRENT, is used. FreeBSD 7 has to be patched.
</pre>

* ```err```: 该打开的文件所关联的错误信息码

* ```failed```: 所关联的错误信息

* ```valid```: 文件的有效时间

* ```min_uses```: 本字段牵扯到```open_file_cache```指令，指令中inactive参数时间内文件的最少访问次数，如果超过这个数字，该打开的文件描述符将会一直

关于```disable_symlinks_from```与```disable_symlinks```字段，主要用于决定当打开文件的时候如何处理符号链接(symbolic links)。其中前一个字段用于指定从哪一个字段开始检查符号链接， 而后一个字段用于指定是否禁止符号链接。在```objs/ngx_auto_config.h```头文件中有如下定义：
<pre>
#ifndef NGX_HAVE_OPENAT
#define NGX_HAVE_OPENAT  1
#endif
</pre>

* ```test_dir```: 有时可能在程序运行过程中，一个directory变成了一个file, 通过此字段控制是否需要再进行dir的检测

* ```test_only```: 只用于测试nginx中某一个特定的文件，以了解相关信息，并不会真正把其放入到nginx的静态文件cache中

* ```log```: 本字段当前主要用于指示是否只以```非阻塞只读```方式打开一个文件

* ```errors```: 主要用于指示当操作缓存文件出现错误时，如何处理。 当此字段为0， 则出现错误时返回失败； 当此字段为1， 则出现错误时```可能```会进行重建操作。本字段主要用于指示是否能够忍受errors

* ```events```: 主要用于指示是否为打开的文件绑定相应的事件。在有些操作系统上如删除文件等都会产生相应的事件

* ```is_dir```: 用于指示此文件是否是一个目录

* ```is_file```: 用于指示此文件是否是一个普通的文件

* ```is_link```: 用于指示此文件是否是一个链接文件

* ```is_exec```: 用于支持此文件是否是一个可执行文件

* ```is_directio```: 主要用于指示此文件是否已经开启了directio

## 2. ngx_cached_open_file_t数据结构 
{% highlight string %}
typedef struct ngx_cached_open_file_s  ngx_cached_open_file_t;

struct ngx_cached_open_file_s {
    ngx_rbtree_node_t        node;
    ngx_queue_t              queue;

    u_char                  *name;
    time_t                   created;
    time_t                   accessed;

    ngx_fd_t                 fd;
    ngx_file_uniq_t          uniq;
    time_t                   mtime;
    off_t                    size;
    ngx_err_t                err;

    uint32_t                 uses;

#if (NGX_HAVE_OPENAT)
    size_t                   disable_symlinks_from;
    unsigned                 disable_symlinks:2;
#endif

    unsigned                 count:24;
    unsigned                 close:1;
    unsigned                 use_event:1;

    unsigned                 is_dir:1;
    unsigned                 is_file:1;
    unsigned                 is_link:1;
    unsigned                 is_exec:1;
    unsigned                 is_directio:1;

    ngx_event_t             *event;
};
{% endhighlight %}

本数据结构用于表示一个需要被```cached```的打开文件。下面我们简要介绍一下该数据结构：

* ```node```: 用于表示该ngx_cached_open_file_s结构在nginx静态缓存红黑树的哪一个节点上

* ```queue```: 用于表示该ngx_cached_open_file_s结构所在的队列

* ```name```: 用于表示该缓冲的文件的名称

* ```created```: 用于指明该文件是最先是在什么时候被放入静态缓存的

* ```accessed```: 用于指示该缓存文件最近访问时间

* ```fd```: 该缓存文件的文件句柄

* ```uniq```: 该缓存文件的inode节点号，用作唯一标识

* ```mtime```: 对该缓存文件的最近更新时间

* ```uses```: 当前该缓存文件自从被添加到缓存红黑树之后被使用到的次数

关于```disable_symlinks_from```与```disable_symlinks```字段，主要用于决定当打开文件的时候如何处理符号链接(symbolic links)。其中前一个字段用于指定从哪一个字段开始检查符号链接， 而后一个字段用于指定是否禁止符号链接。在```objs/ngx_auto_config.h```头文件中有如下定义：
<pre>
#ifndef NGX_HAVE_OPENAT
#define NGX_HAVE_OPENAT  1
#endif
</pre>

* ```count```: 是文件的引用计数，表示现在文件被几个请求使用中

* ```close```: 用于指示是否需要真正被关闭

* ```use_event```: 指示是否使用event

* ```is_dir```: 用于指示此文件是否是一个目录

* ```is_file```: 用于指示此文件是否是一个普通的文件

* ```is_link```: 用于指示此文件是否是一个链接文件

* ```is_exec```: 用于支持此文件是否是一个可执行文件

* ```is_directio```: 主要用于指示此文件是否已经开启了directio

* ```event```: 该打开的缓存文件所关联的事件

注意：
<pre>
结构体ngx_open_file_info_t与ngx_cached_open_file_t的不同主要表现在：前者用于描述一个打开
的缓存文件；而后者用于描述一个需要打开的缓冲文件，因此其要存储文件名等相关信息。

</pre>


## 3. ngx_open_file_cache_t结构
{% highlight string %}
typedef struct {
    ngx_rbtree_t             rbtree;
    ngx_rbtree_node_t        sentinel;
    ngx_queue_t              expire_queue;

    ngx_uint_t               current;
    ngx_uint_t               max;
    time_t                   inactive;
} ngx_open_file_cache_t;
{% endhighlight %}

本结构用于缓存nginx中的静态文件。下面简要介绍一下本数据结构：

* ```rbtree```: 缓存文件红黑树的根，主要是为了方便查找

* ```sentinel```: 红黑树sentinel节点

* ```expire_queue```: 缓存文件过期队列

* ```current```: 当前缓存元素的个数

* ```max```: 最大的缓存个数

* ```inactive```: 缓存的过期时间

## 4. ngx_open_file_cache_cleanup_t结构
{% highlight string %}
typedef struct {
    ngx_open_file_cache_t   *cache;
    ngx_cached_open_file_t  *file;
    ngx_uint_t               min_uses;
    ngx_log_t               *log;
} ngx_open_file_cache_cleanup_t;
{% endhighlight %}
本数据结构指示当前要清除的缓存文件结构。下面我们简要介绍一下各字段的含义：

* ```cache```: 用于指示当前要cleanup的cache

* ```file```: 用于指示当前要cleanup的文件

* ```min_uses```: 低于此值则会cleanup

* ```log```: 所对应的日志

## 5. ngx_open_file_cache_event_t结构
{% highlight string %}
typedef struct {

    /* ngx_connection_t stub to allow use c->fd as event ident */
    void                    *data;
    ngx_event_t             *read;
    ngx_event_t             *write;
    ngx_fd_t                 fd;

    ngx_cached_open_file_t  *file;
    ngx_open_file_cache_t   *cache;
} ngx_open_file_cache_event_t;
{% endhighlight %}
本数据结构用于指示nginx静态缓存所对应的事件。下面简要介绍一下各字段的含义：

* ```data```: 可以是任何内容

* ```read```: 读事件

* ```write```: 写事件

* ```fd```: 所关联的文件句柄

* ```file```: 所关联的静态缓冲文件

* ```cache```: 所关联的缓存

## 6. 相关函数声明
{% highlight string %}
//1) 初始化一个nginx静态文件缓存
ngx_open_file_cache_t *ngx_open_file_cache_init(ngx_pool_t *pool,
    ngx_uint_t max, time_t inactive);


//2) 打开一个缓存文件
ngx_int_t ngx_open_cached_file(ngx_open_file_cache_t *cache, ngx_str_t *name,
    ngx_open_file_info_t *of, ngx_pool_t *pool);


#endif /* _NGX_OPEN_FILE_CACHE_H_INCLUDED_ */
{% endhighlight %}



<br />
<br />

**[参看]**

1. [第二章 OpenResty(nginx+lua)开发入门](http://jinnianshilongnian.iteye.com/blog/2186448)

2. [nginx open_file_cache指令影响静态文件更新时间](https://www.cnblogs.com/sunsweet/p/3338684.html)

3: [nginx对静态文件cache的处理分析](https://blog.csdn.net/weiyuefei/article/details/35782523)

4. [使用nginx的proxy_cache实现静态资源的缓存](http://www.jackieathome.net/archives/411.html)

5. [nginx静态资源缓存策略配置](https://blog.csdn.net/yu12377/article/details/77875045)

6. [Nginx 静态资源缓存设置](https://www.w3cschool.cn/nginxsysc/nginxsysc-cache.html)

<br />
<br />
<br />

