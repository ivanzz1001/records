---
layout: post
title: auto/sources脚本解析-part4
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

在configure脚本中，运行完auto/options和auto/init脚本后，接下来就运行auto/sources脚本。这个脚本是为编译做准备的，定义了所有需要编译的modules，以及编译这些modules需要哪些源文件。


<!-- more -->

这些需要编译的modules都会通过脚本的方式写到ngx_modules.c文件中，然后编译进整个应用程序。因此nginx在启动时才知道自己需要启动那些modules。这种做法的一个好处就是，人们可以扩充很多modules，但是我们可以通过脚本的方式控制需要编译哪些，而不会造成任何不必要的代码被编译进nginx应用程序。

参看：```http://blog.csdn.net/poechant/article/details/7327217```

## 1. auto/sources脚本

在分析脚本之前，我们这里贴出其源代码：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


CORE_MODULES="ngx_core_module ngx_errlog_module ngx_conf_module"

CORE_INCS="src/core"

CORE_DEPS="src/core/nginx.h \
           src/core/ngx_config.h \
           src/core/ngx_core.h \
           src/core/ngx_log.h \
           src/core/ngx_palloc.h \
           src/core/ngx_array.h \
           src/core/ngx_list.h \
           src/core/ngx_hash.h \
           src/core/ngx_buf.h \
           src/core/ngx_queue.h \
           src/core/ngx_string.h \
           src/core/ngx_parse.h \
           src/core/ngx_parse_time.h \
           src/core/ngx_inet.h \
           src/core/ngx_file.h \
           src/core/ngx_crc.h \
           src/core/ngx_crc32.h \
           src/core/ngx_murmurhash.h \
           src/core/ngx_md5.h \
           src/core/ngx_sha1.h \
           src/core/ngx_rbtree.h \
           src/core/ngx_radix_tree.h \
           src/core/ngx_rwlock.h \
           src/core/ngx_slab.h \
           src/core/ngx_times.h \
           src/core/ngx_shmtx.h \
           src/core/ngx_connection.h \
           src/core/ngx_cycle.h \
           src/core/ngx_conf_file.h \
           src/core/ngx_module.h \
           src/core/ngx_resolver.h \
           src/core/ngx_open_file_cache.h \
           src/core/ngx_crypt.h \
           src/core/ngx_proxy_protocol.h \
           src/core/ngx_syslog.h"


CORE_SRCS="src/core/nginx.c \
           src/core/ngx_log.c \
           src/core/ngx_palloc.c \
           src/core/ngx_array.c \
           src/core/ngx_list.c \
           src/core/ngx_hash.c \
           src/core/ngx_buf.c \
           src/core/ngx_queue.c \
           src/core/ngx_output_chain.c \
           src/core/ngx_string.c \
           src/core/ngx_parse.c \
           src/core/ngx_parse_time.c \
           src/core/ngx_inet.c \
           src/core/ngx_file.c \
           src/core/ngx_crc32.c \
           src/core/ngx_murmurhash.c \
           src/core/ngx_md5.c \
           src/core/ngx_rbtree.c \
           src/core/ngx_radix_tree.c \
           src/core/ngx_slab.c \
           src/core/ngx_times.c \
           src/core/ngx_shmtx.c \
           src/core/ngx_connection.c \
           src/core/ngx_cycle.c \
           src/core/ngx_spinlock.c \
           src/core/ngx_rwlock.c \
           src/core/ngx_cpuinfo.c \
           src/core/ngx_conf_file.c \
           src/core/ngx_module.c \
           src/core/ngx_resolver.c \
           src/core/ngx_open_file_cache.c \
           src/core/ngx_crypt.c \
           src/core/ngx_proxy_protocol.c \
           src/core/ngx_syslog.c"


EVENT_MODULES="ngx_events_module ngx_event_core_module"

EVENT_INCS="src/event src/event/modules"

EVENT_DEPS="src/event/ngx_event.h \
            src/event/ngx_event_timer.h \
            src/event/ngx_event_posted.h \
            src/event/ngx_event_connect.h \
            src/event/ngx_event_pipe.h"

