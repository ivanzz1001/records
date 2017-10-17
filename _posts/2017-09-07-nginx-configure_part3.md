---
layout: post
title: auto/init脚本解析-part3
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

在configure中运行完auto/option脚本之后，接着就会运行auto/init脚本。其主要是定义生成的文件名。

<br />
<br />


<!-- more -->

参看：```http://blog.csdn.net/poechant/article/details/7327206```

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


**(1) Makefile文件名变量**

默认情况下是:objs/Makefile
{% highlight string %}
NGX_MAKEFILE=$NGX_OBJS/Makefile
{% endhighlight %}

**(2) 源文件名变量**

默认情况下是:objs/ngx_modules.c
{% highlight string %}
NGX_MODULES_C=$NGX_OBJS/ngx_modules.c
{% endhighlight %}

**(3) 头文件名变量**

默认情况下是：
* objs/ngx_auto_header.h
* objs/ngx_auto_config.h

{% highlight string %}
NGX_AUTO_HEADERS_H=$NGX_OBJS/ngx_auto_headers.h
NGX_AUTO_CONFIG_H=$NGX_OBJS/ngx_auto_config.h
{% endhighlight %}


**(4) 自动测试文件名和配置错误文件名变量**

默认情况下是：
* objs/autotest
* objs/autoconf.err

{% highlight string %}
NGX_AUTOTEST=$NGX_OBJS/autotest
NGX_AUTOCONF_ERR=$NGX_OBJS/autoconf.err
{% endhighlight %}


**(5) STUBS相关变量**

默认情况下是：
* objs/autoconf.err
* objs/Makefile

{% highlight string %}
# STUBs
NGX_ERR=$NGX_OBJS/autoconf.err
MAKEFILE=$NGX_OBJS/Makefile
{% endhighlight %}


**(6) PCH相关变量**
{% highlight string %}
NGX_PCH=
NGX_USE_PCH=
{% endhighlight %}

**(7) 测试所在环境下"-n"和"\c"**

由于Nginx支持多种操作系统，比如Mac OS、Linux、Solaris等，不同的系统下的shell也有小的差别。所以在auto/init脚本中有如下两个变量：
* ngx_n: 若为空，表示会在末尾添加一个换行符；否则值为-n表示不在末尾添加一个换行符。
* ngx_c：若为空，则不具备退行功能；否则值为\c表示具有退行功能。

{% highlight string %}
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
{% endhighlight %}

说明：

在UNIX下，echo输出内容以后，默认会添加输出一个换行符，以便下一次echo从下一行开始输出。而在输出内容末尾添加一个\c可以改变这一规则，输出内容完毕后光标仍然在本行，下一个echo的内功接在后面。



**(8) 创建并写入Makefile**
{% highlight string %}
# create Makefile

cat << END > Makefile

default:	build

clean:
	rm -rf Makefile $NGX_OBJS
END
{% endhighlight %}


<br />
<br />
<br />

