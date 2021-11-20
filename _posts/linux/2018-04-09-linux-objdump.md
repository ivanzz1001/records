---
layout: post
title: Linux中objdump的使用
tags:
- LinuxOps
categories: linux
description: linux调试
---

本章我们介绍一下Linux环境下对obj对象文件的分析。其中主要涉及到```objdump```、```ar```以及```nm```三个工具

<!-- more -->

## 1 objdump命令

在Linux环境下，我们可以使用objdump命令对目标文件(obj)或可执行文件进行反汇编，它以一种可阅读的格式让你更多的了解二进制文件可能带有的附加信息。

### 1.1 用法
objdump命令的基本使用方法如下：
{% highlight string %}
objdump [-a|--archive-headers]
   [-b bfdname|--target=bfdname]
   [-C|--demangle[=style] ]
   [-d|--disassemble]
   [-D|--disassemble-all]
   [-z|--disassemble-zeroes]
   [-EB|-EL|--endian={big | little }]
   [-f|--file-headers]
   [-F|--file-offsets]
   [--file-start-context]
   [-g|--debugging]
   [-e|--debugging-tags]
   [-h|--section-headers|--headers]
   [-i|--info]
   [-j section|--section=section]
   [-l|--line-numbers]
   [-S|--source]
   [-m machine|--architecture=machine]
   [-M options|--disassembler-options=options]
   [-p|--private-headers]
   [-P options|--private=options]
   [-r|--reloc]
   [-R|--dynamic-reloc]
   [-s|--full-contents]
   [-W[lLiaprmfFsoRt]|
    --dwarf[=rawline,=decodedline,=info,=abbrev,=pubnames]
            [=aranges,=macro,=frames,=frames-interp,=str,=loc]
            [=Ranges,=pubtypes,=trace_info,=trace_abbrev]
            [=trace_aranges,=gdb_index]
   [-G|--stabs]
   [-t|--syms]
   [-T|--dynamic-syms]
   [-x|--all-headers]
   [-w|--wide]
   [--start-address=address]
   [--stop-address=address]
   [--prefix-addresses]
   [--[no-]show-raw-insn]
   [--adjust-vma=offset]
   [--special-syms]
   [--prefix=prefix]
   [--prefix-strip=level]
   [--insn-width=width]
   [-V|--version]
   [-H|--help]
   objfile...
{% endhighlight %}

下面我们简要介绍一下各选项的含义：
<pre>
-a
--archive-header
    假如任何一个objfile是库文件，则显示相应的header信息（输出格式类似于ls -l命令）。除了可以列出
    ar tv所能展示的信息，objdump -a还可以显示lib文件中每一个对象文件的格式。

--adjust-vma=offset
    当使用objdump命令来dump信息的时候，将section addresses都加上offset。此选项主要用于section
    addresses与符号表不对应的情形下，比如我们需要将相应的section放到某一个特殊的地址处时。

-b bfdname
--target=bfdname
    为obj文件指定对象码(object-code)格式。本选项是非必需的，objdump命令可以自动的识别许多种格式。
    例如：
       objdump -b oasys -m vax -h fu.o
    上面的命令用于fu.o的头部摘要信息，并明确指出了fu.o这个对象文件是vax平台上由oasys编译器编译而来。
    我们可以使用-i选项来列出所支持的所有平台格式

-C
--demangle[=style]
    将底层(low-level)的符号名解码成用户级(user-level)的名称。除了会去掉由系统添加的头部下划线之外，
    还使得C++的函数名以便于理解的方式显示出来。

-g
--debugging
    用于显示调试信息。这会使得objdump命令尝试解析STABS和IEEE格式的调试信息，然后以C语言语法格式将相应
    的调试信息进行输出。仅仅支持某些类型的调试信息。有些其他的格式被readelf -w支持

-e
--debugging-tags
    类似于-g选项。但是产生的输出信息格式兼容ctags工具

-d
--disassemble
    从objfile中对机器指令进行反汇编。本选项只对那些包含指令的section进行反汇编。


-D
--disassemble-all
    类似于-d，但是本选项会对所有的sections进行反汇编，而不仅仅是那些包含指令的sections。

    本选项会微妙的影响代码段的反汇编。当使用-d选项的时候，objdump会假设代码中出现的所有symbols都在对应
    的boundary范围之内，并且不会跨boundary来进行反汇编； 而当使用-D选项时，则并不会有这样的假设。这就
    意味着-d与-D选项在反汇编时，可能输出结果会有些不同，比如当数据存放在代码段的情况下。


--prefix-addresses
    反汇编的时候，显示每一行的完整地址。这是一种比较老的反汇编格式

-EB
-EL
--endian={big|little}
    指定目标文件的大小端。这仅仅会影响到反汇编。这在对某一些并未指定大小端信息的obj文件进行反汇编时很有
    用，比如S-records

-f
--file-headers
    显示每一个obj文件的整体头部摘要信息

-F
--file-offsets
    当在对sections进行反汇编时，无论是否显示相应的symbol，都会显示其在文件内的偏移(offset)。

-h
--section-headers
--headers
    显示obj文件各个sections的头部摘要信息。

    obj文件中segments可能会被relocate，比如在ld时通过使用-Ttext、-Tdata或者-Tbss选项。然而，有一些
    对象文件格式，比如a.out，其本身并没有保存起始地址信息。在这种情况下，尽管ld可以正确的对这些sections
    进行relocate，但是使用objdump -h来查看各sections的头部摘要信息时则不能正确的显示地址信息。

-H
--help
    objdump的帮助信息

-i
--info
    显示objdump所支持的所有arch以及obj格式。-m和-b选项可用到这


-j name
--section=name
    仅仅显示指定名称为name的section的信息


-l
--line-numbers
    用文件名和行号标注相应的目标代码，仅仅和-d、-D或者-r一起使用时有效。通常要求具有调试信息，即编译时使用
    了-g之类的选项。


-m machine
--architecture=machine
    指定反汇编目标文件时使用的架构。当待反汇编的目标文件其本身并没有包含arch信息时(如S-records文件)，我们
    就可以使用此选项来进行指定。我们可以使用objdump -i来列出所支持的arch。

-p
--private-headers
    显示objfile文件格式的专属信息。具体的输出取决于object file的格式，对于某一些格式，可能并没有一些额外
    的信息输出

-r
--reloc
    显示文件的重定位入口。如果和-d或者-D一起使用，重定位部分以反汇编后的格式显示出来

-R
--dynamic-reloc
    显示文件的动态重定位入口，仅仅对于动态目标文件意义，比如某些共享库。

-s
--full-contents
    显示指定section的所有内容。默认情况下，对于所有非空section都会显示

-S
--source
    将反汇编代码与源代码交叉显示。通常在调试版本能够较好的显示尤其当编译的时候指定了-g这种调试参数时，
    效果比较明显。隐含了-d参数。

--show-raw-insn
    在进行反汇编时，显示每条汇编指令对应的机器码。默认情况下会显示，除非使用了--prefix-addresses

--no-show-raw-insn
    反汇编时，不显示汇编指令的机器码。当使用了--prefix-addresses时，默认就不会显示机器码

--start-address=address
    从指定地址开始显示数据，该选项影响-d、-r和-s选项的输出

--stop-address=address
    显示数据直到指定地址为止，该项影响-d、-r和-s选项的输出

-t
--syms 
    显示文件的符号表入口。类似于nm -s提供的信息 

-T
--dynamic-syms
    显示文件的动态符号表入口，仅仅对动态目标文件意义，比如某些共享库。它显示的信息类似于 nm -D(--dynamic)
    显示的信息

-V
--version
    打印objdump的版本信息

-x
--all-headers
    显示所可用的header信息，包括符号表、重定位入口。-x 等价于-a -f -h -r -t 同时指定

-z
--disassemble-zeroes
    一般反汇编输出将省略大块的零，该选项使得这些零块也被反汇编

@file
    可以将选项集中到一个文件中，然后使用这个@file选项载入
</pre>

### 1.2 常用符号表字段
* **```.text```** 已编译程序的机器代码

* **```.rodata```** 只读数据，比如printf语句中的格式串和开关(switch)语句的跳转表

* **```.data```** 已初始化的全局C变量。局部C变量在运行时被保存在栈中，既不出现在.data中，也不出现在.bss中

* **```.bss```** 未初始化的全局C变量。在目标文件中，这个节(section)不占据实际的空间，它仅仅只是一个占位符。目标文件格式区分```初始化```和```未初始化```变量是为了空间效率。在目标文件中，未初始化变量不需要占用任何实际的磁盘空间。

* **```.symtab```** 一个符号表(symbol table)，它存放在程序中被定义和引用的函数和全局变量的信息。一些程序员错误的认为必须通过```-g```选项来编译一个程序，得到符号表信息。实际上，每个可重定位目标文件在```.symtab```中都有一张符号表。然而，和编译器中的符号表不同，```.symtab```符号表不包含局部变量的表目。

* **```.rel.text```** 当链接器把这个目标文件和其他文件结合时，```.text```节中的许多位置都需要修改。一般而言，任何调用外部函数或者引用全局变量的指令都需要修改。另一方面，调用本地函数的指令则不需要修改。注意，可执行目标文件中并不需要重定位信息，因此通常省略，除非使用者显式地指示链接器包含这些信息。

* **```.rel.data```** 被模块定义或引用的任何全局变量的信息。一般而言，任何已初始化全局变量的初始值是全局变量或者外部定义函数的地址都需要被修改。

* **```.debug```** 一个调试符号表，其有些表目是程序中定义的局部变量和类型定义，有些表目是程序中定义和引用的全局变量，有些是原始的C源文件。只有以```-g```选项调用编译程序时，才会得到这张表。

* **```.line```** 原始C源程序中的行号和```.text```节中机器指令之间的映射。只有以```-g```选项调用编译程序时，才会得到这张表。

* **```.strtab```** 一个字符串表，其内容包括```.symtab```和```.debug```节中的符号表，以及节头部中的节名字。字符串表就是以NULL结尾的字符串序列。


## 1.3 使用举例

###### 1.3.1 测试代码示例

* 头文件add.h
{% highlight string %}
#ifndef __ADD_H_
#define __ADD_H_



#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */



extern int module_id;
extern char *module_name;


int add(int a, int b);





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif
{% endhighlight %}

* 源文件add.c
{% highlight string %}
#include "add.h"


int module_id = 1001;
char *module_name = "calc";

int add(int a, int b)
{
        return a+b;
}
{% endhighlight %}

* 测试文件main.c
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include "add.h"


int out_sum;


int main(int argc, char *argv[])
{
        int id = module_id;
        char *name = module_name;
        int sum;


        printf("module：%d %s\n", id, name);

        sum = add(2,3);
        printf("sum: %d\n", sum);


        out_sum = sum;

        return 0x0;
}
{% endhighlight %}

###### 1.3.2 编译代码
执行如下命令编译目标代码：
<pre>
# gcc -g -c add.c -o add.o
# gcc -g -c main.c -o main.o
# gcc -o main main.o add.o
# ls
add.c  add.h  add.o  main  main.c  main.o
</pre>


###### 1.3.3 测试

1) **查看对象文件的所有sections的头部摘要信息**

