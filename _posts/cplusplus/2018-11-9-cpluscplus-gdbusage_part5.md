---
layout: post
title: GDB调试： 改变程序的执行
tags:
- cplusplus
categories: cplusplus
description: GDB的使用
---

本文档主要参看<<Debugging with GDB>> ```Tenth Edition, for gdb version 8.0.1```。

本文我们主要会介绍如何使用GDB更改程序的执行，另外还会简要介绍GDB中源文件的搜索。




<!-- more -->


## 1. 改变程序的执行


### 1.1  改变字符串的值

1) 示例程序
{% highlight string %}
#include <stdio.h>

int main(int argc, char *argv[])
{
        char p1[] = "Sam";
        char *p2 = "Bob";

        printf("p1 is %s, p2 is %s\n", p1, p2);
        return 0;
}
{% endhighlight %}

2) 技巧

使用GDB调试程序时，可以使用```set```命令改变字符串的值，以上面程序为例：
{% highlight string %}
# gcc -g -c test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40053c: file test.c, line 5.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:5
5               char p1[] = "Sam";
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
6               char *p2 = "Bob";
(gdb) n
8               printf("p1 is %s, p2 is %s\n", p1, p2);
(gdb) set main::p1="ivan1001"
Too many array elements
(gdb) set main::p1="Jil"
(gdb) set main::p2="Bill"
(gdb) n
p1 is Jil, p2 is Bill
9               return 0;
{% endhighlight %}

可以看到```p1```和```p2```字符串都发生了变化，也可以通过访问内存地址的方法来改变字符串的值：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40053c: file test.c, line 5.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:5
5               char p1[] = "Sam";
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
6               char *p2 = "Bob";
(gdb) n
8               printf("p1 is %s, p2 is %s\n", p1, p2);
(gdb) p p1
$1 = "Sam"
(gdb) p &p1
$2 = (char (*)[4]) 0x7fffffffe540
(gdb) set p1[0]='C'
(gdb) p p1
$3 = "Cam"
(gdb) set {char [4]} 0x7fffffffe540 = "Ace"
(gdb) n
p1 is Ace, p2 is Bob
9               return 0;
{% endhighlight %}
在改变字符串的值时候，一定要注意内存越界的问题。

### 1.2 设置变量的值

1) 示例程序
{% highlight string %}
#include <stdio.h>

int func()
{
	int i = 2;
	
	return i;
}

int main(int argc, char *argv[])
{
	int a = 0;

	a = func();
	printf("%d\n", a);

	return 0;
}
{% endhighlight %}

2) 调试技巧

在GDB中，可以用```set var variable=expr```命令设置变量的值，以上面代码为例：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b func
Breakpoint 1 at 0x400531: file test.c, line 5.
(gdb) b test.c:15
Breakpoint 2 at 0x400560: file test.c, line 15.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, func () at test.c:5
5               int i = 2;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) set var i=8
(gdb) p i
$1 = 8
(gdb) c
Continuing.

Breakpoint 2, main (argc=1, argv=0x7fffffffe638) at test.c:15
15              printf("%d\n", a);
(gdb) n
2
17              return 0;
(gdb)
{% endhighlight %}

上面我们可以看到在func()函数里用set命令把```i```的值修改成了8。然而后面的返回值却仍为2，这主要是由于在产生汇编代码时，直接将2作为立即数压入了栈：
{% highlight string %}
000000000040052d <func>:
  40052d:       55                      push   %rbp
  40052e:       48 89 e5                mov    %rsp,%rbp
  400531:       c7 45 fc 02 00 00 00    movl   $0x2,-0x4(%rbp)
  400538:       8b 45 fc                mov    -0x4(%rbp),%eax
  40053b:       5d                      pop    %rbp
  40053c:       c3                      retq 
{% endhighlight %}

我们可以在稍后一点的位置修改```i```的值即可正确返回```8```。

<br />

也可以用```set {type}address=expr```的方式，含义是给存储地址在address，变量类型为type的变量赋值，仍以上面代码为例：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b func
Breakpoint 1 at 0x400531: file test.c, line 5.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, func () at test.c:5
5               int i = 2;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
7               return i;
(gdb) p &i
$1 = (int *) 0x7fffffffe51c
(gdb) set {int}0x7fffffffe51c=8
(gdb) p i
$2 = 8
(gdb) c
Continuing.
8
[Inferior 1 (process 82849) exited normally]
{% endhighlight %}

