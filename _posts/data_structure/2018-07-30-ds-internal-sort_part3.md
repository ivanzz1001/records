---
layout: post
title: 内部排序（下）
tags:
- data-structure
categories: data-structure
description: 内部排序
---

本节我们介绍一下```归并排序```和```基数排序```。


<!-- more -->


## 1. 归并排序

归并排序（Merging Sort)是又一类不同的排序方法。```归并```的含义是将两个或两个以上的有序表组合成一个新的有序表。它的实现方法早已为读者所熟悉，无论是顺序存储结构还是链表存储结构，都可以在```O(m+n)```的时间量级上实现。利用归并的思想容易实现排序。假设初始序列含有```n```个记录，则可看成是```n```个有序的子序列，每个子序列的长度为1，然后两两归并，得到```⌈n/2⌉```个长度为2或1的有序子序列；再两两归并，.....，如此重复，直至得到一个长度为```n```的有序序列为止，这种排序方法称为```2-路```归并排序。如下图所示为```2-路```归并排序的一个例子：

![ds-merge-sort](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_merge_sort.jpg)

```2-路```归并排序的核心操作是将一维数组中前后相邻的两个有序序列归并为一个有序序列，其算法如下所示：
{% highlight string %}
//将有序的SR[i..m]和SR[m+1..n]归并为有序的TR[i..n]
void Merge(RcdType *SR, RcdType *TR, int i,int m, int n)
{
	for(j=m+1, k=i; i<=m && j<=n; ++k)
	{
		if(LQ(SR[i].key, SR[j].key))
			TR[k] = SR[i++];
		else
			TR[k] = SR[j++];
	}
	
	//将剩余的SR[i..m]复制到TR
	if(i<=m)
		TR[k..n] = SR[i..m];

	//将剩余的SR[j..n]复制到TR
	if(j<=n)
		TR[k..n] = SR[j..n]
}
{% endhighlight %}

一趟归并排序的操作是，调用```⌈n/(2h)⌉```次算法merge将SR[1..n]中前后相邻且长度为h的有序段进行两两归并，得到前后相邻、长度为2h的有序段，并存放在TR[1..n]中，整个归并排序需进行```⌈log2^n⌉```趟。可见，实现归并排序需和待排记录等数量的辅助空间，其时间复杂度为```O(nlogn)```。

递归形式的```2-路```归并排序的算法如下所示：
{% highlight string %}
//将SR[s..t]归并排序为TR1[s..t]
void MSort(RcdType *SR, RcdType *TR1, int s,int t)
{
	if(s == t)
		TR1[s] = SR[s];
	else{
		//将SR[s..t]平分为SR[s..m]和SR[m+1,t]
		m = (s + t)>>1;
		MSort(SR, TR2, s,m);
		MSort(SR, TR2, m+1, t);
		Merge(TR2, TR1,s,m,t);
	}
}

void MergeSort(SqList *L)
{
	MSort(L->r, L->r, 1, L->length);
}
{% endhighlight %}
值得提醒的是，```递归形式```的算法在形式上较简洁，但实用性很差。与```快速排序```相比,归并排序的最大特点是： 它是一种稳定的排序方法。一般情况下，很少利用```2-路```归并排序算法进行内部排序。

下面给出归并排序的```迭代形式```:
{% highlight string %}
void MergeSort(SqList *L)
{
	if (L->length <= 1)
		return;
		
	distance = 1;
	src = L->r;
	dst = TR;
	
	while(distance < L->length)
	{
	
		for(i = 1; i + distance <= L->length; )
		{
			j = i + 2*distance - 1;
			j = (j > L->length) ? L->length : j;
			
			merge(src, dst, i, i + distance -1, j);
			
			i = j + 1;
		}
		
		if (i <= L->length)
			copy(dst, src, i, L->length);
		
		src <-> dst;
		distance *= 2;
	}
	
	if (src != L->r)
		copy(L->r, src, 1, L->length);
	
}
{% endhighlight %}

