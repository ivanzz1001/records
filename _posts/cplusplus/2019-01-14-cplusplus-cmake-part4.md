---
layout: post
title: CMake的使用
tags:
- cplusplus
categories: cplusplus
description: CMake的使用
---


本文从一个更顶层的视角来介绍CMake的使用，在此做一个记录，以便于后期复习总结。

<!-- more -->


## 1. C/C++程序编译
对于简单的C/C++程序的编译，我们通常可以抽象为如下：
<pre>
# gcc -o {target} $(CFLAGS) $(source-files) $(LDFLAGS) $(LIBS)
</pre>

针对比较大型的项目：

* target可能有多个

* CFLAGS含有编译对应target的选项，比如通过```-I```指定头文件搜索路径，通过```-D```指定一些宏，```-O2```指定优化级别，```-g```开启调试功能等

* source-files可能是由散落在多个不同目录中的众多源文件组成

* LDFLAGS指定目标target所要链接的库的查找目录

* LIBS指定链接的库

在源代码越来越庞大，编译选项越来越多的情况下，如果每一次都直接使用上述命令来编译的话，可能导致命令十分长，而且不便于维护。


针对上述问题，解决思路就是Makefile。Makefile可以将上述各个部分进行拆解，并提供一些半自动化的推导，从而极大地协助IT开发人员完成项目的构建。但是针对大型项目，Makefile本身写起来也比较困难，因此又诞生了```CMake```。CMake就是用于产生Makefile的工具。


因此对于CMake而言，目标很明确，就是要尽可能简单的产生target、CFLAGS、source-files、LDFLAGS、LIBS等各个编译要素。可以说CMake也确实是这样做的：

1. [cmake-buildsystem(7)](https://cmake.org/cmake/help/v3.22/manual/cmake-buildsystem.7.html): 主要用于产生target

2. [cmake-commands(7)](https://cmake.org/cmake/help/v3.22/manual/cmake-commands.7.html): 提供一些方法，用于产生CFLAGS、source-files等等

3. [cmake-compile-features(7)](https://cmake.org/cmake/help/v3.22/manual/cmake-compile-features.7.html)

4. [cmake-variables(7)](https://cmake.org/cmake/help/v3.22/manual/cmake-variables.7.html): CMake中一些有特殊含义的变量

下面我们围绕这些方面进行介绍。


## 2. CMake buildsystem





<br />
<br />

**[参看]**


1. [cmake documents](https://cmake.org/cmake/help/v3.22/)

2. [cmake运行原理](https://blog.csdn.net/ztemt_sw2/article/details/81384538)

<br />
<br />
<br />


