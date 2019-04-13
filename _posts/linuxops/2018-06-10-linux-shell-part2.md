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
现在那个答案已经是我们期望通过字符串比较得到的了。

第二个问题更细微，除非你经常处理大小写字母，否则几乎遇不到。sort命令处理大写字母的方法刚好跟test命令的相反。让我们在脚本中测试一下这个特性：
{% highlight string %}
# cat test10 
#!/bin/bash

# testing string sort order

val1=Testing
val2=testing

if [ $val1 \> $val2 ]
then
   echo "'$val1' is greater than '$val2'"
else
   echo "'$val1' is less than '$val2'"
fi

# ./test10 
'Testing' is less than 'testing'
# sort testfile
testing
Testing
{% endhighlight %}
在test命令中大写字母会被当成小于小写字母的。但当你将同样的字符串放进文件中并用sort命令排序时，小写字母会先出现。这是由各个命令使用的排序技术不同造成的。test命令会使用标准的ASCII顺序，根据每个字符的ASCII数值来决定排序顺序。sort命令使用的是系统的本地化语言设置中定义的排序顺序。对于英语，本地化设置指定了在排序顺序中小写字母出现在大写字母前。
<pre>
警告： 注意，test命令使用标准的数学比较符号来表示字符串比较， 而用文本代码来表示数值比较。这个细微的特性被很多程序员理解反了。
      如果你为数值使用了数学运算符号，shell会将它们当成字符串值，可能无法产生正确结果。
</pre>

3） **字符串大小**

```-n```和```-z```参数用来检查一个变量是否含有数据：
{% highlight string %}
# cat test11 
#!/bin/bash

# testing string length

val1=testing
val2=''

if [ -n "$val1" ]
then
   echo "The string '$val1' is not empty"
else
   echo "The string '$val1' is empty"
fi

if [ -z "$val2" ]
then
  echo "The string '$val2' is empty"
else
  echo "The string '$val2' is not empty"
fi

if [ -z "$val3" ]
then 
   echo "The string '$val3' is empty"
else
   echo "The string '$val3' is not empty"
fi

# ./test11 
The string 'testing' is not empty
The string '' is empty
The string '' is empty
{% endhighlight %}
这个例子创建了两个字符串变量。val1变量包含了一个字符串， val2变量则以空字符串创建。后续比较如下：
<pre>
if [ -n "$val1" ]
</pre>
判断val1变量是否长度非零； 而它的长度正好为非零，所以then部分被执行了；
<pre>
if [ -z "$val2" ]
</pre>
判断val2变量长度是否为零； 而它的长度正好为零，所以then部分被执行了；
<pre>
if [ -z "$val3" ]
</pre>
判断val3变量长度是否为零。这个变量并未在shell脚本中定义过，所以它说明字符串长度仍然为零，尽管它并未被定义过。

<pre>
警告： 空的和未初始化的变量对shell脚本测试来说可能有灾难性的影响。如果你不是很确定一个变量的内容，最好在数值或字符串
      比较中使用它之前先通过-n或-z来测试一下变量是否含有值。
</pre>

3) **文件比较**

最后一类测试比较可能是shell编程中最强大的也是最经常用到的比较。test命令允许你测试Linux文件系统上文件和目录的状态。下表列出了这些比较：
<pre>
                      表： test命令的文件比较功能

  比 较                         描述
--------------------------------------------------------------------------------------
 -d file                检查file是否存在并是一个目录

 -e file                检查file是否存在

 -f file                检查file是否存在并是一个文件

 -r file                检查file是否存在并可读

 -s file                检查file是否存在并非空

 -w file                检查file是否存在并可写

 -x file                检查file是否存在并可执行

 -O file                检查file是否存在并属当前用户所有

 -G file                检查file是否存在并且默认组与当前用户相同

file1 -nt file2         检查file1是否比file2新

file1 -ot file2         检查file1是否比file2旧 
</pre>
这些条件使你能够在shell脚本中检查文件系统中的文件，并且经常用在要访问文件的脚本中。鉴于它们被如此广泛地使用，我们来逐个看看：

* 检查目录

