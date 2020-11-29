---
layout: post
title: Lua面向对象编程
tags:
- lua
categories: lua
description: lua开发
---


在Lua中，在很多场景下table就是一个object。与objects类似，table也有相关的状态；table也有一个独立于其值(table中的元素)的标识符(self)；特别是，两个具有相同值(table中的所有元素都相等）的table是不同的对象。此外，与objects类似，table也有一个独立的生命周期。




<!-- more -->


 Object通常都有它们自己的操作，tables也可以有自己的操作行为，看如下代码片段：
{% highlight string %}
Account = {balance = 0}

function Account.withDraw(v)
  Account.balance = Account.balance - v 
  
end   

{% endhighlight %}
在上面的定义中，创建了一个新的函数，并将其保存在Account对象的withDraw字段中，然后我们可以通过如下的方式来调用：
{% highlight string %}
Account.withDraw(100.00)
{% endhighlight %}

这种类型的函数也就是我们所说的```method```。然而，在我们的编程实践中，我们会发现在函数内部使用一个全局变量其实是一件十分可怕的事情。首先，这样会使得函数只能用于某一个特定的对象；其次，即使针对该特定对象，函数也只能对保存在该特定全局变量的对象工作正常。假如我们更改了object的名称，withDraw可能就不能正常的工作了：
{% highlight string %}
a, Account = Account, nil
a.withdraw(100.00) -- ERROR!
{% endhighlight %}
这样的行为表现就破坏了面向对象的基本准则：object拥有读了的生命周期

一种更符合标准的方法就是为相关的操作提供一个```receiver```，为此我们可以在对应的method中添加一个额外的参数用于存储receiver。该额外的参数通常会命名为```self```或者```this```:
{% highlight string %}
function Account.withdraw (self, v)
  self.balance = self.balance - v
end
{% endhighlight %} 
现在，我们就可以指定需要在哪个object上执行对应的操作：
{% highlight string %}
a1 = Account; Account = nil
...
a1.withdraw(a1, 100.00) -- OK
{% endhighlight %}
通过使用```self```参数，我们就可以对不同的对象使用相同的method:
{% highlight string %}
a2 = {balance=0, withdraw = Account.withdraw}
...
a2.withdraw(a2, 260.00)
{% endhighlight %}

在任何面向对象的编程语言当中，```self```参数都是一个中心要点。对于大多数的OO语言，都有相应的机制来为编程人员隐藏此参数。Lua也可以通过使用```:```操作符来隐藏此参数。下面我们来重写前面的withDraw()方法：
{% highlight string %}
function Account:withdraw (v)
   self.balance = self.balance - v
end
{% endhighlight %}
之后，我们就可以使用```a2:withDraw(260.00)```来调用了。

通过使用冒号操作符(```:```)，自动的在函数定义中添加了一个隐藏的参数，同时在调用的时候也是会自动的加上此参数。但是值得指出的是，这里```:```操作符仅仅只是一个语法糖，并没有引入新的东西。我们可以使用```.```这种方式来定义method，然后通过```:```这种方式来调用，或者相反。例如：
{% highlight string %}
Account = { balance=0,
  withdraw = function (self, v)
    self.balance = self.balance - v
  end
}
function Account:deposit (v)
  self.balance = self.balance + v
end

Account.deposit(Account, 200.00)
Account:withdraw(100.00)
{% endhighlight %}



## 1. Classes
到目前为止，我们的objects已经有了identity、state以及对应state下的operations。但现在仍然缺乏如下概念：

* class system

* inheritance

* privacy

现在我们来处理第一个问题：如何创建多个具有相似行为的object？特别是，我们如何创建多个Account呢？

大多数面向对象的编程语言都提供了class的概念，其作为创建object的模板。在这些编程语言中，每一个object都是一个特定class的实例。在Lua语言中，并没有class的概念；metatable的概念与此有几分类似，但是如果将metadata作为class也并不能使得我们可以走的太远，毕竟metadata只是类似，还有很多的机制是不支持的。相反，在Lua中，我们可以参考prototype-based language（比如Self， Javascript也是遵循此路线来实现的面向对象编程)来默认class。在这些语言中，objects（这里称为A)并没有classes，相反每一个object都有一个prototype，prototype本身其实也是一个object(这里称为B）：当A对某些operation不能识别的时候，其就会到prototype这个对象中来查找。在这样一些语言当中，为了表示class，我们会简单的创建一个object来作为其他对象的prototype。因此，从这个意义上来说，可以认为classes和prototypes都可以作为存放多个objects公共行为的场所。

在Lua中，我们可以使用继承的思想来实现prototypes，其实我们在上一章```The __index metamethod```一章中就已经介绍过。特别是，假如我们有两个object，分别为A和B，我们需要做的就是将B设置为A的一个prototype：
{% highlight string %}
setmetatable(A, {__index = B})
{% endhighlight %}

这样设置之后，当A中没有相关的operations时，其就会自动的在B中进行查找。这样只是从另一个角度将B看成了A对象的class。

现在我们回到上面关于银行账户的例子。为了创建与Account具有相似行为的其他账户，通过使用```__index```元方法，我们安排这些新创建的对象都继承自Account：
{% highlight string %}
Account = { balance=0}

function Account:withDraw(v)
  self.balance = self.balance - v
end
  
function Account:deposit (v)
  self.balance = self.balance + v
end



local mt = {__index = Account}

function Account.new (o)
  o = o or {} -- create table if user does not provide one
  setmetatable(o, mt)
  return o
end
{% endhighlight %}
编写完上述代码之后，当我们采用如下的方式来创建一个account时会发生什么呢？
{% highlight string %}
a = Account.new{balance = 0}
a:deposit(100.00)
{% endhighlight %}
当我们创建一个银行账户a，其就由一个mt来作为其metatable。因此，当我们调用a:diposit(100.00)函数时，其实实际会调用a.diposit(a, 100.00)；这里冒号(```:```)仅仅只是一个语法糖。然而，Lua并不能在table a中找到```diposit```，因此就会查询到metatable中的```__index```，实际的调用过程就类似于如下：
{% highlight string %}
Account.deposit(a, 100.00)
{% endhighlight %}

即，Lua会调用metatable中的deposit()函数，并向其传递一个self参数。因此，新的银行账户就从Account中继承了deposit()方法。通过相同的机制，它也从Account中继承了所有的fields。

在这里，我们可以有两个小的改进。第一个改进就是我们并不需要为metatable角色单独创建一个table，相反我们可以使用Account表本身来实现此目的；第二个改进就是我们可以对new()函数本身也使用冒号(```:```)语法：
{% highlight string %}
function Account:new (o)
  o = o or {}
  self.__index = self
  setmetatable(o, self)
  return o
end

{% endhighlight %}
现在，当我们调用Account:new()时，隐藏参数self的值就是Account，并且我们将```Account.__index```设置为了其本身，且将Account设置为了新创建对象的object的metatable。到这里，我们发现似乎并没有从这种更改(冒号语法）中获得太大的收益，但其实不然，其中的优势就在于当我们介绍类继承的时候，self就会变得非常有用（我们会在下一节讲解）。


类的继承不仅仅在于method，同样在新的银行账户对象中也会继承其他的字段。因此，class不仅可以提供methods，也可以为实例提供常量和默认值。请记住，在我们定义Account第一版代码中，我们为balance设置了0。因此，假如我们创建一个新的对象，在其中我们并没有给balance赋初始值的话，那么其将会继承自Account：
{% highlight string %}
b = Account:new()
print(b.balance) --> 0
{% endhighlight %}

当我们在object b上调用diposit()函数，其就等价于运行如下代码：
{% highlight string %}
b.balance = b.balance + v
{% endhighlight %}
首先会评估b.balance的值为0，然后赋值函数会给b.balance设置一个新的初始值。后面再通过b.balance来获取账户余额时，将不会再调用到```__index``` metamethod，因为现在object b已经有了其自己的balance字段了。


下面给出两个很容易混淆的示例，以进一步加深对其中微妙关系的理解：

* ```示例1```

{% highlight string %}
Account = { balance=0 }

local mt = {__index = Account}

function Account.new (o)
  o = o or {} -- create table if user does not provide one
  setmetatable(o, mt)
  return o
end

function Account:withDraw(v)
    self.balance = self.balance - v
end
  
function Account:deposit (v)
  self.balance = self.balance + v
end

a = Account:new()
b = Account:new()

a:deposit(100.00)
print(a.balance)

b:deposit(200.00)
print(a.balance)
print(b.balance)
{% endhighlight %}
编译运行：
<pre>
100.0
300.0
300.0
</pre>

* ```示例2```
{% highlight string %}
Account = { balance=0 }

local mt = {__index = Account}

function Account:new (o)
  o = o or {} -- create table if user does not provide one
  
  self.__index = self 
  setmetatable(o, mt)
  return o
end

function Account:withDraw(v)
    self.balance = self.balance - v
end
  
function Account:deposit (v)
  self.balance = self.balance + v
end

a = Account:new()
b = Account:new()

a:deposit(100.00)
print(a.balance)

b:deposit(200.00)
print(a.balance)
print(b.balance)
{% endhighlight %}
编译运行：
<pre>
100.0
100.0
200.0
</pre>

* ```示例3```
{% highlight string %}
Account = { balance=0 }

function Account:new (o)
  o = o or {} -- create table if user does not provide one
  self.__index = self
  setmetatable(o, self)
  return o
end

function Account:withDraw(v)
    self.balance = self.balance - v
end
  
function Account:deposit (v)
  self.balance = self.balance + v
end

a = Account:new()
b = Account:new()

a:deposit(100.00)
print(a.balance)

b:deposit(200.00)
print(a.balance)
print(b.balance)
{% endhighlight %}

编译运行：
<pre>
100.0
100.0
200.0
</pre>

## 2. Inheritance

由于classes也是objects，他们也可以从其他的class那里获取methods，这样一个关系使得在Lua中实现继承的语义变得十分简单。

现在我们假设有一个base-class ```Account```，如下：
{% highlight string %}
Account = {balance = 0}

function Account:new (o)
  o = o or {}
  self.__index = self
  setmetatable(o, self)
  return o
end

function Account:deposit (v)
  self.balance = self.balance + v
end

function Account:withdraw (v)
  if v > self.balance then
    error"insufficient funds"
  end
  
  self.balance = self.balance - v
end
{% endhighlight %}
从上面的基类中，我们想要派生出一个子类： SpecialAccount，该类账户允许用户提取超过其越的资金。我们首先创建一个empty class来从基类中继承相关的方法：
{% highlight string %}
SpecialAccount = Account:new()
{% endhighlight %}
到这里，SpecialAccount仅仅只是Account的一个实例。微妙之处就发生在这里：
{% highlight string %}
s = SpecialAccount:new{limit=1000.00}
{% endhighlight %}
SpecialAccount从Account那里继承了new()方法，然而此时当new()函数被执行时，通过self这个隐含的参数传递的就是SpecialAccount了，因此s的metatable就变为SpecialAccount了。之后，当我们调用s:deposit(100.00)时，Lua在s中找不到deposit字段，因此其就会向上查找到SpecialAccount，在SpecialAccount中也找不到deposit字段，因此会再向上查找到Account中，然后在那里找到了deposit()的原始实现。

接下来在SpecialAccount中，我们可以实现对父类方法的重写：
{% highlight string %}

function SpecialAccount:withdraw (v)
  if v - self.balance >= self:getLimit() then
    error"insufficient funds"
  end
  self.balance = self.balance - v
end


function SpecialAccount:getLimit ()
  return self.limit or 0
end
{% endhighlight %}
现在我们调用s:withdraw(200.00)，Lua就不会再找入到Account中了，因为其会在SpecialAccount中叨叨withdraw字段。由于s.limit设置的透支余额为1000，因此当调用withdraw()方法时会返回一个负数余额。

在Lua中，object有趣的一个方面在于我们并不需要创建新的class来添加新的行为。假如只有某一个对象需要特定行为的话，我们可以直接在该对象中实现对应行为。例如，假设账户s代表某一个特定客户，其limit总是为其账户余额的10%，那么我们就可以单独修改此账户：
{% highlight string %}
function s:getLimit ()
  return self.balance * 0.10
end
{% endhighlight %}
这样修改完成之后，当我们调用s:withdraw(200.00)时，其会调用的SpecialAccount:withdraw()方法，然后当调用到self:getLimit()时就会调用到我们这里定义的s:getLimit()。

## 3. Multiple Inheritance
在Lua中，由于object并不是基础原语，我们可以有多种方式来实现面向对象编程。正如我们上面所介绍的，我们可以使用metamethod的index来实现（这也可能是最简单、高效和灵活的是现房方式）。无论如何，也还有一些其他的实现方式，并且在某一些特定的场景可能会更适合。这里我们会看到另外一种实现方式，并且其可以实现多继承。


本实现方式的关键核心要点在于使用```__index```这一metamethod。记住，当一个table的metatable中含有```__index```字段时，Lua如果在其原表中不能找到所请求的字段，那么就会调用```__index```字段所保存的函数。```__index```函数就可以在其想要的parent中找缺失的key。

多继承意味着一个class拥有不止一个superclass，因此我们不应该使用父类方法来创建子类(subclasses)。相反，我们会定义专门的独立方法createClass来实现此目的，createClass()的参数有创建新class所需要的全部父类。参看如下```Figure 21.2```示例，该函数会创建一个table来表示新的类，然后通过```__index```来实现多继承。不管多继承如何实现，每一个object实例仍然只属于一个class对象，然后会从该class处找到所有的方法。因此，```class、superclass```之间的关系 与 ```instance、class```之间的关系是不同的。 特别是，类不能同时是其instance及子类的metatable。在如下的例子中，我们将class设置为instance的metatable，但是会创建一个新的table来作为class的metatable.


**Figure 21.2 An implementation of multiple inheritance**
{% highlight string %}
--[[

   look up for 'k' in list of tables 'plist'
   
--]]


local function search (k, plist)
  for i = 1, #plist do
    local v = plist[i][k] -- try 'i'-th superclass
    if v then 
      return v 
    end
  end
  
end


function createClass (...)
  local c = {} -- new class
  local parents = {...} -- list of parents

  -- class searches for absent methods in its list of parents
  setmetatable(c, 
    {
      __index = function (t, k)
        return search(k, parents)
      end
    })
  
  
  -- prepare 'c' to be the metatable of its instances
  c.__index = c
  
  -- define a new constructor for this new class
  function c:new (o)
    o = o or {}
    setmetatable(o, c)
    return o
  end
  
  return c -- return new class
  
end
{% endhighlight %}

如下我们通过简单的例子来演示一下createClass()的使用。假设我们其中的一个父类为Account；另一个父类为Named，其有两个方法： setname()与getname()，如下：
{% highlight string %}
Named = {}
function Named:getname ()
  return self.name
end

function Named:setname (n)
  self.name = n
end
{% endhighlight %}
这里我们要创建一个新的类NamedAccount，其父类为Account和Named，我们可以简单的调用createClass()方法：
{% highlight string %}
NamedAccount = createClass(Account, Named)
{% endhighlight %}

然后我们再创建一个实例：
{% highlight string %}
account = NamedAccount:new{name = "Paul"}
print(account:getname()) --> Paul
{% endhighlight %}


<br />
<br />

参看:

1. [面向对象（Object-Oriented）编程](https://blog.csdn.net/glfxml/article/details/108755537)


<br />
<br />
<br />

