---
layout: post
title: 两种高效的IO处理模式
tags:
- cplusplus
categories: cplusplus
description: 两种高效的IO处理模式
---


服务器程序通常需要处理三类事件： IO事件、信号及定时器事件。这里我们不会具体讨论三种类型的事件，而是从整体上介绍一下两种高效的事件处理模式： Reactor模式和Proactor模式。

随着网络设计模式的兴起，Reactor和Proactor事件处理模式应运而生。同步IO模型通常用于实现Reactor模式，异步IO模型则用于实现Proactor模式。不过，其实我们也可以通过```多线程```等技术，使用同步IO模拟出Proactor模式。

平时接触的开源产品如Redis、ACE，事件模型都使用的Reactor模式；而同样做事件处理的Proactor，由于操作系统的原因，相关的开源产品也少；这里学习下其模型结构，重点对比下两者的异同点。




<!-- more -->


## 1. Reactor模式

### 1.1 reactor模式结构
如下是```Reactor```模式结构：

![cpp-reactor-model](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_reactor_model.jpg)

Reactor包含如下角色：

* **Handle句柄**： 用来标识socket连接或是打开文件；

* **Synchronous Event Demultiplexer**: 同步事件多路分解器，通常是由操作系统内核实现的一个函数(如select/epoll)。用于阻塞等待发生在句柄集合上的一个或多个事件。

* **Event Handler**: 事件处理接口

* **Concrete Event HandlerA**: 实现应用程序所提供的特定事件处理逻辑；

* **Reactor**： 反应器，定义一个接口，实现以下功能
<pre>
1. 供应用程序注册和删除关注的事件句柄；

2. 运行事件循环；

3. 有就绪事件到来时，分发事件到之前注册的回调函数上处理；
</pre>

关于**反应器**(reactor)名字中```反应```的由来：**'反应'** 即 **'倒置'**、**'控制逆转'**，具体事件处理程序不调用反应器，而是由反应器分配一个具体事件处理程序，具体事件处理程序对某个指定的事件做出反应。这种```控制逆转```又称为 '好莱坞法则'(不要调用我，让我来调用你）

### 1.2 业务流程及时序图

下面我们给出```Reactor```模式下相应的业务流程及时序图：

![cpp-reactor-timeflow](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_reactor_model.jpg)

1) 应用启动，将关注的事件handle注册到Reactor中；

2) 调用Reactor，进入无限事件循环，等待注册的事件到来；

3) 事件到来，select返回，Reactor将事件分发到之前注册的回调函数中处理；


## 2. Proactor模式
### 2.1 Proactor模式结构

如下是```Proactor```模式结构：

![cpp-proactor-model](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_proactor_model.jpg)

Proactor主动器模式包含如下角色：

* **Handle句柄**： 用来标识socket连接或是打开文件；

* **Asynchronous Operation Processor**: 异步操作处理器，负责执行异步操作，一般由操作系统内核实现；

* **Asynchronous Operation**: 异步操作

* **Completion Event Queue**: 完成事件队列。异步操作完成的结果放到队列中等待后续使用；

* **Proactor**: 主动器，为应用程序进程提供事件循环。从完成事件队列中取出异步操作的结果，分发调用相应的后续处理逻辑

* **Completion Handler**: 完成事件接口，一般是由回调函数组成的接口；

* **Concrete Completion Handler**: 完成事件处理逻辑；实现接口定义特定的应用处理逻辑。

### 2.2 业务流程及时序图

下面我们给出```Proactor```模式下相应的业务流程及时序图：

![cpp-proactor-timeflow](https://ivanzz1001.github.io/records/assets/img/cplusplus/cpp_proactor_timeflow.jpg)

1) 应用程序启动，调用异步操作处理器提供的异步操作接口函数，调用之后应用程序和异步操作处理就独立运行；应用程序可以调用新的异步操作，而其它操作可以并发进行。

2） 应用程序启动Proactor主动器，进行无限的事件循环，等待完成事件到来；

3） 异步操作处理器执行异步操作，完成后将结果放入到完成事件队列；

4） 主动器从完成事件队列中取出结果，分发到相应的完成事件回调函数处理逻辑中；


## 3. Reactor与Proactor的区别
Reactor模式与Proactor模式有很多方面的不同：

1） **主动与被动**

以主动写为例：
<pre>
Reactor将handle放到select()，等待可写就绪，然后调用write()写入数据。写完处理后续逻辑

Proactor调用aio_write()后立即返回，由内核负责写操作，写完后调用相应的回调函数处理后续逻辑；
</pre>

可以看出，Reactor被动的等待指示事件的到来并做出反应，它有一个等待的过程，做什么都要先放入到监听事件集合中等待handler可用时再进行操作；Proactor直接调用异步读写操作，调用后立即返回。
<pre>
Reactor处理的是就绪事件，而Proactor处理的是完成事件。
</pre>

2) **实现**

Reactor实现了一个被动的事件分离和分发模型，服务等待请求事件的到来，再通过不受间断的同步处理事件，从而做出反应；

Proactor实现了一个主动的事件分离和分发模型，这种设计允许多个任务并发的执行，从而提高吞吐量；并可执行耗时长的任务（各个任务间互不影响）

3） **优点**
<pre>
Reactor实现相对简单，对于耗时短的处理场景处理高效。
操作系统可以在多个事件源上等待，并且避免了多线程编程相关的性能开销和编程复杂性。
事件的串行化对应用是透明的，可以顺序的同步执行而不需要加锁。
事务分离： 将与应用无关的多路分解和分配机制和与应用相关的回调函数分离开来；

Proactor性能更高，能够处理耗时长的并发场景。
</pre>

4） **缺点**

Reactor处理耗时长的操作会造成事件分发的阻塞，影响到后续事件的处理；

Proactor实现逻辑复杂；依赖操作系统对异步的支持，目前实现了纯异步操作的操作系统少，实现优秀的如Window IOCP，但由于其windows系统用于服务器的局限性，目前应用范围较小； 而Unix/Linux系统对纯异步的支持有限，应用事件驱动的主流还是通过select/epoll来实现。

5） **适用场景**

Reactor: 同时接收多个服务请求，并且依次同步的处理它们的事件驱动程序；

Proactor: 异步接收和同时处理多个服务请求的事件驱动程序；






<br />
<br />

**[参看]**

1. [IO设计模式：Reactor和Proactor对比](https://www.cnblogs.com/me115/p/4452801.html)




<br />
<br />
<br />