```-d```测试会检查指定的文件名是否在系统上以目录形式存在。当写文件到某个目录之前，或者是将文件放置到某个目录位置之前时，这会非常有用：
{% highlight string %}
# cat test11
#!/bin/bash

# look before you leap

if [ -d $HOME ]
then 
  echo "Your '$HOME' directory exists"
  cd $HOME
  ls -a
else
  echo "There is a problem with your HOME directory"
fi

# ./test11
Your '/root' directory exists
.   anaconda-ks.cfg  .bash_logout   .bashrc  .config  .dbus                 original-ks.cfg  test11     .xauth3YSsaJ
..  .bash_history    .bash_profile  .cache   .cshrc   initial-setup-ks.cfg  .tcshrc          workspace  .xauth9JwBId
{% endhighlight %}

示例代码中使用了```-d```测试条件来检查用户的$HOME目录是否存在。如果它存在的话，它将继续使用cd命令来切换到$HOME目录并进行目录列表。

2) **检查对象是否存在**

```-e```比较允许你在脚本中使用对象前检查文件或目录对象是否存在：
{% highlight string %}
# cat test12
#!/bin/bash

# checking if a directory exists

if [ -e $HOME ]
then
   echo "OK on the directory, now to check the file"

   #checking if a file exists
   if [ -e $HOME/testing ]
   then
      
     #the file exists, append data to it
     echo "Appending data to existing file"
     date >> $HOME/testing
   else
   
     #the file does not exist. create a new file
     echo "Creating new file"
     date > $HOME/testing
   fi

else
  echo "Sorry. you do not have a HOME directory"
fi


# ./test12
OK on the directory, now to check the file
Creating new file
# ./test12
OK on the directory, now to check the file
Appending data to existing file
{% endhighlight %}

第一个检查用```-e```比较来判断用户是否有$HOME目录。如果有，下一个```-e```比较会检查并判断testing文件是否存在于$HOME目录中。如果文件不存在，shell文件会用单个大于号（输出重定向符号）来用date命令的输出创建一个新文件。你第二次运行这个shell脚本时，它会使用双大于号，这样它就能将date的输出追加到已经存在的文件后面。

3) **检查文件**

```-e```比较适用于文件和目录。要确定指定的对象是个文件，你必须使用```-f```比较：
{% highlight string %}
# cat test13 
#!/bin/bash

# check if a file
if [ -e $HOME ]
then
   echo "the object exists, is it a file?"

   if [ -f $HOME ]
   then
      echo "Yes, it is a file"
   else
      echo "No, It is not a file"

      if [ -f $HOME/.bash_history ]
      then
        echo "But this is a file"
      fi
   fi
else
   echo "Sorry,the object does not exist"

fi

# ./test13
the object exists, is it a file?
No, It is not a file
But this is a file
{% endhighlight %}
这一小段脚本作了很多检查。首先，它用```-e```比较测试$HOME是否存在。如果存在，继续用```-f```选项测试它是不是一个文件。如果它不是文件（当然不会是文件了），我们用```-f```比较来测试$HOME/.bash_history是不是一个文件（当然，是个文件）。

4) **检查是否可读**

在尝试从文件中读取数据之前，最好先测试一下是否能读文件。你可以通过```-r```比较测试：
{% highlight string %}
# cat test14 
#!/bin/bash

# testing if you can read a file

pwfile=/etc/shadow

# first,test if the file exists, and is a file
if [ -f $pwfile ]
then
   #now test if you can read it
    
   if [ -r $pwfile ]
   then
      tail $pwfile
   else
      echo "Sorry, I am unable to read the $pwfile file"
   fi
else
   echo "Sorry, the file $pwfile does not exist"

fi


# ./test14
Sorry, I am unable to read the /etc/shadow file
{% endhighlight %}
/etc/shadow文件含有系统用户加密后的密码。所以它对系统上的普通用户是不可读的。```-r```比较判断出我没有这个文件的读权限，所以test命令失败了。而且bash shell执行了if-then语句的else部分。

5） **检查空文件**

你应该使用```-s```比较来检查文件是否为空，尤其是在你要删除文件时。当```-s```比较成功时要特别小心，它说明文件中有数据：
{% highlight string %}
# cat test15 
#!/bin/bash

