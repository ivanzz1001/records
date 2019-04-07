---
layout: post
title: awk命令的使用
tags:
- LinuxOps
categories: linuxOps
description: awk命令的使用
---

本节主要介绍一下awk/gawk命令的使用。
<pre>
# cat /etc/centos-release
CentOS Linux release 7.3.1611 (Core) 

# uname -a
Linux sz-oss-01.localdomain 3.10.0-514.el7.x86_64 #1 SMP 
Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
</pre>

<!-- more -->

## 1. gawk基础
前面我们讲过sed编辑器，虽然其是自动修改文本文件的非常方便的工具，但它也有自身的限制。通常你需要一个用来处理文件中的数据的更高级工具，它能提供一个类编程环境，允许修改和重新组织文件中的数据。这正是gawk的特点。

gawk程序是Unix中的原始awk程序的GNU版本。gawk程序让流编辑器迈上了一个新的台阶，它提供了一种编程语言而不只是编辑器命令。在gawk语言中，你可以做下面的事：

* 定义变量来保存数据

* 使用算术和字符串操作符来处理数据

* 使用结构化编程概念，比如if-then语句和循环，来为数据处理增加逻辑；

* 提取数据文件中的数据元素并将它们按另一顺序或格式重新放置，从而生成格式化报告；

gawk程序的报告生成能力通常用来从大文本文件中提取数据元素并将它们格式化成可读的报告。最完美的例子是格式化日志文件。在日志文件中找出错误行会很难，gawk程序允许从日志文件中只过滤出你要看的数据元素，并以某种更容易读取重要数据的方式将它们格式化。

1） **gawk命令格式**

gawk程序的基本格式如下：
{% highlight string %}
gawk options program file
{% endhighlight %}
下表显示了gawk程序的可用选项：
<pre>
   选项                          含义
----------------------------------------------------------------------
 -F fs            指定行中分隔数据字段的字段分隔符

 -f file          指定要读取的程序文件名（program)

 -v var=value     定义gawk程序中的一个变量及其默认值

 -mf N            指定要处理的数据文件中的最大字段数

 -mr N            指定数据文件中的最大数据行数

 -w keyword       指定gawk的兼容模式或警告等级
</pre>
命令行选项提供了一个简单的途经来定制gawk程序中的功能。我们会在探索gawk时进一步了解这些选项。

gawk的强大之处在于程序脚本。你可以写脚本来读取文本行的数据，然后处理并显示数据，创建任何类型的输出报告。

2） **从命令行读取程序脚本**

gawk程序脚本用一对花括号来定义。你必须将脚本命令放到两个括号中。由于gawk命令行假定脚本是单个文本字符串，你必须将脚本放到单引号中。下面是个在命令行上指定简单的gawk程序脚本的例子：
{% highlight string %}
# gawk '{print "Hello John!"}'
{% endhighlight %}
这个程序脚本会定义一个命令，即```print```命令。print命令名副其实：它会将文本打印到STDOUT。如果你尝试运行这个命令，你可能会有些失望，因为不会有什么立即发生。因为并没有在命令行上指定文件名，gawk程序会从STDIN接收数据。在运行这个程序时，它会一直等待从STDIN输入的文本。

如果你输入一行文本并按下回车键，gawk会对这行文本运行一遍所有的程序脚本：
{% highlight string %}
# gawk '{print "Hello John!"}'
This is a test
Hello John!
Hello
Hello John!
This is another test
Hello John!
{% endhighlight %}
跟sed编辑器一样，gawk程序会针对数据流中的每行文本执行一遍程序脚本。由于程序脚本被设置为显示固定的文本字符串，因而不管你在数据流中输入什么文本，你都会得到同样的文本输出。

要终止gawk程序，你必须发出信号说明数据流已经结束了。bash shell提供了一对组合键来生成EOF(End-Of-File)字符。```CTRL+D```组合键会在bash中产生一个EOF字符。使用这对组合键就能终止gawk程序并返回到命令行界面提示符下。


3) **使用数据字段变量**

gawk的基本特性之一是它处理文本文件中数据的能力。它会自动给每行中的每个数据元素分配一个变量。默认情况下，gawk会将如下变量分配给它在文本行中发现的每个数据字段：

* $0 代表整个文本行

* $1 代表文本行中的第1个数据字段

