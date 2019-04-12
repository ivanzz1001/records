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

有时你需要检查脚本代码中的多种条件。不用写多个分立的if-then语句，你可以用else部分的替代版本，称作elif。

elif会通过另一个if-then语句来延续else部分：
{% highlight string %}
if command1
then
   commands
elif command2
then
   more commands
fi
{% endhighlight %}
elif语句行提供了另一个要测试的命令，类似于原始的if语句。如果elif后命令的退出状态码是0，则bash会执行第二个then语句部分的命令。

你可以继续将多个elif语句串起来，形成一个大的if-then-elif嵌套组合：
{% highlight string %}
if command1
then
    command set 1
elif command2
then
    command set 2
elif command3
then
    command set 3
elif command4
then
    command set 4
fi
{% endhighlight %}
每块命令都会根据哪个命令会返回退出状态码0来执行。记住，bash shell会依次执行if语句，只有第一个返回退出状态码0的语句中的then部分会被执行。我们后面第7节，你将会了解如何用case命令而不用嵌套多个if-then语句。

## 4. test命令
到目前为止，你所了解到的if语句中的命令都是普通shell命令。你可能想问，if-then语句是否能测试跟命令的退出码无关的条件。

答案是不能。但是，在bash shell中有个好用的工具可以帮你通过if-then语句测试其他条件。

test命令提供了if-then语句中测试不同条件的途径。如果test命令中列出的条件成立，test命令就会退出并返回状态码0，这样if-then语句就与其他编程语言中的if-then语句以类似的方式工作了。如果条件不成立，test命令就会退出并返回退出状态码1，这样if-then语句就会失效。

test命令的格式非常简单：
{% highlight string %}
test condition
{% endhighlight %}

condition是test命令要测试的一系列参数和值。当用在if-then中时，test命令看起来是这样的：
{% highlight string %}
if test condition
then
   commands
fi
{% endhighlight %}
bash shell提供了另一种在if-then语句中声明test命令的方法：
{% highlight string %}
if [ condition ]
then
   commands
fi
{% endhighlight %}
方括号定义了test命令中用到的条件。注意，你必须在左括号右侧和右括号左侧各加一个空格，否则会报错。

test命令可以判断3类条件：

* 数值比较

* 字符串比较

* 文件比较

在后面我们将会介绍如何在if-then语句中使用这3类条件测试。


### 4.1 数值比较

使用test命令最常见的情形是对两个数值进行比较。下表列出了测试两个值时可用的条件参数：
<pre>
              表： test命令的数值比较功能

  比 较                        描述
----------------------------------------------------------------------------------
n1 -eq n2             检查n1是否与n2相等

n1 -ge n2             检查n1是否大于或等于n2

n1 -gt n2             检查n1是否大于n2

n1 -le n2             检查n1是否小于或等于n2

n1 -lt n2             检查n1是否小于n2

n1 -ne n2             检查n1是否不等于n2
</pre>
数值条件测试可以用在数字和变量上。这里有个例子：
{% highlight string %}
# cat test5
#!/bin/bash

# using numeric test comparisons
val1=10
val2=11

if [ $val1 -gt 5 ]
then
   echo "The test value '$val1' is greater then 5"
fi

if [ $val1 -eq $val2 ]
then
   echo "the values are equal"
else
   echo "the values are different"
fi
{% endhighlight %}

第一个条件测试：
<pre>
if [ $val1 -gt 5 ]
</pre>
会测试变量val1的值是否大于5。第二个条件测试：
<pre>
if [ $val1 -eq $val2 ]
</pre>
会测试变量val1的值是否和变量val2的值相等。运行脚本并观察结果：
<pre>
# ./test5
The test value '10' is greater then 5
the values are different
</pre>

两个数值条件测试都跟预期的一样执行了。

但是测试数值条件也有个限制。先下面的脚本：
{% highlight string %}
# cat test6
#!/bin/bash

# testing floating point numbers
val1=`echo "scale=4; 10/3" | bc`

echo "The test value is '$val1'"

