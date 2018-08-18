---
layout: post
title: GDB使用_part1
tags:
- cplusplus
categories: cplusplus
description: GDB的使用
---

本文档主要参看<<Debugging with GDB>> ```Tenth Edition, for gdb version 8.0.1```，详细的介绍一下GDB的使用方法。

<!-- more -->


## 1. GDB的启动与退出
本节我们主要讲述一下如何启动以及退出GDB，主要包含如下：

* 输入```gdb```来启动gdb调试

* 输入```quit```或者```Ctrl-D```来退出GDB

### 1. 启动GDB
通常情况下，在启动gdb时传入```可执行程序名```这样一个参数即可：
<pre>
# gdb program
</pre>
你也可以在启动时同时指定一个```可执行程序名```和一个```core```文件：
<pre>
# gdb program core
</pre>
另外，假如你想调试一个正在运行的进程的话，也可以在启动时指定一个进程ID作为第二个参数：
<pre>
# gdb program 1234
</pre>
上面的命令会将进程1234绑定到GDB（除非你刚好有一个名称为1234的文件，这种情况下将会优先将1234作为一个core文件来看待）。


要使用上面第二种形式的命令行参数，需要依赖于一个完整的操作系统；而当你使用GDB来远程调试一个主板的时候，这可能没有一个```进程```的概念，并且通常情况下也并不能获得一个core dump文件。假如gdb不能attach到一个进程，或者读取core dump文件的时候，都会给出相应的警告。







<br />
<br />

**[参看]**






<br />
<br />
<br />





