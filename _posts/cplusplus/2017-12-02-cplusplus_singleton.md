---
layout: post
title: C++单例的实现
tags:
- cplusplus
categories: cplusplus
description: C++单例的实现
---


在C++中要写一个正确的单例类实现其实也是有一定难度的，有很多过去在生产环境中被认为是正确的写法（特别是所谓的```double-checked locking```写法)，都被发现存在潜在的bug。本文记录一下正确的C++单例实现，以备查阅。



<!-- more -->

## 1. 动机
>保证一个类仅有一个实例，并提供一个该实例的全局访问点。 ——《设计模式》GoF

在软件系统中，经常有这样一些特殊的类，必须保证它们在系统中只存在一个实例，才能确保它们的逻辑正确性、以及良好的效率。

所以得考虑如何绕过常规的构造器(不允许使用者new出一个对象)，提供一种机制来保证一个类只有一个实例。

**应用场景**

* Windows的Task Manager(任务管理器）就是很典型的单例模式，你不能同时打开两个任务管理器。Windows的回收站也是同理

* 应用程序的日志应用，一般都可以用单例模式实现，只有一个实例去操作文件

* 读取配置文件，读取的配置项是公有的，一个地方读取了所有地方都能用，没有必要所有的地方都能读取一遍配置

* 数据库连接池，多线程的线程池。
 
## 2. 单例模式的实现

下面我们给出几种单例模式的实现，并对每一种实现做相应的讲解。

### 2.1 实现1： 线程不安全版本
教学版，即懒汉版（Lazy Singleton）：单例实例在第一次被使用时才进行初始化，这叫做延迟初始化。

{% highlight string %}
class Singleton{
public:
    static Singleton* getInstance(){
        // 先检查对象是否存在
        if (m_instance == nullptr) {
            m_instance = new Singleton();
        }
        return m_instance;
    }
private:
	Singleton();                        //私有构造函数，不允许使用者自己生成对象
	~Singleton();
	Singleton(const Singleton& other){}
	Singleton & operator=(const Singleton &other){}

    static Singleton* m_instance; //静态成员变量 
};

Singleton* Singleton::m_instance=nullptr;               //静态成员需要先初始化
{% endhighlight %}

这是单例模式最经典的实现方式，将构造函数和拷贝构造函数都设为私有的，而且采用了延迟初始化的方式，在第一次调用```getInstance()```的时候才会生成对象，不调用就不会生成对象，不占据内存。然而，在多线程的情况下，这种方法是不安全的。


分析：正常情况下，如果线程A调用```getInstance()```时，将m_instance 初始化了，那么线程B再调用getInstance()时，就不会再执行new了，直接返回之前构造好的对象。然而存在这种情况，线程A执行m_instance = new Singleton()还没完成，这个时候m_instance仍然为nullptr，线程B也正在执行m_instance = new Singleton()，这是就会产生两个对象，线程A和B可能使用的是同一个对象，也可能是两个对象，这样就可能导致程序错误。同时，还会发生内存泄漏。


### 2.2 实现2： 线程不安全版，解决内存泄露

Lazy Singleton存在内存泄露的问题，有两种解决方法：

* 使用智能指针

* 使用静态的嵌套类对象

对于第二种解决方法，代码如下：
{% highlight string %}
class Singleton{
public:
    static Singleton* getInstance(){
        // 先检查对象是否存在
        if (m_instance == nullptr) {
            m_instance = new Singleton();
        }
        return m_instance;
    }

private:
	class Deletor {
	public:
		~Deletor() {
			if(Singleton::instance != NULL)
				delete Singleton::instance;
		}
	};
	static Deletor deletor;

private:
	Singleton();                        //私有构造函数，不允许使用者自己生成对象
	~Singleton();
	Singleton(const Singleton& other);
	Singleton & operator=(const Singleton &other);

    static Singleton* m_instance; //静态成员变量 
};

Singleton* Singleton::m_instance=nullptr;               //静态成员需要先初始化
{% endhighlight %}

在程序运行结束时，系统会调用静态成员```deletor```的析构函数，该析构函数会删除单例的唯一实例。使用这种方法释放单例对象有以下特征：

* 在单例内部定义专有的嵌套类

* 在单例类内定义私有的专门用于释放的静态成员

* 利用程序在结束时析构全局变量的特性，选择最终的释放时机

在单例内再定义一个嵌套类，总是感觉很麻烦。


### 2.3 实现3： 线程安全，锁的代价过高
{% highlight string %}
//线程安全版本，但锁的代价过高
Singleton* Singleton::getInstance() {
    Lock lock; //伪代码 加锁
    if (m_instance == nullptr) {
        m_instance = new Singleton();
    }
    return m_instance;
}
{% endhighlight %}
分析：这种写法不会出现上面两个线程都执行new的情况，当线程A在执行m_instance = new Singleton()的时候，线程B如果调用了```getInstance()```，一定会被阻塞在加锁处，等待线程A执行结束后释放这个锁。从而是线程安全的。

但这种写法的性能不高，因为每次调用```getInstance()```都会加锁释放锁，而这个步骤只有在第一次new Singleton()才是有必要的，只要m_instance被创建出来了，不管多少线程同时访问，使用```if (m_instance == nullptr)``` 进行判断都是足够的（只是读操作，不需要加锁），没有线程安全问题，加了锁之后反而存在性能问题。

### 2.4 实现4： 双检查锁，由于内存读写reoder导致不安全

上面的做法是不管三七二十一，某个线程要访问的时候，先锁上再说，这样会导致不必要的锁的消耗，那么，是否可以先判断下if (m_instance == nullptr) 呢，如果满足，说明根本不需要锁啊！这就是所谓的双检查锁(DCL)的思想，DCL即double-checked locking。
{% highlight string %}
//双检查锁，但由于内存读写reorder不安全
Singleton* Singleton::getInstance() {
    //先判断是不是初始化了，如果初始化过，就再也不会使用锁了
    if(m_instance==nullptr){
        Lock lock; //伪代码
        if (m_instance == nullptr) {
            m_instance = new Singleton();
        }
    }
    return m_instance;
}
{% endhighlight %}
上面的代码看起来很棒！只有在第一次必要的时候才会使用锁，之后就和实现一中一样了。

在相当长的一段时间，迷惑了很多人，在2000年的时候才被人发现漏洞，而且在每种语言上都发现了。原因是内存读写的乱序执行（编译器的问题）。

我们来分析，```m_instance = new Singleton();```这句话可以分成三个步骤来执行：

* 1: 分配一个```Singleton```类型对象所需要的内存；

* 2: 在分配的内存处构造```Singleton```类型的对象；

* 3：把分配的内存地址赋给指针```m_instance```

你可能会认为这三个步骤是按顺序执行的，但实际上只能确定```步骤1```是最先执行的，步骤2、3却不一定。问题就出现在这。假如某个```线程A```在调用执行m_instance = new Singleton()的时候是按照```1、3、2```的顺序，那么刚刚执行完```步骤3```给Singleton类型分配了内存（此时```m_instance```就不是nullptr了）就切换到了```线程B```，由于```m_instance```已经不是nullptr了，所以线程B会直接执行```return m_instance```得到一个对象，而这个对象并没有真正的被构造。严重的bug就这么发生了。

### 2.5 实现5：C++ 11版本的跨平台实现
```java```和```c#```发现这个问题后，就加上一个关键字```volatile```，在声明```m_instance```变量的时候，要加上```volatile```修饰，编译器看到之后，就知道这个地方不能reorder（一定要先分配内存，再执行构造器，都完成之后再赋值）。

而对于c++标准却一直没有改正，所以```VC++```在```2005版本```也加入了这个关键字，但是这并不能够跨平台（只支持微软平台）。

而到了```c++ 11```版本，终于有了这样的机制帮助我们实现跨平台的方案。
{% highlight string %}
//C++ 11版本之后的跨平台实现 
// atomic c++11中提供的原子操作
std::atomic<Singleton*> Singleton::m_instance;
std::mutex Singleton::m_mutex;

/*
* std::atomic_thread_fence(std::memory_order_acquire); 
* std::atomic_thread_fence(std::memory_order_release);
* 这两句话可以保证他们之间的语句不会发生乱序执行。
*/
Singleton* Singleton::getInstance() {
    Singleton* tmp = m_instance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);//获取内存fence
	
    if (tmp == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        tmp = m_instance.load(std::memory_order_relaxed);
		
        if (tmp == nullptr) {
            tmp = new Singleton;
            std::atomic_thread_fence(std::memory_order_release);//释放内存fence
            m_instance.store(tmp, std::memory_order_relaxed);
        }
    }
    return tmp;
}
{% endhighlight %}

