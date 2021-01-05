---
layout: post
title: lua基本概念
tags:
- lua
categories: lua
description: lua开发
---

本章介绍一下Lua的一些基本概念，使我们从整体上对Lua有一个基本的了解(如下讲解针对Lua 5.4.1版本）。

<!-- more -->


## 1. 值(Values)与类型(Types)

Lua是一种动态类型的语言，这就意味着变量(variables)并没有类型，只有值。在Lua语言中并没有类型定义，所有的值(Values)都携带了他们自己的类型。
c
在Lua中所有的values都是一级(first-class)的，这就意味着任何的value都可以保存在变量中，通过参数传递给函数，或者作为一个结果被返回。

在Lua中有8种基本的类型：

* ni： 本类型只有一个值**nil**，其主要目的就是为了区分其他的value，通常用于指示一个变量并没有一个有效值

* boolean： 本类型有两个值(**false**和**true**)

>注：在进行条件判断时**nil**以及**false**都会被判断为false，他们都被统一称作false值，而任何其他的值都会被判断为true。

* number: 代表了整数(integer)类型以及实数浮点(real floating-point)类型，其有两个子类型： ```integer```和```float```。标准的Lua使用64位整数(integer)类型以及双精度(double-precision)浮点类型，但其实你也可以将Lua重新编译为32位整数类型与32位单精度浮点类型。主要是在一些小型机或者嵌入式系统上，我们可能会将Lua的number类型编译为32位整数与32位浮点数类型（参看luaconf.h头文件中的LUA_32BITS宏定义）。

除非另有说明，否则根据整数补码的通常规则，在处理整数值时所有溢出都会环绕。（换句话说，实际结果是唯一可表示的整数，该整数与数学结果的模2^n相等，其中n是整数类型的位数。）

关于每一种子类型(subtype)在什么时候被使用， Lua都具有详尽的规则，但其也可以在需要时进行自动的转换。因此，我们在编程时大部分情况下可以忽略integer与float的区别。


* string： 代表了一个不可变(immutable)的字节序列。Lua是8位纯净值：字符串可以包含任何8位值，包括嵌入的零（```'\0'```）。Lua也与编码无关。它不对字符串的内容做任何假设。Lua中任何字符串的长度必须适合Lua整数。

* function: Lua可以调用Lua函数或者C语言函数，这两者都可以用function来表示。

* userdata： 主要是用于将任何的C语言数据保存到Lua变量中。一个userdata代表了一块原始内存(raw memory)。有两种不同种类的userdata，分别为

  * full userdata: 由Lua管理的具有一块内存的对象

  * light userdata: 它是一个C语言指针值

Lua中对于userdata类型，除了赋值操作以及identity test之外，其并没有预先定义好的操作。通过使用metatables，开发人员可以为userdata定义相关的操作。我们并不能够直接通过Lua来创建或修改userdata值，而是要通过C API的方式来进行，这就保证了宿主程序(host program)以及C库对起拥有数据的完整性。


* thread： 代表了一个独立的执行线程，主要被用于实现协程(coroutines)。Lua线程与操作系统的线程是两个概念，没有直接的相关性。Lua在所有系统上都支持coroutines，甚至包括哪些本身就不支持原生线程的系统。

* table： 本类型用于实现关联数组（关联数组的索引序号可以为除```nil```与```NaN```之外的任何类型）。Tables可以是异构的；也就是说，它们可以包含所有类型的值（nil除外）。与值nil相关的任何键(key)均不视为table的一部分。相反，任何不属于表的键都具有关联值nil。

Table是Lua中唯一的数据结构机制，它们可以用来表示普通数组，列表(list)，符号表(symbol tables)，集合(sets)，记录(records)，图形(graphs)，树(trees)等。为了表示记录(records)，Lua使用字段名(field name)来做索引，为了实现此，Lua在语言层面通过提供a.name语法糖来支持这种表示a["name"]。在Lua中，有多种十分方便的方法来创建tables。

类似于索引，table的值域也可以是任何类型。特别指出的是，由于functions也是顶级(fist-class)类型的值，因此table的值域也可以包含functions。因此table也可以用来容纳methods。

tables的索引遵循该语言中原始相等性的定义。 当且仅当和原始相等（即没有metamethods时等于）时，表达式a[i]和a[j]表示同一表元素。特别是，具有整数值的浮点数等于其各自的整数（例如 1.0 == 1）。为避免歧义，如果一个浮点数作为tables的key，那么其将自动装换为对应的整数。例如，如果您编写```a[2.0] = true```，那么实际插入到table中的key值为整数2。


tables、functions、threads、full userdata值都是object类型： 变量并不会真正保存它们的值，而是保存着它们的引用。赋值(assignment)、参数传递(parameter passing)、以及函数返回均是返回对应值的引用，这就意味着不会有任何形式的拷贝。

## 2. Environments and the Global Environment

对任何一个未声明的变量名(称为free name)引进引用，其都会被转换为```_ENV.var```。而且，每一个chunk都是在一个外部的局部```_ENV```中被编译，因此```_ENV```本身并不是一个free name。请参看如下示例：
{% highlight string %}
 --file: testEnv.lua
    
x = 300
y = 100

print(_ENV == _G)   -- prints true, since the default _ENV is set to the global table

print("pos x : " .. x .." pos y : " .. y)   -- pos x : 300 pos y : 100

local function setEnv(t)
    assert(type(t) == "table" , "Notice: The env you pass ".. type(t) .." is not a table .")
    local print = print
    local _ENV = t
    --get the local variable x , y  and print , as same as _ENV.x , _ENV.y , _ENV.print
    print("pos x : " .. x .." pos y : " .. y)   -- pos x : 800 pos y : 500
end

local env = {x =800 , y = 500 }
setEnv(env)

print("pos x : " .. x .." pos y : " .. y)   -- pos x : 300 pos y : 100
{% endhighlight %}
运行结果如下：
<pre>
true
pos x : 300 pos y : 100
pos x : 800 pos y : 500
pos x : 300 pos y : 100
</pre>



不管是否存在外部的```_ENV```变量，或者是否需要对free name进行装换，```_ENV```其实也可以完全作为一个普通名称来使用。你可以使用该名称(```_ENV```)来定义新的变量或参数。例如，在上面代码的```setEnv```函数中，我们就使用```_ENV```命令定义了一个局部变量，这样它就覆盖了chunk中默认传进来的```_G```这个ENV。


任何作为```_ENV```值的table都可以摆称为```environment```,例如在上面的setEnv()函数中，t就可以称为environment。

此外，Lua还保留了一个不同的environment称为全局环境。该global environment被保存在Lua C库的一个特殊的索引中。在Lua中，会将global environment初始化到一个名为```_G```的全局变量中。如果```_G```全局变量，其影响的仅仅是你自己的代码，并不会影响底层Lua C库中的global environment。

当Lua加载一个chunk的时候，其对应的```_ENV```的默认值就是global environment。因此，默认情况下，Lua代码中的free names引用的都是global environment，所以我们也可以将这些free names称为全局变量。另外，所有的标准库(standard library)都是在global environment中加载，并且其中的一些函数也需要在该global environment中运行。你可以使用```load```(或者```loadfile```)来加载一个不同environment的chunk（如果在C代码中，你必须先加载对应的chunk，然后修改其第一个upvalue值）。

## 3. Error Handling
在Lua中，有许多操作会导致报告相应的错误。error会中断程序的正常执行流程，我们可以通过捕获相应的错误来使得程序能够继续执行。

此外，我们也可以通过调用error()函数来显示的报告一个错误。

在Lua中，要捕获相应的error，我们可以使用pcall或者xpcall来调用一个函数。pcall会采用保护模式(protect mode)来调用函数，如果在函数运行过程中发生错误，都会将控制权马上返回给pcall，然后pcall返回一个对应的状态码。

由于Lua是一种可扩展的嵌入式语言，Lua代码的执行是由对应宿主程序的C代码来调用的（如果单独使用Lua的话，那么```lua程序```就是对应的宿主程序）。通常我们都是以```protected```模式来调用Lua，这样在编译或执行Lua代码块的过程中如果出现错误，那么可以将控制权交还给宿主程序，然后宿主程序就可以采取适当的措施，比如打印一条错误消息日志。

无论何时产生错误，都会产生一个```error object```以描述相关的错误消息。但是请注意，Lua本身内部所产生error object都是string类型，你也可以自己产生其他类型的error object。至于如何处理error object，这是由开发人员本身来决定的。由于历史原因，error object通常也被称为error message，尽管有一些error object可能并不是string类型。

当你使用```xpcall```(C语言中使用lua_pcall)时，可以在相应的函数参数中传递一个message handler，这样当错误发生的情况下，就会回调相应的message handler。该回调函数被执行时，会传递进入原始的error object，然后返回一个新的error object。

## 4、Metatables与Metamethods
Lua中的每一个值(value)都```可以拥有```一个对应的metatable。metatable其实是一个普通的Lua table，其定义了原始value在各种事件下的行为。你可以改变value对应的metatable中的相关字段，从而改变该value的一些行为。例如，当一个非数值(non-numeric)value作为加法操作的一个操作数时，Lua会检查该value对一个metatable的```__add```字段。假如发现有该字段，那么其就会调用该字段来执行加法操作。

metatable表中每一个事件对应的key值都是一个字符串，格式为```__event_name```，key对应的value被称为metavalue。对于大部分事件来说，metavalue都必须是一个函数(function)，被称为metamethod。例如，在前面的例子当中，key为```__add```，对应的metamethod则为执行加法操作的回调函数。除非另行说明，否则metamethod通常是一个callable value，即是一个回调函数或者是一个拥有__call metamethod的value。

我们可以使用getmetatable()函数来查询相应的value值；可以使用setmetatable()来替换相应的metatable。默认情况下，只有string类型的value有一个metatable，而其他类型并没有metatable。

metatable所支持的一些操作事件有：
<pre>
__add    __sub    __mul    __div    __mod    __pow    __unm    __idiv    __band    __bor    __bxor

__bnot   __shl    __shr    __concat  __len   __eq    __le    __index    __newindex    __call  
</pre>

除了上面所列出的事件key，metatables可以拥有如下key: ```__gc```、```__close```、```__mode```、```__name```(通常可作为tostring()方法的实现).

由于metatable是普通的table，因此其可以包含任意的key，而不仅仅是上面所例举的。标准lib库中的一些函数可能会利用到metatable中的其他一些字段来实现相关的特定功能。



## 5. 垃圾回收
Lua实现了自动的内存管理。这就意味着我们并不需要关心对象内存的分配与释放。Lua通过垃圾回收器来收集dead objects，从而实现内存的自动管理。Lua中所使用的所有内存都实现了自动的内存管理：strings、tables、userdata、functions、threads、internal structures等。

当GC检测到一个object在正常的程序执行过程(normal execution)中并不会再被访问到时，其就会认为该object处于dead状态。(这里normal execution并不包括finalizer，finalizer会导致dead object重新复活)。由于Lua并不了解C代码，因此其并不会通过寄存器来收集对象是否accessible的信息。

Lua中GC有两种工作模式： 增量模式、通用模式

通常情况下我们采用默认的GC模式以及参数配置即可满足大部分的使用场景。然而，如果程序需要耗费大量的时间来分配和释放内存的话，那么我们可能就需要定制GC。这里值得注意的是，GC的配置在不同版本、不同平台之间都是不可移植的。

在C语言中，我们可以通过使用lua_gc()来调整GC的工作模式及相关参数，在lua中我们可以使用collectgarbage()来调整。

### 5.1 增量垃圾收集
在增量模式下，每一个GC周期内都会在程序执行过程中插入小步的垃圾收集步骤(mark-and-sweep)。在此种模式下，回收器使用3个参数来控制垃圾回收周期：

* garbage-collector pause: 用于控制多长时间启动新一轮的垃圾回收。在上一次垃圾回收之后，当内存的使用达到了```n%```时就触发新一轮的垃圾回收。

* garbage-collector step multiplier：用于控制相对内存分配的垃圾回收速度

* garbage-collector step size



### 5.2 通用垃圾收集
通用模式下，GC会进行frequent minor收集，其只会扫描最近所创建的objects。假如在完成minor collection之后所占用的内存仍然超过了限制值，那么GC就会启动stop-the-world major collection，此时其会遍历所有的objects。通用模式主要由两个参数控制： minor multiplier、major multiplier.

### 5.3 垃圾收集的metamethods
我们可以设置GC的metamethods以收集tables或者full userdata(使用C API）。这些metamethods被称为finalizers，在垃圾回收器检测到相应的tables或userdata处于dead状态时，就会回调这些metamethods。可以将Finalizers与GC协同工作，从而实现对一些资源的管理（例如关闭文件、网络、数据库连接，或者释放自己的内存）。

对于一个table或者userdata对象来说，如果在垃圾收集的时候要被finalized，那么我们必须将其标记为finalization。我们必须在设置该object的metatable时将该对象标记为finalization，并且对应的metatable要有```__gc```这个key。值得注意的是，如果我们这是metatable时没有设置```__gc```字段，而是在后续才创建该字段，那么该object并不会被标记finalization。

当我们执行lua_close()时，Lua会调用所有被标记为finalization的对象的finalizers函数，从而完成相关的资源释放。



### 5.4 Weak Tables
所谓的Weak Table，是指其中所有的elements是weak references。垃圾回收器会忽略弱引用。换句话说，假如某一个对象只存在弱引用的话，那么垃圾回收器将会回收该对象。

weak table中的元素可以是weak keys，也可以是weak values，或者两者都是weak的。拥有weak values的table允许垃圾回收器回收其values，但是并不会回收其key。如果weak table中key和value均是weak的，那么垃圾回收器就会同时回收key和value。

一个table是否为weak是通过对应metatable中的```__mode```字段来控制的。假如存在该字段的话，其取值可以为：

* k: 表明该table的key是weak的；

* v: 表明该table的value是weak的；

* kv: 表明该table的key、value都是weak的；

## 6. Coroutines

Lua是支持coroutines的，也被称为collaborative multithreading。在Lua中，一个coroutine代表了一个独立的执行线程。与操作系统中的线程的区别在于，coroutine仅仅只会通过调用一个yield函数挂起相应的执行。

我们可以使用coroutine.create()来创建一个coroutine。该函数接受的唯一参数就是协程需要执行的主函数。create()函数仅仅只会创建一个coroutine，然后返回相应的操作句柄，其并不会启动coroutine。

如果我们要启动执行一个coroutine，那么我们需要调用coroutine.resume()。当你coroutine.resume()时，传递给该函数的第一个参数即是对应的coroutine句柄，剩余的参数就会被传递给协程的执行主函数。在coroutine运行起来之后，它会一直运行到结束或者主动调用了yield()。

coroutine可以通过两种方式来终止： 正常情况下，当主函数返回时，coroutine终止；异常情况下，当出现unprotected错误时，对应的coroutine也会终止。在第一种情况下，coroutine.resume()会返回true，以及coroutine主函数的其他返回值；而在发生错误是，coroutine.resume()返回false，以及相应的error object信息（此时，coroutine并不会清空栈，因此我们后面可以通过相应的debug API获取到相应的堆栈信息）。

我们可以通过调用coroutine.yield()来主动放弃coroutine的执行。当主动放弃coroutine的执行时，coroutine.resume()会马上返回（注： 即使不是直接在coroutine主函数中调用yield()，其仍然会退出coroutine的执行)。调用yield()放弃执行coroutine时，coroutine.resume()仍然会返回true，以及传递给coroutine.yield()的参数。当你下一次重新恢复执行同一个coroutine时，向coroutine.resume()传递上一次coroutine.yield()的返回值，这样其就可以从上一次yield的地方重新恢复执行。（注： 假如上一次yield()返回10个值，那么下一次恢复时传递给coroutine.resume()的参数也应该是10个，这样就能保证从上一次中断的地方不会因参数不匹配，导致代码运行错误）。

参看如下代码示例：
{% highlight string %}
function foo (a)
       print("foo", a)
       return coroutine.yield(2*a)
     end
     
     co = coroutine.create(function (a,b)
           print("co-body", a, b)
           local r = foo(a+1)
           print("co-body", r)
           local r, s = coroutine.yield(a+b, a-b)
           print("co-body", r, s)
           return b, "end"
     end)
     
     print("main", coroutine.resume(co, 1, 10))
     print("main", coroutine.resume(co, "r"))
     print("main", coroutine.resume(co, "x", "y"))
     print("main", coroutine.resume(co, "x", "y"))
{% endhighlight %}
运行：
<pre>
co-body 1       10
foo     2
main    true    4
co-body r
main    true    11      -9
co-body x       y
main    true    10      end
main    false   cannot resume dead coroutine
</pre>

## 3. The Language

本节主要介绍Lua编程中相关的语法、语义。

### 3.1 语法规定

Lua是一种形式非常自由的语言(free-form)，其会忽略代码中不必要的空格。

1） **关键字**

Lua支持如下关键字：
<pre>
and       break     do        else      elseif    end
false     for       function  goto      if        in
local     nil       not       or        repeat    return
then      true      until     while
</pre>

Lua是大小写敏感的。

2） **所支持的运算符**

Lua所支持的运算符如下：
{% highlight string %}
+     -     *     /     %     ^     #
&     ~     |     <<    >>    //
==    ~=    <=    >=    <     >     =
(     )     {     }     [     ]     ::
;     :     ,     .     ..    ...
{% endhighlight %}


字符串字面量可以用单引号或双引号括起来。

### 3.2 变量

变量用于存储特定的值。根据作用域不同，Lua中有三种不同类型的变量： 全局变量、局部变量、表中的域(table fields)


变量在使用前，需要在代码中进行声明，即创建该变量。编译程序执行代码之前编译器需要知道如何给语句变量开辟存储区，用于存储变量的值。

在Lua中，任何变量名都假定为全局的，除非是特别的将其声明为local。局部变量的作用域为从声明位置开始到所在语句块结束。变量的默认值均为 nil。


参看如下示例：
{% highlight string %}
-- test.lua 文件脚本
a = 5               -- 全局变量
local b = 5         -- 局部变量

function joke()
    c = 5           -- 全局变量
    local d = 6     -- 局部变量
end

joke()
print(c,d)          --> 5 nil

do
    local a = 6     -- 局部变量
    b = 6           -- 对局部变量重新赋值
    print(a,b);     --> 6 6
end

print(a,b)          --> 5 6
{% endhighlight %}
运行：
<pre>
5       nil
6       6
5       6
</pre>

### 3.3 Statements

Lua支持大部分其他编程语言所支持的语法。这包括： 代码块、assignments、control structures、function calls、以及变量声明

###### 3.3.1 Blocks
代码块是指用```{}```包围起来的一段代码，其中的代码会被顺序执行。例如：
{% highlight string %}
{
	print("hello, world 1");
	print("hello, world 2");
}
{% endhighlight %}

###### 3.3.2 Chunks
Lua的编译单元(unit)称为```chunk```。从语义上来说，一个chunk就是一个简单的代码块。

一个chunk可以被保存在一个lua文件中，也可以被保存在一个字符串中。要执行一个chunk，我们首先必须先进行加载，将后将代码块进行预编译为虚拟机指令，之后再由虚拟机解释执行相关的指令。

###### 3.3.3 赋值操作
Lua支持多变量连续赋值。当变量个数和值的个数不一致时，Lua会一直以变量个数为基础采取以下策略：

* 变量个数 > 值的个数:  按变量个数补足nil

* 变量个数 < 值的个数:  多余的值会被忽略

例如：
{% highlight string %}
a, b, c = 0, 1
print(a,b,c)             --> 0   1   nil
 
a, b = a+1, b+1, b+2     -- value of b+2 is ignored
print(a,b)               --> 1   2
 
a, b, c = 0
print(a,b,c)            
{% endhighlight %}
编译运行：
<pre>
0       1       nil
1       2
0       nil     nil
</pre>

###### 3.3.4 Control Structures

Lua支持的控制结构主要包括： if、while、repeat(其实也包括for，我们会在下一节讲解）

在控制结构中条件表达式可以返回任何值，其中false与nil会被判断为```false```，其他值都会被判断为```true```。特别需要指出的是数值0和空字符串也会被判断为```true```。

break语句可以跳出while、repeat、until循环。return语句用于从一个函数或者chunk(我们可以将一个chunk理解为一个匿名函数)中返回相应的值。


下面给出一个代码示例：
{% highlight string %}
a, b, c, d = nil, 1, 2, 3


print("1) test if ");
if(not a)
then 
    print("a is false");
elseif (a == 1)
then
    print("a is equal to 1");
else
    print("unmatched");
end    


print("\n2) test while");
while (b)
do
    print(b);
    b = b -1;
    
    if(b <= -1)
    then
        print("b is already less than -1, exit");
        break 
    end
end

print("\n3) test repeat");
repeat 
    print(c);
    c = c+1;
until(c < 5)   


print("\n4) test label")
if(d < 3)
then
    goto SUCCESS_END;
else
     goto FAILURE_END;
end

::SUCCESS_END:: 
do 
    print("this is success end");
    return 0;                                --这里必须要用do将代码块包围起来，否则return不能放这里
end

::FAILURE_END::
print("this is failure end");
return -1;
{% endhighlight %}
编译运行：
<pre>
1) test if
a is false

2) test while
1
0
b is already less than -

