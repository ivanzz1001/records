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

```COPY```指令有两个格式：

* ```COPY [--chown=<user>:<group>] <src>... <dest>```

* ```COPY [--chown=<user>:<group>] ["<src>",... "<dest>"]```: 本格式支持路径中带有空格

<pre>
说明： --chown特性只在Linux上通过Dockerfile构建有效，在Windows上是无效的。
</pre>

COPY指令从src处拷贝文件或目录，并且将它们添加到容器的dest路径。可以通过```<src>```指定多个路径，但是指定的文件路径或目录都会被解释称为相对于build context的路径。

```<dest>```可以是一个绝对路径，或者是相对路径（相对于WORKDIR)。例如：

<pre>
COPY hom* /mydir/        # adds all files starting with "hom"
COPY hom?.txt /mydir/    # ? is replaced with any single character, e.g., "home.txt"
</pre>


## 5. ENTRYPOINT指令
ENTRYPOINT指令有两种形式：

* ```ENTRYPOINT ["executable", "param1", "param2"]```: 这是exec形式，也是我们所推荐的

* ```ENTRYPOINT command param1 param2```: 这是shell形式

一个ENTRYPOINT指令允许你将一个容器配置成为可执行的。例如，可以通过如下默认的方式启动nginx，监听80端口：
<pre>
docker run -i -t --rm -p 80:80 nginx
</pre>
这样```docker run <image>```后的命令行参数都会追加加到exec形式的ENTRYPOINT后边，并覆盖```CMD```指令指定的参数。这就允许你向entrypoint传递参数。

shell形式的ENTRYPOINT将不能够使用```CMD```指令或```run```命令行提供的参数，并且会作为```/bin/sh -c```的一个子指令来执行，这样就不能够直接传递信号到该可执行文件。并且也不能以```PID 1```来执行，因此也收不到Unix发送过来的信号，这样将不能够通过如下方式来停止容器：
{% highlight string %}
docker stop <container>
{% endhighlight %}

假如一个Dockerfile中有多个ENTRYPOINT，则只有最后一个ENTRYPOINT有效。

### 5.1 Exec形式的ENTRYPOINT例子
你可以使用exec形式的ENTRYPOINT来设置相对固定的默认命令行参数，然后使用CMD指令来设置一些额外的、很可能经常被改变的参数：
{% highlight string %}
FROM ubuntu
ENTRYPOINT ["top", "-b"]
CMD ["-c"]
{% endhighlight %}
你可以通过如下的方式来运行该镜像：
<pre>
# docker run -it --rm --name test  top -H
</pre>

如下的Dockerfile展示了使用ENTRYPOINT来在前台运行一个Apache:
{% highlight string %}
FROM debian:stable
RUN apt-get update && apt-get install -y --force-yes apache2
EXPOSE 80 443
VOLUME ["/var/www", "/var/log/apache2", "/etc/apache2"]
ENTRYPOINT ["/usr/sbin/apache2ctl", "-D", "FOREGROUND"]
{% endhighlight %}

### 5.2 Shell形式的ENTRYPOINT例子

shell形式的ENTRYPOINT会以/bin/sh -c来执行，该形式会使用shell处理来替换shell环境变量，并且会忽略任何```CMD```指令或docker run命令行传递进来的参数。为了确保能够用docker stop来停止ENTRYPOINT指定的可执行程序，你需要添加exec来执行：
{% highlight string %}
FROM ubuntu
ENTRYPOINT exec top -b
{% endhighlight %}


## 6. VOLUME指令
VOLUME指令格式如下：
<pre>
VOLUME ["/data"]
</pre>
本指令用于在容器中创建挂载点

## 7. USER指令
指令格式如下：
{% highlight string %}
USER <user>[:<group>] or
USER <UID>[:<GID>]
{% endhighlight %}
```USER```指令用于设置当你运行镜像时，以什么样的用户身份执行。它会影响到RUN、CMD、ENTRYPOINT指令。

## 8. WORKDIR指令
指令格式如下：
<pre>
WORKDIR /path/to/workdir
</pre>

```WORKDIR```指令用于设置RUN、CMD、ENTRYPOINT、ADD、COPY指令的工作目录。

## 9. ARG指令
指令格式如下：
{% highlight string %}
ARG <name>[=<default value>]
{% endhighlight %}
```ARG```指令可以用于定义一个变量，用户在构建镜像时可以通过在```docker build```命令后添加```--build-arg <varname>=<value>```标志来传递参数。假如在构建时传递了在Dockerfile中不存在的参数，则在构建时可能输入如下警告：
<pre>
[Warning] One or more build-args [foo] were not consumed.
</pre>
一个Dockerfile可以包含1个或多个```ARG```指令，例如：
{% highlight string %}
FROM busybox
ARG user1
ARG buildno
{% endhighlight %}

