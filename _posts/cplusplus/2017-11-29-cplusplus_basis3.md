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

```->```运算符重载经常会被用于实现智能指针。标准库中的智能指针```unique_ptr```以及```shared_ptr```均对```->```进行运算符重载。

值得注意的是，对于普通的指针，通常在使用```->```时其与单目运算符```*```、```[]```具有相同的含义，例如：
{% highlight string %}
p−>m == (∗p).m           //is true
(∗p).m == p[0].m        // is true
p−>m == p[0].m          // is true
{% endhighlight %}
但是对于```->```重载运算符，一般天然是没有这样相同含义的保证。如果要提供类似相同的含义，我们通常需要进行进一步的重载保证：
{% highlight string %}
template<typename T>
class Ptr {
	Y∗ p;

public:
	Y∗ operator−>() { return p; }            // dereference to access member
	Y& operator∗() { return ∗p; }            // dereference to access whole object
	Y& operator[](int i) { return p[i]; }    // dereference to access element
	// ...
};
{% endhighlight %}
>注：运算符->重载必须是一个非静态(non-static)成员函数，并且其返回值必须是一个指针或者是一个可以使用->的类对象。

虽然```->```与```.```操作符含义相似，但是我们并不能对```.```(成员选择符)运算符进行重载。


### 2.4 Increment and Decrement

在智能指针(smart pointer)被发明之后，通常我们还会提供自增操作(++)与自减操作(--)，以使其更类似于C++的内置类型。在C++运算符中，increment 与 decrement运算符较为特殊，其既可以作为前置运算符，也可作为后置运算符。因此，我们在运算符重载时需要提供前置与后置两种，例如：
{% highlight string %}
template<typename T>
class Ptr{
	T *ptr;
	T *array;
	int sz;

public:
	template<int N>
	      Ptr(T *p, T(&a)[N]);      //bind to array a, sz == N, initial value p

	Ptr(T *p, T *a, int s);         //bind to array a of size s, initial value p

	Ptr(T *p);                      //bind to single object, sz == 0, initial value p

	Ptr& operator++();              //prefix
	Ptr operator++(int);            //postfix

	Ptr& operator--();              //prefix
	Ptr operator--(int);            //postfix

	T& operator*();                 //prefix
};
{% endhighlight %}

在上述的运算符重载函数中，参数int用于指明该运算符是进行的后置重载。在该函数中参数int是不会被使用到，其仅仅是作为一个dummy用于区分是前置运算符重载还是后置运算符重载。


### 2.5 Allocation and Deallocation

1) **理论介绍**

我们知道new运算符会调用*operator new()*来获得其所需要的内存，类似地，delete运算符会通过调用*operator delete()*来释放其所占用的内存。用户可以对全局的*operator new()*与*operator delete()*进行重载，也可以对某个特定类(particular class)进行的*operator new()*与*operator delete()*进行重载。

使用标准库(standard-library)中的```size_t```类型作为参数，我们可以通过如下方式来对全局(global)的new/delete运算符进行重载，例如：
{% highlight string %}
void * operator new(size_t);               //use for individual object
void * operator new[](size_t);             //use for array
void operator delete(void *, size_t);      //use for individual object
void operator delete[](void *, size_t);    //use for array
{% endhighlight %}

当我们需要在自由空间(free store)上为```X```类型的对象分配内存时，我们就可以使用new操作符，其就会调用*operator new(sizeof(X))*来分配内存；类似地，当我们需要在自由空间上通过new来为含有N个元素且类型为```X```的数组分配空间时，其就会调用```operator new[](N*sizeof(X))```。new表达式也许会分配多于参数所指定的```N*sizeof(X)```大小的内存，这在为字符串分配空间时经常会这样。

通常我们并推荐对全局的operator new()与operator delete()进行重载，因为这造成的影响太大。更好的选择是，单独为某个class提供new/delete运算符重载。该class可以是多个派生类的基类。在如下的例子中，Employee类就为其本身及其派生类提供了一个特定的allocator与deallocator:
{% highlight string %}
class Employee{
public:
	//...

	void * operator new(size_t);
	void operator delete(void *, size_t);

	void * operator new[](size_t);
	void operator delete(void *, size_t);
};
{% endhighlight %}
对于*operator new()*与*operator delete()*函数，其实际上是static成员。因此其并没有this指针，也并不能修改一个对象。它们只负责提供相应的空间，使得可以通过constructor来初始化一个对象，以及通过destructor来释放一个对象占用的空间。
{% highlight string %}
void *Employee::operator new(size_t s)
{
	//allocate s bytes of memory and return a pointer to it
}

void Employee::operator delete(void *p, size_t s)
{
	if(p){     //delete only if p!=0;

	      //assume p points to s bytes of memory allocated by Employee::operator new()
	      //and free that memory for reuse
	}
}
{% endhighlight %}
到此为止，对于为什么要使用神秘的```size_t```参数就变得显而易见了。size_t用于指定被删除对象的大小。删除一个普通的(plain)Employee对象我们可以传递sizeof(Employee)；而删除一个派生自Employee的Manager对象，假如Manager并没有其自身的operator delete()函数时，其所传递的值就为sizeof(Manager)。这样就使得一个class-specific allocator不用在每次内存分配时另外保存额外的size信息。自然地，一个class-specific allocator也可以保存size_t信息(类似于通用的allocator)，这样调用operator delete()删除时就可以忽略size_t参数。然而，如果不提供size_t参数，这通常使得我们难以较大的提高内存分配速率。

