---
layout: post
title: cmake的安装及使用
tags:
- cplusplus
categories: cplusplus
description: cmake的安装及使用
---

我们接上文继续讲解cmake的使用。

<!-- more -->

## 1. cmake tutorial 

### 1.1 Adding System Introspection 
下面我们考虑向工程中添加一些依赖于特定目标平台(target platform)的代码实现。比如在下面的例子中，我们会添加```log```和```exp```两个函数，而这些函数可能在某些目标平台上不存在。

加入当前的目标平台支持```log```和```exp```，那么在mysqrt()函数中我们会使用这些函数来计算平方根。首先我们在MathFunctions/CMakeLists.txt中使用```CheckSymbolExists```模块来校验这些函数是否可用。另外在某一些目标平台上，我们可能还需要链接```m```库，然后再次检查是否含有这些函数。





<br />
<br />

**[参看]**

1. [官方教程](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

2. [cmake官网](https://cmake.org/)

3. [cmake install](https://cmake.org/install/)

4. [超详细的CMake教程](https://www.cnblogs.com/ybqjymy/p/13409050.html)

5. [官方cmake命令](https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html)

<br />
<br />
<br />


