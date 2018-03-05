---
layout: post
title: nginx工具型脚本-part5
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---


我们在分析configure脚本的过程中，发现里面定义了很多工具型的脚本。这里我们统一的来讲述一下nginx工具型脚本。

<!-- more -->


<br />
<br />
参看：http://blog.csdn.net/poechant/article/details/7347046


## 1. auto/have脚本

脚本内容如下：
{% highlight string %}
# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


cat << END >> $NGX_AUTO_CONFIG_H

#ifndef $have
#define $have  1
#endif

END
{% endhighlight %}

(1) 主要功能

向自动配置头文件中标示有指定的参数的宏定义。

(2) 作用对象

```$NGX_AUTO_CONFIG_H```变量所表示的自动生成头文件，默认为objs/ngx_auto_config.h

(3) 示例

如果have值为```SOME_FLAG```，则引用该脚本而运行后，objs/ngx_auto_config.h头文件中将追加如下内容：
{% highlight string %}
#ifndef SOME_FLAG
#define SOME_FLAG 1
#endif
{% endhighlight %}


## 2. auto/nohave脚本
与auto/have类似，这里就不做进一步介绍：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


cat << END >> $NGX_AUTO_CONFIG_H

#ifndef $have
#define $have  0
#endif

END
{% endhighlight %}

(1) 主要功能 

向自动配置头文件中标示没有指定的参数的宏定义。

(2) 作用对象

```$NGX_AUTO_CONFIG_H```变量所表示的自动生成头文件，默认为objs/ngx_auto_config.h

(3) 示例

如果have值为```SOME_FLAG```，则引用该脚本而运行后，objs/ngx_auto_config.h头文件中将追加如下内容：
{% highlight string %}
#ifndef SOME_FLAG
#define SOME_FLAG 0
#endif
{% endhighlight %}



## 3. auto/define脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


cat << END >> $NGX_AUTO_CONFIG_H

#ifndef $have
#define $have  $value
#endif

END
{% endhighlight %}

(1) 主要功能

向自动配置头文件中标示指定参数的值.

(2) 作用对象

```$NGX_AUTO_CONFIG_H```变量所表示的自动生成头文件，默认为objs/ngx_auto_config.h

(3) 示例

如果have值为```SOME_FLAG```，value值为1234,则引用该脚本而运行后，objs/ngx_auto_config.h头文件中将追加如下内容：
{% highlight string %}
#ifndef SOME_FLAG
#define SOME_FLAG 1234
#endif
{% endhighlight %}

## 4. auto/have_headers脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


cat << END >> $NGX_AUTO_HEADERS_H

#ifndef $have
#define $have  1
#endif

END
{% endhighlight %}

(1) 主要功能

向自动头文件中标示指定参数存在.

(2) 作用对象

```$NGX_AUTO_HEADERS_H```变量所表示的自动生成头文件，默认为objs/ngx_auto_headers.h。

```(注：与auto/have的不同主要在于此)```

(3) 示例

如果have值为```SOME_HEADER```，则引用该脚本而运行后，objs/ngx_auto_headers.h头文件中将追加如下内容：
{% highlight string %}
#ifndef SOME_HEADER
#define SOME_HEADER 1
#endif
{% endhighlight %}


## 5. auto/feature脚本

脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


echo $ngx_n "checking for $ngx_feature ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for $ngx_feature

END

ngx_found=no

if test -n "$ngx_feature_name"; then
    ngx_have_feature=`echo $ngx_feature_name \
                   | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ`
fi

if test -n "$ngx_feature_path"; then
    for ngx_temp in $ngx_feature_path; do
        ngx_feature_inc_path="$ngx_feature_inc_path -I $ngx_temp"
    done
fi

cat << END > $NGX_AUTOTEST.c

#include <sys/types.h>
$NGX_INCLUDE_UNISTD_H
$ngx_feature_incs

int main() {
    $ngx_feature_test;
    return 0;
}

END


ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS $ngx_feature_inc_path \
          -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_TEST_LD_OPT $ngx_feature_libs"

ngx_feature_inc_path=