# testing if a file is empty

file=t15test

touch $file

if [ -s $file ]
then
   echo "The $file file exists and has no data in it"
else
   echo "The $file exists and is empty"
fi

date > $file
if [ -s $file ]
then
   echo "The $file file has data in it"
else
   echo "The $file is still empty"
fi


# ./test15 
The t15test exists and is empty
The t15test file has data in it
{% endhighlight %}
touch命令创建了这个文件但不会写入任何数据。在我们使用date命令并将其输出重定向到文件中后,```-s```比较说明文件中有数据。

6) **检查文件是否可写**

```-w```比较会判断你是否对文件有可写权限：
{% highlight string %}
# cat test16 
#!/bin/bash

# checking if a file is writable

logfile=$HOME/t16test
touch $logfile

chmod u-w $logfile

now=`date +%Y%m%d-%H%M`

if [ -w $logfile ]
then
   echo "The program run at: $now" > $logfile
   echo "The first attemt succeeded"
else
   echo "The first attempt failed"
fi

chmod u+w $logfile
if [ -w $logfile ]
then
   echo "The program run at:$now" > $logfile
   echo "The second attempt succeeded"
else
   echo "The second attempt failed"
fi


# ./test16      //这里以普通用户身份执行
The first attempt failed
The second attempt succeeded
# cat /home/ivan1001/t16test 
The program run at:20190412-2047
{% endhighlight %}
这个脚本内容很多。首先，它在你的$HOME目录定义了一个日志文件，将文件名存进变量logfile中，创建文件并通过chmod命令移除该用户对文件的写权限。下一步，它创建了变量now并通过date命令保存了一个时间戳。在这之后，它会检查你是否对新日志文件有些权限（你刚刚移除了写权限）。由于现在你没有了写权限，你应该会看到那条未成功消息。

之后，脚本又通过chmod命令重新赋予该用户写权限，并再次尝试写文件。这次写入成功了。

7） **检查是否可执行**

```-x```比较是一个简单的判断你对某个文件是否有执行权限的方法。虽然可能大多数命令用不到它，但如果你要在shell脚本中运行大量脚本，它可能很方便(以普通用户身份创建并运行test16以及test17)：
{% highlight string %}
# cat test17
#!/bin/bash

# testing file execution

if [ -x test16 ]
then
   echo "you can run the script"
   ./test16
else
   echo "Sorry, you are unable to execute the script"
fi


# ./test17
you can run the script
The first attempt failed
The second attempt succeeded

# chmod u-x test16
# ./test17
Sorry, you are unable to execute the script
{% endhighlight %}
这段示例shell脚本用```-x```比较来测试是否有写权限执行test16脚本。如果有写权限，它会运行这个脚本（注意，即使是在shell脚本中，你必须用正确的路径来执行不在你的$PATH路径中的脚本）。在首次成功运行test16脚本后，更改文件的权限并再试。这次， ```-x```比较失败了，因为你没有test16脚本的执行权限。

8) **检查所属关系**

```-O```比较允许你轻松地测试你是否是文件的属主：
{% highlight string %}
# cat test18 
#!/bin/bash

# check file ownership


if [ -O /etc/passwd ]
then
   echo "You are the owner of the /etc/passwd file"
else
   echo "Sorry, you are not the owner of the /etc/passwd file"
fi

# ./test18 
Sorry, you are not the owner of the /etc/passwd file
# su
Password:
# ./test18
You are the owner of the /etc/passwd file
{% endhighlight %}
这段脚本用```-O```比较来测试运行该脚本的用户是否是/etc/passwd文件的属主。第一次，这个脚本运行在普通用户账户下，所以测试失败了。第二次，我们用su命令切换到root用户，测试通过了。


9） **检查默认属组关系**

```-G```比较会检查文件的默认组，如果它匹配了用户的默认组，那就通过了。由于```-G```比较只会检查默认组而非用户所属的所有组，这会叫人有点困惑。这里有个例子：
{% highlight string %}
# cat test19 
#!/bin/bash

# check file group test

