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
{% highlihgt string %}
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



<br />
<br />

**[参看]**


1. [100个gdb小技巧](https://www.kancloud.cn/wizardforcel/gdb-tips-100/146771)

2. [设置 GDB 代码搜索路径](https://blog.csdn.net/caspiansea/article/details/42447203)



<br />
<br />
<br />