3) test repeat
2

4) test label
this is failure end
</pre>


###### 3.3.5 For循环
for循环有两种类型：数值类型(numerical)、泛型类型(generic)

1) **数值类型循环**

语法规则如下：
{% highlight string %}
for Nam=exp1, exp2, exp3
do

end
{% endhighlight %}
上面的for循环有三个控制条件：initial value、limit、step。假如未设置step的话，那么其默认值为1。

假如initial value以及step均为整数的话，那么循环是按整数来进行（注： limit可以不是一个整数)；否则这三个值都会被转换为浮点数（此种情况下应该要注意浮点数的精度问题）

假如step为0，那么此种循环直接报错；假如step大于0，那么当Name大于limit时，循环结束；假如step小于0，那么Name小于limit时，循环结束。

如下我们给出一个示例：
{% highlight string %}
function f(x)  
    print("function")  
    return x*2  
end

print("the first numerical loop")
for i=10,1,-1 do
    print(i)
end

print("\nthe second numerical loop")
for i=1,f(5) do 
    print(i)  
end
{% endhighlight %}
编译运行：
<pre>
the first numerical loop
10
9
8
7
6
5
4
3
2
1

the second numerical loop
function
1
2
3
4
5
6
7
8
9
10
</pre>

2) **泛型循环**