EVENT_SRCS="src/event/ngx_event.c \
            src/event/ngx_event_timer.c \
            src/event/ngx_event_posted.c \
            src/event/ngx_event_accept.c \
            src/event/ngx_event_connect.c \
            src/event/ngx_event_pipe.c"


SELECT_MODULE=ngx_select_module
SELECT_SRCS=src/event/modules/ngx_select_module.c
WIN32_SELECT_SRCS=src/event/modules/ngx_win32_select_module.c

POLL_MODULE=ngx_poll_module
POLL_SRCS=src/event/modules/ngx_poll_module.c

KQUEUE_MODULE=ngx_kqueue_module
KQUEUE_SRCS=src/event/modules/ngx_kqueue_module.c

DEVPOLL_MODULE=ngx_devpoll_module
DEVPOLL_SRCS=src/event/modules/ngx_devpoll_module.c

EVENTPORT_MODULE=ngx_eventport_module
EVENTPORT_SRCS=src/event/modules/ngx_eventport_module.c

EPOLL_MODULE=ngx_epoll_module
EPOLL_SRCS=src/event/modules/ngx_epoll_module.c

IOCP_MODULE=ngx_iocp_module
IOCP_SRCS=src/event/modules/ngx_iocp_module.c

FILE_AIO_SRCS="src/os/unix/ngx_file_aio_read.c"
LINUX_AIO_SRCS="src/os/unix/ngx_linux_aio_read.c"

UNIX_INCS="$CORE_INCS $EVENT_INCS src/os/unix"

UNIX_DEPS="$CORE_DEPS $EVENT_DEPS \
            src/os/unix/ngx_time.h \
            src/os/unix/ngx_errno.h \
            src/os/unix/ngx_alloc.h \
            src/os/unix/ngx_files.h \
            src/os/unix/ngx_channel.h \
            src/os/unix/ngx_shmem.h \
            src/os/unix/ngx_process.h \
            src/os/unix/ngx_setaffinity.h \
            src/os/unix/ngx_setproctitle.h \
            src/os/unix/ngx_atomic.h \
            src/os/unix/ngx_gcc_atomic_x86.h \
            src/os/unix/ngx_thread.h \
            src/os/unix/ngx_socket.h \
            src/os/unix/ngx_os.h \
            src/os/unix/ngx_user.h \
            src/os/unix/ngx_dlopen.h \
            src/os/unix/ngx_process_cycle.h"

# add to UNIX_DEPS
#            src/os/unix/ngx_gcc_atomic_amd64.h \
#            src/os/unix/ngx_gcc_atomic_sparc64.h \
#            src/os/unix/ngx_gcc_atomic_ppc.h \
#            src/os/unix/ngx_sunpro_atomic_sparc64.h \
#            src/os/unix/ngx_sunpro_x86.il \
#            src/os/unix/ngx_sunpro_amd64.il \
#            src/os/unix/ngx_sunpro_sparc64.il \


UNIX_SRCS="$CORE_SRCS $EVENT_SRCS \
            src/os/unix/ngx_time.c \
            src/os/unix/ngx_errno.c \
            src/os/unix/ngx_alloc.c \
            src/os/unix/ngx_files.c \
            src/os/unix/ngx_socket.c \
            src/os/unix/ngx_recv.c \
            src/os/unix/ngx_readv_chain.c \
            src/os/unix/ngx_udp_recv.c \
            src/os/unix/ngx_send.c \
            src/os/unix/ngx_writev_chain.c \
            src/os/unix/ngx_udp_send.c \
            src/os/unix/ngx_channel.c \
            src/os/unix/ngx_shmem.c \
            src/os/unix/ngx_process.c \
            src/os/unix/ngx_daemon.c \
            src/os/unix/ngx_setaffinity.c \
            src/os/unix/ngx_setproctitle.c \
            src/os/unix/ngx_posix_init.c \
            src/os/unix/ngx_user.c \
            src/os/unix/ngx_dlopen.c \
            src/os/unix/ngx_process_cycle.c"

