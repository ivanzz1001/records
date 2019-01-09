---
layout: post
title: 线索二叉树
tags:
- data-structure
categories: data-structure
description: 线索二叉树
---

本章我们主要介绍一下线索二叉树。


<!-- more -->


## 1. 线索二叉树
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

## 2. 线索二叉树的实现

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

		InThreading(T， &pre);

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
		InThreading(p->lchild);			//左子树线索化
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

		InThreading(p->rchild);			//右子树线索化
	}
}
{% endhighlight %}

<br />
<br />


