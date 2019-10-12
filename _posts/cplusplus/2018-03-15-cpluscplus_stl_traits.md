---
layout: post
title: STL traits技巧
tags:
- cplusplus
categories: cplusplus
description: STL traits技巧
---


本文主要介绍一下stl中经常用到的traits技巧。


<!-- more -->


## 1. C++ traits技术
关于traits技术，官方也没有一个很明确的理论描述，也有点晦涩。我们从网上找到几个关于traits的描述：

* c++之父Bjarne Stroustrup关于traits的描述

>Think of a trait as a small object whose main purpose is to carry information used by another object or algorithm to determine "policy" or "implementation details".

简单翻译一下： trait是一个小对象，其主要目标是将另一个对象或算法的相关信息提取出来，用于确定```策略```或```实现细节```

* wiki中对traits的描述

>Traits both provide a set of methods that implement behaviour to a class, and require that the class implement a set of methods that parameterize the provided behaviour.

简单翻译一下： traits提供了一系列的方法用于实现一个类的行为，同时也要求该类对traits所指定的行为提供参数化实现。

综合上面，我认为可以这样理解： traits是对某一类```特征```或```行为```的抽象，同时要求在编译时(compile-time)就能够完成抽象的具体化。我们知道abstract class也可以通过继承的方式实现类似的功能，但abstract class是在运行时完成抽象的具体化的。如下图所示：

![cpp-traits](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_traits.jpg)

上图所示，无论是traits还是abstract class中均有两种对象，其中traits是通过萃取的方式来提取共同特征或行为的，而abstract是通过继承的方式来获得共同的特征或行为。可以看到，traits具有比abstract class更松散的耦合性。


traits不是一种语法特性，而是一种模板编程技巧。Traits在C++标准库，尤其是STL中，有着不可替代的作用。

## 2. 如何在编译期间区分类型
下面我们看一个实例，有四个类：Farm、Worker、Teacher和Doctor，我们需要区分他们是脑力劳动者还是体力劳动者，以便做出不同的行动。

这里的问题在于，我们需要为*两种类型提供一个统一的接口，但是对于不同的类型，必须做出不同的实现*。我们不希望写两个函数，然后让用户去区分。于是我们借助了函数重载，在每个类的内部内置一个work_type，然后根据每个类的work_type，借助强大的函数重载机制，实现了编译期的类型区分，也就是编译期多态。

代码如下：
{% highlight string %}
#include <iostream>
using namespace std;

//两个标签类
struct brain_worker{};              //脑力劳动
struct physical_worker{};           //体力劳动

class Worker{
public:
	typedef physical_worker worker_type;

};

class Farmer{
public:
	typedef physical_worker worker_type;

};

class Teacher{
public:
	typedef brain_worker worker_type;
	
};

class Doctor{
public:
	typedef brain_worker worker_type;
	
};

template<typename T>
void __distinction(const T &t, brain_worker){
	cout<<"脑力劳动者"<<endl;
}

template<typename T>
void __distinction(const T &t, physical_worker){
	cout<<"体力劳动者"<<endl;
}

template<typename T>
void distinction(const T &t){
	typename T::worker_type _type;        //为了实现重载
	__distinction(t, _type);
}

int main(int argc, char *argv[]){
	Worker w;
	Farmer f;
	Teacher t;
	Doctor d;
	
	distinction(w);
	distinction(f);
	distinction(t);
	distinction(d);
	
	return 0x0;
}