* add.o目标文件
{% highlight string %}
# objdump -h ./add.o

./add.o:     file format elf32-i386

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         0000000d  00000000  00000000  00000034  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .data         00000008  00000000  00000000  00000044  2**2
                  CONTENTS, ALLOC, LOAD, RELOC, DATA
  2 .bss          00000000  00000000  00000000  0000004c  2**0
                  ALLOC
  3 .rodata       00000005  00000000  00000000  0000004c  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .debug_info   0000008e  00000000  00000000  00000051  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
  5 .debug_abbrev 0000006f  00000000  00000000  000000df  2**0
                  CONTENTS, READONLY, DEBUGGING
  6 .debug_aranges 00000020  00000000  00000000  0000014e  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
  7 .debug_line   00000035  00000000  00000000  0000016e  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
  8 .debug_str    0000007f  00000000  00000000  000001a3  2**0
                  CONTENTS, READONLY, DEBUGGING
  9 .comment      00000036  00000000  00000000  00000222  2**0
                  CONTENTS, READONLY
 10 .note.GNU-stack 00000000  00000000  00000000  00000258  2**0
                  CONTENTS, READONLY
 11 .eh_frame     00000038  00000000  00000000  00000258  2**2
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, DATA
{% endhighlight %}

* main.o目标文件
{% highlight string %}
# objdump -h main.o

main.o:     file format elf32-i386

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         00000071  00000000  00000000  00000034  2**0
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  1 .data         00000000  00000000  00000000  000000a5  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  2 .bss          00000000  00000000  00000000  000000a5  2**0
                  ALLOC
  3 .rodata       00000019  00000000  00000000  000000a5  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .debug_info   0000010c  00000000  00000000  000000be  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
  5 .debug_abbrev 0000009e  00000000  00000000  000001ca  2**0
                  CONTENTS, READONLY, DEBUGGING
  6 .debug_aranges 00000020  00000000  00000000  00000268  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
  7 .debug_line   0000004b  00000000  00000000  00000288  2**0
                  CONTENTS, RELOC, READONLY, DEBUGGING
  8 .debug_str    00000129  00000000  00000000  000002d3  2**0
                  CONTENTS, READONLY, DEBUGGING
  9 .comment      00000036  00000000  00000000  000003fc  2**0
                  CONTENTS, READONLY
 10 .note.GNU-stack 00000000  00000000  00000000  00000432  2**0
                  CONTENTS, READONLY
 11 .eh_frame     00000044  00000000  00000000  00000434  2**2
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, DATA
{% endhighlight %}

