---
layout: post
title: Linux命令行与shell脚本
tags:
- LinuxOps
categories: linuxOps
description: Linux命令行与shell脚本
---


这里我们简单记录一下常见的Linux命令行与Shell脚本。

<!-- more -->

## 1. shell算术运算

这里主要有以下四种方法：

(1) 使用expr外部程式
<pre>
[root@localhost test-src]# r=`expr 2 + 3`         //注意操作数和运算符之间要有空白
[root@localhost test-src]# echo $r
5
[root@localhost test-src]# x=`expr 4 \* 5`
[root@localhost test-src]# echo $x
20
[root@localhost test-src]# y=`expr \( 5 - 3 \) \* 3 + 1`
[root@localhost test-src]# echo $y
7
</pre>

(2) 使用```使用 $(())```
<pre>
[root@localhost test-src]# a=$((1+1))             //这里数与运算符之间没有格式要求
[root@localhost test-src]# echo $a
2
[root@localhost test-src]# b=$(((2+2)*3))
[root@localhost test-src]# echo $b
12
</pre>


(3) 使用```$[]```
<pre>
[root@localhost test-src]# a=$[2+3]              //这里数与运算符之间没有格式要求
[root@localhost test-src]# echo $a
5
[root@localhost test-src]# b=$[ 5 + 6 ]
[root@localhost test-src]# echo $b
11
</pre>

(4) 使用let命令
<pre>
[root@localhost test-src]# n=20
[root@localhost test-src]# let n=n+1
[root@localhost test-src]# echo $n
21
</pre>


<br />
<br />
<br />


