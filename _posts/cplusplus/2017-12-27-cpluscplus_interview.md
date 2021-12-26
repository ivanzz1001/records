---
layout: post
title: C/C++笔试面试（转)
tags:
- cplusplus
categories: cplusplus
description: C/C++笔试面试（转)
---

本文主要记录一下C/C++语言常见的一些笔试面试知识点。


<!-- more -->


## 1. new与malloc的10点区别

**1) 申请的内存所在位置**

new操作符从```自由存储区```(free store)上为对象动态分配内存空间，而malloc函数从```堆```上动态分配内存。自由存储区是C++基于new操作符的一个抽象概念，凡是通过new操作符进行内存申请，该内存即为自由存储区。而堆是操作系统中的术语，是操作系统所维护的一块特殊内存，用于程序的内存动态分配， C语言使用malloc从堆上分配内存，使用free释放已分配的对应内存。

那么自由存储区是否能够是堆（问题等价于new是否能够在堆上动态分配内存），这取决于operator new的实现细节。自由存储区不仅可以是堆，也可以是静态存储区，这都看operator new在哪里为对象分配内存。

特别地，new甚至可以不为对象分配内存！ ```定位new```的功能可以办到这一点：
{% highlight string %}
new (place_address) type
{% endhighlight %}
上面place_address是一个指针，代表一块内存地址。当使用上面这种仅以一个地址调用new操作符时，new操作符调用特殊的operator new，也就是下面这个版本：
{% highlight string %}
void *operator new(size_t, void *);   //不允许重定义这个版本的operator new
{% endhighlight %}

这个operator new不分配任何内存，它只是简单地返回指针实参，然后右new表达式负责在```place_address```指定的地址进行对象的初始化工作。


**2) 返回类型安全性**

new操作符内存分配成功时，返回的是对象类型的指针，类型严格与对象匹配，无需进行类型转换，故new是符合```类型安全性```的操作符。而malloc内存分配成功则是返回```void *```，需要通过强制类型转换将```void *```指针转换成我们需要的类型。**类型安全很大程度上可以等价于内存安全，类型安全的代码不会试图访问自己没被授权的内存区域**。关于C++的类型安全可说的又有很多了。

**3) 内存分配失败时的返回值**

new内存分配失败时，会抛出bad_alloc异常，它**不会**返回```NULL```； malloc分配内存失败时返回```NULL```。在使用C语言时，我们习惯在malloc分配内存后判断分配是否成功： 
{% highlight string %}
int *a = malloc(sizeof(int));
if(NULL == a){
	...
}else{
	...
}
{% endhighlight %}

从C语言走入C++阵营的新手可能会把这个习惯带入C++:
<pre>
int *a = new int();
if(NULL == a){
	...
}else{
	...
}
</pre>
实际上这样做一点意义也没有，因为new根本不会返回NULL，而且程序能够执行到```if```语句已经说明内存分配成功了，如果失败早就抛出异常了。正确的做法应该是使用异常机制：
{% highlight string %}
try{
	int *a = new int();
}catch(bad_alloc){
	...
}
{% endhighlight %}
如果想顺便了解下异常基础，可以看[C++ 异常机制分析](http://www.cnblogs.com/QG-whz/p/5136883.html)


**4) 是否需要指定内存大小**

使用new操作符申请内存分配时无须指定内存块的大小，编译器会根据类型信息自行计算，而malloc则需要显式地指出所需内存的尺寸：
{% highlight string %}
class A{...};

A *ptr = new A;

A *ptr = (A *)malloc(sizeof(A));	//需要显式指定所需内存大小sizeof(A)
{% endhighlight %}
当然了，我这里使用malloc来为我们自定义类型分配内存是不怎么合适的，请看下一条。


**5) 是否调用构造函数/析构函数**

使用new操作符来分配对象内存时会经历3个步骤：
<pre>
1. 调用operator new函数(对于数组是operator new[])分配一块足够大的，原始的，未命名的内存空间以便存储特定
   类型的对象

2. 编译器运行相应的构造函数以构造对象, 并为其传入初值

3. 对象构造完成后，返回一个指向该对象的指针
</pre>

使用```delete```操作符来释放对象内存时会经历两个步骤：
<pre>
1. 调用对象的析构函数