泛型循环也被称为iterators。每一次迭代的时候，iterator函数都会被调用，并产生一个新的值。当迭代产生一个nil值的时候，循环结束。

此种循环语法规则如下：
{% highlight string %}
for var_1, ···, var_n in explist do body end
{% endhighlight %}

变量名```var_i```可以在循环体中被使用，其中第一个变量var_1是控制变量。

在循环开始之前，首先会计算explist并产生4个结果值：

* iterator function

* state

* initial value for the control variable

* closing value

之后在每一次迭代过程中，lua都会调用iterator function，该函数接收两个参数：state和control variable。该迭代函数将返回结果赋值给```var_i```变量列表。假如控制变量的值变为了```nil```，那么整个循环将结束；否则，将执行循环体中的内容，并执行下一个迭代。

closing value类似于一个```to-be-closed```变量，可用于在循环结束后释放相应的资源。

我们不应该在循环体中修改控制变量的值。


参看如下示例：
{% highlight string %}
a = {"one", "two", "three"}
for i, v in ipairs(a) do
    print(i, v)
end 

for var_1, var_2, var_3 in ipairs(a)
do
    print(var_1, var_2, var_3)
end
{% endhighlight %}
编译运行：
<pre>
1       one
2       two
3       three
1       one     nil
2       two     nil
3       three   nil
</pre>

