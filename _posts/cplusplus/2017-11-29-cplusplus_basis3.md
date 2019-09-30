---
layout: post
title: C++运算符重载
tags:
- cplusplus
categories: cplusplus
description: C/C++基础
---

本章主要记录一下C/C++基础方面的一些内容，以备后续查验。


<!-- more -->

## 1. 运算符重载
在C++中为了使用方便，可以对运算符进行重载，可重载的运算符有：
{% highlight string %}
+       −       ∗        /        %          ˆ          &
|       ˜       !        =        <          >          +=
−=      ∗=      /=       %=       ˆ=         &=         |=
<<      >>      >>=      <<=      ==         !=         <=
>=      &&      ||       ++       −−         −>∗        ,
−>      []      ()       new      new[]      delete     delete[]
{% endhighlight %}
如下运算符并不能被重载：

* ```::```: 域限定符(scope resolution)

* ```.```: 成员选择(member select)

* ```.*```: 访问指针成员变量

这些操作符所接受的第二个参数通常是一个名称(name)，而不是一个值(value)，来访问对应的成员。假若允许这些操作符进行重载的话，可能会导致一些微妙的问题的出现。

此外```?:```运算符也一般不能够进行重载。

### 1.1 单目运算符与双目运算符

1) **双目运算符重载**

对于双目运算符的重载，有两种方式：

* 带有1个参数的非静态成员函数

* 带有2个参数的非成员函数

对于任意类型的双目运算符**@**,则**aa@bb**可以被解释为**aa.operator@(bb)**或者**operator@(aa, bb)**。假如上述两种重载方式都被定义了的话，则由相应的规则(overload resolution)决定会被解释为哪种方式。例如：
{% highlight string %}
class X{
public:
	void operator+(int);
	X(int);
};

void operator+(X,X);

void operator+(X, double);

void f(X a)
{
	a + 1;    //a.operator+(1)
	1 + a;    //::operator+(X(1),a)
	a + 1.0;  //::operator+(a, 1.0)
}
{% endhighlight %}

2) **单目运算符重载**

对于单目运算符重载，不管是prefix还是postfix，也有两种方式：

* 不带参数的非静态成员函数

* 带有1个参数的非成员函数

对于任何前置(prefix)单目运算符**@**，**@aa**可以被解释为**aa.operator@()**或者**operator@(aa)**,假如两种重载方式均被定义了的话，则由相应的规则(overload resolution)决定会被解释为哪种方式。

对于任何后置(postfix)单目运算符**@**，**aa@**可以被解释为**aa.operator@(int)**或者**operator@(aa, int)**。同样假如两种重载方式均被定义了的话，则由相应的规则(overload resolution)决定会被解释为哪种方式。

注意，我们并不能改变一个运算符的单目性与双目性。
{% highlight string %}
class X{
public:                 //members(with implict this pointer)

	X* operator&();     //prefix unary &(address of)
    X operator&(X);     //binary &(and)
	X operator++(int);  //postfix increment 
	X operator&(X,X);   //error: ternary &
	X operator/();      //error: unary /
};

//nonmember functions:

X operator-(X);         //prefix unary minus 
X operator-(X,X);       //binary minus
X operator--(X &, int); //postfix decrement
X operator-();          //error: no operand
X operator-(X,X,X);     //error: ternary
X operator%(X);         //error: unary
{% endhighlight %}
对于操作符operator=, operator[], operator(), operator->()必须被定义为非静态成员函数。

对于运算符重载函数，其所支持的参数传递方法只有：

* Pass-By-Value(按值传递)

* Pass-By-Reference(按引用传递)

在参数传递中并不支持指针。





<br />
<br />

**[参看]:**




<br />
<br />
<br />





