---
layout: post
title: 数据段、代码段、堆栈段、BSS段的区别
tags:
- cplusplus
categories: cplusplus
description: 数据段、代码段、堆栈段、BSS段的区别
---


本文结合实际的例子讲述一下数据段、代码段、堆栈段以及BSS段。




<!-- more -->


## 1. Linux段管理
在Linux下内存分配是以页为单位的，而页是通过段管理，各个段之间是独立的，方便管理。一个简单的程序被编译成目标文件后的结构如下：

![cpp-segment-example](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_segment_example.jpg)

从上图可以看出，已初始化的全局变量和局部静态变量保存在```.data```段中，未初始化的全局变量和未初始化的局部静态变量保存在```.bss```段中。


目标文件各个段在文件中的布局如下：

![cpp-segment-layout](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_segment_layout.png)


下面我们介绍一下各个段：

* **init段**

程序初始化入口代码，在main()之前运行。

* **bss段**

BSS段属于静态内存分配。通常是指用来存放程序中未初始化的全局变量和未初始化的局部静态变量。未初始化的全局变量和未初始化的局部静态变量默认值是0，本来这些变量也可以放到```data```段的，但是因为它们都是0，所以它们在```data```段分配空间并且存放数据0是没有必要的。

在程序运行时，才会给BSS段里面的变量分配内存空间。

在目标文件(```*.o```)和可执行文件中，BSS段只是为未初始化的全局变量和未初始化的局部静态变量预留位置而已，它并没有内容，所以它不占据空间。

 
section table中保存了BSS段（未初始化的全局变量和未初始化的局部静态变量）内存空间大小总和。可以通过```objdump -h *.o```命令查看到。

* **data段**

数据段(data segment)通常是指用来存放程序中已初始化的全局变量和已初始化的静态变量的一块内存区域。数据段属于静态内存分配。

* **text段**

代码段(code segment / text segment)通常是指用来存放程序执行代码的一块内存区域。这部分区域的大小在程序运行前就已经确定，并且内存区域通常属于只读，某些架构也允许代码段为可写，即允许修改程序。在代码段中，也有可能包含一些只读的常数变量，例如字符串常量等。


* **rodata段**

存放的是只读数据，比如字符串常量，全局```const```变量和```#define```定义的常量。本段又称为```常量区```。例如：
<pre>
char *p = "123456";
</pre>
这里```123456```就存放在rodata段中。

但是注意，并不是所有的常量都放在```rodata```段的，其特殊情况如下：

1) 有些立即数与指令编译在一起直接放在代码段；

2） 对于字符串常量，编译器会去掉重复的常量，让程序的每个字符串常量只有一份

3） 有些系统中rodata段是多个进程共享的，目的是为了提高空间的利用率

* **strtab段**

存储的是变量名、函数名等。例如： 
<pre>
char *szPath = "/root";
void func();
</pre>
上述变量名```szPath```和函数名```func```存储在strtab段里。

* **shstrtab段**

bss、text、data等段名也存储在这里。



* **rel.text段**

