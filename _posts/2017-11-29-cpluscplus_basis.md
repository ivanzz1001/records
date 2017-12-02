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




<br />
<br />



<br />
<br />
<br />





