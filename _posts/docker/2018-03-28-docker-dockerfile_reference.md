---
layout: post
title: Dockerfile参考
tags:
- docker
categories: docker
description: Dockerfile参考
---


docker可以通过读取```Dockerfile```中的指令来自动构建镜像。一个```Dockerfile```就是一个文本文档，其中包含了编译一个镜像的所有命，然后调用```docker build```来创建出相应的镜像。

本章我们会讲述一下Dockefile中可以使用的命令。


<!-- more -->


## 1. 用法
```docker build```命令通过一个```Dockerfile```和```上下文环境```来创建docker镜像。构建的```上下文环境```就是就是一个指定的```PATH```或者```URL```。```PATH```是指本地文件系统中的一个目录；而```URL```是指一个git仓库地址。

一个```上下文环境```会被递归的处理。因此，一个```PATH```中的任何子目录或者```URL```下的所有子仓库或子模块都属于该上下文。如下的例子使用当前目录来作为构建上下文：
<pre>
# docker build .
Sending build context to Docker daemon  6.51 MB
</pre>

这个构建过程是由docker daemon来完成的，而并不是由docker client来完成。构建过程的第一步就是将整个上下文（recursively)发送给docker daemon。在大部分情况下，最好使用一个空目录来作为上下文环境，在该目录下存放Dockerfile文件，然后只添加一些必要的文件来构建该Dockerfile。

要在构建上下文中使用一个文件，```Dockerfile```必须通过相应的指令来引用到该文件，例如：```COPY```指令。为了提高整个构建的效率，可以通过添加一个```.dockerignore```文件来排除掉上下文目录中的一些文件或目录。

通常，构建时用到的```Dockerfile```名称为Dockerfile，并且被放在构建上下文的根目录。你也可以在```docker build```命令后通过使用```-f```选项来指定Dockerfile的存放位置。例如：
<pre>
# docker build -f /path/to/a/Dockerfile .
</pre>

也可以通过使用一个```-t```选项来为新构建的镜像指定```仓库```和```标签```:
<pre>
# docker build -t shykes/myapp .
</pre>

如果要为构建出来的镜像指定不同的仓库，则可以在```docker build```命令后添加多个```-t```选项：
<pre>
# docker build -t shykes/myapp:1.0.2 -t shykes/myapp:latest .
</pre>

在docker daemon运行Dockerfile中的指令之前，docker daemon会进行一个基本的检查，假如有语法错误的话则返回相应的报错：
<pre>
# docker build -t test/myapp .
Sending build context to Docker daemon 2.048 kB
Error response from daemon: Unknown instruction: RUNCMD
</pre>

```Docker daemon```会一条一条的执行Dockerfile中的指令。如果有必要，则会将每一条指令的执行结果提交到新的镜像中，最后会输出新构建的docker镜像的sha256摘要信息。构建完成后，```Docker Daemon```就会清除传送给它的上下文环境。值得注意的是，每一条指令都是独立运行的，并没有上下文关系，例如```RUN cd /tmp```指令执行后并不会影响到下一条指令的执行（即并不是在/tmp目录)。

在任何时候，docker都会复用缓存的镜像来加速```docker build```进程，这可以通过输出控制台中的```Using cache```信息看出。例如：
{% highlight string %}
# docker build -t svendowideit/ambassador .
Sending build context to Docker daemon 15.36 kB
Step 1/4 : FROM alpine:3.2
 ---> 31f630c65071
Step 2/4 : MAINTAINER SvenDowideit@home.org.au
 ---> Using cache
 ---> 2a1c91448f5f
Step 3/4 : RUN apk update &&      apk add socat &&        rm -r /var/cache/
 ---> Using cache
 ---> 21ed6e7fbb73
Step 4/4 : CMD env | grep _TCP= | (sed 's/.*_PORT_\([0-9]*\)_TCP=tcp:\/\/\(.*\):\(.*\)/socat -t 100000000 TCP4-LISTEN:\1,fork,reuseaddr TCP4:\2:\3 \&/' && echo wait) | sh
 ---> Using cache
 ---> 7ea8aef582cc
Successfully built 7ea8aef582cc
{% endhighlight %}

注意： 构建时所采用的缓存只会来源于有相同parent chain的本地镜像。

## 2. Dockerfile格式
Dockerfile的基本格式如下：
<pre>
# Comment
INSTRUCTION arguments
</pre>
其中指令部分是不区分大小写的，然而通常建议使用大写，并以此来区分arguments。

dockr按Dockerfile中指令的顺序执行。一个Dockerfile```必须以'FROM'指令起始```。```FROM```指令指定了你当前构建所采用的基础镜像。只有```ARG```指令可以出现在FROM指令之前，用于声明FROM指令所使用的参数。

