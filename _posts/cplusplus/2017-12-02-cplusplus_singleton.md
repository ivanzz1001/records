---
layout: post
title: C++单例的实现
tags:
- cplusplus
categories: cplusplus
description: C++单例的实现
---


在C++中要写一个正确的单例类实现其实也是有一定难度的，有很多过去在生产环境中被认为是正确的写法（特别是所谓的```double-checked locking```写法)，都被发现存在潜在的bug。本文记录一下正确的C++单例实现，以备查阅。



<!-- more -->

 




<br />
<br />

**[参考]**


1. [设计模式之单例模式](https://segmentfault.com/a/1190000015950693)

2. [C++ 单例模式](https://zhuanlan.zhihu.com/p/37469260)

3. [C++完美单例模式](https://www.jianshu.com/p/69eef7651667)

<br />
<br />
<br />





