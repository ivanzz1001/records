---
layout: post
title: GDB调试： 打印
tags:
- cplusplus
categories: cplusplus
description: GDB的使用
---

本文档主要参看<<Debugging with GDB>> ```Tenth Edition, for gdb version 8.0.1```。

本文我们主要会介绍通过GDB打印程序中的各种数据类型。




<!-- more -->


## 1. 打印ASCII字符串和宽字符串
1) 示例程序
{% highlight string %}
#include <stdio.h>
#include <wchar.h>


int main(int argc, char *argv[])
{
        char str1[] = "abcd";

        wchar_t str2[] = L"abcd";

        return 0x0;
}
{% endhighlight %}

2) 调试技巧

用GDB调试程序时，可以使用```x/s```命令打印ASCII字符串。以上面程序为例：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x4004f8: file test.c, line 7.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:7
7               char str1[] = "abcd";
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
9               wchar_t str2[] = L"abcd";
(gdb) x/s str1
0x7fffffffe540: "abcd"
(gdb) 
{% endhighlight %}
可以看到打印出了str1字符串的值。

打印宽字符字符串时，要根据宽字符的长度决定如何打印。仍以上面程序为例：
{% highlight string %}
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x4004f8: file test.c, line 7.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:7
7               char str1[] = "abcd";
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
9               wchar_t str2[] = L"abcd";
(gdb) n
11              return 0x0;
(gdb) p sizeof(wchar_t)
$1 = 4
(gdb) x/ws str2
0x7fffffffe520: U"abcd"
(gdb) 
{% endhighlight %}

由于当前平台宽字符的长度为4个字节，则用```x/ws```命令； 如果是2个字节，则用```x/hs```命令。




## 2. 打印大数组中的内容

1) 示例程序
{% highlight string %}
int main(int argc, char *argv)
{
	int array[201];
	int i;

	for (i = 0; i < 201; i++)
		array[i] = i;

	return 0;
}

{% endhighlight %}

2) 调试技巧

在gdb中，如果要打印大数组的内容，缺省最多会显示200个元素：
{% highlight string %}
# gcc -g -c -o test.o test.c
[root@bogon workspace]# gcc -o test test.o
[root@bogon workspace]# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b 9
Breakpoint 1 at 0x40052a: file test.c, line 9.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, main (argc=1, argv=0x7fffffffe638 "[\350\377\377\377\177") at test.c:9
9               return 0;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) p array
$1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 
  43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 
  85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 
  121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 
  154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 
  187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199...}
(gdb)
{% endhighlight %}

可以使用如下命令设置这个最大限制数：
<pre>
(gdb) set print elements number-of-elements
</pre>
也可以使用如下命令，设置为没有限制：
<pre>
(gdb) set print elements 0

或者
(gdb) set print elements unlimited
</pre>


另外，如果要打印数组中任意连续元素的值，可以使用```p array[index]@num```命令。其中```index```是数组索引（从0开始计数），```num```是连续多少个元素。以上面代码为例：
{% highlight string %}
(gdb) p array[10]@10
$6 = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19}
{% endhighlight %}
可见打印了```array```数组的第10~19功10个元素。

如果要打印从数组开头连续元素的值，也可以使用命令```p *array@num```。例如：
<pre>
(gdb) p *array@10
$7 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
</pre>


## 3. 打印数组的索引下标

1) 示例程序
{% highlight string %}
#include <stdio.h>

int num[10] = {
	1 << 0,
	1 << 1,
	1 << 2,
	1 << 3,
	1 << 4,
	1 << 5,
	1 << 6,
	1 << 7,
	1 << 8,
	1 << 9
};

int main(int argc, char *argv[])
{
  int i;

  for (i = 0; i < 10; i++)
    printf ("num[%d] = %d\n", i, num[i]);

  return 0;
}
{% endhighlight %}

2) 调试技巧

在gdb中，当打印一个数组时，缺省是不打印索引下标的：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40053c: file test.c, line 20.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:20
20        for (i = 0; i < 10; i++)
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) p num
$1 = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512}
(gdb) 
{% endhighlight %}

