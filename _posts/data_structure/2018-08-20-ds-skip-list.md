---
layout: post
title: 跳跃表的实现
tags:
- data-structure
categories: data-structure
description: 跳跃表的实现
---


本章我们主要介绍以下跳跃表的实现。


<!-- more -->


## 1. Skip List介绍
skip list在[wiki](https://en.wikipedia.org/wiki/Skip_list)中介绍如下：在计算机科学中，skip list是一种允许对有序序列进行快速查找的数据结构。通过维持一个链式层级结构，每一个后继序列都会跳过比上层更少的元素，这样就使得快速查找成为可能，请参看下图。查找一般会从最顶层的稀疏序列开始，直到找到两个元素，其中一个小于等于所查找元素，另一个大于等于所查找元素。

![ds-skip-list](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_skip_list_1.jpg)


skip list是一种随机化的数据结构，其效率可比拟于二叉查找树（对于大多数操作需要O(logn)平均时间）。跳跃表是在很多应用中有可能替代平衡树而作为实现方法的一种数据结构。跳跃表的算法有同平衡树一样的渐进的预期时间边界，并且更简单、更快速和使用更少的空间。目前开源的Redis和LevelDB都有用到它，它的效率和红黑树以及AVL树不相上下，但是跳跃表的原理相当简单，只要能熟练操作链表，就能轻松实现一个Skip List。跳跃表是有Pugh在1990年提出的。

## 2. skip list定义及构造步骤

### 2.1 跳跃表的性质
首先应该了解跳跃表的性质：

* 一个跳跃表应该由多个层(level)组成；

* 每一层都是一个有序的链表，排列顺序为由高层到底层，都至少包含两个链表节点，分别是前面的head节点和后面的```nil```节点；

* 最底层（level 1)的链表包含了所有元素；

* 如果一个元素出现在某一层的链表中，那么在该层之下的链表也全都会出现;

* 每个节点包含两个指针，一个指向同一层链表中的下一个元素，另一个指向下面一层具有相同值的元素；

参照上面所示的跳跃表，可以看到总共有4层，最上面一层是最高层（Level4)，最下面一层是最底层（level1)，然后每一列中的链表节点的值都相同，用指针来链接着。跳跃表的层数跟结构中最高节点的高度相同。理想情况下，跳跃表结构中第一层存在所有节点，第二层只有一半的节点，而且是均匀间隔的，第三层则存在```1/4```的节点，并且是均匀间隔的，以此类推，这样理想的层数就是```logN```。

### 2.2 跳跃表数据结构
{% highlight string %}
typedef struct skiplist_node{
	double score;
	void *obj;
	struct skiplist_node *backward;

	int levels;
	struct skiplist_node_level{
		struct skiplist_node *forward;
		//unsigned int span;
	}level[];
}skiplist_node;

typedef struct skiplist{
	struct skiplist_node *header;
	struct skiplist_node *tail;

	int level;
	int length;

	void (*free)(void *obj);
	int (*match)(void *obj1, void *obj2);
};
{% endhighlight %}

下面先给出跳跃表数据结构的一个示意图：

![ds-skip-list](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_skip_list_type.jpg)

下面我们分别讲述一下上面两个数据结构各字段含义：

**1) struct skiplist数据结构**

```skiplist```是表示跳跃表的数据结构，其各属性字段含义如下：

* header: 指向跳跃表的表头节点；

* tail: 指向跳跃表的表尾节点；

* level: 记录目前跳跃表内，层数最大的那个节点的层数（表头节点的层数不计算在内）。

* length: 记录跳跃表的长度，也即是，跳跃表目前包含节点的数量（表头节点不计算在内)。

* free: 用于释放obj对象的回调函数

* match: 用于比较两个obj对象的函数

在```skiplist```结构中，obj按score有序。

**2) struct skiplist_node数据结构**

```skiplist_node```代表跳跃表的一个节点，其各属性的含义如下：

* levels: 用于保存节点的层数。比如上边score为1.0的节点，层数为4； score为2.0的节点，层数为2； score为3.0的节点，层数为5.

