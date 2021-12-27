---
layout: post
title: lambda表达式(C++11)
tags:
- cplusplus
categories: cplusplus
description: lambda表达式(C++11)
---


Lambda表达式，有时候也被称为lambda函数(lambda function)或直接被简称为```lambda```，其是一种定义与使用匿名```函数对象```的简化方式。当我们需要向一个算法函数的某个参数传递一个operation的时候，lambda表达式就十分的方便。本章我们会详细介绍一下lambda。




<!-- more -->

## 1. lambda函数概述

lambda表达式是一种匿名函数，即没有函数名的函数；该匿名函数是由数学中的λ演算而来的。通常情况下，lambda函数的语法定义为：
{% highlight string %}
[capture list](parameters) mutable noexcept ->return-type { body }
{% endhighlight %}

其中：

* ```[capture list]```: 捕获列表。捕获列表总是作为lambda的开始，即出现于lambda的开始处。它是lambda的引出符（即开始标志）。编译器可以根据该```标志```来做出判断该函数是否为lambda函数。同时```捕获列表```能够捕获上下文中的变量以作为lambda函数使用

* (parameters): 参数列表。和C/C++中的普通函数意义一样。该部分是可选的，意味着如果我们不需要进行参数传递时，可以连括号```()```都一起省略掉。

* mutable: 修饰符（可选），用于指明lambda表达式可能会修改lambda的状态

* noexcept: 修饰符（可选）

* ```->return-type```: 用于指定函数的返回类型（可选）

* body: 指明要执行的代码

## 2. 实现模型
Lambda表达式有多种实现方式，并且也有多种有效的方式来对其进行优化。我们可以将lambda表达式语法想象为```function object```的一种简化形式，如下我们看一个简单的示例：
{% highlight string %}
//output v[i] to os if v[i] % m == 0
void print_modulo(const vector<int> &v, ostream &os, int m)
{
	for_each(begin(v), end(v), [&os,m](int x){
		if(x%m==0) os<<x<<"\n";}
	};
}
{% endhighlight %}
我们可以定义一个等价的```function object```:
{% highlight string %}
class Modulo_print{
	ostream &os;	//members to hold the capture list
	int m;

public:
	Modulo_print(ostream &s, int mm):os(s),m(mm){}	//capture
	void operator()(int x) const{
		if(x%m==0) os<<x<<"\n");
	}
};
{% endhighlight %}
上面捕获列表```[&os, m]```变成了两个成员变量，通过一个构造函数来完成初始化。```&os```表明我们存储的是一个引用，而```m```表明我们存储的一份拷贝。

lambda的body部分简单的变成了operator()()函数的body部分。这里由于lambda并不需要返回值，因此operator()()的返回值是void。默认情况下operator()()是const的，因此lambda的body部分并不会修改捕获列表中的变量。这是我们通常遇到的情况，但假如你想在lambda的body部分修改lambda的状态的话，则可以将lambda表达式声明为mutable，这对应于去掉const后的operator()()。

由lambda表达式所产生的类对象(class object)被称为```closure object```，因此我们可以采用如下方式来编写```print_modulo()```函数：
{% highlight string %}
void print_modulo(const vector<int> &v, ostream &os, int m)
{
	for_each(begin(v), end(v), Modulo_print(os, m));
}
{% endhighlight %}

>注:If a lambda potentially captures every local variable by reference (using the capture list [&]), the closure may be optimized to simply contain a pointer to the enclosing stack frame.

## 3. lambda补充说明
上面最后一个版本的print_modulo()看起来很优雅，将一些复杂的操作细节封装起来命名为Modulo_print也是很好的设计理念。一个单独的类定义也留有更多的空间来添加注释，这一点比直接在一些函数参数列表中使用lambda表达式更优雅。

然而，很多lambda表达式都很小巧，并且通常只会使用一次。对于这种情况，等价于定义一个局部类(local)，之后马上使用该类对象。例如：
{% highlight string %}
void print_modulo(const vector<int> &v, ostream &os, int m)
{
	class Modulo_print{
		ostream &os;	//members to hold the capture list
		int m;
	
	public:
		Modulo_print(ostream &s, int mm):os(s),m(mm){}	//capture
		void operator()(int x) const{
			if(x%m==0) os<<x<<"\n");
		}
	};
	for_each(begin(v), end(v), Modulo_print(os, m));
}
{% endhighlight %}

