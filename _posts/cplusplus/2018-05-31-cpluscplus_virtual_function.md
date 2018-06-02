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
    int **p = (int **)&a;
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
# gcc -o test2 test2.cpp -lstdc++ 
test2.cpp: In function ‘int main(int, char**)’:
test2.cpp:41:45: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
     pvirtual_func pvfunc = (pvirtual_func)**p;

# ./test2
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





<br />
<br />

**[参考]**

1. [C++继承时的对象内存模型](https://www.cnblogs.com/haoyul/p/7287719.html)

2. [虚函数的实现的基本原理](https://www.cnblogs.com/malecrab/p/5572730.html) 注： 本文在内存模型方面存在一定错误

3. [C++获取类中成员函数的函数指针](https://blog.csdn.net/tingzhushaohua/article/details/76512298)

<br />
<br />
<br />





