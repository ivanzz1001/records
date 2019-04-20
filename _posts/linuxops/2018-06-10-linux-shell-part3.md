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

for循环通过定义好的变量（本例中是字母i）遍历了这些命令。在每个迭代中，```$i```变量包含了for循环中赋给的值。在每次迭代后，循环的迭代过程会作用在变量上，在本例中，变量增一。

### 2.2 使用多个变量
C语言风格的for命令也允许你为迭代使用多个变量。循环会单独处理每个变量，允许你为每个变量定义不同的迭代过程。
{% highlight string %}
# cat test9 
#!/bin/bash

# multiple variables

for((a=1,b=10;a<=10 && b>=5;a++,b--))
do
   echo "$a -- $b"
done

# ./test9 
1 -- 10
2 -- 9
3 -- 8
4 -- 7
5 -- 6
6 -- 5
{% endhighlight %}

变量a和b每个都用不同的值来初始化并且定义了不同的迭代过程。循环每个迭代中增加变量a的同时，减少了变量b。

## 3. while命令
while命令某种意义上是if-then语句和for循环的混杂体。while命令允许你定义一个要测试的命令，然后循环执行一组命令，只要定义的测试命令返回的是退出状态码0。它会在每个迭代的一开始测试test命令。在test命令返回非零退出状态码时，while命令会停止执行那组命令。

1) **while的基本格式**

while命令的基本格式是：
{% highlight string %}
while test command
do
    other commands
done
{% endhighlight %}
while命令中定义的test命令和if-then中定义的是一样的格式。和if-then语句中一样，你可以使用任何普通的bash shell命令，或者用test命令作为条件，比如变量值。

while命令的关键是，指定的test命令的退出状态码必须随着循环中运行的命令改变。如果退出状态码从来不变，那while循环将会一直不停的循环。

最常见的test命令的用法是，用方括号来查看循环命令中用到的shell变量的值：
{% highlight string %}
# cat test10 
#!/bin/bash

# while command test

var1=10
while [ $var1 -gt 5 ]
do
   echo $var1
   var1=$[$var1-1]
done

# ./test10 
10
9
8
7
6
{% endhighlight %}
while命令定义了每次迭代时检查的测试条件：
<pre>
while [ $var1 -gt 5 ]
</pre>
只有测试条件成立，while命令才会继续遍历执行定义好的命令。在这些命令中，测试条件中用到的变量必须被修改，否则你就进入了一个无限循环。在本例中，我们用shell算术来将变量值减一：
<pre>
var1=$[$var1-1]
</pre>
while循环会在测试条件不再成立时停止。

2) **使用多个测试命令**

在极少数情况中，while命令允许你在while语句行定义多个测试命令。只有```最后```一个测试命令的退出状态码会被用来决定什么时候结束循环。如果你不够小心，这可能会导致一些有意思的结果。下面的例子将会说明：
{% highlight string %}
# cat test11 
#!/bin/bash

# testing a multicommand while loop

var1=10

while echo $var1
    [ $var1 -ge 0 ]
do
    echo "This is inside the loop"
    var1=$[$var1-1]
done

# ./test11 
10
This is inside the loop
9
This is inside the loop
8
This is inside the loop
7
This is inside the loop
6
This is inside the loop
5
This is inside the loop
4
This is inside the loop
3
This is inside the loop
2
This is inside the loop
1
This is inside the loop
0
This is inside the loop
-1
{% endhighlight %}

注意在本例中做了什么。在while语句中定义了两个测试命令：
<pre>
while echo $var1
    [ $var1 -ge 0 ]
</pre>
第一个测试会简单的显示var1变量的当前值。第二个测试用方括号来决定var1变量的值。在循环内部，echo语句会显示一条简单的消息，说明循环被执行了。注意当你运行本示例时输出是如何结尾的：
<pre>
This is inside the loop
-1
</pre>
while循环会在var1变量等于0时执行echo语句，然后将var1变量的值减1。下一步，测试命令会为下个迭代执行。echo测试命令语句被执行了，显示了var变量的值（现在比0小）。shell直到执行test测试命令了while循环才会停止。

这说明在含有多个命令的while语句中，在每次迭代中所有的测试命令都会被执行，包括最后一个测试命令不成立的最后那次循环。要小心这种用法。另一个要小心的是你如何指定多个测试命令。注意每个测试命令都是在单独的一行上。


## 4. until命令
until命令和while命令工作的方式完全相反。util命令要求你指定一个通常输出非零退出状态码的测试命令。只有测试命令的退出状态码非零，bash shell才会执行循环中列出的那些命令。一旦测试命令返回了退出状态码0，循环就结束了。

如你所期望的，until命令的格式如下：
<pre>
until test commands
do
    other commands
