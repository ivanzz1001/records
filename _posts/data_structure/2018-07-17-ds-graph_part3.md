---
layout: post
title: 数据结构图的连通性问题
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


设E(G)为连通图G中所有边的集合，则从图中任一顶点出发遍历图时，必定将E(G)分成两个集合T(G)和B(G)，其中T(G)是遍历图过程中经历的边的集合；B(G)是剩余的边的集合。显然，T(G)和图G中所有顶点一起构成连通图G的极小连通子图。按照[图的定义和术语](https://ivanzz1001.github.io/records/post/data-structure/2018/07/16/ds-graph)一节中的定义，它是连通图的一棵生成树，并且称由深度优先搜索得到的为```深度优先生成树```；由广度优先搜索得到的为```广度优先生成树```。例如，下图7.15(b)和(c)所示分别为连通图G4的深度优先生成树和广度优先生成树，图中虚线为集合B(G)中的边。

![ds-graph-gentree](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_graph_gentree.jpg)

对于非连通图，每个连通分量中的顶点集，和遍历时走过的边一起构成若干棵生成树，这些连通分量的生成树组成非连通图的生成森林。如下图7.15(e)所示为G3的深度优先生成森林，它由三棵深度优先生成树组成。

![ds-graph-genforest](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_graph_genforest.jpg)

假设以```孩子兄弟链表```作为生成森林的存储结构，则```算法7.7```生成非连通图的深度优先森林，其中DFSTree函数如```算法7.8```所示。显然，算法7.7的时间复杂度和遍历相同。

**```算法7.7```**:
{% highlight string %}
void DFSForest(Graph G, CSTree &T){
	//建立无向图G的深度优先生成森林的（最左）孩子（右）兄弟链表T
	
	T = NULL;
	
	for(v = 0; v < G.vexnum; v++)
		visited[v] = FALSE;
		
	for(v = 0; v < G.vexnum; v++){
		if(!visited[v]){
			p = (CSTree)malloc(sizeof(CSNode));      //分配跟节点
			*p = {GetVex(G, v), NULL, NULL};         //给该节点赋值
			
			if(!T){
				T = p;                               //是第一棵生成树的根（T的根）
			}else{
				q->nextsibling = p;                  //是其他生成树的根（前一棵的根的"兄弟"）
			}
			
			q = p;                                   //q指示当前生成树的根
			
			DFSTree(G, v, p);                        //建立以p为根的生成树
		}
	}

}
{% endhighlight %}

**```算法7.8```**:
{% highlight string %}
void DFSTree(Graph G, int v, CSTree &T){
	//从第v个顶点出发，深度优先遍历图G，建立以T为根的生成树
	
	visited[v] = TRUE;
	first = TRUE;
	
	for(w = FirstAdjVex(G, v); w >= 0; w = NextAdjVex(G, v, w)){
		if(!visited[w]){
			p = (CSTree)malloc(sizeof(CSNode));            //分配孩子节点
			
			*p = {GetVex(G, w), NULL, NULL};
			if(first){                                     //w是v的第一个未被访问的邻接顶点
				T->lchild = p; first = FALSE;              //是根的左孩子节点
				
			}else{                                         //w是v的其他未被访问的邻接顶点
				q->nextsibling = p;                        //是上一邻接顶点的右兄弟节点
			} 
			
			q = p;
			DFSTree(G, w, q);                              //从第w个顶点出发深度优先遍历图G，建立子生成树q
		
		}
	}
}
{% endhighlight %}

如下是对无向图G3的深度优先生成森林的```孩子兄弟链表```结构：

![ds-graph-forest-store](https://ivanzz1001.github.io/records/assets/img/data_structure/ds_graph_forest_store.jpg)






<br />
<br />


