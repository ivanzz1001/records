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


### 1.1 设计一个简单的空间配置器
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

“只能有限度搭配PJ STL”是因为，PJ STL未完全遵循STL规格，其所供应的许多容器都需要一个非标准的空间配置器接口allocator::_Charalloc()。“只能有限度的搭配RW STL”则是因为，RW STL在许多容器上运用了缓冲区，情况复杂得多，JJ::allocator无法与之兼容。至于完全无法应用于SGI STL，是因为SGI STL在这个项目上根本就逸脱了STL标准规格，使用一个专属的、拥有次层(sub-allocation)能力的、效率优越的特殊配置器，稍后有详细介绍。

>注：对比新版SGI STL v3.3中allocator的实现，JJ::allocator应该是可以兼容的。

我想我可以提前先做一点说明。事实上SGI STL仍然提供了一个标准的配置器接口，只是把它做了一层隐藏。这个标准的配置器名为```simple_alloc```，稍后便会提到。


## 2. 具备次配置力(sub-allocation)的SGI空间配置器
SGI STL的配置器与众不同，也与标准规范不同，其名称为```alloc```而非allocator，而且不接受任何参数。换句话说，如果你要在程序中明白采用SGI配置器，则不能采用标准写法：
{% highlight string %}
vector<int, std::allocator<int> > iv;            //in VC or CB
{% endhighlight %}

必须这么写：
{% highlight string %}
vector<int, std::alloc> iv;                     //in GCC
{% endhighlight %}

>说明：在SGI STL v3.3版本stl_config.h中，默认是采用std::alloc而非std::allocator
{%highlight string %}
// Use standard-conforming allocators if we have the necessary language
// features.  __STL_USE_SGI_ALLOCATORS is a hook so that users can 
// disable new-style allocators, and continue to use the same kind of
// allocators as before, without having to edit library headers.
# if defined(__STL_CLASS_PARTIAL_SPECIALIZATION) && \
     defined(__STL_MEMBER_TEMPLATES) && \
     defined(__STL_MEMBER_TEMPLATE_CLASSES) && \
    !defined(__STL_NO_BOOL) && \
    !defined(__STL_NON_TYPE_TMPL_PARAM_BUG) && \
    !defined(__STL_LIMITED_DEFAULT_TEMPLATES) && \
    !defined(__STL_USE_SGI_ALLOCATORS) 
#   define __STL_USE_STD_ALLOCATORS
# endif

# ifndef __STL_DEFAULT_ALLOCATOR
#   ifdef __STL_USE_STD_ALLOCATORS
#     define __STL_DEFAULT_ALLOCATOR(T) allocator< T >
#   else
#     define __STL_DEFAULT_ALLOCATOR(T) alloc
#   endif
# endif
{% endhighlight %}


SGI STL allocator未能符合标准规格（注：新版SGI STL v3.3中allocator似乎已经符合了标准规格），这个事实通常不会给我们带来困扰，因为通常我们使用缺省的空间配置器，很少需要自行指定配置器名称，而SGI STL的每一个容器都已经指定其缺省的空间配置器为```alloc```。例如下面的vector声明：
{% highlight string %}
template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp) >
class vector : protected _Vector_base<_Tp, _Alloc> 
{
  // requirements:
};
{% endhighlight %}

### 2.1 SGI标准的空间配置器std::allocator
在SGI STL v3.3版本中，SGI提供的std::allocator似乎已经已经符合了标准，这里不再对其进行说明。

### 2.2 SGI特殊的空间配置器std::alloc

一般而言，我们所习惯的C++内存配置操作和释放操作是这样的：
{% highlight string %}
class Foo{...};

Foo *pf = new Foo;                       //内存配置，然后构造对象
delete pf;                               //将对象析构，然后释放内存
{% endhighlight %}


这其中的```new```算式内含两阶段操作：1） 调用```::operator new```配置内存；2) 调用Foo::Foo()构造对象内容。```delete```算式也内含两阶段操作：1）调用Foo::~Foo()将对象析构；2）调用```::operator delete```释放内存。

为了精密分工，STL allocator决定将这两阶段操作区分开来。内存配置操作由alloc::allocate()负责，内存释放操作由alloc::deallocate()负责；对象构造操作由::construct()负责，对象析构由::destroy()负责。


STL标准规格告诉我们，配置器定义于```<memory>```之中，SGI ```<memory>```内含以下两个文件：
{% highlight string %}
#include <stl_alloc.h>                      //负责内存空间的配置与释放
#include <stl_construct.h>                  //负责对象内容的构造与析构
{% endhighlight %}

内存空间的配置/释放与对象内容的构造/析构，分别落在这两个文件身上。其中<stl_construct.h>定义有两个基本函数：构造用的construct()和析构用的destroy()。在一头栽进复杂的内存动态配置与释放之前，让我们先看清楚这两个函数如何完成对象的构造和析构。

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part2_1.jpg)



### 2.3 构造和析构基本工具: construct()和destory()
下面是<stl_construct.h>的部分内容：
{% highlight string %}
#include <new.h>                             //欲使用placement new，需先包含此文件


template <class _T1, class _T2>
inline void _Construct(_T1* __p, const _T2& __value) {
  new ((void*) __p) _T1(__value);
}

template <class _T1>
inline void _Construct(_T1* __p) {
  new ((void*) __p) _T1();
}


template <class _Tp>
inline void _Destroy(_Tp* __pointer) {
  __pointer->~_Tp();
}

template <class _ForwardIterator>
void
__destroy_aux(_ForwardIterator __first, _ForwardIterator __last, __false_type)
{
  for ( ; __first != __last; ++__first)
    destroy(&*__first);
}

