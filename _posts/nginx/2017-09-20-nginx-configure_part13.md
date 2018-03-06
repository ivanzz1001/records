---
layout: post
title: auto/unix脚本分析-part13
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---


本节我们介绍auto/unix脚本，该脚本主要是做相应的特性的检查。

<!-- more -->



<br />
<br />

## 1. auto/unix脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


NGX_USER=${NGX_USER:-nobody}

if [ -z "$NGX_GROUP" ]; then
    if [ $NGX_USER = nobody ]; then
        if grep nobody /etc/group 2>&1 >/dev/null; then
            echo "checking for nobody group ... found"
            NGX_GROUP=nobody
        else
            echo "checking for nobody group ... not found"

            if grep nogroup /etc/group 2>&1 >/dev/null; then
                echo "checking for nogroup group ... found"
                NGX_GROUP=nogroup
            else
                echo "checking for nogroup group ... not found"
                NGX_GROUP=nobody
            fi
        fi
    else
        NGX_GROUP=$NGX_USER
    fi
fi


ngx_feature="poll()"
ngx_feature_name=
ngx_feature_run=no
ngx_feature_incs="#include <poll.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int  n; struct pollfd  pl;
                  pl.fd = 0;
                  pl.events = 0;
                  pl.revents = 0;
                  n = poll(&pl, 1, 0);
                  if (n == -1) return 1"
. auto/feature

if [ $ngx_found = no ]; then
    EVENT_POLL=NONE
fi


ngx_feature="/dev/poll"
ngx_feature_name="NGX_HAVE_DEVPOLL"
ngx_feature_run=no
ngx_feature_incs="#include <sys/devpoll.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int  n, dp; struct dvpoll  dvp;
                  dp = 0;
                  dvp.dp_fds = NULL;
                  dvp.dp_nfds = 0;
                  dvp.dp_timeout = 0;
                  n = ioctl(dp, DP_POLL, &dvp);
                  if (n == -1) return 1"
. auto/feature

if [ $ngx_found = yes ]; then
    CORE_SRCS="$CORE_SRCS $DEVPOLL_SRCS"
    EVENT_MODULES="$EVENT_MODULES $DEVPOLL_MODULE"
    EVENT_FOUND=YES
fi


if test -z "$NGX_KQUEUE_CHECKED"; then
    ngx_feature="kqueue"
    ngx_feature_name="NGX_HAVE_KQUEUE"
    ngx_feature_run=no
    ngx_feature_incs="#include <sys/event.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="int kq; kq = kqueue()"
    . auto/feature

    if [ $ngx_found = yes ]; then

        have=NGX_HAVE_CLEAR_EVENT . auto/have
        EVENT_MODULES="$EVENT_MODULES $KQUEUE_MODULE"
        CORE_SRCS="$CORE_SRCS $KQUEUE_SRCS"
        EVENT_FOUND=YES

        ngx_feature="kqueue's NOTE_LOWAT"
        ngx_feature_name="NGX_HAVE_LOWAT_EVENT"
        ngx_feature_run=no
        ngx_feature_incs="#include <sys/event.h>"
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test="struct kevent  kev;
                          kev.fflags = NOTE_LOWAT;"
        . auto/feature


        ngx_feature="kqueue's EVFILT_TIMER"
        ngx_feature_name="NGX_HAVE_TIMER_EVENT"
        ngx_feature_run=yes
        ngx_feature_incs="#include <sys/event.h>
                          #include <sys/time.h>"
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test="int      kq;
                  struct kevent    kev;
                  struct timespec  ts;

                  if ((kq = kqueue()) == -1) return 1;

                  kev.ident = 0;
                  kev.filter = EVFILT_TIMER;
                  kev.flags = EV_ADD|EV_ENABLE;
                  kev.fflags = 0;
                  kev.data = 1000;
                  kev.udata = 0;

                  ts.tv_sec = 0;
                  ts.tv_nsec = 0;

                  if (kevent(kq, &kev, 1, &kev, 1, &ts) == -1) return 1;

                  if (kev.flags & EV_ERROR) return 1;"

        . auto/feature
    fi
fi


if [ "$NGX_SYSTEM" = "NetBSD" ]; then

    # NetBSD 2.0 incompatibly defines kevent.udata as "intptr_t"

    cat << END >> $NGX_AUTO_CONFIG_H

#define NGX_KQUEUE_UDATA_T

END

else
    cat << END >> $NGX_AUTO_CONFIG_H

#define NGX_KQUEUE_UDATA_T  (void *)

END

fi


ngx_feature="crypt()"
ngx_feature_name=
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="crypt(\"test\", \"salt\");"
. auto/feature


if [ $ngx_found = no ]; then

    ngx_feature="crypt() in libcrypt"
    ngx_feature_name=
    ngx_feature_run=no
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=-lcrypt
    . auto/feature

    if [ $ngx_found = yes ]; then
        CRYPT_LIB="-lcrypt"
    fi
fi


ngx_feature="F_READAHEAD"
ngx_feature_name="NGX_HAVE_F_READAHEAD"
ngx_feature_run=no
ngx_feature_incs="#include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="fcntl(0, F_READAHEAD, 1);"
. auto/feature


ngx_feature="posix_fadvise()"
ngx_feature_name="NGX_HAVE_POSIX_FADVISE"
ngx_feature_run=no
ngx_feature_incs="#include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="posix_fadvise(0, 0, 0, POSIX_FADV_SEQUENTIAL);"
. auto/feature


ngx_feature="O_DIRECT"
ngx_feature_name="NGX_HAVE_O_DIRECT"
ngx_feature_run=no
ngx_feature_incs="#include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="fcntl(0, F_SETFL, O_DIRECT);"
. auto/feature


if [ $ngx_found = yes -a "$NGX_SYSTEM" = "Linux" ]; then
    have=NGX_HAVE_ALIGNED_DIRECTIO . auto/have
fi

ngx_feature="F_NOCACHE"
ngx_feature_name="NGX_HAVE_F_NOCACHE"
ngx_feature_run=no
ngx_feature_incs="#include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="fcntl(0, F_NOCACHE, 1);"
. auto/feature


ngx_feature="directio()"
ngx_feature_name="NGX_HAVE_DIRECTIO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="directio(0, DIRECTIO_ON);"
. auto/feature


ngx_feature="statfs()"
ngx_feature_name="NGX_HAVE_STATFS"
ngx_feature_run=no
ngx_feature_incs="$NGX_INCLUDE_SYS_PARAM_H
                  $NGX_INCLUDE_SYS_MOUNT_H
                  $NGX_INCLUDE_SYS_VFS_H"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct statfs  fs;
                  statfs(\".\", &fs);"
. auto/feature


ngx_feature="statvfs()"
ngx_feature_name="NGX_HAVE_STATVFS"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/statvfs.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct statvfs  fs;
                  statvfs(\".\", &fs);"
. auto/feature


ngx_feature="dlopen()"
ngx_feature_name="NGX_HAVE_DLOPEN"
ngx_feature_run=no
ngx_feature_incs="#include <dlfcn.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="dlopen(NULL, RTLD_NOW | RTLD_GLOBAL); dlsym(NULL, NULL)"
. auto/feature


if [ $ngx_found != yes ]; then

    ngx_feature="dlopen() in libdl"
    ngx_feature_libs="-ldl"
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_LIBS="$CORE_LIBS -ldl"
        NGX_LIBDL="-ldl"
    fi
fi


ngx_feature="sched_yield()"
ngx_feature_name="NGX_HAVE_SCHED_YIELD"
ngx_feature_run=no
ngx_feature_incs="#include <sched.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="sched_yield()"
. auto/feature


if [ $ngx_found != yes ]; then

    ngx_feature="sched_yield() in librt"
    ngx_feature_libs="-lrt"
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_LIBS="$CORE_LIBS -lrt"
    fi
fi


ngx_feature="SO_SETFIB"
ngx_feature_name="NGX_HAVE_SETFIB"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, SOL_SOCKET, SO_SETFIB, NULL, 0)"
. auto/feature


ngx_feature="SO_REUSEPORT"
ngx_feature_name="NGX_HAVE_REUSEPORT"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, SOL_SOCKET, SO_REUSEPORT, NULL, 0)"
. auto/feature


ngx_feature="SO_ACCEPTFILTER"
ngx_feature_name="NGX_HAVE_DEFERRED_ACCEPT"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, SOL_SOCKET, SO_ACCEPTFILTER, NULL, 0)"
. auto/feature


# BSD way to get IPv4 datagram destination address

