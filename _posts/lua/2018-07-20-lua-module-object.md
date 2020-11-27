---
layout: post
title: lua模块开发及面向对象开发
tags:
- lua
categories: lua
description: lua开发
---


本文简单记录一下Lua中的模块开发及面向对象开发的相关知识要点。


<!-- more -->


## 1. Lua中Module开发的基本要点
在Lua中创建一个Module其实很简单： 创建一个table，然后把所有我们想要导出(export)的方法加入到该table中，之后返回该table即可

### 1.1 示例1
参看如下示例(```simple_module.lua```)：
{% highlight string %}
--[[

    A simple module for complex numbers
  
--]]

local M = {}                     -- the module


-- creates a new complex number
local function new(r, i)
  return {r = r, i = i}
 end  
 
M.new = new                      -- add new to the module


-- constant 'i'

M.i = new(0, 1)


function M.add(c1, c2)
  
  return new(c1.r + c2.r, c1.i + c2.i)
end

function M.sub(c1, c2)
  
  return new(c1.r - c2.r, c1.i - c2.i)
end  

function M.mul (c1, c2)
  
  return new(c1.r*c2.r - c1.i*c2.i, c1.r*c2.i + c1.i*c2.r)
end

local function inv (c)
  
  local n = c.r^2 + c.i^2
  return new(c.r/n, -c.i/n)
end

function M.div (c1, c2)
  
  return M.mul(c1, inv(c2))
end

function M.tostring (c)
  
  return string.format("(%g,%g)", c.r, c.i)
end

return M
{% endhighlight %}
下面我们在另一个文件中调用此模块：
{% highlight string %}
local sm = require("simple_module")


print(sm.i)

local a = sm.new(4, 8)
local b = sm.new(1, 2)

print(sm.tostring(a), sm.tostring(b))


local c = sm.add(a, b)
local d = sm.sub(a, b)
local e = sm.mul(a, b)
local f = sm.div(a, b)

print(sm.tostring(c))
print(sm.tostring(d))
print(sm.tostring(e))
print(sm.tostring(f))
{% endhighlight %}
编译运行：
<pre>
table: 00000000005fa910
(4,8)	(1,2)
(5,10)
(3,6)
(-12,16)
(4,0)
</pre>

在上面的示例中，我们通过在```new()```及```inv()```前面加上local，从而使得这两个函数变为private。

另外可能有一些用户不太喜欢最后的```return M```语句，我们也可以采用如下方式来将对应的module table直接加入到package.loaded中：
{% highlight string %}
local M={}

package.loaded[...] = M

---as before, without the return statement
{% endhighlight %}

### 1.2 示例2
另一种编写module的方法就是将所有的function都定义为local，在最后返回导出表。参看如下示例(simple_complex.lua):
{% highlight string %}
--[[

    Module with export list
  
--]]

local function new(r, i)
  return {r = r, i = i}
end   

-- defines constant 'i'
local i = new(0, 1)

local function add(c1, c2)
  
  return new(c1.r + c2.r, c1.i + c2.i)
end

local function sub(c1, c2)
  
  return new(c1.r - c2.r, c1.i - c2.i)
end  

local function mul (c1, c2)
  
  return new(c1.r*c2.r - c1.i*c2.i, c1.r*c2.i + c1.i*c2.r)
end

local function inv (c)
  
  local n = c.r^2 + c.i^2
  return new(c.r/n, -c.i/n)
end

local function div (c1, c2)
  
  return mul(c1, inv(c2))
end

local function tostring (c)
  
  return string.format("(%g,%g)", c.r, c.i)
end

return {
    new = new,
    i   = i,
    ["add"] = add,             -- display to use another style
    sub = sub,
    mul = mul,
    div = div,
    tostring = tostring,
}
{% endhighlight %}

下面我们在另一个文件中调用此模块：
{% highlight string %}
local complex = require("simple_complex")


print(complex.i)

local a = complex.new(4, 8)
local b = complex.new(1, 2)

print(complex.tostring(a), complex.tostring(b))


local c = complex.add(a, b)
local d = complex.sub(a, b)
local e = complex.mul(a, b)
local f = complex.div(a, b)

print(complex.tostring(c))
print(complex.tostring(d))
print(complex.tostring(e))
print(complex.tostring(f))
{% endhighlight %}
编译运行：
<pre>
table: 00000000007aa8d0
(4,8)	(1,2)
(5,10)
(3,6)
(-12,16)
(4,0)
</pre>


### 1.3 submodules与packages
Lua允许module的名称具有层次结构，我们可以使用```.```来分割module的名称。例如，```mod.sub```就是mod的一个submodule。在Lua中，package其实就是一个树状的modules集合。其实这很好理解，通过将具有相似功能、或具有紧密联系的module放在一起，使其形成一个package，这样也方便人们阅读及管理。

