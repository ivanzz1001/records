---
layout: post
title: GDB调试之栈帧、汇编
tags:
- cplusplus
categories: cplusplus
description: GDB的使用
---

本文档主要参看<<Debugging with GDB>> ```Tenth Edition, for gdb version 8.0.1```。

本文我们主要介绍通过GDB来查看栈帧信息，以及汇编信息。


<!-- more -->


## 1. 检查调用栈帧

当程序暂停之后，你需要知道的第一件事情就是程序停止在什么地方，以及程序为什么会暂停在这里。每一次程序进行一次函数调用，都会产生相应的调用信息。这些信息包括函数调用的位置、函数调用参数、和被调用函数的本地局部变量。这些信息被保存在一个被称为栈帧(stack frame)的地方。stack frame是在内存中分配的一块区域，我们称之为调用栈。

当程序暂停之后，我们可以通过GDB相应的命令来查看栈帧中的信息。其中一个栈帧(stack frame)会被GDB所选中，并且后续许多GDB的相关命令是隐式的在所选中栈帧(selected frame)上操作。特别是，无论何时当你使用GDB来获取某一个变量的值时，其值都会在栈帧中找到。

一般来说，当程序暂停时，GDB都会自动的选择当前执行帧，并打印出简单的描述信息，这有些类似于```frame```命令。


### 1.1 Stack Frames
调用栈被分割成了一些连续的被称为```stack frames```（简称为frames)块，每一帧都包含了调用一个函数所相关的数据。```frame```包含有传递给函数的参数、函数的本地局部变量、和该函数正在执行的地址。

当程序启动之后，stack只有一帧(frame)，那就是函数```main```。这被称为```初始帧```(initial frame)或者```最外层帧```(outermost frame)。每一次一个函数被调用，就会创建一个新的帧； 而每一次一个函数返回，对应于该函数调用的帧将会被移除。假如一个函数是递归的，则针对同一个函数会有很多帧。对于当前正在调用的函数的帧，我们称之为```最里层帧```(innermost frame)。这是所有仍存在的帧中最新被创建的。

在程序中，栈帧都是由它们的地址所标识。每一个栈帧都由许多字节组成，其中的每一个字节都有其对应的地址；每一种计算机都有其相应的方法来获取栈帧的地址。通常情况下，当在执行该帧的时候，栈帧的地址会保存在```帧指针寄存器```(frame pointer register)中。


GDB会为每一个存在的栈帧指定一个数值编号，对于```innermost```帧编号为0，外层的帧的编号逐层加1。这些数字并不是实际存在于程序中的；它们只是GDB所指定的一个编号，GDB的一些命令可以使用这些编号来进行操作。


有一些编译器提供一种方法来编译函数，使得在操作这些函数时不需要栈帧。（例如，GCC的```-fomit-frame-pointer```选项将会生成没有```frame```的函数）。这通常用在那些被频繁使用的库函数中，以节省建立frame的时间。GDB对于在处理这种函数调用时有相应的限制。假如innermost函数调用并没有stack frame，GDB无论如何都会将其当成是一个独立的帧，将其编号为0，以允许正确的跟踪函数调用链。然而对于栈中的其他```frameless function```则没有这样的规定。


### 1.2 回溯(Backtraces)

一个backtrace会整体上描述程序是如何调用到此处的。每一帧(frame)会显示一行，对于许多帧的话，会从frame 0开始一层一层显示。下面给出一些经常使用的命令：

1） **backtrace / bt**

用于回溯整个调用栈： 对于栈中的所有帧来说，每一行对应一个帧。在任何时间你都可以通过使用```CTRL-c```来暂停回溯。

2） **backtrace n / bt n**

与上面```backtrace```类似，但是只回溯innermost的```n```个帧。

3) **backtrace -n / bt -n**

与上面```backtrace```类似，但是只回溯outermost的```n```个帧。

4) **backtrace full / bt full / bt full n / bt full -n**

与上面```backtrace```类似，但同时也会打印本地局部变量的值。

<br />

