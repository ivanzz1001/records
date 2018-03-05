---
layout: post
title: auto/os/conf脚本分析-part10
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---


本节我们介绍auto/os/conf脚本，其主要是处理与操作系统紧密相关的一些内容。所涉及到的脚本文件主要两个：

* auto/os/linux脚本
* auto/os/conf脚本

<!-- more -->

根据如下脚本：
{% highlight string %}
NGX_SYSTEM=`uname -s 2>/dev/null`
NGX_RELEASE=`uname -r 2>/dev/null`
NGX_MACHINE=`uname -m 2>/dev/null`

echo " + $NGX_SYSTEM $NGX_RELEASE $NGX_MACHINE"

NGX_PLATFORM="$NGX_SYSTEM:$NGX_RELEASE:$NGX_MACHINE";
{% endhighlight %}
得到当前我们Ubuntu16.04是属于Linux平台，因此我们这里介绍对应于Linux平台的auto/os/linux脚本。


## 1. auto/os/linux脚本

脚本内容如下：
{% highlight string %}


# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


have=NGX_LINUX . auto/have_headers

CORE_INCS="$UNIX_INCS"
CORE_DEPS="$UNIX_DEPS $LINUX_DEPS"
CORE_SRCS="$UNIX_SRCS $LINUX_SRCS"

ngx_spacer='
'

cc_aux_flags="$CC_AUX_FLAGS"
CC_AUX_FLAGS="$cc_aux_flags -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"


# Linux kernel version

version=$((`uname -r \
    | sed -n -e 's/^\([0-9][0-9]*\)\.\([0-9][0-9]*\)\.\([0-9][0-9]*\).*/ \
                                                 \1*256*256+\2*256+\3/p' \
             -e 's/^\([0-9][0-9]*\)\.\([0-9][0-9]*\).*/\1*256*256+\2*256/p'`))

version=${version:-0}


# posix_fadvise64() had been implemented in 2.5.60

if [ $version -lt 132412 ]; then
    have=NGX_HAVE_POSIX_FADVISE . auto/nohave
fi

# epoll, EPOLLET version

ngx_feature="epoll"
ngx_feature_name="NGX_HAVE_EPOLL"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/epoll.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int efd = 0;
                  struct epoll_event ee;
                  ee.events = EPOLLIN|EPOLLOUT|EPOLLET;
                  ee.data.ptr = NULL;
                  efd = epoll_create(100);
                  if (efd == -1) return 1;"
. auto/feature

if [ $ngx_found = yes ]; then
    have=NGX_HAVE_CLEAR_EVENT . auto/have
    CORE_SRCS="$CORE_SRCS $EPOLL_SRCS"
    EVENT_MODULES="$EVENT_MODULES $EPOLL_MODULE"
    EVENT_FOUND=YES


    # EPOLLRDHUP appeared in Linux 2.6.17, glibc 2.8

    ngx_feature="EPOLLRDHUP"
    ngx_feature_name="NGX_HAVE_EPOLLRDHUP"
    ngx_feature_run=no
    ngx_feature_incs="#include <sys/epoll.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="int efd = 0, fd = 0;
                      struct epoll_event ee;
                      ee.events = EPOLLIN|EPOLLRDHUP|EPOLLET;
                      ee.data.ptr = NULL;
                      epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ee)"
    . auto/feature
fi


# O_PATH and AT_EMPTY_PATH were introduced in 2.6.39, glibc 2.14

