---
layout: post
title: lua模块开发
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

  return res 
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

现在，我们想要使用```加法```操作来实现两个集合的并集。为了实现此目的，我们会使所有代表集合的table都共享同一个metatable。在该metatable中将会定义如何响应加法操作。我们的第一个步骤就是创建一个普通的table，然后将其设置为集合表的metatable:
{% highlight string %}
local mt = {}            -- metatable for sets
{% endhighlight %}

接下来我们修改```Set.new()```方法，在其中添加一行，用于将mt设置为集合的metatable:
{% highlight string %}
function Set.new(l)             --- 2nd version

	local set = {}
  setmetatable(set, mt)
  
  for _, v in ipairs(l) do
    set[v] = true 
  end

  return set 
end
{% endhighlight %}
修改之后，每一次我们调用Set.new()来创建集合的时候，创建出来的集合都拥有相同的metatable：
{% highlight string %}
s1 = Set.new{10, 20,30, 50}
s2 = Set.new{30, 1}

print(getmetatable(s1)   --> table: 0x00672B60
print(getmetatable(s1)   --> table: 0x00672B60
{% endhighlight %}


最后，我们向metatable中添加一个metamethod ```__add```，通过该元方法字段来描述如何执行加法操作：
{% highlight string %}
mt.__add = Set.union
{% endhighlight %}

这样设置之后，当我们只想两个集合的加法操作时，其就会调用Set.union()函数。

如下是我们采用加法操作来求集合的并集：
{% highlight string %}
s3 = s1 + s2
print(Set.tostring(s3))     --> {1, 10, 20, 30, 50}
{% endhighlight %}

相似地，我们也可以使用乘法来实现交集操作：
{% highlight string %}
mt.__mul = Set.intersection

print(Set.tostring((s1 + s2)*s1)) --> {10, 20, 30, 50}
{% endhighlight %}



----------

在Lua中对于每一种算术操作，都有一个对应的metamethod名，下面我们简单列出：

* ```__add```: 加法操作

* ```__sub```: 减法操作

* ```__mul```: 乘法操作

* ```__div```: 除法操作

* ```__idiv```: floor division

* ```__unm```: 负号操作

* ```__mod```: 求模运算

* ```__pow```: 指数运算

同样，对位操作也有相关的metamethod:

* ```__band```: 按位与操作

* ```__bor```: 按位或操作

* ```__bxor```: 按位异或

* ```__bnot```: 按位取反

* ```__shl```: 左移操作

* ```__shr```: 右移操作

此外，我们也可以通过```__concat```这样一个metamethod来实现concatenation operator。



----------
当我们执行两个集合的加法操作时，会调用对应metamethod来进行，这一点是没有疑问的。但是假如有一个表达式，其对应的两个操作数各自有不同的metamethod，例如：
{% highlight string %}
s = Set.new({1,2,3})

s = s + 8
{% endhighlight %} 
Lua在查找metamethod时，遵循如下的步骤： 假如第一个操作数有metatable且含有对应操作的metamethod，那么lua就会直接使用此metamethod， 此时并不理会第二个操作数是什么；否则，假如第二个操作数有metatable且含有对应操作的metamethod，那么Lua就使用此metamethod； 否则，Lua不能执行此操作，并报告相应的错误。因此在上面的例子中，其会调用Set.union()操作。

Lua并不关心这些混合类型的操作，但是我们在实现时应该需要考虑。假如我们运行```s = s + 8```，那么在函数Set.union()中将会产生如下的错误提示：
<pre>
bad argument #1 to 'pairs' (table expected, got number)
</pre>

假如我们想要获取到更详细的错误消息，在Set.union()函数的实现中，我们需要检查两个操作数的类型，例如：
{% highlight string %}
function Set.union(a, b)
  if getmetatable(a) ~= mt or getmetatable(b) ~= mt then
    error("attempt to 'add' a set with a non-set value", 2)
  end
  
  local res = Set.new({})
  
  for k in pairs(a) do
    res[k] = true 
  end
  
  for k in pairs(b) do 
    res[k] = true
  end
  
  return res 
  
end 
{% endhighlight %}
在这里，我们注意到error()函数的第二个参数，其用于设置调用此操作时，在哪一层级打印对应的错误消息。

### 2.2 Relational Metamethods
metatables也允许我们自定义关系运算符的含义。我们可以通过如下metamethod:

* ```__eq```: 相等运算的判断

* ```__lt```: 小于

* ```__le```: 小于等于

但是请注意，并没有单独的metamethods来实现另外的三个关系操作： Lua会将```a ~= b```转换为```not (a == b)```运算； 将```a > b```转换为```b < a```运算；将```a >= b```转换为```b <= a```运算。

在老版本中，Lua会将所有的关系运算符转换成一个单独的运算，例如将```a <= b```转换成```not (b < a)```，但是这种转换存在一定的问题（这里我们不做详细介绍）。

在我们上面集合的例子中，其实也是会有相似地问题。在集合运算中，```<=```运算的含义一般为集合的包含关系： a<=b表示a是b的一个子集。假如按照此含义来解释，那么可能同时出现```a <=b```与```b < a```都为false的情况。例如a={1,2}, b = {3}，此时```a<=b```为false，且```b < a```也为false。因此，我们必须同时实现```__le```和```_lt```操作：
{% highlight string %}
mt.__le = function (a, b) -- subset
  for k in pairs(a) do
    if not b[k] then return false end
  end

  return true
end

mt.__lt = function (a, b) -- proper subset
  return a <= b and not (b <= a)
end
{% endhighlight %}

最后，我们可以定义两个集合是否相等的实现：
{% highlight string %}
mt.__eq = function (a, b)
   return a <= b and b <= a
end
{% endhighlight %}

在拥有上面这些定义之后，我们就可以对两个集合进行比较了：
{% highlight string %}
s1 = Set.new{2, 4}
s2 = Set.new{4, 10, 2}

print(s1 <= s2) --> true
print(s1 < s2) --> true
print(s1 >= s1) --> true
print(s1 > s1) --> false
{% endhighlight %}
对于相等的比较具有一些限制。假如两个object具有不同的基本类型，则进行相等比较时返回```false```，此时甚至不会调用metamethod。因此，Set集合与一个number比较时永远返回false，而不会管metamethod如何定义。

### 2.3 Library-Defined Metamethods
到目前为止，我们所遇见的所有metamethods都是由Lua内核来使用的。Lua虚拟机会检测对应操作数的metatable，然后找到对应的metamethod来执行相关操作。然而，由于metatables也是一个普通的table，任何用户都可以使用。因此，在对应的Lib库中，也经常会定义和使用自己的metatable。

函数tostring()是一个典型的例子。正如我们前面所看到的，对一个table使用tostring()时会返回如下简单信息：
{% highlight string %}
print({}) --> table: 0x8062ac0
{% endhighlight %}

上面的print()函数总是会调用tostring()方法来格式化输出。然而，当格式化一个值时，tostring()首先会检查是否有一个名为```__tostring```的metamethod。在这种情况下，Lua会调用此metamethod来格式化对应的object。该metamethod的返回值就是tostring()函数的返回值。

在我们上面的Set集合的例子中，我们已经定义了一个函数来格式化set。因此我们只需要将此函数设置为metatable的```__tostring```域：
{% highlight string %}
mt.__tostring = Set.tostring
{% endhighlight %}

这样设置之后，当我们调用print()来打印一个集合时，print就会调用该集合的tostring()方法来完成输出：
{% highlight string %}
s1 = Set.new{10, 4, 5}
print(s1) --> {4, 5, 10}
{% endhighlight %}

函数setmetatable()和getmetatable()也同样会使用一个metafield，通过这种方法来保护metatables。假设想要保护自己定义的集合(Set)，以防用户看见或修改集合的metatable，我们可以在metatable中设置```__metatable```字段，这样getmetatable()就会返回该字段的值，而调用setmetatable()则会报错：
{% highlight string %}
mt.__metatable = "not your business"
s1 = Set.new{}

print(getmetatable(s1)) --> not your business
setmetatable(s1, {})
stdin:1: cannot change protected metatable
{% endhighlight %}


从Lua5.2版本开始，pairs也有一个metamethod，因此我们可以修改table的遍历方式，以及为非table对象添加相应的变量方法。当一个对象有```__pairs```这样一个metamethod时，pairs()将会调用此方法来完成相应工作。

### 2.4 Table-Access Metamethods
算术运算、位运算、以及关系运算的metamethods都定义了相应的方式来应对各种错误场景，它们并不会改变Lua语言所定义的正常行为。Lua也提供了一种方式来修改table的行为，主要适用于如下两种场景： 

* 访问table中所缺失的fields

* 修改table中所缺失的fields


###### 2.4.1 The ```__index``` metamethod

在前面我们看到，当访问table中缺失的field时，返回nil。事实上，这种访问会触发Lua解释器查找```__index```这样一个metamethod： 假如并没有这样一个metamethod，则结果会返回nil； 否则会调用该metamethod来提供相应的结果。



关于```__index```的典型例子就是继承(inheritance)。假设我们想要创建多个table来描述windows，每一个table都必须描述多个windows参数，比如position、size、color schema等等。所有的这些参数都有默认值，因此当我们想要创建window对象的时候，我们只需要传递非默认值参数即可。第一种方法是提供一个构造函数来填充缺省字段；第二种方法就是就是创建```新window```来从```prototype window```那里继承缺省field。我们首先声明如下原型：

{% highlight string %}
-- create the prototype with default values
prototype = {x = 0, y = 0, width = 100, height = 100}

{% endhighlight %}

之后，我们定义一个构造函数来创建```新window```，这些新window共享同一个metatable:
{% highlight string %}
local mt = {}   -- create a metatable

--declare the constructor function
function new(o)
  setmetatable(o, mt)
	
  return o
end
{% endhighlight %}
现在我们来设置metamethod ```__index```:
{% highlight string %}
mt.__index = function(_, key)
   return prototype[key]
end
{% endhighlight %}
写完上述代码之后，我们创建一个新的window对象，然后来查询其缺省的field:
{% highlight string %}
w = new({x = 10, y = 20})

print(w.width)   -->100
{% endhighlight %}

上面的代码中，当Lua检测到w并没有所请求的字段，但是有一个metable，在metable中有```__index```，因此Lua就会调用```__index```元方法，并在调用时传递两个参数：w(the table)和width(the absent key)。在```__index```方法中会查找prototype从而返回相应的结果。

在Lua中使用```__index```这样一个metamethod来实现继承是十分常见的，为此Lua提供了另一种简单的实现方式。除了可以将```__index```设置为一个function外，还可以直接将一个table赋值给```__index```，如此当访问缺失的field时，就会直接访问```__index```所指向的table。仍然拿上面的例子来说明，我们可以直接采用如下的方式来定义```__index```:
{% highlight string %}
mt.__index = prototype
{% endhighlight %}

现在，当Lua查询到metatable的```__index```域时，其发现值为prototype（是一个table)，因此Lua就会再一次访问该table，执行prototype["width"]操作，从而返回相应的结果。

通过将```__index```设置为一个table，就可以简单快速的实现单继承。如果将```__index```设置为一个函数，则可能会使得代码更臃肿，但是却可以提供更好的灵活性： 可以实现多继承、caching、以及其他的一些功能。关于继承(inheritance)我们会在Lua面向对象编程一章中进一步讲解。

当我们想要访问一个table，但是并不想触发调用其```__index```元方法的话，我们可以使用```rawget()```方法。调用rawget(t, i)方法时，会以raw的方式来访问table t： 即这是一个primitive访问，并不会关心metable。以raw的方式访问相关field并不能提高效率，只是有的时候我们可能需要此特性。




###### 2.4.2 The ```__newindex``` metamethod

metamethod ```__index```主要用于table对field的访问，而```__newindex```主要用于table对field的更新。当我们为table的缺省field赋值时，Lua解释器会查询```__newindex```元方法： 假如有该metamethod，那么解释器就会调用此方法，而并不会直接进行赋值操作。与```__index```类似，假如```__newindex```的值是一个table的话，那么解释器就会对该table来进行赋值（而不是对原始table进行赋值）。同样，Lua也提供了一个raw function来忽略metamethod: 调用rawset(t, k, v)等价于不触发任何metamethod调用的t[k] = v操作。

通过组合使用```__index```和```__newindex```，我们可以获得十分强大的构造能力，比如read-only tables、默认值table、面向对象编程的继承特性等。




###### 2.4.3 Tables with default values

通常情况下table中任何字段的默认值都为nil，我们可以通过metatable来轻易的改变默认值：

{% highlight string %}
function setDefault(t, d)

  local mt = {
    __index = function()
      return d
    end
  }

  setmetatable(t, mt)

end 

tab = {x = 10, y = 20}
print(tab.x, tab.z)      -->10 nil
setDefault(tab, 0)
print(tab.x, tab.z)      -->10 0
{% endhighlight %}
在上面的代码中，我们调用setDefault()之后，任何访问tab缺省字段时都会调用```__index```，该函数会返回0.

函数setDefault()创建了一个闭包(closure)，以及为每个需要默认值的table创建了metatable。假如我们有许多table都需要默认值的话，这可能会导致产生十分多的closure和metatable，因此代价可能极高。然而，由于metatable已经将默认值d包装进其对应的metamethod中了，因此我们并不能使用一个单独的metatable来设置不同的默认值。为了使得不同的table都可以共用同一个metatable，我们可以在每个table中选用一个没有被占用的field来存放默认值。例如，我们可以选择```___```来存放此默认值：
{% highlight string %}
local mt= {
  __index = function(t)
    return t.___
  end
}

function setDefault(t, d)
  t.___ = d
  setmetatable(t, mt)
end
{% endhighlight %}

上面的代码我们只在setDefault()函数外部创建了一次metatable（及相应的metamethod)，所耗费的代价明显会比前面一个示例低。

