---
layout: post
title: linux中find命令的用法
tags:
- LinuxOps
categories: linuxOps
description: linux中find命令的用法
---


本文主要记录一下Linux中find命令的用法。

<!-- more -->


## 1. find命令使用说明

find用于在一个目录结构中查找文件。
<pre>
SYNOPSIS
       find [-H] [-L] [-P] [-D debugopts] [-Olevel] [path...] [expression]
</pre>

find支持众多选项，这里不会所有都进行讲解。

### 1.1 find按时间查找

find命令支持按时间查找。
<pre>
# find [path...] [-atime/-ctime/-mtime/-amin/-cmin/-mmin] [-/+] num
# find [path...] [-anewer/-cnewer/newer] file
</pre>
上面```num```的含义为：

* ```+num```: 大于num

* ```-num```: 小于num

* ```num```: 等于num

对于```atime/ctime/mtime```，其对应的时间单位为```num*24```小时； 对于```amin/cmin/mmin```，其对应的时间单位为分钟。

例如：
<pre>
//查找访问时间为1天以内的文件
# find ./ -atime -1  -type f
./release.asc

//查找修改时间比release.asc后的文件(-newer即为修改时间)
# find ./ -newer release.asc -type f
</pre>





<br />
<br />
<br />