在多线程程序中，GDB默认情况下只会打印当前线程的的backtrace。如果要显示所有线程的backtrace的话，那么可以使用```thread apply```命令。例如，假如你使用'thread apply all backtrace'，GDB就会打印出所有线程的backtrace。当你在调试一个多线程程序的coredump时，经常会这样做。


backtrace的每一行都会打印```帧的编号```(frame number)以及函数名称。如下是一个调用```bt 3```命令的示例：
<pre>
#0 m4_traceon (obs=0x24eb0, argc=1, argv=0x2b8c8)
at builtin.c:993
#1 0x6e38 in expand_macro (sym=0x2b600, data=...) at macro.c:242
#2 0x6840 in expand_token (obs=0x0, t=177664, td=0xf7fffb08)
at macro.c:71
(More stack frames follow...)
</pre>
上面打印了最里层的3个帧。


### 1.3 选择一个帧
大多数用于检查栈和其他数据的命令都是在当前所选择的```帧```上来进行的。如下就是一些用于选择帧的命令，所有的这些命令在选择完```栈帧```（stack frame)之后都会打印一个简短的描述信息。

1） **frame n / f n**

用于选择编号为```n```的帧。

2） **frame stack-addr [ pc-addr ] / f stack-addr [ pc-addr ]**

选择```stack-addr```位置处的帧。这主要用在一些栈帧链被损坏的情况下，因为在这种情况下，GDB可能不能够为所有的frame都指定编号了。另外，假如你的程序有多个栈时，也可以通过此命令来进行切换。后面可选```pc-addr```可以用于指定该栈帧的PC。

3) **up n**

向上移动```n```个帧。默认情况下，```n```的值为1。若```n>0```，则向outermost方向移动

4） **down n**

向下移动```n```个帧。默认情况下，```n```的值为1。若```n>0```，则向innermost方向移动。

<br />

上面所有的这些命令在执行之后会打印出两行描述帧的信息。其中第一行用于显示帧的编号、函数名称、参数、源代码名称、以及当前在栈帧中执行到的行号； 第二行用于显示该行的源代码。例如：
<pre>
(gdb) up
#1 0x22f0 in main (argc=1, argv=0xf7fffbf4, env=0xf7fffbfc)
at env.c:10
10 read_input_file (argv[i]);
</pre>

在如上信息打印之后，执行不带参数的```list```命令会打印以该执行点为中心的10行代码。


### 1.4 帧信息
有很多其他的命令来打印所选择栈帧(selected stack frame)的信息：

1） **frame / f**

当并不携带任何参数时，本命令并不会改变所选中的帧，但是会打印当前所选中栈帧的一个简短描述信息。而如果携带参数时，则会改变当前所选中的栈帧。


2） **info frame / info f**

用于打印当前所选中栈帧的详细信息，主要包括：

* 帧的地址

* 下一个帧的地址（called by this frame)

* 上一个帧的地址( caller of this frame)

* 栈帧所对应的编程语言

* 该帧参数的地址

* 帧局部变量的地址

* 程序计数器中保存的值（即本函数执行完的返回地址）

* frame中哪些寄存器的值被保存

3） **info frame addr / info f addr**

用于打印指定地址处的栈帧详细信息，而并不需要说事先选择该栈帧

4） **info args**

打印所选中栈帧的参数，每一个参数对应一行。

5) **info locals**

打印所选中栈帧的局部变量的值，每一个对应一行。这是所选中栈帧(selected frame)可访问的所有变量。（包括声明为static的变量以及自动化变量）

### 1.5 使用示例

1.5.1 **打印函数堆栈信息**

1) 示例程序
{% highlight string %}
#include <stdio.h>

int func(int a, int b)
{
	int c = a * b;
	
	printf("c is %d\n", c);
	
	return c;
}

int main(int argc,char *argv[])
{
	func(1,2);
	
	return 0x0;
}
{% endhighlight %}

2) 调试技巧

