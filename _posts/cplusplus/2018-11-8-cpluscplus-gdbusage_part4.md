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
#include <stdio.h>
int global_var;

void change_var(){
    global_var=100;
}

int main(int argc, char *argv[]){
    change_var();
    return 0;
}
{% endhighlight %}

2) 调试技巧

在```intel x86```处理器上，GDB默认显示汇编指令格式是```AT & T```格式。例如：
{% highlight string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) disassemble change_var
Dump of assembler code for function change_var:
   0x00000000004004ed <+0>:     push   %rbp
   0x00000000004004ee <+1>:     mov    %rsp,%rbp
   0x00000000004004f1 <+4>:     movl   $0x64,0x200b35(%rip)        # 0x601030 <global_var>
   0x00000000004004fb <+14>:    pop    %rbp
   0x00000000004004fc <+15>:    retq   
End of assembler dump.
(gdb) disassemble main
Dump of assembler code for function main:
   0x00000000004004fd <+0>:     push   %rbp
   0x00000000004004fe <+1>:     mov    %rsp,%rbp
   0x0000000000400501 <+4>:     sub    $0x10,%rsp
   0x0000000000400505 <+8>:     mov    %edi,-0x4(%rbp)
   0x0000000000400508 <+11>:    mov    %rsi,-0x10(%rbp)
   0x000000000040050c <+15>:    mov    $0x0,%eax
   0x0000000000400511 <+20>:    callq  0x4004ed <change_var>
   0x0000000000400516 <+25>:    mov    $0x0,%eax
   0x000000000040051b <+30>:    leaveq 
   0x000000000040051c <+31>:    retq   
End of assembler dump.
{% endhighlight %}

可以使用```set disassembly-flavor```命令将格式改为```intel```格式：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) set disassembly-flavor intel
(gdb) disassemble change_var
Dump of assembler code for function change_var:
   0x00000000004004ed <+0>:     push   rbp
   0x00000000004004ee <+1>:     mov    rbp,rsp
   0x00000000004004f1 <+4>:     mov    DWORD PTR [rip+0x200b35],0x64        # 0x601030 <global_var>
   0x00000000004004fb <+14>:    pop    rbp
   0x00000000004004fc <+15>:    ret    
End of assembler dump.
(gdb) disassemble main
Dump of assembler code for function main:
   0x00000000004004fd <+0>:     push   rbp
   0x00000000004004fe <+1>:     mov    rbp,rsp
   0x0000000000400501 <+4>:     sub    rsp,0x10
   0x0000000000400505 <+8>:     mov    DWORD PTR [rbp-0x4],edi
   0x0000000000400508 <+11>:    mov    QWORD PTR [rbp-0x10],rsi
   0x000000000040050c <+15>:    mov    eax,0x0
   0x0000000000400511 <+20>:    call   0x4004ed <change_var>
   0x0000000000400516 <+25>:    mov    eax,0x0
   0x000000000040051b <+30>:    leave  
   0x000000000040051c <+31>:    ret    
End of assembler dump.
{% endhighlight %}
目前```set disassembly-flavor```命令只能用在```intel x86```处理器上，并且取值只有```intel```和```att```。

3) 函数调用汇编分析

从上面我们可以看到，通过使用```callq <address>```指令来调用一个函数。其中```address```为调用函数的入口地址。进入调用函数之后首先将```rbp```入栈，以保存上一个栈帧的基址。接着```rbp```保存了当前```rsp```的值。而在函数退出时，会从栈中恢复```rbp```的值。

参看下图：

![gdb-stack-frame](https://ivanzz1001.github.io/records/assets/img/cplusplus/gdb_stack_frame.jpg)



4) 带参数和返回值的函数调用

上面我们基本弄清楚了函数的调用流程，下面我们就简单来看一下带参数和返回值的函数调用：
{% highlight string %}
#include <stdio.h>

int sum(int a, int b, int c, int d, int e, int f, int g, int h,int i,int j)
{
        int total;

        total = a + b + c + d + e + f + g + h + i + j;

        return total;
}