done
</pre>
类似于while命令，你可以在until命令语句中有多个测试命令。只有最后一个命令的退出状态码决定bash shell是否执行定义好的其他命令。

下面是使用util命令的一个例子：
{% highlight string %}
# cat test12 
#!/bin/bash

# using the util command

var1=100

until [ $var1 -eq 0 ]
do
    echo $var1
    var1=$[$var1-25]
done

# ./test12 
100
75
50
25
{% endhighlight %}
本例中会测试var1变量来决定until循环何时停止。只要变量的值等于0了，util命令就会停止循环了。同while命令一样，在和until命令一起使用多个测试命令时要注意：
{% highlight string %}
# cat test13 
#!/bin/bash

# using the until command

var1=100

until echo $var1
     [ $var1 -eq 0 ]
do
    echo "Inside the loop: $var1"
    var1=$[$var1 - 25]
done

# ./test13 
100
Inside the loop: 100
75
Inside the loop: 75
50
Inside the loop: 50
25
Inside the loop: 25
0
{% endhighlight %}
shell会执行指定的测试命令，只有在最后一个命令成立时停止。


## 5. 嵌套循环
循环语句可以在循环内使用任意类型的命令，包括其他循环命令。这种称为嵌套循环(nested loop)。注意，在使用嵌套循环时，你是在迭代中使用迭代，命令运行的次数是乘积关系。不注意这点有可能会在脚本中造成问题。

这里有个在for循环中嵌套for循环的简单例子：
{% highlight string %}
# cat test14 
#!/bin/bash

# nesting for loops

for((a=1;a<=3;a++))
do
    echo "Starting loop $a:"
    for((b=1;b<=3;b++))
    do
       echo "    Inside loop: $b"
    done
done

# ./test14 
Starting loop 1:
    Inside loop: 1
    Inside loop: 2
    Inside loop: 3
Starting loop 2:
    Inside loop: 1
    Inside loop: 2
    Inside loop: 3
Starting loop 3:
    Inside loop: 1
    Inside loop: 2
    Inside loop: 3
{% endhighlight %}
这个被嵌套的循环（也称为内部循环）会在外部循环的每次迭代中遍历一遍它所有的值。注意，两个循环的do和done命令没有任何差别。bash shell知道当第一个done命令执行时是指内部循环而非外部循环。

在混用循环命令时也一样，比如在while循环内部放置一个for循环：
{% highlight string %}
# cat test15 
#!/bin/bash

# placing a for loop inside a while loop

var1=5

while [ $var1 -ge 0 ]
do
    echo "Outer loop: $var"
    for((var2=1; var2<3;var2++))
    do
       var3=$[$var1*$var2]
       echo "   Inner loop: $var1 * $var2 = $var3"
    done
   
    var1=$[$var1-1]
done

# ./test15 
Outer loop: 
   Inner loop: 5 * 1 = 5
   Inner loop: 5 * 2 = 10
Outer loop: 
   Inner loop: 4 * 1 = 4
   Inner loop: 4 * 2 = 8
Outer loop: 
   Inner loop: 3 * 1 = 3
   Inner loop: 3 * 2 = 6
Outer loop: 
   Inner loop: 2 * 1 = 2
   Inner loop: 2 * 2 = 4
Outer loop: 
   Inner loop: 1 * 1 = 1
   Inner loop: 1 * 2 = 2
Outer loop: 
   Inner loop: 0 * 1 = 0
   Inner loop: 0 * 2 = 0
{% endhighlight %}

同样，shell能够区分开内部for循环的do和done命令与外部while循环的do和done命令。

## 6. 循环处理文件数据

通常，你必须遍历存储在文件中的数据。这要求结合已经讲过的两种技术：

* 使用嵌套循环

* 修改IFS环境变量

通过修改```IFS```环境变量，你就能强制for命令将文件中的每行都当成单独的一个条目来处理，即便数据中有空格也是如此。一旦你从文件中提取出了单独的行，你可能需要再次循环来提取其中的数据。

经典的例子是处理/etc/passwd文件中的数据。这要求你逐行遍历/etc/passwd文件并将IFS变量的值改成冒号，这样你就能分隔开每行中的各个单独的数据段了：
{% highlight string %}
# cat test16 
#!/bin/bash

# changing the IFS value

IFS_OLD=$IFS
IFS=$'\n'

for entry in `cat /etc/passwd`
do
    echo "Values in $entry -"
    IFS=:
    
    for value in $entry
    do
       echo "     $value"
    done
done
{% endhighlight %}

这个脚本使用了两个不同的IFS值来解析数据。第一个IFS值解析出/etc/passwd文件中的单独的行。内部for循环进一步将IFS的值修改为冒号，允许你从/etc/passwd的行中解析出单独的值。

在运行这个脚本时，你会得到如下输出：
<pre>
# ./test16 | more
Values in root:x:0:0:root:/root:/bin/bash -
     root
     x
     0
     0
     root
     /root
     /bin/bash
