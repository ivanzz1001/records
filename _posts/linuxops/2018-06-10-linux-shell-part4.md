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

每次引用```func1```函数名时，bash shell会回到func1函数的定义并指定你在那里定义的命令。

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
















<br />
<br />

**[参看]**






<br />
<br />
<br />


