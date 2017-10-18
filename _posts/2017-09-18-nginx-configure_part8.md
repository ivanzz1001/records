---
layout: post
title: auto/cc/conf脚本分析-part8
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

auto/cc/conf脚本是作为编译器配置的一个总控脚本，主要完成编译器配置的选择及相关特性的测试。




<!-- more -->

<br />
<br />
参看：http://blog.csdn.net/poechant/article/details/7351264


## 1. auto/cc/conf脚本

在进行具体的脚本分析之前，我们这里贴出其源代码：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


LINK="\$(CC)"

MAIN_LINK=
MODULE_LINK="-shared"

ngx_include_opt="-I "
ngx_compile_opt="-c"
ngx_pic_opt="-fPIC"
ngx_objout="-o "
ngx_binout="-o "
ngx_objext="o"
ngx_binext=
ngx_modext=".so"

ngx_long_start=
ngx_long_end=

ngx_regex_dirsep="\/"
ngx_dirsep='/'

ngx_regex_cont=' \\\
	'
ngx_cont=' \
	'
ngx_tab=' \
		'
ngx_spacer=

ngx_long_regex_cont=$ngx_regex_cont
ngx_long_cont=$ngx_cont

. auto/cc/name

if test -n "$CFLAGS"; then

    CC_TEST_FLAGS="$CFLAGS $NGX_CC_OPT"

    case $NGX_CC_NAME in

        ccc)
            # Compaq C V6.5-207

            ngx_include_opt="-I"
        ;;

        sunc)

            MAIN_LINK=
            MODULE_LINK="-G"

            case "$NGX_MACHINE" in

                i86pc)
                    NGX_AUX=" src/os/unix/ngx_sunpro_x86.il"
                ;;

                sun4u | sun4v)
                    NGX_AUX=" src/os/unix/ngx_sunpro_sparc64.il"
                ;;

            esac

            case $CPU in

                amd64)
                    NGX_AUX=" src/os/unix/ngx_sunpro_amd64.il"
                ;;

            esac
        ;;

    esac

else

    case $NGX_CC_NAME in
        gcc)
            # gcc 2.7.2.3, 2.8.1, 2.95.4, egcs-1.1.2
            #     3.0.4, 3.1.1, 3.2.3, 3.3.2, 3.3.3, 3.3.4, 3.4.0, 3.4.2
            #     4.0.0, 4.0.1, 4.1.0

            . auto/cc/gcc
        ;;

        clang)
            # Clang C compiler

            . auto/cc/clang
        ;;

        icc)
            # Intel C++ compiler 7.1, 8.0, 8.1

            . auto/cc/icc
        ;;

        sunc)
            # Sun C 5.7 Patch 117837-04 2005/05/11

            . auto/cc/sunc
        ;;

        ccc)
            # Compaq C V6.5-207

            . auto/cc/ccc
        ;;

        acc)
            # aCC: HP ANSI C++ B3910B A.03.55.02

            . auto/cc/acc
        ;;

        msvc*)
            # MSVC++ 6.0 SP2, MSVC++ Toolkit 2003

            . auto/cc/msvc
        ;;

        owc)
            # Open Watcom C 1.0, 1.2

            . auto/cc/owc
        ;;

        bcc)
            # Borland C++ 5.5

            . auto/cc/bcc
        ;;

    esac

    CC_TEST_FLAGS="$CC_TEST_FLAGS $NGX_CC_OPT"

fi

CFLAGS="$CFLAGS $NGX_CC_OPT"
NGX_TEST_LD_OPT="$NGX_LD_OPT"

if [ "$NGX_PLATFORM" != win32 ]; then

    if test -n "$NGX_LD_OPT"; then
        ngx_feature=--with-ld-opt=\"$NGX_LD_OPT\"
        ngx_feature_name=
        ngx_feature_run=no
        ngx_feature_incs=
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test=
        . auto/feature

        if [ $ngx_found = no ]; then
            echo $0: error: the invalid value in --with-ld-opt=\"$NGX_LD_OPT\"
            echo
            exit 1
        fi
    fi


    ngx_feature="-Wl,-E switch"
    ngx_feature_name=
    ngx_feature_run=no
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=-Wl,-E
    ngx_feature_test=
    . auto/feature

    if [ $ngx_found = yes ]; then
        MAIN_LINK="-Wl,-E"
    fi


    ngx_feature="gcc builtin atomic operations"
    ngx_feature_name=NGX_HAVE_GCC_ATOMIC
    ngx_feature_run=yes
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="long  n = 0;
                      if (!__sync_bool_compare_and_swap(&n, 0, 1))
                          return 1;
                      if (__sync_fetch_and_add(&n, 1) != 1)
                          return 1;
                      if (n != 2)
                          return 1;
                      __sync_synchronize();"
    . auto/feature


    if [ "$NGX_CC_NAME" = "ccc" ]; then
        echo "checking for C99 variadic macros ... disabled"
    else
        ngx_feature="C99 variadic macros"
        ngx_feature_name="NGX_HAVE_C99_VARIADIC_MACROS"
        ngx_feature_run=yes
        ngx_feature_incs="#include <stdio.h>
