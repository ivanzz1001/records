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

在上面我们```gawk基础```中我们演示了gawk中的一种内建变量类型————数据字段变量。数据字段变量允许你使用美元符号( ```$``` )和数据字段在数据行中的位置对应的数值来引用该数据行中的字段。因此，要引用数据行中的第一个数据字段，就用变量$1； 要引用第二个字段，就用$2；依次类推。

字段是由字段分隔符来划定的。默认情况下，字段分隔符是一个空白符，也就是空格符或者制表符(tab)。在上面```gawk基础```中，我们讲了如何在命令行下使用命令行参数```-F```或者在gawk程序中使用特殊的内建变量FS来更改字段分隔符。

内建变量FS是控制gawk如何处理输入输出数据中的字段和数据行的一组变量中的一个。下面列出了该组内建变量：
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

```FIELDWIDTHS```变量定义了4个字段，gawk依此来解析数据行。每个数据行中用以表示数字的字符串根据定义好的字段宽度值来分割。
<pre>
警告： 一定要记住，一旦设定了FIELDWIDTHS变量的值，就不能改变了。这种方法并不适用于变长的字段
</pre>

变量```RS```和```ORS```定义了gawk程序如何处理数据流中的数据行。默认情况下，gawk将RS和ORS设为换行符。默认的RS值表明，输入数据流中的每行新文本就是一个新数据行。


有时，你会碰到在数据流中字段占了多行的情况。经典的例子是包含地址和电话号码的数据，其中地址和电话号码各占一行：
{% highlight string %}
Riley Mullen
123 Main Street
Chicago. IL 60601
(312)555-1234
{% endhighlight %}
如果你用默认的FS和RS变量值来读取这组数据，gawk 就会把每行误读为一个单独的数据行，并把数据行中的每个空格当做字段分隔符。这绝非你想要的。

要解决这个问题，只需把FS变量设置成换行符。这就表明数据流中的每行都是一个单独的字段，每行上的所有数据都属于同一个字段。但现在令你头疼的是无从判断一个新的数据行从何开始。

要解决这个问题，只需把RS变量设置成空字符串，然后在数据行间留一个空白行。gawk会把每个空白行当作一个数据行分隔符。下面就是个使用这种方法的例子：
{% highlight string %}
# cat data2
Riley Mullen
123 Main Street
Chicago. IL 60601
(312)555-1234

Frank Williams
456 Oak Streat
Indianapolis. IN 46201
(317)555-9876

Haley Snell
4231 Elm Streat 
Detroit. MI 48201
(313)555-4938
# gawk 'BEGIN{FS="\n"; RS=""} {print $1,"\t", $4}' data2
Riley Mullen     (312)555-1234
Frank Williams   (317)555-9876
Haley Snell      (313)555-4938
{% endhighlight %}
太好了，现在gawk把文件中的每行都当成一个字段，把空白行当做数据行分隔符。

* 数据变量

除了字段和数据行分隔符变量外，gawk还提供了一些其他的内建变量来帮助你了解数据发生了什么变化并提取shell环境信息。下表列出了gawk中的其他内建变量：
<pre>
         更多的gawk内建变量

 变 量                   描述
--------------------------------------------------------------------
ARGC            当前命令行参数个数
ARGIND          当前文件在ARGV中的位置
ARGV            包含命令行参数的数组
CONVFMT         数字的转换格式（参见printf语句）；默认值为%.6g
ENVIRON         当前shell环境变量及其值组成的关联数组
ERRNO           当读取或关闭输入文件发生错误时的系统错误号
FILENAME        用作gawk输入数据的数据文件的文件名
FNR             当前数据文件中处理过的数据行数
IGNORECASE      设成非零值时，忽略gawk命令中出现的字符串的字符大小写
NF              数据文件中的字段总数
NR              已处理的输入数据行总数目
OFMT            数字的输出格式；默认值为%.6g
RLENGTH         由match函数所匹配的子字符串的长度
RSTART          由match函数所匹配的子字符串的起始位置
</pre>
你应该能从shell脚本编程中认识上面的一些变量。ARGC和ARGV变量允许从shell中获得命令行参数的总数以及它们的值。但这可能有点麻烦，因为gawk并不会将程序脚本当成命令行参数的一部分：
{% highlight string %}
# cat data1
line 1
line 2
# gawk -v lines=2 'BEGIN{print ARGC; for(i=0;i<ARGC;i++) print ARGV[i];}' data1
2
gawk
data1
{% endhighlight %}
ARGC变量表明命令行上有两个参数。这包括gawk命令和data1参数（记住，程序脚本并不算参数）。ARGV数组从代表该命令的索引0开始。第一个数组值是gawk命令后的第一个命令行参数。
<pre>
说明： 注意，跟shell变量不同，在脚本中引用gawk变量时，变量名前不加美元符
</pre>
```ENVIRON```变量看起来可能有点陌生。它使用关联数组来提取shell环境变量。关联数组用文本作为数组的索引值，而不用数值。

数组索引中的文本是shell环境变量，而数组的值则是shell环境变量的值。下面有个例子：
{% highlight string %}
# gawk 'BEGIN{
> print ENVIRON["HOME"]
> print ENVIRON["PATH"]
> }'
/root
/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/root/bin
{% endhighlight %}
ENVIRON["HOME"]变量从shell中提取了HOME环境变量的值。类似地，ENVIRON["PATH"]提取了PATH环境变量的值。你可以用这种方法来从shell中提取任何环境变量的值来在gawk程序中使用。

