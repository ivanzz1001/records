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


也可以通过```--args```选项来为gdb所调试的可执行文件传递参数。例如：
<pre>
# gdb --args gcc -O2 -c foo.c
</pre>
上面的命令将会导致gdb调试gcc这个可执行程序，并且设置gcc的命令行参数为```-O2 -c foo.c```。


在启动GDB时可以通过传递```--silent```来禁止gdb的一些前置信息(如GDB版本号等）的打印。通过```gdb --help```或者```gdb -h```来查看gdb的帮助信息，

### 1.2 选择文件
当gdb启动之后，其会读取任何参数（而不是选项）以作为可执行文件和core dump文件（或进程pid)。这与分别通过```-se```、```-c```(或```-p```）选项来指定是一样的（gdb会读取第一个不带选项的参数，将其看做是带了```-se```选项； 接着读取第二个不带选项打的参数，将其看做是带了```-c```或```-p```选项）。假如第二个参数是一个```十进制数```，则gdb会尝试将其attach到一个进程，假如attach失败，则尝试将其当做一个core dump文件来看待。因此，假如你刚好有一个名称以数字开头的coredump文件，你可以通过类似于```./1234```这样的方式来避免gdb将其当成是一个pid。

在大多数嵌入式系统当中，gdb可能并未配置有包含core dump功能的支持，这时如遇到第二个参数，则会打印相应的警告信息并且忽略该参数。

下面我们列出gdb支持的一些选项：

* ```--symbols file```(或```-s file```): 从file文件中读取符号表

* ```--exec file```(或```-e file```): 将file作为可执行文件来执行，并将该参数后的另外一个不带选项的参数作为core dump文件

* ```--se file```: 从文件中读取符号表，并将其作为一个可执行文件

* ```--core file```(或```-c file```): 将file作为一个core dump文件

* ```--pid number```(或```-p number```): 连接到指定pid的进程，相当于执行了一个```attach```命令。

* ```--command file```(或```-x file```): 从file文件中执行相应的命令。对文件内容的执行严格参照```source```命令。

* ```--eval-command command```(或```-ex command```): 执行一个单独的gdb命令。本选项可多次使用，以调用多个命令。也可以搭配```--command```来使用。例如：
<pre>
# gdb -ex 'target sim' -ex 'load' \
  -x setbreakpoints -ex 'run' a.out
</pre>

* ```--init-command file```(或```-ix file```): 在加载可执行文件之前先执行file文件中指定的命令。

* ```-init-eval-command command```(或```-iex command```): 在加载可执行文件之前先执行一个单独的gdb命令

* ```-directory directory```(或```-d directory```): 将directory添加到gdb搜寻源文件及脚本文件的路径当中

* ```--readnow```(或```-r```): 马上读取每一个符号文件的所有符号表。默认情况下，读取符号表是根据所需以递增的方式来执行的。这会使得加载速度变慢，但是后续的执行速度会更快。

### 1.3 选择gdb的执行模式
你可以以不同的模式来运行gdb，例如```batch```模式或```quiet```模式。下面介绍几个常用的：

* ```--quiet```或```--silent```或```-q```: 表示以安静模式启动gdb，不要打印相关的介绍信息和版权信息。

* ```--args```:用于改变对命令行的解释，使得在可执行文件后面的参数会作为可执行文件的参数，而不是gdb的参数来处理。注意： 本选项会停止gdb的选项处理，因为gdb会认为这些选项都是属于可执行文件的。

* ```--version```: 用于使gdb在使用时打印相关的版本信息

### 1.4 退出GDB
一般可以通过```quit```或者```q```或者```Ctrl-d```来退出gdb。另外，```Ctrl-c```并不能退出gdb，其只是停止gdb当前正在执行的命令。

如果当前gdb已经attach到了一个进程，可以通过detach命令来解除。

### 1.5 shell命令
假如你想在gdb调试期间执行shell命令，可以不用退出gdb，直接以```shell command-string```这样的方式执行shell命令即可。








<br />
<br />

**[参看]**






<br />
<br />
<br />





