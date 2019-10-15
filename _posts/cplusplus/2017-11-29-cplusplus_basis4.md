---
layout: post
title: C++模板
tags:
- cplusplus
categories: cplusplus
description: C/C++基础
---

本章主要记录一下C++模板方面的一些内容，以备后续查验。


<!-- more -->

## 1. 整体介绍
Template通过将将类型作为参数，从而对泛型编程(generic programing)提供了直接的支持。在定义类(class)、函数(function)或者类型别名(type alias)时，C++的模板机制允许将一个类型(type)或值(value)作为参数来传入。


在C++中，通常每一个标准库的抽象都是通过模板来实现的（例如： string、ostream、regex、complex、list、map、unique_ptr、thread、future、tuple、function)，一些关键的操作函数也是通过模板来实现的（例如： string的比较函数、输出运算符<<、complex算术运算操作、list插入删除、以及sort排序等）。

>注： 由class template所产生的类其实就是一个普通类，与手写(handwritten)类完全一样，并未引入任何新的运行时机制。而实际上，使用模板还可以降低产生的代码量，因为对于有一些在代码中未用到的成员函数，在进行代码生成时就会被忽略。

经常有些人对于```class template```与```template class```感到困惑，其实在实际使用过程中对这两个概念我们并不一定要严格区分。但这里既然提出来了，我们就来讲述一下两者之间的微妙区别：

* class template: 是一个模板。我们通常看到的
{% highlight string %}
template <typename C> 
class String{
};
{% endhighlight %}
就是一个模板。

* template class: 由模板产生的类。结合上面String<char>就是一个由模板产生的类。

同理，对于function template与template function也有相似的微妙区别。

通常情况下，一个class template的成员的声明与实现均在类中，但实际上这并不是必须的。例如：
{% highlight string %}
template <typename C>
class String{
public:
	String();

	String & operator+=(C c);
};

template <typename C>
String<C>::String(){
}

template <typename C>
String & String<C>::operator+=(C c){
}
{% endhighlight %}

## 2. 模板的特化

### 2.1 模板参数
1) **类型参数(type parameter)**

通常我们使用```typename```或者```class```来指定类型参数，例如：
{% highlight string %}
template <typename T>
void f(T);
{% endhighlight %}

2) **值参数(value parameter)**

值参数必须为如下类型：

* 整数常量表达式

* 对象的指针或引用类型，或者是一个函数名

* 非重载的成员指针

* NULL指针

3) **operation参数**

考虑标准库中map的一个简单实现版本：
{% highlight string %}
template <typename Key, class V>
class map{
	//...
};
{% endhighlight %}
针对map中的Key，我们如何提供相应的比较准则呢？

* 首先我们不能在container实现中将相应的比较准则进行硬编码，因为通常情况下container并不需要暴露元素的具体类型。例如，默认情况下map使用```<```来进行比较，但是并不是所有类型的key都支持通过```<```来进行比较

* 其次，我们不能在container的实现中将相应的排序准则进行硬编码，因为对于同一个key可能会有多种排序方式。例如，我们经常会用到的key类型是string，但是string可以有一系列的排序准则（如是否忽略大小写）


因此，排序准则通常并不会直接内置于container类型中。原则上，对于一个map的排序准则通常有如下指定方法：

* 通过一个```值参数```(value argument)，例如cmp函数指针

* 通过一个```类型参数```(type argument)指明需要进行比较的对象的类型

上面第一种方法较为简单，举例如下：
{% highlight string %}
template <typename Key, typename V, bool (*cmp)(const Key &, const Key &)>
class map{
	//...
};
{% endhighlight %}

上面map需要用户提供一个比较函数来作为参数：
{% highlight string %}
bool insensitive(const string &	x, const string &y){
	//compare case insensitive(eg. "hello" equals "HELLO")
}
map<string, int, insensitive> m;            //compare using insensitive
{% endhighlight %}
然而这样做其实并不是很灵活。特别是，map的设计者并不能决定到底是采用一个函数指针(function pointer)还是函数对象(function object)来比较key类型。同时，也因为相应参数的比较要依赖于key类型，这使得我们很难提供一个默认的比较准则。

