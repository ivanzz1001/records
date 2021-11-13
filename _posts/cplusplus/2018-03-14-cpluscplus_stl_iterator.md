---
layout: post
title: STL容器迭代器失效情况分析、总结
tags:
- cplusplus
categories: cplusplus
description: STL容器迭代器失效情况分析
---


本文主要介绍一下STL容器迭代器由于```插入```、```删除```元素可能引起的迭代器失效。


<!-- more -->


## 1. 迭代器失效
当使用一个容器的insert或者erase函数通过迭代器```插入```或```删除```元素**可能**会导致迭代器失效，因此我们为了避免危险，应该获取insert或者erase返回的迭代器，以便用重新获取的新的有效的迭代器进行正确的操作：
{% highlight string %}
iter = vec.insert(iter);

iter = vec.erase(iter);
{% endhighlight %}

迭代器失效类型：

* 由于插入元素，使得容器元素整体```迁移```导致存放原容器元素的空间不再有效，从而使得指向原空间的迭代器失效；

* 由于删除元素，使得某些元素```次序```发生变化导致原本指向某元素的迭代器不再指向期望指向的元素。

我们经常会看到如下代码：
{% highlight string %}
#incude <iostream>
#include <vector>

int main(int argc, char *argv[])
{
	std::vector<int> vec;
	
	vec.push_back(100);
	vec.push_back(200);
	vec.push_back(300);
	vec.push_back(400);
	vec.push_back(500);
	
	for(std::vector<int>::iterator it=vec.begin(); it != vec.end();)
		it = vec.erase(it);
		
	return 0x0;
}
{% endhighlight %}
我们知道，在执行vector删除操作时，删除点之后的迭代器均会失效。因此在上面的循环中，代码正确性的保证在于每一次循环都要重新调用vec.end()函数。具体是不是这样呢？我们通过如下示例来验证：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>


int getmax()
{
   printf("get max function\n");
   return 5;
}