{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
体力劳动者
体力劳动者
脑力劳动者
脑力劳动者
</pre>
在distinction函数中，我们先从类型中提取出worker_type,然后根据它的类型，选取不同的实现。问题来了，如果不在类中内置worker_type，无法更改了，那么怎么办？

### 2.1 使用traits
我们的解决方案是，借助一种叫traits的技巧。

我们写一个模板类，但是不提供任何实现：
{% highlight string %}
//类型traits
template <typename T>
class TypeTraits;
{% endhighlight %}

然后，我们为每一个类型提供一个模板特化：
{% highlight string %}
//为每个类型提供一个特化版本
template<>
class TypeTraits<Worker>{
public:
	typedef physical_worker worker_type;
};

template<>
class TypeTraits<Farmer>{
public:
	typedef physical_worker worker_type;
};

template<>
class TypeTraits<Teacher>{
public:
	typedef brain_worker worker_type;
};

template<>
class TypeTraits<Doctor>{
public:
	typedef brain_worker worker_type;
};
{% endhighlight %}
然后在distinction函数中，不再是直接寻找内置类型，而是通过traits抽取出来：
{% highlight string %}
template<typename T>
void distinction(const T &t){
	typename TypeTraits<T>::worker_type _type;
	__distinction(t, _type);
}
{% endhighlight %}

上面两种方式的本质区别在于： 第一种是在class的内部内置type，第二种则是在类的外部，使用模板特化，class本身对于type并不知情。

下面给出完整源码：
{% highlight string %}
#include <iostream>
using namespace std;

//两个标签类
struct brain_worker{};              //脑力劳动
struct physical_worker{};           //体力劳动

class Worker{};
class Farmer{};
class Teacher{};
class Doctor{};

template <typename T>
class TypeTraits;

//为每个类型提供一个特化版本
template<>
class TypeTraits<Worker>{
public:
	typedef physical_worker worker_type;
};

template<>
class TypeTraits<Farmer>{
public:
	typedef physical_worker worker_type;
};

template<>
class TypeTraits<Teacher>{
public:
	typedef brain_worker worker_type;
};

template<>
class TypeTraits<Doctor>{
public:
	typedef brain_worker worker_type;
};

template<typename T>
void __distinction(const T &t, brain_worker){
	cout<<"脑力劳动者"<<endl;
}

template<typename T>
void __distinction(const T &t, physical_worker){
	cout<<"体力劳动者"<<endl;
}

template<typename T>
void distinction(const T &t){
	typename TypeTraits<T>::worker_type _type;
	__distinction(t, _type);
}

int main(int argc, char *argv[]){
	Worker w;
	Farmer f;
	Teacher t;
	Doctor d;
	
	distinction(w);
	distinction(f);
	distinction(t);
	distinction(d);
	
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
体力劳动者
体力劳动者
脑力劳动者
脑力劳动者
</pre>

### 2.2 两种方式结合
上面我们实现了目的，类中没有worker_type时，也可以正常运行。但是模板特化相对于内置类型，还是麻烦了一些。

于是我们仍然使用内置类型，也仍然使用traits抽取worker_type，方法就是为TypeTraits提供一个默认实现，默认去使用内置类型，把二者结合起来。这样我们去使用TypeTraits<T>::worker_type时，有内置类型的就使用默认实现，无内置类型的就需要提供特化版本。
{% highlight string %}
class Worker{
public:
	typedef physical_worker worker_type;

};

class Farmer{
public:
	typedef physical_worker worker_type;

};

class Teacher{
public:
	typedef brain_worker worker_type;
	
};

class Doctor{
public:
	typedef brain_worker worker_type;
	
};

//类型traits
template<typename T>
class TypeTraits{
public:
	typedef typename T::worker_type worker_type;
};
{% endhighlight %}
OK，我们现在想添加一个新的class，于是我们有两种选择：

* 在class内部内置worker_type，通过traits的默认实现去抽取type

* 不内置worker_type，而是通过模板的特化，提供worker_type

例如：
{% highlight string %}
class Staff{};

template<>
class TypeTraits<Staff>{
public:
	typedef brain_worker worker_type;
};
{% endhighlight %}

测试仍然正常：
{% highlight string %}
Staff s;
distinction(s);
{% endhighlight %}

### 2.3 进一步简化
这里我们考虑的是内置的情形，对于那些要内置type的类，如果type个数过多，程序编写就容易出现问题，我们考虑使用继承，先定义一个base类：
{% highlight string %}
template <typename T>
struct type_base{
	typedef T worker_type;
};
{% endhighlight %}
所有的类型，通过public继承这个类即可：
{% highlight string %}
class Worker : public type_base<physical_worker>{
};

class Farmer : public type_base<physical_worker>{
};

class Teacher : public type_base<brain_worker>{
};

class Doctor : public type_base<brain_worker>{
};
{% endhighlight %}
看到这里我们应该明白，traits相对于简单内置类型的做法，强大之处在于： 如果一个类型无法内置type，那么就可以借助函数特化，从而借助于traits。而内置类型仅仅适用于class类型。

以STL中的迭代器为例，很多情况下我们需要辨别迭代器的类型。例如distance函数计算两个迭代器的距离，有的迭代器具有随机访问能力，如vector，有的则不能，如list。我们计算两个迭代器的距离，就需要先判断迭代器能否相减，因为只有具备随机访问能力的迭代器才具有这个能力。

我们可以使用内置类型来解决。可是，许多迭代器是使用指针实现的，指针不是class，无法内置类型，于是STL采用traits来辨别迭代器的类型。

最后，我们应该认识到，**traits的基石是模板特化**。

## 3. value traits
我们在上文介绍了type traits，下面我们简要介绍一下value traits。我们来看一下累加函数：
{% highlight string %}
template <typename T>
struct traits;

template<>
struct traits<char>{
	typedef int AccuT;
};

template<>
struct traits<int>{
	typedef int AccuT;
};

template <class T>
typename traits<T>::AccuT accum3(const T* ptr, int len){
	traits<T>::AccuT total = traits<T>::AccuT();
	
	for(int i = 0; i<len; i++){
		total += *(ptr + i);
	}
	
	return total;
}
{% endhighlight %}
注意如下这行代码：
{% highlight string %}
traits<T>::AccuT total = traits<T>::AccuT();
{% endhighlight %}

如果AccuT是int、float等类型，那么int()、float()就会初始化成0，没有问题，那么万一对应的类型不可以()初始化呢？

这个时候就用到了value traits，可以把traits<int>改写成如下：
{% highlight string %}
template<>
struct traits<int>{
	typedef int AccuT;

	static AccuT const Zero = 0;
};
{% endhighlight %}
这个traits里面有一个类型和一个值。然后把累加函数改成：
{% highlight string %}
template<class T>
typename traits<T>::AccuT accum3(const T* ptr, int len){
	typename traits<T>::AccuT total = traits<T>::Zero;

	for(int i = 0; i < len; i++){
		total += *(ptr + i);
	}

	return total;
};
{% endhighlight %}

这样就可以解决初始化问题了。然后就算变动，也只需要修改traits里面的Zero了。

但是这么做也有个问题，就是并不是所有的类型都可以在类里面初始化。比如，我们把traits<int>的返回值类型改成double:
{% highlight string %}
template<>
struct traits<int>{
	typedef double AccuT;
	static AccuT const Zero = 0;
};
{% endhighlight %}
这样编译器直接报错(vs2012):
>error C2864: 'traits<int>::Zero' : only static const integral data members can be initialized within a class

有些人会在类外面来初始化这个值，比如: double const traits<int>::Zero = 0；这也是个办法。但是感觉这不是个好办法。更一般的是在traits内部搞个静态函数，然后在累加函数里面调用函数而不是静态变量。代码：
{% highlight string %}
#include <iostream>

template<typename T>
struct traits;
 
template<>
struct traits<char>
{
        typedef int AccuT;
        static AccuT Zero(){return 0;}
};
 
template<>
struct traits<int>
{
        typedef double AccuT;
        static AccuT Zero(){ return 0.0; };
};
 
template<class T>
typename traits<T>::AccuT accum3(const T* ptr, int len)
{
        typename traits<T>::AccuT total = traits<T>::Zero();
 
        for (int i = 0; i < len; i++)
        {
                total += *(ptr + i);
        }
 
        return total;
}
 
 
int main(int argc, char* argv[])
{
        int sz[] = {1, 2, 3};
        traits<int>::AccuT avg = accum3(sz, 3) / 3;
        std::cout<<"avg: "<<avg<<std::endl; 

        char ch[] = {'a', 'b', 'c'};
        traits<char>::AccuT avg2 = accum3(ch, 3)/3;
        std::cout<<"avg: "<<char(avg2)<<std::endl;

        return 0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
avg: 2
avg: b
</pre>

## 4. policy 与 traits
一讲到traits，相应的就会联系到policy。那么policy是干啥的呢？看一下下面的累加代码：
{% highlight string %}
template <class T>
typename traits<T>::AccuT accum(T *ptr, int len){
	typename traits<T>::AccuT total = traits<T>::Zero();

	for(int i = 0; i < len; i++){
		total += *(ptr + i);
	}

	return total;
}
{% endhighlight %}
注意```total += *(ptr+i);```这一行，这里有两个问题：

* 并不是所有类型都支持```+=```操作符，如果传进来的是一个自定义类型，而且没有```+=```，怎么办？

* 如果并不想使用```+=```，比如传进来的是个字符串，然后想逆序拼接

想解决这2个问题，首先想到的应该就是这个地方不要hard code了，通过传进来参数的方法来动态修改这行代码。一般有两种方法：

* 传进来一个函数对象，然后通过函数对象来调用相应的函数

* 使用policy类

这里我们介绍第二种方法： policy。我们把累加函数改成：
{% highlight string %}
template<class T, class P> 
typename traits<T>::AccuT accum(T *ptr, int len){
	typename traits<T>::AccuT total = traits<T>::Zero();

	for(int i = 0; i < len; i++){
		P::accumulate(total, *(ptr+i));
	}

	return total;
}
{% endhighlight %}
注意，我们多加了一个模板参数P，然后调用P::accumulate()。现在我们来定义这个P类：
{% highlight string %}
class P{
public:
	template<class T1, class T2>
	static void accumulate(T1 &total, T2 v){
		total += v;
	}
};
{% endhighlight %}

P类里面只有一个函数，是个静态模板函数。这样我们就可以调用了：
{% highlight string %}
int sz[] = {1,2,3};

traits<int>::AccuT avg = accum<int, P>(sz,3)/3;
{% endhighlight %}
上面我们得到结果为2。

那如果我现在要传入一个字符数组，并且想逆序拼接，应该怎么做呢？

1） 首先把traits<char>的返回类型改为string
{% highlight string %}
template<>
struct traits<char>{
typedef std::string AccuT;

static AccuT Zero(){return std::string("");}
};
{% endhighlight %}
然后，新增一个policy类：
{% highlight string %}
class P2{
public:
	template<class T1, class T2> 
	void accumulate(T1 &total, T2 v){
		total.insert(0,1,v);
	}
};
{% endhighlight %}
调用一下，就会发现返回值是```cba```了：
{% highlight string %}
char str[] = {'a', 'b', 'c'};

traits<char>::AccuT ret = accum<char,P2>(str, 3); 
{% endhighlight %}
如果调用accum(char, P>(str, 3)；就会返回```abc```，因为string本身也支持```+=```，所有P::accumulate()可以正常运行，并且顺序拼接。



<br />
<br />

**[参看]:**

1. [模板：什么是Traits](https://www.cnblogs.com/inevermore/p/4122278.html)

2. [C++模板 - value traits](https://blog.csdn.net/zj510/article/details/41961969)

3. [traits and policy](https://blog.csdn.net/zj510/article/category/1270585)

4. [stl源代码](https://github.com/gcc-mirror/gcc)

<br />
<br />
<br />





