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




<br />
<br />
**[参看]:**

1. [AVL树原理及实现](http://www.cnblogs.com/nullzx/p/6075644.html)

2. [AVL树](https://baike.baidu.com/item/AVL%E6%A0%91/10986648?fr=aladdin)

<br />
<br />
<br />