下面给出一个具体实现：
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>



void merge(int *src, int *dst, int s, int m, int n)
{
	int i,j,k;

	for(i = s, j = m+1, k = s; i <= m && j <= n; k++){
		if(src[i] <= src[j])
			dst[k] = src[i++];
		else
			dst[k] = src[j++];
	
	}

	for(; i <= m; i++, k++)
		dst[k] = src[i];

	for(; j <= n; j++, k++)
		dst[k] = src[j];
}

/*
 * 将src中的数据元素利用aux_space辅助空间合并到src中
 */
void merge_sort(int *src, int *aux_space, int start, int end)
{
	if(start == end)
		aux_space[start] = src[start];
	else{
		int middle = (start + end) >> 1;

		merge_sort(aux_space, src, start, middle);
		merge_sort(aux_space, src, middle + 1, end);

		merge(aux_space, src, start, middle, end);
	}
}

int msort(int *src, int len)
{
	if(!src || len <= 0)
		return -1;
	else if(len == 1)
		return 0;

	int *aux_space = (int *)malloc(sizeof(int) * len);
	if(!aux_space)
		return -2;

	memcpy(aux_space, src, sizeof(int)*len);

	merge_sort(src, aux_space, 0, len-1);
	return 0x0;
}

int main(int argc, char *argv[])
{
	int a[10] = {30, 20, 10, 60, 100, 1100, 5, 90, 300, 20};

	if(msort(a, 10) != 0)
	{
		printf("invalid input\n");
		return -1;
	}

	for(int i = 0; i<10; i++)
		printf("%d  ", a[i]);

	printf("\n");

	return 0x0;
}
{% endhighlight %}


## 2. 基数排序
```基数排序```(Radix Sorting)是和前面所述各类排序方法完全不相同的一种排序方法。从前面的讨论可见，实现排序主要是通过关键字间的比较和移动记录这两种操作，而实现基数排序不需要进行记录关键字间的比较。基数排序是一种借助多关键字排序的思想对单逻辑关键字进行排序的方法。


### 2.1 多关键字的排序

什么是多关键字排序问题？ 先看一个具体的例子。

已知扑克牌中52张牌面的次序关系为：

![ds-multikey-sort](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_multikey_sort.jpg)
每一张牌有两个```关键字```: **花色**( 梅花 < 方块 < 红桃 < 黑桃) 和 **面值**(2<3<...<A)，且```花色```的地位高于```面值```，在比较任意两张牌面的大小时，必须先比较```花色```，若花色相同，则再比较面值。由此，将扑克牌整理成如上所述次序关系时，通常采用的办法是： 先按不同花色分成有次序的4堆，每一堆的牌均具有相同的```花色```, 然后分别对每一堆按```面值```大小整理有序。

也可采用另一种办法： 先按不同```面值```分成13堆，然后将这13堆牌自小至大叠在一起（```3```在```2```之上，```4```在```3```之上，...，最上面的4张```A```)，然后将这副牌整个颠倒过来再重新按不同```花色```分成4堆，最后将这4堆牌按自小至大的次序合在一起（```梅花```在最小面，```黑桃```在最上面）， 此时同样得到一副满足如上次序关系的牌。这两种整理扑克牌的方法便是两种多关键字的排序方法。

一般情况下，假设有```n```个记录的序列：
<pre>
{R1,R2, ..., Rn}       (10-10)
</pre>

![ds-multikey-sort2](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_multikey_sort2.jpg)

