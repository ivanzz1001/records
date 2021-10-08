---
layout: post
title: LevelDB实现细节
tags:
- leveldb
categories: leveldb
description: leveldb源代码分析
---


本文参考LevelDB官方文档，来大体介绍一下LevelDB的相关实现细节。


<!-- more -->


## 1. Files
LevelDB的实现本质上类似于单个[BigTable tablet (section 5.3)](https://research.google/pubs/pub27898/)。然而，LevelDB的文件组织形式与BigTable tablet不同，下面我们会进行介绍。

在LevelDB中，每一个database都是对应于目录中的一系列文件。如下是一个LevelDB数据库目录：
<pre>
[root@localhost testdb1]# ls
000005.ldb  000006.log  CURRENT  LOCK  LOG  LOG.old  MANIFEST-000004
</pre>
有多种不同类型的文件，下面我们会逐一介绍。

### 1.1 Log files
一个日志文件（```*.log```)按顺序存放着当前最近的更新（注：这里的按顺序是按写入顺序，并不是按key的顺序）。每一个更新都会按顺序追加到当前的日志文件中。当日志文件达到预先设定的值时(默认情况下，大约为4MB)，其就会被转换成sorted table，同时生成一个新的日志文件以存放新的更新数据。

此外，写入到当前日志文件中的数据也会同时保存在一个内存结构（即```memtable```）中。当每次进行读数据时，会首先查询该memtable，这样读操作就能反映到所有的日志更新。	


## 2. Sorted tables

一个sorted table(```*.ldb```)按key的顺序存放着一系列的entries。其中，每一个entry或者是一个k/v，或者是一个key的删除标记（删除标记会被用来去掉那些老的sorted tables中已过时的value值）。

sorted tables集合是通过一系列的levels来组织的。从日志文件生成的sorted table会被放置在一个特殊的young level(也被称为```level-0```)中。当yong level下的文件个数超过了指定的阈值（当前为4）时，则该yong level下的所有文件会与```level-1```下与之有重叠的那些文件merge到一块（每2MB数据会创建一个新的```level-1```文件）。

yong level下的文件可能会包含重叠(overlapping)的keys。然而，其他levels下的文件则各自拥有独立不重叠的key范围（注：由于Level-0下的文件是由日志直接产生，还未合并，因此会存在重叠）。考虑```Level-L```(L>=1)，当该Level下文件的总大小超过```10^L```MB(例如，level-1就是10MB， level-2就是100MB, ...)时，Level-L下的```一个```文件就会与Level-(L+1)下所有重叠的文件进行合并，并在Level-(L+1)下形成一系列新的文件。通过这一合并过程，就会将yong level中新的更新数据逐渐的迁移到最大的level中（为了降低seek开销，在合并时会采用bulk read和bulk write)。

