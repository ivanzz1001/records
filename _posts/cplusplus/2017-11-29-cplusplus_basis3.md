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


## 2. 特殊运算符重载
在上面的章节中我们介绍了一些基本的运算符重载，在下面我们将对一些较为特殊的运算符进行介绍：
{% highlight string %}
[]    ()    ->     ++    --    new    delete
{% endhighlight %}

### 2.1 下标操作符
一个operator[]函数可以被用于类对象的下标操作，其中函数的参数可以是任何类型，这就使得其可以被用于vector、关联数组等数据结构中。

如下例子中，我们可以定义一个简单的关联数组类型：
{% highlight string %}
struct Assoc{
	vector<pair<string,int>> vec;     //vector of {name, value} pairs

	const int& operator[](const string &) const;
	int& operator[](const string &);
};
{% endhighlight %}
上面```Assoc```对象维持了一个vector,其中vector的每一个元素是std::pair。如下我们使用一种常用但低效的方式来实现：
{% highlight string %}
//search for s; return a reference to its value if found;
//otherwise,make a new pair {s,0} and return a reference to its value
int& Assoc::operator[](const string & s)
{
	for(auto x : vec)
		if(s == x.first)
			return x.second;

	vec.push_back({s, 0});        //initial value: 0

	return vec.back().second;     //return last element
}
{% endhighlight %}
有了上述重载函数之后，我们就可以按如下方式来使用Assoc:
{% highlight string %}
int main(int argc, char *argv[])
{
	Assoc values;

	string buf;
	while(cin >> buf) 
		++value[buf];

	for(auto x : values.vec)
		cout<<'{'<<x.first<<','<<x.second<<'}'<<endl;

	return 0x0;
}
{% endhighlight %}
>注： operator[]必须是一个非静态成员函数


### 2.2 Function Call
函数调用，其表现形式为*expression(expression-list)*，可以被解释为一个双目操作符，其中```expression```可以被认为是左操作数，```expression-list```可以被认为是右操作数。对于函数调用操作符```()```其同样也可以被重载，例如：
{% highlight string %}
struct Action{
	int operator()(int);

	pair<int, int> operator()(int, int);

	double operator()(double);
	
	// ...
};

void f(Action act)
{
	int x = act(2);
	auto y = act(3,4);

	double z = act(2.3);

	//...
}
{% endhighlight %}
对一个object进行()运算符重载，这样可以使得其行为类似于仿函数。这样一种函数对象(function object)使得我们可以在写代码的时候将其作为参数来执行一些特定操作，例如：
{% highlight string %}
class Add{
	complex val;

public:
	Add(complex c):val{c}{}                       //save a value

	Add(double r, double i):val{{r,i}}{}

	
	void operator(complex &c) const{ c += val;}    //add value to argument
};
{% endhighlight %}
上述Add类首先会使用一个complex对象来进行初始化，当Add对象通过使用```()```来调用时，其就会将该值加到对应的调用参数上。例如：
{% highlight string %}
void h(vector<complex> &vec, list<complex>& lst, complex z)
{
	for_each(vec.begin(), vec.end(), Add{2,3});

	for_each(lst.begin(), lst.end(), Add{z});
}
{% endhighlight %}

>注： operator()()必须是一个非静态成员函数

下面给出一个完整的例子：
{% highlight string %}
#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <complex>
	

class Add{
	std::complex<double> val;
	
public:
	Add(std::complex<double> c):val(c){}
	
	Add(double r, double i):val(r,i){}
	

	void operator()(std::complex<double> & c){
		c += val;
	}

};



void hfunc(std::vector<std::complex<double> >& vec, std::list<std::complex<double> > &lst, std::complex<double> z)
{
	std::for_each(vec.begin(), vec.end(), Add(2,3));
	
	std::for_each(lst.begin(), lst.end(), Add(z));
}


int main(int argc, char* argv[])
{
	std::vector<std::complex<double> > vec;    //注： 这里std::complex<double>后必须要有一个空格，否则VC6.0中编译不过
	std::list<std::complex<double> > lst;
	std::complex<double> z(10,10);
	
	vec.push_back(std::complex<double>(1,1));
	vec.push_back(std::complex<double>(2,2));
	
	lst.push_back(std::complex<double>(3,3));
	lst.push_back(std::complex<double>(4,4));
	
	hfunc(vec, lst, z);
	
	for(std::vector<std::complex<double> >::iterator it = vec.begin(); it != vec.end();it++)
		std::cout<<*it<<std::endl;
	
	
	for(std::list<std::complex<double> >::iterator it2 = lst.begin(); it2 != lst.end(); it2++)
		std::cout<<*it2<<std::endl;

	return 0x0;
}
{% endhighlight %}


### 2.3 Dereferencing
解引用操作符```->```可以被定义为一个单目后置操作符(unary postfix operator)，例如：
{% highlight string %}
class Ptr{
	//...
	X* operator->();
};
{% endhighlight %}
Ptr类对象可以被用于访问类```X```的成员，类似于指针操作。例如：
{% highlight string %}
void f(Ptr p)
{
	p->m = 7;    //(p.operator->())->m = 7
}
{% endhighlight %}
在实现智能指针时重载```->```操作符就变得十分有用。


### 2.4 Increment and Decrement



<br />
<br />

**[参看]:**




<br />
<br />
<br />