int main(int argc, char *argv[])
{
        int a, b, c, d, e, f, g, h, i,j, total;

        a = 1;
        b = 2;
        c = 3;
        d = 4;
        e = 5;
        f = 6;
        g = 7;
        h = 8;
        i = 9;
        j = 10;

        total = sum(a, b, c, d, e, f, g, h, i,j);

        printf("sum: %d\n", total);

        return 0;
}
{% endhighlight %}


如下我们编译调试:
{% highlight string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) disassemble sum
Dump of assembler code for function sum:
   0x000000000040052d <+0>:     push   %rbp
   0x000000000040052e <+1>:     mov    %rsp,%rbp
   0x0000000000400531 <+4>:     mov    %edi,-0x14(%rbp)
   0x0000000000400534 <+7>:     mov    %esi,-0x18(%rbp)
   0x0000000000400537 <+10>:    mov    %edx,-0x1c(%rbp)
   0x000000000040053a <+13>:    mov    %ecx,-0x20(%rbp)
   0x000000000040053d <+16>:    mov    %r8d,-0x24(%rbp)
   0x0000000000400541 <+20>:    mov    %r9d,-0x28(%rbp)
   0x0000000000400545 <+24>:    mov    -0x18(%rbp),%eax
   0x0000000000400548 <+27>:    mov    -0x14(%rbp),%edx
   0x000000000040054b <+30>:    add    %eax,%edx
   0x000000000040054d <+32>:    mov    -0x1c(%rbp),%eax
   0x0000000000400550 <+35>:    add    %eax,%edx
   0x0000000000400552 <+37>:    mov    -0x20(%rbp),%eax
   0x0000000000400555 <+40>:    add    %eax,%edx
   0x0000000000400557 <+42>:    mov    -0x24(%rbp),%eax
   0x000000000040055a <+45>:    add    %eax,%edx
   0x000000000040055c <+47>:    mov    -0x28(%rbp),%eax
   0x000000000040055f <+50>:    add    %eax,%edx
   0x0000000000400561 <+52>:    mov    0x10(%rbp),%eax
   0x0000000000400564 <+55>:    add    %eax,%edx
   0x0000000000400566 <+57>:    mov    0x18(%rbp),%eax
   0x0000000000400569 <+60>:    add    %eax,%edx
   0x000000000040056b <+62>:    mov    0x20(%rbp),%eax
   0x000000000040056e <+65>:    add    %eax,%edx
   0x0000000000400570 <+67>:    mov    0x28(%rbp),%eax
   0x0000000000400573 <+70>:    add    %edx,%eax
   0x0000000000400575 <+72>:    mov    %eax,-0x4(%rbp)
   0x0000000000400578 <+75>:    mov    -0x4(%rbp),%eax
   0x000000000040057b <+78>:    pop    %rbp
   0x000000000040057c <+79>:    retq   
