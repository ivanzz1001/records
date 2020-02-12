---
layout: post
title: 解决Linux系统buff/cache过大的问题(转)
tags:
- LinuxOps
categories: linuxOps
description: Linux系统buff/cache过大的问题
---



发现这个问题是因为项目中开始时加载库有时候很快有时候又很慢，才发现这个问题。linux是先将库文件读到cache中去的，所以二次加载时会很快，造成时间不定。

<!-- more -->

## 1. linux系统的buff/cache
在Linux系统中，我们经常用free命令来查看系统内存的使用状态。在一个centos7系统上，free命令的显示内容大概是这样一个状态：
<pre>
# free -m
              total        used        free      shared  buff/cache   available
Mem:           2012         418         509           9        1084        1339
Swap:          2044           0        2044
</pre>
```free```命令的默认显示单位是KB，这里使用```-m```选项表示显示单位是MB。这里我们的服务器是2GB的内存。这个命令几乎是每一个使用过Linux的人必会的命令，但越是这样的命令，似乎真正明白的人越少（我是说比例越少)。

根据目前网络上技术文档的内容，大家普遍认为，buffers和cache所占用的内存空间是可以在内存压力较大的时候被释放当做空闲空间用的。但真的是这样么？在论证这个题目之前，我们先简要介绍一下```buff```和```cache```的意思。

### 1.1 什么是buff/cache
buffer和cache是两个在计算机技术中被滥用的名词，放在不同语境下会与不同的意义。在Linux的内存管理中，这里的```buff```是指Linux内存的```Buffer cache```；而这里的```cache```是指Linux内存的```Page cache```。翻译成中文可以叫做**缓冲区缓存**和**页面缓存**。

在历史上，它们一个(buff)被用来当作对io设备写的缓存，而另一个(cache)被用来当做对IO设备读的缓存。这里的IO设备，主要指的是块设备文件和文件系统上的普通文件。但是现在，它们的意义已经不一样了。在当前的内核中，**```page cache```顾名思义就是针对内存页的缓存，说白了就是，如果有内存是以page进行分配管理的，都可以使用page cache作为其缓存来管理使用**。当然，不是所有的内存都是以页（page）进行管理的，也有很多是**针对块(block)进行管理的，这部分内存使用如果要用到cache功能，则都集中到buffer cache中来使用**（作者注： 从这个角度出发，buffer cache是不是改名叫做block cache更好？）。然而，也不是所有块(block)都有固定长度，系统上块的长度主要是根据所使用的块设备决定的，而页长度在x86上无论是32位还是64位都是4K。

明白了两套缓存系统的区别，就可以理解他们究竟都可以用来做什么了。

### 1.2 什么是page cache 
page cache主要用来作为*文件系统上文件数据的缓存*来用，尤其是针对当进程对文件有read/write操作的时候。如果你仔细想想的话，作为可以映射文件到内存的系统调用: mmap()是不是很自然的也应该用到page cache？在当前的系统实现里，page cache也被作为其他类型的缓存设备来用，所以事实上page cache也负责了大部分的块设备文件的缓存工作。

### 1.3 什么是buffer cache
buffer cache主要是设计*用来在系统对块设备进行读写的时候，对块数据进行缓存*。这意味着某些对块的操作会使用buffer cache进行缓存,比如我们在格式化文件系统的时候。一般情况下两个缓存系统是一起配合使用的，比如当我们对一个文件进行写操作的时候，page cache的内容会被改变，而buffer cache则可以用来将page标记为不同的缓冲区，并记录是哪一个缓冲区被修改了。这样，内核在后续执行脏数据的回写(writeback)时，就不用将整个page写回，只需要写回修改的部分即可。

### 1.4 如何回收cache？
Linux内核会在内存将要耗尽的时候，触发内存回收的工作，以便释放出内存给急需内存的进程使用。一般情况下，这个操作中主要的内存释放都来自于```buff/cache```的释放，尤其是被使用更多的page cache空间。既然它主要用来做缓存，只是在内存够用的时候加快进程对文件的读写速度，那么在内存压力较大的情况下，当然有必要清空释放cache，以作为free空间分给相关进程使用。所以一般情况下，我们认为buffer/cache空间可以被释放，这个理解是正确的。

但是这种清缓存的工作也并不是没有成本。理解cache是干什么的就可以明白*清缓存必须保证cache中的数据跟对应文件中的数据一致，才能对cache进行释放*。所以伴随着cache清除行为的，一般都是系统IO飙高。因为内核要比对cache中的数据和对应硬盘文件上的数据是否一致，如果不一致则需要写回(writeback)，之后才能回收。

在系统中，除了内存将被耗尽的时候可以清缓存以外，我们还可以使用下面的方式来人工触发缓存清除的动作：
<pre>
# cat /proc/sys/vm/drop_caches
0
# sync; echo 1 > /proc/sys/vm/drop_caches
</pre>
上面注意，我们需要在触发缓存清除动作前先执行```sync```命令。

当然，我们也可以向*/proc/sys/vm/drop_caches*这个文件写入不同的值，它们所表示的含义分别为：
<pre>
//写入1： 表示清除page cache
# sync; echo 1 > /proc/sys/vm/drop_caches


//写入2： 表示清除回收slab分配器中的对象（包括目录项缓存和inode缓存）。slab分配器是内核中管理内存的一种机制
# sync; echo 2 > /proc/sys/vm/drop_caches 


//写入3： 表示清除page cache和slab 分配器中的缓冲对象
# sync; echo 3 > /proc/sys/vm/drop_caches 
</pre>


<br />
<br />

**[参看]:**

1. [解决Linux系统buff/cache过大的问题](https://blog.csdn.net/u013427969/article/details/83315104)



<br />
<br />
<br />


