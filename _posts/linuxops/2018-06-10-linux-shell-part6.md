---
layout: post
title: 高级脚本编程之呈现数据
tags:
- LinuxOps
categories: linuxOps
description: 高级脚本编程之呈现数据
---



到目前为止，出现的脚本都是通过将数据打印在屏幕上或将数据重定向到文件中来显示信息。本章，我们将将演示如何将脚本的输出重定向到Linux系统的不同位置。




<!-- more -->

## 1. 理解输入和输出
到目前为止，你已经知道了两种显示脚本输出的方法：

* 在显示器屏幕上显示输出

* 将输出重定向到文件

这两种方法都会将数据输出全部显示或完全不显示。但有时可能希望在显示器上显示一些数据，其他数据保存到文件中。对于这些情况，了解Linux如何处理输入输出会派上一点用场，这样你就能将脚本输出放到正确位置。

下面几节会介绍如何用标准的Linux输入和输出系统来帮助将脚本输出重定向到特定位置。

### 1.1 标准文件描述符
Linux系统将每个对象当做文件来处理。这包括输入和输出的过程。Linux用文件描述符来标识每个文件对象。文件描述符是一个非负整数，可以唯一的标识会话中打开的文件。每个过程一次最多可以有9个文件描述符。出于特殊目的，bash shell保留了最早的3个文件描述符（0、1和2）。这些在下表中显示了：
<pre>
              表： Linux的标准文件描述符

文件描述符           缩写                 描述
-----------------------------------------------------------------
    0              STDIN               标准输入
    1              STDOUT              标准输出
    2              STDERR              标准错误 
</pre>
这三个特殊文件描述符会处理脚本的输入和输出。shell用它们来将shell默认的输入和输出（默认情况下通常是显示器）定向到相应的位置。下面几节将会进一步介绍这些标准的文件描述符。

1） **STDIN**

STDIN文件描述符代表shell的标准输入。对终端界面来说，标准输入是键盘。shell从STDIN文件描述符对应的键盘获得输入，在用户输入时处理每个字符。

在使用输入重定向符号(<)时，Linux会用重定向指定的文件来替换标准输入文件描述符。它会读取文件并提取数据，就如同它是键盘上键入的。

许多bash命令能接受STDIN的输入，尤其是没有在命令行上指定文件的话。下面是个用cat命令处理STDIN输入的数据的例子：
{% highlight string %}
# cat
this is a test
this is a test
this is a second test.
this is a second test.
{% endhighlight %}
当你在命令行上只输入cat命令自身时，它会接受STDIN的输入。当你在每行输入时，cat命令会将每行显示在输出中。

但你也可以通过STDIN重定向符号强制cat命令接受来自另一个非STDIN文件的输入：
{% highlight string %}
# cat < testfile
This is the first line.
This is the second line.
This is the third line.
{% endhighlight %}
现在cat命令会用testfile文件中的行作为输入。你可以使用这种技术来向任何能从STDIN接受数据的shell命令输入数据。

2） **STDOUT**

STDOUT文件描述符代表标准的shell输出。在终端界面上，标准输出就是终端显示器。shell的所有输出（包括shell中运行的程序和脚本）会被定向到标准输出中，也就是显示器。

默认情况下，很多bash命令会定向输出到STDOUT文件描述符。我们可以使用重定向来更改：
{% highlight string %}
# ls -l > test2
# cat test2
total 16
-rwxrwxrwx. 1 root root 225 Apr 22 10:38 myfuncs
-rwxrwxrwx. 1 root root 336 Apr 22 14:11 test14
-rwxrwxrwx. 1 root root 323 Apr 22 16:01 test15
-rw-r--r--. 1 root root   0 Apr 22 17:24 test2
-rw-r--r--. 1 root root  73 Apr 22 17:18 testfile
{% endhighlight %}

通过输出重定向符号，通常会显示到显示器的所有输出会被shell重定向到指定的重定向文件。

你也可以将数据追加到某个文件。可以用```>>```符号来完成：
{% highlight string %}
# who >> test2
# cat test2
total 16
-rwxrwxrwx. 1 root root 225 Apr 22 10:38 myfuncs
-rwxrwxrwx. 1 root root 336 Apr 22 14:11 test14
-rwxrwxrwx. 1 root root 323 Apr 22 16:01 test15
-rw-r--r--. 1 root root   0 Apr 22 17:24 test2
-rw-r--r--. 1 root root  73 Apr 22 17:18 testfile
ivan1001 :0           2019-04-22 16:15 (:0)
root     pts/0        2019-04-15 11:48 (192.168.79.1)
ivan1001 pts/3        2019-04-22 16:19 (:0)
{% endhighlight %}
who命令生成的输出会被追加到test2文件中。

