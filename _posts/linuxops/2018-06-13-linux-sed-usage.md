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
如果要修改原文件，可以添加```-i```选项。例如：
<pre>
# sed -i 's/dog/cat/g' ./data1
# cat data1
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy cat.
The quick brown fox jumps over the lazy cat.
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
{% endhighlight %}
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

### 2.3 删除行
文本替换命令不是sed编辑器中的唯一命令。如果需要删除文本流中的特定行，可以用删除(delete)命令。

删除命令```d```名副其实，它会删除匹配指定寻址模式的所有行。使用删除命令时要特别小心，因为你忘记了加一个寻址模式的话，流中的所有文本行都会被删除：
{% highlight string %}
# cat data1
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.
The quick brown fox jumps over the lazy dog.

# sed 'd' data1
{% endhighlight %}
很明显，删除命令在和指定地址一起使用使用时最有用。则允许你从数据流中删除特定的文本行，不管是通过行号指定：
{% highlight string %}
# tee data7 <<EOF
> This is line number 1
> This is line number 2
> This is line number 3
> This is line number 4
> EOF
This is line number 1
This is line number 2
This is line number 3
This is line number 4

# sed '3d' data7
This is line number 1
This is line number 2
This is line number 4
{% endhighlight %}

还可以通过特定行范围指定：
<pre>
# sed '2,3d' data7
This is line number 1
This is line number 4
</pre>
或是通过文件尾特殊字符：
<pre>
# sed '3,$d' data7
This is line number 1
This is line number 2
</pre>
sed编辑器的模式匹配特性也适用于删除命令：
{% highlight string %}
# sed '/number 1/d' data7
This is line number 2
This is line number 3
This is line number 4
{% endhighlight %}

sed编辑器会删掉包含匹配指定模式的文本行。
<pre>
说明： 记住，sed编辑器不会修改原始文件。你删除的行只是从sed编辑器的输出中消失了。
原始文件仍然包含那些“删掉” 的行
</pre>

你可以用两个文本模式来删除某个范围的行，但是这么做时要小心。你指定的第一个模式会“打开”行删除功能，第二个模式会“关闭”行删除功能。sed编辑器会删除两个指定行之间的所有行（包括指定的行）：
<pre>
# cat data6
This is line number 1.
This is line number 2.
This is line number 3.
This is line number 4.
# sed '/1/,/3/d' data6
This is line number 4.
</pre>

除此之外，你要特别小心，因为只要sed编辑器在数据流中匹配到了开始模式，删除功能就会打开。则可能会导致意外的结果：
<pre>
# cat data8
This is line number 1.
This is line number 2.
This is line number 3.
This is line number 4.
This is line number 1 again.
This is text you want to keep.
This is the last line in the file.
# sed '/1/,/3/d' data8
This is line number 4
</pre>
第二个出现数字```1```的行再次触发了删除命令，删除了数据流中的剩余行，因为停止模式再没找到。当然，如果你指定了一个从未在文本中出现的停止模式，会出现另一个明显的问题：
<pre>
# sed '/1/,/5/d' data8
</pre>
因为删除功能在匹配到第一个模式的时候打开了，但一直没匹配到结束模式，所以整个数据流都被删掉了。

### 2.4 插入和附加文本
如你所期望的，跟其他编辑器类似，sed编辑器允许你向数据流插入和附加文本行。两个操作的区别可能比较费解：

* 插入(insert)命令```i```会在指定的行前增加一个新行

* 追加(append)命令```a```会在指定行后增加一个新行

这两条命令的费解之处在于它们的格式。不能在单个命令行上使用这两条命令。你必须指定是要将行插入还是附加到另一行。格式如下：
<pre>
sed '[address]command\new line'
</pre>
上面new line中的文本将会出现在sed编辑器输出中你指定的位置。记住，当使用插入命令时，文本会出现在数据流文本的前面：
{% highlight string %}
# echo "This is line 2" | sed 'i\This is line 1'
This is line 1
This is line 2
{% endhighlight %}
当使用附加命令时，文本会出现在数据流文本的后面：
{% highlight string %}
# echo "This is line 2" | sed 'a\This is line 1'
This is line 2
This is line 1
{% endhighlight %}
在命令行界面提示符上使用sed编辑器时，你会看到次提示符来提醒输入新的行数据。你必须在该行完成sed编辑器命令。一旦你输入了结尾的单引号，bash shell就会执行该命令了：
{% highlight string %}
# echo "This is line 2" | sed 'i\
> This is line 1.'
This is line 1.
This is line 2
{% endhighlight %}
这样能够给数据流中的文本前面或后面添加文本，但是如果要添加到数据流里面呢？