template <class _ForwardIterator> 
inline void __destroy_aux(_ForwardIterator, _ForwardIterator, __true_type) {}


//如果元素的数值型别(value type)有non-trival destructor...
template <class _ForwardIterator, class _Tp>
inline void 
__destroy(_ForwardIterator __first, _ForwardIterator __last, _Tp*)
{
  typedef typename __type_traits<_Tp>::has_trivial_destructor
          _Trivial_destructor;
  __destroy_aux(__first, __last, _Trivial_destructor());
}

template <class _ForwardIterator>
inline void _Destroy(_ForwardIterator __first, _ForwardIterator __last) {
  __destroy(__first, __last, __VALUE_TYPE(__first));
}

inline void _Destroy(char*, char*) {}
inline void _Destroy(int*, int*) {}
inline void _Destroy(long*, long*) {}
inline void _Destroy(float*, float*) {}
inline void _Destroy(double*, double*) {}
#ifdef __STL_HAS_WCHAR_T
inline void _Destroy(wchar_t*, wchar_t*) {}
#endif /* __STL_HAS_WCHAR_T */
{% endhighlight %}
这两个作为构造、析构之用的函数被设计为全局函数，符合STL的规范。此外，STL还规定配置器必须拥有名为construct()和destroy()的两个成员函数，然而真正在SGI STL中大显身手的那个名为std::alloc的配置器并未遵守这一规则（稍后可见）

上述construct()接受一个指针p和一个初值value，该函数的用途就是将初值设定到指针所指的空间上。C++的placement new运算子可用来完成这一任务。

destroy()有两个版本，第一个版本接受一个指针，准备将该指针所指之物析构掉。这很简单，直接调用该对象的析构函数即可。第二版本接受first和last两个迭代器（所谓迭代器，我们会在后面第3章进行详细介绍），准备将[first, last)范围内的所有对象析构掉。我们不知道这个范围有多大，万一很大，而每个对象的析构函数都无关痛痒(所谓trival destructor)，那么一次次调用这些无关痛痒的析构函数，对效率是一种伤害。因此，这里首先利用```__VALUE_TYPE()```获得迭代器所指对象的型别，再利用```__type_traits<T>```判断该型别的析构函数是否无关痛痒。若是(```__true_type```)，则什么也不做就结束；否则(```__false_type```)，这才以循环方式巡访整个范围，并在循环中每经历一个对象就调用第一个版本的destroy()。

这样的观念很好，但C++本身并不支持对“指针所指之物”的型别判断，也不支持对“对象析构函数是否为trivial”的判断，因此，上述的```__VALUE_TYPE()```和```__type_traits<>```该如何实现呢？我们会在第3.7节有详细介绍。

### 2.4 空间的配置与释放
看完了内存配置后的对象构造行为和内存释放前的对象析构行为，现在我们来看看内存的配置和释放。

对象构造前的空间配置和对象析构后的空间释放，由```<stl_alloc.h>```负责，SGI对此的设计哲学如下：

* 向system heap要求空间；

* 考虑多线程(multi-threads)状态；

* 考虑内存不足时的应变措施；

* 考虑过多“小型区块”可能造成的内存碎片(fragment)问题；

为了将问题控制在一定的复杂度内，以下的讨论以及所摘录的源代码，皆排除多线程状态的处理。

C++的内存配置基本操作是```::operator new()```，内存释放基本操作是```::operator delete()```。这两个全局函数相当于C的malloc()和free()函数。是的，正是如此，SGI正是以malloc()和free()完成内存的配置与释放。

考虑到小型区块所可能造成的内存破碎问题，SGI设计了双层级配置器，第一级配置器直接使用malloc()和free()，第二级配置器则视情况采用不同的策略：当配置区块超过128bytes时，视之为“足够大”，便调用第一级配置器；当配置区块小于128bytes时，视之为“过小”，为了降低额外负担(overhead，见2.2.6节)，便采用复杂的memory pool整理方式，而不再求助于第一级配置器。整个设计究竟只开放第一级配置器，或是同时开放第二级配置器，取决于```__USE_MALLOC```是否被定义（唔，我们可以轻易测试出来，SGI STL并未定义```__USE_MALLOC```):
{% highlight string %}
# ifdef __USE_MALLOC

typedef __malloc_alloc_template<0> malloc_alloc;
typedef malloc_alloc alloc;                                             //令alloc为第一级配置器
typedef malloc_alloc single_client_alloc;

# else

...
typedef __default_alloc_template<__NODE_ALLOCATOR_THREADS, 0> alloc;    //令alloc为第二级配置器
typedef __default_alloc_template<false, 0> single_client_alloc;

#endif
{% endhighlight %}
其中```__malloc_alloc_template```就是第一级配置器，```__default_alloc_template```就是第二级配置器。稍后分别有详细分析。再次提醒你注意，alloc并不接受任何templete型别参数。

无论alloc被定义为第一级或第二级配置器，SGI还为它再包装一个接口如下，使配置器的接口能够符合STL规格：
{% highlight string %}
template<class _Tp, class _Alloc>
class simple_alloc {

public:
    static _Tp* allocate(size_t __n)
      { return 0 == __n ? 0 : (_Tp*) _Alloc::allocate(__n * sizeof (_Tp)); }

    static _Tp* allocate(void)
      { return (_Tp*) _Alloc::allocate(sizeof (_Tp)); }

    static void deallocate(_Tp* __p, size_t __n)
      { if (0 != __n) _Alloc::deallocate(__p, __n * sizeof (_Tp)); }

