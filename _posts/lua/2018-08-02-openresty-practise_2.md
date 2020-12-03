---
layout: post
title: FFI库
tags:
- lua
categories: lua
description: lua开发
---



FFI库，是LuaJIT中最重要的一个扩展库。它允许从纯Lua代码调用外部C函数，并且可以直接使用C数据结构。

FFI库可以极大的消除在C代码中编写冗长的Lua/C绑定函数，也不需要单独再学习一门绑定语言(binding language)————FFI可以直接解析纯C的声明。而这些声明，我们可以直接从C语言头文件或者参考手册中拷贝过来即可。通过FFI我们只需要绑定相关的lib库就可以了，而不需要处理其中脆弱的绑定生成器。

当前FFI库已经集成进LuaJIT里面了（其并不能作为一个单独的模块来使用）。JIT编译器生成的用于从Lua代码访问C数据结构的代码与C编译器生成的代码相当。在JIT编译的代码中可以inline方式调用C函数，而不像是通过传统的Lua/C API方式调用，因此性能也会极高。

简单解释一下Lua扩展C库，对于那些能够被Lua调用的C函数来说，它的接口必须遵循Lua要求的形式，就是:
<pre>
typedef int (*lua_CFunction)(lua_State* L)
</pre>

这个函数包含的参数是lua_State类型的指针。可以通过这个指针进一步获取通过lua代码传入的参数。这个函数的返回值类型是一个整形，表示返回值的数量。需要注意的是，用C编写的函数无法把返回值返回给Lua代码，而是通过虚拟栈来传递Lua和C之间的调用参数和返回值。不仅在编程上开发效率变低，而且性能上比不上FFI库调用C函数。


<!-- more -->


下面我们给出两个示例：

###### Motivating Example: Calling External C Functions

编写如下lua脚本调用外部C函数：
{% highlight string %}
local ffi = require("ffi")                  -- 1
ffi.cdef[[                                  -- 2
  int printf(const char *fmt, ...);
]]

ffi.C.printf("Hello %s!\n", "world")        -- 6
{% endhighlight %}
现在我们来简单的分析一下上面的代码块：

* 第一行调用require来加载FFI库

* 第二行在cdef中添加C函数的声明

* 第六行调用C函数

事实上，底层的调用逻辑远比上面代码展示的复杂：在第6行中，会使用标准的C命名空间ffi.C，然后在该空间内查找```printf```符号表（注： 这里对相关符号的查找与C可执行程序类似，在Linux下一般为/lib、/lib64、/usr/local/lib、/usr/local/lib64目录，如果相关符号不在这些目录下，则可能需要通过LD_LIBRARY_PATH等参数来进行设置）。函数执行的返回结果是某一特定类型的object，而向函数传递的参数也会自动的从Lua object转换成对应的C类型。

上面使用```printf()```的例子实现的功能相对太简单，我们直接在Lua中调用io.write()或者string.format()就可以实现。下面我们给出一个复杂一点的例子：在windows上弹出一个对话框
{% highlight string %}
local ffi = require("ffi")
ffi.cdef[[
  int MessageBoxA(void *w, const char *txt, const char *cap, int type);
]]
ffi.C.MessageBoxA(nil, "Hello world!", "Test", 0)
{% endhighlight %}



###### Motivating Example: Using C Data Structures

FFI库也允许你创建和访问C数据结构。当然主要目的是为了方便的与C函数进行交互，但是也可以单独的使用这些数据结构。

Lua是一门构建于高级数据类型之上的编程语言，其具有灵活性、可扩展性、以及动态性等特点，这也是我们为什么热爱Lua的原因。但是这在很多方面可能也会导致效率低下，此时你可能会想要使用一种更低级(low-level)的数据类型，比如某一个固定数据结构(fixed structures)类型的大型数组，如果我们用Lua来实现的话，则需要一个大型的table，然后里面每一个元素是一个tiny table。这不但造成内存使用效率不高，还会导致性能不佳。

如下是一个操作彩色图像的骨架的benchmark。

1) **纯lua版本**

首先我们来看纯Lua版本的实现：
{% highlight string %}
local floor = math.floor