```MSD```和```LSD```只约定按什么样的```关键字次序```来进行排序，而未规定对每个关键字进行排序时所用的方法。但从上面所述可以看出这两种排序方法的不同特点： 若按MSD进行排序，必须将序列逐层分割成若干子序列，然后对各子序列分别进行排序；而按LSD进行排序时，不必分成子序列，对每个关键字都是整个序列参加排序，但对```K_i```(0<=i<=d-2)进行排序时，只能用稳定的排序方法。另一方面，按LSD进行排序时，在一定条件下（即对前一个关键字K_i(0<=i<=d-2)的不同值，后一个关键字```K_i+1```均取相同值），也可以不利用前面章节介绍的各种通过关键字间的比较来实现排序的方法，而是通过若干次```分配```和```收集```来实现排序，如上述第二种整理扑克牌的方法那样。



### 2.2 链式基数排序
基数排序是借助```分配```和```收集```两种操作对单逻辑关键字进行排序的一种**内部**排序方法。

有的逻辑关键字可以看成由若干个关键字复合而成的。例如，若关键字是数值，若其值都在```0<=K<=999```范围内，则可把每一个十进制数看成一个关键字，即可认为K由3个关键字(K_0, K_1,K_2)组成，其中```K_0```是百位数， ```K_1```是十位数， ```K_2```是个位数； 又若关键字是由5个字母组成的单词，则可看成是由5个关键字(K_0, K_1, K_2, K_3, K_4)组成。由于如此分解而得到的每个关键字```K_j```都在相同的范围内(对于数字： 0<=K_j<=9； 对于字母： 'A'<=K_j<='Z')，则按LSD进行排序更为方便，只要从最低数位关键字起，按关键字的不同值将序列中记录```分配```到```RADIX```个队列中后再```收集```之， 如此重复d次。按这种方法实现排序称之为```基数排序```，其中```基```指的是RADIX的取值范围，在上述两种关键字的情况下，它们分别是10和26。


实际上，早在计算机出现之前，利用卡片分类机对穿孔卡上的记录进行排序就是用的这种方法。然而，在计算机出现之后却长期得不到应用，原因是所需的辅助存储量(RADIX * N个记录空间）太大。直到1954年有人提出用```计数```代替```分配```才使基数排序得以在计算机上实现，但此时仍需要n个记录和2*RADIX个计数单元的辅助空间。此后，有人提出用链表作存储结构，则又省去了n个记录的辅助空间。下面我们就来介绍这种```链式基数排序```方法。

先看一个具体的例子。首先以静态链表存储```n```个待排记录，并令表头指针指向第一个记录，如下图(a)所示；第一趟分配对最低数位关键字（个位数）进行，改变记录的指针值将链表中的记录分配至10个链队列中去，每个队列中的记录关键字的个位数相等，如下图(b)所示，其中```f[i]```和```e[i]```分别为第i个队列的头指针和尾指针；第一趟收集是改变所有非空队列的队尾记录的指针域，令其指向下一个非空队列的对头记录，重新将10个队列中的记录链成一个链表，如下图(c)所示：


![ds-radix-sort](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_radix_sort.jpg)


第二趟分配，第二趟收集及第三趟分配和第三趟收集分别是对十位数和百位数进行的，其过程和个位数相同，如下图(d)~(g)所示。至此，排序完毕。

![ds-radix-sort2](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_radix_sort2.jpg)

**1) 链式基数排序数据结构**

在描述算法之前，我们定义如下新的数据类型：
{% highlight string %}
#define MAX_NUM_OF_KEY	8			//关键字项数的最大值
#define RADIX			10			//关键字基数，此时是十进制整数的基数
#define MAX_SPACE		10000

//静态链表的节点类型
typedef struct{
	KeysType keys[MAX_NUM_OF_KEY];  //关键字
	InfoType otheritems;			//其他数据项
	int next;
}SLCell;


//静态链表类型
typedef struct {
	SLCell r[MAX_SPACE];			//静态链表的可利用空间， r[0]为头节点
	int keynum;						//记录的当前关键字个数
	int recnum;						//静态链表的当前长度
}SLList;

typedef int ArrType[RADIX];			//指针数组类型
{% endhighlight %}


