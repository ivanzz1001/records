---
layout: post
title: 静态库链接动态库时，如何使用该静态库
tags:
- cplusplus
categories: cplusplus
description: 静态库链接动态库时，如何使用该静态库
---

文章转载自：

- [静态库链接动态库时，如何使用该静态库](https://www.cnblogs.com/fnlingnzb-learner/p/8119729.html)





<!-- more -->



## 1. 静态库链接动态库

网上有各种静态库的创建与使用的例子，但都是超级简单的例子，比如静态库就直接来个printf()就完事了！其实实际使用时，静态库会复杂很多， 比如会调用很多其他的动态库。

下图就是个例子：

![cpp-link](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp-link-10000-01.jpg)

假设`libXXX.a`用了libpthread.so的函数`pthread_create()`，那`libXXX.a`在链接时，有没有把pthread_create()函数copy到自己身上，使其完全独立？main.c在链接时，只需要链接`libXXX.a`，还是连`libpthread.so`也要链接？这就是本文要讨论的内容。


为了证实问题，我们写个测试程序吧。一个是`libXXX.a`，一个时main.c。


### 1.1 静态库文件libXXX.a源代码

1） static_lib_example.h

```
//static_lib_example.h
#ifndef STATIC_LIB_EXAMPLE_H_INCLUDED
#define STATIC_LIB_EXAMPLE_H_INCLUDED

int testFunc(int x);

#endif
```

2) static_lib_example.c

```
//static_lib_example.c
#include "static_lib_example.h"
#include <stdio.h>
#include <pthread.h>

/* this function is run by the second thread */
void *thread_exe(void *x_void_ptr)
{
    /* increment x to 100 */
    int *x_ptr = (int *)x_void_ptr;
    while(++(*x_ptr) < 100);
        printf("x increment finished\n");

    return NULL;
}

int testFunc(int x)
{
    printf(" testFunc %i\n",x);
    pthread_t inc_x_thread;
    int y;
    /* create a second thread which executes thread_exe(&y) */
    if(pthread_create(&inc_x_thread, NULL, thread_exe, &y)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    return 0;
}
```


一个头文件，一个原文件，很简单。好吧，然后得来个Makefile

3) Makefile文件

```
#chenxf make file
#some notice
#target: prerequisites - the rule head
#$@ - means the target
#$^ - means all prerequisites
#$< - means just the first prerequisite

OUT_LIB = libstatic_lib_example.a

LIB_SRC := static_lib_example.c

TEMP_O := static_lib_example.o

$(TEMP_O): $(LIB_SRC)
        @gcc -c -o $@ $^

all:  $(OUT_LIB)

$(OUT_LIB): $(TEMP_O)
        @ar rcs $@ $^
clean:
        @rm -r *.a *.o
```

其实Makefile相当于2句命令。所以你不写Makefile，敲下面2句command也行:
```
gcc -c static_lib_example.c -o static_lib_example.o

ar rcs libstatic_lib_example.a static_lib_example.o
```

好了，编译结果如下：

```
# ls
libstatic_lib_example.a Makefile static_lib_example.c static_lib_example.h static_lib_example.o
```

### 1.2 调用静态库的程序main.c源代码

我们就写一个main.c，它会链接libstatic_lib_example.a，并调用函数`testFunc(int x)`:

```
//main.c
#include "static_lib_example.h"
int main(int argc, char* argv[])
{
    testFunc(100);
    return 1;
}
```

### 1.3 编译main.c的情况分析

编译main.c（只需要一个命令，就不写Makefile啦):

```
# gcc -g -O3 -Wall main.c -o main -I/home/chenxf/static_lib_sample/ -L/home/chenxf/static_lib_sample/ -lstatic_lib_example
```

- `-I`表示头文件路径

- `-L`表示库的路径，即libstatic_lib_example.a的路径

上面的命令，表示main程序，只链接静态库`libXXX.a`。

编译结果：

```
/home/chenxf/static_lib_sample/libstatic_lib_example.a(static_lib_example.o)：在函数‘testFunc’中： 
static_lib_example.c:(.text+0x86)：对‘pthread_create’未定义的引用 
static_lib_example.c:(.text+0xd4)：对‘pthread_join’未定义的引用 
collect2: error: ld returned 1 exit status 
```

出错啦！！！！！！ 

在main.c链接的时候，说找不到pthread_create了！

看来，静态库libXXX.a并没有把动态库的函数copy到自己身上，只留了符号表，所以main.c要用libXXX.a时，还必须链接动态库libpthread.so。也就是:
```
gcc -g -O3 -Wall main.c -o main -I/home/chenxf/static_lib_sample/ -L/home/chenxf/static_lib_sample/ -static -lstatic_lib_example -lpthread
```
>ps: libpthread.so在默认的系统库目录/usr/lib，不需要再写-L/usr/lib/


这样一写，就OK啦！编译就成功了！

我们还可以用 nm 工具，来确认`libXXX.a`到底有木有把pthread_create()函数copy到自己身上。

```
# nm libstatic_lib_example.a

static_lib_example.o:
                 U fwrite
                 U printf
                 U pthread_create
                 U pthread_join
                 U puts
                 U stderr
0000000000000040 T testFunc
0000000000000000 T thread_exe
```
U表示仅仅调用，而没有定义。也就是该库并不独立，而是依赖于其他库，比如libpthread.so





<br />
<br />
<br />


