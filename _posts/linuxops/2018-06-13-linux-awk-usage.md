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

gawk还允许指定程序脚本何时运行。默认情况下，gawk会从输入中读取一行文本，然后针对文本行的数据执行程序脚本。有时，可能需要在处理数据前运行脚本，比如为报告创建开头部分。```BEGIN```关键字就是用来做这个的。它会强制gawk在读取数据前执行BEGIN关键字后指定的程序脚本：
{% highlight string %}
# gawk 'BEGIN{print "Hello World!"}'
Hello World!
{% endhighlight %}
这次print命令会在读取任何数据前显示文本。但在它显示了文本后，它会快速退出而不用等待任何数据。这样的原因是在gawk处理任何数据前，BEGIN关键字只执行指定的脚本。要想在正常的程序脚本中处理数据，必须用另一个脚本段来定义程序：
{% highlight string %}
# cat data4
Line 1
Line 2
Line 3
# gawk 'BEGIN{print "The data4 File Contents:"} {print $0}' ./data4
The data4 File Contents:
Line 1
Line 2
Line 3
{% endhighlight %}
现在在gawk执行了BEGIN脚本后，它会用第二段脚本来处理任何文件数据。这么做时要小心，注意这两段脚本在gawk命令行上会被当成一个文本字符串。你需要相应地加上单引号。

7) **在处理数据后运行脚本**

跟```BEGIN```关键字类似，```END```关键字允许你指定一个程序脚本，gawk会在读完数据后执行它：
{% highlight string %}
# gawk 'BEGIN{print "The data4 File Contents:"} {print $0} END{print "End of File"}' ./data4
The data4 File Contents:
Line 1
Line 2
Line 3
End of File
{% endhighlight %}

当gawk程序完成打印文件内容后，它会执行END脚本中的命令。这是在处理完所有正常数据后给报告添加结尾部分的最佳方法。

你可以将所有这些内容放到一起，组成一个很小的程序脚本文件从简单的数据文件创建一份完整的报告：
{% highlight string %}
# cat script4
BEGIN{
print "The latest list of users and shells"
print "Userid   Shell"
print "------   -------"
FS=":"
}

{
print $1 "\t" $7
}

END{
print "This conludes the listing."
}
{% endhighlight %}

这个脚本用BEGIN脚本来为报告创建开头部分。它还定义了一个称作FS的特殊变量。这是定义字段分隔符的另一种方法。这样你就不用依赖脚本用户来在命令行选项中定义字段分隔符了。

这里有段运行gawk程序脚本时截取的输出：
{% highlight string %}
Userid  Shell
------  -------
root    /bin/bash
daemon  /usr/sbin/nologin
bin     /usr/sbin/nologin
sys     /usr/sbin/nologin
sync    /bin/sync
games   /usr/sbin/nologin
man     /usr/sbin/nologin
lp      /usr/sbin/nologin
mail    /usr/sbin/nologin
news    /usr/sbin/nologin
uucp    /usr/sbin/nologin
...
This conludes the listing.
{% endhighlight %}

与预想的一样，BEGIN脚本创建了开头的文本，程序脚本处理了指定数据文件(/etc/passwd)中的信息，END脚本生成了结尾的文本。



## 2. gawk进阶

### 2.1 使用变量

所有编程语言共有的一个重要特性是使用变量来存取值。gawk编程语言支持两种不同类型的变量：

* 内建变量

* 自定义变量

gawk有一些内建变量。这些变量存放用来处理数据文件中的数据字段和数据行的信息。你也可以在gawk程序里创建你自己的变量。下面我们将逐步介绍gawk程序里如何使用变量。

1) **内建变量**

gawk程序使用内建变量来引用程序数据里的一些特殊功能。本节将介绍gawk程序中可用的内建变量并演示如何使用它们。

* 字段和数据行分隔符变量