**2) 链式基数排序算法**

* 分配算法

如下是链式基数排序中一趟分配的算法：
{% highlight string %}
//静态链表L的r域中记录已按(keys[0],keys[1],...,keys[i-1])有序
//本算法按第i个关键字keys[i]建立RADIX个子表，使同一子表中记录的keys[i]相同
//f[0..RADIX-1]和e[0..RADIX-1]分别指示各子表中第一个和最后一个记录
void Distribute(SLCell *r,int i, ArrType *f, ArrType *e)
{
	//各子表初始化为空表
	for(j = 0;j<RADIX;j++)
		f[j] = 0;

	for(p = r[0].next;p; p = r[p].next)
	{
		//ord将记录中的第i个关键字映射到[0..RADIX-1]
		j = ord(r[p].keys[i]);

		if(!f[j])
			f[j] = p;
		else
			r[e[j]].next = p;

		e[j] = p;
	}
}
{% endhighlight %}

* 收集算法

如下是链式基数排序一趟收集的算法：
{% highlight string %}
//本算法按keys[i]自小至大地将f[0..RADIX-1]所指各子表依次链成一个链表,
//e[0..RADIX-1]为各子表的尾指针
void Collect(SLCell *r, int i, ArrType *f, ArrType *e)
{
	//找到第一个非空子表，succ为求后继函数
	for(j = 0; !f[j]; j =succ(j));

	r[0].next = f[j];
	t = e[j];

	while(j<RADIX)
	{
		for(j = succ(j); j<RADIX-1 && !f[j]); j = succ(j));

		if(f[j])
		{
			r[t].next = f[j];
			t = e[j];
		}
	}

	r[t].next = 0;
	
}
{% endhighlight %}

* 链式基数排序算法

如下是链式基数排序算法的实现：
{% highlight string %}
//L是采用静态链表表示的顺序表
//对L作基数排序，使得L成为按关键字自小到大的有序静态链表，L->r[0]为头节点
void RadixSort(SLList *L)
{
	for(i = 0;i<L->recnum;i++)
		L->r[i].next = i+1;

	L->r[L->recnum].next = 0;

	//按最低位优先次序对各关键字进行分配和收集
	for(i = 0;i<L->keynum;++i)
	{
		Distribute(L->r, i, f,e);
		Collect(L->r, i, f,e);
	}

}
{% endhighlight %}

**3) 链式基数排序时间、空间复杂度**

从上面的算法容易看出，对于```n```个记录（假设每个记录含d个关键字，每个关键字的取值范围为rd个值）进行链式基数排序的时间复杂度为```O(d(n+rd))```,其中每一趟分配的时间复杂度为```O(n)```，每一趟收集的时间复杂度为```O(rd)```，整个排序需进行d趟分配和收集。所需辅助空间为```2rd```个队列指针。当然，由于需用链表作存储结构，则相对于其他以顺序结构存储记录的排序方法而言，还增加了n个指针域的空间。


## 3. 各种内部排序方法的比较讨论
综合前面讨论的各种内部排序方法，大致有如下结果：

|     排序方法       |    平均时间   |   最坏情况    |     辅助存储    |
|:-----------------:|:------------:|:------------:|:---------------|
|    简单排序        |   O(n^2)     |  O(n^2)      |   O(1)         |
|    快速排序        |   O(nlogn)   |  O(n^2)      |   O(logn)      |
|    堆排序          |   O(nlogn)   |  O(nlogn)    |   O(1)         |
|    归并排序        |   O(nlogn)   |  O(nlogn)    |   O(n)         |
|    基数排序        |   O(d(n+rd)) |  O(d(n+rd))  |   O(rd)        |

从上表可以得出如下结论：

* 从平均时间性能而言，快速排序最佳，其所需时间最省，但快速排序在最坏情况下的时间性能不如堆排序和归并排序。而后两者相比较的结果是，在n较大时，归并排序所需时间较堆排序省，但它所需的辅助存储量最多。

