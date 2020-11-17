---
layout: post
title: 遍历二叉树和线索二叉树
tags:
- data-structure
categories: data-structure
description: 遍历二叉树和线索二叉树
---

本章我们主要介绍一下遍历二叉树和线索二叉树。


<!-- more -->

## 1. 遍历二叉树
在二叉树的一些应用中，常常要求在树中查找具有某种特征的结点，或者对树中全部结点逐一进行某种处理。这就提出了一个**遍历二叉树**(traversing binary tree)的问题。即如何按某条搜索路径巡访树中每个结点，使得每个结点均被访问一次，且仅被访问一次。“访问”的含义很广，可以是对结点做各种处理，如输出结点的信息等。遍历对线性结构来说，是一个很容易解决的问题。而对二叉树则不然，由于二叉树是一种非线性结构，每个结点都可能有两棵子树，因而需要寻找一种规律，以便使二叉树上的结点能排列在一个线性队列上，从而便于遍历。

回顾二叉树的递归定义可知，二叉树是由3个基本单元组成：根节点、左子树和右子树。因此，若能依次遍历这三部分，便是遍历了整棵二叉树。假如L、D、R分别表示遍历左子树、访问根节点和遍历右子树，则可有DLR、LDR、LRD、DRL、RDL、RLD这6种遍历二叉树的方案。若限定先左后右，则只有前3种情况，分别称之为先（根）序遍历、中（根）序遍历和后（根）序遍历。基于二叉树的递归定义，可得下述遍历二叉树的递归算法定义。

* 先序遍历

先序遍历二叉树的操作定义为： 
<pre>
若二叉树为空，则空操作；否则

1） 访问根结点；

2） 先序遍历左子树；

3） 先序遍历右子树
</pre>


* 中序遍历

中序遍历二叉树的操作定义为： 
<pre>
若二叉树为空，则空操作；否则

1） 中序访问左子树；

2） 访问根结点；

3） 中序遍历右子树
</pre>

* 后序遍历

后序遍历二叉树的操作定义为： 
<pre>
若二叉树为空，则空操作；否则

1） 后续遍历左子树；

2） 后序遍历右子树；

3） 访问根结点
</pre>

如下算法6.1给出了先序遍历二叉树基本操作的递归算法在二叉链表上的实现。读者可类似地实现中序遍历和后续遍历的递归算法，此处不再一一举例。

**算法6.1:**
{% highlight string %}
Status PreOrderTraverse(BiTree T, Status (*Visit)(TElemType e))
{
	//采用二叉链表存储结构，Visit是对结点操作的应用函数
	//先序遍历二叉树T，对每个结点调用函数Visit一次且仅一次。一旦Visit失败，则操作失败
	
	//最简单的Visit函数是：
	//	Status PrintElement(TElemType e)
	//	{
	//		printf(e);                        //输出元素e的值，实用时，加上格式串
	//		return OK;
	//	}
	
	//调用实例： PreOrderTraverse(T, PrintElement);
	
	if(T){
		if(Visit(T->data))
			if(PreOrderTraverse(T->lchild, Visit))
				if(PreOrderTraverse(T->rchild, Visit))
					return OK;
					
		return ERROR;			
	}else
		return OK;
	
}
{% endhighlight %}
例如图6.9所示的二叉树表示下述表达式：
<pre>
          a + b * (c - d) - e / f
</pre>

![ds-tree-exp](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_tree_exp.jpg)

若先序遍历此二叉树，按访问结点的先后次序将结点排列起来，可得到二叉树的先序序列为：
<pre>
         - + a * b - c d / e f                   (6-3)
</pre>
类似地，中序遍历此二叉树，可得此二叉树的中序序列为：
<pre>
         a + b * c - d - e / f                   (6-4)
</pre>

后序遍历此二叉树，可得此二叉树的后序序列为
<pre> 
         a b c d - * + e f / -                   (6-5)
</pre>

从表达式来看，以上3个序列(6-3)、(6-4)、(6-5)恰好为表达式的前缀式（波兰式）、中缀表示和后缀表示（逆波兰式）。

