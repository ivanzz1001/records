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
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>

typedef int ElemType;

typedef struct MSegment{
	ElemType *base;
	int size;
	
	int offset;
}m_segment_t;

typedef struct LoserTree{
	int k;
	
	int *losers;               //losers[i]为-1，表示绝对最小值; losers[i]为-2，表示绝对最大值 
	m_segment_t *segments;
}loser_tree_t;


loser_tree_t * create_loser_tree(int k){
	if(k <= 1)
		return NULL;
		
	loser_tree_t *root = (loser_tree_t *)malloc(sizeof(loser_tree_t));
	if(root){
		root->k = k;
		
		root->losers = (int *)malloc(sizeof(int) * k);
		root->segments = (m_segment_t *)malloc(sizeof(m_segment_t) * k);
		
		if(!root->losers || !root->segments){
			free(root->losers);
			free(root->segments);
			free(root);
			return NULL;
		}
		
		memset(root->losers, 0x0, sizeof(int)*k);
		memset(root->segments, 0x0, sizeof(m_segment_t)*k);
		
		return root;
	}
	
	
	return root;
		
}

void print_loser_tree(loser_tree_t *root){
	
	for(int i= 0;i<root->k;i++){
		printf("第%d个归并段：", i);
		
		for(int j=0;j<root->segments[i].size;j++)
			printf("%d ", root->segments[i].base[j]);
		printf("\n");
		
	}
	
}

void adjust(loser_tree_t *root, int s){
	
	//adjust the loser-tree from the leafs to the root
	int t = ((root->k -1) + (s + 1)) >> 1;   //losers[t] is the parent of s
	
	
	int nexts = s;
	
	if(root->segments[s].offset >= root->segments[s].size)
		nexts = -2;
	
	while(t > 0){
		
		if(root->losers[t] == -1){
			root->losers[t] = nexts;
			nexts = -1;
		}
		else if(nexts == -2){
			
			nexts = root->losers[t];
			root->losers[t] = -2;
		}
		else if(root->losers[t] == -2 || nexts == -1)
		{
			//do nothing
		}
		else{
			ElemType e = *(root->segments[nexts].base + root->segments[nexts].offset);
			ElemType f = *(root->segments[root->losers[t]].base + root->segments[root->losers[t]].offset);
		
			if(e > f){
				int curLoser = nexts;
				nexts = root->losers[t];
				root->losers[t] = curLoser;
			}
		}
		
		t = t>>1;
	}
	
	root->losers[t] = nexts;
}


int init_loser_tree(loser_tree_t *root){
	
	for(int i = 0;i<root->k;i++)
		root->losers[i] = -1;
	
	printf("将创建%d个归并段，每个归并段中以从小到大顺序输入若干元素(元素与元素之间以空格分割)\n", root->k);
	for(int i=0;i<root->k;i++){
		
		char buf[2048] = {0};
		char *p = buf;
		char *end;
		std::vector<long> vec;
		long num;
		
		printf("请输入第 %d 个归并段元素(元素之间以空格分割): ", i);
		fgets(p, 2048, stdin);
		
		while(num = strtol(p, &end, 10)){
			vec.push_back(num);
			p = end+1;
		}
		
		if(vec.size() > 0){
				root->segments[i].size = vec.size();
				root->segments[i].base = (ElemType *)malloc(sizeof(ElemType) * vec.size());
				if(!root->segments[i].base){
					printf("初始化归并段 %d 失败\n", i);
					return -1;
				}
				
				for(int j= 0;j<vec.size();j++)
					root->segments[i].base[j] = (int)vec[j];
		}
		
	}
	
	printf("\n");
	printf("=====输出我们创建的归并段=====\n");
	print_loser_tree(root);
	printf("\n");
	
	for(int i = root->k-1;i>=0;i--){
		adjust(root, i);
	}
	
	
	return 0x0;
}

void multiways_merge(loser_tree_t *root){
	
	while(root->losers[0] != -2){
		int s = root->losers[0];
		if(s == -1){
			printf("skip definitly minimum\n");
			continue;
		}
		
		printf("%d ", *(root->segments[s].base + root->segments[s].offset));
		root->segments[s].offset++;
		
		adjust(root, s);
	}
	printf("\n");
}

int main(int argc, char *argv[]){
	loser_tree_t *root = NULL;
	
	root = create_loser_tree(5);
	init_loser_tree(root);
	
	multiways_merge(root);
	
	return 0x0;
}
{% endhighlight %}
编译运行：
{% highlight string %}
# gcc -o loser_tree loser_tree.cpp -lstdc++
# ./loser_tree 
将创建5个归并段，每个归并段中以从小到大顺序输入若干元素(元素与元素之间以空格分割)
请输入第 0 个归并段元素(元素之间以空格分割): 
请输入第 1 个归并段元素(元素之间以空格分割): 3 20
请输入第 2 个归并段元素(元素之间以空格分割): 4 
请输入第 3 个归并段元素(元素之间以空格分割): 50
请输入第 4 个归并段元素(元素之间以空格分割): 201

=====输出我们创建的归并段=====
第0个归并段：
第1个归并段：3 20 
第2个归并段：4 
第3个归并段：50 
第4个归并段：201 

3 4 20 50 201
{% endhighlight %}


<br />
<br />
**[参看]:**



<br />
<br />
<br />


