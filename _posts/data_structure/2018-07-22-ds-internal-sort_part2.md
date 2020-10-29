---
layout: post
title: 内部排序（中）
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
int Partion(SqList &L, int low, int high)
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
int Partion(SqList &L, int low, int high)
{
	L.r[0] = L.r[low];
	pivotkey = L.r[low].key;

	while(low < high)
	{
		while(low < high && L.r[high].key >= pivotkey) high--;
		L.r[low] = L.r[high];

		while(low < high && L.r[low].key <= pivotkey) low++;
		L.r[high] = L.r[low];
	}

	L.r[low] = L.r[0];
	return low;
}
{% endhighlight %}

**1) 递归形式快速排序**

递归形式快速排序算法如下所示：
{% highlight string %}
void QSort(SqList &L,int low, high)
{
	if(low < high)
	{
		pivotloc = Partion(L, low, high);
		
		QSort(L, low, pivotloc-1);
		QSort(L, pivotloc+1, high);
	}
}

void QuickSort(SqList &L)
{
	QSort(L, 1, L.length);
}
{% endhighlight %}

注意上面递归形式的排序算法，需要注意防止栈溢出。一般需要优先对较小的划分进行排序。

**2) 迭代形式快速排序**
{% highlight string %}
void QSort(SqList &L, int low, high)
{
	stack_init(&stack);

	while(low < high || !stack_empty(stack))
	{

		if(low < high)
		{
			pivotloc = Partion(L, low, high);
			if(high - pivotloc >= pivotloc - low)
			{
				push_back(stack, pivotloc + 1);
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

void QuickSort(SqList &L)
{
	QSort(L, 1, L.length);
}
{% endhighlight %}

**3） 迭代形式的另一种实现**
{% highlight string %}
void QSort(SqList &L, int low, int high){

	//计算机中整数的最大值用64bit来表示
	int pl[64], ph[64], psize;
	
	psize = 0;
	
	while(low < high || psize){
		
		if low < high{
			pivotloc = Partion(L, low, high);
			if (high - pivotloc >= pivotloc - low){
				pl[psize] = pivotloc + 1;
				ph[psize] = high;
				psize++;
			
				high = pivotloc - 1;
			}else{
				pl[psize] = low;
				ph[psize] = pivotloc -1;
				psize++;
				
				low = pivotloc + 1;
			}
		}else{
			low = pl[psize];
			high = ph[psize];
			psize--;
		}
	}
}

void QuickSort(SqList &L)
{
	QSort(L, 1, L.length);
}
{% endhighlight %}

**4) 快速排序时间复杂度**

快速排序的平均时间为```Tavg(n) = knlnn```, 其中n为待排序序列中记录的个数， k为某个常数，经验证明，在所有同数量级的此类（先进的）排序方法中，快速排序的常数因子k最小。因此，就平均时间而言，快速排序是目前被认为是最好的一种内部排序方法。但是，若初始记录序列按关键字有序或基本有序时，快速排序将蜕化为起泡排序，其时间复杂度为```O(n^2)```。为改进之，通常依```三者取中```的法则来选取枢轴记录，即比较L.r[s].key、L.r[t].key和L.r[(s+t)/2].key，取三者中其关键字取中值的记录为枢轴，只要将该记录和L.r[s]互换，算法可保持不变。经验证明，采用三者取中的规则可大大改善快速排序在最坏情况下的性能。然而，即使如此，也不能使快速排序在待排记录序列已按关键字有序的情况下达到```O(n）```的时间复杂度。为此，可如下所述修改```一次划分```算法： 在指针high减1和low增1的同时进行```起泡```操作，即在相邻的两个记录处于```逆序```时进行互换，同时在算法中附设两个布尔型变量分别指示low和high在从两端向中间的移动过程中是否进行交换过记录的操作，若指针low在从低端向中间的移动过程中没有进行交换记录的操作，则不再需要对低端子表进行排序； 类似地，若指针high在从高端向中间的移动过程中没有进行交换记录的操作，则不需要对高端子表进行排序。显然，如此```划分```将进一步改善快速排序的平均性能。

如下我们给出相应的示例代码：
{% highlight string %}
int Partion(SqList &L, int low, int high, int *lswapped, int *hswapped)
{
	L.r[0] = L.r[low];
	pivotkey = L.r[low].key;
	
	*lswapped = *hswapped = 0;

	while(low < high)
	{
		while(low < high && L.r[high].key >= pivotkey)
		{
			if (L.r[high].key < L.r[high-1]).key
			{
				L.r[high] <-> L.r[high-1];
				*hswapped = 1;
			}
			high--;
		}	
		L.r[low] = L.r[high];

		
		while(low < high && L.r[low] <= pivotkey)
		{
			
			if (L.r[low].key > L.r[low+1].key){
				L.[low] <-> L.[row+1];
				*lswapped = 1;
			}
			low++;
		}
		
		L.r[high] = L.r[low];
	}

	L.r[low] = L.r[0];
	return low;
}

void QSort(SqList &L,int low, high)
{
	if(low < high)
	{
		int lswapped, hswapped;
		
		pivotloc = Partion(L, low, high, &lswapped, &hswapped);
		
		if (lswapped == 1)
			QSort(L, low, pivotkey-1);
			
		if (hswapped == 1)
			QSort(L, pivotloc+1, high);
		
	}
}

void QuickSort(SqList &L)
{
	QSort(L, 1, L.length);
}

{% endhighlight %}

**5） 快速排序空间复杂度**

由上面的讨论可知，从时间上看，快速排序的平均性能优于前面讨论的各种排序方法， 从空间上看，插入排序（除2-路插入排序外)需要一个记录的附加空间即可，但快速排序需要一个栈空间来实现递归。若每一趟排序都将记录序列均匀的分割成长度相接近的两个子序列，则栈的最大深度为```⌊log2^n⌋+1```（包括最外层参量进栈），但是，若每趟排序之后枢轴位置均偏向子序列的一端，则为最坏情况，栈的最大深度为n。我们可以适当改进算法，在一趟排序之后比较分割所得两部分长度，先对长度短的子序列中的记录进行快速排序，则栈的最大深度可降为```O(logn)```。


## 2. 选择排序
选择排序(Selection Sort)的基本思想是： 每一趟在```n-i+1```(i=1,2,...,n-1)个记录中选取关键字最小的记录作为有序序列中第```i```个记录。其中最简单且为读者最熟悉的是```简单选择排序```(Simple Selection Sort)。

### 2.1 简单选择排序
一趟简单选择排序的操作为： 通过```n-i```次关键字间的比较，从```n-i+1```个记录中选出关键字最小的记录，并和第```i```(1<=i<=n)个记录交换之。

显然，对L.r[1..n]中记录进行简单选择排序的算法为： 令i从1至n-1， 进行n-1趟选择操作，算法如下所示。容易看出简单选择排序的过程中，所需进行记录移动的操作次数较少，其最小值为0， 最大值为3(n-1)。然而， 无论记录的初始排序如何，所需进行的关键字间的比较次数相同，均为```n(n-1)/2```。因此，总的时间复杂度为也是```O(n^2)```。

{% highlight string %}
void SelectSort(SqList &L)
{
	for(i = 1;i<L.length;i++)
	{
		j = SelectMin(L, i);
		if(i != j)
			L.r[i]<->L.r[j];
	}
}
{% endhighlight %}

那么，是否可以改进呢？ 从上述可见，选择排序的主要操作是进行关键字间的比较，因此改进简单选择排序应从如何减少```比较```出发考虑。显然，在n个关键字中选出最小值，至少进行```n-1```次比较，然而，继续在剩余的n-1个关键字中选择次小值就并非一定要进行n-2次比较，若能利用前n-1次比较所得信息，则可以减少以后各趟选择排序中所用的比较次数。实际上，体育比赛中锦标赛便是一种选择排序。例如，在8个运动员中决出前3名至多需要进行11场比赛，而不是7+6+5=18场比赛（它的前提是： 若乙胜丙，甲胜乙，则认为甲必胜丙）。如下图所示：

![ds-championship](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_championship.jpg)

图中最低层的叶子节点中8个选手之间经过第一轮的4场比赛之后选拔出4个优胜者“CHA”， “BAO”， “DIAO”和“WANG”， 然后经过两场半决赛和一场决赛之后，选拔出冠军“BAO”。显然，按照锦标赛的传递顺序，亚军只能产生于分别在决赛，半决赛和第一轮比赛中输给冠军的选手。由此，在经过“CHA”和“LIU”，“CHA”和“DIAO”的两场比赛之后，选拔出亚军“CHA”。同理，选拔出殿军的比赛只要在“ZHAO”，“LIU”， “DIAO” 3个选手之间进行即可。按照这种锦标赛的思想，可导出树形选择排序。

### 2.2 树形选择排序
树形选择排序（Tree Selection Sort)，又称为锦标赛排序(Tournament Sort),是一种按照锦标赛的思想进行选择排序的方法。首先对n个记录的关键字进行两两比较，然后在其中```⌈n/2⌉```个较小者之间进行两两比较，如此重复，直至选出最小关键字的记录为止。这个过程可以用一棵有```n```个叶子节点的完全二叉树表示。如下图中的二叉树表示从8个关键字中选出最小关键字的过程：

![ds-tree-sort](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_treeselect_sort.jpg)

 
8个叶子节点中依次存放排序之前的8个关键字，每个非终端节点中的关键字均等于其左、右孩子节点中较小的关键字，则根节点中的关键字即为叶子节点中的最小关键字。在输出最小关键字之后，根据关系的可传递性，欲选出次小关键字，仅需将叶子节点中的最小关键字```13```改为```最大值```，然后从该叶子节点开始，和其左（或右）兄弟的关键字进行比较，修改从叶子节点到根的路径上各节点的关键字，则根节点的关键字即为次小关键字。同理，可依次选出从小到大的所有关键字（参见图(b)和图(c))。由于含有n个叶子节点的完全二叉树的深度为```⌈log2^n⌉ + 1```，则在树形选择排序中，除了最小关键字以外，每选择一个次小关键字仅需进行```⌈log2^n⌉```次比较，因此它的时间复杂度为```O(nlogn)```。但是这种排序方法尚有辅助存储空间较多，和```最大值```进行多余的比较等缺点。为了弥补，J.willioms在1964年提出了另一种形式的选择排序————堆排序。