Docker将以```#```开头的行作为注释行，除非该行是一个有效的```parser指令```。在行中的```#```标志都会被视为一个参数。例如：
<pre>
# Comment
RUN echo 'we are running some # of cool things'
</pre>

注意： 在注释中并不支持line continuation字符

## 3. Parser指令
```Parser指令```是可选的，如果存在的话会影响到后续Dockerfile的处理。```Parser指令```在镜像构建时并不会增加镜像的层数，并且并不会在构建时显示为一个单独的构建步骤。Parser指令被写作一种特殊类型的注释```# directive=value```, 并且一个单独的解析指令只能出现一次。

一旦一个注释行、空行或者构建指令被处理完成，Docker就不会再去寻找parser指令了，这时docker会将所有的解析指令当做注释。因此，所有的解析指令必须出现在Dockerfile的最前面。

Parser指令是不区分大小写的，然而通常将它们写成小写。在parser指令之后，通常会跟一个空行。parser指令不允许换行。针对上面的规则，如下都是无效的parser指令：

1） **无效解析指令： 换行**
<pre>
# direc \
tive=value
</pre>

2) **无效指令： 多次重复出现**
<pre>
# directive=value1
# directive=value2

FROM ImageName
</pre>

3) **无效指令： 出现在构建指令之后**

下面由于出现在构建指令之后，因此会被当做是注释：
<pre>
FROM ImageName
# directive=value
</pre>


4) **无效指令： 出现在注释行之后**

下面由于出现在注释行之后，因此并不是一个有效的解析指令:
<pre>
# About my dockerfile
# directive=value
FROM ImageName
</pre>

5) **无效指令： 出现在不能识别的指令之后**

下面因为```unknowndirective```不能被识别，因此不会被认为是一个解析指令，会被当做注释；而后续的```knowndirective```则由于出现在注释之后，因此也不是一个有效的解析指令。
<pre>
# unknowndirective=value
# knowndirective=value
</pre>

6) **解析指令等价性**

如下的解析指令是等价的（由于解析指令不区分大小写、不支持换行）：
<pre>
#directive=value
# directive =value
#	directive= value
# directive = value
#	  dIrEcTiVe=value
</pre>


7) **escape解析指令**

Dockerfile当前支持的解析指令有：escape指令。例如：
<pre>
# escape=\ (backslash)
</pre>
或者：
<pre>
# escape=` (backtick)
</pre>