可以看到```i```的值被修改成为8

<br />

另外寄存器也可以作为变量，因此同样可以修改寄存器的值：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b func
Breakpoint 1 at 0x400531: file test.c, line 5.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, func () at test.c:5
5               int i = 2;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
7               return i;
(gdb) si
8       }
(gdb) disassemble func
Dump of assembler code for function func:
   0x000000000040052d <+0>:     push   %rbp
   0x000000000040052e <+1>:     mov    %rsp,%rbp
   0x0000000000400531 <+4>:     movl   $0x2,-0x4(%rbp)
   0x0000000000400538 <+11>:    mov    -0x4(%rbp),%eax
=> 0x000000000040053b <+14>:    pop    %rbp
   0x000000000040053c <+15>:    retq   
End of assembler dump.
(gdb) set var $eax=100
(gdb) c
Continuing.
100
[Inferior 1 (process 82909) exited normally]
{% endhighlight %}

可以看到，因为eax寄存器存储着函数的返回值，所以当把eax寄存器的值改为100后，函数的返回值也变成了100.

### 1.3 修改PC寄存器的值

1) 示例程序
{% highlight string %}
#include <stdio.h>

int main(int argc, char *argv[])
{
	int a = 0;
	
	a++;
	a++;
	
	printf("a: %d\n", a);
	return 0x0;
}
{% endhighlight %}

2) 调试技巧

PC寄存器会存储程序下一条要执行的指令，通过修改这个寄存器的值，可以达到改变程序执行流程的母的。上面的程序会输出```a=2```，下面介绍一下如何通过修改PC寄存器的值，改变程序执行流程：

{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40053c: file test.c, line 5.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:5
5               int a = 0;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
7               a++;
(gdb) info line 7
Line 7 of "test.c" starts at address 0x400543 <main+22> and ends at 0x400547 <main+26>.
(gdb) info line 8
Line 8 of "test.c" starts at address 0x400547 <main+26> and ends at 0x40054b <main+30>.
(gdb) info registers
rax            0x40052d 4195629
rbx            0x0      0
rcx            0x400570 4195696
rdx            0x7fffffffe648   140737488348744
rsi            0x7fffffffe638   140737488348728
rdi            0x1      1
rbp            0x7fffffffe550   0x7fffffffe550
rsp            0x7fffffffe530   0x7fffffffe530
r8             0x7ffff7dd7e80   140737351876224
r9             0x0      0
r10            0x7fffffffe3a0   140737488348064
r11            0x7ffff7a3da40   140737348098624
r12            0x400440 4195392
r13            0x7fffffffe630   140737488348720
r14            0x0      0
r15            0x0      0
rip            0x400543 0x400543 <main+22>
eflags         0x206    [ PF IF ]
cs             0x33     51
ss             0x2b     43
ds             0x0      0
es             0x0      0
fs             0x0      0
gs             0x0      0
(gdb) p $pc
$1 = (void (*)()) 0x400543 <main+22>
{% endhighlight %}

通过```info line 7```和```info line 8```命令我们可以知道两条```a++;```语句的汇编指令的起始地址分别为```0x400543```和```0x400547```。

如下我们跳过第7行的```a++;```命令：
{% highlight string %}
(gdb) set var $pc=0x400547
(gdb) n
10              printf("a: %d\n", a);
(gdb) n
a: 1
11              return 0x0;
(gdb) 
{% endhighlight %}

上面可以看到程序输出```a=1```，也就是跳过了第一条```a++;```语句。


### 1.4 跳转到指定位置执行

1) 示例程序
{% highlight string %}
#include <stdio.h>


int func(int x)
{
	if(x < 0)
		puts("error")；
}

int main(int argc, char *argv[])
{
	int i = 1;
	
	func(i--);
	func(i--);
	func(i--);
	
	return 0x0;
}
{% endhighlight %}

2) 调试技巧

当调试程序时，你可能不小心走错了地方：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x400559: file test.c, line 12.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:12
12              int i = 1;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
14              func(i--);
(gdb) n
15              func(i--);
(gdb) n
16              func(i--);
(gdb) n
error
18              return 0x0;
(gdb) 
{% endhighlight %}