if [ -G $HOME/testing ]
then
   echo "You are in the same group as the file"
else
   echo "The file is not owned by your group"
fi


# ls $HOME/testing -al
-rw-rw-r-- 1 ivan1001 ivan1001 0 Apr 12 22:32 /home/ivan1001/testing
# ./test19
You are in the same group as the file
# sudo chgrp sambashare $HOME/testing
[sudo] password for ivan1001: 
# ./test19
The file is not owned by your group
{% endhighlight %}
第一次运行该脚本时，$HOME/testing文件是在ivan1001组的，所以```-G```比较通过了。下一次，组被改成了nogroup，用户也是其中一员，但```-G```比较失败了，因为它只比较默认组，不会去比较其他额外的组。

<pre>
//我们可以通过如下命令来查看所有的组：
# cat /etc/group
</pre>

10) **检查文件日期**


最后一组方法用来进行两个文件创建日期相关的比较。这在编写安装软件的脚本时非常有用。有时你不会想安装一个比系统上已安装文件还要早的文件。

```-nt```比较会判定某个文件是否比另一个文件更新。如果文件更新，那它会有一个比较近的文件创建日期。```-ot```比较会判定某个文件是否比另一个文件更老。如果文件更老，它会有一个更早的创建日期。
{% highlight string %}
# cat test20
#!/bin/bash

# testing file dates

if [ ./test19 -nt ./test18 ]
then
   echo "The test19 file is newer than test18"
else
   echo "The test18 file is newer than test19"
fi

if [ ./test17 -ot ./test19 ]
then
   echo "The test17 file is older than test19"
fi

# ./test20
The test19 file is newer than test18
The test17 file is older than test19
{% endhighlight %}

比较中用到的文件路径是相对于你运行该脚本的目录来说的。如果你要检查的文件是可以被移来移去的，这可能会造成一些问题。另一个问题是，这些比较中没有哪个会先检查文件是否存在。试试下面这个测试：
{% highlight string %}
# cat test21
#!/bin/bash

# testing file dates

if [ ./badfile1 -nt ./badfile2 ]
then
   echo "The badfile1 is newer than badfile2"
else
   echo "The badfile2 is newer than badfile1"
fi


# ./test20 
The badfile2 is newer than badfile1
{% endhighlight %}
这个小例子演示了如果文件不存在，```-nt```比较会返回一个无效的条件。在你尝试在```-nt```或```-ot```比较中使用文件之前，必须先确认文件存在。

## 5. 复合条件测试

if-then语句允许你使用布尔逻辑来组合测试。有两种布尔运算符可用：

* [ condition1 ] && [ condition2 ]

* [ condition1 ] || [ condition2 ]

第一个布尔运算使用AND布尔运算符来组合两个条件。要让then部分的命令执行，两个条件都必须满足。

第二个布尔运算使用OR布尔运算符来组合两个条件。如果任何一个条件最后都能得到一个真值，then部分的命令就会执行。

{% highlight string %}
# cat test22 
#!/bin/bash

# testing compound comparisons

if [ -d $HOME ] && [ -w $HOME/testing ]
then
   echo "The file exists and you can write to it"
else
   echo "I cannot write to the file"
fi

# ./test22 
I cannot write to the file
# touch $HOME/testing
# ./test22 
The file exists and you can write to it
{% endhighlight %}
使用AND布尔运算符时，两个比较都必须满足。第一个比较会检查用户的$HOME目录是否存在。第二个比较会检查在用户的$HOME目录是否有个叫testing的文件，以及用户是否有该文件的写权限。如果两个比较中的任意一个失败了，if语句就会失败，shell就会执行else部分的命令。如果两个比较都通过了，if语句就通过了，shell就会执行then部分的命令。

## 6. if-then的高级特性

bash shell有两项较新的扩展，提供了可在if-then语句中使用的高级特性：

* 用于数学表达式的双尖括号

* 用于高级字符串处理功能的双方括号；

后面几部分将会进一步描述这些特性中的每一项。

1) **使用双尖括号**

