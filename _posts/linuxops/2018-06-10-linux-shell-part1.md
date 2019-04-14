---
layout: post
title: shell编程之构建基本脚本
tags:
- LinuxOps
categories: linuxOps
description: shell编程之构建基本脚本
---

本章主要讨论编写脚本的基础知识。在开始编写自己的shell脚本大作之前，你必须知道这些基本概念。


<!-- more -->


## 1. 使用多个命令

到目前为止，你已经了解了如何使用shell的命令行界面提示符来输入命令和查看命令的结果了。shell脚本的关键在于输入多个命令并处理每个命令的结果，即使有可能将一个命令的结果传给另一个命令。shell允许你只用一步就将多个命令串联起来使用。

如果要两个命令一起运行，可在同一提示行输入它们，用分号分隔开：
{% highlight string %}
# date; who
Sat Apr 13 05:00:06 PDT 2019
ivan1001 tty7         2019-04-04 23:14 (:0)
root     pts/19       2019-04-12 20:34 (192.168.10.111)
{% endhighlight %}
恭喜你，你刚刚已经写了一个脚本了。这个简单的脚本只用到了两个bash shell命令。date命令先运行，显示了当前日期和时间；后面紧跟着who命令的输入，显示当前是谁登陆到了系统上。使用这种办法，你就能将任意多个命令串连在一起使用了，只要不超过最大命令行字符数255就行。

然而这种技术仅适用于小的脚本。它有一个致命的缺陷，即每次运行之前你都必须在命令提示符下输入整个命令。但不需要手动将这些命令都输入命令行中，你可以将命令合并成一个简单的文本文件。在需要运行这些命令时，你可以简单地运行这个文本文件。


## 2. 创建shell脚本文件

要将shell命令放到一个文本文件中，首先需要用一个文本编辑器来一个文件，然后将命令输入到文件中。

在创建shell脚本文件时，必须在文件的第一行指定要使用的shell。其格式为：
<pre>
#!/bin/bash
</pre>

