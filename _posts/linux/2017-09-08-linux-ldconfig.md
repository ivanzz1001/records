---
layout: post
title: Linux中ldconfig的使用
tags:
- LinuxOps
categories: linux
description: Linux中ldconfig的使用
---


本文我们介绍一下Linux系统中ldconfig命令的使用。

<!-- more -->

## 1. ldconfig命令
```/sbin/ldconfig```命令用于配置```动态链接器```在运行时的绑定，其会为如下三种共享链接库创建必要的links以及cache:

* */etc/ld.so.conf*文件中指定的共享库

* 受信任目录的共享库(/lib、/usr/lib)

* 在命令行通过```ldconfig -f```选项指定的共享库







<br />
<br />

**[参看]:**

1. [pkg-config官网](https://www.freedesktop.org/wiki/Software/pkg-config/)

2. [ldconfig命令](http://man.linuxde.net/ldconfig)

3. [ldconfig命令用法笔记](https://blog.csdn.net/philosophyatmath/article/details/51094619)

4. [PKG_CONFIG_PATH变量 与 ld.so.conf 文件](http://www.cnblogs.com/s_agapo/archive/2012/04/24/2468925.html)

5. [Linux下运行时链接库的路径顺序](https://blog.csdn.net/npu_wy/article/details/38642191)
<br />
<br />
<br />





