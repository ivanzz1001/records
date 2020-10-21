---
layout: post
title: goroutine调度器(转)
tags:
- go-language
categories: go-language
description: goroutine调度器
---

Go语言在2016年再次拿下```TIBOE```年度编程语言称号，这充分证明了Go语言这几年在全世界范围内的受欢迎程度。如果要对世界范围内的gopher发起一次```“你究竟喜欢Go的哪一点”```的调查，我相信很多Gopher会提到: **goroutine**。

Goroutine是Go语言原生支持并发的具体实现，你的Go代码无一例外地跑在goroutine中。你可以启动许多甚至成千上万的goroutine，Go的runtime负责对goroutine进行管理。所谓的管理就是```“调度”```，粗糙地说```调度```就是决定何时哪个goroutine将获得资源开始执行、哪个goroutine应该停止执行让出资源、哪个goroutine应该被唤醒恢复执行等。goroutine的调度是Go team care的事情，大多数gopher们无需关心。但个人觉得适当了解一下Goroutine的调度模型和原理，对于编写出更好的go代码是大有裨益的。因此，在这篇文章中，我将和大家一起来探究一下goroutine调度器的演化以及模型/原理。

>注意： 这里要写的并不是对goroutine调度器的源码分析，国内的雨痕老师在其《Go语言学习笔记》一书的下卷 “源码剖析” 中已经对Go 1.5.1的scheduler实现做了细致且高质量的源码分析了，对Go scheduler的实现特别感兴趣的gopher可以移步到这本书中去。这里关于goroutine scheduler的介绍主要是参考了Go team有关scheduler的各种design doc、国外Gopher发表的有关scheduler的资料，当然雨痕老师的书也给我了很多的启示。

<!-- more -->

## 1. Goroutine调度器







<br />
<br />
**[参看]：**

1. [也谈goroutine调度器](https://tonybai.com/2017/06/23/an-intro-about-goroutine-scheduler/)

2. [Golang 的 goroutine 是如何实现的](https://www.zhihu.com/question/20862617)

3. [图解go运行时调度器](https://tonybai.com/2020/03/21/illustrated-tales-of-go-runtime-scheduler/)

4. [深度解密Go语言之context](https://zhuanlan.zhihu.com/p/68792989)

<br />
<br />
<br />

