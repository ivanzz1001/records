---
layout: post
title: auto/init脚本解析-part3
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

在configure中运行完auto/option脚本之后，接着就会运行auto/init脚本。其主要是定义生成的文件名。

参看：```http://blog.csdn.net/Poechant/article/details/7327211```

<!-- more -->


## 1. auto/init脚本

在分析脚本之前，我们这里贴出其源代码：
{% highlight string %}

# Copyright (C) Igor Sysoev
# Copyright (C) Nginx, Inc.


NGX_MAKEFILE=$NGX_OBJS/Makefile
NGX_MODULES_C=$NGX_OBJS/ngx_modules.c

NGX_AUTO_HEADERS_H=$NGX_OBJS/ngx_auto_headers.h
NGX_AUTO_CONFIG_H=$NGX_OBJS/ngx_auto_config.h

NGX_AUTOTEST=$NGX_OBJS/autotest
NGX_AUTOCONF_ERR=$NGX_OBJS/autoconf.err

# STUBs
NGX_ERR=$NGX_OBJS/autoconf.err
MAKEFILE=$NGX_OBJS/Makefile


NGX_PCH=
NGX_USE_PCH=


# check the echo's "-n" option and "\c" capability

if echo "test\c" | grep c >/dev/null; then

    if echo -n test | grep n >/dev/null; then
        ngx_n=
        ngx_c=

    else
        ngx_n=-n
        ngx_c=
    fi

else
    ngx_n=
    ngx_c='\c'
fi


# create Makefile

cat << END > Makefile

default:	build

clean:
	rm -rf Makefile $NGX_OBJS
END

{% endhighlight %}


## 2. Makefile文件名变量

默认情况下是:objs/Makefile
{% highlight string %}
NGX_MAKEFILE=$NGX_OBJS/Makefile
{% endhighlight %}

## 3. 源文件名变量
默认情况下是:objs/ngx_modules.c
{% highlight string %}
NGX_MODULES_C=$NGX_OBJS/ngx_modules.c
{% endhighlight %}

## 4.




<br />
<br />
<br />