local function image_ramp_green(n)
  local img = {}
  local f = 255/(n-1)
  for i=1,n do
    img[i] = { red = 0, green = floor((i-1)*f), blue = 0, alpha = 255 }
  end
  return img
end

local function image_to_grey(img, n)
  for i=1,n do
    local y = floor(0.3*img[i].red + 0.59*img[i].green + 0.11*img[i].blue)
    img[i].red = y; img[i].green = y; img[i].blue = y
  end
end

local N = 400*400
local img = image_ramp_green(N)
for i=1,1000 do
  image_to_grey(img, N)
end
{% endhighlight %}
>注：我们执行上面的代码，耗时2.91s。

上面的代码会创建一个拥有160000个元素的table，其中每一个元素又是一个子table，该子table中含有4个元素。我们首先创建了一个green ramp的图像，之后再调用了1000次将其转换成灰度图像。


2) **FFI实现版本**

下面我们再来看一个FFI版本的实现(注意我们标注mark的部分)：
{% highlight string %}
local ffi = require("ffi")                                             ---mark 1
ffi.cdef[[
typedef struct { uint8_t red, green, blue, alpha; } rgba_pixel;
]]

local function image_ramp_green(n)
  local img = ffi.new("rgba_pixel[?]", n)                              ---mark 2
  local f = 255/(n-1)
  for i=0,n-1 do                                                       ---mark 3
    img[i].green = i*f                                                 ---mark 4
    img[i].alpha = 255
  end
  return img
end

local function image_to_grey(img, n)
  for i=0,n-1 do                                                       ---mark 3
    local y = 0.3*img[i].red + 0.59*img[i].green + 0.11*img[i].blue    ---mark 5
    img[i].red = y; img[i].green = y; img[i].blue = y
  end
end

local N = 400*400
local img = image_ramp_green(N)
for i=1,1000 do
  image_to_grey(img, N)
end
{% endhighlight %}
>注：我们执行上面的代码，耗时0.42s

下面我们来分析一下上面的代码：

* mark 1: 加载ffi库，并声明了一个low-level数据结构rgba_pixel。这里我们选择使用struct来保存一个像素的4个值(RGBA)

* mark 2: 直接使用ffi.new()来创建数据结构。在这里```?```是一个数组长度的占位符

* mark 3: C数组是从索引0开始的，因此在上面的代码中索引的范围是[0, n-1]。某些人可能会多分配一个元素来使其从1位置开始（0位置保留不用）

* mark 4: 由于ffi.new()默认会以0填充数组，因此这里我们只需要设置green与alpha字段即可

