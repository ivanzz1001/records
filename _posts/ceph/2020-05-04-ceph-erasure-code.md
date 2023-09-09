---
layout: post
title: Erasure-Code(纠删码) 最佳实践(转)
tags:
- ceph
categories: ceph
description: Erasure-Code(纠删码) 最佳实践
---


本文主要介绍一下纠删码的基本原理，以便更好的理解ceph中相关代码的实现。

>ps: 文章转载自https://zhuanlan.zhihu.com/p/106096265, 主要目的是为了后续方便的找到相关文章，以防原文丢失。

<!-- more -->

## 1. 纠删码原理
这个星球产生的数据越来越庞大，差不多2010年开始各大互联网公司大都上线了系统以应对数据膨胀带来的成本增长。Erasure-Code（纠删码）技术应用其中。典型如Google 新一代分布式存储系统colossus系统的Reed-solomon算法、Window Azure Storage 的LRC算法等等。


EC(Erasure-Code)算法的最底层的基本的数学原理：
<pre>
行列矩阵中一种特殊矩阵的性质：即任意MxN（M行N列{M<N}）的行列式，其任意MxM的子矩阵都是可逆，以实现数据恢复运算。
</pre>
如下图所示，以一个典型的例子进行说明。

D1～D5通过```矩阵A(8*5)```相乘得到D1～D5的原始数据和C1～C3的校验数据块（Figure-1）。假设此时原始数据块D1、D4和校验数据块C1发生损坏。那要如何才能读取D1、D4等数据块、还原C1校验数据块？这个时候就依赖矩阵运算的特性。首先可知从A获取子矩阵B‘ （5*5）与原始数据相乘可以得到D2、D3、D5、C2、C3（即现有还未损坏的数据）（Figure-1），那反过来说，当前的问题就是：如何通过已有的B‘和D2、D3、D5、C2、C3还原得到D1、D4数据块和C1校验块。此时利用矩阵运算，假设B可逆，在等式2两边分别乘上B’的可逆矩阵B'-1，这样就可以通过B'-1 和已有的D2、D3、D5、C2、C3 进行矩阵运算还原得到D1和D4数据块。C1可以通过（B11～B15）与已经恢复的数据(D1～D5)相乘获得。该过程可行的核心保障就是需要确保矩阵A的任意```5*5```的子矩阵的可逆矩阵都是存在的，这样才能确保丢失8块数据中的任意3块数据都可以进行数据还原。核心的重点就是需要找到这样的矩阵A，其中黄色部分就是范德蒙矩阵（这里对此不多做展开，自行google或者参看任何矩阵论的教材都有清晰的说明）。

![erasure-code](https://ivanzz1001.github.io/records/assets/img/ceph/erasure/erasure-code.jpg)

## 2. EC与数据放置
首先看如何对数据进行数据放置。比如HDFS、colossus、Ceph 将数据条带化的放置在不同的chunk(Stripe placement)。而Windows Azure Storage 则使用连续数据块放置方式(Contiguous placement)。各自有各自的特点。

![stripe-placement](https://ivanzz1001.github.io/records/assets/img/ceph/erasure/erasure-code.jpg)

如上图所示，假设*abcdfghigklmnopqrstuvwxyz*为原始的一段数据内容，在EC场景下可以有2种截然不同的数据划分方式。

* Stripe Placement: 条带的数据放置方式，即将数据顺序进行拆散，逻辑放置在不同的数据块中，打破了数据原先的物理相邻顺序。

* Contiguous PlaceMent: 连续的数据放置方式，即保留数据原来的顺序，除了数据分块的边界(如上图D1、D2)的边界，核心上来说数据逻辑上还是保持了相邻的顺序。

这2种方式各有各的特点，如上图所示，在工程上D1～D5数据块 、C1～C3校验块一般都按照故障域原则放置在不同可用区的不同的磁盘上。


### 2.1 Stripe Placement 的特点


1) 一份数据的读取可以同时利用多个磁盘的吞吐能力，但是对于IOPS来说是放大（换句话说对大块数据读取比较友好），缺点就是失去了数据的locality（这在Hadoop大数据体系中将计算放置在数据附近来说是很关键的一点）；

2) 及时EC，即不用等凑足整一份大的数据才进行EC写入，基本在凑足EC的条带大小即可进行写入，也就是说在线数据写入可以直接以EC的体系。


### 2.2 Contiguous Placement的特点
Contiguous Placement的特点则相对来说相反：

$a + b = c$

1) 数据都是临近放置，所以一般情况下的数据的读取就跟副本形式一样，在一个数据节点是就可以获得，对于小IO来说比较友好，对于大IO没有明显的缺陷。

2) 不能进行及时EC。需要进行凑足一定的数据才能够形成D1到D5的数据块进行EC，所以一般来说比较适合做后台的EC。比如Window Azure Storage 是先写三副本的Extent，在Extent seal（关闭掉）之后后台异步得将数据EC。




## 5. ceph纠删码插件介绍





<br />
<br />

**[参看]**

1. [Erasure-Code(纠删码) 最佳实践](https://zhuanlan.zhihu.com/p/106096265)

2. [线性代数：行列式](https://www.bilibili.com/video/BV1hD4y1R776/?p=10&vd_source=2699f104de8828a576fed54818f8cd79)

3. [Erasure Code 原理和工程化介绍](https://link.zhihu.com/?target=https%3A//blog.openacid.com/storage/ec-1/)

4. [纠删码(erasure code)介绍](https://zhuanlan.zhihu.com/p/554262696)

<br />
<br />
<br />