ngx_feature="IP_RECVDSTADDR"
ngx_feature_name="NGX_HAVE_IP_RECVDSTADDR"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_IP, IP_RECVDSTADDR, NULL, 0)"
. auto/feature


# Linux way to get IPv4 datagram destination address

ngx_feature="IP_PKTINFO"
ngx_feature_name="NGX_HAVE_IP_PKTINFO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_IP, IP_PKTINFO, NULL, 0)"
. auto/feature


# RFC 3542 way to get IPv6 datagram destination address

ngx_feature="IPV6_RECVPKTINFO"
ngx_feature_name="NGX_HAVE_IPV6_RECVPKTINFO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_IPV6, IPV6_RECVPKTINFO, NULL, 0)"
. auto/feature


ngx_feature="TCP_DEFER_ACCEPT"
ngx_feature_name="NGX_HAVE_DEFERRED_ACCEPT"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>
                  #include <netinet/tcp.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_TCP, TCP_DEFER_ACCEPT, NULL, 0)"
. auto/feature


ngx_feature="TCP_KEEPIDLE"
ngx_feature_name="NGX_HAVE_KEEPALIVE_TUNABLE"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>
                  #include <netinet/tcp.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_TCP, TCP_KEEPIDLE, NULL, 0);
                  setsockopt(0, IPPROTO_TCP, TCP_KEEPINTVL, NULL, 0);
                  setsockopt(0, IPPROTO_TCP, TCP_KEEPCNT, NULL, 0)"
. auto/feature


ngx_feature="TCP_FASTOPEN"
ngx_feature_name="NGX_HAVE_TCP_FASTOPEN"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>
                  #include <netinet/tcp.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_TCP, TCP_FASTOPEN, NULL, 0)"
. auto/feature


ngx_feature="TCP_INFO"
ngx_feature_name="NGX_HAVE_TCP_INFO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>
                  #include <netinet/tcp.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="socklen_t optlen = sizeof(struct tcp_info);
                  struct tcp_info ti;
                  ti.tcpi_rtt = 0;
                  ti.tcpi_rttvar = 0;
                  ti.tcpi_snd_cwnd = 0;
                  ti.tcpi_rcv_space = 0;
                  getsockopt(0, IPPROTO_TCP, TCP_INFO, &ti, &optlen)"
. auto/feature


ngx_feature="accept4()"
ngx_feature_name="NGX_HAVE_ACCEPT4"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="accept4(0, NULL, NULL, SOCK_NONBLOCK)"
. auto/feature

if [ $NGX_FILE_AIO = YES ]; then

    ngx_feature="kqueue AIO support"
    ngx_feature_name="NGX_HAVE_FILE_AIO"
    ngx_feature_run=no
    ngx_feature_incs="#include <aio.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="int  n; struct aiocb  iocb;
                      iocb.aio_sigevent.sigev_notify = SIGEV_KEVENT;
                      n = aio_read(&iocb)"
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_SRCS="$CORE_SRCS $FILE_AIO_SRCS"
    fi

    if [ $ngx_found = no ]; then

        ngx_feature="Linux AIO support"
        ngx_feature_name="NGX_HAVE_FILE_AIO"
        ngx_feature_run=no
        ngx_feature_incs="#include <linux/aio_abi.h>
                          #include <sys/eventfd.h>"
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test="struct iocb  iocb;
                          iocb.aio_lio_opcode = IOCB_CMD_PREAD;
                          iocb.aio_flags = IOCB_FLAG_RESFD;
                          iocb.aio_resfd = -1;
                          (void) eventfd(0, 0)"
        . auto/feature

        if [ $ngx_found = yes ]; then
            have=NGX_HAVE_EVENTFD . auto/have
            have=NGX_HAVE_SYS_EVENTFD_H . auto/have
            CORE_SRCS="$CORE_SRCS $LINUX_AIO_SRCS"
        fi
    fi

    if [ $ngx_found = no ]; then

        ngx_feature="Linux AIO support (SYS_eventfd)"
        ngx_feature_incs="#include <linux/aio_abi.h>
                          #include <sys/syscall.h>"
        ngx_feature_test="int  n = SYS_eventfd;
                          struct iocb  iocb;
                          iocb.aio_lio_opcode = IOCB_CMD_PREAD;
                          iocb.aio_flags = IOCB_FLAG_RESFD;
                          iocb.aio_resfd = -1;"
        . auto/feature

        if [ $ngx_found = yes ]; then
            have=NGX_HAVE_EVENTFD . auto/have
            CORE_SRCS="$CORE_SRCS $LINUX_AIO_SRCS"
        fi
    fi

    if [ $ngx_found = no ]; then
        cat << END

$0: no supported file AIO was found
Currently file AIO is supported on FreeBSD 4.3+ and Linux 2.6.22+ only

END
        exit 1
    fi

else

    ngx_feature="eventfd()"
    ngx_feature_name="NGX_HAVE_EVENTFD"
    ngx_feature_run=no
    ngx_feature_incs="#include <sys/eventfd.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="(void) eventfd(0, 0)"
    . auto/feature

    if [ $ngx_found = yes ]; then
        have=NGX_HAVE_SYS_EVENTFD_H . auto/have
    fi

    if [ $ngx_found = no ]; then

        ngx_feature="eventfd() (SYS_eventfd)"
        ngx_feature_incs="#include <sys/syscall.h>"
        ngx_feature_test="int n = SYS_eventfd"
        . auto/feature
    fi
fi


have=NGX_HAVE_UNIX_DOMAIN . auto/have

ngx_feature_libs=


# C types

ngx_type="int"; . auto/types/sizeof

ngx_type="long"; . auto/types/sizeof

ngx_type="long long"; . auto/types/sizeof

ngx_type="void *"; . auto/types/sizeof; ngx_ptr_size=$ngx_size
ngx_param=NGX_PTR_SIZE; ngx_value=$ngx_size; . auto/types/value


# POSIX types

NGX_INCLUDE_AUTO_CONFIG_H="#include \"ngx_auto_config.h\""

ngx_type="uint32_t"; ngx_types="u_int32_t"; . auto/types/typedef
ngx_type="uint64_t"; ngx_types="u_int64_t"; . auto/types/typedef

ngx_type="sig_atomic_t"; ngx_types="int"; . auto/types/typedef
. auto/types/sizeof
ngx_param=NGX_SIG_ATOMIC_T_SIZE; ngx_value=$ngx_size; . auto/types/value

ngx_type="socklen_t"; ngx_types="int"; . auto/types/typedef

ngx_type="in_addr_t"; ngx_types="uint32_t u_int32_t"; . auto/types/typedef

ngx_type="in_port_t"; ngx_types="u_short"; . auto/types/typedef

ngx_type="rlim_t"; ngx_types="int"; . auto/types/typedef

. auto/types/uintptr_t

. auto/endianness

ngx_type="size_t"; . auto/types/sizeof
ngx_param=NGX_MAX_SIZE_T_VALUE; ngx_value=$ngx_max_value; . auto/types/value
ngx_param=NGX_SIZE_T_LEN; ngx_value=$ngx_max_len; . auto/types/value

ngx_type="off_t"; . auto/types/sizeof
ngx_param=NGX_MAX_OFF_T_VALUE; ngx_value=$ngx_max_value; . auto/types/value
ngx_param=NGX_OFF_T_LEN; ngx_value=$ngx_max_len; . auto/types/value

ngx_type="time_t"; . auto/types/sizeof
ngx_param=NGX_TIME_T_SIZE; ngx_value=$ngx_size; . auto/types/value
ngx_param=NGX_TIME_T_LEN; ngx_value=$ngx_max_len; . auto/types/value
ngx_param=NGX_MAX_TIME_T_VALUE; ngx_value=$ngx_max_value; . auto/types/value


# syscalls, libc calls and some features


if [ $NGX_IPV6 = YES ]; then
    ngx_feature="AF_INET6"
    ngx_feature_name="NGX_HAVE_INET6"
    ngx_feature_run=no
    ngx_feature_incs="#include <sys/socket.h>
                      #include <netinet/in.h>
                      #include <arpa/inet.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="struct sockaddr_in6  sin6;
                      sin6.sin6_family = AF_INET6;"
    . auto/feature
fi


ngx_feature="setproctitle()"
ngx_feature_name="NGX_HAVE_SETPROCTITLE"
ngx_feature_run=no
ngx_feature_incs="#include <stdlib.h>"
ngx_feature_path=
ngx_feature_libs=$NGX_SETPROCTITLE_LIB
ngx_feature_test="setproctitle(\"test\");"
. auto/feature


ngx_feature="pread()"
ngx_feature_name="NGX_HAVE_PREAD"
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="char buf[1]; ssize_t n; n = pread(0, buf, 1, 0);
                  if (n == -1) return 1"
