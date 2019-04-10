---
layout: post
title: shell编程之结构化命令
tags:
- LinuxOps
categories: linuxOps
description: shell编程之结构化命令
---


在前一章<<shell编程之构建基本脚本>>中给出的那些shell脚本，shell按照出现的次序来处理shell脚本中的每个单独命令。对于顺序操作来说这已经足够了，如果你只想所有的命令都能按照正确的顺序执行。然而，并非所有程序都如此操作。


许多程序要求在shell脚本中的命令间有一些逻辑流控制。这意味着shell在某些环境下执行特定的命令，但在其他某些环境下执行另外一些命令。有一类命令会基于变量值或其他命令的结果等条件使脚本跳过或循环执行命令。这样的命令通常称为结构化命令(structured command)。

结构化命令允许你改变程序执行的顺序，在某些条件下执行一些命令而在其他条件下跳过另一些命令。在bash shell中有不少结构化命令，我们会逐个研究它们。在本章，我们来看一下if-then语句。


<!-- more -->

## 1. 使用if-then语句
结构化命令中，最基本的类型就是if-then语句。if-then语句有如下格式：
{% highlight string %}
if command
then
   commands
fi
{% endhighlight %}
如果你在用其他编程语言的if-then语句，这种形式可能会让你有点困惑。在其他编程语言中，if语句之后的对象是一个等式来测试```TRUE```还是```FALSE```值。bash shell的if语句并非如此工作。

bash shell的if语句会运行if行定义的那个命令。如果该命令的退出状态码（参见前一章<<构建基本脚本>>)是0（该命令运行成功），位于then部分的命令就会被执行。如果该命令的退出状态码是其他什么值，那then部分的命令就不会被执行，bash shell会继续执行脚本中的下一条命令。
{% highlight string %}
# cat test1
#!/bin/bash

# testing the if statement
if date
then
   echo "it worked"
fi
{% endhighlight %}

这个脚本在if行采用了date命令。如果命令成功结束了，echo语句就会显示该文本字符串。当你在命令行运行该脚本时，你会得到如下结果：
<pre>
# ./test1
Wed Apr 10 02:38:38 PDT 2019
it worked
</pre>
shell执行了了if行的date命令。由于退出状态码是0，它就又执行了then部分的echo语句。注意，如果我们将上述代码在命令行上写成一行来执行的话，其格式应该如下：
<pre>
# if date; then echo "It worked"; fi
Wed Apr 10 02:58:21 PDT 2019
It worked
</pre>

下面是另外一个例子：
{% highlight string %}
# cat test2
#!/bin/bash

# testing a bad command
if asdfg
then
  echo "it did not work"
fi

echo "we are outside of the if statement"

# ./test2
./test2: line 4: asdfg: command not found
we are outside of the if statement
{% endhighlight %}
在这个例子中，我们在if语句行故意放了一个不能工作的命令。





<br />
<br />

**[参看]**






<br />
<br />
<br />


