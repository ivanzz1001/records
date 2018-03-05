---
layout: post
title: auto/threads脚本分析-part14
tags:
- nginx
categories: nginx
description: nginx编译脚本解析
---

本节我们介绍auto/threads脚本，其主要用于线程相关的配置。


<!-- more -->


## auto/threads脚本

脚本内容如下：
{% highlight string %}

# Copyright (C) Nginx, Inc.


if [ $USE_THREADS = YES ]; then

    if [ "$NGX_PLATFORM" = win32 ]; then
        cat << END

$0: --with-threads is not supported on Windows

END
        exit 1
    fi

    have=NGX_THREADS . auto/have
    CORE_DEPS="$CORE_DEPS $THREAD_POOL_DEPS"
    CORE_SRCS="$CORE_SRCS $THREAD_POOL_SRCS"
    CORE_LIBS="$CORE_LIBS -lpthread"
fi
{% endhighlight %}

下面进行脚本分析：

在auto/options中,```USE_THREADS```默认会被初始化为```NO```，我们可以通过```--with-threads```选项来启用。


如果```USE_THREADS```值为```YES```，接着判断是否为win32平台。如果是,则打印相关信息退出；否则执行如下：
{% highlight string %}
have=NGX_THREADS . auto/have
CORE_DEPS="$CORE_DEPS $THREAD_POOL_DEPS"
CORE_SRCS="$CORE_SRCS $THREAD_POOL_SRCS"
CORE_LIBS="$CORE_LIBS -lpthread"
{% endhighlight %}

首先向objs/ngx_auto_config.h头文件中写入相关宏定义，然后再把相应的源代码文件包含到```CORE_DEPS```和```CORE_SRCS```中:
<pre>
THREAD_POOL_DEPS=src/core/ngx_thread_pool.h
THREAD_POOL_SRCS="src/core/ngx_thread_pool.c
                  src/os/unix/ngx_thread_cond.c
                  src/os/unix/ngx_thread_mutex.c
                  src/os/unix/ngx_thread_id.c"
</pre>
上述是在auto/sources脚本中定义的。

<br />
注意： 虽然nginx整体上是一个异步、事件驱动的框架。但是很多第三方模块使用了阻塞调用；即使在当前官方的NGINX代码中，依然无法在全部场景中避免使用阻塞，Nginx1.7.11中实现的线程池机制解决了这个问题。




<br />
<br />
<br />

