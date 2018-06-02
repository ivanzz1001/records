---
layout: post
title: C++对象内存模型及虚函数实现
tags:
- cplusplus
categories: cplusplus
description: C++对象内存模型及虚函数实现
---

本文讲述一下C++对象内存模型及虚函数实现原理。本文参考网上很多文章并经过亲自试验， 修正了其中的一些错误。实验操作系统环境为64bit Centos7.3：

<!-- more -->
<pre>
# uname -a
Linux compile 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux

# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 
</pre>



## 1. C++类基础

**1) 类中的元素**


一个C++类中存在的元素可能有如下：

* 成员变量

* 成员函数

* 静态成员变量

* 静态成员函数

* 虚函数

* 纯虚函数


**2) 影响对象大小因素**

C++中，影响对象大小的因素主要有：

* 成员变量

* 虚函数表指针（_vftptr）

* 虚基类表指针(_vbtptr)

* 内存对齐

其中,```_vftptr```和```_vbtptr```的初始化由对象的构造函数、赋值运算符自动完成； 对象生命周期结束后，由对象的析构函数来销毁。对象所关联的类型(type_info)，通常放在virtual table的第一个slot中。

**3) 虚继承与虚基类**

* 虚继承： 在继承定义中包含了virtual关键字的继承关系

* 虚基类： 在虚继承体系中通过virtual继承而来的基类

这里需要注意的是：
{% highlight string %}
class CDerive : public virtual CBase{
};
{% endhighlight %}
上面代码```CDerive```虚继承了CBase， 而```CBase```称之为```CDerive```的虚基类， 而不是说```CBase```本身就是个虚基类， 因为CBase还可以为不是虚继承体系中的基类。

虚函数被派生后，仍然为虚函数，即使在派生类中省去virtual关键字。

虚基类的构造和析构是由最终子类负责调用的（而不是直接派生子类）


 

## 2. C++类成员函数调用

在具体讲解C++对象模型及虚函数之前，我们先给出一个示例，讲解一下C++类成员函数的调用：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>


class A{
public:
    static void static_func()
    {
         printf("call static func\n");
    }

    void nonstatic_func()
    {
         printf("call nonstatic func\n");
    }

    virtual void virtual_func()
    {
        printf("call virtual func\n");
    }
};


int main(int argc,char *argv[])
{

    A a;

    void (*pfunc_1)() = A::static_func;
    (*pfunc_1)();


    //Note: here must use &A::nonstatic_func
    void (A::*pfunc_2)() = &A::nonstatic_func;
    (a.*pfunc_2)();


    // virtual member function
    uintptr_t **p = (uintptr_t **)&a;
    typedef void (*pvirtual_func)(A *);
    pvirtual_func pvfunc = (pvirtual_func)**p;
    pvfunc(&a);

    // another method to call virtual function
    void (A::*pfunc_3)() = &A::virtual_func;
    (a.*pfunc_3)();

    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
call static func
call nonstatic func
call virtual func
call virtual func
</pre>

上面我们展示了C++类中相关成员函数的调用方法。

## 3. 对象内存布局
下面我们分不同的情况，分别讨论C++对象的内存布局。

注：下文举例的类图中函数均为虚函数（*斜体* 表示该函数为虚函数）

### 3.1 单一类

**1) 空类**

![cpp-obj-nonmemer](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_object_nonmember.jpg)

```sizeof(CNull)=1```，用于标识该对象。

**2) 只有成员变量的类**

![cpp-obj-model](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_object_model_figure2.jpg)

<pre>
int nVarSize = sizeof(CVariable) = 12
</pre>

我们用如下程序进行测试：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>


class CVariable{
private:
   int m_a;
   int m_b;
   int m_c;
};

int main(int argc,char *argv[])
{
   CVariable pVarA;

   printf("&pVarA: %p\n", &pVarA);
   printf("sizeof(CVariable): %d\n", sizeof(CVariable));

   return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -g -o test test.cpp -lstdc++
# gdb ./test
(gdb) p pVarA
$1 = {m_a = -8096, m_b = 32767, m_c = 0}
(gdb) p &pVarA
$2 = (CVariable *) 0x7fffffffdf70
(gdb) p &pVarA.m_a
$3 = (int *) 0x7fffffffdf70
(gdb) p &pVarA.m_b
$4 = (int *) 0x7fffffffdf74
(gdb) p &pVarA.m_c
$5 = (int *) 0x7fffffffdf78
</pre>


**3) 只有虚函数的类**

![cpp-obj-model](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_object_model_figure3.jpg)

<pre>
int nVFuntionSize = sizeof(CVFuction) = 8（虚表指针）
</pre>

我们通过如下程序测试：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

class CVFunction{
public:
   virtual void f(){ printf("f()\n"); }
   virtual void g(){ printf("g()\n"); }
   virtual void h(){ printf("h()\n"); }
};

