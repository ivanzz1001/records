---
layout: post
title: C++内存管理
tags:
- cplusplus
categories: cplusplus
description: C++内存管理
---

本文主要介绍一下C++的内存管理机制。


<!-- more -->


## 1. C++内存管理
### 1.1 C++内存存储区划分
在C++中，内存主要分为5个存储区：

* 栈(Stack): 局部变量，函数参数等存储在该区，由编译器自动分配和释放，栈属于计算机系统的数据结构。进栈出栈有相应的计算机指令支持，而且分配专门的寄存器存储栈的地址，效率很高，内存空间是连续的，但栈的内存空间有限。

* 堆(Heap):需要程序员手动分配和释放(new、delete），属于动态分配方式。内存空间几乎没有限制，内存空间不连续，因此会产生内存碎片。

* 全局/静态存储区：全局变量、静态变量分配到该区，到程序结束时自动释放，包括```DATA段```和```BSS段```。其中，```初始化```的全局变量和静态变量存放在```DATA```段，未初始化的全局变量和静态变量存放在```BSS```段。BSS段特点： 在程序执行前BSS段自动清零，所以未初始化的全局变量和静态变量在程序执行前已经为0了。

* 文字常量区： 存放常量，而且不允许修改。程序结束后由系统释放（即```rodata```段）

* 代码段： 存放程序的二进制代码



### 1.2 使用存储区的三种方式

1) **静态存储区(Static Memory)**

全局变量、静态变量及静态类成员存储在该区，在编译期间就进行分配，生存期到程序结束。存储在该区的对象只初始化一次，且在程序运行期间地址固定不变。

2） **自动存储区(Automatic Memory)**

局部变量，函数参数等存储在该区，由编译器自动分配和释放。

3） **自由存储区(Free Store)**

由程序员手动分配和释放内存(new/delete)


### 1.3 堆和栈的区别
1) **空间大小：** 栈的内存空间是连续的，空间大小通常是系统预先规定好的，即栈顶地址和最大空间是确定的，通常大小为8KB； 而堆的内存空间是不连续的，由一个记录空间的链表负责管理，因此内存空间几乎没有限制，在32位系统下，内存空间的大小可达4GB。

2） **管理方式：** 栈由编译器自动分配和释放，而堆需要程序员来手动分配和释放，若忘记delete，容易产生内存泄露。

3） **生长方向不同：** 对于栈，通常是向着地址减少的方向生长的，而堆是向着内存地址增大的方向生长
堆和栈的区别：

4） **碎片问题：** 由于栈的内存空间是连续的，先进后出的方式保证不会产生零碎的空间； 而堆分配是每次在空闲链表中查找符合分配条件的内存来分配，每次分配的空间的大小一般不会正好等于要申请的内存大小，频繁的new/delete操作势必会产生大量的空间碎片。

5） **分配效率：** 栈属于机器系统提供的数据结构，计算机会在底层对栈提供支持，出栈进栈由专门的指令执行，因此效率较高； 而堆是C/C++函数库提供的，当申请空间时需要按照一定的算法搜索足够大小的内存空间，当没有足够的空间时，还需要额外的处理，因此效率较低。


## 2. C++ 智能指针的使用与实现

C++的智能指针其实就是对普通指针的封装（即封装成一个类），通过重载```*```和```->```两个运算符，使得智能指针表现的就像普通指针一样。

C++程序设计中使用堆内存是非常频繁的操作，堆内存的申请和释放都由程序员自己管理。程序员自己管理堆内存可以提高程序的效率，但是整体来说堆内存的管理是很麻烦的，C++11中引入了智能指针的概念，方便管理堆内存。使用普通指针，容易造成堆内存泄露（忘记释放）、二次释放、程序发生异常时内存泄露等问题，使用智能指针能更好的管理堆内存。


### 2.1 智能指针的实现原理
智能指针(smart pointer)的通用实现技术是使用引用计数(reference count)。智能指针类将一个计数器与类指向的对象相关联，引用计数跟踪该类有多少个对象的指针指向同一对象。每次创建类的新对象时，初始化指针就将引用计数置为1； 当对象作为另一对象的副本而创建时，拷贝构造函数拷贝指针并增加与之相应的引用计数； 对一个对象进行赋值时，赋值操作符减少左操作数所指对象的引用计数（如果引用计数减至0，则删除对象），并增加右操作数所指对象的引用计数； 调用析构函数时，析构函数减少引用计数（如果引用计数减至0，则删除基础对象）。