要给数据流行中插入或附加数据，你必须用寻址来告诉sed编辑器你想让数据出现在什么位置。你可以在用这些命令时只指定一个行地址。可以匹配一个数字行号或文本模式，但不能用地址区间。这符合逻辑，因为你只能将文本插入或附加到单个行的前面或后面，而不能是行区间的前面或后面。

下面是将一个新行插入到数据流第3行前面的例子：
<pre>
# cat data10
This is line number 1.
This is line number 2.
This is line number 3.
This is line number 4.
# cat data10 | sed '3i\This is a insert line.'
This is line number 1.
This is line number 2.
This is a insert line.
This is line number 3.
This is line number 4.
</pre>

下面是将一个新行附加到数据流中第3行后面的例子：
<pre>
# sed '3a\This is a insert line.' data10
This is line number 1.
This is line number 2.
This is line number 3.
This is a insert line.
This is line number 4.
</pre>
它使用跟插入命令相同的过程，只是将新文本行方放到了指定行号的后面。如果你有一个多行数据流，你要将新行附加到数据流的末尾，只要用代表数据最后一行的美元符号($)就可以了：
<pre>
# sed '$a\This is a insert line.' data10
This is line number 1.
This is line number 2.
This is line number 3.
This is line number 4.
This is a insert line.
</pre>
同样的方法也适用于你要再数据流开始的地方增加一个新行。只要在第一行之前插入新行就可以了。

要插入或附加多行文本，你必须对新行文本中的每一行使用反斜线，直到你要插入或附加的文本的最后一行：
{% highlight string %}
# sed '$a^Chis is an insert line.' data10
# sed '1i\
> This is one line of new text.\
> This is another line of new text.' data10
This is one line of new text.
This is another line of new text.
This is line number 1.
This is line number 2.
This is line number 3.
This is line number 4.
{% endhighlight %}
指定的两行都会添加到数据流中。

### 2.5 修改行
修改(change)命令允许修改数据流中整行文本的内容。它跟插入和附加命令的工作机制一样，你必须在sed命令中单独指定新行：
{% highlight string %}
# cat data10
This is line number 1.
This is line number 2.
This is line number 3.
This is line number 4.
# sed '3c\
> This is a changed line of text' data10
This is line number 1.
This is line number 2.
This is a changed line of text
This is line number 4.
{% endhighlight %}
在这个例子中，sed编辑器会修改第3行中的文本。你也可以用文本模式来寻址：
{% highlight string %}
# sed '/number 3/c\
This is a changed line of text.' data10
This is line number 1.
This is line number 2.
This is a changed line of text.
This is line number 4.
{% endhighlight %}
文本模式修改命令会修改它匹配的数据流中的任意文本行：
{% highlight string %}
# cat data8
This is line number 1.
This is line number 2.
This is line number 3.
This is line number 4.
This is line number 1 again.
This is text you want to keep.
This is the last line in the file.

# sed '/number 1/c\
> This is a changed line of text' data8
This is a changed line of text
This is line number 2.
This is line number 3.
This is line number 4.
This is a changed line of text
This is text you want to keep.
This is the last line in the file.
{% endhighlight %}

你可以在修改命令中使用地址区间，但结果可能并不是你想要的：
{% highlight string %}
# cat data7
This is line number 1
This is line number 2
This is line number 3
This is line number 4

# sed '2,3c\
> This is a changed line of text.' data7
This is line number 1
This is a changed line of text.
This is line number 4
{% endhighlight %}
sed编辑器会用这一行文本来替换数据流中的两行文本，而不是逐一修改那两行文本。

## 2.6 转换命令
转换(transform,y)命令是唯一可以处理单个字符的sed编辑器命令。转换命令格式如下：
<pre>
[address]y/inchars/outchars/
</pre>
转换命令会进行inchars和outchars值的一对一映射。inchars中第一个字符会被转换为outchars中第一个字符，第二个字符会被转换成outchars中的第二个字符。这个映射会一直持续到处理完指定字符。如果inchars和outchars长度不同，sed编辑器会产生一条错误消息。