但是，如果你对文件用了标准的输出重定向，你可能会遇到一个问题。下面的例子说明了可能会出现什么情况：
{% highlight string %}
# ls -al badfile > test3
ls: cannot access badfile: No such file or directory
# cat test3
{% endhighlight %}
当命令发生错误时，shell并未将错误消息重定向到输出重定向文件。shell创建了输出重定向文件，但错误消息却显示在了显示器屏幕上。注意，在显示test3文件的内容时并没有任何错误。test3文件创建成功了，只是里面是空的。

shell处理错误消息是跟处理普通输出分开的。如果你创建了在后台模式下运行的shell脚本，通常你必须依赖发送到日志文件的输出消息。用这种方法，它们就没法保存在日志文件中。你需要换种方法来处理。

3） **STDERR**

shell通过特殊的STDERR文件描述符来处理错误消息。STDERR文件描述符代表shell的标准错误输出。shell或shell中运行的程序和脚本出错时生成的错误消息都会发送到这个位置。

默认情况下，STDERR文件描述符会和STDOUT文件描述符指向同样的地方（尽管分配给它们的文件描述符不同）。也就是说，默认情况下，错误消息也会输出到显示器输出中。

但从上面的例子可以看出，重定向STDOUT并不会自动重定向STDERR。处理脚本时，你常常会想改变这种行为，如果你想将错误消息保存到日志文件中时尤其如此。

### 1.2 重定向错误

你已经知道如何用重定向符号来重定向STDOUT数据。成功定向STDERR数据也没太大差别，你只要在使用重定向符号时定义STDERR文件描述符就可以了。有几种办法来处理。

1) **只重定向错误**

如你在上表（表： Linux的标准文件描述符）中看到的，STDERR文件描述符被设为2。你可以选择只重定向错误消息，将该文件描述符值放在重定向符号前。该值必须紧紧地放在重定向符号前，否则不会工作：
{% highlight string %}
# ls -al badfile 2> test4
# cat test4
ls: cannot access 'badfile': No such file or directory
root@ubuntu:~/workspace# 
{% endhighlight %}

现在运行该命令，错误消息不会出现在屏幕上了。而是，该命令生成的任何错误消息都会保存在输出文件中。用这种方法，shell会只重定向错误消息，而非普通数据。这里有个在同一输出中混用STDOUT和STDERR的例子：
{% highlight string %}
# ls -al test badtest test2 2> test5
-rw-r--r-- 1 root root 55 Apr 22 08:41 test
# cat test5
ls: cannot access 'badtest': No such file or directory
ls: cannot access 'test2': No such file or directory
{% endhighlight %}
ls命令的正常STDOUT输出仍然会发送到默认的STDOUT文件描述符，也就是显示器。由于该命令重定向了文件描述符2的输出（STDERR）到了一个输出文件，shell会将生成的任何错误消息直接发送到指定的重定向文件中。

2）**重定向错误和数据**

如果你想重定向错误和正常输出，你必须用两个重定向符号。你需要在想要重定向的每个数据前添加对应的文件描述符，并将它们指向对应的保存数据的输出文件：
{% highlight string %}
# ls -al test test2 test3 badtest 2>test6 1>test7
# cat test6
ls: cannot access 'test': No such file or directory
ls: cannot access 'badtest': No such file or directory

# cat test7
-rw-r--r-- 1 root root 0 Apr 22 09:11 test2
-rw-r--r-- 1 root root 0 Apr 22 09:11 test3
{% endhighlight %}

shell利用```1>```符号将ls命令本该输出到STDOUT的正常输出重定向到了test7文件。任何本该输出到STDERR的错误消息通过```2>```符号被重定向到了test6文件。

你可以用这种方法来将脚本的正常输出和脚本生成的错误消息分离开来。它允许你轻松地识别错误，而不用在成千上万行正常输出数据中艰难查找。