    static void deallocate(_Tp* __p)
      { _Alloc::deallocate(__p, sizeof (_Tp)); }
};
{% endhighlight %}
其内部四个成员函数其实都是单纯的转调用，调用传递给配置器（可能是第一级也可能是第二级）的成员函数。这个接口使配置器的配置单位从bytes转为个别元素的大小(sizeof(T))。SGI STL容器全都使用这个```simple_alloc```接口，例如：
{% highlight string %}
//stl_config.h
# ifndef __STL_DEFAULT_ALLOCATOR
#   ifdef __STL_USE_STD_ALLOCATORS
#     define __STL_DEFAULT_ALLOCATOR(T) allocator< T >
#   else
#     define __STL_DEFAULT_ALLOCATOR(T) alloc
#   endif
# endif


template <class _Tp, class _Alloc> 
class _Vector_base {
	...

protected:
  typedef simple_alloc<_Tp, _Alloc> _M_data_allocator;
  _Tp* _M_allocate(size_t __n)
    { return _M_data_allocator::allocate(__n); }
  void _M_deallocate(_Tp* __p, size_t __n) 
    { _M_data_allocator::deallocate(__p, __n); }	
};

template <class _Tp, class _Alloc = __STL_DEFAULT_ALLOCATOR(_Tp) >
class vector : protected _Vector_base<_Tp, _Alloc> 
{

};
{% endhighlight %}

一、二级配置器的关系，接口包装，及实际运用方式，可于图2-2略见端倪。

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part2_2a.jpg)

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part2_2b.jpg)


### 2.5 第一级配置器__malloc_alloc_template剖析
首先我们观察第一级配置器：
{% highlight string %}
//malloc-based allocator. 通常比稍后介绍的default alloc速度慢
//一般而言是thread-safe，并且对于空间的运用比较高效(efficient)
//以下是第一级配置器
//注意，无“template型别参数”。至于“非型别参数”inst，则完全没有派上用场。

template <int __inst>
class __malloc_alloc_template {

private:

  //以下函数用来处理内存不足的情况（oom: out of memory)
  static void* _S_oom_malloc(size_t);
  static void* _S_oom_realloc(void*, size_t);

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
  static void (* __malloc_alloc_oom_handler)();
#endif

public:

  static void* allocate(size_t __n)
  {
    void* __result = malloc(__n);
    if (0 == __result) __result = _S_oom_malloc(__n);    //当无法满足需求时，改用oom_malloc()
    return __result;
  }

  static void deallocate(void* __p, size_t /* __n */)
  {
    free(__p);
  }

  static void* reallocate(void* __p, size_t /* old_sz */, size_t __new_sz)
  {
    void* __result = realloc(__p, __new_sz);
    if (0 == __result) __result = _S_oom_realloc(__p, __new_sz);
    return __result;
  }

  static void (* __set_malloc_handler(void (*__f)()))()
  {
    void (* __old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = __f;
    return(__old);
  }

};


// malloc_alloc out-of-memory handling

#ifndef __STL_STATIC_TEMPLATE_MEMBER_BUG
template <int __inst>
void (* __malloc_alloc_template<__inst>::__malloc_alloc_oom_handler)() = 0;
#endif


template <int __inst>
void*
__malloc_alloc_template<__inst>::_S_oom_malloc(size_t __n)
{
    void (* __my_malloc_handler)();
    void* __result;

    for (;;) {                                                //不断尝试释放、配置、在释放、再配置....
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*__my_malloc_handler)();                             //调用处理例程，企图释放内存
        __result = malloc(__n);                               //再次尝试配置内存
        if (__result) return(__result);
    }
}


template <int __inst>
void* __malloc_alloc_template<__inst>::_S_oom_realloc(void* __p, size_t __n)
{
    void (* __my_malloc_handler)();
    void* __result;

    for (;;) {                                                  //不断尝试释放、配置、在释放、再配置....
        __my_malloc_handler = __malloc_alloc_oom_handler;
        if (0 == __my_malloc_handler) { __THROW_BAD_ALLOC; }
        (*__my_malloc_handler)();                               //调用处理例程，企图释放内存
        __result = realloc(__p, __n);                           //再次尝试配置内存
        if (__result) return(__result);
    }
}


//注意，以下直接将参数inst指定为0
typedef __malloc_alloc_template<0> malloc_alloc;
{% endhighlight %}

第一级配置器以malloc()，free()，realloc()等C函数执行实际的内存配置、释放、重配置操作，并实现出类似C++ new-handler的机制。是的，它不能直接运用C++ new-handler机制，因为它并非使用::operator new来配置内存。

所谓C++ new handler机制是，你可以要求系统在内存配置需求无法被满足时，调用一个所指定的函数。换句话说，一旦::operator new无法完成任务，在丢出std::bad_alloc异常状态之前，会先调用由客端指定的处理例程。该处理例程通常即被称为new-handler。new-handler解决内存不足的做法有特定的模式，请参考<<Effective C++>> 2e条款7。

注意，SGI以malloc而非::operator new来配置内存（我所能够想象的一个原因是历史因素，另一个原因是C++并未提供相应于realloc()的内存配置操作），因此，SGI不能直接使用C++的set_new_handler()，必须仿真一个类似的set_malloc_handler()。

请注意，SGI第一级配置器的allocate()和reallocate()都是在调用malloc()和realloc()不成功后，改调用oom_malloc()和oom_realloc()。后两者都有内循环，不断调用“内存不足处理例程”，期望在某次调用之后，获得足够的内存而圆满完成任务。但如果“内存不足例程”并未被客端设定，oom_alloc()和oom_realloc()便老实不客气地调用__THROW_BAD_ALLOC，丢出bad_alloc异常信息，并利用exit(1)硬生生终止程序。


