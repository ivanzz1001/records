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

现在我们考虑如何求解一个```节点x```的后继。对于```节点x```，如果其右子树不为空，那么x的后继一定是其右子树的最左边的节点。而如果x的右子树为空，并且有一个后继，那么其后继必然是x的最```底层```的祖先，并且后继的```左孩子```也是x的一个祖先，因此为了找到这样的后继节点，只需要从x开始沿着树向上移动，直到遇到一个节点，这个节点是其双亲的左孩子。（例如： 上图中节点12的后继节点是16）

下面给出求后继节点的伪代码：
{% highlight string %}
struct BSTNode *successor(BSTNode *x)
{
    if(x->right)
    {
       x = x->right;
       while(x->left)  x=x->left;
       return x;
    }
   
	parent = x->parent;
    while(parent && parent->right == x)
    {
        x=parent;
        parent = parent->parent;
    }



    return parent;
}
{% endhighlight %}

**2) x节点的前驱**

下面我们考虑如何求解一个```节点x```的前驱。对于```节点x```，如果它是其双亲节点的右节点，则直接前驱为其双亲节点。而如果x其父节点的左节点， 那么其前驱必然是x的最```底层```的祖先，并且前驱的```右孩子```也是x的祖先， 因此为了知道这样的前驱节点， 只需要从x沿着树向上移动， 知道遇到一个节点，这个节点是其双亲的右孩子。（例如： 上图中的节点10的前驱节点是9）

下面给出求前驱节点的伪代码：
{% highlight string %}
struct BSTNode *predecessor(BSTNode *x)
{
   parent = x->parent;
   while(parent && parent->left == x)
   {
        x = parent;
        parent = parent->parent;
   }

   return parent;
}
{% endhighlight %}





<br />
<br />
**[参看]:**

1. [深入理解二叉搜索树（BST）](https://blog.csdn.net/u013405574/article/details/51058133)

<br />
<br />
<br />