* level: 代表着各层，节点中L1、L2、L3等字样标记节点的各个层。L1代表第一层， L2代表第二层，以此类推。每个层有一个前进指针和跨度(注：上面数据结构中我们注释了span,暂未使用）。前进指针用于访问位于表尾方向的其他节点，而跨度则记录了前进指针所指向节点和当前节点的距离。在上面图中，连线上带有数字的箭头就代表前进指针，而那个数字就是跨度。当程序从表头向表尾进行遍历时，访问会沿着层的前进指针进行。注意这里```level```定义的是一个零长度数组，因此需要放在结构体的最后。实际上，我们可以定义为```level[1]```，因为对于一个节点至少有一个前进指针，这样我们就可以把该字段存放在任何位置了。

* backward指针： 节点中用```BW```字样标记节点的后退指针，它指向位于当前节点的前一个节点。后退指针在程序从表尾向表头遍历时使用。

* score: 各个节点中的```1.0```、```2.0```和```3.0```是节点所保存的分值。在跳跃表中，节点按各自所保存的分值从小到大排列。

* obj: 各个节点中的```obj1```、```obj2```和```obj3```是节点所保存的成员对象。

注意表头节点和其他节点的构造是一样的：表头节点也有后退指针、分值和成员对象，不过表头节点的这些属性都不会被用到，所以上图省略了这些部分，只显示了表头节点的各个层。

### 2.3 跳跃表的初始化
如下图所示是一个初始化的空跳跃表：

![ds-skip-list-empty](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_skip_list_empty.jpg)

下面给出相应的代码实现：
{% highlight string %}
// create skiplist node
skiplist_node *create_skiplist_node(int levels, double score, void *obj)
{
	skiplist_node *node = (skiplist_node *)malloc(sizeof(skiplist_node) + levels * sizeof(struct skiplist_node_level));
	if(!node)
		return NULL;

	node->levels = levels;
	node->score = score;
	node->obj = obj;
	node->backward = NULL;

	while(levels)
	{
		node->level[--levels].forward = NULL;
	}

	return node;

}

//Init skiplist
skiplist *skiplist_create()
{
	skiplist *sl = (skiplist *)malloc(sizeof(skiplist));
	if(!sl)
		return NULL;
	sl->header = create_skiplist_node(SKIPLIST_MAX_LEVEL,0.0,NULL);
	if(!sl->header)
	{
		free(sl);
		return NULL;
	}

	sl->tail = NULL;
	sl->level = 0x0;
	sl->length = 0x0;
	sl->free = NULL;
	sl->match = NULL;

	srandom(time(0));   //for later used
	return sl;
}
{% endhighlight %}

### 2.4 跳跃表的搜索
在跳跃表中查找一个元素```x```，按照如下几个步骤进行：
{% highlight string %}
1) 从最上层的链的开头开始；

2)  假设当前位置为```p```，它向右指向的节点为```q```，且q的值为```y```，将```y```与```x```作比较：
    2.1) x == y，输出查询成功及相关信息；
    2.2) x > y, 从p向右移动到q的位置；
    2.3) x < y, 从p向下移动一格；

3) 如果当前位置在最底层的链中，且还要往下移动的话，则输出查询失败；
{% endhighlight %}
参看下图所示：

![ds-skip-list-search](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_skip_list_search.jpg)

下面我们给出跳跃表搜索的代码：
{% highlight string %}
//这里我们参照相关代码，添加了一个内部使用的队列（实际上，我们不需要这样一个队列)
//For internal use
typedef struct s_queue{
	int matches;
	int slots;
	void **objs;
}s_queue;

s_queue *s_queue_init()
{
	int slots = 4;
	s_queue *queue = (s_queue *)malloc(sizeof(s_queue));
	if(!queue)
		return NULL;
	queue->objs = (void **)malloc(sizeof(void *)*slots);
	if(!queue->objs)
	{
		free(queue);
		return NULL;
	}
	queue->slots = slots;
	queue->matches = 0x0;
	return queue;
}

