#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"


struct TmpBNode{
	int key;
	struct bnode_t *child;
};

BTree * create_btree(int m)
{
	BTree *btree = (BTree *)malloc(sizeof(BTree));
	btree->m = m;
	btree->root = NULL;

	return btree;
}

static BNode *create_bnode(int m)
{
	BNode *bnode = (BNode *)malloc(sizeof(BNode));
	if(!bnode)
		return NULL;

	bnode->keys = (int *)malloc(sizeof(int) * m);
	bnode->children = (BNode **)malloc(sizeof(BNode *) * m);
	if(!bnode->keys || !bnode->children)
	{
		free(bnode->keys);
		free(bnode->children);
		free(bnode);
		return NULL;
	}

	bnode->key_number = 0;
	bnode->parent = NULL;
	memset(bnode->keys, 0x0, sizeof(int) * m);
	memset(bnode->children, 0x0, sizeof(BNode *)*m);
	return bnode;

}

static int insert_combine_node(BTree *btree, BNode *node, struct TmpBNode *tmpNode)
{
	int i,j, insert_pos, middle;
	int right_cnt;
	int flag;

	if(node->key_number < btree->m-1)
	{

		//directly combine
		insert_pos = node->key_number + 1;

		for(i = node->key_number; i>=1;i--)
		{
			if(tmpNode->key > node->keys[i])
				break;
			else{
				node->keys[i+1] = node->keys[i];
				node->children[i+1] = node->children[i];
				insert_pos--;
			}
		}

		node->keys[insert_pos] = tmpNode->key;
		node->children[insert_pos] = tmpNode->child;
		if(tmpNode->child)
			tmpNode->child->parent = node;

		node->key_number = node->key_number + 1;
		return 0x0;

	}

	BNode *newNode = create_bnode(btree->m);
	if(!newNode)
		return -1;



	middle = (node->key_number + 1)/2;
	right_cnt = node->key_number + 1 - middle;
	newNode->key_number = right_cnt;


	flag = 0;
	i = node->key_number;
	while(right_cnt)
	{
		if(!flag && tmpNode->key > node->keys[i])
		{
			newNode->keys[right_cnt] = tmpNode->key;
			newNode->children[right_cnt] = tmpNode->child;
			if(tmpNode->child)
				tmpNode->child->parent = newNode;

			flag = 1;
			right_cnt--;
		}
		else{
			newNode->keys[right_cnt] = node->keys[i];
			newNode->children[right_cnt] = node->children[i];
			right_cnt--;
			i--;
		}
	}

	if(!flag)
	{
		insert_pos = i+1;

		for(i; i>=1;i--)
		{
			if(tmpNode->key > node->keys[i])
				break;
			else{
				node->keys[i+1] = node->keys[i];
				node->children[i+1] = node->children[i];
				insert_pos--;
			}
		}

		node->keys[insert_pos] = tmpNode->key;
		node->children[insert_pos] = tmpNode->child;
		if(tmpNode->child)
			tmpNode->child->parent = node;

	}

	newNode->children[0] = node->children[middle];
	tmpNode->key = node->keys[middle];
	tmpNode->child = newNode;
	node->key_number = middle-1;

	if(node->parent)
	{
		return insert_combine_node(btree, node->parent, tmpNode);
	}
	else{
		BNode * root = create_bnode(btree->m);
		if(!root)
			return -1;
		root->parent = NULL;
		root->key_number = 1;
		root->keys[1] = tmpNode->key;
		root->children[0] = node;
		root->children[1] = tmpNode->child;
		if(tmpNode->child)
			tmpNode->child->parent = root;

		btree->root = root;

		return 0x0;
	}
}