2. 编译器调用operator delete(或operator delete[])函数释放内存空间
</pre>

总之来说，new/delete会调用对象的构造函数/析构函数以完成对象的构造/析构，而malloc则不会。参看如下例子(test.cpp)：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

class A{
public:
	A():a(1),b(1.1){}

private:
	int a;
	double b;
};

int main(int argc,char *argv[])
{
	A *ptr = (A *)malloc(sizeof(A));

	return 0x0;
}
{% endhighlight %}
编译，然后我们在```return```处设置断点调试运行：
{% highlight string %}
# gcc -g -o test test.cpp -lstdc++
# gdb ./test
(gdb) b test.cpp:17
Breakpoint 1 at 0x4005f3: file test.cpp, line 17.
(gdb) r
Starting program: /data/home/lzy/just_for_test/./test 
Breakpoint 1, main (argc=1, argv=0x7fffffffe088) at test.cpp:17
17          return 0x0;
Missing separate debuginfos, use: debuginfo-install glibc-2.17-157.el7_3.2.x86_64
(gdb) p ptr
$1 = (A *) 0x602010
(gdb) p ptr->a
$2 = 0
(gdb) p ptr->b
$3 = 0
{% endhighlight %}
从上面可以看出，A的默认构造函数并没有被调用，因为数据成员a、b的值并没有得到初始化，这也是上面我为什么说使用malloc/free来处理C++的自定义类型不合适。其实不止自定义类型，标准库中凡是需要构造/析构的类型通通不合适。

**6） 对数组的处理**

C++提供了```new[]```与```delete[]```来专门处理数组类型：
<pre>
A *ptr = new A[10];		//分配10个A对象
</pre>
使用```new[]```分配的内存必须使用```delete[]```进行释放：
<pre>
delete []ptr;
</pre>

new对数组的支持体现在它会分别调用构造函数初始化每一个数组元素，释放对象时为每个对象调用析构函数。注意```delete[]```要与```new[]```配套使用，不然会出现数组对象部分释放的现象，造成内存泄露。

至于malloc，它并不知道你在这块内存上要放的是数组还是啥别的东西，反正它就给你一块原始的内存，再给你个内存地址就完事。所以如果要动态分配一个数组的内存，还需要我们手动指定数组的大小：
<pre>
int *ptr = (int *)malloc(sizeof(int)*10);		//分配一个10个int元素的数组
</pre>

**7) new与malloc是否可以相互调用**

```operator new、operator delete```的实现可以基于malloc，而malloc的实现不可以去调用```new```。下面是编写operator new/operator delete的一种简单方式，其他版本也与之类似：
{% highlight string %}
void *operator new(size_t size){
	void *mem = malloc(size);
	if(mem)
		return mem;
	else
		throw bad_alloc();
}

void operator delete(void *mem) noexcept{
	free(mem);
}
{% endhighlight %}

**8) 是否可以被重载**

operator new/operator delete可以被重载。标准库(C++11)是定义了operator new函数和operator delete函数的12个重载版本：
{% highlight string %}
throwing (1)	void* operator new (std::size_t size);
nothrow (2)	    void* operator new (std::size_t size, const std::nothrow_t& nothrow_value) noexcept;
placement (3)	void* operator new (std::size_t size, void* ptr) noexcept;


ordinary (1)	void operator delete (void* ptr) noexcept;
nothrow (2)	    void operator delete (void* ptr, const std::nothrow_t& nothrow_constant) noexcept;
placement (3)	void operator delete (void* ptr, void* voidptr2) noexcept;


throwing (1)	void* operator new[] (std::size_t size);
nothrow (2)	    void* operator new[] (std::size_t size, const std::nothrow_t& nothrow_value) noexcept;
placement (3)	void* operator new[] (std::size_t size, void* ptr) noexcept;


ordinary (1)	void operator delete[] (void* ptr) noexcept;
nothrow (2)	    void operator delete[] (void* ptr, const std::nothrow_t& nothrow_constant) noexcept;
placement (3)	void operator delete[] (void* ptr, void* voidptr2) noexcept;
{% endhighlight %}
我们可以自定义上面函数版本中的任意一个，前提是自定义版本必须位于```全局作用域```或者```类作用域```中。太细节的东西不在这里讲述。总之，我们知道我们有足够的自由去重载operator new/operator delete，以决定我们的new与delete如何为对象分配内存，如何回收对象。而```malloc/free```并不允许重载。