### 2.1 Manifest
一个```MANIFEST```文件会列出每个Level下的sorted tables，以及对应的keys范围和其他一些重要的元数据信息。无论何时，当一个数据库被重新打开时，就会创建一个新的```MANIFEST```文件(文件名会嵌入一个新的数字)。```MANIFEST```文件会被格式化为一个log，当相应的服务状态发生(比如add或remove文件）改变时就会追加到该log中。

如下我们以十六进制(```:%!xxd```)方式查看```MANIFEST-000004```:
<pre>
# vi MANIFEST-000004
0000000: 56f9 b8f8 1c00 0101 1a6c 6576 656c 6462  V........leveldb
0000010: 2e42 7974 6577 6973 6543 6f6d 7061 7261  .BytewiseCompara
0000020: 746f 72fa 8504 af3b 0001 0206 0900 0307  tor....;........
0000030: 04e8 0707 0005 c5bd 0214 6974 776f 726c  ..........itworl
0000040: 6431 3233 2d30 0101 0000 0000 0000 1669  d123-0.........i
0000050: 7477 6f72 6c64 3132 332d 3939 3901 e803  tworld123-999...
0000060: 0000 0000 000a 
</pre>

### 2.2 Current

```CURRENT```是一个简单的文本文件，其包含了当前最新的MANIFEST文件。

下面我们来查看一下：
<pre>
# cat CURRENT 
MANIFEST-000004
</pre>

### 2.3 Info logs
Infomation消息会被打印到名称为```LOG```和```LOG.old```文件中。

下面我们来看看这两个文件：
<pre>
# cat LOG
2021/10/01-21:27:02.256722 140208539686720 Recovering log #3
2021/10/01-21:27:02.257181 140208539686720 Level-0 table #5: started
2021/10/01-21:27:02.258028 140208539686720 Level-0 table #5: 40645 bytes OK
2021/10/01-21:27:02.259382 140208539686720 Delete type=0 #3
2021/10/01-21:27:02.259394 140208539686720 Delete type=3 #2

# cat LOG.old 
2021/10/01-21:24:39.876023 140387033372480 Creating DB ./testdb1 since it was missing.
2021/10/01-21:24:39.884158 140387033372480 Delete type=3 #1
</pre>

### 2.4 Others

其他的文件被用于一些杂项的目的。比如```LOCK```文件，```*.dbtmp```文件。

## 3. Level 0
当log file持续增长达到一定的size(默认为4MB)时：会创建一个新类型的memtable(即immtable))和一个新的log file，并且新的数据更新写入到新产生的log file中。

并同时在后台执行：

1) 将前一个memtable中的数据写入到sstable中；

2）丢弃该memtable;

3) 删除旧的log file以及旧的memtable;

4) 添加新的sstable到yong level(level-0)中

## 4. Compactions

当Level-L的大小达到了规定的限制值后，就会触发一个后台线程对其进行compact操作。在压缩时，会从Level-L中选出一个文件，然后与Level-(L+1)中的所有重叠文件进行合并。值得注意的是，即使level-L下的这个文件只与level-(L+1)下的某文件的一部分有重叠，也需要读取整个level-(L+1)下的那个文件进行compaction，之后这个旧的Level-(L+1)下的文件就会被丢弃。另外，由于Level-0比较特别（Level-0中的文件相互之间可能是有重叠），因此我们会对从Level-0到Level-1的compaction做特殊处理：如果该Level-0文件与其他的Level-0文件有重叠时，就会选择多个level-0文件来进行compaction。

一个compaction操作会将它选定的那些文件合并到Level-(L+1)，并产生一系列新的level-(L+1)下的文件。在将当前选定的文件与Level-(L+1)下的文件合并后产生的输出达到了指定的目标大小(2MB)时，就会产生一个新的Level-(L+1)文件。此外，如果当前合并后产生的输出的key范围与Level-(L+2)下超过10个文件有重叠，那么也会产生一个新的Level-(L+1)文件。通过这后面一条准则，可以确保之后对Level-(L+1)下的一个文件进行compaction操作时，可以不用从Level-(L+2)中选择太多的文件。

当合并完成，旧的文件就会被丢弃，然后新的文件会加入到serving state中（即MANIFEST）.


针对指定的Level进行compactions时，是按整个key空间进行rotate的。更具体地，比如对于Level-L，我们会记住它上次compaction时的最后的key值，然后在下一次对Level-L进行compaction时，我们会选择排在该key之后的第一个文件开始(如果没有这样的文件，那我们就再从头开始)。

compaction会丢弃掉被覆盖的那些value值。同时，如果当前的key带有删除标记，且不与更大编号的Levels中的文件的key范围重叠，则直接删除该key。

## 5. Timing

对Level-0的compaction操作会读取Level-0中最多4个1MB的文件，并且在最坏情况下会读取所有的Level-1文件(共10MB)，这样我们最多会进行14MB的读操作，以及14MB的写操作。