针对text段的重定位表，还有```rel.data```(针对data段的重定位表）


* **heap堆**

堆是用于存放进程运行中被动态分配的内存段，它的大小并不固定，可动态扩张或缩减。当进程调用malloc()等函数分配内存时，新分配的内存就被动态添加到堆上（堆被扩张）；当利用free()等函数释放内存时，被释放的内存从堆中被剔除（堆被缩减）。


* **stack栈**

是用户存放程序临时创建的局部变量，也就是说我们函数括弧```{}```中定义的变量（但不包括static声明的变量，static意味着在数据段中存放变量）。除此以外，在函数被调用时，其参数也会被压入发起调用的进程栈中，并且待到调用结束后，函数的返回值也会被存放回栈中。由于栈的先进后出特点，所以栈特别方便用来保存/恢复调用现场。从这个意义上讲，我们可以把栈看成一个寄存、交换临时数据的内存区。

![cpp-memory-layout](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_memory_layout.jpg)




## 2. 示例

## 2.1 示例代码
{% highlight string %}
#include <stdio.h>


int a_test;

const int b_test = 2;

static int c_test = 3;

static int d_test;

int e_test = 0;


char *p_test = "hello,world";

const char *q_test = "good";




int main(int argc, char *argv[])
{
        static int m_test;
        static int n_test = 0;

        char *s_test = "just for test";

        printf("a: %d\n", a_test);
        printf("b: %d\n", b_test);
        printf("c: %d\n", c_test);
        printf("d: %d\n", d_test);
        printf("e: %d\n", e_test);

        printf("p: %s\n", p_test);
        printf("q: %s\n", q_test);

        printf("m: %d\n", m_test);
        printf("n: %d\n", n_test);
        printf("s: %s\n", s_test);

        return 0x0;

}
{% endhighlight %}

通过下面的命令逐步编译运行上面```test.c```程序：
<pre>
# gcc -S -o test.s test.c
# gcc -c -o test.o test.s
# gcc -o test test.o
# ls
test  test.c  test.o  test.s
</pre>


### 2.2 查看test可执行文件布局
<pre>
# objdump -h test

test:     file format elf32-i386

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .interp       00000013  08048154  08048154  00000154  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 .note.ABI-tag 00000020  08048168  08048168  00000168  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  2 .note.gnu.build-id 00000024  08048188  08048188  00000188  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .gnu.hash     00000020  080481ac  080481ac  000001ac  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .dynsym       00000050  080481cc  080481cc  000001cc  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  5 .dynstr       0000004c  0804821c  0804821c  0000021c  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  6 .gnu.version  0000000a  08048268  08048268  00000268  2**1
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  7 .gnu.version_r 00000020  08048274  08048274  00000274  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  8 .rel.dyn      00000008  08048294  08048294  00000294  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  9 .rel.plt      00000010  0804829c  0804829c  0000029c  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 10 .init         00000023  080482ac  080482ac  000002ac  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 11 .plt          00000030  080482d0  080482d0  000002d0  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 12 .plt.got      00000008  08048300  08048300  00000300  2**3
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 13 .text         00000262  08048310  08048310  00000310  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 14 .fini         00000014  08048574  08048574  00000574  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 15 .rodata       00000071  08048588  08048588  00000588  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 16 .eh_frame_hdr 0000002c  080485fc  080485fc  000005fc  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 17 .eh_frame     000000cc  08048628  08048628  00000628  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 18 .init_array   00000004  08049f08  08049f08  00000f08  2**2
                  CONTENTS, ALLOC, LOAD, DATA
 19 .fini_array   00000004  08049f0c  08049f0c  00000f0c  2**2
                  CONTENTS, ALLOC, LOAD, DATA
 20 .jcr          00000004  08049f10  08049f10  00000f10  2**2
                  CONTENTS, ALLOC, LOAD, DATA
 21 .dynamic      000000e8  08049f14  08049f14  00000f14  2**2
                  CONTENTS, ALLOC, LOAD, DATA
 22 .got          00000004  08049ffc  08049ffc  00000ffc  2**2
                  CONTENTS, ALLOC, LOAD, DATA
 23 .got.plt      00000014  0804a000  0804a000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, DATA
 24 .data         00000014  0804a014  0804a014  00001014  2**2
                  CONTENTS, ALLOC, LOAD, DATA
 25 .bss          00000018  0804a028  0804a028  00001028  2**2
                  ALLOC
 26 .comment      00000035  00000000  00000000  00001028  2**0
                  CONTENTS, READONLY
</pre>

1） **VMA和LMA**

我们先简要介绍一下```VMA```和```LMA```这两个字段：

* ```VMA```(virtual memory address): 程序区段在执行时期的地址

* ```LMA```(load memory address): 某程序区段加载时的地址。因为我们知道程序运行前要经过：编译、链接、装载、运行等过程。装载到哪里呢？ 没错，就是LMA对应的地址里。

一般情况下，```LMA```和```VMA```都是相等的，不等的情况主要发生在一些嵌入式系统上。

2） **segment布局**

如下我们简要的画出各区段的一个布局：

![cpp-lma-vma](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_lma_vma.jpg)

其实这里我们看到尽管bss段长度为```0x00000018```，但是我们看到```.bss```段和```.comment```段在文件中的偏移是一样的，这就说明bss段数据是不用包含在可执行程序中的。

### 2.3 汇编文件test.s分析
{% highlight string %}
# cat test.s
        .file   "test.c"
        .comm   a_test,4,4
        .globl  b_test
        .section        .rodata
        .align 4
        .type   b_test, @object
        .size   b_test, 4
b_test:
        .long   2
        .data
        .align 4
        .type   c_test, @object
        .size   c_test, 4
c_test:
        .long   3
        .local  d_test
        .comm   d_test,4,4
        .globl  e_test
        .bss
        .align 4
        .type   e_test, @object
        .size   e_test, 4
e_test:
        .zero   4
        .globl  p_test
        .section        .rodata
.LC0:
        .string "hello,world"
        .data
        .align 4
        .type   p_test, @object
        .size   p_test, 4
p_test:
        .long   .LC0
        .globl  q_test
        .section        .rodata
.LC1:
        .string "good"
        .data
        .align 4
        .type   q_test, @object
        .size   q_test, 4
q_test:
        .long   .LC1
        .section        .rodata
.LC2:
        .string "just for test"
.LC3:
        .string "a: %d\n"
.LC4:
        .string "b: %d\n"
.LC5:
        .string "c: %d\n"
.LC6:
        .string "d: %d\n"
.LC7:
        .string "e: %d\n"
.LC8:
        .string "p: %s\n"
.LC9:
        .string "q: %s\n"
.LC10:
        .string "m: %d\n"
.LC11:
        .string "n: %d\n"
.LC12:
        .string "s: %s\n"
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
        subl    $20, %esp
        movl    $.LC2, -12(%ebp)
        movl    a_test, %eax
        subl    $8, %esp
        pushl   %eax
        pushl   $.LC3
        call    printf
        addl    $16, %esp
        movl    $2, %eax
        subl    $8, %esp
        pushl   %eax
        pushl   $.LC4
        call    printf
        addl    $16, %esp
        movl    c_test, %eax
        subl    $8, %esp
        pushl   %eax
        pushl   $.LC5
        call    printf
        addl    $16, %esp
        movl    d_test, %eax
        subl    $8, %esp
        pushl   %eax
        pushl   $.LC6
        call    printf
        addl    $16, %esp
        movl    e_test, %eax
        subl    $8, %esp
        pushl   %eax
        pushl   $.LC7
        call    printf
        addl    $16, %esp
        movl    p_test, %eax
        subl    $8, %esp
        pushl   %eax
        pushl   $.LC8
        call    printf
        addl    $16, %esp
        movl    q_test, %eax
        subl    $8, %esp
        pushl   %eax
        pushl   $.LC9
        call    printf
        addl    $16, %esp
        movl    m_test.1942, %eax
        subl    $8, %esp
        pushl   %eax
        pushl   $.LC10
        call    printf
        addl    $16, %esp
        movl    n_test.1943, %eax
        subl    $8, %esp
        pushl   %eax
        pushl   $.LC11
        call    printf
        addl    $16, %esp
        subl    $8, %esp
        pushl   -12(%ebp)
        pushl   $.LC12
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
        .local  m_test.1942
        .comm   m_test.1942,4,4
        .local  n_test.1943
        .comm   n_test.1943,4,4
        .ident  "GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609"
        .section        .note.GNU-stack,"",@progbits
{% endhighlight %}

在讲解各变量存放位置之前，我们先简要介绍一下```.comm```和```.lcomm```:

* **.comm**: 声明为未初始化的通用内存区域；

* **.lcomm**: 声明为未初始化的本地内存区域；

其实上述两个均是属于bss段，不过作用域范围有些不同。```.lcomm```是为不会从本地汇编代码之外进行访问的数据保留的。两者使用的格式为：
<pre>
.comm symbol, length
.lcomm symbol, length
</pre>

例如：
{% highlight string %}
.section .bss
.lcomm buffer, 1000
{% endhighlight %}

该语句把1000字节的内存地址赋予标签buffer，在声明本地通用内存区域的程序之外的函数是不能访问他们的。

在bss段声明的好处是，数据不包含在可执行文件中。在数据段中定义数据时，它必须被包含在可执行程序中，因为必须使用特定值初始化它。因为不使用数据初始化bss段中声明的数据区域，所以内存区域被保留在运行时使用，并且不必包含在最终的程序中。


下面我们就简要分析一下各变量：

* **变量a_test**
<pre>
.comm   a_test,4,4
</pre>
我们可以看到是存放在bss段的通用内存区域的。

* **变量b_test**
<pre>
        .globl  b_test
        .section        .rodata
        .align 4
        .type   b_test, @object
        .size   b_test, 4
b_test:
        .long   2
</pre>
我们看到是存放在```.rodata```段的一个全局变量，值为2

* **变量c_test**
<pre>
        .data
        .align 4
        .type   c_test, @object
        .size   c_test, 4
c_test:
        .long   3
</pre>

可以看到是存放在data段的非全局变量，值为3.

* **变量d_test**
<pre>
        .local  d_test
        .comm   d_test,4,4
</pre>
我们可以看到是存放在bss段的一个局部变量，占用4个字节，且4字节对齐。

* **变量e_test**
<pre>
        .bss
        .align 4
        .type   e_test, @object
        .size   e_test, 4
e_test:
        .zero   4
</pre>
我们看到是存放在bss段的一个全局变量。

* **变量p_test**
<pre>
        .globl  p_test
        .section        .rodata
.LC0:
        .string "hello,world"
        .data
        .align 4
        .type   p_test, @object
        .size   p_test, 4
p_test:
        .long   .LC0
</pre>
我们看到```hello,world```本身是处于```.rodata```段的，而变量```p_test```是存放在全局data段的。因此可以在全局的范围内使用```p_test```并使其指向不同的位置，然而如果要改变其当前指向位置的值，那么程序将会崩溃，因为其当前执行位置为```.rodata```段。

* **变量q_test**
<pre>
        .globl  q_test
        .section        .rodata
.LC1:
        .string "good"
        .data
        .align 4
        .type   q_test, @object
        .size   q_test, 4
q_test:
        .long   .LC1
</pre>
从生成的汇编代码来看，其与```p_test```是一样的。

* **其他.rodata**段数据
<pre>
        .section        .rodata
.LC2:
        .string "just for test"
.LC3:
        .string "a: %d\n"
.LC4:
        .string "b: %d\n"
.LC5:
        .string "c: %d\n"
.LC6:
        .string "d: %d\n"
.LC7:
        .string "e: %d\n"
.LC8:
        .string "p: %s\n"
.LC9:
        .string "q: %s\n"
.LC10:
        .string "m: %d\n"
.LC11:
        .string "n: %d\n"
.LC12:
        .string "s: %s\n"
</pre>
我们看到一些常量字符串都放在```.rodata```段中。

* **变量m_test**
<pre>
        .local  m_test.1942
        .comm   m_test.1942,4,4
</pre>
我们可以看到是在main()作用域范围内的一个本地bss数据。

* **变量n_test**
<pre>
        .local  n_test.1943
        .comm   n_test.1943,4,4
</pre>
与```m_test```一致。

* **变量s_test**： 这里我们并未在汇编中找到该变量，其直接在被使用的地方优化成了一个立即数。


说明：如果我们在main()函数中定义如下
<pre>
char r_test[] = "are you ok";
</pre>
该变量及值都是在被处理后放入到栈中的。



<br />
<br />

**[参看]**

1. [数据段、代码段、堆栈段、BSS段的区别](https://blog.csdn.net/xtydtc/article/details/52900911)

2. [用PowerPC汇编写C语言程序](http://blog.chinaunix.net/uid-20528014-id-376596.html)

3. [Linux段管理，BSS段，data段，.rodata段，text段](https://blog.csdn.net/wdxin1322/article/details/40512071)

4. [bss、data和rodata区别与联系](https://blog.csdn.net/laiqun_ai/article/details/8528366)

5. [linux 目标文件(*.o) bss,data,text,rodata,堆,栈](https://blog.csdn.net/sunny04/article/details/40627311)

6. [u-boot链接分析](http://emb.hqyj.com/Column/Column345.htm)

7. [linux汇编语法](https://blog.csdn.net/darennet/article/details/41126621)

8. [带点的(一般都是ARM GNU伪汇编指令)](https://blog.csdn.net/qqliyunpeng/article/details/45116615)

9. [U-Boot学习笔记(四):TEXT_BASE的理解](https://blog.csdn.net/sdsh1880gm/article/details/53487535)

<br />
<br />
<br />