### 2.2 C++标准库中的智能指针
智能指针在C++11版本之后提供，包含在头文件```<memory>```中，shared_ptr、unique_ptr、weak_ptr：

* **shared_ptr**

利用引用计数，每有一个指针指向相同的一片内存时，引用计数+1，每当一个指针取消指向一片内存时，引用计数-1， 当引用计数减为0时释放内存。参看如下示例(shared_ptr.cpp)：
{% highlight string %}
#include <iostream>
#include <memory>


int main(int argc, char *argv[])
{
    int a = 10;

    std::shared_ptr<int> ptra = std::make_shared<int>(a);
    std::shared_ptr<int> ptra2(ptra);    //copy construct
    std::cout<<ptra.use_count()<<std::endl;


    int b = 20;
    int *pb = &b;
    //std::shared_ptr<int> ptrb = pb;       //error
    std::shared_ptr<int> ptrb = std::make_shared<int>(b);
    ptra2 = ptrb;
    pb = ptrb.get();        //获取原始指针

    std::cout<<ptra.use_count()<<std::endl;
    std::cout<<ptrb.use_count()<<std::endl;

    return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# ./shared_ptr 
2
1
2
</pre>


* **weak_ptr**

```weak_ptr```用于辅助shared_ptr来解决循环引用的问题。weak_ptr是为了配合shared_ptr而引入的一种智能指针，因为它不具有普通指针的行为，没有重载```operator*```和```operator->```，它的最大作用在于协助shared_ptr工作，像旁观者那样观测资源的使用情况。weak_ptr可以从一个```shared_ptr```或者另一个```weak_ptr```对象来构造，获得资源的观测权。但weak_ptr没有共享资源，它的构造函数不会引起指针引用计数的增加。

使用```weak_ptr```的成员函数use_count()可以观测资源的引用计数； 另一个成员函数expired()的功能等价于```user_count()==0```，但更快，表示被观测的资源（也就是shared_ptr管理的资源）已经不复存在。weak_ptr可以使用一个非常重要的成员函数**lock()**从被观测的shared_ptr获得一个可用的shared_ptr对象，从而操作资源。但当**expired()==true**的时候，lock()函数将返回一个存储空指针的shared_ptr。参看如下示例（weak_ptr.cpp):
{% highlight string %}
#include <iostream>
#include <memory>

