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

脚本主要是用于配置```CFLAGS```编译选项。

## 2. 获取gcc版本号
{% highlight string %}
NGX_GCC_VER=`$CC -v 2>&1 | grep 'gcc version' 2>&1 \
                         | sed -e 's/^.* version \(.*\)/\1/'`

echo " + gcc version: $NGX_GCC_VER"

have=NGX_COMPILER value="\"gcc $NGX_GCC_VER\"" . auto/define
{% endhighlight %}
上述代码获取gcc版本号，然后将版本信息通过宏定义的形式写入到ngx_auto_config.h头文件中.

## 3. 设置编译器的```-pipe```特性
pipe特性是指在编译的各阶段(stage)使用pipe来通信，而不是采用临时文件的方式来通信。在有一些系统上，汇编器并不能够从pipe读取数据，因此不能正常工作。GNU汇编器一般都具有pipe特性。

{% highlight string %}
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
{% endhighlight %}

运行auto/feature脚本，在编译选项中加入```-pipe```选项，以判断是否支持此特性。

## 4. 平台是否支持casa汇编指令
{% highlight string %}
case "$NGX_MACHINE" in

    sun4u | sun4v | sparc | sparc64 )
        # "-mcpu=v9" enables the "casa" assembler instruction
        CFLAGS="$CFLAGS -mcpu=v9"
    ;;

esac
{% endhighlight %}
在sun4u、sun4v、sparc、sparc64平台上支持casa汇编指令，因此在编译选项中添加-mcpu=v9的选项。


## 5. 编译器优化选项的设置
{% highlight string %}
# optimizations

#NGX_GCC_OPT="-O2"
#NGX_GCC_OPT="-Os"
NGX_GCC_OPT="-O"
{% endhighlight %}

## 6. fomit-frame-pointer选项
{% highlight string %}
#CFLAGS="$CFLAGS -fomit-frame-pointer"
{% endhighlight %}

在函数中禁止保留帧指针(frame pointer)到寄存器中。则就避免了生成frame pointer的保存、建立、恢复的代码，加快了程序的运行。但是在有些系统上启用此功能后会使得编译出来的程序不能进行调试。

从gcc4.6版本开始，默认会启用此功能，而如果需要保留帧指针(frame pointer)，则可以使用```--enable-frame-pointer```选项。

一般情况下，在非调试程序中，都会给编译器添加```--fomit-frame-pointer```选项。

## 7. 针对CPU的优化
{% highlight string %}
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
{% endhighlight %}

如上根据不同的CPU类型进行优化，然后将相应的优化选项添加到gcc的编译选项中。

```CC_AUX_FLAGS```作为辅助参数，并不会反映到最后生成的Makefile文件中，其只在脚本解析过程中被用到。而CFLAGS会作为编译选项反映到最后生成的Makefile文件中。

这里在auto/options脚本当中，CPU默认被初始化为NO,我们后续也没有通过```--with-cpu-opt```选项来对CPU进行专门的设置。


## 8. nginx依赖库的编译选项
{% highlight string %}
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
{% endhighlight %}
nginx会依赖于pcre、md5、zlib等，nginx在编译过程中可以直接编译这些依赖库的源代码，这里为其设置编译选项。

## 9. 编译器警告的设置
{% highlight string %}
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
{% endhighlight %}
下面对gcc编译器警告相关的一些参数进行说明：
* -w: 关闭编译时的警告。也就是编译后不显示任何warning，因为有时在编译后编译器会显示一些例如数据转换之类的警告，这些警告我们平时可以忽略。
* -Wall: 显示所有警告
* -W: 类似于-Wall，但只显示编译器认为出现错误的警告
* -Werror: 将所有警告都认为是发生了错误。

```-Wpointer-arith```: 对函数指针或者void *类型的指针进行算术操作时给出警告。-Wall 并不会打开此项。
<pre>
char a[] = {'a','b','c','d'};
void *pa = a;

void *p = pa + 1;   //此处产生警告
</pre>
	
```-Wno-unused-parameter````: 表示对未被使用的函数参数不产生警告。



## 10. 打开调试选项
{% highlight string %}
# debug
CFLAGS="$CFLAGS -g"
{% endhighlight %}


## 11. 预处理选项
{% highlight string %}
if [ ".$CPP" = "." ]; then
    CPP="$CC -E"
fi
{% endhighlight %}
如果没有设置预处理器，则将```$CC -E```作为预处理器。


<br />
<br />
<br />

