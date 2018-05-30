---
layout: post
title: 红黑树的原理及实现
tags:
- data-structure
categories: data-structure
description: 红黑树的原理及实现
---


本文我们从内存数据的```查找```出发, 引出```2-3```树， 然后在此基础上讲述一下红黑树的原理及实现。


<!-- more -->


## 1. 查找

我们使用```符号表```这个词来描述一张抽象的表格，我们会将信息（值）存储在其中， 然后按照指定的键来搜索并获取这些信息。键和值的具体意义取决于不同的应用。符号表可能会保存很多键和很多信息，因此实现一张高效的符号表也是一项很有挑战的任务。

我们一般会用三种经典的数据类型来实现高效的符号表： 二叉查找树、红黑树、散列表


### 1.1 二分查找

我们使用有序数组来存储```键```，经典的二分查找能够根据数组的索引大大减少每次查找所需的比较次数。在查找时，我们先将被查找的键和子数组的中间键比较。如果被查找的键小于中间键，我们就在左子数组中继续查找； 如果大于我们就在右子数组中继续查找，否则中间键就是我们要找的键。

一般情况下，二分查找都比顺序查找快得多， 它也是众多实际应用程序的最佳选择。对于一个静态表（不允许插入）来说，将其在初始化时就排好序是值得的。

当然，二分查找也有很多应用场景下不适合。现代应用需要同时能够支持高效的查找和插入两种操作的符号表实现。 也就是说， 我们需要在构造庞大的符号表的同时能够任意插入（也许还有删除）键值对，同时也要能够完成查找操作。

要支持高效的插入操作，我们似乎需要一种链式结构。但单链接的链表是无法使用二分查找的，因为二分查找的高效来自于能够快速通过索引取得任何子数组的中间元素。为了将二分查找的效率和链表的灵活性结合起来，我们需要更加复杂的数据结构。

能够同时拥有两者的就是```二叉查找树```。

### 1.2 二叉查找树

一棵二叉查找树(BST)是一棵二叉树，其中每个节点都含有一个可比较的键（以及相关联的值）且每个节点的键都大于其左子树任意节点的键而小于右子树的任意节点的键。

![ds-bst-figure1](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_bst_figure1.jpg)

一棵二叉树代表了一组键（及其相应值）的集合，而同一集合可以用多棵不同的二叉树表示。如果我们将一棵二叉查找树的所有键投影到一条直线上，保证一个节点的左子树中的键出现在它的左边， 右子树中的键出现在它的右边，那么我们一定可以得到一条有序的键列：

![ds-bst-figure2](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_bst_figure2.jpg)


**1) 查找**

在二叉查找树中查找一个键的递归算法： 如果树是空的，则查找未命中。如果查找的键和根节点的键相等，查找命中。否则我们就在适当的子树中继续查找。如果被查找的键较小就选择左子树；较大就选择右子树。

在二叉查找树中，随着我们不断向下查找，当前节点所表示的子树的大小也在缩小（理想情况下是减半）

**2) 插入**

查找代码几乎和二分查找一样简单，这种简洁性是二叉查找树的重要特性之一。而二叉查找树的另一个更重要的特性就是插入的实现难度和查找差不多。

当查找一个不存在于树中的节点并结束于一条空链时，我们需要做的就是将链接指向一个含有被查找的键的新节点。如果被查找的键小于根节点的键，我们会继续在左子树中插入该键； 否则在右子树中插入该键。

**3) 分析**

使用二叉查找树的算法的运行时间取决于树的形状，而树的形状又取决于键被插入的先后顺序。在最好的情况下，一棵含有```N个节点```的节点的树是完全平衡的，每条空链接和根节点的距离都为```logN```。在最坏的情况下，搜索路径上可能有N个节点。但一般情况下树的形状和最好情况更接近。

![ds-bst-figure3](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_bst_figure3.jpg)

