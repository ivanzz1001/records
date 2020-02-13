---
layout: post
title: 
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
* ```**.text**``` 已编译程序的机器代码

* ```**.rodata**``` 只读数据，比如printf语句中的格式串和开关(switch)语句的跳转表




<br />
<br />
**[参看]:**

1. [Linux系统下:OBJ文件格式分析工具–objdump, nm,ar](http://qiusuoge.com/12021.html)

2. [linux objdump 反汇编命令](https://blog.csdn.net/whatday/article/details/99154104)

<br />
<br />
<br />





