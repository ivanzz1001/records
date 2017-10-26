---
layout: post
title: os/unix/ngx_setproctitle源代码分析
tags:
- nginx
categories: nginx
description: nginx源代码分析
---


本文首先介绍一下Linux中setproctitle()的原理，然后给出nginx中相应的源代码并进行解析。


<!-- more -->


## 1. 介绍

每一个C程序都有个main函数作为程序启动的入口函数。main函数的原型是：
{% highlight string %}
int main(int argc, char *argv[])
{% endhighlight %}

其中，argc表示命令行参数的个数； argv是一个指针数组，保存所有命令行字符串。而Linux进程名称是通过命令行参数argv[0]来表示的。

而我们可以通过```char **environ```这一系统定义的全局变量来访问操作系统环境变量。
<pre>
命令行参数argv与环境变量environ是放在一块连续的内存中表示的，并且environ仅跟在argv后面。
</pre>

下面我们给出两个示例程序：

**1) 环境变量environ存放位置**

test3.c源代码：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int main(int argc,char *argv[])
{
   int i;

   for(i = 0;i<argc;i++)
   {
        printf("0x%x : %s\n",argv[i], argv[i]);
   }

   printf("\n");

   for(i = 0; environ[i] && i < 2; i++)
   {
        printf("0x%x : %s\n",environ[i], environ[i]);  
   }

   printf("\nargv: 0x%x  environ: 0x%x\n",argv, environ);
   
   for(i =0;i<argc+2;i++)
   {
      printf("&argv[%d]: 0x%x\n",i,&argv[i]);
   }

   return 0x0;

}
{% endhighlight %} 

编译执行(当前测试允许运行在64位操作系统)：
<pre>
[root@localhost test-src]# gcc -o test3 test3.c
[root@localhost test-src]# ./test3 date=2017-10-26
0x5a186745 : ./test3
0x5a18674d : date=2017-10-26

0x5a18675d : XDG_SESSION_ID=23
0x5a18676f : HOSTNAME=localhost.localdomain

argv: 0x5a184a78  environ: 0x5a184a90
&argv[0]: 0x5a184a78
&argv[1]: 0x5a184a80
&argv[2]: 0x5a184a88
&argv[3]: 0x5a184a90
</pre>


由上测试结果，我们发现argv与environ内存是按如下方式组织的：

![linux_argv_env](https://ivanzz1001.github.io/records/assets/img/linux/linux_argv_env.jpg)




<br />
<br />

[参看]:


1. [Linux修改进程名称(setproctitle())](http://blog.csdn.net/fivedoumi/article/details/51144086)

2. [Linux修改进程名称](http://www.cnblogs.com/lisuyun/articles/6549894.html)

3. [nginx多进程模型之热代码平滑升级](http://blog.csdn.net/brainkick/article/details/7192144)




<br />
<br />
<br />