记住，设计“内存不足处理例程”是客端地责任，设定“内存不足处理例程”也是客端的责任。再一次提醒你，“内存不足处理例程”解决问题的做法有着特定的模式，请参考[Meyers98]条款7.


### 2.6 第二级配置器__default_alloc_template剖析
第二级配置器多了一些机制，避免太多小额区块造成内存的碎片。小额区块带来的其实不仅是内存碎片，配置时的额外负担(overhead)也是一个大问题。额外负担永远无法避免，毕竟系统要靠这多出来的空间来管理内存，如下图2-3所示。但是区块愈小，额外负担所占的比例就愈大，愈显得浪费。

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part2_3.jpg)

SGI第二级配置器的做法是，如果区块够大，超过128bytes时，就移交第一级配置器处理。当区块小于128bytes时，则以内存池(memory pool)管理，此法又称为次层配置(sub-allocation)：每次配置一大块内存，并维护对应之自由链表(free-list)。下次若再有相同大小的内存需求，就直接从free-lists中拔出。如果客端释还小额区块，就由配置器回收到free-lists中————是的，别忘了，配置器除了负责配置，也负责回收。为了方便管理，SGI第二级配置器会主动将任何小额区块的内存需求量上调至8的倍数（例如客端要求30bytes，就自动调整为32bytes)，并维护16个free-lists，各自管理大小分别为8，16，24，32，40，48，56， 64， 72， 80， 88， 96， 104， 112， 120， 128bytes的小额区块。free-lists的节点结构如下：
{% highlight string %}
union _Obj {
    union _Obj* _M_free_list_link;
    char _M_client_data[1];    /* The client sees this.        */
};
{% endhighlight %}

诸君或许会想，为了维护链表(lists)，每个节点需要额外的指针（指向下一个节点），这不又造成另一种额外负担吗？你的顾虑是对的，但早已有好的解决办法。注意，上述obj所用的是union，由于union之故，从其第一字段观之，obj可被视为一个指针，指向相同形式的另一个obj；从其第二字段观之，obj可被视为一个指针，指向实际区块。如图2-4所示。一物二用的结果是，不会为了维护链表所必须的指针而造成内存的另一种浪费（我们正在努力节省内存的开销呢）。这种技巧在强型(strongly typed)语言如Java中行不通，但是在非强型语言如C++中十分普遍。

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part2_4.jpg)


下面是第二级配置器的部分实现内容：
{% highlight string %}
#if defined(__SUNPRO_CC) || defined(__GNUC__)
// breaks if we make these template class members:
  enum {_ALIGN = 8};                                  //小型区块的上调边界
  enum {_MAX_BYTES = 128};                            //小型区块的上限
  enum {_NFREELISTS = 16};                            //free-lists个数
#endif


//以下是第二级配置器
//注意，无“template型别参数”，且第二参数完全没派上用场
//第一参数用于多线陈环境下。本书不讨论多线程环境
template <bool threads, int inst>
class __default_alloc_template {

  //ROUND_UP将bytes上调至8的倍数
  static size_t _S_round_up(size_t __bytes) 
    { return (((__bytes) + (size_t) _ALIGN-1) & ~((size_t) _ALIGN - 1)); }


__PRIVATE:
  union _Obj {
        union _Obj* _M_free_list_link;
        char _M_client_data[1];    /* The client sees this.        */
  };


private:
  //16个free-lists
# if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
    static _Obj* __STL_VOLATILE _S_free_list[]; 
        // Specifying a size results in duplicate def for 4.1
# else
    static _Obj* __STL_VOLATILE _S_free_list[_NFREELISTS]; 
# endif  


  //以下函数根据区块大小，决定使用第n号free-list。n从0起算
  static  size_t _S_freelist_index(size_t __bytes) {
        return (((__bytes) + (size_t)_ALIGN-1)/(size_t)_ALIGN - 1);


  // 返回一个大小为n的对象，并可能加入到free-list(Returns an object of size __n, and optionally adds to size __n free list)
  static void* _S_refill(size_t __n);


  // Allocates a chunk for nobjs of size size.  nobjs may be reduced
  // if it is inconvenient to allocate the requested number.
  static char* _S_chunk_alloc(size_t __size, int& __nobjs);


  // Chunk allocation state.
  static char* _S_start_free;                      //内存池起始位置。只在chunk_alloc()中变化
  static char* _S_end_free;                        //内存池结束位置。只在chunk_alloc()中变化
  static size_t _S_heap_size;


public:
  /* __n must be > 0      */
  static void* allocate(size_t __n)

  /* __p may not be 0 */
  static void deallocate(void* __p, size_t __n);

  static void* reallocate(void* __p, size_t __old_sz, size_t __new_sz);
};


//以下是static data member的定义与初值设定
template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_start_free = 0;

template <bool __threads, int __inst>
char* __default_alloc_template<__threads, __inst>::_S_end_free = 0;

template <bool __threads, int __inst>
size_t __default_alloc_template<__threads, __inst>::_S_heap_size = 0;

template <bool __threads, int __inst>
typename __default_alloc_template<__threads, __inst>::_Obj* __STL_VOLATILE
__default_alloc_template<__threads, __inst> ::_S_free_list[
# if defined(__SUNPRO_CC) || defined(__GNUC__) || defined(__HP_aCC)
    _NFREELISTS
# else
    __default_alloc_template<__threads, __inst>::_NFREELISTS
# endif
] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
{% endhighlight %}


### 2.7 空间配置函数allocate()
身为一个配置器，```__default_alloc_template```拥有配置器的标准接口函数allocate()。此函数首先判断区块大小，大于128bytes就调用第一级配置器，小于128bytes就检查对应的free-list。如果free-list之内有可用的区块，就直接拿来用；如果没有可用区块，就将区块大小上调至8倍数边界，然后调用refill()，准备为free list重新填充空间。refill()将于稍后介绍。
{% highlight string %}
static void* allocate(size_t __n)
{
	void* __ret = 0;
	
	if (__n > (size_t) _MAX_BYTES) {
		__ret = malloc_alloc::allocate(__n);
	}
	else {
		_Obj* __STL_VOLATILE* __my_free_list = _S_free_list + _S_freelist_index(__n);

		// Acquire the lock here with a constructor call.
		// This ensures that it is released in exit or during stack
		// unwinding.
		#     ifndef _NOTHREADS
			/*REFERENCED*/
			_Lock __lock_instance;
		#     endif

		_Obj* __RESTRICT __result = *__my_free_list;
		if (__result == 0)
			__ret = _S_refill(_S_round_up(__n));              //没找到可用的free list，准备重新填充free list
		else {
			*__my_free_list = __result -> _M_free_list_link;
			__ret = __result;
		}
	}
	
	return __ret;
};
{% endhighlight %}

区块自free list调出的操作，如下图2-5所示。

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part2_5.jpg)

