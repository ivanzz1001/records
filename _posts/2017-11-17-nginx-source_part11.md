---
layout: post
title: os/unix/ngx_dlopen.c(h)源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---



本节我们主要介绍一下nginx中ngx_dlopen.c加载动态链接库相关函数。

<!-- more -->


## 1. os/unix/ngx_dlopen.h头文件

头文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Maxim Dounin
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_DLOPEN_H_INCLUDED_
#define _NGX_DLOPEN_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define ngx_dlopen(path)           dlopen((char *) path, RTLD_NOW | RTLD_GLOBAL)
#define ngx_dlopen_n               "dlopen()"

#define ngx_dlsym(handle, symbol)  dlsym(handle, symbol)
#define ngx_dlsym_n                "dlsym()"

#define ngx_dlclose(handle)        dlclose(handle)
#define ngx_dlclose_n              "dlclose()"


#if (NGX_HAVE_DLOPEN)
char *ngx_dlerror(void);
#endif


#endif /* _NGX_DLOPEN_H_INCLUDED_ */

{% endhighlight %}

这里主要是定义了动态链接库加载相关的函数：

* ngx_dlopen(path)： 打开动态链接库

* ngx_dlopen_n: 对ngx_dlopen()函数定义相应的打印字符串

* ngx_dlsym(handle,symbol): 从动态链接库中加载某个符号

* ngx_dlsym_n: 对ngx_dlsym()函数定义相应的打印字符串

* ngx_dlclose(handle): 关闭动态链接库

* ngx_dlclose_n: 对ngx_dlclose()函数定义相应的打印字符串

这里在ngx_auto_config.h头文件中具有如下定义：
<pre>
#ifndef NGX_HAVE_DLOPEN
#define NGX_HAVE_DLOPEN  1
#endif
</pre>
因此，定义有```char *ngx_dlerror(void);```函数。





## 2. os/unix/ngx_dlopen.c源文件

源文件内容如下：
{% highlight string %}

/*
 * Copyright (C) Maxim Dounin
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#if (NGX_HAVE_DLOPEN)

char *
ngx_dlerror(void)
{
    char  *err;

    err = (char *) dlerror();

    if (err == NULL) {
        return "";
    }

    return err;
}

#endif
{% endhighlight %}


<br />
<br />

**[参看]:**

1. [采用dlopen、dlsym、dlclose加载动态链接库【总结】](https://www.cnblogs.com/Anker/p/3746802.html)


<br />
<br />
<br />