eval "/bin/sh -c \"$ngx_test\" >> $NGX_AUTOCONF_ERR 2>&1"


if [ -x $NGX_AUTOTEST ]; then

    case "$ngx_feature_run" in

        yes)
            # /bin/sh is used to intercept "Killed" or "Abort trap" messages
            if /bin/sh -c $NGX_AUTOTEST >> $NGX_AUTOCONF_ERR 2>&1; then
                echo " found"
                ngx_found=yes

                if test -n "$ngx_feature_name"; then
                    have=$ngx_have_feature . auto/have
                fi

            else
                echo " found but is not working"
            fi
        ;;

        value)
            # /bin/sh is used to intercept "Killed" or "Abort trap" messages
            if /bin/sh -c $NGX_AUTOTEST >> $NGX_AUTOCONF_ERR 2>&1; then
                echo " found"
                ngx_found=yes

                cat << END >> $NGX_AUTO_CONFIG_H

#ifndef $ngx_feature_name
#define $ngx_feature_name  `$NGX_AUTOTEST`
#endif

END
            else
                echo " found but is not working"
            fi
        ;;

        bug)
            # /bin/sh is used to intercept "Killed" or "Abort trap" messages
            if /bin/sh -c $NGX_AUTOTEST >> $NGX_AUTOCONF_ERR 2>&1; then
                echo " not found"

            else
                echo " found"
                ngx_found=yes

                if test -n "$ngx_feature_name"; then
                    have=$ngx_have_feature . auto/have
                fi
            fi
        ;;

        *)
            echo " found"
            ngx_found=yes

            if test -n "$ngx_feature_name"; then
                have=$ngx_have_feature . auto/have
            fi
        ;;

    esac

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

### 5.1 脚本分析

我们首先来分析一下整个脚本的工作过程。

**(1) 格式化提示信息**

还记得我们在```auto/init脚本分析```的文章中介绍过ngx_n和ngx_c两个变量，其实主要是防止换行。 在auto/feature中有如下：
{% highlight string %}
echo $ngx_n "checking for $ngx_feature ...$ngx_c"
{% endhighlight %}

其实就是打印出一句: checking for ```$ngx_feature``` ...，然后换行。当然存在```$ngx_n```和```$ngx_c```都为空的情况，此时就真的会换行了。

<br />

**(2) 文件中生成信息提示**
{% highlight string %}
cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for $ngx_feature

END
{% endhighlight %}

然后向```NGX_AUTOCONF_ERR```表示的文件添加自动配置错误信息。该文件是在auto/init文件中初始化的,其值为:
<pre>
NGX_AUTOCONF_ERR=$NGX_OBJS/autoconf.err
</pre>
默认情况下为objs/autoconf.err

<br />

**(3) 初始化相关变量**
{% highlight string %}
ngx_found=no

if test -n "$ngx_feature_name"; then
    ngx_have_feature=`echo $ngx_feature_name \
                   | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ`
fi

if test -n "$ngx_feature_path"; then
    for ngx_temp in $ngx_feature_path; do
        ngx_feature_inc_path="$ngx_feature_inc_path -I $ngx_temp"
    done
fi
{% endhighlight %}

上述首先初始化ngx_found为no; 接着判断```$ngx_feature_name```长度是否为0，不为0的话则将```$ngx_feature_name```转换成大写保存在ngx_have_feature变量中； 接着再判断```$ngx_feature_path```长度是否为0，不为0的话则在```$ngx_feature_path```中的每一个路径前加上 -I 选项，将结果保存在ngx_feature_inc_path变量中。

<br />

**(4) 生成feature测试程序**
{% highlight string %}
cat << END > $NGX_AUTOTEST.c

#include <sys/types.h>
$NGX_INCLUDE_UNISTD_H
$ngx_feature_incs

int main() {
    $ngx_feature_test;
    return 0;
}

END
{% endhighlight %}


```$NGX_AUTOTEST```是在auto/init脚本中初始化为```$NGX_OBJS/autotest```的，默认为objs/autotest。加上后缀名则为objs/autotest.c。 

其中```$ngx_feature_incs``` 和 ```$ngx_feature_test```都算是auto/feature脚本的参数。

