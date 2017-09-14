---
layout: post
title: auto/cc/name脚本分析-part6
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---


在分析编译器配置总控代码之前，我们先来分析auto/cc/name脚本，此脚本主要是完成编译器名称的设置，从而在后续脚本中可以根据编译器名称选择对应的编译器。

参看：http://blog.csdn.net/poechant/article/details/7355325


<!-- more -->



## 1. auto/cc/name脚本

在进行具体的脚本分析之前，我们这里贴出其源代码：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


if [ "$NGX_PLATFORM" != win32 ]; then

    ngx_feature="C compiler"
    ngx_feature_name=
    ngx_feature_run=yes
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test=
    . auto/feature

    if [ $ngx_found = no ]; then
        echo
        echo $0: error: C compiler $CC is not found
        echo
        exit 1
    fi

fi


if [ "$CC" = cl ]; then
    NGX_CC_NAME=msvc
    echo " + using Microsoft Visual C++ compiler"

elif [ "$CC" = wcl386 ]; then
    NGX_CC_NAME=owc
    echo " + using Open Watcom C compiler"

elif [ "$CC" = bcc32 ]; then
    NGX_CC_NAME=bcc
    echo " + using Borland C++ compiler"

elif `$CC -V 2>&1 | grep '^Intel(R) C' >/dev/null 2>&1`; then
    NGX_CC_NAME=icc
    echo " + using Intel C++ compiler"

elif `$CC -v 2>&1 | grep 'gcc version' >/dev/null 2>&1`; then
    NGX_CC_NAME=gcc
    echo " + using GNU C compiler"

elif `$CC -v 2>&1 | grep '\(clang\|LLVM\) version' >/dev/null 2>&1`; then
    NGX_CC_NAME=clang
    echo " + using Clang C compiler"

elif `$CC -V 2>&1 | grep 'Sun C' >/dev/null 2>&1`; then
    NGX_CC_NAME=sunc
    echo " + using Sun C compiler"

elif `$CC -V 2>&1 | grep '^Compaq C' >/dev/null 2>&1`; then
    NGX_CC_NAME=ccc
    echo " + using Compaq C compiler"

elif `$CC -V 2>&1 | grep '^aCC: ' >/dev/null 2>&1`; then
    NGX_CC_NAME=acc
    echo " + using HP aC++ compiler"

else
    NGX_CC_NAME=unknown

fi
{% endhighlight %}

此脚本主要是根据```$CC```的名称设置```NGX_CC_NAME```的值。如下我们队该脚本进行简单分析。

## 2. C编译器特征判断

脚本代码如下：
{% highlight string %}
if [ "$NGX_PLATFORM" != win32 ]; then

    ngx_feature="C compiler"
    ngx_feature_name=
    ngx_feature_run=yes
    ngx_feature_incs=
    ngx_feature_path=
    ngx_feature_libs=
    ngx_feature_test=
    . auto/feature

    if [ $ngx_found = no ]; then
        echo
        echo $0: error: C compiler $CC is not found
        echo
        exit 1
    fi

fi
{% endhighlight %}

Windows 平台的编译器叫做MSVC，其他平台的都统称为C Compiler。上述脚本首先判断是否为win32平台，如果不是的话，则判断其是否具有c编译器特征。根据前面我们队auto/feature脚本的介绍，其会生成如下内容的autotest.c文件：
{% highlight string %}
#include <sys/types.h>

int main() {
	;
	return 0;
}
{% endhighlight %}

编译并运行此脚本，ngx_found值会被设置为yes。

假如ngx_found值为no的话，则会打印出：
<pre>
./auto/cc/name: error: C compiler $CC is not found
</pre>
然后退出整个脚本的运行。


## 3. 设置NGX_CC_NAME

脚本内容如下：
{% highlight string %}
if [ "$CC" = cl ]; then
    NGX_CC_NAME=msvc
    echo " + using Microsoft Visual C++ compiler"

elif [ "$CC" = wcl386 ]; then
    NGX_CC_NAME=owc
    echo " + using Open Watcom C compiler"

elif [ "$CC" = bcc32 ]; then
    NGX_CC_NAME=bcc
    echo " + using Borland C++ compiler"

elif `$CC -V 2>&1 | grep '^Intel(R) C' >/dev/null 2>&1`; then
    NGX_CC_NAME=icc
    echo " + using Intel C++ compiler"

elif `$CC -v 2>&1 | grep 'gcc version' >/dev/null 2>&1`; then
    NGX_CC_NAME=gcc
    echo " + using GNU C compiler"

elif `$CC -v 2>&1 | grep '\(clang\|LLVM\) version' >/dev/null 2>&1`; then
    NGX_CC_NAME=clang
    echo " + using Clang C compiler"

elif `$CC -V 2>&1 | grep 'Sun C' >/dev/null 2>&1`; then
    NGX_CC_NAME=sunc
    echo " + using Sun C compiler"

elif `$CC -V 2>&1 | grep '^Compaq C' >/dev/null 2>&1`; then
    NGX_CC_NAME=ccc
    echo " + using Compaq C compiler"

elif `$CC -V 2>&1 | grep '^aCC: ' >/dev/null 2>&1`; then
    NGX_CC_NAME=acc
    echo " + using HP aC++ compiler"

else
    NGX_CC_NAME=unknown

fi
{% endhighlight %}
上述脚本设置```NGX_CC_NAME```，并且打印出对应的信息。







<br />
<br />
<br />

