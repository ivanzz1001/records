---
layout: post
title: GDB调试之程序的暂停与继续
tags:
- cplusplus
categories: cplusplus
description: GDB的使用
---

本文档主要参看<<Debugging with GDB>> ```Tenth Edition, for gdb version 8.0.1```。

使用调试器的主要目的是你能够在程序结束之前能够暂停程序。因此，假如程序在运行过程中出现问题，我们就可以检查并找出其中的原因。

在GDB内部，一个程序的暂停可能有多种方式产生，例如通过信号(signal)、断点(breakpoint)、通过```step```命令执行单步调试。在程序暂停之后，你可以检查并改变变量的值，设置新的断点或者移除老的断点，然后继续执行程序。通常情况下，GDB所打印的消息已经提供了充足的关于程序状态的信息，但是你也可以通过使用如下命令来显示的查看：

* **info program**: 用于显示当前调试程序的状态： 程序是否在运行、是什么样的进程、为什么会暂停
<pre>
(gdb) info program
        Using the running image of child process 68595.
Program stopped at 0x400565.
It stopped after being stepped.
</pre>



<!-- more -->


## 1. Breakpoints、Watchpoints与Catchpoints
```breakpoint```可以使得程序无论执行到哪一个点时都能够暂停。你可以使用```break```命令及其相应的变体来设置断点。此外，在有一些系统上，在运行程序前，你也可以在```共享链接库```中设置断点。


```watchpoint```是一种特殊的breakpoint，它会在某一个表达式的值发生改变的时候暂停程序。该表达式可以是一个变量的值，也可以是通过操作符连接起来的多个变量，例如'a+b'。因此，有的时候watchpoint也被称为```data breakpoint```。我们会使用一个不同的命令来设置watchpoint，但除此之外你可以像管理breakpoint那样来管理watchpoint: 启用、禁用、删除breakpoint与watchpoint都使用相同的命令。

```catchpoint```是另一种特殊的breakpoint，其会在发生某一类事件时暂停程序，比如C++抛出了一个异常或者加载一个链接库。我们会使用不同的命令来设置catchpoint，但除此之外你可以像管理breakpoint那样来管理catchpoint。


对每一个breakpoint、watchpoint、以及catchpoint当其被创建时，GDB都会为其指定一个数值。breakpoints相关的许多命令都会用到该数值。你可以启用、禁用一个breakpoint；假如禁用的话，对程序的执行不会有任何影响，直到该breakpoint被再次启用为止。


有一些GDB命令可以接受由空格分割的breakpoints列表，列表可以是一个breakpoint，比如```5```；也可以是一个范围，比如```5-7```。当为命令指定一个breakpoint列表时，则列表中的所有breakpoints都会被操作。


### 1.1 设置Breakpoints

我们可以使用```break```命令来设置breakpoints，我们可以使用```$bpnum```变量来引用当前最新设置的breakpoint。下面介绍一下设置断点的一些命令：

1) **break location**

在指定的位置设置一个breakpoint，```location```可以是一函数名（function name)、行号(line number)、或者是一个指令的地址。在开始执行breakpoint位置的指令之前，GDB会暂停程序的执行。关于```location```如何指定，请参看下面第二节。

2) **break**

当不携带任何参数时，break命令会在所选择栈帧的下一条指令处设置断点(breakpoint)。这样当程序返回到该栈帧时，执行程序马上就会被暂停。


3）**break ... if cond**

设置一个断点，并在执行到该断点处时先判定```cond```，只有条件为真，程序才会真正在该断点处暂停。这里我们给出一个简单的使用示例：
{% highlight string %}
#include <stdio.h>


int main(int argc,char *argv[])
{
        int i = 0;
        int sum = 0;

        for(i = 0; i<=200; i++)
        {
                sum += i;
        }

        printf("%d\n",sum);

        return 0x0;
}
{% endhighlight %}
采用如下命令进行编译：
<pre>
# gcc -g -c test.c
# gcc -o test test.o
</pre>

