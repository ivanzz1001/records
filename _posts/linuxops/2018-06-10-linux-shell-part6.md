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













<br />
<br />

**[参看]**






<br />
<br />
<br />