int insert_combine_node2(BTree *btree, BNode *node, struct TmpBNode *tmpNode)
{
	int i,j;
	if(node->key_number < btree->m-1)
	{
		//directly combine
		for(i = node->key_number + 1; i >= 2; i--)
		{
			if(tmpNode->key > node->keys[i-1])
				break;
			node->keys[i] = node->keys[i-1];
			node->children[i] = node->children[i-1];
		}

		if(i == 1)
		{
			if(tmpNode->child)
			{
				// combine from lower recusive
				node->keys[i] = tmpNode->key;
				node->children[i] = tmpNode->child;
				tmpNode->child->parent = node;
			}
			else{
				node->keys[i] = tmpNode->key;
				node->children[i] = node->children[0];
				node->children[0] = tmpNode->child;

			}
		}
		else{
			node->keys[i] = tmpNode->key;
			node->children[i] = tmpNode->child;
			if(tmpNode->child)
				tmpNode->child->parent = node;
		}

		node->key_number = node->key_number + 1;
		return 0x0;
	}


	BNode *newNode = create_bnode(btree->m);
	if(!newNode)
		return -1;



	//find the insert position

	int left, right, middle, insert_pos;

    if(tmpNode->key > node->keys[node->key_number])
    {
		insert_pos = node->key_number + 1;
    }
	else{
		left = 1;
		right = node->key_number;

		while(left < right)
		{
			middle = (left + right) >> 1;

			if(node->keys[middle] > tmpNode->key)
				right = middle -1;
			else
				left = middle + 1;
		}

		insert_pos = left;

	}

	middle = (1 + node->key_number + 1) >> 1;



	int right_cnt, flag;
	right_cnt =  node->key_number + 1 - middle;
	flag = 0x0;


	newNode->key_number = right_cnt;
	if(insert_pos == node->key_number + 1)
	{
		newNode->keys[right_cnt] = tmpNode->key;
		newNode->children[right_cnt] = tmpNode->child;
		if(tmpNode->child)
			tmpNode->child->parent = newNode;

		right_cnt--;
	}


	i = node->key_number;
	while(right_cnt)
	{
		if(i == insert_pos)
		{
			newNode->keys[right_cnt] = node->keys[i];
			newNode->children[right_cnt] = node->children[i];
			flag = 0x1;
			right_cnt--;

			if(right_cnt)
			{
				newNode->keys[right_cnt] = tmpNode->key;
				newNode->children[right_cnt] = tmpNode->child;
				if(tmpNode->child)
					tmpNode->child->parent = newNode;

				right_cnt--;
				i--;
			}
		}
		else{
			newNode->keys[right_cnt] = node->keys[i];
			newNode->children[right_cnt] = node->children[i];
			right_cnt--;
			i--;
		}

	}


	while(insert_pos <= i)
	{
		if(insert_pos == i)
		{
			if(flag == 0x0)
			{
				node->keys[i+1] = node->keys[i];
				node->children[i+1] = node->children[i];
			}

			if(insert_pos == 1)
			{
				if(tmpNode->child)
				{
					//combine from lower recusive
					node->keys[i] = tmpNode->key;
					node->children[i] = tmpNode->child;
					tmpNode->child->parent = node;
				}
				else{
					node->keys[i] = tmpNode->key;
					node->children[i] = node->children[0];
					node->children[0] = tmpNode->child;
				}

			}
			else{
				node->keys[i] = tmpNode->key;
				node->children[i] = tmpNode->child;
				if(tmpNode->child)
					tmpNode->child->parent = node;
			}

			i--;

		}
		else{
			node->keys[i+1] = node->keys[i];
			node->children[i+1] = node->children[i];
			i--;
		}
	}

	newNode->children[0] = node->children[middle];
	tmpNode->key = node->keys[middle];
	tmpNode->child = newNode;
	node->key_number = middle -1;

	if(node->parent)
	{
		return insert_combine_node2(btree, node->parent, tmpNode);
	}
	else{
		BNode * root = create_bnode(btree->m);
		if(!root)
			return -1;
		root->key_number = 1;
		root->keys[1] = tmpNode->key;
		root->children[0] = node;
		root->children[1] = newNode;
		root->parent = NULL;

		node->parent = root;
		newNode->parent = root;

		btree->root = root;
		return 0x0;
	}

}

int insert_btree(BTree *btree, int key)
{
	BNode *p = btree->root;
	BNode *q = NULL;
	int i;

	while(p)
	{
		for(i = 0;i<p->key_number;i++)
		{
			if(p->keys[i+1] == key)
				return -1;
			else if(p->keys[i+1] > key)
				break;

		}
		q = p;
		p = p->children[i];
	}
	if(!q)
	{
		btree->root = (BNode *)malloc(sizeof(BNode));
		if(!btree->root)
			return -1;

		BNode *newNode = create_bnode(btree->m);
		if(!newNode)
			return -1;

		newNode->key_number = 1;
		newNode->keys[1] = key;
		newNode->children[0] = newNode->children[1] = NULL;
		newNode->parent = NULL;
		btree->root = newNode;

		return 0x0;

	}

	struct TmpBNode tmpNode;
	tmpNode.key = key;
	tmpNode.child = NULL;

	return insert_combine_node(btree, q, &tmpNode);

}

static void inorder_tranverse(BNode *node)
{
	int i = 0;
	if(!node)
		return;

	for(i = 0;i<= node->key_number; i++)
	{
		inorder_tranverse(node->children[i]);

		if(i < node->key_number)
			printf("%d ", node->keys[i+1]);
	}


}
void tranverse(BTree *btree)
{
	inorder_tranverse(btree->root);
}

