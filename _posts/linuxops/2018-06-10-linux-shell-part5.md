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

有些选项会带上一个额外的参数值。在这种情况下，命令看起来像下面这样：
<pre>
# ./testing -a test1 -b -c -d test2
</pre>
当命令行选项要求额外的参数时，脚本必须能检测并能正确地处理。下面有个如何处理的例子：
{% highlight string %}
# cat test17 
#!/bin/bash

# extracting command line options and values

while [ -n "$1" ]
do
   case "$1" in
       -a) echo "Found the -a option";;
       -b) param="$2"
           echo "Found the -b option, with parameter value $param"
           shift;;
       -c) echo "Found the -c option";;
       --) shift
           break;;
       *) echo "$1 is not an option";;
   esac 

   shift
done


count=1

for param in "$@"
do
   echo "Parameter #$count = $param"

   count=$[$count + 1]
done

# ./test17 -a -b test1 -d
Found the -a option
Found the -b option, with parameter value test1
-d is not an option
{% endhighlight %}
在这个例子中，case语句定义了3个它要处理的选项。```-b```选项也要求一个额外的参数值。由于要处理的参数是```$1```，额外的参数值就应该位于$2(因为所有的参数在处理完之后都会被移出去）。只要将参数值从$2变量中提取出来就可以了。

只用基本的特性，这个过程就能工作，不管按什么顺序放置选项（但要记住包含每个选项相应的选项参数）：
<pre>
# ./test17 -b test1 -a -d
Found the -b option, with parameter value test1
Found the -a option
-d is not an option
</pre>

现在shell脚本中已经有了处理命令行选项的基本能力，但还有一些限制。比如，如果你想将多个选项放进一个参数中时，它就不能工作了：
<pre>
# ./test17 -ac
-ac is not an optio
</pre>
在Linux中，合并选项是一个很常见的用法，而且如果脚本要更用户友好一些，那么也要给用户提供这种特性。幸好，有另外一种处理选项的方法能够帮忙。

### 4.2 使用getopt命令
getopt命令是一个在处理命令行选项和参数时非常方便的工具。它能够识别命令行参数，从而在脚本中解析它们时更方便。

1） **命令的格式**

getopt命令可以接受一系列任意形式的命令行选项和参数，并自动将它们转换成适当的格式。它的命令格式如下：
<pre>
getopt options optstring parameters
</pre>
optstring 是这个过程的关键所在。它定义了命令行有效的选项字母，还定义了哪些选项字母需要参数值。

首先，在optstring 中列出你要在脚本中用到的每个命令行选项字母。然后，在每个需要参数值的选项字母后加一个冒号。getopt命令会基于你定义的optstring解析提供的参数。

下面是个getopt如何工作的简单例子：
<pre>
# getopt ab:cd -a -b test1 -cd test2 test3
 -a -b test1 -c -d -- test2 test3
</pre>
optstring定义了4个有效选项字母： a、b、c和d。它还定义了选项字母b需要一个参数值。当getopt命令运行时，它会检查提供的参数列表，并基于提供的optstring解析。注意它会自动将```-cd```选项分成两个单独的选项，并插入双破折号来分开行中的额外参数。

如果你指定了一个不在optstring中的选项，默认情况下，getopt会产生一条错误消息：
<pre>
# getopt ab:cd -a -b test1 -cde test2 test3
getopt: invalid option -- 'e'
 -a -b test1 -c -d -- test2 test3
</pre>
如果想忽略这条错误消息，可以在命令后加```-q```选项：
<pre>
# getopt -q ab:cd -a -b test1 -cde test2 test3
 -a -b 'test1' -c -d -- 'test2' 'test3'
root@ubuntu:~/workspace# 
</pre>
注意，getopt命令必须列在optstring之前。现在你可以在脚本中使用此命令处理命令行选项了。

2) **在脚本中使用getopt**

你可以在脚本中使用getopt来格式化输入给脚本的任何命令行选项。但用起来略微复杂。

方法是用getopt命令生成的格式化后的版本来替换已有的命令行选项和参数。用set命令能够做到。

在第5章中，你就已经见过set命令了。set命令能够和shell中的不同变量一起工作。第5章介绍了如何使用set命令来显示所有的系统环境变量。

set命令的选项之一是双破折线，它会将命令行参数替换成set命令的命令行的值。