int main(int argc, char *argv[])
{
   int i;

   for(i = 0; i<getmax(); i++)
        printf("%d ", i);

   return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.c
# ./test
get max function
0 get max function
1 get max function
2 get max function
3 get max function
4 get max function
</pre>
我们看到在循环时，每一次都会重新调用```getmax()```函数.

### 1.1 vector迭代器失效

* 当插入(push_back)一个元素后，end操作返回的迭代器肯定失效；

* 当插入(push_back)一个元素后，如果vector的capacity发生了改变，则需要重新加载整个容器，此时first和end操作返回的迭代器都会失效；

* 当进行删除操作(erase,pop_back)后，指向删除点的迭代器全部失效，指向删除点后面的元素的迭代器也将全部失效；

参看如下示例：
{% highlight string %}
#include <iostream>
#include <vector>
using namespace std;


int main(int argc, char *argv[])
{
	vector<int> vec;
	
	vec.push_back(100);
	vec.push_back(300);
	vec.push_back(400);
	vec.push_back(500);
	
	vector<int>::iterator iter;
	
	for(iter = vec.begin(); iter != vec.end(); iter++)
	{
		if(*iter == 300){
			iter = vec.erase(iter);			//此时iter指向400
			cout<<"next iter: " << *iter << endl;
		}
	}
	
	for(iter = vec.begin(); iter != vec.end(); iter++)
		cout<<*iter << "  ";
	
	cout<<endl;	
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o vector_iterator vector_iterator.cpp -lstdc++
# ./vector_iterator 
next iter: 400
100  400  500 
</pre>

### 1.2 list迭代器失效

* 插入操作(insert)和接合操作(splice)不会造成原有的list迭代器失效，这在vector中是不成立的，因为vector的插入操作可能造成记忆体重新配置，导致所有的迭代器全部失效；

* list的删除操作(erase)也只有指向被删元素的那个迭代器失效，其他迭代器不受影响。（list目前只发现这一种失效情况）

### 1.3 deque迭代器失效

* 在deque容器首部或者尾部插入元素，不会使得任何迭代器失效；
<pre>
注： 通过vs2012测试，不管前端插入还是后端插入，都会使迭代器失效
</pre>

* 在deque容器的首部或者尾部删除元素，只会使指向被删元素的迭代器失效；

* 在deque容器的任何其他位置进行```插入```或```删除```操作都将使指向该容器元素的所有迭代器失效；


### 1.4 set和map迭代器失效

与list相同，当对其进行insert或者erase操作时，操作之前的所有迭代器，在操作完成之后都依然有效，但被删除元素的迭代器失效。


## 2. deque容器的内部实现原理
双端队列(deque)是一种支持向两端高效地插入数据、支持随机访问的容器。其内部实现原理如下：双端队列的数据被表示为一个分段数组，容器中的元素分段存放在一个个大小固定的数组中，此外容器还需要维护一个存放这些数组首地址的索引数组。参见下图

![cpp-stl-iterator](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_stl_iterator.jpg)

由于分段数组的大小是固定的，并且它们的首地址被连续存放在索引数组中，因此可以对其进行随机访问，但效率比vector低很多。

* 向两端加入新元素时，如果这一端的分段数组未满，则可以直接加入； 如果这一端的分段数组已满，只需创建新的分段数组，并把该分段数组的地址加入到索引数组中即可。无论哪种情况，都不需要对已有元素进行移动，因此在双端队列的两端加入新的元素都具有较高的效率。

* 当删除双端队列容器两端的元素时，由于不需要发生元素的移动，效率也是非常高的。

* 双端队列中间插入元素时，需要将插入点到某一端之间的所有元素向容器的这一端移动，因此向中间插入元素效率较低，而且往往插入位置越靠近中间，效率越低。删除队列中元素时，情况也类似，由于被删元素到某一端之间的所有元素都要向中间移动，删除的位置越靠近中间，效率越低。

**注意：** 在除了首尾两端的其他地方插入和删除元素，都将会导致指向deque元素的任何pointers、references、iterators失效。不过，deque的内存重分配优于vector，因为其内部结构显示不需要复制所有元素。


 
## 3. C++标准模板库(STL)迭代器的原理及实现
迭代器(iterator)是一种抽象的设计理念，通过迭代器可以在不了解容器内部原理的情况下遍历容器。除此之外，STL中迭代器一个最重要的作用就是作为容器(vector、list等)与STL算法的```粘结剂```，只要容器提供迭代器的接口，同一套算法代码可以利用在完全不同的容器中，这是抽象思想的经典应用。

### 3.1 使用迭代器遍历不同的容器
如下所示的代码演示了迭代器是如何将容器和算法结合在一起的，其中使用了3种不同的容器，```.begin()```和```.end()```方法返回一个指向容器第一个元素和一个指向容器**最后一个元素后面一个位置**的迭代器，也就是说begin()和end()返回的迭代器是一个前闭后开的，一般用**[begin, end)** 表示。对于不同的容器，我们都使用同一个**accumulate()**函数，原因就在于**acccumulate()**函数的实现无需考虑容器的种类，只需要容器传入的begin()和end()迭代器能够完成标准迭代器的要求即可。
{% highlight string %}
std::vector<int> vec{1,2,3};
std::list<int> lst{4,5,6};
std::deque<int> deq{7,8,9};

std::cout<<std::accumulate(vec.begin(), vec.end(), 0) << std::endl;
std::cout<<std::accumulate(lst.begin(), lst.end(), 0) << std::endl;
std::cout<<std::accumulate(deq.begin(), deq.end(), 0) << std::endl;
{% endhighlight %}

### 3.2 迭代器的实现
迭代器的作用就是提供一个遍历容器内部所有元素的接口，因此迭代器的内部必须保存一个与容器相关联的指针，然后重载各种运算操作来方便遍历，其中最重要的就是```* 运算符```和```-> 运算符```，以及```++```、```--```等可能需要的运算符重载。实际上这和C++标准库的智能指针(smart pointer)很像，智能指针也是将一个指针封装，然后通过引用计数或是其他方法完成自动释放内存的功能，为了达到和原有指针一样的功能，也需要对```*```、```->```等运算符进行重载。下面参照智能指针实现一个简单vector迭代器，其中几个**typedef**暂时不用管，我们后面会提到。vecIter主要作用就是包裹一个指针，不同容器内部数据结构不相同，因此迭代器操作符重载的实现也会不同。比如```++```操作符。对于线性分配内存的数组来说，直接对指针执行```++```操作即可； 但是如果容器是List就需要采用元素内部的方法，比如```ptr->next()```之类的方法访问下一个元素。因此，STL容器都实现了自己的专属迭代器。

下面我们给出一个普通数组的迭代器的实现(array_iterator.cpp)：
{% highlight string %}
#include <iostream>
#include <numeric>

template<class Item>
class vecIter{
	Item *ptr;
	
public:
	typedef std::forward_iterator_tag iterator_category;
	typedef Item value_type;
	typedef Item *pointer;
	typedef Item &reference;
	typedef std::ptrdiff_t difference_type;
	
public:
	vecIter(Item *p = 0):ptr(p){}
	
	Item & operator*() const{
		return *ptr;
	}
	
	Item * operator->() const{
		return ptr;
	}
	
	//pre
	vecIter &operator++(){
		++ptr;
		
		return *this;
	}
	
	vecIter operator++(int){
		vecIter tmp = *this;
		++*this;
		
		return tmp;
	}
	
	bool operator==(const vecIter &iter){
		return ptr == iter.ptr;
	}
	
	bool operator!=(const vecIter &iter){
		return !(*this == iter);
	}
};

int main(int argc, char *argv[])
{
	int a[] = {1,2,3,4};
	
	std::cout<<std::accumulate(vecIter<int>(a), vecIter<int>(a+4), 0)<<std::endl;
	
	return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o array_iterator array_iterator.cpp -lstdc++
# ./array_iterator 
10
</pre>

其实对于新版stl，可以直接将一个数组类型通过```iterator_traits<_Tp *>```将其特化为random_access_iterator_tag，这也是为什么新版std::vector实现中并没有看到类似于*typedef std::forward_iterator_tag iterator_category;*的语句的原因：
{% highlight string %}
template <class _Iterator>
struct iterator_traits {
  typedef typename _Iterator::iterator_category iterator_category;
  typedef typename _Iterator::value_type        value_type;
  typedef typename _Iterator::difference_type   difference_type;
  typedef typename _Iterator::pointer           pointer;
  typedef typename _Iterator::reference         reference;
};

template <class _Tp>
struct iterator_traits<_Tp*> {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;
  typedef _Tp*                        pointer;
  typedef _Tp&                        reference;
};

template <class _Tp>
struct iterator_traits<const _Tp*> {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;
  typedef const _Tp*                  pointer;
  typedef const _Tp&                  reference;
};
{% endhighlight %}
下面我们来验证一下普通数组直接调用std::accumulate()计算和：
{% highlight string %}
#include <stdio.h>
#include <numeric>

int main(int argc, char *argv[])
{
        int a[] = {1,2,3,4,5};

        int sum = std::accumulate(a, a+5, 0);

        printf("sum: %d\n", sum);

        return 0x0;
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o test test.cpp -lstdc++
# ./test
sum: 15
</pre>

### 3.3 迭代器的相应型别
我们都知道```type_traits```可以萃取出类型的型别，根据不同的型别可以执行不同的处理流程。那么对于迭代器来说，是否有针对不同特性迭代器的优化方法呢？ 答案是肯定的。拿一个STL算法库中的distance()函数来说，distance函数接受两个迭代器参数，然后计算两者之间的距离。显然，对于不同的迭代器计算效率差别很大。比如对于vector容器来说，由于内存是连续分配的，因此指针直接相减即可获得两者的距离；而list容器是链表结构，内存一般都不是连续分配，因此只能通过一级一级调用next()或者其他函数，每调用一次再判断迭代器是否相等来计算距离。vector迭代器计算distance的效率为**O(1)**，而list则为**O(n)**，n为距离的大小。

因此，根据迭代器不同的特性，将迭代器分为5类：

* Input Iterator: 这种迭代器所指的对象为只读的

* Output Iterator: 所指的对象只能进行写入操作

* Forward Iterator: 该类迭代器可以在一个正确的区间中进行读写操作，它拥有Input Iterator的所有特性，和Output Iterator的部分特性，以及单步```向前```迭代元素的能力

* Bidirectional Iterator: 该类迭代器是在Forward Iterator的基础上提供了单步```向后```迭代元素的能力，从而使得可以双向移动

* Random Access Iterator： 前4种迭代器只提供部分指针算术能力（前3种支持```++```运算符，后一种还支持```--```运算符)，而本迭代器则支持所有指针的算术运算，包括```p+n```、```p-n```、```p[n]```、```p1-p2```、```p1<p2```

上述5种迭代器的继承关系如下图所示：

![cpp-iterator-top](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_iterator_top.jpg)

了解了迭代器的类型，我们就能解释vector的迭代器和list迭代器的区别了。显然，vector迭代器具有所有指针算术运算能力，而list由于是双向链表，因此只有双向读写不能随机访问元素。故vector的迭代器种类为**Random Access Iterator**，而list的迭代器种类为**Bidirectional Iterator**。我们只需要根据不同的迭代器种类，利用```traits```编程技巧萃取出迭代器型别，然后由C++重载机制就能够对不同型别的迭代器采用不同的处理流程了。为此，对于每个迭代器都必须定义型别```iterator_category```，也就是上文代码中的**typedef std:forward_iterator_tag iterator_category**，实际上可以直接继承STL中定义的iterator模板，模板后三个参数都有默认值，因此继承时只需要指定前两个模板参数即可。如下所示，STL定义了5个空类型作为迭代器的标签：
{% highlight string %}
template<class Category,class T,class Distance = ptrdiff_t,class Pointer=T*,class Reference=T&>
class iterator{
    typedef Category iterator_category;
    typedef T        value_type;
    typedef Distance difference_type;
    typedef Pointer  pointer;
    typedef Reference reference;
};

struct input_iterator_tag{};
struct output_iterator_tag{};
struct forward_iterator_tag:public input_iterator_tag{};
struct bidirectional_iterator_tag:public forward_iterator_tag{};
struct random_access_iterator_tag:public bidirectional_iterator_tag{};
{% endhighlight %}


### 3.4 利用迭代器种类更有效的实现distance函数
回到distance函数，有了前面的基础，我们可以根据不同迭代器种类实现distance函数（distance.cpp)：
{% highlight string %}
#include <iostream>
#include <vector>
#include <list>

# if 0
template <class _InputIterator, class _Distance>
inline void __distance(_InputIterator __first, _InputIterator __last,
                       _Distance& __n, std::input_iterator_tag)
{
  while (__first != __last) { ++__first; ++__n; }
}

template <class _RandomAccessIterator, class _Distance>
inline void __distance(_RandomAccessIterator __first, 
                       _RandomAccessIterator __last, 
                       _Distance& __n, std::random_access_iterator_tag)
{
  __n += __last - __first;
}

template <class _InputIterator, class _Distance>
inline void distance(_InputIterator __first, 
                     _InputIterator __last, _Distance& __n)
{
 typedef typename std::iterator_traits<_InputIterator>::iterator_category _Category;
  __distance(__first, __last, __n, _Category());
}
#else
//注： 这里需要放在一个新的namespace中，否则可能会与STL中的相冲突	
namespace DT{
template<class InputIterator>
inline typename std::iterator_traits<InputIterator>::difference_type distance(InputIterator first, InputIterator last){
	typedef typename std::iterator_traits<InputIterator>::iterator_category _Category;
	return __distance(first, last, _Category());
}
template<class InputIterator>
inline typename std::iterator_traits<InputIterator>::difference_type __distance(InputIterator first, InputIterator last, std::input_iterator_tag){
	typename std::iterator_traits<InputIterator>::difference_type n = 0;
	while (first != last){
		++first; ++n;
	}
	return n;
}

template<class InputIterator>
inline typename std::iterator_traits<InputIterator>::difference_type \
__distance(InputIterator first, InputIterator last, std::random_access_iterator_tag){
	return last - first;
}
}	
#endif


int main(int argc, char *argv[])
{

	int a[] = {1,2,3,4};
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(3);
	vec.push_back(4);
	
	std::list<int> lst;
	lst.push_back(1);
	lst.push_back(2);
	lst.push_back(3);
	lst.push_back(4);
	

	# if 0
		int vec_distance = 0, lst_distance = 0, carr_distance = 0;
		distance(vec.begin(), vec.end(), vec_distance);
		distance(lst.begin(), lst.end(), lst_distance);
		distance(a, a + sizeof(a)/sizeof(*a), carr_distance);
		
		std::cout<<"vec distance:"<<vec_distance<<std::endl;
		std::cout<<"lst distance:"<<lst_distance<<std::endl;
		std::cout<<"c-array distance:"<<carr_distance<<std::endl;
	#else
		std::cout<<"vec distance:"<<DT::distance(vec.begin(), vec.end())<<std::endl;
		std::cout<<"lst distance:"<<DT::distance(lst.begin(), lst.end())<<std::endl;
		std::cout<<"c-array distance:"<<DT::distance(a, a + sizeof(a)/sizeof(*a))<<std::endl;
	#endif
	
}
{% endhighlight %}
编译运行：
<pre>
# gcc -o distance distance.cpp -lstdc++
# ./distance 
vec distance:4
lst distance:4
c-array distance:4
</pre>

上面通过STL定义的```iterator_traits```模板可以萃取不同种类的迭代器特性，```iterator_traits```还对指针和常量指针有特化版本，因此也可以萃取原生指针的特性。具体实现如下：
{% highlight string %}
template <class _Tp, class _Distance> struct input_iterator {
  typedef input_iterator_tag iterator_category;
  typedef _Tp                value_type;
  typedef _Distance          difference_type;
  typedef _Tp*               pointer;
  typedef _Tp&               reference;
};

struct output_iterator {
  typedef output_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;
};

template <class _Tp, class _Distance> struct forward_iterator {
  typedef forward_iterator_tag iterator_category;
  typedef _Tp                  value_type;
  typedef _Distance            difference_type;
  typedef _Tp*                 pointer;
  typedef _Tp&                 reference;
};


template <class _Tp, class _Distance> struct bidirectional_iterator {
  typedef bidirectional_iterator_tag iterator_category;
  typedef _Tp                        value_type;
  typedef _Distance                  difference_type;
  typedef _Tp*                       pointer;
  typedef _Tp&                       reference;
};

template <class _Tp, class _Distance> struct random_access_iterator {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                        value_type;
  typedef _Distance                  difference_type;
  typedef _Tp*                       pointer;
  typedef _Tp&                       reference;
};

template <class _Iterator>
struct iterator_traits {
  typedef typename _Iterator::iterator_category iterator_category;
  typedef typename _Iterator::value_type        value_type;
  typedef typename _Iterator::difference_type   difference_type;
  typedef typename _Iterator::pointer           pointer;
  typedef typename _Iterator::reference         reference;
};

template <class _Tp>
struct iterator_traits<_Tp*> {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;
  typedef _Tp*                        pointer;
  typedef _Tp&                        reference;
};

template <class _Tp>
struct iterator_traits<const _Tp*> {
  typedef random_access_iterator_tag iterator_category;
  typedef _Tp                         value_type;
  typedef ptrdiff_t                   difference_type;
  typedef const _Tp*                  pointer;
  typedef const _Tp&                  reference;
};
{% endhighlight %}

### 3.5 小结

STL使用迭代器算法和容器结合，利用迭代器```型别```可以针对不同迭代器编写更加高效的算法，这一点很重要的思想就是： 利用C++重载机制和参数推导机制将```运行期```决议问题提前到```编译期```决议，也就是说，我们不需要在运行时判断迭代器的类型，而是在编译期就已经决定。这很符合C++模板编程的理念。在后续STL学习中，我们会实现自己的各种容器，也必须实现各种各样的迭代器，因此迭代器的学习还远没有停止。



## 4. C++中模板使用时候typename和class的区别
在C++ Template中很多地方都用到了```typename```与```class```这两个关键字，而且好像可以替换，是不是这两个关键字完全一样呢？ 相信学习C++的人对```class```这个关键字都非常明白，class用于定义类，在C++中引入模板后，最初定义模板的方法为：
{% highlight string %}
template<class T>....
{% endhighlight %}

在这里```class```关键字表明```T```是一个类型，后来为了避免class在这两个地方的使用可能给人带来混淆，所以引入```typename```这个关键字，它的作用同class一样表明后面的符号为一个类型，这样在定义模板的时候就可以使用下面的方式了：
{% highlight string %}
template<typename T>...
{% endhighlight %}
在模板定义语法中关键字```class```与```typename```的作用完全一样。

```typename```难道仅仅在模板定义中起作用吗？ 其实不是这样，```typename```另外一个作用为： 使用嵌套依赖类型(nested depended name), 如下所示
{% highlight string %}
class MyArray{
public:
	typedef int LengthType;
	....
};

template<class T>
void MyMethod(T myarr){
	typedef typename T::LengthType LengthType;
	LengthType length = myarr.GetLength();
}
{% endhighlight %}
这个时候```typename```的作用就是告诉C++编译器，typename后面的字符串为一个类型名称，而不是成员函数或者成员变量。这个时候如果前面没有```typename```，编译器没有任何办法知道T::LengthType是一个类型还是一个成员名称(静态数据成员或者静态函数），所以编译不能够通过。



<br />
<br />

**[参看]:**

1. [STL容器迭代器失效情况分析、总结](https://blog.csdn.net/weixin_41413441/article/details/81591656)

2. [STL的erase()陷阱-迭代器失效总结](https://www.cnblogs.com/blueoverflow/p/4923523.html)

3. [C++标准模板库(STL)迭代器的原理与实现](https://blog.csdn.net/wutao1530663/article/details/64922389)

4. [SGI STL](https://github.com/steveLauwh/SGI-STL)

5. [STL源代码下载](http://labmaster.mi.infn.it/Laboratorio2/serale/www.sgi.com/tech/stl/download.html)

6. [土木硕士转行互联网小结](https://blog.csdn.net/wutao1530663/article/details/78022572)

7. [c++中模板使用时候typename和class的区别](https://blog.csdn.net/u011619422/article/details/44218473)

8. [STL源码剖析---移动advance和距离distance](https://blog.csdn.net/qq_41822235/article/details/83076790)

9. [traits - wiki](https://en.wikipedia.org/wiki/Trait_(computer_programming))

10. [活用C++模板之traits](https://m.2cto.com/kf/201208/149791.html)

<br />
<br />
<br />