### 2.3 堆排序

堆排序(Heap Sort)只需要一个记录大小的辅助空间，每个待排序的记录仅占有一个存储空间。堆的定义如下：n个元素的序列{k1,k2,..., kn}当且仅当满足以下关系时，称之为堆。

![ds-heap-definition](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_heap_definition.jpg)

若将和此序列对应的一维数组（即以一维数组作此序列的存储结构）看成是一棵完全二叉树，则堆的含义表明： 完全二叉树中所有非终端节点的值均不大于（或不小于）其左、右孩子节点的值。由此，若序列{k1, k2, ..., kn}是堆，则堆顶元素（或完全二叉树的根）必为序列中n个元素的最小值（或最大值）。例如，下列两个序列为堆，对应的完全二叉树如下所示：

![ds-heap-example](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_heap_example.jpg)

若在输出堆顶的最小值之后，使得剩余```n-1```个元素的序列又建成一个堆，则得到n个元素的次小值。如此反复执行，便能得到一个有序序列，这个过程称之为```堆排序```。由此，实现堆排序需要解决两个问题：

* 如何由一个无序序列建成一个堆？

* 如何在输出堆顶元素之后，调整剩余元素成为一个新的堆？

**1) 堆顶元素的调整**

下面我们先讨论第二个问题。例如，下图(a)是个堆，假设输出堆顶元素之后，以堆中最后一个元素替代之，如图(b)所示。此时根节点的左右子树均为堆，则仅需自上至下进行调整即可。首先以堆顶元素和其左右子树根节点的值比较之，由于右子树根节点的值小于左子树根节点的值且小于根节点的值，则将```27```和```97```交换之； 由于97替代了27之后破坏了右子树的“堆”，则需进行和上述相同的调整，直至叶子节点，调整后的状态如图(c)所示，此时堆顶为```n-1```个元素中的最小值。重复上述过程，将堆顶元素27和堆中最后一个元素97交换且调整，得到如图（d）所示新的堆。

