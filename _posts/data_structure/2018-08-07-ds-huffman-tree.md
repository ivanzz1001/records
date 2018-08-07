---
layout: post
title: 赫夫曼树及其应用
tags:
- data-structure
categories: data-structure
description: 赫夫曼树及其应用
---

本章我们主要介绍一下赫夫曼树及其应用。赫夫曼(Huffman)树，又称最优树，是一类带权路径长度最短的树，有着广泛的应用。


<!-- more -->


## 1. 最优二叉树（赫夫曼树)

首先给出路径和路径长度的概念。从树中一个节点到另一个节点之间的分支构成这两个节点之间的路径，路径上的分支数目称作**路径长度**。**树的路径长度**是从树根到每一节点的路径长度之和。完全二叉树就是这种路径长度最短的二叉树。

若将上述概念推广到一般情况，考虑带权的节点。节点的带权路径长度为从该节点到树根之间的路径长度与节点上权的乘积。树的带权路径长度为树中所有```叶子节点```的带权路径长度之和，通常记作：![ds-huffman-length](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_huffman_length.jpg)






<br />
<br />


