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



<br />
<br />

**[参看]:**

1. [网络带宽的测试算法研究](http://www.docin.com/p-575514222.html)

2. [C++静态变量内存分配，编译阶段，解密 ](http://blog.163.com/lucky_jeck/blog/static/12711474201311182464554/)
<br />
<br />
<br />