另外，如果愿意，你也可以将STDERR和STDOUT的输出重定向到同一个输出文件。为此，bash shell提供了特殊的重定向符号```&>```:
{% highlight string %}
# ls -al test test2 test3 badtest &>test7
root@ubuntu:~/workspace# cat test7
ls: cannot access 'test': No such file or directory
ls: cannot access 'badtest': No such file or directory
-rw-r--r-- 1 root root 0 Apr 22 09:11 test2
-rw-r--r-- 1 root root 0 Apr 22 09:11 test3
{% endhighlight %}
当使用```&>```符号时，命令生成的所有输出都会发送到同一位置，包括数据和错误。你会注意到错误消息中有一条跟你期望的顺序不同。这条针对badtest文件的错误消息（列出的最后一个文件）出现在输出文件中的第二行。bash shell会自动给错误消息分配较标准输出来说更高的优先级。这样你就能在一处地方查看错误消息了，而不用翻遍整个输出文件。


## 2. 在脚本中重定向输出

你可以在脚本中用STDOUT和STDERR文件描述符来在多个位置生成输出，只要简单地重定向相应的文件描述符。有两种方法来在脚本中重定向输出：

* 临时重定向每行输出

* 永久重定向脚本中的所有命令

1） **临时重定向**

如果你要故意在脚本中生成错误消息，可以将单独的一行输出重定向到STDERR。你所需要做的是使用输出重定向符来将输出重定向到STDERR文件描述符。在重定向到文件描述符时，你必须在文件描述符数字之前加一个and符(&):
{% highlight string %}
echo "This is an error message" >&2
{% endhighlight %}

注意， ```>&2```为一个整体，中间不能有空格。上面这行会将文本显示在脚本的STDERR文件描述符所指向的任何位置，而不是通常的STDOUT。下面是个用此特性的脚本例子：
{% highlight string %}
# cat test8
#!/bin/bash

# testing STDERR messages

echo "This is an error" >&2

echo "This is normal output"
{% endhighlight %}
正常运行这个脚本，你可能看不出什么区别：
<pre>
# ./test8
This is an error
This is normal output
</pre>
记住，默认情况下Linux会将STDERR定向到STDOUT。但是，如果你在运行脚本时重定向了STDERR，脚本中所有定向到STDERR的文本都会被重定向：
{% highlight string %}
# ./test8 2> test9
This is normal output
# cat test9
This is an error
{% endhighlight %}
太好了！通过STDOUT显示的文本显示在了屏幕上，而发送给STDERR的echo语句的文本则重定向到了输出文件。

这个方法非常适合在脚本中生成错误消息。如果有人用了你的脚本，他们可以轻松地通过STDERR文件描述符重定向错误消息，如上所示。

2） **永久重定向**

如果脚本中有大量数据需要重定向，那重定向每个echo语句就会很繁琐。取而代之，你可以用exec命令告诉shell在脚本执行期间重定向某个特定文件描述符：
{% highlight string %}
# cat test10 
#!/bin/bash

# redirecting all output to a file

exec 1>testout

echo "This is a test of redirecting all output."

echo "from a script to another file"

echo "without having to redirect every individual line"


# ./test10 
# cat testout
This is a test of redirecting all output.
from a script to another file
without having to redirect every individual line
{% endhighlight %}
exec命令会启动一个新的shell并将STDOUT文件描述符重定向到文件。脚本中发给STDOUT的所有输出会被重定向到文件。

你可以在脚本中间重定向STDOUT:
{% highlight string %}
# cat ./test11
#!/bin/bash

# redirecting output to different locations

exec 2>testerror

echo "This is the start of the script"
echo "now redirecting all output to another location"

exec 1>testout

echo "This output should go to the testout file"
echo "but this should go to the testerror file" >&2

# ./test11
This is the start of the script
now redirecting all output to another location

# cat testout
This output should go to the testout file

# cat testerror 
but this should go to the testerror file
{% endhighlight %}
这个脚本用exec命令来将发给STDERR的输出重定向到文件testerror。下一步，脚本用语句显示了一些行到STDOUT。那之后，再次使用exec命令来将STDOUT重定向到testout文件。注意，尽管STDOUT被重定向了，你仍然可以指定将echo语句的输出发给STDERR，在本例中仍然是重定向到testerror文件中。

这个特性在你要将脚本的部分输出重定向到另一个位置时能派上用场，比如错误日志。在使用该特性时你会碰到个小问题。

一旦你重定向了STDOUT或STDERR，你就无法轻易将它们重定向回原来的位置。你需要在重定向中来回切换的话，有个办法可以用。在本文的第4节将会讨论该方法以及如何在脚本中使用。