End of assembler dump.
(gdb) disassemble main
Dump of assembler code for function main:
   0x000000000040057d <+0>:     push   %rbp
   0x000000000040057e <+1>:     mov    %rsp,%rbp
   0x0000000000400581 <+4>:     sub    $0x60,%rsp
   0x0000000000400585 <+8>:     mov    %edi,-0x34(%rbp)
   0x0000000000400588 <+11>:    mov    %rsi,-0x40(%rbp)
   0x000000000040058c <+15>:    movl   $0x1,-0x4(%rbp)
   0x0000000000400593 <+22>:    movl   $0x2,-0x8(%rbp)
   0x000000000040059a <+29>:    movl   $0x3,-0xc(%rbp)
   0x00000000004005a1 <+36>:    movl   $0x4,-0x10(%rbp)
   0x00000000004005a8 <+43>:    movl   $0x5,-0x14(%rbp)
   0x00000000004005af <+50>:    movl   $0x6,-0x18(%rbp)
   0x00000000004005b6 <+57>:    movl   $0x7,-0x1c(%rbp)
   0x00000000004005bd <+64>:    movl   $0x8,-0x20(%rbp)
   0x00000000004005c4 <+71>:    movl   $0x9,-0x24(%rbp)
   0x00000000004005cb <+78>:    movl   $0xa,-0x28(%rbp)
   0x00000000004005d2 <+85>:    mov    -0x18(%rbp),%r9d
   0x00000000004005d6 <+89>:    mov    -0x14(%rbp),%r8d
   0x00000000004005da <+93>:    mov    -0x10(%rbp),%ecx
   0x00000000004005dd <+96>:    mov    -0xc(%rbp),%edx
   0x00000000004005e0 <+99>:    mov    -0x8(%rbp),%esi
   0x00000000004005e3 <+102>:   mov    -0x4(%rbp),%eax
   0x00000000004005e6 <+105>:   mov    -0x28(%rbp),%edi
   0x00000000004005e9 <+108>:   mov    %edi,0x18(%rsp)
   0x00000000004005ed <+112>:   mov    -0x24(%rbp),%edi
   0x00000000004005f0 <+115>:   mov    %edi,0x10(%rsp)
   0x00000000004005f4 <+119>:   mov    -0x20(%rbp),%edi
   0x00000000004005f7 <+122>:   mov    %edi,0x8(%rsp)
   0x00000000004005fb <+126>:   mov    -0x1c(%rbp),%edi
   0x00000000004005fe <+129>:   mov    %edi,(%rsp)
   0x0000000000400601 <+132>:   mov    %eax,%edi
   0x0000000000400603 <+134>:   callq  0x40052d <sum>
   0x0000000000400608 <+139>:   mov    %eax,-0x2c(%rbp)
   0x000000000040060b <+142>:   mov    -0x2c(%rbp),%eax
   0x000000000040060e <+145>:   mov    %eax,%esi
   0x0000000000400610 <+147>:   mov    $0x4006c0,%edi
   0x0000000000400615 <+152>:   mov    $0x0,%eax
   0x000000000040061a <+157>:   callq  0x400410 <printf@plt>
   0x000000000040061f <+162>:   mov    $0x0,%eax
   0x0000000000400624 <+167>:   leaveq 
   0x0000000000400625 <+168>:   retq   
End of assembler dump.
(gdb) 
{% endhighlight %}

上面我们可以看到有一部分参数通过寄存器来进行传递，有一部分通过栈来进行传递。我们来看一下在调用栈：

![gdb-stack-change](https://ivanzz1001.github.io/records/assets/img/cplusplus/gdb_stack_change.jpg)

这里注意， ```leaveq```会将当前栈清空，而```retq```会恢复rip寄存器。

### 2.2 在函数的第一条汇编指令打断点

1) 示例程序
{% highlight string %}
#include <stdio.h>

int global_var;

void change_var()
{
        global_var = 10;
}

int main(int argc, char *argv[])
{
        change_var();

        return 0x0;
}
{% endhighlight %}


2) 调试技巧

通常给函数打断点的命令: ```b func```，不会把断点设置在汇编指令层次函数的开头。例如：
{% highlight string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b main
Breakpoint 1 at 0x40050c: file test.c, line 12.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:12
12              change_var();
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) disassemble
Dump of assembler code for function main:
   0x00000000004004fd <+0>:     push   %rbp
   0x00000000004004fe <+1>:     mov    %rsp,%rbp
   0x0000000000400501 <+4>:     sub    $0x10,%rsp
   0x0000000000400505 <+8>:     mov    %edi,-0x4(%rbp)
   0x0000000000400508 <+11>:    mov    %rsi,-0x10(%rbp)
=> 0x000000000040050c <+15>:    mov    $0x0,%eax
   0x0000000000400511 <+20>:    callq  0x4004ed <change_var>
   0x0000000000400516 <+25>:    mov    $0x0,%eax
   0x000000000040051b <+30>:    leaveq 
   0x000000000040051c <+31>:    retq   
End of assembler dump
{% endhighlight %}