* main可执行程序
{% highlight string %}
# objdump -h main

main:     file format elf32-i386

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
 13 .text         000001e2  08048310  08048310  00000310  2**4
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 14 .fini         00000014  080484f4  080484f4  000004f4  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
 15 .rodata       00000026  08048508  08048508  00000508  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 16 .eh_frame_hdr 00000034  08048530  08048530  00000530  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
 17 .eh_frame     000000ec  08048564  08048564  00000564  2**2
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
 24 .data         00000010  0804a014  0804a014  00001014  2**2
                  CONTENTS, ALLOC, LOAD, DATA
 25 .bss          00000008  0804a024  0804a024  00001024  2**2
                  ALLOC
 26 .comment      00000035  00000000  00000000  00001024  2**0
                  CONTENTS, READONLY
 27 .debug_aranges 00000040  00000000  00000000  00001059  2**0
                  CONTENTS, READONLY, DEBUGGING
 28 .debug_info   0000019a  00000000  00000000  00001099  2**0
                  CONTENTS, READONLY, DEBUGGING
 29 .debug_abbrev 0000010d  00000000  00000000  00001233  2**0
                  CONTENTS, READONLY, DEBUGGING
 30 .debug_line   00000080  00000000  00000000  00001340  2**0
                  CONTENTS, READONLY, DEBUGGING
 31 .debug_str    000000f1  00000000  00000000  000013c0  2**0
                  CONTENTS, READONLY, DEBUGGING
{% endhighlight %}

2) **显示目标文件的整体头部摘要信息**

* add.o目标文件
{% highlight string %}
# objdump -f ./add.o

./add.o:     file format elf32-i386
architecture: i386, flags 0x00000011:
HAS_RELOC, HAS_SYMS
start address 0x00000000
{% endhighlight %}

* main.o目标文件
{% highlight string %}
# objdump -f ./main.o

./main.o:     file format elf32-i386
architecture: i386, flags 0x00000011:
HAS_RELOC, HAS_SYMS
start address 0x00000000
{% endhighlight %}

* main目标文件
{% highlight string %}
# objdump -f ./main

./main:     file format elf32-i386
architecture: i386, flags 0x00000112:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x08048310
{% endhighlight %}

3) **查看目标文件符号表**

* add.o符号表
{% highlight string %}
# objdump -t ./add.o

./add.o:     file format elf32-i386

SYMBOL TABLE:
00000000 l    df *ABS*  00000000 add.c
00000000 l    d  .text  00000000 .text
00000000 l    d  .data  00000000 .data
00000000 l    d  .bss   00000000 .bss
00000000 l    d  .rodata        00000000 .rodata
00000000 l    d  .debug_info    00000000 .debug_info
00000000 l    d  .debug_abbrev  00000000 .debug_abbrev
00000000 l    d  .debug_aranges 00000000 .debug_aranges
00000000 l    d  .debug_line    00000000 .debug_line
00000000 l    d  .debug_str     00000000 .debug_str
00000000 l    d  .note.GNU-stack        00000000 .note.GNU-stack
00000000 l    d  .eh_frame      00000000 .eh_frame
00000000 l    d  .comment       00000000 .comment
00000000 g     O .data  00000004 module_id
00000004 g     O .data  00000004 module_name
00000000 g     F .text  0000000d add
{% endhighlight %}


* main.o符号表
{% highlight string %}
# objdump -t ./main.o

./main.o:     file format elf32-i386

SYMBOL TABLE:
00000000 l    df *ABS*  00000000 main.c
00000000 l    d  .text  00000000 .text
00000000 l    d  .data  00000000 .data
00000000 l    d  .bss   00000000 .bss
00000000 l    d  .rodata        00000000 .rodata
00000000 l    d  .debug_info    00000000 .debug_info
00000000 l    d  .debug_abbrev  00000000 .debug_abbrev
00000000 l    d  .debug_aranges 00000000 .debug_aranges
00000000 l    d  .debug_line    00000000 .debug_line
00000000 l    d  .debug_str     00000000 .debug_str
00000000 l    d  .note.GNU-stack        00000000 .note.GNU-stack
00000000 l    d  .eh_frame      00000000 .eh_frame
00000000 l    d  .comment       00000000 .comment
00000004       O *COM*  00000004 out_sum
00000000 g     F .text  00000071 main
00000000         *UND*  00000000 module_id
00000000         *UND*  00000000 module_name
00000000         *UND*  00000000 printf
00000000         *UND*  00000000 add
{% endhighlight %}

* main符号表
{% highlight string %}
# objdump -t ./main

./main:     file format elf32-i386