ngx_feature="O_PATH"
ngx_feature_name="NGX_HAVE_O_PATH"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/stat.h>
                  #include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int fd; struct stat sb;
                  fd = openat(AT_FDCWD, \".\", O_PATH|O_DIRECTORY|O_NOFOLLOW);
                  if (fstatat(fd, \"\", &sb, AT_EMPTY_PATH) != 0) return 1"
. auto/feature


# sendfile()

CC_AUX_FLAGS="$cc_aux_flags -D_GNU_SOURCE"
ngx_feature="sendfile()"
ngx_feature_name="NGX_HAVE_SENDFILE"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/sendfile.h>
                  #include <errno.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int s = 0, fd = 1;
                  ssize_t n; off_t off = 0;
                  n = sendfile(s, fd, &off, 1);
                  if (n == -1 && errno == ENOSYS) return 1"
. auto/feature

if [ $ngx_found = yes ]; then
    CORE_SRCS="$CORE_SRCS $LINUX_SENDFILE_SRCS"
fi


# sendfile64()

CC_AUX_FLAGS="$cc_aux_flags -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"
ngx_feature="sendfile64()"
ngx_feature_name="NGX_HAVE_SENDFILE64"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/sendfile.h>
                  #include <errno.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int s = 0, fd = 1;
                  ssize_t n; off_t off = 0;
                  n = sendfile(s, fd, &off, 1);
                  if (n == -1 && errno == ENOSYS) return 1"
. auto/feature


ngx_include="sys/prctl.h"; . auto/include

# prctl(PR_SET_DUMPABLE)

ngx_feature="prctl(PR_SET_DUMPABLE)"
ngx_feature_name="NGX_HAVE_PR_SET_DUMPABLE"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/prctl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) == -1) return 1"
. auto/feature


# sched_setaffinity()

ngx_feature="sched_setaffinity()"
ngx_feature_name="NGX_HAVE_SCHED_SETAFFINITY"
ngx_feature_run=no
ngx_feature_incs="#include <sched.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="cpu_set_t mask;
                  CPU_ZERO(&mask);
                  sched_setaffinity(0, sizeof(cpu_set_t), &mask)"
. auto/feature


# crypt_r()

ngx_feature="crypt_r()"
ngx_feature_name="NGX_HAVE_GNU_CRYPT_R"
ngx_feature_run=no
ngx_feature_incs="#include <crypt.h>"
ngx_feature_path=
ngx_feature_libs=-lcrypt
ngx_feature_test="struct crypt_data  cd;
                  crypt_r(\"key\", \"salt\", &cd);"
. auto/feature


ngx_include="sys/vfs.h";     . auto/include


CC_AUX_FLAGS="$cc_aux_flags -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"
{% endhighlight %}


### 1.1 向头文件中定义NGX_LINUX宏

执行如下脚本向objs/ngx_auto_headers.h头文件中定义```NGX_LINUX```宏：
{% highlight string %}
have=NGX_LINUX . auto/have_headers
{% endhighlight %}

### 1.2 指定所包含的头文件、源文件、依赖文件
{% highlight string %}
CORE_INCS="$UNIX_INCS"
CORE_DEPS="$UNIX_DEPS $LINUX_DEPS"
CORE_SRCS="$UNIX_SRCS $LINUX_SRCS"
{% endhighlight %}

在我们前面介绍的auto/sources脚本中，我们分别已经设置了```$UNIX_INCS```，```$UNIX_DEPS```，```$LINUX_DEPS```,```$UNIX_SRCS```以及```$LINUX_SRCS```。

### 1.3 设置空格符、编译辅助参数
{% highlight string %}
ngx_spacer='
'

cc_aux_flags="$CC_AUX_FLAGS"
CC_AUX_FLAGS="$cc_aux_flags -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"
{% endhighlight %}

```cc_aux_flags```当前为空(并没有在auto/cc/gcc脚本中被设置)。

```CC_AUX_FLAGS```的值当前为```-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64```

### 1.4 计算kernel版本号
{% highlight string %}
# Linux kernel version

version=$((`uname -r \
    | sed -n -e 's/^\([0-9][0-9]*\)\.\([0-9][0-9]*\)\.\([0-9][0-9]*\).*/ \
                                                 \1*256*256+\2*256+\3/p' \
             -e 's/^\([0-9][0-9]*\)\.\([0-9][0-9]*\).*/\1*256*256+\2*256/p'`))

version=${version:-0}


# posix_fadvise64() had been implemented in 2.5.60

if [ $version -lt 132412 ]; then
    have=NGX_HAVE_POSIX_FADVISE . auto/nohave
fi
{% endhighlight %}

如当前**uname -r**值为```3.10.0-514.el7.x86_64```，则完成匹配后输出：```3*256*256+10*256+0```, version的值最后就为199168。

```posix_fadvise64()```函数在2.5.60(```132412=2*256*256+5*256+60```)内核版本以前版本并没有实现，这里根据当前版本号做相应的宏定义。

函数posix_fadvise64()相关说明：
<pre>
Programs can use posix_fadvise() to announce an intention to access file data in a specific pattern in the future,
thus allowing the kernel to perform appropriate optimizations.
</pre>


### 1.5 epoll特性检测
{% highlight string %}
# epoll, EPOLLET version

ngx_feature="epoll"
ngx_feature_name="NGX_HAVE_EPOLL"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/epoll.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int efd = 0;
                  struct epoll_event ee;
                  ee.events = EPOLLIN|EPOLLOUT|EPOLLET;
                  ee.data.ptr = NULL;
                  efd = epoll_create(100);
                  if (efd == -1) return 1;"
. auto/feature

if [ $ngx_found = yes ]; then
    have=NGX_HAVE_CLEAR_EVENT . auto/have
    CORE_SRCS="$CORE_SRCS $EPOLL_SRCS"
    EVENT_MODULES="$EVENT_MODULES $EPOLL_MODULE"
    EVENT_FOUND=YES


    # EPOLLRDHUP appeared in Linux 2.6.17, glibc 2.8

    ngx_feature="EPOLLRDHUP"
    ngx_feature_name="NGX_HAVE_EPOLLRDHUP"
    ngx_feature_run=no
    ngx_feature_incs="#include <sys/epoll.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="int efd = 0, fd = 0;
                      struct epoll_event ee;
                      ee.events = EPOLLIN|EPOLLRDHUP|EPOLLET;
                      ee.data.ptr = NULL;
                      epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ee)"
    . auto/feature
fi
{% endhighlight %}

如果检测到当前系统支持epoll，则向```$CORE_SRCS```变量中添加```$EPOLL_MODULE```，同时将```EVENT_FOUND```置为yes。


### 1.6 检查O_PATH特性
{% highlight string %}
# O_PATH and AT_EMPTY_PATH were introduced in 2.6.39, glibc 2.14

ngx_feature="O_PATH"
ngx_feature_name="NGX_HAVE_O_PATH"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/stat.h>
                  #include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int fd; struct stat sb;
                  fd = openat(AT_FDCWD, \".\", O_PATH|O_DIRECTORY|O_NOFOLLOW);
                  if (fstatat(fd, \"\", &sb, AT_EMPTY_PATH) != 0) return 1"
. auto/feature
{% endhighlight %}

```O_PATH```选项获取一个文件句柄，并主要在该句柄上进行两类操作：获取在文件系统树中的位置、单纯在文件描述符上的一些操作。详见如下：
<pre>
O_PATH (since Linux 2.6.39)
    Obtain  a file descriptor that can be used for two purposes: to indicate a location in the file-system tree and to perform operations that act purely at
    the file descriptor level.  The file itself is not opened, and other file operations (e.g., read(2), write(2), fchmod(2), fchown(2), fgetxattr(2))  fail
    with the error EBADF.

    The following operations can be performed on the resulting file descriptor:

    *  close(2); fchdir(2) (since Linux 3.5); fstat(2) (since Linux 3.6).

    *  Duplicating the file descriptor (dup(2), fcntl(2) F_DUPFD, etc.).

    *  Getting and setting file descriptor flags (fcntl(2) F_GETFD and F_SETFD).

    *  Retrieving open file status flags using the fcntl(2) F_GETFL operation: the returned flags will include the bit O_PATH.

    *  Passing the file descriptor as the dirfd argument of openat(2) and the other "*at()" system calls.

    *  Passing the file descriptor to another process via a UNIX domain socket (see SCM_RIGHTS in unix(7)).

    When O_PATH is specified in flags, flag bits other than O_DIRECTORY and O_NOFOLLOW are ignored.

    If  the  O_NOFOLLOW flag is also specified, then the call returns a file descriptor referring to the symbolic link.  This file descriptor can be used as
    the dirfd argument in calls to fchownat(2), fstatat(2), linkat(2), and readlinkat(2) with an empty pathname to have the calls operate  on  the  symbolic
    link.
</pre>


### 1.7 检查sendfile()特性
sendfile()直接在两个文件句柄之间拷贝数据，因为这里拷贝都是在内核层面完成，因此sendfile()比read()、write()这样的组合更高效，因为后者需要在用户空间传递数据。
{% highlight string %}
# sendfile()

CC_AUX_FLAGS="$cc_aux_flags -D_GNU_SOURCE"
ngx_feature="sendfile()"
ngx_feature_name="NGX_HAVE_SENDFILE"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/sendfile.h>
                  #include <errno.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int s = 0, fd = 1;
                  ssize_t n; off_t off = 0;
                  n = sendfile(s, fd, &off, 1);
                  if (n == -1 && errno == ENOSYS) return 1"
. auto/feature

if [ $ngx_found = yes ]; then
    CORE_SRCS="$CORE_SRCS $LINUX_SENDFILE_SRCS"
fi
{% endhighlight %}


### 1.8 检查sendfile64()特性
与上述sendfile()类似，只不过支持64bit offset.
{% highlight string %}
# sendfile64()

CC_AUX_FLAGS="$cc_aux_flags -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"
ngx_feature="sendfile64()"
ngx_feature_name="NGX_HAVE_SENDFILE64"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/sendfile.h>
                  #include <errno.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int s = 0, fd = 1;
                  ssize_t n; off_t off = 0;
                  n = sendfile(s, fd, &off, 1);
                  if (n == -1 && errno == ENOSYS) return 1"
. auto/feature
{% endhighlight %}


### 1.9 检查prctl(PR_SET_DUMPABLE)特性
{% highlight string %}
ngx_include="sys/prctl.h"; . auto/include

# prctl(PR_SET_DUMPABLE)

ngx_feature="prctl(PR_SET_DUMPABLE)"
ngx_feature_name="NGX_HAVE_PR_SET_DUMPABLE"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/prctl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) == -1) return 1"
. auto/feature
{% endhighlight %}
prctl()函数主要用于进程方面的操作。

### 1.10 检查sched_setaffinity()特性
sched_setaffinity()主要用于进程调度时的CPU亲和性设置。
{% highlight string %}
# sched_setaffinity()

ngx_feature="sched_setaffinity()"
ngx_feature_name="NGX_HAVE_SCHED_SETAFFINITY"
ngx_feature_run=no
ngx_feature_incs="#include <sched.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="cpu_set_t mask;
                  CPU_ZERO(&mask);
                  sched_setaffinity(0, sizeof(cpu_set_t), &mask)"
. auto/feature
{% endhighlight %}


### 1.11 检查crypt_r()特性
{% highlight string %}
# crypt_r()

ngx_feature="crypt_r()"
ngx_feature_name="NGX_HAVE_GNU_CRYPT_R"
ngx_feature_run=no
ngx_feature_incs="#include <crypt.h>"
ngx_feature_path=
ngx_feature_libs=-lcrypt
ngx_feature_test="struct crypt_data  cd;
                  crypt_r(\"key\", \"salt\", &cd);"
. auto/feature
{% endhighlight %}

crypt_r()函数用于密码加密，它是一个基于DES标准的算法。

### 1.12 其他
{% highlight string %}
ngx_include="sys/vfs.h";     . auto/include


CC_AUX_FLAGS="$cc_aux_flags -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"
{% endhighlight %}

```sys/vfs.h```头文件一般定义为如下：
{% highlight string %}
#ifndef _LINUX_VFS_H
#define _LINUX_VFS_H

#include <linux/statfs.h>

#endif
{% endhighlight %}

```-D_FILE_OFFSET_BITS=64```表示采用64bit文件偏移offset,针对32位系统会typedef做自动调整。

```-D_GNU_SOURCE```这个参数表示你编写符合GNU规范的代码。GNU相对POSIX有一些增强，也有一些缺少，总体来说GNU的实现应该是更好一点。但是这关系到软件的可移植性。


## 2. auto/os/conf脚本

脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


echo "checking for $NGX_SYSTEM specific features"

case "$NGX_PLATFORM" in

    FreeBSD:*)
        . auto/os/freebsd
    ;;

    Linux:*)
        . auto/os/linux
    ;;

    SunOS:*)
        . auto/os/solaris
    ;;

    Darwin:*)
        . auto/os/darwin
    ;;

    win32)
        . auto/os/win32
    ;;

    DragonFly:*)
        have=NGX_FREEBSD . auto/have_headers
        CORE_INCS="$UNIX_INCS"
        CORE_DEPS="$UNIX_DEPS $FREEBSD_DEPS"
        CORE_SRCS="$UNIX_SRCS $FREEBSD_SRCS"

        echo " + sendfile() found"
        have=NGX_HAVE_SENDFILE . auto/have
        CORE_SRCS="$CORE_SRCS $FREEBSD_SENDFILE_SRCS"

        ngx_spacer='