![ds-heap-adjust](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_heap_adjust.jpg)

我们称上述这个自堆顶至叶子的调整过程为```筛选```。


**2） 建堆**

从一个无序序列建堆的过程就是一个反复“筛选”的过程。若将此序列看成是一个完全二叉树，则最后一个非终端节点是第```⌊n/2⌋```个元素，由此```筛选```只需从```⌊n/2⌋```个元素开始。例如下图（a）中的二叉树表示一个有8个元素的无序序列
{% highlight string %}
{49, 38, 65, 97, 76, 13, 27, 49}
{% endhighlight %}
则筛选从第4个元素开始，由于```97>49```，则交换之，交换后的序列如下图（b）所示，同理，在第3个元素65被筛选之后序列的状态如图（c）所示。由于第2个元素38不大于其左、右子树根的值，则筛选后序列不变。图（e）所示为筛选根元素49之后建成的堆。
![ds-heap-build](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_heap_build.jpg)

**3） 堆排序算法**

下面我们会分别给出堆排序算法和筛选算法。为使排序结果按通常的```从小到大```顺序排列： 记录序列按关键字非递减有序排列，则在堆排序的算法中先建一个```大顶堆```，即先选得一个关键字为最大的记录并与序列中最后一个记录交换，然后对序列中前```n-1```个记录进行筛选，重新将它调整为一个```大顶堆```，如此反复直至排序结束。由此，“筛选”应沿关键字较大的孩子节点向下进行。

