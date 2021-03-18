---
layout: post
title: OpenResty最佳实践(1)
tags:
- lua
categories: lua
description: lua开发
---



本文主要介绍openresty基础方面的内容。


<!-- more -->

## 1. Lua和LuaJIT的区别
Lua非常高效，它运行的比许多其他脚本(如Perl、Python、Ruby)都快，这点在第三方的独立测评中得到了证实。尽管如此，仍然会有人不满足，他们中觉得“嗯，还不够快!”。LuaJIT就是一个为了再榨出一些速度的尝试，它利用即时编译(Just-in Time)技术把Lua代码编译成本地机器码后交由CPU直接执行。LuaJIT2的测评报告表明，在数值运算、循环与函数调用、协程切换、字符串操作等许多方面它的加速效果都很显著。凭借着FFI特性，LuaJIT2在那些需要频繁地调用外部C/C++代码的场景，也要比标准Lua解释器快很多。目前，LuaJIT2已经支持包括i386、x86_64、ARM、PowerPC以及Mips等多种不同的体系结构。

LuaJIT是采用C和汇编语言编写的Lua解释器与即时编译器。LuaJIT被设计成全兼容标准的Lua5.1语言，同时可选地支持Lua 5.2和Lua 5.3中的一些不破坏向后兼容性的有用特性。因此，标准Lua语言的代码可以不加修改地运行在LuaJIT之上。LuaJIT和标准Lua解释器的一大区别是：LuaJIT的执行速度，即使是其汇编编写的Lua解释器，也要比标准Lua5.1解释器快很多，可以说是一个高效的Lua实现。另一个区别是，LuaJIT支持比标准Lua5.1语言更多的基本原语和特性，因此功能上也要更加强大。







<br />
<br />

参看:

1. [Lua官网链接](http://www.lua.org)

2. [LuaJIT官网链接](http://luajit.org)


<br />
<br />
<br />