## 3. 在脚本中重定向输入
你可以用在脚本中重定向STDOUT和STDERR的同样方法来将STDIN从键盘重定向到其他位置。exec命令允许你将STDIN重定向到Linux系统上的文件中。
{% highlight string %}
exec 0< testfile
{% endhighlight %}
这个命令会告诉shell它应该从文件testfile中获得输入，而不是STDIN。这个重定向只要在脚本需要输入时就会作用。下面是该用法的实例：
{% highlight string %}
# cat test12
#!/bin/bash

# redirecting file input

exec 0<testfile

count=1

while read line
do
    echo "Line #$count: $line"
    count=$[$count + 1]
done

# ./test12
Line #1: This is the first line.
Line #2: This is the second line.
Line #3: This is the third line.
{% endhighlight %}
在上一章<<高级脚本编程之处理用户输入>>介绍了如何使用read命令来读取用户在键盘上输入的数据。将STDIN重定向到文件后，当read命令试图从STDIN读入数据时，它会到文件去读取数据，而不是键盘。

这是在脚本中从要处理的文件中读取数据的绝妙方法。Linux系统管理员的一项日常任务就是从要处理的日志文件中读取数据。这是完成该任务最简单的办法。

## 4. 创建自己的重定向
在脚本中重定向输入和输出时，并不局限于这3个默认的文件描述符。我曾提到过，在shell中最多可以有9个打开的文件描述符。其他6个文件描述符会从3排到8，并且当做输入或输出重定向都行。你可以将这些文件描述符中的任意一个分配给文件，然后在脚本中使用它们。本节将介绍如何在脚本中使用其他文件描述符。

1） **创建输出文件描述符**

你可以用exec命令来给输出分配文件描述符。和标准的文件描述符一样，一旦你给一个文件位置分配了另外一个文件描述符，那个重定向就会一直有效，直到你重新分配。这里有个在脚本中使用其他文件描述符的简单例子：
{% highlight string %}
# cat test13
#!/bin/bash

# using an alternative file descriptor

exec 3>test13out

echo "This should display on the monitor"
echo "and This should be stored in the file" >&3
echo "Then this should be back on the monitor"

# ./test13 
This should display on the monitor
Then this should be back on the monitor
# cat test13out 
and This should be stored in the file
{% endhighlight %}
这个脚本用```exec```命令来将文件描述符3重定向到另一个文件位置。当脚本执行echo语句时，它们如你所期望的那样，显示在STDOUT上。但你重定向到文件描述符3的那行echo语句输出到了另外那个文件。它允许你在显示器上保持正常的输出，而将特定信息重定向到文件中，比如日志文件。

你也可以使用exec命令来将输出追加到现有文件中，而不是创建一个新文件：
{% highlight string %}
exec 3>>test13out
{% endhighlight %}
现在输出会被追加到test13out文件，而不是创建一个新文件。

2) **重定向文件描述符**

现在介绍怎么从已重定向的文件描述符中恢复。你可以分配另外一个文件描述符给标准文件描述符，反之亦然。这意味着你可以重定向STDOUT的原来位置到另一个文件描述符，然后将该文件描述符重定向回STDOUT。这听起来有点复杂，但实际上相当直接。这个简单的例子能帮你理清楚：
{% highlight string %}
# cat test14 
#!/bin/bash

# storing STDOUT, then come back to it

exec 3>&1

exec 1>test14out

echo "This should store in the output file"
echo "along with this line"

exec 1>&3

echo "Now things should be back to normal"



# ./test14
Now things should be back to normal
# cat test14out 
This should store in the output file
along with this line
{% endhighlight %}

这个例子有点叫人抓狂，我们来一段一段地看。首先，脚本将文件描述符3重定向到文件描述符1的当前位置，也就是STDOUT。这意味着任何发送给文件描述符3的输出都将出现在显示器上。

第二个exec命令将STDOUT重定向到文件，shell现在会将发送给STDOUT的输出直接重定向到输出文件中。但是，文件描述符3仍然指向STDOUT原来的位置，也就是显示器。如果此时将输出数据发送给描述符3，它仍然会出现在显示器上，尽管STDOUT已经被重定向过。

在向STDOUT（现在指向一个文件）发送一些输出之后，脚本会将STDOUT重定向到文件描述符3的当前位置（仍然是设置到显示器）。这意味着现在STDOUT指向了它原来的位置，也就是显示器。

这个方法可能有点叫人困惑，但它是在脚本中临时将输出重定向然后再将输出恢复到通常设置的通用办法。

3） **创建输入文件描述符**

你可以用和重定向输出文件描述符同样的办法来重定向输入文件描述符。在重定向到文件之前，先将STDIN文件描述符保存到另外一个文件描述符，然后在读取完文件之后再将STDIN恢复到它原来的位置：
{% highlight string %}
# cat test15 
#!/bin/bash

