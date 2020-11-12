---
layout: post
title: lua开发
tags:
- lua
categories: lua
description: lua开发
---

本章记录一下lua开发的一些基础知识。

<!-- more -->

## 1. Lua介绍
Lua是一种强大、高效、轻量级的嵌入式脚本语言，其由巴西里约热内卢天主教大学(Pontifical Catholic University of Rio de Janeiro)的一个研究小组于1993年开发。Lua以标准C语言编写并以源代码形式开放，其设计的目的是为了嵌入应用程序中，从而为应用程序提供灵活的扩展和定制功能。


### 1.1 Lua特性

* 轻量级： 它用标准C语言编写并以源代码形式开放，编译后仅仅一百余KB，可以很方便的嵌入别的程序里。

* 可扩展： Lua提供了非常易于使用的扩展接口和机制----由宿主语言(通常是C或者C++)提供这些功能，Lua可以使用它们，就像本来就内置的功能一样。 

* 其他特性

  * 支持面向过程(procedure-oriented)编程和函数式编程(functional programming)

  * 自动内存管理
  
  * 只提供了一种通用类型的表(table)，用它可以实现数组、哈希表、集合、对象；

  * 语言内置模式匹配
  
  * 闭包(closure)函数也可以看做一个值
  
  * 提供多线程（协同进程，并非操作系统所支持的线程）支持

  * 通过闭包和table可以很方便地支持面向对象编程所需要的一些关键机制，比如数据抽象、虚函数、继承和重载等；

### 1.2 Lua应用场景

* 游戏开发

* 独立应用脚本

* Web应用脚本

* 扩展和数据库插件(如：MySQL Proxy和MySQL Workbench)

* 安全系统（如入侵检测系统）

## 2. Lua安装

Lua是以源代码形式来分发的，在使用之前我们需要先构建Lua环境。由于Lua是采用纯ANSI C来实现的，因此我们可以不做任何修改在大部分平台上直接编译。此外，Lua也可以不做任何修改，以C++的方式来进行编译。如下我们会介绍如何在类Unix平台（比如Linux、Mac OS X)来编译Lua。

