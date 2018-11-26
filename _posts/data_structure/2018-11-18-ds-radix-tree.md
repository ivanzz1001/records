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
在计算机科学中，radix tree(也被称为radix trie，或者compact prefix tree)用于表示一种```空间优化)的trie```(prefix tree)数据结构。 假如树中的一个节点是父节点的唯一子节点(the only child)的话，那么该子节点将会与父节点进行合并，这样就使得radix tree中的每一个内部节点最多拥有```r```个孩子， ```r```为正整数且等于```2^n```(n>=1)。不像是一般的trie树，radix tree的边沿(edges)可以是一个或者多个元素。参看如下：

![ds-radix-tree](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_radix_tree.png)


在radix tree的wiki中说到如下场景也很适合使用此数据结构来存储：
<pre>
This makes radix trees much more efficient for small sets (especially if the strings are long) and
for sets of strings that share long prefixes.
</pre>

不像是平常的树结构（在进行key的比较时，是整个key从头到尾进行比较），radix key在每个节点进行key的比较时是以```chunck```为单位来进行的，每一个chunk中的bit数目等于```radix tree```的基数```r```。

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

![radix-tree-insert](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_radixtree_insert.png)




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


