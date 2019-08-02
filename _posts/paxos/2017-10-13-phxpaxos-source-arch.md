---
layout: post
title: phxpaxos源代码整体架构
tags:
- paxos
categories: paxos
description: phxpaxos原理
---


Paxos协议是分布式系统设计中的一个非常重要的协议，本文转载自[微信后台团队公众号团队所发表一系列Paxos的文章](https://mp.weixin.qq.com/s/WEi2kojApSP8PBupdP_8yw)，中间针对自己的理解略有修改或注释。在此处做一个备份，一方面为了加深对Paxos协议的理解，另一方面也方便自己的后续查找，防止文章丢失。


<!-- more -->

## 1. phxpaxos整体架构
通过对phxecho这个实例的分析，phxpaxos的整体抽象层次如下所示：

![paxos-arch-abstract](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_arch_abstract.jpg)

一个phxpaxos节点可以有多个Group，Group与Group之间完全隔离，不会进行任何通信。每一个Group可以挂载多个StateMachine。

phxpaxos的整体类图结构如下所示：

![paxos-class-diagram](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_class_diagram.jpg)

图中```PNode```代表着一个PhxPaxos节点，其拥有多个Group，并关联着DFNetwork对象。DFNetwork拥有m_oUDPRecv、m_oUDPSend、m_oTcpIOThread三个属性，分别控制着UDP数据包的接收、UDP数据包的发送，以及TCP数据的处理。EventLoop是整个TCP请求事件驱动器，MessageEvent代表着一个具体的TCP socket连接对象。

>phxpaxos中，对于TCPsocket的读写是完全分离的，一个socket连接要么只能读，要么只能写。


## 2. phxpaxos数据的接收与发送流程
本节我们将介绍phxpaxos中数据的接收与发送流程，中间暂时不会涉及到任何与业务相关的操作。

### 2.1 UDP数据包的发送
UDP数据包的发送是由```UDPSend```这个类来完成的。参看如下发送流程：

![paxos-udp-send](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_udp_send.jpg)

对于UDP数据的发送，整体流程较为简单，这里不再赘述。


### 2.2 UDP数据的接收
UDP数据包的接收主要由```UDPRecv```和```IOLoop```两个类来负责。参看如下接收流程：

![paxos-udp-receive](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_udp_receive.jpg)

两个类的分工很明确，UDPRecv类负责从socket里面获取UDP数据包，IOLoop管理着一个消息队列和定时器队列。UDPRecv获取到的数据包会被投送到IOLoop的消息队列中，以做后续的进一步处理。


### 2.3 TCP数据包的发送
对于TCP包的发送，主要由```TcpIOThread```来负责。但是TcpIOThread其实封装了如下三种对象：

* TcpAcceptor对象

* TcpRead对象数组

* TcpWrite对象数组

phxpaxos这样划分，可以使得每一个对象所完成的功能十分的纯粹。下面我们就简要来看一下针对TCP数据包的发送流程：

![paxos-tcp-write](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_tcp_write.jpg)

此处结构比较复杂，涉及到众多的对象，我们先来梳理一下这些对象之间的关系：

![paxos-tcp-write](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_tcp_arch.jpg)
通过上面的类图，我们可以很清楚的发现作者的意图： 一个TcpWrite对象关联着一个TcpClient以及一个EventLoop。其中EventLoop作为TCP读写事件的驱动器，不断的监听对应的socket上是否具有读写事件；TcpClient对象的作用是用于建立MessageEvent，为EventLoop提供源源不断的动力。

通过上面的分析，TCP数据包的发送过程就一目了然了。



### 2.4 TCP数据包的接收

对于TCP包的接收，因为一般先要接受来自对端的TCP连接，因此还涉及到TcpAccepter对象，连接建立后才能接收到对方发来的数据。如下我们给出接收流程：

![paxos-tcp-read](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_tcp_read.jpg)

在上面MessageEvent::OnRead()后面其实还有一长串流程：

![paxos-tcp-process](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_tcp_process.jpg)


到此为止，整个数据的发送与接收流程就已经介绍完毕。


## 3. phxpaxos的初始化
这里我们以phxecho为例来探讨一下phxpaxos的初始化过程，以进一步了解paxos的运行机理。

![paxos-startup](https://ivanzz1001.github.io/records/assets/img/paxos/paxos_startup.jpg)

group对象的初始化较为复杂，我们会在后面相关章节进行进一步的分析。

### 3.1 phxpaxos日志存储

默认情况下，我们会在phxecho程序运行的当前目录下创建```logpath_<ip>_<port>```文件夹，用于保存phxpaxos的操作日志。现在我们来大体看一下该文件夹的结构：
<pre>
# tree logpath_127.0.0.1_11111
logpath_127.0.0.1_11111
└── g0
    ├── 000015.log
    ├── 000016.ldb
    ├── CURRENT
    ├── LOCK
    ├── LOG
    ├── LOG.old
    ├── MANIFEST-000013
    └── vfile
        ├── 0.f
        ├── LOG
        └── meta

2 directories, 10 files
</pre>
我们可以看到，对于每一个group，有一个单独的目录来存放其oplog。里面leveldb主要用于存放```instanceID```、```SystemVariables```以及```MasterVariables```；而vfile文件夹会存放我们提交过的```proposal```信息，从而有我们整个操作的完整记录。

```vfile```文件夹下有如下三类文件：
{% highlight string %}
<x>.f文件： 用于记录我们Accept过的proposal。为了使文件不至于过大，通常本文件会以100MB作为分隔，<x>的编号随之递增，
           因此随着程序的运行，可能会有0.f、1.f、2.f等文件

LOG文件： 用于保存操作<x>.f文件时的时间等信息

meta文件： 其作为vfile文件夹下的一个元数据文件。因为vfile文件夹下可能会有众多文件0.f、1.f、2.f等，meta文件会用来记录
          当前我们要操作的是哪个文件，里面会记录我们要操作的文件编号，即<x>的值，并加了一个crc32的值对其进行校验，防止
          由于某些误操破坏了meta文件。
{% endhighlight %}
```<x>.f```文件中每条记录的格式如下所示：
{% highlight string %}
|-----------------------------------------------------
|  Length       |  InstanceID     |   message        |
| (4 bytes)     |    (8 bytes)    |                  |
|-----------------------------------------------------
{% endhighlight %}
其中Length字段保存的是InstanceID与message的总长度，即8+length(message)。上面message消息格式如下：
{% highlight string %}
message AcceptorStateData
{
	required uint64 InstanceID = 1;
	required uint64 PromiseID = 2;
	required uint64 PromiseNodeID = 3;
	required uint64 AcceptedID = 4;
	required uint64 AcceptedNodeID = 5;
	required bytes AcceptedValue = 6;
	required uint32 Checksum = 7;
}
{% endhighlight %}



<br />
<br />
**参看：**

1. [PhxPaxos源码分析之关于PhxPaxos](https://www.jianshu.com/p/9f1a874a39e5)

2. [PhxPaxos源码解析（1）之概述篇](https://blog.csdn.net/weixin_41713182/article/details/88147487)



<br />
<br />
<br />