```$NGX_INCLUDE_UNISTD_H```似乎没有地方定义

<br />

**(5) 编译feature测试程序**
{% highlight string %}
ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS $ngx_feature_inc_path \
          -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_TEST_LD_OPT $ngx_feature_libs"

ngx_feature_inc_path=

eval "/bin/sh -c \"$ngx_test\" >> $NGX_AUTOCONF_ERR 2>&1"
{% endhighlight %}

首先ngx_test变量保存编译命令，然后再执行eval，使用ngx_test变量保存的编译命令编译feature测试程序，并将编译输出写到```$NGX_AUTOCONF_ERR```中。

上述eval命令中首先将标准输出(stdout)重定向到了```$NGX_AUTOCONF_ERR```文件中，接着将标准错误(stderr)重定向到了标准输出(stdout), 因此最后标准错误也会重定向到```$NGX_AUTOCONF_ERR```文件中。

ngx_test编译命令中：```$CC_TEST_FLAGS```、```$CC_AUX_FLAGS```、```$NGX_TEST_LD_OPT```一般与编译器相关，我们后续会介绍。

<br />

**(6) 执行测试程序**
{% highlight string %}
if [ -x $NGX_AUTOTEST ]; then

    case "$ngx_feature_run" in

        yes)
            # /bin/sh is used to intercept "Killed" or "Abort trap" messages
            if /bin/sh -c $NGX_AUTOTEST >> $NGX_AUTOCONF_ERR 2>&1; then
                echo " found"
                ngx_found=yes

                if test -n "$ngx_feature_name"; then
                    have=$ngx_have_feature . auto/have
                fi

            else
                echo " found but is not working"
            fi
        ;;

        value)
            # /bin/sh is used to intercept "Killed" or "Abort trap" messages
            if /bin/sh -c $NGX_AUTOTEST >> $NGX_AUTOCONF_ERR 2>&1; then
                echo " found"
                ngx_found=yes

                cat << END >> $NGX_AUTO_CONFIG_H

#ifndef $ngx_feature_name
#define $ngx_feature_name  `$NGX_AUTOTEST`
#endif

END
            else
                echo " found but is not working"
            fi
        ;;

        bug)
            # /bin/sh is used to intercept "Killed" or "Abort trap" messages
            if /bin/sh -c $NGX_AUTOTEST >> $NGX_AUTOCONF_ERR 2>&1; then
                echo " not found"

            else
                echo " found"
                ngx_found=yes

                if test -n "$ngx_feature_name"; then
                    have=$ngx_have_feature . auto/have
                fi
            fi
        ;;

        *)
            echo " found"
            ngx_found=yes

            if test -n "$ngx_feature_name"; then
                have=$ngx_have_feature . auto/have
            fi
        ;;

    esac

else
    echo " not found"

    echo "----------"    >> $NGX_AUTOCONF_ERR
    cat $NGX_AUTOTEST.c  >> $NGX_AUTOCONF_ERR
    echo "----------"    >> $NGX_AUTOCONF_ERR
    echo $ngx_test       >> $NGX_AUTOCONF_ERR
    echo "----------"    >> $NGX_AUTOCONF_ERR
fi
{% endhighlight %}