### 2.8 空间释放函数deallocate()
身为一个配置器，```__default_alloc_template```拥有配置器的标准接口函数deallocate()。该函数首先判断区块大小，大于128bytes就调用第一级配置器，小于128bytes就找出对应的free list，将区块回收。

{% highlight string %}
/* __p may not be 0 */
static void deallocate(void* __p, size_t __n)
{
	if (__n > (size_t) _MAX_BYTES)
		malloc_alloc::deallocate(__p, __n);
	else {
		_Obj* __STL_VOLATILE*  __my_free_list = _S_free_list + _S_freelist_index(__n);
		_Obj* __q = (_Obj*)__p;
	
		// acquire lock
		#       ifndef _NOTHREADS
		/*REFERENCED*/
		_Lock __lock_instance;
		#       endif /* _NOTHREADS */


		//调整free list，回收区块
		__q -> _M_free_list_link = *__my_free_list;
		*__my_free_list = __q;
		// lock is released here
	}
}
{% endhighlight %}

区块回收纳入free list的操作，如下图2-6所示。

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part2_6.jpg)


### 2.9 重新填充free lists
回头讨论先前说过的allocate()。当它发现free list中没有可用区块了时，就调用refill()，准备为free list重新填充空间。新的空间将取自内存池（经由chunk_alloc()完成）。缺省取得20个新节点（新区块），但万一内存池空间不足，获得的节点数（区块数）可能小于20：
{% highlight string %}

//返回一个大小为n的对象，并且有时候会为适当的free list增加节点
//假设n已经适当上调至8的倍数
template <bool __threads, int __inst>
void* __default_alloc_template<__threads, __inst>::_S_refill(size_t __n)
{
	int __nobjs = 20;
	char* __chunk = _S_chunk_alloc(__n, __nobjs);
	_Obj* __STL_VOLATILE* __my_free_list;
	_Obj* __result;
	_Obj* __current_obj;
	_Obj* __next_obj;
	int __i;
	
	//如果只获得一个区块，这个区块就分配给调用者用，free list无新节点
	if (1 == __nobjs) return(__chunk);
	__my_free_list = _S_free_list + _S_freelist_index(__n);
	
	/* Build free list in chunk */
	__result = (_Obj*)__chunk;
	*__my_free_list = __next_obj = (_Obj*)(__chunk + __n);
	for (__i = 1; ; __i++) {
		__current_obj = __next_obj;
		__next_obj = (_Obj*)((char*)__next_obj + __n);

		if (__nobjs - 1 == __i) {
			__current_obj -> _M_free_list_link = 0;
			break;
		} else {
			__current_obj -> _M_free_list_link = __next_obj;
		}
	}
	return(__result);
}
{% endhighlight %}

### 2.10 内存池(memory pool)
从内存池中取空间给free list使用，是chunk_alloc()的工作：
{% highlight string %}

