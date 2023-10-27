---
layout: post
title: goroutine调度实现原理
tags:
- go-language
categories: go-language
description: goroutine调度器
---

goroutine调度器的实现主要是如下3个文件:

* runtime/runtime2.go

* runtime/proc.go

* runtime/asm_amd64.s

花了一天的时间了阅读这部分代码实现，主要目的是了解goroutine调度器的基本运作过程和整体实现原理，并不对细节进行过多深入，但即便是如此“不求甚解”的阅读，在很多地方仍然是感到一头雾水，比如：

* goroutine调度循环

* goroutine的创建及放入runq的操作

好在网上有很多大牛也做过这一部分的源代码详解，通过阅读他们的文章，相关的疑惑才稍微逐渐的解开。

>ps: go version go1.18.10 linux/arm64




<!-- more -->





<br />
<br />
**[参看]：**

1. [Golang runtime 浅析](https://www.cnblogs.com/yjf512/archive/2012/07/19/2599304.html)

2. [Golang 协程/线程/进程 区别以及 GMP 详解](https://www.jianshu.com/p/a17485ac6d73)

3. [golang 理解goroutine](https://blog.csdn.net/u010412301/article/details/79123826)

4. [golang官网源代码](https://github.com/golang/go)

5. [Go语言高阶：调度器系列（1）起源](https://cloud.tencent.com/developer/article/1456594)

6. [Golang 调度器 GMP 原理与调度全分析](https://learnku.com/articles/41728)

7. [Golang深入理解GPM模型](https://www.bilibili.com/video/BV19r4y1w7Nx/?p=2&spm_id_from=pageDriver&vd_source=2699f104de8828a576fed54818f8cd79)

8. [万字长文深入浅出 Golang Runtime](https://zhuanlan.zhihu.com/p/95056679?from=groupmessage)

9. [Go 程序是怎样跑起来的？](https://baijiahao.baidu.com/s?id=1647240035815253532&wfr=spider&for=pc)

10. [GoLang之schedule 循环如何启动](https://blog.csdn.net/weixin_52690231/article/details/124886643)

11. [调度循环](https://github.com/golang-design/under-the-hood/blob/main/book/zh-cn/part2runtime/ch06sched/schedule.md)

<br />
<br />
<br />

