---
layout: post
title: 数据结构之2-3树
tags:
- data-structure
categories: data-structure
description: 数据结构之2-3树
---


```2-3树```是最简单的```B-树```(Balanced tree)结构，其每个非叶子节点都有```2个```或```3个```子女，而且所有叶子都在同一层上。虽然```2-3树```在实际应用中不多，但是理解```2-3树```对理解```红黑树```具有很大的帮助。

<!-- more -->

## 1. 定义

一棵```2-3查找树```或为一棵空树，活由以下节点组成：

* **2-节点**: 含有一个键（及其对应的值）和两条链接， 左链接指向的```2-3树```中的键都小于该节点，右链接指向的```2-3树```中的键都大于该节点；

* **3-节点**: 含有两个键（及其对应的值）和三条链接， 左链接指向的```2-3树```中的键都小于该节点， 中链接指向的```2-3树```中的键都位于该节点的两个键之间，右链接指向的```2-3树```中的键都大于该节点；

和以前一样，我们将指向一棵空树的链接称为```空链接```。```2-3查找树```如下图所示：

![ds-23tree-definition](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_23tree_definition.jpg)

一棵完美平衡的```2-3查找树```所有空链接到根节点的的距离都相同。下面讲述一下```2-3查找树```的基本操作：

* 查找节点

* 插入节点

* 变换

* 构造2-3树

* 删除节点

## 2. 查找节点

要判断一个键是否存在树中，先将它和根节点中的键比较， 如果它和其中任意一个相等，查找命中； 否则就根据比较的结果找到指向相应区间的链接，并在其指向的子树中递归地继续查找， 如果找到了空链接上，则查找未命中。

![ds-23tree-find](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_23tree_find.jpg)


## 3. 插入节点
要在```2-3树```中插入一个新节点，我们可以和```二叉查找树```一样对```2-3树```进行一次未命中查找，然后把新节点挂在树的底部。但这样的话无法保证```2-3树```的完美平衡性。我们使用```2-3树```的主要原因在于它能够在插入之后继续保持平衡。下面我们根据```未命中```查找结束时的节点类型，分多种不同情况说明：

**1) 向2-节点中插入新键**

当未命中查找结束于一个```2-节点```时，直接将```2-节点```替换为一个```3-节点```，并将要插入的键保存其中：

![ds-23tree-insert1](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_23tree_insert_1.jpg)

**2) 向一棵只含3-节点的树中插入新键**

先临时将新键存入唯一的```3-节点```中，使其成为一个```4-节点```，再将它转化为一棵由3个```2-节点```组成的```2-3树```，分解后树高会增加1：

![ds-23tree-insert2](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_23tree_insert_2.jpg)

**3) 向一个双亲节点为2-节点的3-节点中插入新键**

先构造一个临时的```4-节点```并将其分解，分解时将中键移动到双亲节点中（中键移动后，其双亲节点的中的位置由键的大小确定）：

![ds-23tree-insert3](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_23tree_insert_3.jpg)


**4) 向一个双亲节点为3-节点的3-节点插入新键**

一直向上分解构造的临时```4-节点```，并将中键移动到更高层双亲节点，直到遇到一个```2-节点```并将其替换为一个不需要继续分解的```3-节点```，或者是到达树根(```3-节点```)

![ds-23tree-insert4](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_23tree_insert_4.jpg)


**5） 分解根节点**

如果从插入节点到根节点的路径上全是```3-节点```，根将最终被替换为一个临时的```4-节点```，将临时的```4-节点```分解为三个```2-节点```，分解后树高会增加1：

![ds-23tree-insert5](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_23tree_insert_5.jpg)






<br />
<br />
**[参看]:**

1. [2-3树删除和插入操作的小结](https://blog.csdn.net/sumoyu/article/details/8277220)

2. [2-3查找树的插入与删除](https://blog.csdn.net/hello_world_lvlcoder/article/details/72615092)

3. [2-3树](https://baike.baidu.com/item/2-3%E6%A0%91/3484656?fr=aladdin)

<br />
<br />
<br />


