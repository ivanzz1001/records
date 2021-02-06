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

Lua支持面向过程编程(procedural programming)、面向对象(object-oriented)编程、函数式编程(functional programming)、数据驱动编程(data-driven programming)以及数据描述。

Lua将简单的过程语法与基于关联数组和可扩展语义的强大数据描述结构结合在一起。Lua是一种动态类型的语言，可通过基于寄存器的虚拟机解释字节码来运行，并且通过一个通用的垃圾回收器从而实现了自动的内存管理，因此非常适合配置，脚本编写和快速原型制作。

Lua被实现为一个库(library)，用clean C（标准C和C ++的公共子集）编写。Lua的发行版包括一个名为```lua```的主机程序，其使用Lua Library来提供完整独立的Lua解释器，以供交互式或批处理使用。Lua既可作为强大、轻量级、可嵌入的脚本语言来使用，也可作为强大且高效的独立语言来使用。

作为一门扩展语言，Lua中并没有所谓的main程序：其是通过嵌入到宿主客户端(host client)来工作的，因此也可以将Lua称为```嵌入式程序```(embedding program)或者简单的称为```宿主```(host)。宿主程序(host program)可以通过调用函数来执行一系列的Lua代码，读写Lua变量，也可以注册相关的C函数以供Lua脚本调用。通过使用C函数，Lua可以被扩展到能够处理众多不同领域的问题，从而创建出一个共享语法框架的定制编程语言。



### 1.1 Lua特性

* 轻量级： 它用标准C语言编写并以源代码形式开放，编译后仅仅一百余KB，可以很方便的嵌入别的程序里。

* 可扩展： Lua提供了非常易于使用的扩展接口和机制----由宿主语言(通常是C或者C++)提供这些功能，Lua可以使用它们，就像本来就内置的功能一样。 

* 其他特性

  * 支持过程式编程(procedural programming)、面向对象(object-oriented)编程、函数式编程(functional programming)、数据驱动编程(data-driven programming)以及数据描述。

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

### 4.1 x-studio
通常我们学习一门编程语言，都会选择一个好用的IDE工具，这里我们可以使用```x-studio```，如下图所示：

![x-sudio](https://ivanzz1001.github.io/records/assets/img/lua/x-studio.jpg)

x-studio是一款轻量级且强大的开发人员IDE，软件大小仅15M左右，不仅具有UI编辑功能，还具有Lua代码编辑和调试功能。致力于Sublime Text & VSCODE一样的编辑体验， VS一样的调试体验，支持自动代码缩进，Ctrl+I修正代码缩进，让编辑和调试Lua变得简单易用，轻量级IDE，轻松愉悦的体验；由作者历时5年精心打造和雕琢而成。作者秉承用心，用灵魂做好软件的理念，将毕生所学融入软件的灵魂，依然在持续维护和优化该项目。最新版本更是增加了全量资源加密解决方案，简单方便，易于集成。x-studio官网请[点击](https://x-studio.net/)

1) **如何创建简单调试工程（适合Lua初学者)**

通过菜单【文件】【新建工程】在弹出新建工程对话框里工程类型的下拉列表中选择【Lua Debug]，设置好工程目录，Lua Engine默认luajit(有lua51、lua52、lua53、lua54可选），也可以点击按钮自定义引擎来选择游戏exe作为lua调试目标（适用于任何含lua5.1以上或者luajit虚拟机的exe，包括cocos2d lua游戏的exe)，然后点【确定】即可新建简单的Lua工程。新建工程后会默认创建一个名为```example.lua```的helloworld脚本，此时便可以开始Lua的学习编辑和调试之旅了。

2） **cocos2d-x-3.x创建的lua工程**

对于cocos2d-x-3.x创建的lua工程，可以直接打开工程文件快速创建调试工程。马上开始Lua编辑调试之旅吧。由于x-studio专门针对cocos2d-x做了优化，只要启动目标exe和工作目录设置正确，再也不用担心断点无法命中问题（笔者在使用BabeLua时便经常遇到这种头疼问题，也经常有BabeLua使用者求助），另外由于专门针对cocos2-x做了优化，启动调试的速度也是杠杆的，给用户带来了非常轻松愉悦的调试体验。

3） **x-studio拥有64位调试引擎**

