---
layout: post
title: 败者树
tags:
- data-structure
categories: data-structure
description: 败者树
---

本节主要介绍一些败者树及其使用场景，然后给出一个败者树的实现。

<!-- more -->

## 1. 败者树介绍
```败者树```是树形选择排序的一种变形， 主要会用于外部多路归并排序。在大部分情况下我们接触到的都是```胜者树```, 即每个非终端节点均表示其左、右孩子节点中的```胜者```。反之， 如果在双亲节点中记下刚进行完的这场比赛中的败者，而让胜者去参加更高一层的比赛，便可得到一棵```败者树```。

如```下图(a)```所示为一棵5-路归并的败者树```ls[0..4]```，图中方形节点表示叶子节点（也可看成是外节点），分别为5个归并段中当前参加归并选择的记录的关键字； 败者树中根节点```ls[1]```的双亲节点ls[0]为“冠军”，在此指示各归并段中的最小关键字记录为第三段中的当前记录； 节点ls[3]指示 b1 和 b2 两个叶子节点中的败者即b2，而胜者b1和b3(b3是叶子节点b3、b4和b0经过两场比赛后选出来的获胜者）进行比较， 节点ls[1]则指示它们中的败者为b1。在选得最小关键字的记录之后，只要修改叶子节点b3中的值，使其为同一归并段中的下一个记录的关键字，然后从该节点向上和双亲节点所指的关键字进行比较，败者留在该双亲节点，胜者继续向上直至树根的双亲。

![ds-loser-tree](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_loser_tree.jpg)


```上图(b)```所示，当第3个归并段中第2个记录参加归并时，选得的最小关键字记录为第一个归并段中的记录。为了防止在归并过程中某个归并段变空，可以在每个归并段中附加一个关键字为最大值的记录。当选出“冠军”记录的关键字为最大值时，表明此次归并已经完成。败者树的初始化也容易实现，只要先令所有的非终端节点指向一个含最小关键字的叶子节点，然后从各个叶子节点出发调整非终端节点为新的败者即可。


## 2. 败者树实现

### 2.1 败者树头文件

头文件```loser_tree.h```:
{% highlight string %}
#ifndef __LOSER_TREE_H_
#define __LOSER_TREE_H_


typedef int ElemType;

struct Elements{
	ElemType *elems;
	int count;
};


struct LoserTree{
	int k;
	int *losers;

	struct Elements *base;

	ElemType ** current_merges;
	int *offsets;

};


#define DEFINITELY_MINIMUM		(ElemType *)0

#define DEFINITELY_MAXIMUM		(ElemType *)-1



struct LoserTree *create_losertree(struct Elements *merge_ways, int waycnt);

void print_losertree(struct LoserTree *loserTree);

void multiways_merge(struct LoserTree *loserTree);


#endif
{% endhighlight %}

### 2.2 败者树源文件

源文件```loser_tree.c```:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loser_tree.h"


static ElemType *read_element(struct Elements *vector, int offset)
{
	if(offset >= vector->count)
		return NULL;

	return vector->elems + offset;
}

static int elem_compare(ElemType *a, ElemType *b)
{
	if(a == DEFINITELY_MAXIMUM || b == DEFINITELY_MINIMUM)
		return 1;
	else if(a == DEFINITELY_MINIMUM || b == DEFINITELY_MAXIMUM)
		return -1;
	else
		return (*a - *b);
}
static void adjust(struct LoserTree *loserTree, int s)
{
	int t;
	int tmp;

	//adjust the loser-tree from the leafs to the root
	t = (s + loserTree->k) >> 1;			//ls[t] is the parent of loserTree->currents[s]

	while(t > 0)
	{
		if(elem_compare(loserTree->current_merges[s], loserTree->current_merges[loserTree->losers[t]]) > 0)
		{
			tmp = loserTree->losers[t];
			loserTree->losers[t] = s;
			s = tmp;
		}

		t = t>>1;
	}

	loserTree->losers[0] = s;
}


struct LoserTree *create_losertree(struct Elements *merge_ways, int waycnt)
{
	struct LoserTree *loserTree = NULL;
	int i;

	if((loserTree = (struct LoserTree *)malloc(sizeof(struct LoserTree))) == NULL)
		return NULL;


	memset(loserTree, 0x0, sizeof(struct LoserTree));
	loserTree->k = waycnt;
	loserTree->base = merge_ways;

	loserTree->losers = (int *)malloc(sizeof(int)*waycnt);
	loserTree->current_merges = (ElemType **)malloc(sizeof(ElemType *) * (waycnt + 1));
	loserTree->offsets = (int *)malloc(sizeof(int)*waycnt);

	if(!loserTree->losers || !loserTree->current_merges || !loserTree->offsets)
		goto FAILURE;

	loserTree->current_merges[waycnt] = DEFINITELY_MINIMUM;
	for(i = 0;i<waycnt; i++)
	{
		loserTree->losers[i] = waycnt;
		loserTree->offsets[i] = 0;
	}

	for(i = 0;i<waycnt;i++)
	{
		ElemType *elem = read_element(&loserTree->base[i], loserTree->offsets[i]);
		if(!elem)
		{
			loserTree->current_merges[i] = DEFINITELY_MAXIMUM;
		}
		else{
			loserTree->current_merges[i] = elem;
			loserTree->offsets[i]++;
		}
	}

	for(i = waycnt-1;i>=0;i--)
	{
		adjust(loserTree, i);
	}

	return loserTree;


FAILURE:
	free(loserTree->losers);
	free(loserTree->current_merges);
	free(loserTree->offsets);
	free(loserTree);

	return NULL;
}


void print_losertree(struct LoserTree *loserTree)
{
	int i;

	printf("%d-ways loser tree:\n", loserTree->k);

	for(i = loserTree->k-1;i>=0;i--)
	{
		ElemType *elem = loserTree->current_merges[loserTree->losers[i]];
		if(elem == DEFINITELY_MINIMUM)
		{
			printf("losers[%d]:%d  --> %s\n", i, loserTree->losers[i], "definitely-min");
		}
		else if(elem == DEFINITELY_MAXIMUM)
		{
			printf("losers[%d]:%d  --> %s\n", i, loserTree->losers[i], "definitely-max");
		}
		else{
			printf("losers[%d]:%d  --> %d\n", i, loserTree->losers[i], *elem);
		}

	}
}


void multiways_merge(struct LoserTree *loserTree)
{
	ElemType *elem;
	int q;

	while(loserTree->current_merges[loserTree->losers[0]] != DEFINITELY_MAXIMUM)
	{
		q = loserTree->losers[0];
		elem = loserTree->current_merges[q];

		if(elem == DEFINITELY_MINIMUM)
		{
			printf("definitely-min ");
		}
		else if(elem == DEFINITELY_MAXIMUM)
		{
			printf("definitely-max ");
		}
		else{
			printf("%d ", *elem);
		}

		//Input next merge data
		elem = read_element(&loserTree->base[q], loserTree->offsets[q]);
		if(!elem)
		{
			loserTree->current_merges[q] = DEFINITELY_MAXIMUM;
		}
		else{
			loserTree->current_merges[q] = elem;
			loserTree->offsets[q]++;
		}

		//adjust the loser tree
		adjust(loserTree, q);
	}
}
{% endhighlight %}

### 2.3 测试

如下是测试文件```main.c```:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include "loser_tree.h"

int main(int argc, char *argv[])
{
	int i;
	int b0[] = {10, 15, 16};
	int b1[] = {9, 18, 20};
	int b2[] = {20, 22, 40};
	int b3[] = {6, 15, 25};
	int b4[] = {12, 37, 48};

	struct Elements merge_ways[] = {
			{b0, 3},
			{b1, 3},
			{b2, 3},
			{b3, 3},
			{b4,3}
		};


	struct LoserTree *loserTree = create_losertree(merge_ways,5);
	if(!loserTree)
	{
		printf("create loser tree failure\n");
		return -1;
	}
	print_losertree(loserTree);

	printf("\nbegin %d-ways merge:\n", loserTree->k);
	multiways_merge(loserTree);
	printf("\n\n");

	print_losertree(loserTree);
	return 0x0;
}
{% endhighlight %}
编译运行：
{% highlight string %}
# gcc -o loser_tree *.c
# ./loser_tree 
5-ways loser tree:
losers[4]:4  --> 12
losers[3]:2  --> 20
losers[2]:0  --> 10
losers[1]:1  --> 9
losers[0]:3  --> 6

begin 5-ways merge:
6 9 10 12 15 15 16 18 20 20 22 25 37 40 48 

5-ways loser tree:
losers[4]:4  --> definitely-max
losers[3]:2  --> definitely-max
losers[2]:3  --> definitely-max
losers[1]:0  --> definitely-max
losers[0]:1  --> definitely-max
{% endhighlight %}

下面给出败者树初始化后的示意图：

![ds-loser-tree](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_losertree_init.jpg)



<br />
<br />
**[参看]:**



<br />
<br />
<br />