当你要在gawk程序中记录数据字段和数据行时，FNR、NF和NR变量就能派上用场。有时你不知道数据行中到底有多少个数据字段。NF变量允许你指定数据行中的最后一个数据字段，而不用知道它的具体位置：
{% highlight string %}
# gawk 'BEGIN{FS=":"; OFS=":"} {print $1,$NF}' /etc/passwd
root:/bin/bash
bin:/sbin/nologin
daemon:/sbin/nologin
adm:/sbin/nologin
lp:/sbin/nologin
sync:/bin/sync
shutdown:/sbin/shutdown
halt:/sbin/halt
mail:/sbin/nologin
operator:/sbin/nologin
games:/sbin/nologin
...
{% endhighlight %}

NF变量含有数据文件中最后一个数据字段的数字值。你可以在它前面加个美元符将它用作字段变量。

FNR和NR变量彼此类似，但略有不同。FNR变量含有处理过的当前数据文件中的数据行总数，而NR变量则含有处理过的所有数据行总数。让我们看几个例子来了解一下这个差别：
{% highlight string %}
# cat data1
data11 data12 data13 data14 data15
data21 data22 data23 data24 data25
data31 data32 data33 data34 data35
# gawk '{print $1, "FNR="FNR}' data1 data1
data11 FNR=1
data21 FNR=2
data31 FNR=3
data11 FNR=1
data21 FNR=2
data31 FNR=3
{% endhighlight %}
在这个例子中，gawk程序的命令行定义了两个输入文件（它两次指定了同样的输入文件）。这个脚本会打印第一个数据字段的值和FNR变量的当前值。注意，当gawk程序处理第二个数据文件时，FNR值被设回1了。

现在，让我们加上NR变量看看会输出什么：
{% highlight string %}
# cat data1
data11 data12 data13 data14 data15
data21 data22 data23 data24 data25
data31 data32 data33 data34 data35
# gawk '{print $1, "FNR="FNR, "NR="NR} END{print "There were", NR, "records processed"} ' data1 data1
data11 FNR=1 NR=1
data21 FNR=2 NR=2
data31 FNR=3 NR=3
data11 FNR=1 NR=4
data21 FNR=2 NR=5
data31 FNR=3 NR=6
There were 6 records processed
{% endhighlight %}
FNR变量的值在gawk处理第二个数据文件时被重置了，而NR变量则在进入第二个数据文件后继续计数。结果是，如果只使用一个数据文件作为输入，那么FNR和NR的值将会相同。如果使用多个数据文件作为输入，那么FNR的值会在处理每个数据文件时被重置，而NR的值则会继续计数直到处理完所有的数据文件。
<pre>
说明： 在使用gawk时你可能会注意到，gawk脚本通常会比shell脚本中其他部分还要大一些。在上面的例子中，为了简单起见，我们利用shell
的多行功能直接在命令行上运行了gawk脚本。当你在shell脚本中使用gawk时，你应该将不同的gawk命令放到不同的行，这样会比较容易阅读和
理解，而不要在shell脚本中将所有的命令都塞到同一行。还有，如果你发现在不同的shell脚本中用到了同样的gawk脚本，记得将这段gawk脚本
放到一个单独的文件中，并用-f参数来在shell脚本中引用它。
</pre>

2） **自定义变量**

跟任何其他经典编程语言一样，gawk允许你定义自己的变量来在程序代码中使用。gawk自定义变量名可以是任意数目的字母、数字和下划线，但不能以数字开头。还有，要记住gawk变量名区分大小写。

* 在脚本中给变量赋值

在gawk程序中给变量赋值跟在shell脚本中赋值类似，都用```赋值语句```:
{% highlight string %}
# gawk 'BEGIN{testing="This is a test"; print testing}'
This is a test
{% endhighlight %}
print语句的输出是testing变量的当前值。跟shell脚本变量一样，gawk变量可以保存数值和文本值：
{% highlight string %}
# gawk 'BEGIN{
> testing="This is a test"
> print testing
> testing=45
> print testing
> }' 
This is a test
45
{% endhighlight %}
在上面这个例子中，testing变量的值会从文本值变成数值。赋值语句还可以包含数学算式来处理数字值：
{% highlight string %}
gawk 'BEGIN{x=4; x = x * 2 + 3; print x}'
11
{% endhighlight %}
如你在这个例子中看到的，gawk编程语言包含了用来处理数字值的标准数学操作符。其中包括求余符号(%)和幂运算符号(```^或**```)

* 在命令行上给变量赋值

你也可以用gawk命令行来给程序中的变量赋值。这允许你在普通代码的外面赋值，即时改变变量的值。这里有个使用命令行变量来显示文件中特定数据字段的例子：
{% highlight string %}
# cat script1 
BEGIN{
FS="."
}
{
print $n
}
# gawk -f ./script1 n=2 data1
data12
data22
data32