int main(int argc, char *argv[])
{
    std::shared_ptr<int> shPtr = std::make_shared<int>(10);
    std::cout<<shPtr.use_count()<<std::endl;

    std::weak_ptr<int> wkPtr(shPtr);
    std::cout<<wkPtr.use_count()<<std::endl;

    if(!wkPtr.expired())
    {
        std::shared_ptr<int> shPtr2 = wkPtr.lock();
        *shPtr2 = 100;

        std::cout<<shPtr2.use_count()<<std::endl;
        std::cout<<wkPtr.use_count()<<std::endl;
        std::cout<<"value: "<<*shPtr<<std::endl;
    }

    return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o weak_ptr weak_ptr.cpp -std=c++11 -lstdc++
# ./weak_ptr 
1
1
2
2
value: 100
</pre>

* **unique_ptr**

unique_ptr```唯一```拥有其所指对象，同一时刻只能有一个unique_ptr指向给定对象（禁止拷贝、赋值），可以释放所有权，转移所有权.


### 2.3 智能指正实现示例
如下是一个线程安全的智能指针实现示例(smart_point.cpp)：
{% highlight string %}
/*************************************************************************
        > Motto : Be better!
        > Author: ShadowsGtt 
        > Mail  : 1025814447@qq.com
        > Time  : 2018年07月24日 星期二 16时32分26秒
 ************************************************************************/
 
#include<iostream>
#include<mutex>
using namespace std;
 
/*  实现一个线程安全的智能指针 */
 
 
/* 引用计数基类 */
class Sp_counter
{
    private :
        size_t *_count;
        std::mutex mt;
    public :
        Sp_counter()
        {
            cout<<"父类构造,分配counter内存"<<endl;
            _count = new size_t(0);
        }
        virtual ~Sp_counter()
        {
            if(_count && !(*_count) ){
                cout<<"父类析构"<<endl;
                cout<<"[释放counter内存]"<<endl;
                delete _count;
                _count = NULL;
            }
        }
    Sp_counter &operator=(Sp_counter &spc)
    {
        cout<<"父类重载="<<endl;
        cout<<"[释放counter内存]"<<endl;
        delete _count;
        this->_count = spc._count;
        return *this;
    }
    Sp_counter &GetCounter()
    {
        return *this;
    }
    size_t Get_Reference()
    {
        return *_count;
    }
    virtual void Increase()
    {
        mt.lock();
        (*_count)++;
        //cout<<"_count++:"<<*_count<<endl;
        mt.unlock();
    }
    virtual void Decrease()
    {
            mt.lock();
            (*_count)--;
            //cout<<"_count--:"<<*_count<<endl;
            mt.unlock();
        }
};
 
template<typename T>
class smart_pointer : public Sp_counter
{
    private :
        T *_ptr;
    public :
        smart_pointer(T *ptr = NULL);               
        ~smart_pointer();                               
        smart_pointer(smart_pointer<T> &);
        smart_pointer<T> &operator=(smart_pointer<T> &);
        T &operator*();
        T *operator->(void);
        size_t use_count();
        
};
 
 
/* 子类参构造函数&带参数构造函数 */
template<typename T>
inline smart_pointer<T>::smart_pointer(T *ptr)
{
    if(ptr){
        cout<<"子类默认构造"<<endl;
        _ptr = ptr;
        this->Increase();
    }
}   
 
 
/* 子类析构函数 */
template<typename T>
smart_pointer<T>::~smart_pointer()
{
    /* 指针非空才析构 */
    if(this->_ptr){
        cout<<"子类析构,计数减1"<<endl;
        if(this->Get_Reference())
            this->Decrease();
        if(!(this->Get_Reference()) ){
            cout<<"(((子类析构,主内存被释放)))"<<endl;
            delete _ptr;
            _ptr = NULL;
        }
    }
}
 
/* 得到引用计数值 */
template<typename T>
inline size_t smart_pointer<T>::use_count()
{
    return this->Get_Reference();
}
 
/* 拷贝构造 */
template<typename T>
inline smart_pointer<T>::smart_pointer(smart_pointer<T> &sp)
{
    cout<<"子类拷贝构造"<<endl;
 
    /* 防止自己对自己的拷贝 */
    if(this != &sp){
        this->_ptr = sp._ptr;
        this->GetCounter() = sp.GetCounter();
        this->Increase();
    }
    
}
/* 赋值构造 */
template<typename T>
inline smart_pointer<T> &smart_pointer<T>::operator=(smart_pointer<T> &sp)
{
 
    /* 防止自己对自己的赋值以及指向相同内存单元的赋值 */
    if(this != &sp){
 
        cout<<"赋值构造"<<endl;
 
        /* 如果不是构造一个新智能指针并且两个只能指针不是指向同一内存单元 */
        /* =左边引用计数减1,=右边引用计数加1 */
        if(this->_ptr && this->_ptr != sp._ptr){
            this->Decrease();
 
            /* 引用计数为0时 */
            if(!this->Get_Reference()){
                cout<<"引用计数为0,主动调用析构"<<endl;
                this->~smart_pointer();
                //this->~Sp_counter();
                cout<<"调用完毕"<<endl;
            }
        }
 
        this->_ptr = sp._ptr;
        this->GetCounter() = sp.GetCounter();
        this->Increase();
    }
    return *this;
}
 
/* 重载解引用*运算符 */
template<typename T>
inline T &smart_pointer<T>::operator*()
{
    return *(this->_ptr);
}
template<typename T>
inline T *smart_pointer<T>::operator->(void)
{
    return this->_ptr;
}
 
int main()
{
    int *a = new int(10);
    int *b = new int(20);
    cout<<"-------------默认构造测试----------------->"<<endl;
    cout<<"构造sp"<<endl;
    smart_pointer<int> sp(a);
    cout<<"sp.use_count:"<<sp.use_count()<<endl;
    cout<<"------------------------------------------>"<<endl<<endl;
    
    {
        cout<<"-------------拷贝构造测试----------------->"<<endl;
        cout<<"构造sp1  :sp1(sp)"<<endl;
        smart_pointer<int> sp1(sp);
        cout<<"构造sp2  :sp2(sp)"<<endl;
        smart_pointer<int> sp2(sp1);
        cout<<"sp1和sp2引用计数为3才是正确的"<<endl;
        cout<<"sp1.use_count:"<<sp1.use_count()<<endl;
        cout<<"sp2.use_count:"<<sp2.use_count()<<endl;
        cout<<"------------------------------------------>"<<endl<<endl;
        cout<<"调用析构释放sp1,sp2"<<endl;
    }
    cout<<"-------------析构函数测试----------------->"<<endl;
    cout<<"此处sp.use_count应该为1才是正确的"<<endl;
    cout<<"sp.use_count:"<<sp.use_count()<<endl;
    cout<<"------------------------------------------>"<<endl<<endl;
    
    cout<<"-------------赋值构造测试----------------->"<<endl;
    cout<<"构造sp3  :sp3(b)"<<endl;
    smart_pointer<int> sp3(b);
    cout<<"sp3.use_count:"<<sp3.use_count()<<endl;
    cout<<"sp3 = sp"<<endl;
    sp3 = sp;
    cout<<"sp3先被释放,然后sp3引用计数为2才正确,sp的引用计数为2才正确"<<endl;
    cout<<"sp3.use_count:"<<sp3.use_count()<<endl;
    cout<<"sp.use_count :"<<sp.use_count()<<endl;
    cout<<"------------------------------------------>"<<endl<<endl;
    
    cout<<"-------------解引用测试----------------->"<<endl;
    cout<<"*sp3:"<<*sp3<<endl;
    cout<<"*sp3 = 100"<<endl;
    *sp3 = 100;
    cout<<"*sp3:"<<*sp3<<endl;
    cout<<"------------------------------------------>"<<endl;
 
    
 
   // cout<<"sp3.use_count:"<<sp3.use_count()<<endl;
    //cout<<"sp.use_count:"<<sp.use_count()<<endl;
 
 
    cout<<"===================end main===================="<<endl;
    return 0;
}
{% endhighlight %}

>注：上面拷贝构造函数一般可以不用添加防止自拷贝

编译运行：
<pre>
# gcc -o smart_pointer smart_pointer.cpp -std=c++11 -lstdc++
# ./smart_pointer 
-------------默认构造测试-----------------
构造sp
父类构造,分配counter内存
子类默认构造
sp.use_count:1
------------------------------------------

-------------拷贝构造测试-----------------
构造sp1  :sp1(sp)
父类构造,分配counter内存
子类拷贝构造
父类重载=
[释放counter内存]
构造sp2  :sp2(sp)
父类构造,分配counter内存
子类拷贝构造
父类重载=
[释放counter内存]
sp1和sp2引用计数为3才是正确的
sp1.use_count:3
sp2.use_count:3
------------------------------------------

调用析构释放sp1,sp2
子类析构,计数减1
子类析构,计数减1
-------------析构函数测试-----------------
此处sp.use_count应该为1才是正确的
sp.use_count:1
------------------------------------------

-------------赋值构造测试-----------------
构造sp3  :sp3(b)
父类构造,分配counter内存
子类默认构造
sp3.use_count:1
sp3 = sp
赋值构造
引用计数为0,主动调用析构
子类析构,计数减1
(((子类析构,主内存被释放)))
父类析构
[释放counter内存]
调用完毕
父类重载=
[释放counter内存]
sp3先被释放,然后sp3引用计数为2才正确,sp的引用计数为2才正确
sp3.use_count:2
sp.use_count :2
------------------------------------------

-------------解引用测试-----------------
*sp3:10
*sp3 = 100
*sp3:100
------------------------------------------
===================end main====================
子类析构,计数减1
子类析构,计数减1
(((子类析构,主内存被释放)))
父类析构
[释放counter内存]
</pre>


<br />
<br />

**[参看]:**

1. [c++内存管理](https://www.cnblogs.com/mrlsx/p/5411874.html)

2. [C++内存管理（超长，例子很详细，排版很好）](https://blog.csdn.net/caogenwangbaoqiang/article/details/79788368)


<br />
<br />
<br />





