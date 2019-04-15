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




<br />
<br />

**[参看]**






<br />
<br />
<br />