//例子2
# for i in {1..9}; do echo $i | gawk '{print disk, "\t", $0}' disk=disk_$i; done
disk_1   1
disk_2   2
disk_3   3
disk_4   4
disk_5   5
disk_6   6
disk_7   7
disk_8   8
disk_9   9
{% endhighlight %}
这个特性允许你改变脚本的行为而不需要修改实际的脚本代码。第一个例子显示了文件的第二个数据字段， 而第二个例子传递了```disk_$i```参数。

使用命令行参数来定义变量值会有个问题。在你设置了变量后，这个值在代码的```BEGIN```部分不可用：
{% highlight string %}
# cat data1
data11,data12,data13,data14,data15
data21,data22,data23,data24,data25
data31,data32,data33,data34,data35

# cat script2
BEGIN{
print "The starting value is", n
FS=","
}
{
print $n
}

# gawk -f ./script2 n=3 data1
The starting value is 
data13
data23
data33
{% endhighlight %}
你可以使用```-v```命令行参数来解决这个问题。它允许你指定在BEGIN代码部分之前设定的变量。在命令行上，```-v```命令行参数必须放在脚本代码之前。
{% highlight string %}
# gawk -v n=3 -f ./script2 data1
The starting value is 3
data13
data23
data33
{% endhighlight %}
现在n变量在BEGIN代码部分中已经含有命令行上设的值了。

### 2.2 处理数组
许多编程语言都提供数组来在单个变量中存储多个值。gawk编程语言使用```关联数组```来提供数组功能。

关联数组跟数字数组不同之处在于它的索引值可以是任意文本字符串。你不需要用连续的数字来标识数组中的数据元素。相反，关联数组用各种字符串来引用值。每个索引字符串都必须是唯一的，并唯一地标识赋给它的数据元素。如果你熟悉其他编程语言的话，这跟哈希表和字典是同一个概念。

后面几节将会带你逐步熟悉在gawk程序中使用关联数组。

1） **定义数组变量**

你可以用标准赋值语句来定义数组变量。数组变量赋值的格式如下：
{% highlight string %}
var[index] = element
{% endhighlight %}
其中```var```是变量名，index是关联数组的索引值，element是数据元素值。这里有些gawk中数组变量的例子：
<pre>
capital["Illinois"] = "Springfield"
capital["Indiana"] = "Indianapolis"
capital["Ohio"] = "Columbus"
</pre>
在引用数组变量时，必须包含索引值来提取相应的数据元素值：
{% highlight string %}
# gawk 'BEGIN{
> capital["Illinois"] = "Springfield"
> print capital["Illinois"]
> }'
Springfield
{% endhighlight %}
在引用数组变量时，数据元素的值会出现。数据元素值是数字值时也一样：
{% highlight string %}
# gawk 'BEGIN{
> var[1]=34
> var[2]=3
> total = var[1] + var[2]
> print total
> }'
37
{% endhighlight %}
如你在这个例子中看到的，可以像使用gawk程序中的其他变量一样使用数组变量。

2） **遍历数组变量**

关联数组变量的问题在于你可能无法知晓索引值是什么。跟使用连续数字作为索引值的数字数组不同，关联数组的索引可以是任何东西。

如果要在gawk中遍历一个关联数组，你可以用for语句的一种特殊形式：
{% highlight string %}
for (var in array)
{
	statements
}
{% endhighlight %}

这个for语句会在每次将关联数组array的下一个索引值赋给变量var时，执行一遍statements。重要的是记住这个变量是索引值而不是数组元素值。你可以将这个变量用作数组的索引，轻松的取出数据元素值：
{% highlight string %}
# gawk 'BEGIN{
> var["a"] = 1
> var["g"] = 2
> var["m"] = 3
> var["u"] = 4
>
> for (test in var)
> {
> print "index:", test, " - value:", var[test];
> }
> }'
index: u  - value: 4
index: m  - value: 3
index: a  - value: 1
index: g  - value: 2
{% endhighlight %}

注意，索引值不会按任何特定顺序返回，但它们每个都会有个对应的数据元素值。明白这点很重要，因为你不能指望着返回的值都是按顺序的，你只能确定索引值和数据值是对应的。

3) **删除数组变量**

从关联数组中删除数组索引要用一个特别的命令：
<pre>
delete array[index]
</pre>
删除命令会从数组中删除关联索引值和相关的数据元素值。
{% highlight string %}
# gawk 'BEGIN{
> var["a"] = 1
> var["g"] = 2
> for (test in var)
> {
> print "index:", test, " - value:", var[test];
> }
> delete var["g"]
> 
> print "---------------------"
> for (test in var)
> {
> print "index:", test, " - value:", var[test];
> }
> }'
index: a  - value: 1
index: g  - value: 2
---------------------
index: a  - value: 1
{% endhighlight %}
一旦从关联数组中删除了索引值，你就没法再提取它了。

### 2.3 使用模式
gawk程序支持几种类型的匹配模式来过滤数据行，跟sed编辑器大同小异。在```gawk基础```中已经介绍了使用中的两种特别模式，BEGIN和END关键字是用来在读取数据流之前或之后执行命令的两种特殊模式。类似地，你可以创建其他模式来在数据流中出现匹配数据时执行一些命令。

本节将会演示如何在gawk脚本中用匹配模式来限定程序脚本作用在哪些数据行上。

1） **正则表达式**

