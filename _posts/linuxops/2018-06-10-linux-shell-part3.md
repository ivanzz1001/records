---
layout: post
title: shell编程之更多结构化命令
tags:
- LinuxOps
categories: linuxOps
description: shell编程之更多结构化命令
---


在前一章<<shell编程之结构化命令>>里，你了解了如何通过检查命令的输出和变量的值来改变shell脚本程序的流程。在本章中，我们会继续介绍能够控制shell脚本流程的结构化命令。你会了解如何重复一些过程和命令，也就是循环执行一组命令直至达到了某个特定条件。本章将会讨论和演示bash shell的循环命令for、while和util。


<!-- more -->


## 1. for命令

重复执行一系列命令在编程中很常见。通常你需要重复一组命令直至达到某个特定条件，比如处理某个目录下的所有文件、系统上的所有用户或是某个文本文件中的所有行。

bash shell提供了for命令，允许创建一个遍历一系列值的循环。每个迭代都通过一个该系列中的值执行一组预定义的命令。下面是bash shell中for命令的基本格式：
{% highlight string %}
for var in list
do 
   commands
done
{% endhighlight %}
在list参数中，你提供了迭代中要用的一系列值。你可以通过几种不同的途经来指定列表中的值。

在每个迭代中，变量var会包含列表中的当前值。第一个迭代会使用列表中的第一个值，第二个迭代使用第二个值，依次类推，直到列表中的所有值都过了一遍。

在do和done语句之间输入的命令可以是一条或多条标准的bash shell命令。在这些命令中，$var变量包含着这次迭代对应的当前那个列表中的值。

<pre>
说明： 只要你愿意，也可以将do语句和for语句放在同一行，但必须用分号将其同列表中的值分开
      for var in list; do commands; done
</pre>

1) **读取列表中的值**

for命令最基本的用法就是遍历for命令自身中定义的一系列值：
{% highlight string %}
# cat test1 
#!/bin/bash

# basic for command

for test in Alabama Alaska Arizona Arkansas California Colorado
do
  echo "The next state is $test"
done

# ./test1 
The next state is Alabama
The next state is Alaska
The next state is Arizona
The next state is Arkansas
The next state is California
The next state is Colorado
{% endhighlight %}
 
每次for命令遍历提供的值列表时，它会将列表中的下个值赋给$test变量。$test变量像for命令语句中的任何其他脚本变量一样使用。在最后一次迭代后，$test变量的值会在shell脚本的剩余部分一直保持有效。它会一直保持最后一次迭代的值（除非你修改了它）：
{% highlight string %}
# cat test1b 
#!/bin/bash

# tesing the for variable after the looping 

for test in Alabama Alaska Arizona Arkansas California Colorado
do
  echo "The next state is $test"
done

echo "The last state we visited is $test"

test=Connecticut
echo "Wait, now we're visiting $test"

# ./test1b 
The next state is Alabama
The next state is Alaska
The next state is Arizona
The next state is Arkansas
The next state is California
The next state is Colorado
The last state we visited is Colorado
Wait, now we're visiting Connecticut
{% endhighlight %}
$test变量保持了它的值，也允许我们修改它的值并在for命令循环之外跟其他变量一起使用。

2) **读取列表中的复杂值**

事情并不像for循环看上去那么简单。有时你会遇到难处理的数据。下面是个给shell脚本程序员带来麻烦的经典的例子：
{% highlight string %}
# cat badtest 
#!/bin/bash

# another example of how not to use the for command

for test in I don't know if this'll work
do 
  echo "word: $test"
done

# ./badtest 
word: I
word: dont know if thisll
word: work
{% endhighlight %}

真麻烦。shell看到了列表中的单引号并尝试使用它们来定义一个单独的数据值，这个过程一团混乱。

有两种办法来解决这个问题：

* 使用转义字符（反斜线）来将单引号转义；

* 使用双引号来定义用到单引号的值

每个解决方法都并非那么神奇，但每个都能解决这个问题：
{% highlight string %}
# cat test2 
#!/bin/bash

# another example of how not to use the for command

for test in I don\'t know if "this'll" work
do 
  echo "word: $test"
done

# ./test2 
word: I
word: don't
word: know
word: if
word: this'll
word: work
{% endhighlight %}

在第一个有问题的值上，添加了反斜线字符来转义```don't```值中的单引号。在第二个有问题的值上，将```this'll```值用双引号圈起来。这两种方法都能正常工作，辨别出这个值。