//假设size已经适当上调至8的倍数。请注意参数nobjs是paas by reference
template <bool __threads, int __inst>
char*
__default_alloc_template<__threads, __inst>::_S_chunk_alloc(size_t __size, 
                                                            int& __nobjs)
{
	char* __result;
	size_t __total_bytes = __size * __nobjs;
	size_t __bytes_left = _S_end_free - _S_start_free;                 //内存池剩余空间
	
	if (__bytes_left >= __total_bytes) {
 		
		//内存池剩余空间完全满足需求量
		__result = _S_start_free;
		_S_start_free += __total_bytes;
		return(__result);
	} else if (__bytes_left >= __size) {

		//内存池剩余空间不能完全满足需求量，但足够供应一个（含）以上的区块
		__nobjs = (int)(__bytes_left/__size);
		__total_bytes = __size * __nobjs;
		__result = _S_start_free;
		_S_start_free += __total_bytes;
		return(__result);
	} else {

		//内存池剩余空间连一个区块的大小都无法提供
		size_t __bytes_to_get = 2 * __total_bytes + _S_round_up(_S_heap_size >> 4);
 
		// 以下试着让内存池中的残余零头还有利用价值
		if (__bytes_left > 0) {
			_Obj* __STL_VOLATILE* __my_free_list = _S_free_list + _S_freelist_index(__bytes_left);
	
			((_Obj*)_S_start_free) -> _M_free_list_link = *__my_free_list;
			*__my_free_list = (_Obj*)_S_start_free;
		}

		//配置heap空间，用来补充内存池
		_S_start_free = (char*)malloc(__bytes_to_get);
		if (0 == _S_start_free) {

			//heap空间不足，malloc()失败
			size_t __i;
			_Obj* __STL_VOLATILE* __my_free_list;
			_Obj* __p;

	
			//试着检视我们手上拥有的东西。这不会造成伤害。我们不打算尝试配置较小的区块，因为那在多进程(multi-process)机器上容易导致
			//灾难。以下搜寻适当的free list，所谓适当是指“尚有未用区块，且区块足够大”之free list
			for (__i = __size; __i <= (size_t) _MAX_BYTES; __i += (size_t) _ALIGN) {
				__my_free_list = _S_free_list + _S_freelist_index(__i);
				__p = *__my_free_list;

				if (0 != __p) {                                //free list尚有未用区块
					*__my_free_list = __p -> _M_free_list_link;
					_S_start_free = (char*)__p;
					_S_end_free = _S_start_free + __i;

					return(_S_chunk_alloc(__size, __nobjs));
					//注意，任何残余零头终将被编入适当的free-list中备用
				}
			}

			_S_end_free = 0;	                              //如果出现意外（山穷水尽，到处都没内存可用了）

			//调用第一级配置器，看看out-of-memory机制能否尽点力
			_S_start_free = (char*)malloc_alloc::allocate(__bytes_to_get);

			//这会导致抛出异常(exception)，或内存不足的情况获得改善
		}

		_S_heap_size += __bytes_to_get;
		_S_end_free = _S_start_free + __bytes_to_get;

		//递归调用自己，为了修正nobjs
		return(_S_chunk_alloc(__size, __nobjs));
	}
}
{% endhighlight %}
上述的chunck_alloc()函数以end_free - start_free来判断内存池的水量。如果水量充足，就直接调出20个区块返回给free list。如果水量不足以提供20个区块，但还足够供应一个以上的区块，就拔出这不足20个区块的空间出去。这时候其pass by reference的nobjs参数将被修改为实际能够供应的区块数。如果内存池连一个区块空间都无法供应，对客端显然无法交代，此时便利用malloc()从heap中配置内存，为内存池注入源头活水以应付需求。新水量的大小为需求量的两倍，再加上一个随着配置次数增加而愈来愈大的附加量。

举个例子，见图2-7，假设程序一开始，客端就调用chunk_alloc(32,20)，于是malloc()配置40个32bytes区块，其中第1个交出，另19个交给free_list[3]维护，余20个留给内存池。接下来客端调用chunk_alloc(64, 20)，此时free_list[7]空空如也，必须向内存池要求支持。内存池只够供应(32 x 20)/64 = 10个64bytes区块，就把这10个区块返回，第1个交给客端，余9个由free_list[7]维护。此时内存池全空。接下来调用chunk_alloc(96,20)，此时free_list[11]空空如也，必须向内存池要求支持，而内存池此时也是空的，于是以malloc()配置40+n(附加量）个96bytes区块，其中第1个交出，剩余19个交给free_list[11]维护，余20+n（附加量）个区块留给内存池...

万一山穷水尽，整个system heap空间都不够了（以至无法为内存注入源头活水），malloc()行动失败，chunk_alloc()就四处寻找有无“尚有未用区块，且区块够大”之free_list。找到了就挖一块交出，找不到就调用第一级配置器。第一级配置器其实也是使用malloc()来配置内存，但它有out-of-memory处理机制（类似new-handler机制），或许有机会释放其他的内存拿来此处使用。如果可以，就成功，否则发出bad_alloc异常。

以上便是整个第二级空间配置器的设计。

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part2_7.jpg)

回想一下2.2.4节最后提到的那个提供配置器标准接口的simple_alloc:
{% highlight string %}

template<class _Tp, class _Alloc>
class simple_alloc {
  ...
};
{% endhighlight %}

SGI容器通常以这种方式来使用配置器：
{% highlight string %}
template <class T, class Alloc = alloc>                    //缺省使用alloc配置器
class vector{
public:
  typedef T value_type;
  ...

protected:  
  //专属之空间配置器，每次配置一个元素大小
  typedef simple_alloc<value_type, Alloc> data_allocator;
  ... 
};
{% endhighlight %}

其中第二个template参数所接受的缺省参数alloc，可以是第一级配置器，也可以是第二级配置器。不过，SGI STL已经把它设为第二级配置器，见2.2.4节及图2-2b。


## 3. 内存基本处理工具
STL定义有五个全局函数，作用于未初始化空间上。这样的功能对于容器的实现很有帮助，我们会在第4章容器实现代码中，看到它们肩负的重任。前两个函数是2.2.3节说过的，用于构造的construct()和用于析构的destroy()，另三个函数是uninitialized_copy(), uninitialized_fill(), uninitialized_fill_n()，分别对应于高层次函数copy()、fill()、fill_n()————这些都是STL算法，将在第6章介绍。如果你要使用本节的三个低层次函数，应该包含<memory>，不过SGI把它们实际定义于<stl_uninitialized>。

### 3.1 uninitialized_copy
{% highlight string %}
template <class _InputIter, class _ForwardIter>
inline _ForwardIter
  uninitialized_copy(_InputIter __first, _InputIter __last,
                     _ForwardIter __result);
{% endhighlight %}
uninitialized_copy()使我们能够将内存的配置与对象的构造行为分离开来。如果作为输出目的地的[result, result + (last -first))范围内的每一个迭代器都指向未初始化区域，则uninitialized_copy()会使用copy constructor，给身为输入来源之[first, last)范围内的每一个对象产生一份复制品，放进输出范围中。换句话说，针对输入范围内的每一个迭代器i，该函数会调用```construct(&*(result + (i - first)), *i)```，产生```*i```的复制品，放置于输出范围的相对位置上。式中的construct()已于2.2.3节讨论过。