### 2.6 实现6： pthread_once函数
在linux中，pthread_once()函数可以保证某个函数只执行一次：
{% highlight string %}
声明: int pthread_once(pthread_once_t once_control, void (init_routine) (void))；

功能: 本函数使用初值为PTHREAD_ONCE_INIT的once_control
变量保证init_routine()函数在本进程执行序列中仅执行一次。 
{% endhighlight %}

示例如下：
{% highlight string %}
class Singleton{
public:
    static Singleton* getInstance(){
        // init函数只会执行一次
        pthread_once(&ponce_, &Singleton::init);
        return m_instance;
    }

private:
    Singleton(); //私有构造函数，不允许使用者自己生成对象
    Singleton(const Singleton& other);
    //要写成静态方法的原因：类成员函数隐含传递this指针（第一个参数）
    static void init() {
        m_instance = new Singleton();
    }

    static pthread_once_t ponce_;
    static Singleton* m_instance; //静态成员变量 
};

pthread_once_t Singleton::ponce_ = PTHREAD_ONCE_INIT;
Singleton* Singleton::m_instance=nullptr;              //静态成员需要先初始化
{% endhighlight %}

### 2.7 实现7: c++ 11版本最简洁的跨平台方案
```实现5```的方案有点麻烦，```实现6```的方案不能跨平台。其实```c++ 11```中已经提供了std::call_once方法来保证函数在多线程环境中只被调用一次，同样，他也需要一个once_flag的参数。用法和pthread_once类似，并且支持跨平台。