你可能遇到的另一个问题是有多个词的值。记住，for循环假定每个值都是用空格分隔的。如果有包含空格的数据值，你可能会遇到另一个问题：
{% highlight string %}
# cat badtest2 
#!/bin/bash

# another example of how not to use the for command

for test in Nevada New Hampshire New Mexio New York North California
do 
    echo "Now going to $test"
done

# ./badtest2 
Now going to Nevada
Now going to New
Now going to Hampshire
Now going to New
Now going to Mexio
Now going to New
Now going to York
Now going to North
Now going to California
{% endhighlight %}
这不是我们想要的结果。for命令用空格来划分列表中的每个值。如果在单独的数据值中有空格，那么你必须用双引号来将这些值圈起来：
{% highlight string %}
# cat test3 
#!/bin/bash

# another example of how not to use the for command

for test in Nevada "New Hampshire" "New Mexio" "New York" "North California"
do 
    echo "Now going to $test"
done

# ./test3 
Now going to Nevada
Now going to New Hampshire
Now going to New Mexio
Now going to New York
Now going to North California
{% endhighlight %}
现在for命令可以正确的区分不同值了。还有，注意当你在某个值两边使用双引号时，shell不会将双引号当成值的一部分。

3) **从变量读取列表**

通常shell脚本遇到的情况是，你将一系列值都集中存储在了一个变量中，然后需要遍历整个列表。你也可以通过for命令完成这个：
{% highlight string %}
# cat test4 
#!/bin/bash

# using a variable to hold the list

list="Alabama Alaska Arizona Arkansas Colorado"
list=$list" Connecticut"

for state in $list
do 
   echo "Have you ever visited $state"
done

# ./test4 
Have you ever visited Alabama
Have you ever visited Alaska
Have you ever visited Arizona
Have you ever visited Arkansas
Have you ever visited Colorado
Have you ever visited Connecticut
{% endhighlight %}

$list变量包含了给迭代用的标准文本值列表。注意，代码还是用了另一个赋值语句来向$list变量包含的已有列表添加了一个值。这是向变量中存储的已有文本字符串尾部添加文本的一个常用方法。

4） **从命令读取值**

生成列表中要用的值的另外一个途径就是使用命令的输出。你可以用反引号来执行任何能产生输出的命令，然后在for命令中使用该命令的输出：
{% highlight string %}
# cat test5 
#!/bin/bash

# reading values from a file

file="states"

for state in `cat $file`
do
   echo "Visit beautiful $state"
done

# cat states 
Alabama
Alaska
Arizona
Arkansas
Colorado
Connecticut
Delaware
Florida 
Georgia

# ./test5 
Visit beautiful Alabama
Visit beautiful Alaska
Visit beautiful Arizona
Visit beautiful Arkansas
Visit beautiful Colorado
Visit beautiful Connecticut
Visit beautiful Delaware
Visit beautiful Florida
Visit beautiful Georgia
{% endhighlight %}

这个例子在反引号中使用了cat命令来输出文件states的内容。你会注意到states文件每行有一个州，而不是通过空格分割的。for命令依然以每次一行的方式遍历了cat命令的输出，假定每个州都是在单独的一行上。但这并没有解决数据中有空格的问题，如果你列出了一个名字中有空格的州，for命令仍然会将每个单词当作单独的值。这是有原因的，下一节我们将会了解到。

<pre>
说明： test5的代码范例将文件名赋给变量，只用了文件名而不是路径。这要求文件和脚本位于同一个目录中。如果不是的话，你需要使用
      全路径名（不管是绝对路径还是相对路径）来引用文件位置。
</pre>

5） **更改字段分隔符**

这个问题的原因是特殊的环境变量```IFS```，称为内部字段分隔符(internal field seperator)。IFS环境变量定义了bash shell用作字段分隔符的一系列字符。默认情况下，bash shell会将下列字符当做字段分隔符：

* 空格

* 制表符

* 换行符

如果bash shell在数据中看到了这些字符中的任意一个，它就会假定你在列表中开始了一个新的数据段。在处理可能含有空格的数据（比如文件名时），这会非常麻烦，如你在上一个脚本示例中看到的。

