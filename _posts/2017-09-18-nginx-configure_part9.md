---
layout: post
title: auto/headers脚本分析-part9
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

本节我们介绍auto/headers脚本，其主要用于包含类Unix操作系统下的头文件。所关联的脚本文件主要有两个：

* auto/include脚本文件
* auto/headers脚本文件




<!-- more -->

## 1. auto/include脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


echo $ngx_n "checking for $ngx_include ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for $ngx_include

END


ngx_found=no

cat << END > $NGX_AUTOTEST.c

$NGX_INCLUDE_SYS_PARAM_H
#include <$ngx_include>

int main() {
    return 0;
}

END


ngx_test="$CC -o $NGX_AUTOTEST $NGX_AUTOTEST.c"

eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"

if [ -x $NGX_AUTOTEST ]; then

    ngx_found=yes

    echo " found"

    ngx_name=`echo $ngx_include \
              | tr abcdefghijklmnopqrstuvwxyz/. ABCDEFGHIJKLMNOPQRSTUVWXYZ__`


    have=NGX_HAVE_$ngx_name . auto/have_headers

    eval "NGX_INCLUDE_$ngx_name='#include <$ngx_include>'"

    #STUB
    eval "NGX_$ngx_name='#include <$ngx_include>'"

else
    echo " not found"

    echo "----------"    >> $NGX_AUTOCONF_ERR
    cat $NGX_AUTOTEST.c  >> $NGX_AUTOCONF_ERR
    echo "----------"    >> $NGX_AUTOCONF_ERR
    echo $ngx_test       >> $NGX_AUTOCONF_ERR
    echo "----------"    >> $NGX_AUTOCONF_ERR
fi

rm -rf $NGX_AUTOTEST*
{% endhighlight %}

下面我们对脚本进行分析。

### 1.1 打印相关提示信息

分别向控制台及autoconf.err文件中打印相应的提示信息：
{% highlight string %}
echo $ngx_n "checking for $ngx_include ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for $ngx_include

END
{% endhighlight %}

```$ngx_n```及```$ngx_c```我们前面讲述过，分别指相应的换行符和退行符。```$ngx_include```这里为所需要包含的头文件。

### 1.2 生成测试程序并编译
如下首先将ngx_found的值置为no,然后生成autotest.c源文件并编译。
{% highlight string %}
ngx_found=no

cat << END > $NGX_AUTOTEST.c

$NGX_INCLUDE_SYS_PARAM_H
#include <$ngx_include>

int main() {
    return 0;
}

END


ngx_test="$CC -o $NGX_AUTOTEST $NGX_AUTOTEST.c"

eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"
{% endhighlight %}

### 1.3 处理程序运行结果
{% highlight string %}
if [ -x $NGX_AUTOTEST ]; then

    ngx_found=yes

    echo " found"

    ngx_name=`echo $ngx_include \
              | tr abcdefghijklmnopqrstuvwxyz/. ABCDEFGHIJKLMNOPQRSTUVWXYZ__`


    have=NGX_HAVE_$ngx_name . auto/have_headers

    eval "NGX_INCLUDE_$ngx_name='#include <$ngx_include>'"

    #STUB
    eval "NGX_$ngx_name='#include <$ngx_include>'"

else
    echo " not found"

    echo "----------"    >> $NGX_AUTOCONF_ERR
    cat $NGX_AUTOTEST.c  >> $NGX_AUTOCONF_ERR
    echo "----------"    >> $NGX_AUTOCONF_ERR
    echo $ngx_test       >> $NGX_AUTOCONF_ERR
    echo "----------"    >> $NGX_AUTOCONF_ERR
fi
{% endhighlight %}

根据生成的程序是否可执行，分成如下两种情况处理：

**(1) 程序存在并且可执行**

将ngx_found变量置为yes,同时对```$ngx_include```变量所引用的头文件名将小写转换成成大写，将```/.```转换成```__```保存在变量```$ngx_name```中。

接着再调用auto/have_headers脚本文件向ngx_auto_headers.h头文件中写入```NGX_HAVE_$ngx_name```的宏定义。

最后再赋值给```NGX_INCLUDE_$ngx_name```和```NGX_$ngx_name```这两个变量，用于保存对应的头文件信息。


**(2) 程序不存在或者不可执行**

向控制台及objs/autoconf.err文件打印相应的信息。

### 1.4 删除生成的测试程序
{% highlight string %}
rm -rf $NGX_AUTOTEST*
{% endhighlight %}


## 2. auto/headers脚本

脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


ngx_include="unistd.h";      . auto/include
ngx_include="inttypes.h";    . auto/include
ngx_include="limits.h";      . auto/include
ngx_include="sys/filio.h";   . auto/include
ngx_include="sys/param.h";   . auto/include
ngx_include="sys/mount.h";   . auto/include
ngx_include="sys/statvfs.h"; . auto/include
ngx_include="crypt.h";       . auto/include
{% endhighlight %}

脚本比较简单，就是测试下述头文件是否存在。存在的话向对应的头文件写入相关宏定义；不存在的话则打印出相应的提示信息。

* unistd.h头文件
* inttypes.h头文件
* limits.h头文件
* sys/filio.h头文件
* sys/param.h头文件
* sys/mount.h头文件
* sys/statvfs.h头文件
* crypt.h头文件

<br />
<br />
<br />

