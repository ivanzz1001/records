---
layout: post
title: 数据结构之Radix Tree
tags:
- data-structure
categories: data-structure
description: 数据结构之Radix Tree
---

本文我们简要介绍一下Radix Tree的相关概念及实现原理。

<!-- more -->


## 1. radix tree定义
在计算机科学中，radix tree(也被称为radix trie，或者compact prefix tree)用于表示一种```空间优化的trie```(prefix tree)数据结构。 假如树中的一个节点是父节点的唯一子节点(the only child)的话，那么该子节点将会与父节点进行合并，这样就使得radix tree中的每一个内部节点最多拥有```r```个孩子， ```r```为正整数且等于```2^n```(n>=1)。不像是一般的trie树，radix tree的边沿(edges)可以是一个或者多个元素。参看如下：

![ds-radix-tree](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_radix_tree.png)


在radix tree的wiki中说到如下场景也很适合使用此数据结构来存储：
<pre>
This makes radix trees much more efficient for small sets (especially if the strings are long) and
for sets of strings that share long prefixes.

即元素个数不是太多，但是元素之间通常有很长的相同前缀时很适合采用radix tree来存储
</pre>

不像是平常的树结构（在进行key的比较时，是整个key从头到尾进行比较），radix key在每个节点进行key的比较时是以```bit组```(chunk-of-bits)为单位来进行的，每一个chunk中的bit数目等于```radix tree```的基数```r```。