gdb可以设置条件断点，也就是只有在条件满足时，断点才会被触发。例如：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40053c: file test.c, line 6.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:6
6               int i = 0;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) list 10
5       {
6               int i = 0;
7               int sum = 0;
8
9               for(i = 0; i<=200; i++)
10              {
11                      sum += i;
12              }
13
14              printf("%d\n",sum);
(gdb) b 11 if i==101
Breakpoint 2 at 0x400553: file test.c, line 11.
(gdb) c
Continuing.

Breakpoint 2, main (argc=1, argv=0x7fffffffe638) at test.c:11
11                      sum += i;
(gdb) p sum
$1 = 5050
(gdb)
{% endhighlight %}

4) **tbreak args**

设置一个断点，但是只会在该断点出暂停一次。程序在该断点处暂停一次之后，断点就会被自动的删除。

5） **rbreak expr**

在所有匹配表达式```expr```的函数上设置断点。


6） **info breakpoints [list...] / info break [list...]**

用于打印出所有设置但并未删除的breakpoints、watchpoints和catchpoints。打印消息主要包含如下几个方面的内容：

* Breakpoint Numbers: 断点编号

* Type: 断点类型(breakpoint、watchpoint、catchpoint)

* Disposition: 当命中该checkpoint时，用于指示将该checkpoint禁止或删除；

* Enabled or Disabled: 用于指示该checkpoint当前是```enable```状态还是```disable```状态；

* Address: 断点设置在内存中的位置。对于一个挂起(pending)的断点来说，我们并不知道其位置，因此会显示为```<PENDING>```。

* What： 断点在所在的源代码文件的名称及行号。对于一个挂起的断点，假如当前相应的共享库文件并未加载，那么其将会显示设置该断点时候的命令，而等到后续对应的共享库加载之后，就会显示文件名及行号。


<br />
一个断点有可能会对应程序中多个不同的location，比如如下情景：

* 程序中有多个函数拥有相同的名称。

* 对于C++构造函数，GCC编译器会产生不同版本的实例；

* 对于C++模板函数，函数中指定的一行可以对应任意数量的实例；

* 对于inline函数，指定的source line可能会对应多个不同的位置

在所有上面这些情况下，在所有相关的位置GDB都会插入一个断点。


对于一个breakpoint，如果指向多个位置的话，那么在```info breakpoint```时会采用多行来表示。例如：
{% highlight string %}
Num Type Disp Enb Address What
1 breakpoint keep y <MULTIPLE>
stop only if i==1
breakpoint already hit 1 time
1.1 y 0x080486a2 in void foo<int>() at t.cc:8
1.2 y 0x080486ca in void foo<double>() at t.cc:8
{% endhighlight %}

对于上面例子所显示的breakpoint，每一个Location都可以单独的```enable```或者```disable```。但是却并不能单独的从列表中进行删除。

<br />

此外，在共享库中设置断点也是很常见的一种情况。当程序运行的时候，共享库可以显示的加载和卸载。为了支持这种用例，当共享库在加载或卸载时，GDB都会更新断点的位置。典型情况下，你会在程序调试会话的开始在共享库中设置一个断点，而这时共享链接库可能还没有加载或者共享库中的符号仍不可用。当你尝试设置一个断点时，GDB会询问你是否设置一个```pending breakpoint```（即那些地址仍未知的breakpoint）。

在程序运行之后，当一个新的共享链接库被加载时，GDB都会重新评估所有的breakpoints。当该新加载的动态链接库包含了相应的symbol或者某一行被pending breakpoint所引用的话，该breakpoint就算完成解析并成为一个普通的breakpoint。而当该共享链接库卸载时，所有引用其符号及源文件行的breakpoints又会变成pending状态。




### 1.3 设置catchpoint
你可以使用```catchpoint```来暂停一些情形下程序的执行，比如C++异常或者加载一个贡献链接库。我们使用```catch```命令来进行设置catchpoint:

1) **catch event**

当event发生时，就会暂停程序的执行。event可以是如下一些：

* throw [regexp] / rethrow [regexp] / catch [regexp]

