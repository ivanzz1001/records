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

>注： C声明并不会通过C预处理器来传递，因此除#pragma外，不要使用预处理token。如果已存在的C头文件中含有#define，请将其替换为enum、static const或typedef，或者通过一个外部的C预处理器来传递该文件。请注意不要在头文件中包含无关的冗余头文件。




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

###### 2.2.2 ffi.C

这是一个默认的C库名称空间（请注意，这里是大写的```C```)，其绑定到了目标系统上的默认符号集或lib库上，这通常与C编译器所提供的默认符号集和lib库相同。

在Posix系统上，ffi.C绑定默认或全局名称空间的符号(symbols)，这包括所有加载进全局名称空间中的导出符号表，至少包含libc、libm、libdl(linux)、libgcc(假如采用gcc编译的话），以及LuaJIT自身所提供的Lua/C API中的符号表。

在Window操作系统上，ffi.C绑定到```*.exe```所导出的符号表、lua51.dll以及C运行时库```msvcrt*.dll```、kernel32.dll、user32.dll、gdi32.dll。

使用方法如下：
{% highlight string %}
clib = ffi.load(name [,global])
{% endhighlight %}
上面会加载name所指定的动态链接库，并且返回一个绑定对应符号表的新的C库名称空间。在Posix系统上，如果global设置为true，则相应库的符号表也会被加载进全局名称空间中。

假如name是一个路径的话，则会从该指定的路径加载lib库；否则name会根据不同的操作系统被规范化，然后在对应的搜索路径下查找。

在Posix系统上，假如name不包含```.```的话，那么会默认加上```.so```，同时在必要时也会加上前缀```lib```。因此，当我们执行ffi.load("z")时则会在搜索路径查找libz.so动态链接库。

在Windows操作系统上，假如name不包含```.```的话，那么默认会加上```.dll```。比如，当我们执行ffi.load("ws2_32")时则会在DLL默认查找路径下查找ws3_32.dll。


### 2.2 Creating cdata Objects

我们在上面对Lua如何调用C函数进行了介绍，但是光能调用C函数是远远不够的，我们还需要对C的变量，变量类型进行处理。

1） **ffi.typeof()**
{% highlight string %}
ctype = ffi.typeof(ct)
{% endhighlight %}
ffi.typeof()用于创建一个ctype对象。通常用其来解析cdecl一次，然后将产生的ctype对象用作constructor来使用。

参看如下示例：
{% highlight string %}
local ffi = require('ffi')

ffi.cdef[[
struct s1 {
    int a;
    int b;
};
typedef struct {
    int c;
    int d;
} s2;
union u {
    int a;
    long b;
    float c;
};
enum e {
    Male,
    Female
};
]]

print(ffi.typeof("int8_t"))
print(ffi.typeof("uint8_t"))
print(ffi.typeof("int16_t"))
print(ffi.typeof("uint16_t"))
print(ffi.typeof("int32_t"))
print(ffi.typeof("uint32_t"))
print(ffi.typeof("int64_t"))
print(ffi.typeof("uint64_t"))
print(ffi.typeof("double"))
print(ffi.typeof("float"))
print(ffi.typeof("bool"))
print(ffi.typeof("struct s1"))
print(ffi.typeof("s2"))
print(ffi.typeof("union u"))
print(ffi.typeof("enum e"))
print(ffi.typeof("struct s1*"))
print(ffi.typeof("struct s1[]"))
{% endhighlight %}
编译运行：
{% highlight string %}
ctype<char>
ctype<unsigned char>
ctype<short>
ctype<unsigned short>
ctype<int>
ctype<unsigned int>
ctype<int64_t>
ctype<uint64_t>
ctype<double>
ctype<float>
ctype<bool>
ctype<struct s1>
ctype<struct 98>
ctype<union u>
ctype<enum e>
ctype<struct s1 *>
ctype<struct s1 []>
{% endhighlight %}


2) **ffi.new()**

如下的API函数用于创建cdata对象，并且所创建的所有cdata对象都可以被gc的。
{% highlight string %}
cdata = ffi.new(ct [,nelem] [,init...])
cdata = ctype([nelem,] [init...])
{% endhighlight %}
上述代码用于实现为指定的```ct```创建一个cdata对象。如果是VLA/VLS类型的话，那么需要指定nelem参数。上面第二种语法形式使用ctype作为构造函数，除此之外完全等价。

