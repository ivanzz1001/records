---
layout: post
title: 红黑树的实现示例(二)
tags:
- data-structure
categories: data-structure
description: 红黑树的实现示例
---

本章主要给出一个红黑树的实现示例。(转自[红黑树(一)之 原理和算法详细介绍](http://www.cnblogs.com/skywang12345/p/3245399.html))


<!-- more -->

## 1. 相应头文件

头文件```rbtree.h```:
{% highlight string %}
#ifndef __RBTREE_H_
#define __RBTREE_H_


#define RED		0x0
#define BLACK	0x1

typedef int Type;


//define red-black tree node
typedef struct RBTreeNode{
	unsigned int color;		//RED or BLACK
	Type key;
	struct RBTreeNode *left;
	struct RBTreeNode *right;
	struct RBTreeNode *parent;
}Node, *RBTree;




typedef struct rb_root{
	Node *node;
}RBRoot;




//Create red-black tree
RBRoot *create_rbtree();


void destroy_rbtree(RBRoot **root);

//insert node to rbtree
int insert_rbtree(RBRoot *root, Type key);


//remove node from rbtree
void delete_rbtree(RBRoot *root, Type key);


void preorder_rbtree(RBRoot *root);

void inorder_rbtree(RBRoot *root);


void postorder_rbtree(RBRoot *root);


//This is the 'Recursive' version. if found key in root, then returns 0; else returns -1
int rbtree_search(RBRoot *root, Type key);



//This is the 'UnRecursive' version. If found key in root, then returns 0; else returns -1
int iterative_rbtree_search(RBRoot *root, Type key);



//find the minimum value. If found returns 0; else returns -1
int rbtree_minimum(RBRoot *root, int *value);



//find the maximum value. If found returns 0; else return -1
int rbtree_maximum(RBRoot *root, int *value);



//print the red-black tree
void print_rbtree(RBRoot *root);



#endif
{% endhighlight %}


## 2. 相应源文件

如下是源文件```rbtree.c```:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include "rbtree.h"



#define rb_parent(r)		((r)->parent)
#define rb_color(r)			((r)->color)
#define rb_is_red(r)		((r)->color==RED)
#define rb_is_black(r)		((r)->color==BLACK)
#define rb_set_black(r)		do{(r)->color = BLACK;}while(0)
#define rb_set_red(r)		do{(r)->color = RED;}while(0)
#define rb_set_parent(r, p) do{(r)->parent = (p);}while(0)
#define rb_set_color(r, c)	do{(r)->color = (c);}while(0)


RBRoot *create_rbtree()
{
	RBRoot *root = (RBRoot *)malloc(sizeof(RBRoot));
	if(root)
		root->node = NULL;
	return root;
}

static void preorder(RBTree tree)
{
	if(tree)
	{
		printf("%d ", tree->key);
		preorder(tree->left);
		preorder(tree->right);
	}
}

void preorder_rbtree(RBRoot *root)
{
	if(root)
		preorder(root->node);
}


static void inorder(RBTree tree)
{
	if(tree)
	{
		inorder(tree->left);
		printf("%d ", tree->key);
		inorder(tree->right);
	}
}


void inorder_rbtree(RBRoot *root)
{
	if(root)
		inorder(root->node);
}

static void postorder(RBTree tree)
{
	if(tree)
	{
		postorder(tree->left);
		postorder(tree->right);
		printf("%d ", tree->key);
	}
}

void postorder_rbtree(RBRoot *root)
{
	if(root)
		postorder(root->node);
}

static Node *search(RBTree node, Type key)
{
	if(!node || node->key == key)
		return node;
	else if(key < node->key)
		return search(node->left, key);
	else
		return search(node->right, key);
}


int rbtree_search(RBRoot *root, Type key)
{
	if(root)
		return search(root->node, key) ? 0 : -1;
}


static Node *iterative_search(RBTree node, Type key)
{
	while(node && node->key != key)
	{
		if(key < node->key)
			node = node->left;
		else
			node = node->right;
	}
	return node;
}

int iterative_rbtree_search(RBRoot * root,Type key)
{
	if(root)
		return iterative_search(root->node,key) ? 0 : -1;
}


//find the minimum node
static Node *minimum(RBTree tree)
{
	if(!tree)
		return NULL;

	while(tree->left)
		tree = tree->left;
	return tree;
}

int rbtree_minimum(RBRoot *root, int *value)
{
	Node *node = NULL;

	if(root)
		node = minimum(root->node);

	if(!node)
		return -1;

	*value = node->key;
	return 0x0;
}

//find the maximum
static Node *maximum(RBTree tree)
{
	if(!tree)
		return NULL;

	while(tree->right)
		tree = tree->right;
	return tree;
}

int rbtree_maximum(RBRoot *root, int *value)
{
	Node *node = NULL;

	if(root)
		node = maximum(root->node);

	if(!node)
		return -1;

	*value = node->key;
	return 0x0;
}


//find successor node
static Node *rbtree_successor(RBTree node)
{
	//1) if node has right child
	if(node->right)
		return minimum(node->right);

	//2) if node doesn't have right child, then there are the following two cases:
	//   2.1) node is a 'left-child', so the successor is its parent
	//   2.2) node is a 'right-child', then find the node's  'lowest parent', and the 'lowest parent' should have 'left-child',
	//         the founded 'lowest parent' is the node's successor

	Node *p = node->parent;
	while(p && (node == p->right))
	{
		node = p;
		p = p->parent;
	}

	return p;
}


static Node *rbtree_predecessor(RBTree node)
{
	//1) If node has left child
	if(node->left)
		return maximum(node->left);

	//2) if node doesn't left child, then there are the following two cases:
	//  2.1) node is the 'right child', so the predecessor is its parent;
	//  2.2) node is the 'left child', so find the 'lowest parent', and the 'lowest parent' should have 'right child',
	//         the founded 'lowest parent' is the node's predecessor

	Node *p = node->parent;
	while(p && (node == p->left))
	{
		node = p;
		p = p->parent;
	}

	return p;
}



static void rbtree_left_rotate(RBRoot *root, Node *node)
{
	Node *right = node->right;

	node->right = right->left;
	if(right->left)
	{
		right->left->parent = node;
	}

	right->parent = node->parent;

	if(node->parent)
	{
		if(node->parent->left == node)
			node->parent->left = right;
		else
			node->parent->right = right;
	}
	else{
		root->node = right;
	}

	right->left = node;
	node->parent = right;
}

static void rbtree_right_rotate(RBRoot *root, Node *node)
{
	Node *left = node->left;

	node->left = left->right;
	if(left->right)
	{
		left->right->parent = node;
	}

	left->parent = node->parent;
	if(node->parent)
	{
		if(node->parent->left == node)
			node->parent->left = left;
		else
			node->parent->right = left;
	}
	else{
		root->node = left;
	}

	left->right = node;
	node->parent = left;
}


static void rbtree_insert_fixup(RBRoot *root, Node *node)
{
	Node *parent, *grand_parent;

	while((parent = rb_parent(node)) && rb_is_red(parent))
	{
		grand_parent = rb_parent(parent);

		if(parent == grand_parent->left)
		{
			Node *uncle = grand_parent->right;

			//Case 1: uncle is 'red'
			if(uncle && rb_is_red(uncle))
			{
				rb_set_black(uncle);
				rb_set_black(parent);
				rb_set_red(grand_parent);

				node = grand_parent;
				continue;
			}

			//Case 2: uncle is 'black', and node is a 'right' child
			if(parent->right == node)
			{
				rbtree_left_rotate(root, parent);

				Node *tmp;
				tmp = parent;
				parent = node;
				node = tmp;
			}

			//Case 3: uncle is 'black', and node is a 'left' child
			rb_set_black(parent);
			rb_set_red(grand_parent);
			rbtree_right_rotate(root, grand_parent);

		}
		else{
			Node *uncle = grand_parent->left;

			//Case 1: uncle is 'red'
			if(uncle && rb_is_red(uncle))
			{
				rb_set_black(uncle);
				rb_set_black(parent);
				rb_set_red(grand_parent);

				node = grand_parent;
				continue;
			}

			//Case 2: uncle is 'black', and node is a 'left' child
			if(parent->left == node)
			{
				rbtree_right_rotate(root, parent);

				Node *tmp;
				tmp = parent;
				parent = node;
				node = tmp;
			}

			//Case 3: uncle is 'black', and node is a 'right' child
			rb_set_black(parent);
			rb_set_red(grand_parent);
			rbtree_left_rotate(root, grand_parent);
		}
	}

	rb_set_black(root->node);
}



static void rbtree_insert(RBRoot *root, Node *node)
{
	Node *p = root->node;
	Node *q = NULL;

	while(p)
	{
		q = p;

		//Here not test p->key == node->key

		if(node->key < p->key)
			p = p->left;
		else
			p = p->right;
	}

	rb_parent(node) = q;

	if(q)
	{
		if(node->key < q->key)
			q->left = node;
		else
			q->right = node;
	}
	else{
		root->node = node;
	}

	node->color = RED;

	rbtree_insert_fixup(root, node);

}



static Node *create_rbtree_node(Type key, Node *parent, Node *left, Node *right)
{
	Node *node = (Node *)malloc(sizeof(Node));
	if(!node)
		return NULL;

	node->key = key;
	node->left = left;
	node->right = right;

	node->color = BLACK;     //Default 'BLACK'

	return node;
}


int insert_rbtree(RBRoot *root, Type key)
{
	Node *node = NULL;

	if(search(root->node,key) != NULL)
		return -1;

	if((node = create_rbtree_node(key, NULL, NULL, NULL)) == NULL)
		return -1;

	rbtree_insert(root, node);
	return 0x0;
}


static void rbtree_delete_fixup(RBRoot *root, Node *node, Node *parent)
{
	Node *brother;

	while((!node || rb_is_black(node)) && node != root->node)
	{
		if(parent->left == node)
		{
			brother = parent->right;

			if(rb_is_red(brother))
			{
				//Case 1: brother is 'red'
				rb_set_black(brother);
				rb_set_red(parent);
				rbtree_left_rotate(root, parent);
				brother = parent->right;
			}

			if((!brother->left || rb_is_black(brother->left)) &&
				(!brother->right || rb_is_black(brother->right)))
			{
				//Case 2: brother is 'black', and brother's two children are color-black
				rb_set_red(brother);
				node = parent;
				parent = rb_parent(node);
			}
			else{
				if(!brother->right || rb_is_black(brother->right))
				{
					//Case 3: brother is 'black', and brother's left child is color-red, brother's right child is color-black
					rb_set_black(brother->left);
					rb_set_red(brother);
					rbtree_right_rotate(root, brother);
					brother = parent->right;
				}

				//Case 4: brother is 'black' , and brother's right child is color-red, brother's left child is any-color
				rb_set_color(brother, rb_color(parent));
				rb_set_black(parent);
				rb_set_black(brother->right);
				rbtree_left_rotate(root, parent);
				node = root->node;
				break;
			}

		}
		else{
			brother = parent->left;

			if(rb_is_red(brother))
			{
				//Case 1: brother is 'red'
				rb_set_black(brother);
				rb_set_red(parent);
				rbtree_right_rotate(root, parent);
				brother = parent->left;
			}

			if((!brother->left || rb_is_black(brother->left)) &&
				(!brother->right || rb_is_black(brother->right)))
			{
				//Case 2: brother is 'black', and brother's two children is color-black
				rb_set_red(brother);
				node = parent;
				parent = rb_parent(node);
			}
			else{
				if(!brother->left || rb_is_black(brother->left))
				{
					//Case 3: brother is 'black', and brother's left child is color-black, brother's right child is color-red
					rb_set_black(brother->right);
					rb_set_red(brother);
					rbtree_left_rotate(root, brother);
					brother = parent->left;
				}

				//Case 4: brother is 'black', and brother's left child is color-red, brother's right child is any-color
				rb_set_color(brother, rb_color(parent));
				rb_set_black(parent);
				rb_set_black(brother->left);
				rbtree_right_rotate(root,parent);
				node = root->node;
				break;
			}
		}
	}

	if(node)
		rb_set_black(node);
}


static void rbtree_delete(RBRoot *root, Node *node)
{
	Node *child;
	Node *parent;
	int color;

	if(node->left && node->right)
	{
		//find its's successor node
		Node *replace = node->right;
		while(replace->left)
			replace = replace->left;

		if(rb_parent(node))
		{
			if(rb_parent(node)->left == node)
				rb_parent(node)->left = replace;
			else
				rb_parent(node)->right = replace;
		}
		else{
			//node is the root
			root->node = replace;
		}

		// 'child' is node-replace's right child
		child = replace->right;
		parent = rb_parent(replace);
		color = replace->color;		//save the color

		if(parent == node)
		{
			parent = replace;
		}
		else{
			//child is NOT NULL
			if(child)
				rb_set_parent(child, parent);

			parent->left = child;
			replace->right = node->right;
			rb_set_parent(node->right, replace);
		}


		replace->parent = node->parent;
		replace->color = node->color;
		replace->left = node->left;
		node->left->parent = replace;

		if(color == BLACK)
			rbtree_delete_fixup(root, child, parent);

		free(node);
		return;
	}

	if(node->left)
		child = node->left;
	else
		child = node->right;

	parent = node->parent;
	color = node->color;		//save the color

	if(child)
		child->parent = parent;

	if(parent)
	{
		if(parent->left == node)
			parent->left = child;
		else
			parent->right = child;
	}
	else{
		root->node = child;
	}

	if(color == BLACK)
		rbtree_delete_fixup(root, child, parent);

	free(node);
}


void delete_rbtree(RBRoot *root, Type key)
{
	Node *node;

	if((node = search(root->node, key)) != NULL)
	{
		rbtree_delete(root, node);
		return;
	}
}

static void rbtree_destroy(RBTree tree)
{
	if(!tree)
		return;


	rbtree_destroy(tree->left);
	rbtree_destroy(tree->right);
	free(tree);

}

void destroy_rbtree(RBRoot **root)
{
	if(!*root)
		return;

	rbtree_destroy((*root)->node);
	(*root)->node = NULL;

	free(*root);
	*root = NULL;
}



static void rbtree_print(RBTree tree, Type key, int direction)
{
	if(tree != NULL)
	{
		if(direction == 0)		//tree is 'root' node
			printf("%2d(B) is root\n", tree->key);
		else
			printf("%2d(%s) is %2d's %6s child\n", tree->key, rb_is_red(tree) ? "R" : "B", key, direction == 1 ? "right" : "left");

		rbtree_print(tree->left,tree->key, -1);
		rbtree_print(tree->right, tree->key, 1);
	}
}


void print_rbtree(RBRoot *root)
{
	if(root && root->node)
		rbtree_print(root->node, root->node->key, 0x0);
}
{% endhighlight %}


## 3. 测试源文件

如下是测试源文件```main.c```:
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rbtree.h"


#define CHECK_INSERT	0x0
#define CHECK_DELETE	0x0

#define LENGTH(a)		((sizeof(a)/sizeof(a[0])))


int main(int argc,char *argv[])
{
	int a[] = {10,40, 30, 60, 90, 70, 20, 50, 80};
	int i, length;
	RBRoot *root = NULL;

	length = LENGTH(a);

	if((root = create_rbtree()) == NULL)
	{
		printf("memory allocation failure\n");
		return -1;
	}

	printf("== origin data:");
	for(i = 0;i<length;i++)
		printf("%d ", a[i]);
	printf("\n");


	for(i = 0;i<length; i++)
	{
		insert_rbtree(root, a[i]);

		#if CHECK_INSERT
			printf("== add node: %d\n", a[i]);
			printf("== rbtree detail:\n");
			print_rbtree(root);
			printf("\n");
		#endif
	}

	printf("== preorder:");
	preorder_rbtree(root);

	printf("\n== inorder:");
	inorder_rbtree(root);


	printf("\n== postorder: ");
    postorder_rbtree(root);
    printf("\n");


	if(rbtree_minimum(root, &i) == 0)
		printf("== minimum value: %d\n", i);

	if(rbtree_maximum(root, &i) == 0)
		printf("== maximum value: %d\n", i);

	printf("== rbtree detail: \n");
	print_rbtree(root);
	printf("\n");



	#if CHECK_DELETE
		for(i = 0;i<length;i++)
		{
			delete_rbtree(root, a[i]);
			printf("== remove node: %d\n", a[i]);

			if(root)
			{
				printf("== rbtree detail:\n");
				print_rbtree(root);
				printf("\n");
			}
		}
	#endif

	destroy_rbtree(&root);

	return 0x0;

}
{% endhighlight %}
编译运行：
<pre>
# gcc -o rbtree *.c
# ./rbtree
== origin data:10 40 30 60 90 70 20 50 80 
== preorder:30 10 20 60 40 50 80 70 90 
== inorder:10 20 30 40 50 60 70 80 90 
== postorder: 20 10 50 40 70 90 80 60 30 
== minimum value: 10
== maximum value: 90
== rbtree detail: 
30(B) is root
10(B) is 30's   left child
20(R) is 10's  right child
60(R) is 30's  right child
40(B) is 60's   left child
50(R) is 40's  right child
80(B) is 60's  right child
70(R) is 80's   left child
90(R) is 80's  right child
</pre>



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