然后，该方法会将原始的脚本的命令行参数传给getopt命令，之后再将getopt命令的输出传给set命令，用getopt格式化后的命令行参数来替换原始的命令行参数。看起来如下：
<pre>
set -- `getopt -q ab:cd "$@"`
</pre>
现在原始的命令行参数变量的值会被getopt命令的输出替换，而getopt已经为我们格式化好了命令行参数。

利用该方法，现在我们就就可以写出能帮我们处理命令行参数的脚本了：
{% highlight string %}
# cat test18 
#!/bin/bash

# extracting command line options and values with getopt

set -- `getopt -q ab:c "$@"`

while [ -n "$1" ]
do
    case "$1" in
       -a) echo "Found the -a option";;

       -b) param=$2
           echo "Found the -b option, with parameter value $param"
           shift;;

       -c) echo "Found the -c option";;

       --) shift
           break;;

        *) echo "$1 is not an option"
      esac

      shift
done

count=1

for param in "$@"
do
   echo "Parameter #$count: $param"

   count=$[$count + 1]

done
{% endhighlight %}
你会注意到它跟test17脚本一样。唯一不同的是加入了getopt命令来帮助格式化命令行参数。现在运行脚本并加上复杂选项，可以看出它工作得更好了：
<pre>
# ./test18 -ac
Found the -a option
Found the -c option
</pre>
当然，所有的原始功能还能顺利工作：
<pre>
# ./test18 -a -b test1 -cd test2 test3 test4
Found the -a option
Found the -b option, with parameter value 'test1'
Found the -c option
Parameter #1: 'test2'
Parameter #2: 'test3'
Parameter #3: 'test4'
</pre>
现在事情看起来好多了。但是，仍然有一个小问题潜伏在getopt命令中。看看这个例子：
<pre>
# ./test18 -a -b test1 -cd "test2 test3" test4
Found the -a option
Found the -b option, with parameter value 'test1'
Found the -c option
Parameter #1: 'test2
Parameter #2: test3'
Parameter #3: 'test4'
</pre>
getopt命令并不擅长处理带空格的参数值。它会将空格当做参数分隔符，而不是根据双引号将二者当做一个参数。幸而，还有另外一个办法能解决这个问题。

### 4.3 使用更高级的getopts
bash shell包含了getopts命令(注意是复数）。它跟近亲getopt看起来很像，但有一些扩展功能。

与getopt将命令行上找到的选项和参数处理后只生成一个输出不同，getopts命令能够和已有的shell参数变量对应地顺序工作。

getopts命令的格式如下：
<pre>
getopts optstring variable
</pre>
optstring值类似于getopt命令中的那个。有效的选项字母都会列在optstring中，如果选项字母要求有个参数值，就加一个冒号。要去掉错误消息的话，可以在optstring之前加一个冒号。getopts命令将当前参数保存在命令行中定义的variable中。

getopts命令会用到两个环境变量。如果选项需要跟一个参数值，```OPTARG```环境变量就会保存这个值。```OPTIND```环境变量保存了参数列表中getopts正在处理的参数位置。这样你就能在处理完选项之后继续处理其他命令行参数了。

让我们看个使用getopts命令的简单例子：
{% highlight string %}
# cat test19 
#!/bin/bash

# simple demonstration of the getopts command

while getopts :ab:c opt
do
   case "$opt" in
     a) echo "Found the -a option";;
      
     b) echo "Found the -b option, with value $OPTARG";;

     c) echo "Found the -c option";;

     *) echo "Unknown option: $opt";;

   esac
done

# ./test19 -ab test1 -c
Found the -a option
Found the -b option, with value test1
Found the -c option
{% endhighlight %}
while语句定义了getopts命令，指明了要查找哪些命令行选项，以及每次迭代中存储它们的变量名。

你会注意到在本例中case语句的用法有些不同。getopts命令解析命令行选项时，它会移除开头的单破折线，所以在case定义中不用单破折线。

getopts命令有几个好用的功能。对新手来说，你可以在参数值中包含空格：
<pre>
# ./test19 -b "test1 test2" -a
Found the -b option, with value test1 test2
Found the -a option
</pre>

另一个好用的功能是将选项字母和参数值放在一起使用，而不用加空格：
<pre>
# ./test19 -abtest1
Found the -a option
Found the -b option, with value test1
</pre>
getopts命令能够从```-b```选项中正确解析出test1值。getopts命令的另一个好用的功能是，它能够将命令行上找到的所有未定义的选项统一输出成问好：
<pre>
# ./test19 -d
Unknown option: ?
# ./test19 -acde
Found the -a option
Found the -c option
Unknown option: ?
Unknown option: ?
</pre>
optstring中未定义的选项字母会以问号形式发送给代码。

