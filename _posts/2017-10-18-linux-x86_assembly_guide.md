---
layout: post
title: x86 Assembly Guide
tags:
- LinuxOps
categories: linux
description: x86 Assembly Guide
---


本章我们主要介绍一下基本的X86汇编，文中只覆盖了很小一部分可用的汇编指令集。有很多种不同的汇编语言来产生x86机器代码，这里（CS216)我们介绍的是```Microsoft Macro Assembler(MASM)```汇编。MASM使用标准的Intel语法来书写x86汇编代码。


<!-- more -->


## 1. Registers

现代X86处理器(i386及之后）都有8个32bit的通用寄存器，如下图所示。而这些寄存器的名字也大体遵循其历史名称。例如，EAX被称作所谓的累加器,因为它被用作一系列的算术运算；ECX被称作为计数器，因为它通常被用于存储循环索引。在现代汇编指令集当中，由于大部分的寄存器已经失去了它们原来的专用属性


![x86-registers](https://ivanzz1001.github.io/records/assets/img/linux/x86-registers.png)













<br />
<br />

**[参看]:**

1. [x86 Assembly Guide](http://www.cs.virginia.edu/~evans/cs216/guides/x86.html)


<br />
<br />
<br />


