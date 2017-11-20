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

* 假若在程序启动的时候，环境变量LD_LIBRARY_PATH被定义，则会搜索该变量所指定的目录（出于安全上的考虑，假若可执行程序设置了set-user-ID和set-group-ID的话，则会忽略此环境变量）

* (ELF only) 假若调用程序的可执行文件包含DT_RUNPATH tag的话，然后该tag所列出的目录会被搜索。

* 查询缓存文件/etc/ld.so.cache以检查是否有名称为filename的entry（ld.so.cache缓存文件有ldconfig维护)

* 搜索/lib和/usr/lib目录

假若该加载的动态链接库有依赖于其他库的话，那些依赖库也会被动态链接器按上述相同的规则进行加载。

dlopen()函数的flag参数必须要包含如下两个值之一：

* RTLD_LAZY: 执行lazy binding。只有在引用到这些symbols的代码被执行时才会解析符号表。假若符号表从来不会被引用到，则动态链接库的符号表不会被解析。（对于lazy binding，其只会发生在函数引用上，而对于变量的引用，则在动态链接库一加载时就会被bind）

* RTLD_NOW: 假若这个值被指定，或者环境变量LD_BIND_NOW被设置为一个非空字符串，所有在库中为定义的符号都会在dlopen()函数返回前被解析。假若解析失败，则返回错误。

另外参数flag还可以与如下的一个或多个值进行按位或操作：

* RTLD_GLOBAL: 使当前库所加载的符号表可以被后续加载库所查找到

* RTLD_LOCAL: 与RTLD_GLOBAL刚好相反，并且如果未被指定的话，默认值为RTLD_LOCAL。当前库所加载的符号表并不能被后续所加载的库所找到。

* RTLD_NODELETE

* RTLD_NOLOAD

* RTLD_DEEPBIND


假若filename为NULL，则返回代表当前主应用程序的handle。当把这个handle传递给dlsym()函数时，将首先会查找本主应用程序中的符号表，然后再是程序启动时加载的动态链接库，然后还会从用RTLD_GLOBAL标志dlopen()的动态链接库中进行查找。

在动态链接库加载时，如果该库中有```_init()```方法，并且被导出，则首先会执行该方法。下面我们给出一个例子：

calc.c源文件：
{% highlight string %}
int add(int a,int b)
{
    return (a + b);
}

int sub(int a, int b)
{
    return (a - b);
}

int mul(int a, int b)
{
    return (a * b);
}

int div(int a, int b)
{
    return (a / b);
}
{% endhighlight %}

编译：
<pre>
[root@localhost test-src]# gcc -fPIC -shared -o libcalc.so calc.c 
</pre>

编写main函数(test.c):
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

typedef int (*OperFunc)(int,int);

int main(int argc,char *argv[])
{
   void *handle = NULL, *handle2 = NULL;
   int ret;

   handle = dlopen("./libcalc.so",RTLD_NOW|RTLD_GLOBAL);
   if(handle == NULL)
   {
      printf("err: %s\n",dlerror());
      return -1;
   }

   OperFunc addptr = (OperFunc)dlsym(handle,"add");
   OperFunc subptr = (OperFunc)dlsym(handle,"sub");
   OperFunc mulptr = (OperFunc)dlsym(handle,"mul");
   OperFunc divptr = (OperFunc)dlsym(handle,"div");

   if(addptr)
   {
      ret = (*addptr)(2,3);
      printf("ret: %d\n",ret);
   }

   if(subptr)
   {
      ret = (*subptr)(5,2);
      printf("ret: %d\n",ret);
   }
   
   if(mulptr)
   {
      ret = (*mulptr)(3,4);
      printf("ret: %d\n",ret);
   }

   if(divptr)
   {
      ret = (*divptr)(20,4);
      printf("ret: %d\n",ret);
   }

   handle2 = dlopen(NULL,RTLD_LAZY);     //这里演示NULL情况，从当前程序的Global符号表中加载
   if(handle2 == NULL)
   {
      printf("err: %s\n",dlerror());
      return -2;
   }

   OperFunc add2ptr = (OperFunc)dlsym(handle2,"add");
   if(add2ptr)
   {
       ret = (*add2ptr)(100,200);
       printf("ret: %d\n",ret);
   }

   dlclose(handle2);
   dlclose(handle);
   return 0;

}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.c -rdynamic -ldl
[root@localhost test-src]# ./test
ret: 5
ret: 3
ret: 12
ret: 5
ret: 300
</pre>

注： ```-rdynamic```选项主要是将所有符号加载到符号表中。对其更详细解释请参看如下：
<pre>
-rdynamic
Pass the flag ‘-export-dynamic’ to the ELF linker, on targets that support
it. This instructs the linker to add all symbols, not only used ones, to the
dynamic symbol table. This option is needed for some uses of dlopen or to
allow obtaining backtraces from within a program.
</pre>

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

ngx_dlerror()函数较为简单，只是对dlerror()函数进行了一个简单的封装。



<br />
<br />

**[参看]:**

1. [采用dlopen、dlsym、dlclose加载动态链接库【总结】](https://www.cnblogs.com/Anker/p/3746802.html)


<br />
<br />
<br />