###### 3.3.6 Local Declarations

局部变量声明的语法如下所示：
{% highlight string %}
local var_1 = 0;
local var_2;

local var3, var4 = 3, "this is a string";
{% endhighlight %}
对于未赋值的变量，其值默认都为```nil```。

此外在声明变量时，还可以为变量指明属性，Lua变量支持的属性有两个：

* const: 用于指明该变量是一个常量类型，即初始化之后我们将不能够再修改其对应的值；

* close: 用于指明该变量是一个to-be-closed变量，

参看如下示例：
{% highlight string %}
local a <const> = 1;
local fd <close> = io.open("./1.txt", "r");
{% endhighlight %}


###### 3.3.7 To-be-close Variables
to-be-close变量类似于constant local变量，但是其会在超出作用域范围时被关闭。无论是block执行完毕退出，还是在block中通过break、goto、return退出，还是由于error退出，均会导致相关的变量被关闭。

这里关闭一个变量意味着会调用其```__close()```元方法(metamethod)。当调用此metamethod时，向该函数传递的第一个变量就是该close变量本身，第二个参数为error对象，表示调用```__close()```的退出原因。假如并不是因为error退出，那么对应的error的值即为nil。

在为to-be-close变量赋值时，要求对应的值必须要有```__close```元方法(metamethod)或者是一个false值。


