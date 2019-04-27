---
layout: post
title: 高级脚本编程之处理用户输入
tags:
- LinuxOps
categories: linuxOps
description: 高级脚本编程之处理用户输入
---


到目前为止你看到的都是如何编写脚本处理数据、变量和Linux系统上的文件。有时，你需要写个和运行脚本的人交互的脚本。bash shell提供了一些不同的方法来从用户处获得数据，包括命令行参数（添加在命令后的数据值）、命令行选项（可修改命令行为的单字母值）以及直接从键盘读取输入的能力。本章将会讨论如何将这些不同的方法放进你的bash shell脚本来从运行脚本的用户处获得数据。




<!-- more -->

## 1. 命令行参数
向shell脚本传数据的最基本方法是使用命令行参数。命令行参数允许在运行脚本时向命令行添加数据值：
{% highlight string %}
# cat addem 
#!/bin/bash

echo $[$1+$2]

# ./addem 10 30
40
{% endhighlight %}
本例向脚本addem传递了两个命令行参数（10和30）。脚本会通过特殊的变量来处理命令行参数。后面几节将会介绍如何在bash shell脚本中使用命令行参数。

### 1.1 读取参数
bash shell会将一些称为```位置参数```(positional parameter)的特殊变量分配给命令行输入的所有参数。这甚至包括shell执行的程序的名字。位置参数变量是标准的数字： $0是程序名，$1是第一个参数，$2是第二个参数，依次类推，直到第九个参数$9。

下面是在shell脚本中使用单个命令行参数的简单例子：
{% highlight string %}
# cat test1 
#!/bin/bash

# using one command line parameter

factorial=1

for((number=1; number <= $1; number++))
do 
    factorial=$[$factorial * $number]
done

echo "The factorial of $1 is $factorial"

# ./test1 5
The factorial of 5 is 120
{% endhighlight %}
可以在shell脚本中像使用其他变量一样使用$1变量。shell脚本会自动将命令行参数的值分配给变量，不需要做任何处理。

如果需要输入更多的命令行选项，则在命令行上每个参数都必须用空格分开：
{% highlight string %}
# cat test2
#!/bin/bash

# testing two command line parameters

total=$[$1 * $2]
echo "The first parameter is: $1"
echo "The second parameter is: $2"

echo "The total value is: $total"

# ./test2 2 5
The first parameter is: 2
The second parameter is: 5
The total value is: 10
{% endhighlight %}
shell会将每个参数分配给对应的变量。

在本例中，用到的命令行参数都是数值。也可以在命令行上用文本字符串：
{% highlight string %}
# cat test3 
#!/bin/bash

# testing string parameters

echo "Hello $1, glad to meet you"

# ./test3 Rich
Hello Rich, glad to meet you
{% endhighlight %}
shell将输入到命令行的字符串值传给了脚本。但在用含有空格的文本字符串时，会遇到问题：
<pre>
# ./test3 Rich Blum
Hello Rich, glad to meet you
</pre>
记住，每个参数都是用空格分割的，所以shell会将空格当成分割两个值的分隔符。要在参数值中包含空格，必须要用引号（单引号、双引号均可）：
{% highlight string %}
# ./test3 'Rich Blum'
Hello Rich Blum, glad to meet you

# ./test3 "Rich Blum"
Hello Rich Blum, glad to meet you
{% endhighlight %}
<pre>
说明： 将文本字符串作为参数传递时，引号并不是数据的一部分。它们只是将数据的开始和结尾与
      其他内容分开。
</pre>

如果脚本需要多于9个命令行参数，你仍然可以处理，但是需要稍微修改一下变量名。在第9个变量之后，你必须在变量数字周围加花括号，比如${10}。下面是个实现的例子：
{% highlight string %}
# cat test4 
#!/bin/bash

# handling lots of parameters

total=$[${10} * ${11}]

echo "The tenth parameter is: ${10}"
echo "The eleventh parameter is: ${11}"
echo "The total is: $total"

