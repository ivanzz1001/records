---
layout: post
title: 数据结构之Trie
tags:
- data-structure
categories: data-structure
description: 数据结构之Trie
---


本文主要介绍Trie树的一些基本概念，转载自[《数据结构之Trie》](http://blog.csdn.net/qq_33583069/article/details/51942534)。


<!-- more -->


## 1. Trie树的基本概念
{% highlight string %}
In computer science, a trie, also called digital tree and sometimes radix tree or prefix tree (as they can be searched by prefixes), 
is an ordered tree data structure that is used to store a dynamic set or associative array where the keys are usually strings.

在计算机科学中，一个Trie树，又称为数字树，有时候也被称为基数树(radix tree)或者前缀树(prefix tree,这是因为它们可以按前缀来进行查找)。
它是一种有序的树形数据结构，通常用于存储动态集(dynamic set)或者关联数组(associative array)，动态集与关联数组的键通常都是string类型。


Unlike a binary search tree, no node in the tree stores the key associated with that node; instead, its position in the tree defines the key with which it is associated.
All the descendants of a node have a common prefix of the string associated with that node, and the root is associated with the empty string. Values are not necessarily 
associated with every node. Rather, values tend only to be associated with leaves, and with some inner nodes that correspond to keys of interest. For the space-optimized 
presentation of prefix tree, see compact prefix tree.

和二叉搜索树不一样，树中没有节点来存储与该节点所关联的键(key)；相反，它在树中的位置定义了它与该节点关联的键。



{% endhighlight %}






<br />
<br />
**[参看]:**

1. [数据结构之Trie](http://blog.csdn.net/qq_33583069/article/details/51942534)

2. [ Trie实践：一种比哈希表更快的数据结构](http://blog.csdn.net/stevenkylelee/article/details/38343985)


3. [AVL树](https://baike.baidu.com/item/AVL%E6%A0%91/10986648?fr=aladdin)

4. [字典树](https://baike.baidu.com/item/%E5%AD%97%E5%85%B8%E6%A0%91/9825209?fr=aladdin&fromid=517527&fromtitle=Trie%E6%A0%91)
<br />
<br />
<br />