#define var(dummy, ...)  sprintf(__VA_ARGS__)"
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test="char  buf[30]; buf[0] = '0';
                          var(0, buf, \"%d\", 1);
                          if (buf[0] != '1') return 1"
        . auto/feature
     fi


    ngx_feature="gcc variadic macros"
    ngx_feature_name="NGX_HAVE_GCC_VARIADIC_MACROS"
    ngx_feature_run=yes
    ngx_feature_incs="#include <stdio.h>
#define var(dummy, args...)  sprintf(args)"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="char  buf[30]; buf[0] = '0';
                      var(0, buf, \"%d\", 1);
                      if (buf[0] != '1') return 1"
    . auto/feature


    ngx_feature="gcc builtin 64 bit byteswap"
    ngx_feature_name="NGX_HAVE_GCC_BSWAP64"
    ngx_feature_run=no
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="__builtin_bswap64(0)"
    . auto/feature


#    ngx_feature="inline"
#    ngx_feature_name=
#    ngx_feature_run=no
#    ngx_feature_incs="int inline f(void) { return 1 }"
#    ngx_feature_path=
#    ngx_feature_libs=
#    ngx_feature_test=
#    . auto/feature
#
#    if [ $ngx_found = yes ]; then
#    fi

fi

{% endhighlight %}


## 2. 链接器
通过如下命令指定连接器：
{% highlight string %}
LINK="\$(CC)"
{% endhighlight %}

注意，这里使用```\$(CC)```，而不是```$(CC)```。后者表示的是执行CC命令返回的结果，而前者表示将$(CC)作为一个字符串来看待。

## 3. 编译及链接选项

指定主链接选项和模块链接选项：
{% highlight string %}
MAIN_LINK=
MODULE_LINK="-shared"

ngx_include_opt="-I "
ngx_compile_opt="-c"
ngx_pic_opt="-fPIC"
ngx_objout="-o "
ngx_binout="-o "
ngx_objext="o"
{% endhighlight %}

上面```-fPIC```选项表示生成地址无关代码(Position Independent Code)。通常情况下```-shared```产生的共享库都需要添加此选项。它是通过全局偏移表(Global Offset Table,GOT)的方式访问所有常量地址的。

## 4. 生成文件扩展名

目标文件扩展名、可执行文件扩展名：
{% highlight string %}
ngx_objext="o"
ngx_binext=
ngx_modext=".so"
{% endhighlight %}


## 5. ngx_long_start 和 ngx_long_end
这两个变量是在编译选项中使用的，与平台相关。
{% highlight string %}
ngx_long_start=
ngx_long_end=
{% endhighlight %}

```ngx_long_start:```
<pre>
在 bcc 中，设置为'@&&|
	'
在 msvc 中，设置为'@<<
	'
在 owc 中，设置为' '
</pre>

```ngx_long_end:```
<pre>
在 bcc 中，设置为'|'
在 msvc 中，设置为'<<'
在 owc 中，设置为' '
</pre>


## 6. 一些分隔符
{% highlight string %}
ngx_regex_dirsep="\/"
ngx_dirsep='/'

ngx_regex_cont=' \\\
	'
ngx_cont=' \
	'
ngx_tab=' \
		'
ngx_spacer=

ngx_long_regex_cont=$ngx_regex_cont
ngx_long_cont=$ngx_cont
{% endhighlight %}

* ngx_regex_dirsep: 指定模式匹配表达式中目录分割符
* ngx_dirseq: 指定目录分隔符
* ngx_regex_cont: 匹配 ```空格 + \\\ + 1个制表符```
* ngx_cont: 匹配 ```空格 + \ + 1个制表符```
* ngx_tab: 匹配 ```空格 + \ + 2个制表符```
* ngx_spacer: ngx空格

## 7. 执行auto/cc/name脚本

执行auto/cc/name脚本，选择对应的编译器保存在NGX_CC_NAME变量中：
{% highlight string %}
. auto/cc/name
{% endhighlight %}


## 8. 根据编译器设置相应的编译连接选项
{% highlight string %}
if test -n "$CFLAGS"; then

    CC_TEST_FLAGS="$CFLAGS $NGX_CC_OPT"

    case $NGX_CC_NAME in

        ccc)
            # Compaq C V6.5-207

            ngx_include_opt="-I"
        ;;

        sunc)

            MAIN_LINK=
            MODULE_LINK="-G"

            case "$NGX_MACHINE" in

                i86pc)
                    NGX_AUX=" src/os/unix/ngx_sunpro_x86.il"
                ;;

                sun4u | sun4v)
                    NGX_AUX=" src/os/unix/ngx_sunpro_sparc64.il"
                ;;

            esac

            case $CPU in

                amd64)
                    NGX_AUX=" src/os/unix/ngx_sunpro_amd64.il"
                ;;

            esac
        ;;

    esac

