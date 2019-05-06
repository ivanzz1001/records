---
layout: post
title: 高级脚本编程之创建函数
tags:
- LinuxOps
categories: linuxOps
description: 高级脚本编程之创建函数
---

通常在编写shell脚本时，你会发现在多个地方使用了同一段代码。如果只是一小段代码，一般也无关紧要。但要在shell脚本中多次重写大块代码段就会比较辛苦了。bash shell支持用户定义的函数，这样就解决了这个难题。你可以将shell脚本代码放进函数中封装起来，这样就能在脚本中的任何地方多次使用它了。本章将会带你逐步了解创建自己的shell脚本函数的过程，并演示如何在shell脚本应用程序中使用它们。


<!-- more -->

## 1. 基本的脚本函数
在开始编写较复杂的shell脚本时，你会发现自己在重用执行特定任务的部分代码。有时这部分代码很简单，比如显示一条文本信息，或者从脚本用户那里获得一个答案； 有时则会比较复杂，作为较大进程中的一部分多次使用。

在每种情况下，在脚本中一遍又一遍的写同样的代码块都会是件烦人的事。如果能只写一次代码块，而在脚本中多次引用该代码块就太好了。

bash shell有个特性允许你这么做。函数(function)是可以起个名字并在代码中任何位置重用的代码块。你要在脚本中使用该代码块时，只要使用分配的函数名就行了（这个过程称为调用函数）。本节将会介绍如何在shell脚本中创建和使用函数。

### 1.1 创建函数
有两种格式可以用来在bash shell脚本中创建函数。第一种格式采用关键字function，后跟分配给该代码块的函数名：
{% highlight string %}
//注意： name后需要有一个空格
function name {
    commands
}

或

function name() {
    commands
}
{% endhighlight %}
name属性定义了赋予函数的唯一名称。你必须给脚本中定义的每个函数赋个唯一的名称。

commands是构成函数的一条或多条bash shell命令。在调用该函数时，bash shell会按命令在函数中出现的顺序执行命令，跟在普通脚本中一样。

在bash shell脚本中定义函数的第二种格式跟在其他编程语言中定义函数很像：
{% highlight string %}
//注意： 圆括号后可以不用空格
name() {
    commands
}
{% endhighlight %}
函数名后的圆括号为空，表明正在定义的是一个函数。这种格式的命名规则和上个定义shell脚本函数的格式一样。



### 1.2 使用函数
要在脚本中使用函数，在行上指定函数名就行了，跟使用其他shell命令一样：
{% highlight string %}
# cat test1 
#!/bin/bash

# using a function in a script

function func1 {
   echo "This is an example of a function"
}

count=1

while [ $count -le 5 ]
do
    func1
    
    count=$[$count + 1]
done

echo "This is the end of the loop"

func1

echo "Now this is the end of the script"


# ./test1 
This is an example of a function
This is an example of a function
This is an example of a function
This is an example of a function
This is an example of a function
This is the end of the loop
This is an example of a function
Now this is the end of the script
{% endhighlight %}

每次引用```func1```函数名时，bash shell会回到func1函数的定义并执行你在那里定义的命令。

函数定义不必是shell脚本中最前面的事，但要小心。如果在函数被定义前使用函数，你会收到一条错误消息：
{% highlight string %}
# cat test2 
#!/bin/bash

# using a function located in the middle of a script

count=1

echo "This line comes before the function definition"

function func1 {
   echo "This is an example of a function"
}

while [ $count -le 5 ]
do
   func1
   count=$[$count + 1]
done

echo "This is the end of the loop"

func2

echo "Now this is the end of the script"

function func2 {
   echo "This is an example of a function"
}

# ./test2 
This line comes before the function definition
This is an example of a function
This is an example of a function
This is an example of a function
This is an example of a function
This is an example of a function
This is the end of the loop
./test2: line 21: func2: command not found
Now this is the end of the script
{% endhighlight %}