POSIX_DEPS=src/os/unix/ngx_posix_config.h

THREAD_POOL_MODULE=ngx_thread_pool_module
THREAD_POOL_DEPS=src/core/ngx_thread_pool.h
THREAD_POOL_SRCS="src/core/ngx_thread_pool.c
                  src/os/unix/ngx_thread_cond.c
                  src/os/unix/ngx_thread_mutex.c
                  src/os/unix/ngx_thread_id.c"

FREEBSD_DEPS="src/os/unix/ngx_freebsd_config.h src/os/unix/ngx_freebsd.h"
FREEBSD_SRCS=src/os/unix/ngx_freebsd_init.c
FREEBSD_SENDFILE_SRCS=src/os/unix/ngx_freebsd_sendfile_chain.c

LINUX_DEPS="src/os/unix/ngx_linux_config.h src/os/unix/ngx_linux.h"
LINUX_SRCS=src/os/unix/ngx_linux_init.c
LINUX_SENDFILE_SRCS=src/os/unix/ngx_linux_sendfile_chain.c


SOLARIS_DEPS="src/os/unix/ngx_solaris_config.h src/os/unix/ngx_solaris.h"
SOLARIS_SRCS=src/os/unix/ngx_solaris_init.c
SOLARIS_SENDFILEV_SRCS=src/os/unix/ngx_solaris_sendfilev_chain.c


DARWIN_DEPS="src/os/unix/ngx_darwin_config.h src/os/unix/ngx_darwin.h"
DARWIN_SRCS=src/os/unix/ngx_darwin_init.c
DARWIN_SENDFILE_SRCS=src/os/unix/ngx_darwin_sendfile_chain.c


WIN32_INCS="$CORE_INCS $EVENT_INCS src/os/win32"

WIN32_DEPS="$CORE_DEPS $EVENT_DEPS \
            src/os/win32/ngx_win32_config.h \
            src/os/win32/ngx_time.h \
            src/os/win32/ngx_errno.h \
            src/os/win32/ngx_alloc.h \
            src/os/win32/ngx_files.h \
            src/os/win32/ngx_shmem.h \
            src/os/win32/ngx_process.h \
            src/os/win32/ngx_atomic.h \
            src/os/win32/ngx_thread.h \
            src/os/win32/ngx_socket.h \
            src/os/win32/ngx_os.h \
            src/os/win32/ngx_user.h \
            src/os/win32/ngx_dlopen.h \
            src/os/win32/ngx_process_cycle.h"

WIN32_CONFIG=src/os/win32/ngx_win32_config.h

WIN32_SRCS="$CORE_SRCS $EVENT_SRCS \
            src/os/win32/ngx_errno.c \
            src/os/win32/ngx_alloc.c \
            src/os/win32/ngx_files.c \
            src/os/win32/ngx_shmem.c \
            src/os/win32/ngx_time.c \
            src/os/win32/ngx_process.c \
            src/os/win32/ngx_thread.c \
            src/os/win32/ngx_socket.c \
            src/os/win32/ngx_wsarecv.c \
            src/os/win32/ngx_wsarecv_chain.c \
            src/os/win32/ngx_udp_wsarecv.c \
            src/os/win32/ngx_wsasend.c \
            src/os/win32/ngx_wsasend_chain.c \
            src/os/win32/ngx_win32_init.c \
            src/os/win32/ngx_user.c \
            src/os/win32/ngx_dlopen.c \
            src/os/win32/ngx_event_log.c \
            src/os/win32/ngx_process_cycle.c \
            src/event/ngx_event_acceptex.c"

NGX_WIN32_ICONS="src/os/win32/nginx.ico"
NGX_WIN32_RC="src/os/win32/nginx.rc"


HTTP_FILE_CACHE_SRCS=src/http/ngx_http_file_cache.c
{% endhighlight %}

在auto/sources脚本中，其一般采用如下4个变量来定义一个模块：

* 模块名称
* 模块头文件所在目录
* 模块头文件
* 模块源代码文件


## 2. nginx核心模块

