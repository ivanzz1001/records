---
layout: post
title: auto/types脚本分析-part11
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

本节我们主要分析auto/types目录下的4个脚本文件：```sizeof脚本```,```typedef脚本```,```uintptr_t脚本```以及```value脚本```。


<!-- more -->

顾名思义，auto/types目录下的这些脚本主要就是处理类型相关的一些工作。


## 1. auto/types/sizeof脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


echo $ngx_n "checking for $ngx_type size ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for $ngx_type size

END

ngx_size=

cat << END > $NGX_AUTOTEST.c

#include <sys/types.h>
#include <sys/time.h>
$NGX_INCLUDE_UNISTD_H
#include <signal.h>
#include <stdio.h>
#include <sys/resource.h>
$NGX_INCLUDE_INTTYPES_H
$NGX_INCLUDE_AUTO_CONFIG_H

int main() {
    printf("%d", (int) sizeof($ngx_type));
    return 0;
}

END


ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS \
          -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_LD_OPT $ngx_feature_libs"

eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"


if [ -x $NGX_AUTOTEST ]; then
    ngx_size=`$NGX_AUTOTEST`
    echo " $ngx_size bytes"
fi


case $ngx_size in
    4)
        ngx_max_value=2147483647
        ngx_max_len='(sizeof("-2147483648") - 1)'
    ;;

    8)
        ngx_max_value=9223372036854775807LL
        ngx_max_len='(sizeof("-9223372036854775808") - 1)'
    ;;

    *)
        echo
        echo "$0: error: can not detect $ngx_type size"

        echo "----------"    >> $NGX_AUTOCONF_ERR
        cat $NGX_AUTOTEST.c  >> $NGX_AUTOCONF_ERR
        echo "----------"    >> $NGX_AUTOCONF_ERR
        echo $ngx_test       >> $NGX_AUTOCONF_ERR
        echo "----------"    >> $NGX_AUTOCONF_ERR

        rm -rf $NGX_AUTOTEST*

        exit 1
esac


rm -rf $NGX_AUTOTEST*
{% endhighlight %}
下面我们对脚本进行分析：

### 1.1 打印相关提示信息
分别向控制台及autoconf.err文件中打印相应的提示信息：
{% highlight string %}
echo $ngx_n "checking for $ngx_type size ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for $ngx_type size

END
{% endhighlight %}

```$ngx_n```与```$ngx_c```我们前面讲述过，主要是为了禁止换行。```$ngx_type```为当前我们需要检查的数据类型。

### 1.2 生成并编译程序
{% highlight string %}
ngx_size=

cat << END > $NGX_AUTOTEST.c

#include <sys/types.h>
#include <sys/time.h>
$NGX_INCLUDE_UNISTD_H
#include <signal.h>
#include <stdio.h>
#include <sys/resource.h>
$NGX_INCLUDE_INTTYPES_H
$NGX_INCLUDE_AUTO_CONFIG_H

int main() {
    printf("%d", (int) sizeof($ngx_type));
    return 0;
}

END


ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS \
          -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_LD_OPT $ngx_feature_libs"

eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"
{% endhighlight %}
将ngx_size值置为空，接着生成objs/autotest.c程序，然后编译。这里```$NGX_LD_OPT```与```$ngx_feature_libs```值为空.

```NGX_INCLUDE_UNISTD_H```与```NGX_INCLUDE_INTTYPES_H```通过在auto/headers脚本调用auto/include分别被设置为了```#include<unistd.h>```与```#include <inttypes.h>```。而```NGX_INCLUDE_AUTO_CONFIG_H```会在auto/unix脚本中根据需要进行设置。

### 1.3 执行程序，并根据执行结果进行处理
{% highlight string %}
if [ -x $NGX_AUTOTEST ]; then
    ngx_size=`$NGX_AUTOTEST`
    echo " $ngx_size bytes"
fi