要解决这个问题，你可以在shell脚本中临时更改IFS环境变量的值来限制一下被bash shell当做字段分隔符的字符。但这种方式有点奇怪。比如，如果你修改IFS的值使其只能识别换行符，你必须这么做：
<pre>
IFS=$'\n'
</pre>
将这个语句加入到脚本中，告诉bash shell在数据值中忽略空格和制表符。对前一个脚本使用这种方法，将获得如下输出：
{% highlight string %}
# cat states 
Alabama
Alaska
Arizona
Arkansas
Colorado
Connecticut
Delaware
Florida 
Georgia
New York
New Hampshire
North Carolina

# cat test5b 
#!/bin/bash

# reading values from a file

file="states"

IFS=$'\n'

for state in `cat $file`
do
   echo "Visit beautiful $state"
done

# ./test5b 
Visit beautiful Alabama
Visit beautiful Alaska
Visit beautiful Arizona
Visit beautiful Arkansas
Visit beautiful Colorado
Visit beautiful Connecticut
Visit beautiful Delaware
Visit beautiful Florida 
Visit beautiful Georgia
Visit beautiful New York
Visit beautiful New Hampshire
Visit beautiful North Carolina
{% endhighlight %}
现在shell脚本能够使用列中含有空格的值了。

<pre>
警告： 在处理长脚本时，可能在一个地方需要修改IFS的值，然后忘掉它了并在脚本中其他地方以为还是默
认的值。一个可参考的简单实践是在修改IFS之前保存原来的IFS值，之后再恢复它。

这种技术可以这样编程：
IFS.old=$IFS
IFS=$'\n'

// here use the new IFS value in code

IFS=$IFS.old
这会为脚本中后面的操作保证IFS的值恢复到了默认值。
</pre>

还有其他一些IFS环境变量的绝妙用法。假定你要遍历一个文件中用冒号分隔的值（比如在/etc/passwd文件中），你要做的就是将IFS的值设为冒号：
{% highlight string %}
IFS=$':'
{% endhighlight %}
如果你要指定多个IFS字符，只要将它们在赋值行串起来就行：
{% highlight string %}
IFS=$'\n:;"'
{% endhighlight %}
这个赋值会将换行符、冒号、分号和双引号作为字段分隔符。如何使用IFS字符解析数据没有任何限制。

6） **用通配符读取目录**

最后，你可以用for命令来自动遍历某个目录下的文件。进行此操作时，你必须在文件名或路径名中使用通配符。它会强制shell使用文件文件扩展匹配（file globbing)。文件扩展匹配是生成匹配指定通配符的文件名或路径名的过程。

这个特性在处理目录中的文件而你不知道所有的文件名时非常好用：
{% highlight string %}
# cat test6 
#!/bin/bash

# iterate through all the files in a directory

