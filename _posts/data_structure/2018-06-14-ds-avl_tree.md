---
layout: post
title: AVL树原理及实现
tags:
- data-structure
categories: data-structure
description: AVL树原理及实现
---

本文主要讲述一下AVL树的原理及实现。


<!-- more -->


## 1. AVL树定义

在计算机科学中，AVL树是最先发明的自平衡二叉查找树。对于一般的搜索二叉树而言，如果数据恰好是按照从小到大（或从大到小）的顺序插入的，那么搜索二叉树就退化为链表， 这个时候插入和删除的时间都会上升到```O(n)```， 而对于海量数据而言是无法忍受的。即使是一棵由完全随机的数据构造成的搜索二叉树， 从统计角度去分析，在进行若干次插入和删除操作，这棵搜索二叉树的高度也不能令人满意。这个时候大家就希望能有一种二叉树解决上述问题。平衡搜索二叉树，它的基本原理就是在插入和删除的时候，根据情况进行调整，以降低二叉树的高度。平衡搜索二叉树的代表就是AVL树和红黑树。


**1） AVL树的定义**


```AVL树```中任何节点的两棵子树的高度之差的绝对值不超过1，所以它也被称为高度平衡树。查找、插入和删除在平均和最坏情况下都是O(logn)。增加和删除可能需要通过一次或多次树旋转来重新平衡这棵树。AVL树得名于它的发明者```G.M. Adelson-Velsky 和 E.M. Landis```，他们在1962年论文"An algorithm for the organization of informatio"中发表了它。
<pre>
注： AVL树定义并不是说从根节点到叶子节点的最长距离比最短距离大1
</pre>


![ds-avl-tree](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_avl_tree.png)
上图就是一棵AVL树，从根节点到叶子节点的最短距离是5，最长距离是9.

**2) AVL树的特点**

* 本身首先是一棵二叉搜索树

* 带有平衡条件： 每个节点的左右子树的高度之差的绝对值（平衡因子）最多为1。
<pre>
注： 节点的平衡因子是它的左子树的高度减去右子树的高度。带有平衡因子1、0或-1的节点被认为是平衡的。
</pre>


## 2. 旋转的定义
因为很多书中对```旋转```的定义不一致，所以我们有必要在这里说明一下：

* 以某一节点为轴，它的左枝顺时针旋转，作为新子树的根， 我们称之为```顺时针旋转```（clockwise)或者```右旋转```；

![ds-node-right-rotate](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_node_right_rotate.jpg)

下面给出右旋转伪代码：
{% highlight string %}
struct AVLNode *right_rotate(struct AVLNode *node)
{
    struct AVLNode *left = node->left;
    node->left = left->right;
    left->right = node;

    return left;
}
{% endhighlight %}

* 以某一节点为轴，它的右枝逆时针旋转，作为新子树的根， 我们称为```逆时钟旋转```(anti clockwise)或者```左旋转```;

![ds-node-left-rotate](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_node_left_rotate.jpg)

下面给出左旋转伪代码：
{% highlight string %}
struct AVLNode *left_rotate(struct AVLNode *node)
{
    struct AVLNode *right = node->right;
    node->right = right->left;
    right->left = node;

    return right;
}
{% endhighlight %}

说明： 对于上面的```右旋转```、```左旋转```, 都不会影响到旋转节点的父节点。


## 3. AVL树的插入操作

AVL树的插入操作首先会按照普通搜索二叉树的插入操作进行，当插入一个数据后， 我们会沿着插入数据时所经过的节点回溯。回溯的过程中会判断回溯路径中每个节点的左子树高度与右子树高度之差的绝对值是否超过1， 如果超过1我们进行调整， 调整的目的是使得该节点满足AVL树的定义。调整的情况可以分为以下四旋转操作， 旋转操作可以降低树的高度， 同时不改变搜索二叉树的性质（即任何一个节点左子树中节点的key小于该节点的key， 右子树中节点的key大于该节点的key)。

**1) 情形1**

节点```X```左子树比右子树高度大2，且插入节点位于```X```左孩子节点```XL```左子树上：

![ds-avl-right-rotate](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_avl_right_rotate.jpg)



**2) 情形2**

节点```X```的右子树比左子树高度大2， 且插入节点位于```X```右孩子节点```XR```的右子树上。

![ds-avl-left-rotate](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_avl_left_rotate.jpg)


**3) 情形3**

节点```X```左子树比右子树高度大2， 且插入节点位于```X```左孩子```XL```的右子树上：

![ds-avl-left-right-rotate](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_avl_left_right_rotate.jpg)

**4) 情形4**

节点```X```的右子树比左子树高度大2， 且插入节点位于```X```右孩子```XR```的左子树上：

![ds-avl-right-left-rotate](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_avl_right_left_rotate.jpg)


## 4. AVL树的删除操作

AVL树的删除操作和插入操作一样，首先会按照普通搜索二叉树的删除操作进行。 当删除一个数据后， 和插入操作一样， 我们通常采取的策略是沿着删除数据时所经过的节点回溯， 回溯的过程中会判断该节点的```左右子树```的高度之差是否超过1， 如果超过1， 我们就进行调整， 调整的目的是使得该节点满足AVL树的定义。调整情况可以分为4种，和插入过程完全一样，这里不再赘述。




<br />
<br />
**[参看]:**

1. [AVL树原理及实现](http://www.cnblogs.com/nullzx/p/6075644.html)

2. [AVL树](https://baike.baidu.com/item/AVL%E6%A0%91/10986648?fr=aladdin)

<br />
<br />
<br />


