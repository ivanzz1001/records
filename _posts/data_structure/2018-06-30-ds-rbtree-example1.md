---
layout: post
title: 红黑树的实现示例(一)
tags:
- data-structure
categories: data-structure
description: 红黑树的实现示例
---

本章主要给出一个红黑树的实现示例。


<!-- more -->


## 1. 相应头文件

如下是红黑树实现的相应头文件```rb_tree.h```:
{% highlight string %}
#ifndef __RB_TREE_H_
#define __RB_TREE_H_
#include <stdio.h>
#include <stdlib.h>


#define COLOR_RED  0x0
#define COLOR_BLACK 0x1

typedef struct RBNode{
   int key;
   unsigned char color;
   struct RBNode *left;
   struct RBNode *right;
   struct RBNode *parent;
}rb_node_t, *rb_tree_t;


int insert_rbtree(rb_tree_t *root, rb_node_t *node);

int delete_rbtree(rb_tree_t *root, int key);



void inorder_rbtree(rb_tree_t root);



#endif
{% endhighlight %}

## 2. 相应源文件
{% highlight string %}
#include "rb_tree.h"



static rb_node_t *rb_rotate_right(rb_tree_t *root, rb_node_t *node)
{
    rb_node_t *left = node->left;

    if(node->left = left->right)
    {
       left->right->parent = node;
    }

    left->right = node;
   
    if(left->parent = node->parent)
    {
       if(node->parent->left == node)
       {
          node->parent->left = left;
       }
       else{
          node->parent->right = left;
       }
    }
    else{
       *root = left;
    }
   
    node->parent = left;
    return left;
}

static rb_node_t *rb_rotate_left(rb_tree_t *root, rb_node_t *node)
{
    rb_node_t *right = node->right;

    if(node->right = right->left)
    {
       right->left->parent = node;
    }
    
    right->left = node;
    
    if(right->parent = node->parent)
    {
        if(node->parent->left == node)
        {
             node->parent->left = right;
        }
        else{
             node->parent->right = right;
        }
    }
    else{
       *root = right;
    }

    node->parent = right;
    return right;
}


int rb_insert_fixup(rb_tree_t *root, rb_node_t *node)
{
	rb_node_t *parent;
	rb_node_t *grand_parent;

	//If parent exist, and the color of parent is RED
	while((parent = node->parent) && parent->color == COLOR_RED)
	{
		grand_parent = parent->parent;

		 //parent node is grand_parent node's left child(grand_parent should not be NULL, because parent->color==COLOR_RED)
		 if(grand_parent->left == parent)
		 {
			 rb_node_t *uncle = grand_parent->right;

			 //Case 1: uncle is RED
			 if(uncle && uncle->color == COLOR_RED)
			 {
				 parent->color = COLOR_BLACK;
				 uncle->color = COLOR_BLACK;
				 grand_parent->color = COLOR_RED;

				 node = grand_parent;
				 continue;
			 }

			 //Case 2: uncle is BLACK, and node is parent's right child
			 if(parent->right == node)
			 {
				 rb_rotate_left(root, parent);

				 // reset parent and node pointer
				 rb_node_t *tmp;
				 tmp = parent;
				 parent = node;
				 node = tmp;

				 //Here successful convert Case 2 to Case3
			 }

			 //Case 3: uncle is BLACK, and node is parent's left child
			 parent->color = COLOR_BLACK;
			 grand_parent->color = COLOR_RED;
			 rb_rotate_right(root, grand_parent);

		 }
		 else{
		 	rb_node_t *uncle = grand_parent->left;

			//Case 1: uncle is RED
			if(uncle && uncle->color == COLOR_RED)
			{
				parent->color = COLOR_BLACK;
				uncle->color = COLOR_BLACK;
				grand_parent->color = COLOR_RED;

				node = grand_parent;
				continue;
			}

			//Case 2: uncle is BLACK, and node is parent's left child
			if(parent->left == node)
			{
				 rb_rotate_right(root,parent);

				 //reset parent and node pointer
				 rb_node_t *tmp;
				 tmp = parent;
				 parent = node;
				 node = tmp;

				 //Here success convert Case 2 to Case 3
			}

			//Case 3: uncle is BLACK, and node is parent's right child
			parent->color = COLOR_BLACK;
			grand_parent->color = COLOR_RED;
			rb_rotate_left(root, grand_parent);
		 }
	}

	(*root)->color = COLOR_BLACK;
	return 0x0;
}


