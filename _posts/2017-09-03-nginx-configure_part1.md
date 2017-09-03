---
layout: post
title: nginx编译脚本解析-part1
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

在编译安装nginx的时候，首先要执行configure配置脚本来进行安装的配置准备，包括环境检查及生成文件。nginx的高效与编译脚本的配置也有很大的关系，nginx的很多特性都由编译脚本控制生成，此外又由于编译脚本本身较为复杂，因此这里我们有必要对nginx编译脚本进行详细且深入的分析。这里我们的分析过程是按照脚本的执行顺序来进行的。

<!-- more -->

## 1. configure配置脚本
这里我们首先给出```configure```：
{% highlight string %}
#!/bin/sh

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


LC_ALL=C
export LC_ALL

. auto/options
. auto/init
. auto/sources

test -d $NGX_OBJS || mkdir -p $NGX_OBJS

echo > $NGX_AUTO_HEADERS_H
echo > $NGX_AUTOCONF_ERR

echo "#define NGX_CONFIGURE \"$NGX_CONFIGURE\"" > $NGX_AUTO_CONFIG_H


if [ $NGX_DEBUG = YES ]; then
    have=NGX_DEBUG . auto/have
fi


if test -z "$NGX_PLATFORM"; then
    echo "checking for OS"

    NGX_SYSTEM=`uname -s 2>/dev/null`
    NGX_RELEASE=`uname -r 2>/dev/null`
    NGX_MACHINE=`uname -m 2>/dev/null`

    echo " + $NGX_SYSTEM $NGX_RELEASE $NGX_MACHINE"

    NGX_PLATFORM="$NGX_SYSTEM:$NGX_RELEASE:$NGX_MACHINE";

    case "$NGX_SYSTEM" in
        MINGW32_*)
            NGX_PLATFORM=win32
        ;;
    esac

else
    echo "building for $NGX_PLATFORM"
    NGX_SYSTEM=$NGX_PLATFORM
fi

. auto/cc/conf

if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/headers
fi

. auto/os/conf

if [ "$NGX_PLATFORM" != win32 ]; then
    . auto/unix
fi

. auto/threads
. auto/modules
. auto/lib/conf

case ".$NGX_PREFIX" in
    .)
        NGX_PREFIX=${NGX_PREFIX:-/usr/local/nginx}
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;

    .!)
        NGX_PREFIX=
    ;;

    *)
        have=NGX_PREFIX value="\"$NGX_PREFIX/\"" . auto/define
    ;;
esac

if [ ".$NGX_CONF_PREFIX" != "." ]; then
    have=NGX_CONF_PREFIX value="\"$NGX_CONF_PREFIX/\"" . auto/define
fi

have=NGX_SBIN_PATH value="\"$NGX_SBIN_PATH\"" . auto/define
have=NGX_CONF_PATH value="\"$NGX_CONF_PATH\"" . auto/define
have=NGX_PID_PATH value="\"$NGX_PID_PATH\"" . auto/define
have=NGX_LOCK_PATH value="\"$NGX_LOCK_PATH\"" . auto/define
have=NGX_ERROR_LOG_PATH value="\"$NGX_ERROR_LOG_PATH\"" . auto/define

have=NGX_HTTP_LOG_PATH value="\"$NGX_HTTP_LOG_PATH\"" . auto/define
have=NGX_HTTP_CLIENT_TEMP_PATH value="\"$NGX_HTTP_CLIENT_TEMP_PATH\""
. auto/define
have=NGX_HTTP_PROXY_TEMP_PATH value="\"$NGX_HTTP_PROXY_TEMP_PATH\""
. auto/define
have=NGX_HTTP_FASTCGI_TEMP_PATH value="\"$NGX_HTTP_FASTCGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_UWSGI_TEMP_PATH value="\"$NGX_HTTP_UWSGI_TEMP_PATH\""
. auto/define
have=NGX_HTTP_SCGI_TEMP_PATH value="\"$NGX_HTTP_SCGI_TEMP_PATH\""
. auto/define

. auto/make
. auto/lib/make
. auto/install

# STUB
. auto/stubs

have=NGX_USER value="\"$NGX_USER\"" . auto/define
have=NGX_GROUP value="\"$NGX_GROUP\"" . auto/define

if [ ".$NGX_BUILD" != "." ]; then
    have=NGX_BUILD value="\"$NGX_BUILD\"" . auto/define
fi

. auto/summary
{% endhighlight %}


其实除了configure之外，其他的自动脚本都在auto目录下。通过分析configure脚本源码，我们可以看到，configure首先运行了auto目录下的几个自动脚本，如下：

* auto/options
* auto/init
* auto/sources

然后根据相应的配置生成对应的makefile文件。这里作为一个总览，我们给出configure的输出，以做我们后面分析时的参考。具体对各个模块的分析，我们再以单独的章节来进行讲述。

## 2. configure配置输出

如下是我们执行configure后的输出：
{% highlight string %}
root@ubuntu:/home/ivan1001/Share/nginx-1.10.3# ./configure \
--sbin-path=/usr/local/nginx/nginx \
--conf-path=/usr/local/nginx/nginx.conf \
--pid-path=/usr/local/nginx/nginx.pid \
--with-http_ssl_module \
--with-pcre=../pcre-8.40 \
--with-zlib=../zlib-1.2.11


checking for OS
 + Linux 4.8.0-36-generic i686