int main(int argc,char *argv[])
{
   CVFunction pVFunc;
   uintptr_t **p = (uintptr_t **)&pVFunc;

   typedef void (*pvirtualfunc)(CVFunction *);

   printf("sizeof(CVFuncation): %d\n", sizeof(CVFunction));

   pvirtualfunc func1 = (pvirtualfunc)**p;
   func1(&pVFunc);

   pvirtualfunc func2 = (pvirtualfunc)(*(*p + 1));
   func2(&pVFunc);

   pvirtualfunc func3 = (pvirtualfunc)(*(*p + 2));
   func3(&pVFunc);
  
   return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -g -o test test.cpp -lstdc++
# ./test
sizeof(CVFuncation): 8
f()
g()
h()
</pre>


**4) 有成员变量、虚函数的类**

![cpp-obj-model](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_object_model_figure4.jpg)

<pre>
int nParentSize = sizeof(CParent) = 16 (找出最大长度的成员长度M（结构体的长度一定是该成员的整数倍））
</pre>

我们通过如下程序测试：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

class CParent{
private:
  int m_nAge;
public:
   virtual void f0(){ printf("f0()\n"); }
   virtual void g0(){ printf("g0()\n"); }
   virtual void h0(){ printf("h0()\n"); }
};

int main(int argc,char *argv[])
{
   CParent pParentA;
   uintptr_t **p = (uintptr_t **)&pParentA;
 
   typedef void (*pvirtualfunc)(CParent *);

   printf("sizeof(CParent): %d\n", sizeof(CParent));

   pvirtualfunc func1 = (pvirtualfunc)**p;
   func1(&pParentA);

   pvirtualfunc func2 = (pvirtualfunc)(*(*p + 1));
   func2(&pParentA);

   pvirtualfunc func3 = (pvirtualfunc)(*(*p + 2));
   func3(&pParentA);
  
   return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -g -o test test.cpp -lstdc++
# ./test
sizeof(CParent): 16
f0()
g0()
h0()
</pre>

### 3.2 单一继承

这里介绍一下```单一继承```情况下的内存模型（含成员变量、虚函数、虚函数覆盖）：

![cpp-obj-model](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_object_model_figure5.jpg)

<pre>
int nChildSize = sizeof(CChildren) = 16
</pre>

我们通过如下程序进行测试：
{% highlight string %}
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

class CParent{
private:
  int m_nAge;
public:
   virtual void f0(){ printf("f0()\n"); }
   virtual void g0(){ printf("g0()\n"); }
   virtual void h0(){ printf("h0()\n"); }
};

class CChildren : public CParent{
private:
   int m_nChildren;

public:
   virtual void f0(){ printf("children f0()\n"); }
   virtual void g1(){ printf("g1()\n"); }
   virtual void h0(){ printf("children h0()\n"); }
};

int main(int argc,char *argv[])
{
   CChildren pChildrenA;
   uintptr_t **p = (uintptr_t **)&pChildrenA;
 
   typedef void (*pvirtualfunc)(CChildren *);

   printf("sizeof(CChildren): %d\n", sizeof(CChildren));

   pvirtualfunc func1 = (pvirtualfunc)**p;
   func1(&pChildrenA);

   pvirtualfunc func2 = (pvirtualfunc)(*(*p + 1));
   func2(&pChildrenA);

   pvirtualfunc func3 = (pvirtualfunc)(*(*p + 2));
   func3(&pChildrenA);

   pvirtualfunc func4 = (pvirtualfunc)(*(*p + 3));
   func4(&pChildrenA);
  
   return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -g -o test test.cpp -lstdc++
# ./test
sizeof(CChildren): 16
children f0()
g0()
children h0()
g1()
</pre>


## 3.2 多继承

这里介绍一下```多继承```情况下的内存模型（含成员变量、虚函数、虚函数覆盖）：

![cpp-obj-model](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_object_model_figure6.jpg)

<pre>
int nChildSize = sizeof(CChildren) = 32
</pre>
我们通过如下程序进行测试：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

class CParent1{
private:
  int m_nParent1;
public:
   virtual void f0(){ printf("f0()\n"); }
   virtual void g0(){ printf("g0()\n"); }
   virtual void h0(){ printf("h0()\n"); }
};

class CParent2{
private:
   int m_nParent2;

public:
   virtual void f1(){ printf("f1()\n"); }
   virtual void g1(){ printf("g1()\n"); }
   virtual void h1(){ printf("h1()\n"); }
};

class CChildren : public CParent1, public CParent2{
private:
   int m_nChildren;

public:
   virtual void f0(){ printf("children f0()\n"); }
   virtual void g1(){ printf("children g1()\n"); }
   virtual void f2(){ printf("children f2()\n"); }

   virtual void h0(){ printf("children h0()\n"); }
   virtual void h1(){ printf("children h1()\n"); }
   virtual void h2(){ printf("children h2()\n"); }
};

int main(int argc,char *argv[])
{
   CChildren pChildrenA;
   uintptr_t **p = (uintptr_t **)&pChildrenA;
 
   typedef void (*pvirtualfunc)(CChildren *);

   printf("sizeof(CChildren): %d\n", sizeof(CChildren));

   pvirtualfunc func1 = (pvirtualfunc)**p;
   func1(&pChildrenA);

   pvirtualfunc func2 = (pvirtualfunc)(*(*p + 1));
   func2(&pChildrenA);

   pvirtualfunc func3 = (pvirtualfunc)(*(*p + 2));
   func3(&pChildrenA);

   pvirtualfunc func4 = (pvirtualfunc)(*(*p + 3));
   func4(&pChildrenA);

   pvirtualfunc func5 = (pvirtualfunc)(*(*p + 4));
   func5(&pChildrenA);
  
   pvirtualfunc func6 = (pvirtualfunc)(*(*p + 5));
   func6(&pChildrenA);

   pvirtualfunc func7 = (pvirtualfunc)(*(*p + 6));
   func7(&pChildrenA);

   printf("===============================\n");
   char *q = (char *)&pChildrenA;
   q = q + 8 + 4 +4;
   p = (uintptr_t **)q;

   pvirtualfunc func8 = (pvirtualfunc)**p;
   func8(&pChildrenA);

   return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
sizeof(CChildren): 32
children f0()
g0()
children h0()
children g1()
children f2()
children h1()
children h2()
===============================
f1()
</pre>

### 3.3 深度为2的继承

这里介绍一下```深度为2的继承```情况下的内存模型（含成员变量、虚函数、虚函数覆盖）：

![cpp-obj-model](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_object_model_figure7.jpg)
<pre>
int nGrandSize = sizeof(CGrandChildren) = 40
</pre>

我们通过如下程序进行测试：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

class CParent1{
private:
  int m_nParent1;
public:
   virtual void f0(){ printf("f0()\n"); }
   virtual void g0(){ printf("g0()\n"); }
   virtual void h0(){ printf("h0()\n"); }
};

class CParent2{
private:
   int m_nParent2;

public:
   virtual void f1(){ printf("f1()\n"); }
   virtual void g1(){ printf("g1()\n"); }
   virtual void h1(){ printf("h1()\n"); }
};

class CChildren : public CParent1, public CParent2{
private:
   int m_nChildren;

public:
   virtual void f0(){ printf("children f0()\n"); }
   virtual void g1(){ printf("children g1()\n"); }
   virtual void f2(){ printf("children f2()\n"); }

   virtual void h0(){ printf("children h0()\n"); }
   virtual void h1(){ printf("children h1()\n"); }
   virtual void h2(){ printf("children h2()\n"); }
};

class CGrandChildren: public CChildren{
private:
    int m_nGrandChildren;

public:
   virtual void f0(){ printf("grand children f0()\n"); }
   virtual void h1(){ printf("grand children h1()\n"); }
   virtual void f2(){ printf("grand children f2()\n"); }
   virtual void f3(){ printf("grand children f3()\n"); }
};

int main(int argc,char *argv[])
{
   CGrandChildren pGrandChildrenA;
   uintptr_t **p = (uintptr_t **)&pGrandChildrenA;
 
   typedef void (*pvirtualfunc)(CGrandChildren *);

   printf("sizeof(CGrandChildren): %d\n", sizeof(CGrandChildren));

   pvirtualfunc func1 = (pvirtualfunc)**p;
   func1(&pGrandChildrenA);
   
   pvirtualfunc func2 = (pvirtualfunc)(*(*p + 1));
   func2(&pGrandChildrenA);
   
   pvirtualfunc func3 = (pvirtualfunc)(*(*p + 2));
   func3(&pGrandChildrenA);

   pvirtualfunc func4 = (pvirtualfunc)(*(*p + 3));
   func4(&pGrandChildrenA);

   pvirtualfunc func5 = (pvirtualfunc)(*(*p + 4));
   func5(&pGrandChildrenA);

   pvirtualfunc func6 = (pvirtualfunc)(*(*p + 5));
   func6(&pGrandChildrenA);

   pvirtualfunc func7 = (pvirtualfunc)(*(*p + 6));
   func7(&pGrandChildrenA);

   pvirtualfunc func8 = (pvirtualfunc)(*(*p + 7));
   func8(&pGrandChildrenA);

   printf("=============================\n");

   char *q = (char *)&pGrandChildrenA;
   q = q + 8 + 4 + 4;

   p = (uintptr_t **)q;

   pvirtualfunc func9 = (pvirtualfunc)**p;
   func9(&pGrandChildrenA);

   return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
sizeof(CGrandChildren): 40
grand children f0()
g0()
children h0()
children g1()
grand children f2()
grand children h1()
children h2()
grand children f3()
=============================
f1()
</pre>

<br />
<br />

**[参考]**

1. [C++继承时的对象内存模型](https://www.cnblogs.com/haoyul/p/7287719.html)

2. [虚函数的实现的基本原理](https://www.cnblogs.com/malecrab/p/5572730.html) 注： 本文在内存模型方面存在一定错误

3. [C++获取类中成员函数的函数指针](https://blog.csdn.net/tingzhushaohua/article/details/76512298)

<br />
<br />
<br />





