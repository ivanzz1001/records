---
layout: post
title: libpaxos库的安装
tags:
- paxos
categories: paxos
description: libpaxos库的安装
---

libpaxos库实现了paxos协议。在了解具体的paxos协议之前，我们先来编译安装libpaxos库。

<!-- more -->

参看文章：

* [搭建paxos测试过程](http://blog.csdn.net/zhaoforyou/article/details/53573407)
* [libpaxos官网](https://bitbucket.org/sciascid/libpaxos)


## 1. LibPaxos简要介绍
在这里我们要介绍的是LibPaxos3,它是对LibPaxos2的完全重写。LibPaxos3在如下方面得到了提高：

* 不适用组播
* 更清晰的设计
* 基于CMake的更好的构建系统
* 拥有单元测试用例

LibPaxos3被分割成两个库： libpaxos和libevpaxos。

LibPaxos(见```libpaxos/paxos```)实现了Paxos公开协议的内核，并没有夹杂网络部分的代码。libpaxos并不依赖与任何特定的网络库。

Libevpaxos(见```libpaxos/evpaxos```)是实际的网络Paxos的实现。该库构建在libpaxos及libevent上。

LibPaxos3需要需要依赖于libevent,msgpack。可选支持LMDB。

## 2. LibPaxos源代码编译


### 2.1 编译安装libevent
libevent是一个异步事件回调封装。支持epoll，select，poll等模式；支持定时器、IO、信号等事件。官网地址为：http://libevent.org/。

这里我们下载```libevent-2.0.22-stable.tar.gz ```版本，执行如下命令：
<pre>
wget https://github.com/libevent/libevent/releases/download/release-2.0.22-stable/libevent-2.0.22-stable.tar.gz

tar -zxvf libevent-2.0.22-stable.tar.gz

cd libevent-2.0.22-stable/

make

make install 
</pre>

 





<br />
<br />
<br />