第一个函数func1在脚本中是在几条语句之后才定义的，这当然没任何问题。当func1函数在脚本中被使用时，shell知道去哪里找它。

然而，脚本却试图在func2函数被定义之前使用它。由于func2函数还没定义，脚本运行到使用它的地方时，产生了一条错误消息。

你也必须注意函数名。记住，函数名必须是唯一的，否则也会有问题。如果你重定义了函数，新定义会覆盖原来函数的定义，而不会产生任何错误消息：
{% highlight string %}
# cat test3 
#!/bin/bash

# tesing using a duplicate function name

function func1 {
   echo "This is the first definition of the function name"
}

func1

function func1 {
   echo "This is a repeat of the same function name"
   
}

func1

echo "This is the end of the script"


# ./test3 
This is the first definition of the function name
This is a repeat of the same function name
This is the end of the script
{% endhighlight %}
func1函数的原始定义工作正常，但func1函数在第二次定义后，后续该函数的出现都会用第二个定义。

## 2. 返回值

bash shell会把函数当做小型脚本，运行结束时会返回一个退出状态码。有3种不同的方法来为函数生成退出状态码。

1） **默认退出状态码**

默认情况下，函数的退出状态码是函数中最后一条命令返回的退出状态码。在函数执行结束后，你可以用标准的```$?```变量来获取函数的退出状态码：
{% highlight string %}
# cat test4 
#!/bin/bash

# testing the exit status of a function

func1() {
   echo "trying to display a non-existent file"

   ls -l badfile
}

echo "testing the function:"

func1

echo "The exit status is: $?"


# ./test4 
testing the function:
trying to display a non-existent file
ls: cannot access badfile: No such file or directory
The exit status is: 2
{% endhighlight %}

函数的退出状态码是2，这是因为函数中的最后一条命令没有成功运行。但无法知道函数中其他命令是否成功运行。看下面的例子：
{% highlight string %}
# cat test4b 
#!/bin/bash

# testing the exit status of a function

func1() {
   ls -l badfile

   echo "trying to display a non-existent file"

}

echo "testing the function:"

func1

echo "The exit status is: $?"


# ./test4b 
testing the function:
ls: cannot access badfile: No such file or directory
trying to display a non-existent file
The exit status is: 0
{% endhighlight %}
这次，由于函数以成功运行的echo语句结尾，函数的退出状态码就是0，尽管函数中有一条命令没有成功运行。使用函数的默认退出状态码是很危险的。幸运的是，有几种办法可以解决这个问题。

2） **使用return命令**

bash shell使用return命令来退出函数并返回特定的退出状态码。return命令允许指定一个整数值来定义函数的退出状态码，从而提供了编程设定函数退出状态码的简便途径：
{% highlight string %}
# cat test5 
#!/bin/bash

# using the return command in a function

function dbl {

   read -p "Enter a value:" value
   
   echo "double the value"

   return $[$value * 2]
}

dbl

echo "The new value is: $?"

# ./test5 
Enter a value:5
double the value
The new value is: 10
{% endhighlight %}
dbl函数会将$value变量中用户输入的值翻倍，然后用return命令返回结果。脚本用```$?```变量显示了该值：

但当用这种方法从函数中返回值时，要小心了。记住下面两条技巧来避免问题：

* 记住，函数一结束就取返回值

* 记住，退出状态码必须在0~255之间

如果你在用```$?```变量提取函数返回值之前执行了其他命令，函数的返回值可能会丢失。记住，```$?```变量会返回执行的最后一条命令的退出状态码。

第二个问题定义了使用这种返回值方法的限制。由于退出状态码必须小于256，函数的结果必须生成一个小于256的整数值。任何大于256的值都会返回一个错误值：
<pre>
# ./test5 
Enter a value:200
double the value
The new value is: 144
</pre>
要返回较大的整数值或者字符串值的话，你就不能用这种返回值的方法了。取而代之，你必须使用另外一种方法，下节将会介绍。

