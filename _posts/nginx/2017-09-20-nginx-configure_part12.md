---
layout: post
title: auto/endianness脚本分析-part12
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

本节我们介绍auto/endianness脚本，其主要用于识别当前主机的大小端格式。


<!-- more -->


## 1. auto/endianness脚本

脚本内容如下：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


echo $ngx_n "checking for system byte ordering ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for system byte ordering

END


cat << END > $NGX_AUTOTEST.c

int main() {
    int i = 0x11223344;
    char *p;

    p = (char *) &i;
    if (*p == 0x44) return 0;
    return 1;
}

END

ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS \
          -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_LD_OPT $ngx_feature_libs"

eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"

if [ -x $NGX_AUTOTEST ]; then
    if $NGX_AUTOTEST >/dev/null 2>&1; then
        echo " little endian"
        have=NGX_HAVE_LITTLE_ENDIAN . auto/have
    else
        echo " big endian"
    fi

    rm -rf $NGX_AUTOTEST*

else
    rm -rf $NGX_AUTOTEST*

    echo
    echo "$0: error: cannot detect system byte ordering"
    exit 1
fi
{% endhighlight %}

下面我们对脚本内容进行分析：

### 1.1 打印相关提示信息
{% highlight string %}
echo $ngx_n "checking for system byte ordering ...$ngx_c"

cat << END >> $NGX_AUTOCONF_ERR

----------------------------------------
checking for system byte ordering

END
{% endhighlight %}

```$ngx_n```与```$ngx_c```我们前面讲述过，主要是为了禁止换行。


### 1.2 生成并编译程序
{% highlight string %}
cat << END > $NGX_AUTOTEST.c

int main() {
    int i = 0x11223344;
    char *p;

    p = (char *) &i;
    if (*p == 0x44) return 0;
    return 1;
}

END

ngx_test="$CC $CC_TEST_FLAGS $CC_AUX_FLAGS \
          -o $NGX_AUTOTEST $NGX_AUTOTEST.c $NGX_LD_OPT $ngx_feature_libs"

eval "$ngx_test >> $NGX_AUTOCONF_ERR 2>&1"
{% endhighlight %}
生成objs/autotest.c程序，然后编译。这里```$NGX_LD_OPT```与```$ngx_feature_libs```值为空.


### 1.3 执行程序，并根据执行结果进行处理
{% highlight string %}
if [ -x $NGX_AUTOTEST ]; then
    if $NGX_AUTOTEST >/dev/null 2>&1; then
        echo " little endian"
        have=NGX_HAVE_LITTLE_ENDIAN . auto/have
    else
        echo " big endian"
    fi

    rm -rf $NGX_AUTOTEST*

else
    rm -rf $NGX_AUTOTEST*

    echo
    echo "$0: error: cannot detect system byte ordering"
    exit 1
fi
{% endhighlight %}
如果程序不存在或不可执行，则直接报错退出；否则根据可执行程序objs/autotest的执行结果，返回0表示为小端格式，返回1则表示为大端格式。




<br />
<br />
<br />

