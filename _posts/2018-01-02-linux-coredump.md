---
layout: post
title: coredump的使用
tags:
- LinuxOps
categories: linux
description: coredump的使用
---

本章我们主要讲述一下Linux下C/C++程序coredump功能的使用。



<!-- more -->

Linux中很多信号(signal)的默认动作会使进程结束并产生一个coredump文件，即一个该进程结束时的内存镜像文件。一些调试器（如GDB）可以通过该镜像来获得程序结束时候的一些状态。下面列出几种信号，它们在发生时会产生coredump文件：

|      Signal     |       Action         |      Comment            |
|:---------------:|:--------------------:|:-----------------------:|
|   SIGQUIT       |       Core           | Quit from keyboard      |
|   SIGILL        |       Core           | Illegal Instruction     |
|   SIGABRT       |       Core           | Abort signal from abort |
|   SIGSEGV       |       Core           | Invalid memory reference|      
|   SIGTRAP       |       Core           | Trace/breakpoint trap   |

在控制终端，我们可以使用 ctrl+\ 来终止一个进程，这会向进程发出SIGQUIT信号，默认是会产生coredump的。还有其他情景也会产生coredump，如：程序调用abort()函数、访存错误、非法指令等等。

## 1. 开启coredump功能




<br />
<br />
<br />