看起来在第16行，调用```func()```函数的时候出错了。常见的办法是在16行设置一个断点，然后从头```run```一次。如果你当前的环境支持反向执行，那么就更好了； 但如果不支持，你也可以直接```jump```到16行再执行一次：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x400559: file test.c, line 12.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:12
12              int i = 1;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
14              func(i--);
(gdb) n
15              func(i--);
(gdb) n
16              func(i--);
(gdb) n
error
18              return 0x0;
(gdb) b test.c:16
Breakpoint 2 at 0x400580: file test.c, line 16.
(gdb) jump 16
Continuing at 0x400580.

Breakpoint 2, main (argc=1, argv=0x7fffffffe638) at test.c:16
16              func(i--);
(gdb) s
func (x=-2) at test.c:6
6               if(x < 0)
(gdb) p x
$1 = -2
(gdb) n
7                       puts("error");
{% endhighlight %}
需要注意的是：jump命令只改变pc的值，所以改变程序执行可能会出现不同的结果，比如变量```i```的值。通过（临时）断点的配合，可以让你的程序跳到指定的位置，并停下来。

### 1.5 使用断点命令改变程序的执行

1) 示例程序
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

void drawing(int n)
{
	if(n != 0)
		puts("Try again?\nAll you need is a dollar, and a dream");
	else
		puts("You win $3000!");
}

int main(int argc, char *argv[])
{
	int n;
	
	srand(time(NULL));
	
	n = rand() % 10;
	
	printf("Your number is %d\n", n);
	drawing(n);
	
	return 0x0;
}
{% endhighlight %}

2) 调试技巧

这个例子程序可能不太好，只是可以用来演示下断点命令的用法：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b drawing
Breakpoint 1 at 0x400658: file test.c, line 6.
(gdb) command 1
Type commands for breakpoint(s) 1, one per line.
End with a line saying just "end".
>silent
>set var n=0
>continue
>end
(gdb) r
Starting program: /root/workspace/./test 
Your number is 1
You win $3000!
[Inferior 1 (process 89237) exited normally]
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) 
{% endhighlight %}
可以看到，当程序运行到断点处，会自动把变量n的值修改为0，然后继续执行。

如果你在调试一个大程序，重新编译一次会花费很长时间，比如调试编译器的bug，那么你可以用这种方式在GDB中先实验性的修改下试试，而不需要修改源码，重新编译。


### 1.6 修改被调试程序的二进制文件

1) 示例程序
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

void drawing(int n)
{
	if(n != 0)
		puts("Try again?\nAll you need is a dollar, and a dream");
	else
		puts("You win $3000!")
}

int main(int argc, char *argv[])
{
	int n;
	
	srand(time(NULL));
	
	n = rand() % 10;
	
	printf("Your number is %d\n", n);
	drawing(n);
	
	return 0x0;
}
{% endhighlight %}

2） 调试技巧

GDB不仅可以用来调试程序，还可以修改程序的二进制代码。缺省情况下，GDB是以只读的方式加载程序。但是可以通过命令行选项指定为可写：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb --write -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) show write
Writing into executable and core files is on.
(gdb)
{% endhighlight %}

也可以在GDB中使用命令设置并重新加载程序：
{% highlight string %}
# gdb -q
(gdb) set write on
(gdb) file ./test
Reading symbols from /root/workspace/test...done.
{% endhighlight %}

接下来查看反汇编：
{% highlight string %}
(gdb) disassemble /mr drawing
Dump of assembler code for function drawing:
5       {
   0x000000000040064d <+0>:     55      push   %rbp
   0x000000000040064e <+1>:     48 89 e5        mov    %rsp,%rbp
   0x0000000000400651 <+4>:     48 83 ec 10     sub    $0x10,%rsp
   0x0000000000400655 <+8>:     89 7d fc        mov    %edi,-0x4(%rbp)

6               if(n != 0)
   0x0000000000400658 <+11>:    83 7d fc 00     cmpl   $0x0,-0x4(%rbp)
   0x000000000040065c <+15>:    74 0c   je     0x40066a <drawing+29>

7                       puts("Try again?\nAll you need is a dollar, and a dream");
   0x000000000040065e <+17>:    bf 90 07 40 00  mov    $0x400790,%edi
   0x0000000000400663 <+22>:    e8 88 fe ff ff  callq  0x4004f0 <puts@plt>
   0x0000000000400668 <+27>:    eb 0a   jmp    0x400674 <drawing+39>

8               else
9                       puts("You win $3000!");
   0x000000000040066a <+29>:    bf c1 07 40 00  mov    $0x4007c1,%edi
   0x000000000040066f <+34>:    e8 7c fe ff ff  callq  0x4004f0 <puts@plt>

10      }
   0x0000000000400674 <+39>:    c9      leaveq 
   0x0000000000400675 <+40>:    c3      retq   