创建出的cdata对象会使用可选的```init```参数，根据[rules for initializers](http://luajit.org/ext_ffi_semantics.html#init)来进行初始化。

>注： 假如你想创建某一类型的多个对象，请只解析```cdecl```一次，然后通过ffi.typeof()函数来获取其ctype，之后就可以重复的使用ctype来作为构造函数。

另外一点需要注意的是，当使用匿名的struct声明时，我们每一次在ffi.new()中使用它时都会创建一个新的、不同的ctype对象。这可能并不是我们想要的结果，特别是是我们想要创建多个cdata对象的情况下。在C语言标准中，不同的匿名struct通常是不能相互赋值的（即使它们有相同的fields，也不能相互赋值）。同样，JIT编译器也会将它们看做是不同的类型。通常我们还是建议在ffi.cdef()定义非匿名struct，或者通过ffi.typeof()来为匿名struct创建一个ctype对象。


参看如下示例：
{% highlight string %}
local ffi = require "ffi"
local myffi = ffi.load('myffi')
ffi.cdef[[
   int add(int x, int y); 
   ]]

--[[
    方式1： 直接调用
--]]
local result_1 = myffi.add(1, 2)
print("直接调用:".. result_1)

--[[
    方式2: 采用ffi.new()构造cdata
--]]
local ct = ffi.typeof("int")
local a = ffi.new(ct, 10)
local b = ffi.new("int", 20)
local result_2 = myffi.add(a, b)
print("ffi.new()方式：", type(ct), type(a) , type(b) , ct, a, b, result_2)

--[[
     方式3： 采用类型对象构造
--]]
local ct2 = ffi.typeof("int")
local c = ct2(30)
local d = ct2(40)
local result_3 = myffi.add(c, d)
print("ctype方式：" , type(ct2), type(c) , type(d) , ct2, c, d, result_3)
{% endhighlight %}
编译运行：
{% highlight string %}
# /usr/local/openresty/luajit/bin/luajit ./myffi.lua
直接调用:3
ffi.new()方式： cdata   cdata   cdata   ctype<int>      cdata<int>: 0x7f58e608b860      cdata<int>: 0x7f58e607f698      30
ctype方式：     cdata   cdata   cdata   ctype<int>      cdata<int>: 0x7f58e608ba58      cdata<int>: 0x7f58e608ba78      70
{% endhighlight %}
从上面的输出，我们看到cdata是封装C的一个object类型，通常我们不能直接使用print()来打印。ctype也是一种特殊的cdata。


----------
如下我们再选一个结构体类型的例子:

首先编写如下point.c文件：
{% highlight string %}
struct point{
    int x;
    int y;
};

struct point point_add(struct point **p)
{
    int res_x = p[0]->x + p[1]->x;
    int res_y = p[0]->y + p[1]->y;

    struct point res = {res_x, res_y};
    return res;
}
{% endhighlight %}
执行如下命令编译为动态链接库：
<pre>
# gcc -g -o libmypoint.so -fpic -shared point.c
</pre>

然后编写如下代码Lua代码来调用：
{% highlight string %}
local ffi = require "ffi"
local myffi = ffi.load('mypoint', true)
ffi.cdef[[
  struct point{
      int x;
      int y;
  };
  typedef struct point *pointp;

  struct point point_add(struct point **p);
]]


local ct1 = ffi.typeof("struct point")
local ct2 = ffi.typeof("struct point *[?]")

local a = ct1({1,1})
local b = ct1({2,2})

local pa = ffi.cast("struct point *", a)
local pb = ffi.cast("struct point *", b)

local pts = ffi.new(ct2, 2, pa, pb)
local res = myffi.point_add(pts)
print(res.x, res.y)
{% endhighlight %}
编译运行：
<pre>
# /usr/local/openresty/luajit/bin/luajit ./mypoint.lua
3       3
</pre>


3) **ffi.cast()**

语法形式如下：
{% highlight string %}
cdata = ffi.cast(ct, init)
{% endhighlight %}
用于为指定的ct创建一个scalar cdata对象，该cdata对象会被初始化为```init```的值。

该函数的主要用途是：