在通常的shell脚本的行里，井号(#)用作注释行。shell脚本中的注释行是不被shell执行的。然而，shell脚本文件的第一行是个特例，井号后接叹号告诉shell用哪个shell来运行脚本（是的，你可以用bash shell来运行你的脚本程序，也可以用其他的shell）

在指定了shell之后，可以在文件的每行输入命令，后加一个回车符。之前提到过，注释可用井号添加。例如：
{% highlight string %}
#!/bin/bash

# This script displays the date and who's logged on
date
who
{% endhighlight %}
这就是所谓的脚本内容了。如有需要，可以用分号来在一行输入你想要的两个命令。但在shell脚本中，你可以用不同的行来列出命令。shell会根据命令在文件中出现的顺序来处理命令。

还有，要注意另有一行也以井号（#)开头，并添加了一个注释。以井号开头的行都不会被shell处理（除了以```#!```开头的第一行）。在脚本中留下注释来说明脚本做了什么，这种方法非常好，所以两年后会来再看这个脚本时，你还可以人容易记起来你做了什么。

将这个脚本保存在名为test1的文件中，就基本好了。在运行脚本前，还要做其他一些事。

现在运行脚本，如下结果可能会叫你有点失望：
<pre>
# test1
bash: test1: command not found
</pre>
你要跨过的第一个障碍是让bash shell能找到这个脚本文件。我们知道，shell会通过```PATH```环境变量来查找命令。快速的查看一下PATH环境变量就可以指出我们的问题了：
<pre>
# echo $PATH
/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games
</pre>

PATH环境变量被设成只在一组目录中查找命令。要让shell找到test1脚本，我们只需采取如下做法之一：

* 将shell脚本文件所处的目录添加到PATH环境变量中

* 在提示符中用绝对或相对文件路径来引用shell脚本文件

<pre>
提示： 有些Linux发行版将$HOME/bin目录添加进了PATH环境变量。它在每个用户的HOME目录下提供了
一个存放脚本文件的地方，shell可以在那里查找要执行的命令
</pre>

在这个例子中，我们将用第二种方式来告诉shell脚本文件所处的确切位置。记住，要引用当前目录下的文件，你要在shell中使用单点操作符：
<pre>
# ./test1
bash: ./test1: Permission denied
</pre>

现在shell已经可以找到脚本文件了，但还有一个问题。shell表明你还没有执行文件的权限。快速查看文件权限就找到问题所在：
<pre>
# ls -al test1
-rw-r--r-- 1 root root 74 Apr 13 07:59 test1
</pre>
在创建test1文件时，umask的值决定了新文件的默认权限设置。由于umas变量设成了022，系统创建的文件只有文件属主才有读写权限。

下一步是通过chmod命令赋予文件属主执行文件的权限：
<pre>
# chmod u+x ./test1
# ./test1
Sat Apr 13 08:03:10 PDT 2019
ivan1001 tty7         2019-04-04 23:14 (:0)
root     pts/19       2019-04-12 20:34 (192.168.10.111)
</pre>
成功了！ 现在万事具备，能够执行新的shell脚本文件了

## 3. 显示消息
许多shell命令会产生自己的输出，这些输出会显示在脚本所运行的控制台显示器上。然而许多情况下，你可能想要添加自己的文本消息来告诉脚本用户脚本正在做什么。你可以通过```echo```命令来做这个。echo命令能显示一个简单的文本字符串，如果你通过如下命令添加了字符串：
{% highlight string %}
# echo This is test
This is test
{% endhighlight %}

注意，默认情况下，你不需要使用引号将要显示的文本字符串圈起来。但有时在字符串中出现引号的话可能就比较麻烦：
{% highlight string %}
# echo Let's see if this'll work
Lets see if thisll work
{% endhighlight %}
echo命令可以用单引号或双引号将文本字符串圈起来。如果你在字符串中用到了它们，你需要在文本中使用其中一种引号，而用另外一种来将字符串圈起来。
{% highlight string %}
# echo "This is a test to see if you're paying attention"
This is a test to see if you're paying attention
# echo 'Rich says "scripting is easy"'
Rich says "scripting is easy"
{% endhighlight %}
现在所有的引号都正确地在输出中显示了。

你可以将echo语句添加到shell脚本中任何需要显示额外信息的地方：
{% highlight string %}
# cat test1 
#!/bin/bash

# This script displays the date and who's logged on

echo The time and date are:
date

echo "Let's see who's logged into the system: "
who
{% endhighlight %}
当运行这个脚本时，它会产生如下输出：
<pre>
# ./test1
The time and date are:
Sat Apr 13 20:22:57 PDT 2019
Let's see who's logged into the system: 
ivan1001 tty7         2019-04-04 23:14 (:0)
root     pts/19       2019-04-13 20:10 (192.168.10.111)
</pre>
很好，但如果你想在同一行显示一个文本字符串作为命令输出，应该怎么办呢？ 你可以用echo语句的```-n```参数。只要将第一个echo语句改成这样就行：
{% highlight string %}
echo -n "The time and date are:"
{% endhighlight %}

你需要在字符串的两侧使用引号来保证在显示的字符串尾部有一个空格。命令输出将会紧接着字符串结束的地方开始。现在输出会是这个样子：
<pre>
# ./test1 
The time and date are: Sat Apr 13 20:28:28 PDT 2019
Let's see who's logged into the system: 
ivan1001 tty7         2019-04-04 23:14 (:0)
root     pts/19       2019-04-13 20:10 (192.168.10.111)
</pre>

完美！ echo命令是shell脚本中同用户交互的重要工具。你会在很多情况下用到它，尤其是当你要显示脚本中变量的值时。我们下面继续了解这个。

## 4. 使用变量

运行shell脚本中的单个命令很有用，但它有自身的限制。通常你可能会要用shell命令中的其他数据来处理信息。这点可以通过变量来完成。变量允许你临时性的将信息存储在shell脚本中，以便和脚本中的其他命令一起使用。本节将介绍如何在shell脚本中使用变量。

1） **环境变量**

你已经亲自了解了一种Linux变量： 环境变量。你也可以在脚本中访问这些值。

shell维护着一组环境变量，用来记录特定的系统信息。比如系统的名称，登陆到系统上的用户的名称，用户的系统ID（也称为UID），用户的默认主目录以及shell查找程序的搜索路径。你可以用```set```命令来显示一份完整的活动的环境变量列表：
{% highlight string %}
# set
BASH=/bin/bash
BASH_ALIASES=()
BASH_ARGC=()
BASH_ARGV=()
BASH_CMDS=()
BASH_COMPLETION_COMPAT_DIR=/etc/bash_completion.d
BASH_LINENO=()
BASH_REMATCH=()
BASH_SOURCE=()
...
{% endhighlight %}
你可以在环境变量名称之前加个美元符($)来在脚本中使用这些环境变量。下面的脚本中将会演示：
{% highlight string %}
# cat test2
#!/bin/bash

# display user information from the system

echo "User info for userid: $USER"
echo "UID: $UID"
echo "HOME: $HOME"
{% endhighlight %}

$USER、$UID和$HOME环境变量用来显示已登录用户的有关信息。输出看起来应该是这样子的：
<pre>
# ./test2
User info for userid: root
UID: 0
HOME: /root
</pre>
注意，echo命令中的环境变量会在脚本运行时替换成当前值。还有在第一个字符串中我们可以将$USER系统变量放置到双引号中，而shell依然能够知道我们的意图。但采用这种方法也有一个问题，看看下面这个例子会怎样：
{% highlight string %}
# echo "The cost of the item is $15"
The cost of the item is 5
{% endhighlight %}
显然这不是我们想要的。只要脚本在引号中看到美元符，它就会以为你在引用一个变量。在这个例子中，脚本会尝试显示$1(但并未定义），再显示数字5。要显示美元符，你必须在它前面放置一个反斜线：
{% highlight string %}
# echo "The cost of the item is \$15"
The cost of the item is $15
{% endhighlight %}

看起来好多了。反斜线允许shell脚本将美元符解释成为实际的美元符，而不是变量。下一节将会介绍如何在脚本中创建自己的变量。
<pre>
说明： 你可能还见过${variable}形式引用的变量。变量名两侧额外的花括号通常用来帮助识别
美元符后的变量名。
</pre>

2） **用户变量**

除了环境变量，shell脚本还允许在脚本中定义和使用自己的变量。定义变量允许临时存储数据并在整个脚本中使用，从而使shell脚本看起来更像计算机程序。

用户变量可以是任何不超过20个字母、数字或下划线的文本字符串。用户变量区分大小写，所以变量Var1和变量var1是不同的。这个小规矩经常让脚本编程初学者感到头疼。

值通过等号赋值给用户变量。在变量、等号和值之间不能出现空格（另一个困扰初学者的用法）。这里有一些给用户变量赋值的例子：
{% highlight string %}
var1=10
var2=57
var3=testing
var4="still more testing"
{% endhighlight %}
shell脚本会自动决定变量值的数据类型。在脚本的整个生命周期里，shell脚本中定义的变量一直保持着它们的值，但在shell脚本完成时删除掉。

类似于系统变量，用户变量可以通过美元符引用：
{% highlight string %}
# cat test3 
#!/bin/bash

# testing variables

days=10
guest="Katie"
echo "$guest checked in $days days ago"
days=5
guest="Jessica"
echo "$guest checked in $days days ago"
{% endhighlight %}
运行脚本会有如下输出：
<pre>
# ./test3 
Katie checked in 10 days ago
Jessica checked in 5 days ago
</pre>
变量每次被引用时，都会输出当前赋给它的值。重要的是记住，引用一个变量值时需要使用美元符，而引用变量来对其进行赋值时则不要使用美元符。通过一个例子你就能明白我的意思：
{% highlight string %}
# cat test4
#!/bin/bash

# assigning a variable value to another variable

value1=10
value2=$value1

echo "The resulting value is $value2"
{% endhighlight %}
当你在赋值语句中使用value1变量的值时，你仍然必须使用美元符。这段代码产生如下输出：
<pre>
# ./test4 
The resulting value is 10
</pre>
要是你忘了用美元符，使得value2的赋值弄成这样：
{% highlight string %}
value2=value1
{% endhighlight %}
你会得到如下输出：
<pre>
# ./test4
The resulting value is value1
</pre>
没有美元符，shell会将变量名解释成普通的文本字符串，通常这并不是你想要的结果。

3) **反引号**

shell脚本中最有用的特性之一就是```反引号（`)```。注意，这并非是那个你所习惯的用来圈起字符串的普通单引号字符。由于在shell脚本之外很少用到它，你可能甚至都不知道在键盘什么地方能找到它。但你必须慢慢熟悉它，因为这是许多shell脚本中的重要组件。提示：在美式键盘上，它通常和波浪线(~)位于同一键位。

反引号允许你将shell命令的输出赋给变量。尽管这看起来并不那么重要，但它却是脚本编程中的一个主要构件。

你必须用反引号把整个命令行命令圈起来：
<pre>
testing=`date`
</pre>
shell会运行反引号中的命令，并将其输出赋值给变量testing。这里有个通过普通的shell命令输出创建变量的例子：
{% highlight string %}
# cat test5 
#!/bin/bash

# using the backtick character

testing=`date`


echo "The date and time are: " $testing
{% endhighlight %}

变量testing收到了date命令的输出，并在echo语句中用来显示它。运行这个shell脚本生成如下输出：
<pre>
# ./test5 
The date and time are:  Sun Apr 14 03:12:26 PDT 2019
</pre>

这个例子并没什么特别吸引人的地方（你也可以很轻松地将该命令放在echo语句中），但只要将命令的输出放到变量里，你就能用它来干任何事情了。

下面这个例子很常见，它在脚本中通过反引号获得当前日期并用它来生成唯一文件名：
{% highlight string %}
# cat test 
#!/bin/bash

# copy the /usr/bin directory listing to a log file

today=`date +%y%m%d`
ls /usr/bin -al > log.$today
{% endhighlight %}
today变量被赋予格式化后的date命令的输出。这是用来为日志文件名抓取日志信息常用的一种技术。```+%y%m%d```格式告诉date命令将日期显示为两位数的年、月、日的组合：
<pre>
# date +%y%m%d
190414
</pre>
这个脚本将值赋给一个变量，之后再将其作为文件名的一部分。文件自身含有目录列表的重定向输出（我们将在下一节详细讨论）。运行脚本之后，你应该能在目录中看到一个新文件：
<pre>
# ./test 
# ls -al log.190414 
-rw-r--r-- 1 root root 108706 Apr 14 03:21 log.190414
root@ubuntu:~/workspace# 
</pre>
目录中出现的日志文件采用$today变量的值作为文件名的一部分。日志文件的内容是/usr/bin目录的列出输出。如果脚本在后一天运行，日志文件名会是```log.190415```，因此每天创建一个新文件。

## 5. 重定向输入和输出

有些时候你想要保存某个命令的输出而非在显示器上显示它。bash shell提供了一些不同的操作符来讲某个命令的输出重定向到另一个位置（比如文件）。重定向可以通过将某个文件重定向到某个命令上来用在输入上，也可以用在输出上。本节将介绍如何在shell脚本中使用重定向。

1) **输出重定向**

重定向最基本的类型是将命令的输出发到一个文件中。bash shell采用```大于号(>)```来完成这项功能：
{% highlight string %}
command > outputfile
{% endhighlight %}
之前显示器上出现的命令的输出会被保存到指定的输出文件中：
{% highlight string %}
# date > test6
# ls -al test6
-rw-r--r-- 1 root root 29 Apr 14 03:56 test6
# cat test6
Sun Apr 14 03:56:54 PDT 2019
{% endhighlight %}

重定向操作符创建了一个文件test6（通过默认的umask设置）并将date命令的输出重定向到test6文件中。如果输出文件已经存在了，则这个重定向操作符会用新的文件数据覆盖已经存在的文件：
{% highlight string %}
# who > test6
# cat test6
ivan1001 tty7         2019-04-04 23:14 (:0)
root     pts/19       2019-04-13 20:10 (192.168.10.111)
{% endhighlight %}

现在test6文件的内容保存的是who命令的输出。

有时，取代覆盖文件的内容，你可能想要将命令的输出追加到已有文件上，比如你正在创建一个记录系统上某个操作的日志文件。在这种情况下，你可以用```双大于号(>>)```来追加数据：
{% highlight string %}
# date >> test6
# cat test6
ivan1001 tty7         2019-04-04 23:14 (:0)
root     pts/19       2019-04-13 20:10 (192.168.10.111)
Sun Apr 14 05:01:15 PDT 2019
{% endhighlight %}

test6文件仍然包含早些时候who命令处理的数据，加上现在新从date命令获得的输出。

2) **输入重定向**

输入重定向和输出重定向正好相反。输入重定向将文件的内容重定向到命令，而非将命令的输出重定向到文件。

输入重定向符号是```小于号(<)```:
{% highlight string %}
command < inputfile
{% endhighlight %}
记住它的简易办法是在命令行上，命令总是在左侧，而重定向符号“指向”数据流动的方向。小于号说明数据正在从输入文件流向命令。

这里有个和```wc```命令一起使用输入重定向的例子：
{% highlight string %}
# cat test6
ivan1001 tty7         2019-04-04 23:14 (:0)
root     pts/19       2019-04-13 20:10 (192.168.10.111)
Sun Apr 14 05:01:15 PDT 2019
# wc < test6
  3  16 129
{% endhighlight %}
```wc```命令提供了对数据中文本的计数。默认情况下，它会输出3个值：

* 文本中的行数；

* 文本中的词数；

* 文本的字节数

通过将文本文件重定向到wc命令，你可以得到对文件中行、词和字节的快速计数。这个例子说明test6文件有3行、16个单词以及129字节。

还有另外一种输入重定向的方法，称为**内联输入重定向**(inline input redirection)。这种方法允许你在命令行而不是在文件指定输入重定向的数据。乍看一眼，则可能有点奇怪，但有些应用会用到这个过程。

内联输入重定向符号是```双小于号(<<)```。除了这个符号，你必须指定一个文本标记来划分要输入数据的开始和结尾。你可以用任何字符串的值来作为文本标记，但在数据的开始和结尾必须一致：
{% highlight string %}
command << marker
data
marker
{% endhighlight %}
在命令行上使用内联输入重定向时，shell会用PS2环境变量中定义的次提示符来提示输入数据。下面是使用它的情况：
{% highlight string %}
# wc << EOF
> test string 1
> test string 2
> test string 3
> EOF
 3  9 42
{% endhighlight %}
次提示符一直提示输入更多数据，直到你输入了作为文本标记的那个字符串值。wc命令会对内敛输入重定向提供的数据执行行、词和字节计数。

## 6. 管道
有时你需要发送某个命令的输出作为另一个命令的输入。可以用重定向，只是有些笨拙：
{% highlight string %}
# rpm -qa >> rpm.list
# sort < rpm.list
a11y-profile-manager-indicator/xenial,now 0.1.10-0ubuntu3 i386 [installed]
account-plugin-facebook/xenial,now 0.12+16.04.20160126-0ubuntu1 all [installed]
account-plugin-flickr/xenial,now 0.12+16.04.20160126-0ubuntu1 all [installed]
account-plugin-google/xenial,now 0.12+16.04.20160126-0ubuntu1 all [installed]
accountsservice/xenial-updates,now 0.6.40-2ubuntu11.3 i386 [installed]
acl/xenial,now 2.2.52-3 i386 [installed]
acpid/xenial,now 1:2.0.26-1ubuntu2 i386 [installed]
acpi-support/xenial,now 0.142 i386 [installed]
activity-log-manager/xenial,now 0.9.7-0ubuntu23 i386 [installed,upgradable to: 0.9.7-0ubuntu23.16.04.1]
{% endhighlight %}

rpm命令管理着通过Red Hat包管理系统(RPM)安装到系统上的软件包，比如上面列出的Fedora系统。在和```-qa```参数一起使用时，它会生成已安装包的列表，但并不会遵循某种特定的顺序。如果你要在查找某个特定的包或一组包，可能会比较难在rpm命令的输出中找到它。





<br />
<br />

**[参看]**






<br />
<br />
<br />