int insert_rbtree(rb_tree_t *root, rb_node_t *node)
{
	rb_node_t *p = *root;
	rb_node_t *q = NULL;

	//find the position we need to insert
	while(p)
	{
		q = p;
		if(p->key == node->key)
			return 1;
		else if(p->key > node->key)
			p = p->left;
		else
			p = p->right;
	}

	node->parent = q;

	if(q != NULL)
	{
		if(node->key < q->key)
			q->left = node;
		else
			q->right = node;
	}
	else{
		*root = node;
	}

	node->color = COLOR_RED;

	return rb_insert_fixup(root, node);
}


static int rbtree_delete_fixup(rb_tree_t *root, rb_node_t *node, rb_node_t *parent)
{
	rb_node_t *brother = NULL;

	while((!node || node->color == COLOR_BLACK) && node != *root)
	{
		if(parent->left == node)
		{
			//The left branch


			//Note: brother can't be NULL, because we have delete a black node, 
			//and the tree is balanced before we delete the node
			brother = parent->right;

			if(brother->color == COLOR_RED)
			{
			    //1) Case 1: x's brother is COLOR_RED
				brother->color = COLOR_BLACK;
				parent->color = COLOR_RED;
				rb_rotate_left(root,node);
				brother = parent->right;
			}


			if((!brother->left || brother->left->color == COLOR_BLACK) &&
				(!brother->right || brother->right->color == COLOR_BLACK))
			{
			   //2) Case 2: x's brother is COLOR_BLACK, is its two child is NULL or COLOR_BLACK
			   brother->color = COLOR_RED;
			   node = parent;
			   parent = node->parent;
			}
			else{
				if(!brother->right || brother->right->color == COLOR_BLACK)
				{
				   //3) Case 3: x's brother is COLOR_BLACK, and brother left child is COLOR_RED, 
				   // right child is COLOR_BLACK

	
				   brother->left->color = COLOR_BLACK;
				   brother->color = COLOR_RED;
				   rb_rotate_right(root,brother);
				   brother = parent->right;
				}

				//4) Case 4: x's brother is COLOR_BLACK, and brother's right child is COLOR_RED
				brother->color = parent->color;
				parent->color = COLOR_BLACK;
				brother->right->color = COLOR_BLACK;
				rb_rotate_left(root, parent);

				node = *root;
				break;
	
			}
			
			
		}
		else{
			//The right branch
			brother = parent->left;


			if(brother->color == COLOR_RED)
			{
			    //1) Case 1: x's brother is COLOR_RED
				brother->color = COLOR_BLACK;
				parent->color = COLOR_RED;
				rb_rotate_right(root,parent);
				brother = parent->left;
			}

			if((!brother->left || brother->left->color == COLOR_BLACK) &&
				(!brother->right || brother->right->color == COLOR_BLACK))
			{
			   //2) Case 2: x's brother is COLOR_BLACK, is its two child is NULL or COLOR_BLACK
			   brother->color = COLOR_RED;
			   node = parent;
			   parent = node->parent;
			}
			else{
				if(!brother->left || brother->left->color == COLOR_BLACK)
				{
				   //3) Case 3: x's brother is COLOR_BLACK, and brother right child is COLOR_RED, 
				   // left child is COLOR_BLACK


				   brother->right->color = COLOR_BLACK;
				   brother->color = COLOR_RED;
				   rb_rotate_left(root,brother);
				   brother = parent->left;
				}

				//4) Case 4: x's brother is COLOR_BLACK, and brother's left child is COLOR_RED
				brother->color = parent->color;
				parent->color = COLOR_BLACK;
				brother->left->color = COLOR_BLACK;
				rb_rotate_right(root, parent);

				node = *root;
				break;
			}
		}
	}


	if(node)
	{
	    node->color = COLOR_BLACK;
	}
	return 0x0;

}

