---
layout: post
title: STL traits技巧
tags:
- cplusplus
categories: cplusplus
description: STL traits技巧
---


本文主要介绍一下stl中经常用到的traits技巧。


<!-- more -->





## 1. C++ traits技术
关于traits技术，官方也没有一个很明确的理论描述，也有点晦涩。我们从网上找到几个关于traits的描述：

* c++之父Bjarne Stroustrup关于traits的描述

>Think of a trait as a small object whose main purpose is to carry information used by another object or algorithm to determine "policy" or "implementation details".

简单翻译一下： trait是一个小对象，其主要目标是将另一个对象或算法的相关信息提取出来，用于确定```策略```或```实现细节```

* wiki中对traits的描述

>Traits both provide a set of methods that implement behaviour to a class, and require that the class implement a set of methods that parameterize the provided behaviour.

简单翻译一下： traits提供了一系列的方法用于实现一个类的行为，同时也要求该类对traits所指定的行为提供参数化实现。

综合上面，我认为可以这样理解： traits是对某一类```特征```或```行为```的抽象，同时要求在编译时(compile-time)就能够完成抽象的具体化。我们知道abstract class也可以通过继承的方式实现类似的功能，但abstract class是在运行时完成抽象的具体化的。如下图所示：

![cpp-traits](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_traits.jpg)

上图所示，无论是traits还是abstract class中均有两种对象，其中traits是通过萃取的方式来提取共同特征或行为的，而abstract是通过继承的方式来获得共同的特征或行为。可以看到，traits具有比abstract class更松散的耦合性。


traits不是一种语法特性，而是一种模板编程技巧。Traits在C++标准库，尤其是STL中，有着不可替代的作用。







<br />
<br />

**[参看]:**

1. [模板：什么是Traits](https://www.cnblogs.com/inevermore/p/4122278.html)

<br />
<br />
<br />





