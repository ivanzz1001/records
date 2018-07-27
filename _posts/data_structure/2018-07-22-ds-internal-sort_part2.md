---
layout: post
title: 内部排序
tags:
- data-structure
categories: data-structure
description: 内部排序
---

本节我们介绍一下```交换排序```和```选择排序```。


<!-- more -->


## 1. 交换排序

### 1.1 冒泡排序
本章我们讨论一类借助```交换```进行排序的方法，其中最简单的一种就是人们所熟知的```起泡排序```(Bubble Sort)。下面给出冒泡排序的示例：
{% highlight string %}
void BubbleSort(int *a, int length)
{
	for(i = length-1; i > 0;i--)
	{
		for(j = 0;j<i;j++)
		{
			if(a[j] > a[j+1])
			{
				tmp = a[j];
				a[j] = a[j+1];
				a[j+1] = tmp;
			}
		}
	}
}
{% endhighlight %}
分析冒泡排序的效率，容易看出，若初始序列为```正序```序列，则只需进行一趟排序，在排序过程中进行```n-1```次关键字间的比较，且不移动记录； 反之，若初始序列为```逆序```序列，则需进行```n-1```趟排序，需进行```n(n-1)/2```次比较，并作等数量级的记录移动。因此，总的时间复杂度为```O(n^2)```。

### 1.2 快速排序
快速排序(Quick Sort)是对起泡排序的一种改进。它的基本思想是， 通过一趟排序将待排记录分割成独立的两部分，其中一部分记录的关键字均比另一部分记录的关键字小，则可分别对这两部分记录继续进行排序，以达到整个序列有序。

假设待排序的序列为{L.r[s], L.r[s+1], ..., L.r[t]}，首先任意选取一个记录（通常可选第一个记录L.r[s])作为```枢轴```(或支点）(pivot)，然后按下述原则排列其余记录：将所有关键字比它小的记录都安置在它的位置之前，将所有关键字比它大的记录都安置在它的位置之后。由此可以该```枢轴```记录最后所落的位置```i```作为分界线，将序列{L.r[s], L.r[s+1], ..., L.r[t]}分割成两个子序列：{L.r[s], L.r[s+1], ..., L.r[i-1]}和{L.r[i+1], L.r[i+2], ..., L.r[t]}，这个过程我们称作一趟快速排序（或一次划分）。

一趟快速排序的具体做法是： 附设两个指针low和high，它们的初值分别是low和high，设枢轴记录的关键字为pivotkey，则首先从high所指位置向前搜索找到第一个关键字小于pivotkey的记录和枢轴记录互相交换， 然后从low所指位置起向后搜索，找到第一个关键字大于pivotkey的记录和枢轴记录互相交换，重复这两步直至low=high为止。具体算法如下：
{% highlight string %}
//交换顺序表L中子表L.r[low..high]的记录，是枢轴记录到位，并返回其所在的位置，此时
//在它之前的记录不大于它，在它之后的记录不小于它
int Partion(SqList L, int low, int high)
{
	pivotkey = L.r[low].key;

	while(low < high)
	{
		while(low < high && L.r[high].key >= pivotkey)	high--;
		L.r[low] <-> L.r[high];

		while(low < high && L.r[low] <= pivotkey)	low++;
		L.r[low] <-> L.r[high];
	}

	return low;
}
{% endhighlight %}

在具体实现上述算法时，每交换一对记录需进行3次记录移动（赋值）操作。而实际上，在排序过程中对枢轴记录的赋值是多余的，因为只有在一趟排序结束时，即low=high的位置才是枢轴记录的最后位置。由此可以改写上述算法，先将枢轴记录暂存在r[0]的位置上，排序过程只做r[low]和r[high]的单向移动，直至一趟排序结束后再将枢轴记录移至正确位置上：
{% highlight string %}
int Partion(SqList L, int low, int high)
{
	L.r[0] = L.r[low];
	pivotkey = L.r[low].key;

	while(low < high)
	{
		while(low < high && L.r[high] >= pivotkey)	high--;
		L.r[low] = L.r[high];

		while(low < high && L.r[low] <= pivotkey) low++;
		L.r[high] = L.r[low];
	}

	L.r[low] = L.r[0];
	return low;
}
{% endhighlight %}

**1) 递归形式快速排序**