![ds-tree-traverserep](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_tree_traverserep.jpg)


## 2. 线索二叉树
我们知道：遍历二叉树是以一定规则将二叉树中节点排列成一个线性序列，得到二叉树中节点的先序序列或中序序列或后序序列。这实质上是对一个非线性结构进行线性化操作，使每个节点（除第一个和最后一个外）在这些线性序列中有且仅有一个直接前驱和直接后继（在不至于混淆的情况下，我们省去直接二字）。例如下图所示的二叉树的节点的中序序列为```a+b*c-d-c/f```中```'c'```的前驱是```'*'```，后继是```'-'```。

![ds-expression-tree](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_expression_tree.jpg)

但是，当以二叉链表作为存储结构时，只能找到节点的左、右孩子信息，而不能直接得到节点在任一序列中的前驱和后继信息，这种信息只有在遍历的动态过程中才能得到。

如何保存这种在遍历过程中得到的信息呢？ 一个最简单的办法是在每个节点上增加两个指针域```fwd```和```bkwd```，分别指示节点在依任一次序遍历时得到的前驱和后继信息。显然，这样做使得结构的存储密度大大降低。另一方面，在有n个节点的二叉链表中必定存在n+1个空链域。由此设想能否利用这些空链域来存放节点的前驱和后继信息。

试作如下规定： 若节点有左子树，则其lchild域指示其左孩子，否则令lchild域指示其前驱； 若节点有右子树，则其rchild域指示其右孩子，否则令rchild域指示其后继。为了避免混淆，尚需改变节点结构，增加两个标志域：

![ds-thread-tree-node](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_thread_tree_node.jpg)


以这种节点结构构成的二叉链表作为二叉树的存储结构，叫做**线索链表**，其中指向节点前驱和后继的指针，叫做**线索**。加上线索的二叉树称之为```线索二叉树```(Threaded Binary Tree)。例如： 下图(a)所示为中序线索二叉树，与其对应的中序线索链表如图(b)所示。其中，实线为指针（指向左、右子树），虚线为线索（指向前驱和后继）。对二叉树以某种次序遍历使其变为线索二叉树的过程叫做**线索化**。

![ds-inorder-thread](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_inorder_thread.jpg)


在线索树上进行遍历，只要先找到序列中的第一个节点，然后依次找到节点后继直至其后继为空时而止。如何在线索树中找到节点后继？ 以上图中的```中序线索树```为例来看，树中所有叶子节点的右链是线索，则右链域直接指示了节点的后继，如```节点b```的后继为```节点*```。树中所有非终端节点的右链均为指针，则无法由此得到后继的信息。然而，根据中序遍历的规律可知，节点的后继应是遍历其右子树时访问的第一个节点，即右子树中最左下的节点。例如，在找```节点*```的后继时，首先沿右指针找到其右子树的根```节点-```，然后顺其左指针往下直至其左标志为1的节点，即为```节点*```的后继，在图中是节点c。反之，在中序线索树中找前驱的的规律树：若其左标志为```1```，则左链为线索，指示其前驱，否则遍历其左子树时最后访问的一个节点（左子树中最右下的节点）为其前驱。


在后序线索树中找节点的后继较复杂，可分为3种情况：1） 若节点x是二叉树的根，则其后继为空； 2）若节点x是其双亲节点的右孩子或是其双亲节点的左孩子且其双亲没有右子树，则其双亲为后继节点； 3） 若节点x是其双亲的左孩子，且其双亲有右子树，则其后继为双亲的右子树上按后序遍历列出的第一个节点。例如下图所示为后序后继线索二叉树，节点B的后继为节点C，节点C的后继为节点D，节点F的后继为节点G，而节点D的后继为节点E。可见，在后序线索化树上找后继时需要知道节点双亲，即需带标志域的三叉链表作存储结构。

![ds-postorder-thread](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_postorder_thread.jpg)

