---
layout: post
title: gperftools的使用
tags:
- LinuxOps
categories: linux
description: linux调试
---


本文介绍一下gperftools工具的使用，在此做个记录以便后续查阅。

<!-- more -->

## 1. gperftools介绍
gperftools由一个支持高性能(high-performance)、多线程(multi-threaded)的malloc()实现，以及若干实用的性能分析工具所组成。

gperftools来源于Google Performance Tools，是目前我所见过的最快的malloc，能够搭配threads以及STL进行良好的工作。gperftools主要支持如下5个功能：

* [Thread-caching (TC) malloc (docs/tcmalloc.html)](https://gperftools.github.io/gperftools/tcmalloc.html)

* [Heap-checking using tcmalloc (docs/heap_checker.html)](https://gperftools.github.io/gperftools/heap_checker.html)

* [Heap-profiling using tcmalloc (docs/heapprofile.html)](https://gperftools.github.io/gperftools/heapprofile.html)

* [CPU profiler (docs/cpuprofile.html)](https://gperftools.github.io/gperftools/cpuprofile.html)

* [pprof and Remote Servers (docs/pprof_remote_servers.html)](https://gperftools.github.io/gperftools/pprof_remote_servers.html)

## 1.1 gperftools的安装

当前最新版本的```gperftools```为gperftools-2.9.1，我们下载此版本进行安装。

1) **下载gperftools-2.9.1**

执行如下命令下载gperftools-2.9.1，并解压：
<pre>
# mkdir gperftools-inst
# cd gperftools-inst/
# wget https://github.com/gperftools/gperftools/releases/download/gperftools-2.9.1/gperftools-2.9.1.tar.gz
# ls
gperftools-2.9.1.tar.gz

# tar -zxvf gperftools-2.9.1.tar.gz 
# cd gperftools-2.9.1/
# ls
aclocal.m4  ChangeLog      config.guess  configure.ac  docs            install-sh  m4           missing   pprof-symbolize     src          vsprojects
AUTHORS     ChangeLog.old  config.sub    COPYING       gperftools.sln  libtool     Makefile.am  NEWS      README              test-driver
benchmark   compile        configure     depcomp       INSTALL         ltmain.sh   Makefile.in  packages  README_windows.txt  TODO
</pre>

2）**下载libunwind**

gperftools依赖于libunwind，官方建议我们安装新版的libunwind，因此这里我们可以到[libunwind download](https://download.savannah.gnu.org/releases/libunwind/)下载libunwind-1.5.0版本:
<pre>
# mkdir libunwind-inst
# cd libunwind-inst/
# wget --no-check-certificate https://download.savannah.gnu.org/releases/libunwind/libunwind-1.5.0.tar.gz

# ls
libunwind-1.5.0.tar.gz
# tar -zxvf libunwind-1.5.0.tar.gz 
# cd libunwind-1.5.0/
# ls
acinclude.m4  AUTHORS    config     configure.ac  doc      INSTALL      Makefile.in  README  tests
aclocal.m4    ChangeLog  configure  COPYING       include  Makefile.am  NEWS         src     TODO
</pre>

我们将libunwind-1.5.0安装到*/usr/local/libunwind-1.5.0*目录：
<pre>
# mkdir /usr/local/libunwind-1.5.0
# ./configure --prefix=/usr/local/libunwind-1.5.0
# make
# make install

# ls /usr/local/libunwind-1.5.0/
include  lib
</pre>



2) **安装gperftools-2.9.1**

上面解压完成后，我们查看其```README```以及```INSTALL```文件，以了解相关安装步骤。

根据```INSTALL```文档说明，安装```gperftools-2.9.1```需要依赖于```autoconf```、```automake```以及```libtool```工具，但目前新版本不用安装这些似乎也可以。

执行如下命令：
<pre>
# mkdir -p /usr/local/gperftools-2.9.1
# ./configure --prefix=/usr/local/gperftools-2.9.1 --enable-frame-pointers --enable-libunwind \
CPPFLAGS=-I/usr/local/libunwind-1.5.0/include LDFLAGS=-L/usr/local/libunwind-1.5.0/lib

# make 
# make install
# ls /usr/local/gperftools-2.9.1/
bin  include  lib  share
</pre>

安装成功之后，执行如下命令验证：
<pre>
# ls /usr/local/gperftools-2.9.1/bin -al
total 360
drwxr-xr-x 2 root root   4096 Nov 28 02:07 .
drwxr-xr-x 6 root root   4096 Nov 28 02:07 ..
-rwxr-xr-x 1 root root 178256 Nov 28 02:07 pprof
-rwxr-xr-x 1 root root 178256 Nov 28 02:07 pprof-symbolize


# /usr/local/gperftools-2.9.1/bin/pprof --version
pprof (part of gperftools 2.0)

Copyright 1998-2007 Google Inc.

This is BSD licensed software; see the source for copying conditions
and license information.
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.
</pre>

其实/usr/local/gperftools-2.9.1/bin/pprof是一个perf脚本文件，我们来看一下：
<pre>
# file /usr/local/gperftools-2.9.1/bin/pprof
/usr/local/gperftools-2.9.1/bin/pprof: Perl script, ASCII text executable
</pre>







<br />
<br />
**[参看]:**

1. [gperftools GitHub](https://github.com/gperftools/gperftools)

2. [gperftools wiki](https://github.com/gperftools/gperftools/wiki)

3. [gperftools download](https://github.com/gperftools/gperftools/releases)

5. [如何用gperftools分析深度学习框架的内存泄漏问题](https://cloud.tencent.com/developer/news/237228)

6. [利用Valgrind和gperftools解决内存问题](https://www.jianshu.com/p/6854085d54cd)

7. [TestGperftools](https://github.com/01joy/test-gperftools)

8. [性能测试工具CPU profiler(gperftools)的使用心得](https://cloud.tencent.com/developer/article/1433465)

9. [使用 gperftools 检测内存泄露](https://fuchencong.com/2021/04/22/develop-tools-1/)


<br />
<br />
<br />