* mark 5: 这里可以省略对math.floor()的调用，这是因为当把浮点数转换成整数的时候，会自动向零方向进行截断(例如将1.1转换成整数，自动截断为1）。

现在我们来看一下上述改变所产生的影响： 首先内存占用从22MB下降为640KB(400*400*4)，下降幅度达到35倍。这让我们看到tables占用了过量的内存。顺便说一下，在x64平台上会占用多达40MB的内存空间。

其次，从性能方面考虑： 在我的机器上，纯Lua版本运行了大概2.91s，而FFI版本只运行了0.42s。性能方面也有7倍的差距。

聪明的读者可能会注意到：对于纯Lua版本，如果使用索引来访问颜色的话(用[1]来替换.read，[2]来替换.green等）会更经凑和高效。这当然是正确的，通常可以获得一个1.7倍的效率。但请注意，对于FFI版本，也同样可以使用此方式。










## 1.  FFI Semantics
本节描述FFI库与Lua及C语言交互的详细的语法。FFI库被设计成与C语言交互的接口，并可在其中以纯C语言来进行数据结构及函数的声明，几乎完全遵守C语言语法( it closely follows the C language semantics)。为了使与Lua语言语义的顺畅互操作需要一些小的让步



## 2. fffi.* API Functions

### 2.1  Glossary

* **cdecl**: 抽象的C类型声明（a Lua string)

* **ctype**: 一个C类型的对象。这是由ffi.typeof()返回的一种特定类型的cdata，作为cdata类型数据的构造函数使用

* **cdata**: 一种C数据对象，用于保存ctype对应的值

* **ct**: 一种C类型规范，可被用于大部分API函数。作为一个模板对象，可以是cdecl、ctype、或者cdata

* **cb**: 一个回调(callback)对象。这是一个C数据对象，用于保存一个特定的函数指针。在C代码中调用此函数，则会运行所关联的Lua函数

* **VLA**：变长数组

* **VLS**： 一个变长结构体就是C语言类型的struct，其最后一个element是变长的，例如
{% highlight string %}
struct student{
	int age;
	char info[0];
};
{% endhighlight %}

### 2.2 Declaring and Accessing External Symbols


###### 2.2.1 ffi.cdef(def)

可以在其中添加多个C类型、外部符号（外部变量、函数）的声明。def必须是一个Lua字符串。建议使用如下的方式的语法糖来作为string参数：
{% highlight string %}
ffi.cdef[[
  typedef struct foo { int a, b; } foo_t;    // Declare a struct and typedef.

  int dofoo(foo_t *f, int n);               /* Declare an external C function. */
]]
{% endhighlight %}

上面在```[[]]```之间的字串必须是一系列的C声明，由分号分割。如果是单个符号声明的话，最后一个分号可省略（不建议省略）。

请注意，这里仅仅只是声明，它们并没有被绑定到任何地址。绑定是通过C库的名称空间来获得的。

>注： 不要在上面声明中定义宏，除#pragma之外

顺带一提的是，并不是所有的 C 标准函数都能满足我们的需求，那么如何使用第三方库函数或自定义的函数 呢，这会稍微麻烦一点，不用担心，你可以很快学会。首先创建一个```myffi.c```，其内容如下：
{% highlight string %}
int add(int x, int y)
{
	return x + y;
}
{% endhighlight %}
然后在Linux下执行如下命令生成动态链接库：
<pre>
# gcc -g -o libmyffi.so -fpic -shared myffi.c
# ls
libmyffi.so  myffi.c
</pre>
之后我们编写一个Lua来调用（myffi.lua)：
{% highlight string %}
local ffi = require "ffi"
local myffi = ffi.load('myffi')
ffi.cdef[[
   int add(int x, int y); 
]]
local res = myffi.add(1, 2)
print(res) 
{% endhighlight %}
编译运行(需要采用LuaJIT来执行）：
<pre>
# pwd
/home/lzy/just_for_test
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
# /usr/local/openresty/luajit/bin/luajit ./myffi.lua
3
</pre>
上面由于我们的```libmyffi.so```动态链接库并不在标准的库搜索路径下，因此我们通过LD_LIBRARY_PATH来指定。

我们还看到在代码中有如下一行：
<pre>
ffi.load(name [,global])
</pre>

ffi.load()会通过给定的 name 加载动态库，返回一个绑定到这个库符号的新的C库命名空间，在 POSIX 系统中，如果 global 被设置为true ，这个库符号被加载到一个全局命名空间。另外这个name可以是一个动态库的路径，那么会根据路径来查找，否则的话会在默认的搜索路径中去找动态库。在 POSIX 系统中，如果在 name 这个字段中没有写上点符号```.```，那么 .so 将会被自动添加进去，例如 ffi.load("z") 会在默认的共享库搜寻路径中去查找 libz.so ，在 windows 系统，如果没有包含点号，那么 .dll 会被自动加上。

除此之外，还能使用```ffi.C```(调用ffi.cdef中声明的系统函数) 来直接调用add()函数，记得要在ffi.load()的时候加上参数 true ，例如 ffi.load('myffi', true)，这样就将该符号加载到全局C符号表中：
{% highlight string %}
local ffi = require "ffi"
ffi.load('myffi',true)
ffi.cdef[[
   int add(int x, int y);             /* don't forget to declare */
]]
local res = ffi.C.add(1, 2)
print(res)                            -- output: 3 Note: please use luajit to run this script.
{% endhighlight %}




<br />
<br />

参看:

1. [FFI库](http://luajit.org/ext_ffi.html)


<br />
<br />
<br />

