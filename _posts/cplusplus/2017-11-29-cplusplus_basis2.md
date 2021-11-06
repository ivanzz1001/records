---
layout: post
title: C++拷贝构造函数与赋值构造函数
tags:
- cplusplus
categories: cplusplus
description: C/C++基础
---

本章主要记录一下C/C++基础方面的一些内容，以备后续查验。当前所使用编译器版本：
<pre>
# gcc --version
gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-44)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
</pre>


<!-- more -->


## 1. 前言
C++中经常用到拷贝构造函数、赋值构造函数，这些函数的调用时机各有区别，这里进行简单总结：
{% highlight string %}
A();                           //默认构造函数
A(const A& a);                 //拷贝构造函数
A(A&& a);                     //移动构造函数
A& operator=(const A& a);     //赋值构造函数(拷贝赋值)
A& operator=(A&& a);           //赋值构造函数(移动赋值)
{% endhighlight %}

先写一个类，用作之后的示例：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <utility>

class A{
public:
	A() = default;
	
	~A(){
		printf("delete: 0x%x\n", this);
	}
	
	A(int t){
		x = new int(0);
		y = t;
		
		printf("address: 0x%x point: 0x%x value: %d\n", this, x, y);
	}
	
	A(const A &a){
		printf("拷贝构造\n");
		this->x = a.x;
		this->y = a.y;
	}

	A(A &&a){
		printf("移动构造\n");
		this->x = a.x;
		this->y = a.y;
	}
	
	A & operator=(const A &a){
		printf("赋值构造\n");
		this->x = a.x;
		this->y = a.y;
	}
	
public:
	int *x;
	int y;
};

A func(){

	A ret(3);
	
	printf("stack address: 0x%x point: 0x%x value: %d\n", &ret, ret.x, ret.y);
	
	return ret;
}

void g(A ret){
	printf("stack address: 0x%x point: 0x%x value: %d\n", &ret, ret.x, ret.y);
}
{% endhighlight %}


## 2. 拷贝构造函数
1) **对象需要通过另外一个对象进行初始化**
{% highlight string %}
int main(int argc, char *argv[])
{
        A a(1);
        A c = a;

        printf("global address: 0x%x point: 0x%x value: %d\n", &c, c.x, c.y);

        return 0x0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++ -std=c++11
# ./test
address: 0x8a722030 point: 0x10c2010 value: 1
拷贝构造
global address: 0x8a722020 point: 0x10c2010 value: 1
delete: 0x8a722020
delete: 0x8a722030
</pre>

2) **对象通过值传递方式进入函数**
{% highlight string %}
int main(int argc, char *argv[])
{
	A a (1);
	g (a);
	
	return 0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++ -std=c++11
# ./test
address: 0x62020930 point: 0x1694010 value: 1
拷贝构造
stack address: 0x62020940 point: 0x1694010 value: 1
delete: 0x62020940
delete: 0x62020930
</pre>

3) **当对象以值传递的方式从函数返回**
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>

class A{
public:
	A() = default;
	
	~A(){
	        printf("delete: 0x%x\n", this);
	}
	
	A(int t){
	        x = new int(0);
	        y = t;
	
	        printf("address: 0x%x point: 0x%x value: %d\n", this, x, y);
	}
	
	A(const A &a) = delete;    //注意把拷贝构造设为禁止使用
	
	A & operator=(const A &a){
	        printf("赋值构造\n");
	        this->x = a.x;
	        this->y = a.y;
	}

public:
	int *x;
	int y;
};

A func(){

	A ret(3);
	
	printf("stack address: 0x%x point: 0x%x value: %d\n", &ret, ret.x, ret.y);
	
	return ret;
}

void g(A ret){
	printf("stack address: 0x%x point: 0x%x value: %d\n", &ret, ret.x, ret.y);
}

