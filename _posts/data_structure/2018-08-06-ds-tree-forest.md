---
layout: post
title: 树和森林
tags:
- data-structure
categories: data-structure
description: 树和森林
---

本节我们将讨论树的表示及其遍历操作，并建立森林与二叉树的对应关系。


<!-- more -->


## 1. 树的存储结构
在大量的应用中，人们曾使用多种形式的存储结构来表示树。这里，我们介绍3种常用的链表结构。

### 1.1 双亲表示法

假设以一组连续空间存储树的结点，同时在每个结点中附设一个指示器指示其双亲节点在链表中的位置，其形式说明如下：
{% highlight string %}
//-------树的双亲表存储表示--------

#define MAX_TREE_SIZE 100

typedef struct PTNode{      //节点结构
	TElemType data;
	int parent;             //双亲位置域
}PTNode;

typedef struct{             //树结构
	PTNode nodes[MAX_TREE_SIZE];
	int r,n;                //根的位置和节点数
}PTree;
{% endhighlight %}
例如，图```6.13```展示一棵树及其双亲表示的存储结构。

![ds-tree-parent](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_tree_parent.jpg)


这种存储结构利用了每个节点（除根以外）只有唯一的双亲性质。PARENT(T,x)操作可以在常量时间内实现。反复调用PARENT操作，直到遇到无双亲的节点时，便找到了树的根，这就是ROOT(x)操作的过程。但是在这种表示方法中，求节点的孩子时需要遍历整个结构。

### 1.2 孩子表示法
由于树中每个节点可能有多棵子树，则可用多重链表，即每个节点有多个指针域，其中每个指针指向一棵子树的根节点，此时链表中的结点可以有如下两种结点格式：

![ds-tree-child](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_graph_child.jpg)




     


<br />
<br />
**[参看]:**


<br />
<br />
<br />


