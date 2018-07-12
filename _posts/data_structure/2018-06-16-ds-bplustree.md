---
layout: post
title: B+树详解
tags:
- data-structure
categories: data-structure
description: B+树详解
---

本文详细介绍一下B+树的相关原理及实现。


<!-- more -->

## 1. B+树

```B+树```是一种数据结构，是一个N叉排序树，每个节点通常有多个孩子，一棵```B+树```包含根节点、内部节点和叶子节点。根节点可能是一个叶子节点， 也可能是一个包含两个或两个以上孩子节点的节点。

```B+树```通常用于数据库和操作系统的```文件系统```中。NTFS、ReiserFS、NSS、XFS、JFS、ReFS和BFS等文件系统都在使用```B+树```作为元数目索引。```B+树```的特点是能够保持数据稳定有序， 其插入与修改拥有较稳定的对数时间复杂度。```B+树```元素自底向上插入。

### 1.1 B+树的定义

```B+树```是应文件系统所需而出的一种```B-树```的变型树。一棵```m阶```的```B+树```和m阶的```B-树```的差异在于：

**1)** 有n棵子树的节点中含有n个关键字(即每个关键字对应一棵子树)；

**2)** 所有叶子节点中包含了全部关键字的信息， 及指向含这些关键字记录的指针，且叶子节点本身依关键字的大小自小而大顺序链接； 

**3)** 所有的非终端节点可以看成是索引部分，节点中仅含有其子树（根节点）中的最大（或最小)关键字

**4)** 除根节点外，其他所有节点中所含关键字的个数必须```>=⌈m/2⌉```(注意： ```B-树```是除根以外的所有非终端节点至少有⌈m/2⌉棵子树)

下图是所示为一棵3阶的```B+树```，通常在```B+树```上有两个指针头， 一个指向根节点，另一个指向关键字最小的叶子节点。因此，可以对```B+树```进行两种查找运算： 一种是从最小关键字起顺序查找，另一种是从根节点开始，进行随机查找。

![ds-bplus-tree1](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_bplus_tree1.jpg)

下图是另一棵```3阶B+树```:

![ds-bplus-tree2](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_bplus_tree2.jpg)




<br />
<br />
**[参看]:**

1. [B-树，B+树，B*树详解](https://blog.csdn.net/aqzwss/article/details/53074186)

2. [原B+tree详解及实现(C语言)](https://blog.csdn.net/xiaohusaier/article/details/77101640)

3. [B+树](https://baike.baidu.com/item/B+%E6%A0%91/7845683)

<br />
<br />
<br />