3） **使用函数输出**

正如同可以将命令的输出保存到shell变量中一样，也可以将函数的输出保存到shell变量中。可以用这种技术来获得任何类型的函数输出，并将其保存到变量中：
<pre>
result=`dbl`
</pre>
这个命令会将dbl函数的输出赋给```$result```变量。下面是在脚本中使用这种方法的例子：
{% highlight string %}
# cat test5b 
#!/bin/bash

# using the echo to return a value

function dbl {
   read -p "Enter a value: " value

   echo $[$value * 2]
}

result=`dbl`

echo "The new value is $result"

# ./test5b 
Enter a value: 200
The new value is 400
{% endhighlight %}
新函数会用echo语句来显示计算结果。该脚本会捕获dbl函数的输出，而不是看函数的退出状态码。

本例演示了一个小技巧。你会注意到dbl函数输出了两条消息。read命令输出了一条简短的消息来向用户询问输入值。bash shell脚本会聪明地不将它作为STDOUT输出的一部分，并且忽略掉它。如果你用echo语句生成这条消息来向用户查询，则shell命令会将其与输出值一起读进变量中。

<pre>
说明： 通过这种方法，你还可以返回浮点值和字符串值。这让它非常适合返回函数值
</pre>


## 3. 在函数中使用变量
你可能已经注意到，上一节的test5例子中，我们在函数里用了一个叫做```$value```的变量来保存处理的值。在函数中使用变量时，你需要注意一下如何定义和处理它们。这是shell脚本中常见的产生问题的原因。本节将会复习一下在shell脚本函数内外处理变量的一些方法。

### 3.1 向函数传递参数

如我们在**2.2节**中提到的，bash shell会将函数当做小型脚本来对待。这意味着你可以向函数传递参数，就跟普通脚本一样。

函数可以使用标准的参数环境变量来代表命令行上传给函数的参数。例如，函数名会在```$0```变量中定义，函数命令行上的任何参数都会通过```$1```、```$2```等定义。也可以用特殊变量```$#```来判断传给函数的参数数目。

在脚本中指定参数时，必须将参数和函数放在同一行，像这样：
{% highlight string %}
func1 $value1 10
{% endhighlight %}
然后函数可以用参数环境变量来获得参数值。这里有个使用此方法向函数传值的例子：
{% highlight string %}
# cat test6 
#!/bin/bash

# passing parameters to a function