我们可以用基本正则表达式(BRE)或扩展正则表达式(ERE)来过滤程序脚本作用在数据流中哪些行上。

在使用正则表达式时，正则表达式必须出现在它要控制的程序脚本的左花括号前：
{% highlight string %}
# cat data1
data11,data12,data13,data14,data15
data21,data22,data23,data24,data25
data31,data32,data33,data34,data35
# gawk 'BEGIN{FS=","} /11/{print $1}' data1
data11
{% endhighlight %}

正则表达式```/11/```匹配了数据字段中含有字符串```11```的数据行。gawk程序会用正则表达式对数据行中所有的数据字段进行匹配，包括字段分隔符：
{% highlight string %}
# gawk 'BEGIN{FS=","} /.d/{print $1}' data1
data11
data21
data31
{% endhighlight %}
这个例子在正则表达式中匹配了用作字段分隔符的```逗号```。这也并不总是好的，它可能会造成试图匹配某个数据字段中的特定数据，而这些数据也可能出现在其他数据字段中。如果需要用一个正则表达式来对一个特定数据实例进行匹配，你应该使用匹配操作符。


2) **匹配操作符**

匹配操作符(matching operator)允许将正则表达式限定在数据行中的特定数据字段。匹配操作符是波浪线(~)。你要一起指定匹配操作符、数据字段变量以及要匹配的正则表达式：
{% highlight string %}
$1 ~ /^data/
{% endhighlight %}
```$1```变量代表数据行中第一个数据字段。这个表达式会过滤出第一个字段以文本data开头的所有数据行。下面是在gawk程序脚本中使用匹配操作符的例子：
{% highlight string %}
# cat data1
data11,data12,data13,data14,data15
data21,data22,data23,data24,data25
data31,data32,data33,data34,data35
# gawk 'BEGIN{FS=","} $2 ~ /^data2/ {print $0}' data1
data21,data22,data23,data24,data25
{% endhighlight %}

匹配操作符会用正则表达式```/^data2/```来匹配第二个数据字段，该正则表达式指明字符串要以文本data2开头。

这是个gawk程序脚本中常用的在数据文件中查找特定元素的强大工具：
{% highlight string %}
# gawk -F: '$1 ~ /root/ {print $1, $NF}' /etc/passwd
root /bin/bash
{% endhighlight %}
这个例子会在第一个数据段中查找文本root。当它在数据行中找到这个模式时，它会打印数据行的第一个和最后一个数据字段值。

你也可以用```!符号```来排除正则表达式的匹配：
{% highlight string %}
$1 !~ /expression/
{% endhighlight %}
如果数据行中没有找到匹配正则表达式的文本，那程序脚本就会作用到数据行数据：
{% highlight string %}
# gawk -F: '$1 !~ /root/ {print $1,$NF}' /etc/passwd
daemon /usr/sbin/nologin
bin /usr/sbin/nologin
sys /usr/sbin/nologin
sync /bin/sync
games /usr/sbin/nologin
man /usr/sbin/nologin
...
{% endhighlight %}
在这个例子中，gawk程序脚本会打印/etc/passwd文件中所有不匹配用户ID root的数据行的用户ID和登陆shell。


3) **数学表达式**

除了正则表达式，你也可以在匹配模式中用数学表达式。这个功能在匹配数据字段中的数字值时非常有用。举个例子，如果你想显示所有属于root用户组（组ID为0）的系统用户，你可以用这个脚本：
{% highlight string %}
# gawk -F: '$4 == 0 {print $1}' /etc/passwd
root
{% endhighlight %}
这段脚本会查看第4个数据字段含有值0的数据行。在这个Linux系统中，有1个用户属于root用户组。

你可以使用任意的普通数学比较表达式：
<pre>
x == y: 值x等y

x <= y: 值x小于等于y

x < y: 值x小于y

x >= y: 值x大于等于y

x > y: 值x大于y
</pre>
也可以对文本数据使用表达式，但必须小心。跟正则表达式不同，表达式必须完全匹配。数据必须跟模式正好匹配：
{% highlight string %}
# cat data1
data11,data12,data13,data14,data15
data21,data22,data23,data24,data25
data31,data32,data33,data34,data35

# gawk -F, '$1 == "data" {print $1}' data1
# gawk -F, '$1 == "data11" {print $1}' data1
data11
{% endhighlight %}
第一个测试没有匹配任何数据行，因为第一个数据字段的值不是任何数据行中的数据。第二个测试用值data11匹配了一个数据行。

### 2.4 结构化命令
gawk编程语言支持常见的结构化编程命令。本节将会介绍每个命令并演示如何在gawk编程环境中使用它们。

1) **if语句**

gawk编程语言支持标准的if-then-else格式的if语句。你必须为if语句定义一个评估条件，并将其用圆括号括起来。如果条件评估为TRUE，紧跟在if语句后的语句会执行。如果条件评估为FALSE，那这条语句就会被跳过。可以用这种格式：
<pre>
if(condition)
	statement1