nginx核心模块是整个nginx运行的一个最基本框架，其他任何模块的运行都需要依赖于这个核心模块。因此如果我们自己想要复用nginx来编写一个最简单的应用程序，其他模块都可以被裁减掉，然而核心模块是必不可少的.

(1) 模块名称

nginx核心模块名称为```CORE_MODULES```，该变量记录了nginx的核心模块。默认包括：ngx_core_module、ngx_errlog_module和ngx_conf_module。相应初始化代码如下：
{% highlight string %}
CORE_MODULES="ngx_core_module ngx_errlog_module ngx_conf_module"
{% endhighlight %}

(2) 模块头文件所在目录

```CORE_INCS```变量记录了nginx核心模块所在的目录：
{% highlight string %}
CORE_INCS="src/core"
{% endhighlight %}

(3) 模块头文件

```CORE_DEPS```变量记录了nginx核心模块所依赖的头文件。DEPS的含义为dependencies。其包含src/core/目录下的35个头文件，唯独没有包括为src/core/ngx_regex.h和src/core/ngx_thread_pool.h这两个：

{%highlight string %}
CORE_DEPS="src/core/nginx.h \
           src/core/ngx_config.h \
           src/core/ngx_core.h \
           src/core/ngx_log.h \
           src/core/ngx_palloc.h \
           src/core/ngx_array.h \
           src/core/ngx_list.h \
           src/core/ngx_hash.h \
           src/core/ngx_buf.h \
           src/core/ngx_queue.h \
           src/core/ngx_string.h \
           src/core/ngx_parse.h \
           src/core/ngx_parse_time.h \
           src/core/ngx_inet.h \
           src/core/ngx_file.h \
           src/core/ngx_crc.h \
           src/core/ngx_crc32.h \
           src/core/ngx_murmurhash.h \
           src/core/ngx_md5.h \
           src/core/ngx_sha1.h \
           src/core/ngx_rbtree.h \
           src/core/ngx_radix_tree.h \
           src/core/ngx_rwlock.h \
           src/core/ngx_slab.h \
           src/core/ngx_times.h \
           src/core/ngx_shmtx.h \
           src/core/ngx_connection.h \
           src/core/ngx_cycle.h \
           src/core/ngx_conf_file.h \
           src/core/ngx_module.h \
           src/core/ngx_resolver.h \
           src/core/ngx_open_file_cache.h \
           src/core/ngx_crypt.h \
           src/core/ngx_proxy_protocol.h \
           src/core/ngx_syslog.h"
{% endhighlight %}

(4) 模块源代码文件

```CORE_SRCS```变量记录了nginx核心模块所依赖的源代码文件。SRCS的含义是sources，包含src/core目录下的34个源文件，仅仅没有包含src/core/ngx_regex.c和src/core/ngx_thread_pool.c这两个：

{% highlight string %}
CORE_SRCS="src/core/nginx.c \
           src/core/ngx_log.c \
           src/core/ngx_palloc.c \
           src/core/ngx_array.c \
           src/core/ngx_list.c \
           src/core/ngx_hash.c \
           src/core/ngx_buf.c \
           src/core/ngx_queue.c \
           src/core/ngx_output_chain.c \
           src/core/ngx_string.c \
           src/core/ngx_parse.c \
           src/core/ngx_parse_time.c \
           src/core/ngx_inet.c \
           src/core/ngx_file.c \
           src/core/ngx_crc32.c \
           src/core/ngx_murmurhash.c \
           src/core/ngx_md5.c \
           src/core/ngx_rbtree.c \
           src/core/ngx_radix_tree.c \
           src/core/ngx_slab.c \
           src/core/ngx_times.c \
           src/core/ngx_shmtx.c \
           src/core/ngx_connection.c \
           src/core/ngx_cycle.c \
           src/core/ngx_spinlock.c \
           src/core/ngx_rwlock.c \
           src/core/ngx_cpuinfo.c \
           src/core/ngx_conf_file.c \
           src/core/ngx_module.c \
           src/core/ngx_resolver.c \
           src/core/ngx_open_file_cache.c \
           src/core/ngx_crypt.c \
           src/core/ngx_proxy_protocol.c \
           src/core/ngx_syslog.c"
{% endhighlight %}


