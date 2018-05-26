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

```ngx_open_file_info_t```用于nginx中描述一个打开的额缓存文件， 下面我们简要分析一下各字段的含义：

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


<br />
<br />

**[参看]**

1. [第二章 OpenResty(nginx+lua)开发入门](http://jinnianshilongnian.iteye.com/blog/2186448)

2. [nginx open_file_cache指令影响静态文件更新时间](https://www.cnblogs.com/sunsweet/p/3338684.html)

3: [nginx对静态文件cache的处理分析](https://blog.csdn.net/weiyuefei/article/details/35782523)

<br />
<br />
<br />

