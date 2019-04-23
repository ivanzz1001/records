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




<br />
<br />

**[参看]**






<br />
<br />
<br />