如下有个使用转换命令的简单例子：
{% highlight string %}
# cat data8
This is line number 1.
This is line number 2.
This is line number 3.
This is line number 4.
This is line number 1 again.
This is text you want to keep.
This is the last line in the file.

# sed 'y/123/789/' data8
This is line number 7.
This is line number 8.
This is line number 9.
This is line number 4.
This is line number 7 again.
This is text you want to keep.
This is the last line in the file
{% endhighlight %}

如你在输出中看到的，inchars模式中指定字符的每个实例都会被替换成outchars模式中相同位置的那个字符。

转换命令是一个全局命令，也就是说，它会自动替换文本行中找到的指定字符的所有实例，而不会考虑它们出现的位置：
{% highlight string %}
# echo "This 1 is a test of 1 try." | sed 'y/123/456/'
This 4 is a test of 4 try.
{% endhighlight %}
sed编辑器替换了文本行中匹配到的字符“1”的两个实例。你无法限定只更改该字符出现的某个地方。

### 2.7 回顾打印
我们在前面2.1节中介绍了如何使用```p标记```和替换命令显示sed编辑器修改过的行。这里有3条也能用来打印数据流中的信息的命令：

* 小写p命令用来打印文本行

* 等号（=）命令用来打印行号

* l(小写L）命令用来列出行

1） **打印行**

跟替换命令中的p标记类似，p命令可以打印sed编辑器输出输出中的一行。如果只用这个命令，也没什么特别：
{% highlight string %}
# echo "This is a test." | sed 'p'
This is a test.
This is a test.
{% endhighlight %}
它打印出的数据文本是你已经知道的。打印命令最常见的用法是打印包含匹配文本模式的行：
{% highlight string %}
# cat data7
This is line number 1
This is line number 2
This is line number 3
This is line number 4

# sed -n '/number 3/p' data7
This is line number 3
{% endhighlight %}
在命令行上用```-n```选项，你就能禁止其他行，只打印包含匹配文本模式的行。

你也可以用它来快速打印数据流中的一些行:
{% highlight string %}
# sed -n '2,3p' data7
This is line number 2
This is line number 3
{% endhighlight %}
如果需要在修改之前查看行，那么你可以使用打印命令，比如与替换或修改命令一起使用。你可以创建一个脚本来在修改行之前显示该行：
{% highlight string %}
# sed -n '/3/{
> p
> s/line/test/p
> }' data7
This is line number 3
This is test number 3
{% endhighlight %}

sed编辑器命令会查找包含数字“3”的行，然后执行两条命令。首先，脚本用p命令来打印出该行原来的版本；然后它用s命令替换文本，并用p标记打印出了最终文本。输出同时显示了原来的行文本和新的行文本。

2） **打印行号**

等号(=)命令会打印行在数据流中的当前行号。行号由数据流中的换行符决定。每次数据流中出现一个换行符，sed编辑器会认为它结束了一行文本：
{% highlight string %}
# sed '=' data1
1
The quick brown fox jumps over the lazy dog.
2
The quick brown fox jumps over the lazy dog.
3
The quick brown fox jumps over the lazy dog.
4
The quick brown fox jumps over the lazy dog.
{% endhighlight %}
sed编辑器在实际的文本行出现前打印了行号。如果你要在数据流中查找特定文本模式的话，等号命令能派得上用场：
{% highlight string %}
# cat data7
This is line number 1
This is line number 2
This is line number 3
This is line number 4

# sed -n '/number 4/{
> =
> p
> }' data7
4
This is line number 4
{% endhighlight %}
使用了```-n```选项，你就能让sed编辑器只显示包含匹配文本模式的行号和文本。

3) **列出行**

列出命令(l)允许打印数据流中的文本和不可打印的ASCII字符。任何不可打印字符都用它们的八进制值前加一个反斜线或标准C风格的命名法（用于常见的不可打印字符），比如```\t```来代表制表符：
{% highlight string %}
# cat data9
This    line    contains 
# sed -n 'l' data9
This\tline\tcontains\ttabs.$
{% endhighlight %}

制表符的位置显示的是```\t```。行尾的美元符表明这里是换行符。如果数据流包含了转义字符，则list命令会用八进制码来显示它。