checking for C compiler ... found
 + using GNU C compiler
 + gcc version: 5.4.0 20160609 (Ubuntu 5.4.0-6ubuntu1~16.04.4) 
checking for gcc -pipe switch ... found
checking for -Wl,-E switch ... found
checking for gcc builtin atomic operations ... found
checking for C99 variadic macros ... found
checking for gcc variadic macros ... found
checking for gcc builtin 64 bit byteswap ... found
checking for unistd.h ... found
checking for inttypes.h ... found
checking for limits.h ... found
checking for sys/filio.h ... not found
checking for sys/param.h ... found
checking for sys/mount.h ... found
checking for sys/statvfs.h ... found
checking for crypt.h ... found
checking for Linux specific features
checking for epoll ... found
checking for EPOLLRDHUP ... found
checking for O_PATH ... found
checking for sendfile() ... found
checking for sendfile64() ... found
checking for sys/prctl.h ... found
checking for prctl(PR_SET_DUMPABLE) ... found
checking for sched_setaffinity() ... found
checking for crypt_r() ... found
checking for sys/vfs.h ... found
checking for nobody group ... not found
checking for nogroup group ... found
checking for poll() ... found
checking for /dev/poll ... not found
checking for kqueue ... not found
checking for crypt() ... not found
checking for crypt() in libcrypt ... found
checking for F_READAHEAD ... not found
checking for posix_fadvise() ... found
checking for O_DIRECT ... found
checking for F_NOCACHE ... not found
checking for directio() ... not found
checking for statfs() ... found
checking for statvfs() ... found
checking for dlopen() ... not found
checking for dlopen() in libdl ... found
checking for sched_yield() ... found
checking for SO_SETFIB ... not found
checking for SO_REUSEPORT ... found
checking for SO_ACCEPTFILTER ... not found
checking for IP_RECVDSTADDR ... not found
checking for IP_PKTINFO ... found
checking for IPV6_RECVPKTINFO ... found
checking for TCP_DEFER_ACCEPT ... found
checking for TCP_KEEPIDLE ... found
checking for TCP_FASTOPEN ... found
checking for TCP_INFO ... found
checking for accept4() ... found
checking for eventfd() ... found
checking for int size ... 4 bytes
checking for long size ... 4 bytes
checking for long long size ... 8 bytes
checking for void * size ... 4 bytes
checking for uint32_t ... found
checking for uint64_t ... found
checking for sig_atomic_t ... found
checking for sig_atomic_t size ... 4 bytes
checking for socklen_t ... found
checking for in_addr_t ... found
checking for in_port_t ... found
checking for rlim_t ... found
checking for uintptr_t ... uintptr_t found
checking for system byte ordering ... little endian
checking for size_t size ... 4 bytes
checking for off_t size ... 8 bytes
checking for time_t size ... 4 bytes
checking for setproctitle() ... not found
checking for pread() ... found
checking for pwrite() ... found
checking for pwritev() ... found
checking for sys_nerr ... found
checking for localtime_r() ... found
checking for posix_memalign() ... found
checking for memalign() ... found
checking for mmap(MAP_ANON|MAP_SHARED) ... found
checking for mmap("/dev/zero", MAP_SHARED) ... found
checking for System V shared memory ... found
checking for POSIX semaphores ... not found
checking for POSIX semaphores in libpthread ... found
checking for struct msghdr.msg_control ... found
checking for ioctl(FIONBIO) ... found
checking for struct tm.tm_gmtoff ... found
checking for struct dirent.d_namlen ... not found
checking for struct dirent.d_type ... found
checking for sysconf(_SC_NPROCESSORS_ONLN) ... found
checking for openat(), fstatat() ... found
checking for getaddrinfo() ... found
checking for OpenSSL library ... found
creating objs/Makefile

Configuration summary
  + using PCRE library: ../pcre-8.40
  + using system OpenSSL library
  + md5: using OpenSSL library
  + sha1: using OpenSSL library
  + using zlib library: ../zlib-1.2.11

  nginx path prefix: "/usr/local/nginx"
  nginx binary file: "/usr/local/nginx/nginx"
  nginx modules path: "/usr/local/nginx/modules"
  nginx configuration prefix: "/usr/local/nginx"
  nginx configuration file: "/usr/local/nginx/nginx.conf"
  nginx pid file: "/usr/local/nginx/nginx.pid"
  nginx error log file: "/usr/local/nginx/logs/error.log"
  nginx http access log file: "/usr/local/nginx/logs/access.log"
  nginx http client request body temporary files: "client_body_temp"
  nginx http proxy temporary files: "proxy_temp"
  nginx http fastcgi temporary files: "fastcgi_temp"
  nginx http uwsgi temporary files: "uwsgi_temp"
  nginx http scgi temporary files: "scgi_temp"


{% endhighlight %}


## 3. make输出
如下是执行make命令后的输出：
{% highlight string %}
root@ubuntu:/home/ivan1001/Share/nginx-1.10.3# make
make -f objs/Makefile
make[1]: Entering directory '/home/ivan1001/Share/nginx-1.10.3'
cd ../pcre-8.40 \
&& if [ -f Makefile ]; then make distclean; fi \
&& CC="cc" CFLAGS="-O2 -fomit-frame-pointer -pipe " \
./configure --disable-shared 
make[2]: Entering directory '/home/ivan1001/Share/pcre-8.40'
 rm -f pcretest pcregrep
