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




## 2. 示例

1） **示例代码**
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

2) **查看test可执行文件布局**
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




<br />
<br />

**[参看]**

1. [数据段、代码段、堆栈段、BSS段的区别](https://blog.csdn.net/xtydtc/article/details/52900911)

2. [用PowerPC汇编写C语言程序](http://blog.chinaunix.net/uid-20528014-id-376596.html)

3. [Linux段管理，BSS段，data段，.rodata段，text段](https://blog.csdn.net/wdxin1322/article/details/40512071)

4. [bss、data和rodata区别与联系](https://blog.csdn.net/laiqun_ai/article/details/8528366)

5. [linux 目标文件(*.o) bss,data,text,rodata,堆,栈](https://blog.csdn.net/sunny04/article/details/40627311)

6. [Linux内存管理（text、rodata、data、bss、stack&heap）](https://www.cnblogs.com/annie-fun/p/6633531.html?utm_source=itdadao&utm_medium=referral)

7. [linux汇编语法](https://blog.csdn.net/darennet/article/details/41126621)

8. [带点的(一般都是ARM GNU伪汇编指令)](https://blog.csdn.net/qqliyunpeng/article/details/45116615)

<br />
<br />
<br />





