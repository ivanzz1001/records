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

**注意：** 在除了首尾两端的其他地方插入和删除元素，都将会导致指向deque元素的任何pointers、references、iterators失效。不过，deque的内存分配由于vector，因为其内部结构显示不需要复制所有元素。


 
## 3. 







<br />
<br />

**[参看]:**

1. [STL容器迭代器失效情况分析、总结](https://blog.csdn.net/weixin_41413441/article/details/81591656)

2. [STL的erase()陷阱-迭代器失效总结](https://www.cnblogs.com/blueoverflow/p/4923523.html)

3. [C++标准模板库(STL)迭代器的原理与实现](https://blog.csdn.net/wutao1530663/article/details/64922389)

4. [SGI STL](https://github.com/steveLauwh/SGI-STL)

5. [STL源代码下载](http://labmaster.mi.infn.it/Laboratorio2/serale/www.sgi.com/tech/stl/download.html)

<br />
<br />
<br />





