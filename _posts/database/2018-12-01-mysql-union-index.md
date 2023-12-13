---
layout: post
title: MySQL联合索引底层实现(转)
tags:
- database
categories: database
description:  MySQL联合索引底层实现
---


索引是帮助MySQL高效获取数据的排好序的数据结构。本文介绍一下索引的底层实现。

>文章转载自[mysql建立联合索引 mysql联合索引底层实现](https://blog.51cto.com/u_13229/7046242)，主要是为了防止原文丢失，并方便自己后续的阅读学习。


<!-- more -->

## 1. MySQL为什么需要索引？
首先需要明白我们的表数据都是写在磁盘、硬盘上的。

每次表插入数据，不一定是连续存放的，比如A表第一条数据插入之后，过了一会才插入第二条数据，但是由于磁盘写数据是一个磁道一个磁道进行写入的，可能在你写入第一条数据之后，有其他程序把这个磁盘写满了，所以下一条数据可能写到了其他地方，所以这就造成了我们一张表的数据可能是随机分布也可能是连续存储的。


日常我们使用MYSQL 一个select可能查询到5条数据，假如这5条数据，分别都在5个不同的磁盘上，意味着我们要进行5次IO，所以为了减少IO交互（IO减少了，性能就提高了），基于此索引诞生了。


## 2. 索引数据结构-优劣分析
### 2.1 二叉树

![db-binary-tree](https://ivanzz1001.github.io/records/assets/img/db/index/db_binary_tree.png)

1) **特性**

![db-binary-tree-feature](https://ivanzz1001.github.io/records/assets/img/db/index/db_binary_tree_feature.png)

* 二叉树有根节点，左子树和右子树组成；

* 左边的元素是小于父元素的，右边的元素是大于它的父元素的；

* 树节点，都是从根节点开始查找。

2） **缺点**

![db-binary-tree-bad](https://ivanzz1001.github.io/records/assets/img/db/index/db_binary_tree_bad.png)

比如ID是递增的，因为二叉树的特性，依次插入1~7，就成了一个链表形式，那这个时候比如要查找6这个元素，起码要找6次，遇到这种情况效率是比较低下的。

### 2.2 Hash
1） **特性**

* 查找时，对索引的key进行一次hash计算就可以定位出数据存储的位置，然后去这个链表遍历到匹配的，
就可以拿到索引的磁盘文件地址，可能一次磁盘IO就拿到数据了。效率非常快

* 很多时候Hash索引要比B+ 树索引更高效

![db-hash](https://ivanzz1001.github.io/records/assets/img/db/index/db_hash.png)


2) **缺点**

* 仅能满足 ```=```、```IN```，不支持范围查询，所以范围查询时，不好定位，只能全表扫描，用不到索引。

* hash冲突问题



### 2.3 B树(多叉树)

1) **特性**

* 一个节点可以有多个元素，减少去磁盘寻址次数，提高效率；

* 节点中的数据索引从左到右递增排列；

* 所有索引元素不重复

* 叶节点具有相同的深度（所有叶子节点都是在同一列，按从左到右排序），叶节点的指针为空

* 每个节点不仅存放元素地址，还存放元素值

为了控制树的高度，在一开始划分空间的时候，划分的大一点，这样就可以横向去放索引，横向放的越多，树的高度就越小（也就是B树):

![db-btree](https://ivanzz1001.github.io/records/assets/img/db/index/db_btree.png)

既然知道了树的高度决定了查询效率，所以想尽可能的横向放更多的元素。

2） **缺点**

* B树每个元素不仅存放元素地址，还存放元素的data,这就造成了节点横向放的元素相对有限，因为每一个节点的空间是有限的。

* B树的查找性能不稳定，查找根节点和查找叶子结点的性能是完全不同的，B+树因为所有data都只存储在叶子结点，所以查找性能稳定


### 2.4 B+树
B树的进阶版本，MySQL采用的就是B+树作为底层数据结构。

MySQL表数据都存储在磁盘上，一般情况下，是在MySQL安装目录下的data目录，一个数据文件就是一个数据库实例，这个实例下存放的就是数据库表。

1） **特性**

* 非叶子节点不存储data，只存储索引(冗余)，可以放更多的索引	

* 叶子节点包含所有索引字段	

* 叶子节点用指针连接，提高区间访问的性能（存储这个节点在磁盘上的位置，会存储相邻节点的位置,对于范围查找，如```>``` ，```in```等很有帮助）

* 节点中的数据索引从左到右递增排列，排好序的

![db-bplustree](https://ivanzz1001.github.io/records/assets/img/db/index/db_bplustree.png)

2) **缺点**

* 索引会有重复

## 3. MySQL存储引擎

### 3.1 聚簇索引和非聚簇索引
* 聚簇索引: 叶子节点包含了完整的数据记录（innoDB存储引擎），就要聚簇索引

* 非聚簇索引：叶子节点不包含完整数据(MyISAM存储引擎)，存储的是磁盘文件地址，索引和数据分开存储的叫非聚簇索引。

### 3.2 MyISAM存储引擎索引实现

如下是MyISAM存储引擎结构图：

![db-myisam-index](https://ivanzz1001.github.io/records/assets/img/db/index/db_myisam_index.webp)

MyISAM会有三个文件：

* frm文件: 数据表结构

* MYD文件: 数据

* MYI: 索引

走索引情况下取数据流程：

1) MySQL找到匹配索引后，先去MYI索引文件里，找到这个索引下存放的磁盘文件地址

2） 拿到磁盘文件地址后，根据这个地址去MYD数据文件里去定位具体的数据行，拿到所有数据。


### 3.3 InnoDB存储引擎索引实现

1） 主键索引

可以看到叶子节点存储的索引和实际的所有值：

![db-innodb-index](https://ivanzz1001.github.io/records/assets/img/db/index/db_innodb_index.webp)


2) 二级索引(普通索引)

![db-innodb-normal](https://ivanzz1001.github.io/records/assets/img/db/index/db_innodb_normal.webp)

如上图，Alice这个值(它是一个name字段)被作为一个普通索引，叶子节点存储的实际上是18这个主键值。

下面这条SQL语句要查询name为Alice的所有值，这种情况会进行一个回表查询：
{% highlight string %}
select * from table where name = 'Alice';
{% endhighlight %}

3) 回表查询(主键索引和二级索引的区别)

通过下图我们可以看到其叶子节点存储的data只是一个主键值，并没有其他值。那是如何获取到数据所有字段的值呢？

![db-innodb-normal](https://ivanzz1001.github.io/records/assets/img/db/index/db_innodb_normal.webp)

它通过主键ID回表再去查询一次，就可以获取到这条记录的所有字段的值。


但是如果只是下面这种情况，则不需要进行回表操作，因为它一次查询就已经获得到了id:
{% highlight string %}
select id from table where name = 'Alice';
{% endhighlight %}

4) 为什么二级索引不存放记录的所有数据？

这里主要牵扯到2个问题：

* 节省存储空间（主键索引和二级索引都存储一条记录的完整数据的话，会有大量的冗余数据）

* 一致性问题（试想如果主键索引成功了，二级没成功，那么就出现了一致性问题，虽然也可以通过其他方式解决，但是就增加了复杂度)

5) innoDB文件

