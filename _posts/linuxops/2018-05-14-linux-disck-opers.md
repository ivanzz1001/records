---
layout: post
title: linux中硬盘相关操作
tags:
- LinuxOps
categories: linuxOps
description: linux中硬盘相关操作
---


本节我们介绍一下Linux中硬盘的相关操作： 查看硬盘信息、分区、硬盘检测等


<!-- more -->




## 1. sgdisk用法

**1) 硬盘格式化**

下面删除硬盘/dev/sda上面的所有分区信息。
<pre>
#  sgdisk -Zog /dev/sda
</pre>

**2) 修复硬盘**
<pre>
# xfs_repair /dev/sda1
</pre>




<br />
<br />

**[参看]**

1. [SGDISK 的使用](http://www.rodsbooks.com/gdisk/sgdisk.html)

2. [sgdisk基本用法](https://blog.csdn.net/ygtlovezf/article/details/76269800)

3. [sgdisk常用操作](http://hustcat.github.io/sgdisk-basic/)

2. [GNU Parted的使用](https://wiki.archlinux.org/index.php/GNU_Parted)



<br />
<br />
<br />


