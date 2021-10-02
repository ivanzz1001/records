---
layout: post
title: gcc编译、静态库与动态库
tags:
- cplusplus
categories: cplusplus
description: gcc编译、静态库与动态库
---



本文我们讲述一下Linux中动、静态库的编译与链接。当前的操作系统环境以及GCC版本如下：
<pre>
# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core)

# gcc --version
gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-44)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

</pre>

<!-- more -->

## 1. GCC编译器

1） **GCC工作流程**

![cpp-gcc](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_gcc_figure1.jpg)

2) **GCC常用参数**
{% highlight string %}
-v              查看版本
-o              产生目标文件
-I <目录>       指定头文件目录
-O0/-O1/-O3    没有优化/缺省值/优化级别最高
-Wall          提示更多警告信息
-c             只编译程序
-E             生成预处理文件
-g             包含调试信息
{% endhighlight %}



## 2. 静态库

1） **静态库的命名格式**

<pre>
lib + 库的名字 + .a
</pre>
例如：libMyTest.a(MyTest为静态库的名字）

2）**静态库作用分析**

在项目开发过程中，经常出现优秀代码重用现象，又或者提供给第三方功能模块却又不想让其看到源代码，这些时候，通常的做法是将代码封装成库，生成的静态库要和头文件同时发布。

优点：

* 寻址方便，速度快

* 库在链接时被打包到可执行文件中，直接发布可执行程序即可以使用

缺点：

* 静态库的代码被加载到可执行程序中，因此体积过大

* 如果静态库的函数发生改变，必须重新编译可执行程序

### 2.1 静态库的制作与使用

测试代码的目录结构如下图所示，include中存放的是头文件，lib中存放的是静态（动态）库，src中存放的是源代码，main.c是发布代码
<pre>
# tree
.
├── include
│   └── head.h
├── lib
├── main.c
└── src
    ├── add.c
    ├── div.c
    ├── mul.c
    └── sub.c

3 directories, 6 files
</pre>

其中各文件内容如下：

* include/head.h文件
{% highlight string %}
#ifndef __HEAD_H__
#define __HEAD_H__

int add(int a, int b); 
int sub(int a, int b); 

int mul(int a, int b);
int div2(int a, int b);


#endif
{% endhighlight %}

* src/add.c文件
{% highlight string %}
#include "head.h"

int add(int a, int b)
{
	return a+b;
}
{% endhighlight %}

* src/sub.c文件
{% highlight string %}
#include "head.h"

int sub(int a, int b)
{
	return a-b;
}
{% endhighlight %}

* src/mul.c文件
{% highlight string %}
#include "head.h"

int mul(int a, int b)
{
	return a*b;
}
{% endhighlight %}

* src/div.c文件
{% highlight string %}
#include "head.h"

int div2(int a, int b)
{
	return a/b;
}
{% endhighlight %}

* main.c文件
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include "head.h"


int main(int argc, char *argv[])
{
	int a, b, res;

	a = 10;
	b = 2;

	res = add(a, b);
	printf("a + b = %d\n", res);

	res = sub(a, b);
	printf("a - b = %d\n", res);

	res = mul(a, b);
	printf("a * b = %d\n", res);

	res = div2(a, b);
	printf("a / b = %d\n", res);

	return 0x0;
}
{% endhighlight %}

1) **得到.o文件**

执行如下命令编译源文件：
<pre>
# cd src
# gcc -c *.c -I ../include
# tree
.
├── add.c
├── add.o
├── div.c
├── div.o
├── mul.c
├── mul.o
├── sub.c
└── sub.o

0 directories, 8 files
</pre>

2） **创建静态库**
<pre>
//将所有.o文件打包为静态库，r将文件插入静态库中，c创建静态库，不管库是否存在，s写入一个目标文件索引到库中，或者更新一个存在的目标文件索引
# ar rcs libMyMath.a *.o
# ls
add.c  add.o  div.c  div.o  libMyMath.a  mul.c  mul.o  sub.c  sub.o

//将静态库文件放置lib文件夹下
# cp libMyMath.a ../lib

//查看库中包含的函数等信息
# nm libMyMath.a 

add.o:
0000000000000000 T add

div.o:
0000000000000000 T div2

mul.o:
0000000000000000 T mul

sub.o:
0000000000000000 T sub   
# tree
.
├── include
│   └── head.h
├── lib
│   └── libMyMath.a
├── main.c
└── src
    ├── add.c
    ├── add.o
    ├── div.c
    ├── div.o
    ├── libMyMath.a
    ├── mul.c
    ├── mul.o
    ├── sub.c
    └── sub.o

3 directories, 12 files        
</pre>

3) **使用静态库**

* 方法1
<pre>
# gcc + 源文件 + -L 静态库路径 + -l静态库名 + -I头文件目录 + -o 可执行文件名
</pre>