innoDB有两个文件：

* frm文件：表结构

* ibd文件: 数据+索引

### 3.4 innoDB与MyISAM存储引擎的最大区别

* MyISAM是用单独的文件来存放索引和数据的

* innoDB将索引和数据存放与同一个文件中。


## 4. 联合索引（复合索引)

多个字段组成一个索引，要想索引生效，必须按照创建索引的顺序才行。

![db-union-index](https://ivanzz1001.github.io/records/assets/img/db/index/db_union_index.webp)

索引是一个排好序的数据结构。这里在彻底搞明白联合索引之前需要知道一个概念```最左前缀原则```.

### 4.1 最左前缀原则

即按照*创建联合索引的先后顺序维护在底层的数据结构里*，所以想要联合索引生效必须按顺序来：

{% highlight string %}
//1: 可生效
select * from table where name='Bill' and age = 32;

//2： 不会生效
select * from table where age = 30;
{% endhighlight %}

1） 对于上述第一条SQL语句，先匹配name等于'Bill'，找到后就不糊再匹配后面的数据。然后只需要在name=Bill的基础上，去进行第二个字段age=32的比较，以此类推，后边第三、第四个字段也是同逻辑

2）对于上述第二条SQL语句，由于它略过了name，导致不是一个好排序的情况下，它只会进行全表扫描。




<br />
<br />
**[参看]**:

1. [MySQL联合索引底层实现](https://blog.51cto.com/u_13229/7046242)

2. [MySQL联合索引遵循最左前缀匹配原则](https://baijiahao.baidu.com/s?id=1740182728757585855&wfr=spider&for=pc)


<br />
<br />
<br />