case $ngx_size in
    4)
        ngx_max_value=2147483647
        ngx_max_len='(sizeof("-2147483648") - 1)'
    ;;

    8)
        ngx_max_value=9223372036854775807LL
        ngx_max_len='(sizeof("-9223372036854775808") - 1)'
    ;;

    *)
        echo
        echo "$0: error: can not detect $ngx_type size"

        echo "----------"    >> $NGX_AUTOCONF_ERR
        cat $NGX_AUTOTEST.c  >> $NGX_AUTOCONF_ERR
        echo "----------"    >> $NGX_AUTOCONF_ERR
        echo $ngx_test       >> $NGX_AUTOCONF_ERR
        echo "----------"    >> $NGX_AUTOCONF_ERR

        rm -rf $NGX_AUTOTEST*

        exit 1
esac
{% endhighlight %}

判断生成的objs/autotest文件是否可执行，可执行的话则执行程序并将结果保存到ngx_size变量中。然后根据ngx_size的值计算出对应的最大值保存在ngx_max_value变量中，最大长度值保存在ngx_max_len变量中。

### 1.4 删除生成的测试程序
{% highlight string %}
rm -rf $NGX_AUTOTEST*
{% endhighlight %}


## 2. auto/types/typedef脚本

脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


echo $ngx_n "checking for $ngx_type ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for $ngx_type

END

ngx_found=no

for ngx_try in $ngx_type $ngx_types
do

    cat << END > $NGX_AUTOTEST.c

#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
$NGX_INCLUDE_INTTYPES_H

int main() {
    $ngx_try i = 0;
    return (int) i;
}

END

    ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS \
              -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_LD_OPT $ngx_feature_libs"

    eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"

    if [ -x $NGX_AUTOTEST ]; then
        if [ $ngx_try = $ngx_type ]; then
            echo " found"
            ngx_found=yes
        else
            echo ", $ngx_try used"
            ngx_found=$ngx_try
        fi
    fi

    if [ $ngx_found = no ]; then
        if [ $ngx_try = $ngx_type ]; then
            echo $ngx_n " $ngx_try not found$ngx_c"
        else
            echo $ngx_n ", $ngx_try not found$ngx_c"
        fi

        echo "----------"    >> $NGX_AUTOCONF_ERR
        cat $NGX_AUTOTEST.c  >> $NGX_AUTOCONF_ERR
        echo "----------"    >> $NGX_AUTOCONF_ERR
        echo $ngx_test       >> $NGX_AUTOCONF_ERR
        echo "----------"    >> $NGX_AUTOCONF_ERR
    fi

    rm -rf $NGX_AUTOTEST*

    if [ $ngx_found != no ]; then
        break
    fi
done

if [ $ngx_found = no ]; then
    echo
    echo "$0: error: can not define $ngx_type"

    exit 1
fi

if [ $ngx_found != yes ]; then
    echo "typedef $ngx_found  $ngx_type;"   >> $NGX_AUTO_CONFIG_H
fi
{% endhighlight %}

### 2.1 打印相关提示信息
{% highlight string %}
echo $ngx_n "checking for $ngx_type ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for $ngx_type

END
{% endhighlight %}
```$ngx_n```与```$ngx_c```我们前面讲述过，主要是为了禁止换行。```$ngx_type```为当前我们需要检查的数据类型。

### 2.2 循环检查对应的类型
{% highlight string %}
ngx_found=no

for ngx_try in $ngx_type $ngx_types
do

    cat << END > $NGX_AUTOTEST.c

#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>
$NGX_INCLUDE_INTTYPES_H

int main() {
    $ngx_try i = 0;
    return (int) i;
}

END

    ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS \
              -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_LD_OPT $ngx_feature_libs"

    eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"

    if [ -x $NGX_AUTOTEST ]; then
        if [ $ngx_try = $ngx_type ]; then
            echo " found"
            ngx_found=yes
        else
            echo ", $ngx_try used"
            ngx_found=$ngx_try
        fi
    fi

    if [ $ngx_found = no ]; then
        if [ $ngx_try = $ngx_type ]; then
            echo $ngx_n " $ngx_try not found$ngx_c"
        else
            echo $ngx_n ", $ngx_try not found$ngx_c"
        fi

        echo "----------"    >> $NGX_AUTOCONF_ERR
        cat $NGX_AUTOTEST.c  >> $NGX_AUTOCONF_ERR
        echo "----------"    >> $NGX_AUTOCONF_ERR
        echo $ngx_test       >> $NGX_AUTOCONF_ERR
        echo "----------"    >> $NGX_AUTOCONF_ERR
    fi

    rm -rf $NGX_AUTOTEST*

    if [ $ngx_found != no ]; then
        break
    fi