如果你需要实现一个容器，uninitialized_copy()这样的函数会为你带来很大的帮助，因为容器的全区间构造函数(range constructor)通常以两个步骤完成：

* 配置内存区块，足以包含范围内的所有元素

* 使用uninitialized_copy()，在该内存区块上构造元素

C++标准规格书要求uninitialized_copy()具有“commit or rollback”语义，意思就是要么“构造出所有必要元素”，要么（当有任何一个copy constructor失败时）“不构造任何东西”。

### 3.2 uninitialized_fill
{% highlight string %}
template <class _ForwardIter, class _Tp>
inline void uninitialized_fill(_ForwardIter __first,
                               _ForwardIter __last, 
                               const _Tp& __x);
{% endhighlight %}
uninitialized_fill()也能够使我们将内存配置与对象的构造行为分离开来。如果[first, last)范围内的每个迭代器都指向未初始化的内存，那么uninitialized_fill()会在该范围内产生x(上式第三参数）的复制品。换句话说，uninitialized_fill()会针对操作范围内的每个迭代器i，调用```construct(&*i, x)```，在i所指之处产生x的复制品。式中construct()已于2.2.3节讨论过。

与uninitialized_copy()一样，uninitialized_fill()具有“commit or rollback”语义，它要么产生出所有必要元素，要么不产生任何元素。如果有任何一个copy constructor丢出异常(exception)，uninitialized_fill()必须能够将已产生的所有元素析构掉。

### 3.3 uninitialized_fill_n
{% highlight string %}
template <class _ForwardIter, class _Size, class _Tp>
inline _ForwardIter 
uninitialized_fill_n(_ForwardIter __first, _Size __n, const _Tp& __x);
{% endhighlight %}

uninitialized_fill_n()能够使我们将内存配置与对象行为分离开来，它会为指定范围内的所有元素设定相同的值。

如果[first, first+n)范围内的每一个迭代器都指向未初始化的内存，那么uninitialized_fill_n()会调用copy constructor，在该范围内产生x(上式第三参数）的复制品。也就是说，面对[first, first+n)范围内的每个迭代器i，uninitialized_fill_n()会调用```construct(&*i, x)```，在对应位置处产生x的复制品。式中construct()已于2.2.3节讨论过。

uninitialized_fill_n()也具有“commit or rollback”语义：要么产生出所有必要元素，否则不产生任何元素。如果有任何一个copy constructor丢出异常(exception)，uninitialized_fill_n()必须析构已产生的所有元素。


以下分别介绍这三哥函数的实现法。其中所呈现的iterators（迭代器）、value_type()、__type_traits、__true_type、__false_type、is_POD_type等实现技术，都将于第3章介绍。

1) uninitialized_fill_n

首先是uninitialized_fill_n()的源代码，本函数接受三个参数：
* 迭代器first指向欲初始化空间的起始处
* n表示欲初始化空间的大小
* x表示初值
{% highlight string %}
template <class _ForwardIter, class _Size, class _Tp>
inline _ForwardIter 
uninitialized_fill_n(_ForwardIter __first, _Size __n, const _Tp& __x)
{
  return __uninitialized_fill_n(__first, __n, __x, __VALUE_TYPE(__first));
}
{% endhighlight %}
>注：上面利用__VALUE_TYPE()取出first的value type

这个函数的进行逻辑是，首先萃取出迭代器first的value type（详见第3章），然后判断该型别是否为POD型别：
{% highlight string %}
template <class _ForwardIter, class _Size, class _Tp, class _Tp1>
inline _ForwardIter 
__uninitialized_fill_n(_ForwardIter __first, _Size __n, const _Tp& __x, _Tp1*)
{
  typedef typename __type_traits<_Tp1>::is_POD_type _Is_POD;
  return __uninitialized_fill_n_aux(__first, __n, __x, _Is_POD());
}
{% endhighlight %}
>注：关于__type_traits的用法，详见3.7节

**POD**意指**Plain Old Data**，也就是标量型别(scalar types)或传统的C struct型别。POD型别必然拥有trival ctor/dtor/copy/assignment，因此，我们可以对POD型别采用最有效率的初值填写手法，而对non-POD型别采用最保险安全的做法：
{% highlight string %}
//如果copy construction等同于assignment，而且destructor是trival，以下就有效。
//如果是POD型别，执行流程就会转进到以下函数。这是藉由function template的参数推导机制而得
template <class _ForwardIter, class _Size, class _Tp>
inline _ForwardIter
__uninitialized_fill_n_aux(_ForwardIter __first, _Size __n,
                           const _Tp& __x, __true_type)
{
  return fill_n(__first, __n, __x);
}

//如果不是POD型别，执行流程就会转进到以下函数。这是藉由function template的参数推导机制而得
template <class _ForwardIter, class _Size, class _Tp>
_ForwardIter
__uninitialized_fill_n_aux(_ForwardIter __first, _Size __n,
                           const _Tp& __x, __false_type)
{
  _ForwardIter __cur = __first;
  __STL_TRY {
    for ( ; __n > 0; --__n, ++__cur)
      _Construct(&*__cur, __x);
    return __cur;
  }
  __STL_UNWIND(_Destroy(__first, __cur));
}
{% endhighlight %}

2) uninitialized_copy