## 3. nginx事件模块
nginx事件模块包含定时器事件和网络事件两种。

(1) 模块名称

nginx事件模块名称为```EVENT_MODULES```，该变量记录了nginx的事件模块。该模块包括:ngx_events_module和ngx_event_core_module.

{% highlight string %}
EVENT_MODULES="ngx_events_module ngx_event_core_module"
{% endhighlight %}

(2) 模块头文件所在目录

```EVENT_INCS```变量记录了nginx事件模块所在的目录：
{% highlight string %}
EVENT_INCS="src/event src/event/modules"
{% endhighlight %}

(3) 模块头文件

```EVENT_DEPS```变量记录了nginx事件模块所依赖的头文件。总共包括5个头文件，都在src/event目录下，唯独不包括该目录下的src/event/ngx_event_openssl.h文件，该文件属于openssl模块的头文件：
{% highlight string %}
EVENT_DEPS="src/event/ngx_event.h \
            src/event/ngx_event_timer.h \
            src/event/ngx_event_posted.h \
            src/event/ngx_event_connect.h \
            src/event/ngx_event_pipe.h"
{% endhighlight %}


(4) 模块源代码文件

```EVENT_SRCS```变量就了nginx事件模块所依赖的源代码文件。总共包含6个源文件，都在src/event目录下，唯独不包含该目录下的src/event/ngx_event_openssl.c和src/event/ngx_event_openssl_stapling.c源文件：
{% highlight string %}
EVENT_SRCS="src/event/ngx_event.c \
            src/event/ngx_event_timer.c \
            src/event/ngx_event_posted.c \
            src/event/ngx_event_accept.c \
            src/event/ngx_event_connect.c \
            src/event/ngx_event_pipe.c"
{% endhighlight %}


## 4. 事件驱动模型

nginx事件驱动模型包括包括：select、poll、kqueue、devpoll、eventport、epoll、iocp、aiso。其实它是属于上面nginx事件模块的一部分，存在于上面nginx事件模块头文件目录的src/event/modules目录下。后面我们会专门详细介绍这些事件驱动模型的原理和异同，这里不赘述。

(1) select模型
{% highlight string %}
SELECT_MODULE=ngx_select_module
SELECT_SRCS=src/event/modules/ngx_select_module.c
WIN32_SELECT_SRCS=src/event/modules/ngx_win32_select_module.c
{% endhighlight %}

(2) poll模型
{% highlight string %}
POLL_MODULE=ngx_poll_module
POLL_SRCS=src/event/modules/ngx_poll_module.c
{% endhighlight %}

(3) kqueue模型
{% highlight string %}
KQUEUE_MODULE=ngx_kqueue_module
KQUEUE_SRCS=src/event/modules/ngx_kqueue_module.c
{% endhighlight %}


(4) devpoll模型
{% highlight string %}
DEVPOLL_MODULE=ngx_devpoll_module
DEVPOLL_SRCS=src/event/modules/ngx_devpoll_module.c
{% endhighlight %}


(5) eventport模型
{% highlight string %}
EVENTPORT_MODULE=ngx_eventport_module
EVENTPORT_SRCS=src/event/modules/ngx_eventport_module.c
{% endhighlight %}

(6) epoll模型
{% highlight string %}
EPOLL_MODULE=ngx_epoll_module
EPOLL_SRCS=src/event/modules/ngx_epoll_module.c
{% endhighlight %}

(7) iocp模型
{% highlight string %}
IOCP_MODULE=ngx_iocp_module
IOCP_SRCS=src/event/modules/ngx_iocp_module.c
{% endhighlight %}


(8) aio模型
{% highlight string %}
FILE_AIO_SRCS="src/os/unix/ngx_file_aio_read.c"
LINUX_AIO_SRCS="src/os/unix/ngx_linux_aio_read.c"
{% endhighlight %}


## 5. 操作系统相关

这里从整体上分为类Unix和Windows两大操作系统。我们分成两部分来讲解。