如果这样来进行对比的话，则使用lambda表达式明显会简洁许多。而如果我们确实想要一个名称(name)的话，我们可以通过如下方式来对一个lambda表达式进行命名：
{% highlight string %}
void print_modulo(const vector<int> &v, ostream &os, int m)
{
	auto Modulo_print = [&os, m](int x){if(x%m==0) os<<x<<"\n";}
	
	for_each(begin(v), end(v), Modulo_print); 
}
{% endhighlight %}
对lambda表达式进行命名是一个很好的理念，因为这么做会强制我们更仔细的考虑相关操作的设计，并且也可以简化相关的代码布局。

如下的for循环是另外一种使用for_each() lambda表达式的形式：
{% highlight string %}
void print_modulo(const vector<int> &v, ostream &os, int m)
{
	for(auto x : v)
		if(x % m == 0) os << x <<"\n";
}
{% endhighlight %}
大部分人会发现上面的写法会比任何的lambda表达式版本更简洁。然而，因为for_each是一个很特别的算法，并且vector<int>也是一个很特别的container。考虑如下的泛化版本：
{% highlight string %}
template<class C>
void print_modulo(const C &v, ostream &os, m)
{
	for(auto x: v)
		if(x % m == 0)  os << x << "\n");
}
{% endhighlight %}

这个版本的print_modulo()对于std::map也是可以正常工作的。C++的range-for语句特别适合于从头到尾的遍历某个区间的元素，STL的容器也经常使用此方法来进行遍历。例如，使用for语句来对std::map进行深度优先的遍历。那我们如何进行广度优先的遍历呢？上面print_modulo()的for循环版本就不能很好的处理这种改变，因此我们必须重写一个算法。例如：
{% highlight string %}
template<class C>
void print_modulo(const C& v, ostream& os, int m)
{
	breadth_first(begin(v), end(v), [&os,m]{if(x%m==0)  os<<x<<"\n";});
}
{% endhighlight %}
这样，lambda的body部分可以实现一个泛化版本的算法。如果我们使用for_each的话，是采用深度优先来进行遍历，而不是广度优先。

上面将lambda表达式作为一个参数的遍历算法与普通的for循环遍历算法性能是一样的。这样我们要考虑的就只有使用的方便性这一个方面了。

## 4. Capture
lambda的主要用途就是可以通过参数来传递代码。lambda以inline的方式来完成，而并不需要指定一个函数名称。有一些lambda并不需要访问本地环境，这种情况下我们可以指定一个空的capture list，例如：
{% highlight string %}
void algo(vector<int> &v)
{
	sort(v.begin(), v.end());     //sort values
	//...

	sort(v.begin(), v.end(), [](int x, int y){return abs(x) < abs(y);});	//sort absolute values
}
{% endhighlight %}
而如果我们想要访问一个本地变量的话，则会报如下错误：
{% highlight string %}
void f(vector<int> &v)
{
	bool sensitive = true;
	//...

	sort(v.begin(), v.end(), 
		[](int x, int y){return sensitive?x<y:abs(x)<abs(y);}		//error: can't access sensitive
	);
}
{% endhighlight %}
上面我们使用了lambda表达式的introducer ```[]```。这是最简单的lambda introducer，在调用环境中并不允许引用任何本地局部变量。一个lambda表达式的introducer可以有多种形式：

* ```[]```: 空capture list。这意味着在lambda的body中不能引用当前上下文环境中的任何局部名称。对于这种lambda表达式，数据就只能从参数或者是非局部变量中来获取。

* ```[&]```: 指示通过引用的方式来capture。当前上下文的所有local names都可以在body中使用，所有的局部变量都通过引用的方式来进行访问

* ```[=]```: 指示通过value的方式来capture。当前上下文的所有local names都可以在body中使用，并且所有的变量都以按值传递的方式传递到body中去。

* ```[capture-list]```: 显式的指明需要capture哪些环境；capture-list是需要捕获的本地变量名称列表，可以通过```按值```或```按引用```的方式来捕获。capture-list也可以包含this。