除了这一特殊的Level-0的compactions之外，我们只会从Level-L中选取一个2MB的文件，然后在最坏的情况下其最多会与Level-(L+1)中的12个文件有重叠（注：这里12=10+2，其中```10```是因为Level-(L+1)中文件的总大小为Level-L中文件的总大小的10倍；而另外的```2```个是由于Level-L中该带合并的文件的boundaries与Level-(L+1)文件范围并不对齐)。因此，在最差情况下，compaction操作会```读写```各26MB的数据。假设硬盘的IO速率为100MB/s(注：现代硬盘大概都能达到这个范围），则最坏情况下compaction操作会耗费大概0.5s。

假如我们对后台写操作进行一些限制，比如限制在全部IO带宽（100MB）的10%，则一个compaction操作可能需要耗费5s。假如用户正以10MB/s的速度进行写入，则可能会创建大量的Level-0文件（多达50个Level-0文件来容纳这```5*10MB```新写入的数据）。这会极大的增加读操作的耗时，这是因为每次进行读取操作时都需要读取Level-0中的所有文件，然后将结果进行合并)。那么要如何解决这个问题呢？下面给出几种参考解决方案：

* Solution 1：要解决该问题，我们可以在level-0中文件的个数太大时，增加log切换的阈值。但缺点就是，如果log切换的阈值增大，则需要耗费更多的内存来维持与之对应的memtable。

* Solution 2：当level-0下的文件数上升很快时，我们可以人为地降低写操作速率。

* Solution 3：尽量降低wide merge的开销。由于大多数的level-0下的文件的block可能都已经缓存在cache里了，因此我们只需要关注merge迭代过程中O(N)的复杂度的这部分。

## 6. Number of files

为减少文件个数，我们可以通过为更高编号Level下的文件使用更大的文件大小，以取代固定的2MB大小来实现，但这可能会导致在compaction过程中更高的耗费。另外，我们也可以通过将这些文件shard到不同的目录中，从而降低单个文件夹下文件的个数。

于2011年2月4日在ext3文件系统上所做了一个实验：目录中文件的个数与文件打开时间的关系(注：进行了100K次打开操作)。有如下实验结果：
<pre>
| Files in directory | Microseconds to open a file |
|-------------------:|----------------------------:|
|               1000 |                           9 |
|              10000 |                          10 |
|             100000 |                          16 |
</pre>
看起来在现代文件系统中，没有必要进行目录切分(通过上面结果可以看出来，目录下的文件数对文件打开时间影响很小，起码不是线性关系)。

## 7. Recovery

* 读取CURRENT文件找到最新提交的MANIFEST文件名

* 读取所找到的MANIFEST文件

* 清除旧的文件

* 可以在这里打开所有的sstables文件，但通常来说采用lazy open的方式可能会更好...

* 将日志转化为一个新的level 0下的sstable

* 将新写入的数据写到一个新的日志文件中（同时在日志头中recovery sequence)

## 8. Garbage collection of files

DeleteObsoleteFiles()会在每次compaction结束及recovery结束后调用。它会找到数据库中所有文件的名称。然后删掉除当前log file外的所有其他日志文件，也删除掉那些不被level所引用的sstable files以及那些非active compactions所产生的输出文件。












<br />
<br />

**[参看]**

1. [Leveldb高效存储实现](https://stor.51cto.com/art/201903/593197.htm)

2. [LevelDB深入浅出之整体架构](https://zhuanlan.zhihu.com/p/67833030)

3. [leveldb源码阅读系列](https://zhuanlan.zhihu.com/p/80684560)

4. [LevelDB入门教程十篇](https://zhuanlan.zhihu.com/p/25349591)

5. [LevelDB github官网](https://github.com/google/leveldb)

6. [LevelDB官方文档](https://github.com/google/leveldb/blob/master/doc/index.md)

7. [LevelDb实现原理](https://blog.csdn.net/gdutliuyun827/article/details/70911342)

8. [leveldb相关](https://www.zhihu.com/topic/19819000/hot)

9. [leveldb实现细节](https://github.com/google/leveldb/blob/master/doc/impl.md)


10. [google C++代码规范](https://google.github.io/styleguide/cppguide.html)

<br />
<br />
<br />