假如我们担心名称冲突的话，我们也很容易保证相应key的唯一性。我们只需要用一个```exclusive table```来作为key即可：
{% highlight string %}
local key = {} -- unique key

local mt = {
  __index = function (t) 
    return t[key] 
  end
}

function setDefault (t, d)
  t[key] = d
  setmetatable(t, mt)
end
{% endhighlight %}

###### 2.4.4 Tracking table accesses
假设我们需要跟踪对table的每一次访问，使用```__index```或```__newindex```只能够应对哪些缺省的field，因此如果要捕获对table的所有访问，则我们需要将table设置为空表。假如我们我们要监控对table的所有访问操作，我们可以创建一个代理(proxy)。该proxy是一个空表(empty table)，其拥有相关的metamethods来跟踪所有的访问，并将访问请求转发到原表。下面的代码是一个实现的案例：
{% highlight string %}
--[[

  Tracking table access

--]]

function track(t)
  
  local proxy = {}    --proxy table for t
  
  --create a metatable for the proxy
  local mt = {
    __index = function(_, k)
      
      print("*access to element " .. tostring(k))
      return t[k]   --access the original table
      
    end,
    
    __newindex = function(_, k, v)
      
      print("*update of element " .. tostring(k))
      t[k] = v       --update original table
      
    end,
    
    __pairs = function()
      
      return function(_, k)       --iteration function
        local nextkey, nextvalue = next(t, k)
        
        if nextkey ~= nil then    --avoid last value
          print("*traversing element " .. tostring(nextkey))
        end
        
        return nextkey, nextvalue
        
      end  
      
    end,
    
    __len = function()
      return #t
    end  
    
  }
  
  
  setmetatable(proxy, mt)
  
  return proxy
  