* $2 代表文本行中的第2个数据字段

* $n 代表文本行中的第n个数据字段

每个数据字段在文本行中都是通过字段分隔符来划分的。gawk读取一行文本时，它会用预定义的字段分隔符来划分每个数据字段。gawk中默认的字段分隔符是任意的空白字符（例如空格或制表符）。

下面是个gawk程序读取文本文件并显示第1数据字段的例子：
{% highlight string %}
# cat data3
One line of test text.
Two lines of test text.
Three lines of test text.

# gawk '{print $1}' ./data3
One
Two
Three
{% endhighlight %}
该程序用```$1```字段变量来仅显示每行文本的第1个数据字段。

如果你要读取用其他字段分隔符的文件，可以用```-F```选项指定：
{% highlight string %}
# gawk -F: '{print $1}' /etc/passwd
root
daemon
bin
sys
sync
games
man
lp
mail
news
uucp
proxy
www-data
backup
...
{% endhighlight %}
这个简短的程序显示了系统上密码文件的第1个数据字段。由于/etc/passwd文件用冒号来分隔数据字段，因而如果要分隔开每个数据元素，则必须在gawk选项中将它指定为字段分隔符。

4） **在程序脚本中使用多个命令**

如果某编程语言只能执行一条命令，那么它不会有太大用处。gawk编程语言允许你将多条命令组合成一个正常的程序。要在命令行上的程序脚本中使用多条命令，只要在每条命令之间放个分号即可：
{% highlight string %}
# echo "My name is Rich" | gawk '{$4="Christine"; print $0}'
My name is Christine
{% endhighlight %}
第一条命令会将一个值赋给$4字段变量。第二条命令会打印整个数据字段。注意，输出中gawk程序已经将原文本中的第4个数据字段替换成了新值。

也可以用次提示符来一行一行地输入程序脚本命令：
{% highlight string %}
# gawk '{
> $4="testing"
> print $0
> }'
This is not a good test.
This is not testing good test.
{% endhighlight %}
在你用了开始的单引号后，bash shell会出现次提示符来提示你输入更多数据。你可以每次在每行添加一条命令，直到你输入了结尾的单引号。要退出程序，只要按下```CTRL+D```组合键来表明数据结束。

5） **从文件中读取程序**

跟sed编辑器一样，gawk编辑器允许将程序存储到文件中，然后再在命令行中引用：
{% highlight string %}
# cat script2
{print $1 "'s home director is " $6}

# gawk -F: -f ./script2 /etc/passwd
root's home director is /root
daemon's home director is /usr/sbin
bin's home director is /bin
sys's home director is /dev
sync's home director is /bin
games's home director is /usr/games
man's home director is /var/cache/man
lp's home director is /var/spool/lpd
mail's home director is /var/mail
...
{% endhighlight %}
script2程序脚本会再次使用print命令来打印/etc/passwd文件的主目录数据字段（字段变量$6)，以及userid数据字段(字段变量$1)。

可以在程序文件中指定多条命令。要这么做的话，只要将每条命令放到一个新的行就好了，不需要用分号：
{% highlight string %}
# cat script3
{
text="'s home director is "
print $1 text $6
}
# gawk -F: -f ./script3 /etc/passwd
root's home director is /root
daemon's home director is /usr/sbin
bin's home director is /bin
sys's home director is /dev
sync's home director is /bin
games's home director is /usr/games
man's home director is /var/cache/man
lp's home director is /var/spool/lpd
mail's home director is /var/mail
news's home director is /var/spool/news
uucp's home director is /var/spool/uucp
proxy's home director is /bin
...
{% endhighlight %}
script3程序脚本定义了一个变量来保存print命令中用到的文本字符串。你会注意到，gawk程序在引用变量值时并未像shell脚本一样使用美元符。

6) **在处理数据前运行脚本**


## 1. awk

向awk传递参数：
{% highlight string %}
for i in sda sdb sdc sdd sde sdf sdg sdh sdi sdj sdk; do path=`udevadm info -q path -n /dev/$i`; udevadm info -q env -p $path | grep ID_WWN= | awk 'BEGIN{FS="="} {print disk,"win-"$2}' disk=$i;done >> ./disk-id.txt
{% endhighlight %}







<br />
<br />

**[参看]**





<br />
<br />
<br />