test -z "pcre_chartables.c testsavedregex teststderr testtemp* testtry testNinput testtrygrep teststderrgrep testNinputgrep" || rm -f pcre_chartables.c testsavedregex teststderr testtemp* testtry testNinput testtrygrep teststderrgrep testNinputgrep
test -z "libpcre.la   libpcreposix.la libpcrecpp.la" || rm -f libpcre.la   libpcreposix.la libpcrecpp.la
rm -f ./so_locations
rm -rf .libs _libs
 rm -f pcrecpp_unittest pcre_scanner_unittest pcre_stringpiece_unittest
rm -f *.o
test -z "pcrecpp_unittest.log pcre_scanner_unittest.log pcre_stringpiece_unittest.log RunTest.log RunGrepTest.log" || rm -f pcrecpp_unittest.log pcre_scanner_unittest.log pcre_stringpiece_unittest.log RunTest.log RunGrepTest.log
test -z "pcrecpp_unittest.trs pcre_scanner_unittest.trs pcre_stringpiece_unittest.trs RunTest.trs RunGrepTest.trs" || rm -f pcrecpp_unittest.trs pcre_scanner_unittest.trs pcre_stringpiece_unittest.trs RunTest.trs RunGrepTest.trs
test -z "test-suite.log" || rm -f test-suite.log
rm -f *.lo
rm -f *.tab.c
test -z "libpcre.pc libpcre16.pc libpcre32.pc libpcreposix.pc libpcrecpp.pc pcre-config pcre.h pcre_stringpiece.h pcrecpparg.h" || rm -f libpcre.pc libpcre16.pc libpcre32.pc libpcreposix.pc libpcrecpp.pc pcre-config pcre.h pcre_stringpiece.h pcrecpparg.h
test . = "." || test -z "" || rm -f 
rm -f config.h stamp-h1
rm -f libtool config.lt
rm -f TAGS ID GTAGS GRTAGS GSYMS GPATH tags
rm -f cscope.out cscope.in.out cscope.po.out cscope.files
rm -f config.status config.cache config.log configure.lineno config.status.lineno
rm -rf ./.deps
rm -f Makefile
make[2]: Leaving directory '/home/ivan1001/Share/pcre-8.40'
checking for a BSD-compatible install... /usr/bin/install -c
checking whether build environment is sane... yes
checking for a thread-safe mkdir -p... /bin/mkdir -p
checking for gawk... no
checking for mawk... mawk
checking whether make sets $(MAKE)... yes
checking whether make supports nested variables... yes
checking whether make supports nested variables... (cached) yes
checking for style of include used by make... GNU
checking for gcc... cc
checking whether the C compiler works... yes
checking for C compiler default output file name... a.out
checking for suffix of executables... 
checking whether we are cross compiling... no
checking for suffix of object files... o
checking whether we are using the GNU C compiler... yes
checking whether cc accepts -g... yes
checking for cc option to accept ISO C89... none needed
checking whether cc understands -c and -o together... yes
checking dependency style of cc... gcc3
checking for ar... ar
checking the archiver (ar) interface... ar
checking for gcc... (cached) cc
checking whether we are using the GNU C compiler... (cached) yes
checking whether cc accepts -g... (cached) yes
checking for cc option to accept ISO C89... (cached) none needed
checking whether cc understands -c and -o together... (cached) yes
checking dependency style of cc... (cached) gcc3
checking for g++... g++
checking whether we are using the GNU C++ compiler... yes
checking whether g++ accepts -g... yes
checking dependency style of g++... gcc3
checking how to run the C preprocessor... cc -E
checking for grep that handles long lines and -e... /bin/grep
checking for egrep... /bin/grep -E
checking for ANSI C header files... yes
checking for sys/types.h... yes
checking for sys/stat.h... yes
checking for stdlib.h... yes
checking for string.h... yes
checking for memory.h... yes
checking for strings.h... yes
checking for inttypes.h... yes
checking for stdint.h... yes
checking for unistd.h... yes
checking for int64_t... yes
checking build system type... i686-pc-linux-gnu
checking host system type... i686-pc-linux-gnu
checking how to print strings... printf
checking for a sed that does not truncate output... /bin/sed
checking for fgrep... /bin/grep -F
checking for ld used by cc... /usr/bin/ld
checking if the linker (/usr/bin/ld) is GNU ld... yes
checking for BSD- or MS-compatible name lister (nm)... /usr/bin/nm -B
checking the name lister (/usr/bin/nm -B) interface... BSD nm
checking whether ln -s works... yes
checking the maximum length of command line arguments... 1572864
checking how to convert i686-pc-linux-gnu file names to i686-pc-linux-gnu format... func_convert_file_noop
checking how to convert i686-pc-linux-gnu file names to toolchain format... func_convert_file_noop
checking for /usr/bin/ld option to reload object files... -r
checking for objdump... objdump
checking how to recognize dependent libraries... pass_all
checking for dlltool... dlltool
checking how to associate runtime and link libraries... printf %s\n
checking for archiver @FILE support... @
checking for strip... strip
checking for ranlib... ranlib
checking command to parse /usr/bin/nm -B output from cc object... ok
checking for sysroot... no
checking for a working dd... /bin/dd
checking how to truncate binary pipes... /bin/dd bs=4096 count=1
checking for mt... mt
checking if mt is a manifest tool... no
checking for dlfcn.h... yes
checking for objdir... .libs
checking if cc supports -fno-rtti -fno-exceptions... no
checking for cc option to produce PIC... -fPIC -DPIC
checking if cc PIC flag -fPIC -DPIC works... yes
checking if cc static flag -static works... yes
checking if cc supports -c -o file.o... yes
checking if cc supports -c -o file.o... (cached) yes
checking whether the cc linker (/usr/bin/ld) supports shared libraries... yes
checking dynamic linker characteristics... GNU/Linux ld.so
checking how to hardcode library paths into programs... immediate
checking whether stripping libraries is possible... yes
checking if libtool supports shared libraries... yes
checking whether to build shared libraries... no
checking whether to build static libraries... yes
checking how to run the C++ preprocessor... g++ -E
checking for ld used by g++... /usr/bin/ld
checking if the linker (/usr/bin/ld) is GNU ld... yes
checking whether the g++ linker (/usr/bin/ld) supports shared libraries... yes
checking for g++ option to produce PIC... -fPIC -DPIC
checking if g++ PIC flag -fPIC -DPIC works... yes
checking if g++ static flag -static works... yes
checking if g++ supports -c -o file.o... yes
checking if g++ supports -c -o file.o... (cached) yes
checking whether the g++ linker (/usr/bin/ld) supports shared libraries... yes
checking dynamic linker characteristics... (cached) GNU/Linux ld.so
checking how to hardcode library paths into programs... immediate
checking whether ln -s works... yes
checking whether the -Werror option is usable... yes
checking for simple visibility declarations... yes
checking for ANSI C header files... (cached) yes
checking limits.h usability... yes
checking limits.h presence... yes
checking for limits.h... yes
checking for sys/types.h... (cached) yes
checking for sys/stat.h... (cached) yes
checking dirent.h usability... yes
checking dirent.h presence... yes
checking for dirent.h... yes
checking windows.h usability... no
checking windows.h presence... no
checking for windows.h... no
checking for alias support in the linker... no
checking for alias support in the linker... no
checking string usability... yes
checking string presence... yes
checking for string... yes
checking bits/type_traits.h usability... no
checking bits/type_traits.h presence... no
checking for bits/type_traits.h... no
checking type_traits.h usability... no
checking type_traits.h presence... no
checking for type_traits.h... no
checking for strtoq... yes
checking for long long... yes
checking for unsigned long long... yes
checking for an ANSI C-conforming const... yes
checking for size_t... yes
checking for bcopy... yes
checking for memmove... yes
checking for strerror... yes
checking zlib.h usability... yes
checking zlib.h presence... yes
checking for zlib.h... yes
checking for gzopen in -lz... yes
checking bzlib.h usability... no
checking bzlib.h presence... no
checking for bzlib.h... no
checking for libbz2... no
checking that generated files are newer than configure... done
configure: creating ./config.status
config.status: creating Makefile
config.status: creating libpcre.pc
config.status: creating libpcre16.pc
config.status: creating libpcre32.pc
config.status: creating libpcreposix.pc
config.status: creating libpcrecpp.pc
config.status: creating pcre-config
config.status: creating pcre.h
config.status: creating pcre_stringpiece.h
config.status: creating pcrecpparg.h
config.status: creating config.h
config.status: executing depfiles commands
config.status: executing libtool commands
config.status: executing script-chmod commands
config.status: executing delete-old-chartables commands