SYMBOL TABLE:
08048154 l    d  .interp        00000000              .interp
08048168 l    d  .note.ABI-tag  00000000              .note.ABI-tag
08048188 l    d  .note.gnu.build-id     00000000              .note.gnu.build-id
080481ac l    d  .gnu.hash      00000000              .gnu.hash
080481cc l    d  .dynsym        00000000              .dynsym
0804821c l    d  .dynstr        00000000              .dynstr
08048268 l    d  .gnu.version   00000000              .gnu.version
08048274 l    d  .gnu.version_r 00000000              .gnu.version_r
08048294 l    d  .rel.dyn       00000000              .rel.dyn
0804829c l    d  .rel.plt       00000000              .rel.plt
080482ac l    d  .init  00000000              .init
080482d0 l    d  .plt   00000000              .plt
08048300 l    d  .plt.got       00000000              .plt.got
08048310 l    d  .text  00000000              .text
080484f4 l    d  .fini  00000000              .fini
08048508 l    d  .rodata        00000000              .rodata
08048530 l    d  .eh_frame_hdr  00000000              .eh_frame_hdr
08048564 l    d  .eh_frame      00000000              .eh_frame
08049f08 l    d  .init_array    00000000              .init_array
08049f0c l    d  .fini_array    00000000              .fini_array
08049f10 l    d  .jcr   00000000              .jcr
08049f14 l    d  .dynamic       00000000              .dynamic
08049ffc l    d  .got   00000000              .got
0804a000 l    d  .got.plt       00000000              .got.plt
0804a014 l    d  .data  00000000              .data
0804a024 l    d  .bss   00000000              .bss
00000000 l    d  .comment       00000000              .comment
00000000 l    d  .debug_aranges 00000000              .debug_aranges
00000000 l    d  .debug_info    00000000              .debug_info
00000000 l    d  .debug_abbrev  00000000              .debug_abbrev
00000000 l    d  .debug_line    00000000              .debug_line
00000000 l    d  .debug_str     00000000              .debug_str
00000000 l    df *ABS*  00000000              crtstuff.c
08049f10 l     O .jcr   00000000              __JCR_LIST__
08048350 l     F .text  00000000              deregister_tm_clones
08048380 l     F .text  00000000              register_tm_clones
080483c0 l     F .text  00000000              __do_global_dtors_aux
0804a024 l     O .bss   00000001              completed.7209
08049f0c l     O .fini_array    00000000              __do_global_dtors_aux_fini_array_entry
080483e0 l     F .text  00000000              frame_dummy
08049f08 l     O .init_array    00000000              __frame_dummy_init_array_entry
00000000 l    df *ABS*  00000000              main.c
00000000 l    df *ABS*  00000000              add.c
00000000 l    df *ABS*  00000000              crtstuff.c
0804864c l     O .eh_frame      00000000              __FRAME_END__
08049f10 l     O .jcr   00000000              __JCR_END__
00000000 l    df *ABS*  00000000              
08049f0c l       .init_array    00000000              __init_array_end
08049f14 l     O .dynamic       00000000              _DYNAMIC
08049f08 l       .init_array    00000000              __init_array_start
08048530 l       .eh_frame_hdr  00000000              __GNU_EH_FRAME_HDR
0804a000 l     O .got.plt       00000000              _GLOBAL_OFFSET_TABLE_
080484f0 g     F .text  00000002              __libc_csu_fini
00000000  w      *UND*  00000000              _ITM_deregisterTMCloneTable
08048340 g     F .text  00000004              .hidden __x86.get_pc_thunk.bx
0804a014  w      .data  00000000              data_start
0804847c g     F .text  0000000d              add
00000000       F *UND*  00000000              printf@@GLIBC_2.0
0804a01c g     O .data  00000004              module_id
0804a024 g       .data  00000000              _edata
0804a020 g     O .data  00000004              module_name
080484f4 g     F .fini  00000000              _fini
0804a014 g       .data  00000000              __data_start
00000000  w      *UND*  00000000              __gmon_start__
0804a018 g     O .data  00000000              .hidden __dso_handle
0804850c g     O .rodata        00000004              _IO_stdin_used
00000000       F *UND*  00000000              __libc_start_main@@GLIBC_2.0
08048490 g     F .text  0000005d              __libc_csu_init
0804a02c g       .bss   00000000              _end
08048310 g     F .text  00000000              _start
08048508 g     O .rodata        00000004              _fp_hw
0804a024 g       .bss   00000000              __bss_start
0804a028 g     O .bss   00000004              out_sum
0804840b g     F .text  00000071              main
00000000  w      *UND*  00000000              _Jv_RegisterClasses
0804a024 g     O .data  00000000              .hidden __TMC_END__
00000000  w      *UND*  00000000              _ITM_registerTMCloneTable
080482ac g     F .init  00000000              _init
{% endhighlight %}

4) **对目标文件进行反汇编**

* add.o进行反汇编
{% highlight string %}
# objdump -d ./add.o

./add.o:     file format elf32-i386


Disassembly of section .text:

00000000 <add>:
   0:   55                      push   %ebp
   1:   89 e5                   mov    %esp,%ebp
   3:   8b 55 08                mov    0x8(%ebp),%edx
   6:   8b 45 0c                mov    0xc(%ebp),%eax
   9:   01 d0                   add    %edx,%eax
   b:   5d                      pop    %ebp
   c:   c3                      ret 
{% endhighlight %}

* main.o进行反汇编
{% highlight string %}
# objdump -d -S ./main.o

./main.o:     file format elf32-i386


Disassembly of section .text:

00000000 <main>:

int out_sum;


int main(int argc, char *argv[])
{
   0:   8d 4c 24 04             lea    0x4(%esp),%ecx
   4:   83 e4 f0                and    $0xfffffff0,%esp
   7:   ff 71 fc                pushl  -0x4(%ecx)
   a:   55                      push   %ebp
   b:   89 e5                   mov    %esp,%ebp
   d:   51                      push   %ecx
   e:   83 ec 14                sub    $0x14,%esp
        int id = module_id;
  11:   a1 00 00 00 00          mov    0x0,%eax
  16:   89 45 ec                mov    %eax,-0x14(%ebp)
        char *name = module_name;
  19:   a1 00 00 00 00          mov    0x0,%eax
  1e:   89 45 f0                mov    %eax,-0x10(%ebp)
        int sum;


        printf("module：%d %s\n", id, name);
  21:   83 ec 04                sub    $0x4,%esp
  24:   ff 75 f0                pushl  -0x10(%ebp)
  27:   ff 75 ec                pushl  -0x14(%ebp)
  2a:   68 00 00 00 00          push   $0x0
  2f:   e8 fc ff ff ff          call   30 <main+0x30>
  34:   83 c4 10                add    $0x10,%esp

        sum = add(2,3);
  37:   83 ec 08                sub    $0x8,%esp
  3a:   6a 03                   push   $0x3
  3c:   6a 02                   push   $0x2
  3e:   e8 fc ff ff ff          call   3f <main+0x3f>
  43:   83 c4 10                add    $0x10,%esp
  46:   89 45 f4                mov    %eax,-0xc(%ebp)
        printf("sum: %d\n", sum);
  49:   83 ec 08                sub    $0x8,%esp
  4c:   ff 75 f4                pushl  -0xc(%ebp)
  4f:   68 10 00 00 00          push   $0x10
  54:   e8 fc ff ff ff          call   55 <main+0x55>
  59:   83 c4 10                add    $0x10,%esp


        out_sum = sum;
  5c:   8b 45 f4                mov    -0xc(%ebp),%eax
  5f:   a3 00 00 00 00          mov    %eax,0x0

        return 0x0;
  64:   b8 00 00 00 00          mov    $0x0,%eax
}
  69:   8b 4d fc                mov    -0x4(%ebp),%ecx
  6c:   c9                      leave  
  6d:   8d 61 fc                lea    -0x4(%ecx),%esp
  70:   c3                      ret    
{% endhighlight %}

* main进行反汇编
{% highlight string %}
# objdump -d -S ./main

./main:     file format elf32-i386


Disassembly of section .init:

080482ac <_init>:
 80482ac:       53                      push   %ebx
 80482ad:       83 ec 08                sub    $0x8,%esp
 80482b0:       e8 8b 00 00 00          call   8048340 <__x86.get_pc_thunk.bx>
 80482b5:       81 c3 4b 1d 00 00       add    $0x1d4b,%ebx
 80482bb:       8b 83 fc ff ff ff       mov    -0x4(%ebx),%eax
 80482c1:       85 c0                   test   %eax,%eax
 80482c3:       74 05                   je     80482ca <_init+0x1e>
 80482c5:       e8 36 00 00 00          call   8048300 <__libc_start_main@plt+0x10>
 80482ca:       83 c4 08                add    $0x8,%esp
 80482cd:       5b                      pop    %ebx
 80482ce:       c3                      ret    

Disassembly of section .plt:

080482d0 <printf@plt-0x10>:
 80482d0:       ff 35 04 a0 04 08       pushl  0x804a004
 80482d6:       ff 25 08 a0 04 08       jmp    *0x804a008
 80482dc:       00 00                   add    %al,(%eax)
        ...

080482e0 <printf@plt>:
 80482e0:       ff 25 0c a0 04 08       jmp    *0x804a00c
 80482e6:       68 00 00 00 00          push   $0x0
 80482eb:       e9 e0 ff ff ff          jmp    80482d0 <_init+0x24>

080482f0 <__libc_start_main@plt>:
 80482f0:       ff 25 10 a0 04 08       jmp    *0x804a010
 80482f6:       68 08 00 00 00          push   $0x8
 80482fb:       e9 d0 ff ff ff          jmp    80482d0 <_init+0x24>

Disassembly of section .plt.got:

08048300 <.plt.got>:
 8048300:       ff 25 fc 9f 04 08       jmp    *0x8049ffc
 8048306:       66 90                   xchg   %ax,%ax

Disassembly of section .text:

08048310 <_start>:
 8048310:       31 ed                   xor    %ebp,%ebp
 8048312:       5e                      pop    %esi
 8048313:       89 e1                   mov    %esp,%ecx
 8048315:       83 e4 f0                and    $0xfffffff0,%esp
 8048318:       50                      push   %eax
 8048319:       54                      push   %esp
 804831a:       52                      push   %edx
 804831b:       68 f0 84 04 08          push   $0x80484f0
 8048320:       68 90 84 04 08          push   $0x8048490
 8048325:       51                      push   %ecx
 8048326:       56                      push   %esi
 8048327:       68 0b 84 04 08          push   $0x804840b
 804832c:       e8 bf ff ff ff          call   80482f0 <__libc_start_main@plt>
 8048331:       f4                      hlt    
 8048332:       66 90                   xchg   %ax,%ax
 8048334:       66 90                   xchg   %ax,%ax
 8048336:       66 90                   xchg   %ax,%ax
 8048338:       66 90                   xchg   %ax,%ax
 804833a:       66 90                   xchg   %ax,%ax
 804833c:       66 90                   xchg   %ax,%ax
 804833e:       66 90                   xchg   %ax,%ax

08048340 <__x86.get_pc_thunk.bx>:
 8048340:       8b 1c 24                mov    (%esp),%ebx
 8048343:       c3                      ret    
 8048344:       66 90                   xchg   %ax,%ax
 8048346:       66 90                   xchg   %ax,%ax
 8048348:       66 90                   xchg   %ax,%ax
 804834a:       66 90                   xchg   %ax,%ax
 804834c:       66 90                   xchg   %ax,%ax
 804834e:       66 90                   xchg   %ax,%ax

08048350 <deregister_tm_clones>:
 8048350:       b8 27 a0 04 08          mov    $0x804a027,%eax
 8048355:       2d 24 a0 04 08          sub    $0x804a024,%eax
 804835a:       83 f8 06                cmp    $0x6,%eax
 804835d:       76 1a                   jbe    8048379 <deregister_tm_clones+0x29>
 804835f:       b8 00 00 00 00          mov    $0x0,%eax
 8048364:       85 c0                   test   %eax,%eax
 8048366:       74 11                   je     8048379 <deregister_tm_clones+0x29>
 8048368:       55                      push   %ebp
 8048369:       89 e5                   mov    %esp,%ebp
 804836b:       83 ec 14                sub    $0x14,%esp
 804836e:       68 24 a0 04 08          push   $0x804a024
 8048373:       ff d0                   call   *%eax
 8048375:       83 c4 10                add    $0x10,%esp
 8048378:       c9                      leave  
 8048379:       f3 c3                   repz ret 
 804837b:       90                      nop
 804837c:       8d 74 26 00             lea    0x0(%esi,%eiz,1),%esi

08048380 <register_tm_clones>:
 8048380:       b8 24 a0 04 08          mov    $0x804a024,%eax
 8048385:       2d 24 a0 04 08          sub    $0x804a024,%eax
 804838a:       c1 f8 02                sar    $0x2,%eax
 804838d:       89 c2                   mov    %eax,%edx
 804838f:       c1 ea 1f                shr    $0x1f,%edx
 8048392:       01 d0                   add    %edx,%eax
 8048394:       d1 f8                   sar    %eax
 8048396:       74 1b                   je     80483b3 <register_tm_clones+0x33>
 8048398:       ba 00 00 00 00          mov    $0x0,%edx
 804839d:       85 d2                   test   %edx,%edx
 804839f:       74 12                   je     80483b3 <register_tm_clones+0x33>
 80483a1:       55                      push   %ebp
 80483a2:       89 e5                   mov    %esp,%ebp
 80483a4:       83 ec 10                sub    $0x10,%esp
 80483a7:       50                      push   %eax
 80483a8:       68 24 a0 04 08          push   $0x804a024
 80483ad:       ff d2                   call   *%edx
 80483af:       83 c4 10                add    $0x10,%esp
 80483b2:       c9                      leave  
 80483b3:       f3 c3                   repz ret 
 80483b5:       8d 74 26 00             lea    0x0(%esi,%eiz,1),%esi
 80483b9:       8d bc 27 00 00 00 00    lea    0x0(%edi,%eiz,1),%edi

080483c0 <__do_global_dtors_aux>:
 80483c0:       80 3d 24 a0 04 08 00    cmpb   $0x0,0x804a024
 80483c7:       75 13                   jne    80483dc <__do_global_dtors_aux+0x1c>
 80483c9:       55                      push   %ebp
 80483ca:       89 e5                   mov    %esp,%ebp
 80483cc:       83 ec 08                sub    $0x8,%esp
 80483cf:       e8 7c ff ff ff          call   8048350 <deregister_tm_clones>
 80483d4:       c6 05 24 a0 04 08 01    movb   $0x1,0x804a024
 80483db:       c9                      leave  
 80483dc:       f3 c3                   repz ret 
 80483de:       66 90                   xchg   %ax,%ax

