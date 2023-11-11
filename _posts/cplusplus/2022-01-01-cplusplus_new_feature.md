---
layout: post
title: C/C++标准版本及不同版本的新特性(转)
tags:
- cplusplus
categories: cplusplus
description: C/C++标准版本及不同版本的新特性
---

本文记录一下C/C++标准版本，以及不同版本的新特性。其中:

* C语言: C89/90、C95、C99、C11、C17

* C++语言: C++98、C++03、C++11、C++14、C++17、C++20



<!-- more -->

## 1. 发行标准
### 1.1 C语言标准

* C89 (ANSI X3.159-1989)： 由美国国家标准协会（ANSI）于1989年发布的C语言标准，也被称作ANSI C。

* C90 (ISO/IEC 9899:1990)： 与C89基本相同，是C89在国际化上的延伸，由国际标准化组织（ISO）和国际电工委员会（IEC）于1990年采纳的C语言标准。

* C95 (ISO/IEC 9899:1995)： 是对ISO C90标准的一次修订，增加了一些新特性，例如支持多字节字符等。

* C99 (ISO/IEC 9899:1999)： 由ISO/IEC在1999年采纳的新版C语言标准，新增了一些特性，如内联函数、边长数组、严格类型别名、具名结构初始化等。

* C11 (ISO/IEC 9899:2011)： 由ISO/IEC于2011年发布的C语言标准，增加了更多的新特性，例如静态断言、通用结构初始器、匿名结构和联合等。

* C17 (ISO/IEC 9899:2018)： 于2018年发布的当前最新的C语言标准，主要修复了C11标准中的一些问题，没有引入新特性。

### 1.2 C++语言标准
* C++98： 于1998年发布的C标准，是最早的国际标准化版本，包含了面向对象编程、模板等基本特性。

* C++03： 于2003年发布，对C++98进行一些小修小补，主要是修复C++98的一些bug和漏洞。

* C++11： 于2011年发布，被视为现代C++的开始，引入了多个重要特性，如自动类型推导、基于范围的 for 循环、Lambda 表达式、智能指针等。

* C++14： 于2014年发布的C++标准，以更大的灵活性和性能优化为目标，引入了多个新特性，进行了增量式改进，例如泛型Lambda表达式、返回类型后置等。

* C++17： 于2017年发布，进一步完善C++特性，如结构化绑定、并行算法库、内联变量等。

* C++20： 最新的C标准，已经获得批准，编译器也已经开始支持其中的新特性。它引入了模块、概念、协程等重要特性 ，对C++语言进行了较大的扩展。

这些标准制定了C语言和C++的基本规范，各个编译器需要支持这些标准以确保代码的正确执行和相互兼容。在实际编程过程中，需要根据所使用编译器的支持情况选择合适的标准。


## 2. 版本新特性
### 2.1 C语言版本新特性

1) **C89/90**

* 函数原型

* const 限定符

* volatile 限定符

* enum 枚举类型

* void 指针类型

* 单行注释，使用 ```//```


2) **C99**

* 可变长度数组（VLA）

* 行内函数 (inline)

* 类型宽度宏，如 `UINT32_MAX`

* 严格的类型别名规则（strict aliasing）

* 灵活的数组成员 (Flexible Array Member)

* 复合文字 (Compound Literals)

* 布尔数据类型 `_Bool`

* 复数和虚数数据类型

3) **C11**

* 多线程支持

* 原子操作

* 静态断言（Static assertions）

* 无类型泛型宏（Generic selection）

* 匿名结构和匿名联合

* 外部变量的对齐声明

* 类型泛化表达式


### 2.2 C++语言特性

1） **C++98**

* 命名空间（namespaces）

* 类模板（class templates）

* 异常处理（exceptions）

* 运行时类型识别（RTTI）

* 标准模板库（STL）

* bool 类型

* 类型转换操作符

2） **C++03**

在C++98基础上修复了一些bug和漏洞

3） **C++11**

* 自动类型推导（auto）

* 基于范围的 for 循环（Range-based for loops）

* Lambda 表达式

* 右值引用和移动语义（Rvalue references and move semantics）

* 初始化列表（Initializer lists）

* 类型推断 decltype

* constexpr 编译时计算

* 强类型枚举（Scoped enumerations）

* nullptr为NULL的替代品

* 智能指针（shared_ptr, unique_ptr, weak_ptr）

* 并发编程（包括多线程的支持）

4） **C++14**

* 泛型Lambda表达式

* 返回类型后置（函数返回类型推导）

* 二进制字面值

* 编译时整数序列（整数常量模板）

* 引入传递引用类型的函数

* 引入类型deprecated属性 (废弃声明)

5） **C++17**

* 结构化绑定（Structured bindings）

* 并行算法库（Parallel algorithms）

* 内联变量（Inline variables）

* 文件系统库（Filesystem library）

* 变体类型（std::variant）

* 可选类型（std::optional）

* 任务型未来（std::future）

6) **C++20**

* 概念（Concepts）

* 范围（Ranges）

* 协程（Coroutines）

* 模块（Modules）

* 连续表述（constexpr features）

* Lambda 表达式的优化

* std::span 视图经常使用的一段连续内存

* std::format 格式库


<br />
<br />

**[参看]:**

1. [cpluscpus reference](https://cplusplus.com/reference/algorithm/sort/)

2. [cppreference](https://en.cppreference.com)

3. [C/C++标准版本及不同版本的新特性](https://blog.csdn.net/crr411422/article/details/125592160)

4. [深入剖析C++ 11新特性](https://blog.csdn.net/qq_65139309/article/details/130540716)

5. [cpp_new_feature](https://github.com/0voice/cpp_new_features)

6. [c++ shell](https://cpp.sh)


<br />
<br />
<br />