# redirecting input file descriptors

exec 6<&0

exec 0<testfile

count=1

while read line
do
   echo "Line #$count: $line"
   count=$[$count + 1]
done


exec 0<&6

read -p "Are you done now? " answer

case $answer in
Y|y) echo "Goodby";;
N|n) echo "Sorry, this is the end";;
esac

# ./test15 
Line #1: This is the first line.
Line #2: This is the second line.
Line #3: This is the third line.
Are you done now? y
Goodby
{% endhighlight %}
在这个例子中，文件描述符6用来保存STDIN的位置。然后脚本将STDIN重定向到一个文件。read命令的所有输入都是从重定向后的STDIN中来的，也就是输入文件。

在读取了所有行之后，脚本会将STDIN重定向到文件描述符6，从而将STDIN恢复到原来的位置。该脚本用了另外一个read命令来测试STDIN是否恢复正常了。这次它会等待键盘的输入。

4) **创建读写文件描述符**

尽管看起来可能会很奇怪，你也可以打开单个文件描述符来作为输入和输出。你可以用同一个文件描述符来从文件中读取数据，并将数据写到同一个文件中。

但用这种方法时，你要特别小心。由于你在向同一个文件进行读取数据、写入数据操作，shell会维护一个内部指针，指明现在在文件中什么位置。任何读或写都会从文件指针上次保存的位置开始。如果你不够小心，它会产生一些有意思的结果。看看下面这个例子：
{% highlight string %}
# cat test16
#!/bin/bash

# testing input/output file descriptor

exec 3<>testfile

read line <&3

echo "Read: $line"

echo "This is a test line" >&3


# ./test16 
Read: This is the first line.
# cat testfile
This is the first line.
This is a test line
ine.
This is the third line.
{% endhighlight %}
这个例子用了exec命令来将用作读取输入和写入输出的文件描述符3分配给文件testfile。下一步，它通过分配好的文件描述符来用read命令读取文件中的第一行，然后将读取的那行输入显示在STDOUT上。之后，它用echo语句来向用通过文件描述符打开的文件写入一行数据。

在运行脚本时，最开始事情看起来都还正常。输出说明脚本读了testfile文件中的第一行。但在运行脚本后，显示testfile文件的内容，你会看到写到文件中的数据覆盖了已有的数据。

当脚本向文件中写入数据时，它会从文件指针所处的位置开始。read命令读取了第一行数据，所以它会让文件指针指向第二行数据的第一个字符。在echo语句将数据输出到文件时，它会将数据放在文件指针的当前位置，覆盖该位置的任何数据。

5) **关闭文件描述符**

如果你创建了新的输入或输出文件描述符，shell脚本会在退出时自动关闭它们。然而还有一些情况，你需要在脚本结束前手动关闭文件描述符。

要关闭文件描述符，将它重定向到特殊符号```&-```。脚本看起来如下：
{% highlight string %}
exec 3>&-
{% endhighlight %}
该语句会关闭文件描述符3，从而阻止在脚本中再使用它。这里有个例子说明当你尝试使用已关闭的文件描述符时会怎样：
{% highlight string %}
# cat badtest 
#!/bin/bash

# testing closing file descriptors


exec 3<>test17file

echo "This is a test line of data" >&3

exec 3>&-

echo "This won't work" >&3

# ./badtest 
./badtest: line 12: 3: Bad file descriptor
{% endhighlight %}
一旦关闭了文件描述符，你就不能在脚本中向它写入任何数据了，否者shell会生成错误消息。

在关闭文件描述符时还要注意另一件事。如果后面你在脚本中打开了同一个输出文件，shell会用一个新文件来替换已有文件。这意味着如果你要输出任何数据，它将会覆盖已有文件。考虑下面这个问题的例子：
{% highlight string %}
# cat test17 
#!/bin/bash

# testing closing file descriptors

exec 3>test17file

echo "This is a test line of data" >&3
exec 3>&-

cat test17file

exec 3>test17file

echo "This'll be bad" >&3

# ./test17 
This is a test line of data
# cat test17file
This'll be bad
{% endhighlight %}

在向test17file文件发送一个数据字符串并关闭该文件描述符后，脚本用了cat命令来显示文件的内容。到目前为止，一切都还好。下一步，脚本重新打开了该输出文件并向它发送了另一个数据字符串。当你显示该输出文件的内容时，你所能看到的只有第二个数据字符串。shell覆盖了原来的输出文件。

