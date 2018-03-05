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

现代X86处理器(i386及之后）都有8个32bit的通用寄存器，如下图所示。而这些寄存器的名字也大体遵循其历史名称。例如，EAX被称作所谓的累加器,因为它被用作一系列的算术运算；ECX被称作为计数器，因为它通常被用于存储循环索引。在现代汇编指令集当中，由于大部分的寄存器已经失去了它们原来的专用属性，通常只有两个寄存器被专门保留---stack pointer(esp,栈顶寄存器)和base pointer(ebp,基址寄存器）。

对于EAX、EBX、ECX、EDX 4个通用寄存器，其内部又可以划分成不同的段。例如，对于EAX的低位2个字节，我们可以将其作为16bit的寄存器来使用，称为AX。对于AX的低1字节我们可以将其作为一个单独的8位寄存器来使用，称作AL;对于AX的高1字节我们也可以将其作为一个单独的8位寄存器来使用，称作AH。而这些名称其实都是指向了同一个寄存器。当一个2字节的数据被放入DX寄存器后，它会影响到DH、DL、EDX的值。这些子寄存器主要是为了兼容老旧的、16位版本的指令集。然而，它们有时候可以很方便的处理小于32bit的数据（例如，1字节的ASCII字符）。

当在汇编语言中引用寄存器的时候，名称对大小写是不敏感的。例如，EAX与eax都会引用到相同的寄存器。


![x86-registers](https://ivanzz1001.github.io/records/assets/img/linux/x86-registers.png)













<br />
<br />

**[参看]:**

1. [x86 Assembly Guide](http://www.cs.virginia.edu/~evans/cs216/guides/x86.html)


<br />
<br />
<br />


