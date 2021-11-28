---
layout: post
title: 常见硬盘IOPS参考值(转）
tags:
- LinuxOps
categories: linux
description: 常见硬盘IOPS参考值
---


本文介绍一下常见硬盘的IOPS参考值，在此做个记录，以便后续查阅。


<!-- more -->

## 1. 硬盘IOPS
IOPS(全称:IO per second)，即每秒读写(IO)操作的次数，多用于OLTP/数据库、小文件存储等场合，衡量随机访问的性能。

关于IOPS，请参看[《磁盘性能指标--IOPS理论》](https://www.cnblogs.com/zengkefu/p/5634299.html)

## 2. 常见硬盘IOPS参考
常见硬盘IOPS参考如下：
<pre>
尺寸        转速           硬盘类型(数据传输协议)            IOPS
-----------------------------------------------------------------
2.5      10000rpm                SAS                      113 
2.5      15000rpm                SAS                      156 
3.5      15000rpm                SAS                      146 
　　
2.5      5400rpm                 SATA                     71 
3.5      7200rpm                 SATA                     65 
　　
3,5      10000rpm              SCSI(U320)                 104 
3,5      15000rpm              SCSI(U320)                 141 
　　
3.5      10000rpm               FC                        125 
3.5      15000rpm               FC                        150
　　
3.5      10000rpm               FATA                      119 
</pre>

## 3. 三星830系列(SSD)IOPS指标

* 三星（SAMSUNG）830系列 512G 2.5英寸 SATA-3固态硬盘(MZ-7PC512B/WW) Basic Kit ----3500元


<pre>
特性：

连续读取: 最大520 MB/秒 
连续写入: 最大400MB/秒
随机读取: 最大80000IOPS 
随即写入: 最大36000IOPS
</pre>

* 三星（SAMSUNG）830系列 128G 2.5英寸 SATA-3固态硬盘(MZ-7PC128B/WW)----670元
<pre>
特性：

连续读取: 最大520MB/秒 
连续写入: 最大320MB/秒 
随机读取: 最大80000IOPS 
随机写入: 最大30000IOPS
</pre>

* 三星（SAMSUNG）830系列 64G 2.5英寸 SATA-3固态硬盘(MZ-7PC064B/WW) ----450元
<pre>
特性:

连续读取: 最大520MB/秒 
连续写入: 最大160MB/秒 
随机读取: 最大75000 IOPS 
随机写入: 最大16000 IOPS
</pre>

测评文章：

[超越550MB/秒 三星512G固态硬盘评测](https://memory.zol.com.cn/260/2600260_all.html)

[SSD潜在杀手 三星830固态硬盘评测](https://lcd.zol.com.cn/288/2880622_all.html)

![samsung-830](https://ivanzz1001.github.io/records/assets/img/linux/samsung-disk-830.png)

>注：上图中的QD是队列深度，实际情况队列深度不会超过4，所以队列深度为1最有参考价值。

## 4. 其他厂家硬盘IOPS参考


* 美光（Crucial）M4系列 128G 2.5英寸 SATA-3固态硬盘(CT128M4SSD2)----780元
<pre>
特性：

连续读取：500 MB/sec(SATA 6Gb/s)
连续写入：175 MB/sec (SATA 6Gb/s)
随机读取：4k 45,000 IOPS
随机写入：4k 20,000 IOPS
</pre>

* 美光（Crucial）M4系列 64G 2.5英寸 SATA-3固态硬盘(CT064M4SSD2) ----460元
<pre>
特性：

连续读取: (up to) 500 MB/sec (SATA 6Gb/s)
连续写入: (up to) 95 MB/sec (SATA 6Gb/s)
随机读取: 4k 45,000 IOPS
随机写入: 4k 20,000 IOPS
</pre>

* 英特尔 Intel SSDSC2CT120A3K5-CBOX 120G 固态硬盘330 系列 ----1000元
<pre>
特性：

顺序读取：（最高）500 MB/秒
连续写入： (最高) 400 MB/秒
随机读取: 4KB 42,000 IOPS
随机写入: 4KB 52,000 IOPS
</pre>


* 英特尔 Intel SSDSC2CT060A3K5-CBOX 60G 固态硬盘330 系列 ----470元
<pre>
特性：

顺序读取：（最高）500 MB/秒
连续写入： (最高) 400 MB/秒
随机读取: 4KB 42,000 IOPS
随机写入: 4KB 52,000 IOPS
</pre>

<br />
<br />
**[参看]:**

1. [常见硬盘IOPS参考值](https://blog.csdn.net/iteye_10774/article/details/82606323)

<br />
<br />
<br />





