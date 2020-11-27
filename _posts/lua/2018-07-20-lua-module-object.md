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






<br />
<br />

参看:




<br />
<br />
<br />