End of assembler dump.
{% endhighlight %}

修改二进制代码（注意大小端和指令长度）：
{% highlight string %}
(gdb) set var *(short *)0x000000000040065c=0x0ceb
(gdb) disassemble /mr drawing
Dump of assembler code for function drawing:
5       {
   0x000000000040064d <+0>:     55      push   %rbp
   0x000000000040064e <+1>:     48 89 e5        mov    %rsp,%rbp
   0x0000000000400651 <+4>:     48 83 ec 10     sub    $0x10,%rsp
   0x0000000000400655 <+8>:     89 7d fc        mov    %edi,-0x4(%rbp)

6               if(n != 0)
   0x0000000000400658 <+11>:    83 7d fc 00     cmpl   $0x0,-0x4(%rbp)
   0x000000000040065c <+15>:    eb 0c   jmp    0x40066a <drawing+29>

7                       puts("Try again?\nAll you need is a dollar, and a dream");
   0x000000000040065e <+17>:    bf 90 07 40 00  mov    $0x400790,%edi
   0x0000000000400663 <+22>:    e8 88 fe ff ff  callq  0x4004f0 <puts@plt>
   0x0000000000400668 <+27>:    eb 0a   jmp    0x400674 <drawing+39>

8               else
9                       puts("You win $3000!");
   0x000000000040066a <+29>:    bf c1 07 40 00  mov    $0x4007c1,%edi
   0x000000000040066f <+34>:    e8 7c fe ff ff  callq  0x4004f0 <puts@plt>

10      }
   0x0000000000400674 <+39>:    c9      leaveq 
   0x0000000000400675 <+40>:    c3      retq   

End of assembler dump.
(gdb)
{% endhighlight %}
可以看到，条件跳转指令```je```已经被改为无条件跳转```jmp```了。

我们退出，然后再运行：
<pre>
# ./test
Your number is 4
You win $3000!
</pre>


## 2. 源文件查找
可执行程序有时候并不会记录它们是用哪一个目录的源文件来进行编译的，它们通常只记住一个文件名。即使在编译时会记住相应的目录，但在编译后调试程序之前也可能会将目录移动。GDB有一系列的目录以用于查找源文件；这被称为```source path```。任何时候，GDB需要一个源文件时，其都会尝试```source path```列表中的所有目录。注意这里是按照列表中的目录出现的先后顺序来进行查找，直到找到所期望的文件名。

例如，假设一个可执行文件引用了文件```/usr/src/foo-1.0/lib/foo.c```，我们当前的source path是```/mnt/cross```。则GDB首先会按字面量(**/usr/src/foo-1.0/lib/foo.c**)来进行查找；假如查找失败，则会尝试**'/mnt/cross/usr/src/foo-1.0/lib/foo.c'**来进行查找；假如仍失败，在会尝试 **'/mnt/cross/foo.c'**；假如还是失败的话，则会打印相应的错误消息。GDB并不会查找源文件名称的某一个部分，例如不会查找 **'/mnt/cross/src/foo-1.0/lib/foo.c'**。相似的，也不会查找source path下的子目录： 假如source path是 **'/mnt/cross'**，可执行文件引用的是```foo.c```，那么GDB不会查找其是否在 **'/mnt/cross/usr/src/foo-1.0/lib**目录下。

对于```Plain file names```，相对目录前缀的文件名等都会按上面描述的方式来进行处理。例如，假如source path是 **'/mnt/cross'**，并且源文件名被记录为 **'../lib/foo.c'**，那么GDB首先会查找**'../lib/foo.c'**，接着查找 **'/mnt/cross/../lib/foo.c'**，再接着查找 **'/mnt/cross/foo.c'**。

值得注意的是，可执行文件的查找路径并不会被用于定位源文件。