</pre>
或者你可以将它放在一行上，像这样：
<pre>
if(condition) statement1
</pre>
这里有个演示这种格式的简单的例子：
{% highlight string %}
# cat data4
10
5
13
50 
34
# gawk '{if($1 > 20) print $1}' ./data4
50
34
{% endhighlight %}
并不复杂。如果需要在if语句中执行多条语句，你必须用花括号将它们括起来：
{% highlight string %}
# gawk '{
> if($1 > 20)
> {
> x = $1 * 2
> print x
> }
> }' data4
100
68
{% endhighlight %}
gawk的if语句也支持else子句，允许在if条件不成立的情况下执行一条或多条语句。这里有个使用else子句的例子：
{% highlight string %}
# gawk '{
> if($1 > 20)
> {
> x = $1 * 2
> print x
> }
> else{
> x = $1 / 2
> print x
> }
> }' data4
5
2.5
6.5
100
68
{% endhighlight %}
你可以在单行上使用else子句，但必须在if语句部分之后使用分号：
<pre>
if(condition) statement1; else statement2
</pre>
这里是上一个例子的单行格式版本：
{% highlight string %}
# gawk '{if($1 > 20) print $1 * 2; else print $1 /2}' data4
5
2.5
6.5
100
68
{% endhighlight %}

这个格式更紧凑，但也更难理解。

2) **while语句**

while语句为gawk程序提供了一个基本的循环功能。下面是while语句的格式：
<pre>
while(condition)
{
	statements
}
</pre>
while循环允许遍历一组数据，并检查结束迭代的条件。在计算中必须使用每个数据行中的多个数据值时，它能帮得上忙：
{% highlight string %}
# tee data5 << EOF
> 130 120 135
> 160 113 140
> 145 170 215
> EOF
130 120 135
160 113 140
145 170 215

# gawk '{
> total = 0
> i = 1
> while(i<4)
> {
>     total += $i
>     i++
> }
> avg = total / 3
> print "Average:", avg
> }' data5
Average: 128.333
Average: 137.667
Average: 176.667
{% endhighlight %}
while语句会遍历数据行中的数据字段，将每个值都加到total变量上，然后将计数器变量i增一。当计数器值等于4时，while的条件编程FALSE，循环结束，然后会执行脚本中的下条命令。那条语句会计算平均值，然后平均值会打印出来。这个过程会为数据文件中的每个数据行不断重复。

gawk编程语言支持在while循环中使用break和continue语句，允许从循环中跳出：
{% highlight string %}
# gawk '{
> total=0
> i = 1
> while(i<4)
> {
>     total += $i;
>     if(i == 2)
>        break;
>     i++;
> }
> avg = total /2
> print "The average of the first two data elements is:", avg
> }' data5
The average of the first two data elements is: 125
The average of the first two data elements is: 136.5
The average of the first two data elements is: 157.5
{% endhighlight %}
break语句用来在i变量的值为2时从while循环中跳出。

3) **do-while语句**

do-while语句类似于while语句，但会在检查条件语句之前执行命令。下面是do-while语句的格式：
{% highlight string %}
do{
   statements
}while(condition)
{% endhighlight %}

这种格式保证了语句会在条件被评估之前至少执行一次。当你需要在条件被评估之前执行一些语句时这非常有用：
{% highlight string %}
# gawk '{
> total = 0
> i = 1
> do{
>   total += $i
>   i++
> }while(total < 150)
> print "total:", total
> }' data5
total: 250
total: 160
total: 315
{% endhighlight %}

这个脚本会从每个数据行读取数据字段并将它们加在一起，直到累加结果达到150。如果第一个数据字段大于150（如在第二个数据行中看到的），则脚本会保证在条件被评估前至少读取第一个数据字段。

4） **for语句**

for语句是许多编程语言用来做循环的常见方法。gawk编程语言支持C风格的for循环：
<pre>
for(variable assignment; condition; iteration process)
</pre>
它帮助将几个功能合并到一个语句中来简化循环：
{% highlight string %}
# gawk '{
> total = 0
> for(i=1;i<4;i++)
> {
>   total += $i;
> }
> avg = total / 3
> print "Average:", avg
> }' data5
Average: 128.333
Average: 137.667
Average: 176.667
{% endhighlight %}
定义了for循环中的迭代计数器，你就不用担心要像使用while语句一样自己负责给计数器增一。

### 2.5 格式化打印
你可能已经注意到了```print```语句在gawk如何显示数据上并未提供多少控制。你能做的大概只是控制输出字段分隔符(OFS)。如果你正在创建详细报告，通常你需要将数据按特定的格式放到特定的位置。

解决办法是使用格式化打印命令，称为```printf```。如果你熟悉C语言编程的话，gawk中的printf命令用法一样，允许指定具体的如何显示数据的指令。

下面是printf命令的格式：
<pre>
printf "format string", var1, var2, ...
</pre>
format string是格式化输出的关键。它会用文本元素和```格式化控制符```来具体指定如何呈现格式化输出。格式化控制符是一种特殊的代码，它会指明什么类型的变量可以显示以及如何显示。gawk程序会将每个格式化控制符作为命令中列出的每个变量的占位符使用。第一个格式化控制符会匹配列出的第一个变量，第二个会匹配第二个变量，依次类推。

格式化控制符采用如下格式：
<pre>
%[modifier]control-letter
</pre>
其中control-letter是指明显示什么类型数据值的单字符码，而modifier定义了另一个可选的格式话特性。下表列出了可用在格式化控制符中的控制字母：
<pre>
                 表： 格式化控制符的控制字母

