---
layout: post
title: C++类成员初始化顺序
tags:
- cplusplus
categories: cplusplus
description: C++类成员初始化顺序
---

本文讲述一下C++类成员初始化顺序。在此做一个记录。

<!-- more -->

 
## 1. C++类成员初始化顺序
开门见山，我们先给出如下示例(member_init.cpp)：
{% highlight string %}
#include <iostream>


int set_i()
{
	std::cout<<"set_i() called"<<std::endl;
	return 0x01;
}

int set_j()
{
	std::cout<<"set_j() called"<<std::endl;
	return 0x01;
}

class A{
public:
	A()
	{
		std::cout<<"A constructor called"<<std::endl;
	}
};

class B{
public:
	B():j(set_j()),i(set_i())
	{
		std::cout<<"B constructor called"<<std::endl;
	}

private:
	int i;
	A ca;
	int j;

	static A sca;
};

A B::sca;

int main(int argc,char *argv[])
{
	std::cout<<"main ..."<<std::endl;

	B cb;

	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o member_init member_init.cpp -lstdc++
# ./member_init 
A constructor called
main ...
set_i() called
A constructor called
set_j() called
B constructor called
</pre>
可以看出，成员初始化的顺序和成员初始化表里面的顺序是没有关系的，只和成员的声明顺序有关。另一方面我们看到，对于类的静态成员，其会在main()函数调用前先执行。



<br />
<br />

**[参考]**


1. [C++成员初始化顺序](https://www.cnblogs.com/xxNote/p/4200491.html)


<br />
<br />
<br />