例如，我们这里编译并链接main.c:
<pre>
# gcc main.c -L lib -l MyMath -I include -o app
# ./app
a + b = 12
a - b = 8
a * b = 20
a / b = 5
</pre>


* 方法2
<pre>
# gcc + 源文件 + -I头文件 + libxxx.a + -o 可执行文件名
</pre>

例如，我们这里编译并链接main.c:
<pre>
# gcc main.c -I include lib/libMyMath.a -o app
# ./app
a + b = 12
a - b = 8
a * b = 20
a / b = 5
</pre>

## 3. 动态库(共享库)

1) **动态库的命名格式**

<pre>
lib + 库的名字 + .so
</pre>
例如：libMyTest.so(MyTest为动态库的名字）

2）**动态库的作用与分析**

共享库的代码是在可执行程序运行时才载入内存的，在编译过程中仅简单的引用，因此代码体积较小。

优点：

* 节省内存

* 易于更新，不用重新编译可执行程序，运行时自动加载

缺点：

* 延时绑定，速度略慢

### 3.1 动态库的制作与使用

测试代码的目录结构与静态库相同，这里不再列出。

1）**生成与位置无关的.o文件**

<pre>
//参数-fPIC表示生成与位置无关代码
# gcc -fPIC *.c -I ../include -c   
# tree
.
├── add.c
├── add.o
├── div.c
├── div.o
├── libMyMath.a
├── mul.c
├── mul.o
├── sub.c
└── sub.o

0 directories, 9 files
</pre>

2) **创建动态库**
<pre>
# gcc -shared -o libMyMath.so *.o
# cp libMyMath.so ../lib
# cd ..
# tree
.
├── app
├── include
│   └── head.h
├── lib
│   └── libMyMath.so
├── main.c
└── src
    ├── add.c
    ├── add.o
    ├── div.c
    ├── div.o
    ├── libMyMath.a
    ├── libMyMath.so
    ├── mul.c
    ├── mul.o
    ├── sub.c
    └── sub.o

3 directories, 15 files
</pre>

3) **使用动态链接库**

* 方法1
<pre>
# gcc + 源文件 + -L 动态库路径 + -l动态库名 + -I头文件目录 + -o 可执行文件名
</pre>
例如，我们这里编译并链接main.c:
<pre>
# gcc main.c -L lib -l MyMath -I include -o app
# ./app
./app: error while loading shared libraries: libMyMath.so: cannot open shared object file: No such file or directory
</pre>

我们看到上面执行失败，找不到链接库，没有给动态链接器（ld-linux.so.2）指定好动态库libMyMatht.so 的路径。执行如下命令：
<pre>
# pwd
/root/workdir
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/root/workdir/lib
# ./app
a + b = 12
a - b = 8
a * b = 20
a / b = 5
</pre>

* 方法2
<pre>
# gcc + 源文件 + -I头文件 + libxxx.so + -o 可执行文件名
</pre>

例如，我们这里编译并链接main.c:
<pre>
# gcc main.c -I include lib/libMyMath.so -o app
# ./app
a + b = 12
a - b = 8
a * b = 20
a / b = 5
# echo $LD_LIBRARY_PATH
# ld 
</pre>
上面我们看到并不需要设置```LD_LIBRARY_PATH```就直接执行成功了。这是因为我们已经将链接路径包含进可执行程序了，参看如下：
{% highlight string %}
# ldd ./app
	linux-vdso.so.1 =>  (0x00007ffe635be000)
	lib/libMyMath.so (0x00007fc0f5b45000)
	libc.so.6 => /lib64/libc.so.6 (0x00007fc0f5770000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fc0f5d48000)
{% endhighlight %}

### 3.2. 如何解决第一种方法中找不到链接库的问题
使用命令ldd app可以查看当前的链接库情况:
{% highlight string %}
# ldd ./app
	linux-vdso.so.1 =>  (0x00007fff1f352000)
	libMyMath.so => not found
	libc.so.6 => /lib64/libc.so.6 (0x00007fdc772ce000)
	/lib64/ld-linux-x86-64.so.2 (0x00007fdc776a4000)
{% endhighlight %}

* 方法1
<pre>
# export LD_LIBRARY_PATH=自定义动态库的路径
//（只能起到临时作用，关闭终端后失效）
</pre>
LD_LIBRARY_PATH ： 指定查找共享库（动态链接库）时除了默认路径之外的其他路径，该路径在默认路径之前查找

* 方法2

将上述命令写入home目录下的.bashrc文件中，保存后重启终端生效（永久）

* 方法3
直接将动态库拷贝到user/lib的系统目录下（强烈不推荐!!）

* 方法4

将libMyMath.so所在绝对路径追加入到/etc/ld.so.conf文件，使用```sudo ldconfig -v```更新。


<br />
<br />

**[参看]**

1. [gcc编译、静态库与动态库](https://blog.csdn.net/daidaihema/article/details/80902012)

<br />
<br />
<br />





