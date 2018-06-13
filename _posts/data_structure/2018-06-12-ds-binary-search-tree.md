---
layout: post
title: 深入理解二叉搜索树（BST）
tags:
- data-structure
categories: data-structure
description: 深入理解二叉搜索树（BST）
---

本文主要讲述一下二叉搜索树相关原理及操作。


<!-- more -->


## 1. 二叉搜索树

二叉查找树（Binary Search Tree），又称为二叉搜索树或二叉排序树。它或者是一棵空树，或者是具有下列性质的```二叉树```: 若它的左子树不空，则左子树上所有节点的值均小于它根节点的值； 若它的右子树不空，则右子树上所有节点的值均大于它的根节点的值； 它的左、右子树也分别为二叉排序树。

## 2. 节点查找

在二叉搜索树```b```中查找```x```的过程如下：
<pre>
若b是空树，则搜索失败，否则：

若x等于b的根节点的数据域之值，则查找成功；否则：
若x小于b的根节点的数据域之值， 则搜索左子树； 否则：
搜索右子树
</pre>

![ds-bst-search](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_bst_search.jpg)

下面给出BST查找的递归算法与非递归算法：

1） 递归算法
{% highlight string %}
struct BSTNode *search(BSTNode *root, int key)
{
    if(!root || root->key == key)
		return root;
    else if(key < root->key)
        return search(root->left, key);
    else
        return search(root->right, key);
}
{% endhighlight %}

2) 非递归算法
{% highlight string %}
struct BSTNode *search(BSTree *root, int key)
{
	BSTNode *p = root;
    while(p)
    {
         if(p->key == key)
            return p;
         else if(p->key > key)
            p = p->left;
         else
            p = p->right;
    }

    return p;
}
{% endhighlight %}

## 3. 前驱与后继
 
对于给定的一棵二叉搜索树，如果所有节点的key均不相同，那么```节点x```的前驱是指小于```x.key```的最大关键字节点； 而一个节点x的后继是指大于```x.key```的最小关键字节点。

**1） x节点后继**







<br />
<br />
**[参看]:**

1. [深入理解二叉搜索树（BST）](https://blog.csdn.net/u013405574/article/details/51058133)

<br />
<br />
<br />


