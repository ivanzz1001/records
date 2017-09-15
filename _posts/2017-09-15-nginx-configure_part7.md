---
layout: post
title: auto/cc/gcc脚本分析-part7
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---



在nginx源代码auto/cc目录下有很多编译器配置相关的脚本。除了```conf脚本```及```name脚本```之外，其他的都直接与编译器相关。Nginx的出色跨平台性（Linux、Darwin、Solaris、Win32 等）就有这些脚本的贡献，这些脚本主要有：

<!-- more -->
* acc: HP ANSI C++编译器的相关配置
* bcc: Borland C++编译器的相关配置
* ccc: Compaq C编译器的相关配置
* clang: clang编译器的相关配置，其是一个```较新```的C、C++、Objective-C、Objective-C++语言的轻量级编译器
* gcc： GNU C编译器的相关配置
* icc: Intel C++编译器的相关配置
* msvc: Microsoft Visual C++编译器的相关配置
* owc: Open Watcom C编译器的相关配置
* sunc: Sun C编译器的相关配置

这些配置脚本都是与编译器的特性直接相关。这里我们以我们最常用的gcc编译器为例来进行讲解。


## 1. auto/cc/gcc脚本

在进行具体的脚本分析之前，我们这里贴出其源代码：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


# gcc 2.7.2.3, 2.8.1, 2.95.4, egcs-1.1.2
#     3.0.4, 3.1.1, 3.2.3, 3.3.2, 3.3.3, 3.3.4, 3.4.0, 3.4.2
#     4.0.0, 4.0.1, 4.1.0


NGX_GCC_VER=`$CC -v 2>&1 | grep 'gcc version' 2>&1 \
                         | sed -e 's/^.* version \(.*\)/\1/'`

echo " + gcc version: $NGX_GCC_VER"

have=NGX_COMPILER value="\"gcc $NGX_GCC_VER\"" . auto/define


# Solaris 7's /usr/ccs/bin/as does not support "-pipe"

CC_TEST_FLAGS="-pipe"

ngx_feature="gcc -pipe switch"
ngx_feature_name=
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test=
. auto/feature

CC_TEST_FLAGS=

if [ $ngx_found = yes ]; then
    PIPE="-pipe"
fi


case "$NGX_MACHINE" in

    sun4u | sun4v | sparc | sparc64 )
        # "-mcpu=v9" enables the "casa" assembler instruction
        CFLAGS="$CFLAGS -mcpu=v9"
    ;;

esac


# optimizations

#NGX_GCC_OPT="-O2"
#NGX_GCC_OPT="-Os"
NGX_GCC_OPT="-O"

#CFLAGS="$CFLAGS -fomit-frame-pointer"

case $CPU in
    pentium)
        # optimize for Pentium and Athlon
        CPU_OPT="-march=pentium"
        NGX_CPU_CACHE_LINE=32
    ;;

    pentiumpro | pentium3)
        # optimize for Pentium Pro, Pentium II and Pentium III
        CPU_OPT="-march=pentiumpro"
        NGX_CPU_CACHE_LINE=32
    ;;

    pentium4)
        # optimize for Pentium 4, gcc 3.x
        CPU_OPT="-march=pentium4"
        NGX_CPU_CACHE_LINE=128
    ;;

    athlon)
        # optimize for Athlon, gcc 3.x
        CPU_OPT="-march=athlon"
        NGX_CPU_CACHE_LINE=64
    ;;

    opteron)
        # optimize for Opteron, gcc 3.x
        CPU_OPT="-march=opteron"
        NGX_CPU_CACHE_LINE=64
    ;;

    sparc32)
        # build 32-bit UltraSparc binary
        CPU_OPT="-m32"
        CORE_LINK="$CORE_LINK -m32"
        NGX_CPU_CACHE_LINE=64
    ;;

    sparc64)
        # build 64-bit UltraSparc binary
        CPU_OPT="-m64"
        CORE_LINK="$CORE_LINK -m64"
        NGX_CPU_CACHE_LINE=64
    ;;

    ppc64)
        # build 64-bit PowerPC binary
        CPU_OPT="-m64"
        CPU_OPT="$CPU_OPT -falign-functions=32 -falign-labels=32"
        CPU_OPT="$CPU_OPT -falign-loops=32 -falign-jumps=32"
        CORE_LINK="$CORE_LINK -m64"
        NGX_CPU_CACHE_LINE=128
    ;;

esac

CC_AUX_FLAGS="$CC_AUX_FLAGS $CPU_OPT"

case "$NGX_GCC_VER" in
    2.7*)
        # batch build
        CPU_OPT=
    ;;
esac


CFLAGS="$CFLAGS $PIPE $CPU_OPT"

if [ ".$PCRE_OPT" = "." ]; then
    PCRE_OPT="-O2 -fomit-frame-pointer $PIPE $CPU_OPT"
else
    PCRE_OPT="$PCRE_OPT $PIPE"
fi

if [ ".$MD5_OPT" = "." ]; then
    MD5_OPT="-O2 -fomit-frame-pointer $PIPE $CPU_OPT"
else
    MD5_OPT="$MD5_OPT $PIPE"
fi

if [ ".$ZLIB_OPT" = "." ]; then
    ZLIB_OPT="-O2 -fomit-frame-pointer $PIPE $CPU_OPT"
else
    ZLIB_OPT="$ZLIB_OPT $PIPE"
fi


# warnings

# -W requires at least -O
CFLAGS="$CFLAGS ${NGX_GCC_OPT:--O} -W"

CFLAGS="$CFLAGS -Wall -Wpointer-arith"
#CFLAGS="$CFLAGS -Wconversion"
#CFLAGS="$CFLAGS -Winline"
#CFLAGS="$CFLAGS -Wmissing-prototypes"


case "$NGX_GCC_VER" in
    [3-5].*)
        # we have a lot of the unused function arguments
        CFLAGS="$CFLAGS -Wno-unused-parameter"
        # 4.2.1 shows the warning in wrong places
        #CFLAGS="$CFLAGS -Wunreachable-code"

        # deprecated system OpenSSL library on OS X
        if [ "$NGX_SYSTEM" = "Darwin" ]; then
            CFLAGS="$CFLAGS -Wno-deprecated-declarations"
        fi
    ;;

    *)
        # we have a lot of the unused function arguments
        CFLAGS="$CFLAGS -Wno-unused"
    ;;
esac


# stop on warning
CFLAGS="$CFLAGS -Werror"

# debug
CFLAGS="$CFLAGS -g"

# DragonFly's gcc3 generates DWARF
#CFLAGS="$CFLAGS -g -gstabs"

if [ ".$CPP" = "." ]; then
    CPP="$CC -E"
fi
{% endhighlight %}

脚本主要是用于配置CFLAGS编译选项。

## 2 






<br />
<br />
<br />

