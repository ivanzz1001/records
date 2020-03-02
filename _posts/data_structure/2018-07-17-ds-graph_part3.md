---
layout: post
title: 数据结构之: 图的连通性问题
tags:
- data-structure
categories: data-structure
description: 数据结构之图的连通性
---


在这一节中，我们将利用遍历图的算法来求解图的连通性问题，并讨论：

* 最小代价```生成树```

* 重连通性与通信网络的经济性和可靠性的关系

<!-- more -->


## 1. 无向图的连通分量和生成树

在对无向图进行遍历时，对于连通图，仅需从图中任一顶点出发，进行深度优先搜索或广度优先搜索，便可访问到图中所有顶点。对非连通图，则需从多个顶点出发进行搜索，而每一次从一个新的起始点出发进行搜索过程中得到的顶点访问序列恰为其各个连通分量中的顶点集。例如，下图中```G3```是非连通图，按照其邻接表(```图(b)```)进行深度优先搜索遍历，3次调用DFS过程（分别从顶点A、D和G出发）得到的顶点访问序列为：
<pre>
A L M J B F C        D E       G K H I
</pre>

![ds-graph-adjtbl](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_graph_adjtbl.jpg)

这3个顶点集分别加上所有依附于这些顶点的边，边构成了非连通图G3的3个连通分量，见如下图：

![ds-graph-partial](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_graph_partial.jpg)



      





<br />
<br />


