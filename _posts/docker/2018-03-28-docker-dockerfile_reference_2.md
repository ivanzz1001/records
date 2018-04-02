---
layout: post
title: Dockerfile参考（2）
tags:
- docker
categories: docker
description: Dockerfile参考（2）
---


我们接着上一张继续讲解Dockerfile中相关的指令。


<!-- more -->


## 1. EXPOSE指令

指令格式如下：
{% highlight string %}
EXPOSE <port> [<port>/<protocol>...]
{% endhighlight %}

```EXPOSE```指令用于通知docker，该容器在运行时监听了某个指定网络上的port。你可以指定是TCP端口还是UDP端口，假如没有指定的话，则默认为TCP端口。

```EXPOSE```指令实际上并不会发布该端口，它的功能是作为文档的性质告诉镜像的构建者及使用者，docker中的应用程序监听了某个端口。在运行容器的过程中，为了publish该端口，可以在执行```docker run```命令的时候添加```-p```选项，也可以使用```-P```选项来publish所有已经exposed的端口到high-order端口处。例如：
<pre>
EXPOSE 80/tcp
EXPOSE 80/udp
</pre>
在上面这种情况下，假如你使用```docker run -P```来运行，docker会在host高端口处映射两个端口到container的80/tcp与80/udp。

## 2. ENV指令
```ENV```指令格式信息如下：
{% highlight string %}
ENV <key> <value>
ENV <key>=<value> ...
{% endhighlight %}
```ENV```指令用于设置环境变量的值，环境变量的值存在于后续的整个构建过程。通过ENV指令设置的环境变量将会持久化到一个运行的容器中，你可以通过```docker inspect```来查看。

例子：
<pre>
ENV myName John Doe
ENV myDog Rex The Dog
ENV myCat fluffy
</pre>


## 3. ADD指令
```ADD指令```有两种形式：

* ```ADD [--chown=<user>:<group>] <src>... <dest>```

* ```ADD [--chown=<user>:<group>] ["<src>",... "<dest>"]```: 本形式主要是处理包含空格的路径
<pre>
说明： --chown特性只在Linux上通过Dockerfile构建有效，在Windows上是无效的。
</pre>

ADD指令会从src处拷贝文件、目录到dest处。可以指定多个```src```源，但假如该源是文件或目录的话，则源路径会被解释为相对于build context。此外，src可以包含匹配：
<pre>
ADD hom* /mydir/        # adds all files starting with "hom"
ADD hom?.txt /mydir/    # ? is replaced with any single character, e.g., "home.txt"
</pre>
而对于```dest```,如果指定为相对路径，则是相对于```WORKDIR```：
<pre>
ADD test relativeDir/          # adds "test" to `WORKDIR`/relativeDir/
ADD test /absoluteDir/         # adds "test" to /absoluteDir/
</pre>

## 4. COPY指令




<br />
<br />

**[参考]**

1. [habor](https://github.com/vmware/harbor)

2. [企业级Docker Registry开源工具Harbor的介绍以及使用指南](https://my.oschina.net/xxbAndy/blog/786712)

3. [docker registry接入ceph Swift API](http://blog.51cto.com/bingdian/1893658)
<br />
<br />
<br />