for file in /usr/local/*
do 
   if [ -d "$file" ]
   then
       echo "$file is a directory"
   elif [ -f "$file" ]
   then
       echo "$file is a  regular file"
   fi

done

# ./test6 
/usr/local/bin is a directory
/usr/local/etc is a directory
/usr/local/games is a directory
/usr/local/go is a directory
/usr/local/include is a directory
/usr/local/lib is a directory
/usr/local/lib64 is a directory
/usr/local/libexec is a directory
/usr/local/nginx is a directory
/usr/local/openssl is a directory
/usr/local/redis-3.2.11 is a directory
/usr/local/sbin is a directory
/usr/local/share is a directory
/usr/local/src is a directory
{% endhighlight %}

for命令会遍历```/usr/local/*```输出的结果。该代码用test命令测试了每个条目（使用方括号方法），以查看它是个目录（通过-d参数)还是个文件（通过-f参数）。

注意，在这个例子中，我们和if语句里的测试处理的有些不同：
<pre>
if [ -d "$file" ]
</pre>
在Linux中，目录名和文件名中包含空格当然是合法的。要容纳这种值，你应该将```$file```变量用双引号圈起来。如果不这么做，遇到含有空格的目录名或文件名时会有错误产生：
<pre>
./test6: line 6: [: too many arguments
./test6: line 9: [: too many arguments
</pre>
在test命令中，bash shell会将额外的单词当做参数，造成错误。

你也可以在for命令中通过列出一系列的目录通配符来将目录查找方法和列表方法合并进同一个for语句：
{% highlight string %}
# cat test7 
#!/bin/bash

# iterate through all the files in a directory

for file in /usr/local/* /root/Downloads/*
do 
   if [ -d "$file" ]
   then
       echo "$file is a directory"
   elif [ -f "$file" ]
   then
       echo "$file is a  regular file"
   fi

done

# ./test7 
/usr/local/bin is a directory
/usr/local/etc is a directory
/usr/local/games is a directory
/usr/local/go is a directory
/usr/local/include is a directory
/usr/local/lib is a directory
/usr/local/lib64 is a directory
/usr/local/libexec is a directory
/usr/local/nginx is a directory
/usr/local/openssl is a directory
/usr/local/redis-3.2.11 is a directory
/usr/local/sbin is a directory
/usr/local/share is a directory
/usr/local/src is a directory
/root/Downloads/Create_DB.sql is a  regular file
/root/Downloads/curl-7.29.0-46.el7.x86_64.rpm is a  regular file
/root/Downloads/libcurl-7.29.0-46.el7.x86_64.rpm is a  regular file
/root/Downloads/libcurl-devel-7.29.0-46.el7.x86_64.rpm is a  regular file
/root/Downloads/mysql is a directory
{% endhighlight %}

for语句首先使用了文件扩展匹配来遍历通配符生成的文件列表，然后它会遍历列表中的下一个文件。你可以将任意多的通配符放进列表中。
<pre>
警告： 注意，你可以在列表数据中放入任何东西。即使文件或目录不存在，for语句也会尝试处理你放到列表中的任何东西。在处理文件
      或目录时，这可能会是个问题。你也不知道你正在尝试遍历一个并不存在的目录： 在处理之前测试一下文件或目录总是好的。
</pre>

## 2. C语言风格的for命令

如果你用C语言编过程，你可能会对bash shell使用for命令的方式有点惊奇。在C语言中，for循环通常定义一个变量，然后这个变量会在每次迭代时自动改变。通常，程序员会将这个变量用作计数，并在每次迭代中让计数器增一或减一。bash的for命令也提供了这个功能。本节将会告诉你如何在bash shell中使用C语言风格的for命令。

### 2.1 C语言的for命令
C语言的for命令有一个用来指明变量的特殊方法、一个必须保持成立才能继续迭代的条件，以及另一个为每个迭代改变变量的方法。当指定条件不成立时，for循环就会停止。条件等式通过标准的数学符号定义，比如，考虑下面的C语言代码：
{% highlight string %}
for(i=0;i<10;i++)
{
      printf("the next number is %d\n", i);
}
{% endhighlight %}
这段代码产生了一个简单的迭代循环，其中变量```i```作为计数器。第一部分将一个默认值赋给该变量。中间的部分定义了循环重复的条件。当定义的条件不成立时，for循环就停止迭代了。最后一部分定义了迭代的过程。在每次迭代之后，最后一部分中定义的表达式会被执行。在本例中，i变量在每次迭代后增一。

bash shell也支持一个版本的for循环，看起来跟C语言风格的for循环类似，虽然它也有一些细微的不同，包括一些会叫shell程序员困惑的东西。这是bash中C语言风格的for循环的基本格式：
{% highlight string %}
//注： 如下格式没有空格的限制，与C语言类似
for (( variable assignment; condition; iteration process ))
{% endhighlight %}

C语言风格的for循环的格式可能对bash shell脚本程序员来说有点困难，由于它使用了C语言风格的变量引用方式而不是shell风格的变量引用方式。C语言风格的for命令看起来如下：
{% highlight string %}
for((i=0;i<10;i++))
{% endhighlight %}

注意，有一些事情没有遵循标准的bash shell for命令：

* 给变量赋值可以有空格

* 条件中的变量不以美元符开头；

* 迭代过程的算式未用expr命令格式

shell开发人员创建了这种格式以更贴切地模仿C语言风格的for命令。这对C语言程序员来说很好，但也可能将即使是专家级的shell程序员弄得一头雾水。在脚本中使用C语言风格的for循环时要小心。

这里有个在bash shell程序中使用C语言风格的for命令的例子：
{% highlight string %}
# cat test8 
#!/bin/bash

# testing the C-style for loop

for((i=1;i<=10;i++))
do
    echo "The next number is $i"
done

# ./test8 
The next number is 1
The next number is 2
The next number is 3
The next number is 4
The next number is 5
The next number is 6
The next number is 7
The next number is 8
The next number is 9
The next number is 10
{% endhighlight %}











<br />
<br />

**[参看]**






<br />
<br />
<br />


