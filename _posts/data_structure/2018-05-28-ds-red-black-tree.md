---
layout: post
title: 红黑树的原理及实现
tags:
- data-structure
categories: data-structure
description: 红黑树的原理及实现
---


本文我们主要介绍一下红黑树的实现原理。在了解红黑树之前，请先参看```2-3树```以及```avl树```的相关实现。


<!-- more -->


## 1. 红黑树

红黑树(Red-Black Tree)是一种自平衡二叉查找树。是在计算机科学中用到的一种数据结构，典型的用途是实现关联数组。它是在1972年由```Rudolf Bayer```发明的， 当时被称为平衡二叉B树(symmetric binary B-trees)。后来，在1978年被```Leo J. Guibas```和```Robert Sedgewich```修改为如今的```红黑树```。红黑树和```AVL树```类似， 都是在进行插入和删除操作时通过特定操作保持二叉查找树的平衡， 从而获得较高的查找性能。它虽然复杂， 但它的最坏情况运行时间也是非常好的，并且在实践中是高效的： 它可以在```O(logn)```时间内做查找、插入和删除操作， 这里n是树中元素的数目。


### 1.1 数据结构
红黑树的统计性能要好于```平衡二叉树```(即AVL树）， 因此红黑树在很多地方都有应用。在```C++ STL```中，很多部分（包括set、multiset、map、multimap）应用了红黑树的变体（SGI STL中的红黑树有一些变化，这些修改提供了更好的性能，以及对set操作的支持）。其他的平衡树还有： AVL树、SBT树、伸展树、TREAP树。

### 1.2 红黑树的性质

红黑树是每个节点都带有颜色属性的二叉查找树， 颜色或```红色```或```黑色```。 在二叉查找树一般的要求外，对于任何有效的红黑树我们增加了如下的额外要求：

* 性质1： 节点是红色或黑色

* 性质2： 根节点是黑色

* 性质3： 每个叶节点(nil节点, 空节点)是黑色的。 注意： 这里的叶子节点是```nil叶子```

* 性质4： 每个红色节点的两个子节点都是黑色。（从每个叶子到根的所有路径上不能有两个连续的红色节点）

* 性质5： 从任一节点到其每个```叶子```的所有路径都包含相同数目的黑色节点

<pre>
注意： 
1） 红黑树中的叶子节点均指nil叶子

2） 性质5确保没有一条路径会比其他路径长出2倍。因而，红黑树是相对接近平衡的二叉树
</pre>

![ds-rb-tree](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_rb_tree.jpg)















<br />
<br />
**[参看]:**


1. [查找（一）史上最简单清晰的红黑树讲解](http://blog.csdn.net/yang_yulei/article/details/26066409)

2. [红黑树的插入与删除](http://m.blog.csdn.net/article/details?id=51504764)

3. [浅谈算法和数据结构： 九 平衡查找树之红黑树](http://www.cnblogs.com/yangecnu/p/Introduce-Red-Black-Tree.html) 

4. [数据结构： 2-3树与红黑树](http://blog.csdn.net/aircattle/article/details/52347955)

5. [数据结构与算法](https://blog.csdn.net/hello_world_lvlcoder/article/category/6655685/1)

6. [红黑树(一)之 原理和算法详细介绍](http://www.cnblogs.com/skywang12345/p/3245399.html)
<br />
<br />
<br />