# ./test4 1 2 3 4 5 6 7 8 9 10 11 12
The tenth parameter is: 10
The eleventh parameter is: 11
The total is: 110
{% endhighlight %}
这个技术将允许你向脚本添加任意多的要用的命令行参数。

### 1.2 读取程序名
你可以用```$0```参数来获取shell在命令行启动的程序的名字。这在写有多个功能的工具时很方便。但有个小问题需要处理下。看下面这个简单的例子中会怎样：
{% highlight string %}
# cat test5 
#!/bin/bash

# testing the $0 parameter

echo "The command entered is: $0"

# ./test5
The command entered is: ./test5
# /root/workspace/test5 
The command entered is: /root/workspace/test5
{% endhighlight %}
当传给$0变量的真实字符串是整个脚本的路径时，程序中就会使用整个路径，而不仅仅是程序名。

如果你要写个基于命令行下运行的脚本名来执行不同功能的脚本，这需要一点功夫。你需要能去掉命令行下运行脚本的任何路径。

幸而有个方便的小命令正好可以做这个。basename命令只会返回程序名而不包括路径。让我们修改一下示例脚本，看看它如何工作：
{% highlight string %}
# cat test5b 
#!/bin/bash

# using basename with the $0 parameter

name=`basename $0`
echo "The command entered is: $name"

# ./test5b 
The command entered is: test5b
# /root/workspace/test5b 
The command entered is: test5b
{% endhighlight %}
现在好多了。你可以用这种方法来编写基于所用的脚本名而执行不同功能的脚本。这里有个简单的例子来说明：
{% highlight string %}
# cat test6
#!/bin/bash

# testing a multi-function script

name=`basename $0`

if [ $name = "addem" ]
then
    total=$[$1 + $2]
elif [ $name = "multem" ]
then
    total=$[$1 * $2]
fi

echo "The calculated value is: $total"

# cp test6 addem
# ln -sf ./test6 multem
# ls -al
total 8
drwxr-xr-x.  2 root root   46 Apr 26 17:28 .
dr-xr-x---. 41 root root 4096 Apr 26 10:57 ..
-rwxrwxrwx.  1 root root  213 Apr 26 17:28 addem
lrwxrwxrwx.  1 root root    7 Apr 26 17:28 multem -> ./test6
-rwxrwxrwx.  1 root root  213 Apr 26 17:26 test6
# ./addem 2 5
The calculated value is: 7
# ./multem 2 5
The calculated value is: 10
{% endhighlight %}
本例从test6的代码创建了两个不同的文件名，一个通过复制文件创建，另一个通过链接创建。在两种情况中，脚本都会先获得脚本的基名，然后根据该值执行相应的功能。

### 1.3 测试参数
在shell脚本中使用命令行参数时要小心些。如果脚本不加参数运行，可能会出问题：
{% highlight string %}
# cat addem 
#!/bin/bash

# testing a multi-function script

name=`basename $0`

if [ $name = "addem" ]
then
    total=$[$1 + $2]
elif [ $name = "multem" ]
then
    total=$[$1 * $2]
fi

echo "The calculated value is: $total"

# ./addem 2
./addem: line 9: 2 + : syntax error: operand expected (error token is "+ ")
The calculated value is:
{% endhighlight %}

当脚本认为参数变量中会有数据而实际上并没有时，你很可能会从脚本得到一个错误消息。这种写脚本的方法并不可取。在使用数据前检查数据确实已经存在于变量里很有必要：
{% highlight string %}
# cat test7 
#!/bin/bash

# testing parameters before use

if [ -n "$1" ]
then
    echo "Hello $1, glad to meet you"
else
    echo "Sorry, you did not identify yourself"
fi

# ./test7 Rich
Hello Rich, glad to meet you
# ./test7 
Sorry, you did not identify yourself
{% endhighlight %}
在本例中，在test命令里使用了```-n```参数来检查命令行参数中是否有数据。在下一节中，你会看到还有另一种检查命令行参数的方法。