. auto/feature


ngx_feature="pwrite()"
ngx_feature_name="NGX_HAVE_PWRITE"
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="char buf[1]; ssize_t n; n = pwrite(1, buf, 1, 0);
                  if (n == -1) return 1"
. auto/feature


# pwritev() was introduced in FreeBSD 6 and Linux 2.6.30, glibc 2.10

ngx_feature="pwritev()"
ngx_feature_name="NGX_HAVE_PWRITEV"
ngx_feature_run=no
ngx_feature_incs='#include <sys/uio.h>'
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="char buf[1]; struct iovec vec[1]; ssize_t n;
                  vec[0].iov_base = buf;
                  vec[0].iov_len = 1;
                  n = pwritev(1, vec, 1, 0);
                  if (n == -1) return 1"
. auto/feature


ngx_feature="sys_nerr"
ngx_feature_name="NGX_SYS_NERR"
ngx_feature_run=value
ngx_feature_incs='#include <errno.h>
                  #include <stdio.h>'
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test='printf("%d", sys_nerr);'
. auto/feature


if [ $ngx_found = no ]; then

    # Cygiwn defines _sys_nerr
    ngx_feature="_sys_nerr"
    ngx_feature_name="NGX_SYS_NERR"
    ngx_feature_run=value
    ngx_feature_incs='#include <errno.h>
                      #include <stdio.h>'
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test='printf("%d", _sys_nerr);'
    . auto/feature
fi


if [ $ngx_found = no ]; then

    # Solaris has no sys_nerr
    ngx_feature='maximum errno'
    ngx_feature_name=NGX_SYS_NERR
    ngx_feature_run=value
    ngx_feature_incs='#include <errno.h>
                      #include <string.h>
                      #include <stdio.h>'
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test='int  n;
                      char *p;
                      for (n = 1; n < 1000; n++) {
                          errno = 0;
                          p = strerror(n);
                          if (errno == EINVAL
                              || p == NULL
                              || strncmp(p, "Unknown error", 13) == 0)
                          {
                              break;
                          }
                      }
                      printf("%d", n);'
    . auto/feature
fi


ngx_feature="localtime_r()"
ngx_feature_name="NGX_HAVE_LOCALTIME_R"
ngx_feature_run=no
ngx_feature_incs="#include <time.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct tm t; time_t c=0; localtime_r(&c, &t)"
. auto/feature


ngx_feature="posix_memalign()"
ngx_feature_name="NGX_HAVE_POSIX_MEMALIGN"
ngx_feature_run=no
ngx_feature_incs="#include <stdlib.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="void *p; int n; n = posix_memalign(&p, 4096, 4096);
                  if (n != 0) return 1"
. auto/feature


ngx_feature="memalign()"
ngx_feature_name="NGX_HAVE_MEMALIGN"
ngx_feature_run=no
ngx_feature_incs="#include <stdlib.h>
                  #include <malloc.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="void *p; p = memalign(4096, 4096);
                  if (p == NULL) return 1"
. auto/feature


ngx_feature="mmap(MAP_ANON|MAP_SHARED)"
ngx_feature_name="NGX_HAVE_MAP_ANON"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/mman.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="void *p;
                  p = mmap(NULL, 4096, PROT_READ|PROT_WRITE,
                           MAP_ANON|MAP_SHARED, -1, 0);
                  if (p == MAP_FAILED) return 1;"
. auto/feature


ngx_feature='mmap("/dev/zero", MAP_SHARED)'
ngx_feature_name="NGX_HAVE_MAP_DEVZERO"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/mman.h>
                  #include <sys/stat.h>
                  #include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test='void *p; int  fd;
                  fd = open("/dev/zero", O_RDWR);
                  p = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
                  if (p == MAP_FAILED) return 1;'
. auto/feature


ngx_feature="System V shared memory"
ngx_feature_name="NGX_HAVE_SYSVSHM"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/ipc.h>
                  #include <sys/shm.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int  id;
                  id = shmget(IPC_PRIVATE, 4096, (SHM_R|SHM_W|IPC_CREAT));
                  if (id == -1) return 1;
                  shmctl(id, IPC_RMID, NULL);"
. auto/feature


ngx_feature="POSIX semaphores"
ngx_feature_name="NGX_HAVE_POSIX_SEM"
ngx_feature_run=yes
ngx_feature_incs="#include <semaphore.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="sem_t  sem;
                  if (sem_init(&sem, 1, 0) == -1) return 1;
                  sem_destroy(&sem);"
. auto/feature


if [ $ngx_found = no ]; then

    # Linux has POSIX semaphores in libpthread
    ngx_feature="POSIX semaphores in libpthread"
    ngx_feature_libs=-lpthread
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_LIBS="$CORE_LIBS -lpthread"
    fi
fi


if [ $ngx_found = no ]; then

    # Solaris has POSIX semaphores in librt
    ngx_feature="POSIX semaphores in librt"
    ngx_feature_libs=-lrt
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_LIBS="$CORE_LIBS -lrt"
    fi
fi


