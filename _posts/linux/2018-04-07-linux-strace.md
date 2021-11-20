---
layout: post
title: strace命令的使用
tags:
- LinuxOps
categories: linux
description: strace命令的使用
---

本文介绍一下Linux中strace命令的使用。

<!-- more -->


## 1. strace跟踪进程中的系统调用
strace常用来跟踪进程执行时的系统调用和所接收的信号。在Linux世界，进程不能直接访问硬件设备，当进程需要访问硬件设备（如读取磁盘文件，接收网络数据等等）时，必须由用户态模式切换至内核态模式，通过系统调用访问硬件设备。strace可以跟踪到一个进程产生的系统调用，包括参数、返回值，执行消耗的时间。

### 1.1 输出参数含义
每一行都是一条系统调用，```等号```左边是系统调用的函数名及参数，右边是该调用的返回值。strace显示这些调用的参数并返回符号形式的值。strace从内核接收信息，而且不需要以任何特殊的方式来构建内核。
<pre>
# strace cat /dev/null
execve("/bin/cat", ["cat", "/dev/null"], [/* 22 vars */]) = 0
brk(0)                                  = 0xab1000
access("/etc/ld.so.nohwcap", F_OK)      = -1 ENOENT (No such file or directory)
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f29379a7000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
...
</pre>

### 1.2 参数

1) **与输出格式相关的参数**
<pre>
-a column
    设置返回值的输出位置.默认 为40.

-i 输出系统调用的入口指针.

-k 跟踪每一个系统调用的执行栈

-o filename
    将strace的输出写入文件filename

-q 禁止输出关于attach/detach消息.

-r 打印出执行每个系统调用时的相对时间戳

-s strsize
    指定输出的字符串的最大长度(默认为32)。文件名一直全部输出.

-t 时：分：秒
-tt 时：分：秒 . 微秒
-ttt 计算机纪元以来的秒数 . 微秒

-T 显示每一调用所耗的时间，精确到微秒

-x 以十六进制形式输出非标准字符串
-xx 所有字符串以十六进制形式输出.
</pre>

2）**与统计信息相关的参数**

<pre>
-c 统计每一系统调用的所执行的时间,次数和出错的次数等.
</pre>

3) **与Filtering相关的参数**
<pre>
-e expr
   一个限定表达式，用于指定跟踪哪些events或者指定如何进行跟踪。格式如下:
   [qualifier=][!][?]value1[,[?]value2]...

   qualifier只能是 trace, abbrev, verbose, raw, signal, read, write, fault, inject, kvm其中之一。
   value是用来限定的符号或数字。默认的 qualifier是 trace。感叹号是否定符号。

   例如: -e open等价于 -e trace=open,表示只跟踪open调用。而-e trace=!open表示跟踪除了open以外的其他调用。

   对于value前面的问号(?)，其表示当没有匹配的系统调用时，抑制相应的错误信息输出。

   另外，还有两个特殊的value符号 all 和 none.

  （注意有些shell使用!来执行历史记录里的命令,所以有时要使用\!)

-e trace=set
    只跟踪指定的系统 调用.例如: -e trace=open,close,rean,write表示只跟踪这四个系统调用.默认的为set=all.

-e trace=%file
-e trace=file (deprecated)
    只跟踪有关文件操作的系统调用。我们可以将此命令看成是-e strace=open,stat,chmod,unlink...等操作的简写形式，
    这有助于我们找出相应进程当前正在引用(reference)哪些文件。

-e trace=%process
-e trace=process (deprecated)
    只跟踪有关进程控制的系统调用。这可用于跟踪fork, wait, exec等系统调用


-e trace=%network
-e trace=network (deprecated)
    跟踪与网络有关的所有系统调用.

-e trace=%signal
-e trace=signal (deprecated)
    跟踪所有与系统信号有关的 系统调用

-e trace=%ipc
-e trace=ipc   (deprecated)
    跟踪所有与进程通讯有关的系统调用

-e trace=%desc
-e trace=desc (deprecated)
    跟踪所有与文件描述符相关的系统调用。


-e trace=%memory
-e trace=memory (deprecated)
    跟踪所有与memory mapping相关的系统调用

-e trace=%stat
    Trace stat syscall variants

-e trace=%lstat
    Trace lstat syscall variants.

-e trace=%fstat
    Trace fstat and fstatat syscall variants.

-e trace=%%stat
    Trace syscalls used for requesting file status (stat, lstat, fstat, fstatat, statx, and their variants).



-e abbrev=set
    设定strace输出的系统调用的结果集，指定哪些系统调用中的大型数组或结构体内容缩减显示，默认为abbrev=all，abbrev=none等价于-v选项。
    如strace -e abbrev=execve ./test仅显示execve调用中argv[]和envp[]的部分内容。

-e raw=set
    指定哪些系统调用中的参数以原始未解码的形式(即16进制)显示。当用户不信任strace解码或需要了解参数实际数值时有用

-e signal=set
    指定跟踪的系统信号.默认为all.如 signal=!SIGIO(或者signal=!io),表示不跟踪SIGIO信号.

-e read=set
    以16进制和ASCII码对照形式显示从指定文件描述符中读出的所有数据，如-e read=3,5可观察文件描述符3和5上的输入动作。
    该选项独立于系统调用read的常规跟踪(由-e trace=read选项控制)

-e write=set
    以16进制和ASCII码对照形式显示写入指定文件描述符的所有数据，如-e write=3,5可观察文件描述符3和5上的输出动作。
    该选项独立于系统调用write的常规跟踪(由-e trace=write选项控制)

-P path
   跟踪访问指定path的系统调用。可以使用多个-P选项以指定多个路径

-v 输出所有的系统调用.一些调用关于环境变量,状态,输入输出等调用由于使用频繁,默认不输出.
</pre>

4) **Tracing相关参数**
<pre>
-f 跟踪由fork调用所产生的子进程.
-ff 如果提供-o filename,则所有进程的跟踪结果输出到相应的filename.pid中,pid是各进程的进程号.
</pre>

5) **Startup相关参数**
<pre>
-p pid
    跟踪指定的进程pid.

-u username
    以username的UID和GID执行被跟踪的命令
</pre>

6) **Miscellaneous参数**
<pre>
-d 输出strace本身的一些调试信息到标准错误

-F 尝试跟踪vfork调用.在-f时,vfork不被跟踪.

-h 输出简要的帮助信息.

-V 输出strace的版本信息.
</pre>

### 1.3 命令实例
1） **跟踪可执行程序**
<pre>
# strace -f -F -o ./strace_out.txt myserver
</pre>
```-f -F```选项告诉strace同时跟踪fork和vfork出来的进程。```-o```选项把所有strace输出写入到strace_out.txt文件中，```myserver```是需要启动的程序。

2) **跟踪服务程序**
<pre>
# strace -o output.txt -T -tt -e trace=all -p 28979
</pre>
跟踪28979进程的所有系统调用(-e trace=all)，并统计系统调用的花费时间，以及开始时间（并以可视化的时分秒格式显示），最后将记录结果存在output.txt文件里面。




<br />
<br />
**[参看]:**

1. [linux tools](https://linuxtools-rst.readthedocs.io/zh_CN/latest/tool/readelf.html)

2. [strace工具使用手册](https://blog.csdn.net/Huangxiang6/article/details/81295752)

<br />
<br />
<br />