pcre-8.40 configuration summary:

    Install prefix .................. : /usr/local
    C preprocessor .................. : cc -E
    C compiler ...................... : cc
    C++ preprocessor ................ : g++ -E
    C++ compiler .................... : g++
    Linker .......................... : /usr/bin/ld
    C preprocessor flags ............ : 
    C compiler flags ................ : -O2 -fomit-frame-pointer -pipe  -fvisibility=hidden
    C++ compiler flags .............. : -O2 -fvisibility=hidden -fvisibility-inlines-hidden
    Linker flags .................... : 
    Extra libraries ................. : 

    Build 8 bit pcre library ........ : yes
    Build 16 bit pcre library ....... : no
    Build 32 bit pcre library ....... : no
    Build C++ library ............... : yes
    Enable JIT compiling support .... : no
    Enable UTF-8/16/32 support ...... : no
    Unicode properties .............. : no
    Newline char/sequence ........... : lf
    \R matches only ANYCRLF ......... : no
    EBCDIC coding ................... : no
    EBCDIC code for NL .............. : n/a
    Rebuild char tables ............. : no
    Use stack recursion ............. : yes
    POSIX mem threshold ............. : 10
    Internal link size .............. : 2
    Nested parentheses limit ........ : 250
    Match limit ..................... : 10000000
    Match limit recursion ........... : MATCH_LIMIT
    Build shared libs ............... : no
    Build static libs ............... : yes
    Use JIT in pcregrep ............. : no
    Buffer size for pcregrep ........ : 20480
    Link pcregrep with libz ......... : no
    Link pcregrep with libbz2 ....... : no
    Link pcretest with libedit ...... : no
    Link pcretest with libreadline .. : no
    Valgrind support ................ : no
    Code coverage ................... : no

cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/nginx.o \
        src/core/nginx.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_log.o \
        src/core/ngx_log.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_palloc.o \
        src/core/ngx_palloc.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_array.o \
        src/core/ngx_array.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_list.o \
        src/core/ngx_list.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_hash.o \
        src/core/ngx_hash.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_buf.o \
        src/core/ngx_buf.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_queue.o \
        src/core/ngx_queue.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_output_chain.o \
        src/core/ngx_output_chain.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_string.o \
        src/core/ngx_string.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_parse.o \
        src/core/ngx_parse.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_parse_time.o \
        src/core/ngx_parse_time.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_inet.o \
        src/core/ngx_inet.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_file.o \
        src/core/ngx_file.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_crc32.o \
        src/core/ngx_crc32.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_murmurhash.o \
        src/core/ngx_murmurhash.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_md5.o \
        src/core/ngx_md5.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_rbtree.o \
        src/core/ngx_rbtree.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_radix_tree.o \
        src/core/ngx_radix_tree.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_slab.o \
        src/core/ngx_slab.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_times.o \
        src/core/ngx_times.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_shmtx.o \
        src/core/ngx_shmtx.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_connection.o \
        src/core/ngx_connection.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_cycle.o \
        src/core/ngx_cycle.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_spinlock.o \
        src/core/ngx_spinlock.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_rwlock.o \
        src/core/ngx_rwlock.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_cpuinfo.o \
        src/core/ngx_cpuinfo.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_conf_file.o \
        src/core/ngx_conf_file.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_module.o \
        src/core/ngx_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_resolver.o \
        src/core/ngx_resolver.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_open_file_cache.o \
        src/core/ngx_open_file_cache.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_crypt.o \
        src/core/ngx_crypt.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_proxy_protocol.o \
        src/core/ngx_proxy_protocol.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_syslog.o \
        src/core/ngx_syslog.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/event/ngx_event.o \
        src/event/ngx_event.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/event/ngx_event_timer.o \
        src/event/ngx_event_timer.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/event/ngx_event_posted.o \
        src/event/ngx_event_posted.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/event/ngx_event_accept.o \
        src/event/ngx_event_accept.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/event/ngx_event_connect.o \
        src/event/ngx_event_connect.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/event/ngx_event_pipe.o \
        src/event/ngx_event_pipe.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_time.o \
        src/os/unix/ngx_time.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_errno.o \
        src/os/unix/ngx_errno.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_alloc.o \
        src/os/unix/ngx_alloc.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_files.o \
        src/os/unix/ngx_files.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_socket.o \
        src/os/unix/ngx_socket.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_recv.o \
        src/os/unix/ngx_recv.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_readv_chain.o \
        src/os/unix/ngx_readv_chain.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_udp_recv.o \
        src/os/unix/ngx_udp_recv.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_send.o \
        src/os/unix/ngx_send.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_writev_chain.o \
        src/os/unix/ngx_writev_chain.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_udp_send.o \
        src/os/unix/ngx_udp_send.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_channel.o \
        src/os/unix/ngx_channel.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_shmem.o \
        src/os/unix/ngx_shmem.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_process.o \
        src/os/unix/ngx_process.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_daemon.o \
        src/os/unix/ngx_daemon.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_setaffinity.o \
        src/os/unix/ngx_setaffinity.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_setproctitle.o \
        src/os/unix/ngx_setproctitle.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_posix_init.o \
        src/os/unix/ngx_posix_init.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_user.o \
        src/os/unix/ngx_user.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_dlopen.o \
        src/os/unix/ngx_dlopen.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_process_cycle.o \
        src/os/unix/ngx_process_cycle.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_linux_init.o \
        src/os/unix/ngx_linux_init.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/event/modules/ngx_epoll_module.o \
        src/event/modules/ngx_epoll_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/os/unix/ngx_linux_sendfile_chain.o \
        src/os/unix/ngx_linux_sendfile_chain.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/event/ngx_event_openssl.o \
        src/event/ngx_event_openssl.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/event/ngx_event_openssl_stapling.o \
        src/event/ngx_event_openssl_stapling.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/src/core/ngx_regex.o \
        src/core/ngx_regex.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http.o \
        src/http/ngx_http.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_core_module.o \
        src/http/ngx_http_core_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_special_response.o \
        src/http/ngx_http_special_response.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_request.o \
        src/http/ngx_http_request.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_parse.o \
        src/http/ngx_http_parse.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_log_module.o \
        src/http/modules/ngx_http_log_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_request_body.o \
        src/http/ngx_http_request_body.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_variables.o \
        src/http/ngx_http_variables.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_script.o \
        src/http/ngx_http_script.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_upstream.o \
        src/http/ngx_http_upstream.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_upstream_round_robin.o \
        src/http/ngx_http_upstream_round_robin.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_file_cache.o \
        src/http/ngx_http_file_cache.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_write_filter_module.o \
        src/http/ngx_http_write_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_header_filter_module.o \
        src/http/ngx_http_header_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_chunked_filter_module.o \
        src/http/modules/ngx_http_chunked_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_range_filter_module.o \
        src/http/modules/ngx_http_range_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_gzip_filter_module.o \
        src/http/modules/ngx_http_gzip_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_postpone_filter_module.o \
        src/http/ngx_http_postpone_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_ssi_filter_module.o \
        src/http/modules/ngx_http_ssi_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_charset_filter_module.o \
        src/http/modules/ngx_http_charset_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_userid_filter_module.o \
        src/http/modules/ngx_http_userid_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_headers_filter_module.o \
        src/http/modules/ngx_http_headers_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/ngx_http_copy_filter_module.o \
        src/http/ngx_http_copy_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_not_modified_filter_module.o \
        src/http/modules/ngx_http_not_modified_filter_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_static_module.o \
        src/http/modules/ngx_http_static_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_autoindex_module.o \
        src/http/modules/ngx_http_autoindex_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_index_module.o \
        src/http/modules/ngx_http_index_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_auth_basic_module.o \
        src/http/modules/ngx_http_auth_basic_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_access_module.o \
        src/http/modules/ngx_http_access_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_limit_conn_module.o \
        src/http/modules/ngx_http_limit_conn_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_limit_req_module.o \
        src/http/modules/ngx_http_limit_req_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_geo_module.o \
        src/http/modules/ngx_http_geo_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_map_module.o \
        src/http/modules/ngx_http_map_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_split_clients_module.o \
        src/http/modules/ngx_http_split_clients_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_referer_module.o \
        src/http/modules/ngx_http_referer_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_rewrite_module.o \
        src/http/modules/ngx_http_rewrite_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_ssl_module.o \
        src/http/modules/ngx_http_ssl_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_proxy_module.o \
        src/http/modules/ngx_http_proxy_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_fastcgi_module.o \
        src/http/modules/ngx_http_fastcgi_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_uwsgi_module.o \
        src/http/modules/ngx_http_uwsgi_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_scgi_module.o \
        src/http/modules/ngx_http_scgi_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_memcached_module.o \
        src/http/modules/ngx_http_memcached_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_empty_gif_module.o \
        src/http/modules/ngx_http_empty_gif_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_browser_module.o \
        src/http/modules/ngx_http_browser_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_upstream_hash_module.o \
        src/http/modules/ngx_http_upstream_hash_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_upstream_ip_hash_module.o \
        src/http/modules/ngx_http_upstream_ip_hash_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_upstream_least_conn_module.o \
        src/http/modules/ngx_http_upstream_least_conn_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_upstream_keepalive_module.o \
        src/http/modules/ngx_http_upstream_keepalive_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs -I src/http -I src/http/modules \
        -o objs/src/http/modules/ngx_http_upstream_zone_module.o \
        src/http/modules/ngx_http_upstream_zone_module.c