### 9.1 ARG指令的使用
你可以使用```ARG```指令或者```ENV```指令来为```RUN```指令指定参数，但假如ARG指令与ENV指令使用相同的变量名称，则ENV指令总是会覆盖ARG指令。 

## 10. ONBUILD指令
指令格式如下：
<pre>
ONBUILD [INSTRUCTION
</pre>
```ONBUILD```指令用于添加```trigger```指令到镜像A。后续如果构建镜像B，且以镜像A作为基础镜像(FROM A)的话，则会触发A中设置的ONBUILD指令。注意ONBUILD指令不会传递到```孙子```镜像的构建中。

## 11. STOPSIGNAL指令
指令格式如下：
<pre>
STOPSIGNAL signal
</pre>
用于设置传递何种信号以让容器退出。

## 12. HEALTHCHECK指令
HEALTHCHECK指令有两种形式：

* ```HEALTHCHECK [OPTIONS] CMD command```: 通过在容器中运行一个命令来检查容器的健康状况

* ```HEALTHCHECK NONE```: 禁止继承任何base image的health check

HEALTHCHECK指令告诉docker如何检查container是否仍在工作。这可以用于检查如web服务器一直在主循环中卡住，从而不能接受新的连接，即使web server进程仍然在继续运行这种状况。

当一个容器指定了HEALTHCHECK，容器就会多一个```health status```状态，该状态的初始值为```starting```。当健康检查通过，状态就会变成```healthy```，而在连续多次检测失败之后就会变为```unhealthy```。

```[OPTIONS]```可以为：

* ```--interval=DURATION```: 默认值为30s

* ```--timeout=DURATION```: 默认值为30s

* ```--start-period=DURATION```: 默认值为0

* ```--retries=N```: 默认值为3

在container启动之后interval时间内会进行第一次健康检查，之后每隔interval时间再进行一次检查。如果进行一次健康检查时在timeout时间内都没有返回结果，则认为此次检查失败。

以cmd的退出状态作为容器的健康状态，可能的取值有：

* 0: 成功，容器当前处于healthy状态，可以使用

* 1: 容器当前处于unhealthy状态，当前不能正常工作

* 2: 保留，暂未使用

例如我们可以通过如下命令每隔5分钟检查当前web能否在3s内提供主页服务：
<pre>
HEALTHCHECK --interval=5m --timeout=3s \
  CMD curl -f http://localhost/ || exit 1
</pre>
为了有助于调试，健康检查的标准输出及标准错误都会被存储到health status中，可以通过docker inspect来查看（当前只会保存钱4096个字节，因此这种输出尽量短）


## 13. SHELL指令
指令格式如下：
<pre>
SHELL ["executable", "parameters"]
</pre>
```SHELL```指令允许覆盖shell命令执行时所用的默认shell。默认情况下Linux中的shell为```["/bin/sh","-c"]```, 而windows中shell为```["cmd","/S","/C"]```。

SHELL指令可以出现多次，每一次出现都会覆盖上一次。例如：
{% highlight string %}
FROM microsoft/windowsservercore

# Executed as cmd /S /C echo default
RUN echo default

# Executed as cmd /S /C powershell -command Write-Host default
RUN powershell -command Write-Host default

# Executed as powershell -command Write-Host hello
SHELL ["powershell", "-command"]
RUN Write-Host hello

# Executed as cmd /S /C echo hello
SHELL ["cmd", "/S", "/C"]
RUN echo hello
{% endhighlight %}

## 14. Dockerfile示例
{% highlight string %}
# Nginx
#
# VERSION               0.0.1

FROM      ubuntu
LABEL Description="This image is used to start the foobar executable" Vendor="ACME Products" Version="1.0"
RUN apt-get update && apt-get install -y inotify-tools nginx apache2 openssh-server
# Firefox over VNC
#
# VERSION               0.3

FROM ubuntu

# Install vnc, xvfb in order to create a 'fake' display and firefox
RUN apt-get update && apt-get install -y x11vnc xvfb firefox
RUN mkdir ~/.vnc
# Setup a password
RUN x11vnc -storepasswd 1234 ~/.vnc/passwd
# Autostart firefox (might not be the best way, but it does the trick)
RUN bash -c 'echo "firefox" >> /.bashrc'

EXPOSE 5900
CMD    ["x11vnc", "-forever", "-usepw", "-create"]
{% endhighlight %}


<br />
<br />

**[参考]**

1. [habor](https://github.com/vmware/harbor)

2. [企业级Docker Registry开源工具Harbor的介绍以及使用指南](https://my.oschina.net/xxbAndy/blog/786712)

3. [docker registry接入ceph Swift API](http://blog.51cto.com/bingdian/1893658)
<br />
<br />
<br />