...
</pre>

内部循环会解析出/etc/passwd每行中的每个单独的值。这用来处理通常导入电子表格采用的逗号分割的数据也很方便。

## 7. 控制循环
你可能会想，一旦启动了循环，就必须等待循环完成所有的迭代。不是这样的，有几个命令能帮助我们控制循环内部的情况：

* break命令

* continue命令

每个命令在如何控制循环的执行上有不同的用法。下面几节将会介绍如何使用这些命令来控制循环的执行。

### 7.1 break命令
break命令是退出进行中的循环的一个简单方法。你可以用break命令来退出任意类型的循环，包括while和until循环。

有几种情况你可以使用break命令。本节将会介绍这些方法中的每一种。

1） **跳出单个循环**

在shell执行break命令时，它会尝试跳出正在处理的循环：
{% highlight string %}
# cat test17 
#!/bin/bash

# breaking out of a for loop

for var1 in 1 2 3 4 5 6 7 8 9 10
do
    if [ $var1 -eq 5 ]
    then 
          break
    fi
   
    echo "Iteration number: $var1"
done
echo "The for loop is completed"

# ./test17 
Iteration number: 1
Iteration number: 2
Iteration number: 3
Iteration number: 4
The for loop is completed
{% endhighlight %}
for循环通常都会遍历一遍列表中指定的所有值。但当满足if-then的条件时，shell会执行break命令，for循环会停止。

这种方法同样适用于while和until循环：
{% highlight string %}
# cat test18 
#!/bin/bash

# breaking out of a while loop

var1=1

while [ $var1 -lt 10 ]
do
    if [ $var1 -eq 5 ]
    then
        break
    fi

    echo "Iteration: $var1"
    var1=$[$var1 + 1]
done

echo "The while loop is completed"


# ./test18 
Iteration: 1
Iteration: 2
Iteration: 3
Iteration: 4
The while loop is completed
{% endhighlight %}
while循环会在if-then的条件满足时执行break命令，终止。

2) **跳出内部循环**

在处理多个循环时，break命令会自动终止你所在最里面的循环：
{% highlight string %}
# cat test19 
#!/bin/bash

# breaking out of an inner loop

for((a=1;a<4;a++))
do
    echo "Outer loop: $a"

    for ((b=1;b<100;b++))
    do
         if [ $b -eq 5 ]
         then
              break
         fi
 
         echo "   inner loop: $b" 
    done
done

# ./test19 
Outer loop: 1
   inner loop: 1
   inner loop: 2
   inner loop: 3
   inner loop: 4
Outer loop: 2
   inner loop: 1
   inner loop: 2
   inner loop: 3
   inner loop: 4
Outer loop: 3
   inner loop: 1
   inner loop: 2
   inner loop: 3
   inner loop: 4
{% endhighlight %}
内部循环的for语句指明一直重复直到变量等于100。但内部循环的if-then语句指明当变量b的值等于5时执行break命令。注意，即使内部循环通过break命令终止了，外部循环依然按指定的继续执行。

3）**跳出外部循环**

有时你在内部循环，但需要停止外部循环。break命令接受单个命令行参数值：
<pre>
break n
</pre>
其中n说明了要跳出的循环层级。默认情况下，n为1，表明跳出的是当前的循环。如果你将n设为2，break命令就会停止下一级的外部循环：
{% highlight string %}
# cat test20 
#!/bin/bash

# breaking out of an outer loop

for((a=1;a<4;a++))
do
   echo "Outer loop: $a"
   for ((b=1;b<100;b++))
   do
       if [ $b -gt 4 ]
       then
            break 2
       fi

       echo "    Inner loop: $b"
   done
done

# ./test20 
Outer loop: 1
    Inner loop: 1
    Inner loop: 2
    Inner loop: 3
    Inner loop: 4
{% endhighlight %}
注意，当shell执行了break命令时，外部循环停止了。

### 7.2 continue命令
continue命令是提早结束执行循环内部的命令但并不完全终止整个循环的一个途径。它允许你在循环内部设置shell不执行命令的条件。这里有个在for循环中使用continue命令的简单例子：
{% highlight string %}
# cat test21 
#!/bin/bash

# using the continue command

for((var1=1; var1<15;var1++))
do
   if [ $var1 -gt 5 ] && [ $var1 -lt 10 ]
   then
       continue
   fi

   echo "Iteration number: $var1"
done

# ./test21 
Iteration number: 1
Iteration number: 2
Iteration number: 3
Iteration number: 4
Iteration number: 5
Iteration number: 10
Iteration number: 11
Iteration number: 12
Iteration number: 13
Iteration number: 14
{% endhighlight %}