### 5.1 类Unix操作系统

相关头文件所在目录为：
{% highlight string %}
UNIX_INCS="$CORE_INCS $EVENT_INCS src/os/unix"
{% endhighlight %}

所有Unix相关头文件：
{% highlight string %}
UNIX_DEPS="$CORE_DEPS $EVENT_DEPS \
            src/os/unix/ngx_time.h \
            src/os/unix/ngx_errno.h \
            src/os/unix/ngx_alloc.h \
            src/os/unix/ngx_files.h \
            src/os/unix/ngx_channel.h \
            src/os/unix/ngx_shmem.h \
            src/os/unix/ngx_process.h \
            src/os/unix/ngx_setaffinity.h \
            src/os/unix/ngx_setproctitle.h \
            src/os/unix/ngx_atomic.h \
            src/os/unix/ngx_gcc_atomic_x86.h \
            src/os/unix/ngx_thread.h \
            src/os/unix/ngx_socket.h \
            src/os/unix/ngx_os.h \
            src/os/unix/ngx_user.h \
            src/os/unix/ngx_dlopen.h \
            src/os/unix/ngx_process_cycle.h"

# add to UNIX_DEPS
#            src/os/unix/ngx_gcc_atomic_amd64.h \
#            src/os/unix/ngx_gcc_atomic_sparc64.h \
#            src/os/unix/ngx_gcc_atomic_ppc.h \
#            src/os/unix/ngx_sunpro_atomic_sparc64.h \
#            src/os/unix/ngx_sunpro_x86.il \
#            src/os/unix/ngx_sunpro_amd64.il \
#            src/os/unix/ngx_sunpro_sparc64.il \

{% endhighlight %}

所有Unix相关源文件：
{% highlight string %}
UNIX_SRCS="$CORE_SRCS $EVENT_SRCS \
            src/os/unix/ngx_time.c \
            src/os/unix/ngx_errno.c \
            src/os/unix/ngx_alloc.c \
            src/os/unix/ngx_files.c \
            src/os/unix/ngx_socket.c \
            src/os/unix/ngx_recv.c \
            src/os/unix/ngx_readv_chain.c \
            src/os/unix/ngx_udp_recv.c \
            src/os/unix/ngx_send.c \
            src/os/unix/ngx_writev_chain.c \
            src/os/unix/ngx_udp_send.c \
            src/os/unix/ngx_channel.c \
            src/os/unix/ngx_shmem.c \
            src/os/unix/ngx_process.c \
            src/os/unix/ngx_daemon.c \
            src/os/unix/ngx_setaffinity.c \
            src/os/unix/ngx_setproctitle.c \
            src/os/unix/ngx_posix_init.c \
            src/os/unix/ngx_user.c \
            src/os/unix/ngx_dlopen.c \
            src/os/unix/ngx_process_cycle.c"
{% endhighlight %}

posix相关配置头文件：
{% highlight string %}
POSIX_DEPS=src/os/unix/ngx_posix_config.h
{% endhighlight %}

多线程模块：
{% highlight string %}
THREAD_POOL_MODULE=ngx_thread_pool_module
THREAD_POOL_DEPS=src/core/ngx_thread_pool.h
THREAD_POOL_SRCS="src/core/ngx_thread_pool.c
                  src/os/unix/ngx_thread_cond.c
                  src/os/unix/ngx_thread_mutex.c
                  src/os/unix/ngx_thread_id.c"
{% endhighlight %}
```注：thread pool是较新版本的nginx新添加的一个线程池模块```



如下是具体的类Unix操作系统的一些实现：

(1) freebsd操作系统

freebsd操作系统相关头文件、源文件、以及sendfile机制的源文件：
{% highlight string %}
FREEBSD_DEPS="src/os/unix/ngx_freebsd_config.h src/os/unix/ngx_freebsd.h"
FREEBSD_SRCS=src/os/unix/ngx_freebsd_init.c
FREEBSD_SENDFILE_SRCS=src/os/unix/ngx_freebsd_sendfile_chain.c
{% endhighlight %}