假如有多个to-be-close变量同时超出作用域，那么h后声明的to-be-close变量将会先被释放。

假如在执行```__close```方法时其内部出现了error，那么对其中错误的处理方式与正常情况下的错误处理保持一致。

### 3.4 表达式
这里我们对函数调用以及泛型表达式做一个说明。函数调用的返回值以及泛型都可能会产生多个结果值。假如函数调用仅仅是作为一个statement，那么其返回值将会被忽略；假如expression A是作为一个表达式列表中的最后一个元素，或者是表达式列表的唯一元素，那么expression A 所有的返回结果值将不会被忽略；在其他情况下，Lua会将result list调整为1个元素，而将其他元素都忽略掉。

参看如下示例：
{% highlight string %}
f()                -- adjusted to 0 results
g(f(), x)          -- f() is adjusted to 1 result
g(x, f())          -- g gets x plus all results from f()
a,b,c = f(), x     -- f() is adjusted to 1 result (c gets nil)
a,b = ...          -- a gets the first vararg argument, b gets
                -- the second (both a and b can get nil if there
                -- is no corresponding vararg argument)

a,b,c = x, f()     -- f() is adjusted to 2 results
a,b,c = f()        -- f() is adjusted to 3 results
return f()         -- returns all results from f()
return ...         -- returns all received vararg arguments
return x,y,f()     -- returns x, y, and all results from f()
{f()}              -- creates a list with all results from f()
{...}              -- creates a list with all vararg arguments
{f(), nil}         -- f() is adjusted to 1 result
{% endhighlight %}