**9) 能够直观地重新分配内存**

使用malloc分配的内存后，如果在使用过程中发现内存不足，可以使用realloc函数进行内存重新分配实现内存的扩充。realloc先判断当前的指针所指内存是否有足够的连续空间，如果有，原地扩大可分配的内存地址，并返回原来的地址指针；如果空间不够，先按照新指定的大小分配空间，然后将原有数据从头到尾拷贝到新分配的内存区域，接着释放原来的内存区域。

new没有这样直观的配套设施来扩充内存。

**10） 客户处理内存分配不足**

在operator new抛出异常以反映一个未获得满足的需求之前，它会先调用一个用户指定的错误处理函数，这就是new-handler。new_handler是一个指针类型:
{% highlight string %}
namespace std{
	typedef void (*new_handler)();
}
{% endhighlight %}
指向了一个没有参数没有返回值的函数，即为错误处理函数。为了指定错误处理函数，客户需要调用```set_new_handler```，这是一个声明于标准库的函数：
{% highlight string %}
namespace std{
	new_handler set_new_handler(new_handler p) throw();
}
{% endhighlight %}
```set_new_handler```的参数为new_handler指针，指向了operator new无法分配足够内存时该调用的函数。其返回值也是个指针，指向set_new_handler()函数被调用前正在执行（但马上就要发生替换）的那个new_handler函数。

对于malloc，客户并不能够去编程决定内存不足以分配时要干什么事，只能看着malloc返回NULL。

下面我们将上述的10点差异整理成表格：

![cpp-new-malloc](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_new_malloc.jpg)

## 2. C++虚函数与纯虚函数的区别

* 虚函数和纯虚函数可以定义在同一个类中，含有纯虚函数的类被称为抽象类（abstract class)，而只含有虚函数的类(class)不能被称为抽象类。

* 虚函数可以被直接使用，也可以被子类(subclass)重载以后以多态的形式调用，而纯虚函数必须在子类(subclass)中实现该函数才可以使用，因为纯虚函数在基类(base class)只有声明没有定义。

* 虚函数和纯虚函数都可以在子类(subclass)中被重载，以多态的形式被调用

* 虚函数和纯虚函数通常存在于抽象基类(abstract base class)之中，被继承的子类重载，目的是提供一个统一的接口

* 虚函数的定义形式为virtual {method body}, 而纯虚函数的定义形式为virtual {} = 0。在虚函数与纯虚函数中不能有static标识符，原因很简单，被static修饰的函数在编译时要求前期bind，然而虚函数却是动态绑定(runtime bind)，而且被两者修饰的函数生命周期(life cycle)也不一样。

* 虚函数必须实现，如果不实现，编译器将报错，错误提示为：
<pre>
error LNK****: unresolved external symbol "public: virtual void __thiscall
ClassName::virtualFunctionName(void)"
</pre> 

* 对于虚函数来说，父类和子类都有各自的版本，由多态方式调用时动态绑定

* 实现了纯虚函数的子类，该纯虚函数在子类中就变成了虚函数，子类的子类即孙子类可以覆盖该虚函数，有多态方式调用时候动态绑定

* 虚函数是C++中用于实现多态(polymorphism)的机制。核心理念就是通过基类访问派生类定义的函数

* 多态性指相同对象收到不同消息或不同对象收到相同消息时产生不同的实现动作。C++支持两种多态性： 编译时多态和运行时多态
<pre>
编译时多态性： 通过重载函数实现
运行时多态性： 通过虚函数实现
</pre>

* 如果一个类中含有纯虚函数，那么任何试图对该类实例化的语句都将导致错误产生，因为抽象基类是不能直接被调用的。必须被子类继承重载以后，根据要求调用其子类方法。


## 3. 基础知识

1) **C++中指针与引用的区别**

* 指针本身占用空间，存放的是变量的地址，引用只是变量的别名

* 指针可以为NULL，引用不可以为空