## 2. 特殊参数变量
在bash shell中有些特殊变量，它们会记录命令行参数。本节将会介绍它们都是哪些变量以及如何使用它们。

1） **参数计数**

如你在上一节中看到的，通常在脚本中使用命令行参数之前应该检查一下命令行参数。对于使用多个命令行参数的脚本来说，这有点麻烦。

你可以只数一下命令行中输入了多少个参数，而不测试每个参数。bash shell为此提供了一个特殊变量。

```$#```特殊变量含有脚本运行时就有的命令行参数的个数。你可以在脚本中任何地方使用这个特殊变量，就跟普通变量一样：
{% highlight string %}
# cat test8 
#!/bin/bash

# getting the number of parameters

echo "There were $# parameter(s) supplied"

# ./test8 
There were 0 parameter(s) supplied
# ./test8 1 2 3 4 5
There were 5 parameter(s) supplied
# ./test8 "Rich Blum"
There were 1 parameter(s) supplied
{% endhighlight %}
现在你就能在使用参数前测试参数的总数了：
{% highlight string %}
# cat test9
#!/bin/bash

# testing parameters 

if [ $# -ne 2 ]
then
    echo "Usage: test9 a b"
else
    total=$[$1 + $2]
    echo "The total is: $total"
fi

# ./test9 
Usage: test9 a b
# ./test9 10
Usage: test9 a b
# ./test9 10 15
The total is: 25
# ./test9 10 15 20
Usage: test9 a b
{% endhighlight %}

if-then语句用test命令来对命令行提供的参数总数执行数值测试。如果参数总数不对，你可以打印一条错误消息说明脚本的正确用法。

这个变量还提供了一个简便方法来在命令行上抓取最后一个参数，而不用知道到底用了多少个参数。不过你需要花点功夫。

如果你仔细考虑过，你可能会觉得既然```$#```变量含有参数的总数值，那变量```${$#}```就代表了最后一个命令行参数变量。试试看会发生什么：
{% highlight string %}
# cat badtest1 
#!/bin/bash

# testing grabbing last parameter

echo "The last parameter was: ${$#}"

# ./badtest1 
The last parameter was: 23341
{% endhighlight %}
啊，怎么了？显然，出了问题。它表明你不能在花括号内使用美元符。你必须将美元符换成```感叹号```。很奇怪，它竟然能工作：
{% highlight string %}
# cat test10
#!/bin/bash

# grabbing the last parameter

params=$#

echo "The last parameter is $params"
echo "The last parameter is ${!#}"

# ./test10 1 2 3 4 5
The last parameter is 5
The last parameter is 5
# ./test10 
The last parameter is 0
The last parameter is ./test10
{% endhighlight %}

太好了。这个测试将```$#```变量的值赋给了变量params，然后也按特殊命令行参数变量的格式使用了该变量。两个版本都能工作。重要的是要注意，当命令行上没有任何参数时，```$#```的值为零，在params变量中也为零，但```${!#}```变量会返回命令行用到的脚本名。

2) **抓取所有的数据**

有些情况下，你只想抓取命令行上提供的所有参数，然后遍历它们。你可以使用一组其他的特殊变量，而不用先用```$#```变量来判断命令行上有多少参数然后再遍历它们。

```$*```和```$@```变量提供了多所有参数的快速访问。这两个都能够在单个变量中存储所有的命令行参数。

```$*```变量会将命令行上提供的所有参数当做单个单词保存。每个词是指命令行上出现的每个值。基本上，```$*```变量会将这些都当做一个参数，而不是多个对象。

反过来说，```$@```变量会将命令行上提供的所有参数当做同一字符串中的多个独立的单词。它允许你遍历所有的值，将提供的每个参数分隔开来。这通常通过for命令完成。

理解这两个变量如何工作可能容易叫人困惑。让我们看个例子，你就能理解二者之间的区别了：
{% highlight string %}
# cat test11 
#!/bin/bash

# testing $* and $@

