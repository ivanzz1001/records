---
layout: post
title: STL源码分析学习笔记(allocator)
tags:
- cplusplus
categories: cplusplus
description: STL源码分析学习笔记
---


以STL的运用角度而言，空间配置器是最不需要介绍的东西，它总是隐藏在一切组件（更具体地说是指容器，container)的背后，默默工作，默默付出。但若以STL的实现角度而言，第一个需要介绍的就是空间配置器，因为整个STL的操作对象（所有的数值）都存放在容器之内，而容器一定需要配置空间以置放资料。不先掌握空间配置器的原理，难免在阅读其他STL组件的实现时处处遇到拦路石。

为什么不说allocator是内存配置器而说它是空间配置器呢？因为空间不一定是内存，空间也可以是磁盘或其他辅助存储介质。是的，你可以写一个allocator，直接向硬盘取空间。以下介绍的是SGI STL提供的配置器，配置的对象，呃，是的，是内存。



<!-- more -->

## 1. 空间配置器的标准接口
根据STL的规范，以下是allocator的必要接口（截取自```stl_alloc.h```）：
{% highlight string %}
template <class _Tp>
class allocator {
  typedef alloc _Alloc;          // The underlying allocator.
public:
  typedef size_t     size_type;
  typedef ptrdiff_t  difference_type;
  typedef _Tp*       pointer;
  typedef const _Tp* const_pointer;
  typedef _Tp&       reference;
  typedef const _Tp& const_reference;
  typedef _Tp        value_type;

  template <class _Tp1> struct rebind {
    typedef allocator<_Tp1> other;
  };

  allocator() __STL_NOTHROW {}
  allocator(const allocator&) __STL_NOTHROW {}

  //泛化的copy constructor
  template <class _Tp1> allocator(const allocator<_Tp1>&) __STL_NOTHROW {}

  ~allocator() __STL_NOTHROW {}

  pointer address(reference __x) const { return &__x; }
  const_pointer address(const_reference __x) const { return &__x; }

  // __n is permitted to be 0.  The C++ standard says nothing about what
  // the return value is when __n == 0.
  _Tp* allocate(size_type __n, const void* = 0) {
    return __n != 0 ? static_cast<_Tp*>(_Alloc::allocate(__n * sizeof(_Tp))) 
                    : 0;
  }

  // __p is not permitted to be a null pointer.
  void deallocate(pointer __p, size_type __n)
    { _Alloc::deallocate(__p, __n * sizeof(_Tp)); }

  size_type max_size() const __STL_NOTHROW 
    { return size_t(-1) / sizeof(_Tp); }

  void construct(pointer __p, const _Tp& __val) { new(__p) _Tp(__val); }
  void destroy(pointer __p) { __p->~_Tp(); }
};
{% endhighlight %}


## 2. 设计一个简单的空间配置器
根据前述的标准接口，我们可以自行完成一个功能简单、接口不怎么齐全的allocator如下(```jjalloc.h```):
{% highlight string %}
#ifndef _JJALLOC_
#define _JJALLOC_

#include <new>              //for placement new
#include <cstddef>         //for ptrdiff_t, size_t
#include <cstdlib>         //for exit()
#include <climits>         //for UINT_MAX
#include <iostream>        //for cerr


namespace JJ{

template <class T>
inline T* _allocate(ptrdiff_t size, T*)
{
	set_new_handler(0);
	T *tmp = (T *)(::operator new((size_t)(size * sizeof(T))));
	if(tmp == 0){
		std::cerr << "out of memory" << std::endl;
		exit(1);
	}
	
	return tmp;
}


template <class T>
inline void _deallocate(T *buffer)
{
	::operator delete(buffer);
}

template<class T1, class T2>
inline void _construct(T1 *p, const T2& value)
{
	new(p) T2(value);
}

template <class T>
inline void _destory(T *ptr)
{
	ptr->~T();
}

template<class T>
class allocator{
public:
	typedef T             value_type;
	typedef T*            pointer;
	typedef const T*      const_pointer;
	typedef T&            reference;
	typedef const T&      const_reference;
	typedef size_t        size_type;
	typedef ptrdiff_t     difference_type;   //ptrdiff_t类型变量通常用来保存两个指针减法操作的结果
	
	//rebind allocator of type U
	template <class U>
	struct rebind{
		typedef allocator<U> other;
	};
	
	
	//hint used for locality. 
	pointer allocate(size_type n, const void *hint = 0)
	{
		return _allocate((difference_type)n, (pointer)0);
	}
	
	void deallocate(pointer p, size_type n)
	{
		_deallocate(p);
	}
	
	void construct(pointer p, const T& value){
		_construct(p, value);
	}
	
	void destroy(pointer p){
		_destory(p);
	}
	
	pointer address(reference x){
		return (pointer)&x;
	}
	
	const_pointer address(const_reference x)
	{
		return (const_pointer)&x;
	}
	
	size_type max_size() const{
		return size_type(UINT_MAX)/sizeof(T);
	}
};

}

#endif

{% endhighlight %}


将```JJ::allocator```应用于程序之中，我们发现，它只能有限度地搭配PJ STL和RW STL，例如(```jjalloc.cpp```)：
{% highlight string %}
#include "jjalloc.h"
#include <vector>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	int ia[5] = {0, 1, 2, 3, 4};
	unsigned int i;
	
	vector<int, JJ::allocator<int> > iv(ia, ia+5);
	
	for(i = 0; i<iv.size(); i++)
		cout<<iv[i]<< ' ';
		
	cout<<endl;
	
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -fpermissive -o jjalloc jjalloc.cpp -lstdc++
# ./jjalloc 
0 1 2 3 4 
</pre>







<br />
<br />

**[参看]:**



<br />
<br />
<br />