* ```[&, capture-list]```: 指示通过引用的方式来capture所有局部变量，而capture-list中的变量只能采用按值传递，不能按引用。

* ```[=, capture-list]```: 指示通过按值的方式来capture所有局部变量，而capture-list中的变量只能采用按引用传递，不能按值

默认情况下，总是按值来capture一个local name，如果要按引用来capture，则必须加上```&```符号。只有按引用传递的变量才允许在调用环境中修改变量的值。如下：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

template<class C>
void func(C c){
        c();
}

int main(int argc,char *argv[])
{
        int a = 1;
        int b = 2;

        auto addp = [&a,&b](){a=a+1; b=b+2;std::cout<<a<<"  "<<b<<std::endl;};

        func(addp);
        return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -std=c++11 -lstdc++
# ./test
</pre>
由于我们在lambda的body中修改了局部变量a和b，因此不能按值传递，否则编译时出现如下错误：
{% highlight string %}
# gcc -o test test.cpp -std=c++11 -lstdc++
test.cpp: In lambda function:
test.cpp:15:23: error: assignment of read-only variable ‘a’
  auto addp = [a,b](){a=a+1; b=b+2;std::cout<<a<<"  "<<b<<std::endl;};
                       ^
test.cpp:15:30: error: assignment of read-only variable ‘b’
  auto addp = [a,b](){a=a+1; b=b+2;std::cout<<a<<"  "<<b<<std::endl;};
{% endhighlight %}

另外我们特别要注意的一点是，当按引用来capture时，要注意所引用对象的生命周期，否则可能出现一些难以发现的问题。

假如我们想要捕获一个可变模板参数，可以使用```...```，例如：
{% highlight string %}
template<typename... Var>
void algo(int s, Var... v)
{
	auto helper = [&s,&v...] { return s∗(h1(v...)+h2(v...)); }
	// ...
}
{% endhighlight %}
>Beware that is it easy to get too clever about capture. Often, there is a choice between capture and argument passing. When that’s the case, capture is usually the least typing but has the greatest potential for confusion.


## 4. lambda与lifecycle
一个lambda可能会比其调用者具有更长的生命周期，比如我们将一个lambda传递到另外一个线程，又或者被调用者将一段lambda表达式保存起来以供后续使用。例如：
{% highlight string %}
void setup(Menu &m)
{
	//...
	Point p1,p2,p3;

	//compute positions of p1,p2,and p3
	m.add("draw triangle",[&]{m.draw(p1,p2,p3);});		//probable disaster
	
	//...
}
{% endhighlight %}
我们这里假设add()函数将(name, action)添加到menu，draw()函数用于实现画图，这样我们就留下了一个巨坑：当setup完成之后，过了几分钟，用户再来按```draw triangle```按钮的话，lambda会尝试访问当前已经不在生命周期中的变量。按引用capture时，就特别容易出现这样的问题。

假如一个lambda的生命周期长于调用者的话，那么我们最好将所有的本地局部信息拷贝进closure object中，例如对于上面的setup例子，我们可以改为：
{% highlight string %}
m.add("draw triangle",[=]{ m.draw(p1,p2,p3); });
{% endhighlight %}

## 5. lambda与this
在一个类成员函数当中，如果我们要使用lambda来访问该类的另一个成员函数，应该怎么做？ 我们可以在capture-list中添加this，例如：
{% highlight string %}
class Request{
	function<map<string,string>(const map<string, string>&)> oper;	//operation
	map<string,string> values;	//arguments
	map<string,string> results;	//targets

public:
	Request(const string &s);	//parse and store request

	void execute()
	{
		[this](){results=oper(values);}	//do oper to values yielding results
	}
};
{% endhighlight %}
类的成员总是通过reference方式被capture。这里，```[this]```暗示着成员变量或成员函数均是通过this来访问的，而不是直接拷贝进lambda中。不幸的是，[this]与[=]并不兼容，这就暗示着在多线程程序中如果使用不当很容易造成race condition(竞争条件）。




<br />
<br />

**[参看]**

1. [C++11 之 lambda函数的详细使用](https://blog.csdn.net/lixiaogang_theanswer/article/details/80905445)


<br />
<br />
<br />