5) **列出打开的文件描述符**

对你来说，只有9个文件描述符可用，你可能会觉得要让事情简单直接并不难。但有时要记住哪个文件描述符被重定向到了哪里很难。为了便于理清楚，bash shell提供了lsof命令。

lsof命令会列出整个linux系统打开的所有文件描述符。这是个有争议的功能，因为它会向非系统管理员用户提供linux系统的信息。鉴于此，许多Linux系统隐藏了该命令，这样用户就不会一不小心就发现了。

在我的Fedora Linux系统上，lsof命令位于/usr/sbin目录。要用普通用户来运行它，我必须通过全路径名来引用它：
<pre>
# /usr/sbin/lsof
</pre>
它会产生大量的输出。它会显示当前Linux系统上打开的每个文件的有关信息。这包括后台运行的所有进程以及登录到系统的任何用户。

有足够的命令行选项和参数帮助过滤lsof的输出。最常用的有```-p```和```-d```选项，前者允许指定进程ID(PID)，后者允许指定要显示的文件描述符个数。

要知道该进程的当前PID，你可以用特殊环境变量```$$```(shell会将它设置为当前PID）。```-a```选项用来对其他两个选项的结果执行布尔AND运算，产生如下输出：
{% highlight string %}
# /usr/sbin/lsof -a -p $$ -d 0,1,2
lsof: WARNING: can't stat() fuse.gvfsd-fuse file system /run/user/1000/gvfs
      Output information may be incomplete.
COMMAND  PID USER   FD   TYPE DEVICE SIZE/OFF NODE NAME
bash    4808 root    0u   CHR  136,0      0t0    3 /dev/pts/0
bash    4808 root    1u   CHR  136,0      0t0    3 /dev/pts/0
bash    4808 root    2u   CHR  136,0      0t0    3 /dev/pts/0
{% endhighlight %}
上例显示了当前进程(bash shell)的默认文件描述符。lsof的默认输出中有7列信息，见下表所示：
<pre>
                     表： lsof的默认输出

   列                        描述
------------------------------------------------------------------------------------------
COMMAND             正在运行的命令名的前9个字符
PID                 进程的PID
USER                进程属猪的登录名
FD                  文件描述符值以及访问类型（r代表读，w代表写，u代表读写）
TYPE                文件的类型（CHR代表字符型，BLK代表块类型，DIR代表目录，REG代表常规文件）
DEVICE              设备的设备号(主设备号和从设备号）
SIZE                如果有的话，文件的大小
NODE                本地文件的节点号
NAME                文件名
</pre>
与STDIN、STDOUT和STDERR关联的文件类型是字符型。因为STDIN、STDOUT和STDERR文件描述符都指向终端的设备名。所有3种标准文件都支持读和写（尽管向STDIN写数据以及从STDOUT读数据看起来有点奇怪）。

现在，我们看一下打开了多个替代性文件描述符的脚本中lsof命令的结果：
{% highlight string %}
# cat test18
#!/bin/bash

# testing lsof with file descriptors

exec 3>test18file1
exec 6>test18file2
exec 7<testfile

/usr/sbin/lsof -a -p $$ -d 0,1,2,3,6,7

# ./test18 
lsof: WARNING: can't stat() fuse.gvfsd-fuse file system /run/user/1000/gvfs
      Output information may be incomplete.
COMMAND   PID USER   FD   TYPE DEVICE SIZE/OFF      NODE NAME
test18  27314 root    0u   CHR  136,0      0t0         3 /dev/pts/0
test18  27314 root    1u   CHR  136,0      0t0         3 /dev/pts/0
test18  27314 root    2u   CHR  136,0      0t0         3 /dev/pts/0
test18  27314 root    3w   REG    8,3        0 138317852 /root/workspace/test18file1
test18  27314 root    6w   REG    8,3        0 138317865 /root/workspace/test18file2
test18  27314 root    7r   REG    8,3        0 142445969 /root/workspace/testfile
{% endhighlight %}

该脚本创建了3个替代性文件描述符，两个作为输出（3和6），一个作为输入（7）。在脚本运行lsof命令时，你可以在输出中看到新的文件描述符。我们去掉了输出中的第一部分，这样你就能看到文件名的结果了。文件名显示了文件描述符中采用的文件的完整路径名。它将将每个文件都显示成REG类型的，说明它们是文件系统中的常规文件。

## 6. 阻止命令输出

有时候你不想显示脚本的输出。这在将脚本作为后台进程执行时很常见。如果在脚本后台运行时有任何错误消息出现，shell会通过电子邮件将它们发送给进程的属主。这会很麻烦，尤其是在你运行会生成很多烦琐的小错误的脚本时。

要解决这个问题，你可以将STDERR重定向到一个称作null文件的特殊文件。null文件跟它的名字很像，文件里什么都没有。shell输出到null文件的的任何数据都不会保存，这样它们就被丢弃了。

在Linux系统上null文件的标准位置是/dev/null。你重定向到该位置的任何数据都会被丢掉，不会显示：
{% highlight string %}
# ls -al > /dev/null
# cat /dev/null
{% endhighlight %}
这是阻止任何错误消息而不保存它们的一个通用方法：
{% highlight string %}
# ls -al badfile test16 2>/dev/null
-rw-r--r-- 1 root root 0 Apr 24 08:46 test16
{% endhighlight %}
你也可以在输入重定向将/dev/null作为输入文件。由于/dev/null文件不含有任何内容，程序员通常用它来快速移除现有文件中的数据而不用先删除文件再创建：
{% highlight string %}
# cat testfile 
This is the first line.
This is the second line.
This is the third line.


# cat /dev/null > testfile
# cat testfile
{% endhighlight %}
文件testfile仍然存在系统上，但现在它是空文件。这是清除日志文件的一个通用方法，它们必须留在原来位置等待应用程序写入。


## 7. 创建临时文件
Linux系统有一个特殊的留给临时文件用的目录位置。Linux使用/tmp目录来存放不需要一直保留的文件。大多数Linux发行版配置了系统在启动时自动删除/tmp目录的所有文件。

系统上的任何用户账户都有权限在/tmp目录中读和写。这个特性为你提供了简单地创建临时文件的途经，而不用管清理工作。

有个特殊的命令可以用来创建临时文件。mktemp命令可以轻松地在/tmp目录中创建一个唯一的临时文件。shell会创建这个文件，但不用默认的umask值。它会将文件的读和写权限分配给文件的属主，并将你设置成文件的属主。一旦创建了文件，你就在脚本中有了完整的读写权限，但其他人没法访问（当然，root用户除外）。

1） **创建本地临时文件**

默认情况下，mktemp会在本地目录中创建一个文件。要用mktemp命令在本地目录中创建一个临时文件，你只要指定一个文件名模板就行了。模板可以包含任意文本文件名，在文件名末尾加上6个```X```就行了：
<pre>
# mktemp testing.XXXXXX
testing.2lUbyW
#  ls -al testing*
-rw-------. 1 root root 0 Apr 26 10:18 testing.2lUbyW
</pre>
mktemp命令会用6个字符码替换这6个X，从而保证文件名在目录中是唯一的。你可以创建多个临时文件，它可以保证每个文件都是唯一的：
<pre>
# mktemp testing.XXXXXX
testing.Rhr8Iw
# mktemp testing.XXXXXX
testing.hMgsNx
# mktemp testing.XXXXXX
testing.b74WZF
[root@localhost workspace]#  ls -al testing*
-rw-------. 1 root root 0 Apr 26 10:18 testing.2lUbyW
-rw-------. 1 root root 0 Apr 26 10:21 testing.b74WZF
-rw-------. 1 root root 0 Apr 26 10:21 testing.hMgsNx
-rw-------. 1 root root 0 Apr 26 10:21 testing.Rhr8Iw
</pre>

如你所看到的，mktemp命令的输出正是它所创建的文件的名字。在脚本中使用mktemp命令时，你可能要将文件名保存到变量中，这样你就能在后面的脚本中引用了：
{% highlight string %}
# cat test19 
#!/bin/bash

# creating and using a temp file

tempfile=`mktemp test19.XXXXXX`

exec 3>$tempfile

echo "This script writes to temp fille '$tempfile'"

echo "This is the first line." >&3
echo "This is the second line." >&3
echo "This is the third line." >&3

exec 3>&-

echo "Done creating temp file. The contents are:"

cat $tempfile

rm -rf $tempfile 2>/dev/null

# ./test19 
This script writes to temp fille 'test19.jzj3WH'
Done creating temp file. The contents are:
This is the first line.
This is the second line.
This is the third line.
{% endhighlight %}
这个脚本用mktemp命令来创建临时文件并将文件名赋给```$tempfile```变量。然后，它将这个临时文件作为文件描述符3的输出重定向文件。在将临时文件名显示在STDOUT之后，它会将一些行写到临时文件，然后它会显示临时文件的内容，并用rm命令删除。

2) **在/tmp目录创建临时文件**

```-t```选项会强制mktemp命令来在系统的临时目录来创建该文件。在用这个特性时，mktemp命令会返回用来创建临时文件的全路径，而不是只有文件名：
<pre>
# mktemp -t test.XXXXXX
/tmp/test.u4wDZ6
# ls -al /tmp/test*
-rw-------. 1 root root 0 Apr 26 10:56 /tmp/test.u4wDZ6
</pre>

由于mktemp命令返回了全路径名，你就可以在Linux系统上的任何目录下引用该临时文件，而不用管它将临时文件放在了什么位置：
{% highlight string %}
# cat test20 
#!/bin/bash

# creating a temp file in /tmp

tempfile=`mktemp -t tmp.XXXXXX`

echo "This is a test file." >$tempfile

echo "This is the second line of the test." >>$tempfile

echo "The temp file is located at: $tempfile"

cat $tempfile

rm -rf $tempfile
# ./test20 
The temp file is located at: /tmp/tmp.el7oX3
This is a test file.
This is the second line of the test.
{% endhighlight %}

在mktemp创建临时文件时，它会将全路径名返回给变量。这样你就能在任何命令中使用该值来引用该临时文件了。

3) **创建临时目录**