基于上面提到的种种，第二种方式（通过传递一个类型参数)通常更为常用，并且也被标准库所采用。例如：
{% highlight string %}
template <typename Key, class V, typename Compare = std::less<Key>>
class map{
public:
	map(){/*...*/                       //use the default comparison
	map(Compare c):cmp(c){/*...*/}      //override the default 

	//...
	Compare cmp{}                       //default comparison
};
{% endhighlight %}

最常见的情况是使用```less-than```来进行比较。假如我们想要采用一个不同的比较准则，我们可以提供一个函数对象(function object):
{% highlight string %}
map<string, int> m1;                        //use the default comparison(less<string>)
map<string, int,std::greater<string>> m2;   //compare using greater<string>()
{% endhighlight %}

通过使用函数对象(function object)来传递比较操作(compare operation)具有多方面的好处：

* 一个定义在类中的简单成员函数通常是inline的，然而如果要通过一个函数指针来以inline方式调用的话，这需要编译器进行一些额外的工作

* 一个没有数据成员的function object的传递并不会有run-time消耗

* 通过function object可以传递多个操作，而这并不会增加额外的run-time消耗

4） **Template参数**

我们也可以向一个class template中传递template，例如：
{% highlight string %}
template <typename T, template<typename> class C>
class Xrefd{
	C<T> mems;
	C<T *>refs;
	//...
};
template <typename T> 
using My_Vec = vector<T>;              //use default allocator

Xrefd<Entry, My_Vec> x1;               //store cross reference for Entrys in a vector

template<typename T>
class My_Container{
	//...
};
Xrefd<Record, My_Container> x2;        //store cross references for Records in a My_Container
{% endhighlight %}
将template作为一个模板参数传递的使用场景通常是： 使用多种字段类型来实例化（例如上面的例子中的```T```与```T*```)。

5） **默认的模板参数**

假如每次使用map时我们都需要明确的指定```比较准则```的话，则会显得十分的繁琐。因此，我们可以指定less<Key>作为```Compare```默认的模板参数值，而对于一些非常见的比较准则我们就可以显示地去指定：
{% highlight string %}
template<typename Key, class V, typename Compare=std::less<Key>>
class map{
public:
	explicit map(const Compare& comp={});
	//...
};

map<string, int> m1;                   //will use less<string> for comparisons
map<string, int, less<string>> m2;     //same type as m1


struct No_case{
	//define operator()() do do case-insensitive string comparison
};
map<string, int, No_case> m3;         //m3 is of a different type from m1 and m2
{% endhighlight %}

6) **默认的函数模板参数**

函数模板也可以指定默认的模板参数值。例如：
{% highlight string %}
template<typename Target=string, typename Source=string>
Target to(Source arg){
	stringstream interpreter;
	Target result;

	if(!(interpreter<<arg)||                // write arg into stream
	  !(interpreter >> result) ||           //read result from stream
	  !(interpreter >>std::ws).eof())       //stuff left in stream
	 throw runtime_error{"to<>()failed"};

	return result;
}
{% endhighlight %}
对函数模板，通常情况下只有在模板参数不能被自动推导或没有默认值的情况下才需要显式的指定。例如：
{% highlight string %}
auto x1 = to<string, double>(1.2);             //very explicit(and verbose)
auto x2 = to<string>(1.2);                     //source is deduced to double
auto x3 = to<>(1.2);                           //Target is default to string, source is deduced to double
auto x4 = to(1.2);                             //the<>is redundant                
{% endhighlight %}



<br />
<br />

**[参看]:**

1. [string和basic_string的关系](https://blog.csdn.net/robot8me/article/details/78691446)


<br />
<br />
<br />