* 覆盖指针的兼容性检查

* 将指针转换成地址

* 将地址转换成指针

参看如下示例：
{% highlight string %}
local ffi = require "ffi"

local a = ffi.new("int", 1)
local b = ffi.new("int", 2)
local arr = ffi.new("int[?]", 2, 100, 200)

--[[
   转换为指针
--]]
local pa = ffi.cast("int *", a)   
local pb = ffi.cast("int *", b)

--[[
   将指针转换为数字
--]]
local c = ffi.cast("int", pa)
local d = ffi.cast("int", pb)

print(tonumber(c), tonumber(d), tonumber(arr[0]), tonumber(arr[1]))   -->1      2     100     200
{% endhighlight %}


4) **ffi.metatype()**

函数原型如下：
{% highlight string %}
ctype = ffi.metatype(ct, metatable)
{% endhighlight %}
用于为指定的ct创建一个ctype对象，并将其关联到metatable。ct只允许为struct/union、complex numbers、vectors。如果ct是其他类型的话，那么可能会自动将其封装为struct类型之后来操作。

上述方法关联的metatable是永久性的，并且之后不能被修改。无论是metatable本身，还是```__index```都不能被修改。当使用此类型的对象时，所关联的metatable就会被自动应用。

Lua所有的标准metamethods都已在ffi中实现了，可以直接调用。对于二元运算(binary operations)，首先会检查该值对应的ctype是否有对应的metamethod。```__gc```元方法只会作用于struct/union类型上，并且在创建的时候会隐式的执行ffi.gc()操作。

下面我们给出一个ffi.metatype()的例子：
{% highlight string %}
local ffi = require("ffi")


ffi.cdef[[
  typedef struct { 
    double x;
    double y; 
  } point_t;
]]


local point

local mt = {
  __add = function(a, b) 
      return point(a.x+b.x, a.y+b.y) 
    end,
    
  __len = function(a) 
      return math.sqrt(a.x*a.x + a.y*a.y) 
    end,
    
  __index = {
    area = function(a) 
      return a.x*a.x + a.y*a.y 
     end,
  },
}

point = ffi.metatype("point_t", mt)

local a = point(3, 4)
print(a.x, a.y)              --> 3     4
print(#a)                    --> 5


print(a:area())              --> 25

local b = a + point(0.5, 8)
print(#b)                    --> 12.5
{% endhighlight %}


5) **ffi.gc()**

函数原型如下：
{% highlight string %}
cdata = ffi.gc(cdata, finalizer)
{% endhighlight %}
为```指针```或者aggregate cdata对象关联一个finalizer。The cdata object is returned unchanged.

通过本函数，可以将不受管理的资源(unmanaged resources)安全的集成进Lua JIT的自动内存管理。典型的使用方法如下：
{% highlight string %}
local p = ffi.gc(ffi.C.malloc(n), ffi.C.free)

...

p = nil                    -- Last reference to p is gone.
                           -- GC will eventually run finalizer: ffi.C.free(p)
{% endhighlight %}
对于userdata对象来说，cdata finalizer的工作方式类似于```__gc``` metamethod：当引用cdata对象的最后一个引用取消后，所关联的finalizer就会被调用（注： 调用时，会将该cdata object作为参数传入）。finalizer可以是一个函数，或者是一个cdata function，或者是一个cdata function pointer。我们可以通过将finalizer设置为nil，这样就可以移除一个已存在的finalizer，例如，我们在手动删除resouce之前将其移除：
{% highlight string %}
ffi.C.free(ffi.gc(p, nil))            -- Manually free the memory.
{% endhighlight %} 


下面我们给出一个ffi.gc()的例子：
{% highlight string %}
local ffi = require("ffi")


ffi.cdef[[
  typedef struct { 
    double x;
    double y; 
  } point_t;
]]


local point = ffi.new("point_t", {3.0, 4.0})
local p = ffi.gc(point, function() 
      print("finalizer")
    end)
  
print(p.x, p.y)

p = nil

print("terminate")
{% endhighlight %}

另外需要注意的一点是：如果你要分配一个 cdata 数组给一个指针的话，你必须保持持有这个数据的cdata对象活跃，下面给出一个官方的示例：
{% highlight string %}
ffi.cdef[[
typedef struct { int *a; } foo_t;
]]


local s = ffi.new("foo_t", ffi.new("int[10]"))             -- WRONG!
local a = ffi.new("int[10]")                               -- OK
local s = ffi.new("foo_t", a)
-- Now do something with 's', but keep 'a' alive until you're done.
{% endhighlight %}

6) **ffi.new()与ffi.C.malloc()的区别**

