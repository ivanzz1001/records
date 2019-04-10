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
在这个例子中，我们在if语句行故意放了一个不能工作的命令。由于这个命令没法工作，它会输出一个非零0的状态码，bash shell会跳过then部分的echo语句。还要注意，运行if语句中的那个命令所生成的错误消息依然会显示在脚本的输出中。有时你不想这种情况发生，我们后面会讨论如何避免这种情况。

在then部分，你可以用多个命令。你可以像脚本中其他地方一样列出多条命令。bash shell会将这部分命令当成一个块。在if语句行的那个命令返回退出状态码0时执行这块命令，在该命令返回非零退出状态码时跳过这块命令：
{% highlight string %}
# cat test3
#!/bin/bash

# testing multiple commands in the then section

testuser=ivan1001
if grep $testuser /etc/passwd
then
   echo "the bash files for user $testuser are:"
   ls -a /home/$testuser/.b*
fi
{% endhighlight %}
if语句行使用了grep命令来在/etc/passwd文件中查找某个特定用户名是否在当前系统上使用。如果有用户使用了那个登录名，脚本会显示一些文本并列出该用户主目录(HOME)的bash文件：
{% highlight string %}
# ./test3
ivan1001:x:1000:1000:Ubuntu16.04,,,:/home/ivan1001:/bin/bash
the bash files for user ivan1001 are:
/home/ivan1001/.bash_history  /home/ivan1001/.bash_logout  /home/ivan1001/.bashrc
{% endhighlight %}
但是，如果你将testuser变量设置成一个系统上不存在的用户，则什么都不会显示：
{% highlight string %}
# ./test3
{% endhighlight %}
看起来也没什么新鲜的。可能我们在这里显示一些消息说明这个用户在系统中未找到会稍微友好一些。是的，我们可以做到，用if-then语句的另外一个特性。

<pre>
说明：你可能会在一些脚本中看到if-then的另一种形式
if command;then
   commands
fi

在要执行的命令结尾加个分号，你就能在同一行使用then语句了，这样看起来就更像其他编程语言中的if-then语句
是如何处理的了。
</pre>

## 2. if-then-else语句
在if-then语句中，不管命令是否成功执行，你都只有一种选择。如果命令返回一个非零退出状态码，bash shell仅会继续执行脚本中的下一条命令。在这种情况下，如果能够执行另一组命令就好了。这正是if-then-else语句的作用。

if-then-else在语句中提供了另外一组命令：
{% highlight string %}
if command
then
    commands
else
    commands
fi
{% endhighlight %}
当if语句中的命令返回退出状态码0时，then部分中的命令会被执行，跟普通的if-then语句一样。当if语句中的命令返回非零退出状态码时，bash shell会执行else部分中的命令。

现在你可以参考如下修改测试脚本：
{% highlight string %}
# cat test4
#!/bin/bash

#testing the else section

testuser=badtest
if grep $testuser /etc/passwd
then
   echo "The files for user '$testuser' are:"
   ls -a /home/$testuser/.b*
else
   echo "the user name '$testuser' does not exist on this system"
fi

# ./test4
the user name 'badtest' does not exist on this system
{% endhighlight %}
这样就更友好了。跟then部分一样，else部分可以包含多条命令。fi语句说明else部分结束了。

## 3. 嵌套if






<br />
<br />

**[参看]**






<br />
<br />
<br />