无论什么时候```reset```或者```rearrange``` source path，GDB都会清除其所缓存的任何信息： 源文件是在什么目录查找的、里面的某个信息在源文件的哪一行。

当你启动GDB的时候，其source path只包括```cdir```和```cwd```,并按前后顺序排列。要添加其他的目录，我们必须使用```directory```命令。关于```cdir```与```cwd```，我们这里简单说明一下：
<pre>
$cdir : compilation directory
$cwd : current working directory
</pre>


另外，对于```source path```，GDB提供了一系列的命令来管理众多的source path替换规则(substitution rule)。当源文件在编译之后进行了移动，此时```替换规则```指明了如何重写(rewrite)存储在程序调试信息中的源文件目录。一条替换规则由两个字符串组成，其中第一个字符串指明需要重写的路径，第二个字符串指明其应该如何被重写，这里我们分别将这两个字符串命名为```from```和```to```。GDB会简单的使用```to```来替换```from```目录的开始部分，然后使用相应的结果来查找源文件。

拿前一个例子来说，我们假设```foo-1.0```目录树已经从```/usr/src```移动到了```/mnt/cross```，那么你可以告诉GDB在所有路径名称当中使用```/mnt/cross```来替换```/usr/src```。因此，这里GDB首先就会从 **'/mnt/cross/foo-1.0/lib/foo.c'**查找，而不是从原路径 **'/usr/src/foo-1.0/lib/foo.c'**来进行查找。要定义一个源路径替换规则，可以使用我们后面介绍的**set substitute-path**命令。
<pre>
注： 这里为了避免产生一些异常的替换情况，一般只限制在完全匹配的前缀替换。
</pre>

在很多情况下，你可以使用```directory```命令来获得一样的效果。然而```set substitute-path```在处理复杂的目录结构时会更加方便。通过使用```directory```命令，你需要添加工程中每一个子目录。假如你在保持内部目录不变的情况下将整个目录树移动到别的地方，那么```set substitute-path```一条命令就可以告诉GDB所有的资源新路径。

### 2.1 相关命令介绍

1) **directory dirname ... / dir dirname ...**

添加目录```dirname```到source path的最前面。本命令可以同时指定多个目录，目录之间以```:```分割（Windows上一般是以```;```分割）或者以空格分割。你也可以指定一个当前已存在于```source path```的目录，这样就会使得该目录会前移，从而加速GDB的搜索速度。

你可以使用```$cdir```来引用编译目录，使用```$cwd```来引用当前工作目录。注意```$cwd```是不用于```.```的，其中前者可能会在GDB Session中发生改变，而后者是在你添加一个路径到source path时的那一刻所在的路径。

2) **directory**

此命令用于将source path重置回```$cdir```和```$cwd```。

3） **set directories path-list**

将source path设置为```path-list```，假如路径中没有```$cdir```和```$cwd```的话，则也会被添加。

4) **show directories**

用于显示当前的source path。

5） **set substitute-path from to**

此命令用于定义一条source path替换规则，并且将其添加到当前已存在的规则列表的末尾。假如当前规则列表中有一条规则的```from```与当前要添加的规则的```from```相同，则该旧的规则将会被删除。

举一个例子，假如文件 **'/foo/bar/baz.c'**被移动到了 **'/mnt/cross/baz.c'**目录下，那么你可以执行如下命令：
<pre>
(gdb) set substitute-path /foo/bar /mnt/cross
</pre>

上面的命令将会用 **'/mnt/cross'**替换 **'/foo/bar'**，这样就使得GDB能够查找的移动有的```baz.c```文件。


6） **unset substitute-path [path]**

此命令用于删除删除某条替换规则。假如```[path]```未指定的话，将会删除所有替换规则。

7) **show substitute-path [path]**

用于打印某条替换规则。假如```[path]```未指定的话，将会打印所有替换规则。

<br />

另外，也可以在GDB启动时通过```-d```选项来事先指定查找目录。例如：
<pre>
# gdb `find srcdir -type d -printf '-d %p '` -q prog

# gdb -q `find ../P2P_SDK/ -type d -printf '-d %p '` ./test
Reading symbols from /root/workspace/test...done.
(gdb) show directories
Source directories searched: /root/workspace/../P2P_SDK/headers:/root/workspace/../P2P_SDK/doc:/root/workspace/../P2P_SDK/linux-example/signal_server ...
(gdb) 
</pre>