### 2.8 用sed和文件一起工作
替换命令包含允许你和文件一起工作的标记。还有一些sed编辑器命令也允许你这么做，而不用替换文本。

1） **向文件写入**

```w命令```用来向文件写入行，命令的格式如下：
<pre>
[address]w filename
</pre>
filename可以指定为相对路径或绝对路径，但不管是哪种，运行sed编辑器的人都必须有文件的写权限。地址可以是sed中支持的任意类型的寻址方式，例如单个行号、文本模式或一系列行号或文本模式。

下面是将数据流中的前两行打印到一个文本文件中的例子：
{% highlight string %}
# cat data7
This is line number 1
This is line number 2
This is line number 3
This is line number 4
# sed '1,2w test' data7 
This is line number 1
This is line number 2
This is line number 3
This is line number 4
# cat test
This is line number 1
This is line number 2
{% endhighlight %}
当然，如果你不想让行显示到STDOUT上，你可以用```-n```选项。

如果你要基于一些公共文本值从主文件创建一份数据文件，比如下面的邮件列表中的，那么它会非常好用：
{% highlight string %}
# cat data11
Blum.Katie      Chicago. IL
Mullen.Riley    West Lafayette. IN
Snell.Haley     Ft.Wayne. IN
Woenker.Matthew         Springfield. IL
Wisecarver.Emma         Grant Park.  IL


# sed -n '/IN/w incustomers' data11
# cat incustomers 
Mullen.Riley    West Lafayette. IN
Snell.Haley     Ft.Wayne. IN
{% endhighlight %}
sed编辑器会只将包含文本模式的数据行写入目标文件。

2） **从文件读取数据**

你已经了解了如何在sed命令行上向数据流中插入或附加文本。读取命令(r)允许你将一个独立文件中的数据插入到数据流中。

读取命令的格式如下：
<pre>
[address]r filename
</pre>
filename参数指定了数据文件的绝对或相对路径名。你不能对读取命令使用地址区间，而只能指定单独一个行号或文本模式地址。sed编辑器会将文件中的文本插入到地址后：
{% highlight string %}
# cat data12
This is an add line.
This is the second line.

# sed '3r data12' data7
This is line number 1
This is line number 2
This is line number 3
This is an add line.
This is the second line.
This is line number 4
{% endhighlight %}
sed编辑器将数据文件中的所有文本行插入到数据流中了。同样的方法在使用文本模式地址时也适用：
{% highlight string %}
# sed '/number 3/r data12' data7
This is line number 1
This is line number 2
This is line number 3
This is an add line.
This is the second line.
This is line number 4
{% endhighlight %}
如果你要在数据流的末尾添加文本，只要用美元符($)就行了：
{% highlight string %}
# sed '$r data12' data7
This is line number 1
This is line number 2
This is line number 3
This is line number 4
This is an add line.
This is the second line.
{% endhighlight %}
读取命令的另一个很好的用途是，将它和删除命令一起使用来用另一文件中的数据替换文件中的占位文本。例如，假如你有个这样的保存在文本文件中的套用信件：
{% highlight string %}
# cat letter
Would the following people:
LIST
please report to the office.
{% endhighlight %}

套用信件将通用占位文本LIST放在名单的位置。要在占位文本后插入名单，你只要用读取命令就可以了。但这样的话，占位文本仍然会留在输出中。要删除占位文本，你可以用删除命令。结果看起来如下：
{% highlight string %}
# cat data11
Blum.Katie      Chicago. IL
Mullen.Riley    West Lafayette. IN
Snell.Haley     Ft.Wayne. IN
Woenker.Matthew         Springfield. IL
Wisecarver.Emma         Grant Park.  IL

# sed '/LIST/{
> r data11
> d
> }' letter
Would the following people:
Blum.Katie      Chicago. IL
Mullen.Riley    West Lafayette. IN
Snell.Haley     Ft.Wayne. IN
Woenker.Matthew         Springfield. IL
Wisecarver.Emma         Grant Park.  IL
please report to the office.
{% endhighlight %}
现在占位文本已经被替换成了数据文件中的名单。






<br />
<br />

**[参看]**

1. [cat](https://blog.csdn.net/apache0554/article/details/45508631)




<br />
<br />
<br />