双尖括号命令允许你将高级数学表达式放入比较中。test命令只允许在比较中进行简单的算术操作。双尖括号命令提供了更多的为用过其他编程语言的程序员所熟悉的数学符号。双尖括号命令的格式如下：
<pre>
(( expression ))
</pre>

术语expression可以是任意的数学赋值或比较表达式。除了test命令使用的标准数学运算符，下表列出了双尖括号命令中会用到的其他运算符：
{% highlight string %}
             表： 双尖括号命令符号

  符 号                          描述
-------------------------------------------------------------
  val++                        后增
  val--                        后减
  ++val                        先增
  --val                        先减
  !                            逻辑求反
  ~                            位求反
  **                           幂运算
  <<                           左位移
  >>                           右位移
  &                            按位与
  |                            按位或
  &&                           逻辑与
  ||                           逻辑或
{% endhighlight %}
你可以在if语句中用双尖括号命令，也可以在脚本中的普通命令里使用来赋值：

{% highlight string %}
# cat test23 
#!/bin/bash

# using double parenthesis

val1=10

if (( $val1 ** 2 > 90 ))
then
   (( val2 = $val1 ** 2 ))
   echo "the square of $val1 is $val2"
fi

# ./test23 
the square of 10 is 100
{% endhighlight %}

注意，你不需要将双尖括号中表达式里的大于号转义。这是尖括号命令提供的另一个高级特性。

2) **使用双方括号**


双方括号命令提供了针对字符串比较的高级特性。双方括号命令的格式如下：
<pre>
[[ expression ]]
</pre>
双方括号里的expression使用了test命令中采用的标准字符串进行比较。但它提供了test命令未提供的另一个特性————模式匹配（pattern matching).

在模式匹配中，你可以定义一个正则表达式来匹配字符串值：
{% highlight string %}
# cat test24 
#!/bin/bash

# using pattern matching

if [[ $USER == r* ]]
then
   echo "Hello $USER"
else
   echo "Sorry, I do not know you"
fi

# ./test24 
Hello root
{% endhighlight %}
双方括号命令匹配了$USER环境变量来看它是否以字母```r```开头。如果是的话，比较就会通过，shell会执行then部分的命令。

## 7. case命令

你经常会发现自己在尝试计算一个变量的值或在一组可能的值中寻找特定值。在这种情形下，你最终必须写出很长的if-then-else语句，像这样：
{% highlight string %}
# cat test25 
#!/bin/bash

# looking for a possible value

if [ $USER = "root" ]
then
   echo "Welcome $USER"
   echo "Please enjoy your visit"
elif [ $USER = "barbara" ]
then
   echo "Welcome $USER"
   echo "Please enjoy your visit"
elif [ $USER = "testing" ]
then
   echo "Special testing account"
elif [ $USER = "jessica" ]
then
   echo "Do not forget to logout when you're done"
else
   echo "Sorry, you are not allowed here"
fi
   
# ./test25 
Welcome root
Please enjoy your visit
{% endhighlight %}
elif语句继续进行if-then检查，为单个比较变量寻找特定值。

你可以使用case命令，而不用写出那么多elif语句来不断检查相同变量的值。case命令会检查单个变量格式的多个值：
{% highlight string %}
case variable in
pattern1 | pattern2) commands1;;
pattern3) commands2;;
*) commands3;;
esac
{% endhighlight %}
case命令会将指定的变量同不同模式进行比较。如果变量和模式是匹配的，那么shell会执行为该模式指定的命令。你可以通过竖线操作符来分隔模式，在一行列出多个模式。星号会捕获所有跟列出的模式都不匹配的值。这里有个将if-then-else程序转换成用case命令的例子：
{% highlight string %}
# cat test26 
#!/bin/bash

# using the case command


case $USER in
root | barbara)
    echo "Welcome, $USER"
    echo "Please enjoy your visit";;
testing)
    echo "Special testing account";;
jessica)
    echo "Do not forget to log off when you're done";;
*)
    echo "Sorry, you are not allowed here";;
esac


# ./test26 
Welcome, root
Please enjoy your visit
{% endhighlight %}
case命令提供了一个更清晰的方法来为变量每个可能的值指定不同的选项。


<br />
<br />

**[参看]**






<br />
<br />
<br />


