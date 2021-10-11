---
layout: post
title: CPU体系结构之cache
tags:
- computer-basis
categories: computer-basis
description: CPU体系结构之cache
---



本文我们讲述一下CPU体系结构之cache，文章转载自[CPU体系结构之cache小结](https://blog.csdn.net/yhb1047818384/article/details/79604976)，略作修改。


<!-- more -->


## 1. What's cache?

CPU缓存(cache memory)是位于CPU和内存之间的临时存储器，它的容量比内存小但交换速度快。在缓存中的数据是内存的一小部分，但这一小部分是短时间内CPU即将访问的，当CPU调用大量数据时，就可避开内存直接从缓存中调用，从而加快读取速度。

在CPU中加入缓存是一种高效的解决方案，这样整个内存存储器(缓存 + 内存）就变成了既有缓存的高速度，又有内存的大容量的存储系统了。缓存对CPU的性能影响很大，主要是因为CPU的数据交换顺序以及CPU与缓存间的交换带宽引起的。


下图是一个典型的存储器层次结构，我们看到一共使用了三级缓存。

![memory-hierarchical](https://ivanzz1001.github.io/records/assets/img/computer_basis/memory_hierarchical.jpg)

## 2. Why should I care about cache?

![cpu-cache](https://ivanzz1001.github.io/records/assets/img/computer_basis/cpu_cache_latencies.jpg)

从延迟上看，做一次乘法一般只要三个周期，而做一次CPU的内存访问需要167个cycle，如果需要提升程序性能，减少CPU的memory访问至关重要。因此，需要采用容量小但是更快的存储器（cache）。

## 3. 为什么要有多级CPU Cache
随着科技发展，热点数据的体积越来越大，单纯的增加一级缓存大小的性价比已经很低了。

二级缓存就是一级缓存的缓冲器：一级缓存制造成本很高因此它的容量有限，二级缓存的作用就是存储那些CPU处理时需要用到、一级缓存又无法存储的数据。

同样道理，三级缓存和内存可以看作是二级缓存的缓冲器，它们的容量递增，但单位制造成本却递减。

另外需要注意的是，L3 Cache和L1，L2 Cache有着本质的区别。L1和L2 Cache都是每个CPU core独立拥有一个，而L3 Cache是几个Cores共享的，可以认为是一个更小但是更快的```内存```(memory)。

![cpu-cache](https://ivanzz1001.github.io/records/assets/img/computer_basis/cache_latency.jpg)

使用dmidecode命令查看cache size:
<pre>
# dmidecode -t cache
# dmidecode 3.0
Scanning /dev/mem for entry point.
SMBIOS 2.7 present.

Handle 0x0014, DMI type 7, 19 bytes
Cache Information
        Socket Designation: CPU Internal L2
        Configuration: Enabled, Not Socketed, Level 2
        Operational Mode: Write Back
        Location: Internal
        Installed Size: 1024 kB
        Maximum Size: 1024 kB
        Supported SRAM Types:
                Unknown
        Installed SRAM Type: Unknown
        Speed: Unknown
        Error Correction Type: Single-bit ECC
        System Type: Unified
        Associativity: 8-way Set-associative

Handle 0x0015, DMI type 7, 19 bytes
Cache Information
        Socket Designation: CPU Internal L1
        Configuration: Enabled, Not Socketed, Level 1
        Operational Mode: Write Back
        Location: Internal
        Installed Size: 256 kB
        Maximum Size: 256 kB
        Supported SRAM Types:
                Unknown
        Installed SRAM Type: Unknown
        Speed: Unknown
        Error Correction Type: Single-bit ECC
        System Type: Other
        Associativity: 8-way Set-associative

Handle 0x0016, DMI type 7, 19 bytes
Cache Information
        Socket Designation: CPU Internal L3
        Configuration: Enabled, Not Socketed, Level 3
        Operational Mode: Write Back
        Location: Internal
        Installed Size: 8192 kB
        Maximum Size: 8192 kB
        Supported SRAM Types:
                Unknown
        Installed SRAM Type: Unknown
        Speed: Unknown
        Error Correction Type: Single-bit ECC
        System Type: Unified
        Associativity: 16-way Set-associative
		
# lscpu
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                8
On-line CPU(s) list:   0-7
Thread(s) per core:    2
Core(s) per socket:    4
座：                 1
NUMA 节点：         1
厂商 ID：           GenuineIntel
CPU 系列：          6
型号：              60
型号名称：        Intel(R) Xeon(R) CPU E3-1246 v3 @ 3.50GHz
步进：              3
CPU MHz：             1781.308
BogoMIPS：            6984.00
虚拟化：           VT-x
L1d 缓存：          32K
L1i 缓存：          32K
L2 缓存：           256K
L3 缓存：           8192K
NUMA 节点0 CPU：    0-7
</pre>

通过lscpu命令我们看到，当前主机有1个CPU socket， 每个CPU socket有4个core，每个core有2个线程。上述```dmidecode```命令得出的L1、L2似乎是4个物理CPU的cache总和:
{% highlight string %}
sum(L1 cache) = 4 *(L1d + L1i) = 256KB
sum(L2 cache) =  4 * L2 = 4 * 256KB = 1024KB
{% endhighlight %}
>Tips: 上述```L1d```和```L1i```分别为L1数据缓存(Data cache)和L1指令缓存(Instruction cache)


## 4. cpu与cache 内存交互的过程
CPU接收到指令后，它会最先向CPU中的一级缓存（L1 Cache）去寻找相关的数据，虽然一级缓存是与CPU同频运行的，但是由于容量较小，所以不可能每次都命中。这时CPU会继续向下一级的二级缓存（L2 Cache）寻找，同样的道理，当所需要的数据在二级缓存中也没有的话，会继续转向L3 Cache、内存(主存)和硬盘.

程序运行时可以使用perf工具观察cache-miss的rate.

## 5. 什么是cache line

Cache Line可以简单的理解为CPU Cache中的最小缓存单位。内存与高速缓存之间或高速缓存与高速缓存之间的数据移动不是以单个字节或甚至word完成的。相反，移动的最小数据单位称为缓存行，有时称为缓存块。


目前主流的CPU Cache的Cache Line大小都是64Bytes。假设我们有一个512字节的一级缓存(L1)，那么按照64B的缓存单位大小来算，这个一级缓存所能存放的Cache Line个数就是512/64 = 8个。

可以通过执行如下命令来查看cache line的大小：
<pre>
# tree /sys/devices/system/cpu/cpu0/cache
/sys/devices/system/cpu/cpu0/cache
├── index0
│   ├── coherency_line_size
│   ├── level
│   ├── number_of_sets
│   ├── physical_line_partition
│   ├── shared_cpu_list
│   ├── shared_cpu_map
│   ├── size
│   ├── type
│   └── ways_of_associativity
├── index1
│   ├── coherency_line_size
│   ├── level
│   ├── number_of_sets
│   ├── physical_line_partition
│   ├── shared_cpu_list
│   ├── shared_cpu_map
│   ├── size
│   ├── type
│   └── ways_of_associativity
├── index2
│   ├── coherency_line_size
│   ├── level
│   ├── number_of_sets
│   ├── physical_line_partition
│   ├── shared_cpu_list
│   ├── shared_cpu_map
│   ├── size
│   ├── type
│   └── ways_of_associativity
└── index3
    ├── coherency_line_size
    ├── level
    ├── number_of_sets
    ├── physical_line_partition
    ├── shared_cpu_list
    ├── shared_cpu_map
    ├── size
    ├── type
    └── ways_of_associativity

4 directories, 36 files

# cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size
64

# cat /sys/devices/system/cpu/cpu0/cache/index0/size
32K
# cat /sys/devices/system/cpu/cpu0/cache/index1/size
32K
# cat /sys/devices/system/cpu/cpu0/cache/index2/size
256K
# cat /sys/devices/system/cpu/cpu0/cache/index3/size
8192K
</pre>


1) **cache line的影响**

{% highlight string %}
for (int i = 0; i < N; i+=k) 
    arr[i] *= 3;
{% endhighlight %}


![cpu-cache-line](https://ivanzz1001.github.io/records/assets/img/computer_basis/cache_line.jpg)

注意当步长在1到16范围内，循环运行时间几乎不变。但从16开始，每次步长加倍，运行时间减半。

这是由于16个整型数占用64字节（一个缓存行），for循环步长在1到16之间必定接触到相同数目的缓存行：即数组中所有的缓存行。当步长为32，我们只有大约每两个缓存行接触一次；当步长为64，只有每四个接触一次。

## 6. cache写机制

Cache写机制分为write through和write back两种。

* Write-through： Write is done synchronously both to the cache and to the backing store.

* Write-back (or Write-behind)： Writing is done only to the cache. A modified cache block is written back to the store, just before it is replaced.

Write-through（直写模式）在数据更新时，同时写入缓存Cache和后端存储。此模式的优点是操作简单；缺点是因为数据修改需要同时写入存储，数据写入速度较慢。

Write-back（回写模式）在数据更新时只写入缓存Cache。只在数据被替换出缓存时，被修改的缓存数据才会被写到后端存储。此模式的优点是数据写入速度快，因为不需要写存储；缺点是一旦更新后的数据未被写入存储时出现系统掉电的情况，数据将无法找回。

## 7. cache 一致性
多个处理器对某个内存块同时读写，会引起冲突的问题，这也被称为Cache一致性问题。

Cache一致性问题出现的原因是在一个多处理器系统中，多个处理器核心都能够独立地执行计算机指令，从而有可能同时对某个内存块进行读写操作，并且由于我们之前提到的回写和直写的Cache策略，导致一个内存块同时可能有多个备份，有的已经写回到内存中，有的在不同的处理器核心的L1、L2 Cache中。由于Cache缓存的原因，我们不知道数据写入的时序性，因而也不知道哪个备份是最新的。

此外，还有另一种可能，假设有两个线程A和B共享一个变量，当线程A处理完一个数据之后，通过这个变量通知线程B，然后线程B对这个数据接着进行处理，如果两个线程运行在不同的处理器核心上，那么运行线程B的处理器就会不停地检查这个变量，而这个变量存储在本地的Cache中，因此就会发现这个值总也不会发生变化。

为了正确性，一旦一个核心更新了内存中的内容，硬件就必须要保证其他的核心能够读到更新后的数据。目前大多数硬件采用的策略或协议是MESI或基于MESI的变种：

* M代表更改(modified): 表示缓存中的数据已经更改，在未来的某个时刻将会写入内存；

* E代表排除(exclusive): 表示缓存的数据只被当前的核心所缓存；

* S代表共享(shared): 表示缓存的数据还被其他核心缓存；

* I代表无效(invalid): 表示缓存中的数据已经失效，即其他核心更改了数据

## 8. cache的局部性
程序在一段时间内访问的数据通常具有```局部性```，比如对一维数组来说，访问了地址x上的元素，那么以后访问地址x+1、x+2上元素的可能性就比较高；现在访问的数据，在不久之后再次被访问的可能性也比较高。局部性分为```“时间局部性”```和```“空间局部性”```，时间局部性是指当前被访问的数据随后有可能访问到；空间局部性是指当前访问地址附近的地址可能随后被访问。处理器通过在内存和核心之间增加缓存以利用局部性增强程序性能，这样可以用远低于缓存的价格换取接近缓存的速度。


1) **时间局部性**

* 代码1
{% highlight string %}
for (loop=0; loop<10; loop++) {
    for (i=0; i<N; i++) {
        ... = ... x[i] ...
    }
}
{% endhighlight %}

* 代码2
{% highlight string %}
for (i=0; i<N; i++) {
    for (loop=0; loop<10; loop++) {
        ... = ... x[i] ...
    }
}
{% endhighlight %}

代码二的性能优于代码一，x的元素现在被重复使用，因此更有可能留在缓存中。 这个
重新排列的代码在使用x[i]时显示更好的时间局部性。

2）**空间局部性**

一个矩阵乘法的例子：

* 代码1
{% highlight string %}
for i=1..n
    for j=1..n
        for k=1..n
            c[i,j] += a[i,k]*b[k,j]
{% endhighlight %}


* 代码2
{% highlight string %}
for i=1..n
    for k=1..n
        for j=1..n
            c[i,j] += a[i,k]*b[k,j]
{% endhighlight %}

代码2的性能优于代码一的性能。

两者实现上的差异如下图所示：

![space-locality](https://ivanzz1001.github.io/records/assets/img/computer_basis/space_locality.png)

```代码2```的b[k,j]是按行访问的，所以存在良好的空间局部性，cache line被充分利用。而```代码1```中，b [k，j]由列访问。 由于行的存储矩阵，因此对于每个缓存行加载，只有一个元素用于遍历。


## 9. cache替换策略
Cache工作原理要求它尽量保存最新数据，当从主存向Cache传送一个新块，而Cache中可用位置已被占满时，就会产生Cache替换的问题。

常用的替换算法有下面三种：

1） **LFU**

LFU（Least Frequently Used，最不经常使用）算法将一段时间内被访问次数最少的那个块替换出去。每块设置一个计数器，从0开始计数，每访问一次，被访块的计数器就增1。当需要替换时，将计数值最小的块换出，同时将所有块的计数器都清零。

这种算法将计数周期限定在对这些特定块两次替换之间的间隔时间内，不能严格反映近期访问情况，新调入的块很容易被替换出去。


2) **LRU**

LRU（Least Recently Used，近期最少使用）算法是把CPU近期最少使用的块替换出去。这种替换方法需要随时记录Cache中各块的使用情况，以便确定哪个块是近期最少使用的块。每块也设置一个计数器，Cache每命中一次，命中块计数器清零，其他各块计数器增1。当需要替换时，将计数值最大的块换出。

LRU算法相对合理，但实现起来比较复杂，系统开销较大。这种算法保护了刚调入Cache的新数据块，具有较高的命中率。LRU算法不能肯定调出去的块近期不会再被使用，所以这种替换算法不能算作最合理、最优秀的算法。但是研究表明，采用这种算法可使Cache的命中率达到90%左右。

3) **随机替换**

最简单的替换算法是随机替换。随机替换算法完全不管Cache的情况，简单地根据一个随机数选择一块替换出去。随机替换算法在硬件上容易实现，且速度也比前两种算法快。缺点则是降低了命中率和Cache工作效率。



## 10. cache映射

主存与cache的地址映射方式有全相联方式、直接方式和组相联方式三种。

1） **直接映射**

将一个主存块存储到唯一的一个Cache行。多对一的映射关系，但一个主存块只能拷贝到cache的一个特定行位置上去。


cache的```行号i```和主存的```块号j```有如下函数关系：i=j mod m（m为cache中的总行数）

![direct-mapping](https://ivanzz1001.github.io/records/assets/img/computer_basis/direct_mapping.jpg)


优点：硬件简单，容易实现

缺点：命中率低， Cache的存储空间利用率低


2) **全相联映射**

可以将一个主存块存储到任意一个Cache行。

主存的一个块直接拷贝到cache中的任意一行上。


![full-mapping](https://ivanzz1001.github.io/records/assets/img/computer_basis/full_mapping.jpg)

优点：命中率较高，Cache的存储空间利用率高

缺点：线路复杂，成本高，速度低

3) **组相联**

组相联映射实际上是直接映射和全相联映射的折中方案，其组织结构如下图所示。*主存和Cache都分组，主存中一个组内的块数与Cache中的分组数相同，组间采用直接映射，组内采用全相联映射*。也就是说，将Cache分成2^u组，每组包含2^v块，主存块存放到哪个组是固定的，至于存到该组哪一块则是灵活的。也即主存的某块只能映射到Cache的特定组中的任意一块。

主存的某块b与Cache的组k之间满足以下关系：k=b%(2^u)


例如，主存分为256组，每组8块，Cache分为8组，每组2块。


![group-mapping](https://ivanzz1001.github.io/records/assets/img/computer_basis/group_mapping.jpg)



组间采用直接映射，组内为全相联

硬件较简单，速度较快，命中率较高


<br />
<br />

**参看:**

1. [大话处理器(书籍)](http://www.baidu.com)

2. [CPU体系结构之cache小结](https://blog.csdn.net/yhb1047818384/article/details/79604976)

3. [如何利用CPU Cache写出高性能代码](https://blog.csdn.net/melody157398/article/details/117887454)

4. [Go调度：Part I - OS调度程序](https://zhuanlan.zhihu.com/p/141540756)

<br />
<br />
<br />

