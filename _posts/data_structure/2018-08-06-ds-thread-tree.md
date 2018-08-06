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



<br />
<br />