x-studio同时还具备64位调试引擎，可调试Unity (slua ulua/tolua xlua)，更多功能和使用细节请阅读软件文档:[x-studio软件文档](https://docs.x-studio.net/zh_CN/latest/)

### 4.2 ZeroBrane Studio
此外，我们还可以使用官方所推荐的[ZeroBrane Studio](http://www.lua.org/start.html)，但是其目前并不支持Lua5.4。

ZeroBrane Studio支持以多种方式调试LuaJIT应用程序：

* v0.39+版本的ZeroBrane Studio内置包含LuaJIT(v2.02)解释器，并且是其默认解释器（Project | Lua Interpreter | Lua)

* 可以使用remote debugging来远程调试LuaJIT程序

ZeroBrane Studio目前一个窗口只能打开一个工程，路径的查找都是基于工程的。笔者在使用时就碰到一个一直找不到对应Lua文件的问题，此问题就与此相关。


关于ZeroBrane Studio Preference的定制，可以参考[这里](https://studio.zerobrane.com/doc-editor-preferences#keyboard-shortcuts)。如下给出一个简短示例：
{% highlight string %}
--[[--
  Use this file to specify **System** preferences.
  Review [examples](+D:\Program Files (x86)\ZeroBraneStudio\cfg\user-sample.lua) or check [online documentation](http://studio.zerobrane.com/documentation.html) for details.
--]]--
editor.fontsize = 12

editor.defaulteol = wxstc.wxSTC_EOL_L
editor.usewrap = false
{% endhighlight %}


## 5. Lua作为嵌入式语言的执行原理
这里我们不详细讲解Lua嵌入到宿主程序中运行的原理，而是直接给出一个相应的示例，让读者有一个大体的了解。

### 5.1 示例1

1） **编写lua脚本**

这里我们编写一个名称为```add.lua```的脚本：
{% highlight string %}
function add(x, y)
    return x + y;
end
{% endhighlight %}

2) **编写宿主程序**

如下我们编写宿主程序```call_lua_add.c```:
{% highlight string %}
/*
 * 注： 如果是C++代码，请用extern "C"
 *
 *
 *
 * extern "C"{
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
   }
   
 * 
 */
 
#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


/*
 * call lua add function 
 */
int call_lua_add(lua_State *L)
{
	lua_getglobal(L, "add");                  //获取add函数
	lua_pushnumber(L, 123);                  //第一个操作数入栈
	lua_pushnumber(L, 456);                  //第二个操作数入栈
	lua_pcall(L, 2, 1, 0);                   //调用函数，2个参数，1个返回值
	int sum = (int)lua_tonumber(L, -1);      //获取栈顶元素（结果）
	lua_pop(L, 1);                           //栈顶元素出栈
	
	return sum;
}

int main(int argc, char *argv[])
{
	lua_State *L = luaL_newstate();              //新建lua解释器
	luaL_openlibs(L);                            //载入lua所有函数库
	
	luaL_dofile(L, "add.lua");                   //执行"Test.lua"文件中的代码
	printf("%d\n", call_lua_add(L));
	
	lua_close(L);                                //释放
	return 0;
}
{% endhighlight %}

注： 由于我们并没有将lua安装到标准的/usr/local目录，因此这里我们引用lua头文件时是```#include "lua.h"```

3） **编译**

当前我们的代码在/home/compile-machine/LuaInst/LuaTest目录下：
<pre>
# pwd
/home/compile-machine/LuaInst/LuaTest
# ls
add.lua  call_lua_add.c
</pre>

我们的lua环境安装在/home/compile-machine/LuaInst/lua-5.4.1/install目录下：
<pre>
# pwd
/home/compile-machine/LuaInst/lua-5.4.1/install
# ls
bin  include  lib  man  share
</pre>

因此我们采用如下方式来进行编译：
<pre>
# gcc -I/home/compile-machine/LuaInst/lua-5.4.1/install/include -o call_lua_add.o -c call_lua_add.c

# gcc -o call_lua_add call_lua_add.o -L/home/compile-machine/LuaInst/lua-5.4.1/install/lib -ldl -lm -llua
# ./call_lua_add
579
</pre>

### 5.2 示例2

1) **编写C代码call_lua_plus.c**
{% highlight string %}
#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


/*
 * 每一个Lua注册的函数都必须是这个原型，它已经在lua.h中定义了：
 *
 * typedef int (*lua_CFunction) (lua_State *L);
 *
*/
static int lua_plus(lua_State *L)
{
	lua_Integer a = luaL_checkinteger(L, 1);           //第一个参数的额索引是1
	lua_Integer b = luaL_checkinteger(L, 2);
	lua_pushinteger(L, a + b);                         //将结果入栈
	
	/* 这里可以看出，C可以返回给Lua多个结果，
     * 通过多次调用lua_push*()，之后return返回结果的数量。
	 *
	 * 由于c函数返回了一个int类型的返回值个数。因此，当压入返回值之前，
	 * 不必要清理栈，lua会自动移除返回值下面的任何数据
     */
	return 1;                                          
}

int main(int argc,char *argv[])
{
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	lua_pushcfunction(L, lua_plus);                     //将C函数转换为Lua的"function"并压入虚拟栈
	lua_setglobal(L, "myplus");                         //弹出栈顶元素，并在Lua中用名为"myplus"的全局变量存储。
	
	if (luaL_dostring(L, "print(myplus(2,4));")){	
		printf("Failed to invoke");
	}
	lua_close(L);
	
	return 0x0;
}
{% endhighlight %}

2) **编译**
<pre>
# gcc -I/home/compile-machine/LuaInst/lua-5.4.1/install/include -o call_lua_plus.o -c call_lua_plus.c
# gcc -o call_lua_plus call_lua_plus.o -L/home/compile-machine/LuaInst/lua-5.4.1/install/lib -lc -lm -ldl -llua
# ./call_lua_plus
6
</pre>