可见，在中序线索二叉树上遍历二叉树，虽则时间复杂度亦为```O(n)```，但常数因子要比一般普通的二叉树遍历算法小，且不需要设栈。因此，若在某程序中所用二叉树需经常遍历或查找节点在遍历所得线性序列中的前驱和后继，则应采用线索链表作存储结构。

## 2.1 线索二叉树的实现

**1) 线索二叉树数据结构**

如下是二叉树的二叉线索存储表示：
{% highlight string %}
typedef enum PointerTag{Link, Thread};


typedef struct BiThrNode{
	TElemType data;
	struct BiThrNode *lchild, *rchild;   //左右孩子指针
	PointerTag LTag, rTag;
}BiThrNode, *BiThrTree;
{% endhighlight %}

**2) 中序遍历线索二叉树**

为方便起见，仿照线性表的存储结构，在二叉树的线索链表上也添加一个头节点，并令其lchild域的指针指向二叉树的根节点，其rchild域的指针指向中序遍历时访问的最后一个节点；反之，令二叉树中序序列中的第一个节点的lchild域指针和最后一个节点rchild域的指针均指向头节点。这好比为二叉树建立了一个双向线索链表，既可以从第一个节点起顺后继进行遍历，也可以从最后一个节点起顺前驱进行遍历。（请参看上图```中序线索链表```)。如下是以双向线索链表为存储结构时对二叉树进行遍历的算法：
{% highlight string %}
//T 指向头节点，头节点的左链lchild指向根节点，可参见后面的线索化算法
//中序遍历二叉线索树T的非递归算法，对每个数据元素调用函数Visit()
Status InOrderTraverse_Thr(BiThrTree T, Status (*Visit)(TElemType e))
{
	p = T->lchild;

	while(p != T)
	{
		while(p->LTag == Link)
			p = p->lchild;

		if(!Visit(p->data))
			return ERROR;

		while(p->RTag == Thread &&  p->rchild != T)   
		{
			p = p->rchild;
			Visit(p->data);
		}

		p = p->rchild;
	}

	return OK;
}
{% endhighlight %}


**3） 二叉树的线索化**

下面我们介绍一下二叉树的线索化。由于线索化二叉树的实质是将二叉链表中的空指针改为指向前驱或后继的线索，而前驱或后继的信息只有在遍历时才能得到，因此线索化的过程即为在遍历的过程中修改空指针的过程。为了记录下遍历过程中访问节点的先后关系，附设一个指针```pre```始终指向刚刚访问过的节点，若```指针p```指向当前访问的节点，则pre指向它的前驱。由此我们可得中序遍历建立线索化链表的算法：
{% highlight string %}
//中序遍历二叉树T，并将其中序线索化，Thrt指向头节点
Status InorderThreading(BiThrTree *Thrt, BiThrTree T)
{
	(*Thrt) = (BiThrTree)malloc(sizeof(BiThrNode));
	if(!(*Thrt))
		exit(OVERFLOW);

	Thrt->LTag = Link;
	Thrt->RTag = Thread;    //建立头结点
	
	Thrt->rchild = Thrt;    //右指针回指
	if(!T)
	{
		Thrt->lchild = Thrt;			//若二叉树为空，则左指针回指
	}
	else{
		Thrt->lchild = T;
		pre = *Thrt;

		InThreading(T, &pre);

		pre->rchild = Thrt;         //最后一个节点线索化
		pre->RTag = Thread;
		Thrt->lchild = pre;
	}

	return OK;
}

void InThreading(BiThrTree p, BiThrTree *pre)
{
	if(p)
	{
		InThreading(p->lchild, pre);			//左子树线索化
		if(!p->lchild)					//前驱线索
		{
			p->LTag = Thread;
			p->lchild = (*pre);
		}
		if(!(*pre)->rchild)					//后继线索
		{
			(*pre)->RTag = Thread;
			(*pre)->rchild =  p;
		}
		(*pre) = p;                        //保持pre指向p的前驱

		InThreading(p->rchild, pre);			//右子树线索化
	}
}
{% endhighlight %}

<br />
<br />


