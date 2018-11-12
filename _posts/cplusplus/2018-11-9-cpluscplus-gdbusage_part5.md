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
<pre>
000000000040052d <func>:
  40052d:       55                      push   %rbp
  40052e:       48 89 e5                mov    %rsp,%rbp
  400531:       c7 45 fc 02 00 00 00    movl   $0x2,-0x4(%rbp)
  400538:       8b 45 fc                mov    -0x4(%rbp),%eax
  40053b:       5d                      pop    %rbp
  40053c:       c3                      retq 
</pre>

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