## 6. Lua调用C语言

如下我们介绍一下如何在Lua中调用C库函数。

1） **编写C程序**

编写如下名为```my_luac.c```的程序：
{% highlight string %}
#include <stdio.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
 
LUALIB_API int luaopen_mylib(lua_State *L );
 
//自定义函数
static int my_add(lua_State *L)
{
    int x = lua_tonumber(L,1);   //第一个参数,转换为数字
    int y = lua_tonumber(L,2);   //第二个参数,转换为数字
	
    int sum = x + y;
    lua_pushnumber(L, sum);
    return 1;                    //返回sum计算结果
}
 
static int showstr(lua_State *L)
{
   //从lua中传入的第一个参数
   const char *str = lua_tostring (L, 1);
   printf ("c program str = %s\n", str);
   
   return 0;
}
 
//函数列表
static  luaL_Reg funclist[] =
{
    {"add", my_add},                //my_add()函数，lua中访问时使用名称为add
    {"show", showstr},              //showstr()函数，lua中访问时使用名称为show
    {NULL, NULL}                    //最后必须有这个
};
 
//注册函数列表方便扩展
//我们编译后的动态库名称为mylib.so
LUALIB_API int luaopen_mylib(lua_State *L )
{
    luaL_newlib(L, funclist);    //lua中使用mylibfunc.add访问my_add函数
	
    return 1;
}
 
#if 0
//直接注册一个函数
LUALIB_API int luaopen_mylib(lua_State *L )
{
    lua_register(L, "add", my_add);
    return 1;
}
#endif
{% endhighlight %}

2) **编列lua脚本my_lua.lua**
{% highlight string %}
mylib = require "mylib"
local sum = mylib.add(3,9)

print("sum=",sum)
local str = mylib.show("haha")

{% endhighlight %}

3) **编译C程序为动态库**
<pre>
# gcc -o mylib.so -shared -fPIC -I/home/compile-machine/LuaInst/lua-5.4.1/install/include my_luac.c
# ls 
mylib.so  my_luac.c  my_lua.lua
</pre>

4) **测试Lua调用C程序**
<pre>
# /home/compile-machine/LuaInst/lua-5.4.1/install/bin/lua ./my_lua.lua
sum=    12.0
c program str = haha
</pre>

<br />
<br />

参看:


1. [Lua官网](http://www.lua.org/)

2. [lua documentation](http://www.lua.org/docs.html)

3. [Programming in Lua, Fourth Edition](http://www.lua.org/pil/)

4. [Linux下安装lua开发环境](https://blog.csdn.net/qq_27855219/article/details/83790126)

5. [第2课 - 搭建Lua开发环境](https://www.cnblogs.com/shiwenjie/p/6693998.html)

6. [Linux下安装lua开发环境](https://blog.csdn.net/qq_27855219/article/details/83790126)

7. [Lua安装](http://www.lua.org/manual/5.4/readme.html)

8. [lua_setglobal](https://www.wenjiangs.com/doc/lua_setglobal)

<br />
<br />
<br />