控制字母                      描述
------------------------------------------------------------------------
  c                 将一个变量作为ASCII字符显示
  d                 显示一个整数值
  i                 显示一个整数值（与d一样）
  e                 用科学计数法显示一个数
  f                 显示一个浮点值
  g                 用科学计数法或浮点数中较短的显示
  o                 显示一个八进制值
  s                 显示一个文本字符串
  x                 显示一个十六进制值
  X                 显示一个十六进制值，但用大写字母A~F
</pre>
因此，如果你需要显示一个字符串变量，你可以用格式化控制符```%s```；如果你需要显示一个整数值，你可以用```%d```或者```%i```(%d是C风格中用来显示十进制数的）。如果你要用科学计数法显示很大的值，你会用```%e```格式化控制符：
{% highlight string %}
# gawk 'BEGIN{
> x = 10 * 100
> printf "The answer is: %e\n", x
> }'
The answer is: 1.000000e+03
{% endhighlight %}
除了控制字母外，还有3种修饰符可以用来进一步控制输出：

* width： 指定了输出字段最小宽度的数字值。如果输出短于这个值，printf会向右对齐，并用空格来填充这段空间。如果输出比指定的宽度还要长，它会覆盖width值

* prec: 指定了浮点数中小数点后面位数的数字值，或者文本字符串中显示的最大字符数；

* -(减号): 减号指明在向格式化空间中放入数据时采用左对齐而不是右对齐

在使用printf语句时，你对输出如何呈现有着完全的控制权。举个例子，在前面我们用```print```命令来显示数据行中的数据字段：
{% highlight string %}
# cat data2
Riley Mullen
123 Main Street
Chicago. IL 60601
(312)555-1234

Frank Williams
456 Oak Streat
Indianapolis. IN 46201
(317)555-9876

Haley Snell
4231 Elm Streat 
Detroit. MI 48201
(313)555-4938
# gawk 'BEGIN{FS="\n"; RS=""} {print $1,"\t", $4}' data2
Riley Mullen     (312)555-1234
Frank Williams   (317)555-9876
Haley Snell      (313)555-4938
{% endhighlight %}

你可以用```printf```命令来帮助格式化输出，使得输出看起来好一些。首先，让我们将print命令换成printf命令并看看那么做会怎样：
{% highlight string %}
# gawk 'BEGIN{FS="\n"; RS=""} {printf "%s\t%s\n",$1,$4}' data2
Riley Mullen    (312)555-1234
Frank Williams  (317)555-9876
Haley Snell     (313)555-4938
{% endhighlight %}
它会产生跟print命令相同的输出。printf命令用```%s```格式化控制符来作为这两个字符串值占位符。

注意你需要在printf命令的末尾手动添加换行符来生成新行。没加的话，printf命令会继续用同一行来打印后续输出。

如果你需要用几个单独的printf命令来在同一行上打印多个输出，它会非常有用：
{% highlight string %}
# cat data1
data11,data12,data13,data14,data15
data21,data22,data23,data24,data25
data31,data32,data33,data34,data35

# gawk 'BEGIN{FS=","} {printf "%s ", $1} END{printf "\n"}' data1
data11 data21 data31 
{% endhighlight %}
每个printf的输出都会出现在同一行上。为了终止该行，END部分打印了一个换行符。

下一步，让我们用修饰符来格式化第一个字符串值：
{% highlight string %}
# gawk 'BEGIN{FS=","; RS=""} {printf "%16s  %s\n", $1, $4}' data2
    Riley Mullen  (312)555-1234
  Frank Williams  (317)555-9876
     Haley Snell  (313)555-4938
{% endhighlight %}
通过添加一个值16的修饰符，我们强制第一个字符串的输出采用16位字符。默认情况下，printf命令使用右对齐来将数据放到格式化空间中。要改成左对齐，只要给修饰符加一个减号就行了：
{% highlight string %}
# gawk 'BEGIN{FS=","; RS=""} {printf "%-16s  %s\n", $1, $4}' data2
Riley Mullen      (312)555-1234
Frank Williams    (317)555-9876
Haley Snell       (313)555-4938
{% endhighlight %}
现在看起来专业多了。

printf命令在处理浮点值时也非常有用。通过为变量指定一个格式，你可以让输出看起来更统一：
{% highlight string %}
# cat data5
130 120 135
160 113 140
145 170 215
# gawk '{
> total = 0
> for(i=1;i<4;i++)
> {
>    total += $i;
> }
> avg = total / 3
> printf "Average: %5.1f\n", avg
> }' data5
Average: 128.3
Average: 137.7
Average: 176.7
{% endhighlight %}
使用```%5.1f```格式指定符，你可以强制printf命令将浮点值近似到小数点后1位。

### 2.6 内建函数
gawk编程语言提供了一些内置函数，可进行一些常见的数学、字符串以及时间函数运算。你可以在gawk程序中利用这些函数来减少脚本中的编码工作。本节将会带你逐步熟悉gawk中这些不同的内建函数。

1） **数学函数**

如果你用任意类型的语言编过程，那么你可能会很熟悉在代码中使用内建函数来进行一些常见的数学函数运算。gawk编程语言不会让这些想借助高级数学功能降低编码量的人失望。

