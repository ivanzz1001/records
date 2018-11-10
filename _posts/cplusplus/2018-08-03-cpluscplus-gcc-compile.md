---
layout: post
title: gcc程序的编译过程和链接原理
tags:
- cplusplus
categories: cplusplus
description: gcc程序的编译过程和链接原理
---



本文主要讲述一下gcc程序的编译过程和链接原理。然后再会介绍一下gcc各组件模块及相关的使用方法。

<!-- more -->


## 1. 程序编译流程

![gcc-compile-flow](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_compile_flow.jpg)

下面我们来看一下gcc编译的一些常用选项：

* **-o <file>**: 指定将输出写入到file文件

* **-E**: 只进行预处理，不会进行编译、汇编和链接

* **-S**: 只进行编译，不进行汇编和链接

* **-c**: 只进行编译和汇编，不进行链接

一个C/C++文件要经过预处理(preprocessing)、编译(compilation)、汇编（assembly)、和链接(link)才能变成可执行文件。

## 2. 程序编译示例

1) **示例程序**
{% highlight string %}
#include <stdio.h> 

#define   MAX  20 
#define   MIN  10 

#define  _DEBUG 
#define   SetBit(x)  (1<<x) 

int main(int argc, char* argv[]) 
{
        printf("Hello World \n"); 
        printf("MAX = %d,MIN = %d,MAX + MIN = %d\n",MAX,MIN,MAX + MIN); 

        #ifdef _DEBUG 
                printf("SetBit(5) = %d,SetBit(6) = %d\n",SetBit(5),SetBit(6));
                printf("SetBit( SetBit(2) ) = %d\n",SetBit( SetBit(2) ));
        #endif   

        return 0; 

{% endhighlight %}


2) **预处理**

我们使用如下命令对上述```hello.c```文件进行预处理：
<pre>
# gcc -E -o hello.i hello.c
</pre>
查看预处理后的结果：
{% highlight string %}
# cat hello.i
# 1 "hello.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "hello.c"
# 1 "/usr/include/stdio.h" 1 3 4
# 27 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/features.h" 1 3 4
# 367 "/usr/include/features.h" 3 4
# 1 "/usr/include/i386-linux-gnu/sys/cdefs.h" 1 3 4
# 410 "/usr/include/i386-linux-gnu/sys/cdefs.h" 3 4
# 1 "/usr/include/i386-linux-gnu/bits/wordsize.h" 1 3 4
# 411 "/usr/include/i386-linux-gnu/sys/cdefs.h" 2 3 4
# 368 "/usr/include/features.h" 2 3 4
# 391 "/usr/include/features.h" 3 4
# 1 "/usr/include/i386-linux-gnu/gnu/stubs.h" 1 3 4






# 1 "/usr/include/i386-linux-gnu/gnu/stubs-32.h" 1 3 4
# 8 "/usr/include/i386-linux-gnu/gnu/stubs.h" 2 3 4
# 392 "/usr/include/features.h" 2 3 4
# 28 "/usr/include/stdio.h" 2 3 4





# 1 "/usr/lib/gcc/i686-linux-gnu/5/include/stddef.h" 1 3 4
# 216 "/usr/lib/gcc/i686-linux-gnu/5/include/stddef.h" 3 4

# 216 "/usr/lib/gcc/i686-linux-gnu/5/include/stddef.h" 3 4
typedef unsigned int size_t;
# 34 "/usr/include/stdio.h" 2 3 4

# 1 "/usr/include/i386-linux-gnu/bits/types.h" 1 3 4
# 27 "/usr/include/i386-linux-gnu/bits/types.h" 3 4
# 1 "/usr/include/i386-linux-gnu/bits/wordsize.h" 1 3 4
# 28 "/usr/include/i386-linux-gnu/bits/types.h" 2 3 4


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;

...

extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) ;


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__));
# 942 "/usr/include/stdio.h" 3 4

# 2 "hello.c" 2


