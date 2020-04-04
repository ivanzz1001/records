---
layout: post
title: C++智能指针
tags:
- cplusplus
categories: cplusplus
description: C++智能指针
---

本文主要介绍一下C++的智能指针。


<!-- more -->

## 1. 智能指针

首先我们在理解智能指针之前我们先了解一下什么是```RAII```思想。RAII全称是**Resource Acquisition Is Initialization**，直译过来就是```资源获取即初始化```，该机制是由Bjarne Stroustrup首先提出的，是一种利用对象生命周期来控制程序资源(如内存、文件句柄、网络连接、互斥量等）的简单技术。

对于RAII概念清楚后，我们就可以理解为智能指针就是```RAII```的一种体现。智能指针呢，它是利用了类的构造和析构，用一个类来管理资源的申请和释放，这样做的好处是什么呢？ 下面我们来分析一下。 	





<br />
<br />

**[参看]:**

1. [C++中的三种智能指针分析（RAII思想）](https://blog.csdn.net/GangStudyIT/article/details/80645399)

2. [C++中的RAII介绍](https://www.cnblogs.com/jiangbin/p/6986511.html)

3. [浅谈shared_ptr及shared_ptr涉及到的循环引用问题](https://blog.csdn.net/qq_34992845/article/details/69218843)
<br />
<br />
<br />





