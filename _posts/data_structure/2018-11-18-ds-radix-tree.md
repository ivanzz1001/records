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


在radix tree的wiki中说到如下场景也很适合使用此数据结构来存储：
<pre>
This makes radix trees much more efficient for small sets (especially if the strings are long) and
for sets of strings that share long prefixes.
</pre>




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