首先判断```$NGX_AUTOTEST```(即autotest）文件是否存在并且是可执行的，根据条件是否成立分如下两种情况：

**```I ) 文件存在且可执行```**

根据$ngx_feature_run的值不同又可以分成如下几种情况：

* $ngx_feature_run值为yes

执行如下指令：
{% highlight string %}
# /bin/sh is used to intercept "Killed" or "Abort trap" messages
if /bin/sh -c $NGX_AUTOTEST >> $NGX_AUTOCONF_ERR 2>&1; then
    echo " found"
    ngx_found=yes

    if test -n "$ngx_feature_name"; then
        have=$ngx_have_feature . auto/have
    fi

else
    echo " found but is not working"
fi
{% endhighlight %}
执行```$NGX_AUTOTEST```(即autotest)，然后将标准输出以及标准错误都写到autoconf.err中。如果执行autotest返回成功，且```$ngx_feature_name```长度不为0，则调用auto/have脚本向ngx_auto_config.h头文件写入```$ngx_have_feature```特征； 如果执行autotest失败，则打印相应的提示。


* $ngx_feature_run值为value

执行如下指令：
{% highlight string %}
# /bin/sh is used to intercept "Killed" or "Abort trap" messages
if /bin/sh -c $NGX_AUTOTEST >> $NGX_AUTOCONF_ERR 2>&1; then
    echo " found"
    ngx_found=yes

    cat << END >> $NGX_AUTO_CONFIG_H

#ifndef $ngx_feature_name
#define $ngx_feature_name  `$NGX_AUTOTEST`
#endif

END
else
    echo " found but is not working"
fi
{% endhighlight %}

执行```$NGX_AUTOTEST```(即autotest)，然后将标准输出以及标准错误都写到autoconf.err中。如果执行autotest返回成功，则向 ngx_auto_config.h头文件中写入```$ngx_feature_name```宏定义，该宏定义的值为执行autotest的结果。



* $ngx_feature_run值为bug

执行如下指令：
{% highlight string %}
# /bin/sh is used to intercept "Killed" or "Abort trap" messages
if /bin/sh -c $NGX_AUTOTEST >> $NGX_AUTOCONF_ERR 2>&1; then
    echo " not found"

else
    echo " found"
    ngx_found=yes

    if test -n "$ngx_feature_name"; then
        have=$ngx_have_feature . auto/have
    fi
fi
{% endhighlight %}

看到与```$ngx_feature_run值为yes```刚好相反。


* $ngx_feature_run为其他值

执行如下指令：
{% highlight string %}
echo " found"
ngx_found=yes

if test -n "$ngx_feature_name"; then
    have=$ngx_have_feature . auto/have
fi
{% endhighlight %}

直接不执行```$NGX_AUTOTEST```，判断```$ngx_feature_name```长度是否为0，如果非0则调用auto/have脚本向ngx_auto_config.h头文件中写入```$ngx_have_feature```特征。


<br />
<br />

**```II ) 文件不存在或不可执行```**

执行如下部分，将相应信息写到```$NGX_AUTOCONF_ERR```(即autoconf.err)文件中：
{% highlight string %}
echo " not found"

echo "----------"    >> $NGX_AUTOCONF_ERR
cat $NGX_AUTOTEST.c  >> $NGX_AUTOCONF_ERR
echo "----------"    >> $NGX_AUTOCONF_ERR
echo $ngx_test       >> $NGX_AUTOCONF_ERR
echo "----------"    >> $NGX_AUTOCONF_ERR
{% endhighlight %}


### 5.2 主要功能
auto/feature脚本主要用于检测当前系统是否具有某项特性，如果有相应的特性，则通过ngx_found=yes返回，否则通过ngx_found=no返回。

### 5.3 处理变量
主要处理的变量有：
{% highlight string %}
$ngx_n
$ngx_c

$ngx_feature
$ngx_feature_name
$ngx_feature_run
$ngx_feature_incs
$ngx_feature_path
$ngx_feature_libs
$ngx_feature_test

$NGX_AUTOCONF_ERR
$NGX_AUTOTEST

$NGX_INCLUDE_UNISTD_H 

$CC
$CC_TEST_FLAGS
$CC_AUX_FLAGS
$NGX_TEST_LD_OPT
{% endhighlight %}


### 5.4 示例

(1) 测试当前系统是否具有-pipe特性
{% highlight string %}
CC_TEST_FLAGS="-pipe"

ngx_feature="gcc -pipe switch"
ngx_feature_name=
ngx_feature_run=no
ngx_feature_incs=
ngx_feature_path=
ngx_feature_libs=
ngx_feature_test=
. auto/feature
{% endhighlight %}

<br />

(2) 检查当前编译器是否支持gcc可变参数宏
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

### 5.5 删除测试程序
执行如下命令删除测试程序：
{% highlight string %}
rm -rf $NGX_AUTOTEST*
{% endhighlight %}



<br />
<br />
<br />