cc -c -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I ../pcre-8.40 -I ../zlib-1.2.11 -I objs \
        -o objs/ngx_modules.o \
        objs/ngx_modules.c
cd ../pcre-8.40 \
&& make libpcre.la
make[2]: Entering directory '/home/ivan1001/Share/pcre-8.40'
  CC       libpcre_la-pcre_byte_order.lo
  CC       libpcre_la-pcre_compile.lo
  CC       libpcre_la-pcre_config.lo
  CC       libpcre_la-pcre_dfa_exec.lo
  CC       libpcre_la-pcre_exec.lo
  CC       libpcre_la-pcre_fullinfo.lo
  CC       libpcre_la-pcre_get.lo
  CC       libpcre_la-pcre_globals.lo
  CC       libpcre_la-pcre_jit_compile.lo
  CC       libpcre_la-pcre_maketables.lo
  CC       libpcre_la-pcre_newline.lo
  CC       libpcre_la-pcre_ord2utf8.lo
  CC       libpcre_la-pcre_refcount.lo
  CC       libpcre_la-pcre_string_utils.lo
  CC       libpcre_la-pcre_study.lo
  CC       libpcre_la-pcre_tables.lo
  CC       libpcre_la-pcre_ucd.lo
  CC       libpcre_la-pcre_valid_utf8.lo
  CC       libpcre_la-pcre_version.lo
  CC       libpcre_la-pcre_xclass.lo
rm -f pcre_chartables.c
ln -s ./pcre_chartables.c.dist pcre_chartables.c
  CC       libpcre_la-pcre_chartables.lo
  CCLD     libpcre.la
ar: `u' modifier ignored since `D' is the default (see `U')
make[2]: Leaving directory '/home/ivan1001/Share/pcre-8.40'
cd ../zlib-1.2.11 \
&& make distclean \
&& CFLAGS="-O2 -fomit-frame-pointer -pipe " CC="cc" \
        ./configure \