可以看到，程序停在了第六条汇编指令处（箭头所指位置）。如果要把断点设置在汇编指令层次函数的开头，要使用如下命令```b *func```，例如：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b *main
Breakpoint 1 at 0x4004fd: file test.c, line 11.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, main (argc=0, argv=0x7fffffffe630) at test.c:11
11      {
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) disassemble
Dump of assembler code for function main:
=> 0x00000000004004fd <+0>:     push   %rbp
   0x00000000004004fe <+1>:     mov    %rsp,%rbp
   0x0000000000400501 <+4>:     sub    $0x10,%rsp
   0x0000000000400505 <+8>:     mov    %edi,-0x4(%rbp)
   0x0000000000400508 <+11>:    mov    %rsi,-0x10(%rbp)
   0x000000000040050c <+15>:    mov    $0x0,%eax
   0x0000000000400511 <+20>:    callq  0x4004ed <change_var>
   0x0000000000400516 <+25>:    mov    $0x0,%eax
   0x000000000040051b <+30>:    leaveq 
   0x000000000040051c <+31>:    retq   
End of assembler dump.
{% endhighlight %}

可以看到，程序停在了第一条汇编指令处（箭头所指位置）。


### 2.3 自动反汇编后面要执行的代码

1） 示例程序
{% highlight string %}
#include <stdio.h>



int sum(int a, int b)
{
        return a + b;
}


int main(int argc,char *argv[])
{
        int a, b, total;

        a = 1;
        b = 2;

        total = sum(a, b);

        printf("total: %d\n", total);
        return 0x0;
}
{% endhighlight %}

2) 反汇编演示
{% highlight string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) set disassemble-next-line on
(gdb) start
Temporary breakpoint 1 at 0x400550: file test.c, line 15.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:15
15              a = 1;
=> 0x0000000000400550 <main+15>:        c7 45 fc 01 00 00 00    movl   $0x1,-0x4(%rbp)
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) si
16              b = 2;
=> 0x0000000000400557 <main+22>:        c7 45 f8 02 00 00 00    movl   $0x2,-0x8(%rbp)
(gdb) si
18              total = sum(a, b);
=> 0x000000000040055e <main+29>:        8b 55 f8        mov    -0x8(%rbp),%edx
   0x0000000000400561 <main+32>:        8b 45 fc        mov    -0x4(%rbp),%eax
   0x0000000000400564 <main+35>:        89 d6   mov    %edx,%esi
   0x0000000000400566 <main+37>:        89 c7   mov    %eax,%edi
   0x0000000000400568 <main+39>:        e8 c0 ff ff ff  callq  0x40052d <sum>
   0x000000000040056d <main+44>:        89 45 f4        mov    %eax,-0xc(%rbp)
{% endhighlight %}
上面可以看到，对于相应的指令已经反汇编出来了。

3) 调试技巧

如果要在任意情况下反汇编后面要执行的代码，则
<pre>
(gdb) set disassemble-next-line on
</pre>

如果要在后面的代码没有源码的情况下才反汇编后面要执行的代码，则
<pre>
(gdb) set disassemble-next-line auto
</pre>
关闭这个功能：
<pre>
(gdb) set disassemble-next-line off
</pre>


### 2.4 将源程序和汇编指令映射起来

1) 示例程序
{% highlight string %}
#include <stdio.h>

typedef struct{
        int a;
        int b;
        int c;
        int d;
}ex_st;

int main(int argc, char *argv[])
{
        ex_st st = {1, 2, 3, 4};

        printf("%d,%d,%d,%d\n", st.a, st.b, st.c, st.d);

        return 0;
}
{% endhighlight %}

2) 技巧1

