---
layout: post
title: MySQL事务日志
tags:
- database
categories: database
description:  MySQL事务日志
---

innodb事务日志包括redo log和undo log。redo log是重做日志，提供前滚操作；undo log是回滚日志，提供回滚操作。

undo log不是redo log的逆向过程，其实它们都算是用来恢复的日志：

* redo log通常是物理日志，记录的是数据页的物理修改，而不是某一行或几行修改成怎样怎样，它用来恢复提交后的物理数据页（恢复数据页，且只能恢复到最后一次提交的位置）。

* undo log用来回滚行记录到某个版本。undo log一般是逻辑日志，根据每行记录进行记录。


<!-- more -->

## 1. redo log





<br />
<br />
**[参看]**:

1. [mysql的undo log和redo log](https://www.cnblogs.com/wyy123/p/7880077.html)

2. [MySQL日志系统：redo log、binlog、undo log 区别与作用](https://blog.csdn.net/u010002184/article/details/88526708)

3. [详细分析MySQL事务日志(redo log和undo log)](https://www.cnblogs.com/f-ck-need-u/archive/2018/05/08/9010872.html)


<br />
<br />
<br />

