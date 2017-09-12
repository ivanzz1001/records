---
layout: post
title: nginx工具型脚本-part5
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---


我们在分析configure脚本的过程中，发现里面定义了很多工具型的脚本。这里我们统一的来讲述一下nginx工具型脚本。

参看：http://blog.csdn.net/poechant/article/details/7347046


<!-- more -->


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

如果have值为SOME_FLAG，则引用该脚本而运行后，objs/ngx_auto_config.h头文件中将追加如下内容：
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

如果have值为SOME_FLAG，则引用该脚本而运行后，objs/ngx_auto_config.h头文件中将追加如下内容：
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

如果have值为SOME_FLAG，value值为1234,则引用该脚本而运行后，objs/ngx_auto_config.h头文件中将追加如下内容：
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

如果have值为SOME_HEADER，则引用该脚本而运行后，objs/ngx_auto_headers.h头文件中将追加如下内容：
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




<br />
<br />
<br />