当我们查询submodule的时候，require会将对应的```.```转换成另一个字符，通常是目录分隔符。例如，假设我们采用```/```来作为目录分隔符，并且有如下搜索路径：
<pre>
./?.lua;/usr/local/lua/?.lua;/usr/local/lua/?/init.lua
</pre>
那么当我们执行```require("a.b")```时，其将会尝试打开如下文件：
{% highlight string %}
./a/b.lua
/usr/local/lua/a/b.lua
/usr/local/lua/a/b/init.lua
{% endhighlight %}

上述行为允许一个pakcage中的所有module存在于同一个目录。例如，假设有一个pakage中有```p```、```p.a```、```p.b```三个module，则它们可以存在于同一个目录```p```中：
<pre>
p/init.lua
p/a.lua
p/b.lua
</pre>

>注： 对于C动态链接库并不支持submodule


## 2. metatables与metamethods
在Lua中，对于每一个值(value)通常都有一系列可预见的操作(operations)。我们可以对numbers进行相加，对strings进行concatenate操作，也可以在tables中插入key-value对，等等。然而，我们并不能够直接对tables进行相加，也不能够对functions进行比较，也不能直接对字符串进行函数调用。如果我们要实现这些，就需要用到metatables。

当遇到无法处理的的操作时，metatables允许我们修改对应value的行为。比如，有两个table，分别为a、b，我们可以使用metatables，从而实现a+b的操作。在任何时候，当Lua尝试对两个table进行相加时，都会检查其中是否有table具有metatable，并且该metatable是否有```__add```字段。假如Lua找到了该字段，其就会调用该字段的值（即所谓的```metamethod```)，来完成加法运算。

用面向对象的术语来说，我们可以认为metatables是一种严格类型的类(class)。与class类似，metatables定义了对应实例的行为。然而，metatables比普通的class更严格，因为其只能给预先定义好的一系列操作来指定相应的行为(behavior)；此外，metatables也不允许继承。

>注： 下一节我们介绍Object-Oriented Programming的时候，将会讲解如何使用metatables来构建一个完整的类。

在Lua中，每一个value都可以有一个metatable。对于table及userdata类型的值来说，其拥有独立的metatable；而其他类型的value，统一类型的各个实例都共用同一个metatable。Lua所创建的新tables都是没有metatable的：
{% highlight string %}
t = {}

print(getmetatable(t))    ---> nil 
{% endhighlight %}
我们可以使用```setmetatable()```方法来设置或修改一个table的metatable:
{% highlight string %}
t1 = {}
setmetatable(t, t1)

print(getmetatable(t) == t1)   ---> true
{% endhighlight %}

通过Lua，我们只能够将metatable设置为table类型的值；如果要操作其他类型的metatable值，则我们必须要用C语言代码或者使用debug库（之所以有这样的限制，主要原因在于防止对metatable的过度使用）。

string library对于字符串设置了一个metatable，而其他的类型默认情况下是没有metatable的：
{% highlight string %}
print(getmetatable("hi"))        --> table: 0x80772e0
print(getmetatable("xuxu"))      --> table: 0x80772e0
print(getmetatable(10))          --> nil
print(getmetatable(print))       --> nil
{% endhighlight %}

任何一个table都可以成为一个value的metatable；一组相关的table可以共用同一个metatable，用于描述它们共同的行为； 一个table也可以是其自身的metatable，这样其就可以实现自描述。

>注： 在面向对象的设计中，类的所有实例都共享同一个metatable。因此metatable在Lua的面向对象编程中，具有是非重要的地位。

### 2.1 Arithmetic Metamethods
在本节，我们会通过相关的示例来讲述metatable的一些基础知识。假设我们有一个module，其使用tables来表示集合(set)，在module中实现了计算集合的并集(union)、交集(intersection)等操作。参看如下示例(```arith_meta.lua```):
{% highlight string %}
--[[

   A simple module for sets

--]]

local Set = {}

-- create a new set with the values of a give list
function Set.new(l)
  
  local set = {}
  
  for _, v in ipairs(l) do
    
    set[v] = true 
  end  
  
  return set 
  
end  

function Set.union(a, b)
  
  local res = Set.new()
  
  for k in pairs(a) do
    res[k] = true 
  end
  
  for k in pairs(b) do 
    res[k] = true
  end
  
end   


function Set.intersection (a, b)
  
  local res = Set.new{}
  for k in pairs(a) do
    res[k] = b[k]
  end
  return res
  
end


-- presents a set as a string
function Set.tostring (set)
  
  local l = {} -- list to put all elements from the set
  for e in pairs(set) do
    l[#l + 1] = tostring(e)
  end
  return "{" .. table.concat(l, ", ") .. "}"

end



return Set
{% endhighlight %}





<br />
<br />

参看:




<br />
<br />
<br />