echo "Using the \$* method: $*"

echo "Using the \$@ method: $@"

# ./test11 rich barbara katie jessica
Using the $* method: rich barbara katie jessica
Using the $@ method: rich barbara katie jessica
{% endhighlight %}

表面上看，两个变量产生的是同样的输出，都立即显示了提供的所有命令行参数。下面的例子给出了二者的差异：
{% highlight string %}
# cat test12 
#!/bin/bash

# testing $* and $@

count=1

for param in "$*"
do
    echo "\$* Parameter #$count = $param"
    count=$[$count + 1]
done

count=1

for param in "$@"
do
    echo "\$@ Parameter #$count = $param"
    count=$[$count + 1]
done

# ./test12 rich barbara katie jessica
$* Parameter #1 = rich barbara katie jessica
$@ Parameter #1 = rich
$@ Parameter #2 = barbara
$@ Parameter #3 = katie
$@ Parameter #4 = jessica
{% endhighlight %}

现在清楚多了。通过使用for命令遍历这两个特殊变量，你能看到它们是如何不同地处理命令行参数。```$*```变量会将所有参数当成单个参数，而```$@```变量会单独处理每个参数。这是遍历命令行参数的绝妙方法。

另外，前面我们也说过可以使用```$#```来获取所有参数个数，然后再来遍历，这里给出一个示例：
{% highlight string %}
# cat test12_1 
#!/bin/bash

# tranverse all parameters

for((i=1;i<=$#;i++))
do
   echo "Parameter $i: ${!i}"
done

# ./test12_1 rich barbara katie jessica
Parameter 1: rich
Parameter 2: barbara
Parameter 3: katie
Parameter 4: jessica
{% endhighlight %}

最后，我们对shell中用到的几个$特殊变量作一个总结：
<pre>
            表： shell中$特殊变量

参数变量                说明
----------------------------------------------------------------------
  $#         传递到脚本的参数个数
  $*         以一个单字符串显示所有向脚本传递的参数
  $$         脚本运行的当前进程ID号
  $!         后台运行的最后一个进程的ID号
  $@         与$*类似，但会将命令行上提供的所有参数当做同一字符串中的多个独立的单词 
  $-         显示shell使用的当前选项，与set命令功能相同
  $?         显示最后命令的退出状态。0表示没有错误，其他任何值表明有错误                  
</pre>

## 3. 移动变量
bash shell工具链中另一个工具是shift命令。bash shell提供了shift命令来帮助操作命令行参数。跟字面上的意思一样，shift命令会根据它们的相对位置来移动命令行参数。

在使用shift命令时，默认情况下它会将每个参数变量减一。所以，变量$3的值会移到$2，变量$2的值会移到$1，而变量$1的值则会被删除（注意，变量$0的值，也就是程序名不会改变）。

这是遍历命令行参数的另一个绝妙方法，尤其是在你不知道到底有多少参数时。你可以只操作第一个参数，移动参数，然后继续操作第一个参数。

这里有个例子来解释它如何工作：
{% highlight string %}
# cat test13 
#!/bin/bash

# demonstrating the shift command

count=1

while [ -n "$1" ]
do
   echo "Parameter #$count = $1"
   count=$[$count + 1]

   shift
done

# ./test13 rich barbara katie jessica
Parameter #1 = rich
Parameter #2 = barbara
Parameter #3 = katie
Parameter #4 = jessica
{% endhighlight %}
这个脚本通过测试第一个参数值的长度执行了一个while循环。当第一个参数的长度为零时，循环结束。

测试第一个参数后，shift命令会将所有参数的位置移动一位。

另外，你也可以给shift命令提供一个参数来执行多位移动。只要提供你想移动的位数就行：
{% highlight string %}
# cat test14 
#!/bin/bash

# demonstrating a multi-position shift

echo "The original parameters: $*"

shift 2

echo "Here's the new first parameter: $1"

# ./test14 1 2 3 4 5
The original parameters: 1 2 3 4 5
Here's the new first parameter: 3
{% endhighlight %}
<pre>
警告： 使用shift命令时要小心。当一个参数被移除后，它的值会被丢掉并且无法恢复。
</pre>

## 4. 处理选项
如果你认真读过本书前面的所有内容，你应该见过了几个同时提供了参数和选项的bash命令。选项(Options)是跟在单破折线后面的单个字母，能改变命令的行为。本节将会介绍3种不同的处理shell脚本中选项的方法。

### 4.1 查找选项
表面上看，命令行选项也没什么特殊的。在命令行上，它们紧跟在脚本名之后，就跟命令行参数一样。实际上，如果愿意，你可以像处理命令行参数一样处理命令行选项。

1） **处理简单选项**

在前面的test13脚本中，你看到了如何使用shift命令来向下移动提供给脚本程序的命令行参数。你也可以用同样的方法来处理命令行选项。

在提取每个单独参数时，用case语句来判断参数是否被格式化成了选项：
{% highlight string %}
# cat test15 
#!/bin/bash

# extracting command line options as parameters

while [ -n "$1" ]
do
    case "$1" in
      -a) echo "Found the -a option";;
      -b) echo "Found the -b option";;
      -c) echo "Found the -c option";;
      *) echo "$1 is not an option";;
    esac

    shift
