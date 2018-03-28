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

```Docker daemon```会一条一条的执行Dockerfile中的指令。



<br />
<br />
<br />