ngx_feature="struct msghdr.msg_control"
ngx_feature_name="NGX_HAVE_MSGHDR_MSG_CONTROL"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <stdio.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct msghdr  msg;
                  printf(\"%d\", (int) sizeof(msg.msg_control))"
. auto/feature


ngx_feature="ioctl(FIONBIO)"
ngx_feature_name="NGX_HAVE_FIONBIO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/ioctl.h>
                  #include <stdio.h>
                  $NGX_INCLUDE_SYS_FILIO_H"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int i = FIONBIO; printf(\"%d\", i)"
. auto/feature


ngx_feature="struct tm.tm_gmtoff"
ngx_feature_name="NGX_HAVE_GMTOFF"
ngx_feature_run=no
ngx_feature_incs="#include <time.h>
                  #include <stdio.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct tm  tm; tm.tm_gmtoff = 0;
                  printf(\"%d\", (int) tm.tm_gmtoff)"
. auto/feature


ngx_feature="struct dirent.d_namlen"
ngx_feature_name="NGX_HAVE_D_NAMLEN"
ngx_feature_run=no
ngx_feature_incs="#include <dirent.h>
                  #include <stdio.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct dirent  dir; dir.d_namlen = 0;
                  printf(\"%d\", (int) dir.d_namlen)"
. auto/feature


ngx_feature="struct dirent.d_type"
ngx_feature_name="NGX_HAVE_D_TYPE"
ngx_feature_run=no
ngx_feature_incs="#include <dirent.h>
                  #include <stdio.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct dirent  dir; dir.d_type = DT_REG;
                  printf(\"%d\", (int) dir.d_type)"
. auto/feature


ngx_feature="sysconf(_SC_NPROCESSORS_ONLN)"
ngx_feature_name="NGX_HAVE_SC_NPROCESSORS_ONLN"
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="sysconf(_SC_NPROCESSORS_ONLN)"
. auto/feature


ngx_feature="openat(), fstatat()"
ngx_feature_name="NGX_HAVE_OPENAT"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/stat.h>
                  #include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct stat sb;
                  openat(AT_FDCWD, \".\", O_RDONLY|O_NOFOLLOW);
                  fstatat(AT_FDCWD, \".\", &sb, AT_SYMLINK_NOFOLLOW);"
. auto/feature


ngx_feature="getaddrinfo()"
ngx_feature_name="NGX_HAVE_GETADDRINFO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/socket.h>
                  #include <netdb.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test='struct addrinfo *res;
                  if (getaddrinfo("localhost", NULL, NULL, &res) != 0) return 1;
                  freeaddrinfo(res)'
. auto/feature
{% endhighlight %}

如下我们对脚本进行分析：

## 2. 指定nginx进程user和group
{% highlight string %}
NGX_USER=${NGX_USER:-nobody}

if [ -z "$NGX_GROUP" ]; then
    if [ $NGX_USER = nobody ]; then
        if grep nobody /etc/group 2>&1 >/dev/null; then
            echo "checking for nobody group ... found"
            NGX_GROUP=nobody
        else
            echo "checking for nobody group ... not found"

            if grep nogroup /etc/group 2>&1 >/dev/null; then
                echo "checking for nogroup group ... found"
                NGX_GROUP=nogroup
            else
                echo "checking for nogroup group ... not found"
                NGX_GROUP=nobody
            fi
        fi
    else
        NGX_GROUP=$NGX_USER
    fi
fi
{% endhighlight %}
这里我们在编译时并没有通过```--user```与```--group```选项指定nginx进程运行时的所属用户和组。因此这里```NGX_USER```会被设置为 nobody, 而默认在ubuntu16.04系统中，/etc/group中也没有定义nobody组，但是定义了nogroup组，因此这里NGX_GROUP会被设置为 nogroup。

## 3. 特性检测

auto/unix脚本剩余部分都是进行特性检测。当前相关变量值如下：
<pre>
CC_TEST_FLAGS: 为空

CC_AUX_FLAGS: -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 （在auto/os/linux脚本最后设置）

NGX_TEST_LD_OPT: 为空
</pre>

**(1) poll特性检测**

检查系统是否支持poll函数：
{% highlight string %}
ngx_feature="poll()"
ngx_feature_name=
ngx_feature_run=no
ngx_feature_incs="#include <poll.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int  n; struct pollfd  pl;
                  pl.fd = 0;
                  pl.events = 0;
                  pl.revents = 0;
                  n = poll(&pl, 1, 0);
                  if (n == -1) return 1"
. auto/feature

if [ $ngx_found = no ]; then
    EVENT_POLL=NONE
fi
{% endhighlight %}

注意这里```EVENT_POLL```可以取：

* NO: 初始默认值，表示没有对poll特性进行手动设置
* YES: 通过```--with-poll_module```对poll特性进行了手动设置，会置为YES
* NONE: 通过```--without-poll_module```对poll特性进行了手动设置，或者这里检测没有poll特性，则会设置为NONE


后面auto/modules脚本中会根据这些值决定是否需要编译对应的源代码文件。


**(2) devpoll特性检测**

检测系统是否支持devpoll特性：
{% highlight string %}
ngx_feature="/dev/poll"
ngx_feature_name="NGX_HAVE_DEVPOLL"
ngx_feature_run=no
ngx_feature_incs="#include <sys/devpoll.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int  n, dp; struct dvpoll  dvp;
                  dp = 0;
                  dvp.dp_fds = NULL;
                  dvp.dp_nfds = 0;
                  dvp.dp_timeout = 0;
                  n = ioctl(dp, DP_POLL, &dvp);
                  if (n == -1) return 1"
. auto/feature

if [ $ngx_found = yes ]; then
    CORE_SRCS="$CORE_SRCS $DEVPOLL_SRCS"
    EVENT_MODULES="$EVENT_MODULES $DEVPOLL_MODULE"
    EVENT_FOUND=YES
fi
{% endhighlight %}
如果检测到有devpoll特性，则将```EVENT_FOUND```置为```YES```,表明当前已经找到了事件处理器。

devpoll是Solaris操作系统上的一种类似于epoll的机制。

**(3) 检测kqueue特性**

{% highlight string %}
if test -z "$NGX_KQUEUE_CHECKED"; then
    ngx_feature="kqueue"
    ngx_feature_name="NGX_HAVE_KQUEUE"
    ngx_feature_run=no
    ngx_feature_incs="#include <sys/event.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="int kq; kq = kqueue()"
    . auto/feature

    if [ $ngx_found = yes ]; then

        have=NGX_HAVE_CLEAR_EVENT . auto/have
        EVENT_MODULES="$EVENT_MODULES $KQUEUE_MODULE"
        CORE_SRCS="$CORE_SRCS $KQUEUE_SRCS"
        EVENT_FOUND=YES

        ngx_feature="kqueue's NOTE_LOWAT"
        ngx_feature_name="NGX_HAVE_LOWAT_EVENT"
        ngx_feature_run=no
        ngx_feature_incs="#include <sys/event.h>"
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test="struct kevent  kev;
                          kev.fflags = NOTE_LOWAT;"
        . auto/feature


        ngx_feature="kqueue's EVFILT_TIMER"
        ngx_feature_name="NGX_HAVE_TIMER_EVENT"
        ngx_feature_run=yes
        ngx_feature_incs="#include <sys/event.h>
                          #include <sys/time.h>"
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test="int      kq;
                  struct kevent    kev;
                  struct timespec  ts;

                  if ((kq = kqueue()) == -1) return 1;

                  kev.ident = 0;
                  kev.filter = EVFILT_TIMER;
                  kev.flags = EV_ADD|EV_ENABLE;
                  kev.fflags = 0;
                  kev.data = 1000;
                  kev.udata = 0;

                  ts.tv_sec = 0;
                  ts.tv_nsec = 0;

                  if (kevent(kq, &kev, 1, &kev, 1, &ts) == -1) return 1;

                  if (kev.flags & EV_ERROR) return 1;"

        . auto/feature
    fi
fi


if [ "$NGX_SYSTEM" = "NetBSD" ]; then

    # NetBSD 2.0 incompatibly defines kevent.udata as "intptr_t"

    cat << END >> $NGX_AUTO_CONFIG_H

#define NGX_KQUEUE_UDATA_T

END

else
    cat << END >> $NGX_AUTO_CONFIG_H

#define NGX_KQUEUE_UDATA_T  (void *)

END

fi
{% endhighlight %}

kqueue是一个可伸缩的事件通知接口，从FreeBSD 4.1开始引入，现在NetBSD、OpenBSD、DragonflyBSD和OSX系统上都支持。上面首先检测系统是否支持kqueue事件，如果支持再检测是否支持```EVFILT_TIMER```。

最后再向objs/ngx_auto_config.h头文件添加```NGX_KQUEUE_UDATA_T```宏定义，以兼容不同的版本之间的区别。


**(4) 检测是否支持crypt()特性**
{% highlight string %}
ngx_feature="crypt()"
ngx_feature_name=
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="crypt(\"test\", \"salt\");"
. auto/feature


if [ $ngx_found = no ]; then

    ngx_feature="crypt() in libcrypt"
    ngx_feature_name=
    ngx_feature_run=no
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=-lcrypt
    . auto/feature

    if [ $ngx_found = yes ]; then
        CRYPT_LIB="-lcrypt"
    fi
fi
{% endhighlight %}
crypt()采用DES对称加密算法对密码进行加密。检测如果支持，这将```CRYPT_LIB```置为 -lcrypt。

**(5) 检查是否支持F_READAHEAD特性**
{% highlight string %}
ngx_feature="F_READAHEAD"
ngx_feature_name="NGX_HAVE_F_READAHEAD"
ngx_feature_run=no
ngx_feature_incs="#include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="fcntl(0, F_READAHEAD, 1);"
. auto/feature
{% endhighlight %}

Linux的文件预读readahead，指Linux系统内核将指定文件的某区域预读进页缓存起来，便于接下来对该区域进行读取时，不会因区域(page fault)而阻塞。因为从内存读取比从磁盘读取要快很多。预读可以有效的减少磁盘的寻道次数和应用程序的I/O等待时间，是改进磁盘读I/O性能的重要优化手段之一。

**(6) 检测是否支持posix_fadvise()特性**
{% highlight string %}
ngx_feature="posix_fadvise()"
ngx_feature_name="NGX_HAVE_POSIX_FADVISE"
ngx_feature_run=no
ngx_feature_incs="#include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="posix_fadvise(0, 0, 0, POSIX_FADV_SEQUENTIAL);"
. auto/feature
{% endhighlight %}
对于posix_fadvise的解释如下：
<pre>
Programs can use posix_fadvise() to announce an intention to access file data in a specific pattern in the future,
thus allowing the kernel to perform appropriate optimizations.
</pre>

**(7) 检测是否支持O_DIRECT特性**
{% highlight string %}
ngx_feature="O_DIRECT"
ngx_feature_name="NGX_HAVE_O_DIRECT"
ngx_feature_run=no
ngx_feature_incs="#include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="fcntl(0, F_SETFL, O_DIRECT);"
. auto/feature


if [ $ngx_found = yes -a "$NGX_SYSTEM" = "Linux" ]; then
    have=NGX_HAVE_ALIGNED_DIRECTIO . auto/have
fi
{% endhighlight %}
对于```O_DIRECT```的解释如下：
<pre>
Try to minimize cache effects of the I/O to and from this file.  In general this
will degrade performance, but it is useful in special situations, such  as  when
applications do their own caching.  File I/O is done directly to/from user space
buffers.  The I/O is synchronous, that is, at the completion  of  a  read(2)  or
write(2), data is guaranteed to have been transferred.  See NOTES below for 
further discussion.
</pre>
Linux操作系统下使用```O_DIRECT```特性时还有一些对齐要求。


**(8) 检测是否支持F_NOCACHE特性**
{% highlight string %}
ngx_feature="F_NOCACHE"
ngx_feature_name="NGX_HAVE_F_NOCACHE"
ngx_feature_run=no
ngx_feature_incs="#include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="fcntl(0, F_NOCACHE, 1);"
. auto/feature
{% endhighlight %}
```F_NOCACHE```表示禁止使用缓存。


**(9) 是否支持directio()特性**
{% highlight string %}
ngx_feature="directio()"
ngx_feature_name="NGX_HAVE_DIRECTIO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="directio(0, DIRECTIO_ON);"
. auto/feature
{% endhighlight %}

其与```O_DIRECT```类似，只是这里判断是否直接支持directio()函数。directio()不使用操作系统缓存，使得磁盘IO(或者DMA)直接将数据存入用户空间的buffer。避免内核缓冲的内存消耗与CPU拷贝(数据从内核空间到用户空间的拷贝）的消耗。

DirectIO使用场景：DirectIO要读取大文件，因为每次都要初始化DMA；如果是读取小文件，初始化DMA花费的时间比系统读小文件的时间还长，所以小文件使用directIO没有优势。对于大文件也只是在只读一次，并且后续没有其他应用再次读取此文件的时候，才能有优势，如果后续还有其他应用需要使用，这个时候DirectIO也没有优势。

directio实际上有几方面的优势，不使用系统缓存一方面，另一方面是使用dma直接由dma控制从内存输入到用户空间的buffer中不经过cpu做mov操作，不消耗cpu。


**(10) 检测是否支持statfs()特性**
{% highlight string %}
ngx_feature="statfs()"
ngx_feature_name="NGX_HAVE_STATFS"
ngx_feature_run=no
ngx_feature_incs="$NGX_INCLUDE_SYS_PARAM_H
                  $NGX_INCLUDE_SYS_MOUNT_H
                  $NGX_INCLUDE_SYS_VFS_H"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct statfs  fs;
                  statfs(\".\", &fs);"
. auto/feature
{% endhighlight %}
statfs()查询文件系统相关信息。

**(11) 检测是否支持statvfs()特性**
{% highlight string %}
ngx_feature="statvfs()"
ngx_feature_name="NGX_HAVE_STATVFS"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/statvfs.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct statvfs  fs;
                  statvfs(\".\", &fs);"
. auto/feature
{% endhighlight %}
关于statfs()与statvfs()如此相似，主要是有如下“历史原因”：
<pre>
Originally 4.4BSD defined a statfs() call. Linux later implemented a slightly different call with the same name. Posix standardized it between all freenix and Unix versions by defining statvfs().

statfs() is OS-specific

statvfs() is posix-conforming

As they all return slightly different structures, later ones to come along can't replace the first.

In general you should use statvfs(), the Posix one. Be careful about "use Posix" advice, though, as in some cases (pty, for example) the BSD (or whatever) one is more portable in practice.
</pre>

**(12) 检查是否支持dlopen()特性**
{% highlight string %}
ngx_feature="dlopen()"
ngx_feature_name="NGX_HAVE_DLOPEN"
ngx_feature_run=no
ngx_feature_incs="#include <dlfcn.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="dlopen(NULL, RTLD_NOW | RTLD_GLOBAL); dlsym(NULL, NULL)"
. auto/feature


if [ $ngx_found != yes ]; then

    ngx_feature="dlopen() in libdl"
    ngx_feature_libs="-ldl"
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_LIBS="$CORE_LIBS -ldl"
        NGX_LIBDL="-ldl"
    fi
fi
{% endhighlight %}

dlopen()用于加载动态链接库，并且返回一个表示该动态链接库的句柄。

**(13) 检查是否支持sched_yield()特性**
{% highlight string %}
ngx_feature="sched_yield()"
ngx_feature_name="NGX_HAVE_SCHED_YIELD"
ngx_feature_run=no
ngx_feature_incs="#include <sched.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="sched_yield()"
. auto/feature


if [ $ngx_found != yes ]; then

    ngx_feature="sched_yield() in librt"
    ngx_feature_libs="-lrt"
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_LIBS="$CORE_LIBS -lrt"
    fi
fi
{% endhighlight %}

sche_yield()导致调用线程让出CPU，该线程会根据其优先级移动到线程队列末尾，然后一个新的线程被开始调用。

**(14) 检查是否支持SO_SETFIB特性**
{% highlight string %}
ngx_feature="SO_SETFIB"
ngx_feature_name="NGX_HAVE_SETFIB"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, SOL_SOCKET, SO_SETFIB, NULL, 0)"
. auto/feature
{% endhighlight %}

参看:[FreeBSD Manual Pages](https://www.freebsd.org/cgi/man.cgi?query=setsockopt&sektion=2)

该选项在FreeBSD上被引入，主要作用是：
<pre>
set the associated FIB (routing table) for the socket (set only)
</pre>

**(15) 检查是否支持SO_REUSEPORT特性**
{% highlight string %}
ngx_feature="SO_REUSEPORT"
ngx_feature_name="NGX_HAVE_REUSEPORT"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, SOL_SOCKET, SO_REUSEPORT, NULL, 0)"
. auto/feature
{% endhighlight %}
主要作用：
<pre>
enables duplicate address and port bindings
</pre>

**(16) 检查是否支持SO_ACCEPTFILTER特性**
{% highlight string %}
ngx_feature="SO_ACCEPTFILTER"
ngx_feature_name="NGX_HAVE_DEFERRED_ACCEPT"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, SOL_SOCKET, SO_ACCEPTFILTER, NULL, 0)"
. auto/feature
{% endhighlight %}
主要作用为：```set accept filter	on listening socket```

**(17) 检查是否支持IP_RECVDSTADDR特性**
{% highlight string %}
# BSD way to get IPv4 datagram destination address

ngx_feature="IP_RECVDSTADDR"
ngx_feature_name="NGX_HAVE_IP_RECVDSTADDR"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_IP, IP_RECVDSTADDR, NULL, 0)"
. auto/feature
{% endhighlight %}

该选项导致所接收到的UDP数据报的目的IP地址由函数recvmsg作为辅助数据返回。主要是在BSD操作系统上使用。

**(18) 检查是否支持IP_PKTINFO特性**
{% highlight string %}
# Linux way to get IPv4 datagram destination address

ngx_feature="IP_PKTINFO"
ngx_feature_name="NGX_HAVE_IP_PKTINFO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_IP, IP_PKTINFO, NULL, 0)"
. auto/feature
{% endhighlight %}
该选项导致所接收到的UDP数据包的目的IP地址由函数recvmsg作为辅助数据返回。主要是在Linux操作系统上使用。

参看: [IP套接口选项（转）](http://blog.chinaunix.net/uid-20249205-id-1713888.html)


**(19) 检查是否支持IPV6_RECVPKTINFO特性**
{% highlight string %}
# RFC 3542 way to get IPv6 datagram destination address

ngx_feature="IPV6_RECVPKTINFO"
ngx_feature_name="NGX_HAVE_IPV6_RECVPKTINFO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_IPV6, IPV6_RECVPKTINFO, NULL, 0)"
. auto/feature
{% endhighlight %}
设置在接收到的数据报中添加```IPV6_PKTINFO```控制信息。这些控制信息包含在一个struct in6_pktinfo的机构中（请参看RFC 3542),只允许SOCK_DGRAM和SOCK_RAW socket使用。它是从Linux 2.6.14开始引入。

请参看：[Linux Programmer's Manual](http://www.man7.org/linux/man-pages/man7/ipv6.7.html)


**(20) 检查是否支持TCP_DEFER_ACCEPT特性**
{% highlight string %}
ngx_feature="TCP_DEFER_ACCEPT"
ngx_feature_name="NGX_HAVE_DEFERRED_ACCEPT"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>
                  #include <netinet/tcp.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_TCP, TCP_DEFER_ACCEPT, NULL, 0)"
. auto/feature
{% endhighlight %}

对于```TCP_DEFER_ACCEPT```的解释：
<pre>TCP_DEFER_ACCEPT (since Linux 2.4)
Allow a listener to be awakened only when data arrives on the socket. Takes an integer value (seconds), 
this can bound the maximum number of attempts TCP will make to complete the connection. 
This option should not be used in code intended to be portable. 
</pre>

参看：[Linux TCP_DEFER_ACCEPT的作用](http://blog.csdn.net/for_tech/article/details/54175571)


**(21) 检查是否支持TCP_KEEPIDLE特性**
{% highlight string %}
ngx_feature="TCP_KEEPIDLE"
ngx_feature_name="NGX_HAVE_KEEPALIVE_TUNABLE"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>
                  #include <netinet/tcp.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_TCP, TCP_KEEPIDLE, NULL, 0);
                  setsockopt(0, IPPROTO_TCP, TCP_KEEPINTVL, NULL, 0);
                  setsockopt(0, IPPROTO_TCP, TCP_KEEPCNT, NULL, 0)"
. auto/feature
{% endhighlight %}

* TCP_KEEPIDLE: 对一个连接进行有效性探测之前运行的最大非活跃时间间隔，默认值为7200(即2个小时）
* TCP_KEEPINTVL: 两个探测的时间间隔，默认值为75，即75秒。
* TCP_KEEPCNT: 关闭一个非活跃连接之前，默认值为9次。

参看：

1. [TCP keep-alive的三个参数](http://blog.csdn.net/hengyunabc/article/details/44310193)

2. [TCP Keepalive HOWTO](http://www.tldp.org/HOWTO/html_single/TCP-Keepalive-HOWTO/)


**(22) 检测是否支持TCP_FASTOPEN特性**
{% highlight string %}
ngx_feature="TCP_FASTOPEN"
ngx_feature_name="NGX_HAVE_TCP_FASTOPEN"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>
                  #include <netinet/tcp.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="setsockopt(0, IPPROTO_TCP, TCP_FASTOPEN, NULL, 0)"
. auto/feature
{% endhighlight %}
参看：

1. [Linux TCP_FASTOPEN的作用](http://blog.csdn.net/for_tech/article/details/54237556)

2. [TCP Fast Open](https://en.wikipedia.org/wiki/TCP_Fast_Open)

**(23) 检测是否支持TCP_INFO特性**
{% highlight string %}
ngx_feature="TCP_INFO"
ngx_feature_name="NGX_HAVE_TCP_INFO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <netinet/in.h>
                  #include <netinet/tcp.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="socklen_t optlen = sizeof(struct tcp_info);
                  struct tcp_info ti;
                  ti.tcpi_rtt = 0;
                  ti.tcpi_rttvar = 0;
                  ti.tcpi_snd_cwnd = 0;
                  ti.tcpi_rcv_space = 0;
                  getsockopt(0, IPPROTO_TCP, TCP_INFO, &ti, &optlen)"
. auto/feature
{% endhighlight %}
在内核的函数tcp_getsockopt的代码中，可以看到这个选项TCP_INFO，返回了几乎所有的参数，同时还有其他的许多参数可以得到一些其他的信息。具体每个参数的含义可以参考内核中的注释.

参看：[打印输出tcp拥塞窗口](http://www.cnblogs.com/mydomain/archive/2013/04/18/3027664.html)


**(24) 检测是否支持ACCEPT4特性**
{% highlight string %}
ngx_feature="accept4()"
ngx_feature_name="NGX_HAVE_ACCEPT4"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="accept4(0, NULL, NULL, SOCK_NONBLOCK)"
. auto/feature
{% endhighlight %}
accept4()是属于```_GNU_SOURCE```的一个增强。
<pre>
If flags is 0, then accept4() is the same as accept().  The following values can be bitwise ORed in flags to obtain different behavior:

SOCK_NONBLOCK   Set the O_NONBLOCK file status flag on the new open file description.  Using this flag saves extra  calls  to  fcntl(2)  to  achieve  the  same result.

SOCK_CLOEXEC    Set  the close-on-exec (FD_CLOEXEC) flag on the new file descriptor.  See the description of the O_CLOEXEC flag in open(2) for reasons why this may be useful.
</pre>



**(25) 检测是否支持FILE_AIO特性**
{% highlight string %}
if [ $NGX_FILE_AIO = YES ]; then

    ngx_feature="kqueue AIO support"
    ngx_feature_name="NGX_HAVE_FILE_AIO"
    ngx_feature_run=no
    ngx_feature_incs="#include <aio.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="int  n; struct aiocb  iocb;
                      iocb.aio_sigevent.sigev_notify = SIGEV_KEVENT;
                      n = aio_read(&iocb)"
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_SRCS="$CORE_SRCS $FILE_AIO_SRCS"
    fi

    if [ $ngx_found = no ]; then

        ngx_feature="Linux AIO support"
        ngx_feature_name="NGX_HAVE_FILE_AIO"
        ngx_feature_run=no
        ngx_feature_incs="#include <linux/aio_abi.h>
                          #include <sys/eventfd.h>"
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test="struct iocb  iocb;
                          iocb.aio_lio_opcode = IOCB_CMD_PREAD;
                          iocb.aio_flags = IOCB_FLAG_RESFD;
                          iocb.aio_resfd = -1;
                          (void) eventfd(0, 0)"
        . auto/feature

        if [ $ngx_found = yes ]; then
            have=NGX_HAVE_EVENTFD . auto/have
            have=NGX_HAVE_SYS_EVENTFD_H . auto/have
            CORE_SRCS="$CORE_SRCS $LINUX_AIO_SRCS"
        fi
    fi

    if [ $ngx_found = no ]; then

        ngx_feature="Linux AIO support (SYS_eventfd)"
        ngx_feature_incs="#include <linux/aio_abi.h>
                          #include <sys/syscall.h>"
        ngx_feature_test="int  n = SYS_eventfd;
                          struct iocb  iocb;
                          iocb.aio_lio_opcode = IOCB_CMD_PREAD;
                          iocb.aio_flags = IOCB_FLAG_RESFD;
                          iocb.aio_resfd = -1;"
        . auto/feature

        if [ $ngx_found = yes ]; then
            have=NGX_HAVE_EVENTFD . auto/have
            CORE_SRCS="$CORE_SRCS $LINUX_AIO_SRCS"
        fi
    fi

    if [ $ngx_found = no ]; then
        cat << END

$0: no supported file AIO was found
Currently file AIO is supported on FreeBSD 4.3+ and Linux 2.6.22+ only

END
        exit 1
    fi

else

    ngx_feature="eventfd()"
    ngx_feature_name="NGX_HAVE_EVENTFD"
    ngx_feature_run=no
    ngx_feature_incs="#include <sys/eventfd.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="(void) eventfd(0, 0)"
    . auto/feature

    if [ $ngx_found = yes ]; then
        have=NGX_HAVE_SYS_EVENTFD_H . auto/have
    fi

    if [ $ngx_found = no ]; then

        ngx_feature="eventfd() (SYS_eventfd)"
        ngx_feature_incs="#include <sys/syscall.h>"
        ngx_feature_test="int n = SYS_eventfd"
        . auto/feature
    fi
fi
{% endhighlight %}

首先检查```NGX_FILE_AIO```有没有被设置,如果值为```YES```，则继续检查aio是否可用；否则检查是否支持eventfd().

（注：在auto/options脚本中，```NGX_FILE_AIO```默认被设置为no，可以通过```--with-file-aio```进行设置)


**(26) 设定UNIX_DOMAIN并清空ngx_feature_libs**
{% highlight string %}
have=NGX_HAVE_UNIX_DOMAIN . auto/have

ngx_feature_libs=
{% endhighlight %}



**(27) 检查C类型**
{% highlight string %}
# C types

ngx_type="int"; . auto/types/sizeof

ngx_type="long"; . auto/types/sizeof

ngx_type="long long"; . auto/types/sizeof

ngx_type="void *"; . auto/types/sizeof; ngx_ptr_size=$ngx_size
ngx_param=NGX_PTR_SIZE; ngx_value=$ngx_size; . auto/types/value
{% endhighlight %}
上述开头三行不会向配置文件写入任何内容，因此实际上不会产生任何效果。最后两行求得```void *```类型的长度，然后向objs/ngx_auto_config.h头文件写入```NGX_PTR_SIZE```宏定义。ngx_ptr_size变量会在auto/types/uintptr_t脚本中用到。



**(28) 检查posix类型**
{% highlight string %}
# POSIX types

NGX_INCLUDE_AUTO_CONFIG_H="#include \"ngx_auto_config.h\""

ngx_type="uint32_t"; ngx_types="u_int32_t"; . auto/types/typedef
ngx_type="uint64_t"; ngx_types="u_int64_t"; . auto/types/typedef

ngx_type="sig_atomic_t"; ngx_types="int"; . auto/types/typedef
. auto/types/sizeof
ngx_param=NGX_SIG_ATOMIC_T_SIZE; ngx_value=$ngx_size; . auto/types/value

ngx_type="socklen_t"; ngx_types="int"; . auto/types/typedef

ngx_type="in_addr_t"; ngx_types="uint32_t u_int32_t"; . auto/types/typedef

ngx_type="in_port_t"; ngx_types="u_short"; . auto/types/typedef

ngx_type="rlim_t"; ngx_types="int"; . auto/types/typedef

. auto/types/uintptr_t

. auto/endianness

ngx_type="size_t"; . auto/types/sizeof
ngx_param=NGX_MAX_SIZE_T_VALUE; ngx_value=$ngx_max_value; . auto/types/value
ngx_param=NGX_SIZE_T_LEN; ngx_value=$ngx_max_len; . auto/types/value

ngx_type="off_t"; . auto/types/sizeof
ngx_param=NGX_MAX_OFF_T_VALUE; ngx_value=$ngx_max_value; . auto/types/value
ngx_param=NGX_OFF_T_LEN; ngx_value=$ngx_max_len; . auto/types/value

ngx_type="time_t"; . auto/types/sizeof
ngx_param=NGX_TIME_T_SIZE; ngx_value=$ngx_size; . auto/types/value
ngx_param=NGX_TIME_T_LEN; ngx_value=$ngx_max_len; . auto/types/value
ngx_param=NGX_MAX_TIME_T_VALUE; ngx_value=$ngx_max_value; . auto/types/value
{% endhighlight %}

主要包括以下几个部分：

1) 定义 ```NGX_INCLUDE_AUTO_CONFIG_H```会在auto/types/sizeof脚本中使用到。

2) 一些数据类型的typedef定义检测(这里通过宏```NGX_SIG_ATOMIC_T_SIZE```来保存当前平台的机器字长）

3) 调用auto/types/uintptr_t脚本定义```uintptr_t```与```intptr_t```类型，其类型大小与当前机器字长相同。

4) 调用auto/endianness脚本，获得机器的大小端

5) 处理size_t，off_t,time_t类型


<br />



```如下主要是检测syscalls, libc calls and some features```

**(29) 检测是否支持IPv6**

检查是否支持IPv6 socket地址：
{% highlight string %}
if [ $NGX_IPV6 = YES ]; then
    ngx_feature="AF_INET6"
    ngx_feature_name="NGX_HAVE_INET6"
    ngx_feature_run=no
    ngx_feature_incs="#include <sys/socket.h>
                      #include <netinet/in.h>
                      #include <arpa/inet.h>"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="struct sockaddr_in6  sin6;
                      sin6.sin6_family = AF_INET6;"
    . auto/feature
fi
{% endhighlight %}


**(30) 检测是否支持setproctitle()**
{% highlight string %}
ngx_feature="setproctitle()"
ngx_feature_name="NGX_HAVE_SETPROCTITLE"
ngx_feature_run=no
ngx_feature_incs="#include <stdlib.h>"
ngx_feature_path=
ngx_feature_libs=$NGX_SETPROCTITLE_LIB
ngx_feature_test="setproctitle(\"test\");"
. auto/feature
{% endhighlight %}
setproctitle()用于修改一个进程的名称，其最开始是在FreeBSD 2.2开始引入的。


**(31) 检测是否支持pread()**
{% highlight string %}
ngx_feature="pread()"
ngx_feature_name="NGX_HAVE_PREAD"
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="char buf[1]; ssize_t n; n = pread(0, buf, 1, 0);
                  if (n == -1) return 1"
. auto/feature
{% endhighlight %}
pread()从一个文件中读取指定数量的数据到buffer缓存中。pread()系统调用在多线程应用程序中很有作用，它允许多线程同时对同一个文件句柄fd进行IO操作，而并不会影响到其他线程中该文件的offset。 注意fd所引用的文件必须是```seekable```的.

pread()函数需要：
<pre>
Feature Test Macro Requirements for glibc (see feature_test_macros(7)):

       pread(), pwrite():
           _XOPEN_SOURCE >= 500
           || /* Since glibc 2.12: */ _POSIX_C_SOURCE >= 200809L
</pre>
默认情况下gcc编译器已经定义```_POSIX_C_SOURCE=200809L```,因此这里可以直接使用.

参看：[FEATURE_TEST_MACRO(7)](http://www.man7.org/linux/man-pages/man7/feature_test_macros.7.html)


**（32） 检测是否支持pwrite()**
{% highlight string %}
ngx_feature="pwrite()"
ngx_feature_name="NGX_HAVE_PWRITE"
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="char buf[1]; ssize_t n; n = pwrite(1, buf, 1, 0);
                  if (n == -1) return 1"
. auto/feature
{% endhighlight %}

pwrite()将buffer中的缓存数据写入到文件中。pwrite()系统调用在多线程应用程序中很有作用，它允许多线程同时对同一个文件句柄fd进行IO操作，而并不会影响到其他线程中该文件的offset。注意fd所引用的文件必须是```seekable```的.


**(33) 检测是否支持pwritev()**
{% highlight string %}
# pwritev() was introduced in FreeBSD 6 and Linux 2.6.30, glibc 2.10

ngx_feature="pwritev()"
ngx_feature_name="NGX_HAVE_PWRITEV"
ngx_feature_run=no
ngx_feature_incs='#include <sys/uio.h>'
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="char buf[1]; struct iovec vec[1]; ssize_t n;
                  vec[0].iov_base = buf;
                  vec[0].iov_len = 1;
                  n = pwritev(1, vec, 1, 0);
                  if (n == -1) return 1"
. auto/feature
{% endhighlight %}
与pwrite()类似，但是可以支持分散写操作。

**(34) 检查是否支持SYS_NERR特性**
{% highlight string %}
ngx_feature="sys_nerr"
ngx_feature_name="NGX_SYS_NERR"
ngx_feature_run=value
ngx_feature_incs='#include <errno.h>
                  #include <stdio.h>'
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test='printf("%d", sys_nerr);'
. auto/feature


if [ $ngx_found = no ]; then

    # Cygiwn defines _sys_nerr
    ngx_feature="_sys_nerr"
    ngx_feature_name="NGX_SYS_NERR"
    ngx_feature_run=value
    ngx_feature_incs='#include <errno.h>
                      #include <stdio.h>'
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test='printf("%d", _sys_nerr);'
    . auto/feature
fi


if [ $ngx_found = no ]; then

    # Solaris has no sys_nerr
    ngx_feature='maximum errno'
    ngx_feature_name=NGX_SYS_NERR
    ngx_feature_run=value
    ngx_feature_incs='#include <errno.h>
                      #include <string.h>
                      #include <stdio.h>'
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test='int  n;
                      char *p;
                      for (n = 1; n < 1000; n++) {
                          errno = 0;
                          p = strerror(n);
                          if (errno == EINVAL
                              || p == NULL
                              || strncmp(p, "Unknown error", 13) == 0)
                          {
                              break;
                          }
                      }
                      printf("%d", n);'
    . auto/feature
fi
{% endhighlight %}

sys_nerr为当前系统定义的error个数。
参看：[sys_nerr(3)](https://linux.die.net/man/3/sys_nerr)

**(35) 检查是否支持localtime_r()**
{% highlight string %}
ngx_feature="localtime_r()"
ngx_feature_name="NGX_HAVE_LOCALTIME_R"
ngx_feature_run=no
ngx_feature_incs="#include <time.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct tm t; time_t c=0; localtime_r(&c, &t)"
. auto/feature
{% endhighlight %}
localtime_r()与localtime()类似，但是前者是线程安全的，而后者由于将数据保存在一个全局struct tm类型的静态变量中，因此是非线程安全的。

**(36) 检查是否支持posix_memalign()**
{% highlight string %}
ngx_feature="posix_memalign()"
ngx_feature_name="NGX_HAVE_POSIX_MEMALIGN"
ngx_feature_run=no
ngx_feature_incs="#include <stdlib.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="void *p; int n; n = posix_memalign(&p, 4096, 4096);
                  if (n != 0) return 1"
. auto/feature
{% endhighlight %}
posix_memalign()分配一个指定字节对其的内存：
<pre>
int posix_memalign(void **memptr, size_t alignment, size_t size);

The  function  posix_memalign()  allocates size bytes and places the address of the allocated memory in *memptr.
The address of the allocated memory will be a multiple of alignment, which must be a power of two and a multiple
of sizeof(void *).  If size is 0, then posix_memalign() returns either  NULL,  or  a  unique pointer value that
can later be successfully passed to free(3).
</pre>


**(37) 检测是否支持memalign()**
{% highlight string %}
ngx_feature="memalign()"
ngx_feature_name="NGX_HAVE_MEMALIGN"
ngx_feature_run=no
ngx_feature_incs="#include <stdlib.h>
                  #include <malloc.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="void *p; p = memalign(4096, 4096);
                  if (p == NULL) return 1"
. auto/feature
{% endhighlight %}
与```posix_memalign()```类似，是一个obsolete function （即过时函数)。此外对于posix_memalign()函数，其会检查传入的alignment参数是否合法，而memalign()函数可能不会检查。

**(38) 检测是否支持MAP_ANON特性**
{% highlight string %}
ngx_feature="mmap(MAP_ANON|MAP_SHARED)"
ngx_feature_name="NGX_HAVE_MAP_ANON"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/mman.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="void *p;
                  p = mmap(NULL, 4096, PROT_READ|PROT_WRITE,
                           MAP_ANON|MAP_SHARED, -1, 0);
                  if (p == MAP_FAILED) return 1;"
. auto/feature
{% endhighlight %}
```MAP_ANON```参数与```MAP_ANONYMOUS```同义，映射一块共享内存，需与```MAP_SHARED```参数一起使用。

注意：```MAP_ANON```已过时。

**(39) 检查是否支持MAP_DEVZERO特性**
{% highlight string %}
ngx_feature='mmap("/dev/zero", MAP_SHARED)'
ngx_feature_name="NGX_HAVE_MAP_DEVZERO"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/mman.h>
                  #include <sys/stat.h>
                  #include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test='void *p; int  fd;
                  fd = open("/dev/zero", O_RDWR);
                  p = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
                  if (p == MAP_FAILED) return 1;'
. auto/feature
{% endhighlight %}
有些系统可能不支持上述```MAP_ANON```这样的匿名文件映射（这需要fd为-1)，此时就可以通过先用open()函数打开一个特殊文件/dev/zero，然后再进行映射，从而达到匿名映射的效果。

参看：[Linux 下的两个特殊的文件 -- /dev/null 和 /dev/zero 简介及对比](http://blog.csdn.net/pi9nc/article/details/18257593)

**(40) 检测是否支持SYSVSHM特性**
{% highlight string %}
ngx_feature="System V shared memory"
ngx_feature_name="NGX_HAVE_SYSVSHM"
ngx_feature_run=yes
ngx_feature_incs="#include <sys/ipc.h>
                  #include <sys/shm.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int  id;
                  id = shmget(IPC_PRIVATE, 4096, (SHM_R|SHM_W|IPC_CREAT));
                  if (id == -1) return 1;
                  shmctl(id, IPC_RMID, NULL);"
. auto/feature
{% endhighlight %}

检测是否支持System V 共享内存。

**(41) 检测是否支持POSIX_SEM特性**
{% highlight string %}
ngx_feature="POSIX semaphores"
ngx_feature_name="NGX_HAVE_POSIX_SEM"
ngx_feature_run=yes
ngx_feature_incs="#include <semaphore.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="sem_t  sem;
                  if (sem_init(&sem, 1, 0) == -1) return 1;
                  sem_destroy(&sem);"
. auto/feature


if [ $ngx_found = no ]; then

    # Linux has POSIX semaphores in libpthread
    ngx_feature="POSIX semaphores in libpthread"
    ngx_feature_libs=-lpthread
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_LIBS="$CORE_LIBS -lpthread"
    fi
fi


if [ $ngx_found = no ]; then

    # Solaris has POSIX semaphores in librt
    ngx_feature="POSIX semaphores in librt"
    ngx_feature_libs=-lrt
    . auto/feature

    if [ $ngx_found = yes ]; then
        CORE_LIBS="$CORE_LIBS -lrt"
    fi
fi
{% endhighlight %}

检测是否支持Posix信号量。一般如果操作系统没有提供独立的Posix信号量的话，那么在Linux操作系统下会放在libpthread库中，而在Solaris操作系统下会放在librt库中。

**(42) 检测是否支持MSGHDR_MSG_CONTROL特性**
{% highlight string %}
ngx_feature="struct msghdr.msg_control"
ngx_feature_name="NGX_HAVE_MSGHDR_MSG_CONTROL"
ngx_feature_run=no
ngx_feature_incs="#include <sys/socket.h>
                  #include <stdio.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct msghdr  msg;
                  printf(\"%d\", (int) sizeof(msg.msg_control))"
. auto/feature
{% endhighlight %}
msg_control用于存放辅助数据。

**(43) 检测是否支持FIONBIO特性**
{% highlight string %}
ngx_feature="ioctl(FIONBIO)"
ngx_feature_name="NGX_HAVE_FIONBIO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/ioctl.h>
                  #include <stdio.h>
                  $NGX_INCLUDE_SYS_FILIO_H"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="int i = FIONBIO; printf(\"%d\", i)"
. auto/feature
{% endhighlight %}
检测是否可以通过ioctl(FIONBIO)来设置文件IO非阻塞。

**(44) 检测是否支持GMTOFF特性**
{% highlight string %}
ngx_feature="struct tm.tm_gmtoff"
ngx_feature_name="NGX_HAVE_GMTOFF"
ngx_feature_run=no
ngx_feature_incs="#include <time.h>
                  #include <stdio.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct tm  tm; tm.tm_gmtoff = 0;
                  printf(\"%d\", (int) tm.tm_gmtoff)"
. auto/feature
{% endhighlight %}
tm_gmtoff指定了日期变更线东面时区中UTC东部时区正秒数或UTC西部时区的负秒数。

参看：[Linux系统中的时间处理](http://blog.csdn.net/cywosp/article/details/25839551)


**(45) 检测是否支持D_NAMLEN特性**
{% highlight string %}
ngx_feature="struct dirent.d_namlen"
ngx_feature_name="NGX_HAVE_D_NAMLEN"
ngx_feature_run=no
ngx_feature_incs="#include <dirent.h>
                  #include <stdio.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct dirent  dir; dir.d_namlen = 0;
                  printf(\"%d\", (int) dir.d_namlen)"
. auto/feature
{% endhighlight %}
检测是否具有d_namlen属性，可通过其返回d_name的长度。本属性并不是所有操作系统都支持


**（46） 检测是否支持D_TYPE特性**
{% highlight string %}
ngx_feature="struct dirent.d_type"
ngx_feature_name="NGX_HAVE_D_TYPE"
ngx_feature_run=no
ngx_feature_incs="#include <dirent.h>
                  #include <stdio.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct dirent  dir; dir.d_type = DT_REG;
                  printf(\"%d\", (int) dir.d_type)"
. auto/feature
{% endhighlight %}

检测是否有d_type属性。本属性并不是所有操作系统都支持。

**(47) 检测是否支持_SC_NPROCESSORS_ONLN特性**
{% highlight string %}
ngx_feature="sysconf(_SC_NPROCESSORS_ONLN)"
ngx_feature_name="NGX_HAVE_SC_NPROCESSORS_ONLN"
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="sysconf(_SC_NPROCESSORS_ONLN)"
. auto/feature
{% endhighlight %}

```_SC_NPROCESSORS_ONLN```返回当前可用的CPU核数。

**(48) 检测是否支持openat()**
{% highlight string %}
ngx_feature="openat(), fstatat()"
ngx_feature_name="NGX_HAVE_OPENAT"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/stat.h>
                  #include <fcntl.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="struct stat sb;
                  openat(AT_FDCWD, \".\", O_RDONLY|O_NOFOLLOW);
                  fstatat(AT_FDCWD, \".\", &sb, AT_SYMLINK_NOFOLLOW);"
. auto/feature
{% endhighlight %}
关于openat()的解释如下：
<pre>
The openat() system call operates in exactly the same way as open(2), except for the differences described in this manual page.

If  the pathname given in pathname is relative, then it is interpreted relative to the directory referred to by 
the file descriptor dirfd (rather than relative to the current working directory of the calling process, as is 
done by open(2) for a relative pathname).

If pathname is relative and dirfd is the special value AT_FDCWD, then pathname is interpreted relative to the
 current working directory of the calling  process(like open(2)).

If pathname is absolute, then dirfd is ignored.
</pre>


**(49) 检测是否支持getaddrinfo()**
{% highlight string %}
ngx_feature="getaddrinfo()"
ngx_feature_name="NGX_HAVE_GETADDRINFO"
ngx_feature_run=no
ngx_feature_incs="#include <sys/types.h>
                  #include <sys/socket.h>
                  #include <netdb.h>"
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test='struct addrinfo *res;
                  if (getaddrinfo("localhost", NULL, NULL, &res) != 0) return 1;
                  freeaddrinfo(res)'
. auto/feature
{% endhighlight %}

关于getaddrinfo()的解释如下：
<pre>
The getaddrinfo() function combines the functionality provided by  the  gethostbyname(3)and  getservbyname(3)
functions into a single interface, but unlike the latter functions, getaddrinfo() is reentrant and allows
programs to eliminate IPv4-ver,sus-IPv6 dependencies.
</pre>


## 4. 总结
本文整体篇幅较长，主要是处理了48个类Unix操作系统下的一些特性相关的问题。nginx良好的跨平台性、兼容性就有这些小细节的功劳。



<br />
<br />
<br />