if [ $val1 -gt 3 ]
then
{% endhighlight %}
运行上述脚本：
<pre>
# ./test6
The test value is '3.3333'
./test6: line 8: [: 3.3333: integer expression expected
</pre>
这个例子使用了bash计算器来生成一个浮点值并存储在变量val1中。下一步，它使用了test命令来判断这个值。显然这里出错了。

在前面一章<<构建基本脚本>>中，你已经了解了如何在bash shell中处理浮点数值；在脚本中仍然有一个问题。test命令无法处理val1变量中存储的浮点值。

记住，bash shell能处理的数仅有整数。当你使用bash计算器时，你可以让shell将浮点值作为字符串值存储进一个变量。如果你只是要通过echo语句来显示这个结果，那它可以很好地工作；但它无法在基于数字的函数中工作，例如我们的数值测试条件。尾行恰好说明了你不能在test命令中使用浮点值。

而如果我们需要进行浮点值比较呢？ 其实我们也还是有办法的，请参看下面程序：
{% highlight string %}
# cat test7
#!/bin/bash

# testing floating point numbers
val1=`echo "scale=4; 10/3" | bc`

echo "The test value is '$val1'"

if [ $val1 -gt 3 ]
then
   echo "the result is larger than 3"
fi


if [ `echo "$val1 >= 3" | bc` -eq 1 ]
then
    echo "the result is larger then 3(version 2)"
fi
{% endhighlight %}
运行上述脚本：
<pre>
# ./test7
The test value is '3.3333'
./test6: line 8: [: 3.3333: integer expression expected
the result is larger then 3(version 2)
</pre>
上面我们看到采用version 2版本是可以顺利的进行比较的。

### 4.2 字符串比较

test命令还允许比较字符串值。对字符串进行比较会更繁琐些，你马上就会看到。下表列出了可用来比较两个字符串值的函数：
{% highlight string %}
             表： test命令的字符串比较功能

  比 较                      描述
-------------------------------------------------------------------------------
str1 = str2           检查str1是否和str2相同

str1 != str2          检查str1是否和str2不同

str1 < str2           检查str1是否比str2小

str1 > str2           检查str1是否比str2大

-n str1               检查str1的长度是否非0

-z str1               检查str1的长度是否为0
{% endhighlight %}
下面几节将会详细介绍不同的字符串比较功能。

1) **字符串相等性**

字符串的相等和不等条件不言自明，很容易看出两个字符串值相同还是不同：
{% highlight string %}
# cat test7 
#!/bin/bash

# testing string equality
testuser=root

if [ $USER = $testuser ]
then
   echo "Welcome $testuser"
fi

# ./test7 
Welcome root
{% endhighlight %}
不等字符串条件也允许你判断两个字符串是否具有相同的值：
{% highlight string %}
# cat test8 
#!/bin/bash

# testing string equality
testuser=baduser

if [ $USER != $testuser ]
then
   echo "This is not $testuser"
else
   echo "This is $testuser"
fi

# ./test8
This is not baduser
{% endhighlight %}
比较字符串的相等性时，test的比较会将所有的标点和大小写也考虑在内。

2) **字符串顺序**

要测试一个字符串是否比另一个字符串大就开始变得繁琐了。有两个问题会经常困扰正要开始使用test命令的大于小于特性的shell程序员：

* 大于符号必须转义，否则shell会把它们当做重定向符号而把字符串当做文件名；

* 大于小于顺序和sort命令所采用的不同

在编写脚本时，第一条可能会导致一个不易察觉的严重问题。这里有个shell脚本编程初学时常遇到的例子：
{% highlight string %}
# cat badtest 
#!/bin/bash

# mis-using string comparisions

val1=baseball
val2=hockey

if [ $val1 > $val2 ]
then
   echo "'$val1' is greater then '$val2'"
else
   echo "'$val1' is less then '$val2'"
fi

# ./badtest 
'baseball' is greater then 'hockey'
# ls -l
total 4
-rwxrwxrwx 1 root root 189 Apr 12 02:47 badtest
-rw-r--r-- 1 root root   0 Apr 12 02:47 hockey
{% endhighlight %}
在这个脚本中只用了大于号，没有出现错误， 但结果是错的。脚本把大于号解释成了输出重定向。因此，它创建了一个名为hockey的文件。由于重定向顺利完成了，test命令返回了退出状态码0，而if语句则以为所有命令都成功结束了。

要解决这个问题，你需要正确的转义大于号：
{% highlight string %}
# cat test9
#!/bin/bash

# right-using string comparisions

val1=baseball
val2=hockey

if [ $val1 \> $val2 ]
then
   echo "'$val1' is greater then '$val2'"
else
   echo "'$val1' is less then '$val2'"
fi 


# ./test9
'baseball' is less then 'hockey'
{% endhighlight %}










<br />
<br />

**[参看]**






<br />
<br />
<br />