编译器如何为operator delete()提供正确的size_t呢？delete运算符会匹配所要删除对象的类型。假如我们通过一个父对象的指针来删除一个派生类对象的话，则父类必须要有一个virtual析构函数来提供正确的对象空间大小：
{% highlight string %}
Employee∗ p = new Manager;   // potential trouble (the exact type is lost)
// ...
delete p;                    // hope Employee has a virtual destructor
{% endhighlight %}
通常，deallocator会在析构函数之后被调用（析构函数知道其对应的类大小）


2) **示例**

当我们在C++中使用new和delete时，其实执行的是全局的::operator new()和::operator delete()。首先我们来看一个简单的例子：
{% highlight string %}
class Foo{...};
Foo *pf = new Foo;

delete pf;
{% endhighlight %}
上面的代码底层执行的是什么呢？
<pre>
new包含两阶段的操作：首先调用::operator new()分配内存; 之后调用 Foo::Foo()构造对象内容

delete也分为两部分的操作： 首先调用Foo::~Foo()将对象析构； 之后调用::operator delete()释放内存
</pre>
如下我们给出一个示例对new/delete进行重载：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include<iostream>
using namespace std;

class Foo
{
public:
	int _id;
	long _data;
	string _str;
public:
	Foo():_id(0){
		cout<<"default constructor.this="<<this<<" id="<<_id<<endl;
	}
	Foo(int i):_id(i){
		cout<<"constructor.this="<<this<<" id="<<_id<<endl;
	}
	~Foo() {
		cout<<"destructor.this="<<this<<" id="<<_id<<endl;
	}
	
	static void* operator new(size_t size);
	
	static void operator delete(void* pdead,size_t size);
	
	static void* operator new[](size_t size);
	
	static void operator delete[](void* pdead,size_t size);
};

void* Foo::operator new(size_t size)
{
	Foo* p = (Foo *)malloc(size);
	char *q = (char *)p;
	long *r = (long *)(p+4);
	*r = 20;
	
	cout<<"调用了Foo::operator new"<<endl;
	return p;
}

void Foo::operator delete(void *pdead,size_t size)
{
	cout<<"调用了Foo::operator delete"<<endl;
	free(pdead);
}

void* Foo::operator new[](size_t size)
{
	Foo* p  = (Foo*)malloc(size);
	cout<<"调用了Foo::operator new[]"<<endl;
	return p;
}

void Foo::operator delete[](void *pdead, size_t size)
{
	cout<<"调用了Foo::operator delete[]"<<endl;
	free(pdead);
}

int main(int argc, char *argv[])
{
	Foo* pf = new Foo(7);
	Foo* pf1 = new Foo[10];

	cout<<"_data:"<<pf->_data<<std::endl;

	delete pf;
	delete[] pf1;

	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
调用了Foo::operator new
constructor.this=0x1880010 id=7
调用了Foo::operator new[]
default constructor.this=0x1880038 id=0
default constructor.this=0x1880050 id=0
default constructor.this=0x1880068 id=0
default constructor.this=0x1880080 id=0
default constructor.this=0x1880098 id=0
default constructor.this=0x18800b0 id=0
default constructor.this=0x18800c8 id=0
default constructor.this=0x18800e0 id=0
default constructor.this=0x18800f8 id=0
default constructor.this=0x1880110 id=0
_data:0
destructor.this=0x1880010 id=7
调用了Foo::operator delete
destructor.this=0x1880110 id=0
destructor.this=0x18800f8 id=0
destructor.this=0x18800e0 id=0
destructor.this=0x18800c8 id=0
destructor.this=0x18800b0 id=0
destructor.this=0x1880098 id=0
destructor.this=0x1880080 id=0
destructor.this=0x1880068 id=0
destructor.this=0x1880050 id=0
destructor.this=0x1880038 id=0
调用了Foo::operator delete[]
</pre>
最后看结果和我们预想的一样，::operator new()和::operator delete()都被重载了，并且执行顺序也是我们预想的。但是如果使用全局的::operator new和::operator delete()会怎样呢？
{% highlight string %}
int main(int argc, char *argv[])
{
	Foo* pf = ::new Foo(7);
	Foo* pf1 = ::new Foo[10];

	cout<<"_data:"<<pf->_data<<std::endl;

	::delete pf;
	::delete[] pf1;

	return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
constructor.this=0xcb7010 id=7
default constructor.this=0xcb7038 id=0
default constructor.this=0xcb7050 id=0
default constructor.this=0xcb7068 id=0
default constructor.this=0xcb7080 id=0
default constructor.this=0xcb7098 id=0
default constructor.this=0xcb70b0 id=0
default constructor.this=0xcb70c8 id=0
default constructor.this=0xcb70e0 id=0
default constructor.this=0xcb70f8 id=0
default constructor.this=0xcb7110 id=0
_data:0
destructor.this=0xcb7010 id=7
destructor.this=0xcb7110 id=0
destructor.this=0xcb70f8 id=0
destructor.this=0xcb70e0 id=0
destructor.this=0xcb70c8 id=0
destructor.this=0xcb70b0 id=0
destructor.this=0xcb7098 id=0
destructor.this=0xcb7080 id=0
destructor.this=0xcb7068 id=0
destructor.this=0xcb7050 id=0
destructor.this=0xcb7038 id=0
</pre>
看到我们重载的函数被屏蔽了，因为使用的是全局::operator new()和::operator delete()。





<br />
<br />

**[参看]:**




<br />
<br />
<br />