080483e0 <frame_dummy>:
 80483e0:       b8 10 9f 04 08          mov    $0x8049f10,%eax
 80483e5:       8b 10                   mov    (%eax),%edx
 80483e7:       85 d2                   test   %edx,%edx
 80483e9:       75 05                   jne    80483f0 <frame_dummy+0x10>
 80483eb:       eb 93                   jmp    8048380 <register_tm_clones>
 80483ed:       8d 76 00                lea    0x0(%esi),%esi
 80483f0:       ba 00 00 00 00          mov    $0x0,%edx
 80483f5:       85 d2                   test   %edx,%edx
 80483f7:       74 f2                   je     80483eb <frame_dummy+0xb>
 80483f9:       55                      push   %ebp
 80483fa:       89 e5                   mov    %esp,%ebp
 80483fc:       83 ec 14                sub    $0x14,%esp
 80483ff:       50                      push   %eax
 8048400:       ff d2                   call   *%edx
 8048402:       83 c4 10                add    $0x10,%esp
 8048405:       c9                      leave  
 8048406:       e9 75 ff ff ff          jmp    8048380 <register_tm_clones>

0804840b <main>:

int out_sum;


int main(int argc, char *argv[])
{
 804840b:       8d 4c 24 04             lea    0x4(%esp),%ecx
 804840f:       83 e4 f0                and    $0xfffffff0,%esp
 8048412:       ff 71 fc                pushl  -0x4(%ecx)
 8048415:       55                      push   %ebp
 8048416:       89 e5                   mov    %esp,%ebp
 8048418:       51                      push   %ecx
 8048419:       83 ec 14                sub    $0x14,%esp
        int id = module_id;
 804841c:       a1 1c a0 04 08          mov    0x804a01c,%eax
 8048421:       89 45 ec                mov    %eax,-0x14(%ebp)
        char *name = module_name;
 8048424:       a1 20 a0 04 08          mov    0x804a020,%eax
 8048429:       89 45 f0                mov    %eax,-0x10(%ebp)
        int sum;


        printf("module：%d %s\n", id, name);
 804842c:       83 ec 04                sub    $0x4,%esp
 804842f:       ff 75 f0                pushl  -0x10(%ebp)
 8048432:       ff 75 ec                pushl  -0x14(%ebp)
 8048435:       68 10 85 04 08          push   $0x8048510
 804843a:       e8 a1 fe ff ff          call   80482e0 <printf@plt>
 804843f:       83 c4 10                add    $0x10,%esp

        sum = add(2,3);
 8048442:       83 ec 08                sub    $0x8,%esp
 8048445:       6a 03                   push   $0x3
 8048447:       6a 02                   push   $0x2
 8048449:       e8 2e 00 00 00          call   804847c <add>
 804844e:       83 c4 10                add    $0x10,%esp
 8048451:       89 45 f4                mov    %eax,-0xc(%ebp)
        printf("sum: %d\n", sum);
 8048454:       83 ec 08                sub    $0x8,%esp
 8048457:       ff 75 f4                pushl  -0xc(%ebp)
 804845a:       68 20 85 04 08          push   $0x8048520
 804845f:       e8 7c fe ff ff          call   80482e0 <printf@plt>
 8048464:       83 c4 10                add    $0x10,%esp


        out_sum = sum;
 8048467:       8b 45 f4                mov    -0xc(%ebp),%eax
 804846a:       a3 28 a0 04 08          mov    %eax,0x804a028

        return 0x0;
 804846f:       b8 00 00 00 00          mov    $0x0,%eax
}
 8048474:       8b 4d fc                mov    -0x4(%ebp),%ecx
 8048477:       c9                      leave  
 8048478:       8d 61 fc                lea    -0x4(%ecx),%esp
 804847b:       c3                      ret    

0804847c <add>:

int module_id = 1001;
char *module_name = "calc";

int add(int a, int b)
{
 804847c:       55                      push   %ebp
 804847d:       89 e5                   mov    %esp,%ebp
        return a+b;
 804847f:       8b 55 08                mov    0x8(%ebp),%edx
 8048482:       8b 45 0c                mov    0xc(%ebp),%eax
 8048485:       01 d0                   add    %edx,%eax
}
 8048487:       5d                      pop    %ebp
 8048488:       c3                      ret    
 8048489:       66 90                   xchg   %ax,%ax
 804848b:       66 90                   xchg   %ax,%ax
 804848d:       66 90                   xchg   %ax,%ax
 804848f:       90                      nop

08048490 <__libc_csu_init>:
 8048490:       55                      push   %ebp
 8048491:       57                      push   %edi
 8048492:       56                      push   %esi
 8048493:       53                      push   %ebx
 8048494:       e8 a7 fe ff ff          call   8048340 <__x86.get_pc_thunk.bx>
 8048499:       81 c3 67 1b 00 00       add    $0x1b67,%ebx
 804849f:       83 ec 0c                sub    $0xc,%esp
 80484a2:       8b 6c 24 20             mov    0x20(%esp),%ebp
 80484a6:       8d b3 0c ff ff ff       lea    -0xf4(%ebx),%esi
 80484ac:       e8 fb fd ff ff          call   80482ac <_init>
 80484b1:       8d 83 08 ff ff ff       lea    -0xf8(%ebx),%eax
 80484b7:       29 c6                   sub    %eax,%esi
 80484b9:       c1 fe 02                sar    $0x2,%esi
 80484bc:       85 f6                   test   %esi,%esi
 80484be:       74 25                   je     80484e5 <__libc_csu_init+0x55>
 80484c0:       31 ff                   xor    %edi,%edi
 80484c2:       8d b6 00 00 00 00       lea    0x0(%esi),%esi
 80484c8:       83 ec 04                sub    $0x4,%esp
 80484cb:       ff 74 24 2c             pushl  0x2c(%esp)
 80484cf:       ff 74 24 2c             pushl  0x2c(%esp)
 80484d3:       55                      push   %ebp
 80484d4:       ff 94 bb 08 ff ff ff    call   *-0xf8(%ebx,%edi,4)
 80484db:       83 c7 01                add    $0x1,%edi
 80484de:       83 c4 10                add    $0x10,%esp
 80484e1:       39 f7                   cmp    %esi,%edi
 80484e3:       75 e3                   jne    80484c8 <__libc_csu_init+0x38>
 80484e5:       83 c4 0c                add    $0xc,%esp
 80484e8:       5b                      pop    %ebx
 80484e9:       5e                      pop    %esi
 80484ea:       5f                      pop    %edi
 80484eb:       5d                      pop    %ebp
 80484ec:       c3                      ret    
 80484ed:       8d 76 00                lea    0x0(%esi),%esi