else

    case $NGX_CC_NAME in
        gcc)
            # gcc 2.7.2.3, 2.8.1, 2.95.4, egcs-1.1.2
            #     3.0.4, 3.1.1, 3.2.3, 3.3.2, 3.3.3, 3.3.4, 3.4.0, 3.4.2
            #     4.0.0, 4.0.1, 4.1.0

            . auto/cc/gcc
        ;;

        clang)
            # Clang C compiler

            . auto/cc/clang
        ;;

        icc)
            # Intel C++ compiler 7.1, 8.0, 8.1

            . auto/cc/icc
        ;;

        sunc)
            # Sun C 5.7 Patch 117837-04 2005/05/11

            . auto/cc/sunc
        ;;

        ccc)
            # Compaq C V6.5-207

            . auto/cc/ccc
        ;;

        acc)
            # aCC: HP ANSI C++ B3910B A.03.55.02

            . auto/cc/acc
        ;;

        msvc*)
            # MSVC++ 6.0 SP2, MSVC++ Toolkit 2003

            . auto/cc/msvc
        ;;

        owc)
            # Open Watcom C 1.0, 1.2

            . auto/cc/owc
        ;;

        bcc)
            # Borland C++ 5.5

            . auto/cc/bcc
        ;;

    esac

    CC_TEST_FLAGS="$CC_TEST_FLAGS $NGX_CC_OPT"

fi

CFLAGS="$CFLAGS $NGX_CC_OPT"
NGX_TEST_LD_OPT="$NGX_LD_OPT"
{% endhighlight %}

上面```$NGX_CC_OPT```变量保存设定的额外C编译器选项。```$NGX_LD_OPT```变量保存设定的额外的链接器选项。其值分别是通过```--with-cc-opt```与```--with-ld-opt```来进行设置的，但是我们配置时均未进行设置。

```CC_TEST_FLAGS```与```NGX_TEST_LD_OPT```主要在auto/feature脚本中测试相应特性时使用。

## 9. 测试相关特性
{% highlight string %}
if [ "$NGX_PLATFORM" != win32 ]; then

    if test -n "$NGX_LD_OPT"; then
        ngx_feature=--with-ld-opt=\"$NGX_LD_OPT\"
        ngx_feature_name=
        ngx_feature_run=no
        ngx_feature_incs=
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test=
        . auto/feature

        if [ $ngx_found = no ]; then
            echo $0: error: the invalid value in --with-ld-opt=\"$NGX_LD_OPT\"
            echo
            exit 1
        fi
    fi


    ngx_feature="-Wl,-E switch"
    ngx_feature_name=
    ngx_feature_run=no
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=-Wl,-E
    ngx_feature_test=
    . auto/feature

    if [ $ngx_found = yes ]; then
        MAIN_LINK="-Wl,-E"
    fi


    ngx_feature="gcc builtin atomic operations"
    ngx_feature_name=NGX_HAVE_GCC_ATOMIC
    ngx_feature_run=yes
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="long  n = 0;
                      if (!__sync_bool_compare_and_swap(&n, 0, 1))
                          return 1;
                      if (__sync_fetch_and_add(&n, 1) != 1)
                          return 1;
                      if (n != 2)
                          return 1;
                      __sync_synchronize();"
    . auto/feature


    if [ "$NGX_CC_NAME" = "ccc" ]; then
        echo "checking for C99 variadic macros ... disabled"
    else
        ngx_feature="C99 variadic macros"
        ngx_feature_name="NGX_HAVE_C99_VARIADIC_MACROS"
        ngx_feature_run=yes
        ngx_feature_incs="#include <stdio.h>