如果要打印索引下标，则可以通过如下命令进行设置：
<pre>
(gdb) set print array-indexes on
(gdb) p num
$2 = {[0] = 1, [1] = 2, [2] = 4, [3] = 8, [4] = 16, [5] = 32, [6] = 64, [7] = 128, [8] = 256, [9] = 512}
(gdb) 
</pre>

## 4. 打印函数局部变量的值

1) 示例程序
{% highlight string %}
#include <stdio.h>

void fun_a(void)
{
	int a = 0;
	printf("%d\n", a);
}

void fun_b(void)
{
	int b = 1;
	fun_a();
	printf("%d\n", b);
}

void fun_c(void)
{
	int c = 2;
	fun_b();
	printf("%d\n", c);
}

void fun_d(void)
{
	int d = 3;
	fun_c();
	printf("%d\n", d);
}

int main(int argc, char *argv[])
{
	int var = -1;
	fun_d();
	return 0;
}
{% endhighlight %}

2) 调试技巧1

如果要打印函数局部变量的值，可以使用```bt full```命令。首先我们在函数fun_a()中打上断点，当程序断住时，显示调用栈信息：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b fun_a
Breakpoint 1 at 0x400535: file test.c, line 5.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, fun_a () at test.c:5
5               int a = 0;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) bt
#0  fun_a () at test.c:5
#1  0x0000000000400566 in fun_b () at test.c:12
#2  0x0000000000400590 in fun_c () at test.c:19
#3  0x00000000004005ba in fun_d () at test.c:26
#4  0x00000000004005eb in main (argc=1, argv=0x7fffffffe638) at test.c:33
(gdb) 
{% endhighlight %}

接下来，用```bt full```命令显示各个函数的局部变量值:
{% highlight string %}
(gdb) bt full
#0  fun_a () at test.c:5
        a = 32767
#1  0x0000000000400566 in fun_b () at test.c:12
        b = 1
#2  0x0000000000400590 in fun_c () at test.c:19
        c = 2
#3  0x00000000004005ba in fun_d () at test.c:26
        d = 3
#4  0x00000000004005eb in main (argc=1, argv=0x7fffffffe638) at test.c:33
        var = -1
(gdb)
{% endhighlight %}

也可以使用如下```bt full n```，意思是从内向外显示n个栈帧，及其局部变量，例如：
{% highlight string %}
(gdb) bt full 2
#0  fun_a () at test.c:5
        a = 32767
#1  0x0000000000400566 in fun_b () at test.c:12
        b = 1
(More stack frames follow...)
(gdb) 
{% endhighlight %}

而```bt full -n```，意思是从外向内显示n个栈桢，及其局部变量，例如：
{% highlight string %}
(gdb) bt full -2
#3  0x00000000004005ba in fun_d () at test.c:26
        d = 3
#4  0x00000000004005eb in main (argc=1, argv=0x7fffffffe638) at test.c:33
        var = -1
(gdb)
{% endhighlight %}

3) 调试技巧2

如果只是想打印当前函数局部变量的值，可以使用如下命令：
{% highlight string %}
(gdb) info locals
a = 0
{% endhighlight %}

## 5. 打印进程内存信息

用gdb调试程序时，如果想查看进程的内存映射信息，可以使用```info proc mappings```命令,例如:
{% highlight string %}
(gdb) i proc mappings
process 27676 flags:
PR_STOPPED Process (LWP) is stopped
PR_ISTOP Stopped on an event of interest
PR_RLC Run-on-last-close is in effect
PR_MSACCT Microstate accounting enabled
PR_PCOMPAT Micro-state accounting inherited on fork
PR_FAULTED : Incurred a traced hardware fault FLTBPT: Breakpoint trap

Mapped address spaces:

    Start Addr   End Addr       Size     Offset   Flags
     0x8046000  0x8047fff     0x2000 0xfffff000 -s--rwx
     0x8050000  0x8050fff     0x1000          0 ----r-x
     0x8060000  0x8060fff     0x1000          0 ----rwx
    0xfee40000 0xfef4efff   0x10f000          0 ----r-x
    0xfef50000 0xfef55fff     0x6000          0 ----rwx
    0xfef5f000 0xfef66fff     0x8000   0x10f000 ----rwx
    0xfef67000 0xfef68fff     0x2000          0 ----rwx
    0xfef70000 0xfef70fff     0x1000          0 ----rwx
    0xfef80000 0xfef80fff     0x1000          0 ---sr--
    0xfef90000 0xfef90fff     0x1000          0 ----rw-
    0xfefa0000 0xfefa0fff     0x1000          0 ----rw-
    0xfefb0000 0xfefb0fff     0x1000          0 ----rwx
    0xfefc0000 0xfefeafff    0x2b000          0 ----r-x
    0xfeff0000 0xfeff0fff     0x1000          0 ----rwx
    0xfeffb000 0xfeffcfff     0x2000    0x2b000 ----rwx
    0xfeffd000 0xfeffdfff     0x1000          0 ----rwx
{% endhighlight %}
首先输出了进程的flags，接着是进程的内存映射信息。