可以用```disassemble /m func```命令将函数代码和汇编指令映射起来，以上面代码为例：
{% highlight string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) disassemble /m main
Dump of assembler code for function main:
11      {
   0x000000000040052d <+0>:     push   %rbp
   0x000000000040052e <+1>:     mov    %rsp,%rbp
   0x0000000000400531 <+4>:     sub    $0x20,%rsp
   0x0000000000400535 <+8>:     mov    %edi,-0x14(%rbp)
   0x0000000000400538 <+11>:    mov    %rsi,-0x20(%rbp)

12              ex_st st = {1, 2, 3, 4};
   0x000000000040053c <+15>:    movl   $0x1,-0x10(%rbp)
   0x0000000000400543 <+22>:    movl   $0x2,-0xc(%rbp)
   0x000000000040054a <+29>:    movl   $0x3,-0x8(%rbp)
   0x0000000000400551 <+36>:    movl   $0x4,-0x4(%rbp)

13
14              printf("%d,%d,%d,%d\n", st.a, st.b, st.c, st.d);
   0x0000000000400558 <+43>:    mov    -0x4(%rbp),%esi
   0x000000000040055b <+46>:    mov    -0x8(%rbp),%ecx
   0x000000000040055e <+49>:    mov    -0xc(%rbp),%edx
   0x0000000000400561 <+52>:    mov    -0x10(%rbp),%eax
   0x0000000000400564 <+55>:    mov    %esi,%r8d
   0x0000000000400567 <+58>:    mov    %eax,%esi
   0x0000000000400569 <+60>:    mov    $0x400610,%edi
   0x000000000040056e <+65>:    mov    $0x0,%eax
   0x0000000000400573 <+70>:    callq  0x400410 <printf@plt>

15
16              return 0;
   0x0000000000400578 <+75>:    mov    $0x0,%eax

17      }
   0x000000000040057d <+80>:    leaveq 
   0x000000000040057e <+81>:    retq   

End of assembler dump.
(gdb)
{% endhighlight %}

可以看到，每一条C语句下面时对应的汇编代码。

3） 技巧2

如果只想查看某一行对应的地址范围，可以：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) list
3       typedef struct{
4               int a;
5               int b;
6               int c;
7               int d;
8       }ex_st;
9
10      int main(int argc, char *argv[])
11      {
12              ex_st st = {1, 2, 3, 4};
(gdb) 
13
14              printf("%d,%d,%d,%d\n", st.a, st.b, st.c, st.d);
15
16              return 0;
17      }
(gdb) info line 14
Line 14 of "test.c" starts at address 0x400558 <main+43> and ends at 0x400578 <main+75>.
{% endhighlight %}

如果只想查看这一条语句对应的汇编代码，可以使用```disassemble [start], [end]```命令：
{% highlight string %}
(gdb) disassemble 0x400558, 0x400578
Dump of assembler code from 0x400558 to 0x400578:
   0x0000000000400558 <main+43>:        mov    -0x4(%rbp),%esi
   0x000000000040055b <main+46>:        mov    -0x8(%rbp),%ecx
   0x000000000040055e <main+49>:        mov    -0xc(%rbp),%edx
   0x0000000000400561 <main+52>:        mov    -0x10(%rbp),%eax
   0x0000000000400564 <main+55>:        mov    %esi,%r8d
   0x0000000000400567 <main+58>:        mov    %eax,%esi
   0x0000000000400569 <main+60>:        mov    $0x400610,%edi
   0x000000000040056e <main+65>:        mov    $0x0,%eax
   0x0000000000400573 <main+70>:        callq  0x400410 <printf@plt>
End of assembler dump.
{% endhighlight %}


### 2.5 显示要执行的汇编指令

1) 示例程序
{% highlight string %}
#include <stdio.h>

int global_var;

void change_var()
{
    global_var=100;
}

int main(int argc, char *argv[])
{
    change_var();

    return 0;
}
{% endhighlight %}

2） 调试技巧

