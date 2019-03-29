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










<br />
<br />

**[参看]**

1. [IO设计模式：Reactor和Proactor对比](https://www.cnblogs.com/me115/p/4452801.html)




<br />
<br />
<br />