080484f0 <__libc_csu_fini>:
 80484f0:       f3 c3                   repz ret 

Disassembly of section .fini:

080484f4 <_fini>:
 80484f4:       53                      push   %ebx
 80484f5:       83 ec 08                sub    $0x8,%esp
 80484f8:       e8 43 fe ff ff          call   8048340 <__x86.get_pc_thunk.bx>
 80484fd:       81 c3 03 1b 00 00       add    $0x1b03,%ebx
 8048503:       83 c4 08                add    $0x8,%esp
 8048506:       5b                      pop    %ebx
 8048507:       c3                      ret    
{% endhighlight %}


5) **显示指定section信息**

通过如下命令显示data段信息：
<pre>
# objdump -s -j .data main

main:     file format elf32-i386

Contents of section .data:
 804a014 00000000 00000000 e9030000 29850408  ............)...
</pre>

## 2. ar命令
GNU ar命令可用于创建或修改一个档案库(archive)，也可以从archive中提取相应的信息。一个archive将许多其他的子文件打包在一起，形成一个独立的文件，之后可以从中提取出原始子文件的信息。

原始文件的contents、mode(permissions)、timestamp、owner以及group等信息在打包之后仍会保留，并且在解压提取的时候可以进行恢复。

对于一个由GNU ar命令所维护的archive，其中的子文件名称是可以任意长度的。然而，取决于系统对ar的配置，实际上可能会对文件名长度有一个限制，以兼容一些其他的archive工具。假如存在这样一个限制的话，限制值通常是15或16个字符。


当我们使用```ar -s```选项创建archive的时候，其会对其中的可重定位(relocatable)符号创建一个索引(index)。索引一旦创建，则后续通过ar命令对archive内容进行修改时，这个索引都会被更新。拥有这种index的archive可以加快库的链接速度，并允许库中的函数相互调用。

我们可以使用```nm -s```或者```nm --print-armap```来列出这个索引表。假如一个archive并没有这种索引表，我们可以使用```ranlib```命令来向其添加。

ar命令的基本使用格式如下：
<pre>
# ar [-X32_64] [-]p[mod] [--plugin name] [--target bfdname] [relpos] [count]
       archive [member...]
</pre>


### 2.1 ar命令选项
GNU ar允许你在命令行的第一个参数中将```操作码选项(p)```与```修正符选项(mod)```按任意的顺序合并在一起。

1) **操作码选项**

其中的```p```关键字用于指明我们需要执行何种操作，其可以是如下的任何```一个```：
<pre>
d       从archive删除modules。可以在后面指定要删除的module名称。假如并未指定任何的要删除的文件名，
        则原archive并不会做任何修改。

        假如在使用本选项时指定了 v 修正符，则ar会列出当前删除了哪些module

m       该操作是用于在archive中移动成员
        当库中如果有若干模块都定义有相同的符号(例如函数定义），则archive中该成员的位置顺序会影响到lib
        库的链接。

        假如我们在使用 m 选项时并未指定其他的修正符，则任何要移动的members都会移动到archive的末尾，
        我们可以使用a、b或者i修正符来控制移动到什么位置。

p       显示archive中指定的成员到标准输出。假如指定了 v 修正符，则在输出成员的内容前，将显示成员的名字
        假如并未指定任何成员名字的话，则库中的所有文件都将显示出来。

q       快速追加(quick append)。将新模块添加到archive的结尾处，并不检查是否需要替换。

        修正符a、b和i并不会对本选项有任何影响。新添加的模块(members)总是被追加到archive的末尾。

        修正符v会使得ar列出其所追加的每一个成员文件。

        由于本操作主要考虑的一个点就是速度(speed)，因此在实现ar时可以选择不更新符号表索引（假如符号表索
        引存在的话）。由于很多不同的系统都会假设符号表总是处于最新状态，因此GNU ar即使在进行quick append
        时也会对符号表进行重新构建(rebuild)

        注： GNU ar将'ar -qs'与'ar -r'看成是相同的————替换archive中以存在的成员文件，而将新的成员文件
        追加到archive的末尾

r       在archive中插入(替换)模块。本操作与上面的选项q是不同的，假如新添加的模块在archive中已经存在，
        则首先将新的追加到末尾，然后再删除原来旧的。

        如果若干模块中有一个模块在库中不存在，ar显示一个错误消息，并不替换其他同名模块。

        默认的情况下，新的成员增加在库的结尾处，可以通过a、b、i等修正符来将其添加到某个指定的位置。

s       添加一个索引到archive，或者更新已存在的索引。值的注意的是，本选项不能搭配其他修饰符一起使用。

t       显示archive的模块表清单。通常情况下只会显示成员文件名称，而如果想要查看modes(permissions)、
        timestamp、owner、group以及size等信息时，可以添加 v 选项。

        假如不在本选项后面添加任何member name的话，则会显示archive中的所有members。

x       从库中提取一个成员。如果不指定要提取的模块，则提取库中所有的模块。我们结合使用 v 选项来列出所有
        解压的成员名称

--help  获取ar命令的帮助信息

--version 获取ar命令的版本号
</pre>

2) **修正符选项**

ar命令的修正符选项(mod)可以跟在操作符选项(p)之后。可以在操作符选项之后添加若干个修正符选项：
<pre>
a      在一个已存在的成员之后添加新的file到archive。如果使用任选项a，则我们在relpos位置处指定一个已经
       存在的member。

b      在一个已存在的成员之前添加新的file到archive。如果使用任选项a，则我们在relpos位置处指定一个已经
       存在的member。（与i修正符相同)

c      创建archive。不管库是否存在，都将创建

f      在库中截短指定的名字。缺省情况下，文件名的长度是不受限制的，可以使用此参数将文件名截短，以保证与
       其它系统的兼容。     

i      在一个已存在的成员之前添加新的file到archive。如果使用任选项a，则我们在relpos位置处指定一个已经
       存在的member。（与b修正符相同)

l      本修正符当前并未使用。

N      与count参数一起使用，在库中有多个相同的文件名时指定提取或输出第N个。

o      当提取成员时，保留成员的原始日期数据。如果不指定该任选项，则提取出的模块的时间将标为提取出的时间

P      进行文件名匹配时使用全路径名。ar在创建库时不能使用全路径名（这样的库文件不符合POSIX标准），但是
       有些工具可以

s      写入一个目标文件索引到库中，或者更新一个存在的目标文件索引。甚至对于没有任何变化的库也作该动作。你
       可以单独使用此选项，也可以搭配其他操作符使用。在一个archive上运行'ar -s'等价于在一个archive上运
       行ranlib命令。
      