* exception

* assert

* exec

* syscall

* fork

* vfork

* load [regexp] / unload [regexp]

* signal [signal... / 'all']



### 1.4 pending breakpoint 示例
如下我们给出一个pending breakpoint的示例。

1） **生成动态链接库**

头文件```get.h```:
{% highlight string %}
# cat get.h
#ifndef __GET_H_
#define __GET_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */



int get();

int set(int a);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif
{% endhighlight %}

源文件实现```get.c```:
{% highlight string %}
#include <stdio.h>
#include "get.h"



static int x = 0;

int get()
{
        printf("get x=%d\n", x);
        return x;
}

int set(int a)
{
        printf("set a=%d\n", a);
        x = a;
        return x;
}
{% endhighlight %}

编译生成动态链接库：
<pre>
# gcc get.c -shared -g -fPIC -DDEBUG -o libggg.so 
</pre>

上面我们就准备好了动态链接库。


**2） 测试源代码**

如下是我们测试用源代码：
{% highlight string %}
#include <stdio.h>
#include "get.h"


int main(int argc,char *argv[])
{
        int a = 100;
        int b = get();
        int c = set(a);
        int d = get();

        printf ("a=%d,b=%d,c=%d,d=%d\n",a,b,c,d);

        return 0;
}
{% endhighlight %}
编译：
<pre>
# gcc -g -c test.c
# gcc -o test test.o -L. -lggg
</pre>

**3) 调试技巧**
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b set
Function "set" not defined.
Make breakpoint pending on future shared library load? (y or [n]) y 
Breakpoint 2 (set) pending.
{% endhighlight %}
注意本例子有时候可能事先就加载了，并不一定能看到上面的提示。


### 1.5 catchpoint示例

1.5.1 **设置观察点**

1） 示例程序
{% highlight string %}
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
int a = 0;

void *thread1_func(void *p_arg)
{
        while (1)
        {
                a++;
                sleep(10);
        }
}

int main(int argc, char* argv[])
{
        pthread_t t1;

        pthread_create(&t1, NULL, thread1_func, "Thread 1");
		
        sleep(1000);
        return 0;
}
{% endhighlight %}


2） 调试技巧

GDB可以使用```watch```命令，也就是当一个变量值发生变化时，程序会停下来。以上面程序为例
{% highlight string %}
# gcc -c -g test.c
# gcc -o test test.o -lpthread

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) watch a
Hardware watchpoint 1: a
(gdb) start
Temporary breakpoint 2 at 0x400683: file test.c, line 19.
Starting program: /root/workspace/./test 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".

Temporary breakpoint 2, main (argc=1, argv=0x7fffffffe5e8) at test.c:19
19              pthread_create(&t1, NULL, thread1_func, "Thread 1");
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) c
Continuing.
[New Thread 0x7ffff77ff700 (LWP 12533)]
[Switching to Thread 0x7ffff77ff700 (LWP 12533)]
Hardware watchpoint 1: a

Old value = 0
New value = 1
thread1_func (p_arg=0x400740) at test.c:11
11                      sleep(10);
(gdb) c
Continuing.
Hardware watchpoint 1: a

Old value = 1
New value = 2
thread1_func (p_arg=0x400740) at test.c:11
11                      sleep(10);
(gdb) 
{% endhighlight %}

可以看到，使用```watch a```命令以后，当```a```的值变化： 由0变成1； 由1变成2，程序都会停下来。

此外，也可以使用```watch *(data type)address```这样的命令，仍以上面程序为例：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x400683: file test.c, line 19.
Starting program: /root/workspace/./test 
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe5e8) at test.c:19
19              pthread_create(&t1, NULL, thread1_func, "Thread 1");
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) p &a
$1 = (int *) 0x601040 <a>
(gdb) watch *(int *)0x601040
Hardware watchpoint 2: *(int *)0x601040
(gdb) c
Continuing.
[New Thread 0x7ffff77ff700 (LWP 12596)]
[Switching to Thread 0x7ffff77ff700 (LWP 12596)]
Hardware watchpoint 2: *(int *)0x601040