```escape指令```用于设置Dockerfile的转义符号。假如没有被指定，默认情况下是```\```。

转义符号不但用于一行中的转义字符中，还可用于转义一个新行（即当做本行处理）。值得注意的是，不管Dockerfile中存不存在escape指令，```RUN```命令中的字符都不会发生转义，除非是在行尾部。


## 4. Environment替换
Environment变量（通过ENV指令定义）在Dockerfile中可以被用于很多指令中。在Dockerfile中可以用```$variable_name```或者```${variable_name}```的方式来引用，这两种方式都是相同的。```${variable_name}```形式在类似与```${foo}_bar```这样的情况下很实用。

同时```${variable_name}```语法形式也支持一些标准的```bash```修正：

* ```${variable:-word}```: 表示假如variable被定义，则结果为variable值；否则结果为word

* ```${variable:+word}```: 表示假如variable被定义，则结果为world；否则结果为空字符串

下面举一个使用Environment变量的例子( #后面作为解释）：
{% highlight string %}
FROM busybox
ENV foo /bar
WORKDIR ${foo}   # WORKDIR /bar
ADD . $foo       # ADD . /bar
COPY \$foo /quux # COPY $foo /quux
{% endhighlight %}

Environment变量可以出现在Dockerfile的下列指令中：
<pre>
ADD / COPY / ENV / EXPOSE / FROM / LABEL

STOPSIGNAL / USER / VOLUME / WORKDIR
</pre>

在整个指令中，环境变量替换都会用相同的值。例如：
<pre>
ENV abc=hello
ENV abc=bye def=$abc
ENV ghi=$abc
</pre>

上面def的值为```hello```; 而ghi的值为```bye```。

## 5. ```.dockerignore```文件

在Docker Client发送上下文给docker daemon之前，其会在上下文根目录寻找一个```.dockerignore```文件。假如该文件存在，Docker Client就会修改上下文以排除所匹配到的文件和目录。这可以避免发送一些不必要的大文件或敏感文件到docker daemon，并通过```ADD```或```COPY```命令添加到镜像中。

可以通过```.dockerignore```文件定义所需要忽略的文件列表。例如：
<pre>
# comment
*/temp*
*/*/temp*
temp?
</pre>

## 6. FROM指令

```FROM```指令格式如下：
{% highlight string %}
FROM <image> [AS <name>]

FROM <image>[:<tag>] [AS <name>]

FROM <image>[@<digest>] [AS <name>]
{% endhighlight %}
```FROM```指令初始化整个构建stage，并为后续的指令设置```Base Image```。通常情况下，一个Dockerfile文件以```FROM```指令起始，并且该基础镜像是可以为任何有效的镜像。

* 在Dockerfile中，```ARG```指令是唯一可以放在```FROM```指令之前的指令

* 在一个单独的Dockerfile中，```FROM```指令可以出现多次，以此来构建出多个镜像

* 在```FROM```指令后可以为该构建stage命名（通过```AS name```)，并在后续的```FROM```指令或者```COPY --from=<name|index>```指令中来引用该stage构建出的镜像

* 可以添加一个```tag```或者```digest```(可选），假如没有添加的话则默认为latest

## 7. ARG指令与FROM指令的交互
```FROM```指令支持由```ARG```指令所声明的变量。
<pre>
ARG  CODE_VERSION=latest
FROM base:${CODE_VERSION}
CMD  /code/run-app

FROM extras:${CODE_VERSION}
CMD  /code/run-extras
</pre>

在```FROM```指令之前的```ARG```指令是不属于当前构建stage的，因此其不能在```FROM```指令之后继续使用。而为了要继续使用```FROM```指令之前的```ARG```指令的默认值，我们可以再次声明：
{% highlight string %}
ARG VERSION=latest
FROM busybox:$VERSION
ARG VERSION
RUN echo $VERSION > image_version
{% endhighlight %}

## 8. RUN指令
```RUN```指令有两种形式：

* ```RUN <command>```: 此种形式为shell形式，即该指令在shell环境中运行。在Linux操作系统上默认是```/bin/sh -c```来执行；在Windows上默认是```cmd -S -C```来执行

* ```RUN ["executable","param1","param2"]```: 此种形式为exec形式

```RUN```指令会在当前镜像基础上的一个新layer执行command，并且将结果提交到该新layer。产生的提交后的镜像又会被用于Dockerfile中的下一步。```exec```形式可以避免shell字符串整理，且可以指定在另外一种非默认shell环境下执行命令。例如：
<pre>
RUN ["/bin/bash", "-c", "echo hello"]
</pre>
说明： exec形式中，是以json形式来解析命令，因此必须使用双引号。

## 9. CMD指令

```CMD指令```有三种形式：

* ```CMD ["executable","param1","param2"]```: exec形式，这也是我们所推荐的形式

* ```CMD ["param1","param2"]```: 作为ENTRYPOINT的默认参数

* ```CMD command param1 param2```: shell形式

在一个Dockerfile中只能有一个```CMD```指令。假如有多个CMD指令的话，则只有最后一个CMD有效。```CMD指令```的主要目的提供一个默认操作。该默认操作可以包括一个可执行动作；或者为ENTRYPOINT提供相应的参数（形式2）

对于```CMD```指令的shell形式，则```<command>```会在```/bin/sh -c```中执行：
<pre>
FROM ubuntu
CMD echo "This is a test." | wc 
</pre>

假若你并不想要在shell中执行```<command>```，则必须指定可执行文件的全路径。这也是我们所推荐的形式：
<pre>
FROM ubuntu
CMD ["/usr/bin/wc","--help"]
</pre>


假如你想要容器每次都运行同一个可执行文件，你应该考虑```CMD```结合```ENTRYPOINT```一起使用。

假如你为```docker run```指定了相应的参数，则这些参数会覆盖默认的CMD指令。

{% highlight string %}
注意： 不要把RUN指令与CMD指令给搞混了。RUN指令会运行一个命令然后提交结果到layer；
而CMD指令在构建时并不会执行任何东西，其只是为Image指定一个期望执行的命令
{% endhighlight %}

## 10. LABEL指令
基本格式如下：
{% highlight string %}
LABEL <key>=<value> <key>=<value> <key>=<value> ...
{% endhighlight %}

```LABEL```指令为一个镜像添加元数据。一个LABEL是一个键值对。例如：
<pre>
LABEL "com.example.vendor"="ACME Incorporated"
LABEL com.example.label-with-value="foo"
LABEL version="1.0"
LABEL description="This text illustrates \
that label-values can span multiple lines."
</pre>
一个镜像可以有多个label，你可以在一行中指定多个label，也可以分开多行来指定。label是可以被继承的，```FROM```中镜像的label可以被子镜像继承。假如有两个labe的key相同，但是value不同，则后一个label会覆盖前一个label的值。

## 11. MAINTAINER指令

当前本指令已过时，请用```LABEL```指令代替。

<br />
<br />
<br />