假如你没空或者不想自己编译Lua的话，那么你可以直接获取编译好的[Lua二进制文件](http://lua-users.org/wiki/LuaBinaries)。此外，也可以参看[LuaDist](http://luadist.org/)，其是一个多平台的Lua发布版本。

### 2.1 编译Lua
在大部分的类Unix平台上，我们可以通过简单的执行```make```来进行编译。

1） **下载lua源代码包**

我们可以在[http://www.lua.org/ftp/](http://www.lua.org/ftp/)找到对应的Lua源码，这里我们下载Lua5.4.1版本：
<pre>
# mkdir LuaInst
# cd LuaInst
# wget http://www.lua.org/ftp/lua-5.4.1.tar.gz
# tar -zxvf lua-5.4.1.tar.gz
# cd lua-5.4.1
# ls
doc  Makefile  README  src
</pre>


2) **编译**

在Makefile中会自动推测我们的platform，因此这里我们可以直接执行make来进行编译：
<pre>
# make 
</pre>

假如Makefile自动推测platform失败，我们也可以执行```make help```看当前Lua支持哪些平台，然后选择对应的platform来进行编译：
<pre>
# make help
make[1]: 进入目录“/data/home/lzy/LuaInst/lua-5.4.1/src”
Do 'make PLATFORM' where PLATFORM is one of these:
   guess aix bsd c89 freebsd generic linux linux-readline macosx mingw posix solaris
See doc/readme.html for complete instructions.
make[1]: 离开目录“/data/home/lzy/LuaInst/lua-5.4.1/src”
</pre>

因此假如我们想要指定编译mingw平台的话，那么就可以执行```make mingw```来进行编译。假如上面并没有我们所列出的platform，那么我们可以按如下顺序来选择：

* 与目标平台最接近的

* generic

* c89


----------

上面的整个编译过程会耗费一点时间，并且会在源代码目录下产生3个文件： lua(the interpreter)、luac(the compiler)、liblua.a(the library)。
<pre>
# make
# ls src/
lapi.c     lauxlib.o   lcode.o     lctype.o  ldebug.o  ldump.o  lgc.h     liolib.c    llex.o      lmem.h     lobject.h   lopnames.h  lparser.o  lstring.c  ltable.c   ltm.c  lua.c      lua.hpp    lundump.o   lvm.o
lapi.h     lbaselib.c  lcorolib.c  ldblib.c  ldo.c     lfunc.c  lgc.o     liolib.o    llimits.h   lmem.o     lobject.o   loslib.c    lprefix.h  lstring.h  ltable.h   ltm.h  luac.c     lualib.h   lutf8lib.c  lzio.c
lapi.o     lbaselib.o  lcorolib.o  ldblib.o  ldo.h     lfunc.h  liblua.a  ljumptab.h  lmathlib.c  loadlib.c  lopcodes.c  loslib.o    lstate.c   lstring.o  ltable.o   ltm.o  luac.o     lua.o      lutf8lib.o  lzio.h
lauxlib.c  lcode.c     lctype.c    ldebug.c  ldo.o     lfunc.o  linit.c   llex.c      lmathlib.o  loadlib.o  lopcodes.h  lparser.c   lstate.h   lstrlib.c  ltablib.c  lua    luaconf.h  lundump.c  lvm.c       lzio.o
lauxlib.h  lcode.h     lctype.h    ldebug.h  ldump.c   lgc.c    linit.o   llex.h      lmem.c      lobject.c  lopcodes.o  lparser.h   lstate.o   lstrlib.o  ltablib.o  luac   lua.h      lundump.h  lvm.h       Makefile
</pre>

在编译完成之后，我们可以执行```make test```来检查是否编译成功（执行make test时会运行解释器，并打印出其版本信息):
<pre>
# make test
make[1]: 进入目录“/data/home/lzy/LuaInst/lua-5.4.1/src”
./lua -v
Lua 5.4.1  Copyright (C) 1994-2020 Lua.org, PUC-Rio
make[1]: 离开目录“/data/home/lzy/LuaInst/lua-5.4.1/src”
</pre>


假如我们在Linux平台上编译，我们还可以执行```make linux-readline```命令来编译交互式的Lua解释器，这样就可以直接在命令行上进行行编辑。假如编译此功能时出现了错误，请检查是否已经安装了readline开发包(开发包名称可能为libreadline-dev或者readline-devel)；假如在链接时发生错误，可以尝试执行：
<pre>
# make linux-readline MYLIBS=-ltermcap
</pre>

3) **安装**

在上面编译完成之后，我们可能需要将其安装到系统的某一个official位置。在这种情况下，我们直接执行```make install```即可，官方的安装位置在Makefile中已经定义，默认为：
<pre>
INSTALL_TOP= /usr/local
INSTALL_BIN= $(INSTALL_TOP)/bin
INSTALL_INC= $(INSTALL_TOP)/include
INSTALL_LIB= $(INSTALL_TOP)/lib
INSTALL_MAN= $(INSTALL_TOP)/man/man1
INSTALL_LMOD= $(INSTALL_TOP)/share/lua/$V
INSTALL_CMOD= $(INSTALL_TOP)/lib/lua/$V
</pre>

假如我们想要安装到本地，那么可以执行```make local```命令，此时其会在当前文件夹下创建一个install目录，然后将相关文件安装到其子目录下。如果要本地安装到其他指定的目录，那么可以通过```make install INSTALL_TOP=xxx```命令，这里```xxx```即为指定的目录。
<pre>
# make local

# tree install/
install/
├── bin
│   ├── lua
│   └── luac
├── include
│   ├── lauxlib.h
│   ├── luaconf.h
│   ├── lua.h
│   ├── lua.hpp
│   └── lualib.h
├── lib
│   ├── liblua.a
│   └── lua
│       └── 5.4
├── man
│   └── man1
│       ├── lua.1
│       └── luac.1
└── share
    └── lua
        └── 5.4