function addelem() {
   if [ $# -eq 0 ] || [ $# -gt 2 ]
   then
        echo -1
   elif [ $# -eq 1 ]
   then
        echo $[$1 + $1]
   else
        echo $[$1 + $2]
   fi
}

echo -n "Adding 10 and 15: "
value=`addelem 10 15`
echo $value

echo -n "Let's try adding just one number: "
value=`addelem 10`
echo $value

echo -n "Now trying adding no numbers: "
value=`addelem`
echo $value

echo -n "Finally, try adding three numbers: "
value=`addelem 10 15 20`
echo $value

# ./test6 
Adding 10 and 15: 25
Let's try adding just one number: 20
Now trying adding no numbers: -1
Finally, try adding three numbers: -1
{% endhighlight %}

test6脚本中的```addelem```函数首先会检查脚本传给它的参数数目。如果没有任何参数，或者如果多于两个参数，addelem会返回值-1。如果只有一个参数，addelem会将参数自己加到自己上面生成结果。如果有两个参数，addelem会将它们加起来生成结果。

由于函数使用特殊参数环境变量作为自己的参数值，它不能直接从脚本的命令行获取脚本的参数值。下面的例子将会运行失败：
{% highlight string %}
# cat badtest1 
#!/bin/bash

# trying to access script parameters inside a function

function badfunc1 {

  echo $[$1 * $2]
}

if [ $# -eq 2 ]
then
    value=`badfunc1`
    echo "The result is $value"
else
    echo "Usage: badtest a b"
fi

# ./badtest1 
Usage: badtest a b
# ./badtest1 10 15
./badtest1: line 7: * : syntax error: operand expected (error token is "* ")
The result is
{% endhighlight %}

尽管函数使用了```$1```和```$2```变量，但它们却和脚本主体中的```$1```和```$2```变量不尽相同。要在函数中使用这些值，必须在调用函数时手动将它们传过去：
{% highlight string %}
# cat test7 
#!/bin/bash

# trying to access script parameters inside a function

function func7() {
   echo $[$1 * $2]
}

if [ $# -eq 2 ]
then
  value=`func7 $1 $2`
  echo "The result is $value"
else
  echo "Usage: badtest1 a b"
fi
  
# ./test7 
Usage: badtest1 a b
# ./test7 10 15
The result is 150
{% endhighlight %}

通过将```$1```和```$2```变量传给函数，它们就能跟其他变量一样，功函数使用了。

### 3.2 在函数中处理变量

给shell脚本程序员经常带来麻烦的事是变量的作用域。作用域是什么情况下变量可见。函数中定义的变量可以跟普通变量的作用域不同，也就是说，它们可以在脚本的其他部分隐藏起来。函数会用两种类型的变量：

* 全局变量

* 局部变量

下面几节将会介绍如何在函数中使用这两种类型的变量。

1） **全局变量**

全局变量是在shell脚本中任何地方都有效的变量。如果你在脚本的主体部分定义了一个全局变量，那么你可以在函数内读取它的值。类似地，如果你在函数内定义了一个全局变量，你可以在脚本的主体部分读取它的值。

默认情况下，你在脚本中定义的任何变量都是全局变量。在函数外定义的变量可在函数内正常访问：
{% highlight string %}
# cat test8 
#!/bin/bash

# using a global variable to pass a value

function dbl() {

  value=$[$value * 2]
}

read -p "Enter a value: " value

dbl

echo "The new value is: $value"


# ./test8 
Enter a value: 20
The new value is: 40
{% endhighlight %}

```$value```变量是在函数外定义的，并在函数外被赋值了。但```dbl```函数被调用时，变量及其值在函数中都依然有效。如果变量在函数内被赋予了新值，那么在脚本中引用该变量时，新值依然有效。

但这其实是很危险的事情，尤其是如果你想在不同的shell脚本中使用函数的话。它要求你知道函数中具体使用了哪些变量，包括那些用来计算并没有返回到脚本的值的变量。这里有个例子来说明事情是如何搞砸的：
{% highlight string %}
# cat badtest2 
#!/bin/bash

# demonstrating a bad use of variables

function func1() {
  temp=$[$value + 5]

  result=$[$temp * 2]
}

temp=4
value=6

func1

echo "The result is: $result"

if [ $temp -gt $value ]
then
   echo "temp is larger"
else
   echo "temp is smaller"
fi

# ./badtest2 
The result is: 22
temp is larger
{% endhighlight %}

由于```$temp```变量在函数中用到了，它的值在脚本中使用时，产生了一个意想不到的结果。有个简单的办法可以在函数中解决这个问题，下节将会介绍。

2) **局部变量**

不用在函数中使用全局变量，函数内部使用的任何变量都可以被声明成局部变量。要那么做时，只要在变量声明的前面加上```local```关键字就可以了：
<pre>
local temp
</pre>

也可以在给变量赋值时在赋值语句中使用local关键字：
{% highlight string %}
local temp=$[$value + 5]
{% endhighlight %}

local关键字保证了变量只局限在该函数中。如果脚本中该函数之外有同样名字的变量，那么shell将会保持这两个变量的值是分离的。现在你就能很轻松地让函数变量和脚本变量分离开来，只共用想共用的：
{% highlight string %}
# cat test9 
#!/bin/bash

# demonstrating the local keyword

function func1() {
   local temp=$[$value + 5]

   result=$[$temp * 2]
}

temp=4
value=6

func1

echo "The  result is: $result"

if [ $temp -gt $value ]
then
   echo "temp is larger"
else
   echo "temp is smaller"
fi

# ./test9 
The  result is: 22
temp is smaller
{% endhighlight %}

现在在func1函数中使用```$temp```变量时，并不会影响在主体脚本中赋给```$temp```变量的值。

## 4. 数组变量和函数

这里我们首先介绍一下shell中的可变数组。如下所示，我们可以按此格式定义一个数组：
{% highlight string %}
# mytest=(one two three four five)
{% endhighlight %}
数组中每个元素之间用空格分割，并没有什么很特别的地方。如果你想把数组当做普通的值来显示，你可能要失望了：
{% highlight string %}
# echo $mytest
one
{% endhighlight %}
只有数组的第一个值显示出来了。要引用一个单独的数组元素，你必须要用代表它在数组中位置的数值索引值。数值要用方括号括起来：
{% highlight string %}
# echo ${mytest[2]}
three
{% endhighlight %}

要显示整个数组变量，可用星号作为通配符放在索引值的位置：
<pre>
# echo ${mytest[*]}
one two three four five
</pre>

你也可以改变某个索引值位置的值：
<pre>
# mytest[2]=seven
# echo ${mytest[*]}
one two seven four five
</pre>
你甚至能用unset命令来删除数组中的某个值，但是要小心，这可能会有点复杂。看下面的例子：
{% highlight string %}
# unset mytest[2]
# echo ${mytest[*]}
one two four five
# echo ${mytest[2]}

# echo ${mytest[3]}
four
{% endhighlight %}
这个例子用unset命令来删除索引值为2位置的值。显示整个数组时，看起来像是索引里面已经没这个索引了。但当专门显示索引值为2的位置的值时，能看到这个位置是空的。

最后，可以unset命令后跟上数组名来删除整个数组：
<pre>
# unset mytest
# echo ${mytest[*]}

</pre>
数组变量的遍历可以按如下方式进行：
{% highlight string %}
# for((i=0;i<${#mytest[@]}; i++)); do echo ${mytest[$i]}; done
one
two

four
# for i in ${mytest[*]}; do echo $i; done
one
two
four
five
{% endhighlight %}
上面```${#mytest[@]}```可以获取数组的长度。我们注意到两种遍历方式的输出有些不同，这主要是因为我们前面执行unset命令后，删除了元素的缘故。



有时可变数组会让事情很麻烦，所以在shell脚本编程时并不经常用。接下来我们会讨论在函数中使用数组变量。


### 4.1 向函数传递数组参数

向脚本函数传递数组变量的方法会有点不好理解。将数组变量当做单个参数传递的话，它不会起作用(说明：```$@```用于获取所有参数）：
{% highlight string %}
# cat badtest3 
#!/bin/bash

# trying to pass an array variable

function testit() {

   echo "The parameters are: $@"
   thisarray=$1

   echo "The received array is ${thisarray[*]}"
}

myarray=(1 2 3 4 5)

echo "The original array is: ${myarray[*]}"

testit $myarray

# ./badtest3 
The original array is: 1 2 3 4 5
The parameters are: 1
The received array is 1
{% endhighlight %}
如果你试图将该数组变量当成一个函数参数，函数只会取数组变量的第一个值。

要解决这个问题，你必须将该数组变量的值分解成单个值然后将这些值作为函数参数使用。在函数内部，你可以将所有的参数重组到新的数组变量中。下面是个具体的例子：
{% highlight string %}
# cat test10 
#!/bin/bash

# array variable to function test

function testit() {

   local newarray

   newarray=(`echo "$@"`)

   echo "The new array value is: ${newarray[*]}"
}

myarray=(1 2 3 4 5)

echo "The original array is: ${myarray[*]}"

testit ${myarray[*]}


# ./test10 
The original array is: 1 2 3 4 5
The new array value is: 1 2 3 4 5
{% endhighlight %}
该脚本用```$myarray```变量来保存所有的单个数组值，然后将它们都放在该函数的命令行上。之后该函数从命令行参数重建数组变量。在函数内部，数组仍然可以像其他数组一样使用：
{% highlight string %}
# cat test11 
#!/bin/bash

# adding values in an array

function addarray() {
   local sum=0
   local newarray

   newarray=(`echo $@`)

   for value in ${newarray[*]}
   do
       sum=$[$sum + $value]
   done

   echo $sum
}

myarray=(1 2 3 4 5)

echo "The original array is: ${myarray[*]}"

arg1=`echo ${myarray[*]}`

result=`addarray $arg1`

echo "The result is: $result"


# ./test11 
The original array is: 1 2 3 4 5
The result is: 15
{% endhighlight %}
addarray函数会遍历所有的数组值，将它们加在一起。你可以在myarray数组变量中放置任意多的值，addarray函数会将它们都加起来。

### 4.2 从函数返回数组

从函数里向shell脚本传回数组变量也用类似的方法。函数用echo语句来按正确顺序输出单个数组值，然后脚本再将它们重新放进一个新的数组变量中：
{% highlight string %}
# cat test12 
#!/bin/bash

# returning an array value

function arraydblr() {

   local origarray
   local newarray
   local elements

   local i

   origarray=(`echo $@`)

   newarray=(`echo $@`)

   elements=$[$# - 1]

   for((i=0;i<=$elements;i++))
   do
     newarray[$i]=$[${origarray[$i]} * 2]
   done

   echo ${newarray[*]}

}

myarray=(1 2 3 4 5)

echo "The original array is: ${myarray[*]}"

arg1=`echo ${myarray[*]}`

result=(`arraydblr $arg1`)

echo "The new array is: ${result[*]}"


# ./test12 
The original array is: 1 2 3 4 5
The new array is: 2 4 6 8 10
{% endhighlight %}
该脚本用```$arg1```变量将数组值传给arraydblr函数。arraydblr函数将该数组重组到新的数组变量中，生成该输出数组变量的一个副本。之后，它遍历了数组变量中的单个值，将每个值翻倍，并将结果放到函数中该数组变量的副本中。

之后，arraydblr函数使用echo语句来输出该数组变量值中的每个值。脚本用arraydblr函数的输出来重新生成一个新的数组变量。

## 5. 函数递归

局部函数变量提供的一个特性是自成体系（self-containment)。自成体系的函数不需要使用任何外部资源，除了脚本在命令行上传给它的变量。

这个特性使得函数可以递归地调用，也就是说函数可以调用自己来得到结果。通常，递归函数都有一个最终可以迭代到的基准值。许多高级数学算法用递归来一级一级地降解复杂的方程，直到基准值定义的那级。

递归算法的经典例子是计算阶乘。一个数的阶乘是该数之前的所有数乘以该数的值。因此，要计算5的阶乘，你可以执行如下方程：
<pre>
5! = 1 * 2 * 3 * 4 * 5 = 120
</pre>
使用递归，方程可简化成以下形式：
<pre>
x! = x * (x-1)!
</pre>

也就是说，x的阶乘等于 x 乘以 x-1 的阶乘。这可以用简单的递归脚本表达为：
{% highlight string %}
function factorial() {
    if [ $1 -eq 1]
    then
        echo 1
    else
        local temp=$[$1 - 1]
        local result=`factorial $temp`
        echo $[$result * $1]
    fi
}
{% endhighlight %}
阶乘函数用它自己来计算阶乘的值：
{% highlight string %}
# cat test13 
#!/bin/bash

# using recursion

function factorial() {
   if [ $1 -eq 1 ]
   then
       echo 1
   else
       local temp=$[$1 - 1]
       local result=`factorial $temp`
       echo $[$result * $1]
   fi
}

read -p "Enter value: " value

result=`factorial $value`

echo "The factorial of $value is: $result"

# ./test13 
Enter value: 5
The factorial of 5 is: 120
{% endhighlight %}
使用阶乘函数很容易。创建了这样的函数后，你可能想在其他脚本中使用它。下一步，我们将会了解如何有效地使用它。

## 6. 创建库
很容易发现，在单个脚本中，使用函数可以省去一些键入的工作，但如果你碰巧要在脚本间使用同一个代码块呢？显然，在每个脚本中都定义同样的函数而只使用一次会比较麻烦。

有个方法能解决这个问题。bash shell允许创建函数库文件，然后在需要时在多个脚本中引用该文件。

这个过程的第一步是创建一个包含脚本中所需函数的公用库文件。这里有个简单的称为myfuncs的库文件，它定义了3个简单的函数：
{% highlight string %}
# cat myfuncs 
#!/bin/bash

# my script functions

function addem() {
  echo $[$1 + $2]

}

function multem() {
   echo $[$1 * $2]
}

function divem() {
    if [ $2 -ne 0 ]
    then
       echo $[$1 / $2]
    else
       echo -1
    fi
}
{% endhighlight %}

下一步是在要用这些函数的脚本文件中包含myfuncs库文件。从这里开始，事情变得复杂起来。

问题出在shell函数的作用域上。和环境变量一样，shell函数只对定义它的shell会话有效。如果你在shell命令行界面的提示符下运行myfuncs shell脚本，shell会创建一个新的shell并在新shell中运行这个脚本。它会为那个新shell定义这3个函数，但当你运行另外一个用到这些函数的脚本时，它们却不可用。

这同样适用于脚本。如果你尝试将该库文件当做普通脚本文件运行，函数不会出现在脚本中：
{% highlight string %}
# cat badtest4 
#!/bin/bash

# using a library file the wrong way

./myfuncs

result=`addem 10 15`

echo "the result is: $result"

# ./badtest4 
./badtest4: line 7: addem: command not found
the result is: 
{% endhighlight %}
使用函数库的关键在于```source```命令。source命令会在当前的shell上下文中执行命令，而不是创建一个新的shell来执行命令。可以用source命令来在shell脚本中运行库文件脚本。这样函数就对脚本可用了。

source命令有个快捷别名，称作```点操作符```(dot operator)。要在shell脚本中运行myfuncs库文件，你只需添加下面这行：
<pre>
. ./myfuncs
</pre>
这个例子假定myfuncs库文件和shell脚本位于同一目录。如果不是，你需要使用相应路径访问该文件。这里有个用myfuncs库文件创建脚本的例子：
{% highlight string %}
# cat test14 
#!/bin/bash

# using functions defined in a library file

. ./myfuncs

value1=10
value2=5

result1=`addem $value1 $value2`

result2=`multem $value1 $value2`

result3=`divem $value1 $value2`

echo "The result of adding them is: $result1"
echo "The result of multiplying them is: $result2"
echo "The result of dividing them is: $result3"

# ./test14 
The result of adding them is: 15
The result of multiplying them is: 50
The result of dividing them is: 2
{% endhighlight %}

该脚本成功地使用了myfuncs库文件中定义的函数。

## 7. 在命令行上使用函数
可以用脚本函数来创建一些十分复杂的操作。有时，在命令行界面的提示符下直接使用这些命令也很有必要。

和在shell脚本中将脚本函数当命令使用一样，你也可以在命令行界面上将脚本函数当命令用。这是一个很好的功能，因为一旦你在shell中定义了函数，你就可以在整个系统上直接用它了，而无需担心脚本是不是在PATH环境变量里。重点在于让shell找到这些函数。有几种方法可以实现。

### 7.1 在命令行上创建函数
由于在键入命令时shell就会解释命令，你可以在命令行上直接定义一个函数。有两种方法。一种方法是在一行内定义整个函数：
{% highlight string %}
# function divem() { echo $[$1 / $2]; }
# divem 100 5
20
{% endhighlight %}
当你在命令行上定义函数时，你必须记住在每个命令后面加个分号，这样shell就能知道在哪里分开命令了：
{% highlight string %}
# function doubleit() { read -p "Enter value:" value; echo $[
> $value * 2]; }

# doubleit 
Enter value:20
40
{% endhighlight %}

另一种方法是用多行来定义函数。在定义时，bash shell会使用次提示符来提示输入更多命令。用这种方法，你不用在每条命令末尾放一个分号，只要按下回车键就行：
{% highlight string %}
# function multem() {
> echo $[$1 * $2]
> }

# multem 2 5
10
{% endhighlight %}
在函数的尾部使用花括号，shell就会知道你已经完成了函数的定义。

<pre>
警告： 在命令行上创建函数时要特别小心。如果你给函数起了个跟内建命令或另一个相同的名字，函数将会覆盖
      原来的命令。
</pre>

### 7.2 在.bashrc文件中定义函数
在命令行上直接定义shell函数的明显缺点是当退出shell时，函数就消失了。对于复杂的函数来说，这可能会是个问题。

一个简单得多的方法是将在每次启动新shell时都会加载的地方定义函数。最好的地方就是.bashrc文件。bash shell会在每次启动时在主目录查找这个文件，不管是交互式的还是从现有shell启动一个新的shell。

1） **直接定义函数**

可以直接在主目录的.bashrc文件中定义函数。许多Linux发行版已经在.bashrc文件中定义了一些东西了。所以注意别删掉这些内容，只要在已有文件的末尾加上你写的函数就行了。这里有个例子：
{% highlight string %}
# cat .bashrc
# .bashrc

# Source global definitions
if [ -r /etc/bashrc ]; then
     . /etc/bashrc
fi

function addem() {
    echo $[$1 + $2]
}
{% endhighlight %}
直到下次启动新的bash shell时该函数才会生效。添加并重启bash shell后，你就能在系统上任意地方使用该函数了。

2） **读取函数文件**

只要是在shell脚本中，你都可以用source命令（或者它的别名： 点操作符）来将已有库文件中的函数添加到你的.bashrc脚本中：
{% highlight string %}
# cat .bashrc
# .bashrc

# Source global definitions
if [ -r /etc/bashrc ]; then
     . /etc/bashrc
fi

. /home/ivan1001/libraries/myfuncs
{% endhighlight %}
确保包含了引用库文件的正确路径名，以便于bash shell来查看该文件。下次启动shell时，库中的所有函数都可以在命令行界面下使用了。
<pre>
# addem 10 5
15

# multem 10 5
50

# divem 10 5
2
</pre>
更好的一点是，shell还会将定义好的函数传给子shell进程。这样这些函数在该shell会话中的任何shell脚本中也都可用。你可以写个脚本来测试，直接使用这些函数而不用单独定义或读取它们：
{% highlight string %}
# cat test15
#!/bin/bash

# using a function defined in .bashrc file

value1=10
value2=5

result1=`addem $value1 $value2`

result2=`multem $value1 $value2`

result3=`divem $value1 $value2`

echo "The result of adding them is: $result1"
echo "The result of multiplying them is: $result2"
echo "The result of dividing them is: $result3"


//注意： 实际在Centos上，需要使用source命令来执行，否则识别不了
# source ./test15
The result of adding them is: 15
The result of multiplying them is: 50
The result of dividing them is: 2
{% endhighlight %}
甚至不用读取库文件，这些函数也能在shell脚本中很好地工作。


<br />
<br />

**[参看]**






<br />
<br />
<br />


