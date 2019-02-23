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






<br />
<br />

**[参看]**

1. [cat](https://blog.csdn.net/apache0554/article/details/45508631)




<br />
<br />
<br />