使用```info source```可以查看当前源码信息，其中也包含编译信息：
<pre>
(gdb) info source
Current source file is ../../net/tools/quic/quic_simple_client_bin.cc
Compilation directory is .
Source language is c++.
Producer is unknown.
Compiled with DWARF 2 debugging format.
Does not include preprocessor macro info.
</pre>

使用```pwd```可以看到当前目录:
<pre>
(gdb) pwd
Working directory /root/chromium/src.
</pre>

### 2.2 示例1： 设置源文件查找路径

1) 程序示例
{% highlight string %}
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
	time_t now = time(NULL);
	struct tm local = {0};
	struct tm gmt = {0};
	
	localtime_r(&now, &local);
	gmtime_r(&now, &gmt);
	
	return 0;
}
{% endhighlight %}

2) 调试技巧

有时GDB不能准确地定位到源文件的位置（比如文件被移走了，等等），此时可以用```directory```命令设置查找源文件的路径。以上面程序为例：
<pre>
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# mkdir -p src/moved
# mv ./test src/moved/
</pre>
上面我们编译之后，然后将相应的源文件移走。然后再执行如下命令进行调试:
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x4005e5: file test.c, line 6.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:6
6       test.c: No such file or directory.
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) directory ./src/moved/
Source directories searched: /root/workspace/./src/moved:$cdir:$cwd
(gdb) n
7               struct tm local = {0};
(gdb) n
8               struct tm gmt = {0};
(gdb) n
10              localtime_r(&now, &local);
(gdb) n
11              gmtime_r(&now, &gmt);
(gdb)
{% endhighlight %}

可以看到，使用```directory```(或```dir```)命令设置源文件的查找目录后，GDB就可以正常的解析源代码了。

如果希望在GDB启动时，加载code的位置，避免每次在GDB中再次输入命令，可以使用gdb的```-d```参数：
{% highlight string %}
# gdb -q `find ./ -type d printf '-d %p '` ./test
find: paths must precede expression: printf
Usage: find [-H] [-L] [-P] [-Olevel] [-D help|tree|search|stat|rates|opt|exec] [path...] [expression]
Reading symbols from /root/workspace/test...done.
(gdb) show directories
Source directories searched: $cdir:$cwd
(gdb) q
[root@bogon workspace]# gdb -q `find ./ -type d -printf '-d %p '` ./test
Reading symbols from /root/workspace/test...done.
(gdb) show directories
Source directories searched: /root/workspace/./src/moved:/root/workspace/./src:/root/workspace:$cdir:$cwd
(gdb) start
Temporary breakpoint 1 at 0x4005e5: file test.c, line 6.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:6
6               time_t now = time(NULL);
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
7               struct tm local = {0};
(gdb) 
{% endhighlight %}

### 2.3 替换查找源文件的目录
1) 示例程序
{% highlight string %}
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
	time_t now = time(NULL);
	struct tm local = {0};
	struct tm gmt = {0};
	
	localtime_r(&now, &local);
	gmtime_r(&now, &gmt);
	
	return 0;
}
{% endhighlight %}

2) 调试技巧

有时调试程序时，源代码文件可能已经移动到其他文件夹了。此时可以用```set substitute-path from to```命令设置新的文件夹to目录来替换旧的from。以上述程序为例：
<pre>
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# mkdir -p src/moved
# mv ./test src/moved/
</pre>
上面我们编译之后，然后将相应的源文件移走。然后再执行如下命令进行调试:
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x4005e5: file test.c, line 6.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:6
6       test.c: No such file or directory.
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) set substitute /root/workspace /root/workspace/src/moved
(gdb) n
7               struct tm local = {0};
(gdb) n
8               struct tm gmt = {0};
(gdb) n
10              localtime_r(&now, &local);
(gdb) 
{% endhighlight %}

可以看到，刚开始找不到相应的源文件，替换之后就能够正常找到了。



<br />
<br />

**[参看]**


1. [100个gdb小技巧](https://www.kancloud.cn/wizardforcel/gdb-tips-100/146771)

2. [设置 GDB 代码搜索路径](https://blog.csdn.net/caspiansea/article/details/42447203)



<br />
<br />
<br />





