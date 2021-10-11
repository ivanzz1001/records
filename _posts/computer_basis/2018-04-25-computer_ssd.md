---
layout: post
title: ssd硬盘为什么会越用越慢(转)
tags:
- computer-basis
categories: computer-basis
description: ssd硬盘为什么会越用越慢
---


本文转自[终于知道为什么SSD会越用越慢，解决方法来了！](https://baijiahao.baidu.com/s?id=1656879706211188827)，在此做一个记录，防止文章丢失，也方便查阅。


每当选购电脑时，你会发现，配置存储介绍总是不一样，要么纯固态（SSD），要么纯机械，要么固态+机械的组合硬盘。

两者的主要区别在于固态是半导体存储，机械则是电磁存储。固态硬盘最大读取速度在400-600MB/s，而机械硬盘的最大读取速度不超过200MB/s。




<!-- more -->

虽然SSD的速度比机械硬盘要快上许多，但是它有一个比较严重的问题，那就是用久之后，会明显感觉速度下降了。这到底是什么原因呢？

![disk-slot](https://ivanzz1001.github.io/records/assets/img/computer_basis/disk_slot.jpg)

## 1. 什么是SSD？
首先，我们要先了解SSD的基本概念。**固态硬盘指的是用固态电子存储芯片阵列而制成的硬盘。它由控制单元和存储单元组成**。

目前，市面上的固态硬盘分为两大类，第一类是采用```闪存```（FLASH芯片）作为存储介质，第二类则是采用```DRAM```为存储介质。

基于闪存类的固态硬盘，也就是通常所说的SSD。这种硬盘的适用范围非常广泛，笔记本硬盘、U盘、存储卡等都属于SSD。

**SSD主要由主控、闪存以及缓存构成**。其中，闪存起到了关键性作用，因为它是负责存储数据的闪存颗粒，很大程度上决定了SSD的性能寿命。

![disk-ssd](https://ivanzz1001.github.io/records/assets/img/computer_basis/disk_ssd.jpg)

**闪存是指一种电子式可清除程序化只读存储器的形式，允许在操作中被多次擦或写的存储器。**

目前闪存颗粒有四种类型，分别是```SLC```(Single-Level Cel)、```MLC```(Multi-Level Cel)、```TLC```(Trinary-Level Cell)、```QLC```(Quad-Level Cell)。SLC闪存的一个存储单元只能存储1bit数据，换句话说就是只能存储一个0或者一个1，一共两种状态。MLC闪存的一个存储单元可以存储2bit数据，0和1可以排列组合，拥有00、01、10、11四种状态。以此类推，QLC是最新的闪存颗粒，它的存储容量是SLC的8倍，0和1可以组成16种状态。

![flash-type](https://ivanzz1001.github.io/records/assets/img/computer_basis/flash_type.jpg)

虽然SSD的闪存容量在不断增加，但是擦除和写入次数却减少了。这也就意味着，SSD的使用寿命缩短了。与QLC相比，SLC的使用寿命是它的100倍。

![flash-erase-speed](https://ivanzz1001.github.io/records/assets/img/computer_basis/flash_erase_speed.jpg)

除闪存外，SSD的接口也分为四类，它们是```SATA```、```mSATA```、```M.2```和```PCI-E```


1) **SATA接口**

SATA是一种电脑总线，分别有SATA 1.5Gbit/s、SATA 3Gbit/s和SATA 6Gbit/s三种规格，读写速度依次为150MB/s、300MB/s、600MB/s，适用于几乎所有台式机和笔记本。

2) **mSATA接口**

mSATA是迷你版本SATA接口，拥有M50 msata和M30 msata两种规格，读写速度均为520MB/s，主要适用于超极本，比如联想的E220s、E420s、Y460等。


3) **M.2接口**

M.2是Intel推出的一种替代mSATA新的接口规范，它有两种类型：Socket 2和Socket 3。Socket 2最大的读取速度可以达到700MB/s，而Socket 3的理论带宽可达4GB/s。Socket 2适用于部分中低端笔记本，Socket 3几乎适用于新上市的台式主板和中高端笔记本。

4）**PCI-E接口**

PCI—E是一种高速串行计算机总线，它有5个版本，从1.0到最新的5.0，读写速度分别为250MB/s、500MB/s、984.6MB/s、1969MB/s以及3938MB/s，适用于几乎所有的台式机。

![disk-interface](https://ivanzz1001.github.io/records/assets/img/computer_basis/disk_interface.jpg)

综合来看，SSD闪存颗粒的不同，它的读写速度和擦除次数也就不同，容量越大的闪存，使用寿命就越有限；SSD接口的不同，直接应影响的是读写速度。


## 2. 写入放大和垃圾回收

不过，以上两种原因影响的是SSD使用前的速度，使用后速度下降的原因主要分为两个方面：```写入放大```（WA）和```垃圾回收```（GC）。

**WA是闪存和SSD中的一种不良现象，即实际写入的物理数据量是写入数据量的多倍**。由于FLASH芯片需要先擦除再改写的特性，它需要将目标数据所在的文稿整个读出来缓冲到缓存器中，然后再将你要改写的数据覆盖到缓存器，最后将缓存器写入到另一个文稿中，所以，哪怕你更新一个字节，实际上写入了4KB的数据到FLASH芯片。这也就造成了SSD写入文件大小翻倍的现象，从而导致硬盘容量越来越小，写入速度下降。

![write-enlarge](https://ivanzz1001.github.io/records/assets/img/computer_basis/write_enlarge.jpg)

**GC是当FLASH芯片中再也找不到可以直接写入的空白文稿(注：即page)时，JVW（java虚拟机）将调用垃圾回收机制来回收内存空间**。GC会在FLASH芯片中查找废弃或删除的文稿，然后将这个文稿擦掉，用来存放你的新数据，但由于GC与FLASH的擦写单位不一致，写入时是按照文稿进行的，而擦除则是按照Block文件类型，一个Block中有16个文稿，所以当GC找到一个废弃文稿时，实际上改文稿需要先把Block中的其余15个文件搬到别的地方。因此每当你更新1字节，实际在后台有16个文稿写入，一个文稿大小为4KB，16个文稿就是64KB，这样SSD的容量也会成倍减少，最终导致写入速度下降。

![ssd-write](https://ivanzz1001.github.io/records/assets/img/computer_basis/ssd_write.jpg)

简单点来说，当你的SSD存储数据变多时，空白的文稿会随之变少，为了腾出额外的空间，GC会寻找废弃的文稿，随着次数的增加，SSD的速度自然也就慢了。

另外，GC并不是需要写入数据时才出动，即使SSD处于空闲状态，它也会悄悄地进行操作，提前预留一些空间。

那如何避免SSD因存储数据过多导致速度下降的问题？

厂商的做法是根据SSD等级来制定不同的解决方案，比如：入门级SSD通过缩小可用容量，预留出一部分空间，以此来防止SSD完全写满。最为直观的例子就是有些电脑明明写着512GB的存储容量，实际能用的就只有480GB。

对于中高端的SSD，它们会额外搭载缓存降低写入放大。像```三星970 PRO NVMe M.2 1T```就配备了4GB的缓存容量。

![samsung-ssd](https://ivanzz1001.github.io/records/assets/img/computer_basis/ssd_970pro.jpg)

用户可以通过4K对齐、Trim命令、磨损均衡等功能机制来降低写入放大。

## 3. SSD爆容量对速度的影响

为了进一步确认SSD容量满后是否真的影响速度，我们进行了测试。

1) **PC端SSD测试**

测试前，```MacBook Air（2018款）```的SSD剩余容量为71.58GB，使用Disk Speed Test测出硬盘的写入速度为492.1MB/s，读取速度为1832MB/s。

![speed-test](https://ivanzz1001.github.io/records/assets/img/computer_basis/disk_speed_test1.jpg)

接下来，我们要将电脑的可用容量变小，预留9.02GB的空间，然后再测一次SSD的速度。此时，SSD的写入速度变成了448.2MB/s，读取速度为1791MB/s。

与前一组数据进行对比你会发现，SSD随着可用容量的减小，写入速度和读取速度都有所下降。

![speed-test](https://ivanzz1001.github.io/records/assets/img/computer_basis/disk_speed_test2.jpg)

2）**移动端SSD测试**

PC端测完之后，我们再来看下移动端。MoFirLee手上有一部容量为64GB的iPhone 11，硬盘的剩余容量为27.2GB，使用DiskBench测得的硬盘写入速度为201.7MB/s、142.1MB/s、135.2MB/s，平均写入速度159.7MB/s,读取速度为360.6MB/s、485.9MB/s、387.1MB/s，平均读取速度411.2MB/s。

![speed-test](https://ivanzz1001.github.io/records/assets/img/computer_basis/disk_speed_test3.jpg)

手机硬盘可用容量变为1.17GB后，写入速度为183.7MB/s、159.6MB/s、195.5MB/s，平均写入速度160.6MB/s，读取速度为609.7MB/s、610.3MB/s、510.6MB/s，平均读取速度510.7MB/s。

相比上组数据，第一次的硬盘写入速度确实有所下降，但第二、三次的数据要比上一组快，三次的读取速度都比上组快。出现这种情况的原因可能是主控的算法和优化，所以测得的数据有所差异。

![speed-test](https://ivanzz1001.github.io/records/assets/img/computer_basis/disk_speed_test4.jpg)

最终的得出的结论是当SSD可用容量变小后，写入和读取速度都会有所下降，但实际上主控对这些可能还有其他优化或者控制措施，以及有不同的算法，导致差异。

## 4. 小结

如今可以用8个字来形容SSD的处境，有一种倒退叫进步。目前市面上的SSD，制程工艺越先进，寿命反而缩短，以此换来了更快的速度。从侧边可以看出，厂商们在现有技术无法突破的情况下，选择了牺牲一些东西。

使用前，影响SSD速度的因素有```闪存颗粒```、```接口类型```、```主控的好坏以及升级固件```。使用后，写入放大、垃圾回收机制和不良的存储习惯是影响SSD速度的主要原因。虽然好的SSD速度很快，但是使用寿命会下降，价格也会偏高。便宜的SSD速度一般，使用寿命却很长。至于怎么选，完全看个人需求。

为了避免SSD因可用容量不足，导致速度下降这一问题，最好将每个硬盘的资料全部备份起来，然后再做一个4K对齐。如果你是刚买的电脑，硬盘类型是固体的话，可以在使用前，将SSD进行高级格式化。这样一来，你的SSD速度就不会那么容易下降了。



<br />
<br />

**参看:**



1. [终于知道为什么SSD会越用越慢，解决方法来了！](https://baijiahao.baidu.com/s?id=1656879706211188827)

2. [SLC,MLC,TLC,QLC是什么意思？](https://zhuanlan.zhihu.com/p/50228125)

3. [一篇文章讲清什么是NVMe](https://baijiahao.baidu.com/s?id=1637421199219751340&wfr=spider&for=pc)



<br />
<br />
<br />