递归形式快速排序算法如下所示：
{% highlight string %}
void QSort(SqList L,int low, high)
{
	while(low < high)
	{
		pivotloc = Partion(L, low, high);
		
		QSort(L, low, pivotloc-1);
		QSort(L, pivotloc+1, high);
	}
}

void QuickSort(SqList)
{
	QSort(sqList, 1, L.length);
}
{% endhighlight %}

**2) 迭代形式快速排序**
{% highlight string %}
void QSort(SqList L, int low, high)
{
	stack_init(&stack);

	while(low < high || !stack_empty(stack))
	{

		if(low < high)
		{
			pivotloc = Partion(L, low, high);
			if(high - pivotloc >= pivotloc - low)
			{
				push_back(stack, pivotloc);
				push_back(stack, high)	

				high = pivotloc-1;			
			}
			else{
				push_back(&stack,low);
				push_back(&stack,pivotloc-1);

				low = pivotloc + 1;
			}
		}
		else{
			high = pop(&stack);
			low = pop(&stack);
		}
	}
	
}

void QuickSort(SqList L)
{
	QSort(L, 1, L.length);
}
{% endhighlight %}

**3) 快速排序时间复杂度**

快速排序的平均时间为```Tavg(n) = knlnn```, 其中n为待排序序列中记录的个数， k为某个常数，经验证明，在所有同数量级的此类（先进的）排序方法中，快速排序的常数因子k最小。因此，就平均时间而言，快速排序是目前被认为是最好的一种内部排序方法。但是，若初始记录序列按关键字有序或基本有序时，快速排序将蜕化为起泡排序，其时间复杂度为```O(n^2)```。为改进之，通常依```三者取中```的法则来选取枢轴记录，即比较L.r[s].key、L.r[t].key和L.r[(s+t)/2].key，取三者中其关键字取中值的记录为枢轴，只要将该记录和L.r[s]互换，算法可保持不变。经验证明，采用三者取中的规则可大大改善快速排序在最坏情况下的性能。然而，即使如此，也不能使快速排序在待排记录序列已按关键字有序的情况下达到```O(n）```的时间复杂度。为此，可如下所述修改```一次划分```算法： 在指针high减1和low增1的同时进行```起泡```操作，即在相邻的两个记录处于```逆序```时进行互换，同时在算法中附设两个布尔型变量分别指示low和high在从两端向中间的移动过程中是否进行交换过记录的操作，若指针low在从低端向中间的移动过程中没有进行交换记录的操作，则不再需要对低端子表进行排序； 类似地，若指针high在从高端向中间的移动过程中没有进行交换记录的操作，则不需要对高端子表进行排序。显然，如此```划分```将进一步改善快速排序的平均性能。

**4） 快速排序空间复杂度**

由上面的讨论可知，从时间上看，快速排序的平均性能优于前面讨论的各种排序方法， 从空间上看，插入排序（除2-路插入排序外)需要一个记录的附加空间即可，但快速排序需要一个栈空间来实现递归。若每一趟排序都将记录序列均匀的分割成长度相接近的两个子序列，则栈的最大深度为```⌊log2^n⌋+1```（包括最外层参量进栈），但是，若每趟排序之后枢轴位置均偏向子序列的一端，则为最坏情况，栈的最大深度为n。我们可以适当改进算法，在一趟排序之后比较分割所得两部分长度，先对长度短的子序列中的记录进行快速排序，则栈的最大深度可降为```O(logn)```。


## 2. 选择排序



<br />
<br />