(2) linux操作系统

linux操作系统相关头文件、源文件、以及sendfile机制的源文件：
{% highlight string %}
LINUX_DEPS="src/os/unix/ngx_linux_config.h src/os/unix/ngx_linux.h"
LINUX_SRCS=src/os/unix/ngx_linux_init.c
LINUX_SENDFILE_SRCS=src/os/unix/ngx_linux_sendfile_chain.c
{% endhighlight %}


(3) solaris操作系统

solaris操作系统相关头文件、源文件、以及sendfile机制的源文件:
{% highlight string %}
SOLARIS_DEPS="src/os/unix/ngx_solaris_config.h src/os/unix/ngx_solaris.h"
SOLARIS_SRCS=src/os/unix/ngx_solaris_init.c
SOLARIS_SENDFILEV_SRCS=src/os/unix/ngx_solaris_sendfilev_chain.c
{% endhighlight %}

(4) darwin操作系统

darwin操作系统相关头文件、源文件、以及sendfile机制的源文件:
{% highlight string %}
DARWIN_DEPS="src/os/unix/ngx_darwin_config.h src/os/unix/ngx_darwin.h"
DARWIN_SRCS=src/os/unix/ngx_darwin_init.c
DARWIN_SENDFILE_SRCS=src/os/unix/ngx_darwin_sendfile_chain.c
{% endhighlight %}

### 5.2 Windows操作系统

windows平台相关头文件所在目录：
{% highlight string %}
WIN32_INCS="$CORE_INCS $EVENT_INCS src/os/win32"
{% endhighlight %}

windows平台相关头文件：
{% highlight string %}
WIN32_DEPS="$CORE_DEPS $EVENT_DEPS \
            src/os/win32/ngx_win32_config.h \
            src/os/win32/ngx_time.h \
            src/os/win32/ngx_errno.h \
            src/os/win32/ngx_alloc.h \
            src/os/win32/ngx_files.h \
            src/os/win32/ngx_shmem.h \
            src/os/win32/ngx_process.h \
            src/os/win32/ngx_atomic.h \
            src/os/win32/ngx_thread.h \
            src/os/win32/ngx_socket.h \
            src/os/win32/ngx_os.h \
            src/os/win32/ngx_user.h \
            src/os/win32/ngx_dlopen.h \
            src/os/win32/ngx_process_cycle.h"
{% endhighlight %}

windows相关配置头文件：
{% highlight string %}
WIN32_CONFIG=src/os/win32/ngx_win32_config.h
{% endhighlight %}


windows相关源代码文件：
{% highlight string %}
WIN32_SRCS="$CORE_SRCS $EVENT_SRCS \
            src/os/win32/ngx_errno.c \
            src/os/win32/ngx_alloc.c \
            src/os/win32/ngx_files.c \
            src/os/win32/ngx_shmem.c \
            src/os/win32/ngx_time.c \
            src/os/win32/ngx_process.c \
            src/os/win32/ngx_thread.c \
            src/os/win32/ngx_socket.c \
            src/os/win32/ngx_wsarecv.c \
            src/os/win32/ngx_wsarecv_chain.c \
            src/os/win32/ngx_udp_wsarecv.c \
            src/os/win32/ngx_wsasend.c \
            src/os/win32/ngx_wsasend_chain.c \
            src/os/win32/ngx_win32_init.c \
            src/os/win32/ngx_user.c \
            src/os/win32/ngx_dlopen.c \
            src/os/win32/ngx_event_log.c \
            src/os/win32/ngx_process_cycle.c \
            src/event/ngx_event_acceptex.c"
{% endhighlight %}



nginx在windows平台的图表及资源文件：
{% highlight string %}
NGX_WIN32_ICONS="src/os/win32/nginx.ico"
NGX_WIN32_RC="src/os/win32/nginx.rc"
{% endhighlight %}


## 6. HTTP缓存相关头文件
{% highlight string %}
HTTP_FILE_CACHE_SRCS=src/http/ngx_http_file_cache.c
{% endhighlight %}


<br />
<br />
<br />