Old value = 0
New value = 1
thread1_func (p_arg=0x400740) at test.c:11
11                      sleep(10);
(gdb) c
Continuing.
Hardware watchpoint 2: *(int *)0x601040

Old value = 1
New value = 2
thread1_func (p_arg=0x400740) at test.c:11
11                      sleep(10);
(gdb) 
{% endhighlight %}

上面我们先得到```a```的地址```0x601040```，接着用```watch *(int *)0x601040```设置观察点，可以看到同```watch a```命令效果一样。观察点可以通过软件或硬件的方式实现，取决于具体的系统。但是软件实现的观察点会导致程序运行很慢，使用时需注意。

如果系统支持硬件观测的话，当设置观测点时会打印如下信息：
<pre>
Hardware watchpoint num: expr
</pre>


3） 查看观察点

我们可以使用```info watchpoints```命令来查看当前所设置的所有观察点：
<pre>
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) watch a
Hardware watchpoint 1: a
(gdb) info watchpoints
Num     Type           Disp Enb Address    What
1       hw watchpoint  keep y              a
</pre> 


1.5.2 **设置观察点只对特定线程有效**




<br />

## 2 指定断点位置

很多GDB命令都要接收一个程序代码位置的参数。因为GDB是代码级别（source level)的调试器，因此位置(location)通常是指定源代码中的一些行。location可以使用不同的格式来指定： 基于行的位置、explicit locations、address locations。

### 2.1 基于行的位置
如下是一些不同的```基于行位置```的指定方法：

* **linenum**: 指定当前源文件的行号，当前源文件可以通过如下命令查看
<pre>
(gdb) info source
Current source file is test.c
Compilation directory is /root/workspace
Located in /root/workspace/test.c
Contains 39 lines.
Source language is c.
Compiled with DWARF 2 debugging format.
Does not include preprocessor macro info.
</pre>

* **-offset / +offset**: 用于指定与当前行的偏移，可以通过```where```或```bt```命令来查看当前的行号

* **filename:linenum**: 用于指定源文件中的某一行。假如filename是一个相对路径的文件名，那么其可能匹配多个不同的文件。

* **function**: 会指定到该函数的第一行。例如，对于C语言，则是该函数的起始大括号位置。

* **function:label**: 指定function中的某一个标签

* **filename:function**: 指定filename文件中指定函数的第一行

* **label**: 指定当前栈帧中的某一个标签

### 2.2 Explicit Locations
```Explicit locations```允许用户使用```option-value```对的方式直接指定相应的位置。

假如在程序的源文件中有多个函数(functions)、labels、文件名(basename相同，例如filename.c与filename.cpp)相同，这种情况下使用```Explicit locations```就很有必要。例如，当使用```基于行的方式```指定**'foo:bar'**，那么其可以引用```foo```文件中的```bar```函数，也可以引用```foo```函数中的```bar```标签。因此，GDB必须要搜索文件系统或者符号表才可以确切的知道

如下是指定```explicit locations```的一些选项：

* **-source filename**: 用于指定源文件名称。假如有basename相同的源文件的话，那么可以加上相应的路径。例如'foo/bar/baz.c'。否则的话， GDB会使用其查找到的第一个文件。本选项一般搭配```-function```或```-line```使用。


* **-function function_name**: 用于指定一个函数的名称

* **-label label**: 用于指定一个标签值。如果未指定函数的话，则在栈帧中当前函数中搜索

* **-line number**: 指定行偏移


只要在不引起歧义的情况下，```explicit location```选项可以被简写。例如： ```break -s main.c -li 3```











<br />
<br />

**[参看]**


1. [100个gdb小技巧](https://www.kancloud.cn/wizardforcel/gdb-tips-100/146771)

2. [gdb调试动态链接库](https://blog.csdn.net/yasi_xi/article/details/18552871)

3. [设置 GDB 代码搜索路径](https://blog.csdn.net/caspiansea/article/details/42447203)


<br />
<br />
<br />