顺便一提，可能很多人会有疑问，到底 ffi.new 和 ffi.C.malloc 有什么区别呢？

如果使用 ffi.new 分配的 cdata 对象指向的内存块是由垃圾回收器 LuaJIT GC 自动管理的，所以不需要用户去释放内存。如果使用 ffi.C.malloc 分配的空间便不再使用 LuaJIT 自己的分配器了，所以不是由LuaJIT GC 来管理的，但是，要注意的是ffi.C.malloc返回的指针本身所对应的 cdata 对象还是由 LuaJIT GC 来管理的，也就是这个指针的 cdata 对象指向的是用 ffi.C.malloc()分配的内存空间。这个时候，你应该通过 ffi.gc() 函数在这个 C 指针的 cdata 对象上面注册自己的析构函数，这个析构函数里面你可以再调用 ffi.C.free ，这样的话当 C 指针所对应的 cdata 对象被 Luajit GC 管理器垃圾回收时候，也会自动调用你注册的那个析构函
数来执行 C 级别的内存释放。


请尽可能使用最新版本的 Luajit ， x86_64 上由 LuaJIT GC 管理的内存已经由 1G->2G ，虽然管理的内存变大了，但是如果要使用很大的内存，还是用 ffi.C.malloc 来分配会比较好，避免耗尽了 LuaJIT GC 管理内存的上限，不过还是建议不要一下子分配很大的内存

7) **综合示例**

最后，作为本节的结束，我们给出一个综合的例子：
{% highlight string %}
local ffi = require("ffi")
ffi.cdef[[
  unsigned long compressBound(unsigned long sourceLen);
  
  int compress2(uint8_t *dest, unsigned long *destLen,
    const uint8_t *source, unsigned long sourceLen, int level);
    
  int uncompress(uint8_t *dest, unsigned long *destLen,
    const uint8_t *source, unsigned long sourceLen);
]]

local zlib = ffi.load(ffi.os == "Windows" and "zlib1" or "z")

