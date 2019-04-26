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



<br />
<br />

**[参看]**






<br />
<br />
<br />