```-d```选项告诉mktemp命令来创建一个临时目录而不是临时文件。这样你就能用该目录做想做任何需要的操作了，比如创建额外的临时文件：
{% highlight string %}
# cat test21 
#!/bin/bash

# using a temporary directory

tempdir=`mktemp -d dir.XXXXXX`

cd $tempdir

tempfile=`mktemp temp.XXXXXX`
tempfile2=`mktemp temp.XXXXXX`

exec 7>$tempfile
exec 8>$tempfile2

echo "Sending data to directory $tempdir"

echo "This is a test line of data for '$tempfile'" >&7
echo "This is a test line of data for '$tempfile2'" >&8

# ./test21 
Sending data to directory dir.3CR7bQ
# cat dir.3CR7bQ/temp.orwVXV 
This is a test line of data for 'temp.orwVXV'
# cat dir.3CR7bQ/temp.xXgAwv 
This is a test line of data for 'temp.xXgAwv'
{% endhighlight %}
这段脚本在当前目录创建了一个目录，然后它用cd命令跳进该目录，并创建了两个临时文件。之后这两个临时文件被分配给文件描述符用来存储脚本的输出。

## 8. 记录消息
有时将输出一边发送到显示器一边发送到日志文件，这会有一些好处。你不用将输出重定向两次，只要用特殊的tee命令就行。

