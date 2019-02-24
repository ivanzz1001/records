---
layout: post
title: sed命令的使用
tags:
- LinuxOps
categories: linuxOps
description: sed命令的使用
---

本节主要介绍一下sed命令的使用。


<!-- more -->

## 1. sed编辑器介绍
sed编辑器被称作**流编辑器**(stream editor)，跟普通的交互式文本编辑器恰好相反。在交互式文本编辑器(比如vim)中，你可以用键盘命令来交互式地插入、删除或替换数据中的文本。流编辑器则会在编辑器处理数据之前基于预先提供的一组规则来编辑数据流。

sed编辑器可以基于输入到命令行的或是存储在命令文本文件中的命令来处理数据流中的数据。它每次从输入行中读取一行，用提供的编辑器命令匹配数据、按命令中指定的方式修改数据流中的数据，然后将生成的数据输出到STDOUT。在流编辑器将所有命令与一行数据进行匹配后，它会读取下一行数据并重复这个过程。在流编辑器处理完流中的所有数据行后，它就会终止。


由于命令都是一行一行顺序处理的，sed编辑器必须一次就完成对文本的修改。这使得sed编辑器比交互式编辑器快很多，这样你就能很快地完成对数据的自动修改了。

使用sed命令的格式如下：
<pre>
sed options script file
</pre>
选项参数允许你修改sed命令的行为，同时还包含下面列出的选项：

* -e script: 在处理输入时，将script中指定的命令添加到运行的命令中

* -f file: 在处理输入时，将file中指定的命令添加到运行的命令中

* -n: 不要为每个命令生成输出，等待print命令来输出

script参数指定了将作用在流数据上单个命令。如果需要用多个命令，你必须用```-e```选项来在命令行上指定它们，或用```-f```选项在单独的文件中指定。有大量的命令可以用来处理数据。

### 1.1 在命令行定义编辑器命令
默认情况下，sed编辑器会将指定的命令应用到STDIN输入流上。这样你可以直接将数据管道输出到sed编辑器上处理。这里有个演示如何做的简短例子：
<pre>
# echo "This is a test" | sed 's/test/big test/'
This is a big test
</pre>
在上面的例子中，sed编辑器使用了```s```命令。s命令会用斜线间指定的第二个文本字符串来替换第一个文本字符串。在本例子中，big test替换了test.

在运行这个例子时，它应该立即就显示了结果。这就是使用sed编辑器的强大之处。可以在几乎和交互式编辑器的启动时间一样的时间内就对数据作多处修改。

当然，这个简单的测试只修改了一行数据。在修改多处文件数据时，你应该能得到差不多快的结果：
<pre>
# cat data1
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.

# sed 's/dog/cat/' data1
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy cat.
</pre>
sed命令几乎瞬间就执行完并返回数据。在它处理每行数据的同时，结果也显示出来了。可以在sed编辑器处理完整个文件之前就开始观察结果。

重要的是要记住，sed编辑器自身不会修改文本文件的数据。它只会讲修改后的数据发送到STDOUT。如果你查看原来的文本文件，它仍然会保留着原始数据：
<pre>
# cat data1
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
</pre>


### 1.2 在命令行中使用多个编辑器命令
要在sed命令行上执行多个命令时，只要用```-e```选项就可以了：
<pre>
# sed -e 's/brown/green/;s/dog/cat/' data1
The quick green fox jumps over the lazy cat.
The quick green fox jumps over the lazy cat.
The quick green fox jumps over the lazy cat.
The quick green fox jumps over the lazy cat.
</pre>
两个命令都可以作用到文件中的每行数据上。命令之间必须用```分号```分隔，并且在命令的末尾和分号之间之间不能有空格。

也可以用bash shell中的次提示符来分隔命令，而不用分号。只要输入第一个单引号来开始编写，bash会继续提示你输入更多命令，直到你输入了封尾的单引号：
{% highlight string %}
# sed -e '
> s/brown/gree/
> s/fox/elephant/
> s/dog/cat/
> ' data1
The quick gree elephant jumps over the lazy cat.
The quick gree elephant jumps over the lazy cat.
The quick gree elephant jumps over the lazy cat.
The quick gree elephant jumps over the lazy cat.
{% endhighlight %}
必须记住，要在封尾单引号所在行结束命令。bash shell一旦发现了封尾单引号，就会执行命令。开始后，sed命令就会将你指定的每条命令应用到文本文件中的每一行上。