## 2. radix tree使用场景举例
我们经常使用Radix tree来构建key为字符串的[关联数组(associative arrays)](https://en.wikipedia.org/wiki/Associative_array)；在构建IP路由(ip-routing)的应用方面radix tree也使用广泛，因为IP通常具有大量相同的前缀； 另外radix tree在倒排索引方面也使用广泛。

## 3. radix tree操作
Radix tree支持插入、删除、搜索等方面的操作。```插入操作```是添加一个新的字符串到trie树中并尝试最小化数据存储（即对某些节点进行合并）； ```删除操作```是从trie树中移除一个字符串； 而搜索操作包括严格查找(exact lookup)、前缀查找、后缀查找以及查询某个前缀的所有字符串。所有这些操作的时间复杂度为```O(k)```(k为字符串集合中最长的字符串长度）。

### 3.1 查找
查找操作用于判定一个字符串是否存在于```trie```树中。本操作与普通的```trie树```查找相似，除了某些```edges```可能会包含多个元素之外。

如下的伪代码我们假设这些类已经存在：

**Edge**

* Node targetNode

* string label

**Node**

* Array of Edges edges

* function isLeaf()


{% highlight string %}
function lookup(string x)
{
	// Begin at the root with no elements found
	Node traverseNode := root;
	int elementsFound := 0;

	// Traverse until a leaf is found or it is not possible to continue
	while (traverseNode != null && !traverseNode.isLeaf() && elementsFound < x.length)
	{
		// Get the next edge to explore based on the elements not yet found in x
		Edge nextEdge := select edge from traverseNode.edges where edge.label is a prefix of x.suffix(elementsFound)
		// x.suffix(elementsFound) returns the last (x.length - elementsFound) elements of x

		// Was an edge found?
		if (nextEdge != null)
		{
			// Set the next node to explore
			traverseNode := nextEdge.targetNode;

			// Increment elements found based on the label stored at the edge
			elementsFound += nextEdge.label.length;
		}
		else
		{
			// Terminate loop
			traverseNode := null;
		}
	}

	// A match is found if we arrive at a leaf node and have used up exactly x.length elements
	return (traverseNode != null && traverseNode.isLeaf() && elementsFound == x.length);
}
{% endhighlight %}
下面给出```radix tree```查找的一个过程图：

![radix-tree-lookup](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_radixtree_lookup.png)


### 3.2 插入操作
要插入一个字符串，首先我们需要执行查找，直到查找到某个节点处停止。此时，我们或者可以直接将```input string```的剩余部分添加到一个新的```Edge```中； 或者有一个edge与我们的```input string```的剩余部分有相同的前缀，此种情况下我们需要将该```edge```分裂成两个```edge```（其中第一个edge存储common prefix）然后再进行处理。这里分裂edge确保了一个节点的孩子(children)数目不会超过总的字符串的个数。

如下我们展示了插入时的多种情况（仍有部分未列出）。下图中```r```代表的是根节点（root)。这里我们假设```edge```以空字符串(empty string)作为结尾。

![radix-tree-insert](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_radixtree_insert.jpg)

### 3.3 删除节点

要从radix tree中删除一个字符串```x```，我们必须首先定位到代表```x```的叶子节点。假设该叶子节点存在，我们就移除该叶子节点。此时，我们需要检查该被删节点的父节点，假如父节点只剩下一个孩子节点的话，则将该孩子节点的```incomming label```追加到父节点的```incomming label```上并将该孩子节点移除。

### 3.4 与其他数据结构的比较
(说明： 如下的比较，我们假设key的长度为```k```，数据结构中元素的数目为```n```)

radix tree允许在O(k)的时间复杂度内进行查找、插入、删除等操作，而```balanced trees```在进行这些操作时时间复杂度一般为```O(logn)```。这看起来并不像是一个优势，因为通常情况下```k>=log(n)```，但是在平衡树中key的比较通常是字符串的完整比较，时间复杂度是```O(k)```，并且在实际使用过程中很多key可能还拥有相同的前缀。假如使用```trie```数据结构来进行存储，所有key的比较都是```constant time```，但是如果我们查询的字符串长度为```m```，那么也必须执行```m```次的比较。使用```radix tree```数据结构时，则需要更少的比较操作，并且使用的节点个数也更少。

当然，radix tree也与```trie```树一样有相同的劣势，然而：因为它们只能被应用于字符串元素或者能够高效映射为字符串的元素，它们缺少了平衡查找树那样的通用性，平衡查找树可以应用于任何类型以及任何比较顺序。

而```Hashtables```通常被认为具有```O(1)```时间复杂度的插入与删除操作，但先决条件是我们认为计算```hash key```的操作可以在```constant-time```内完成。当我们把计算```hash key```的时间也考虑在内的话，```hashtables```的插入和删除时间复杂度就变为O(k)，并且在key碰撞的情况下，可能会有更差的时间复杂度。radix tree的插入和删除的时间复杂度为```O(k)```。此外，hashtables也不支持```Find predecessor```与```Find successor```操作。
<pre>
Find predecessor: Locates the largest string less than a given string, by lexicographic order.

Find successor: Locates the smallest string greater than a given string, by lexicographic order.
</pre>



### 3.5 示例代码

下面我们给出一个radix tree实现的示例代码(注意，这里edges我们并没有按字典序来进行存储）：
{% highlight string %}
typedef struct radix_node_s radix_node_t;


struct radix_edge_t{
	radix_node_t *incommingNode;
	char *label;

	struct radix_edge_t *next;		
};

struct radix_node_s{
	struct radix_edge_t *head;
};


char *global_empty_str = ""

int init_radix_tree(radix_node_t *root)
{
	root->head = NULL;
}


int is_leaf(radix_node_t *node)
{
	if(node->head == NULL)
		return 1;

	return 0;
}


/*
 *  description: check wether 'pre' is prefix of label
 * 
 *  return: 0---not prefix    1---is prefix and but not equal    2 --- equal
 */
int is_prefix(char *pre, char *label, char **out)
{
	*out = label;
	
	while(*pre && *pre++ == *label++);

	if(*pre)
		return 0;
	else{
		*out = label;
		if (*label)
			return 1;
		else
			return 2;
	}
	
}

int insert_radix_tree(radix_node_t *root, char *label)
{
	radix_node_t *p = root;
	radix_node_t *prev = root;
	struct radix_edge_t *edge;
	
	char *s = label;
	int a;

	while(p && !is_leaf(p) && *s){
	
		edge = p->head;
		while(edge){
			a = is_prefix(edge->label,s, &s);
			if(a = 1){
				prev = p;
				p = edge->incommingNode;
				break;
			}else if(a == 2){
				return 0;
			}
			edge = edge->next;
		}
		if (!edge)
			break;
	}

	if (prev->head)
	{
		struct radix_edge_t *new_edge = (struct radix_edge_t *)malloc(sizeof(struct radix_edge_t));
		new_edge.incommingNode = NULL;
		new_edge.label = strdup(s);
		new_edge.next = prev->head;
		prev->head = new_edge;
	}else{
		struct radix_edge_t *empty_edge = (struct radix_edge_t *)malloc(sizeof(struct radix_edge_t));
		struct radix_edge_t *new_edge = (struct radix_edge_t *)malloc(sizeof(struct radix_edge_t));

		empty_edge.label = global_empty_str
		empty_edge.incommingNode = NULL;
		empty_edge.next = new_edge;

		new_edge.label = strdup(s);
		new_edge.incommingNode = NULL;
		new_edge.next = NULL;

		prev->head = empty_edge;
	}

	return 0x0;
	
}

{% endhighlight %}


## 2. Linux基数树
对于```长整型```数据的映射，怎样解决Hash冲突和Hash表大小的设计是一个非常头疼的问题。radix树就是针对这对这样的稀疏长整型数据查找，能高速且节省空间地完成映射。借助于Radix树，我们能够实现对于长整型数据类型的路由。利用radix树能够依据一个长整型（比如一个长ID）高速的找到其相应的对象指针。这比用hash映射来的简单，也更节省空间，使用Hash映射hash函数难以设计，不恰当的hash函数可能增大冲突，或浪费空间。

radix tree是一种多叉搜索树，树的叶子节点是实际的数据条目。每一个节点有一个固定的、```2^n```指针指向子节点（每一个指针称为slot，```n```为划分的基的大小）。

### 2.1 插入、删除
radix tree事实上就几乎与传统的二叉树相同。仅仅是在寻找方式上，利用比如一个unsigned int类型的每个比特位作为树节点的推断。例如，我们有一个数据：
<pre>
1000101010101010010101010010101010
</pre>
那么依照radix树的插入就是在根节点开始，假如遇到0，就指向左节点；遇到1就指向右节点。在插入过程中构造树节点，在删除过程中删除树节点。如果认为有太多的malloc()调用的话，可以采用池化技术，预先分配多个节点。

每一个节点使用1个bit位的话，会使树的高度过高，非叶节点过多。故在实际应用中，每一个节点通常使用多个bit位，但多bit位会使节点的子节点slot变多，增大节点的体积，一般选用2个或4个bit位作为树节点就可以。如下图所示：

![radix-tree-bit](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_radixtree_bit.jpg)


1） **插入**

我们在插入一个新节点时，我们依据数据的比特位，在树中向下查找，若没有对应的节点则生成对应节点，直到数据的bit位访问完，则建立叶节点映射对应的对象。

2） **删除**

我们可以```惰性删除```，即沿着路径查找到叶节点后，直接删除叶节点，中间的非叶节点不删除。

### 2.2 radix树与trie树的比较

Radix树与Trie树的思想有点类似，甚至能够把Trie树看成是一个基为26的Radix树（也可以把radix树看做是Trie树的变异）。Trie树一般用于字符串到对象的映射，而radix树一般用于长整数到对象的映射。

trie树主要问题是树的层高。假设要索引的字符串长度非常长非常变态，我们也要建一颗非常高非常变态的树么？ radix树能固定层高（对于较长的字符串，能够用数学公式计算出其特征值，再用radix树存储这些特征值）。




<br />
<br />
**[参看]:**

1. [基数树radix_tree_root](https://blog.csdn.net/flyxiao28/article/details/80368353)

2. [基数树(radix tree)](http://www.cnblogs.com/wuchanming/p/3824990.html)

3. [查找——图文翔解RadixTree（基数树）](https://www.cnblogs.com/wgwyanfs/p/6887889.html)

4. [Radix tree](https://en.wikipedia.org/wiki/Radix_tree)

5. [Linux内核源码分析-基树处理- radix_tree](https://blog.csdn.net/weifenghai/article/details/53113395)

6. [Radix TRee 维护100亿个URL](https://www.xuebuyuan.com/3203631.html)

7. [高级数据结构之基数树ngx_radix_tree_t](https://www.cnblogs.com/blfshiye/p/5269679.html)

8. [基数树结构ngx_radix_tree_t](https://blog.csdn.net/u012819339/article/details/53581203?utm_source=blogxgwz0)

<br />
<br />
<br />