在上面我们```gawk基础```中我们演示了gawk中的一种内建变量类型————数据字段变量。数据字段变量允许你使用美元符号($)和数据字段在数据行中的位置对应的数值来引用该数据行中的字段。因此，要引用数据行中的第一个数据字段，就用变量$1； 要引用第二个字段，就用$2；依次类推。

字段是由字段分隔符来划定的。默认情况下，字段分隔符是一个空白符，也就是空格符或者制表符(tab)。在上面```gawk基础```中，我们讲了如何在命令行下使用命令行参数```-F```或者在gawk程序中使用特殊的内建变量FS来更改字段分隔符。

内建变量FS是控制gawk如何处理输入输出数据中的字段和数据行的一组变量中的一个。下面列出了改组内建变量：
<pre>
                 表： gawk数据字段和数据行变量

  变 量                    描述
--------------------------------------------------------------------
 FIELDWIDTHS      由空格分隔开的定义了每个数据字段确切宽度的一列数字

   FS             输入字段分隔符
   RS             输入数据行分隔符
   OFS            输出字段分隔符
   ORS            输出数据行分隔符
</pre>

变量FS和OFS定义了gawk如何处理数据流中的数据字段。你已经了解了如何使用变量FS来定义什么字符分隔数据行中的字段。变量OFS具备相同的功能，不过是用在print命令的输出上。

默认情况下，gawk将OFS设置为一个空格，所以如果你用命令：
{% highlight string %}
print $1,$2,$3
{% endhighlight %}
你会看到如下输出：
<pre>
field1 field2 field3
</pre>
在下面的例子里，你能看到这点：
{% highlight string %}
# cat data1
data11,data12,data13,data14,data15
data21,data22,data23,data24,data25
data31,data32,data33,data34,data35
# gawk 'BEGIN{FS=","} {print $1,$2,$3}' data1
data11 data12 data13
data21 data22 data23
data31 data32 data33
{% endhighlight %}

print命令会自动将OFS变量的值放置在输出的每个字段间。通过设置OFS变量，你可以在输出中使用任意字符（串）来分隔字段：
{% highlight string %}
# gawk 'BEGIN{FS=","; OFS="-"} {print $1,$2,$3}' data1
data11-data12-data13
data21-data22-data23
data31-data32-data33
# gawk 'BEGIN{FS=","; OFS="--"} {print $1,$2,$3}' data1
data11--data12--data13
data21--data22--data23
data31--data32--data33
# gawk 'BEGIN{FS=","; OFS="<-->"} {print $1,$2,$3}' data1
data11<-->data12<-->data13
data21<-->data22<-->data23
data31<-->data32<-->data33
{% endhighlight %}


```FIELDWIDTHS```变量允许你读取数据行，而不用字段分隔符来划分字段。在一些应用程序中，不用字段分隔符，数据是被放置在数据行的某些列中的。这种情况下，你必须设定```FIELDWIDTHS```变量来匹配数据在数据行中的位置。

一旦设置了```FIELDWIDTHS```变量，gawk就会忽略FS变量，而根据提供的字段宽度大小来计算字段。下面是个采用字段宽度而非字段分隔符的例子：
{% highlight string %}
# cat data2
1005.3247596.37
115-2.349194.00
05810.1298100.1
# gawk 'BEGIN{FIELDWIDTHS="3 5 2 5"} {print $1,$2,$3,$4}' data2
100 5.324 75 96.37
115 -2.34 91 94.00
058 10.12 98 100.1
{% endhighlight %}


## 1. awk

向awk传递参数：
{% highlight string %}
for i in sda sdb sdc sdd sde sdf sdg sdh sdi sdj sdk; do path=`udevadm info -q path -n /dev/$i`; udevadm info -q env -p $path | grep ID_WWN= | awk 'BEGIN{FS="="} {print disk,"win-"$2}' disk=$i;done >> ./disk-id.txt
{% endhighlight %}







<br />
<br />

**[参看]**

1. [Linux下Shell的for循环语句](https://www.cnblogs.com/EasonJim/p/8315939.html)




<br />
<br />
<br />


