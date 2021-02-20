---
layout: post
title: GDB调试：查看指定内存中的内容
tags:
- cplusplus
categories: cplusplus
description: GDB的使用
---


本节介绍一下如何使用GDB查看指定内存中的内容。



<!-- more -->


## 1. print命令查看数组内容
我们可以使用print命令来查看数组内容，格式如下：
<pre>
# p *array@len
</pre>
其中，array是数组第一个元素的地址，可以使用具体的地址或者数组名；len为要显示的元素个数。

参看如下示例：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int a[5] = {1,2,3,4,5}

    for(int i = 0; i<5; i++)
        printf("%d\n", a[i]);


   return 0x0;
}
{% endhighlight %}
编译调试：
{% highlight string %}
#  gcc -g -o test test.c
# gdb -q ./test
Reading symbols from /data/home/lzy/just_for_test/test...done.
(gdb) l
1       #include <stdio.h>
2       #include <stdlib.h>
3       
4       int main(int argc, char *argv[])
5       {
6           int a[5] = {1,2,3,4,5};
7       
8           for(int i = 0; i<5; i++)
9               printf("%d\n", a[i]);
10      
(gdb) b test.c:8
Breakpoint 1 at 0x400558: file test.c, line 8.
(gdb) r
Starting program: /data/home/lzy/just_for_test/./test 

Breakpoint 1, main (argc=1, argv=0x7fffffffe058) at test.c:8
8           for(int i = 0; i<5; i++)
Missing separate debuginfos, use: debuginfo-install glibc-2.17-307.el7.1.x86_64
(gdb) p *a@5
$1 = {1, 2, 3, 4, 5}
(gdb) p *(a+1)@4
$2 = {2, 3, 4, 5}
(gdb) p *(a+2)@3
$3 = {3, 4, 5}
(gdb)
{% endhighlight %}

## 2. examine命令查看指定地址内容
使用examine命令（缩写为x)可以查看指定内存地址的值。语法如下：
{% highlight string %}
# x/[number][format][unit] addr

# gdb --quiet
(gdb) help x
Examine memory: x/FMT ADDRESS.
ADDRESS is an expression for the memory address to examine.
FMT is a repeat count followed by a format letter and a size letter.
Format letters are o(octal), x(hex), d(decimal), u(unsigned decimal),
  t(binary), f(float), a(address), i(instruction), c(char) and s(string).
Size letters are b(byte), h(halfword), w(word), g(giant, 8 bytes).
The specified number of objects of the specified size are printed
according to the format.

Defaults for format and size letters are those previously used.
Default count is 1.  Default address is following last thing printed
with this command or "print".
(gdb) 
{% endhighlight %}
其中：

1） number为一个正整数，表示从当前地址向后显示几个地址的内容.

2) format表示显示的格式。如果地址所指的是字符串，那么格式可以是s，如果地址是指令地址，那么格式可以是i。

* x(hex) 按十六进制格式显示变量。

* d(decimal) 按十进制格式显示变量。

* u(unsigned decimal) 按十进制格式显示无符号整型。

* o(octal) 按八进制格式显示变量。

* t(binary) 按二进制格式显示变量。

* a(address) 按十六进制格式显示变量。

* c(char) 按字符格式显示变量。

* f(float) 按浮点数格式显示变量

* i(instruction) 按指令方式显示指定地址处的内容

* s(string) 按字符串方式显示变量

3) unit表示一个地址单元长度

* b: 表示单字节

* h: 表示双字节

* w: 表示四字节

* g: 表示八字节

 
参看如下示例：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int a[5] = {1,2,3,4,5};
    char b[5] = {'a', 'b', 'c', 'd', 'e'};
    char c[] = "hello";
    float d[5] = {3.14, 3.15, 3.16, 3.17, 3.18};
    double e[5] = {10.0, 20.0, 30.0, 40.0, 50.0};



    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;


   return 0x0;
}
{% endhighlight %}
编译调试：
{% highlight string %}
gdb --quiet ./test
Reading symbols from /data/home/lzy/just_for_test/test...done.
(gdb) l 10
5       {
6           int a[5] = {1,2,3,4,5};
7           char b[5] = {'a', 'b', 'c', 'd', 'e'};
8           char c[] = "hello";
9           float d[5] = {3.14, 3.15, 3.16, 3.17, 3.18};
10          double e[5] = {10.0, 20.0, 30.0, 40.0, 50.0};
11      
12      
13      
14          (void)a;
(gdb) b test.c:14
Breakpoint 1 at 0x40059b: file test.c, line 14.
(gdb) r
Starting program: /data/home/lzy/just_for_test/./test 

Breakpoint 1, main (argc=1, argv=0x7fffffffe058) at test.c:21
21         return 0x0;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-307.el7.1.x86_64
(gdb) x/s c
0x7fffffffdf45: "hello"
(gdb) p *d@5
$1 = {3.1400001, 3.1500001, 3.16000009, 3.17000008, 3.18000007}
(gdb) x/f d
0x7fffffffdf30: 3.1400001
(gdb) x/5f d
0x7fffffffdf30: 3.1400001       3.1500001       3.16000009      3.17000008
0x7fffffffdf40: 3.18000007
(gdb) 

{% endhighlight %}
注： float占用字节数是4。在使用x命令时，字节数需要指定正确，否则可能造成内存混乱，从而出现打印错误。









<br />
<br />

**[参看]**


1. [gdb调试进阶之查看指定内存中的内容](https://blog.csdn.net/hustluy/article/details/12751113)

2. [GDB查看内存](https://www.cnblogs.com/adamwong/p/10538019.html)

<br />
<br />
<br />