下面列出uninitialized_copy()的源代码，本函数接受三个参数：
* 迭代器first指向输入端的起始位置
* 迭代器last指向输入端的结束位置（前闭后开区间）
* 迭代器result指向输出端(欲初始化空间）的起始处

{% highlight string %}
template <class _InputIter, class _ForwardIter>
inline _ForwardIter
  uninitialized_copy(_InputIter __first, _InputIter __last,
                     _ForwardIter __result)
{
  return __uninitialized_copy(__first, __last, __result,
                              __VALUE_TYPE(__result));
}
{% endhighlight %}
这个函数的进行逻辑是，首先萃取出迭代器result的value type(详见第3章），然后判断型别是否为POD型别：
{% highlight string %}
template <class _InputIter, class _ForwardIter, class _Tp>
inline _ForwardIter
__uninitialized_copy(_InputIter __first, _InputIter __last,
                     _ForwardIter __result, _Tp*)
{
  typedef typename __type_traits<_Tp>::is_POD_type _Is_POD;
  return __uninitialized_copy_aux(__first, __last, __result, _Is_POD());
}
{% endhighlight %}
**POD**意指**Plain Old Data**，也就是标量型别(scalar types)或传统的C struct型别。POD型别必然拥有trival ctor/dtor/copy/assignment，因此，我们可以对POD型别采用最有效率的复制手法，而对non-POD型别采用最保险安全的做法：
{% highlight string %}
//如果copy construction等同于assignment，而且destructor是trival，以下就有效。
//如果是POD型别，执行流程就会转进到以下函数。这是藉由function template的参数推导机制而得
template <class _InputIter, class _ForwardIter>
inline _ForwardIter 
__uninitialized_copy_aux(_InputIter __first, _InputIter __last,
                         _ForwardIter __result,
                         __true_type)
{
  return copy(__first, __last, __result);
}


//如果不是POD型别，执行流程就会转进到以下函数。这是藉由function template的参数推导机制而得
template <class _InputIter, class _ForwardIter>
_ForwardIter 
__uninitialized_copy_aux(_InputIter __first, _InputIter __last,
                         _ForwardIter __result,
                         __false_type)
{
  _ForwardIter __cur = __result;
  __STL_TRY {
    for ( ; __first != __last; ++__first, ++__cur)
      _Construct(&*__cur, *__first);
    return __cur;
  }
  __STL_UNWIND(_Destroy(__result, __cur));
}
{% endhighlight %}

针对```char *```和```wchar_t *```两种型别，可以采用最具效率的做法memmove(直接移动内存内容)来复制行为。因此SGI得以为这两种型别设计一份特化版本。
{% highlight string %}
//以下是针对const char *的特化版本
inline char* uninitialized_copy(const char* __first, const char* __last,
                                char* __result) {
  memmove(__result, __first, __last - __first);
  return __result + (__last - __first);
}

//以下是针对const wchar_t *的特化版本
inline wchar_t* 
uninitialized_copy(const wchar_t* __first, const wchar_t* __last,
                   wchar_t* __result)
{
  memmove(__result, __first, sizeof(wchar_t) * (__last - __first));
  return __result + (__last - __first);
}
{% endhighlight %}

3) uninitialized_fill

下面列出uninitialized_fill的源代码，本函数接受三个参数：
* 迭代器first指向输出端（欲初始化空间）的起始处
* 迭代器first指向输出端（欲初始化空间）的结束处（前闭后开区间）
* x表示初值

{% highlight string %}
template <class _ForwardIter, class _Tp>
inline void uninitialized_fill(_ForwardIter __first,
                               _ForwardIter __last, 
                               const _Tp& __x)
{
  __uninitialized_fill(__first, __last, __x, __VALUE_TYPE(__first));
}
{% endhighlight %}
这个函数的进行逻辑是，首先萃取出迭代器result的value type(详见第3章），然后判断型别是否为POD型别：
{% highlight string %}
template <class _ForwardIter, class _Tp, class _Tp1>
inline void __uninitialized_fill(_ForwardIter __first, 
                                 _ForwardIter __last, const _Tp& __x, _Tp1*)
{
  typedef typename __type_traits<_Tp1>::is_POD_type _Is_POD;
  __uninitialized_fill_aux(__first, __last, __x, _Is_POD());
                   
}
{% endhighlight %}
**POD**意指**Plain Old Data**，也就是标量型别(scalar types)或传统的C struct型别。POD型别必然拥有trival ctor/dtor/copy/assignment，因此，我们可以对POD型别采用最有效率的初值填写手法，而对non-POD型别采用最保险安全的做法：
{% highlight string %}
//如果copy construction等同于assignment，而且destructor是trival，以下就有效。
//如果是POD型别，执行流程就会转进到以下函数。这是藉由function template的参数推导机制而得
template <class _ForwardIter, class _Tp>
inline void
__uninitialized_fill_aux(_ForwardIter __first, _ForwardIter __last, 
                         const _Tp& __x, __true_type)
{
  fill(__first, __last, __x);
}


//如果不是POD型别，执行流程就会转进到以下函数。这是藉由function template的参数推导机制而得
template <class _ForwardIter, class _Tp>
void
__uninitialized_fill_aux(_ForwardIter __first, _ForwardIter __last, 
                         const _Tp& __x, __false_type)
{
  _ForwardIter __cur = __first;
  __STL_TRY {
    for ( ; __cur != __last; ++__cur)
      _Construct(&*__cur, __x);
  }
  __STL_UNWIND(_Destroy(__first, __cur));
}
{% endhighlight %}

图2-8以图形显式本节三个函数对效率的特殊考虑。

![cpp-stl](https://ivanzz1001.github.io/records/assets/img/cplusplus/stl/stl_part2_8.jpg)



<br />
<br />

**[参看]:**



<br />
<br />
<br />