此外，也可以用```info files```(还有一个同样作用的命令： ```info target```)命令，它可以更详细地输出进程的内存信息，包括引用的动态链接库等等。例如：
{% highlight string %}
gdb) i files
Symbols from "/data1/nan/a".
Unix /proc child process:
    Using the running image of child Thread 1 (LWP 1) via /proc.
    While running this, GDB does not access memory from...
Local exec file:
    `/data1/nan/a', file type elf32-i386-sol2.
    Entry point: 0x8050950
    0x080500f4 - 0x08050105 is .interp
    0x08050108 - 0x08050114 is .eh_frame_hdr
    0x08050114 - 0x08050218 is .hash
    0x08050218 - 0x08050418 is .dynsym
    0x08050418 - 0x080507e6 is .dynstr
    0x080507e8 - 0x08050818 is .SUNW_version
    0x08050818 - 0x08050858 is .SUNW_versym
    0x08050858 - 0x08050890 is .SUNW_reloc
    0x08050890 - 0x080508c8 is .rel.plt
    0x080508c8 - 0x08050948 is .plt
    ......
    0xfef5fb58 - 0xfef5fc48 is .dynamic in /usr/lib/libc.so.1
    0xfef5fc80 - 0xfef650e2 is .data in /usr/lib/libc.so.1
    0xfef650e2 - 0xfef650e2 is .bssf in /usr/lib/libc.so.1
    0xfef650e8 - 0xfef65be0 is .picdata in /usr/lib/libc.so.1
    0xfef65be0 - 0xfef666a7 is .data1 in /usr/lib/libc.so.1
    0xfef666a8 - 0xfef680dc is .bss in /usr/lib/libc.so.1
{% endhighlight %}

## 6. 打印变量的类型和所在文件

1) 示例程序
{% highlight string %}
#include <stdio.h>

struct child {
  char name[10];
  enum { boy, girl } gender;
};

struct child he = { "Tom", boy };

int main(int argc, char *argv[])
{
	static struct child she = { "Jerry", girl };
	printf ("Hello %s %s.\n", he.gender == boy ? "boy" : "girl", he.name);
	printf ("Hello %s %s.\n", she.gender == boy ? "boy" : "girl", she.name);
	
	return 0;
}
{% endhighlight %}


2) 调试技巧

在gdb中，可以使用如下命令查看变量的类型：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40053c: file test.c, line 13.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:13
13              printf ("Hello %s %s.\n", he.gender == boy ? "boy" : "girl", he.name);
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) whatis he
type = struct child
{% endhighlight %}
如果想查看详细的类型信息：
{% highlight string %}
(gdb) ptype he
type = struct child {
    char name[10];
    enum {boy, girl} gender;
}
{% endhighlight %}
如果想查看定义该变量的文件：
{% highlight string %}
(gdb) info variables he
All variables matching regular expression "he":

File test.c:
struct child he;

Non-debugging symbols:
0x0000000000601050  she.2187
0x00007ffff7ffe070  cachesize
0x00007ffff7ffe078  cache_new
{% endhighlight %}

上面我们看到， GDB会显示所有包含（匹配）该表达式的变量。如果只想查看完全匹配给定名字的变量，则：
{% highlight string %}
(gdb) i variables ^he$
All variables matching regular expression "^he$":

File test.c:
struct child he;
{% endhighlight %}

注意： ```info variables```不会显示局部变量，即使是static的也没有太多的信息。


## 7. 打印内存的值

1) 示例程序
{% highlight string %}
#include <stdio.h>

int main(int argc, char *argv[])
{
	int i = 0;
	char a[100];
	
	for(i = 0;i<sizeof(a); i++)
		a[i]  = i;
		
	return 0x0;	
}

{% endhighlight %}

2) 调试技巧

GDB中使用```x```命令来打印内存的值，格式为```x/nfu addr```。含义为以```f```格式打印从addr开始的```n```个长度单元为```u```的内存在。参数具体含义如下：
<pre>
n: 输出单元的个数；
f: 是输出格式。比如x是以16进制形式输出，o是以8进制形式输出，等等；
u: 标明一个单元的长度。b是一个byte， h是2个byte(halfword)，w是4个byte(word)，g是8个byte(giant word);
</pre>
以上面程序为例：

* 以16进制格式打印数组a的前16个byte的值
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b 11
Breakpoint 1 at 0x400526: file test.c, line 11.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:11
11              return 0x0;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) x/16xb a
0x7fffffffe4e0: 0x00    0x01    0x02    0x03    0x04    0x05    0x06    0x07
0x7fffffffe4e8: 0x08    0x09    0x0a    0x0b    0x0c    0x0d    0x0e    0x0f
(gdb
{% endhighlight %}

* 以无符号10进制格式打印数组a的前16个byte的值
{% highlight string %}
(gdb) x/16ub a
0x7fffffffe4e0: 0       1       2       3       4       5       6       7
0x7fffffffe4e8: 8       9       10      11      12      13      14      15
{% endhighlight %}

* 以二进制格式打印数组a的前16个byte的值
{% highlight string %}
(gdb) x/16tb a
0x7fffffffe4e0: 00000000        00000001        00000010        00000011        00000100        00000101        00000110        00000111
0x7fffffffe4e8: 00001000        00001001        00001010        00001011        00001100        00001101        00001110        00001111
{% endhighlight %}

* 以16进制格式打印数组a的前16个word(4个byte)的值
{% highlight string %}
(gdb) x/16xw a
0x7fffffffe4e0: 0x03020100      0x07060504      0x0b0a0908      0x0f0e0d0c
0x7fffffffe4f0: 0x13121110      0x17161514      0x1b1a1918      0x1f1e1d1c
0x7fffffffe500: 0x23222120      0x27262524      0x2b2a2928      0x2f2e2d2c
0x7fffffffe510: 0x33323130      0x37363534      0x3b3a3938      0x3f3e3d3c
{% endhighlight %}


## 8. 打印源代码行

1) 示例程序
{% highlight string %}
#include <stdio.h>

void fun_a(void)
{
	int a = 0;
	printf("%d\n", a);
}

void fun_b(void)
{
	int b = 1;
	fun_a();
	printf("%d\n", b);
}

void fun_c(void)
{
	int c = 2;
	fun_b();
	printf("%d\n", c);
}

void fun_d(void)
{
	int d = 3;
	fun_c();
	printf("%d\n", d);
}

int main(int argc, char *argv[])
{
	int var = -1;
	fun_d();
	return 0;
}
{% endhighlight %}

2) 调试技巧

在GDB中可以使用list命令来显示源代码以及行号。list命令可以指定行号、函数：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) list 10
5               int a = 0;
6               printf("%d\n", a);
7       }
8
9       void fun_b(void)
10      {
11              int b = 1;
12              fun_a();
13              printf("%d\n", b);
14      }
(gdb) list fun_d
19              fun_b();
20              printf("%d\n", c);
21      }
22
23      void fun_d(void)
24      {
25              int d = 3;
26              fun_c();
27              printf("%d\n", d);
28      }
(gdb)
{% endhighlight %}

还可以指定向前或向后打印:
{% highlight string %}
(gdb) list +
29
30      int main(int argc, char *argv[])
31      {
32              int var = -1;
33              fun_d();
34              return 0;
35      }
(gdb) list -
19              fun_b();
20              printf("%d\n", c);
21      }
22
23      void fun_d(void)
24      {
25              int d = 3;
26              fun_c();
27              printf("%d\n", d);
28      }
(gdb) 
{% endhighlight %}

还可以指定打印范围：
{% highlight string %}
(gdb) list 10,20
10      {
11              int b = 1;
12              fun_a();
13              printf("%d\n", b);
14      }
15
16      void fun_c(void)
17      {
18              int c = 2;
19              fun_b();
20              printf("%d\n", c);
(gdb)
{% endhighlight %}

## 9. 每行打印一个结构体成员

1) 示例程序
{% highlight string %}
#include <pthread.h>


typedef struct{
        int a;
        int b;
        int c;
        int d;
        pthread_mutex_t mutex;
}ex_st;

int main(int argc, char *argv[])
{
        ex_st st = {1, 2, 3, 4, PTHREAD_MUTEX_INITIALIZER};
        printf("%d,%d,%d,%d\n", st.a, st.b, st.c, st.d);

        return 0;
}
{% endhighlight %}


2) 调试技巧

默认情况下，GDB是以一种```紧凑```的方式打印结构体。以上面代码为例：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o
# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) start
Temporary breakpoint 1 at 0x40053c: file test.c, line 15.
Starting program: /root/workspace/./test 

Temporary breakpoint 1, main (argc=1, argv=0x7fffffffe638) at test.c:15
15              ex_st st = {1, 2, 3, 4, PTHREAD_MUTEX_INITIALIZER};
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) n
16              printf("%d,%d,%d,%d\n", st.a, st.b, st.c, st.d);
(gdb) p st
$1 = {a = 1, b = 2, c = 3, d = 4, mutex = {__data = {__lock = 0, __count = 0, __owner = 0, __nusers = 0, __kind = 0, __spins = 0, __list = {__prev = 0x0, __next = 0x0}}, 
    __size = '\000' <repeats 39 times>, __align = 0}}
{% endhighlight %}

可以看到结构体显示很混乱，尤其是结构体里还嵌套着其他结构体时。

可以执行```set print pretty on```命令，这样每行只会显示结构体的一名成员，而且还会根据成员的定义层次进行缩进：
{% highlight string %}
(gdb) set print pretty on
(gdb) p st
$2 = {
  a = 1, 
  b = 2, 
  c = 3, 
  d = 4, 
  mutex = {
    __data = {
      __lock = 0, 
      __count = 0, 
      __owner = 0, 
      __nusers = 0, 
      __kind = 0, 
      __spins = 0, 
      __list = {
        __prev = 0x0, 
        __next = 0x0
      }
    }, 
    __size = '\000' <repeats 39 times>, 
    __align = 0
  }
}
(gdb)
{% endhighlight %}

## 10. 按照派生类型打印对象

1) 示例程序
{% highlight string %}
#include <iostream>
using namespace std;

class Shape 
{
public:
	virtual void draw () {}
};

class Circle : public Shape 
{
	int radius;
public:
	Circle () { radius = 1; }
	void draw () { cout << "drawing a circle...\n"; }
};

class Square : public Shape 
{
	int height;
public:
	Square () { height = 2; }
	void draw () { cout << "drawing a square...\n"; }
};

void drawShape (class Shape &p)
{
	p.draw ();
}

int main(int argc, char *argv[])
{
	Circle a;
	Square b;
	drawShape (a);
	drawShape (b);
	
	return 0;
}
{% endhighlight %}

2) 调试技巧

在GDB中，当打印一个对象时，缺省是按照声明的类型进行打印：
{% highlight string %}
# gcc -g -c -o test.o test.cpp
# gcc -o test test.o -lstdc++

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) list drawShape
22              Square () { height = 2; }
23              void draw () { cout << "drawing a square...\n"; }
24      };
25
26      void drawShape (class Shape &p)
27      {
28              p.draw ();
29      }
30
31      int main(int argc, char *argv[])
(gdb) b 28
Breakpoint 1 at 0x400869: file test.cpp, line 28.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, drawShape (p=...) at test.cpp:28
28              p.draw ();
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64 libgcc-4.8.5-28.el7_5.1.x86_64 libstdc++-4.8.5-28.el7_5.1.x86_64
(gdb) frame
#0  drawShape (p=...) at test.cpp:28
28              p.draw ();
(gdb) p p
$1 = (Shape &) @0x7fffffffe540: {_vptr.Shape = 0x400ad0 <vtable for Circle+16>}
{% endhighlight %}

在这个例子中，p虽然声明为```class Shape```，但它实际的派生类型可能为```class Circle```和```class Square```。如果要缺省按照派生类型进行打印，则可以通过如下命令进行设置：
{% highlight string %}
(gdb) set print object on
(gdb) p p
$2 = (Circle *) 0x7fffffffe540
{% endhighlight %}

当打印对象类型信息时，该设置也会起作用:
{% highlight string %}
(gdb) whatis p
type = Shape &
(gdb) ptype p
type = class Shape {
  public:
    virtual void draw(void);
} &
(gdb) set print object on
(gdb) whatis p
type = /* real type = Circle & */
Shape &
(gdb) ptype p
type = /* real type = Circle & */
class Shape {
  public:
    virtual void draw(void);
} &
(gdb) 
{% endhighlight %}


## 11. 指定程序的输入输出设备

1) 示例程序
{% highlight string %}
#include <stdio.h>

int main(int argc, char *argv[])
{
        int i;

        for (i = 0; i < 100; i++)
        {
                printf("i = %d\n", i);
        }

        return 0;
}
{% endhighlight %}

2) 调试技巧

在GDB中，缺省情况下程序的输入输出是和GDB使用同一个终端。你也可以为程序指定一个单独的输入输出终端。首先我们查看当前终端设备文件名：
<pre>
# tty
/dev/pts/0
</pre>

然后，打开一个新终端，使用如下命令获得设备文件名：
<pre>
# tty
/dev/pts/1
</pre>

接着，我们在```/dev/pts/0```终端下启动程序，通过命令行选项指定程序的输入输出设备：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb -q -tty /dev/pts/1 ./test
Reading symbols from /root/workspace/test...done.
(gdb) r
Starting program: /root/workspace/./test 
[Inferior 1 (process 116521) exited normally]
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb)
{% endhighlight %}
这时，我们可以看到在```/dev/pts/1```终端下输出了打印值。

另外，我们也可以在GDB中通过使用如下命令来切换终端：
<pre>
(gdb) tty /dev/pts/1
</pre>

## 12. 打印调用栈帧中的变量值

1) 示例程序
{% highlight string %}
#include <stdio.h>

int func1(int a)
{
	int b = 1;
	return b * a;
}

int func2(int a)
{
	int b = 2;
	return b * func1(a);
}

int func3(int a)
{
	int b = 3;
	return b * func2(a);
}

int main(int argc, char *argv[])
{
	printf("%d\n", func3(10));
	return 0;
}
{% endhighlight %} 


2) 调试技巧

在GDB中，如果想查看调用栈帧中的变量，可以先切换到该栈帧中，然后打印：
{% highlight string %}
# gcc -g -c -o test.o test.c
# gcc -o test test.o

# gdb -q ./test
Reading symbols from /root/workspace/test...done.
(gdb) b func1
Breakpoint 1 at 0x400534: file test.c, line 5.
(gdb) r
Starting program: /root/workspace/./test 

Breakpoint 1, func1 (a=10) at test.c:5
5               int b = 1;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7.x86_64
(gdb) bt
#0  func1 (a=10) at test.c:5
#1  0x0000000000400560 in func2 (a=10) at test.c:12
#2  0x0000000000400582 in func3 (a=10) at test.c:18
#3  0x00000000004005a1 in main (argc=1, argv=0x7fffffffe638) at test.c:23
(gdb) frame 1
#1  0x0000000000400560 in func2 (a=10) at test.c:12
12              return b * func1(a);
(gdb) p b
$1 = 2
(gdb) frame 2
#2  0x0000000000400582 in func3 (a=10) at test.c:18
18              return b * func2(a);
(gdb) p b
$2 = 3
(gdb) 
{% endhighlight %}

也可以不进行切换，直接打印：
{% highlight string %}
(gdb) p func2::b
$3 = 2
(gdb) p func3::b
$4 = 3
(gdb) 
{% endhighlight %}

同样，对于C++的函数名，需要使用单引号括起来，比如：
{% highlight string %}
(gdb) p '(anonymous namespace)::SSAA::handleStore'::n->pi->inst->dump()
{% endhighlight %}



<br />
<br />

**[参看]**


1. [100个gdb小技巧](https://www.kancloud.cn/wizardforcel/gdb-tips-100/146771)

2. [设置 GDB 代码搜索路径](https://blog.csdn.net/caspiansea/article/details/42447203)



<br />
<br />
<br />





