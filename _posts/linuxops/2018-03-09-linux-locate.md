---
layout: post
title: linux操作系统中locate命令的使用
tags:
- LinuxOps
categories: LinuxOps
description: linux操作系统中locate命令的使用
---

本章我们主要谈一谈Linux操作系统下locate命令的使用.



<!-- more -->


## 1. locate命令

locate命令用于在文件系统中通过名字来查找文件。

### 1.1 简述

```locate```命令用于查找文件或目录。locate命令要比```find -name```快得多，原因在于它不搜索具体目录，而是搜索一个数据库/var/lib/mlocate/mlocate.db。这个数据库中含有本地所有文件信息。Linux系统自动创建这个数据库，并且每天自动更新一次。因此，我们在用whereis 和 locate查找文件时，有时会找到已经被删除的数据；或者刚刚建立的文件，却无法查找到，原因就是因为数据库文件没有被更新。为了避免这种情况，可以在使用locate之前，先使用```updatedb```命令，手动更新数据库。整个locate工作其实是由4部分组成的：

* /usr/bin/updatedb: 主要用来更新数据库，通过crontab自动完成的

* /usr/bin/locate: 查询文件位置

* /etc/updatedb.conf: updatedb的配置文件

* /var/lib/mlocate/mlocate.db: 存放文件信息的文件

 
```locate```会读取由updatedb准备好的一个或多个数据库，然后将满足匹配```PATTERN```的文件写到标准输出，每行一个文件名。假如并未指定```--regex```选项，则```PATTERN```可以包含通配符。假如```PATTERN```中并未包含任何通配符，则locate命令以```*PATTERN*```模式进行查找。

默认情况下，locate命令并不会检查数据库中的文件是否仍然存在，也不会报告在上一次更新数据库之后产生的文件。







<br />
<br />
**[参看]:**

1. [Linux下which、whereis、locate、find 命令的区别](http://blog.chinaunix.net/uid-20554039-id-3035417.html)

2. [每天一个linux命令:locate](https://www.cnblogs.com/xqzt/p/5426666.html)

3. [Linux 命令（文件和目录管理 - locate）](http://blog.csdn.net/liang19890820/article/details/53285624)
<br />
<br />
<br />