getopts命令知道何时停止处理选项，并将参数留给你处理。在getopts处理每个选项时，它会将```OPTIND```环境变量增一。在getopts完成处理时，你可以将OPTIND值和shift命令一起使用来移动参数：
{% highlight string %}
# cat ./test20 
#!/bin/bash

# processing options and parameters with getopts

while getopts :ab:cd opt
do
    case "opt" in
       a) echo "Found the -a option";;

       b) echo "Found the -b option, with value: $OPTARG";;

       c) echo "Found the -c option";;

       d) echo "Found the -d opiton";;

       *) echo "Unknown option: $opt";;
 
    esac

done

shift $[$OPTIND -1]

count=1

for param in "$@"
do
    echo "Parameter #$count: $param"

    count=$[$count + 1]
done


# ./test20 -a -b test1 -d test2 test3 test4
Unknown option: a
Unknown option: b
Unknown option: d
Parameter #1: test2
Parameter #2: test3
Parameter #3: test4
{% endhighlight %}
现在你就有了一个能在所有shell脚本中使用的全功能命令行选项和参数处理工具。

## 5. 将选项标准化

在创建shell脚本时，显然你可以控制具体怎么做。你完全可以决定用哪些字母选项和如何使用。

但有些字母选项在Linux世界里已经演变成某种标准的含义。如果你能在shell脚本中支持这些选项，脚本看起来能更友好些。

下表显示了Linux中用到的一些命令行选项的通用含义：
<pre>
                表： 通用的Linux命令选项

选 项                    描  述
-----------------------------------------------------------------------
 -a                   显示所有对象
 -c                   生成一个计数
 -d                   指定一个目录
 -e                   扩展一个对象
 -f                   指定读入数据的文件
 -h                   显示命令的帮助信息
 -i                   忽略文本大小写
 -l                   产生输出的长格式版本
 -n                   使用非交互模式（批量）
 -o                   指定将所有输出重定向到输出文件
 -q                   以安静模式运行
 -r                   递归地处理目录和文件
 -s                   以安静模式运行
 -v                   生成详细输出
 -x                   排除某个对象
 -y                   对所有问题回答yes
</pre>
通过学习本书时遇到的各种bash命令，你大概已经知道这些选项中大部分的含义了。你的选项也采用同样的含义，会让用户


## 6. 获得用户输入
尽管命令行选项和参数是从脚本用户获得输入的一种重要方式，但有时脚本的交互性还可以更强一些。有时你想要在脚本运行时问个问题，并等待运行脚本的人来回答。bash shell为此提供了read命令。

### 6.1 基本的读取
read命令接受从标准输入（键盘）或另外一个文件描述符（参见<<高级脚本编程之处理用户输入>>一文)的输入。在收到输入后，read命令会将数据放进一个标准变量。下面是read命令的最简单用法：
{% highlight string %}
# cat test21 
#!/bin/bash

# testing the read command

echo -n "Enter your name: "

read name

echo "Hello $name, welcome to my program."

# ./test21 
Enter your name: Rich Blum
Hello Rich Blum, welcome to my program.
{% endhighlight %}

相当简单。注意生成提示的echo命令使用了```-n```选项。它会移掉字符串末尾的换行符，允许脚本用户紧跟其后输入数据，而不是下一行。这让脚本看起来更像表单。

实际上，read命令包含了```-p```选项，允许你直接在read命令行指定提示符：
{% highlight string %}
# cat test22 
#!/bin/bash

# testing the read -p option

read -p "Please enter your age: " age

days=$[$age * 365]

echo "That makes you over $days days old!"

# ./test22 
Please enter your age: 10
That makes you over 3650 days old!
{% endhighlight %}

你会注意到，在第一个例子中当有名字输入时，read命令会将姓和名保存在同一个变量中。read命令会为提示符输入的所有数据分配一个变量，或者你也可以指定多个变量。输入的每个数据值都会分配给表中的下一个变量。如果变量表在数据之前用完了，剩下的数据就都会分配给最后一个变量：
{% highlight string %}
# cat test23 
#!/bin/bash

# entering multiple variables

read -p "Enter your name: " first last

echo "Checking data for $last, $first..."

# ./test23 
Enter your name: Rich Blum Cort
Checking data for Blum Cort, Rich...
{% endhighlight %}

你可以在read命令行中不指定变量。如果那么做了，read命令会将它收到的任何数据都放进特殊环境变量```REPLY```中：
{% highlight string %}
# cat ./test24 
#!/bin/bash

