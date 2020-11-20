---
layout: post
title: dll导出def和lib文件
tags:
- windows
categories: windows
description: dll导出def和lib文件
---


很多时候我们使用的第三方库都是以dll形式提供的，大部分情况下我们可以直接使用，不过有的时候我们可能也需要将```.dll```导出为```.lib```文件。

在Windows下编程的同学，可能都知道可以使用```Depends```这个工具来查看dll依赖项和导出符号，却很少知道在命令行下，还有几个其他很好用的命令，分别是dumpbin和lib，这是vs安装目录下的两个程序。


![win-cmd-dll](https://ivanzz1001.github.io/records/assets/img/windows/win-cmd-dll.jpg)

<!-- more -->

## 1. dumpbin
dumpbin命令的用法如下：
<pre>
用法: DUMPBIN [选项] [文件]

  选项:

   /ALL
   /ARCHIVEMEMBERS
   /CLRHEADER
   /DEPENDENTS
   /DIRECTIVES
   /DISASM[:{BYTES|NOBYTES}]
   /ERRORREPORT:{NONE|PROMPT|QUEUE|SEND}
   /EXPORTS
   /FPO
   /HEADERS
   /IMPORTS[:文件名]
      /LINENUMBERS
   /LINKERMEMBER[:{1|2}]
   /LOADCONFIG
   /NOLOGO
      /OUT:filename
   /PDATA
   /PDBPATH[:VERBOSE]
   /RANGE:vaMin[,vaMax]
   /RAWDATA[:{NONE|1|2|4|8}[,#]]
   /RELOCATIONS
   /SECTION:名称
   /SUMMARY
   /SYMBOLS
   /TLS
</pre>

* ```/HEADERS``` ： 可以查看dll的位数x86/x64，有哪些section。例如
{% highlight string %}
D:\Program Files (x86)\lua-5.4.0_Win64_dllw6_lib>dumpbin /HEADERS lua54.dll

Microsoft (R) COFF/PE Dumper Version 12.00.40629.0
Copyright (C) Microsoft Corporation.  All rights reserved.


Dump of file lua54.dll

PE signature found

File Type: DLL

FILE HEADER VALUES
            8664 machine (x64)
              11 number of sections
        5F3D6282 time date stamp Thu Aug 20 01:33:54 2020
           42C00 file pointer to symbol table
             E26 number of symbols
              F0 size of optional header
            2026 characteristics
                   Executable
                   Line numbers stripped
                   Application can handle large (>2GB) addresses
                   DLL
....
{% endhighlight %}

* ```/DEPENDENTS```: 可以查看依赖项，这和可视化工具Depends功能一样

* ```/EXPORTS xxx.dll```: 查看导出符号，即dll中包含哪些函数

* ```/IMPORTS xxx.dll```: 查看从依赖项中具体需要导入的函数

* ```/LINKERMEMBER xxx.lib```: 可以查看静态导入库中导入了哪些函数

这里我们需要制作dll对应的lib静态库文件，就需要先产生def文件，由```/EXPORTS```选项即可查看，但显示的信息不是按照def文件格式来的，需要我们手动调整：
{% highlight string %}
D:\Program Files (x86)\lua-5.4.0_Win64_dllw6_lib>dumpbin /EXPORTS lua54.dll > lua54_nouse.def
Microsoft (R) COFF/PE Dumper Version 12.00.40629.0
Copyright (C) Microsoft Corporation.  All rights reserved.


Dump of file lua54.dll

File Type: DLL

  Section contains the following exports for lua54.dll

    00000000 characteristics
    5F3D6282 time date stamp Thu Aug 20 01:33:54 2020
        0.00 version
           1 ordinal base
         152 number of functions
         152 number of names

    ordinal hint RVA      name

          1    0 0001C490 luaL_addgsub
          2    1 0001BE90 luaL_addlstring
          3    2 0001BEB0 luaL_addstring
          4    3 0001BEF0 luaL_addvalue
          5    4 0001ABD0 luaL_argerror
          6    5 0001B0D0 luaL_buffinit
          7    6 0001C2C0 luaL_buffinitsize
          8    7 0001B9A0 luaL_callmeta
          9    8 0001B020 luaL_checkany
...
{% endhighlight %}  

上面的导出的```lua54_nouse.def```不是按照def文件格式标准来的，不要用此工具来导```DEF```文件。请使用如下的```pexport```工具。

## 2. pexport

这里我们介绍另一款工具： pexport。可以从这里下载[pexports](http://sourceforge.net/projects/mingw/files/MinGW/Extension/pexports)

这是mingw项目下的一款工具。使用方式如下：
{% highlight string %}
# pexports -o xxx.dll  > xxx.def
{% endhighlight %}

## 3. lib
接下来就是使用lib命令，制作对应的lib文件了。其基本使用方法如下：
{% highlight string %}
用法: LIB [选项] [文件]

  选项:

   /DEF[:文件名]
   /ERRORREPORT:{NONE|PROMPT|QUEUE|SEND}
   /EXPORT:符号
   /EXTRACT:成员名
   /INCLUDE:符号
   /LIBPATH:目录
   /LIST[:文件名]
   /LTCG
   /MACHINE:{ARM|ARM64|EBC|X64|X86}
   /NAME:文件名
   /NODEFAULTLIB[:库]
   /NOLOGO
   /OUT:文件名
   /REMOVE:成员名
   /SUBSYSTEM:{BOOT_APPLICATION|CONSOLE|EFI_APPLICATION|
         EFI_BOOT_SERVICE_DRIVER|EFI_ROM|EFI_RUNTIME_DRIVER|
         NATIVE|POSIX|WINDOWS|WINDOWSCE}[,#[.##]]
   /VERBOSE
   /WX[:NO]
{% endhighlight %}
例如：
<pre>
# lib /def:xxx.def /machine:x86 out/:xxx.lib 

或

# lib /def:xxx.def /machine:x64 out/:xxx.lib
</pre>
以上分别为提取```x86```、```x64```位的引入库文件；out为可选项，若忽略，则生成的lib引入库文件的名称同def文件名。

至此，我们就可以在MSVC编译器中，使用
{% highlight string %}
#pragma comment(lib, "xxx.lib")
{% endhighlight %}
即可直接调用头文件中声明的函数了。



<br />
<br />



**[参看]:**

1. [通过dll或def文件提取lib导入库文件](https://www.cnblogs.com/haomiao/p/5802787.html)

2. [dll导出def和lib文件](https://hewei.blog.csdn.net/article/details/81742924)

<br />
<br />
<br />