当if-then语句的条件被满足时（值大于5而小于10），shell会执行continue命令，跳过循环中的其他命令，但循环会继续。当if-then的条件不再被满足时，一切又回到正轨了。

也可以在while和until循环中使用continue命令，但要特别小心。记住，当shell执行continue命令时，它会跳过剩余的命令。如果你在这些条件中的某条中增加测试条件变量，问题就出现了：
{% highlight string %}
# cat badtest 
#!/bin/bash

# improperly using the continue command in a while loop

var1=0

while echo "while iteration: $var1"
     [ $var1 -lt 15 ]
do
   if [ $var1 -gt 5 ] && [ $var1 -lt 10 ]
   then
       continue
   fi

   echo "   Inner iteration number: $var1"
   var1=$[$var1 + 1]
   
done

# ./badtest | more
while iteration: 0
   Inner iteration number: 0
while iteration: 1
   Inner iteration number: 1
while iteration: 2
   Inner iteration number: 2
while iteration: 3
   Inner iteration number: 3
while iteration: 4
   Inner iteration number: 4
while iteration: 5
   Inner iteration number: 5
while iteration: 6
while iteration: 6
{% endhighlight %}
你可能要确保你将脚本的输出重定向到了more命令，这样才能停止这些。所有一切看起来都正常，直到满足了if-then条件，shell执行了continue命令。当shell执行continue命令时，它会跳过while循环中的其他命令。遗憾的是，这正是while测试命令中被测的```$var1```计数变量增加的地方。这意味着这个变量不会再增长了，正如你从前面连续的输出显示中看到的。

和break命令一样，continue命令也允许通过命令行参数指定要继续哪级循环：
<pre>
continue n
</pre>
其中n定义了要继续的循环层级。下面是继续外部for循环的一个例子：
{% highlight string %}
# cat test22 
#!/bin/bash

# continuing an outer loop

for((a=1;a<=5;a++))
do
   echo "Iteration $a:"

   for((b=1;b<3;b++))
   do
       if [ $a -gt 2 ] && [ $a -lt 4 ]
       then
           continue 2
       fi

       var3=$[$a * $b]
       echo "    The result of $a * $b is $var3"
   done
done

# ./test22 
Iteration 1:
    The result of 1 * 1 is 1
    The result of 1 * 2 is 2
Iteration 2:
    The result of 2 * 1 is 2
    The result of 2 * 2 is 4
Iteration 3:
Iteration 4:
    The result of 4 * 1 is 4
    The result of 4 * 2 is 8
Iteration 5:
    The result of 5 * 1 is 5
    The result of 5 * 2 is 10
{% endhighlight %}
其中的if-then语句：
{% highlight string %}
if [ $a -gt 2 ] && [ $a -lt 4 ]
then
    continue 2
fi
{% endhighlight %}
用continue命令来停止处理循环内的命令但继续处理外部循环。注意值为3的迭代脚本输出未再处理任何内部循环语句，因为continue命令停止了处理过程，但外部循环依然会继续。

## 8. 处理循环的输出

最后，在shell脚本中，你要么```管接```要么重定向循环的输出。你可以在done命令之后添加一个处理命令：
{% highlight string %}
for file in /home/ivan1001/*
do 
    if [ -d "$file" ]
    then 
       echo "$file is a directory"
    elif [ -f "$file" ]
    then 
        echo "$file is a file"
     fi
done > output.txt
{% endhighlight %}

shell会将for命令的结果重定向到文件output.txt中，而不是显示在屏幕上。

考虑下面重定向for命令的输出到文件的例子：
{% highlight string %}
# cat test23 
#!/bin/bash

# redirecting the for output to a file

for((a=1; a < 10; a++))
do
   echo "The number is $a"
done > test23.txt

echo "The command is finished"

# ./test23 
The command is finished
# cat test23.txt 
The number is 1
The number is 2
The number is 3
The number is 4
The number is 5
The number is 6
The number is 7
The number is 8
The number is 9
{% endhighlight %}
shell创建了文件test23.txt，并将for命令的输出重定向到这个文件。shell在for命令之后如常显示了echo语句。

这种方法同样适用于将循环的结果管接给另一个命令：
{% highlight string %}
# cat test24 
#!/bin/bash

# piping a loop to another command

for state in "North Dakota" Connecticut Illinois Alabama Tennessee
do

  echo "$state is the next place to go"
done | sort

echo "This completes our travels"

# ./test24 
Alabama is the next place to go
Connecticut is the next place to go
Illinois is the next place to go
North Dakota is the next place to go
Tennessee is the next place to go
This completes our travels
{% endhighlight %}

state值并没有在for命令列表中以特定次序列出。for命令的输出传给了sort命令，它会改变for命令输出的结果的顺序。运行这个脚本实际上说明了结果已经在脚本内部排好序了。



<br />
<br />

**[参看]**






<br />
<br />
<br />