任何放在```()```中的表达式都只会返回一个值。因此，(f(x,y,z))总是返回一个值，即使f(x,y,z)本身可能会返回多个值。

>注： 假如f(x,y,z)返回多个值，那么(f(x,y,z))将返回第一个值；如果f(x,y,z)不返回值，那么(f(x,y,z))的值将为nil。

###### 3.4.1 算术运算符
Lua支持如下算术运算符：
{% highlight string %}
+       : addition
      
-       : subtraction
     
*       : multiplication
     
/       : float division

//      : floor division

%       : modulo

^       : exponentiation

-       : unary minus
{% endhighlight %}


###### 3.4.2 位运算符

Lua支持如下的位运算符：
{% highlight string %}
&        : bitwise AND

|        : bitwise OR

~        : bitwise exclusive OR

>>       : right shift

<<       : left shift

~        : unary bitwise NOT
{% endhighlight %}

###### 3.4.3 强制转换规范
对于有一些数据类型，Lua会在运行时自动的进行类型转换。比如在进行```位操作```运算时就会自动的将浮点操作数转换为整数。指数运算和浮点除法运算则会自动的将操作数转换为浮点数。甚至，对于字符串连接，可以接收数字作为参数。

###### 3.4.4 关系运算符
Lua支持如下关系运算符：
{% highlight string %}
==       : equality

~=       : inequality

<        : less than

>        : greater than

<=       : less or equal

>=       : greater or equal
{% endhighlight %}

如上关系运算符产生的结果为```true```或者```false```。

对于```==```运算符，首先会比较对应操作数的类型，如果类型不同则直接返回```false```。如果类型相同，然后再会比较对应的值是否相等。对于```字符串```来说，比较的是两个操作数之间的字节内容是否相等；对于数值来说，比较的是两个操作数在数学层面是否相等。

table、userdata以及thread之间的比较是它们之间的引用是否相等，即是否引用的是同一个对象。任何时候你创建一个新的对象(table、userdata、thread)，该新的对象都与已存在的对象不同。

我们也可以为table和userdata指定```__eq```这样一个metamethod，从而按我们自定义的方式来比较连个table(或userdata)是否相等。