使用GDB调试汇编程序时，可以用```display /i $pc```命令显示当程序停止时，将要执行的汇编指令。以上面程序为例：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40050c: file test.c, line 12.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:12
12          change_var();
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) display /i $pc
1: x/i $pc
=> 0x40050c <main+15>:  mov    $0x0,%eax
(gdb) si
0x0000000000400511      12          change_var();
1: x/i $pc
=> 0x400511 <main+20>:  callq  0x4004ed <change_var>
(gdb) si
change_var () at test.c:6
6       {
1: x/i $pc
=> 0x4004ed <change_var>:       push   %rbp
{% endhighlight %}

可以看到，打印出了将要执行的汇编指令。此外，也可以一次显示多条指令：
{% highlight string %}
(gdb) display /3i $pc
2: x/3i $pc
=> 0x4004ed <change_var>:       push   %rbp
   0x4004ee <change_var+1>:     mov    %rsp,%rbp
   0x4004f1 <change_var+4>:     movl   $0x64,0x200b35(%rip)        # 0x601030 <global_var>
{% endhighlight %}
可以看到，一次显示了3条指令。

取消显示可以使用```undisplay```命令。


### 2.6 打印寄存器的值

用GDB调试程序时，如果想查看寄存器的值，可以使用```info registers```命令，例如：
{% highlight string %}
(gdb) info registers
rax            0x4004fd 4195581
rbx            0x0      0
rcx            0x400520 4195616
rdx            0x7fffffffe648   140737488348744
rsi            0x7fffffffe638   140737488348728
rdi            0x1      1
rbp            0x7fffffffe550   0x7fffffffe550
rsp            0x7fffffffe540   0x7fffffffe540
r8             0x7ffff7dd7e80   140737351876224
r9             0x0      0
r10            0x7fffffffe3a0   140737488348064
r11            0x7ffff7a3da40   140737348098624
r12            0x400400 4195328
r13            0x7fffffffe630   140737488348720
r14            0x0      0
r15            0x0      0
rip            0x40050c 0x40050c <main+15>
eflags         0x202    [ IF ]
cs             0x33     51
ss             0x2b     43
ds             0x0      0
es             0x0      0
fs             0x0      0
gs             0x0      0
{% endhighlight %}

以上输出不包括浮点寄存器和向量寄存器的内容。使用```info all-registers```命令，可以输出所有寄存器的内容：
<pre>
(gdb) info all-registers
</pre>
要打印单个寄存器的值，可以使用```info registers regname```或者```p $regname```，例如：
{% highlight string %}
(gdb) info registers rax
rax            0x4004fd 4195581
(gdb) p $rax
$1 = 4195581
{% endhighlight %}

### 2.7 显示程序原始机器码

1) 示例程序
{% highlight string %}
#include <stdio.h>


int main(int argc, char *argv[])
{
        printf("hello,world!\n");

        return 0x0;
}
{% endhighlight %}

2) 调试技巧

使用```disassemble /r```命令可以用16进制形式显示程序的原始机器码。以上面程序为例：
{% highlight string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) disassemble /r main
Dump of assembler code for function main:
   0x000000000040052d <+0>:     55      push   %rbp
   0x000000000040052e <+1>:     48 89 e5        mov    %rsp,%rbp
   0x0000000000400531 <+4>:     48 83 ec 10     sub    $0x10,%rsp
   0x0000000000400535 <+8>:     89 7d fc        mov    %edi,-0x4(%rbp)
   0x0000000000400538 <+11>:    48 89 75 f0     mov    %rsi,-0x10(%rbp)
   0x000000000040053c <+15>:    bf e0 05 40 00  mov    $0x4005e0,%edi
   0x0000000000400541 <+20>:    e8 ca fe ff ff  callq  0x400410 <puts@plt>
   0x0000000000400546 <+25>:    b8 00 00 00 00  mov    $0x0,%eax
   0x000000000040054b <+30>:    c9      leaveq 
   0x000000000040054c <+31>:    c3      retq   
End of assembler dump.
(gdb) 
{% endhighlight %}
>注： 这里汇编语言中,rsp指向的是当前的栈顶元素的位置，而不是下一个待插入元素的位置。因此在执行push指令时，首先会减少rsp的值，然后再将数据复制到rsp指向的内存中。



<br />
<br />

**[参看]**


1. [100个gdb小技巧](https://www.kancloud.cn/wizardforcel/gdb-tips-100/146771)

2. [设置 GDB 代码搜索路径](https://blog.csdn.net/caspiansea/article/details/42447203)

3. [AT&T汇编格式与Intel汇编格式的比较](https://blog.csdn.net/samxx8/article/details/12613643)

4. [函数调用过程探究](http://www.cnblogs.com/bangerlee/archive/2012/05/22/2508772.html)

<br />
<br />
<br />