'
    ;;

    HP-UX:*)
        # HP/UX
        have=NGX_HPUX . auto/have_headers
        CORE_INCS="$UNIX_INCS"
        CORE_DEPS="$UNIX_DEPS $POSIX_DEPS"
        CORE_SRCS="$UNIX_SRCS"
        CC_AUX_FLAGS="$CC_AUX_FLAGS -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED=1"
        CC_AUX_FLAGS="$CC_AUX_FLAGS -D_HPUX_ALT_XOPEN_SOCKET_API"
    ;;

    OSF1:*)
        # Tru64 UNIX
        have=NGX_TRU64 . auto/have_headers
        have=NGX_HAVE_STRERROR_R . auto/nohave
        CORE_INCS="$UNIX_INCS"
        CORE_DEPS="$UNIX_DEPS $POSIX_DEPS"
        CORE_SRCS="$UNIX_SRCS"
    ;;

    GNU:*)
        # GNU Hurd
        have=NGX_GNU_HURD . auto/have_headers
        CORE_INCS="$UNIX_INCS"
        CORE_DEPS="$UNIX_DEPS $POSIX_DEPS"
        CORE_SRCS="$UNIX_SRCS"
        CC_AUX_FLAGS="$CC_AUX_FLAGS -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64"
    ;;

    *)
        CORE_INCS="$UNIX_INCS"
        CORE_DEPS="$UNIX_DEPS $POSIX_DEPS"
        CORE_SRCS="$UNIX_SRCS"
    ;;