int main(int argc, char *argv[])
{
	A c = func();
	printf("global address: 0x%x point: 0x%x value: %d\n", &c, c.x, c.y);
	
	return 0;
}
{% endhighlight %}
编译时发生错误，提示"return ret"调用了被禁止使用的拷贝构造函数：
<pre>
error: use of deleted function 'A:A(const A&)'
</pre>
若将```拷贝构造函数```放开，则能正常编译：
<pre>
# gcc -o test test.cpp -lstdc++ -std=c++11
# ./test
address: 0xd34cdc60 point: 0x1b1c010 value: 3
stack address: 0xd34cdc60 point: 0x1b1c010 value: 3
global address: 0xd34cdc60 point: 0x1b1c010 value: 3
delete: 0xd34cdc60
</pre>

关于此种情况还有如下：
{% highlight string %}
int main(int argc, char *argv[])
{
	A a = A(20);
	printf("global address: 0x%x point: 0x%x value: %d\n", &a, a.x, a.y);
	
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++ -std=c++11
# ./test
address: 0x81dcdff0 point: 0x874010 value: 20
global address: 0x81dcdff0 point: 0x874010 value: 20
delete: 0x81dcdff0
</pre>


## 3. 赋值构造函数
1) **对象以值传递方式从函数返回，且接受返回值的对象已经初始化过**
{% highlight string %}
int main(int argc, char *argv[])
{
	A c;
	c = func();
	printf("global address: 0x%x point: 0x%x value: %d\n", &c, c.x, c.y);
	
	return 0;
}
{% endhighlight %}

编译运行：
<pre>
# gcc -o test test.cpp -lstdc++ -std=c++11
[root@compile just_for_test]# ./test
address: 0xfa9d51c0 point: 0x1ab3010 value: 3
stack address: 0xfa9d51c0 point: 0x1ab3010 value: 3
赋值构造
delete: 0xfa9d51c0
global address: 0xfa9d51b0 point: 0x1ab3010 value: 3
delete: 0xfa9d51b0
</pre>

2) **对象直接赋值给另一个对象，且接受值的对象已经初始化过**
{% highlight string %}
int main(int argc, char *argv[])
{
	A a(1);
	A c;
	c = a;
	printf ("global address: %x, point: %x, value: %d\n", &c, c.x, c.y);
	
	return 0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++ -std=c++11
# ./test
address: 0xc36068d0 point: 0x2183010 value: 1
赋值构造
global address: c36068c0, point: 2183010, value: 1
delete: 0xc36068c0
delete: 0xc36068d0
</pre>
注： 如果把上面的```赋值构造函数```注释掉，还是会调用默认的赋值构造函数。

## 4. 移动构造函数
{% highlight string %}
int main(int argc, char *argv[])
{
	A a(std::move(A(200)));
	
	printf("global address: 0x%x point: 0x%x value: %d\n", &a, a.x, a.y);
	
	return 0x0;

}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++ -std=c++11
# ./test
address: 0x414b3e90 point: 0x21aa010 value: 200
移动构造
delete: 0x414b3e90
global address: 0x414b3e80 point: 0x21aa010 value: 200
delete: 0x414b3e80
</pre>
注意： 这里移动构造赋值也只会调用一次构造函数。

## 5. 总结
对象以值传递方式从函数返回时，若接受返回值的对象已经初始化过，则会调用赋值构造函数，且该对象还会调用析构函数，当对象中包含指针时，会使该指针失效，因此需要重载赋值构造函数，使用类似深拷贝或```移动构造函数```的方法赋值，才能避免指针失效。


对象以值传递方式从函数返回时，若接受返回值的对象是由该返回值初始化，则不会调用任何构造函数,且不会调用析构函数。（这一点，请参看上面2.3部分)。




<br />
<br />

**[参看]:**

1. [C++拷贝构造、赋值构造详解](https://blog.csdn.net/qq_31759205/article/details/80544468)


<br />
<br />
<br />