下面列出了gawk中内建的数学函数：
<pre>
         表： gawk数学函数

 函 数                     描述
atan2(x,y)         x/y的反正切，x和y以弧度为单位
cos(x)             x的余弦，x以弧度为单位
exp(x)             x的指数函数
int(x)             x的整数部分，取靠近零一侧的值
log(x)             x的自然对数
rand()             比0大比1小的随机浮点值
sin(x)             x的正弦，x以弧度为单位
sqrt(x)            x的平方根
srand(x)           为计算随机数指定一个种子值
</pre>
虽然并未提供很多数学函数，但gawk提供了标准数学运算中要用到的一些基本元素。int()函数会生成一个值的整数部分，但它并不会四舍五入取近似值。它的做法更像其他编程语言中的floor()函数。它会生成该值和0之间最接近该值的整数。

这意味着int()函数在值为5.6时返回5，而在值为-5.6时则返回-5。

rand()函数非常适合于创建随机数，但你需要用点技巧才能得到有意义的值。rand()函数会返回一个随机数，但这个随机数只在0和1之间（不包括0或1）。要得到更大的数，你就需要放大返回值。

产生较大整数随机数的常见方法是用rand()函数和int()函数创建一个算法：
{% highlight string %}
x = int(10 * rand())
{% endhighlight %}
这会返回一个0~9（包括0和9）的随机整数值。只要为你的程序用上限值替换掉等式中的10就可以了。

在使用一些数学函数时要小心，因为gawk语言有个它能处理的数值的限定区间。如果超出了这个区间，就会得到一条错误消息：
{% highlight string %}
# gawk 'BEGIN{x=exp(100); print x}'
26881171418161356094253400435962903554686976
# gawk 'BEGIN{x=exp(1000); print x}'
gawk: cmd. line:1: warning: exp: argument 1000 is out of range
inf
{% endhighlight %}
第一个例子会计算e的100次幂，虽然很大但尚在系统的区间内。第二个例子尝试计算e的1000次幂，它已经超出了系统的数值区间，所以产生了一条错误消息。

除了标准数学函数外，gawk还支持一些按位操作数据的函数：

* and(v1,v2): 执行值v1和v2的按位与运算

* compl(val): 执行val的补运算

* lshift(val,count): 将值val左移count位

* or(v1,v2): 执行值v1和v2的按位或运算

* rshift(val,count): 将值val右移count位

* xor(v1,v2): 执行值v1和v2的按位异或运算。

位操作函数在处理数据中的二进制值时非常有用。

2) **字符串函数**

gawk编程语言还提供了一些可用来处理字符串值的函数，如下表所示：
<pre>
                  gawk字符串函数

 函 数                            描述
-----------------------------------------------------------------------------------------------------
asort(s [,d])          将数组s按数据元素值排序。索引值会被替换成表示新的排序顺序的连续数字。另外，如果指定了d，则
                       排序后的数组会存储在数组d中

asorti(s [,d])         将数组s按索引值排序。生成的数组会将索引值作为数据元素值，用连续数字索引来表明排序顺序。另外，
                       如果指定了d，排序后的数组会存储在数组d中

gensub(r, s, h [,t])   查找变量$0或目标字符串t（如果提供了的话）来匹配正则表达式r。如果h是一个以g或G开头的字符串，就
                       用s替换掉匹配的文本。如果h是一个数字，它表示要替换掉第几处r匹配的地方

gsub(r, s [,t])        查找变量$0或目标字符串t（如果提供了的话）来匹配正则表达式r。如果找到了，就全部替换成字符串s

index(s,t)             返回字符串t在字符串s中的索引值；如果没有找到的话返回0

length([s])            返回字符串s的长度；如果没有指定的话，返回$0的长度

match(s, r [,a])       返回字符串s中正则表达式r出现位置的索引。如果指定了数组a，它会存储s中匹配正则表达式的那部分

split(s, a [,r])       将s用FS字符或正则表达式r（如果提供了的话）分开放到数组a中，返回字段的总数

sprintf(format, variables)  用提供的format和variables返回一个类似于printf输出的字符串

sub(r,s [,t])          在变量$0或目标字符串t中查找正则表达式r的匹配。如果找到了，就用字符串s替换掉第一处匹配

substr(s, i [,n])      返回s中从索引值i开始的n个字符组成的子字符串。如果未提供n，则返回s剩下的部分。

tolower(s)             将s中的所有字符串转换成小写

toupper(s)             将s中的所有字符串转换成大写
</pre>
一些字符串函数相对来说显而易见：
{% highlight string %}
# gawk 'BEGIN{x="testing"; print toupper(x); print length(x)}'
TESTING
7
{% endhighlight %}
但一些字符串函数会相当复杂。asort()和asorti()函数是新加的gawk函数，允许你基于数据元素值(asort)或索引值(asorti)对数组变量进行排序。这里有个使用asort()的例子：
{% highlight string %}
# gawk 'BEGIN{
> var["a"] = 1
> var["u"] = 4
> var["m"] = 3
> var["g"] = 2
> asort(var,test)
> for(i in test)
> {
>     print "index:"i, " - value:"test[i]
> }
> }'
index:1  - value:1
index:2  - value:2
index:3  - value:3
index:4  - value:4
{% endhighlight %}
新数组test含有排序后的原数组中的数据元素，但索引值现在变为表明正确顺序的数字值了。