int delete_rbtree(rb_tree_t *root, int key)
{
	rb_node_t *p = *root;

	//find the node
	while(p)
	{
		if(p->key == key)
		 	break;
		else if(p->key > key)
		 	p = p->left;
		else
		 	p = p->right;
	}

	if(!p)
		return -1;


	if(p->left && p->right)
	{
		//get Successor node
		rb_node_t *successor = p->right;

		while(successor->left)
			successor = successor->left;

		if(p->parent)
		{
			if(p->parent->left == p)
				p->parent->left = successor;
			else
				p->parent->right = successor;
		}
		else{
			  *root = successor;
		}

		 rb_node_t *successor_child = successor->right;
		 rb_node_t *successor_parent = successor->parent;
		 int color = successor->color;      //save the color

		 if(successor_parent == p)
		 {
		     successor_parent = successor;
		 }
		 else{
		 	if(successor_child)
				successor_child->parent = successor_parent;

			successor_parent->left = successor_child;

			successor->right = p->right;
			p->right->parent = successor;
		 }

		 successor->parent = p->parent;
		 successor->color = p->color;
		 successor->left = p->left;
		 p->left->parent = successor;

		 if(color == COLOR_BLACK)
		 	rbtree_delete_fixup(root, successor_child, successor_parent);

		 free(p);
		 return 0x0;

	}


	rb_node_t *child = NULL;
	rb_node_t *parent = NULL;
	int color;

	if(p->left)
		child = p->left;
	else
		child = p->right;


	parent = p->parent;
	color = p->color;   //save the color

	if(child)
		child->parent = parent;

	if(parent)
	{
	    if(parent->left == p)
			parent->left = child;
		else
			parent->right = child;
	}
	else{
		*root = child;
	}


	if(color == COLOR_BLACK)
		rbtree_delete_fixup(root, child, parent);

	free(p);

	return 0x0;
}


void inorder_rbtree(rb_tree_t root)
{
	if(!root)
		return;

	inorder_rbtree(root->left);
	printf("%d(%s) ", root->key, root->color == COLOR_RED ? "RED" : "BLACK");
	inorder_rbtree(root->right);
}

{% endhighlight %}



## 3. 测试源文件
{% highlight string %}
#include "rb_tree.h"
#include <string.h>



static rb_node_t *makenode(int key)
{
    rb_node_t *node = (rb_node_t *)malloc(sizeof(rb_node_t));
	if(!node)
		return NULL;

	memset(node, 0x0, sizeof(rb_node_t));
	node->key = key;
}

int main(int argc,char *argv[])
{
	int a[] = {1,2,3,4,5,10,9,8,7,6};
	int i;

	rb_tree_t root = NULL;

	for(i = 0; i< sizeof(a)/sizeof(int); i++)
	{
		rb_node_t *node = makenode(a[i]);
		if(!node)
		{
			printf("memory allocation failure\n");
			return -1;
		}

		if(insert_rbtree(&root, node) < 0)
		{
			printf("rbtree insert failure\n");
			return -2;
		}
	}

	inorder_rbtree(root);

	printf("\n");

	delete_rbtree(&root,5);
	delete_rbtree(&root,8);
	inorder_rbtree(root);
	printf("\n");

	return 0x0;

}
{% endhighlight %}
编译运行：
<pre>
# gcc -o rb_tree *.c
# ./rb_tree 
1(BLACK) 2(BLACK) 3(BLACK) 4(BLACK) 5(BLACK) 6(RED) 7(RED) 8(BLACK) 9(BLACK) 10(BLACK) 
1(BLACK) 2(BLACK) 3(BLACK) 4(BLACK) 6(RED) 7(BLACK) 9(BLACK) 10(BLACK)
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