esac


case "$NGX_MACHINE" in

    i386 | i686 | i86pc)
        have=NGX_HAVE_NONALIGNED . auto/have
        NGX_MACH_CACHE_LINE=32
    ;;

    amd64 | x86_64)
        have=NGX_HAVE_NONALIGNED . auto/have
        NGX_MACH_CACHE_LINE=64
    ;;

    sun4u | sun4v | sparc | sparc64)
        have=NGX_ALIGNMENT value=16 . auto/define
        # TODO
        NGX_MACH_CACHE_LINE=64
    ;;

    ia64 )
        have=NGX_ALIGNMENT value=16 . auto/define
        # TODO
        NGX_MACH_CACHE_LINE=64
    ;;

    *)
        have=NGX_ALIGNMENT value=16 . auto/define
        NGX_MACH_CACHE_LINE=32
    ;;

esac

if test -z "$NGX_CPU_CACHE_LINE"; then
    NGX_CPU_CACHE_LINE=$NGX_MACH_CACHE_LINE
fi

have=NGX_CPU_CACHE_LINE value=$NGX_CPU_CACHE_LINE . auto/define
{% endhighlight %}

这里```$NGX_PLATFORM```为Linux，因此执行auto/os/linux脚本。

```$NGX_MACHINE```为i686，因此执行如下：
{% highlight string %}
have=NGX_HAVE_NONALIGNED . auto/have
NGX_MACH_CACHE_LINE=32
{% endhighlight %}

当前我们并没有进行```$CPU```的设置，因此```$NGX_CPU_CACHE_LINE```当前也没有值，这里根据我们的系统环境会被设置为32。

<br />
<br />

```Tips>>>```：
<pre>
Linux发行版中i386/i686/x86-64有什么区别？

i386 适用于intel和AMD所有32位的CPU以及via采用x86架构的32位CPU。intel平台包括8086，80286，80386，80486，奔腾系列(1,2,3,4)，
赛扬系列，Pentium D系列以及centrino P-M, Core duo等。

x86_64 适用于intel的Core 2 Duo, Centrino Core 2 Duo, Xeno 和 AMD Athlon64/x2， Sempron64/x2, Duron64等采用x86架构的64
位CPU。

i686 只是i386的一个子集，支持的CPU从Pentium 2(686)开始,之前的型号不支持.
</pre>



<br />
<br />
<br />