### 1.3 从文件中读取编辑器命令
最后，如果有大量要处理的sed命令，将它们放进一个文件中通常会更方便些。可以在sed命令中用```-f```选项来指定文件：
{% highlight string %}
# tee script <<EOF
> s/brown/green/
> s/fox/elephant/
> s/dog/cat/
> EOF
s/brown/green/
s/fox/elephant/
s/dog/cat/

# cat script
s/brown/green/
s/fox/elephant/
s/dog/cat/

# sed -f script data1
The quick green elephant jumps over the lazy cat.
The quick green elephant jumps over the lazy cat.
The quick green elephant jumps over the lazy cat.
The quick green elephant jumps over the lazy cat.
{% endhighlight %}

在这种情况下，不用每条命令后面都放一个分号。sed编辑器知道每行都有一条单独的命令。跟在命令行输入命令一样，sed编辑器会从指定文件中读取命令，并将它们应用到数据文件中的每一行上。

## 2. sed编辑器基础
成功使用sed编辑器的关键在于掌握它各式各样的能帮你定制文本编辑行为的命令和格式。本节将介绍一些能集成到脚本中方便使用sed编辑器的基本命令和功能。

### 2.1 更多替换选项
通过上面，我们已经懂得了如何用```s```命令来用新文本替换一行内的文本。但还有一些其他的substitute命令选项能让事情变得更为简单。

**2.1.1 替换标记**

关于substitute命令如何替换字符串中匹配的模式需要注意一点。看看下面这个例子会出现什么情况：
{% highlight string %}
# tee data5 <<EOF
> This is a test of the test script.
> This is the second test of the test script.
> EOF
This is a test of the test script.
This is the second test of the test script.

# sed 's/test/trial/' data5
This is a trial of the test script.
This is the second trial of the test script.
{% endhighlight %}
substitute命令在替换多行中的文本时能正常工作，但默认情况下它只替换每行中出现的第一处。要让替换命令对一行中不同地方出现的文本都起作用，必须使用**替换标记**(substitution flag)。替换标记会在替换命令字符串之后设置：
<pre>
s/pattern/replacement/flags
</pre>
有4种可用的替换标记：

* 数字： 表明新文本将替换第几处模式匹配的地方；

* g: 表明新文本将替换所有已有文本出现的地方；

* p: 表明原来行的内容要打印出来；

* w file: 将替换的结果写到文件中；

下面分别讲述一下这些选项：

**1) '数字'替换标记**

在第一类替换中，你可以指定sed编辑器用新文本替换第几处模式匹配的地方：
<pre>
# sed 's/test/trial/2' data5
This is a test of the trial script.
This is the second test of the trial script.
</pre>
将替换标记指定为2的结果是，sed编辑器只替换每行中第二次出现的匹配模式。

**2) 'g'替换标记**

```g```替换标记使你能替换文本中每处匹配模式出现的地方：
<pre>
# sed 's/test/trial/g' data5
This is a trial of the trial script.
This is the second trial of the trial script.
</pre>

**3) 'p'替换标记**

```p```替换标记会打印包含与substitute命令中指定的模式匹配的行。则通常会和sed的```-n```选项一起使用：
{% highlight string %}
# tee data6 <<EOF
> This is a test line.
> This is a different line.
> EOF
This is a test line.
This is a different line.
 
# sed -n 's/test/trial/p' data6
This is a trial line.
{% endhighligt %}
上面```-n```选项禁止sed编辑器输出。但```p```替换标记会输出修改过的行。将二者配合使用则会只输出被substitute命令修改过的行。

**4) 'w'替换标记**

```w```替换标记会产生同样的输出，不过会将输出保存到指定文件中：
<pre>
# sed 's/test/trial/' data6
This is a trial line.
This is a different line.


# sed 's/test/trial/w test' data6
This is a trial line.
This is a different line.

# cat test
This is a trial line.
</pre>
sed编辑器的正常输出是在STDOUT中，而只有那些包含匹配模式的行才会保存在指定的输出文件中。


**2.1.2 替换字符**

有时你会遇到一些文本字符串中的字符不方便在替换模式中使用的情况。Linux中一个流行的例子是正斜线。

替换文件中的路径名会比较麻烦。比如，如果想用C shell替换/etc/passwd文件中的bash shell，你必须这么做：
{% highlight string %}
# sed 's/\/bin\/bash/\bin\/csh/' /etc/passwd
{% endhighlight %}
由于正斜线通常用作字符串分隔符，因而如果它出现在模式文本中的话，你必须用反斜线来转义。则通常会带来一些困惑和错误。

