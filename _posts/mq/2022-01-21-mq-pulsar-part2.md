---
layout: post
title: Pulsar单机版安装
tags:
- mq
categories: mq
description: Pulsar单机版安装
---


针对本地开发(local development)及测试(testing)，我们可以在主机上以单机模式运行Pulsar。单机模式(standalone mode)包括Pulsar broker，必要的Zookeeper以及BookKeeper组件，并运行在同一个JVM进程中。

>Pulsar in production?
>
>If you're looking to run a full production Pulsar installation, see the [Deploying a Pulsar instance guide](https://pulsar.apache.org/docs/en/deploy-bare-metal).

<!-- more -->

## 1. Install Pulsar standalone

本文介绍单机版Pulsar安装的每一个步骤。

### 1.1 System requirements

目前Pulsar可以运行在64位```macOS```、```Linux```以及```Windows```上。要使用Pulsar，必须安装64-bit JRE/JDK 8或更高版本。

关于JDK的安装，请参看相关文档，这里不做介绍。当前我们的系统环境及Java版本如下：
{% highlight string %}
# uname -a
Linux localhost.localdomain 3.10.0-514.el7.x86_64 #1 SMP Tue Nov 22 16:42:41 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
# cat /etc/redhat-release 
CentOS Linux release 7.3.1611 (Core)

# java -version
java version "1.8.0_301"
Java(TM) SE Runtime Environment (build 1.8.0_301-b09)
Java HotSpot(TM) 64-Bit Server VM (build 25.301-b09, mixed mode)
{% endhighlight %}

>Tip
>
>By default, Pulsar allocates 2G JVM heap memory to start. It can be changed in ```conf/pulsar_env.sh``` file under ```PULSAR_MEM```. This is extra options passed into ```JVM```

>Note
>
>Broker is only supported on 64-bit JVM.

### 1.2 Install Pulsar using binary release

我们可以通过如下方式来下载Pulsar:

* download from the Apache mirror ([Pulsar 2.9.1 binary release](https://archive.apache.org/dist/pulsar/pulsar-2.9.1/apache-pulsar-2.9.1-bin.tar.gz))

* download from the Pulsar [downloads page](https://pulsar.apache.org/download)

* download from the Pulsar [releases page](https://github.com/apache/pulsar/releases/latest)

* use wget:
<pre>
# $ wget https://archive.apache.org/dist/pulsar/pulsar-2.9.1/apache-pulsar-2.9.1-bin.tar.gz
</pre>

当前最新稳定版为```2.9.1```，我们使用该版本。
<pre>
# mkdir pulsar-inst
# cd pulsar-inst
# wget https://dlcdn.apache.org/pulsar/pulsar-2.9.1/apache-pulsar-2.9.1-bin.tar.gz --no-check-certificate

# ls -alh
total 322M
drwxr-xr-x.  2 root root   44 Feb  7 01:37 .
dr-xr-x---. 16 root root 4.0K Feb  7 01:34 ..
-rw-r--r--.  1 root root 322M Dec 16 04:31 apache-pulsar-2.9.1-bin.tar.gz
</pre>

下载完成后解压```apache-pulsar-2.9.1-bin.tar.gz```如下：
{% highlight string %}
# tar -zxvf apache-pulsar-2.9.1-bin.tar.gz
# cd apache-pulsar-2.9.1
# ls
bin  conf  examples  instances  lib  LICENSE  licenses  NOTICE  README
{% endhighlight %}

1) **What your package contains**

在Pulsar二进制包中初始含有如下目录：

* bin: Pulsar命令行工具，比如[pulsar](https://pulsar.apache.org/docs/en/reference-cli-tools#pulsar)、[pulsar-admin](https://pulsar.apache.org/tools/pulsar-admin/)

* conf: Pulsar配置文件，包含[broker configuration](https://pulsar.apache.org/docs/en/reference-configuration#broker)、[ZooKeeper configuration](https://pulsar.apache.org/docs/en/reference-configuration#zookeeper)等

* examples: 一个包含有[Pulsar Functions](https://pulsar.apache.org/docs/en/functions-overview)示例的Java ```JAR```文件。

* lib: Pulsar所用到的一些```JAR```文件

* licenses: ```.txt```形式的license文件

如下的目录会在首次运行Pulsar时创建：

* data: ZooKeeper及BookKeeper的数据存储目录

* instances: 为[Pulsar Functions](https://pulsar.apache.org/docs/en/functions-overview)所创建的Artifacts

* logs: Logs created by the installation.

>Tip
>
>假如你想要使用内置的connectors以及tiered storage offloaders，你可以根据如下的指令来进行安装
>
> * [Install builtin connectors (optional)](https://pulsar.apache.org/docs/en/standalone/#install-builtin-connectors-optional)
>
> * [Install tiered storage offloaders (optional)](https://pulsar.apache.org/docs/en/standalone/#install-tiered-storage-offloaders-optional)
>
> 否则，你可以跳过相关的步骤，然后直接执行[Start Pulsar standalone](https://pulsar.apache.org/docs/en/standalone/#start-pulsar-standalone)步骤。在不安装内置connectors以及tiered storaged offloaders的情况下，Pulsar也可以成功的安装。






<br />
<br />

**[参考]**

1. [pulsar 官网](https://pulsar.apache.org/)

2. [pulsar 中文网](https://pulsar.apache.org/docs/zh-CN/next/concepts-architecture-overview/)

3. [pulsar 集群搭建](https://blog.csdn.net/weixin_33775572/article/details/92127055)


<br />
<br />
<br />