split函数是将数据字段放到数组中以进一步处理的好办法：
{% highlight string %}
# gawk 'BEGIN{FS=","} {
> split($0,var)
> print var[1], var[5]
> }' data1
data11 data15
data21 data25
data31 data35
{% endhighlight %}
新数组使用连续数字作为数组索引，从含有第一个数据字段的索引值1开始。


3） **时间函数**

gawk编程语言包含一些函数来帮助处理时间值，如下表所示：
<pre>
         表： gawk的时间函数

函 数                            描述
--------------------------------------------------------------------------------------
mktime(datespec)             将一个按YYYY MM DD HH MM SS [DST]格式指定的日期转换成时间戳值

strftime(format [,timestamp] 将当前时间的时间戳或timestamp(如果提供了的话）转化成shell函数格式
                             date()的格式化日期

systime()                    返回当前时间的时间戳
</pre>
时间函数通常用来处理日志文件，日志文件通常含有需要进行比较的日期。通过将日期的文本表示转换成epoch时间（自1970-01-01 00:00:00 UTC到现在的秒数），你可以轻松的比较日期。

下面是个在gawk程序中使用时间函数的例子：
{% highlight string %}
# gawk 'BEGIN{
> date = systime()
> day = strftime("%A, %B %d, %Y", date)
> print day
> }'
Tuesday, April 09, 2019
{% endhighlight %}
这个例子用systime函数来从系统获取当前的epoch时间戳，然后用strftime()函数来将它转换成人类可读的方式，转换过程中使用了shell的date命令的日期格式化符。


### 2.7 自定义函数
你并未被限定只能用gawk的内建函数。你可以在gawk程序中创建自定义函数。本节将会介绍如何在gawk程序中定义和使用自定义函数。

1) **定义函数**

要定义自己的函数，你必须用function关键字：
{% highlight string %}
function name([variables])
{
	statements
}
{% endhighlight %}

函数名必须能够唯一标识函数。你可以在调用的gawk程序中传递给这个函数一个或多个变量：
{% highlight string %}
function printthird()
{
	print $3
}
{% endhighlight %}
这个函数会打印数据行中的第3个数据字段。

函数还能用return语句返回值：
{% highlight string %}
return value
{% endhighlight %}

值可以是变量，或者最终能计算出值的算式：
{% highlight string %}
function myrand(limit)
{
	return int(limit*rand())
}
{% endhighlight %}
你可以将函数的返回值赋给gawk程序中的一个变量：
{% highlight string %}
x = myrand(100)
{% endhighlight %}
这个变量最终会含有函数的返回值。

2） **使用自定义函数**

在定义函数时，它必须出现在所有代码块之前（包括BEGIN代码块）。乍一看这可能有点怪异，但它有助于将函数代码和gawk程序的其他部分分开：
{% highlight string %}
# cat data2
Riley Mullen
123 Main Street
Chicago. IL 60601
(312)555-1234

Frank Williams
456 Oak Streat
Indianapolis. IN 46201
(317)555-9876

Haley Snell
4231 Elm Streat 
Detroit. MI 48201
(313)555-4938

# gawk '   
function myprint()
{
    printf "%-16s - %s\n", $1, $4
}

BEGIN{FS="\n"; RS=""}
{
    myprint()
}' data2
Riley Mullen     - (312)555-1234
Frank Williams   - (317)555-9876
Haley Snell      - (313)555-4938
{% endhighlight %}
这段代码定义了mprint()函数，它会格式化数据行中第1个和第4个数据字段供打印用。然后，gawk程序用该函数显示了数据文件中的数据。

一旦定义了函数，你就能在程序的代码中随便使用了。在使用很长的算法时，这会节省许多工作。

3） **创建函数库**

显而易见地，每次使用时都重写一遍函数并不美妙。不过，gawk提供了一种途径来将函数放到一个库文件中，这样你就能在所有的gawk编程中使用了。

首先，你需要创建一个存储所有gawk函数的文件：
{% highlight string %}
# cat funclib 
function myprint()
{
    printf "%-16s - %s\n", $1, $4
}

function myrand(limit)
{
    return int(limit * rand())
}

function printthird()
{
    print $3
}
{% endhighlight %}

funclib文件含有3个函数定义。要使用它们，你需要用```-f```命令行参数。很遗憾，你不能将```-f```命令行参数和内联gawk脚本放到一起使用，不过你可以在同一个命令行中使用多个```-f```参数。

因此，要使用库，只要创建一个含有你的gawk程序的文件，然后在命令行上同时指定库文件和程序文件：
{% highlight string %}
# cat script4
BEGIN{FS="\n"; RS=""}

{
   myprint()
}


# gawk -f ./funclib -f ./script4 data2
Riley Mullen     - (312)555-1234
Frank Williams   - (317)555-9876
Haley Snell      - (313)555-4938
{% endhighlight %}
你要做的是当需要使用库中定义的函数时，将funclib文件加到你的gawk命令行上就可以了。







<br />
<br />

**[参看]**

1. [Linux下Shell的for循环语句](https://www.cnblogs.com/EasonJim/p/8315939.html)




<br />
<br />
<br />