done

# ./test15 -a -b -c -d
Found the -a option
Found the -b option
Found the -c option
-d is not an option
{% endhighlight %}
case语句会检查每个参数是不是有效选项。是的话，相应的命令就会在case语句中运行。不管选项是按什么顺序出现在命令行上，这种方法都适用：
<pre>
# ./test15 -d -c -a
-d is not an option
Found the -c option
Found the -a option
</pre>
case语句会在命令行参数中找到选项时就处理每个选项。如果在命令行上还提供了其他参数，你可以在case语句的通用情况处理部分中处理。

2） **分离参数和选项**

你会经常遇到想在shell脚本中同时使用选项和参数的情况。Linux中处理这个问题的标准方式是用特殊字符来将二者分开，该字符会告诉脚本选项何时结束以及普通参数何时开始。

对于Linux来说，这个特殊字符是双破折线（--）。shell会用双破折线来表明选项结束了。看到双破折线之后，脚本会安全地将剩下的命令行参数当做参数来处理，而不是选项。

要检查双破折线，只要在case语句中加一项就行了：
{% highlight string %}
# cat test16 
#!/bin/bash

# extracting options and parameters

while [ -n "$1" ]
do
   case "$1" in
     -a) echo "Found -a option";;
     -b) echo "Found -b option";;
     -c) echo "Found -c option";;
     --) shift
         break;;
      *) echo "$1 is not an option";;
   esac
   
   shift
done

count=1

for param in $@
do
   echo "Parameter #$count = $param"

   count=$[$count + 1]
done
{% endhighlight %}
这段脚本用break命令来在它遇到双破折线时跳出while循环。由于过早的跳出了循环，我们需要再加一条shift命令来将双破折线移出参数变量。

对于第一个测试，试试用一组普通的选项和参数来运行这个脚本：
<pre>
# ./test16 -c -a -b test1 test2 test3
Found -c option
Found -a option
Found -b option
test1 is not an option
test2 is not an option
test3 is not an option
</pre>
结果说明，在处理脚本时以为所有的命令行参数都是选项。下一步，试试同样的操作，只是这次会用双破折线来将命令行上的选项和参数划分开来：
<pre>
# ./test16 -c -a -b -- test1 test2 test3
Found -c option
Found -a option
Found -b option
Parameter #1 = test1
Parameter #2 = test2
Parameter #3 = test3
</pre>
当脚本遇到双破折线时，它就停止处理选项了，并将剩下的参数都当做命令行参数。

3) **处理带值的选项**





<br />
<br />

**[参看]**

1. [linux的shell函数参数](http://www.cnblogs.com/liuwenbohhh/p/4650463.html)




<br />
<br />
<br />