&& make libz.a
make[2]: Entering directory '/home/ivan1001/Share/zlib-1.2.11'
rm -f *.o *.lo *~ \
   example minigzip examplesh minigzipsh \
   example64 minigzip64 \
   infcover \
   libz.* foo.gz so_locations \
   _match.s maketree contrib/infback9/*.o
rm -rf objs
rm -f *.gcda *.gcno *.gcov
rm -f contrib/infback9/*.gcda contrib/infback9/*.gcno contrib/infback9/*.gcov
cp -p zconf.h.in zconf.h
rm -f Makefile zlib.pc configure.log
make[2]: Leaving directory '/home/ivan1001/Share/zlib-1.2.11'
Checking for shared library support...
Building shared library libz.so.1.2.11 with cc.
Checking for size_t... Yes.
Checking for off64_t... Yes.
Checking for fseeko... Yes.
Checking for strerror... Yes.
Checking for unistd.h... Yes.
Checking for stdarg.h... Yes.
Checking whether to use vs[n]printf() or s[n]printf()... using vs[n]printf().
Checking for vsnprintf() in stdio.h... Yes.
Checking for return value of vsnprintf()... Yes.
Checking for attribute(visibility) support... Yes.
make[2]: Entering directory '/home/ivan1001/Share/zlib-1.2.11'
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o adler32.o adler32.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o crc32.o crc32.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o deflate.o deflate.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o infback.o infback.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o inffast.o inffast.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o inflate.o inflate.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o inftrees.o inftrees.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o trees.o trees.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o zutil.o zutil.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o compress.o compress.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o uncompr.o uncompr.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o gzclose.o gzclose.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o gzlib.o gzlib.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o gzread.o gzread.c
cc -O2 -fomit-frame-pointer -pipe  -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN  -c -o gzwrite.o gzwrite.c
ar rc libz.a adler32.o crc32.o deflate.o infback.o inffast.o inflate.o inftrees.o trees.o zutil.o compress.o uncompr.o gzclose.o gzlib.o gzread.o gzwrite.o 
make[2]: Leaving directory '/home/ivan1001/Share/zlib-1.2.11'
cc -o objs/nginx \
objs/src/core/nginx.o \
objs/src/core/ngx_log.o \
objs/src/core/ngx_palloc.o \
objs/src/core/ngx_array.o \
objs/src/core/ngx_list.o \
objs/src/core/ngx_hash.o \
objs/src/core/ngx_buf.o \
objs/src/core/ngx_queue.o \
objs/src/core/ngx_output_chain.o \
objs/src/core/ngx_string.o \
objs/src/core/ngx_parse.o \
objs/src/core/ngx_parse_time.o \
objs/src/core/ngx_inet.o \
objs/src/core/ngx_file.o \
objs/src/core/ngx_crc32.o \
objs/src/core/ngx_murmurhash.o \
objs/src/core/ngx_md5.o \
objs/src/core/ngx_rbtree.o \
objs/src/core/ngx_radix_tree.o \
objs/src/core/ngx_slab.o \
objs/src/core/ngx_times.o \
objs/src/core/ngx_shmtx.o \
objs/src/core/ngx_connection.o \
objs/src/core/ngx_cycle.o \
objs/src/core/ngx_spinlock.o \
objs/src/core/ngx_rwlock.o \
objs/src/core/ngx_cpuinfo.o \
objs/src/core/ngx_conf_file.o \
objs/src/core/ngx_module.o \
objs/src/core/ngx_resolver.o \
objs/src/core/ngx_open_file_cache.o \
objs/src/core/ngx_crypt.o \
objs/src/core/ngx_proxy_protocol.o \
objs/src/core/ngx_syslog.o \
objs/src/event/ngx_event.o \
objs/src/event/ngx_event_timer.o \
objs/src/event/ngx_event_posted.o \
objs/src/event/ngx_event_accept.o \
objs/src/event/ngx_event_connect.o \
objs/src/event/ngx_event_pipe.o \
objs/src/os/unix/ngx_time.o \
objs/src/os/unix/ngx_errno.o \
objs/src/os/unix/ngx_alloc.o \
objs/src/os/unix/ngx_files.o \
objs/src/os/unix/ngx_socket.o \
objs/src/os/unix/ngx_recv.o \
objs/src/os/unix/ngx_readv_chain.o \
objs/src/os/unix/ngx_udp_recv.o \
objs/src/os/unix/ngx_send.o \
objs/src/os/unix/ngx_writev_chain.o \
objs/src/os/unix/ngx_udp_send.o \
objs/src/os/unix/ngx_channel.o \
objs/src/os/unix/ngx_shmem.o \
objs/src/os/unix/ngx_process.o \
objs/src/os/unix/ngx_daemon.o \
objs/src/os/unix/ngx_setaffinity.o \
objs/src/os/unix/ngx_setproctitle.o \
objs/src/os/unix/ngx_posix_init.o \
objs/src/os/unix/ngx_user.o \
objs/src/os/unix/ngx_dlopen.o \
objs/src/os/unix/ngx_process_cycle.o \
objs/src/os/unix/ngx_linux_init.o \
objs/src/event/modules/ngx_epoll_module.o \
objs/src/os/unix/ngx_linux_sendfile_chain.o \
objs/src/event/ngx_event_openssl.o \
objs/src/event/ngx_event_openssl_stapling.o \
objs/src/core/ngx_regex.o \
objs/src/http/ngx_http.o \
objs/src/http/ngx_http_core_module.o \
objs/src/http/ngx_http_special_response.o \
objs/src/http/ngx_http_request.o \
objs/src/http/ngx_http_parse.o \
objs/src/http/modules/ngx_http_log_module.o \
objs/src/http/ngx_http_request_body.o \
objs/src/http/ngx_http_variables.o \
objs/src/http/ngx_http_script.o \
objs/src/http/ngx_http_upstream.o \
objs/src/http/ngx_http_upstream_round_robin.o \
objs/src/http/ngx_http_file_cache.o \
objs/src/http/ngx_http_write_filter_module.o \
objs/src/http/ngx_http_header_filter_module.o \
objs/src/http/modules/ngx_http_chunked_filter_module.o \
objs/src/http/modules/ngx_http_range_filter_module.o \
objs/src/http/modules/ngx_http_gzip_filter_module.o \
objs/src/http/ngx_http_postpone_filter_module.o \
objs/src/http/modules/ngx_http_ssi_filter_module.o \
objs/src/http/modules/ngx_http_charset_filter_module.o \
objs/src/http/modules/ngx_http_userid_filter_module.o \
objs/src/http/modules/ngx_http_headers_filter_module.o \
objs/src/http/ngx_http_copy_filter_module.o \
objs/src/http/modules/ngx_http_not_modified_filter_module.o \
objs/src/http/modules/ngx_http_static_module.o \
objs/src/http/modules/ngx_http_autoindex_module.o \
objs/src/http/modules/ngx_http_index_module.o \
objs/src/http/modules/ngx_http_auth_basic_module.o \
objs/src/http/modules/ngx_http_access_module.o \
objs/src/http/modules/ngx_http_limit_conn_module.o \
objs/src/http/modules/ngx_http_limit_req_module.o \
objs/src/http/modules/ngx_http_geo_module.o \
objs/src/http/modules/ngx_http_map_module.o \
objs/src/http/modules/ngx_http_split_clients_module.o \
objs/src/http/modules/ngx_http_referer_module.o \
objs/src/http/modules/ngx_http_rewrite_module.o \
objs/src/http/modules/ngx_http_ssl_module.o \
objs/src/http/modules/ngx_http_proxy_module.o \
objs/src/http/modules/ngx_http_fastcgi_module.o \
objs/src/http/modules/ngx_http_uwsgi_module.o \
objs/src/http/modules/ngx_http_scgi_module.o \
objs/src/http/modules/ngx_http_memcached_module.o \
objs/src/http/modules/ngx_http_empty_gif_module.o \
objs/src/http/modules/ngx_http_browser_module.o \
objs/src/http/modules/ngx_http_upstream_hash_module.o \
objs/src/http/modules/ngx_http_upstream_ip_hash_module.o \
objs/src/http/modules/ngx_http_upstream_least_conn_module.o \
objs/src/http/modules/ngx_http_upstream_keepalive_module.o \
objs/src/http/modules/ngx_http_upstream_zone_module.o \
objs/ngx_modules.o \
-ldl -lpthread -lcrypt ../pcre-8.40/.libs/libpcre.a -lssl -lcrypto -ldl ../zlib-1.2.11/libz.a \
-Wl,-E
sed -e "s|%%PREFIX%%|/usr/local/nginx|" \
        -e "s|%%PID_PATH%%|/usr/local/nginx/nginx.pid|" \
        -e "s|%%CONF_PATH%%|/usr/local/nginx/nginx.conf|" \
        -e "s|%%ERROR_LOG_PATH%%|/usr/local/nginx/logs/error.log|" \
        < man/nginx.8 > objs/nginx.8
make[1]: Leaving directory '/home/ivan1001/Share/nginx-1.10.3'
{% endhighlight %}
   

## 4. make install输出
如下是make install输出：
{% highlight string %}
root@ubuntu:/home/ivan1001/Share# cd nginx-1.10.3/
root@ubuntu:/home/ivan1001/Share/nginx-1.10.3# make install
make -f objs/Makefile install
make[1]: Entering directory '/home/ivan1001/Share/nginx-1.10.3'
test -d '/usr/local/nginx' || mkdir -p '/usr/local/nginx'
test -d '/usr/local/nginx' \
        || mkdir -p '/usr/local/nginx'
test ! -f '/usr/local/nginx/nginx' \
        || mv '/usr/local/nginx/nginx' \
                '/usr/local/nginx/nginx.old'
cp objs/nginx '/usr/local/nginx/nginx'
test -d '/usr/local/nginx' \
        || mkdir -p '/usr/local/nginx'
cp conf/koi-win '/usr/local/nginx'
cp conf/koi-utf '/usr/local/nginx'
cp conf/win-utf '/usr/local/nginx'
test -f '/usr/local/nginx/mime.types' \
        || cp conf/mime.types '/usr/local/nginx'
cp conf/mime.types '/usr/local/nginx/mime.types.default'
test -f '/usr/local/nginx/fastcgi_params' \
        || cp conf/fastcgi_params '/usr/local/nginx'
cp conf/fastcgi_params \
        '/usr/local/nginx/fastcgi_params.default'
test -f '/usr/local/nginx/fastcgi.conf' \
        || cp conf/fastcgi.conf '/usr/local/nginx'
cp conf/fastcgi.conf '/usr/local/nginx/fastcgi.conf.default'
test -f '/usr/local/nginx/uwsgi_params' \
        || cp conf/uwsgi_params '/usr/local/nginx'
cp conf/uwsgi_params \
        '/usr/local/nginx/uwsgi_params.default'
test -f '/usr/local/nginx/scgi_params' \
        || cp conf/scgi_params '/usr/local/nginx'
cp conf/scgi_params \
        '/usr/local/nginx/scgi_params.default'
test -f '/usr/local/nginx/nginx.conf' \
        || cp conf/nginx.conf '/usr/local/nginx/nginx.conf'
cp conf/nginx.conf '/usr/local/nginx/nginx.conf.default'
test -d '/usr/local/nginx' \
        || mkdir -p '/usr/local/nginx'
test -d '/usr/local/nginx/logs' \
        || mkdir -p '/usr/local/nginx/logs'
test -d '/usr/local/nginx/html' \
        || cp -R html '/usr/local/nginx'
test -d '/usr/local/nginx/logs' \
        || mkdir -p '/usr/local/nginx/logs'
make[1]: Leaving directory '/home/ivan1001/Share/nginx-1.10.3'
{% endhighlight %}

<br />
<br />
<br />