要解决这个问题，sed编辑器允许选择其他字符来作为substitute命令中字符串分隔符：
{% highlight string %}
# sed 's!/bin/bash!/bin/csh!' /etc/passwd
{% endhighlight %}
在这个例子中，感叹号```!```被用作字符串分隔符，使得路径名很容易被读取和理解。

### 2.2 使用地址
默认情况下，在sed编辑器中使用的命令会作用于文本数据的所有行。如果只想将命令作用于特定某行或某些行，你必须用**行寻址**(line addressing).

在sed编辑器中有两种形式的行寻址：

* 行的数字范围

* 用文本模式来过滤出某行

两种形式都使用相同的格式来指定地址：
<pre>
[address]command
</pre>
也可以为特定地址将多个命令放在一起：
{% highlight string %}
address {
	command1
	command2
	command3
}
{% endhighlight %}
sed编辑器会将指定的每条命令只作用到匹配指定地址的行上。本节将会演示如何在sed编辑器脚本中使用两种寻址方法。

**1） 数字方式的行寻址**
当使用数字方式的行寻址时，你可以用它们在文本流中的行位置来引用行。sed编辑器会将文本流中第一行分配为第一行，然后继续按顺序为新行分配行号。

在命令中指定的地址可以是单个行号，或是用起始行号、逗号以及结尾行号指定的一定范围内的行。这里有个sed命令作用到指定某行的例子：
<pre>
# sed '2s/dog/cat/' data1
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
</pre>
色的编辑器只会修改地址指定的第二行的文本。这里有另一个例子，这次使用了行地址范围：
<pre>
# sed '2,3s/dog/cat/' data1
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy dog.
</pre>
如果想将一条命令作用到文本中某行开始到结尾的所有行，你可以使用特殊地址——美元符:
<pre>
# sed '2,$s/dog/cat/' data1
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy cat.
</pre>
你可能不知道文本中到底有多少行数据，所以美元符用起来通常很方便。

**2) 使用文本模式过滤器**

另一种限制命令作用到哪些行上的方法会稍微复杂一些。sed编辑器允许指定文本模式来过滤出命令要作用的行。格式如下：
<pre>
/pattern/command
</pre>
必须用正斜线将要指定的pattern封起来。sed编辑器会将该命令只作用到包含指定文本模式的行上。举个例子，如果你想只修改用户```ivan1001```的默认shell，那么你会用sed命令：
<pre>
# grep ivan1001 /etc/passwd
ivan1001:x:1000:1000:Ubuntu16.04,,,:/home/ivan1001:/bin/bash

# sed '/ivan1001/s/bash/csh/' /etc/passwd
root:x:0:0:root:/root:/bin/bash
...
ivan1001:x:1000:1000:Ubuntu16.04,,,:/home/ivan1001:/bin/csh
</pre>
该命令只作用到匹配文本模式的行上。虽然使用固定文本模式能帮你过滤出特定的值，就跟上面这个用户名的例子一样，但你能做的通常有些局限。sed编辑器在文本模式中会采用一种称为**正则表达式**(regular expression)的特性来帮助创建能很好地匹配。

正则表达式允许创建高级文本模式——匹配表达式来匹配各种数据。这些表达式会将一系列通配符、特殊字符以及固定文本字符组合到一起，生成一个几乎能匹配任何文本情形的简练模式。正则表达式是shell脚本编程中比较可怕的部分之一。


**3） 组合命令**

如果需要在单行上执行多条命令，可以用花括号将多条命令组合在一起。sed编辑器会处理列在地址行的每条命令：
{% highlight string %}
# sed '2{
> s/fox/elephant/
> s/dog/cat/
> }' data1
The quick brown fox jumps over the lazy dog.
The quick brown elephant jumps over the lazy cat.
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
{% endhighlight %}
两条命令都会用到该地址上。当然，也可以在一组命令前指定一个地址范围：
{% highlight string %}
# sed '3,${
> s/brown/green/
> s/lazy/active/
> }' data1
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
The quick green fox jumps over the active dog.
The quick green fox jumps over the active dog.
{% endhighlight %}
sed编辑器会将所有命令作用到该地址范围内的所有行上。














<br />
<br />

**[参看]**

1. [cat](https://blog.csdn.net/apache0554/article/details/45508631)




<br />
<br />
<br />


