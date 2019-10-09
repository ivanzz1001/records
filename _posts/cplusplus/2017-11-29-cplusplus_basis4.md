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





<br />
<br />

**[参看]:**

1. [string和basic_string的关系](https://blog.csdn.net/robot8me/article/details/78691446)


<br />
<br />
<br />