int s_queue_push(s_queue *queue, void *obj)
{
	if(queue->matches >= queue->slots)
	{
		int slots = queue->slots >> 1;
		void **objs = (void **)realloc(queue->objs, slots * sizeof(void *));
		if(!objs)
			return -1;

		queue->objs = objs;
		queue->slots = slots;
	}

	queue->objs[queue->matches++] = obj;
	return 0x0;

}

static skiplist_node **skiplist_search_all(skiplist *sl, double score, int *count)
{
	int i;
	skiplist_node *p = sl->header;
	skiplist_node **result = NULL;
	s_queue *queue = s_queue_init();
	if(!queue)
		return NULL;

	for(i = sl->level-1; i >= 0; i--)
	{
		while(p->level[i].forward && (p->level[i].forward->score <= score))
		{
			if(p->level[i].forward->score == score)
			{
				s_queue_push(queue, p->level[i].forward);
				goto SEARCH_END;
			}
			p = p->level[i].forward;
		}
	}
SEARCH_END:

	if(*count = queue->matches)
	{
		result = queue->objs;
	}
	else{
		//have no matched object, 
		free(queue->objs);		
	}
	free(queue);
	
	return result;
}


void skiplist_search(skiplist *sl, double score, int *count, void *filter, void ***search_result)
{
	int matches = 0x0;
	int i;
	s_queue *queue = s_queue_init();
	if(!queue)
	{
		*count = 0;
		*search_result = NULL;
		return;
	}
	
	skiplist_node ** nodes = skiplist_search_all(sl, score, &matches);
	
	for(i = 0;i<matches;i++)
	{
		if(filter)
		{
			if(sl->match)
			{
				if(sl->match(nodes[i]->obj, filter))
					s_queue_push(queue, nodes[i]->obj);
			}
			else{
				if(nodes[i]->obj == filter)
					s_queue_push(queue, nodes[i]->obj);
			}
		}
	}

	if(*count = queue->matches)
	{
		*search_result = queue->objs;
	}
	else{
		free(queue->objs);
		*search_result = NULL;
	}

	free(nodes);
	
}
{% endhighlight %}

### 2.5 跳跃表的插入
在插入时首先确定要占据的层数K，这里通常会用到如下两种方法：

* 抛硬币方式：即采用类似于抛硬币的方式，只要是正面就累加，直到遇到反面才停止，最后记录正面的次数并将其作为要添加新元素的层。
{% highlight string %}
int get_random_level()
{
	int level = 1;

	while(random() % 2)
		level++;

	if(level > MAX_LEVEL)
		level = MAX_LEVEL;
	return level;
}
{% endhighlight %}

* 统计概率方式： 先给定一个概率```p```,产生一个0到1之间的随机数，如果这个随机数小于```p```，则将高度加1，直到产生的随机数大于概率p才停止，根据给出的结论，当概率为1/2或者1/4的时候，整体的性能会比较好（其实当p=1/2时，也就是抛硬币的方法）
{% highlight string %}
int get_random_level(double p)
{
	int level = 1;
	double convert = p * RANDMAX;

	while((double)random() < convert)
		level++;
	
	if(level > MAX_LEVEL)
		level = MAX_LEVEL;

	return level;
}
{% endhighlight %} 


<br />
<br />

1. [Skip List（跳跃表）原理详解与实现](http://www.cppblog.com/mysileng/archive/2013/04/06/199159.html)


2. [跳跃表的实现](https://www.oschina.net/code/snippet_1178986_48693)


3. [跳跃表实现(github)](https://github.com/ustcdane/skiplist)

4. [跳跃表的实现](https://www.cnblogs.com/learnhow/p/6749648.html)


5. [跳跃表的原理及实现](https://www.cnblogs.com/George1994/p/7635731.html)

6. [跳跃表 SkipList【数据结构】原理及实现](https://blog.csdn.net/u014427196/article/details/52454462)

7. [深夜学算法之SkipList：让链表飞](https://www.jianshu.com/p/fcd18946994e)
<br />
<br />