#define var(dummy, ...)  sprintf(__VA_ARGS__)"
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test="char  buf[30]; buf[0] = '0';
                          var(0, buf, \"%d\", 1);
                          if (buf[0] != '1') return 1"
        . auto/feature
     fi


    ngx_feature="gcc variadic macros"
    ngx_feature_name="NGX_HAVE_GCC_VARIADIC_MACROS"
    ngx_feature_run=yes
    ngx_feature_incs="#include <stdio.h>
#define var(dummy, args...)  sprintf(args)"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="char  buf[30]; buf[0] = '0';
                      var(0, buf, \"%d\", 1);
                      if (buf[0] != '1') return 1"
    . auto/feature


    ngx_feature="gcc builtin 64 bit byteswap"
    ngx_feature_name="NGX_HAVE_GCC_BSWAP64"
    ngx_feature_run=no
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="__builtin_bswap64(0)"
    . auto/feature


#    ngx_feature="inline"
#    ngx_feature_name=
#    ngx_feature_run=no
#    ngx_feature_incs="int inline f(void) { return 1 }"
#    ngx_feature_path=
#    ngx_feature_libs=
#    ngx_feature_test=
#    . auto/feature
#
#    if [ $ngx_found = yes ]; then
#    fi

fi
{% endhighlight %}

在非win32平台上测试相关特性。

<br />

**(1) 测试传入的链接选项**
{% highlight string %}
if test -n "$NGX_LD_OPT"; then
        ngx_feature=--with-ld-opt=\"$NGX_LD_OPT\"
        ngx_feature_name=
        ngx_feature_run=no
        ngx_feature_incs=
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test=
        . auto/feature

        if [ $ngx_found = no ]; then
            echo $0: error: the invalid value in --with-ld-opt=\"$NGX_LD_OPT\"
            echo
            exit 1
        fi
    fi
{% endhighlight %}

nginx的configure脚本支持```--with-ld-opt```选项。如果该选项值不为空，则测试是否支持对应的链接选项。

<br />

**(2) 测试```-Wl,-E```**
{% highlight string %}
ngx_feature="-Wl,-E switch"
ngx_feature_name=
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=-Wl,-E
ngx_feature_test=
. auto/feature

if [ $ngx_found = yes ]; then
    MAIN_LINK="-Wl,-E"
fi
{% endhighlight %}

```-Wl,option```的含义是：传递一个选项，该选项作为链接器(linker)的选项值。对于链接器的-E选项，gcc文档是这样解释的：
<pre>
If any of these options is used, then the linker is not run, and object file names
should not be used as arguments. See Section 3.2 [Overall Options], page 24.

见:page 175
</pre>
主要功能就是禁止linker选项中包含一些特定的```选项值```和```文件名```.

<br />

**(3) 测试是否具有内置原子操作**
{% highlight string %}
ngx_feature="gcc builtin atomic operations"
ngx_feature_name=NGX_HAVE_GCC_ATOMIC
ngx_feature_run=yes
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="long  n = 0;
                  if (!__sync_bool_compare_and_swap(&n, 0, 1))
                      return 1;
                  if (__sync_fetch_and_add(&n, 1) != 1)
                      return 1;
                  if (n != 2)
                      return 1;
                  __sync_synchronize();"
. auto/feature
{% endhighlight %}

<br />

**(4) 测试是否支持C99可变参数宏**
{% highlight string %}
    if [ "$NGX_CC_NAME" = "ccc" ]; then
        echo "checking for C99 variadic macros ... disabled"
    else
        ngx_feature="C99 variadic macros"
        ngx_feature_name="NGX_HAVE_C99_VARIADIC_MACROS"
        ngx_feature_run=yes
        ngx_feature_incs="#include <stdio.h>
#define var(dummy, ...)  sprintf(__VA_ARGS__)"
        ngx_feature_path=
        ngx_feature_libs=
        ngx_feature_test="char  buf[30]; buf[0] = '0';
                          var(0, buf, \"%d\", 1);
                          if (buf[0] != '1') return 1"
        . auto/feature
     fi
{% endhighlight %}

<br />

**(5) 测试是否支持gcc可变参数宏**
{% highlight string %}
    ngx_feature="gcc variadic macros"
    ngx_feature_name="NGX_HAVE_GCC_VARIADIC_MACROS"
    ngx_feature_run=yes
    ngx_feature_incs="#include <stdio.h>
#define var(dummy, args...)  sprintf(args)"
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test="char  buf[30]; buf[0] = '0';
                      var(0, buf, \"%d\", 1);
                      if (buf[0] != '1') return 1"
    . auto/feature
{% endhighlight %}

<br />

**(6) 测试是否支持内置的64bit 字节交换**
{% highlight string %}
ngx_feature="gcc builtin 64 bit byteswap"
ngx_feature_name="NGX_HAVE_GCC_BSWAP64"
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test="__builtin_bswap64(0)"
. auto/feature
{% endhighlight %}



<br />
<br />
<br />

