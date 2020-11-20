---
layout: post
title: ZeroBrane Studio简易适配Lua 5.4
tags:
- lua
categories: lua
description: lua开发
---

本文介绍ZeroBrane Studio简易适配Lua 5.4 的一些细节。


<!-- more -->

## 1. ZeroBrane Studio简易适配Lua 5.4

Lua5.4版本已经正式发布了，相信不少朋友都已经有所尝试，最简单的测试方法应该就是手动编译一个```lua.exe```，然后直接命令行执行脚本：
<pre>
# lua.exe lua_script_path
</pre>

当然，使用IDE来编写测试脚本会更方便些，自己平时用[ZeroBrane Studio](https://studio.zerobrane.com/)比较多，不过最新版的ZeroBrane Studio还没有直接支持Lua 5.4脚本的运行和调试，自己简单尝试配置适配了一下，发现还是比较容易的。

1) **建立luadeb54.lua脚本**

我们进入ZeroBrane Studio的安装目录，在```interpreters```文件夹下新建```luadeb54.lua```脚本，内容如下：
{% highlight string %}
dofile 'interpreters/luabase.lua'
local interpreter = MakeLuaInterpreter(5.4, ' 5.4')
interpreter.skipcompile = true
return interpreter
{% endhighlight %}

![lua-zerobrane-studio](https://ivanzz1001.github.io/records/assets/img/lua/lua-zerobrane-studio.jpg)

2） **下载Windows版本Lua54**
 
我们可以在[此处](http://luabinaries.sourceforge.net/download.html)下载已经编译好的Lua5.4二进制安装包：

![lua-inst-binary](https://ivanzz1001.github.io/records/assets/img/lua/lua-inst-binary.png)

这里我们把```lua-5.4.0_Win64_bin.zip```与```lua-5.4.0_Win64_dllw6_lib.zip```都下载下来，其中前一个只包含Lua相关的可执行文件，后一个包含对应的头文件及lua54.dll库。

之后我们将lua-5.4.0_Win64_bin文件夹下的```lua54.exe```和```lua54.dll`拷贝到ZeroBrane Studio安装目录的bin文件夹下：

![lua-inst-bin](https://ivanzz1001.github.io/records/assets/img/lua/lua-inst-bin.png)

经过上面两步,我们就已经可以在 ZeroBrane Studio 运行 Lua 5.4 脚本了,不过仍然不能调试。我们需要再执行如下步骤。


3) **创建clibs54文件夹**

在ZeroBrane Studio安装目录的bin文件夹下创建```clibs54```目录，可参考已经存在的clibs53。下面我们先来看一下```clibs53```目录下的情况：
<pre>
# clibs53目录
- mime
   - core.dll
- socket
   - core.dll
- lfs.dll
- lpeg.dll
- ssl.dll
</pre>

因此我们创建的```clibs54```目录下也需要再创建```mime```和```socket```两个子文件夹。接下来我们就需要相关的dll文件了，这些dll文件需要我们自己使用lua54版本的库来进行编译。下面我们逐一讲解。

## 2. 编译依赖库

### 2.1 编译luasocket
clibs54目录下mime和socket两个文件夹下放的是luasocket相关的dll文件。我们需要下载luasocket来进行编译。

1） **下载luasocket**

可以到[luasocket github](https://github.com/diegonehab/luasocket)下载对应的源代码文件。

2） **编译luasocket**

在下载完成的luasocket源代码目录下有对应的```sln```编译工程，这里我们先修改编译时需要用到的配置文件```Lua.props```:
{% highlight string %}
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <PropertyGroup Condition="'$(Platform)'=='x64'" Label="LuaPlat">
    <LUAPLAT>$(Platform)/$(Configuration)</LUAPLAT>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Platform)'=='Win32'" Label="LuaPlat">
    <LUAPLAT>$(Configuration)</LUAPLAT>
  </PropertyGroup>
  <PropertyGroup Label="UserMacros">
    <LUAV>5.4</LUAV>
    <LUAPREFIX>D:\Program Files (x86)\lua-5.4.0_Win64_dllw6_lib\</LUAPREFIX>
    <LUALIB>D:\Program Files (x86)\lua-5.4.0_Win64_dllw6_lib\</LUALIB>
    <LUACDIR>D:\Program Files (x86)\lua-5.4.0_Win64_bin\</LUACDIR>
    <LUALDIR>D:\Program Files (x86)\lua-5.4.0_Win64_bin\</LUALDIR>
    <LUAINC>D:\Program Files (x86)\lua-5.4.0_Win64_dllw6_lib\include</LUAINC>
    <LUALIBNAME>lua54.lib</LUALIBNAME>
  </PropertyGroup>
  <PropertyGroup>
    <_PropertySheetDisplayName>Lua</_PropertySheetDisplayName>
  </PropertyGroup>
  <ItemDefinitionGroup />
  <ItemGroup>
    <BuildMacro Include="LUAPLAT">
      <Value>$(LUAPLAT)</Value>
    </BuildMacro>
    <BuildMacro Include="LUAPREFIX">
      <Value>$(LUAPREFIX)</Value>
    </BuildMacro>
    <BuildMacro Include="LUAV">
      <Value>$(LUAV)</Value>
    </BuildMacro>
    <BuildMacro Include="LUALIB">
      <Value>$(LUALIB)</Value>
    </BuildMacro>
    <BuildMacro Include="LUAINC">
      <Value>$(LUAINC)</Value>
    </BuildMacro>
    <BuildMacro Include="LUACDIR">
      <Value>$(LUACDIR)</Value>
    </BuildMacro>
    <BuildMacro Include="LUALDIR">
      <Value>$(LUALDIR)</Value>
    </BuildMacro>
    <BuildMacro Include="LUALIBNAME">
      <Value>$(LUALIBNAME)</Value>
    </BuildMacro>
  </ItemGroup>
</Project>
{% endhighlight %}

在编译时我们需要有```lua54.lib```库，但是目前我们只在```lua-5.4.0_Win64_dllw6_lib```中发现lua54.dll，因此我们首先得从lua54.dll导出lua54.lib，具体导出方法请参看其他文档。

之后直接编译```luasocket```这个solution里面的连个project:

* mime 

* luasocket

编译出来后将对应mime文件夹下的core.dll拷贝到```clibs54```目录下的mime文件夹中；将luasocket文件夹下的core.dll拷贝到```clibs54```目录下的socket文件夹中。

### 2.2 编译lfs

我们可以去[lfs官方网站](http://math2.org/luasearch/lfs.html)下载源代码。下载完成之后修改修改```config.win```配置文件如下：
{% highlight string %}
LUA_VERSION= 5.4

# Installation directories
# System's libraries directory (where binary libraries are installed)
LUA_LIBDIR= "D:\workspace\lua-5.4.0_Win64_dllw6_lib"

# Lua includes directory
LUA_INC= "D:\workspace\lua-5.4.0_Win64_dllw6_lib\include"

# Lua library
LUA_LIB= "D:\workspace\lua-5.4.0_Win64_dllw6_lib\lua54.lib"

# Compilation directives
WARN= /O2
INCS= /I$(LUA_INC)
CFLAGS= /MD $(WARN) $(INCS)
CC= cl

# $Id: config.win,v 1.7 2008/03/25 17:39:29 mascarenhas Exp $
{% endhighlight %}
>注： 请修改为自己对应的lua库目录即可，路径最好不要有空格、括号等，以避免不必要的麻烦

之后执行如下命令编译：
{% highlight string %}
D:\workspace\luafilesystem>nmake -f Makefile.win

Microsoft (R) 程序维护实用工具 12.00.21005.1 版
版权所有 (C) Microsoft Corporation。  保留所有权利。

        link /dll /def:src\lfs.def /out:src\lfs.dll src\lfs.obj ""D:\workspace\l
ua-5.4.0_Win64_dllw6_lib\lua54.lib""
Microsoft (R) Incremental Linker Version 12.00.40629.0
Copyright (C) Microsoft Corporation.  All rights reserved.

lfs.obj : warning LNK4197: 多次指定导出“luaopen_lfs”；使用第一个规范
   正在创建库 src\lfs.lib 和对象 src\lfs.exp
        IF EXIST src\lfs.dll.manifest mt -manifest src\lfs.dll.manifest -outputr
esource:src\lfs.dll;2
{% endhighlight %}
编译完成后，在luafilesystem根目录下的src文件夹中可以看到生成有： lfs.dll，然后将其拷贝到```clibs54```目录下即可。

### 2.3 编译lpeg

lpeg是一个为Lua所使用的新的模式匹配库，基于解析表达式语法（PEGs）。其官方网址为： [lpeg官网](http://www.inf.puc-rio.br/~roberto/lpeg/)

然后参照如下方法来编译：

1） 新建“Win32 Console Application”工程，工程名为：lpeg



2） 在“Application Settings”中，选择“DLL”和“Empty project”


3） 添加下载的“lpeg.h”、“lpeg.c”到工程中


4） 菜单栏→“Project”→“Properties”，配置“All Configurations”

* "Additional Include Directories"添加"D:\workspace\lua-5.4.0_Win64_dllw6_lib\include"

* "Additional Library Directories"添加"D:\workspace\lua-5.4.0_Win64_dllw6_lib"

* "Additional Dependencies"添加"lua54.lib"



5） 菜单栏 -> “Project”→“Add New Item” -> “Modele-Definition File(.def)”，名称为：lpeg

lpeg.def文件的内容为：
{% highlight string %}
LIBRARY  "lpeg"
EXPORTS luaopen_lpeg
{% endhighlight %}

6) 编译 Release版本

7) 测试使用，新建一个lua脚本```test_lpeg.lua```，内容为：
{% highlight string %}
local lpeg = require  "lpeg"

-- matches a word followed by end-of-string
p = lpeg.R "az"^ 1 * - 1

print(p:match( "hello"))         --> 6
print(lpeg.match(p,  "hello"))   --> 6
print(p:match( "1 hello"))       --> nil
{% endhighlight %}



----------
注： 其实在第5步，我们也可以直接修改```lptree.c```文件:
{% highlight string %}
int luaopen_lpeg (lua_State *L);
int luaopen_lpeg (lua_State *L) {
  luaL_newmetatable(L, PATTERN_T);
  lua_pushnumber(L, MAXBACK);  /* initialize maximum backtracking */
  lua_setfield(L, LUA_REGISTRYINDEX, MAXSTACKIDX);
  luaL_setfuncs(L, metareg, 0);
  luaL_newlib(L, pattreg);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, "__index");
  return 1;
}


修改为：

int __declspec(dllexport) luaopen_lpeg (lua_State *L);
int __declspec(dllexport) luaopen_lpeg (lua_State *L) {
  luaL_newmetatable(L, PATTERN_T);
  lua_pushnumber(L, MAXBACK);  /* initialize maximum backtracking */
  lua_setfield(L, LUA_REGISTRYINDEX, MAXSTACKIDX);
  luaL_setfuncs(L, metareg, 0);
  luaL_newlib(L, pattreg);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, "__index");
  return 1;
}
{% endhighlight %}
这样编译出对应的dll库时对应的函数将被导出。


### 2.4 ssl编译

我们注意到```clibs54```目录下还有ssl.dll这个动态链接库，我们也可以下载对应的[Lua OpenSSL](http://www.25thandclement.com/~william/projects/luaossl.html#download)，具体的编译方法，我们这里不进行介绍。

## 3. 验证ZeroBrane Studio对Lua5.4的支持

完成上面的步骤之后，我们就可以在ZeroBrane Studio中进行基本的(Lua 5.4)脚本运行和调试了：

![zerobrane-studio](https://ivanzz1001.github.io/records/assets/img/lua/zerobrane-studio.png)

写个简单测试脚本：
{% highlight string %}
print("test begin")

local tbcmt = { __close = function() print("close to-be-closed var") end }

local function create_tbcv()
    local tbcv = {}
    setmetatable(tbcv, tbcmt)
    return tbcv
end
    
do
    local tbcv <close> = create_tbcv()
end

print("test end")
{% endhighlight %}

之后编译运行是可以正常输出的。



## 4. 写在最后

说明：

* 本文中介绍的适配方式比较简易,仅适用于较简单的开发场景：


* 对于关键字标准库等的适配没有处理,有兴趣的朋友可以尝试调整 api/lua/baselib.lua 文件

<br />
<br />

参看:

1. [ZeroBrane Studio](https://studio.zerobrane.com/)

2. [ZeroBrane Studio 简易适配 Lua 5.4](https://blog.csdn.net/tkokof1/article/details/106531843)

3. [关于luasocket的编译和部署](https://blog.csdn.net/ljxfblog/article/details/44303393)

4. [Lua lfs库](https://blog.csdn.net/qq_42712784/article/details/88820712)

5. [windows下make工具介绍](https://blog.csdn.net/weixin_34189116/article/details/91848445)

6. [windows下编译lua的lpeg库](https://segmentfault.com/a/1190000008067294)

7. [lpeg官网](http://www.inf.puc-rio.br/~roberto/lpeg/)

8. [LPeg 0 10的编译与使用](https://blog.csdn.net/jhfyuf/article/details/83998805)
<br />
<br />
<br />

