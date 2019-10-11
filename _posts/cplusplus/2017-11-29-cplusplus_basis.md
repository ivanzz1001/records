---
layout: post
title: C/C++基础
tags:
- cplusplus
categories: cplusplus
description: C/C++基础
---

本章主要记录一下C/C++基础方面的一些内容，以备后续查验。


<!-- more -->


## 1. 类的大小

参看如下代码test.cpp:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


class A{
};

class B{
public:
   int a;
};

class C{
public:
    static int a;
};

class D{
public:
   virtual void print();
};

class E{
public: 
    void print();
};

class F : public D{
public:
   virtual void wow();
};

int main(int argc,char *argv[])
{
    int sizeA,sizeB,sizeC,sizeD,sizeE,sizeF;
    sizeA = sizeof(A);
    sizeB = sizeof(B);
    sizeC = sizeof(C);
    sizeD = sizeof(D);
    sizeE = sizeof(E);
    sizeF = sizeof(F);

    printf("sizeof(int): %d\n",sizeof(int));
    printf("sizeof(intprt_t): %d\n",sizeof(intptr_t));
    printf("sizeof(void *): %d\n\n",sizeof(void *));

    printf("sizeA: %d\n",(intptr_t)sizeA);
    printf("sizeB: %d\n",(intptr_t)sizeB);
    printf("sizeC: %d\n",(intptr_t)sizeC);
    printf("sizeD: %d\n",(intptr_t)sizeD);
    printf("sizeE: %d\n",(intptr_t)sizeE);
    printf("sizeF: %d\n",(intptr_t)sizeF);
    return 0;
}
{% endhighlight %}
编译运行：
<pre>
[root@localhost test-src]# gcc -o test test.cpp -lstdc++
[root@localhost test-src]# ./test
sizeof(int): 4
sizeof(intprt_t): 8
sizeof(void *): 8

sizeA: 1
sizeB: 4
sizeC: 1
sizeD: 8
sizeE: 1
sizeF: 8
</pre>


## 2. 基本数据类型大小

64bit操作系统：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[])
{
   printf("sizeof(char): %d\n",sizeof(char));
   printf("sizeof(short): %d\n",sizeof(short));
   printf("sizeof(int): %d\n",sizeof(int));
   printf("sizeof(long): %d\n",sizeof(long));
   printf("sizeof(long long): %d\n",sizeof(long long));
   printf("sizeof(float): %d\n",sizeof(float));
   printf("sizeof(double): %d\n",sizeof(double));
   return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.c
# ./test
sizeof(char): 1
sizeof(short): 2
sizeof(int): 4
sizeof(long): 8
sizeof(long long): 8
sizeof(float): 4
sizeof(double): 8
</pre>

```32bit```操作系统下编译运行：
<pre>
# gcc -o test test.c
# ./test
sizeof(char): 1
sizeof(short): 2
sizeof(int): 4
sizeof(long): 4
sizeof(long long): 8
sizeof(float): 4
sizeof(double): 8
</pre>


## 3. 类中静态成员的初始化时间
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

class A{
public:
   A()
   {
      printf("A constructor\n");
   }
};

class B{
public:
   B()
   {
     printf("B constructor\n");
   }
private:
   static A a;
};

A B::a;

int main(int argc,char *argv[])
{
   printf("main ...\n");
   return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
A constructor
main ...
</pre>
由此我们得出，类的静态成员初始化时间发生在： ```从静态存储区加载到内存时```

## 4. 栈的生长方向

在常见的x86中内存中栈的增长方向就是从高地址向低地址增长，但也有些不常用的处理器可能不同。有多种方法可以获得栈的生长方向。下面我们简单介绍：

1) **通过递归函数调用**

我们可以通过递归函数调用来获得栈的生长方向。参看如下示例：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static int stack_dir;

static void find_stack_direction()
{
    static char *addr = NULL;

    char dummy;

    if(!addr)
    {
        addr = &dummy;
        find_stack_direction();
    }
    else{
        if((uintptr_t)(char *)&dummy > (uintptr_t)addr)
        {
            stack_dir = 1;           //向高地址方向生长
        }else{
            stack_dir = -1;          //向低地址方向生长
        }
    }

}

int main(int argc,char *argv[])
{
    find_stack_direction();

    if(stack_dir > 0)
        printf("向高地址方向生长\n");
    else
        printf("向低地址方向生长\n");

    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.c 
# ./test
向低地址方向生长
</pre>


2) **直接阅读汇编指令**
我们可以通过直接阅读汇编指令来判断。例如，在```IA-32```中，如果在一个过程的开始阶段（准备段）出现类似```sub $0x10,%esp```,说明栈顶指针(%esp)是变小的，因此栈是向低地址增长的。

3) **显示栈顶指针寄存器的内容**

我们可以通过GDB等工具，在某个过程开始阶段和结束阶段分别显示栈顶指针寄存器的内容，比较它们的大小。若开始处的值比结束处的大，则说明是向低地址增长的。


## 5. 大小端
在计算机中经常涉及到大小端的问题，关于大小端的定义如下：

* **大端模式**： 是指数据的高字节保存在内存的低地址中，而数据的低字节保存在内存的高地址中，这样的存储模式有点儿类似于把数据当作字符串顺序处理————地址由小向大增加，而数据从高位往低位放； 这和我们的阅读习惯一致。
<pre>
注： 大端模式又称为网络字节序
</pre>

* **小端模式**： 是指数据的高字节保存在内存的高地址中，而数据的低字节保存在内存的低地址中，这种存储模式将地址的高低和数据位权有效的结合起来，高地址部分权值高，低地址部分权值低。


以下图为例：

![cpp-endian](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_little_big_endian.jpg)


<br />
<br />

**[参看]:**

1. [网络带宽的测试算法研究](http://www.docin.com/p-575514222.html)

2. [C++静态变量内存分配，编译阶段，解密 ](http://blog.163.com/lucky_jeck/blog/static/12711474201311182464554/)

3. [C 语言标准库实现](https://ftp.gnu.org/gnu/glibc/)

4. [栈增长方向与大端/小端问题](https://www.cnblogs.com/xkfz007/archive/2012/06/22/2558935.html)

5. [leetcode](https://leetcode.com/problems/word-break-ii/)

6. [leetcode刷题](http://www.cnblogs.com/grandyang/p/4606334.html)

7. [C++书籍推荐](https://www.cnblogs.com/awesome-share/p/10056179.html)

8. [在线工具](https://tool.lu/timestamp/)

<br />
<br />
<br />