S      不创建目标文件索引，这在创建较大的库时能加快时间。但是这样的lib库不能被链接，因此我们在使用时不要
       添加本选项。

u      一般来说，命令ar -r ...插入所有列出的文件到库中。如果你只想插入列出文件中那些比库中同名文件新的文
       件，就可以使用该任选项。该任选项只用于r操作选项。

v      该选项用来显示执行操作的verbose信息。

V      显示ar命令的版本
</pre>

### 2.2 ar命令使用实例
1) **创建archive**

这里我们使用```ar```命令来为上一节的add.o创建```动态链接库```与```静态链接库```(注： ar命令只能用来创建静态链接库)：

* 创建动态链接库
<pre>
# gcc -fPIC -shared add.c -o libCalc.so      //单个文件
# gcc -fPIC -shared add.o -o libCalc.so      //单个文件

# gcc -fPIC -shared add.c minus.c -o libCalc.so   //多个文件 
# gcc -fPIC -shared add.o minus.o -o libCalc.so   //单个文件
</pre>


* 创建静态链接库
<pre>
# ar -rc libCalc.a add.o    //单个文件
# ar -rc libCalc.a add.c    //注： 静态库可以生成；当运行连接了该静态库的可执行程序会报错：
                              could not read symbols:Archive has no index;run ranlib to add one


# ar -rc libCalc.a add.o minus.o   //多个文件
# ar -rc libCalc.a add.c minus.c   // 注：静态库可以生成；当运行连接了该静态库的可执行程序会报错：
                                    could not read symbols:Archive has no index;run ranlib to add one
</pre>


2） **查看lib库的成员**
<pre>
# ar -rcv libCalc.a add.o
a - add.o
# ls
add.c  add.h  add.o  libCalc.a  main  main.c  main.o
# ar -t libCalc.a
add.o
</pre>


## 3. nm命令
nm命令用于列出目标文件的symbols(Lists symbols from object files)。其基本使用方法如下：
<pre>
nm [-A|-o|--print-file-name] [-a|--debug-syms]
  [-B|--format=bsd] [-C|--demangle[=style]]
  [-D|--dynamic] [-fformat|--format=format]
  [-g|--extern-only] [-h|--help]
  [-l|--line-numbers] [-n|-v|--numeric-sort]
  [-P|--portability] [-p|--no-sort]
  [-r|--reverse-sort] [-S|--print-size]
  [-s|--print-armap] [-t radix|--radix=radix]
  [-u|--undefined-only] [-V|--version]
  [-X 32_64] [--defined-only] [--no-demangle]
  [--plugin name] [--size-sort] [--special-syms]
  [--synthetic] [--target=bfdname]
  [objfile...]
</pre>

对于列出的每一个symbol，nm命令会显示如下信息：

* ```symbol value``` nm会以指定的基数来显示symbol的值，默认是十六进制。

* ```symbol type``` 符号的类型可能是如下类型。If lowercase, the symbol is usually local; 
                    if uppercase, the symbol is global (external).  There are however a
                    few lowercase symbols that are shown for special global symbols ("u", "v" and "w")
<pre>

"A" The symbol's value is absolute, and will not be changed by further
   linking.

"B"
"b" The symbol is in the uninitialized data section (known as BSS).

"C" The symbol is common.  Common symbols are uninitialized data.  When
   linking, multiple common symbols may appear with the same name.  If the
   symbol is defined anywhere, the common symbols are treated as undefined
   references.

"D"
"d" The symbol is in the initialized data section.

"G"
"g" The symbol is in an initialized data section for small objects.  Some
   object file formats permit more efficient access to small data objects,
   such as a global int variable as opposed to a large global array.

"i" For PE format files this indicates that the symbol is in a section
   specific to the implementation of DLLs.  For ELF format files this
   indicates that the symbol is an indirect function.  This is a GNU
   extension to the standard set of ELF symbol types.  It indicates a symbol
   which if referenced by a relocation does not evaluate to its address, but
   instead must be invoked at runtime.  The runtime execution will then
   return the value to be used in the relocation.

"I" The symbol is an indirect reference to another symbol.

"N" The symbol is a debugging symbol.

"p" The symbols is in a stack unwind section.

"R"
"r" The symbol is in a read only data section.

"S"
"s" The symbol is in an uninitialized data section for small objects.

"T"
"t" The symbol is in the text (code) section.

"U" The symbol is undefined.

"u" The symbol is a unique global symbol.  This is a GNU extension to the
   standard set of ELF symbol bindings.  For such a symbol the dynamic
   linker will make sure that in the entire process there is just one symbol
   with this name and type in use.

"V"
"v" The symbol is a weak object.  When a weak defined symbol is linked with a
   normal defined symbol, the normal defined symbol is used with no error.
   When a weak undefined symbol is linked and the symbol is not defined, the
   value of the weak symbol becomes zero with no error.  On some systems,
   uppercase indicates that a default value has been specified.

"W"
"w" The symbol is a weak symbol that has not been specifically tagged as a
   weak object symbol.  When a weak defined symbol is linked with a normal
   defined symbol, the normal defined symbol is used with no error.  When a
   weak undefined symbol is linked and the symbol is not defined, the value
   of the symbol is determined in a system-specific manner without error.
   On some systems, uppercase indicates that a default value has been
   specified.

"-" The symbol is a stabs symbol in an a.out object file.  In this case, the
   next values printed are the stabs other field, the stabs desc field, and
   the stab type.  Stabs symbols are used to hold debugging information.

"?" The symbol type is unknown, or object file format specific.
</pre>

* ```symbol name```

### 3.1 nm命令选项

这里我们只列出几个常用的：
<pre>
-A
-o
--print-file-name
    在列出的symbol前面显示该符号所在的.o文件名

-a
--debug-syms
    显示所有的symbols信息，包括用于调试的symbols

-s
--print-armap
    当列出库中成员的symbols时，也列出索引。索引的内容包含：哪些模块包含哪些名字的映射
</pre>


### 3.2 用法举例
<pre>
# nm -As ./libCalc.a

Archive index:
module_id in add.o
module_name in add.o
add in add.o
./libCalc.a:add.o:00000000 T add
./libCalc.a:add.o:00000000 D module_id
./libCalc.a:add.o:00000004 D module_name
</pre>



<br />
<br />
**[参看]:**

1. [Linux系统下:OBJ文件格式分析工具–objdump, nm,ar](http://qiusuoge.com/12021.html)

2. [linux objdump 反汇编命令](https://blog.csdn.net/whatday/article/details/99154104)

<br />
<br />
<br />