我们假设键的插入顺序是随机的。对于这个模型的分析而言， 二叉查找树和快速排序几乎就是```双胞胎```。 树的根节点就是快速排序的第一个切分元素（左侧的键都比它小，右侧的键都比它大），而这对于所有子树同样适用。这和快速排序中对于子数组的递归排序完全对应。

{% highlight string %}
在由N个随机构建的二叉查找树中， 查找命中平均所需的比较次数为2logN。 N越大这个公式越准确。
{% endhighlight %}

注： 这里我们主要是通过二叉查找树引出```2-3树```，进而引出```红黑树```， 因此对于二叉查找树并不会详细介绍。欲详细了解，请参看其他章节。


## 2. 平衡树
在一棵含有```N```个节点的的树中，我们希望树高为```logN```。这样我们就能够保证所有查找都能在```logN```次比较内结束，就和二分查找一样。不幸的是，在动态插入中保证树的完美平衡的代价太高了。我们放松对完美平衡的要求，使符号表api中所有操作均能够在对数时间内完成。

## 2.1  ```2-3查找树```

为了保证树的平衡性，我们需要一些灵活性， 因此在这里我们允许树中的一个节点保存多个键：

* ```2-节点```: 含有一个键(及值）和两条链接， 左链接指向的```2-3树```中的键都小于该节点， 右链接指向的```2-3树```中的键都大于该节点。

* ```3-节点```: 含有两个键（及值）和三条链接， 左链接指向的```2-3树```中的键都小于该节点， 中链接指向的```2-3树```中的键都位于该节点的两个键之间， 右链接指向的```2-3树```中的键都大于该节点。
{% highlight string %}
注： 这里的2-3指的是2叉-3叉的意思
{% endhighlight %}

![ds-23tree-figure1](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_23tree_firgure1.jpg)

一棵完美平衡的```2-3查找树```所有空链接到根节点的的距离都相同。

**1) 查找**

要判断一个键是否在树中，我们先将它和根节点中的键比较。如果它和其中任何一个相等，查找命中。否则，我们就根据比较的结果找到指向相应区间的链接，并在其指向的子树中递归的继续查找。如果这是个空链接，查找未命中。

下面给出伪代码：
{% highlight string %}
struct two_three_tnode{
    int type;      //node type  0--2节点  1--3节点
    int value;
    struct two_three_tnode *left;
    struct two_three_tnode *right;
    char middle[0];        //store the second value and the middle pointer
};



struct two_three_tnode *find(struct two_three_tnode *root, int tofind)
{
    struct two_three_tnode *p = root;

    while(p)
    {
        if(p->type == 0)
        {
             if(p->value == tofind)
                 return p;
             else if(p->value > tofind)
                 p = p->left;
             else
                 p = p->right;
        }
        else if(p->type == 1)
        {
            int second = *(int *)(p + sizeof(struct two_three_tnode));
            struct two_three_tnode *middle = (struct two_three_tnode *)(p + sizeof(struct two_three_tnode) + sizeof(int));

            if(p->value == tofind)
               return p;
            else if(second == tofind)
               return p;

            if(tofind < p->value)
               p = p->left;
            else if(tofind > second)
               p = p->right;
            else 
               p = middle;
        }
    }

    return NULL;
}
{% endhighlight %}






<br />
<br />
**[参看]:**


1. [查找（一）史上最简单清晰的红黑树讲解](http://blog.csdn.net/yang_yulei/article/details/26066409)

2. [红黑树的插入与删除](http://m.blog.csdn.net/article/details?id=51504764)

3. [浅谈算法和数据结构：八 平衡查找树之2-3树](http://www.cnblogs.com/yangecnu/p/Introduce-2-3-Search-Tree.html)

4：[浅谈算法和数据结构： 九 平衡查找树之红黑树](http://www.cnblogs.com/yangecnu/p/Introduce-Red-Black-Tree.html) 

5：[数据结构： 2-3树与红黑树](http://blog.csdn.net/aircattle/article/details/52347955)


<br />
<br />
<br />


