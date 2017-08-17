---
layout: post
title: 设计模式之singleton
tags:
- c/c++
- 设计模式
categories: 设计模式
description: 设计模式之singleton
---

singleton模式在我们的程序设计中经常会用到，这里我们主要讲解一下singleton模式中的double check机制。

<!-- more -->


singleton模式很简单，但是很重要的一点就是必须要保证在多线程环境下线程的安全。我们使用singleton模式来保证线程的安全，通常我们会这样写：
{% highlight string %}
class singleton{
private:
	static singleton *instance;

public:
	static singleton *getinstance();

private:
	singleton();			//将构造函数设为private以保证只能通过getinstance()获取对象实例。
};

singleton * singleton::instance = NULL;

singleton::singleton()
{
	//init
}

{% endhighlight %}

**版本1：***
{% highlight string %}
singleton * singleton::getinstance()
{
	if(NULL == instance)
	{
		instance = new singleton();
	}
	return instance;
}
{% endhighlight %}









<br />
<br />
<br />


