---
layout: post
title: C++11新特性
tags:
- cplusplus
categories: cplusplus
description: C++11新特性
---


本章记录一下C++11中的一些新特性。

<!-- more -->


## 1. std::conditional实现变量的多类型
如代码所示，Type1、Type2、Type3都是根据模板中的第一个参数来确定类型的。Type1为int，Type2为double， Type3为double。因此a=3；b=4.2；a+b=7.2。
{% highlight string %}
#include <iostream>
#include <type_traits>
#include <typeinfo>

int main(int argc, char *argv[])
{
	typedef typename std::conditional<true, int, double>::type Type1;
	typedef typename std::conditional<false, int, double>::type Type2;
	typedef typename std::conditional<sizeof(int) == sizeof(double), int, double>::type Type3;

	std::cout << typeid(Type1).name() << std::endl;
	std::cout << typeid(Type2).name() << std::endl;
	std::cout << typeid(Type3).name() << std::endl;

	Type1 a = 3.1;
	Type2 b = 4.2;
	std::cout << a +  b << std::endl;

	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o conditional conditional.cpp -lstdc++ -std=c++11
# ./conditional 
i
d
d
7.2
</pre>






<br />
<br />

**[参看]**

1. [C++11新特性之利用std::conditional实现变量的多类型](https://blog.csdn.net/asbhunan129/article/details/86609897)


<br />
<br />
<br />