10 directories, 10 files
</pre>
上面这些目录就是我们开发所需要用到的目录。假如我们需要运行Lua程序，那么我们只需要```bin```以及```man```这两个目录； 假如我们需要在C/C++程序中中内嵌Lua的话，那么需要用到```include```以及```lib```这两个目录。

### 2.2 定制Lua
我们可以通过修改相应的配置文件，从而实现定制Lua环境：

* 如果想要更改安装目录，可以修改Makefile

* 如果想要查看如何Build Lua，可以修改src/Makefile

* 如果想要更改Lua所支持的一些特性(feature)，那么可以修改src/luaconf.h



## 3. 编写第一个Lua程序
按惯例，这里我们编写的第一个程序为输出```Hello, world```。首先创建一个名为```hello_world.lua```的脚本：
<pre>
# cat ./hello_lua
print("Hello, world");
</pre>

然后执行如下的命令解释执行：
<pre>
./install/bin/lua ./helloworld.lua 
hello, world
</pre>
可以看到执行成功。

## 4. Windows上Lua相关的IDE工具
通常我们学习一门编程语言，都会选择一个好用的IDE工具，这里我们可以使用```x-studio```，如下图所示：

![x-sudio](https://ivanzz1001.github.io/records/assets/img/lua/x-studio.jpg)

x-studio是一款轻量级且强大的开发人员IDE，软件大小仅15M左右，不仅具有UI编辑功能，还具有Lua代码编辑和调试功能。致力于Sublime Text & VSCODE一样的编辑体验， VS一样的调试体验，支持自动代码缩进，Ctrl+I修正代码缩进，让编辑和调试Lua变得简单易用，轻量级IDE，轻松愉悦的体验；由作者历时5年精心打造和雕琢而成。作者秉承用心，用灵魂做好软件的理念，将毕生所学融入软件的灵魂，依然在持续维护和优化该项目。最新版本更是增加了全量资源加密解决方案，简单方便，易于集成。x-studio官网请[点击](https://x-studio.net/)

1) **如何创建简单调试工程（适合Lua初学者)**

通过菜单【文件】【新建工程】在弹出新建工程对话框里工程类型的下拉列表中选择【Lua Debug]，设置好工程目录，Lua Engine默认luajit(有lua51、lua52、lua53、lua54可选），也可以点击按钮自定义引擎来选择游戏exe作为lua调试目标（适用于任何含lua5.1以上或者luajit虚拟机的exe，包括cocos2d lua游戏的exe)，然后点【确定】即可新建简单的Lua工程。新建工程后会默认创建一个名为```example.lua```的helloworld脚本，此时便可以开始Lua的学习编辑和调试之旅了。

2） **cocos2d-x-3.x创建的lua工程**

对于cocos2d-x-3.x创建的lua工程，可以直接打开工程文件快速创建调试工程。马上开始Lua编辑调试之旅吧。由于x-studio专门针对cocos2d-x做了优化，只要启动目标exe和工作目录设置正确，再也不用担心断点无法命中问题（笔者在使用BabeLua时便经常遇到这种头疼问题，也经常有BabeLua使用者求助），另外由于专门针对cocos2-x做了优化，启动调试的速度也是杠杆的，给用户带来了非常轻松愉悦的调试体验。

3） **x-studio拥有64位调试引擎**

x-studio同时还具备64位调试引擎，可调试Unity (slua ulua/tolua xlua)，更多功能和使用细节请阅读软件文档:[x-studio软件文档](https://docs.x-studio.net/zh_CN/latest/)




<br />
<br />

参看:


1. [Lua官网](http://www.lua.org/)

2. [lua documentation](http://www.lua.org/docs.html)

3. [Programming in Lua, Fourth Edition](http://www.lua.org/pil/)

1. [Linux下安装lua开发环境](https://blog.csdn.net/qq_27855219/article/details/83790126)

2. [第2课 - 搭建Lua开发环境](https://www.cnblogs.com/shiwenjie/p/6693998.html)





<br />
<br />
<br />

