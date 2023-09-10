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

1) 数据都是临近放置，所以一般情况下的数据的读取就跟副本形式一样，在一个数据节点是就可以获得，对于小IO来说比较友好，对于大IO没有明显的缺陷。

2) 不能进行及时EC。需要进行凑足一定的数据才能够形成D1到D5的数据块进行EC，所以一般来说比较适合做后台的EC。比如Window Azure Storage 是先写三副本的Extent，在Extent seal（关闭掉）之后后台异步得将数据EC。

## 3. EC条带大小与小IO
在大规模的存储系统中，小文件往往会结合索引机制，将小文件合并成一整个大文件。详见对象存储架构设计[A Bite of S3 Storage Arch](https://zhuanlan.zhihu.com/p/103700905)

对于小文件一般是指小于128KB的文件，在Contiguous PlaceMent 条件下小文件在常规情况下的读取方式与传统的多副本类似。但是在高负载情况下和节点故障情况下需要backup request 机制保障latency，在如上5+3的模式下，一个IO，需要其他5个节点的IO进行恢复。

为了避免在5倍的IO造成对用户latency的显著放大（负载情况下慢节点拖慢整个数据读取的速度）。一般来说可以通过Backup Request的方式减少对用户即时访问的影响。window-azure 给出了RS(12+3 )、 LRC（12+2+2）等 纠删码算法情况下EC重建4KB数据的响应时间情况。从```图(a)```可以看出在低压力情况下通过RS方式重建（读取12块数据）相比直接读取的时间差不多是2.5倍(23ms VS 51ms vs)，在通过Backup Requst的方式发送15个请求选最快的12个的策略恢复数据情况下可以获得与单副本差不多的响应时间(29ms)。但是在高压力情况下，读取12块数据的响应时间相比于直接读取的响应时间是(305ms VS 91ms )，同样可以通过backup Request（12+2）的方式来使得响应时间降低到差不多与直接读取差不多的响应时间97ms。也就是说在整体IOPS能力并非瓶颈情况下，可以通过BackUp Request的机制显著降低采用EC技术方案在坏盘等故障情况下对用户IO延迟的直接影响。

![small-io-reconstruction](https://ivanzz1001.github.io/records/assets/img/ceph/erasure/small-io-reconstruction.webp)

而对于Stripe PlaceMent 情况下。如果Stripe Unit 过小，比如4KB，那么可能会导致128KB的小文件读取需要跨很多节点才能够读取完整的数据，相对来说比较费IOPS。这个时候可以适当调整条带大小，使得在正常情况下，小IO的绝大多数情况下的读取可以在单个节点读取，跨越边界情况下读取2个节点。

但是这会导致小文件需要很大的IO填充才能够进行一次写入（满条带写），空间利用率会有比较大的降低。上层直接写入的文件不适合太小，[A Bite of S3 Storage Arch](https://zhuanlan.zhihu.com/p/103700905)中说明的小文件一般来说先可以不通过3副本WAL的方式保障持久性的情况下，通过Merge成更大的MobFile EC的方式来避免文件太小。如下图所示EC 4+2组合，可以使得EC Stripe Unit大小比如为1.5MB。每个分片数据256KB的方式来使得小IO在正常情况下可以在一个节点上读取。

![small-io-stripe](https://ivanzz1001.github.io/records/assets/img/ceph/erasure/small-io-stripe.webp)

## 4. EC与大IO

在大块数据读写情况下，Contiguous PlaceMent 方案，在一般场景下跟传统的多副本策略几乎是一样的。因为数据一般来说都是临近放置，直接按照分块的放置进行直接数据读取即可。但是在异常情况下，按照Window Azure Storage 场景的测试，由于磁盘和网络带宽容易相对容易达到瓶颈，所以采用BackUp Request的并没有啥改善。

如下图所示，RS(read k) 、RS(readk +1）、RS(readk +2)、RS(readk +3) 均没有太大的改善。其发表paper的时候大概是2010年当初其网络(NIC) 大概是1Gbps。而现在其实网络越来越多的是10Gbps、50Gbps、100Gbps。所以今日不同往时，最核心的原因还是看当前系统带宽层面的带宽是否已经饱和。

![large-io-stripe](https://ivanzz1001.github.io/records/assets/img/ceph/erasure/large-io-stripe.webp)

一般情况下可以认为上层业务的大块连续IO读取都是满条带的读取，在Stripe Placement 情况下，满条带的读取在正常情况下和异常情况下从底层读取的数据量可以认为是一致的（如下图左侧图所示），而且当前一般来说EC 解码有硬件加速，即计算层面不太容易成为瓶颈，所以Stripe Placement 在正常度和异常情况下的开销基本可以认为差不多。在极端情况下，数据跨越stripe unit边界的情况下，会带来2倍的IO放大。但是在Contiguous PlaceMent 策略下，则需要更大的范围内的放大，如下EC 4+2 的策略下，可能会导致4倍的放大，在比如12+3等情况下，会有更大的放大。

![stripe-compare](https://ivanzz1001.github.io/records/assets/img/ceph/erasure/stripe-compare.webp)


## 5. 总结
上述分析了EC 结合不同的放置策略Stripe PlaceMent、Congiguous PlaceMent情况下各自的优势和缺点，在这些固有的约束条件下，需要通过合理的架构选择来充分利用EC的优点，屏蔽EC缺点以最大化EC的价值。

在当前机房网络能力越来越强的情况下（如（Flat DataCenter Storage说明），数据的locality总体来说在大多数大数据场景下越来越不重要了，存储计算分离是大趋势。比如S3（对象存储）、EBS等，可以考虑使用 RS + Stripe PlaceMent 结合合理的 Stripe Unit的方案作为底层的纠删码方案，在架构选择层面可以参考之前的2篇博文:

* [对象存储架构设计](https://zhuanlan.zhihu.com/p/103700905)

* [随机IO存储系统EBS架构设计](https://zhuanlan.zhihu.com/p/104726520)


## 5. 附录：ceph纠删码插件介绍

ceph是目前最为流行的分布式存储系统。支持副本备份模式和纠删码备份模式。使用纠删码模式进行备份能达到更高的磁盘利用率。

基于纠删码冗余策略，Ceph添加了几个开源的纠删码库，提供不同的纠删码算法，用户可根据需要选择纠删码算法类型，并创建相应的纠删码池。

OSD：object storage daemon 对象存储设备。当存储设备中有故障，可以从其他健康的设备上获取数据

chunk：数据段。可以用来重新生成原始数据。

k：chunk的数量，把原始数据分成k份

m：编码出来的chunk数量，如果m=2，表示即使丢失了2个OSD数据，也可以恢复出数据。


ceph中ec插件介绍:

1) jerasure是最广泛使用和灵活的插件。使用库代码地址：http://jerasure.org/ 参数格式如下。

<pre>
# ceph osd erasure-code-profile set {name} \
     plugin=jerasure \
     k={data-chunks} \
     m={coding-chunks} \
     technique={reed_sol_van|reed_sol_r6_op|cauchy_orig|cauchy_good|liberation|blaum_roth|liber8tion} \
     [crush-root={root}] \
     [crush-failure-domain={bucket-type}] \
     [crush-device-class={device-class}] \
     [directory={directory}] \
     [--force]
</pre>

2) isa插件, 库代码地址：https://www.intel.com/content/www/us/en/developer/topic-technology/open/overview.html

这个是Intel公司开发的一套纠删码数据存储库:
<pre>
# ceph osd erasure-code-profile set {name} \
     plugin=isa \
     technique={reed_sol_van|cauchy} \
     [k={data-chunks}] \
     [m={coding-chunks}] \
     [crush-root={root}] \
     [crush-failure-domain={bucket-type}] \
     [crush-device-class={device-class}] \
     [directory={directory}] \
     [--force]
</pre>


3) LRC插件，通过创建一些冗余的chunk，能使用更少的OSD来恢复丢失的数据

例如LRC（12,2,2）编码，将12个数据块为一组编码，并进一步将这12个数据块平均分为2个本地组， 每个本地组包括6个数据块，并分别计算出一个local parity，之后把所有12个数据块计算出2个global parities。

当发生任何一个数据块错误时，只需用本地组内的数据和校验块用于计算，即可恢复出原始数据。而恢复代价（通过网络传输的数据块数量）就由传统RS（12,4）编码的12，变为6，恢复过程的网络I/O开销减半，同时空间冗余率保持不变，仍为（12+2+2）/12 = 1.33

<pre>
ceph osd erasure-code-profile set {name} \
     plugin=lrc \
     k={data-chunks} \
     m={coding-chunks} \
     l={locality} \
     [crush-root={root}] \
     [crush-locality={bucket-type}] \
     [crush-failure-domain={bucket-type}] \
     [crush-device-class={device-class}] \
     [directory={directory}] \
     [--force]
</pre>

4) shec插件。库代码地址：http://tracker.ceph.com/projects/ceph/wiki/Shingled_Erasure_Code_(SHEC)

比Reed Solomon 编码更高效的一种方法。参数c表示真正可以允许丢失的chunk数。参数m表示有m个冗余chunk数，不保证能完全恢复。
<pre>
# ceph osd erasure-code-profile set {name} \
     plugin=shec \
     [k={data-chunks}] \
     [m={coding-chunks}] \
     [c={durability-estimator}] \
     [crush-root={root}] \
     [crush-failure-domain={bucket-type}] \
     [crush-device-class={device-class}] \
     [directory={directory}] \
     [--force]
</pre>

![erasure-shec](https://ivanzz1001.github.io/records/assets/img/ceph/erasure/erasure-shec.webp)







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