local function compress(txt)
  local n = zlib.compressBound(#txt)
  local buf = ffi.new("uint8_t[?]", n)
  local buflen = ffi.new("unsigned long[1]", n)
  
  local res = zlib.compress2(buf, buflen, txt, #txt, 9)
  
  assert(res == 0)
  return ffi.string(buf, buflen[0])
end


local function uncompress(comp, n)
  local buf = ffi.new("uint8_t[?]", n)
  local buflen = ffi.new("unsigned long[1]", n)
  
  local res = zlib.uncompress(buf, buflen, comp, #comp)
  assert(res == 0)
  
  return ffi.string(buf, buflen[0])
end


-- Simple test code.
local txt = string.rep("abcd", 1000)
print("Uncompressed size: ", #txt)

local c = compress(txt)
print("Compressed size: ", #c)

local txt2 = uncompress(c, #txt)
assert(txt2 == txt)
{% endhighlight %}



### 2.3 C Type Information
如下的API函数可以返回C类型的相关信息。对于获取cdata对象的一些相关信息很有用：

1） **ffi.sizeof()**

{% highlight string %}
size = ffi.sizeof(ct [,nelem])
{% endhighlight %}
返回ct的字节数。假如不能获取到ct占用的字节数，那么返回nil(例如，void或者function类型，我们就不能获取到确切的大小）。如果要查询VLA/VLS类型，我们需要指定nelem参数。

2） **ffi.alignof**
{% highlight string %}
align = ffi.alignof(ct)
{% endhighlight %}
返回ct的至少需要多少字节对齐。

3) **ffi.offsetof()**
{% highlight string %}
ofs [,bpos,bsize] = ffi.offsetof(ct, field)
{% endhighlight %}
返回field相对于ct开始位置的偏移字节数，ct必须是一个struct。此外，对于bit field(即位域），还会返回field的position及field size，单位为bit。

4) **ffi.istype()**

{% highlight string %}
status = ffi.istype(ct, obj)
{% endhighlight %}
假如obj具有ct所指定的C类型的话，返回true，否则返回false。

说明，在进行比较时C类型限定符（如const等)会被忽略。对于指针类型，则会按标准指针兼容性规则来比较。


### 2.4 Utility Functions

2) **ffi.string()**
{% highlight string %}
str = ffi.string(ptr [,len])
{% endhighlight %}
从ptr指针处创建一个内置的Lua字符串。

假如可选参数len省略的话，则ptr会被转化为```char *```类型，并假设ptr所执行的data是以0结尾，这时字符串的长度会通过strlen()来计算。

否则，ptr会被转化成```void *```类型，并且由len来指定数据的长度。此时data内可以包含0，并且不需要是基于字节的（尽管这可能会导致大小端问题）。

本函数主要用于将由C函数返回的```const char *```指针转化为Lua字符串，然后保存起来或者将其传递给其他需要Lua string的函数。注意，这里Lua string只是对ptr所指向数据的拷贝，完成后与该块内存没有任何关系了。Lua string是纯字节的，可以含有任何的字节数据。

>注：假如知道字符串的长度的话，在调用此函数时指定len会获得更高的性能。



4） **ffi.fill()**
{% highlight string %}
ffi.fill(dst, len [,c])
{% endhighlight %}
以数值c来填充dst所指向len长度的空间。假如c忽略，那么则会默认填充为0。

>注： ffi.fill()可能会作为一个inline函数被调用，用于替代C库中的memset(dst,c, len)

参看如下示例：
{% highlight string %}
local ffi = require("ffi")

local a = ffi.new("unsigned char[?]", 5)

ffi.fill(a, 20, 10)

for i=0, 4 do
  
  print(tonumber(a[i]))
end  
{% endhighlight %}


### 2.5 Target-specific Information

1） **ffi.abi()**
{% highlight string %}
status = ffi.abi(param)
{% endhighlight %}
假如param所指定的参数与目标平台(ABI: Application Binary Interface)相符的话，返回true；否则返回false。当前支持如下参数：

* 32bit: 是一个32位系统架构

* 64bit: 是一个64位系统架构

* le: 小端字节架构

* be: 大端字节架构

* fpu: 目标支持硬件fpu

* softfp: softfp调用准则

* hardfp: hardfp调用准则

* eabi: EABI variant of the standard ABI

* win: Windows variant of the standard ABI

下面我们给出一个示例：
{% highlight string %}
local ffi = require("ffi")

print("32bit:", ffi.abi("32bit"))
print("64bit:", ffi.abi("64bit"))
{% endhighlight %}

2) **ffi.os**

返回目标操作系统的名称。

{% highlight string %}
print(ffi.os)
{% endhighlight %}

3) **ffi.arch**

返回目标操作系统的架构名称。
{% highlight string %}
print(ffi.arch)
{% endhighlight %}

### 2.6 Methods for Callbacks

### 2.7 Extended Standard Library Functions

LuaJIT扩展了如下的标准库函数，用于搭配cdata对象工作：

1） **tonumber()**

{% highlight string %}
n = tonumber(cdata)
{% endhighlight %}
将一个number cdata对象转换成double值，并返回为一个Lua number。注意，在进行转换时可能会损失精度。

2） **tostring()**
{% highlight string %}
s = tostring(cdata)
{% endhighlight %}
对于一个64bit整数(nnnLL或者nnnULL)返回其字符串表示，或者对complex numbers(ri + img i)返回其字符串表示。否则，对于ctype对象返回```ctype<type>```这样的表示，对于cdata对象返回```cdata<type>： address```这样的表示（除非你使用```__tostring```这样的metamethod进行了覆盖）

3） **pairs()、ipairs()**
{% highlight string %}
iter, obj, start = pairs(cdata)
iter, obj, start = ipairs(cdata)
{% endhighlight %}
调用对应ctype的```__pairs```或```__ipairs```元方法

### 2.8 Extensions to the Lua Parser
<br />
<br />

参看:

1. [FFI库](http://luajit.org/ext_ffi.html)

2. [luajit ffi 小结](https://blog.csdn.net/alexwoo0501/article/details/50636785)

<br />
<br />
<br />

