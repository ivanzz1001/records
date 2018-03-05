---
layout: post
title: 设计模式之singleton
tags:
- c/c++
- design-pattern
categories: design-pattern
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

<br />

```版本1```
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

然而，如果在多线程环境下，singleton::getinstance()同时被多个线程调用，也许第一个线程在通过if(NULL == instance)语句后被中断挂起，这时其他线程也会进入该区域，这时instance = new singleton()；语句就会被调用两次或更多，违背了singleton模式的初衷。为了保证对象构造区域为一个互斥区间，这时我们考虑引入mutex互斥信号量。比如：
<br />

```版本2```
{% highlight string %}
singleton * singleton::getinstance()
{
	lock(mutex);
	if(NULL == instance)
	{
		instance = new singleton();
	}
	release(mutex);

	return instance;
}
{% endhighlight %}
现在看起来已经足够安全了，只可能同时有一个线程进入对象构造区域。

此时出现了一个性能问题，每次调用getinstance()方法时都需要执行lock(mutex)与release(mutex)的操作，而事实上第一次调用之后，instance就不是NULL值了。这时候我们就设计了一个double check机制。
<br />


```版本3```
{% highlight string %}
singleton * singleton::getinstance()
{
	if(NULL == instance)
	{
		//对象实例第一次被创建之后，没有线程会进入该区域了，因此该版本的性能与版本1几乎相同，且安全性与版本2一样好。
		lock(mutex);
		if(NULL == instance)
		{
			instance = new singleton();
		}
		release(mutex);
	}
	return instance;
}
{% endhighlight %}







<br />
<br />
<br />