{% highlight string %}
typedef SqList HeapType;	//堆采用顺序表存储表示

//筛选过程
//已知H->r[s..m]中记录的关键字除H->r[s].key之外均满足堆的定义，本函数调整
//H->r[s]的关键字，使H->r[s..m]成为一个大顶堆（对其中记录的关键字而言）
void HeapAdjust(HeapType *H, int s, int m)
{
	rc = H->r[s];

	for(j=2*s;j<=m;j*=2)
	{
		//j为key较大的记录下标
		if(j<m && LT(H->r[j].key, H->r[j+1].key)	
			++j;

		if(!LT(rc.key, H->r[j].key)		//rc应插入在s位置上
			break;

		H->r[s] = H->r[j];
		s = j;
	}

	H->r[s] = rc;
}

//对顺序表H进行堆排序
void HeapSort(HeapType *H)
{
	//把H.r[1..H->length]建成大顶堆
	for(i = H->length/2; i>0;--i)
		HeapAdjust(H, i, H->length);

	for(i = H->length; i>1;--i)
	{
		//将堆顶记录和当前未经排序的子列H->r[1..i]中最后一个记录相互交换
		H->r[1] <->H->r[i];

		HeapAdjust(H,1,i-1);
	}	
}
{% endhighlight %}

**4) 堆排序时间复杂度**

堆排序方法对记录较少的文件并不值得提倡，但对n较大的文件还是很有效的。因为其运行时间主要耗费在建初始堆和调整建新堆时进行的反复```筛选```上。对深度为```k```的堆，筛选算法中进行的关键字比较次数至多为```2(k-1)```次，则在建含n个元素、深度为h的堆时，总共进行的关键字比较次数不超过4n。又因为，n个节点的完全二叉树的深度为```⌊log2^n⌋+1```，则调整建新堆时调用HeapAdjust过程n-1次，总共进行的比较次数不超过```2n⌊log2^n⌋```。由此，堆排序在最坏的情况下，其时间复杂度也为```O(nlogn)```。相对于快速排序来说，这是堆排序最大的优点。此外，堆排序仅需一个记录大小供交换用的辅助存储空间。


<br />
<br />

**[参看]**

1. [leetcode中文网站](https://leetcode-cn.com/problemset/all/)


<br />
<br />


