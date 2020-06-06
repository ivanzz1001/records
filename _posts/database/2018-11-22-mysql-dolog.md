---
layout: post
title: MySQL事务日志(redo log和undo log)
tags:
- database
categories: database
description:  MySQL事务日志
---


在数据库系统中，既有存放数据的文件，也有存放日志的文件。日志在内存中也是有缓存log buffer，也有磁盘文件log file，本文主要描述存放日志的文件。



<!-- more -->




<br />
<br />
**[参看]**:

1. [mysql的undo log和redo log](https://www.cnblogs.com/wyy123/p/7880077.html)

2. [MySQL日志系统：redo log、binlog、undo log 区别与作用](https://blog.csdn.net/u010002184/article/details/88526708)

3. [详细分析MySQL事务日志(redo log和undo log)](https://www.cnblogs.com/f-ck-need-u/archive/2018/05/08/9010872.html)


<br />
<br />
<br />