实际上，还有一种最为简单的方案！

在```C++memory model```中对static local variable，说道：

>The initialization of such a variable is defined to occur the first time control passes through its declaration; for multiple threads calling the function, this means there’s the potential for a race condition to define first.

局部静态变量不仅只会初始化一次，而且还是线程安全的。
{% highlight string %}
class Singleton{
public:
    // 注意返回的是引用。
    static Singleton& getInstance(){
        static Singleton m_instance;  //局部静态变量
        return m_instance;
    }
private:
    Singleton();                        //私有构造函数，不允许使用者自己生成对象
    Singleton(const Singleton& other);
};
{% endhighlight %}
这种单例被称为```Meyers' Singleton```。这种方法很简洁，也很完美，但是注意：

* gcc 4.0之后的编译器支持这种写法。

* C++11及以后的版本（如C++14）的多线程下，正确。

* C++11之前不能这么写。

但是现在都18年了, 新项目一般都支持了```c++11```了。

## 2.8 实现8： 饿汉版（Eager Singleton）
饿汉版（Eager Singleton）：指单例实例在程序运行时被立即执行初始化
{% highlight string %}
class Singleton
{
private:
	static Singleton m_instance;
private:
	Singleton();
	~Singleton();
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);
public:
	static Singleton& getInstance() {
		return m_instance;
	}
}

// initialize defaultly
Singleton Singleton::m_instance;
{% endhighlight %}
由于在```main```函数之前初始化，所以没有线程安全的问题。但是潜在问题在于```no-local static```对象（函数外的static对象）在不同编译单元中的初始化顺序是未定义的。也即，*static Singleton instance;*和*static Singleton& getInstance()*二者的初始化顺序不确定，如果在初始化完成之前调用```getInstance()```方法会返回一个未定义的实例。


## 3. 总结
* Eager Singleton 虽然是线程安全的，但存在潜在问题；

* Lazy Singleton通常需要加锁来保证线程安全，但局部静态变量版本在C++11后是线程安全的；

* 局部静态变量版本（Meyers Singleton）最优雅。




<br />
<br />

**[参考]**


1. [设计模式之单例模式](https://segmentfault.com/a/1190000015950693)

2. [C++ 单例模式](https://zhuanlan.zhihu.com/p/37469260)

3. [C++完美单例模式](https://www.jianshu.com/p/69eef7651667)

<br />
<br />
<br />





