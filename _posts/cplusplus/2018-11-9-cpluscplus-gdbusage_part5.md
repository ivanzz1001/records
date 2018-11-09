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


## 1. 改变程序的


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