使用GDB调试程序时，可以使用```info frame```显示函数堆栈信息。以上面程序为例：
{% highlight  string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) list 5
1       #include <stdio.h>
2
3       int func(int a, int b)
4       {
5               int c = a * b;
6
7               printf("c is %d\n", c);
8
9               return c;
10      }
(gdb) b 7
Breakpoint 1 at 0x400545: file test.c, line 7.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, func (a=1, b=2) at test.c:7
7               printf("c is %d\n", c);
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) info frame
Stack level 0, frame at 0x7fffffffe540:
 rip = 0x400545 in func (test.c:7); saved rip 0x40057c
 called by frame at 0x7fffffffe560
 source language c.
 Arglist at 0x7fffffffe530, args: a=1, b=2
 Locals at 0x7fffffffe530, Previous frame's sp is 0x7fffffffe540
 Saved registers:
  rbp at 0x7fffffffe530, rip at 0x7fffffffe538
(gdb) info registers
rax            0x2      2
rbx            0x0      0
rcx            0x400590 4195728
rdx            0x7fffffffe648   140737488348744
rsi            0x2      2
rdi            0x1      1
rbp            0x7fffffffe530   0x7fffffffe530
rsp            0x7fffffffe510   0x7fffffffe510
r8             0x7ffff7dd7e80   140737351876224
r9             0x0      0
r10            0x7fffffffe3a0   140737488348064
r11            0x7ffff7a3da40   140737348098624
r12            0x400440 4195392
r13            0x7fffffffe630   140737488348720
r14            0x0      0
r15            0x0      0
rip            0x400545 0x400545 <func+24>
eflags         0x202    [ IF ]
cs             0x33     51
ss             0x2b     43
ds             0x0      0
es             0x0      0
fs             0x0      0
gs             0x0      0
(gdb) disassemble func
Dump of assembler code for function func:
   0x000000000040052d <+0>:     push   %rbp
   0x000000000040052e <+1>:     mov    %rsp,%rbp
   0x0000000000400531 <+4>:     sub    $0x20,%rsp
   0x0000000000400535 <+8>:     mov    %edi,-0x14(%rbp)
   0x0000000000400538 <+11>:    mov    %esi,-0x18(%rbp)
   0x000000000040053b <+14>:    mov    -0x14(%rbp),%eax
   0x000000000040053e <+17>:    imul   -0x18(%rbp),%eax
   0x0000000000400542 <+21>:    mov    %eax,-0x4(%rbp)
=> 0x0000000000400545 <+24>:    mov    -0x4(%rbp),%eax
   0x0000000000400548 <+27>:    mov    %eax,%esi
   0x000000000040054a <+29>:    mov    $0x400620,%edi
   0x000000000040054f <+34>:    mov    $0x0,%eax
   0x0000000000400554 <+39>:    callq  0x400410 <printf@plt>
   0x0000000000400559 <+44>:    mov    -0x4(%rbp),%eax
   0x000000000040055c <+47>:    leaveq 
   0x000000000040055d <+48>:    retq   
End of assembler dump.
(gdb) 
{% endhighlight %}
上面我们可以看到，在执行完```info frame```命令后，输出了当前栈帧的地址、指令寄存器的值、局部变量的地址及值等信息。可以参照当前寄存器的值和函数的汇编指令看一下。

<br />

1.5.2 **选择函数栈帧**

1) 示例程序
{% highlight string %}
#include <stdio.h>

int func1(int a)
{
        return 2 * a;
}

int func2(int a)
{
        int c = 0;
        c = 2 * func1(a);
        return c;
}

int func3(int a)
{
        int c = 0;
        c = 2 * func2(a);
        return c;
}

int main(int argc,char *argv[])
{
        printf("%d\n", func3(10));
        return 0;
}
{% endhighlight %}

2) 调试技巧