此外，这里还需要注意的一点是，在进行字符串和数字之间的```==```比较时，并不会自动的将整数转换成字符串（或者将字符串转换为整数）。因此```"0" == 0```的结果为false。对于table t而言，t[0]与t["0"]也是不一样的。


###### 3.4.5 逻辑运算符
Lua中的逻辑运算符有：

* and

* or

* not

与控制结构(control structure)类似，所有的逻辑运算符也都会将```false```、```nil```认为是false，而其他值认为是true。

对于逻辑反运算(not)其结果总是为```true```或者```false```；对于逻辑与运算(and)，假如第一个操作数的值为false或者nil，那么其返回第一个操作数，否则其返回第二个操作数；对于逻辑或运算(or)，假如第一个参数不是false/nil，那么其返回第一个操作数，否则返回第二个操作数。

参看如下示例：
{% highlight string %}
10 or 20            --> 10

10 or error()       --> 10

nil or "a"          --> "a"

nil and 10          --> nil

false and error()   --> false

false and nil       --> false

false or nil        --> nil

10 and 20           --> 20
{% endhighlight %}

###### 3.4.6 Concatenation
在Lua中字符串的连接操作采用两个点(```..```)来实现。假如两个操作数都是字符串或者数字，则数字会自动的转换成字符串，否则调用```__concat```元方法(metamethod)来完成。

###### 3.4.7 求长度运算符
采用单目运算符```#```来实现求长度的运算。

对于一个字符串来说，其长度为所占用的字节数。如果在一个table变量上运用求长度运算符```#```，那么其返回该table的边界。对于一个table的边界，其是满足如下条件的自然数：
{% highlight string %}
(border == 0 or t[border] ~= nil) and t[border + 1] == nil
{% endhighlight %}
换句话说，边界(border)就是table中存在的一个索引，该索引处的元素不为nil，但是该索引下一个位置上的元素为nil。

只有一个border的table，我们称之为序列(sequence)。例如，对于table {10, 20, 30, 40, 50}来说其就是一个sequence，其只有一个border(5)；对于table {10, 20, 30, nil, 50}有连个border，分别是border(3)和border(5)，因此其并不是一个sequence（在索引4处的nil，我们称之为hole)；对于table {nil, 20, 30, nil, nil, 60, ni}具有三个border，分别是border(0)、border(3)、border(6)，并且有3个hole，这3个hole的索引分别是1、4、5，因此其也不是一个sequence。对于table {}来说，其只有一个border(0)，因此是一个sequence。

当t是一个sequence时，```#t```只会返回一个border，即对应于序列的长度。当t不是一个sequence时，```#t```会返回所有的borders。