# testing the REPLY environment variable

read -p "Enter a number: "

factorial=1

for((count=1; count <=$REPLY; count++))
do
    factorial=$[$factorial * $count] 
done

echo "The factorial of $REPLY is $factorial"

# ./test24 
Enter a number: 5
The factorial of 5 is 120
{% endhighlight %}
```REPLY```环境变量会保存输入的所有数据，它可以在shell脚本中像其他变量一样使用。

### 6.2 超时
使用read命令时有个危险，就是脚本很可能会等脚本用户的输入一直等下去。如果脚本必须继续执行下去，不管是否有数据输入，你可以用```-t```选项来指定一个计时器。```-t```选项指定了read命令等待输入的秒数。当计时器过期后，read命令会返回一个非零退出状态码：
{% highlight string %}
# cat ./test25 
#!/bin/bash

# timing the data entry

if read -t 5 -p "Please enter your name: " name
then
    echo "Hello $name, welcome to my script"
else
    echo 
    echo "Sorry, too slow"
fi

# ./test25 
Please enter your name: Rich
Hello Rich, welcome to my script
# ./test25 
Please enter your name: 
Sorry, too slow
{% endhighlight %}
由于计时器过期的话read命令会以非零退出状态码退出，我们可以使用标准的结构化语句，如if-then语句或while循环来轻松地记录发生的情况。在本例中，计时器过期时，if语句不成立，shell会执行else部分的命令。

可以让read命令来对输入的字符计数，而非对输入过程计时。当输入的字符达到预设的字符数时，它会自动退出，将输入的数据赋给变量：
{% highlight string %}
# cat test26 
#!/bin/bash

# getting just one character of input

read -n1 -p "Do you want to continue [Y/N]? " answer

case $answer in
  Y | y) echo 
         echo "find, continue on ...";;
  N | n) echo
         echo "OK, goodbye"
         exit;;
esac

echo "This is the end of the script"

# ./test26 
Do you want to continue [Y/N]? Y
find, continue on ...
This is the end of the script

# ./test26 
Do you want to continue [Y/N]? n
OK, goodbye
{% endhighlight %}
本例中将```-n```选项和值 1 一起使用，告诉read命令在接受单个字符后退出。只要你按下单个字符回显后，read命令就会接受输入并将它传给变量，而不必按回车键。

### 6.3 隐藏方式读取
有时你需要读取脚本用户的输入，但不想输入出现在屏幕上。典型的例子是输入的密码，但还有很多其他需要隐藏的数据类型。

```-s```选项会阻止将传给read命令的数据显示在显示器上（实际上，数据会被显示，只是read命令会将文本颜色设成跟背景色一样）。 这里有个在脚本中使用```-s```选项的例子：
{% highlight string %}
# cat ./test27 
#!/bin/bash

# hiding input data from the monitor

read -s -p "Enter your passwd: " pass

echo 

echo "Is your password really $pass? "

# ./test27 
Enter your passwd: 
Is your password really 123AaBb?
{% endhighlight %}
输入提示符输入的数据不会出现在屏幕上，但会赋给变量，以便在脚本中使用。

### 6.4 从文件中读取
最后，你也可以用read命令来读取Linux系统上文件里保存的数据。每次调用read命令会从文件中读取一行文本。当文件中再没有内容时，read命令会退出并返回非零退出状态码。

其中最难的部分是将文件中的数据传给read命令。最常见的方法是将文件运行cat命令后的输出通过管道直接传给含有read命令的while循环。下面的例子说明怎么处理：
{% highlight string %}
# cat test28
#!/bin/bash

# reading data from a file

count=1

cat test | while read line
do
   echo "Line #$count: $line"
   count=$[$count + 1]
done

echo "Finished processing the file"

# cat test
The quick brown dog jumps over the lazy fox.
This is a test, this is only a test.
O Romeo, Romeo! Wherefore art thou Romeo?
# ./test28
Line #1: The quick brown dog jumps over the lazy fox.
Line #2: This is a test, this is only a test.
Line #3: O Romeo, Romeo! Wherefore art thou Romeo?
Finished processing the file
{% endhighlight %}
while循环会继续通过read命令处理文件中的行，直到read命令以非零退出状态码退出。







<br />
<br />

**[参看]**

1. [linux的shell函数参数](http://www.cnblogs.com/liuwenbohhh/p/4650463.html)




<br />
<br />
<br />