* 上表中的```简单排序```包括除```希尔排序```之外的所有插入排序、冒泡排序和简单选择排序，其中以直接插入排序为最简单，当序列中的记录```基本有序```或n值较小时，它是最佳的排序方法，因此常将它和其他的排序方法，诸如快速排序、归并排序等结合在一起使用。

* 基数排序的时间复杂度也可写成```O(d*n)```。因此，它最适用于n值很大而关键字较小的序列。若关键字也很大，而序列中大多数记录的“最高位关键字”均不同，则亦可先按“最高位关键字”不同将序列分割成若干“小”的子序列，然后进行直接插入排序。

* 从方法的稳定性来比较，基数排序是稳定的内排方法，所有时间复杂度为```O(n^2)```的简单排序法也是稳定的。然而，快速排序、堆排序和希尔排序等时间性能较好的排序方法都是不稳定的。一般来说，排序过程中的```比较```是在“相邻的两个记录关键字”间进行的排序方法是稳定的。值得提出的是，稳定性是由方法本身决定的，对不稳定的排序方法而言，不管其描述形式如何，总能举出一个说明不稳定的实例来。反之，对稳定的排序方法而言，总能找到一种不引起不稳定的描述形式。由于大多数情况下排序是按记录的主关键字进行的，则所用的排序方法是否稳定无关紧要。若排序按记录的次关键字进行，则应根据问题所需慎重选择排序方法及其描述算法。

综上所述，在前面所讨论的 所有排序方法中，没有哪一种是绝对最优的。有的适用于```n```较大的情况，有的适用于```n```较小的情况，有的...，因此，在实用时需根据不同情况适当选用，甚至可将多种方法结合起来使用。


在前面讨论的大多数排序算法是在顺序存储结构上实现的，因此在排序过程中需进行大量记录的移动。当记录很大（即每个记录所占空间较多）时，时间耗费很大，此时可采用静态链表作存储结构。如表插入排序、链式基数排序，以修改指针代替移动记录。但是，有的排序方法，如快速排序和堆排序，无法实现表排序。在这种情况下可以进行```地址排序```，即另设一个地址向量指示相应记录；同时在排序过程中不移动记录而移动地址向量中相应分量的内容。例如对下图(a)所示记录序列进行地址排序时，可附设向量```adr(1:8)```。在开始排序之前令```adr[i] := i```，凡在排序过程中需进行```r[i] := r[j]```的操作时，均以```adr[i] := adr[j]```代替，则在排序结束之后，地址向量中的值指示排序后的记录的次序，r[adr[1]]为关键字最小的记录，r[adr[8]]为关键字最大的记录，如下图(b)所示：

![ds-addr-sort](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_addr_sort.jpg)


最后在需要时可根据addr的值重排记录的物理位置，重排算法如下：
{% highlight string %}
//addr给出顺序表L的有序次序，即L->r[addr[i]]是第i小的记录。
//本算法按addr重排L->r，使其有序
void Rearrange(SqList *L, int addr[])
{
	for(i = 1;i<L->length;++i)
	{
		if(addr[i] != i)
		{
			j = i;
			
			//暂存记录L->r[i]
			L->r[0] = L->r[i];

			//调整L->r[addr[j]]的记录到位直到addr[j]=i为止
			while(addr[j] != i)
			{
				k = addr[j];
				L->r[j] = L->r[k];
				addr[j] = j;
				j = k;
			}		

			L->r[j] = L->r[0];
			addr[j] = j;
		}
	}
}
{% endhighlight %}

从上述算法容易看出，除了在每个小循环中要暂存一次记录外，所有记录均一次移动到位。而每个小循环至少移动两个记录，则这样的小循环至多有```⌊n/2⌋```个，所以重排记录的算法中至多移动记录```⌊3n/2⌋```次。


  



<br />
<br />