###### 3.4.8 优先级
在Lua中，运算符的优先级表如下所示(从低到高）：
{% highlight string %}
or

and

<     >     <=    >=    ~=    ==

|

~

&

<<    >>

..

+     -

*     /     //    %

unary operators (not   #     -     ~)

^
{% endhighlight %}
通常情况下，你可以使用括号```()```来改变一个表达式的优先级顺序。连接运算符```..```和指数运算符```^```具有从右到左的结合性，而其他运算符的结合性为从左到右。



###### 3.4.9 Table Constructors

table的构造可以采用相应的构造表达式来完成。当构造表达式被执行，那么table就构造出来了。我们可以构造一个空的table，也可以在构造的时候直接初始化其中的一些field。table的构造语法如下：
{% highlight string %}
tableconstructor ::= ‘{’ [fieldlist] ‘}’

fieldlist ::= field {fieldsep field} [fieldsep]

field ::= ‘[’ exp ‘]’ ‘=’ exp | Name ‘=’ exp | exp

fieldsep ::= ‘,’ | ‘;’
{% endhighlight %}

如果table初始化中字段(Field)的形式为```[exp1] = exp2```，其表示向table中添加一条key为```exp1```，value为```exp2```的的entry； 如果字段(Field)的形式为```name = exp```，那么其等价于```["name"]=exp```；而对于单独的```exp```形式，其等价于[i] = exp，这里i是从1开始的连续自然数，并且其他形式的field并不会影响i的计数。例如：
{% highlight string %}
a = { [f(1)] = g; "x", "y"; x = 1, f(x), [30] = 23; 45 }
{% endhighlight %}

其等价于：
{% highlight string %}
do
	local t = {}
	t[f(1)] = g
	t[1] = "x"         -- 1st exp
	t[2] = "y"         -- 2nd exp
	t.x = 1            -- t["x"] = 1
	t[3] = f(x)        -- 3rd exp
	t[30] = 23
	t[4] = 45          -- 4th exp
	a = t
end
{% endhighlight %}
>注： 构造函数中的赋值顺序是未定的（影响到的点主要是重复的key)

假如初始化列表的最后一个字段为exp形式，并且该exp是一个函数调用或者是泛型表达式，那么该表达式返回的所有值都会被放入table中。

###### 3.4.10 Function calls
在Lua中函数调用的语法如下：
{% highlight string %}
functioncall ::= prefixexp args
{% endhighlight %}
在一个函数调用中，```prefixexp```和```args```首先会被评估。假如```prefixexp```的值是function类型，那么该函数就会直接以指定的参数被调用；否则，假如```prefixexp```存在```__call```这样的metamethod，那么该元方法会被调用： 第一个参数为prefixexp本身，后面才是	对应的参数列表。

而对于如下形式：
{% highlight string %}
functioncall ::= prefixexp ‘:’ Name args
{% endhighlight %}
其可被用于模仿method。调用v:name(args)等价于v.name(v,args)，只是前者只会对v计算一次。

###### 3.4.11 Function Definitions
函数定义的语法如下：
{% highlight string %}
functiondef ::= function funcbody
funcbody ::= ‘(’ [parlist] ‘)’ block end
{% endhighlight %}

另外，采用如下的语法糖可以简化function的定义：
{% highlight string %}
stat ::= function funcname funcbody
stat ::= local function Name funcbody
funcname ::= Name {‘.’ Name} [‘:’ Name]
{% endhighlight %}

对于上面的语法糖，请参看如下转化：
{% highlight string %}
 The statement

     function f () body end

translates to

     f = function () body end

The statement

     function t.a.b.c.f () body end

translates to

     t.a.b.c.f = function () body end

The statement

     local function f () body end

translates to

     local f; f = function () body end

not to

     local f = function () body end

(This only makes a difference when the body of the function contains references to f.) 
{% endhighlight %}

下面我们给出一些函数参数转化的实例：
{% highlight string %}
 As an example, consider the following definitions:

     function f(a, b) end
     function g(a, b, ...) end
     function r() return 1,2,3 end

Then, we have the following mapping from arguments to parameters and to the vararg expression:

     CALL             PARAMETERS
     
     f(3)             a=3, b=nil
     f(3, 4)          a=3, b=4
     f(3, 4, 5)       a=3, b=4
     f(r(), 10)       a=1, b=10
     f(r())           a=1, b=2
     
     g(3)             a=3, b=nil, ... -->  (nothing)
     g(3, 4)          a=3, b=4,   ... -->  (nothing)
     g(3, 4, 5, 8)    a=3, b=4,   ... -->  5  8
     g(5, r())        a=5, b=1,   ... -->  2  3

{% endhighlight %}

我们使用**return**语句来返回函数的结果。假如在到达end之前都没有return，那么表示该函数不会有结果返回。

另外，对于如下的colon syntax，其被用来模仿method，其会在函数调用时隐式的添加一个额外的参数到参数列表中，参看如下：
{% highlight string %}
     function t.a.b.c:f (params) body end

is syntactic sugar for

     t.a.b.c.f = function (self, params) body end

{% endhighlight %}

## 4. 示例

### 4.1 Lua的注释
如下给出一段lua的示例
{% highlight string %}
-- this is single line note
print("hello, world")


--[[

this is multiline note

--]]

function fact(n)
  if n == 0 then
    return 1
  else
    return n * fact(n - 1)
  end   
end

 --[====[  
 
 this is another multiline note!!!
 
 call the function fact

 -- ]====]
 
print("enter a number:")
a = io.read("*n")                   -- read a number

print(fact(a))
{% endhighlight %}
对于上面第三种注释方式，```[[```之间的```=```数可以为任意值，但是请注意结尾处的```=```个数应与之相同。

### 4.2 泛型
{% highlight string %}
function add(...)
  local s = 0
  
  for _, v in ipairs{...} do
    
      s = s + v
    
  end  
  
  return s 
end

print(add(1,2,3,4,5))
{% endhighlight %}


### 4.3 八皇后问题
{% highlight string %}
N = 8 -- board size


-- check whether position (n,c) is free from attacks
function isplaceok (a, n, c)
  for i = 1, n - 1 do -- for each queen already placed
      if (a[i] == c) or -- same column?
          (a[i] - i == c - n) or -- same diagonal?
          (a[i] + i == c + n) then -- same diagonal?
          return false -- place can be attacked
      end
  end
  return true -- no attacks; place is OK
end

    

-- print a board
function printsolution (a)
  for i = 1, N do -- for each row
      for j = 1, N do -- and for each column
      -- write "X" or "-" plus a space
          io.write(a[i] == j and "X" or "-", " ")
      end
      io.write("\n")
  end
  io.write("\n")
end

-- add to board 'a' all queens from 'n' to 'N'
function addqueen (a, n)
  if n > N then -- all queens have been placed?
      printsolution(a)
  else -- try to place n-th queen
      for c = 1, N do
          if isplaceok(a, n, c) then
              a[n] = c -- place n-th queen at column 'c'
              addqueen(a, n + 1)
          end
      end
  end
end

-- run the program
addqueen({}, 1)
{% endhighlight %}

<br />
<br />

参看:


1. [Basic Concepts](http://www.lua.org/manual/5.4/manual.html)



<br />
<br />
<br />

