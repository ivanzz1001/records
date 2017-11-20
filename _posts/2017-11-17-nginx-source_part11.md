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
因此，定义有```char *ngx_dlerror(void);```函数。下面我们再详细介绍一下加载动态链接库相关的接口函数：

### 1.1 Linux加载动态链接库接口函数

动态链接库加载的基本函数主要有：dladdr,dlclose,dlerror,dlopen,dlsym,dlvsym。其中标准Linux C包含如下几个函数：
{% highlight string %}
#include <dlfcn.h>

void *dlopen(const char *filename, int flag);

char *dlerror(void);

void *dlsym(void *handle, const char *symbol);

int dlclose(void *handle);

Link with -ldl.
{% endhighlight %}

**(1) 描述**

这四个函数dlopen(),dlerror(),dlsym(),dlclose()实现了动态链接库加载的接口。

```dlerror()```: 该函数返回自上一次调用dlerror()函数以来，最新发生在dlopen()、dlsym()、dlclose()函数调用所产生的错误的字符串描述。假若从初始化以来并未发生错误，则会返回NULL；若从上一次调用dlerror()以来，再并未发生新的错误，则第二次也会返回NULL。参看如下例子(test.c)：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>


int main(int argc,char *argv[])
{
   void *handle = NULL;
   char *str1,*str2;

   handle = dlopen("./aa.so",RTLD_NOW|RTLD_GLOBAL);
   if(handle == NULL)
   {
        str1 = dlerror();
        if(str1)
            printf("str1: %s\n",str1);
        else
            printf("str1 is NULL\n");

        str2 = dlerror();
        if(str2)
            printf("str2: %s\n",str2);
        else
            printf("str2 is NULL\n");

       return -1;
   }

   return 0;

}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c -ldl
[root@localhost test-src]# ./test
str1: ./aa.so: cannot open shared object file: No such file or directory
str2 is NULL
</pre>
如上可以看到，连续地两次dlerror()，第二次会将第一次的error信息清除掉。

<br />

```dlopen()```: 该函数用于加载filename指定的链接库，并且返回一个handle。如果filename为NULL，则返回代表当前主应用程序的handle。假若filename中包含"/"，则会将其解释为一个相对或绝对路径。否则，动态链接器会按如下顺序搜索库：

* (ELF only) 假若调用程序的可执行文件包含DT_RPATH tag并且不包含DT_RUNPATH的话，则会搜索DT_RPATH tag所列出的所有目录



<br />

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