# 9 "hello.c"
int main(int argc, char* argv[])
{
 printf("Hello World \n");
 printf("MAX = %d,MIN = %d,MAX + MIN = %d\n",20,10,20 + 10);


  printf("SetBit(5) = %d,SetBit(6) = %d\n",(1<<5),(1<<6));
  printf("SetBit( SetBit(2) ) = %d\n",(1<<(1<<2)));


 return 0;
}

{% endhighlight %}

上面我们看到，预处理就是将要包含的文件插入原文件中、将宏定义展开、根据条件编译命令选择要使用的代码，最后将这些代码输出到一个```.i```文件中等待进一步处理。

3) **编译**

使用如下命令对预处理后的文件进行编译：
<pre>
# gcc -S -o hello.s hello.i
</pre>

编译后的结果：
{% highlight string %}
        .file   "hello.c"
        .section        .rodata
.LC0:
        .string "Hello World "
        .align 4
.LC1:
        .string "MAX = %d,MIN = %d,MAX + MIN = %d\n"
        .align 4
.LC2:
        .string "SetBit(5) = %d,SetBit(6) = %d\n"
.LC3:
        .string "SetBit( SetBit(2) ) = %d\n"
        .text
        .globl  main
        .type   main, @function
main:
.LFB0:
        .cfi_startproc
        leal    4(%esp), %ecx
        .cfi_def_cfa 1, 0
        andl    $-16, %esp
        pushl   -4(%ecx)
        pushl   %ebp
        .cfi_escape 0x10,0x5,0x2,0x75,0
        movl    %esp, %ebp
        pushl   %ecx
        .cfi_escape 0xf,0x3,0x75,0x7c,0x6
        subl    $4, %esp
        subl    $12, %esp
        pushl   $.LC0
        call    puts
        addl    $16, %esp
        pushl   $30
        pushl   $10
        pushl   $20
        pushl   $.LC1
        call    printf
        addl    $16, %esp
        subl    $4, %esp
        pushl   $64
        pushl   $32
        pushl   $.LC2
        call    printf
        addl    $16, %esp
        subl    $8, %esp
        pushl   $16
        pushl   $.LC3
        call    printf
        addl    $16, %esp
        movl    $0, %eax
        movl    -4(%ebp), %ecx
        .cfi_def_cfa 1, 0
        leave
        .cfi_restore 5
        leal    -4(%ecx), %esp
        .cfi_def_cfa 4, 4
        ret
        .cfi_endproc
.LFE0:
        .size   main, .-main
        .ident  "GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609"
        .section        .note.GNU-stack,"",@progbits
{% endhighlight %}

编译就是把C/C++代码翻译成汇编代码。

4) **汇编**

使用如下命令对```hello.s```进行汇编：
<pre>
# gcc -c -o hello.o hello.s
</pre>

汇编后的结果如下：
{% highlight string %}
# file hello.o
hello.o: ELF 32-bit LSB relocatable, Intel 80386, version 1 (SYSV), not stripped
{% endhighlight %}

汇编就是将第三步编译输出的汇编代码翻译成符合一定格式的机器代码，在Linux系统上一般表现为ELF目标文件(OBJ文件）。

5） **链接**

使用如下命令进行链接：
<pre>
# gcc -o hello hello.o

# ls
hello  hello.c  hello.i  hello.o  hello.s
# file hello
hello: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), dynamically linked,
interpreter /lib/ld-linux.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=ecbd660b7657c508631d316cc33dd147fdd0f43b, 
not stripped
</pre>

链接就是将汇编生成的OBJ文件、系统库的OBJ文件、库文件链接起来，最终生成可以在特定平台上运行的可执行程序。

## 3. 链接的原理




<br />
<br />

**[参看]**

1. [gcc程序的编译过程和链接原理](https://blog.csdn.net/czg13548930186/article/details/78331692)

2. [GCC常用参数详解](https://www.cnblogs.com/zhangsir6/articles/2956798.html)


<br />
<br />
<br />