end  
{% endhighlight %}

如下我们展示如何使用此代理(proxy):
{% highlight string %}
t = {}
t = track(t)

t[2] = "hello"      -->*update of element 2

print(t[2])         -->*access to element 2
                    --> hello
{% endhighlight %}
上面的例子中，metamethod ```__index```和```__newindex```遵循了我们设定的规则： 跟踪每一次访问，并且将访问请求转发到原表。对于```__pairs```允许我们遍历proxy，就像是直接遍历原table那样。最后```__len```会返回原table的长度，主要是为了使代理(proxy)也能处理长度运算符：
{% highlight string %}
t = track({10, 20})
print(#t)          --> 2

for k, v in pairs(t) do
 print(k, v) 
end


--> *traversing element 1
--> 1 10
--> *traversing element 2
--> 2 20
{% endhighlight %}

假如我们想要监控(monitor)多个table的话，我们并不需要为每一个table设置不同的metatable。相反，我们可以通过某种方式将每一个proxy映射到原表，然后让所有的代理都共用同一个metatable。可以参看上面介绍的**Table with default values**一节。


###### 2.4.5 Read-only tables

采用代理(proxy)的概念，我们可以很容的实现read-only tables。我们所需要做的就是在任何试图修改table时，报告相应的错误。对于```__index```这个metamethod，我们可以将其值直接设置为原表，因为我们并不需要监控查询操作。参看下面的示例：
{% highlight string %}
function readOnly (t)
  local proxy = {}
  local mt = { -- create metatable
    __index = t,
    __newindex = function (t, k, v)
      error("attempt to update a read-only table", 2)
    end
  }
  
  setmetatable(proxy, mt)
  return proxy
end
{% endhighlight %}

然后，我们可以为weekdays来创建一个只读表：
{% highlight string %}
days = readOnly{"Sunday", "Monday", "Tuesday", "Wednesday","Thursday", "Friday", "Saturday"}

print(days[1]) --> Sunday
days[2] = "Noday"
--> stdin:1: attempt to update a read-only table
{% endhighlight %}



<br />
<br />

参看:




<br />
<br />
<br />