tee命令相当于管道的```T型```接头。它将从STDIN过来的数据同时发给两个目的地。一个目的地是STDOUT，另一个目的地是tee命令行所指定的文件名：
<pre>
tee filename
</pre>
由于tee会将从STDIN过来的数据重定向，你可以用它和管道命令来一起将任何命令的输出重定向：
<pre>
# date | tee testfile
Fri Apr 26 11:30:24 CST 2019
# cat testfile
Fri Apr 26 11:30:24 CST 2019
</pre>
输出出现在了STDOUT中，并且也写入了指定的文件中。注意，默认情况下，tee命令会在每次使用时覆盖输出文件内容：
<pre>
# who | tee testfile
root     pts/0        2019-04-15 11:48 (192.168.79.1)

# cat testfile
root     pts/0        2019-04-15 11:48 (192.168.79.1)
</pre>
如果想将数据追加到文件中，必须用```-a```选项：
<pre>
# date | tee -a testfile
Fri Apr 26 11:34:02 CST 2019

# cat testfile 
root     pts/0        2019-04-15 11:48 (192.168.79.1)
Fri Apr 26 11:34:02 CST 2019
</pre>
利用这个方法，你就能将数据保存在文件中，也能将数据显示在屏幕上了：
{% highlight string %}
# cat test22 
#!/bin/bash

# using the tee command for logging

tempfile=test22file

echo "This is the start of the test." | tee $tempfile

echo "This is the second line of the test." | tee -a $tempfile

echo "This is the end of the test." | tee -a $tempfile

# ./test22 
This is the start of the test.
This is the second line of the test.
This is the end of the test.
# cat test22file 
This is the start of the test.
This is the second line of the test.
This is the end of the test.
{% endhighlight %}
现在你就能在将输出显示给用户的同时也永久保留一份了。



<br />
<br />

**[参看]**






<br />
<br />
<br />