* 指针可以在初始化以后改变指向，引用则一旦初始化就不能改变

* 指针可以有二级操作(即```**p```)，引用无

* 操作指针指向的变量需要解引用(即```*p```)，而操作引用即可达到操作变量的目的

* 指针和引用自增操作的含义不同

2) **#define与const的区别**

* define定义的只是一个字串，没有类型，存储在代码段，编译器不能进行安全检查； 而const有类型，存储在数据段，能够进行安全类型检查。

* define不能够调试，const定义的变量可以

* define在预处理时期进行字串替换，const是在编译时进行


3) **const修饰指针变量**

* ```const int * a```: 这里const修饰的是int,表示指针a所指的变量的值不能被修改。

* ```int * const a```: 这里const修饰的是指针a，因此a在赋值完成之后，不能再进行修改指向其他的地址

* ```int const *a```: 与```const int *a```等价

*  ```const int * const a```: 代表a所指向的对象的值以及它的地址本身都不能被改变

* ```const int * const a```: 同上


4) **进程与线程的区别**

* 进程是资源分配最小单位，线程是程序执行的最小单位；

* 进程有自己独立的地址空间，每启动一个进程，系统都会为其分配地址空间，建立数据表来维护代码段、堆栈段和数据段，线程没有独立的地址空间，它使用相同的地址空间共享数据；

* CPU切换一个线程比切换进程花费小；

* 创建一个线程比进程开销小；

* 线程占用的资源要比进程少很多。

* 线程之间通信更方便，同一个进程下，线程共享全局变量，静态变量等数据，进程之间的通信需要以通信的方式（IPC）进行；（但多线程程序处理好同步与互斥是个难点）

* 多进程程序更安全，生命力更强，一个进程死掉不会对另一个进程造成影响（源于有独立的地址空间），多线程程序更不易维护，一个线程死掉，整个进程就死掉了（因为共享地址空间）；

* 进程对资源保护要求高，开销大，效率相对较低，线程资源保护要求不高，但开销小，效率高，可频繁切换

5） **为什么构造函数不能为虚函数，而析构函数可以为虚函数**

5.1） 构造函数为什么不能为虚函数？

* 存储空间角度： ```虚函数```的调用需要虚函数表指针，而该指针存放在对象的内容空间中，需要调用构造函数才可以创建它的值，否则即使开辟了空间，虚表指针值为随机，也不会找到构造函数； 若构造函数声明为虚函数，那么由于对象还未创建，还没有内存空间，更没有虚函数表地址用来调用虚函数——构造函数。

* 使用上： 从实现上看，vbtl在构造函数调用后才建立，因而构造函数不可能为虚函数。 虚函数主要用于实现多态，在运行时才可以明确调用对象，根据传入的对象类型来调用函数。当一个构造函数被调用时，它做的首要事情之一就是初始化它的```VPTR```。

5.2） 为什么析构函数可以使用虚函数？

对象已经创建，虚表指针存放虚析构函数的地址，基类与派生类都含有析构虚函数。创建基类与子类对象时，都含有各自的虚表指针。在销毁一个对象时，很多时候我们是通过基类的指针来进行的，这时候如果析构函数不是虚函数，就不能正确识别对象类型从而不能正确调用析构函数。

6） **常用的对称加密算法和非对称加密算法**

对称加密算法：DES、3DES、AES

非对称加密算法： RSA、Elgamal、背包算法、Rabin、D-H（Diffie-Hellman）、ECC(椭圆曲线加密算法)

7） **如下代码会有什么问题？**

编写如下代码(test.cpp):
{% highlight string %}
#include <iostream>
#include <string>
#include <string.h>

int main(int argc, char *argv[])
{
    std::string sString;

    memset(&sString, 0x0, sizeof(sString));

    std::cout<<"sString: "<<sString<<std::endl;

    return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
段错误
</pre>


<br />
<br />

**[参看]:**

1. [C/C++笔试面试（转)](http://blog.csdn.net/weiyuefei/article/details/52351712)

2. [细说new与malloc的10点区别](https://www.jianshu.com/p/e97320f90e22)

3. [new与malloc有什么区别](https://www.cnblogs.com/shilinnpu/p/8945637.html)

<br />
<br />
<br />