用GDB调试程序时，当程序暂停后，可以用```frame n```命令选择函数栈帧，其中```n```是层数。以上面程序为例：
{% highlight string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b test.c:5
Breakpoint 1 at 0x400534: file test.c, line 5.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, func1 (a=10) at test.c:5
5               return 2 * a;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) bt
#0  func1 (a=10) at test.c:5
#1  0x0000000000400557 in func2 (a=10) at test.c:11
#2  0x000000000040057d in func3 (a=10) at test.c:18
#3  0x00000000004005a0 in main (argc=1, argv=0x7fffffffe638) at test.c:24
(gdb) frame 2
#2  0x000000000040057d in func3 (a=10) at test.c:18
18              c = 2 * func2(a);
{% endhighlight %}
可以看到程序断住后，最内层的函数帧为第```0```帧。执行```frame 2```命令后，当前的堆栈帧变成了```fun3```的函数帧。也可以用```frame addr```命令选择函数栈帧，其中```addr```时堆栈地址。仍以上面程序为例：
{% highlight string %}
(gdb) frame 2
#2  0x000000000040057d in func3 (a=10) at test.c:18
18              c = 2 * func2(a);
(gdb) info frame
Stack level 2, frame at 0x7fffffffe540:
 rip = 0x40057d in func3 (test.c:18); saved rip 0x4005a0
 called by frame at 0x7fffffffe560, caller of frame at 0x7fffffffe518
 source language c.
 Arglist at 0x7fffffffe530, args: a=10
 Locals at 0x7fffffffe530, Previous frame's sp is 0x7fffffffe540
 Saved registers:
  rbp at 0x7fffffffe530, rip at 0x7fffffffe538
(gdb) frame 0x7fffffffe518
#1  0x0000000000400557 in func2 (a=10) at test.c:11
11              c = 2 * func1(a);
{% endhighlight %}
使用```info frame```命令可以知道 0x7fffffffe518 是fun2()的函数栈帧地址，使用```frame 0x7fffffffe518```可以切换到fun2()的函数栈帧。

<br />

1.5.3 **向上或向下切换函数栈帧**

1） 示例程序
{% highlight string %}
#include <stdio.h>

int func1(int a)
{
        return 2 * a;
}

int func2(int a)
{
        int c = 0;
        c = 2 * func1(a);
        return c;
}

int func3(int a)
{
        int c = 0;
        c = 2 * func2(a);
        return c;
}

int main(int argc,char *argv[])
{
        printf("%d\n", func3(10));
        return 0;
}
{% endhighlight %}


2) 调试技巧

用GDB调试程序时，当程序暂停后，可以用```up n```或者```down n```命令向上或向下选择函数栈帧，其中```n```是层数。以上面程序为例：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b test.c:5
Breakpoint 1 at 0x400534: file test.c, line 5.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, func1 (a=10) at test.c:5
5               return 2 * a;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) bt
#0  func1 (a=10) at test.c:5
#1  0x0000000000400557 in func2 (a=10) at test.c:11
#2  0x000000000040057d in func3 (a=10) at test.c:18
#3  0x00000000004005a0 in main (argc=1, argv=0x7fffffffe638) at test.c:24
(gdb) frame 2
#2  0x000000000040057d in func3 (a=10) at test.c:18
18              c = 2 * func2(a);
(gdb) up 1
#3  0x00000000004005a0 in main (argc=1, argv=0x7fffffffe638) at test.c:24
24              printf("%d\n", func3(10));
(gdb) down 2
#1  0x0000000000400557 in func2 (a=10) at test.c:11
11              c = 2 * func1(a);
(gdb) 
{% endhighlight %}
可以看到程序断住后，先执行```frame 2```命令，切换到```fun3```函数。接着执行```up 1```命令，此时会切换到main函数，也就是会往外层的栈帧移动一层。反之，当执行```down 2```命令后，又会向内层栈帧移动两层。如果不指定```n```，则n默认为1。

还有```up-silently  n```和```down-silently n```这两个命令，与```up n```和```down n```命令区别在于，切换栈帧后，不会打印信息。




## 2. 汇编

### 2.1 设置汇编指令格式

1) 示例程序
{% highlight string %}
{% endhighlight %}



<br />
<br />

**[参看]**


1. [100个gdb小技巧](https://www.kancloud.cn/wizardforcel/gdb-tips-100/146771)

2. [设置 GDB 代码搜索路径](https://blog.csdn.net/caspiansea/article/details/42447203)

3. [AT&T汇编格式与Intel汇编格式的比较](https://blog.csdn.net/samxx8/article/details/12613643)

<br />
<br />
<br />