done
{% endhighlight %}

上面循环检查```$ngx_type```与```$ngx_types```，尝试找到一个可用的类型。


### 2.3 后续处理
{% highlight string %}
if [ $ngx_found = no ]; then
    echo
    echo "$0: error: can not define $ngx_type"

    exit 1
fi

if [ $ngx_found != yes ]; then
    echo "typedef $ngx_found  $ngx_type;"   >> $NGX_AUTO_CONFIG_H
fi
{% endhighlight %}
如果找到的可用类型不是```$ngx_type```的话，则用typedef定义之，并将结果写到objs/ngx_auto_config.h头文件中。

## 3. auto/types/uintptr_t脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


echo $ngx_n "checking for uintptr_t ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for uintptr_t

END

found=no

cat << END > $NGX_AUTOTEST.c

#include <sys/types.h>
$NGX_INTTYPES_H

int main() {
    uintptr_t i = 0;
    return (int) i;
}

END

ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS \
          -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_LD_OPT"

eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"

if [ -x $NGX_AUTOTEST ]; then
    echo " uintptr_t found"
    found=yes
else
    echo $ngx_n " uintptr_t not found" $ngx_c
fi

rm -rf $NGX_AUTOTEST*


if [ $found = no ]; then
    found="uint`expr 8 \* $ngx_ptr_size`_t"
    echo ", $found used"

    echo "typedef $found  uintptr_t;"                   >> $NGX_AUTO_CONFIG_H
    echo "typedef $found  intptr_t;" | sed -e 's/u//g'  >> $NGX_AUTO_CONFIG_H
fi
{% endhighlight %}
下面进行脚本分析：

### 3.1 打印相关提示信息
{% highlight string %}
echo $ngx_n "checking for uintptr_t ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for uintptr_t

END
{% endhighlight %}

```$ngx_n```与```$ngx_c```我们前面讲述过，主要是为了禁止换行。

### 3.2 生成并编译程序
{% highlight string %}
found=no

cat << END > $NGX_AUTOTEST.c

#include <sys/types.h>
$NGX_INTTYPES_H

int main() {
    uintptr_t i = 0;
    return (int) i;
}

END

ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS \
          -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_LD_OPT"

eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"
{% endhighlight %}

```NGX_INTTYPES_H```在auto/headers脚本调用中被设置:
<pre>
auto/include脚本如下行：

eval "NGX_$ngx_name='#include <$ngx_include>'"
</pre>

### 3.3 执行程序，并根据结果进行处理
{% highlight string %}
if [ -x $NGX_AUTOTEST ]; then
    echo " uintptr_t found"
    found=yes
else
    echo $ngx_n " uintptr_t not found" $ngx_c
fi

rm -rf $NGX_AUTOTEST*


if [ $found = no ]; then
    found="uint`expr 8 \* $ngx_ptr_size`_t"
    echo ", $found used"

    echo "typedef $found  uintptr_t;"                   >> $NGX_AUTO_CONFIG_H
    echo "typedef $found  intptr_t;" | sed -e 's/u//g'  >> $NGX_AUTO_CONFIG_H
fi
{% endhighlight %}

判断生成的程序是否可执行，可执行的话将found置为yes，否则会用调用如下语句：
{% highlight string %}
found="uint`expr 8 \* $ngx_ptr_size`_t"
{% endhighlight %}
这里```$ngx_ptr_size```为我们在文章开头auto/types/sizeof脚本求的当前机器字长（在/auto/unix脚本中调用获得）。

然后将found通过typedef的方式定义成uintptr_t并写入到objs/ngx_auto_config.h头文件中。


## 4. auto/types/value脚本
脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


cat << END >> $NGX_AUTO_CONFIG_H

#ifndef $ngx_param
#define $ngx_param  $ngx_value
#endif

END
{% endhighlight %}

向objs/ngx_auto_config.h头文件写入相应的宏定义。





<br />
<br />
<br />

