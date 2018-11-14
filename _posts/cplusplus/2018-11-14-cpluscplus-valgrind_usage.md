---
layout: post
title: valgrind的使用
tags:
- cplusplus
categories: cplusplus
description: valgrind的使用
---


本文我们主要介绍一下valgrind的使用。




<!-- more -->


## 1. valgrind介绍
Valgrind是用于构建动态分析工具的装备性框架。它包括一个工具集，每个工具执行某种类型的调试、分析或类似的任务，以帮助完善你的程序。Valgrind的架构是模块化的，所以可以容易的创建新的工具而又不会扰乱现有的结构。


典型情况下，Valgrind会提供如下一系列的有用工具：

* **Memcheck** 是一个内存错误侦测器。它有助于使你的程序，尤其是那些采用C或C++来写的程序，更加准确；

* **Cachegrind** 是一个缓存和分支预测分析器。其有助于你提高程序的运行性能；

* **Callgrind** 是一个调用图缓存生成分析器。它与Cachegrind的功能有重叠，但是也收集```Cachegrind```不收集的一些信息；

* **Helgrind** 是一个线程错误检测器。它有助于使你的多线程程序更加准确；

* **DRD** 也是一个线程错误检测器。它和```Helgrind```相似，但使用不同的分析技术，所以可能找到不同的问题；

* **Massif** 是一个堆分析器。它有助于使你的程序使用更少的内存；

* **DHAT** 是另一种不同的堆分析器。它有助于理解块(block)的生命周期、块的使用和布局的低效等问题；

* **SGcheck** 是一个仍处于试验状态的工具，用来检测堆和全局数组的溢出。它的功能和```Memcheck```互补：SGcheck能找到一些Memcheck无法找到的问题，反之亦然；

* **BBV** 是一个仍处于试验状态的SimPoint基本款矢量生成器。它对于进行计算机架构的研究和开发很有用处。

另外，也有一些大多数用户不会用到的小工具： ```Lackey```是一个示例工具，用于演示一些装备的基础性内容；```Nulgrind```是一个最小化的Valgrind工具，不做分析或者操作，仅用于测试目的。

## 1.1 valgind工具详解

1） **Memcheck**

最常用的工具，用来检测程序中出现的内存问题，所有对内存的读写都会被检测到，一切对malloc、free、new、delete的调用都会被捕获。所以，它能检测以下问题：
<pre>
1. 对未初始化内存的使用；

2. 读/写释放后的内存块；

3. 读/写超出malloc分配的内存块；

4. 读/写不适当的栈中内存块；

5. 内存泄露，指向一块内存的指针永远丢失；

6. 不正确的malloc/free或者new/delete匹配；

7. memcpy()相关函数中的dst和src指针重叠；
</pre>
这些问题往往是C/C++程序员最头疼的问题，Memcheck能在这里帮上大忙。










<br />
<br />

**[参看]**

1. [Valgrind使用](https://www.cnblogs.com/napoleon_liu/articles/2001802.html)

2. [valgrind官网](http://valgrind.org/)

3. [Valgrind使用说明](https://blog.csdn.net/suifengpiao_2011/article/details/51886186)

4. [Linux：Valgrind使用](https://blog.csdn.net/test1280/article/details/71179526)

5. [如何使用Valgrind memcheck工具进行C/C++的内存泄漏检测](https://www.oschina.net/translate/valgrind-memcheck)

6. [GDB与Valgrind ，调试代码内存的工具](https://www.cnblogs.com/happenlee/p/9931423.html)

<br />
<br />
<br />





